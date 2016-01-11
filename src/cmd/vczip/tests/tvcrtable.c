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

static char *Record1[] =
{
	"123:cdcd:111.222/3:vv:10\n",
	"456:cdcd:222.333/4:uu:20\n",
	"789:cdcd:333.111/1:tt:30\n",
};
static char *Record2[] =
{
	"123:abab:222.333/4:99:11\n",
	"xxx:abab:333.444/1:88:21\n",
	"789:abab:444.111/2:77:31\n",
};

static Vcchar_t	Data[1024];

MAIN()
{
	Vcodex_t	*tz, *uz;
	Vcodex_t	*huf, *rle, *mtf;
	Vcchar_t	*dt, *cmp;
	ssize_t		nc, nu, n, k, r, dtsz1, dtsz2;

	/* construct first half of test data */
	dtsz1 = 0;
	for(k = 0, r = 0; r < 6; ++r, ++k)
	{	if(k >= sizeof(Record1)/sizeof(Record1[0]))
			k = 0;
		n = strlen(Record1[k]);
		if(dtsz1+n > sizeof(Data)/2)
			break;
		memcpy(Data+dtsz1, Record1[k], n);
		dtsz1 += n;
	}

	/* construct second half of test data */
	dtsz2 = dtsz1;
	for(k = 0, r = 0; r < 6; ++r, ++k)
	{	if(k >= sizeof(Record2)/sizeof(Record2[0]))
			k = 0;
		n = strlen(Record2[k]);
		if(dtsz2+n > sizeof(Data))
			break;
		memcpy(Data+dtsz2, Record2[k], n);
		dtsz2 += n;
	}

	/* construct encoder */
	if(!(huf = vcopen(0, Vchuffgroup, 0, 0, VC_ENCODE)) )
		terror("Can't open Vchuffgroup handle to compress");
	if(!(rle = vcopen(0, Vcrle, "0", huf, VC_ENCODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to compress");
	if(!(mtf = vcopen(0, Vcmtf, 0, rle, VC_ENCODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to compress");
	if(!(tz = vcopen(0, Vcrtable, 0, mtf, VC_ENCODE)) )
		terror("Vcrtable: could not open handle to encode");

	/* construct decoder */
	if(!(huf = vcopen(0, Vchuffgroup, 0, 0, VC_DECODE)) )
		terror("Can't open Vchuffgroup handle to decompress");
	if(!(rle = vcopen(0, Vcrle, "0", huf, VC_DECODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to decompress");
	if(!(mtf = vcopen(0, Vcmtf, 0, rle, VC_DECODE|VC_CLOSECODER)) )
		terror("Can't open Vcrle handle to decompress");
	if(!(uz = vcopen(0, Vcrtable, 0, mtf, VC_DECODE)) )
		terror("Vcrtable: could not open handle to decode");

	/* compress first half */
	if((nc = vcapply(tz, Data, dtsz1+4, &cmp)) <= 0 )
		terror("Vcrtable: fail transforming");
	if(vcundone(tz) != 4)
		terror("Vcrtable: wrong size of undone data");
	twarn("Vctable: rawsz=%d cmpsz=%d\n", dtsz1, nc);

	if((nu = vcapply(uz, cmp, nc, &dt)) != dtsz1)
		terror("Vcrtable: fail decoding");
	if(memcmp(dt, Data, nu) != 0)
		terror("Vcrtable: results did not match");

	/* compress all data */
	if((nc = vcapply(tz, Data, dtsz2, &cmp)) <= 0 )
		terror("Vcrtable: fail transforming");
	twarn("Vctable: rawsz=%d cmpsz=%d\n", dtsz2, nc);

	if((nu = vcapply(uz, cmp, nc, &dt)) != dtsz2)
		terror("Vcrtable: fail decoding");
	if(memcmp(dt, Data, nu) != 0)
		terror("Vcrtable: results did not match");

	exit(0);
}
