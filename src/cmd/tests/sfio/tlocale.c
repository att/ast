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
#if _lib_locale
#include	<locale.h>
#endif

tmain()
{
#if _lib_locale
	char		buf[128], cmp[128];
	float		d;
	int		n, decimal, thousand;
	struct lconv*	lv;

	setlocale(LC_ALL, "");

	if(!(lv = localeconv()))
		texit(0);

	decimal = '.';
	if(lv->decimal_point && lv->decimal_point[0])
		decimal = lv->decimal_point[0];

	thousand = 0;
	if(lv->thousands_sep && lv->thousands_sep[0])
		thousand = lv->thousands_sep[0];
		
	if(thousand)
		sfsprintf(cmp, sizeof(cmp), "1%c000", thousand);
	else	sfsprintf(cmp, sizeof(cmp), "1000");
	sfsprintf(buf, sizeof(buf), "%'d", 1000);
	if(strcmp(buf, cmp) != 0)
		terror("Bad printing");
	
	if(thousand)
		sfsprintf(cmp, sizeof(cmp), "1%c000%c10", thousand, decimal);
	else	sfsprintf(cmp, sizeof(cmp), "1000%c10", decimal);
	d = 0.;
	if((n = sfsscanf(cmp, "%'f", &d)) != 1)
		terror("Scan error %d", n);
	if(d < 1000.099 || d > 1000.101)
		terror("Bad scanning");
	sfsprintf(buf, sizeof(buf), "%.2f", d);
	if(strcmp(buf, "1000.10") != 0)
		terror("Deep formatting error");
#endif

	texit(0);
}
