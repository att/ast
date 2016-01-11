/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * sha*sum -- implemented by cksum
 */

#include <cmd.h>

int
b_sha1sum(int argc, register char** argv, Shbltin_t* context)
{
	return b_cksum(argc, argv, context);
}

int
b_sha2sum(int argc, register char** argv, Shbltin_t* context)
{
	return b_cksum(argc, argv, context);
}

int
b_sha256sum(int argc, register char** argv, Shbltin_t* context)
{
	return b_cksum(argc, argv, context);
}

int
b_sha384sum(int argc, register char** argv, Shbltin_t* context)
{
	return b_cksum(argc, argv, context);
}

int
b_sha512sum(int argc, register char** argv, Shbltin_t* context)
{
	return b_cksum(argc, argv, context);
}
