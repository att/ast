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
	char		*tar, *cmp, *t, *b, *endb;
	Vcodex_t	*vch, *vcu;
	ssize_t		k, n, ntar;
	static char	buf[1024*1024];

	if(!(vch = vcopen(0, Vchuffman, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vchuff handle");
	if(!(vcu = vcopen(0, Vchuffman, 0, 0, VC_DECODE)) )
		terror("Cannot open Vchuff handle to decode");

	/* test a few boundary cases */
	tar = "000000000000000011111122222abc012000000111111222222012abc";
	ntar = strlen(tar);
	if((n = vcapply(vch, tar, 0, &cmp)) < 0)
		terror("Vchuff failed0");
	if((n = vcapply(vcu, cmp, n, &t)) != 0)
		terror("Vchuff failed0 decoding");

	if((n = vcapply(vch, tar, 1, &cmp)) < 0)
		terror("Vchuff failed1");
	if((n = vcapply(vcu, cmp, n, &t)) != 1)
		terror("Vchuff failed1 decoding");
	if(t[0] != tar[0])
		terror("Bad byte");

	if((n = vcapply(vch, tar, 16, &cmp)) < 0) /* the string of all zeros */
		terror("Vchuff failed2");
	if((n = vcapply(vcu, cmp, n, &t)) != 16)
		terror("Vchuff failed2 decoding");
	for(k = 0; k < 16; ++k)
		if(t[k] != tar[k])
			terror("Bad byte");

	/* test the original learning algorithm */
	if((n = vcapply(vch, tar, ntar, &cmp)) <= 0)
		terror("Vchuff failed");
	if((n = vcapply(vcu, cmp, n, &t)) != ntar )
		terror("Vchuff returns wrong size");
	for(k = 0; k < n; ++k)
		if(t[k] != tar[k])
			terror("Vchuff computed bad byte");

	/* generate a large array of data using the frequency profile */
	b = buf;
	for(k = 0; k < 256; ++k)
	{	for(n = 0; n < Freq[k]; ++n)
			*b++ = k;
	}
	if((n = vcapply(vch, buf, b-buf, &cmp)) < 0)
		terror("Vchuff failed8");
	if((n = vcapply(vcu, cmp, n, &t)) != b-buf)
		terror("Vchuff failed8 decoding");
	for(k = 0; k < b-buf; ++k)
		if(t[k] != buf[k])
			terror("Bad byte8");

	/* test grouping method */
	b = buf; endb = buf + sizeof(buf);
	while(b < endb)
	{	for(k = 0; k < 256 && b < endb; ++k)
		{	for(n = 0; n < Freq[k] && b < endb; ++n)
				*b++ = k;
		}
	}
	if(!(vch = vcopen(0, Vchuffgroup, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vchuffgroup handle");
	if(!(vcu = vcopen(0, Vchuffgroup, 0, 0, VC_DECODE)) )
		terror("Cannot open Vchuffgroup handle decoding ");
	if((n = vcapply(vch, buf, b-buf, &cmp)) < 0)
		terror("Vchuffgroup failed");
	if((n = vcapply(vcu, cmp, n, &t)) != b-buf)
		terror("Vchuffgroup failed decoding");
	for(k = 0; k < b-buf; ++k)
		if(t[k] != buf[k])
			terror("Bad byte");

	/* test partitioning method */
	if(!(vch = vcopen(0, Vchuffpart, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vchuffpart handle");
	if(!(vcu = vcopen(0, Vchuffpart, 0, 0, VC_DECODE)) )
		terror("Cannot open Vchuffpart handle decoding");
	if((n = vcapply(vch, buf, b-buf, &cmp)) < 0)
		terror("Vchuffpart failed");
	if((n = vcapply(vcu, cmp, n, &t)) != b-buf)
		terror("Vchuffpart failed decoding");
	for(k = 0; k < b-buf; ++k)
		if(t[k] != buf[k])
			terror("Bad byte");

	exit(0);
}
