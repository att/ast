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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * return null io stream for pure codexdata()
 */

#include <sfio_t.h>
#include <codex.h>

static ssize_t
nullread(Sfio_t* f, void* buf, size_t n, Sfdisc_t* disc)
{
	return 0;
}

static ssize_t
nullwrite(Sfio_t* f, const void* buf, size_t n, Sfdisc_t* disc)
{
	return n;
}

static Sfdisc_t		nulldisc = { nullread, nullwrite };

static const char	nullbuf[1];

static Sfio_t		null = SFNEW(nullbuf, 0, 0, SF_WRITE, &nulldisc, 0);

Sfio_t*
codexnull(void)
{
	return &null;
}
