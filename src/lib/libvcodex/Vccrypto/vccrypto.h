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
#ifndef _VCCRYPTO_H
#define _VCCRYPTO_H	1

#include	<vcodex.h>

/* Encryption/Checksumming transforms.
**
** Written by Kiem-Phong Vo.
*/

typedef ssize_t(*Vcxpasskey_f)_ARG_((char*, ssize_t, int));

typedef struct _vcxmethod_s	Vcxmethod_t;
typedef struct _vcx_s		Vcx_t;

struct _vcxmethod_s
{	ssize_t		(*initf) _ARG_((Vcx_t*, Vcxmethod_t*, Vcchar_t*, ssize_t));
	int		(*stopf) _ARG_((Vcx_t*));
	ssize_t		(*encodef) _ARG_((Vcx_t*, const Void_t*, ssize_t, Vcchar_t**));
	ssize_t		(*decodef) _ARG_((Vcx_t*, const Void_t*, ssize_t, Vcchar_t**));
};

struct _vcx_s
{	Vcxmethod_t*	meth;	/* encryption/checksum method	*/
	Void_t*		data;	/* encryption/checksum states	*/
	ssize_t		keyz;	/* encryption/checksum key size	*/
	Vcchar_t	key[128]; /* key data, up to 1024 bits	*/
};

#define vcxinit(_xx, _mt, _ky, _sz)	(*(_mt)->initf)((_xx), (_mt), (_ky), (_sz))
#define vcxstop(_xx)			(*(_xx)->meth->stopf)((_xx))
#define vcxencode(_xx, _dt, _dz, _oo) \
			(!(_xx)->meth->encodef ? -1 : \
				(*(_xx)->meth->encodef)((_xx), (_dt), (_dz), (_oo)) )
#define vcxdecode(_xx, _dt, _dz, _oo) \
			(!(_xx)->meth->decodef ? -1 : \
				(*(_xx)->meth->decodef)((_xx), (_dt), (_dz), (_oo)) )

_BEGIN_EXTERNS_
extern Vcxpasskey_f	Vcxpasskeyf;	/* function to get passkey, if any	*/

extern ssize_t		vcxhash _ARG_((Vcchar_t*, ssize_t, Vcchar_t*, ssize_t));

extern Vcxmethod_t*	Vcxcrcsum;	/* Cyclic Redundancy Coding (32-bit)	*/
extern Vcxmethod_t*	Vcxmd5sum;	/* MD5 one-way hash function (128-bit)	*/
extern Vcxmethod_t*	Vcxaessum;	/* AES one-way hash function (128-bit)	*/

extern Vcxmethod_t*	Vcxaes128;	/* AES-128 encryptor, 128-bit key	*/
extern Vcxmethod_t*	Vcxaes192;	/* AES-192 encryptor, 192-bit key	*/
extern Vcxmethod_t*	Vcxaes256;	/* AES-256 encryptor, 256-bit key	*/

extern Vcmethod_t*	Vcsum;		/* Vcodex checksumming transform	*/
extern Vcmethod_t*	Vccrypt;	/* Vcodex encryption transform		*/
_END_EXTERNS_

#endif /*_VCCRYPTO_H*/
