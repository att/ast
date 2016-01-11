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
#include	<vgraph.h>
#include	<vcrdb.h>

/*	Transforming relational data for better compression.
**
**	Written by Binh Vo and Kiem-Phong Vo (05/17/2006).
*/

#define DO_LCS	0	/* 1 if doing LCS for matching */

/* compute the Hamming distance of two strings */
#if _STD_C
static ssize_t hamming(Vcchar_t* s1, ssize_t n1, Vcchar_t* s2, ssize_t n2)
#else
static ssize_t hamming(s1, n1, s2, n2)
Vcchar_t*	s1;	/* first string		*/
ssize_t		n1;
Vcchar_t*	s2;	/* second string	*/
ssize_t		n2;
#endif
{
	ssize_t		n, h;

	n1 = n1 > n2 ? n2 : n1;
	for(h = 0, n = 0; n < n1; ++n)
	{	if(s1[n] == s2[n])
			h += 1;
	}
	return h; 
}

#if DO_LCS
/* compute the length of a longest common subsequence of two strings */
#if _STD_C
static ssize_t lcs(Vcchar_t* s1, ssize_t n1, Vcchar_t* s2, ssize_t n2, ssize_t* here, ssize_t* last)
#else
static ssize_t lcs(s1, n1, s2, n2, here, last)
Vcchar_t*	s1;	/* first string		*/
ssize_t		n1;
Vcchar_t*	s2;	/* second string	*/
ssize_t		n2;
ssize_t*	here;	/* working space	*/
ssize_t*	last;
#endif
{
	Vcchar_t	*s;
	ssize_t		n, i, k;

	if(n1 <= 0 || n2 <= 0)
		return 0;

	if(n1 < n2) /* make n1 the longer string */
	{	n = n1; n1 = n2; n2 = n;
		s = s1, s1 = s2; s2 = s;
	}

	for(k = 0; k < n2; ++k) /* initialize first row */
	{	if(s1[0] == s2[k])
			break;
		here[k] = 0;
	}
	for(; k < n2; ++k)
		here[k] = 1;

	for(i = 1; i < n1; ++i) /* now the rest of the rows */
	{	int	*t, left, abov, diag;

		/* here points to the current row */
		t = here; here = last; last = t;

		/* first element of this row */
		here[0] = s1[i] == s2[0] ? 1 : 0;

		/* the rest of this row */
		for(abov = last[0], left = here[0], k = 1; k < n2; ++k)
		{	diag = abov + (s1[i] == s2[k] ? 1 : 0);
			abov = last[k];
			if(left < abov)
				left = abov >= diag ? abov : diag;
			else if(left < diag)
				left = diag;
			here[k] = left;
		}
	}

	return here[n2-1];
}
#endif

/* Compute a proxy for compression rate based on matching */
#if __STD_C
ssize_t rdmatch(Vcrdtable_t* tbl, ssize_t f, ssize_t* vect, ssize_t* smpl, ssize_t n_smpl)
#else
ssize_t rdmatch(tbl, f, vect, smpl, n_smpl)
Vcrdtable_t*	tbl;	/* table of all records and fields	*/
ssize_t		f;	/* field to compute weight		*/
ssize_t*	vect;	/* permutation vector, NULL for ident	*/
ssize_t*	smpl;	/* indicators for rows to sample	*/
ssize_t		n_smpl;
#endif
{
	ssize_t		s, r, k;
	Vcchar_t	*dt, *pdt;
	ssize_t		sz, psz, dtsz, mtch;
	Vcrdrecord_t	*rcrd;

#if DO_LCS
	ssize_t		*here, *last;
	if(!(here = (ssize_t*)malloc(2*tbl->fld[f].maxz*sizeof(ssize_t))) )
		return -1;
	last = here + tbl->fld[f].maxz;
#endif

	dtsz = mtch = 0;
	for(rcrd = tbl->fld[f].rcrd, s = 0; s < n_smpl; ++s)
	{	r = smpl[s];
		if((k = vect ? vect[r] : r) < 0 || k >= tbl->recn)
			return -1;
		dt = rcrd[k].data; sz = rcrd[k].dtsz;
#if DO_LCS
		if(s > 0)
			mtch += lcs(pdt, psz, dt, sz, here, last);
#else
		if(s > 0)
			mtch += hamming(pdt, psz, dt, sz);
#endif
		dtsz += sz; pdt = dt; psz = sz;
	}

#if DO_LCS
	free(here);
#endif

	return dtsz - mtch;
}

/* Get the compressive entropy of a field after permuting records by some transform vector */
#if __STD_C
ssize_t rdentropy(Vcrdtable_t* tbl, ssize_t f, ssize_t* vect, ssize_t* smpl, ssize_t n_smpl, Vcodex_t* vcw)
#else
ssize_t rdentropy(tbl, f, vect, smpl, n_smpl, vcw)
Vcrdtable_t*	tbl;	/* table of all records and fields	*/
ssize_t		f;	/* field to compute weight		*/
ssize_t*	vect;	/* permutation vector, NULL for ident	*/
ssize_t*	smpl;	/* indicators for rows to sample	*/
ssize_t		n_smpl;
Vcodex_t*	vcw;	/* weight function, NULL for match()	*/
#endif
{
	ssize_t		dtsz, s, r, k;
	Vcchar_t	*data, *dt;
	Vcrdrecord_t	*rcrd;

	if(!tbl || !vcw || f < 0 || f >= tbl->fldn)
		return -1;

	if(vcw == VCRD_MATCH)
		return rdmatch(tbl, f, vect, smpl, n_smpl);

	/* construct the string of field data */
	if(!(data = (Vcchar_t*)malloc(tbl->recn*tbl->fld[f].maxz)) )
		return -1;
	for(dt = data, dtsz = 0, rcrd = tbl->fld[f].rcrd, s = 0; s < n_smpl; ++s)
	{	r = smpl[s];
		if((k = vect ? vect[r] : r) < 0 || k >= tbl->recn)
			return -1;

		memcpy(dt, rcrd[k].data, rcrd[k].dtsz);
		dt += rcrd[k].dtsz;
		dtsz += rcrd[k].dtsz;
	}

	/* run compressor */
	r = dtsz <= 0 ? 0 : vcapply(vcw, data, dtsz, &dt);

	vcbuffer(vcw, NIL(Vcchar_t*), -1, -1);
	free(data);

	return r;
}

/* training to compute a plan for data transforming */
#if __STD_C
Vcrdplan_t* vcrdmakeplan(Vcrdtable_t* tbl, Vcodex_t* vcw)
#else
Vcrdplan_t* vcrdmakeplan(tbl, wvc)
Vcrdtable_t*	tbl;	/* table of data	*/
Vcodex_t*	vcw;	/* general compressor	*/
#endif
{
	Grnode_t	*nd, *pd;
	Gredge_t	*e;
	ssize_t		f, p, z;
	ssize_t		fldn, recn, maxz;
	Graph_t		*gr = NIL(Graph_t*);
	ssize_t		n_smpl, *smpl, *wght = NIL(ssize_t*); /* field weights */
	Vcrdplan_t	*pl = NIL(Vcrdplan_t*);

	if(!tbl || (fldn = tbl->fldn) <= 0)  /* nothing to do */
		return NIL(Vcrdplan_t*);
	recn = tbl->recn;

	/* construct the identity transformation plan */
	if(!(pl = (Vcrdplan_t*)malloc(sizeof(Vcrdplan_t) + (fldn-1)*sizeof(ssize_t))) )
		return NIL(Vcrdplan_t*);
	pl->fldn = fldn;
	for(f = 0; f < fldn; ++f)
		pl->pred[f] = f;

	if(fldn <= 0 || recn <= 0 || !vcw) /* no transformation */
		return pl;

	/* allocate space for intermediate computation */
	if(!(wght = (ssize_t*)calloc(fldn+recn, sizeof(ssize_t))) )
	{	free(pl); pl = NIL(Vcrdplan_t*);
		goto done;
	}
	smpl = wght + fldn;

	VCRSEED(0xdeadbeef); /* reset random number generator */
	for(n_smpl = 16; n_smpl*n_smpl < recn; ) /* sqrt(recn) */
		n_smpl += 1; /**/DEBUG_PRINT(2,"n_smpl=%d\n",n_smpl);
	for(z = (z = n_smpl/4)*n_smpl >= 4096 ? z : 4096/n_smpl; z > 0; --z)
	{	p = VCRAND()%recn; /* sample a set centered around p */
		if((f = p - n_smpl) < 4) /* first few records tend to be spurious */
			f = 4;
		if((p = f + 2*n_smpl) > recn)
			p = recn;
		for(; f < p; ++f)
			smpl[f] = 1;
	}
	n_smpl = 0; /* shuffle sampled positions to start of list */
	for(f = 0; f < tbl->recn; ++f)
		if(smpl[f] == 1)
			smpl[n_smpl++] = f;
	/**/DEBUG_PRINT(2,"fldn=%d,", fldn); DEBUG_PRINT(2,"recn=%d, ", recn); DEBUG_PRINT(2,"smpl=%d\n", n_smpl);

	for(maxz = 0, f = 0; f < fldn; ++f)
		if(tbl->fld[f].maxz > maxz)
			maxz = tbl->fld[f].maxz;
	maxz *= 2*tbl->recn; /* to adjust the compressive entropy for computing optimum branching */

	/* compute the self-weight and transform vector of each field */
	for(f = 0; f < fldn; ++f)
	{	if(vcrdvector(tbl, f, NIL(Vcchar_t*), 0, VC_ENCODE) == 1)
			wght[f] = 0; /* all field values are equal */
		else if((wght[f] = rdentropy(tbl, f, NIL(ssize_t*), smpl, n_smpl, vcw)) < 0)
		{	free(pl); pl = NIL(Vcrdplan_t*);
			goto done;
		}
		else	wght[f] = maxz - wght[f];
	}

	/* compute the maximum branching that determines the optimum dependency relations */
	if(!(gr = gropen(NIL(Grdisc_t*), GR_DIRECTED)) )
		goto done;
	for(f = 0; f < fldn; ++f)
	{	/* create the node corresponding to this column */
		if(!(nd = grnode(gr, (Void_t*)f, 1)) )
			goto done;
		if(wght[f] <= 0) /* reorder won't do anything */
			continue;

		for(p = 0; p < f; ++p) /* build graph edges */
		{	if(wght[p] <= 0)
				continue;

			if(!(pd = grnode(gr, (Void_t*)p, 0)) )
				goto done;
			if((z = maxz - rdentropy(tbl, f, tbl->fld[p].vect, smpl, n_smpl, vcw)) > wght[f])
			{	if(!(e = gredge(gr, pd, nd, (Void_t*)0, 1)) )
					goto done;
				grbrweight(e, z);
			}
			if((z = maxz - rdentropy(tbl, p, tbl->fld[f].vect, smpl, n_smpl, vcw)) > wght[p])
			{	if(!(e = gredge(gr, nd, pd, (Void_t*)0, 1)) )
					goto done;
				grbrweight(e, z);
			}
		}
	}
	if((z = grbrgreedy(gr)) < 0 )
		goto done; /**/DEBUG_PRINT(2,"Grbranching weight = %d\n", z);

	/* the transform plan */
	for(nd = dtfirst(gr->nodes); nd; nd = dtnext(gr->nodes,nd))
		if(nd->iedge)
			pl->pred[(ssize_t)nd->label] = (ssize_t)nd->iedge->tail->label;

done:	if(gr)
		grclose(gr);
	if(wght)
		free(wght);

	tbl->recn = recn; /* restore original data */
	for(f = 0; f < fldn; ++f)
		tbl->fld[f].type &= ~VCRD_VECTOR;

	return pl;
}

#if __STD_C
void vcrdfreeplan(Vcrdplan_t* pl)
#else
void vcrdfreeplan(pl)
Vcrdplan_t*	pl;
#endif
{
	if(pl)
		free(pl);
}

/* transform a field based on a transform vector */
#if __STD_C
static void fldtransform(Vcrdtable_t* tbl, ssize_t f, ssize_t* vect, Vcrdrecord_t* rtmp, int type)
#else
static void fldtransform(tbl, f, vect, rtmp, type)
Vcrdtable_t*	tbl;	/* table data		*/
ssize_t		f;	/* field to transform	*/
ssize_t*	vect;	/* transform vector	*/
Vcrdrecord_t*	rtmp;	/* temp record space	*/
int		type;	/* VC_ENCODE/DECODE	*/
#endif
{
	ssize_t		r, recn = tbl->recn;
	Vcrdrecord_t	*rcrd = tbl->fld[f].rcrd;

	if(type == VC_ENCODE)
	{	for(r = 0; r < recn; ++r)
		{	rtmp[r].data = rcrd[vect[r]].data; 
			rtmp[r].dtsz = rcrd[vect[r]].dtsz; 
		}
	}
	else
	{	for(r = 0; r < recn; ++r)
		{	rtmp[vect[r]].data = rcrd[r].data; 
			rtmp[vect[r]].dtsz = rcrd[r].dtsz; 
		}
	}

	memcpy(rcrd, rtmp, recn*sizeof(Vcrdrecord_t));
}

/* reconstruct a field based on its predictor. */
#if __STD_C
static int fldinvert(Vcrdtable_t* tbl, Vcrdplan_t* pl, ssize_t f, ssize_t level, Vcrdrecord_t* rtmp)
#else
static int fldinvert(tbl, pl, f, level, rtmp)
Vcrdtable_t*	tbl;	/* table data		*/
Vcrdplan_t*	pl;	/* transform plan	*/
ssize_t		f;	/* field to rebuild	*/
ssize_t		level;	/* recursion level	*/
Vcrdrecord_t*	rtmp;	/* temp record space	*/
#endif
{
	ssize_t		p;

	if(level >= pl->fldn) /* data was corrupted */
		return -1;

	if((p = pl->pred[f]) == f) /* field already rebuilt */
		return 0;

	/* recursively invert predictor of f */
	if(fldinvert(tbl, pl, p, level+1, rtmp) < 0 )
		return -1;

	/* construct transform vector */
	if(vcrdvector(tbl, p, NIL(Vcchar_t*), 0, VC_DECODE) < 0 )
		return -1;

	/* now invert field f to its original data order */
	fldtransform(tbl, f, tbl->fld[p].vect, rtmp, VC_DECODE);
	pl->pred[f] = f;

	return 0;
}

#if __STD_C
int vcrdexecplan(Vcrdtable_t* tbl, Vcrdplan_t* pl, int type)
#else
int vcrdexecplan(tbl, pl, type)
Vcrdtable_t*	tbl;	/* table data			*/
Vcrdplan_t*	pl;	/* transform plan for data	*/
int		type;	/* VC_ENCODE or VC_DECODE	*/
#endif
{
	ssize_t		fldn, recn, f, p;
	Vcrdrecord_t	*rtmp;

	if(!tbl || !pl || tbl->fldn != pl->fldn )
		return -1;
	if(type != VC_ENCODE && type != VC_DECODE)
		return -1;

	if((fldn = tbl->fldn) <= 0 || (recn = tbl->recn) <= 0)
		return 0;

	if(!(rtmp = (Vcrdrecord_t*)malloc(recn*sizeof(Vcrdrecord_t))) )
		return -1;

	for(f = 0; f < fldn; ++f) /* no transform vector done yet */
		tbl->fld[f].type &= ~VCRD_VECTOR;

	if(type == VC_ENCODE)
	{	for(f = 0; f < fldn; ++f) /* build needed transform vectors first */
			if((p = pl->pred[f]) != f)
				vcrdvector(tbl, p, NIL(Vcchar_t*), 0, VC_ENCODE );
		for(f = 0; f < fldn; ++f) /* now transform fields with predictors */
			if((p = pl->pred[f]) != f)
				fldtransform(tbl, f, tbl->fld[p].vect, rtmp, VC_ENCODE);
	}
	else
	{	for(f = 0; f < fldn; ++f)
			if(fldinvert(tbl, pl, f, 0, rtmp) < 0 )
				break;
	}

	free(rtmp);

	return f < fldn ? -1 : 0;
}
