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

static int Freq[256] =
{
155, 18, 15,  4,  2,  2,  0,  3, 14,  5,  3,  4, 17,  1,  0,  3,
136,  2,  5,  5,  5,  1,  2,  3, 11,  2,  4,  2, 14, 13, 12, 13,
 12,  9,  7,  1, 17,  4,  0,  4, 16,  1,  2,  4,  9,  2,  2,  2,
 20,  1,  0,  1,  4,  2,  2,  3,  7,  5,  0,  0,  4,  4,  2,  1,
  7,  1,  7,  2,  9,  7, 11,  8, 23,  3,  3,  1,  5,  2,  2,  4,
 13,  0,  0,  1, 21,  1,  1,  2,  9,  1,  0,  0,  3,  2,  1,  3,
  9,  0,  1,  6,  7,  0,  3,  0,  3,  1,  0,  2, 13,  0,  2,  0,
  6,  1,  1,  5,  1,  0,  0,  0,  3,  0,  2,  0,  1,  2,  1,  2,
 10,  4,  0,  1,  2,  0,  1,  2,  7,  2,  0,  0,  3,  1,  3,  1,
 10,  2,  0,  2,  3,  2,  3,  4,  3,  0,  2,  3, 18,  3,  3,  1,
  5,  4,  3,  5,  7,  8,  6,  2,  3,  4,  0,  3, 16,  0,  6,  2,
 16,  4,  2,  1,  4,  2,  0,  4, 14,  3,  0,  0,  1,  0,  1,  0,
  5,  1,  1,  4,  5,  1,  2,  2,  9,  1,  1,  1,  9,  1,  1,  0,
  7,  1,  2,  0, 12,  1,  1,  1, 14,  0,  1,  1,  2,  3,  2,  5,
  7,  1, 10,  1,  4,  0,  1,  3,  3,  0,  2,  2,  4,  2,  1,  2,
 10,  0,  0,  1,  4,  0,  2,  0,  6,  1,  0,  1,  2,  0,  0, 10
};


MAIN()
{
	char		*cmp, *t, *b, *endb;
	Vcodex_t	*bwz, *unz;
	Vcodex_t	*huf, *rle, *mtf;
	ssize_t		k, n, nb, nc;
	static char	buf[32*1024];

	b = buf; endb = buf + sizeof(buf);
	while(b < endb)
	{	for(k = 0; k < 256 && b < endb; ++k)
		{	for(n = 0; n < Freq[k] && b < endb; ++n)
				*b++ = k;
		}
	}
	nb = b-buf;

	if(!(huf = vcopen(0, Vchuffgroup, 0, 0, VC_ENCODE)) )
		terror("Can't open Vcgroup handle to compress");
	if(!(rle = vcopen(0, Vcrle, "0", huf, VC_ENCODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to compress");
	if(!(mtf = vcopen(0, Vcmtf, 0, rle, VC_ENCODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to compress");
	if(!(bwz = vcopen(0, Vcbwt, 0, mtf, VC_ENCODE|VC_CLOSECODER)) )
		terror("Cannot open Vcbwt handle to compress");

	if(!(huf = vcopen(0, Vchuffgroup, 0, 0, VC_DECODE)) )
		terror("Can't open Vcgroup handle to uncompress");
	if(!(rle = vcopen(0, Vcrle, "0", huf, VC_DECODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to uncompress");
	if(!(mtf = vcopen(0, Vcmtf, 0, rle, VC_DECODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to uncompress");
	if(!(unz = vcopen(0, Vcbwt, 0, mtf, VC_DECODE)) )
		terror("Cannot open Vcunbw handle to decode");

	if((nc = vcapply(bwz, buf, nb, &cmp)) < 0)
		terror("Vcbwzip failed");
	if((n = vcapply(unz, cmp, nc, &t)) != nb)
		terror("Vcunbwzip failed");
	for(k = 0; k < nb; ++k)
		if(t[k] != buf[k])
			terror("Bad byte");

	/* test header encoding */
	if((n = vcextract(bwz, (Void_t**)&b)) < 0)
		terror("vcextract() failed");

	/* test vcdecoder() */
	if(!(unz = vcrestore(b,n)) )
		terror("vcrestore() failed");

	if((n = vcapply(bwz, buf, nb, &t)) < 0)
		terror("Vcbwzip failed after vcextract()");
	if(n != nc || memcmp(t,cmp,n) != 0)
		terror("Vcbwzip get bad string after vcextract()");

	if((n = vcapply(unz, cmp, nc, &t)) < 0)
		terror("Vcunbwzip failed after vcrestore()");
	if(n != nb || memcmp(t,buf,nb) != 0)
		terror("Vcunbwzip get bad string after vcrestore()");

	exit(0);
}
