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
#include	<vctable.h>

/*	Table column dependency transform
**
**	Written by Binh Dao Vo and Kiem-Phong Vo
*/

#define COLSZ	(64*1024)	/* min data to compute #cols	*/
#define UPPERSZ	(3*1024*1024)	/* max data to compute plan	*/

#define TBL_COLUMNS	1	/* defining #columns		*/
#define TBL_LEFT	2	/* use left to right dependency	*/
#define TBL_RIGHT	3	/* use right to left dependency	*/
#define TBL_BOTH	4	/* use right to left dependency	*/
#define TBL_CEE		5	/* use conditional entropy	*/
#define TBL_RLE		6	/* use run-length entropy	*/
#define TBL_SINGLE	7	/* single predictor only	*/

#define SETPLAN(c,p) \
	{	if((p) != (c)->plan && (c)->plan) \
			vctblfreeplan((c)->plan); \
		(c)->plan  = (p); \
	}

typedef struct _tblindex_s
{	ssize_t*	idx;	/* sorted indices of a column	*/
	ssize_t*	bkt;	/* bucket counts for bytes	*/
} Tblindex_t;

typedef struct _tblctxt_s
{	Vccontext_t	ctxt;
	Vctblplan_t*	plan;	/* plan for transforming data	*/
	ssize_t		ncols;	/* #columns for this context	*/
	int		flags;	/* flags to open a plan		*/
} Tblctxt_t;

typedef struct _table_s
{	Vcodex_t*	trans;	/* transpose transform		*/
	Tblctxt_t*	ctxt;	/* default context		*/

	Tblindex_t*	cidx;	/* 1-sorted column indices	*/ 		
	ssize_t*	idx;	/* space for 2-sorted indices	*/

	ssize_t		ncols;	/* number of columns		*/
	ssize_t		nrows;	/* number of rows		*/
	Vcchar_t*	data;	/* table in column-major order	*/
} Table_t;

static Vcmtarg_t _Tbldata[] =
{	{ "columns", "Number of columns is defined as 'columns=length'.", (Void_t*)TBL_COLUMNS },
	{ "left", "Use left to right column dependency (default)", (Void_t*)TBL_LEFT },
	{ "right", "Use right to left column dependency", (Void_t*)TBL_RIGHT },
	{ "both", "Use both sides column dependency", (Void_t*)TBL_BOTH },
	{ "rle", "Use run-length entropy (default)", (Void_t*)TBL_RLE },
	{ "cee", "Use conditional empirical entropy", (Void_t*)TBL_CEE },
	{ "single", "Use a single predictor", (Void_t*)TBL_SINGLE },
	{ 0, 0, (Void_t*)0 }
};

#if __STD_C
static Vctblplan_t* tbltrain(const Void_t* train, size_t trsz, ssize_t ncols, int flags)
#else
static Vctblplan_t* tbltrain(train, trsz, ncols, flags)
Void_t*	train;	/* training data */
size_t	trsz;
ssize_t	ncols;
int	flags;
#endif
{
	ssize_t		sz, uppersz;
	Vctblplan_t	*plan;

	if(!train || trsz <= 0)
		return NIL(Vctblplan_t*);

	/* compute # of columns */
	if(ncols <= 0)
	{	for(sz = trsz < COLSZ ? trsz : COLSZ; sz <= trsz; sz *= 2)
			if((ncols = vcperiod(train, sz)) > 0 )
				break;
		if(ncols <= 0)
			ncols = 1;
	}

	if(trsz < ncols)
		return NIL(Vctblplan_t*);
	if((uppersz = ncols*ncols) < UPPERSZ)
		uppersz = UPPERSZ;
	if((sz = trsz) > uppersz)
		sz = uppersz;

	if(!(plan = vctblmakeplan(train, (sz/ncols)*ncols, ncols, flags)) )
		return NIL(Vctblplan_t*);

	return plan;
}

/* free all data pertaining to column indices */
#if __STD_C
static void clrindex(Table_t* tbl)
#else
static void clrindex(tbl)
Table_t*	tbl;
#endif
{
	Tblindex_t	*ip, *endip;

	if(tbl->cidx)
	{	for(endip = (ip = tbl->cidx) + tbl->ncols; ip < endip; ++ip)
			if(ip->idx)
				free(ip->idx);

		free(tbl->cidx);
		tbl->cidx = NIL(Tblindex_t*);
	}

	if(tbl->idx)
	{	free(tbl->idx);
		tbl->idx = NIL(ssize_t*);
	}
}

/* allocate new index space */
#if __STD_C
static int initindex(Table_t* tbl)
#else
static int initindex(tbl)
Table_t*	tbl;
#endif
{
	if(!(tbl->cidx = (Tblindex_t*)calloc(tbl->ncols, sizeof(Tblindex_t))) )
		return -1;
	if(!(tbl->idx = malloc(tbl->nrows*sizeof(ssize_t))) )
		return -1;
	return 0;
}

#if __STD_C
static ssize_t* getindex(Table_t* tbl, ssize_t pred1, ssize_t pred2)
#else
static ssize_t* getindex(tbl, pred1, pred2)
Table_t*	tbl;	/* to store sorted indices	*/
ssize_t		pred1;	/* primary predecessor index	*/
ssize_t		pred2;	/* secondary predecessor index	*/
#endif
{
	ssize_t		i, p;
	ssize_t		*idx, *bkt, bckt[256];
	Vcchar_t	*data;
	ssize_t		nrows = tbl->nrows;

	/* compute indices sorted by first predictor */
	if(!(idx = tbl->cidx[pred1].idx) )
	{	if(!(idx = (ssize_t*)malloc((nrows+256)*sizeof(ssize_t))) )
			return NIL(ssize_t*);
		tbl->cidx[pred1].idx = idx;
		tbl->cidx[pred1].bkt = bkt = idx + nrows;

		data = tbl->data + pred1*nrows;
		vcbcktsort(idx, NIL(ssize_t*), nrows, data, bkt);
	}
	else	bkt = tbl->cidx[pred1].bkt;

	if(pred2 >= 0) /* additionally sort indices by pred2 */
	{
		data = tbl->data + pred2*nrows;
		for(p = 0, i = 0; i < 256; ++i)
		{	if(bkt[i] == p)
				continue;
			vcbcktsort(tbl->idx+p, idx+p, bkt[i]-p, data, bckt);
			p = bkt[i];
		}

		idx = tbl->idx;
	}

	return idx;
}

#if __STD_C
static ssize_t table(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t table(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	reg Vcchar_t	*dt, *zip, *endz;
	reg ssize_t	*idx;
	Vcchar_t	*zipdt, *hddt;
	ssize_t		np, p, sz, hdsz, dtsz, ncols, nrows;
	float		ratio, planratio;
	Vcio_t		io;
	Vctblplan_t	*plan;
	Tblctxt_t	*ctxt = vcgetcontext(vc, Tblctxt_t*);
	Table_t		*tbl = vcgetmtdata(vc, Table_t*);
	int		trained = 0;
	/**/DEBUG_DECLARE(static int, kpvcall) /**/DEBUG_COUNT(kpvcall); 

#define TRAINED		1
#define RETRAINING	2
re_train: /* recurse to here if necessary to retrain data */

	vc->undone = 0;
	if(size == 0)
		return 0;

	if(!tbl || !ctxt)
		return -1;

	clrindex(tbl);

	plan = ctxt->plan;
	if(trained != RETRAINING)
		ncols = ctxt->ncols;
	if(ncols <= 0 && plan)
		ncols = plan->ncols;

	/* compute the transform plan if not done yet */
	if(!plan || plan->ncols <= 1 || plan->ncols != ncols ||
	   (!plan->good && size > plan->train) )
	{	plan = tbltrain(data, size, ncols, ctxt->flags);
		trained = TRAINED; /* so we won't retrain again */
		SETPLAN(ctxt, plan);
		/**/DEBUG_PRINT(2, "\tNcols=%d, just trained\n", plan->ncols);
	}

	if(!plan || (ncols = plan->ncols) <= 0)
	{	vc->undone = size;
		return 0;
	}

	/* get header data */
	if((hdsz = vctblencodeplan(plan, (Void_t**)&hddt)) < 0)
		return -1;

	/* get memory for rearranged data */
	if(!(zipdt = vcbuffer(vc, NIL(Vcchar_t*), size, vcsizeu(hdsz)+hdsz)) )
		return -1;

	tbl->ncols = ncols;
	tbl->nrows = nrows = size/ncols;
	vc->undone = size - ncols*nrows; /* amount left unprocessed */
	if((dtsz = ncols*nrows) == 0)	/* amount to be processed  */
		return 0;

	/* allocate space for new indices */
	if(initindex(tbl) < 0)
		return -1;

	/* make column-major data */
	vcsetmtarg(tbl->trans, "columns", TYPECAST(Void_t*,ncols), 2);
	if(vcapply(tbl->trans, data, dtsz, &tbl->data) != dtsz)
		return -1;

	/* transform unpredicted columns first, then unpredicted columns */
	for(zip = zipdt, np = 0; np < ncols; ++np)
	{	if(plan->trans[np].pred1 >= 0)
			break;
		dt = tbl->data + plan->trans[np].index*nrows;
		for(endz = zip+nrows; zip < endz; )
			*zip++ = *dt++;
	}
	for(p = np; p < ncols; ++p)
	{	if(!(idx = getindex(tbl, plan->trans[p].pred1, plan->trans[p].pred2)) )
			return -1;
		dt = tbl->data + plan->trans[p].index*nrows;
		for(endz = zip+nrows; zip < endz; )
			*zip++ = dt[*idx++];
	}

	vcbuffer(tbl->trans, tbl->data, -1, -1); /* free transposed data */

	dt = zipdt; sz = dtsz;
	p = vcsizeu(hdsz)+hdsz; /* extra space for coding the plan */
	if(vcrecode(vc, &zipdt, &sz, p, 0) < 0 )
		return -1;
	if(zipdt != dt)
		vcbuffer(vc, dt, -1, -1);

	/* now prepend the plan if any */
	zipdt -= p; sz += p;
	vcioinit(&io, zipdt, p);
	vcioputu(&io, hdsz);
	if(hdsz > 0)
	{	vcioputs(&io, hddt, hdsz);
		vctblencodeplan(plan, NIL(Void_t**)); /* free header memory */
	}

	if((ratio = dtsz/(float)sz) >= (planratio = plan->dtsz/(float)plan->cmpsz) )
	{	plan->dtsz = dtsz;
		plan->cmpsz = sz;
	}
	else if(trained != TRAINED && vc->coder && dtsz > ncols*ncols &&
		((planratio <= 1. && ratio < planratio) || ratio < .5*planratio) )
	{	/**/DEBUG_PRINT(2,"\tNcols=%d, retraining, ", ncols);
		/**/DEBUG_PRINT(2,"previous ratio=%.2f, ", planratio);
		/**/DEBUG_PRINT(2,"current ratio=%.2f\n", ratio);
		trained = RETRAINING;
		SETPLAN(ctxt, NIL(Vctblplan_t*));
		goto re_train;
	}

	/**/DEBUG_PRINT(2,"Ncols=%d, ",ncols); DEBUG_PRINT(2,"Compressed ratio=%.2f\n",ratio);
	if(!(zipdt = vcbuffer(vc, zipdt, sz, -1)) ) /* reduce size to fit */
		return -1;
	if(out)
		*out = zipdt;
	return sz;
}

#if __STD_C
static ssize_t untable(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t untable(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		*idx;
	Vcchar_t	*zip, *endz, *dt;
	Vcchar_t	*zipdt, *output;
	ssize_t		p, sz, ncols, nrows;
	Vcio_t		io;
	Vctblplan_t	*plan;
	Tblctxt_t	*ctxt = vcgetcontext(vc, Tblctxt_t*);
	Table_t		*tbl = vcgetmtdata(vc, Table_t*);

	if(size == 0)
		return 0;

	if(!ctxt && !(ctxt = (Tblctxt_t*)vcinitcontext(vc, (Vccontext_t*)ctxt)) )
		return -1;

	clrindex(tbl);

	/* get header data */
	vcioinit(&io, data, size);
	if((sz = vciogetu(&io)) < 0)
		return -1;
	dt = vcionext(&io); size = vciomore(&io);
	if(sz > 0 )
	{	if(!(plan = vctbldecodeplan(dt, sz)) )
			return -1;
		SETPLAN(ctxt, plan);
	}
	data = (Void_t*)(dt+sz); size -= sz;

	if(!(plan = ctxt->plan) || (ncols = plan->ncols) <= 0 )
		return -1;

	tbl->ncols = ncols;

	/* reconstruct transformed data as necessary */
	zipdt = (Vcchar_t*)data; sz = (ssize_t)size;
	if(vcrecode(vc, &zipdt, &sz, 0, 0) < 0)
		return -1;
	size = (size_t)sz;

	/* get the unpredicted data */
	vcioinit(&io, zipdt, size);
	if((size%ncols) != 0 || (tbl->nrows = nrows = size/ncols) <= 0 )
		return -1;

	for(p = 0; p < ncols; ++p)
		if(plan->trans[p].pred1 >= 0)
			break;
	zip = vcionext(&io); vcioskip(&io, p*nrows);

	if(initindex(tbl) < 0) /* space for sorted index vectors */
		return -1;

	/* allocate space to reconstruct the data */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), size, 0)) )
		return -1;
	tbl->data = output;

	/* reconstruct the unpredicted columns first, then predicted columns */
	for(p = 0; p < ncols; ++p)
	{	if(plan->trans[p].pred1 >= 0)
			break;
		dt = tbl->data + plan->trans[p].index*nrows;
		for(endz = zip+nrows; zip < endz; )
			*dt++ = *zip++;
	}
	for(zip = vcionext(&io); p < ncols; ++p)
	{	if(!(idx = getindex(tbl, plan->trans[p].pred1, plan->trans[p].pred2)) )
			return -1;
		dt = tbl->data + plan->trans[p].index*nrows;
		for(endz = zip+nrows; zip < endz; )
			dt[*idx++] = *zip++;
	}

	vcbuffer(vc, zipdt, -1, -1); /* free the transformed data */

	/* reconstruct row-major data */
	vcsetmtarg(tbl->trans, "columns", TYPECAST(Void_t*,nrows), 2);
	if(vcapply(tbl->trans, output, size, &dt) != size)
		return -1;

	vcbuffer(vc, output, -1, -1); /* free the transposed data */

	if(out)
		*out = dt;
	return (ssize_t)size;
}


#if __STD_C
static int tblevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int tblevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Tblctxt_t	*ctxt;
	char		*data, val[128];
	Vcmtarg_t	*arg;
	Table_t		*tbl;

	if(type == VC_OPENING )
	{	if(!(tbl = (Table_t*)calloc(1, sizeof(Table_t))) )
			return -1;

		if(!(tbl->trans = vcopen(0, Vctranspose, "0", 0, VC_ENCODE)) )
		{	free(tbl);
			return -1;
		}
		if(!(tbl->ctxt = (Tblctxt_t*)vcinitcontext(vc, NIL(Vccontext_t*))) )
		{	vcclose(tbl->trans);
			free(tbl);
			return -1;
		}

		vcsetmtdata(vc, tbl);
		goto vc_setarg;
	}
	else if(type == VC_SETMTARG)
	{ vc_setarg:
		if(!(ctxt = vcgetcontext(vc, Tblctxt_t*)) )
			return -1;
		for(data = (char*)params; data; )
		{	data = vcgetmtarg(data, val, sizeof(val), _Tbldata, &arg);
			switch(TYPECAST(int,arg->data) )
			{ case TBL_COLUMNS:
				ctxt->ncols = (ssize_t)vcatoi(val);
				break;
			  case TBL_LEFT:
				ctxt->flags |= VCTBL_LEFT;
				break;
			  case TBL_RIGHT:
				ctxt->flags |= VCTBL_RIGHT;
				break;
			  case TBL_BOTH:
				ctxt->flags |= VCTBL_LEFT|VCTBL_RIGHT;
				break;
			  case TBL_CEE:
				ctxt->flags |= VCTBL_CEE;
				break;
			  case TBL_RLE:
				ctxt->flags |= VCTBL_RLE;
				break;
			  case TBL_SINGLE:
				ctxt->flags |= VCTBL_SINGLE;
				break;
			}
		}
		return 0;
	}
	else if(type == VC_INITCONTEXT)
	{	if(!params)
			return 0;
		if(!(ctxt = (Tblctxt_t*)calloc(1,sizeof(Tblctxt_t))) )
			return -1;
		*((Tblctxt_t**)params) = ctxt;
		return 1;
	}
	else if(type == VC_FREECONTEXT)
	{	if((ctxt = (Tblctxt_t*)params) )
		{	if(ctxt->plan)
				vctblfreeplan(ctxt->plan);
			free(ctxt);
		}
		return 0;
	}
	else if(type == VC_FREEBUFFER)
	{	if((tbl = vcgetmtdata(vc, Table_t*)) && tbl->trans)
			vcbuffer(tbl->trans, NIL(Vcchar_t*), -1, -1);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if((tbl = vcgetmtdata(vc, Table_t*)) )
		{	if(tbl->trans)
				vcclose(tbl->trans);
			clrindex(tbl);
			free(tbl);
		}
		vcsetmtdata(vc, NIL(Table_t*));
		return 0;
	}
	else	return 0;
}

Vcmethod_t _Vctable =
{	table,
	untable,
	tblevent,
	"table", "Fixed-length record table transform.",
	"[-?\n@(#)$Id: vcodex-table (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	_Tbldata,
	16*1024*1024,
	0
};

VCLIB(Vctable)
