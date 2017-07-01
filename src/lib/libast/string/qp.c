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
 * quoted-printable encode/decode
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <ast.h>
#include <ctype.h>

#include "sfhdr.h"

static const char	hex[] = "0123456789ABCDEFabcdef";

/*
 * qp encode
 */

ssize_t
qpencode(const void* fb, size_t fz, void** fn, void* tb, size_t tz, void** tn)
{
	register unsigned char*	fp;
	register unsigned char*	tp;
	register unsigned char*	fe;
	register unsigned char*	te;
	register int		c;

	fp = (unsigned char*)fb;
	fe = fp + fz;
	tp = (unsigned char*)tb;
	te = tp + tz;
	while (fp < fe)
	{
		if (((c = *fp) != ' ' || !(c = '_')) && (isspace(c) || !isprint(c) || c == '=' || c == '?' || c == '_'))
		{
			if ((te - tp) < 3)
				break;
			if (tb)
			{
				*tp++ = '=';
				*tp++ = hex[c >> 4];
				*tp++ = hex[c & 0x0f];
			}
			else
				tp += 3;
		}
		else if ((te - tp) < 1)
			break;
		else if (tb)
			*tp++ = c;
		else
			tp++;
		fp++;
	}
	if (tn)
		*tn = tp;
	if (fn)
		*fn = fp;
	return tp - (unsigned char*)tb;
}

/*
 * qp decode
 */

ssize_t
qpdecode(const void* fb, size_t fz, void** fn, void* tb, size_t tz, void** tn)
{
	register unsigned char*	fp;
	register unsigned char*	tp;
	register unsigned char*	fe;
	register unsigned char*	te;
	register int		c;

	SFCVINIT();
	fp = (unsigned char*)fb;
	fe = fp + fz;
	tp = (unsigned char*)tb;
	te = tp + tz;
	while (fp < fe && tp < te)
	{
		if ((c = *fp++) == '=')
		{
			if ((fe - fp) < 2)
			{
				fp--;
				break;
			}
			c = _Sfcv36[*fp++] << 4;
			c |= _Sfcv36[*fp++];
		}
		else if (c == '_')
			c = ' ';
		*tp++ = c;
	}
	if (tn)
		*tn = tp;
	if (fn)
		*fn = fp;
	return tp - (unsigned char*)tb;
}
