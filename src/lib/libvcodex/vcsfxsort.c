/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
#include	"vchdr.h"
#include	"vgraph.h"

/* Algorithm to compute the suffix array of a string.
** Sadakane-Larson magic numbering is used to speed up sort.
** Optimum branching is used to determine an optimal order
** 	for sorting groups with same first byte.
** Seward pointer-copy is used to spread sort order.
** 
** Written by Kiem-Phong Vo
*/

#ifdef ITOH_TANAKA
#define has_algorithm	1
#endif
#if !has_algorithm
#define OPT_BRANCHING	1
#endif

#define SWAP(x,y,t)	((t) = (x), (x) = (y), (y) = (t)) /* swap two scalars	*/
#define MEDIAN(x,y,z)	((x) <= (y) ? ((y) <= (z) ? (y) : (z) >= (x) ? (z) : (x)) : \
				      ((y) >= (z) ? (y) : (z) <= (x) ? (z) : (x)) )

#define INSERTSORT	64	/* insertion sort any set <= this	*/

#ifdef DEBUG
static Vcinx_t	Cn, Cz, Gn, Gz;
extern char*		getenv(const char*);

static int chktodo(Vcsfx_t* sfx, Vcinx_t lo, Vcinx_t hi)
{	static int	Dblev = -1;

	if(Dblev < 0)
	{	char *db = getenv("VCDEBUG");
		Dblev = !db ? 0 : db[0] - '0';
	}

	if(lo > hi)
		return 0;
	if(Dblev == 0) /* no checking*/
		return 0;
	if(Dblev == 1) /* checking only full range */
		return (lo == 0 && hi == sfx->nstr-1) ? 1 : 0;
	return 1; /* always checking */
}

/* make sure that the magic numbering is still ok */
static int intcmp(void* one, void* two, void* disc)
{	return (int)(*((Vcinx_t*)one) - *((Vcinx_t*)two));
}
static int chkdata(Vcsfx_t* sfx, Vcinx_t lo, Vcinx_t hi)
{	Vcinx_t	i, endi, k;
	Vcinx_t	*idx = sfx->idx, *inv = sfx->inv, nstr = sfx->nstr;
	Vcinx_t	*ary;

	if(lo > hi || !chktodo(sfx,lo,hi))
		return 1;

	if(!(ary = (Vcinx_t*)malloc((hi-lo+1)*sizeof(Vcinx_t))) )
		return 0;
	for(k = 0, i = lo; i <= hi; ++i, ++k)
		ary[k] = idx[i];
	vcqsort(ary, k, sizeof(Vcinx_t), intcmp, (Void_t*)0);
	for(i = 1; i < k; ++i) /* check distinctness */
		/**/DEBUG_ASSERT(ary[i] != ary[i-1]);
	free(ary);

	for(i = lo; i <= hi; i = k) /* check inversion relations */
	{	endi = inv[idx[i]]; /**/DEBUG_ASSERT(endi >= i);
		for(k = i; k <= endi; ++k)
			/**/DEBUG_ASSERT(inv[idx[k]] == endi);
	}

	return 1;
}

/* make sure that all suffixes in [lo,hi] are properly sorted */
static int cmpsfx(Vcsfx_t* sfx, Vcinx_t p, Vcinx_t s)
{	Vcchar_t	*ps = sfx->str+p, *ss = sfx->str+s;
	Vcinx_t		v, n, k;

	p = sfx->nstr-p; s = sfx->nstr-s;
	for(n = p < s ? p : s, k = 0; k < n; ++k)
	{	if((v = ps[k] - ss[k]) == 0)
			continue;
		if(v > 0)
			sfprintf(sfstdout, "cmpsfx: ps=%d > ss=%d at k=%d\n",
				ps-sfx->str, ss-sfx->str, k);
		return v > 0 ? 1 : -1;
	}
	if(p < s)
		sfprintf(sfstdout, "cmpsfx: ps=%d > ss=%d because np=%d < ns=%d\n",
			ps-sfx->str, ss-sfx->str, p, s);

	return p < s ? 1 : -1;
}
static int chksorted(Vcsfx_t* sfx, Vcinx_t lo, Vcinx_t hi)
{	Vcinx_t	*i, *inv = sfx->inv, *idx = sfx->idx;
	Vcinx_t	*min = idx+lo, *max = idx+hi;

	for(i = min; i <= max; ++i)
		/**/DEBUG_ASSERT(inv[*i] == i-idx);
	min = min == sfx->idx ? min+1 : min;
	max = max < (sfx->idx+sfx->nstr-1) ? max+1 : max;
	for(; min <= max; ++min)
		/**/DEBUG_ASSERT(cmpsfx(sfx, min[-1], min[0]) < 0);

	return 1;
}

/* make sure that list[] is a one-to-one function */
static int chkone2one(Vcinx_t* list, Vcinx_t size)
{	Vcinx_t		i;
	Vcchar_t	*chk;

	if(!(chk = (Vcchar_t*)calloc(size,1)) )
		return 0;
	for(i = 0; i < size; ++i) /* see if list[] is one-to-one */
	{	/**/DEBUG_ASSERT(list[i] >= 0 && list[i] < size);
		chk[list[i]] += 1;
		/**/DEBUG_ASSERT(chk[list[i]] <= 1);
	}
	free(chk);

	return 1;
}

/* make sure that idx[] and inv[] are inverses of one another */
static int chkinverse(Vcinx_t* idx, Vcinx_t* inv, Vcinx_t size)
{	Vcinx_t	i;

	for(i = 0; i < size; ++i) /* check inverse functions */
		/**/DEBUG_ASSERT(inv[idx[i]] == i);

	return 1;
}

/* make sure that all headers of length "len" are the same */
static int chkheader(Vcsfx_t* sfx, Vcinx_t* lo, Vcinx_t* hi, Vcinx_t len)
{
	for(lo += 1; lo <= hi; ++lo)
		/**/DEBUG_ASSERT(memcmp(sfx->str + lo[-1], sfx->str + lo[0], len) == 0);
}

/* make sure that [min,le) and (re,max] have been completely sorted */
static int chkbounds(Vcsfx_t* sfx, Vcinx_t* min, Vcinx_t* max, Vcinx_t* le, Vcinx_t* re)
{
	Vcinx_t	*idx = sfx->idx, *inv = sfx->inv;

	for(; min < le; ++min)
		/**/DEBUG_ASSERT(inv[*min] == (min-idx) && idx[inv[*min]] == *min);
	for(; max > re; --max)
		/**/DEBUG_ASSERT(inv[*max] == (max-idx) && idx[inv[*max]] == *max);

	return 1;
}
#endif /*DEBUG*/

/* bucket sort by first two bytes. */
#define GMIN(g,x,y)	((g)[((x)<<8)+(y)])
#define GMAX(g,x,y,e)	(((g)[((x)<<8)+(y)+1]) - (((x) == (e) && (y) == 255) ? 1 : 0) - 1)

#if __STD_C 
static void sfxbsort(Vcsfx_t* sfx, Vcinx_t* grp)
#else
static void sfxbsort(sfx, grp)
Vcsfx_t*	sfx;
Vcinx_t*	grp;
#endif
{
	int		i, j, v;
	Vcinx_t		n, *g;
	Vcchar_t	*s, *ends;
	Vcinx_t		*idx = sfx->idx, *inv = sfx->inv;

	memset(grp, 0, 256*256*sizeof(Vcinx_t)); /* get frequencies */
	for(ends = (s = sfx->str)+sfx->nstr-1, i = *s; s < ends; i = j)
		grp[(i<<8) + (j = *++s)] += 1;

	for(n = 0, g = grp, i = 0; i < 256; ++i)
	{	for(j = 0; j < 256; ++j, ++g)
			*g = (n += *g); /* next area to be allocated */
		if(i == *ends) /* sort the last suffix now */
		{	inv[sfx->nstr-1] = n;
			idx[n++] = sfx->nstr-1;
		}
	}

	for(n = 0, ends = (s = sfx->str)+sfx->nstr-1, i = *s; s < ends; i = j)
	{	v = (i << 8) + (j = *++s);
		inv[n++] = grp[v]-1; /* rank is highest position of group */
	}
	for(n = 0, ends = (s = sfx->str)+sfx->nstr-1, i = *s; s < ends; i = j)
	{	v = (i << 8) + (j = *++s);
		idx[grp[v] -= 1] = n++; /* put suffix into its bucket */
	}

	grp[256*256] = sfx->nstr; /* end of 255*255-group */

	/**/DEBUG_ASSERT(chkone2one(sfx->idx, sfx->nstr));
}

#if __STD_C /* swap two adjacent lists of integers */
static void sfxswap(Vcinx_t* min, Vcinx_t* mid, Vcinx_t* max)
#else
static void sfxswap(min, mid, max)
Vcinx_t*	min;
Vcinx_t*	mid;
Vcinx_t*	max;
#endif
{
	Vcinx_t	m, n;

	if((n = max-mid) > (m = mid-min+1) )
		n = m;
	for(; n > 0; --n, ++min, --max)
		SWAP(*min, *max, m);
}

#if __STD_C /* multikey-quicksort */
static void sfxqsort(Vcsfx_t* sfx, Vcinx_t* min, Vcinx_t* max, Vcinx_t hdr)
#else
static void sfxqsort(sfx, min, max, hdr)
Vcsfx_t*	sfx;
Vcinx_t*	min;	/* sorting [min,max] 	*/
Vcinx_t*	max;
Vcinx_t		hdr;	/* common header 	*/
#endif
{
	Vcinx_t	*l, *r, *le, *re, k, ln, mn, rn, pi, sz, c[3];
	Vcinx_t	*inv = sfx->inv, *idx = sfx->idx;

q_sort:	if((sz = max-min+1) <= INSERTSORT) /* insertion sort */
	{	for(l = min+1; l <= max; ++l) /* insert *l into [0,l-1] */
		{	pi = *l;
			for(r = l-1; r >= min; --r)
			{	le = inv + (pi+hdr); re = inv + (*r+hdr); /**/DEBUG_ASSERT(le != re);
				while((k = *le - *re) == 0)
 					{ le += 2; re += 2; }
				if(k > 0)
					break;
				*(r+1) = *r;
			}
			*(r+1) = pi;
		}
		for(l = min; l <= max; ++l)
			inv[*l] = l-idx; /* update rank */
		return;
	}

	/* approximating a median-of-9 as a pivot */
	for(k = 0; k < 3; ++k)
	{	ln = inv[min[VCRAND()%sz]+hdr];
		mn = inv[min[VCRAND()%sz]+hdr];
		rn = inv[min[VCRAND()%sz]+hdr];
		c[k] = MEDIAN(ln, mn, rn);
	}
	pi = MEDIAN(c[0],c[1],c[2]);

	/* partition into three parts: <pi, ==pi, >pi */
	for(le = (l = min)-1, re = (r = max)+1; l <= r; )
	{	for(; l <= r; ++l)
		{	if((k = inv[*l + hdr] - pi) > 0)
				break;
			else if(k == 0 && (le += 1) < l)
				SWAP(*le, *l, mn);
		}
		for(; r >= l; --r)
		{	if((k = inv[*r + hdr] - pi) < 0)
				break;
			else if(k == 0 && (re -= 1) > r)
				SWAP(*re, *r, mn);
		}
		if(l < r)
		{	SWAP(*l, *r, mn);
			l += 1; r -= 1;
		}
	} /**/DEBUG_ASSERT(r+1 == l);
	if(le >= min && le < r) /* swap [min,le]=pi && [le+1,r]<pi */
		sfxswap(min, le, r);
	if(l < re && re <= max) /* swap [l,re-1]>pi && [re,max]=pi */
		sfxswap(l, re-1, max);
	le = min + (r-le); re = max - (re-l);
	ln = le-min; mn = re-le+1; rn = max-re; /* partition sizes */
	/**/DEBUG_ASSERT(ln >= 0 && mn >= 0 && rn >= 0 && (ln+rn+mn) == sz );

	/* sort largest part by tail recursion for O(log(n)) stack depth */
	if(ln >= mn && ln >= rn) /* left part has max size */
	{	/* set group rank before sorting larger elements */
		if((pi = (le-1) - idx) != inv[*min])
			for(l = min; l < le; ++l)
				inv[*l] = pi;

		if(mn > 0) /* the middle part with a longer header */
		{	if(mn == 1)
				inv[*re] = re - idx;
			else	sfxqsort(sfx, le, re, hdr+2);
		}

		if(rn > 0) /* sort the right part */
		{	if(rn == 1)
				inv[*max] = max-idx;
			else	sfxqsort(sfx, re+1, max, hdr);
		}

		max = le-1; /* tail recursion */
		goto q_sort;
	}
	else if(mn >= ln && mn >= rn) /* middle part has max size */
	{	if(ln > 0) /* sort the left part */
		{	if(ln == 1)
				inv[*min] = min-idx;
			else	sfxqsort(sfx, min, le-1, hdr);
		}

		if((pi = re - idx) != inv[*le]) /* set rank of middle part */
			for(l = le; l <= re; ++l)
				inv[*l] = pi;

		if(rn > 0) /* sort the right part */
		{	if(rn == 1)
				inv[*max] = max-idx;
			else	sfxqsort(sfx, re+1, max, hdr);
		}

		hdr += 2; /* current known common prefix length */

		/**/DEBUG_ASSERT(hdr%2 == 0);
		rn = hdr/2; /* see if periodic with period hdr/2 */
		rn = (inv[*le + rn] == pi || inv[*re + rn] == pi) ? rn : 0;

		if(rn > 0)
		{	min = le; max = re; /* compute bounds for copying positions */
			for(l = le; l <= re; ++l)
			{	if((r = idx + inv[*l + rn]) < min)
					min = r;
				else if(r > max)
					max = r;
			}
		}
		if(rn > 0 && (le-min) > 0) /* make sure left side is all sorted */
		{	for(l = min > idx ? min-1 : min; l < le; ++l)
				if(inv[*l] != l-idx)
					break;
			rn = l < le ? 0 : rn;
		}
		if(rn > 0 && (max-re) > 0) /* make sure right side is all sorted */
		{	for(r = re+1; r <= max; ++r)
				if(inv[*r] != r-idx)
					break;
			rn = r <= max ? 0 : rn;
		}

		if(rn > 0)
		{	/**/DEBUG_DECLARE(Vcinx_t, n_sz = re-le+1) DEBUG_DECLARE(Vcinx_t, n_cp = 0)
			/**/DEBUG_COUNT(Gn); DEBUG_TALLY(1, Gz, n_sz);

			/* the below two theorems are needed for copying to work */
			/**/DEBUG_ASSERT(chkheader(sfx, min, max, rn));
			/**/DEBUG_ASSERT(chkbounds(sfx, min, max, le, re));
		
			for(l = min; l < le; ++l) /* copy-sort from the left side */
				if(inv[k = *l - rn] == pi)
					{ *le = k; inv[k] = le-idx; le += 1; /**/DEBUG_INCREASE(n_cp); }

			for(r = max; r > re; --r) /* copy-sort from the right side */
				if(inv[k = *r - rn] == pi)
					{ *re = k; inv[k] = re-idx; re -= 1; /**/DEBUG_INCREASE(n_cp); }

			/**/DEBUG_ASSERT(n_cp == n_sz);
			/**/DEBUG_ASSERT(chkone2one(sfx->idx, sfx->nstr));
		}
		else /* tail recursion */
		{	min = le; max = re;
			goto q_sort;
		}
	}
	else /* if(rn >= ln && rn >= mn) - right part has max size */
	{	if(ln > 0) /* sort the left part */
		{	if(ln == 1)
				inv[*min] = min-idx;
			else	sfxqsort(sfx, min, le-1, hdr);
		}

		if(mn > 0) /* the middle part with a longer header */
		{	if(mn == 1)
				inv[*re] = re - idx;
			else	sfxqsort(sfx, le, re, hdr+2);
		}

		min = re+1; /* tail recursion to sort the right part */
		goto q_sort;
	}
}

#if __STD_C /* copy-sort yx-groups based on the sort order of x-suffixes */
static void sfxcsort(Vcsfx_t* sfx, Vcinx_t y, Vcinx_t* grp)
#else
static void sfxcsort(sfx, y, grp)
Vcsfx_t*	sfx;
Vcinx_t		y;	/* common first byte	*/
Vcinx_t*	grp;	/* bounds of xy-groups	*/
#endif
{
	Vcinx_t		i, k, l, r, x, *xy;
	Vcinx_t		omin, omax, pmin[256], pmax[256];
	Vcchar_t	*str = sfx->str;
	Vcinx_t		*inv = sfx->inv, *idx = sfx->idx;
	Vcinx_t		endc = sfx->str[sfx->nstr-1];

	/* bounds of group headed by the same y */
	if((omin = grp[y<<8]) > (omax = grp[(y<<8)+256]-1) )
		return;

	/* set boundary of groups to be copy-sorted */
	for(k = 0, x = 0; x < 256; ++x)
	{	l = GMIN(grp,x,y); r = GMAX(grp,x,y,endc);
		if(l < r && l < (r = inv[idx[l]]) )
			k += 1; /* nontrivial sortable group */
		else if(x != y) /* already sorted */
			l = r = -1;
		else if(l > r) /* empty yy-group */
			l = r = omax+1; /* run first loop below only */
		/*else; non-empty yy-group, run both loops below */

		pmin[x] = l; pmax[x] = r;
	}
	if(k == 0) /* nothing to do */
		return;

	/**/DEBUG_COUNT(Cn); DEBUG_TALLY(1,Cz,omax-omin+1);
	for(l = omin; l < pmin[y]; ++l)
		if((k = idx[l]-1) >= 0 && (i = *(xy = pmin+str[k])) >= 0)
			{ *xy += 1; idx[i] = k; inv[k] = i; }
	for(r = omax; r > pmax[y]; --r)
		if((k = idx[r]-1) >= 0 && (i = *(xy = pmax+str[k])) >= 0)
			{ *xy -= 1; idx[i] = k; inv[k] = i; }

	/**/DEBUG_ASSERT(1 || chkone2one(sfx->idx, sfx->nstr));
	/**/DEBUG_ASSERT(chkdata(sfx, omin, omax));
}

#if __STD_C /* sort the unsorted xy-groups for the same x */
static int sfxosort(Vcsfx_t* sfx, Vcinx_t x, Vcinx_t* grp, int dir)
#else
static int sfxosort(sfx, x, grp, dir)
Vcsfx_t*	sfx;
Vcinx_t	x;	/* common header byte	*/
Vcinx_t*	grp;
int		dir;	/* itoh-tanaka sort dir	*/
#endif
{
	Vcinx_t		l, r, k, i, y, z;
	Graph_t		*gr;
	Grnode_t	*hd, *tl, *nd;
	Gredge_t	*e;
	Vcchar_t	*str = sfx->str, endc = sfx->str[sfx->nstr-1];
	Vcinx_t		nstr = sfx->nstr, *inv = sfx->inv, *idx = sfx->idx;
	int		rv = -1;

	/* construct graph of relations between xy-groups */
	if(!(gr = gropen(NIL(Grdisc_t*), GR_DIRECTED)) )
		goto done;
	for(y = 0; y < 256; ++y)
	{	if(y == x || (dir > 0 && y < x) || (dir < 0 && y > x) )
			continue;
		if((l = GMIN(grp,x,y)) >= (r = GMAX(grp,x,y,endc)) )
			continue; /* single or xx-group */
		if((r = inv[idx[l]]-l+1) <= 1)
			continue; /* already sorted */

		if(!(hd = grnode(gr, TYPECAST(Void_t*,y), 1)) )
			goto done;
		for(k = 0; k < 3; ++k)
		{	if((i = idx[l + VCRAND()%r]+2) >= nstr-1 || str[i] != x )
				continue;
			if((z = str[i+1]) == x || (dir > 0 && z < x) || (dir < 0 && z > x) )
				continue;
			if(!(tl = grnode(gr, TYPECAST(Void_t*,z), 1)) ||
			   !(e  = gredge(gr, tl, hd, (Void_t*)0, 1)) )
				goto done;
			grbrweight(e, grbrweight(e,-1)+1);
		}
	}
	if((k = grbrgreedy(gr)) < 0)
		goto done;

	/* now sort xy-groups in their top-sort order */
	hd = tl = NIL(Grnode_t*); /* first build the list of root nodes */
	for(nd = (Grnode_t*)dtfirst(gr->nodes); nd; nd = (Grnode_t*)dtnext(gr->nodes,nd))
	{	if(!nd->iedge) /* no incoming edge, hence a root */
		{	if(!hd)
				(hd = tl = nd)->link = NIL(Grnode_t*);
			else	(tl = tl->link = nd)->link = NIL(Grnode_t*);
		}
	}
	for(nd = hd; nd; nd = nd->link) /* top-sort */
	{	for(e = nd->oedge; e; e = e->onext)
			(tl = tl->link = e->head)->link = NIL(Grnode_t*);
		l = grp[(x<<8) + TYPECAST(Vcinx_t,nd->label)]; r = inv[idx[l]];
		sfxqsort(sfx, idx+l, idx+r, 2);
		/**/DEBUG_ASSERT(chkdata(sfx, l, r));
	}

	rv = 0; /* successful processing */
	/**/DEBUG_ASSERT(1 || chkone2one(sfx->idx, sfx->nstr));

done:	if(gr)
		grclose(gr);
	return rv;
}

#if __STD_C
Vcsfx_t* vcsfxsort(const Void_t* astr, ssize_t nstr)
#else
Vcsfx_t* vcsfxsort(astr, nstr)
Void_t*	astr;	/* string to be sorted	*/
ssize_t	nstr;	/* length of string	*/
#endif
{
	Vcinx_t		l, r, x, y, endc;
	Vcinx_t		grp[256*256+1];
	Vcchar_t	*str; /* the addressable astr	*/
	Vcinx_t		*idx, *inv; /* index and rank	*/
	Vcsfx_t		*sfx; /* suffix array structure	*/
	int		error = 1;

	if(!(str = (Vcchar_t*)astr) || nstr <= 0)
		{ str = NIL(Vcchar_t*); nstr = 0; }

	if(!(sfx = (Vcsfx_t*)malloc(sizeof(Vcsfx_t)+2*(nstr+1)*sizeof(Vcinx_t))) )
		return NIL(Vcsfx_t*);
	sfx->idx = idx = (Vcinx_t*)(sfx+1);
	sfx->inv = inv = idx+nstr+1;
	sfx->str = str;
	sfx->nstr = (Vcinx_t)nstr;
	idx[sfx->nstr] = inv[sfx->nstr] = sfx->nstr; /* the infinite eos byte */
	if(sfx->nstr <= 1) /* the easy sorting case */
	{	idx[0] = inv[0] = 0;
		return sfx;
	}

	sfxbsort(sfx, grp); /* sort suffixes by first 2 bytes */
	endc = str[nstr-1];

#ifdef ITOH_TANAKA /* sfxqsort half the data using itoh-tanaka strategy */
{	Vcinx_t	c, d;

	c = d = 0;
	for(x = 0; x < 256; ++x)
	for(y = 0; y < 256; ++y)
	{	if(x == y || (l = GMIN(grp,x,y)) >= (r = GMAX(grp,x,y,endc)) )
			continue;
		if(x < y)
			c += r-l+1;
		else	d += r-l+1;
	}
	d = c <= d ? 1 : -1; /* sort direction */

	for(x = d > 0 ? 0 : 255; x >= 0 && x <= 255; x += d)
	{	if(grp[x<<8] > (grp[(x<<8)+256]-1) )
			continue;
		if(sfxosort(sfx, x, grp, d) < 0)
			goto it_err;
		sfxcsort(sfx, x, grp);
	}
	error = 0; /* have done well */
it_err: ;
}
#endif

#ifdef OPT_BRANCHING
{	/* use an optimum branching to determine sort order of x-groups */
	Graph_t		*gr = NIL(Graph_t*);
	Grnode_t	*tl, *hd, *nd;
	Gredge_t	*e;

	if(!(gr = gropen(NIL(Grdisc_t*), GR_DIRECTED)) ) 
		goto ob_err;
	for(x = 0; x < 256; ++x)
	{	if(grp[x<<8] > (grp[(x<<8)+256]-1) )
			continue;
		if(!(hd = grnode(gr, TYPECAST(Void_t*,x), 1)) )
			goto ob_err;
		for(y = 0; y < 256; ++y)
		{	if(x == y || (l = GMIN(grp,x,y)) >= (r = GMAX(grp,x,y,endc)) )
				continue;
			if(!(tl = grnode(gr, TYPECAST(Void_t*,y), 1)) ||
			   !(e  = gredge(gr, tl, hd, (Void_t*)0, 1)) )
				goto ob_err;
			grbrweight(e, r-l+1); 
		}
	}
	if((r = grbrgreedy(gr)) < 0)
		goto ob_err; /**/DEBUG_PRINT(2,"Branching weight=%d\n", r);

	hd = tl = NIL(Grnode_t*); /* first build the list of root nodes */
	for(nd = (Grnode_t*)dtfirst(gr->nodes); nd; nd = (Grnode_t*)dtnext(gr->nodes,nd))
	{	if(!nd->iedge) /* no incoming edge, hence a root */
		{	if(!hd)
				(hd = tl = nd)->link = NIL(Grnode_t*);
			else	(tl = tl->link = nd)->link = NIL(Grnode_t*);
		}
	} /**/DEBUG_ASSERT(hd && tl);
	for(nd = hd; nd; nd = nd->link) /* now process in top-sort order */
	{	for(e = nd->oedge; e; e = e->onext)
			(tl = tl->link = e->head)->link = NIL(Grnode_t*);
		x = TYPECAST(Vcinx_t,nd->label);
		if(sfxosort(sfx, x, grp, 0) < 0)
			goto ob_err;
		sfxcsort(sfx, x, grp); /* copy-sort */
	}
	error = 0; /* have done well */
ob_err:	if(gr)
		grclose(gr);
}
#endif

	/**/DEBUG_PRINT(2,"Cn=%d,",Cn); DEBUG_PRINT(2,"Cz=%d\n",Cz);
	/**/DEBUG_PRINT(2,"Gn=%d,",Gn); DEBUG_PRINT(2,"Gz=%d\n",Gz);
	/**/DEBUG_ASSERT(chkdata(sfx,0,sfx->nstr-1) );
	/**/DEBUG_ASSERT(chksorted(sfx,0,sfx->nstr-1) );
	/**/DEBUG_ASSERT(chkone2one(sfx->idx,sfx->nstr) );
	/**/DEBUG_ASSERT(chkinverse(sfx->idx, sfx->inv, sfx->nstr) );

	if(error)
	{	free(sfx);
		sfx = NIL(Vcsfx_t*);
	}

	return sfx;
}
