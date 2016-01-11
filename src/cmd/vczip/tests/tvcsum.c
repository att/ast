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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"vctest.h"
#include	"vccrypto.h"

/* hold test strings */
typedef struct _pair_s
{	Vcchar_t*	str;
	Vcchar_t*	sum;
} Pair_t;

static Pair_t	Md5[] =
{	{ "0123456789", "781E5E245D69B566979B86E28D23F2C7" },
	{ "abcdefghij", "A925576942E94B2EF57A066101B48876" },
	{ 0, 0 }
};

MAIN()
{
	ssize_t		k, n;
	Vcchar_t	*sum, hex[1024];
	Vcx_t		xx;

	if(vcxinit(&xx, Vcxmd5sum, 0, 0) < 0)
		terror("Initializing md5 handle");
	for(k = 0; Md5[k].str; ++k)
	{	if((n = vcxencode(&xx, Md5[k].str, strlen((char*)Md5[k].str), &sum)) < 0 )
			terror("Encoding data");
		if(n != 16)
			terror("Bad md5 digest length");
		if(vchexcode(sum, n, hex, sizeof(hex), 1) != 32)
			terror("Bad md5 hex coding length");
		if(strcmp((char*)Md5[k].sum, (char*)hex) != 0)
			terror("Bad md5 digest");
	}
	vcxstop(&xx);

	exit(0);
}
