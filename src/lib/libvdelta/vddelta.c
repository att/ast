/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"vdelhdr.h"

/*	Compute a transformation that takes source data to target data
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 5/20/94
*/

#ifdef DEBUG
long	S_copy, S_add;	/* amount of input covered by COPY and ADD	*/
long	N_copy, N_add;	/* # of COPY and ADD instructions		*/
long	M_copy, M_add;	/* max size of a COPY or ADD instruction	*/
long	N_merge;	/* # of merged instructions			*/
#endif

#define MERGABLE(a,c,k) ((a) > 0 && A_ISTINY(a) && \
			 (c) > 0 && C_ISTINY(c) && \
			 (k) >= K_SELF )

typedef struct _table_s	Table_t;
struct _table_s
{	Vdio_t		io;		/* io structure		*/
	uchar*		src;		/* source string	*/
	int		n_src;
	uchar*		tar;		/* target string	*/
	int		n_tar;
	K_DDECL(quick,recent,rhere);	/* address caches	*/
	int*		link;		/* links of elements	*/
	int		size;		/* size of hash table	*/
	int*		hash;		/* hash table		*/
};

/* encode and output delta instructions */
#if __STD_C
static int vdputinst(Table_t* tab, uchar* begs, uchar* here, reg int copy, int n_copy)
#else
static int vdputinst(tab, begs, here, copy, n_copy)
Table_t*	tab;
uchar*		begs;	/* ADD data if any	*/
uchar*		here;	/* current location	*/
reg int		copy;	/* best match if >= 0	*/
int		n_copy;	/* length of match	*/
#endif
{
	reg int	n_add, i_add, i_copy, k_type;
	reg int	n, c_addr, best, d;

	n_add = begs ? here-begs : 0;		/* add size		*/
	c_addr = (here-tab->tar)+tab->n_src;	/* current address	*/
	k_type = 0;

	if(n_copy > 0)	/* process the COPY instruction */
	{	/**/DBTOTAL(N_copy,1); DBTOTAL(S_copy,n_copy); DBMAX(M_copy,n_copy);

		best = copy;
		k_type = K_SELF;
		if((d = c_addr - copy) < best)
		{	best = d;
			k_type = K_HERE;
		}
		for(n = 0; n < K_RTYPE; ++n)
		{	if((d = copy - tab->recent[n]) < 0 || d >= best)
				continue;
			best = d;
			k_type = K_RECENT+n;
		}
		if(best >= I_MORE && tab->quick[n = copy%K_QSIZE] == copy)
		{	for(d = K_QTYPE-1; d > 0; --d)
				if(n >= (d<<VD_BITS) )
					break;
			best = n - (d<<VD_BITS); /**/ASSERT(best < (1<<VD_BITS));
			k_type = K_QUICK+d;
		}

		/**/ASSERT(best >= 0);
		/**/ASSERT((k_type+K_MERGE) < (1<<I_BITS) );

		/* update address caches */
		K_UPDATE(tab->quick,tab->recent,tab->rhere,copy);

		/* see if mergable to last ADD instruction */
		if(MERGABLE(n_add,n_copy,k_type) )
		{	/**/DBTOTAL(N_merge,1);
			i_add = K_TPUT(k_type)|A_TPUT(n_add)|C_TPUT(n_copy);
		}
		else
		{	i_copy = K_PUT(k_type);
			if(C_ISLOCAL(n_copy) )
				i_copy |= C_LPUT(n_copy);
		}
	}

	if(n_add > 0)
	{	/**/DBTOTAL(N_add,1); DBTOTAL(S_add,n_add); DBMAX(M_add,n_add);

		if(!MERGABLE(n_add,n_copy,k_type) )
			i_add = A_ISLOCAL(n_add) ? A_LPUT(n_add) : 0;

		if(VDPUTC((Vdio_t*)tab,i_add) < 0 )
			return -1;
		if(!A_ISLOCAL(n_add) &&
		   (*_Vdputu)((Vdio_t*)tab, (ulong)A_PUT(n_add)) < 0 )
			return -1;
		if((*_Vdwrite)((Vdio_t*)tab, begs, n_add) < 0 )
			return -1;
	}

	if(n_copy > 0)
	{	if(!MERGABLE(n_add,n_copy,k_type) && VDPUTC((Vdio_t*)tab,i_copy) < 0 )
			return -1;

		if(!C_ISLOCAL(n_copy) &&
		   (*_Vdputu)((Vdio_t*)tab, (ulong)C_PUT(n_copy)) < 0 )
			return -1;

		if(k_type >= K_QUICK && k_type < (K_QUICK+K_QTYPE) )
		{	if(VDPUTC((Vdio_t*)tab,(uchar)best) < 0 )
				return -1;
		}
		else
		{	if((*_Vdputu)((Vdio_t*)tab, (ulong)best) < 0 )
				return -1;
		}
	}

	return 0;
}


/* Fold a string */
#if __STD_C
static int vdfold(Table_t* tab, reg uchar* fold, reg uchar* endfold, int target)
#else
static int vdfold(tab, fold, endfold, target)
Table_t*	tab;
reg uchar*	fold;		/* start of area to fold	*/
reg uchar*	endfold;	/* end of area to fold		*/
int		target;		/* 1 if doing the target stream	*/
#endif
{
	reg ulong	key, n;
	reg uchar	*sm, *ends, *ss, *endh;
	reg int		m, list, curm, bestm;
	reg uchar	*add;
	reg int		head, len, n_src = tab->n_src;
	reg uchar	*src = tab->src, *tar = tab->tar;
	reg int		size = tab->size, *link = tab->link, *hash = tab->hash;

	if((endfold-fold) < M_MIN)	/* not much to do */
	{	add = fold;
		goto done;
	}

	if(target)
		curm = (fold - tab->tar) + n_src;
	else	curm = (fold - tab->src);
	bestm = -1;
	len = M_MIN-1;
	add = NIL(uchar*);
	HINIT(key,fold,n);
	for(;;)
	{	for(;;)	/* search for the longest match */
		{	if((m = hash[key&size]) < 0)
				goto endsearch;
			list = m = link[m];	/* head of list */

			if(bestm >= 0) /* skip over past elements */
				for(n = bestm+len; m < n;)
					if((m = link[m]) == list)
						goto endsearch;

			head = len - (M_MIN-1); /* header before the match */
			endh = fold+head;
			for(;;)
			{	if((n = m) < n_src)
				{	if(n < head)
						goto next;
					sm = src + n;
				}
				else
				{	if((n -= n_src) < head)
						goto next;
					sm = tar + n;
				}

				/* make sure that the M_MIN bytes match */
				if(!EQUAL(endh,sm))
					goto next;

				/* make sure this is a real match */
				for(sm -= head, ss = fold; ss < endh; )
					if(*sm++ != *ss++)
						goto next;

				/* extend forward as much as possible */
				ss += M_MIN;
				sm += M_MIN;
				ends = endfold;
				if(m < n_src && (n = (src+n_src)-sm) < (ends-fold) )
					ends = fold+n;
				for(;; ++ss, ++sm)
					if(ss >= ends || *sm != *ss)
						goto extend;

			next:	if((m = link[m]) == list )
					goto endsearch;
			}

		extend: bestm = m-head;
			n = len;
			len = ss-fold;
			if(ss >= endfold)	/* already match everything */
				goto endsearch;

			/* check for a longer match */
			ss -= M_MIN-1;
			if(len == n+1)
				HNEXT(key,ss,n);
			else	HINIT(key,ss,n);
		}

	endsearch:
		if(bestm >= 0)
		{	if(target && vdputinst(tab,add,fold,bestm,len) < 0)
				return -1;

			/* add a sufficient number of suffices */
			ends = (fold += len);
			ss = ends - (M_MIN-1);
			if(target)
				curm = n_src + (ss-tar);
			else	curm = (ss-src);

			len = M_MIN-1;
			add = NIL(uchar*);
			bestm = -1;
		}
		else
		{	if(!add)
				add = fold;
			ss = fold;
			ends = (fold += 1);	/* add one prefix */
		}

		if(ends > (endfold - (M_MIN-1)) )
			ends = endfold - (M_MIN-1);

		if(ss < ends) for(;;)	/* add prefices/suffices */
		{	n = key&size;
			if((m = hash[n]) < 0 )
				link[curm] = curm;
			else
			{	link[curm] = link[m];
				link[m] = curm;
			}
			hash[n] = curm++;

			if((ss += 1) >= ends)
				break;
			HNEXT(key,ss,n);
		}

		if((endfold-fold) < M_MIN)	/* too short to match */
		{	if(!add)
				add = fold;
			goto done;
		}

		HNEXT(key,fold,n);
	}

done:
	if(target)
	{	if((len = (tab->tar+tab->n_tar) - endfold) > 0 ) /* match at end */
			bestm = n_src - len;
		else	bestm = -1;
		if(add || bestm >= 0)
			return vdputinst(tab,add,endfold,bestm,len);
	}

	return 0;
}

#if __STD_C
static int vdprocess(Table_t* tab)
#else
static int vdprocess(tab)
Table_t*	tab;
#endif
{
	reg uchar	*tar, *src, *endsrc, *endtar, *endt;
	reg int		hn, tn, n_src, n_tar;

	/* check boundary conditions */
	src = tab->src;
	tar = tab->tar;
	if((n_tar = tab->n_tar) <= 0)
		return 0;
	if((n_src = tab->n_src) <= 0)
		return vdfold(tab,tar,tar+n_tar,1);

	/* see if there is a large enough match at the start */
#define LARGE_MATCH	(8*M_MIN)
	endtar = tar + (n_tar < n_src ? n_tar : n_src);
	for(; tar < endtar; ++tar, ++src)
		if(*tar != *src)
			break;
	if((hn = tar - tab->tar) < LARGE_MATCH )
	{	tar = tab->tar;
		src = tab->src;
		hn  = 0;
	}
	else
	{	if(vdputinst(tab,NIL(uchar*),tab->tar,0,hn) < 0)
			return -1;
		if(hn == n_tar)
			return 0;
	}

	/* see if there is a large enough match at the end */
	endtar = tab->tar + n_tar - 1;
	endsrc = tab->src + n_src - 1;
	endt = endtar - (n_tar < n_src ? n_tar : n_src);
	if(endt < tar-1)
		endt = tar-1;
	for(; endtar > endt; --endtar, --endsrc)
		if(*endtar != *endsrc)
			break;
	endtar += 1; endsrc += 1;

	if((tn = (tab->tar+n_tar) - endtar) < LARGE_MATCH)
	{	endtar = tab->tar+n_tar;
		endsrc = tab->src+n_src;
		tn = 0;
	}

	/* maintain big enough data for matching the remained */
	if((endtar-tar) >= (n_tar/4))
	{	src -= hn/4;
		endsrc += tn/4;
	}
	if(vdfold(tab,src,endsrc,0) < 0)
		return -1;

	return vdfold(tab,tar,endtar,1);
}


#if __STD_C
long vddelta(Vddisc_t* source, Vddisc_t* target, Vddisc_t* delta)
#else
long vddelta(source, target, delta)
Vddisc_t*	source;	/* source data			*/
Vddisc_t*	target;	/* target data			*/
Vddisc_t*	delta;	/* transform output data	*/
#endif
{
	reg int		size, k, n;
	reg long	p, n_src, n_tar, window;
	Table_t		tab;

	if(!target || (n_tar = target->size) < 0)
		return -1;
	if(n_tar > 0 && !target->data && !target->readf)
		return -1;

	if((n_src = source ? source->size : 0) < 0)
		n_src = 0;
	if(n_src > 0 && !source->data && !source->readf)
		return -1;

	if(!delta || (!delta->data && !delta->writef) )
		return -1;

	tab.n_src = tab.n_tar = tab.size = 0;
	tab.tar = tab.src = NIL(uchar*);
	tab.link = NIL(int*);
	tab.hash = NIL(int*);
	INIT(&tab.io,delta);

	/* try to allocate working space */
	window = DFLTWINDOW;
	while(window > 0)
	{	/* space for the target string */
		if((long)(size = (int)window) > n_tar)
			size = (int)n_tar;
		if(!target->data && !(tab.tar = (uchar*)malloc(size*sizeof(uchar))) )
			goto reduce_window;
		k = size;

		/* space for sliding header or source string */
		if(n_src <= 0)	/* compression only */
		{	if((long)window >= n_tar)
			{	size = 0;
				k += n_tar;
			}
			else
			{	size = (int)HEADER(window);
				k += size;
			}
		}
		else		/* differencing */
		{	if((long)(size = (int)window) > n_src)
				size = (int)n_src;
			k += size;
			if(source->data)
				size = 0;
		}
		if(size > 0 && !(tab.src = (uchar*)malloc(size*sizeof(uchar))) )
			goto reduce_window;

		/* space for the hash table itself */
		n = k/2;
		do (size = n); while((n &= n-1) != 0);
		if(size < 64)
			size = 64;
		k += size;

		if(!(tab.hash = (int*)malloc(k*sizeof(int))) )
			goto reduce_window;

		/* successful */
		tab.link = tab.hash+size;
		tab.size = size-1;
		break;

	reduce_window:
		if(tab.tar)
		{	free((Void_t*)tab.tar);
			tab.tar = NIL(uchar*);
		}
		if(tab.src)
		{	free((Void_t*)tab.src);
			tab.src = NIL(uchar*);
		}
		if((window >>= 1) <= 0)
			return -1;
	}

	/* amount processed */
	n = 0;

	/* output magic bytes and sizes */
	k = sizeof(VD_MAGIC) - 1;
	if((*_Vdwrite)(&tab.io,(uchar*)VD_MAGIC,k) != k ||
	   (*_Vdputu)(&tab.io,(ulong)n_tar) <= 0 ||
	   (*_Vdputu)(&tab.io,(ulong)n_src) <= 0 ||
	   (*_Vdputu)(&tab.io,(ulong)window) <= 0 )
		goto done;

	/* do one window at a time */
	while(n < n_tar)
	{	/* prepare the source string */
		if(n_src <= 0)	/* data compression */
		{	if(n <= 0)
				tab.n_src = 0;
			else
			{	size = (int)HEADER(window);
				if(target->data)
					tab.src = tab.tar + tab.n_tar - size;
				else	memcpy((Void_t*)tab.src,
				   	       (Void_t*)(tab.tar + tab.n_tar - size),
					       size );
				tab.n_src = size;
			}
		}
		else	/* data differencing */
		{	if(n < n_src)
			{	if(window >= n_src)
					p = 0;
				else if((n+window) > n_src)
					p = n_src-window;
				else	p = n;
				if((size = n_src-p) > (int)window)
					size = (int)window;
				if(source->data)
					tab.src = (uchar*)source->data + p;
				else if((*source->readf)(tab.src,size,p,source) != size)
					goto done;
				tab.n_src = size;
			}
			/* else use last window */
		}

		/* prepare the target string */
		if((size = (int)(n_tar-n)) > (int)window)
			size = (int)window;
		tab.n_tar = size;
		if(target->data)
			tab.tar = (uchar*)target->data + n;
		else
		{	size = (*target->readf)(tab.tar, size, (long)n, target);
			if((long)size != tab.n_tar)
				goto done;
		}

		/* reinitialize table before processing */
		for(k = tab.size; k >= 0; --k)
			tab.hash[k] = -1;
		K_INIT(tab.quick,tab.recent,tab.rhere);

		if(vdprocess(&tab) < 0)
			goto done;

		n += size;
	}

done:
	(void)(*_Vdflsbuf)(&tab.io);

	if(!target->data && tab.tar)
		free((Void_t*)tab.tar);
	if(tab.src && ((n_src <= 0 && !target->data) || (n_src > 0 && !source->data) ) )
		free((Void_t*)tab.src);
	if(tab.hash)
		free((Void_t*)tab.hash);

	return tab.io.here + (tab.io.next - tab.io.data);
}
