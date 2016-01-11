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
#include	"vdelhdr01.h"

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

typedef struct _match_s	Match_t;
typedef struct _table_s	Table_t;
struct _match_s
{	Match_t*	next;		/* linked list ptr	*/
};
struct _table_s
{	Vdio_t		io;		/* io structure		*/
	uchar*		src;		/* source string	*/
	int		n_src;
	uchar*		tar;		/* target string	*/
	int		n_tar;
	K_DDECL(quick,recent,rhere);	/* address caches	*/
	Match_t*	base;		/* base of elements	*/
	int		size;		/* size of hash table	*/
	Match_t**	table;		/* hash table		*/
};

/* encode and output delta instructions */
#if __STD_C
static int vdputinst(Table_t* tab, uchar* begs, uchar* here, Match_t* match, int n_copy)
#else
static int vdputinst(tab, begs, here, match, n_copy)
Table_t*	tab;
uchar*		begs;	/* ADD data if any	*/
uchar*		here;	/* current location	*/
Match_t*	match;	/* best match if any	*/
int		n_copy;	/* length of match	*/
#endif
{
	reg int	n_add, i_add, i_copy, k_type;
	reg int	n, c_addr, copy, best, d;

	n_add = begs ? here-begs : 0;		/* add size		*/
	c_addr = (here-tab->tar)+tab->n_src;	/* current address	*/
	k_type = 0;

	if(match)	/* process the COPY instruction */
	{	/**/DBTOTAL(N_copy,1); DBTOTAL(S_copy,n_copy); DBMAX(M_copy,n_copy);

		best = copy = match - tab->base;
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
	else
	{	if((*_Vdflsbuf)((Vdio_t*)tab) < 0)
			return -1;
	}

	return 0;
}


/* Fold a string */
#if __STD_C
static int vdfold(Table_t* tab, int output)
#else
static int vdfold(tab, output)
Table_t*	tab;
int		output;
#endif
{
	reg ulong	key, n;
	reg uchar	*s, *sm, *ends, *ss, *heade;
	reg Match_t	*m, *list, *curm, *bestm;
	reg uchar	*add, *endfold;
	reg int		head, len, n_src = tab->n_src;
	reg int		size = tab->size;
	reg uchar	*src = tab->src, *tar = tab->tar;
	reg Match_t	*base = tab->base, **table = tab->table;

	if(!output)
	{	if(tab->n_src < M_MIN)
			return 0;
		endfold = (s = src) + tab->n_src;
		curm = base;
	}
	else
	{	endfold = (s = tar) + tab->n_tar;
		curm = base+n_src;
		if(tab->n_tar < M_MIN)
			return vdputinst(tab,s,endfold,NIL(Match_t*),0);
	}

	add = NIL(uchar*);
	bestm = NIL(Match_t*);
	len = M_MIN-1;
	HINIT(key,s,n);
	for(;;)
	{	for(;;)	/* search for the longest match */
		{	if(!(m = table[key&size]) )
				goto endsearch;
			list = m = m->next;	/* head of list */

			if(bestm) /* skip over past elements */
			{	for(;;)
				{	if(m >= bestm+len)
						break;
					if((m = m->next) == list)
						goto endsearch;
				}
			}

			head = len - (M_MIN-1); /* header before the match */
			heade = s+head;
			for(;;)
			{	if((n = m-base) < n_src)
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
				if(!EQUAL(heade,sm))
					goto next;

				/* make sure this is a real match */
				for(sm -= head, ss = s; ss < heade; )
					if(*sm++ != *ss++)
						goto next;
				ss += M_MIN;
				sm += M_MIN;
				ends = endfold;
				if((m-base) < n_src && (n = (src+n_src)-sm) < (ends-s) )
					ends = s+n;
				for(; ss < ends; ++ss, ++sm)
					if(*sm != *ss)
						goto extend;
				goto extend;

			next:	if((m = m->next) == list )
					goto endsearch;
			}

		extend: bestm = m-head;
			n = len;
			len = ss-s;
			if(ss >= endfold)	/* already match everything */
				goto endsearch;

			/* check for a longer match */
			ss -= M_MIN-1;
			if(len == n+1)
				HNEXT(key,ss,n);
			else	HINIT(key,ss,n);
		}

	endsearch:
		if(bestm)
		{	if(output && vdputinst(tab,add,s,bestm,len) < 0)
				return -1;

			/* add a sufficient number of suffices */
			ends = (s += len);
			ss = ends - (M_MIN-1);
			if(!output)
				curm = base + (ss-src);
			else	curm = base + n_src + (ss-tar);

			len = M_MIN-1;
			add = NIL(uchar*);
			bestm = NIL(Match_t*);
		}
		else
		{	if(!add)
				add = s;
			ss = s;
			ends = (s += 1);	/* add one prefix */
		}

		if(ends > (endfold - (M_MIN-1)) )
			ends = endfold - (M_MIN-1);

		if(ss < ends) for(;;)	/* add prefices/suffices */
		{	n = key&size;
			if(!(m = table[n]) )
				curm->next = curm;
			else
			{	curm->next = m->next;
				m->next = curm;
			}
			table[n] = curm++;

			if((ss += 1) >= ends)
				break;
			HNEXT(key,ss,n);
		}

		if(s > endfold-M_MIN)	/* too short to match */
		{	if(!add)
				add = s;
			break;
		}

		HNEXT(key,s,n);
	}

	if(output)	/* flush output */
		return vdputinst(tab,add,endfold,NIL(Match_t*),0);
	return 0;
}


#if __STD_C
long _vddelta_01(Vddisc_t* source, Vddisc_t* target, Vddisc_t* delta, long window)
#else
long _vddelta_01(source, target, delta, window)
Vddisc_t*	source;	/* source data			*/
Vddisc_t*	target;	/* target data			*/
Vddisc_t*	delta;	/* transform output data	*/
long		window;	/* amount to process each time	*/
#endif
{
	reg int		size, k, n;
	reg long	p, n_src, n_tar;
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
	tab.base = NIL(Match_t*);
	tab.table = NIL(Match_t**);
	INIT(&tab.io,delta);

	if(window <= 0)
		window = DFLTWINDOW;
	else if(window > MAXWINDOW)
		window = MAXWINDOW;
	if(window > n_tar)
		window = n_tar;
	if(n_src > 0 && window > n_src)
		window = n_src;

	/* try to allocate working space */
	while(window > 0)
	{	/* space for the target string */
		size = (n_tar == 0 || target->data) ? 0 : (int)window;
		if((long)size > n_tar)
			size = (int)n_tar;
		if(size > 0 && !(tab.tar = (uchar*)malloc(size*sizeof(uchar))) )
			goto reduce_window;

		/* space for sliding header or source string */
		if(n_src <= 0)	/* compression only */
			size = target->data ? 0 : HEADER(window);
		else		/* differencing */
		{	size = source->data ? 0 : window;
			if((long)size > n_src)
				size = (int)n_src;
		}
		if(size > 0 && !(tab.src = (uchar*)malloc(size*sizeof(uchar))) )
			goto reduce_window;

		/* space for the hash table elements */
		size = (int)(window < n_tar ? window : n_tar);
		if(n_src <= 0)
			size += (int)(window < n_tar ? HEADER(window) : 0);
		else	size += (int)(window < n_src ? window : n_src);
		if(!(tab.base = (Match_t*)malloc(size*sizeof(Match_t))) )
			goto reduce_window;

		/* space for the hash table */
		n = size/2;
		do (size = n); while((n &= n-1) != 0);
		if(size < 64)
			size = 64;
		while(!(tab.table = (Match_t**)malloc(size*sizeof(Match_t*))) )
			if((size >>= 1) <= 0)
				goto reduce_window;

		/* if get here, successful */
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
		if(tab.base)
		{	free((Void_t*)tab.base);
			tab.base = NIL(Match_t*);
		}
		if((window >>= 1) <= 0)
			return -1;
	}

	/* amount processed */
	n = 0;

	/* output magic bytes and sizes */
	for(k = 0; VD_MAGIC[k]; k++)
		;
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
			{	if(n+window > n_src)
					p = n_src-window;
				else	p = n;
				if(source->data)
					tab.src = (uchar*)source->data + p;
				else
				{	size = (*source->readf)
						(tab.src, (int)window, p, source);
					if((long)size != window)
						goto done;
				}
			} /* else use last window */

			tab.n_src = window;
		}

		/* prepare the target string */
		size = (int)((n_tar-n) < window ? (n_tar-n) : window);
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
			tab.table[k] = NIL(Match_t*);
		K_INIT(tab.quick,tab.recent,tab.rhere);

		if(tab.n_src > 0 && vdfold(&tab,0) < 0)
			goto done;
		if(vdfold(&tab,1) < 0)
			goto done;

		n += size;
	}

done:
	if(!target->data && tab.tar)
		free((Void_t*)tab.tar);
	if(tab.src && ((n_src <= 0 && !target->data) || (n_src > 0 && !source->data) ) )
		free((Void_t*)tab.src);
	if(tab.base)
		free((Void_t*)tab.base);
	if(tab.table)
		free((Void_t*)tab.table);

	return tab.io.here + (tab.io.next - tab.io.data);
}
