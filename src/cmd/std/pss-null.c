/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * process status stream PSS_METHOD_null implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_null

NoN(pss_null)

#else

static int
null_init(register Pss_t* pss)
{
	return 0;
}

static Pssmeth_t null_method =
{
	"null",
	"[-version?@(#)$Id: pss null (AT&T Research) 2003-02-01 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	0,
	null_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

Pssmeth_t*	_pss_method = &null_method;

#endif
