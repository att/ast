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

/*	Stripping head and tail data.
**
**	Written by Kiem-Phong Vo
*/

#define	ST_LINE	01	/* dealing with lines of text	*/
#define	ST_HEAD	02	/* specifying header to strip	*/
#define	ST_TAIL	04	/* specifying tail to strip	*/

typedef struct _strip_s
{	int	type;
	ssize_t	head;	/* amount to strip from head	*/
	ssize_t	tail;	/* amount to strip from tail	*/
} Strip_t;

static Vcmtarg_t	_Stripargs[] =
{	{ "nl", "Data is line-oriented", (Void_t*)ST_LINE },
	{ "head", "Header to strip is 'head=amount'", (Void_t*)ST_HEAD },
	{ "tail", "Tail to strip is 'tail=amount'", (Void_t*)ST_TAIL },
	{ 0, "Not stripping anything, only processing 'nl' if specified", (Void_t*)0 }
};

#if __STD_C
static ssize_t strip(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t strip(vc, data, size, out)
Vcodex_t*	vc;
const Void_t*	data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		z, dsz, hsz, tsz;
	Vcchar_t	*dt, *edt, *hdt, *tdt, *output;
	Strip_t		*st;
	Vcio_t		io;

	if(!vc || !(st = vcgetmtdata(vc,Strip_t*)) )
		return -1;

	/* set valid interval of data to be processed */
	edt = (dt = (Vcchar_t*)data) + (dsz = (ssize_t)size);
	if(!(st->type&ST_LINE) )
		vc->undone = 0;
	else /* dealing with lines of text */
	{	for(; edt > dt; --edt)
			if(edt[-1] == '\n')
				break;
		vc->undone = size - (edt-dt); /* size of partial line */
		dsz = edt - dt; /* size of data to be processed */
	}

	/* find an appropriate head amount to strip */
	hdt = dt; hsz = 0;
	if(st->type&ST_LINE) 
	{	for(z = st->head; z > 0; --z)
		{	for(; dt < edt; ++dt)
				if(*dt == '\n')
					break;	
			if((dt += 1) >= edt)
				break;
		}
		hsz = dt - hdt;
	}
	else
	{	if((hsz = st->head) > dsz)
			hsz = dsz;
		dt += hsz;
	}
	dsz -= hsz;

	/* find an appropriate tail amount to strip */
	tdt = edt; tsz = 0;
	if(st->type&ST_LINE)
	{	for(z = st->tail; z > 0; --z)
		{	if((tdt -= 1) <= dt)
				break;
			for(; tdt > dt; --tdt)
				if(tdt[-1] == '\n')
					break;	
		}
		tsz = edt - tdt;
	}
	else
	{	if((tsz = st->tail) > dsz)
			tsz = dsz;
		tdt -= tsz;
	}
	dsz -= tsz;

	/* recode the middle part of the data */
	if(dsz > 0 && vcrecode(vc, &dt, &dsz, 0, 0) < 0 )
		RETURN(-1);

	/* allocate output buffer */
	z = vcsizeu(hsz) + hsz + vcsizeu(tsz) + tsz + vcsizeu(dsz) + dsz;
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), z, 0)) )
		RETURN(-1);

	/* write out the transformed data */
	vcioinit(&io, output, z);
	vcioputu(&io, hsz);
	vcioputs(&io, hdt, hsz);
	vcioputu(&io, tsz);
	vcioputs(&io, tdt, tsz);
	vcioputu(&io, dsz);
	vcioputs(&io,  dt, dsz);

	if(out)
		*out = (Void_t*)output;
	return z;
}


#if __STD_C
static ssize_t unstrip(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unstrip(vc, data, size, out)
Vcodex_t*	vc;
const Void_t*	data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*hdt, *tdt, *dt, *output;
	ssize_t		hsz, tsz, dsz, z;
	Vcio_t		io;

	if(!vc)
		return -1;
	vc->undone = 0;

	vcioinit(&io, (Vcchar_t*)data, size);

	if((hsz = vciogetu(&io)) < 0 || hsz > vciomore(&io))
		RETURN(-1);
	hdt = vcionext(&io); vcioskip(&io, hsz);

	if((tsz = vciogetu(&io)) < 0 || tsz > vciomore(&io))
		RETURN(-1);
	tdt = vcionext(&io); vcioskip(&io, tsz);

	if((dsz = vciogetu(&io)) < 0 || dsz > vciomore(&io))
		RETURN(-1);
	dt = vcionext(&io); vcioskip(&io, dsz);
	if(dsz > 0 && vcrecode(vc, &dt, &dsz, 0, 0) < 0 )
		RETURN(-1);

	z = hsz + dsz + tsz;
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), z, 0)) )
		RETURN(-1);

	memcpy(output, hdt, hsz);
	memcpy(output+hsz, dt, dsz);
	memcpy(output+hsz+dsz, tdt, tsz);

	if(out)
		*out = output;

	return z;
}

#if __STD_C
static int stripevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int stripevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Strip_t		*st;
	Vcmtarg_t	*arg;
	char		*data, val[1024];

	if(type == VC_OPENING)
	{	if(!(st = calloc(1, sizeof(Strip_t))) )
			return -1;

		for(data = (char*)params; data; )
		{	data = vcgetmtarg(data, val, sizeof(val), _Stripargs, &arg);
			switch(TYPECAST(int,arg->data) )
			{
			case ST_LINE:
				st->type = ST_LINE;
				break;
			case ST_HEAD:
				st->head = (ssize_t)vcatoi(val);
				break;
			case ST_TAIL:
				st->tail = (ssize_t)vcatoi(val);
				break;
			}
		}

		vcsetmtdata(vc, st);
	}
	else if(type == VC_CLOSING)
	{	if((st = vcgetmtdata(vc, Strip_t*)) )
			free(st);

		vcsetmtdata(vc, NIL(Void_t*));
	}

	return 0;
}

Vcmethod_t _Vcstrip =
{	strip,
	unstrip,
	stripevent,
	"strip", "Strip head and tail data.",
	"[-version?strip (AT&T Research) 2009-03-12]" USAGE_LICENSE,
	_Stripargs,
	1024*1024,
	0
};

VCLIB(Vcstrip)
