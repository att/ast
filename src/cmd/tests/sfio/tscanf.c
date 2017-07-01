/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"sftest.h"

#if _hdr_values
#include	<values.h>
#endif

#if _hdr_math
#include	<math.h>
#endif

#if _hdr_float
#include	<float.h>
#endif

#if defined(MAXDOUBLE)
#define	MAXD	MAXDOUBLE
#endif
#if !defined(MAXD) && defined(DBL_MAX)
#define MAXD	DBL_MAX
#endif
#if !defined(MAXD)
#define MAXD	(double)(~((unsigned long)0))
#endif

typedef struct Fmt_s
{
	Sffmt_t		fmt;
	Void_t**	args[2];
	int		arg;
} Fmt_t;

static int
#if __STD_C
extf(Sfio_t* sp, Void_t* vp, Sffmt_t* dp)
#else
extf(sp, vp, dp)
Sfio_t*		sp;
Void_t*		vp;
Sffmt_t*	dp;
#endif
{
	register Fmt_t*		fmt = (Fmt_t*)dp;

	dp->flags |= SFFMT_VALUE;
	*((Void_t**)vp) = fmt->args[fmt->arg++];
	return 0;
}

tmain()
{
	char		str[8], c[4], cl[8];
	int		i, j, k, n;
	unsigned long	a1, a2, a3;
	float		f;
	double		d;
	char*		s;
	Void_t*		vp;
	Sfio_t*		sf;
	Fmt_t		fmt;

	str[0] = 'x'; str[1] = 0;
	n = sfsscanf("123","%[a-z]%d",str,&i);
	if(n != 0)
		terror("Bad %%[ scanning1");
	n = sfsscanf("123","%#[a-z]%d",str,&i);
	if(n != 2 || str[0] != 0 || i != 123)
		terror("Bad %%[ scanning2");

	str[0] = str[1] = str[2] = str[3] =
	str[4] = str[5] = str[6] = str[7] = 'x';
	c[0] = c[1] = c[2] = c[3] = 'x';
	cl[0] = cl[1] = cl[2] = cl[3] =
	cl[4] = cl[5] = cl[6] = cl[7] = 'x';

	i = -1;
	if(sfsscanf("123456789","%.*.*d",4,10,&i) != 1)
		terror("Bad %%d scanning");
	if(i != 1234)
		terror("Expected 1234, got %d", i);

	i = -1;
	if(sfsscanf("0","%i",&i) != 1 || i != 0)
		terror("Bad %%i scanning1");
	i = -1;
	if(sfsscanf("0x","%1i%c",&i,c) != 2 || i != 0 || c[0] != 'x')
		terror("Bad %%i scanning2");
	i = -1;
	if(sfsscanf("0x1","%i",&i) != 1 || i != 1)
		terror("Bad %%i scanning3");
	i = -1;
	if(sfsscanf("07","%i",&i) != 1 || i != 7)
		terror("Bad %%i scanning4");
	i = -1;
	if(sfsscanf("08","%i%i",&i,&j) != 2 || i != 0 || j != 8)
		terror("Bad %%i scanning5");

	sfsscanf("1234567890","%4I*s%2I*c%4I*[0-9]",4,str,2,c,6,cl);

	if(strcmp(str,"123") != 0)
		terror("Bad s");
	if(str[4] != 'x')
		terror("str overwritten");

	if(strncmp(c,"56",2) != 0)
		terror("Bad c");
	if(c[2] != 'x')
		terror("c overwritten");

	if(strcmp(cl,"7890") != 0)
		terror("Bad class");
	if(cl[5] != 'x')
		terror("cl overwritten");

	if(sfsscanf("123 ab","%*d") != 0)
		terror("Bad return value");

	if(sfsscanf("123abcA","%[0-9]%[a-z]%[0-9]",str,c,cl) != 2 ||
	   strcmp(str,"123") != 0 || strcmp(c,"abc") != 0)
		terror("Bad character class scanning");

	if(sfsscanf("123 456 ","%d %d%n",&i,&j,&n) != 2)
		terror("Bad integer scanning");
	if(i != 123 || j != 456 || n != 7)
		terror("Bad return values");

	if(sfsscanf("1 2","%d %d%n",&i,&j,&n) != 2)
		terror("Bad scanning2");
	if(i != 1 || j != 2 || n != 3)
		terror("Bad return values 2");

	if(sfsscanf("1234 1","%2d %d%n",&i,&j,&n) != 2)
		terror("Bad scanning3");
	if(i != 12 || j != 34 || n != 4)
		terror("Bad return values 3");

	if(sfsscanf("011234 1","%3i%1d%1d%n",&i,&j,&k,&n) != 3)
		terror("Bad scanning4");
	if(i != 9 || j != 2 || k != 3 || n != 5)
		terror("Bad return values 4");

	if(sfsscanf("4 6","%f %lf",&f, &d) != 2)
		terror("Bad scanning5");
	if(f != 4 || d != 6)
		terror("Bad return values f=%f d=%f", f, d);

	s = ".1234 .1234";
	if(sfsscanf(s,"%f %lf",&f, &d) != 2)
		terror("Bad scanning6");

	if(f <= .1233 || f >= .1235 || d <= .1233 || d >= .1235)
		terror("Bad return values: f=%.4f d=%.4lf",f,d);

	/* test for scanning max double value */
	s = sfprints("%.14le",MAXD);
	if(!s || s[0] < '0' || s[0] > '9')
		terror("sfprints failed");
	for(i = 0; s[i]; ++i)
		if(s[i] == 'e')
			break;
	if(s[i-1] > '0' && s[i-1] <= '9')
		s[i-1] -= 1;
	sfsscanf(s,"%le",&d);
	if(d > MAXD || d < MAXD/2)
		terror("sfscanf of MAXDOUBLE failed");

	if(!(sf = sftmp(8*1024)) )
		terror("Opening temp file");

	for(k = 2; k <= 64; ++k)
	{	sfseek(sf,(Sfoff_t)0,0);
		for(i = 0; i < 1000; ++i)
			sfprintf(sf,"%#..*d\n",k,i);
		sfseek(sf,(Sfoff_t)0,0);
		for(i = 0; i < 1000; ++i)
		{	if(sfscanf(sf,"%i",&j) != 1)
				terror("Scanf failed");
			if(i != j)
				terror("Wrong scanned value");
		}
	}

	/* test %p */
	s = sfprints("%p", sf);
	sfsscanf(s, "%p", &vp);
	if(vp != (Void_t*)sf)
		terror("Wrong pointer scan");

	if(sfsscanf("2#1001","%i",&i) != 1 || i != 9)
		terror("Bad %%i scanning");
	if(sfsscanf("2#1001","%#i%c",&i,c) != 2 || i != 2 || c[0] != '#')
		terror("Bad %%#i scanning");

	n = -1;
	if(sfsscanf("12345","%d%n",&k,&n) != 1 || k != 12345 || n != 5)
		terror("Bad scanning results");
	n = -1;
	if(sfsscanf("12345","%d %n",&k,&n) != 1 || k != 12345 || n != 5)
		terror("Bad scanning results");
	n = -1;
	if(sfsscanf("12345 ","%d%n",&k,&n) != 1 || k != 12345 || n != 5)
		terror("Bad scanning results");
	n = -1;
	if(sfsscanf("12345 ","%d %n",&k,&n) != 1 || k != 12345 || n != 6)
		terror("Bad scanning results");

	n = sfsscanf("zis.zis.gawpwo", "%..36lu.%..36lu.%..36lu", &a1, &a2, &a3);
	s = sfprints("%d %lu %lu %lu", n, a1, a2, a3);
	if (!s)
		terror("sfprints failed");
	if (strcmp(s, "3 46036 46036 985781544"))
		terror("Base 36 scan failed");

	if(sfsscanf("NaNS", "%g", &f) != 1)
		terror("Scanning NaN failed");

	fmt.fmt.version = SFIO_VERSION;
	fmt.fmt.extf = extf;
	fmt.fmt.eventf = 0;
	fmt.fmt.form = "%d %g";
	fmt.arg = 0;
	fmt.args[0] = (Void_t*)&n; n = 0;
	fmt.args[1] = (Void_t*)&f; f = 0;
	i = sfsscanf("123 3.1415", "%!", &fmt.fmt);
	if(i != 2 || n != 123 || f <= 3.1414 || f >= 3.1416)
		terror("%%! failed i=%d n=%d d=%g", i, n, f);

	k = 0;
	if(sfsscanf("%1", "%%%d", &k) != 1 || k != 1)
		terror("%%%% failed");
	k = 0;
	if(sfsscanf(" %1", "%%%d", &k) != 1 || k != 1)
		terror("%%%% does not skip leading space");
	k = 0;
	if(sfsscanf("%1", "%*[%]%d", &k) != 1 || k != 1)
		terror("%%*%% failed");
	k = 0;
	if(sfsscanf(" %1", "%*[%]%d", &k) == 1 && k == 1)
		terror("%%*%% skips leading space");

	texit(0);
}
