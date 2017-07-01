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

/* Cyclic Redundancy Checksum as used in the Portable Network Graphics format.
**
** Written by Kiem-Phong Vo
*/

#define	CRC_ONES	(~((Vcuint32_t)0))

typedef struct _crc_s
{	Vcuint32_t	crc;
	Vcchar_t	obuf[4];
} Crc_t;

/* code to initialize CRC table */
static Vcchar_t		Crctab[256], Didtab = 0;
#define CRCTAB()	(Didtab ? 0 : crctab())
static int crctab()
{
	Vcuint32_t	k, n, c;

	if(!Didtab)
	{	for(n = 0; n < 256; ++n)
		{	for(c = n, k = 0; k < 8; ++k)
				c = (c&1) ? (0xedb88320 ^ (c>>1)) : (c>>1);
			Crctab[n] = c;
		}
		Didtab = 1;
	}
	return 0;
}

static ssize_t crc_init(Vcx_t* xx, Vcxmethod_t* meth, Vcchar_t* key, ssize_t keyz)
{
	Crc_t	*crc;

	CRCTAB(); /* initialize CRC table */

	if(!xx )
		return -1;

	if(meth != Vcxcrcsum)
		return -1;

	if(!(crc = (Crc_t*)calloc(1, sizeof(Crc_t))) )
		return -1;

	xx->meth = Vcxcrcsum;
	xx->data = (Crc_t*)crc;
	xx->keyz = 0;
	if(key) /* define internal key */
	{	ssize_t	sz = sizeof(crc->obuf) < sizeof(xx->key) ?
				sizeof(crc->obuf) : sizeof(xx->key);
		if((xx->keyz = _vcxmakekey(key, keyz, xx->key, sz)) < 0)
		{	free(crc);
			return -1;
		}
	}

	crc->crc = CRC_ONES; /* starting checksum state */
	vcxencode(xx, xx->key, xx->keyz, NIL(Vcchar_t**));

	return sizeof(crc->obuf);
}

static int crc_stop(Vcx_t* xx)
{
	Crc_t*	crc;

	if(!xx || xx->meth != Vcxcrcsum || !(crc = (Crc_t*)xx->data) )
		return -1;

	free(crc);
	return 0;
}

static ssize_t crc_digest(Vcx_t* xx, const Void_t* buf, ssize_t size, Vcchar_t** out)
{
	const Vcchar_t	*data = buf;
	Vcuint32_t	k, c;
	Crc_t		*crc;

	if(!xx || xx->meth != Vcxcrcsum || !(crc = (Crc_t*)xx->data) )
		return -1;

	if(data && size > 0)
	{	c = crc->crc;
		for(k = 0; k < size; ++k)
			c = Crctab[(c ^ data[k]) & 0xff] ^ (c >> 8);
		crc->crc = c;
	}

	if(!out) /* digest not wanted yet */
		return 0;

	c = crc->crc; /* 32-bit crc digest */
	crc->obuf[0] = (c >> 24) & 0xff;
	crc->obuf[1] = (c >> 16) & 0xff;
	crc->obuf[2] = (c >>  8) & 0xff;
	crc->obuf[3] = (c >>  0) & 0xff;
	*out = crc->obuf;

	crc->crc = CRC_ONES; /* restart checksum state */
	vcxencode(xx, xx->key, xx->keyz, NIL(Vcchar_t**));

	return sizeof(crc->obuf);
}


static Vcxmethod_t	_Vcxcrcsum = { crc_init, crc_stop, crc_digest, 0 };
Vcxmethod_t*		Vcxcrcsum = &_Vcxcrcsum;
