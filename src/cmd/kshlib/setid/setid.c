/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*            Copyright (c) 2013 AT&T Intellectual Property             *
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

#include <ast.h>
#include <cmd.h>
#include <error.h>

static const char usage[] =
"[-?@(#)$Id: setid (AT&T Research) 2013-07-31 $\n]"
USAGE_LICENSE
"[+NAME?setid - set real and or effectve user and or group ids of the "
    "current process]"
"[+DESCRIPTION?\bsetid\b sets real and or effective user and or group "
    "ids of the current process. The call must have the appropriate "
    "privelege: for uids this means the call is root or running in setuid "
    "mode; for gids this means the caller is root or the requested gid is "
    "either the caller's main gid or is one of the caller's supplementary "
    "groups.]"
"[g:group|gid?Sets the real and effective group ids to \agid\a.]:[gid]"
"[G:effective-group|effective-gid?Sets the effective group id to "
    "\agid\a. If \b--group\b is also specified then it controls the group id "
    "and the effective group id is set to \agid\a.]:[gid]"
"[u:user|uid?Sets the real and effective user ids to \auid\a.]:[uid]"
"[U:effective-user|effective-uid?Sets the effective user id to \auid\a. "
    "If \b--user\b is also specified then it controls the user id and the "
    "effective user id is set to \auid\a.]:[uid]"
"\n"
"\n\n"
"\n"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
    "}"
"[+SEE ALSO?\bnewgrp\b(1), \bsetreuid\b(2), \bsetregid\b(2)]"
;

extern int
b_setid(int argc, char** argv, Shbltin_t* context)
{
	char*	rg;
	char*	eg;
	char*	ru;
	char*	eu;

	gid_t	rgid = -1;
	gid_t	egid = -1;
	uid_t	ruid = -1;
	uid_t	euid = -1;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
	rg = eg = ru = eu = "-";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'g':
			rg = opt_info.arg;
			if ((rgid = strgid(rg)) < 0)
				error(2, "%s: unknown group", rg);
			continue;
		case 'G':
			eg = opt_info.arg;
			if ((egid = strgid(eg)) < 0)
				error(2, "%s: unknown group", eg);
			continue;
		case 'u':
			ru = opt_info.arg;
			if ((ruid = struid(ru)) < 0)
				error(2, "%s: unknown user", ru);
			continue;
		case 'U':
			eu = opt_info.arg;
			if ((euid = struid(eu)) < 0)
				error(2, "%s: unknown user", eu);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || *argv)
	{
		error(ERROR_USAGE|2, "%s", optusage(NiL));
		return 2;
	}
#if _lib_setregid
	if ((rgid >= 0 || egid >= 0) && setregid(rgid, egid))
		error(ERROR_SYSTEM|2, "setregid(%s,%s) failed", rg, eg);
#else
	if (rgid >= 0 && setgid(rgid))
		error(ERROR_SYSTEM|2, "setgid(%s) failed", rg);
	if (egid >= 0 && setegid(egid))
		error(ERROR_SYSTEM|2, "setegid(%s) failed", eg);
#endif
#if _lib_setreuid
	if ((ruid >= 0 || euid >= 0) && setreuid(ruid, euid))
		error(ERROR_SYSTEM|2, "setreuid(%s,%s) failed", ru, eu);
#else
	if (ruid >= 0 && setuid(ruid))
		error(ERROR_SYSTEM|2, "setuid(%s) failed", ru);
	if (euid >= 0 && seteuid(euid))
		error(ERROR_SYSTEM|2, "seteuid(%s) failed", eu);
#endif
	return error_info.errors != 0;
}

SHLIB(setid)
