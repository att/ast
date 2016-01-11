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

/*	Squeeze a string.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 2/15/95
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
{	uchar*		delta;		/* output area		*/
	uchar*		tar;		/* target string	*/
	int		n_tar;
	K_DDECL(quick,recent,rhere);	/* address caches	*/
	int*		link;		/* base of elements	*/
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
reg int		copy;	/* best match if any	*/
int		n_copy;	/* length of match	*/
#endif
{
	reg int		n_add, i_add, i_copy, k_type;
	reg int		n, c_addr, best, d;
	uchar		buf[sizeof(long)+1];

	n_add = begs ? here-begs : 0;	/* add size		*/
	c_addr = here-tab->tar;		/* current address	*/
	k_type = 0;

	if(copy >= 0)	/* process the COPY instruction */
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

		STRPUTC(tab, i_add);
		if(!A_ISLOCAL(n_add) )
			STRPUTU(tab, (ulong)A_PUT(n_add), buf);
		STRWRITE(tab, begs, n_add);
	}

	if(n_copy > 0)
	{	if(!MERGABLE(n_add,n_copy,k_type))
			STRPUTC(tab, i_copy);

		if(!C_ISLOCAL(n_copy) )
			STRPUTU(tab, (ulong)C_PUT(n_copy), buf);

		if(k_type >= K_QUICK && k_type < (K_QUICK+K_QTYPE) )
			STRPUTC(tab, (uchar)best);
		else	STRPUTU(tab, (ulong)best, buf);
	}

	return 0;
}


/* Fold a string */
#if __STD_C
static int vdfold(Table_t* tab)
#else
static int vdfold(tab)
Table_t*	tab;
#endif
{
	reg ulong	key, n;
	reg uchar	*s, *sm, *ends, *ss, *heade;
	reg int		m, list, curm, bestm;
	reg uchar	*add, *endfold;
	reg int		head, len;
	reg int		size = tab->size;
	reg uchar	*tar = tab->tar;
	reg int		*link = tab->link, *hash = tab->hash;

	endfold = (s = tar) + tab->n_tar;
	curm = 0;
	if(tab->n_tar < M_MIN)
		return vdputinst(tab,s,endfold,-1,0);

	add = NIL(uchar*);
	bestm = -1;
	len = M_MIN-1;
	HINIT(key,s,n);
	for(;;)
	{	for(;;)	/* search for the longest match */
		{	if((m = hash[key&size]) < 0 )
				goto endsearch;
			list = m = link[m];	/* head of list */

			if(bestm >= 0) /* skip over past elements */
				for(n = bestm+len; m < n;)
					if((m = link[m]) == list)
						goto endsearch;

			head = len - (M_MIN-1); /* header before the match */
			heade = s+head;
			for(;;)
			{	
				if((n = m) < head)
					goto next;
				sm = tar + n;

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
				for(; ss < ends; ++ss, ++sm)
					if(*sm != *ss)
						goto extend;
				goto extend;

			next:	if((m = link[m]) == list )
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
		if(bestm >= 0)
		{	if(vdputinst(tab,add,s,bestm,len) < 0)
				return -1;

			/* add a sufficient number of suffices */
			ends = (s += len);
			ss = ends - (M_MIN-1);
			curm = (ss-tar);

			len = M_MIN-1;
			add = NIL(uchar*);
			bestm = -1;
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

		if(s > endfold-M_MIN)	/* too short to match */
		{	if(!add)
				add = s;
			break;
		}

		HNEXT(key,s,n);
	}

	return vdputinst(tab,add,endfold,-1,0);
}


#if __STD_C
int vdsqueeze(Void_t* target, reg int size, Void_t* delta)
#else
int vdsqueeze(target, size, delta)
Void_t*	target;
reg int	size;
Void_t*	delta;
#endif
{
	reg int		k, n;
	Table_t		tab;
	uchar		buf[sizeof(long)+1];

	if(size <= 0)
		return 0;
	else if(!target || !delta)
		return -1;

	tab.n_tar = size;
	tab.tar = (uchar*)target;
	tab.delta = (uchar*)delta;
	tab.link = NIL(int*);
	tab.hash = NIL(int*);

	/* space for the hash table */
	k = size;
	n = size/2;
	do (size = n); while((n &= n-1) != 0);
	if(size < 64)
		size = 64;
	k += size;

	if(!(tab.hash = (int*)malloc(k*sizeof(int))) )
		return -1;
	tab.link = tab.hash + size;
	tab.size = size-1;

	/* initialize table before processing */
	for(n = tab.size; n >= 0; --n)
		tab.hash[n] = -1;
	K_INIT(tab.quick,tab.recent,tab.rhere);

	STRPUTU(&tab,tab.n_tar,buf);
	n = vdfold(&tab);

	free((Void_t*)tab.hash);

	return n < 0 ? -1 : (tab.delta - (uchar*)delta);
}
