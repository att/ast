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
#include	"vcxhdr.h"

/* Checksumming a block of data.
**
** Written by Kiem-Phong Vo
*/

#define SUM_AES		1
#define SUM_MD5		2
#define SUM_CRC		3

typedef struct _sum_s
{	int	type;	/* checksum method type	*/
	ssize_t	head;	/* #control bytes	*/
	Vcx_t	digest;	/* digest structure	*/
	ssize_t	dgsize;	/* digest size		*/
} Sum_t;

static Vcmtarg_t _Sumargs[] =
{
	{ "aes", "AES digest, aes=size to set size", (Void_t*) SUM_AES },
	{ "md5", "MD5 digest, md5=size to set size", (Void_t*) SUM_MD5 },
	{ "crc", "CRC digest, crc=size to set size", (Void_t*) SUM_CRC },
	{ 0, "Default CRC digest", (Void_t*)SUM_CRC }
};

static ssize_t sum(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
{
	Vcchar_t	*dt, *dig;
	ssize_t		sz;
	Sum_t		*sum;
	Vcchar_t	type = 0; /* control byte */

	if(!vc || !(sum = vcgetmtdata(vc, Sum_t*)) )
		return -1;
	vc->undone = 0;

	if(!(dt = (Vcchar_t*)data) || (sz = (ssize_t)size) <= 0)
		return 0;

	/* process data with secondary coder, then add checksum */
	if(vcrecode(vc, &dt, &sz, sum->head+sum->dgsize, 1) < 0)
		return -1;

	if(vc->undone == size) /* nothing was processed */
	{	type = VC_RAW; /* just output raw data */
		sz = size;
		vc->undone = 0; /* as such, everything is done! */
	}
	else /* a subset was processed */
	{	type = 0;
		size -= vc->undone;
	}

	/* compute the digest of raw data */
	if(vcxencode(&sum->digest, data, size, &dig) < sum->dgsize)
		return -1;

	if(type == VC_RAW) /* nothing was processed */
	{	if(!(dt = vcbuffer(vc, NIL(Vcchar_t*), size, sum->head + sum->dgsize)) )
			return -1;
		memcpy(dt, data, size);
	}

	dt -= sum->head+sum->dgsize; sz += sum->head+sum->dgsize;
	dt[0] = type;
	if(sum->head > 1)
		memset(dt+1, 0, sum->head-1);

	memcpy(dt+sum->head, dig, sum->dgsize);

	if(out)
		*out = dt;
	return sz;
}

static ssize_t unsum(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
{
	Vcchar_t	*dt, *dig, *sig;
	ssize_t		sz;
	Vcchar_t	type;
	Sum_t		*sum;

	if(!vc || !(sum = vcgetmtdata(vc, Sum_t*)) )
		return -1;
	vc->undone = 0;

	if(!(dt = (Vcchar_t*)data) || (sz = (ssize_t)size) <= 0)
		return 0;

	/* status if data was raw or processed by a 2ndary coder */
	if(sum->head <= 0)
		type = 0; /* was always processed */
	else
	{	type = dt[0];
		dt += sum->head;
		sz -= sum->head;
	}

	/* signature */
	sig = dt; dt += sum->dgsize; sz -= sum->dgsize;

	if(type != VC_RAW) /* undo secondary encoding */
		if(vcrecode(vc, &dt, &sz, 0, 0) < 0)
			return -1;

	/* compute the digest of raw data */
	if(vcxencode(&sum->digest, dt, sz, &dig) < sum->dgsize)
		return -1;

	/* check the checksum to see if ok */
	if(memcmp(dig, sig, sum->dgsize) != 0)
		return -1;

	if(out)
		*out = dt;
	return sz;
}

/* construct header data or decoding it */
static int sumheader(Vcodex_t* vc, Vcmtcode_t* mtcd)
{
	Sum_t		*sum;
	Vcmtarg_t	*arg;
	Vcchar_t	*dt, buf[64];
	ssize_t		dgsz, head, sz;
	Vcio_t		io;

	if(vc) /* extracting code for persistent data */
	{	if(!(sum = vcgetmtdata(vc, Sum_t*)) )
			return -1;

		/* find the digest method */
		for(arg = _Sumargs; arg->name; ++arg)
			if(arg->data == TYPECAST(Void_t*,sum->type))
				break;
		if(!arg->name)
			return -1;

		if(!vcstrcode(arg->name, (char*)buf, sizeof(buf)) )
			return -1;

		sz = vcsizeu(sum->dgsize) + strlen((char*)buf) + 1;
		if(!(dt = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
			return -1;
		vcioinit(&io, dt, sz);
		vcioputu(&io, sum->dgsize);
		vcioputs(&io, (char*)buf, strlen((char*)buf));

		/* The below code is partially for compatibility with an older
		** version where the byte after the code name was always zero.
		** In that version, coded data did not have any header bytes.
		** In the current version, coded data has a control byte
		** to tell if it should be passed to a secondary coder.
		** The byte after the code name is now generalized to mean
		** the number of control bytes, sum->head.
		*/
		vcioputc(&io, sum->head);

		mtcd->data = dt;
		mtcd->size = sz;
	}
	else /* reconstructing a handle from header code */
	{	vcioinit(&io, mtcd->data, mtcd->size);
		if((dgsz = vciogetu(&io)) <= 0 ) /* digest size */
			return -1;

		dt = vcionext(&io); /* the name of the digest method */
		sz = vciomore(&io) - 1; /* length of that name */
		vcioskip(&io, sz);
		head = vciogetc(&io); /* number of control bytes */

		for(arg = _Sumargs; arg->name; ++arg)
		{	if(!vcstrcode(arg->name, (char*)buf, sizeof(buf)) )
				return -1;
			if(strncmp((char*)buf, (char*)dt, sz) == 0 && buf[sz] == 0)
				break;
		}
		if(!arg->name)
			return -1;

		/* reconstruct handle */
		vcioinit(&io, buf, sizeof(buf));
		vcioputs(&io, arg->name, strlen(arg->name));
		vcioputc(&io, '=');
		vcitoa((Vcint_t)dgsz, (char*)vcionext(&io), vciomore(&io));
		if(!(mtcd->coder = vcopen(0, Vcsum, (Void_t*)buf, mtcd->coder, VC_DECODE)) )
			return -1;

		if(!(sum = vcgetmtdata(mtcd->coder, Sum_t*)) )
			return -1;
		sum->head = head;
	}

	return 0;
}

static int sumevent(Vcodex_t* vc, int type, Void_t* param)
{
	char		*data, val[1024];
	Vcmtarg_t	*arg;
	Vcxmethod_t	*meth;
	ssize_t		dgsize;
	Sum_t		*sum = NIL(Sum_t*);

	if(type == VC_OPENING)
	{	if(!(sum = (Sum_t*)calloc(1, sizeof(Sum_t))) )
			return -1;

		sum->type = SUM_CRC; meth = Vcxcrcsum;
		sum->head = 1; /* 1 control byte for now */
		sum->dgsize = 0;
		for(data = (char*)param; data; )
		{	val[0] = 0;
			data = vcgetmtarg(data, val, sizeof(val), _Sumargs, &arg);
			switch(TYPECAST(int,arg->data))
			{ case SUM_AES:
				sum->type = SUM_AES; meth = Vcxaessum;
				goto do_size;
			  case SUM_MD5:
				sum->type = SUM_MD5; meth = Vcxmd5sum;
				goto do_size;
			  case SUM_CRC:
				sum->type = SUM_CRC; meth = Vcxcrcsum;
			  do_size:
				if(val[0] >= '0' && val[0] <= '9') 
					sum->dgsize = vcatoi(val);
				else	sum->dgsize = 0;
				break;
			}
		}
		if((dgsize = vcxinit(&sum->digest, meth, 0, 0)) <= 0)
		{	free(sum);
			return -1;
		}

		if(sum->dgsize <= 0 || sum->dgsize > dgsize)
			sum->dgsize = dgsize;

		vcsetmtdata(vc, sum);
		return 0;
	}
	else if(type == VC_EXTRACT || type == VC_RESTORE)
		return sumheader(vc, (Vcmtcode_t*)param) < 0 ? -1 : 1;
	else if(type == VC_CLOSING)
	{	if((sum = vcgetmtdata(vc, Sum_t*)) )
		{	vcxstop(&sum->digest);
			free(sum);
		}
		return 0;
	}
	else	return 0;
}

Vcmethod_t	_Vcsum =
{	sum,
	unsum,
	sumevent,
	"sum", "Checksumming data.",
	"[-?\n@(#)$Id: vcodex-sum (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	_Sumargs,
	1024*1024,
	0
};

VCLIB(Vcsum)
