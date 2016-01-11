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

/*	Transpose a table of some given row length.
**
**	Written by Binh Vo and Kiem-Phong Vo (04/24/2006)
*/

#define TR_PLAIN	1	/* strictly tranposed data only	*/
#define TR_SEPARATOR	2	/* setting record separator	*/
#define TR_COLUMNS	3	/* setting number of columns	*/

typedef struct _transflip_s
{	ssize_t		open;	/* next open record		*/
	ssize_t		rpos;	/* record position		*/
} Transflip_t;

typedef struct _transctxt_s
{	Vccontext_t	ctxt;
	ssize_t		ncols;	/* number of columns in table	*/
	int		rsep;	/* or record separator		*/
} Transctxt_t;

typedef struct _transpose_s
{	int		type;
	Transctxt_t*	ctxt;	/* default context		*/
} Transpose_t;

/* arguments passable to vcopen() */
static Vcmtarg_t _Transargs[] =
{
	{ "rsep", "Rows are separated  by 'rsep=character'.", (Void_t*)TR_SEPARATOR },
	{ "fsep", "This is equivalent to 'rsep'.", (Void_t*)TR_SEPARATOR },
	{ "columns", "Number of columns is defined by 'columns=length'.", (Void_t*)TR_COLUMNS },
	{ "0", "Only transposed data are coded, not meta-data.", (Void_t*)TR_PLAIN },
	{  0 , "Both transposed data and meta-data are coded.", (Void_t*)0 }
};

#if __STD_C
static ssize_t transflip(Vcchar_t* data, ssize_t dtsz, int rsep, Vcchar_t* flip)
#else
static ssize_t transflip(data, dtsz, rsep, flip)
Vcchar_t*	data;
ssize_t		dtsz;
int		rsep;
Vcchar_t*	flip;
#endif
{
	ssize_t		z, p, r, nrows;
	int		byte;
	Transflip_t	*fl;

	if(data[dtsz-1] != rsep)
		RETURN(-1);

	/* count number of rows */
	for(nrows = 0, z = 0; z < dtsz; ++z)
		if(data[z] == rsep)
			nrows += 1;

	/* allocate space for row data */
	if(!(fl = (Transflip_t*)calloc(nrows, sizeof(Transflip_t))) )
		RETURN(-1);
	
	/* compute record starts and record sizes */
	for(p = r = z = 0; z < dtsz; ++z)
	{	if(data[z] == rsep)
		{	fl[r].open = r; /* all slot starting out open */
			fl[r].rpos = p; /* record position */
			p = z+1;
			r += 1;
		}
	}

	/* now flip records */
	while(fl[r = 0].open < nrows)
	{	do
		{	if((p = fl[r].open) > r) /* a done record */
			{	fl[r].open = p < nrows ? fl[p].open : nrows;
				r = p;
			}
			else /* output one byte from this record */
			{	*flip++ = (byte = data[fl[r].rpos]);
				if(byte == rsep) /* record is done */
					fl[r].open = r+1;
				fl[r].rpos += 1;
				r += 1;
			}
		} while(r < nrows);
	}

	free(fl);
	return nrows;
}

#if __STD_C
static ssize_t unflip(Vcchar_t* data, ssize_t dtsz, int rsep, Vcchar_t* flip)
#else
static ssize_t unflip(data, dtsz, rsep, flip)
Vcchar_t*	data;
ssize_t		dtsz;
int		rsep;
Vcchar_t*	flip;
#endif
{
	ssize_t		z, p, r, nrows;
	Transflip_t	*fl;

	if(data[dtsz-1] != rsep)
		RETURN(-1);

	/* count number of rows */
	for(nrows = 0, z = 0; z < dtsz; ++z)
		if(data[z] == rsep)
			nrows += 1;

	/* allocate space for row data */
	if(!(fl = (Transflip_t*)calloc(nrows, sizeof(Transflip_t))) )
		RETURN(-1);
	for(r = 0; r < nrows; ++r)
		fl[r].open = r;

	/* compute the size of each row */
	for(r = z = 0; z < dtsz; )
	{	while((p = fl[r].open) > r) /* find row still open */
		{	fl[r].open = p < nrows ? fl[p].open : nrows;
			r = p;
		}

		if(r < nrows)
		{	fl[r].rpos += 1; /* add to its length */
			if(data[z] == rsep) /* this record is done */
				fl[r].open = r+1;
			z += 1;
		}

		if((r += 1) >= nrows) /* wrap around */
			r = 0;
	}

	/* allocate space for each record */
	for(p = r = 0; r < nrows; ++r)
	{	fl[r].open = r;
		z = fl[r].rpos; /* save current length */
		fl[r].rpos = p; /* set starting position */
		p += z; /* starting position for next record */
	}

	/* rebuild records */
	for(r = z = 0; z < dtsz; )
	{	while((p = fl[r].open) > r) /* find row still open */
		{	fl[r].open = p < nrows ? fl[p].open : nrows;
			r = p;
		}

		if(r < nrows)
		{	flip[fl[r].rpos++] = data[z];
			if(data[z] == rsep) /* this record is done */
				fl[r].open = r+1;
			z += 1;
		}

		if((r += 1) >= nrows)
			r = 0;
	}

	free(fl);
	return nrows;
}

#if __STD_C
static void transfixed(Vcchar_t* oldtbl, ssize_t nrows, ssize_t ncols, Vcchar_t* newtbl)
#else
static void transfixed(oldtbl, nrows, ncols, newtbl)
Vcchar_t*	oldtbl;	/* original table	*/
ssize_t		nrows;	/* #rows in otbl	*/
ssize_t		ncols;	/* #columns in otbl	*/
Vcchar_t*	newtbl;	/* new transposed table	*/
#endif
{
	ssize_t		r, nr, rr, c, nc, cc;
	Vcchar_t	*rdt, *cdt;

#define BLOCK	32	/* transposing small blocks to be cache-friendly */
	for(r = 0; r < nrows; r += BLOCK)
	{	nr = (nrows-r) < BLOCK ? (nrows-r) : BLOCK;
		for(c = 0; c < ncols; c += BLOCK)
		{	nc = (ncols-c) < BLOCK ? (ncols-c) : BLOCK;
			rdt = oldtbl + r*ncols + c;
			for(rr = 0; rr < nr; ++rr, rdt += ncols-nc)
			{	cdt = newtbl + c*nrows + r+rr;
				for(cc = 0; cc < nc; ++cc, cdt += nrows)
					*cdt = *rdt++;
			}
		}
	}
}

/* compute the # of columns from training data */
#if __STD_C
static ssize_t transtrain(const Void_t* train, size_t trsz)
#else
static ssize_t transtrain(train, trsz)
Void_t*	train;	/* training data */
size_t	trsz;
#endif
{
	ssize_t		ncols, sz;

	if(!train || trsz <= 0)
		return 1;

#define SAMPLE	(64*1024)
	for(sz = trsz < SAMPLE ? trsz : SAMPLE; sz <= trsz; sz *= 2)
		if((ncols = vcperiod(train, sz)) > 0)
			break;

	return ncols <= 0 ? 1 : ncols;
}

#if 0 /* combining transpose and run-length-encoding - an optimization  */
#if __STD_C
static ssize_t transrle(Vcodex_t* vc, const Void_t* data,
			size_t ncols, size_t nrows, Void_t** out)
#else
static ssize_t transrle(vc, data, ncols, nrows, out)
Vcodex_t*	vc;
Void_t*		data;
ssize_t		ncols, nrows;
Void_t**	out;
#endif
{
	reg Vcchar_t	*run, *chr, *dt, ch;
	Vcchar_t	*output, *enddt;
	reg ssize_t	c, r;
	ssize_t		hd, sz, size;
	Vcio_t		io;
	Transpose_t	*trans = vcgetmtdata(vc, Transpose_t*);

	size = nrows*ncols;
	hd = vcsizeu(size) + (trans->type == TR_PLAIN ? 0 : vcsizeu(ncols));
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), 2*(size + vcsizeu(size)), hd)) )
		RETURN(-1);

	chr = output + vcsizeu(size);
	run = chr + size + vcsizeu(size);

	ch = -1; r = 0;
	for(enddt = (Vcchar_t*)data+size, c = 0; c < ncols; c += 1)
	for(dt = (Vcchar_t*)data + c; dt < enddt; dt += ncols)
	{	if(*dt == ch)
			r += 1;
		else
		{	if(r > 0)
			{	if(r >= 3 || ch == RL_ESC)
				{	if(r < (1<<7) )
						*run++ = r;
					else if(r < (1<<14) )
					{	*run++ = (r>>7)|128;
						*run++ = r&127;
					}
					else
					{	vcioinit(&io, run, 2*sizeof(ssize_t));	
						vcioputu(&io, r);
						run = vcionext(&io);
					}

					*chr++ = RL_ESC;
					if(ch != RL_ESC || r > 1)
						*chr++ = ch;
				}
				else
				{	*chr++ = ch;
					if(r == 2)
						*chr++ = ch;
				}
			}

			ch = *dt; r = 1;
		}
	}

	if(r > 0)
	{	if(r >= 3 || ch == RL_ESC)
		{	vcioinit(&io, run, 2*sizeof(ssize_t));
			vcioputu(&io, r);
			run = vcionext(&io);

			*chr++ = RL_ESC;
			if(ch != RL_ESC || r > 1)
				*chr++ = ch;
		}
		else
		{	*chr++ = ch;
			if(r == 2)
				*chr++ = ch;
		}
	}

	c = chr - (output + vcsizeu(size)); chr = output + vcsizeu(size);
	r = run - (chr + size + vcsizeu(size)); run = chr + size + vcsizeu(size);

	if(vc->coder->coder) /* note that vc->coder is Vcrle */
	{	sz = 2*(size + vcsizeu(size));
		if((sz = _vcrle2coder(vc->coder,hd,chr,c,run,r,&output,sz)) < 0)
			RETURN(-1);
	}
	else
	{	vcioinit(&io, output, 2*(size+hd));
		vcioputu(&io, c);
		vcioputs(&io, chr, c);
		vcioputu(&io, r);
		vcioputs(&io, run, r);
		sz = vciosize(&io);
	}

	output -= hd;
	vcioinit(&io, output, hd);
	if(trans->type != TR_PLAIN)
		vcioputu(&io, ncols);
	vcioputu(&io, size);

	if(!(output = vcbuffer(vc, output, sz+hd, -1)) )
		RETURN(-1);
	if(out)
		*out = output;
	return sz+hd;
}
#endif

#if __STD_C
static ssize_t transpose(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t transpose(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output, *dt;
	ssize_t		nrows, ncols, sz, z;
	int		rsep; /* if >= 0, record separator for var-length table */
	Vcio_t		io;
	Transctxt_t	*ctxt;
	Transpose_t	*trans;

	vc->undone = 0;
	if((sz = (ssize_t)size) <= 0)
		return 0;

	if(!(trans = vcgetmtdata(vc, Transpose_t*)) )
		RETURN(-1);

	if(!(ctxt = vcgetcontext(vc, Transctxt_t*)) )
		RETURN(-1);

	if((rsep = ctxt->rsep) < 0 && trans->ctxt->rsep >= 0 )
		rsep = trans->ctxt->rsep;
	if(rsep >= 0)
		ncols = 0;
	else if((ncols = ctxt->ncols) <= 0 && trans->ctxt->ncols > 0)
		ncols = trans->ctxt->ncols;
	nrows = 0;

	if(rsep >= 0) /* var-length table */
	{	for(dt = (Vcchar_t*)data, z = sz-1; z >= 0; --z)
			if(dt[z] == rsep)
				break;
		vc->undone = sz - (z+1); /* exclude the dangling record */
		sz = z+1; /* data to be processed */
	}
	else
	{	if(ncols <= 0 && (ncols = transtrain(data,sz)) <= 0 )
			nrows = ncols = 0;
		else
		{	nrows = sz/ncols;
			ctxt->ncols = ncols;
		}
		vc->undone = sz - ncols*nrows;
		sz = nrows*ncols;
	}
	if(sz == 0)
		return 0;

	z = 2*sizeof(ssize_t); /* for coding ncols or rsep */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, z)) )
		RETURN(-1);

	if(rsep >= 0)
	{	if((nrows = transflip((Vcchar_t*)data, sz, rsep, output)) < 0)
			RETURN(-1);
	}
	else	transfixed((Vcchar_t*)data, nrows, ncols, output);

	dt = output;	
	if(vcrecode(vc, &output, &sz, z, 0) < 0 )
		RETURN(-1);
	if(dt != output)
		vcbuffer(vc, dt, -1, -1);

	if(trans->type != TR_PLAIN)
	{	z = vcsizeu(ncols);
		if(ncols <= 0)
			z += 1;
		output -= z; sz += z;
		vcioinit(&io, output, z);
		vcioputu(&io, ncols);
		if(ncols <= 0)
			vcioputc(&io, rsep);
	}

	if(out)
		*out = output;
	return sz;
}


#if __STD_C
static ssize_t untranspose(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t untranspose(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output, *dt;
	ssize_t		nrows, ncols, z;
	int		rsep; /* if >= 0, record separator for var-length table */
	Vcio_t		io;
	Transctxt_t	*ctxt;
	Transpose_t	*trans;

	vc->undone = 0;
	if(size == 0)
		return 0;

	if(!(trans = vcgetmtdata(vc, Transpose_t*)) )
		RETURN(-1);

	if(!(ctxt = vcgetcontext(vc, Transctxt_t*)) )
		RETURN(-1);

	vcioinit(&io, data, size);
	rsep = -1; ncols = nrows = 0;
	if(trans->type != TR_PLAIN)
	{	if((ncols = vciogetu(&io)) < 0)
			RETURN(-1);
		if(ncols == 0)
			rsep = vciogetc(&io);
	}
	else
	{	if((rsep = ctxt->rsep) < 0)
			rsep = trans->ctxt->rsep;
		if(rsep < 0)
			if((ncols = ctxt->ncols) <= 0)
				ncols = trans->ctxt->ncols;
	}

	if(rsep < 0 && ncols <= 0)
		RETURN(-1);

	/* data to be untransposed */
	dt = vcionext(&io);
	z = vciomore(&io);
	if(vcrecode(vc, &dt, &z, 0, 0) < 0)
		RETURN(-1);

	if(rsep < 0) /* fixed-length data */
	{	nrows = z/ncols;
		if(ncols*nrows != z)
			RETURN(-1);
	}

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), z, 0)) )
		RETURN(-1);

	if(rsep < 0)
		transfixed(dt, ncols, z/ncols, output);
	else if(unflip(dt, z, rsep, output) < 0)
		RETURN(-1);

	if(out)
		*out = output;
	return z;
}

#if __STD_C
static int transevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int transevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Transpose_t	*trans;
	Transctxt_t	*ctxt;
	char		*data, val[1024];
	Vcmtarg_t	*arg;

	if(type == VC_OPENING)
	{	if(!(trans = (Transpose_t*)calloc(1,sizeof(Transpose_t))) )
			RETURN(-1);
		if(!(trans->ctxt = (Transctxt_t*)vcinitcontext(vc, NIL(Vccontext_t*))) )
		{	free(trans);
			RETURN(-1);
		}
		vcsetmtdata(vc, trans);
		goto vc_setarg;
	}
	else if(type == VC_CLOSING)
	{	if((trans = vcgetmtdata(vc,Transpose_t*)) )
			free(trans);

		vcsetmtdata(vc, NIL(Transpose_t*));
		return 0;
	}
	else if(type == VC_SETMTARG)
	{ vc_setarg:	
		if(!(ctxt = vcgetcontext(vc, Transctxt_t*)) )
			RETURN(-1);
		for(data = (char*)params; data && *data; )
		{	data = vcgetmtarg(data, val, sizeof(val), _Transargs, &arg);
			switch(TYPECAST(int,arg->data) )
			{ case TR_SEPARATOR: /* setting the record separator */
				ctxt->rsep = val[0];
				ctxt->ncols = 0;
				break;
			  case TR_COLUMNS: /* setting number of columns */
				ctxt->ncols = (ssize_t) vcatoi(val);
				ctxt->rsep = -1;
				break;
			  case TR_PLAIN: /* setting transpose.0 */
			  default :
				if(type == VC_OPENING)
					trans->type = TYPECAST(int,arg->data);
				break;
			}
		}

		return 0;
	}
	else if(type == VC_INITCONTEXT)
	{	if(!params)
			return 0;
		if(!(ctxt = (Transctxt_t*)calloc(1,sizeof(Transctxt_t))) )
			RETURN(-1);
		ctxt->ncols = 0;
		ctxt->rsep = -1;
		*((Transctxt_t**)params) = ctxt;
		return 1;
	}
	else if(type == VC_FREECONTEXT)
	{	if((ctxt = (Transctxt_t*)params) )
			free(ctxt);
		return 0;
	}
	else	return 0;
}

Vcmethod_t _Vctranspose =
{	transpose,
	untranspose,
	transevent,
	"transpose", "Transposing a table.",
	"[-version?transpose (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	_Transargs,
	256*1024,
	0
};

VCLIB(Vctranspose)
