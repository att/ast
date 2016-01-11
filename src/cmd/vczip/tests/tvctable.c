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
*                   Phong Vo <kpv@research.att.com>                    *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"vctest.h"

typedef Vcchar_t	Matrix_t[16][8];

static Matrix_t Mt =
{	'a', '0', 'u', 'd', 'i', 'e', 'a', 'b',
	'a', '1', 'v', 'd', 'j', 'e', 'b', 'c',
	'b', '0', 'x', 'd', 'i', 'e', 'c', 'd',
	'b', '1', 'y', 'd', 'j', 'e', 'd', 'e',

	'b', '2', 'z', 'd', 'i', 'e', 'e', 'f',
	'a', '0', 'u', 'd', 'j', 'e', 'f', 'g',
	'a', '2', 'w', 'd', 'i', 'e', 'g', 'h',
	'b', '1', 'y', 'd', 'j', 'e', 'h', 'i',

	'a', '0', 'u', 'd', 'i', 'e', 'i', 'j',
	'a', '1', 'v', 'd', 'j', 'e', 'j', 'k',
	'a', '2', 'w', 'd', 'i', 'e', 'k', 'l',
	'b', '0', 'x', 'd', 'j', 'e', 'l', 'm',

	'b', '1', 'y', 'd', 'i', 'e', 'm', 'n',
	'b', '2', 'z', 'd', 'j', 'e', 'n', 'o',
	'b', '1', 'y', 'd', 'i', 'e', 'o', 'p',
	'a', '0', 'u', 'd', 'j', 'e', 'p', 'q'
};

int main()
{
	Vcodex_t	*tz, *uz;
	Vcodex_t	*huf, *rle, *mtf;
	Vcchar_t	*mt, *cmp, *tstr;
	Vcchar_t	store[2*sizeof(Mt)];
	ssize_t		nc, nu, n;
	Vcmethod_t	*Vctable;

	if(!(Vctable = vcgetmeth("table", 0)))
		terror("table plugin not found");

	if(!(huf = vcopen(0, Vchuffgroup, 0, 0, VC_ENCODE)) )
		terror("Can't open Vchuffgroup handle to compress");
	if(!(rle = vcopen(0, Vcrle, "0", huf, VC_ENCODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to compress");
	if(!(mtf = vcopen(0, Vcmtf, 0, rle, VC_ENCODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to compress");
	if(!(tz = vcopen(0, Vctable, 0, mtf, VC_ENCODE)) )
		terror("Vctable: could not open handle to transform");

	if((n = vcextract(tz, (Void_t**)(&tstr))) <= 0)
		terror("Cannot get encoding string");
	memcpy(store,tstr,n);

	if((nc = vcapply(tz, Mt, sizeof(Mt), &cmp)) <= 0 )
		terror("Vctable: fail transforming");
	twarn("Vctable: rawsz=%d cmpsz=%d\n", sizeof(Mt), nc);

	if(!(uz = vcrestore(store, n)) )
		terror("Vctable: could not recreate handle to decode");

	if((nu = vcapply(uz, cmp, nc, &mt)) != sizeof(Mt))
		terror("Vctable: fail untransforming");

	if(memcmp(mt, Mt, nu) != 0)
		terror("Vctable: results did not match");

	exit(0);
}
