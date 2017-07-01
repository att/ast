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

/* Generalized block-move parsing of a pair of strings.
**
** Written by Kiem-Phong Vo
*/

#define HMIN		3
#define MMIN(vcpa)	((vcpa)->mmin >= HMIN ? (vcpa)->mmin : HMIN)

#define MAXPAIR		256 /* max # of extended matching pairs	*/
typedef struct _pair_s
{	ssize_t		size;		/* # of pairs found	*/
	Vclzmatch_t	mtch[MAXPAIR];	/* storage for pairs	*/
} Pair_t;		
#define NEXTPAIR(pr)	((pr)->size < MAXPAIR-1 ? ((pr)->mtch + (pr)->size++) : NIL(Vclzmatch_t*))

#define ALPHA		5 /* multiplier for hashing polynomial	*/
typedef struct _obj_s
{	struct _obj_s*	next;
} Obj_t;
typedef struct _hash_s
{	Vchash_t	mask;	/* mask to get hash table index	*/
	Obj_t**		htab;	/* hash table of size (mask+1)	*/
	Obj_t*		src;	/* objects representing source	*/
	Obj_t*		tar;	/* objects representing target	*/
	Vchash_t	coef;	/* max polynomial coefficient	*/
	ssize_t		poly;	/* number of polynomial terms 	*/
} Hash_t;
#define ENTRY(hs,ky)	((hs)->htab + ((ssize_t)((ky) & (hs)->mask)) )
#define INSERT(e,ob)	((ob)->next = *(e), *(e) = (ob) )
#define DELETE(e,p,ob)	((p) ? ((p)->next = (ob)->next) : (*(e) = (ob)->next) )

#if __STD_C
static Hash_t* hashtable(ssize_t mmin, ssize_t nsrc, ssize_t ntar)
#else
static Hash_t* hashtable(mmin, nsrc, ntar)
ssize_t	mmin;	/* segment size to be hashed	*/
ssize_t nsrc;	/* length of source data	*/
ssize_t ntar;	/* length of target data	*/
#endif
{	ssize_t	n, size;
	Hash_t	*hs;

	/* compute size to allocate */
	for(n = 1<<10; n < (nsrc+ntar)/4; n *= 2) /* size of hash table */
		;
	size = sizeof(Hash_t) + n*sizeof(Obj_t*) + (nsrc+ntar)*sizeof(Obj_t);

	if(!(hs = (Hash_t*)calloc(1, size)) )
		return NIL(Hash_t*);

	hs->htab = (Obj_t**)(hs+1);
	hs->src  = (Obj_t*)(hs->htab + n);
	hs->tar  = hs->src + nsrc;
	hs->mask = n - 1;
	
	/* hashing polynomial data */
	hs->poly = (mmin = mmin >= HMIN ? mmin : HMIN);
	for(hs->coef = 1, n = 1; n < mmin; ++n)
		hs->coef *= ALPHA;

	return hs;
}

#if __STD_C
static Vchash_t fkhash(Hash_t* hs, Vcchar_t* ks, Vcchar_t* cm)
#else
static Vchash_t fkhash(hs, ks, cm)
Hash_t*		hs;
Vcchar_t*	ks;	/* string to compute hash key	*/
Vcchar_t*	cm;	/* byte mapping before matching	*/
#endif
{	int		k;
	Vchash_t	ky;

	ky = cm ? cm[ks[0]] : ks[0];
	for(k = 1; k < hs->poly; ++k)
		ky = ky*ALPHA + (cm ? cm[ks[k]] : ks[k]);

	return ky;
}
#define fknext(cf,ks,ns,ky)	(((ky) - (ks)[0]*(cf) )*ALPHA + (ns)[0] )
#define fknextm(cf,ks,ns,ky,cm)	(((ky) - (cm)[(ks)[0]]*(cf) )*ALPHA + (cm)[(ns)[0]] )

#if __STD_C
static Vchash_t rkhash(Hash_t* hs, Vcchar_t* ks, Vcchar_t* cm)
#else
static Vchash_t rkhash(hs, ks, cm)
Hash_t*		hs;
Vcchar_t*	ks;
Vcchar_t*	cm;
#endif
{	int		k;
	Vchash_t	ky;

	ky = cm ? cm[ks[hs->poly - 1]] : ks[hs->poly - 1];
	for(k = hs->poly-2; k >= 0; --k)
		ky = ky*ALPHA + (cm ? cm[ks[k]] : ks[k]);

	return ky;
}

/* extend to match nearby segments */
#if __STD_C
static ssize_t extend(Vclzparse_t* vcpa, Pair_t* pr, int type)
#else
static ssize_t extend(vcpa, pr, type)
Vclzparse_t*	vcpa;
Pair_t*		pr;
int		type;
#endif
{
	ssize_t		m, em, n, en, u, mmin, umax, mbeg;
	Vcchar_t	*ms, *ts, *et, *mstr;
	Vclzmatch_t	*mt = pr->mtch + pr->size - 1;
	Vcchar_t	*cmap = ((vcpa->type&type&VCLZ_MAP) && vcpa->cmap) ? vcpa->cmap : 0;

	/* amounts of matches required and unmatches allowed */
	mmin = vcpa->mmin <= 1 ? 1 : vcpa->mmin;
	umax = mmin*(2*vclogi(mmin) + 1);

	if((m = mt->mpos - vcpa->nsrc) >= 0 )
	{	mstr = vcpa->tar; /* matching in target data */
		mbeg = vcpa->nsrc; /* origin of data */
		ms = mstr + m; /* start of match */
		n = vcpa->ntar - m; /* amount of matchable */
	}
	else
	{	mstr = vcpa->src; /* matching in source data */
		mbeg = 0;
		ms = mstr + mt->mpos;
		n = vcpa->nsrc - mt->mpos;
	}

	/* bounds of matchable data */
	ts = vcpa->tar + (m = mt->tpos - vcpa->nsrc);
	et = ts + (n <= (m = vcpa->ntar-m) ? n : m);

	type &= VCLZ_REVERSE;
	ts += mt->size; ms += (type ? -mt->size : mt->size);
	for(; ts < et; ts += n, ms += (type ? -n : n))
	{	for(u = mmin, n = 0, en = et-ts; n < en; )
		{	/* count unmatches */
			em = (em = n+u) > en ? en : em;
			for(m = n; m < em; ++m)
				if(ms[type ? -m : m] == (cmap ? cmap[ts[m]] : ts[m]))
					break;
			if((u -= (m-n)) <= 0)
				goto done;

			/* count matches */
			for(n = m, m = n+1; m < en; ++m)
				if(ms[type ? -m : m] != (cmap ? cmap[ts[m]] : ts[m]))
					break;
			if((m-n) < mmin) /* not big enough */
			{	if((u += m-n) >= umax)
					goto done;
				n = m;
			}
			else if(!(mt = NEXTPAIR(pr)) )
				goto done;
			else
			{	mt->tpos = (ts - vcpa->tar) + vcpa->nsrc + n;
				mt->mpos = (ms - mstr) + mbeg + (type ? -n : n);
				mt->size = m - n;
				n = m;
				break;
			}
		}
	}

done:	mt = pr->mtch+pr->size-1;
	return (mt->tpos + mt->size) - pr->mtch->tpos;
}

#if __STD_C
static int hashparse(Vclzparse_t* vcpa, ssize_t prune)
#else
static int hashparse(vcpa, prune)
Vclzparse_t*	vcpa;
ssize_t		prune;	/* prune target by this amount	*/
#endif
{
	Vcchar_t	*ks, *ns, *ts, *et, *sm, *em;
	Obj_t		*m, *p, *obj, *add, *endo, **ht;
	ssize_t		n, tp;
	Vchash_t	fk, fm, ky;
	Obj_t		*fpos, *rpos;
	ssize_t		flen, ftyp, rlen, rtyp;
	Pair_t		fpair, rpair, *pair;
	Hash_t		*hs;
	Vchash_t	coef;
	int		reverse = vcpa->type&VCLZ_REVERSE;
	Vcchar_t	*cmap = ((vcpa->type&VCLZ_MAP) && vcpa->cmap) ? vcpa->cmap : NIL(Vcchar_t*);
	ssize_t		mmin = MMIN(vcpa); /* minimum match length */
	/**/DEBUG_ASSERT(vcpa->ntar >= mmin);

	/* initialize hash table */
	if(!(hs = hashtable(mmin, vcpa->nsrc, vcpa->ntar)))
		return -1;
	mmin = hs->poly;
	coef = hs->coef;

	if(vcpa->nsrc >= hs->poly) /* add source data into hash table */
	{	fk = fkhash(hs, vcpa->src, NIL(Vcchar_t*)); ky = fk+1;
		ns = (ks = vcpa->src) + mmin;
		for(endo = (m = hs->src) + vcpa->nsrc - (mmin-1); ; ++ks, ++ns )
		{	if(fk != ky )
				{ ht = ENTRY(hs, fk); INSERT(ht, m); }
			if((m += 1) >= endo)
				break;
			ky = fk; fk = fknext(coef, ks, ns, fk);
		}
	}

	ns = (ks = vcpa->tar) + mmin; et = ks + vcpa->ntar; /* bounds of target data */
	fk = fkhash(hs, ks, NIL(Vcchar_t*));
	fm = cmap ? fkhash(hs, ks, cmap) : 0;
	for(add = obj = hs->tar, endo = obj + vcpa->ntar - (mmin-1); ; )
	{	/**/DEBUG_ASSERT(fkhash(hs, ks, 0) == fk);
		/**/DEBUG_ASSERT(!cmap || fkhash(hs, ks, cmap) == fm);

		fpos = rpos = NIL(Obj_t*); /* match position */
		flen = rlen = vcpa->mmin - 1; /* match length */
		ftyp = rtyp = 0; /* match type */

		/* forward: matching twice for plain and mapped data */
		for(tp = 0; tp <= VCLZ_MAP; tp += VCLZ_MAP)
		{	if(tp == 0) /* plain matching */
				ky = fk;
			else if(!cmap)
				continue;
			else	ky = fm;

			ht = ENTRY(hs, ky);
			if(prune > 0) /* prune target data outside of search window */
			{	for(p = NIL(Obj_t*), m = *ht; m && m >= hs->tar; )
				{	if((obj - m) > prune)
						{ DELETE(ht,p,m); m = p ? p->next : *ht; }
					else	{ p = m; m = m->next; }
				}
			}

			for(m = *ht; m; m = m->next)
			{	ts = ks; /* starting point of string */

				if(m >= hs->tar) /* possible target match */
				{	sm = vcpa->tar + (m - hs->tar);
					em = vcpa->tar + vcpa->ntar;
				}
				else /* possible source match */
				{	sm = vcpa->src + (m - hs->src);
					em = vcpa->src + vcpa->nsrc;
				}
				if((em-sm) > (et-ts) ) /* matchable bound */
					em = sm + (et-ts);
				if((em-sm) <= flen )
					continue;

				if(tp == 0) /* match without mapping */
				{	if(sm[flen] != ts[flen] || sm[flen>>1] != ts[flen>>1] )
						continue;
					for(; sm < em; ++sm, ++ts)
						if(*sm != *ts)
							break;
				}
				else /* match with mapping */
				{	if(sm[flen] != cmap[ts[flen]] || 
					   sm[flen>>1] != cmap[ts[flen>>1]] )
						continue;
					for(; sm < em; ++sm, ++ts)
						if(*sm != cmap[*ts])
							break;
				}

				if((n = ts - ks) > flen)
				{	fpos = m;
					flen = n;
					ftyp = tp;
				}
			}
		}
		if(flen >= vcpa->mmin)
		{	n = 0;
			if(add < obj)	
			{	fpair.mtch[n].tpos = add - hs->src;
				fpair.mtch[n].mpos = -1;
				fpair.mtch[n].size = obj - add;
				n += 1;
			}
			fpair.mtch[n].tpos = obj - hs->src;
			fpair.mtch[n].mpos = fpos - hs->src;
			fpair.mtch[n].size = flen;
			fpair.size = n+1;
			flen = extend(vcpa, &fpair, ftyp);
		}

		/* reverse matching */
		for(tp = 0; reverse && tp <= VCLZ_MAP; tp += VCLZ_MAP)
		{	if(tp == 0)
				ky = rkhash(hs, ks, 0);
			else if(!cmap)
				continue;
			else	ky = rkhash(hs, ks, cmap);

			ht = ENTRY(hs, ky );
			if(prune > 0) /* prune target data outside of search window */
			{	for(p = NIL(Obj_t*), m = *ht; m && m >= hs->tar; )
				{	if((obj - m) > prune)
						{ DELETE(ht,p,m); m = p ? p->next : *ht; }
					else	{ p = m; m = m->next; }
				}
			}

			for(m = *ht; m; m = m->next)
			{	ts = ks; /* starting point of string */

				if(m >= hs->tar) /* possible target match */
				{	sm = vcpa->tar + ((m + mmin-1) - hs->tar);
					em = vcpa->tar;
					if(sm >= ts)
						continue;
				}
				else /* possible source match */
				{	sm = vcpa->src + ((m + mmin-1) - hs->src);
					em = vcpa->src;
				}
				if((sm-em) > (et-ts) ) /* matchable bound */
					em = sm - (et-ts);
				if((sm-em+1) <= rlen )
					continue;

				if(tp == 0) /* match without cmapping */
				{	if(sm[-rlen] != ts[rlen] || sm[-(rlen>>1)] != ts[rlen>>1] )
						continue;	
					for(; sm >= em; --sm, ++ts)
						if(*sm != *ts)
							break;
				}
				else /* match with cmapping */
				{	if(sm[-rlen] != cmap[ts[rlen]] ||
					   sm[-(rlen>>1)] != cmap[ts[rlen>>1]] )
						continue;	
					for(; sm >= em; --sm, ++ts)
						if(*sm != cmap[*ts])
							break;
				}

				if((n = ts - ks) > rlen)
				{	rpos = m + (mmin-1);
					rlen = n;
					rtyp = tp;
				}
			}
		}
		if(rlen >= vcpa->mmin)
		{	n = 0;
			if(add < obj)	
			{	rpair.mtch[n].tpos = add - hs->src;
				rpair.mtch[n].mpos = -1;
				rpair.mtch[n].size = obj - add;
				n += 1;
			}
			rpair.mtch[n].tpos = obj - hs->src;
			rpair.mtch[n].mpos = rpos - hs->src;
			rpair.mtch[n].size = rlen;
			rpair.size = n+1;
			rlen = extend(vcpa, &rpair, rtyp|VCLZ_REVERSE);
		}

		if(flen >= rlen)
			{ n = flen; pair = &fpair; tp = ftyp; }
		else	{ n = rlen; pair = &rpair; tp = rtyp|VCLZ_REVERSE; }

		if(n >= vcpa->mmin) /* a potential match */
		{	if((n = (*vcpa->parsef)(vcpa, tp, pair->mtch, pair->size)) < 0)
				goto done;
			else if(n == 0) /* treat everything as unmatched */
				goto nope;
			else if((add += n) >= endo)
				break;

			for(ky = fk+1;;) /* add parsed data to htab */
			{	if(fk != ky )
					{ ht = ENTRY(hs, fk); INSERT(ht, obj); }
				ky = fk;
				fk = fknext(coef, ks, ns, fk);
				fm = cmap ? fknextm(coef, ks, ns, fm, cmap) : 0;
				ks += 1; ns += 1;
				if((obj += 1) >= add)
					break;
			}
		}
		else 
		{ nope:	ht = ENTRY(hs, fk); INSERT(ht, obj);
			if((obj += 1) >= endo)
				break;
			fk = fknext(coef, ks, ns, fk);
			fm = cmap ? fknextm(coef, ks, ns, fm, cmap) : 0;
			ks += 1; ns += 1;
		}
	}

	if((n = (hs->tar + vcpa->ntar) - add) > 0 )
	{	fpair.mtch[0].tpos = add - hs->src;
		fpair.mtch[0].mpos = -1;
		fpair.mtch[0].size = n;
		n = (*vcpa->parsef)(vcpa, 0, fpair.mtch, 1);
	}

done:	free(hs);
	return n >= 0 ? 0 : -1;
}

#if __STD_C
static int sfxparse(Vclzparse_t* vcpa)
#else
static int sfxparse(vcpa)
Vclzparse_t*	vcpa;
#endif
{
	Vcinx_t		p, r, ad;
	Vcinx_t		lp, lz, rp, rz, savp, savlp, savlz;
	Vcinx_t		nsrc, nstr;
	Vcinx_t		*inv, *idx;
	Vcchar_t	*s1, *s2, *ends, *str = NIL(Vcchar_t*);
	Pair_t		pair;
	Vcsfx_t		*sfx = NIL(Vcsfx_t*);
	Vcinx_t		mmin = MMIN(vcpa);
	int		rv = -1;

	/* catenate source and target strings into a superstring */
	if((nsrc = vcpa->nsrc) == 0)
	{	nstr = vcpa->ntar;
		str = vcpa->tar;
	}
	else if(vcpa->tar == (vcpa->src + nsrc) )
	{	nstr = nsrc + vcpa->ntar;
		str = vcpa->src;
	}
	else
	{	nstr = nsrc + vcpa->ntar;
		if(!(str = (Vcchar_t*)malloc(nstr)) )
			return -1;
		memcpy(str, vcpa->src, nsrc);
		memcpy(str+nsrc, vcpa->tar, vcpa->ntar);
	}
	ends = str+nstr;

	if(!(sfx = vcsfxsort(str,nstr)) ) /* compute suffix array */
		goto done;
	idx = sfx->idx; inv = sfx->inv;

	for(savlz = savlp = savp = 0, ad = p = nsrc; p < nstr; )
	{	for(lz = lp = 0, r = inv[p]-1; r >= 0; --r)
		{	if((lp = idx[r]) < p) /* best left match */
			{	s1 = str+p; s2 = str+lp;
				for(; s1 < ends && *s1 == *s2; ++s1, ++s2 )
					lz += 1;
				if(lp < nsrc && (lp+lz) > nsrc)
					if((lz = nsrc-lp) < mmin )
						lp = lz = 0;
				break;
			}
		}

		for(rz = rp = 0, r = inv[p]+1; r < nstr; ++r)
		{	if((rp = idx[r]) < p) /* best right match */
			{	s1 = str+p; s2 = str+rp;
				for(; s1 < ends && *s1 == *s2; ++s1, ++s2 )
					rz += 1;
				if(rp < nsrc && (rp+rz) > nsrc)
					if((rz = nsrc-rp) < mmin )
						rp = rz = 0;
				break;
			}
		}

		if(rz > lz || (rz == lz && rp > lp) )
			{ lp = rp; lz = rz; } /* best match over all */
			
#define SEARCH	4 /* neighborhood to search for an improved match */ 
		if(savlz == 0 || (p-savp) < SEARCH)
		{	if(lz > savlz )
			{	savlp = lp; savlz = lz; savp = p;
				p += 1; /* improve! go again */
				continue;
			}
			if(savlz > 0 && lz >= (savlz-2) )
			{	p += 1; /* not a bad short, go again */
				continue;
			}
		}
		if(savlz > 0) /* no more improvement, use current best */
		{	lz = savlz; lp = savlp; p = savp;
			savlz = savlp = savp = 0;
		}

		if(lz >= mmin)
		{	r = 0;
			if(ad < p)
			{	pair.mtch[r].tpos = ad;
				pair.mtch[r].mpos = -1;
				pair.mtch[r].size = p-ad;
				r += 1;
			}
			pair.mtch[r].tpos = p;
			pair.mtch[r].mpos = lp;
			pair.mtch[r].size = lz;
			pair.size = r+1;
			extend(vcpa, &pair, 0);
			if((r = (*vcpa->parsef)(vcpa, 0, pair.mtch, pair.size)) < 0 )
				goto done;
			else if(r == 0)
				goto nope;
			else	ad += r; /* advance by amount processed */
		}
		else
		{ nope: p += 1; /* skip over an unmatched position */
		}
	} /**/DEBUG_ASSERT(p == nstr);

	if(ad < p)
	{	pair.mtch[0].tpos = ad;
		pair.mtch[0].mpos = -1;
		pair.mtch[0].size = p-ad;
		if((*vcpa->parsef)(vcpa, 0, pair.mtch, 1) <= 0 )
			goto done;
	}

	rv = 0; /* if got here, data successfully parsed */

done:	if(str && str != vcpa->tar && str != vcpa->src)
		free(str);
	if(sfx)
		free(sfx);
	return rv;
}

#if __STD_C
int vclzparse(Vclzparse_t* vcpa, ssize_t bound)
#else
int vclzparse(vcpa, bound )
Vclzparse_t*	vcpa;
ssize_t		bound;	/* <0: sfxsort, >=0: hashing with pruning if >0 */
#endif
{
	/* error conditions */
	if(!vcpa->parsef)
		return -1;
	if(vcpa->nsrc < 0 || (vcpa->nsrc > 0 && !vcpa->src) )
		return -1;
	if(vcpa->ntar < 0 || (vcpa->ntar > 0 && !vcpa->tar) )
		return -1;

	if(vcpa->ntar == 0 ) /* nothing to do */
		return 0;

	if(vcpa->ntar < MMIN(vcpa) ) /* no match possible */
	{	Vclzmatch_t	mtch;
		mtch.tpos = 0;
		mtch.mpos = -1;
		mtch.size = vcpa->ntar;
		if((*vcpa->parsef)(vcpa, 0, &mtch, 1) != vcpa->ntar)
			return -1;
		else	return 0;
	}

	if((vcpa->type & (VCLZ_REVERSE|VCLZ_MAP)) || bound >= 0 || vcpa->cmap )
		return hashparse(vcpa, bound);
	else	return sfxparse(vcpa);
}
