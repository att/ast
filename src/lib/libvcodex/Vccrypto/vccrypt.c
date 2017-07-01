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

/* Transform to encrypt data based on some given encryption method.
**
** Written by Kiem-Phong Vo.
*/

#ifdef DEBUG
ssize_t prhex(Vcchar_t* dt, ssize_t sz)
{
	Vcchar_t	hex[1024];
	if((sz = vchexcode(dt, sz, hex, sizeof(hex), 1)) > 0)
	{	write(2, hex, sz);
		write(2, "\n", 1);
		return 0;
	}
	else	return sz;
}
#endif

Vcxpasskey_f	Vcxpasskeyf;	/* a global passkey function	*/

#define CR_INIT		001	/* not yet initialized 		*/

#define	CR_AES128	128
#define	CR_AES192	192
#define	CR_AES256	256

typedef struct _crypt_s
{	Vcxmethod_t*	meth;	/* encryption method		*/
	Vcx_t		xx;	/* encryption handle		*/
	ssize_t		head;	/* #control bytes in data	*/
	int		type;
} Crypt_t;

static Vcmtarg_t _Cryptargs[] =
{	{ "aes128", "128-bit Advanced Encryption Standard", (Void_t*)CR_AES128 },
	{ "aes192", "192-bit Advanced Encryption Standard", (Void_t*)CR_AES192 },
	{ "aes256", "256-bit Advanced Encryption Standard", (Void_t*)CR_AES256 },
	{ 0, "Default 128-bit Advanced Encryption Standard", (Void_t*)CR_AES128 }
};

static int crinit(Vcodex_t* vc, Crypt_t* cr)
{
	ssize_t		keyz;
	unsigned int	type;
	Vcchar_t	*key, buf[1024];

	if(!(cr->type&CR_INIT) ) /* only resetting encryption state */
		return 0;

	type = vc->flags & (VC_ENCODE|VC_DECODE);

	if(vc->disc && vc->disc->eventf) /* invoke discipline for passkey */
	{	if((*vc->disc->eventf)(vc, VC_DATA, (Void_t*)1, vc->disc) < 0)
			return -1;
	}

	if(vc->disc && vc->disc->data)
	{	key = (Vcchar_t*) vc->disc->data;
		keyz = (ssize_t) vc->disc->size;
	}
	else /* invoke Vcxpasskeyf to ask for a key */
	{	if(Vcxpasskeyf &&
		   (keyz = (*Vcxpasskeyf)((char*)buf, sizeof(buf), type)) > 0)
			key = buf;
		else	key = NIL(Vcchar_t*);
	}

	if(!key) /* no passkey for encryption */
		return -1;

	if(vcxinit(&cr->xx, cr->meth, key, keyz) < 0)
		return -1;

	if(vc->disc && vc->disc->eventf)
	{	if((*vc->disc->eventf)(vc, VC_DATA, (Void_t*)2, vc->disc) < 0)
		{	vcxstop(&cr->xx);
			return -1;
		}
	}

	if(key == buf) /* clear the passkey from memory */
		memset(buf, 0, sizeof(buf));

	cr->type &= ~CR_INIT; /* successful initialization of encryption handle */
	return 0;
}

static ssize_t vcencrypt(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
{
	Vcchar_t	*dt;
	ssize_t		sz, k;
	Crypt_t		*cr;
	Vcchar_t	type = 0; /* control byte */

	if(!vc || !(cr = vcgetmtdata(vc, Crypt_t*)) )
		return -1;
	vc->undone = 0;

	if(!(dt = (Vcchar_t*)data) || (sz = (ssize_t)size) <= 0)
		return 0;

	/* secondary processing before encryption */
	if(vcrecode(vc, &dt, &sz, 0, 1) < 0)
		return -1;

	if(vc->undone == size)
	{	type = VC_RAW; /* encrypting the given raw input data */
		dt = (Vcchar_t*)data;
		sz = size;
		vc->undone = 0;
	}

	if(crinit(vc, cr) < 0)
		return -1;
	if((sz = vcxencode(&cr->xx, dt, sz, &dt)) <= 0)
		return -1;

	if(out)
	{	Vcchar_t	*cdt;
		if(!(cdt = vcbuffer(vc, NIL(Vcchar_t*), sz + cr->head, 0)) )
			return -1;

		*cdt = type; /* for now, header is just this one byte */
		if(cr->head > 1)
			memset(cdt+1, 0, cr->head-1);

		memcpy(cdt + cr->head, dt, sz);
		*out = cdt;
	}

	return sz+1;
}

static ssize_t vcdecrypt(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
{
	Vcchar_t	*dt;
	ssize_t		sz;
	Vcchar_t	type;
	Crypt_t		*cr;

	if(!vc || !(cr = vcgetmtdata(vc, Crypt_t*)) )
		return -1;
	vc->undone = 0;

	if(!(dt = (Vcchar_t*)data) || (sz = (ssize_t)size) <= 0)
		return 0;

	/* status of raw or processed by a secondary coder */
	if(cr->head <= 0)
		type = 0; /* always pass to a 2ndary coder */
	else
	{	type = dt[0];
		dt += cr->head;
		sz -= cr->head;
	}

	if(crinit(vc, cr) < 0)
		return -1;
	if((sz = vcxdecode(&cr->xx, dt, sz, &dt)) <= 0)
		return -1;

	/* undo secondary processing */
	if(type != VC_RAW)
		if(vcrecode(vc, &dt, &sz, 0, 0) < 0)
			return -1;
	if(out)
		*out = dt;
	return sz;
}

/* construct header data or decoding it */
static int cryptheader(Vcodex_t* vc, Vcmtcode_t* mtcd)
{
	Vcmtarg_t	*arg;
	Vcchar_t	*dt, buf[128];
	ssize_t		sz;
	Crypt_t		*cr;

	if(vc) /* extracting code for persistent data */
	{	if(!(cr = vcgetmtdata(vc, Crypt_t*)) )
			return -1;

		/* find the digest method */
		for(arg = _Cryptargs; arg->name; ++arg)
			if(arg->data == TYPECAST(Void_t*,(cr->type&~CR_INIT)) )
				break;
		if(!arg->name)
			return -1;

		/* code the method name in ASCII-portable code */
		if(!vcstrcode(arg->name, (char*)buf, sizeof(buf)) )
			return -1;
		sz = strlen((char*)buf) + 1;
		if(!(dt = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
			return -1;
		memcpy(dt, buf, sz - 1);

		/* The below code is partially for compatibility with an older
		** version where the byte after the code name was always zero.
		** In that version, coded data did not have any header bytes.
		** In the current version, coded data has a control byte
		** to tell if it should be passed to a secondary coder.
		** The byte after the code name is now generalized to mean
		** the number of control bytes, cr->head.
		*/
		dt[sz-1] = cr->head;

		mtcd->data = dt;
		mtcd->size = sz;
	}
	else /* reconstructing a handle from header code */
	{	dt = (Vcchar_t*)mtcd->data;
		if((sz = (ssize_t)mtcd->size - 1) <= 0)
			return -1;

		for(arg = _Cryptargs; arg->name; ++arg)
		{	if(!vcstrcode(arg->name, (char*)buf, sizeof(buf)) )
				return -1;
			if(strncmp((char*)buf, (char*)dt, sz) == 0 && buf[sz] == 0)
				break;
		}
		if(!arg->name)
			return -1;

		if(!(mtcd->coder = vcopen(0,Vccrypt,(Void_t*)arg->name,mtcd->coder,VC_DECODE)) )
			return -1;

		if(!(cr = vcgetmtdata(mtcd->coder, Crypt_t*)) )
			return -1;
		cr->head = dt[sz]; /* see above long comment */
	}

	return 0;
}

static int cryptevent(Vcodex_t* vc, int type, Void_t* param)
{
	char		*data, val[1024];
	Vcmtarg_t	*arg;
	Crypt_t		*cr = NIL(Crypt_t*);

	if(type == VC_OPENING)
	{	if(!(cr = (Crypt_t*)calloc(1, sizeof(Crypt_t))) )
			return -1;

		cr->type = CR_AES128|CR_INIT;
		cr->head = 1; /* 1 control byte */
		cr->meth = Vcxaes128; /* default encryption method */
		for(data = (char*)param; data; )
		{	val[0] = 0;
			data = vcgetmtarg(data, val, sizeof(val), _Cryptargs, &arg);

			switch(TYPECAST(int,arg->data))
			{ case CR_AES128:
				cr->type = CR_AES128|CR_INIT;
				cr->meth = Vcxaes128;
				break;
			  case CR_AES192:
				cr->type = CR_AES192|CR_INIT;
				cr->meth = Vcxaes192;
				break;
			  case CR_AES256:
				cr->type = CR_AES256|CR_INIT;
				cr->meth = Vcxaes256;
				break;
			}
		}

		vcsetmtdata(vc, cr);
		return 0;
	}
	else if(type == VC_EXTRACT || type == VC_RESTORE)
		return cryptheader(vc, (Vcmtcode_t*)param) < 0 ? -1 : 1;
	else if(type == VC_CLOSING)
	{	if((cr = vcgetmtdata(vc, Crypt_t*)) )
		{	if(!(cr->type&CR_INIT))
				vcxstop(&cr->xx);
			free(cr);
		}
		return 0;
	}
	else	return 0;
}

Vcmethod_t	_Vccrypt =
{	vcencrypt,
	vcdecrypt,
	cryptevent,
	"crypt",
	"Data encryption.",
	"[-?\n@(#)$Id: vcodex-crypt (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	_Cryptargs,
	1024*1024,
	0
};

VCLIB(Vccrypt)
