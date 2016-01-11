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

/* 	This test various forms of integer codings
**
**	Written by Kiem-Phong Vo
*/

MAIN()
{
#define N_LIST		1024
	Vcchar_t	buf[1024*N_LIST];
	Vcio_t		io;
	Vcint_t		v, g, list[N_LIST], copy[N_LIST];

	/* test gamma coding */
	vcioinit(&io, buf, sizeof(buf));
	vciosetb(&io, io.bits, io.nbits, VC_ENCODE);
	if(vcioputg(&io, 1) < 0)
		terror("vcioputg 1\n");
	if(vcioputg(&io, 11) < 0)
		terror("vcioputg 11\n");
	if(vcioputg(&io, 101) < 0)
		terror("vcioputg 101\n");
	if(vcioputg(&io, 1) < 0)
		terror("vcioputg 1\n");
	if(vcioputg(&io, 1001) < 0)
		terror("vcioputg 1001\n");
	if(vcioputg(&io, 10001) < 0)
		terror("vcioputg 10001\n");
	if(vcioputg(&io, 1) < 0)
		terror("vcioputg 1\n");
	if(vcioputg(&io, 100001) < 0)
		terror("vcioputg 100001\n");
	vcioendb(&io, io.bits, io.nbits, VC_ENCODE);

	io.next = io.data;
	vciosetb(&io, io.bits, io.nbits, VC_DECODE);
	if((v = vciogetg(&io)) != 1)
		terror("vciogetg 1 != %d\n", (int)v);
	if((v = vciogetg(&io)) != 11)
		terror("vciogetg 11 != %d\n", (int)v);
	if((v = vciogetg(&io)) != 101)
		terror("vciogetg 101 != %d\n", (int)v);
	if((v = vciogetg(&io)) != 1)
		terror("vciogetg 1 != %d\n", (int)v);
	if((v = vciogetg(&io)) != 1001)
		terror("vciogetg 1001 != %d\n", (int)v);
	if((v = vciogetg(&io)) != 10001)
		terror("vciogetg 10001 != %d\n", (int)v);
	if((v = vciogetg(&io)) != 1)
		terror("vciogetg 1 != %d\n", (int)v);
	if((v = vciogetg(&io)) != 100001)
		terror("vciogetg 100001 != %d\n", (int)v);
	vcioendb(&io, io.bits, io.nbits, VC_DECODE);

	/* Generate a list of positive integers. 
	** Positivity is needed to test the gamma code which does not handle zero.
	*/
	list[0] = 2; list[1] = 6; list[2] = 9; list[3] = 7; list[4] = 2;
	for(v = 5; v < N_LIST; )
	{	int	k, inc;
		inc = random()%2;
		for(k = random()%7 + 1; k > 0 && v < N_LIST; --k, ++v)
		{	if(inc)
				list[v] = list[v-1] + (random()%17);
			else	list[v] = list[v-1] - (random()%53);
			if(list[v] <= 0)
				list[v] = random()%37 + 1;
		}
	}
	memcpy(copy,list,N_LIST*sizeof(Vcint_t));

	/* check coding size of original list using base-128 coding */
	vcioinit(&io, buf, sizeof(buf));
	for(v = 0; v < N_LIST; ++v)
		vcioputu(&io, list[v]);
	twarn("vcioputu %d: output size=%d\n", N_LIST, vciosize(&io));

	vcioinit(&io, buf, sizeof(buf));
	for(v = 0; v < N_LIST; ++v)
		if((g = vciogetu(&io)) != list[v])
			terror("Bad vciogetu %d", g);

	/* check coding size of original list using gamma coding */
	vcioinit(&io, buf, sizeof(buf));
	vciosetb(&io, io.bits, io.nbits, VC_ENCODE);
	for(v = 0; v < N_LIST; ++v)
		vcioputg(&io, list[v]);
	vcioendb(&io, io.bits, io.nbits, VC_ENCODE);
	twarn("vcioputg %d: output size=%d\n", N_LIST, vciosize(&io));

	vcioinit(&io, buf, sizeof(buf));
	vciosetb(&io, io.bits, io.nbits, VC_DECODE);
	for(v = 0; v < N_LIST; ++v)
		if((g = vciogetg(&io)) != list[v])
			terror("Bad vciogetg %d", g);
	vcioendb(&io, io.bits, io.nbits, VC_DECODE);

	/* test integer list io */
	vcioinit(&io, buf, sizeof(buf));
	if((v = vcioputlist(&io, list, N_LIST)) < 0)
		terror("vcioputlist %d\n", (int)v);
	twarn("vcioputlist %d: output size=%d\n", N_LIST, v);

	for(v = 0; v < N_LIST; ++v)
		list[v] = -1;
	vcioinit(&io, buf, sizeof(buf));
	if((v = vciogetlist(&io, list, N_LIST)) != N_LIST)
		terror("vciogetlist %d\n", (int)v);
	for(v = 0; v < N_LIST; ++v)
		if(list[v] != copy[v])
			terror("Wrong value %d\n", (int)list[v]);

	exit(0);
}
