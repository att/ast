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
#include	"vcwhdr.h"

/*	Compute window using block matching.
**
**	Written by Kiem-Phong Vo.
*/

#define PF_MINSIZE	(16)		/* minimum block size		*/
#define PF_DFLTSIZE	(16*PF_MINSIZE)	/* default block size		*/
#define PF_MAXSIZE	(64*PF_MINSIZE)	/* maximum block size		*/

#define PFMERGE(bz)	(16*(bz))	/* merge if overlapped by this	*/
#define PFSMALL(n,bz)	((n) <= 3*(bz))	/* window too small to use	*/

#define PF_COEF		((Vchash_t)17)	/* linear congruential hash 	*/

#define PF_HUGE		(~((Vchash_t)0))

typedef struct _pfseg_s		Pfseg_t;
typedef struct _pffile_s	Pffile_t;
typedef struct _prefix_s	Prefix_t;
typedef struct _pfobj_s		Pfobj_t;
typedef struct _pftab_s		Pftab_t;

struct _pfseg_s
{	Sfoff_t		ldt;	/* data interval		*/
	Sfoff_t		rdt;
	Sfoff_t		lmt;	/* matched data interval	*/
	Sfoff_t		rmt;
	ssize_t		mtch;	/* number of matched bytes	*/
};

struct _pfobj_s
{	Vchash_t	key;
	Pfobj_t*	next;
};
struct _pftab_s
{	Pfobj_t*	list;
};

struct _pffile_s
{	Pfobj_t*	obj;	/* keys representing blocks	*/
	Pftab_t*	tab;
	size_t		mask;

	Vcchar_t*	bloom;	/* Bloom filtering of unmatches	*/
	size_t		bmask;

	Sfio_t*		sf;	/* stream matching against	*/
	Sfoff_t		sfsz;	/* max file size		*/
	ssize_t		maxo;	/* max object with signature	*/

	Sfoff_t		dtpos;	/* position/size of parsed data	*/
	ssize_t		dtsz;
	ssize_t		cseg;	/* current unprocessed segment	*/
	ssize_t		nseg;	/* number of parsed segments	*/
	ssize_t		sseg;	/* size of segment array	*/
	Pfseg_t*	seg;	/* parsed segments in data	*/
};

struct _prefix_s
{	Pffile_t*	srcf;
	Pffile_t*	tarf;
	ssize_t		blksz;	/* block size for signatures	*/
	Vchash_t	coef;	/* high coefficient of hashf	*/
	Vchash_t*	key;	/* temp space for data keys	*/
	ssize_t		nkey;	/* current size of key[]	*/
	Sfoff_t		keyb;	/* base of calculated keys	*/
	Sfoff_t		here;	/* data processed to here	*/
};

/* functions to set and test bits from a Bloom filter */
#define BLOOMDEF(pf,ky,b1,b2)	(((b1) = (ky)&(pf)->bmask), \
			 	 ((b2) = VCHASH(ky)&(pf)->bmask) )
#define BLOOMSET(pf,ky,b1,b2)	(BLOOMDEF(pf,ky,b1,b2), \
		 		 ((pf)->bloom[(b1) >> 3] |= (1 << ((b1)&7)) ), \
		 		 ((pf)->bloom[(b2) >> 3] |= (1 << ((b2)&7)) ) )
#define Bloomtest(pf,ky,b1,b2)	(BLOOMDEF(pf,ky,b1,b2), \
			 	 (((pf)->bloom[(b1) >> 3] &  (1 << ((b1)&7)) ) && \
			 	  ((pf)->bloom[(b2) >> 3] &  (1 << ((b2)&7)) ) ) )
#define BLOOMTEST(pf,ky,b1,b2)	((pf)->bloom ? Bloomtest(pf,ky,b1,b2) : 1)

#define pfindex(pf,ky)		(((ky)>>3) & (pf)->mask)

/* compute the linear congruential hash of a block of data */
#if __STD_C
static Vchash_t pfhash(Vcchar_t* data, ssize_t dtsz)
#else
static Vchash_t pfhash(data, dtsz)
Vcchar_t*	data;
ssize_t		dtsz;
#endif
{
	Vchash_t	key;
	Vcchar_t	*endd;

	for(key = 0, endd = data+dtsz; data < endd; ++data)
		key = key*PF_COEF + *data;
	return key;
}

/* compute hash value of dt given the hash value of (dt-1) */
#define pfnext(dt,sz,ky,cf) \
		( ((ky) - *((dt)-1)*(cf))*PF_COEF + *((dt)+sz-1) )

/* create the structure to search file sf */
#if __STD_C
static Pffile_t* pfopen(Sfio_t* sf, ssize_t sz, int srcfile)
#else
static Pffile_t* pfopen(sf, sz, srcfile)
Sfio_t*		sf;
ssize_t		sz;
int		srcfile;
#endif
{
	ssize_t		k, n, blsz;
	Sfoff_t		sfsz;
	Pffile_t	*pf;

	if((sfsz = sfsize(sf)) < 0 )
		return NIL(Pffile_t*);
	if(sz > 0 && (n = (ssize_t)(sfsz/sz)) > 0)
	{	for(k = 1024; k*2 < n; k *= 2)
			;
		blsz = srcfile ? 2*k : 0; /* Bloom filter size */
	}
	else	n = k = blsz = 0;

	sz = sizeof(Pffile_t) + n*sizeof(Pfobj_t) + k*sizeof(Pftab_t) + blsz;
	if(!(pf = (Pffile_t*)calloc(1,sz)) )
		return NIL(Pffile_t*);

	if(n > 0)
	{	pf->obj = (Pfobj_t*)(pf + 1);
		pf->tab = (Pftab_t*)(pf->obj+n);
		pf->mask = k-1;

		pf->bloom = srcfile ? (Vcchar_t*)(pf->tab + k) : NIL(Vcchar_t*);
		pf->bmask = 8*blsz - 1;
	}

	pf->sf = sf;
	pf->sfsz = sfsz;
	pf->maxo = n <= 0 ? -1 : 0;

	return pf;
}

/* make blocks with addresses < maxp searchable */
#if __STD_C
static int pfbuild(Pffile_t* pf, ssize_t blksz)
#else
static int pfbuild(pf, blksz)
Pffile_t*	pf;
ssize_t		blksz;
#endif
{
	Vchash_t	k, r, n;
	Pfobj_t		*obj, *endo;
	Pftab_t		*tab;
	Void_t		*dt;

	if(pf->maxo < 0) /* no construction possible */
		return 0;

	/* construct keys to represent blocks */
	if(sfseek(pf->sf, (Sfoff_t)0, 0) != 0)
		return -1;
	for(endo = (obj = pf->obj) + (size_t)(pf->sfsz/blksz); obj < endo; ++obj)
	{	if(!(dt = sfreserve(pf->sf, blksz, 0)) || sfvalue(pf->sf) < blksz)
			return -1;
		if((k = pfhash(dt, blksz)) == PF_HUGE)
			k = PF_HUGE-1;

		obj->key = k;
		if(pf->bloom)
			BLOOMSET(pf, k, n, r);
		tab = pf->tab + pfindex(pf, k);
		obj->next = tab->list; tab->list = obj;
	}

	return 0;
}

#if __STD_C
static ssize_t pfsegment(Pffile_t* pf, ssize_t ld, ssize_t lm, ssize_t mn, ssize_t blksz)
#else
static ssize_t pfsegment(pf, ld, lm, mn, blksz)
Pffile_t*	pf;
ssize_t		ld;
ssize_t		lm;
ssize_t		mn;
ssize_t		blksz;
#endif
{
	Pfseg_t	*seg;
	Sfoff_t	ldt, rdt, lmt, rmt;

	ldt = ld + pf->dtpos; rdt = ldt + mn;
	lmt = ((Sfoff_t)lm)*blksz; rmt = lmt + mn;

	/* merge with last segment if overlapping */
	if((seg = pf->nseg > 0 ? pf->seg + pf->nseg-1 : 0) &&
	   lmt <= (seg->rmt+PFMERGE(blksz)) && rmt >= (seg->lmt-PFMERGE(blksz)) )
	{	if(seg->lmt > lmt)
			seg->lmt = lmt;
		if(seg->rmt < rmt)
			seg->rmt = rmt;
		seg->rdt = rdt;
		seg->mtch += mn;
		return mn;
	}

	if(PFSMALL(mn,blksz))  /* too small to make a segment */
		return 0;

	/* creating a new segment */
	if(pf->nseg >= pf->sseg)
	{	pf->seg = (Pfseg_t*)realloc(pf->seg, (pf->nseg+8)*sizeof(Pfseg_t));
		if(!pf->seg)
			return -1;
		pf->sseg = pf->nseg + 8;
	}
	seg = pf->seg + pf->nseg; pf->nseg += 1;
	seg->ldt = ldt; seg->rdt = rdt;
	seg->lmt = lmt; seg->rmt = rmt;
	seg->mtch = mn;
	return mn;
}

/* parse a string of keys into matchable and unmatched data */
#if __STD_C
static ssize_t pfparse(Pffile_t* pf, Vchash_t* dt, ssize_t n, Sfoff_t pos, ssize_t blksz)
#else
static ssize_t pfparse(pf, dt, n, pos, blksz)
Pffile_t*	pf;
Vchash_t*	dt;
ssize_t		n;
Sfoff_t		pos;
ssize_t		blksz;
#endif
{
	ssize_t		i, k, d, dd, mm, mn, total, ucnt;
	Pfobj_t		*o;

	pf->cseg = pf->nseg = 0;
	pf->dtpos = pos;
	pf->dtsz = n;

	total = 0;
	for(d = 0, n -= (blksz-1); d < n; d = dd+mn)
	{	for(mn = 0, mm = -1, dd = d; dd < n; ++dd)
		{	if(!BLOOMTEST(pf, dt[dd], i, k))
				continue;
			for(o = pf->tab[pfindex(pf, dt[dd])].list; o; o = o->next)
			{	/* find a stretch of approximate matches */
				k = dd;	i = o - pf->obj; ucnt = 0;
				for(; k < n && i < pf->maxo; k += blksz, i += 1)
				{	if(dt[k] == pf->obj[i].key)
						ucnt = 0;
					else if(k == dd || ucnt >= 2)
						break; /* too many misses */
					else	ucnt += 1;
				}
				if((k = (k-dd) - ucnt*blksz) > mn)
				{	mn = k; /* current best match length */
					mm = o - pf->obj;
				}
			}
			if(mn > 0)
				break;
		}

		if(mn <= 0) /* done with search for windows */
			break;
		else if((k = pfsegment(pf, dd, mm, mn, blksz)) < 0)
			return -1;
		else	total += k; /* good match */
	}

	return total;
}

/* compute a window to return to application for delta-ing */
#if __STD_C
static int pfwindow(Pffile_t* pf, Vcwmatch_t* wm, ssize_t blksz)
#else
static int pfwindow(pf, wm, blksz)
Pffile_t*	pf;
Vcwmatch_t*	wm;
ssize_t		blksz;
#endif
{
	Pfseg_t	*seg;
	ssize_t	sz;

	seg = pf->cseg >= pf->nseg ? NIL(Pfseg_t*) : pf->seg+pf->cseg;

	/* absorb left unmatched data if any */
	sz = seg ? (ssize_t)(seg->ldt - pf->dtpos) : pf->dtsz;
	if(seg && sz > 0 && sz < (seg->rdt - seg->ldt) )
	{	seg->ldt -= sz;
		if((seg->lmt -= sz) < 0)
			seg->lmt = 0;
		sz = 0;
	}

	if(sz > 0) /* returning an unmatchable window of data */
	{	wm->wpos = -1;
		wm->wsize = 0;
		wm->wdata = 0;
		wm->msize = sz;
		pf->dtpos += sz; pf->dtsz -= sz;
		wm->more = pf->dtsz > 0 ? 1 : 0;
		return 0;
	}

	pf->cseg += 1; /* will be returning whatever is in seg */

	/* absorb any right unmatched block data */
	if(pf->cseg < pf->nseg)
		sz = (ssize_t)((seg+1)->ldt - seg->rdt);
	else	sz = (ssize_t)((pf->dtpos+pf->dtsz) - seg->rdt);
	if(sz > 0)
	{	seg->rdt += sz;
		if((seg->rmt += sz) > pf->maxo*blksz)
			seg->rmt = ((Sfoff_t)pf->maxo)*blksz;
	}

	/* set window */
	wm->wpos = seg->lmt;
	wm->wsize = (ssize_t)(seg->rmt - seg->lmt);
	if(sfseek(pf->sf, seg->lmt, 0) != seg->lmt ||
	   !(wm->wdata = sfreserve(pf->sf, wm->wsize, 0)) ||
	   sfvalue(pf->sf) < wm->wsize)
	{	pf->cseg = pf->nseg = 0;
		pf->dtsz = 0;
		return -1;
	}
	wm->msize = (ssize_t)(seg->rdt - seg->ldt);

	pf->dtpos += wm->msize; pf->dtsz -= wm->msize;
	wm->more = pf->dtsz > 0 ? 1 : 0;

	return 1;
}

#if __STD_C
static Vcwmatch_t* pfmatch(Vcwindow_t* vcw, Void_t* data, size_t dtsz, Sfoff_t here)
#else
static Vcwmatch_t* pfmatch(vcw, data, dtsz, here)
Vcwindow_t*	vcw;
Void_t*		data;	/* target data to be matched	*/
size_t		dtsz;	/* data size			*/
Sfoff_t		here;	/* current target position	*/
#endif
{
	int		rv;
	Vcchar_t	*dt, *endd;
	Vchash_t	k, *key;
	Pffile_t	*pf;
	Vcwmatch_t	*wm = &vcw->match;
	Prefix_t	*pfx = (Prefix_t*)vcw->mtdata;

	if(!pfx->srcf && !pfx->tarf )
		return NIL(Vcwmatch_t*);

	if((pf = pfx->srcf) && ((Sfoff_t)dtsz >= pf->sfsz/2 || dtsz < pfx->blksz) )
	{	wm->type = VCD_SOURCEFILE;
		if(dtsz < pfx->blksz) /* small data, use matching window */
		{	wm->wpos = here;
			wm->wsize = here+dtsz > pf->sfsz ? (ssize_t)(pf->sfsz-here) : dtsz;
		}
		else /* too large, use all source file */
		{	wm->wpos = 0;
			wm->wsize = (ssize_t)pf->sfsz;
		}
		if(sfseek(pf->sf, (Sfoff_t)0, 0) != (Sfoff_t)0 ||
		   !(wm->wdata = sfreserve(pf->sf, wm->wsize, 0)) )
			return NIL(Vcwmatch_t*);
		wm->msize = dtsz;
		wm->more = 0;
		return wm;
	}

target_file: /* returning computed windows in target file */
	if((pf = pfx->tarf) )
	{	if(pf->dtpos == here && dtsz <= pf->dtsz )
		{	if((rv = pfwindow(pf, wm, pfx->blksz)) < 0)
				return NIL(Vcwmatch_t*);
			wm->type = rv == 0 ? 0 : VCD_TARGETFILE;
			return wm;
		}

		pf->cseg = pf->nseg = 0;
		pf->maxo = here/pfx->blksz; /* can now match to here */
	}

	pf = pfx->srcf;
	if(dtsz > pfx->nkey) /* make sure we have space for keys */
	{	pfx->key = (Vchash_t*)realloc(pfx->key, dtsz*sizeof(Vchash_t));
		if((pfx->nkey = pfx->key ? dtsz : 0) <= 0)
			return NIL(Vcwmatch_t*);
	}
	if(here != pfx->here || !pf || pf->dtsz <= 0 )
	{	/* compute all keys for this set of data */
		endd = (dt = (Vcchar_t*)data) + dtsz - (pfx->blksz-1);
		k = pfhash(dt, pfx->blksz);
		*(key = pfx->key) = k == PF_HUGE ? (k>>1) : k;
		for(dt += 1; dt < endd; ++dt)
		{	k = pfnext(dt, pfx->blksz, k, pfx->coef);
			*++key = k == PF_HUGE ? (k>>1) : k;
			/**/DEBUG_ASSERT(k == pfhash(dt, pfx->blksz));
		}
		pfx->keyb = pfx->here = here; /* set base of calculated keys */

		/* match with source data to parse into segments */
		if((pf ? pfparse(pf, pfx->key, dtsz, here, pfx->blksz) : 0) <= 0 )
		{	/* fail source, try target */
			if((pf = pfx->tarf) &&
			   pfparse(pf, pfx->key, dtsz, here, pfx->blksz) > 0 )
				goto target_file;
			else	return NIL(Vcwmatch_t*);
		}
	}

	/**/DEBUG_ASSERT(pf && pf == pfx->srcf && pf->dtsz > 0);
	/**/DEBUG_ASSERT(pfx->here == here);
	if((rv = pfwindow(pf, wm, pfx->blksz)) < 0)
		return NIL(Vcwmatch_t*);
	else
	{	pfx->here = here + wm->msize;
		/**/DEBUG_PRINT(2,"here=%8d ",(ssize_t)here);
		/**/DEBUG_PRINT(2,"dtsz=%8d ",(ssize_t)dtsz);
		/**/DEBUG_PRINT(2,"mtch=%8d ",(ssize_t)wm->msize);
		/**/DEBUG_PRINT(2,"wpos=%8d ",(ssize_t)wm->wpos);
		/**/DEBUG_PRINT(2,"wsiz=%8d \n",(ssize_t)wm->wsize);
		if(rv > 0)
		{	wm->type = VCD_SOURCEFILE;
			return wm;
		}
		else if(!pfx->tarf || wm->msize <= 2*pfx->blksz)
		{	wm->type = 0;
			return wm;
		}
		else /* try matching this segment with target data */
		{	key = pfx->key + (size_t)(here - pfx->keyb);
			dtsz = wm->msize; /* set this for possible tail recursion */
			if(pfparse(pfx->tarf, key, dtsz, here, pfx->blksz) > 0 )
				goto target_file;
			else
			{	wm->type = 0;
				return wm;
			}
		}
	}
}

/* Event handler */
#if __STD_C
static int pfevent(Vcwindow_t* vcw, int type)
#else
static int pfevent(vcw, type)
Vcwindow_t*	vcw;
int		type;
#endif
{
	Sfoff_t		srcsz, tarsz, maxsz;
	ssize_t		sz;
	Prefix_t	*pfx;
	int		rv = -1;

	if(type == VCW_OPENING)
	{
		if(!vcw->disc || (!vcw->disc->srcf && !vcw->disc->tarf) )
			return -1;

		if(!(pfx = (Prefix_t*)calloc(1,sizeof(Prefix_t))) )
			return -1;

		/* compute a suitable block size between [MINSIZE,MAXSIZE] */
		srcsz = vcw->disc->srcf ? sfsize(vcw->disc->srcf) : 0;
		tarsz = vcw->disc->tarf ? sfsize(vcw->disc->tarf) : 0;
		maxsz = srcsz > tarsz ? srcsz : tarsz;
		if(maxsz < PF_MINSIZE)
			sz = 0;
		else
		{	sz = PF_DFLTSIZE;
#define PF_MINBLKCNT	(8*1024)	/* aim for at least this many	*/
#define PF_MAXBLKCNT	(16*1024*1024)	/* likewise for upper bound	*/
			while(sz < PF_MAXSIZE && maxsz/sz > PF_MAXBLKCNT)
				sz += PF_MINSIZE;
			while(sz > PF_MINSIZE && maxsz/sz < PF_MINBLKCNT)
				sz -= PF_MINSIZE;
		} 
		pfx->blksz = sz; /**/DEBUG_PRINT(2,"blksz=%d\n",pfx->blksz);

		pfx->coef = 1; /* highest coef in hashing blocks */
		for(; sz > 1; --sz)
			pfx->coef *= PF_COEF;

		if(srcsz > 0)
		{	if(!(pfx->srcf = pfopen(vcw->disc->srcf, pfx->blksz, 1)) )
				goto do_closing;
			if(pfx->blksz > 0)
			{	if(pfbuild(pfx->srcf, pfx->blksz) < 0)
					goto do_closing;
				pfx->srcf->maxo = pfx->srcf->sfsz/pfx->blksz;
			}
		}

		if(tarsz > 0 && pfx->blksz > 0)
		{	if(!(pfx->tarf = pfopen(vcw->disc->tarf, pfx->blksz, 0)) )
				goto do_closing;
			if(pfbuild(pfx->tarf, pfx->blksz) < 0)
				goto do_closing;
			pfx->tarf->maxo = 0;
		}

		vcw->mtdata = (Void_t*)pfx;
		return 0;
	}
	else if(type == VCW_CLOSING)
	{	rv = 0;
	do_closing:
		if((pfx = (Prefix_t*)vcw->mtdata) )
		{	if(pfx->srcf)
			{	if(pfx->srcf->seg && pfx->srcf->sseg > 0)
					free(pfx->srcf->seg);
				free(pfx->srcf);
			}
			if(pfx->tarf)
			{	if(pfx->tarf->seg && pfx->tarf->sseg > 0)
					free(pfx->tarf->seg);
				free(pfx->tarf);
			}
			if(pfx->key && pfx->nkey > 0)
				free(pfx->key);
			free(pfx);
		}
		vcw->mtdata = NIL(Void_t*);
		return rv;
	}
	else	return 0;
}

Vcwmethod_t	_Vcwprefix =
{	pfmatch,
	pfevent,
	"prefix",
	"Find windows with matching prefixes.",
	"[-version?window::prefix (AT&T Research) 2003-01-01]" USAGE_LICENSE,
};

Vcwmethod_t*	Vcwprefix = &_Vcwprefix;
