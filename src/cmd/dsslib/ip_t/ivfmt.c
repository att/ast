/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2012 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "ivlib.h"

#include <ip6.h>

/*
 * convert address or prefix to string
 * return value in a short-lived buffer
 */

char*
ivfmt(Iv_t* iv, const unsigned char* addr, int bits)
{
	char*	b;
	char*	s;
	char*	e;
	int	i;
	int	n;

	if (iv->size == 16)
		return fmtip6(addr, bits);
	n = (iv->size + (bits >= 0)) * 4 + 1;
	s = b = fmtbuf(n);
	e = b + n;
	s += sfsprintf(s, e - s, "%d", addr[0]);
	for (i = 1; i < iv->size; i++)
		s += sfsprintf(s, e - s, ".%d", addr[i]);
	if (bits >= 0 && bits <= iv->size * 8)
		sfsprintf(s, e - s, "/%d", bits);
	return b;
}
