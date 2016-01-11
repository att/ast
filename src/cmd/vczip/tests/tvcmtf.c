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

MAIN()
{
	char		*tar, *del, *t;
	Vcodex_t	*vc;
	ssize_t		k, n;

	tar =	"00000011111122222abc012000000111111222222012abc";
	/*  MTF: O0000010000020000abc555200000200000200000222555
          ASCII: O     1     2    abc
	    POS: 01234567890123456789012345678901234567890123456
	*/

	if(!(vc = vcopen(0, Vcmtf, "0", 0, VC_ENCODE)) )
		terror("Cannot open Vcmtf handle");

	if((n = vcapply(vc, tar, strlen(tar), &del)) <= 0)
		terror("Vcmtf0 failed");

	if(!(vc = vcopen(0, Vcmtf, "0", 0, VC_DECODE)) )
		terror("Cannot open Vcunmtf0 handle");
	if((n = vcapply(vc, del, n, &t)) != strlen(tar) )
		terror("Vcunmtf0 returns wrong size");
	for(k = 0; k < n; ++k)
		if(t[k] != tar[k])
			terror("Vcunmtf0 computed bad byte");

	tar =	"00000011111122222abc012000000111111222222012abc";
	/*  MTS: 01000011000021000abc550210000210000210000220555
          ASCII: O     1     2    abc
	    POS: 01234567890123456789012345678901234567890123456
	*/

	if(!(vc = vcopen(0, Vcmtf, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vcmtf1 handle");

	if((n = vcapply(vc, tar, strlen(tar), &del)) <= 0)
		terror("Vcmtf1 failed");

	if(!(vc = vcopen(0, Vcmtf, 0, 0, VC_DECODE)) )
		terror("Cannot open Vcunmtf1 handle");
	if((n = vcapply(vc, del, n, &t)) != strlen(tar) )
		terror("Vcunmtf returns wrong size");
	for(k = 0; k < n; ++k)
		if(t[k] != tar[k])
			terror("Vcunmtf computed bad byte");

	exit(0);
}
