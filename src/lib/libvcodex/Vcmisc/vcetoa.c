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

/*	Mapping between AMA blocks and ASCII text lines.
**	This assumes that AMA records are text-only, ie, no packed decimals, etc.
**
**	Written by Kiem-Phong Vo
*/

/* the below macros define the coding of record lengths */
#define MAXSIZE		(1 << 16) /* max allowed record size	*/
#define GETSIZE(dt)	(((dt)[0]<<8)+(dt)[1]) /* get AMA size	*/
#define PUTSIZE(dt,v)	(((dt)[0] = (((v)>>8)&0377)), ((dt)[1] = ((v)&0377)) )

/* EBCDIC to ASCII mappings as defined in the Vcmap transform */
extern Vcmtarg_t _Mapargs[];

#if __STD_C
static ssize_t etoa(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t etoa(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		sz, rz, mz;
	Vcchar_t	*output, *idt, *odt, *mdt;
	Vcodex_t	*map;

	if(!vc || !(map = vcgetmtdata(vc, Vcodex_t*)) )
		return -1;

	if((sz = (ssize_t)size) == 0)
		return 0;

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;

	for(odt = output, idt = (Vcchar_t*)data; sz >= 4; idt += rz, sz -= rz)
	{	if((rz = GETSIZE(idt)) > sz ) /* partial record */
			break;
		if(idt[2] != 0 || idt[3] != 0) /* these bytes are always zero */
			return -1;

		/* map data to ascii, then append a new-line */
		if((mz = vcapply(map, idt+4, rz-4, &mdt)) < 0 )
			return -1;
		memcpy(odt, mdt, mz); odt[mz] = '\n'; odt += mz+1;
		vcbuffer(map, NIL(Vcchar_t*), 0, -1);
	}

	vc->undone = (ssize_t)size - sz;

	if(out)
		*out = output;
	return (odt-output);
}

#if __STD_C
static ssize_t unetoa(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unetoa(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		rz, oz, sz;
	Vcchar_t	*output, *odt, *endo, *idt, *endi, *dt, *rdt;
	Vcodex_t	*map;

	if(!vc || !(map = vcgetmtdata(vc, Vcodex_t*)) )
		return -1;

	/* skip the last partial record */
	for(idt = (Vcchar_t*)data + size-1; idt > (Vcchar_t*)data; idt -= 1)
		if(*idt == '\n')
			break;

	endi = idt+1; /* extent of data that will be processed */
	vc->undone = size - (endi - (Vcchar_t*)data);

	endo = odt = output = NIL(Vcchar_t*);
	for(idt = (Vcchar_t*)data; idt < endi; idt = dt+1)
	{	for(dt = idt; dt < endi; ++dt)
			if(*dt == '\n')
				break;

		/* reverse the mapping from ascii */
		if((rz = vcapply(map, idt, (dt-idt)+1, &rdt)) < 0 )
			return -1;
		if((rz += 3) >= MAXSIZE ) /* ama record size */
			return -1;

		if(rz > (endo-odt) ) /* make buffer for data */
		{	oz = odt-output; sz = oz + 128*rz;
			if(!(output = vcbuffer(vc, output, sz, 0)) )
				return -1;
			odt = output + oz;
			endo = output + sz;
		}

		/* recreate record in ama format */
		PUTSIZE(odt, rz); odt[2] = odt[3] = 0;
		memcpy(odt+4, rdt, rz-4); odt += rz;

		vcbuffer(map, NIL(Vcchar_t*), 0, -1);
	}

	if(out)
		*out = output;
	return (odt-output);
}

#if __STD_C
static int etoaevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int etoaevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Vcodex_t	*map;
	Vcmtarg_t	*arg;
	char		*data, *e2a;

	if(type == VC_OPENING)
	{	e2a = "e2a"; /* get the translation type */
		for(data = (char*)params; data; )
		{	arg = NIL(Vcmtarg_t*); 
			data = vcgetmtarg(data, 0, 0, _Mapargs, &arg);
			if(arg)
				e2a = arg->name;
		}

		if(!(map = vcopen(0, Vcmap, e2a, 0, (vc->flags&(VC_ENCODE|VC_DECODE)))) )
			return -1;

		vcsetmtdata(vc, map);
	}
	else if(type == VC_CLOSING)
	{	if((map = vcgetmtdata(vc, Vcodex_t*)) )
			(void)vcclose(map);
		vcsetmtdata(vc, NIL(Void_t*));
	}

	return 0;
}

Vcmethod_t _Vcetoa =
{	etoa,
	unetoa,
	etoaevent,
	"etoa", "Mapping EBCDIC blocks to/from ASCII text lines.",
	"[-?\n@(#)$Id: vcodex-etoa (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	_Mapargs,
	8*1024*1024,
	0
};

VCLIB(Vcetoa)
