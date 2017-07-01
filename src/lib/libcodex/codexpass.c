/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
 * return password buf
 * return <  0 : error
 * return >= n : n-1 in buf
 */

#include <codex.h>

ssize_t
codexpass(void* buf, size_t n, Codexdisc_t* disc, Codexmeth_t* meth)
{
	char		prompt[2 * CODEX_NAME];

	if (disc->passphrase)
		return strncopy((char*)buf, disc->passphrase, n) - (char*)buf;
	if (disc->passf)
		return (*disc->passf)(buf, n, disc, meth);
	sfsprintf(prompt, sizeof(prompt), "Enter %s passphrase: ", meth->name);
	return codexgetpass(prompt, buf, n);
}
