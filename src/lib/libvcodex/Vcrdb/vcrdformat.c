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
#include	<vcrdb.h>
#include	<ctype.h>

/* Heuristic algorithms to compute the records&fields structure of a
** Relational Data File, if the file is indeed relational.
**
** The basic observation is that fields in a RDF often have different
** statistical characteristics due to different types. Thus, checking
** for changes in the entropy coding of contiguous regions of columns
** can tell how fields may be arranged in a record.
**
** Two cases will be considered:
** 1. Records and fields are defined by special record and field
**    separating characters, or
** 2. Records and fields have fixed lengths.
**
** Written by Kiem-Phong Vo.
*/

typedef ssize_t	Freq_t[256]; /* type to count byte frequencies	*/

/* compute the cost in bits of coding a dataset */
#if __STD_C
static ssize_t rdhuffman(Freq_t freq)
#else
static ssize_t rdhuffman(freq)
Freq_t	freq;
#endif
{
	ssize_t	k, w;
	Freq_t	cdsz;

	/* use the Huffman coder to estimate coding cost */
	if(vchsize(256, freq, cdsz, 0) <= 0 )
	{	for(w = 0, k = 0; k < 256; ++k)
			w += freq[k];
		return (vcsizeu(w)+1)*8; /* cost to encode a run */
	}
	else
	{	for(w = 0, k = 0; k < 256; ++k)
		{	w += freq[k]*cdsz[k]; /* cost for all bytes k */
			w += vcsizeu(cdsz[k])*8; /* cost of k's code */
		}
		return w;
	}
}

/* types and algorithms to detect field and record separators	*/
#define TYPN	8	/* max # different fldn's allowed 	*/
typedef struct _fsep_s	/* structure of each candidate fsep	*/
{	ssize_t		cdsz;	/* coding size			*/
	ssize_t		typn;	/* #types of fields		*/
	ssize_t		typ[TYPN]; /* known distinct fldn's 	*/
	ssize_t		rec[TYPN]; /* corresponding record cnts	*/
} Fsep_t;

/* define set of characters allowed to be field separators */
#define OKRSEP(x)	((x) == '\n')
#define OKFSEP(x)	((x) == ' ' || (x) == '\t' || \
			 (isprint(x) && !isalnum(x) && (x) != '_' && (x) != '"' && (x) != '\'') )
#define OKPRINT(x)	(isprint(x))

#if __STD_C /* check for a set of preferred separators */
static int fsprefer(int fs)
#else
static int fsprefer(fs)
int	fs;
#endif
{
	int	k, p;
	char	*prefer[2];

	prefer[1] = ":|;,&";
	prefer[0] = "%.";

	for(p = 1; p >= 0; --p)
		for(k = 0; prefer[p][k]; ++k)
			if(prefer[p][k] == fs)
				return p;
	return -1;
}

#if __STD_C
static int rdsepar(Vcrdformat_t* rdf, Vcchar_t* data, ssize_t dtsz, int rs)
#else
static int rdsepar(rdf, data, dtsz, rs)
Vcrdformat_t*	rdf;	/* computed format data	*/
Vcchar_t*	data;	/* training data	*/
ssize_t		dtsz;	/* training data size	*/
int		rs;	/* record separator 	*/
#endif
{
	ssize_t		k, z, fn, recn, size, print;
	int		fs, bestfs, prefer, p;
	double		perf;
	Fsep_t		fsep[256];
	Freq_t		freq, *fld = NIL(Freq_t*);

	if(!data || dtsz <= 0 || !OKRSEP(rs) )
		return -1;

	/* compute statistics for all possible field separators */
	print = 0;
	memset(fsep, 0, sizeof(fsep));
	for(recn = 0, size = 0;;)
	{	for(z = size; z < dtsz; ++z)
			if(data[z] == rs)
				break;
		if(z >= dtsz) /* data ends on an incomplete record */
			break;

		memset(freq, 0, sizeof(freq));
		for(; size < z; ++size) /* byte frequencies in this record */
		{	if(OKPRINT(data[size]) )
				print += 1;
			freq[data[size]] += 1;
		}

		size += 1; /* amount of processed data */
		recn += 1; /* # of complete records */

		for(fs = 0; fs < 256; ++fs) /* check each candidate fld sep */
		{	if(!OKFSEP(fs) || (fn = freq[fs]+1) <= 1 || fsep[fs].typn < 0 )
				continue;

			/* find the slot for this fldn type */
			for(k = 0; k < fsep[fs].typn; ++k)
				if(fsep[fs].typ[k] == fn)
					break;

			if(k == TYPN) /* too many fldn types to be a good separator */
				fsep[fs].typn = -1;
			else
			{	if(k == fsep[fs].typn)
					fsep[fs].typn += 1;
				fsep[fs].typ[k] = fn; /* fldn type */
				fsep[fs].rec[k] += 1; /* fldn type */
			}
		}
	}

	if(recn <= 3 || size < 3*dtsz/4 ) /* rs is poor record separator */
		return -1;

	if(100*print < 75*size ) /* data are not printable */
		return -1;

	bestfs = -1; prefer = -1; /* test for preferred separators */
	for(fs = 0; fs < 256; ++fs)
	{	if(fsep[fs].typn <= 0 )
			continue;

		/* move max #fields in any record and most #occurences  */
		for(fn = 0, z = 0, k = 0; k < fsep[fs].typn; ++k)
		{	if(fsep[fs].typ[k] > fn)
				fn = fsep[fs].typ[k];
			if(fsep[fs].rec[k] > z)
				z  = fsep[fs].rec[k];
		} 

		if(z > 999*recn/1000 && (p = fsprefer(fs)) >= 0 && (prefer < 0 || p > prefer) )
		{	fsep[fs].rec[0] = recn+1;
			fsep[fs].typn = 1;
			bestfs = fs;
			prefer = p;
		}
		else if(z < 95*recn/100) /* too few appearances to be a separator */
			fsep[fs].typn = -1;
		else /* to be considered later */
		{	fsep[fs].typ[0] = fn;
			fsep[fs].rec[0] = z;
		}
	}
	if(bestfs >= 0) /* remove all unpreferred candidates */
	{	for(fs = 0; fs < 256; ++fs)
			if(fsep[fs].typn <= 0 || fsprefer(fs) < prefer )
				fsep[fs].typn = -1;
	}

	bestfs = -1; /* one with least variation across records in #fields */
	for(fs = 0; fs < 256; ++fs)
	{	if(fsep[fs].typn <= 0 )
			continue;
		if(bestfs < 0 || fsep[bestfs].typn > fsep[fs].typn )
			bestfs = fs;
	}
	if(bestfs < 0) /* no credible candidate */
		return -1;

	for(fs = 0; fs < 256; ++fs) /* keep those with least variation in #fields */
		if(fsep[fs].typn > fsep[bestfs].typn)
			fsep[fs].typn = -1;

	for(fs = 0; fs < 256; ++fs) /* among those keeps the ones with max #fields */
		if(fsep[fs].typn == fsep[bestfs].typn && fsep[fs].typ[0] > fsep[bestfs].typ[0]) 
			bestfs = fs;
	for(fs = 0; fs < 256; ++fs)
		if(fsep[fs].typn == fsep[bestfs].typn && fsep[fs].typ[0] < fsep[bestfs].typ[0]) 
			fsep[fs].typn = -1;

	/* now find the separator with best coding size among the remainders */
	if(!(fld = (Freq_t*)malloc(fsep[bestfs].typ[0]*sizeof(Freq_t))) )
		return -1;
	for(bestfs = -1, fs = 0; fs < 256; ++fs)
	{	if(fsep[fs].typn <= 0 )
			continue;

		/* compute coding size for all fields given fs as field sep */
		memset(fld, 0, fsep[fs].typ[0]*sizeof(Freq_t));
		for(fn = 0, z = 0; z < size; ++z)
		{	fld[fn][data[z]] += 1;
			if(data[z] == rs) /* see record sep, reset field index */
				fn = 0;
			else if(data[z] == fs) /* see field sep, advance field index */
				fn += 1;
		}
		fsep[fs].cdsz = fsep[fs].typ[0]*8;
		for(fn = 0; fn < fsep[fs].typ[0]; ++fn)
			fsep[fs].cdsz += rdhuffman(fld[fn]);

		if(bestfs < 0 || fsep[fs].cdsz < fsep[bestfs].cdsz )
			bestfs = fs;
	}
	free(fld); /* no need for this anymore */

	/* compute cost for all data */
	memset(freq, 0, sizeof(freq));
	for(z = 0; z < size; ++z)
		freq[data[z]] += 1;
	size = rdhuffman(freq);
	perf = ((double)fsep[bestfs].cdsz)/size;

	if(rdf->perf > 0. && rdf->fldn > 2*fsep[bestfs].typ[0] )
		perf = rdf->perf; /* prefer larger number of fields */
	
	if(prefer < 0 && rdf->perf > 0. && perf >= rdf->perf )
		return 0;
	else	/* a good pair of separators */
	{	if(rdf->fldn > 0 && rdf->fldz)
			free(rdf->fldz);
		rdf->fldn = 0;
		rdf->fldz = NIL(ssize_t*);
		rdf->fsep = bestfs;
		rdf->rsep = rs;
		rdf->perf = perf;
		return 1;
	}
}

/* Algorithms to compute fields assuming that they have fixed lengths */
#define	FLD_MATCH	01	/* repeating across records	*/
#define FLD_ALTER	02	/* byte compositions changed	*/

#if __STD_C /* compute byte frequencies in columns */
static void colfreq(Freq_t* colf, Vcchar_t* data, ssize_t nc, ssize_t nr)
#else
static void colfreq(colf, data, nc, nr)
Freq_t*		colf;	/* byte frequencies	*/
Vcchar_t*	data;	/* raw data 		*/
ssize_t		nc;	/* number of columns	*/
ssize_t		nr;	/* number of rows	*/
#endif
{
	ssize_t	r, c;

	memset(colf, 0, nc*sizeof(Freq_t));
	for(r = 0; r < nr; ++r)
		for(c = 0; c < nc; ++c)
			colf[c][*data++] += 1;
}

#if __STD_C /* compute byte frequencies in fields given columns */
static void fldfreq(Freq_t* fldf, ssize_t* fldz, ssize_t nf, Freq_t* colf)
#else
static void fldfreq(fldf, fldz, nf, colf)
Freq_t*		fldf;	/* field frequencies	*/
ssize_t*	fldz;	/* field lengths	*/
ssize_t		nf;	/* number of fields	*/
Freq_t*		colf;	/* column frequencies	*/
#endif
{
	ssize_t	c, f, k, endc;

	memset(fldf, 0, nf*sizeof(Freq_t));
	for(c = 0, f = 0; f < nf; ++f)
		for(endc = c+fldz[f]; c < endc; ++c)
			for(k = 0; k < 256; ++k)
				fldf[f][k] += colf[c][k];	
}

#ifdef DEBUG
static int printmark(ssize_t* mark, ssize_t nc, int mrkc)
{
	ssize_t	c;
	for(c = 0; c < nc-1; ++c)
		DEBUG_PRINT(2, "%c", mark[c] ? mrkc : '0'+(c%10));
	DEBUG_PRINT(2, "%c\n", mark[c] ? mrkc : '0'+(c%10));
	return 1;
}
static int printfield(ssize_t* fldz, ssize_t nf, int mrkc)
{
	ssize_t	f, c, k;
	for(c = 0, f = 0; f < nf; ++f)
	{	DEBUG_PRINT(2, "%c", mrkc); c += 1;
		for(k = 1; k < fldz[f]; ++k)
			{ DEBUG_PRINT(2, "%c", '0'+(c%10)); c += 1; }
	}
	DEBUG_PRINT(2, "\n", 0);
	return 1;
}
#endif

#if __STD_C /* mark where there are definite matches */
static void fldmark(ssize_t* mark, Vcchar_t* data, ssize_t nc, ssize_t nr, Freq_t* colf)
#else
static void fldmark(mark, data, nc, nr, colf)
ssize_t*	mark;	/* array of marks	*/
Vcchar_t*	data;	/* overall data		*/
ssize_t		nc;	/* number of columns	*/
ssize_t		nr;	/* number of records	*/
Freq_t*		colf;	/* column frequencies	*/
#endif
{
	ssize_t	c, r, k, mf, mb, lb, rb;
	Freq_t	freq, rght;

	memset(mark, 0, nc*sizeof(ssize_t));
	for(c = 0; c < nc; ++c)
	{	mf = 0; mb = -1; /* find most frequent byte */
		memset(freq, 0, sizeof(freq));
		for(r = 0; r < nr; ++r)
			freq[data[r*nc + c]] += 1;
		for(k = 0; k < 256; ++k)
			if(freq[k] > mf)
				{ mf = freq[k]; mb = k; }
		if(100*mf >= 95*nr) /* frequent enough */
		{	mark[c] = FLD_MATCH;
			continue;
		}

		if(c >= nc-1) /* no pair checking */
			continue;

		mf = 0; /* find most frequent next byte */
		memset(freq, 0, sizeof(freq));
		for(r = 0; r < nr; ++r)
			if(data[r*nc + c] == mb)
				freq[data[r*nc + c+1]] += 1;
		for(k = 0; k < 256; ++k)
			if(freq[k] > mf)
				mf = freq[k];
		if(100*mf >= 80*nr) /* frequent enough */
		{	mark[c] = FLD_MATCH;
			if(100*mf >= 90*nr)
				mark[c += 1] = FLD_MATCH;
		}
	} /**/DEBUG_ASSERT(printmark(mark,nc,'x'));

#define SIDE	4 /* first few columns cannot be marked FLD_ALTER. */
		  /* fldpartition() will compensate for this later */
	if(nc < 2*SIDE)
		return;
	memset(freq, 0, sizeof(Freq_t));
	memset(rght, 0, sizeof(Freq_t));
	for(c = 0; c < SIDE; ++c)
	for(k = 0; k < 256; ++k)
	{	freq[k] += colf[c][k];
		rght[k] += colf[c+SIDE][k];
	}
	for(c = SIDE;; ++c) 
	{	lb = rb = mb = 0;
		for(k = 0; k < 256; ++k)
		{	if(freq[k]) /* count left bytes */
				lb += 1;
			if(rght[k]) /* count right bytes */
				rb += 1;
			if(freq[k] && rght[k]) /* intersection */
				mb += 1; 
		}
		if(lb >= 3 && rb >= 3 && mb <= (lb+rb-mb)/10 )
			mark[c] |= FLD_ALTER;

		if(c+SIDE >= nc)
			break; /* done checking */
		else for(k = 0; k < 256; ++k) /* update sides */
		{	freq[k] += colf[c][k] - colf[c-SIDE][k];
			rght[k] += colf[c+SIDE][k] - colf[c][k];
		}
	} /**/DEBUG_ASSERT(printmark(mark,nc,'+'));
}

#if __STD_C /* get fields by alignment */
static ssize_t fldpartition(ssize_t* fldz, ssize_t* mark, ssize_t nc, ssize_t algn)
#else
static ssize_t fldpartition(fldz, mark, nc, algn)
ssize_t*	fldz; 	/* field sizes	*/
ssize_t*	mark; 	/* column marks	*/
ssize_t		nc;	/* #columns	*/
ssize_t		algn;	/* alignment	*/
#endif
{
	ssize_t	c, k, kk, nf;

	nf = 0;
	for(c = 0; c < nc; c = k)
	{	for(k = c; k < nc; ++k)
			if(mark[k] != 0 )
				break;

		if(k < nc)
		{	
			if(mark[k]&FLD_MATCH) /* get all the matches */
			{	for(kk = k+1; kk < nc; ++kk)
					if(!(mark[kk] & FLD_MATCH) )
						break;
				if((kk-k) >= (k-c)/2 ) /* merge with [c,k) if appropriate */
					k = kk > k+1 ? kk : k;
			}
			else if((kk = k+1) < nc && mark[kk] == 0 )
			{	for(; kk < nc; ++kk) /* grab only non-marked elts */
					if(mark[kk] != 0 )
						break;
				k = kk > k+1 ? kk : k;
			}
			else
			{	for(; kk < nc; ++kk) /* grab all marked elts */
					if(mark[kk] == 0 )
						break;
				if((kk-k) >= (k-c)/2 ) /* merge with [c,k) if appropriate */
					k = kk > k+1 ? kk : k;
			}

		}

		k = k >= nc ? nc : k <= c+algn ? c+algn : (k/algn)*algn;
		fldz[nf++] = k-c;
	}
	return nf;
}

#if __STD_C /* cost of a field partition */
ssize_t fldcost(ssize_t* fldz, ssize_t nf, Freq_t* colf)
#else
ssize_t fldcost(fldz, nf, colf)
ssize_t*	fldz;	/* field lengths	*/
ssize_t		nf;	/* number of fields	*/
Freq_t*		colf;	/* column frequencies	*/
#endif
{
	ssize_t	k, c, endc, f, cost;
	Freq_t	freq;

	cost = nf*8;
	for(c = 0, f = 0; f < nf; ++f)
	{	memset(freq, 0, sizeof(Freq_t));
		for(endc = c+fldz[f]; c < endc; ++c)
			for(k = 0; k < 256; ++k)
				freq[k] += colf[c][k];
		cost += rdhuffman(freq);
	}
	return cost;
}

/* pzip-like dynamic programming algorithm adapted for field grouping */
typedef struct _seg_s	Seg_t;
struct _seg_s
{	ssize_t	wght;	/* weight of segment		*/
	Seg_t*	next;	/* next seg in best partition	*/
};

#if __STD_C
static ssize_t fldgrouping(ssize_t* fldz, ssize_t nf, Freq_t* colf)
#else
static ssize_t fldgrouping(fldz, nf, colf)
ssize_t*	fldz;	/* field lengths 	*/
ssize_t		nf;	/* number of fields	*/
Freq_t*		colf;	/* column frequencies	*/
#endif
{
	ssize_t		l, r, k, w, sz;
	Freq_t		allf, *fldf;
	Seg_t		**seg, *best, *s;

	/* allocate the byte frequency matrix and the pzip memoizing matrix.
	** seg[l][r] represents the field group fldz[l],fldz[l+1]...fldz[r].
	*/
	sz = nf*sizeof(Freq_t) + nf*sizeof(Seg_t*) + nf*nf*sizeof(Seg_t);
	if(!(fldf = (Freq_t*)calloc(1,sz)) )
		return -1;
	seg = (Seg_t**)(fldf+nf);
	seg[0] = (Seg_t*)(seg+nf);
	for(k = 1; k < nf; ++k)
		seg[k] = seg[k-1]+nf;

	/* sum byte frequencies for each field */
	fldfreq(fldf, fldz, nf, colf);

	for(l = 0; l < nf; ++l) /* compute initial matrix weights */
	{	memset(allf, 0, sizeof(allf));
		for(r = l; r < nf; ++r)
		{	for(k = 0; k < 256; ++k) /* stats for all columns */
				allf[k] += fldf[r][k];
			seg[l][r].wght = rdhuffman(allf);
		}
	}

	for(r = nf-1; r >= 0; --r) /* pzip dynamic program without recursion */
	{	best = &seg[r][r]; /* find best of row r */
		for(k = r+1; k < nf; ++k) 
			if(seg[r][k].wght <= best->wght)
				best = &seg[r][k]; /* <= favors large |k-r| */

		if(r > 0) /* link all of column r-1 to the best of row r */
		{	for(k = r-1; k >= 0; --k)
			{	seg[k][r-1].next  = best;
				seg[k][r-1].wght += best->wght;
			}
		}
	}

	for(k = 0, s = best; s; k += 1, s = s->next)
	{	l = (s - seg[0]) / nf; /* left side of group	*/
		r = (s - seg[0]) % nf; /* right side of group	*/
		for(sz = 0; l <= r; ++l)
			sz += fldz[l];
		fldz[k] = sz;
	}

	free(fldf);
	return k;
}

#if __STD_C
static int rdfixed(Vcrdformat_t* rdf, Vcchar_t* data, ssize_t nc, ssize_t nr, ssize_t algn, int merge)
#else
static int rdfixed(rdf, data, nc, nr, algn, merge)
Vcrdformat_t*	rdf;
Vcchar_t*	data;
ssize_t		nc;
ssize_t		nr;
ssize_t		algn;
int		merge;
#endif
{
	ssize_t		c, n, nt, nf, fldc, allc;
	ssize_t		*fldz, *tmpz, *mark;
	Freq_t		*colf;

	if(!(fldz = (ssize_t*)calloc(nc, sizeof(ssize_t)) ) )
		return -1;
	if(!(colf = (Freq_t*)calloc(nc, sizeof(Freq_t) + 2*sizeof(ssize_t)) ) )
	{	free(fldz);
		return -1;
	}
	mark = (ssize_t*)(colf+nc);
	tmpz = mark + nc;

	if(nc < 4 || nr < 16) /* too little data */
	{	fldz[0] = nc; nf = 1;
		allc = fldc = 1;
		goto done;
	}

	colfreq(colf, data, nc, nr);

	tmpz[0] = nc; /* cost of coding as a single field */
	allc = fldcost(tmpz, 1, colf);

	fldc = -1; /* compute field partition and cost */
	fldmark(mark, data, nc, nr, colf); /* field marking */
	if(algn > 0 && (nc%algn) == 0 )
	{	nf = fldpartition(fldz, mark, nc, algn);
		/**/DEBUG_ASSERT(printfield(fldz,nf,'|'));
		if(merge && (n = fldgrouping(fldz, nf, colf)) > 0 )
			nf = n;
		/**/DEBUG_ASSERT(printfield(fldz,nf,':'));
		fldc = fldcost(fldz, nf, colf);
	}
	else for(algn = 4; algn >= 1; algn /= 2)
	{	if((nc%algn) != 0 )
			continue;
		nt = fldpartition(tmpz, mark, nc, algn);
		/**/DEBUG_ASSERT(printfield(tmpz,nt,'|'));
		if(merge && (n = fldgrouping(tmpz, nt, colf)) > 0 )
			nt = n;
		/**/DEBUG_ASSERT(printfield(tmpz,nt,':'));
		c = fldcost(tmpz, nt, colf);
		if(fldc < 0 || c < fldc)
		{	fldc = c; nf = nt;
			memcpy(fldz, tmpz, nf*sizeof(ssize_t));
		}
	}

done:	free(colf);

	if(allc <= 0 || fldc <= 0 || (rdf->perf > 0. && ((double)fldc)/allc > rdf->perf) )
	{	free(fldz);
		return 0;
	}
	else /* good partition, keep it */
	{	rdf->fsep = rdf->rsep = -1;
		rdf->fldn = nf;
		rdf->fldz = fldz;
		rdf->perf = ((double)fldc)/allc;
		return 1;
	}
}

#if __STD_C
Vcrdformat_t* vcrdformat(Vcchar_t* data, ssize_t dtsz, int rsep, ssize_t algn, int merge)
#else
Vcrdformat_t* vcrdformat(data, dtsz, rsep, algn, merge)
Vcchar_t*	data;	/* data to compute format 	*/
ssize_t		dtsz;	/* size of data			*/
int		rsep;	/* known record separator	*/
ssize_t		algn;	/* alignment for fixed fields	*/
int		merge;
#endif
{
	ssize_t		nc, nr, sz;
	Vcrdformat_t	rdt, *rdf;

	if(!data || dtsz <= 0)
		return NIL(Vcrdformat_t*);

	memset(&rdt, 0, sizeof(Vcrdformat_t));

	if(rsep > 0) /* check fsep only */
	{	if(rdsepar(&rdt, data, dtsz, rsep) <= 0 )
		{	rdt.rsep = rdt.fsep = rsep;
			rdt.perf = 1.0;
		}
	}
	else
	{	sz = dtsz < 1024*1024 ? dtsz : 1024*1024; /* max training data size */

		if((nc = vcperiod(data, sz)) > 0 && (nr = sz/nc) > 0 )
			rdfixed(&rdt, data, nc, nr, algn, merge);

		if(rdt.perf <= 0.0 || rdt.perf > 0.75)
		{	for(rsep = '\n';; ) /* check all possible rsep's */
			{	if(rdsepar(&rdt, data, sz, rsep) > 0 )
					break;
#if 0 /* reinstall this code for general rsep computation */
				for(;;) /* move to the next credible rsep */
				{	rsep = (rsep+1)%256;
					if(rsep == '\n' || OKRSEP(rsep))
						break;
				}
#endif
				if(rsep == '\n') /* have checked everything */
					break;
			}
		}
	}

	/* construct data to return */
	if(rdt.fldn <= 0 && rdt.perf <= 0.0 )
		rdf = NIL(Vcrdformat_t*); /* failed to find any structure */
	else	rdf = (Vcrdformat_t*)calloc(1, sizeof(Vcrdformat_t)+rdt.fldn*sizeof(ssize_t));
	if(rdf)
	{	memcpy(rdf, &rdt, sizeof(Vcrdformat_t));
		if(rdf->fldn > 0)
		{	rdf->fldz = (ssize_t*)(rdf+1);
			memcpy(rdf->fldz, rdt.fldz, rdf->fldn*sizeof(ssize_t));
		}
	}

	if(rdt.fldn > 0 && rdt.fldz)
		free(rdt.fldz);

	return rdf;
}
