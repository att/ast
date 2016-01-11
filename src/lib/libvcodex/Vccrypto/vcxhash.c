/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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

/* Crypto-hashing a string based on AES-256
**
** Written by Kiem-Phong Vo
*/	

ssize_t vcxhash(Vcchar_t* data, ssize_t dtsz, Vcchar_t* hash, ssize_t hssz)
{
	Vcchar_t	*dg;
	ssize_t		dgsz, sz;
	Vcx_t		xx;

	if(!data || dtsz <= 0 || !hash || hssz <= 0)
		return -1;

	if(vcxinit(&xx, Vcxaessum, NIL(Vcchar_t*), 0) < 0)
		return -1;

	for(dgsz = hssz; dgsz > 0; dgsz -= sz, hash += sz)
	{	if((sz = vcxencode(&xx, data, dtsz, &dg)) <= 0 )
			return -1;
		if(sz > dgsz)
			sz = dgsz;
		memcpy(hash, dg, sz);
	}

	vcxstop(&xx);

	return hssz;
}


/* Internal function for making an encryption key from a user key.
** There are three cases:
** 1. dtsz  < 0: the user key is to be used as-is with length |dtsz|.
** 2. dtsz == 0: the user key is a null-terminated string.
** 3. dtsz  > 0: the user key is a character array of length dtsz.
*/
ssize_t _vcxmakekey(Vcchar_t* data, ssize_t dtsz, Vcchar_t* key, ssize_t kysz)
{
	if(!data)
		return 0;
	if(dtsz == 0) /* a null-terminated string */
		if((dtsz = strlen((char*)data)) == 0)
			return 0;

	if(!key || kysz <= 0)
		return -1;

	if(dtsz > 0)
		return vcxhash(data, dtsz, key, kysz);
	else /* fixed key data */
	{	if((dtsz = -dtsz) > kysz)
			return -1;
		memcpy(key, data, dtsz);
		return dtsz;
	}
}
