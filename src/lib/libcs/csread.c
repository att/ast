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
 * read at most size chars according to op
 *
 *	CS_EXACT	read exactly size chars
 *			multiple reads possible
 *	CS_LIMIT	one read up to size chars
 *	CS_LINE		'\n' '\r\n' '\n\r' terminated line up to size
 *			multiple reads possible
 *	CS_RESTART	restart on interrupt
 *
 * 0 returned on eof
 */

#include "cslib.h"

ssize_t
csread(Cs_t* state, int fd, void* buf, size_t size, int op)
{
	register ssize_t	n;
	register char*		p;
	register char*		e;
	int			restart;

	if (restart = !!(op & CS_RESTART))
		op &= ~CS_RESTART;
	p = (char*)buf;
	e = p + size;
	messagef((state->id, NiL, -9, "read(%d,%d,%s) before", fd, size, op == CS_EXACT ? "EXACT" : op == CS_LIMIT ? "LIMIT" : "LINE"));
	if (op == CS_LINE && size > 1)
	{
		if ((n = cspeek(state, fd, buf, size - 1)) > 0 && (p[n] = 0, e = strchr(p, '\n')))
		{
			e++;
			op = CS_EXACT;
		}
		else
		{
			while (p < e && (n = read(fd, p, 1)) == 1 && *p++ != '\n');
			if (n <= 0 || p == (char*)buf || *(p - 1) != '\n')
				goto bad;
			n = p - (char*)buf;
			messagef((state->id, NiL, -9, "read(%d,%d,%s) [%d] `%-.*s'", fd, size, op == CS_EXACT ? "EXACT" : op == CS_LIMIT ? "LIMIT" : "LINE", n, n, buf));
			return n;
		}
	}
	while (n = read(fd, p, e - p))
	{
		if (n < 0)
		{
			if (restart && errno == EINTR)
				continue;
			break;
		}
		messagef((state->id, NiL, -9, "read(%d,%d,%s) [%d] `%-.*s'", fd, size, op == CS_EXACT ? "EXACT" : op == CS_LIMIT ? "LIMIT" : "LINE", n, n, p));
		p += n;
		if (op == CS_LIMIT || op == CS_LINE && (*(p - 1) == '\n' || n > 1 && *(p - 1) == '\r' && *(p - 2) == '\n'))
			return p - (char*)buf;
		if (p >= e)
		{
			if (op == CS_EXACT)
				return p - (char*)buf;
			break;
		}
		messagef((state->id, NiL, -2, "read(%d,%d,%s) again [%d] `%-.*s'", fd, size, op == CS_EXACT ? "EXACT" : op == CS_LIMIT ? "LIMIT" : "LINE", p - (char*)buf, p - (char*)buf, (char*)buf));
	}
 bad:
	if (p != (char*)buf || n < 0)
	{
		errno = EINVAL;
		messagef((state->id, NiL, -2, "read(%d,%d,%s) invalid record [%d] `%-.*s'", fd, size, op == CS_EXACT ? "EXACT" : op == CS_LIMIT ? "LIMIT" : "LINE", p - (char*)buf, p - (char*)buf, (char*)buf));
		return -1;
	}
	messagef((state->id, NiL, -2, "read(%d,%d,%s) eof", fd, size, op == CS_EXACT ? "EXACT" : op == CS_LIMIT ? "LIMIT" : "LINE"));
	return 0;
}

ssize_t
_cs_read(int fd, void* buf, size_t size, int op)
{
	return csread(&cs, fd, buf, size, op);
}
