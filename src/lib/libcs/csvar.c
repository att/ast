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
 * run time cs var support
 */

#include "cslib.h"

#ifndef TRUST
#define TRUST		"/usr/local:/usr" /* trusted root dirs		*/
#endif

typedef struct
{
	const char*	name;
	const char*	trust;
	const char*	value;
} Var_t;

static Var_t	var[] =
{

{ /* CS_VAR_LOCAL */ "CS_MOUNT_LOCAL", "/tmp/cs"			},
{ /* CS_VAR_PROXY */ "CS_MOUNT_PROXY", "/dev/tcp/proxy/inet.proxy"	},
{ /* CS_VAR_SHARE */ "CS_MOUNT_SHARE", "share/lib/cs"			},
{ /* CS_VAR_TRUST */ "CS_MOUNT_TRUST", TRUST				},

};

char*
csvar(Cs_t* state, int index, int trust)
{
	register char*	s;
	register Var_t*	p;

	if (index < 0 || index >= elementsof(var))
		return 0;
	p = &var[index];
	if (!p->name || trust)
		return (char*)p->trust;
	if (!p->value)
		p->value = ((s = getenv(p->name)) && *s) ? (const char*)s : p->trust;
	return (char*)p->value;
}

char*
_cs_var(int index, int trust)
{
	return csvar(&cs, index, trust);
}
