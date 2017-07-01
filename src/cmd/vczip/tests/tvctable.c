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

#include	<vctable.h>

typedef Vcchar_t	Matrix_t[24][8];

static Matrix_t Mt =
{	'a', '0', 'u', 'd', 'i', 'e', '0', 'a',
	'a', '1', 'v', 'd', 'i', 'e', '1', 'a',
	'b', '0', 'x', 'd', 'j', 'e', '0', 'b',
	'b', '1', 'y', 'd', 'j', 'e', '1', 'b',

	'b', '2', 'z', 'd', 'j', 'e', '2', 'b',
	'a', '0', 'u', 'd', 'i', 'e', '0', 'a',
	'a', '2', 'w', 'd', 'i', 'e', '2', 'a',
	'b', '1', 'y', 'd', 'j', 'e', '1', 'b',

	'a', '0', 'u', 'd', 'i', 'e', '0', 'a',
	'a', '1', 'v', 'd', 'i', 'e', '1', 'a',
	'a', '2', 'w', 'd', 'i', 'e', '2', 'a',
	'b', '0', 'x', 'd', 'j', 'e', '0', 'b',

	'b', '1', 'y', 'd', 'j', 'e', '1', 'b',
	'b', '2', 'z', 'd', 'j', 'e', '2', 'b',
	'b', '1', 'y', 'd', 'j', 'e', '1', 'b',
	'a', '0', 'u', 'd', 'i', 'e', '0', 'a',

	'b', '2', 'z', 'd', 'j', 'e', '2', 'b',
	'a', '0', 'u', 'd', 'i', 'e', '0', 'a',
	'a', '2', 'w', 'd', 'i', 'e', '2', 'a',
	'b', '1', 'y', 'd', 'j', 'e', '1', 'b',

	'a', '0', 'u', 'd', 'i', 'e', '0', 'a',
	'a', '1', 'v', 'd', 'i', 'e', '1', 'a',
	'a', '2', 'w', 'd', 'i', 'e', '2', 'a',
	'b', '0', 'x', 'd', 'j', 'e', '0', 'b',
};

MAIN()
{
	Vcodex_t	*tz, *uz;
	Vcodex_t	*huf, *rle, *mtf;
	Vcchar_t	*mt, *cmp, *tstr;
	Vcchar_t	store[2*sizeof(Mt)];
	ssize_t		nc, nu, n;
	Vctblplan_t	*plan;

	if(!(plan = vctblmakeplan(Mt, sizeof(Mt), sizeof(Mt[0]), VCTBL_RLE)) )
		terror("Vctable: can't make compression plan");
	tmesg("\tTable: size=%d, ", sizeof(Mt) );
	tmesg("ncols=%d\n", plan->ncols);
	tmesg("\tTranform plan:\n");
	for(n = 0; n < plan->ncols; ++n)
	{	tmesg("\t\t%2d: ", plan->trans[n].index);
		tmesg("%2d ", plan->trans[n].pred1);
		tmesg("%2d\n", plan->trans[n].pred2);
	}

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

	if((nc = vcapply(huf, Mt, sizeof(Mt), &cmp)) <= 0 )
		terror("Vchuffgroup: fail transforming");
	twarn("Vchuffgroup: rawsz=%d cmpsz=%d\n", sizeof(Mt), nc);

	exit(0);
}
