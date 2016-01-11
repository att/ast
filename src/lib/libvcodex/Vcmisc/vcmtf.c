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

/*	Move-to-front transformers.
**
**	Written by Kiem-Phong Vo
*/

typedef ssize_t	(*Mtf_f)_ARG_((Vcchar_t*, Vcchar_t*, Vcchar_t*, int));
static ssize_t	mtfp _ARG_((Vcchar_t*, Vcchar_t*, Vcchar_t*, int));
static ssize_t	mtf0 _ARG_((Vcchar_t*, Vcchar_t*, Vcchar_t*, int));

#define MTFC(c,p,m,n) /* know byte, need to compute position */ \
	{ m = mtf[p = 0]; \
	  while(m != c) \
	  	{ n = mtf[p += 1]; mtf[p] = m; m = n; } \
	  mtf[0] = c; \
	}

#define MTFP(c,p,m,n) /* know position, need to compute byte */ \
	{ c = mtf[m = 0]; \
	  while(m != p) \
	  	{ n = mtf[m += 1]; mtf[m] = c; c = n; } \
	  mtf[0] = c; \
	}

/* arguments to select move-to-front coder */
static Vcmtarg_t _Mtfargs[] =
{	{ "0", "Pure move-to-front strategy.", (Void_t*)mtf0 },
	{  0 , "Move-to-front with prediction.", (Void_t*)mtfp }
};

#if __STD_C
static ssize_t mtfp(Vcchar_t* dt, Vcchar_t* enddt, Vcchar_t* output, int encoding)
#else
static ssize_t mtfp(dt, enddt, output, encoding)
Vcchar_t*	dt;		/* data to be un/mtf-ed	*/
Vcchar_t*	enddt;
Vcchar_t*	output;		/* output array		*/
int		encoding;	/* !=0 if encoding	*/
#endif
{
	reg int		c, p, m, n, predc;
	reg Vcchar_t	*o;
	Vcchar_t	mtf[256], succ[256];
	size_t		wght[256];

	predc = 0;
	for(p = 0; p < 256; ++p)
		mtf[p] = succ[p] = p;
	memset(wght,0,sizeof(wght));

/* weight increases slightly but decreases quickly */
#define SUCC(c,p,m,n) \
	{ if(succ[predc] == c)  wght[predc] += 4; \
	  else if((wght[predc] >>= 1) <= 0) \
		{ succ[predc] = c; wght[predc] = 1; } \
	  predc = c; \
	  if(wght[c] > 1 && (c = succ[c]) != mtf[0] ) \
		MTFC(c,p,m,n); /* predicting succ[c] is next */ \
	}

	if(encoding)
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	c = *dt; MTFC(c,p,m,n); *o = p;
			SUCC(c,p,m,n);
		}
	}
	else
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	p = *dt; MTFP(c,p,m,n); *o = c;
			SUCC(c,p,m,n);
		}
	}

	return o-output;
}


/* move to zeroth location */
#if __STD_C
static ssize_t mtf0(Vcchar_t* dt, Vcchar_t* enddt, Vcchar_t* output, int encoding)
#else
static ssize_t mtf0(dt, enddt, output, encoding)
Vcchar_t*	dt;	/* data to be un/mtf-ed	*/
Vcchar_t*	enddt;
Vcchar_t*	output;	/* output array		*/
int		encoding; /* !=0 if encoding	*/
#endif
{
	reg int		c, p, m, n;
	reg Vcchar_t	*o;
	Vcchar_t	mtf[256];

	for(p = 0; p < 256; ++p)
		mtf[p] = p;
	if(encoding)
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	c = *dt; MTFC(c,p,m,n); *o = p;
		}
	}
	else
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	p = *dt; MTFP(c,p,m,n); *o = c;
		}
	}

	return o-output;
}

#if __STD_C
static ssize_t vcmtf(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcmtf(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output, *dt;
	ssize_t		sz;
	Mtf_f		mtff = vcgetmtdata(vc, Mtf_f);

	if((sz = size) == 0)
		return 0;

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		RETURN(-1);

	if((*mtff)((Vcchar_t*)data, ((Vcchar_t*)data)+sz, output, 1) != sz)
		RETURN(-1);

	dt = output;
	if(vcrecode(vc, &output, &sz, 0, 0) < 0)
		RETURN(-1);
	if(dt != output)
		vcbuffer(vc, dt, -1, -1);

	if(out)
		*out = output;
	return sz;
}

#if __STD_C
static ssize_t vcunmtf(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcunmtf(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*dt, *output;
	ssize_t		sz;
	Mtf_f		mtff = vcgetmtdata(vc, Mtf_f);

	if(size == 0)
		return 0;

	dt = (Vcchar_t*)data; sz = size;
	if(vcrecode(vc, &dt, &sz, 0, 0) < 0 )
		RETURN(-1);

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		RETURN(-1);

	if((*mtff)(dt, dt+sz, output, 0) != sz)
		RETURN(-1);

	if(dt != (Vcchar_t*)data)
		vcbuffer(vc, dt, -1, -1);

	if(out)
		*out = output;
	return sz;
}

#if __STD_C
static ssize_t mtfextract(Vcodex_t* vc, Vcchar_t** datap)
#else
static ssize_t mtfextract(vc, datap)
Vcodex_t*	vc;
Vcchar_t**	datap;	/* basis string for persistence	*/
#endif
{
	Vcmtarg_t	*arg;
	char		*ident;
	ssize_t		n;
	Void_t		*mtdt = vcgetmtdata(vc, Void_t*);

	for(arg = _Mtfargs;; ++arg)
		if(!arg->name || arg->data == mtdt)
			break;
	if(!arg->name)
		return 0;

	n = strlen(arg->name);
	if(!(ident = (char*)vcbuffer(vc, NIL(Vcchar_t*), sizeof(int)*n+1, 0)) )
		RETURN(-1);
	if(!(ident = vcstrcode(arg->name, ident, sizeof(int)*n+1)) )
		RETURN(-1);
	if(datap)
		*datap = (Void_t*)ident;
	return n;
}

#if __STD_C
static int mtfrestore(Vcmtcode_t* mtcd)
#else
static int mtfrestore(mtcd)
Vcmtcode_t*	mtcd;
#endif
{
	Vcmtarg_t	*arg;
	char		*ident, buf[1024];

	for(arg = _Mtfargs; arg->name; ++arg)
	{	if(!(ident = vcstrcode(arg->name, buf, sizeof(buf))) )
			return -1;
		if(mtcd->size == strlen(ident) &&
		   strncmp(ident, (char*)mtcd->data, mtcd->size) == 0)
			break;
	}
	mtcd->coder = vcopen(0, Vcmtf, (Void_t*)arg->name, mtcd->coder, VC_DECODE);
	return mtcd->coder ? 1 : -1;
}

#if __STD_C
static int mtfevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int mtfevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Vcmtarg_t	*arg;
	Vcmtcode_t	*mtcd;
	char		*data;

	if(type == VC_OPENING )
	{	for(arg = NIL(Vcmtarg_t*), data = (char*)params; data && *data; )
		{	data = vcgetmtarg(data, 0, 0, _Mtfargs, &arg);
			if(arg && arg->name)
				break;
		}
		if(!arg) /* get the default argument */
			for(arg = _Mtfargs;; ++arg)
				if(!arg->name)
					break;
		vcsetmtdata(vc, arg->data);
		return 0;
	}
	else if(type == VC_EXTRACT)
	{	if(!(mtcd = (Vcmtcode_t*)params) )
			return -1;
		if((mtcd->size = mtfextract(vc, &mtcd->data)) < 0 )
			return -1;
		return 1;
	}
	else if(type == VC_RESTORE)
	{	if(!(mtcd = (Vcmtcode_t*)params) )
			return -1;
		return mtfrestore(mtcd) < 0 ? -1 : 1;
	}

	return 0;
}

Vcmethod_t _Vcmtf =
{	vcmtf,
	vcunmtf,
	mtfevent,
	"mtf", "Move-to-front transform.",
	"[-version?mtf (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	_Mtfargs,
	1024*1024,
	0
};

VCLIB(Vcmtf)
