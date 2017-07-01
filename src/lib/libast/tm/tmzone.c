/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * time conversion support
 */

#include <ast.h>
#include <tm.h>
#include <ctype.h>

/*
 * return timezone pointer given name and type
 *
 * if type==0 then all time zone types match
 * otherwise type must be one of tm_info.zone[].type
 *
 * if end is non-null then it will point to the next
 * unmatched char in name
 *
 * if dst!=0 then it will point to 0 for standard zones
 * and the offset for daylight zones
 *
 * 0 returned for no match
 */

Tm_zone_t*
tmzone(register const char* name, char** end, const char* type, int* dst)
{
	register Tm_zone_t*	zp;
	register char*		p;
	char*			e;
	int			d;

	static Tm_zone_t	fixed;
	static char		off[16];

	tmset(tm_info.zone);
	if ((name[0] == '+' || name[0] == '-') && (fixed.west = tmgoff(name, &e, TM_LOCALZONE)) != TM_LOCALZONE && (!*e || isspace(*e)))
	{
		p = fixed.standard = fixed.daylight = off;
		*p++ = 'Z';
		if ((d = fixed.west) <= 0)
		{
			d = -d;
			*p++ = 'E';
		}
		else
			*p++ = 'W';
		p += sfsprintf(p, sizeof(off) - 2, "%u", d / 60);
		if (d = (d % 60) / 15)
			*p++ = 'A' + d - 1;
		*p = 0;
		fixed.dst = 0;
		if (end)
			*end = e;
		if (dst)
			*dst = 0;
		return &fixed;
	}
	else if ((name[0] == 'Z' || name[0] == 'Y') && (name[1] == 'E' || name[1] == 'W') && name[2] >= '0' && name[2] <= '9')
	{
		e = (char*)name + 2;
		fixed.west = 0;
		while (*e >= '0' && *e <= '9')
			fixed.west = fixed.west * 10 + (*e++ - '0');
		fixed.west *= 60;
		d = 0;
		switch (*e)
		{
		case 'C':
			d += 15;
			/*FALLTHROUGH*/
		case 'B':
			d += 15;
			/*FALLTHROUGH*/
		case 'A':
			d += 15;
			e++;
			break;
		}
		fixed.west += d;
		if (name[1] == 'E')
			fixed.west = -fixed.west;
		fixed.dst = name[0] == 'Z' ? 0 : d ? -d : TM_DST;
		memcpy(fixed.standard = fixed.daylight = off, name, d);
		off[d] = 0;
		if (end)
			*end = e;
		if (dst)
			*dst = 0;
		return &fixed;
	}
	zp = tm_info.local;
	p = 0;
	do
	{
		if (zp->type)
			p = zp->type;
		if (!type || type == p || !p)
		{
			if (tmword(name, end, zp->standard, NiL, 0))
			{
				if (dst)
					*dst = 0;
				return zp;
			}
			if (zp->dst && zp->daylight && tmword(name, end, zp->daylight, NiL, 0))
			{
				if (dst)
					*dst = zp->dst;
				return zp;
			}
		}
		if (zp == tm_info.local)
			zp = tm_data.zone;
		else
			zp++;
	} while (zp->standard);
	return 0;
}
