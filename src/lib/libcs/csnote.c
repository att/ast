/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * note host status change for name
 * this is the daemon side of csstat()
 *
 * NOTE: name must already exist and pwd must be CS_STAT_DIR
 */

#include "cslib.h"

/*
 * encode 4 byte int into 2 bytes
 */

static unsigned short
encode(register unsigned long n)
{
	register int	e;

	e = 0;
	while (n > 03777)
	{
		n >>= 1;
		e++;
	}
	return n | (e << 11);
}

int
csnote(register Cs_t* state, const char* name, register Csstat_t* sp)
{
	unsigned long	idle;
	long		up;

	if (sp->up < 0)
	{
		idle = -sp->up;
		up = 0;
	}
	else
	{
		idle = sp->idle;
		up = sp->up;
	}
#if 0
	{
		unsigned long	a;
		unsigned long	m;

		a = (encode(up) << 16) | encode(idle);
		m = (((sp->load >> 3) & 0377) << 24) | ((sp->pctsys & 0377) << 16) | ((sp->pctusr & 0377) << 8) | (sp->users & 0377);
		error(-1, "csnote: <%lu,%lu> load=%lu:%lu pctsys=%lu:%lu pctusr=%lu:%lu users=%lu:%lu idle=%lu:%lu", sp->load, ((m >> 24) & 0xff) << 3, sp->pctsys, (m >> 16) & 0xff, sp->pctusr, (m >> 8) & 0xff, sp->users, m & 0xff, sp->idle, sp->users ? ((a & 0x7ff) << ((a >> 11) & 0x1f)) : ~0, a, m);
	}
#endif
	return touch(name, (encode(up) << 16) | encode(idle), (((sp->load >> 3) & 0377) << 24) | ((sp->pctsys & 0377) << 16) | ((sp->pctusr & 0377) << 8) | (sp->users & 0377), -1);
}

int
_cs_note(const char* name, Csstat_t* sp)
{
	return csnote(&cs, name, sp);
}
