/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
 * access() euid/egid implementation
 */

#include <ast.h>
#include <errno.h>
#include <ls.h>

#include "FEATURE/eaccess"

#if _lib_eaccess

NoN(eaccess)

#else

#undef	eaccess

#undef	_def_map_ast

#include <ast_map.h>

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int
eaccess(const char* path, register int flags)
{
#if _lib_faccessat && defined(AT_FDCWD) && defined(AT_EACCESS)
	return faccessat(AT_FDCWD, path, flags, AT_EACCESS);
#elif defined(EFF_ONLY_OK)
	return access(path, flags|EFF_ONLY_OK);
#elif _lib_euidaccess
	return euidaccess(path, flags);
#else
	register int	mode;
	struct stat	st;

	static int	init = 0;
	static uid_t	ruid;
	static uid_t	euid;
	static gid_t	rgid;
	static gid_t	egid;

	if (!init)
	{
		ruid = getuid();
		euid = geteuid();
		rgid = getgid();
		egid = getegid();
		init = (ruid == euid && rgid == egid) ? 1 : -1;
	}
	if (init > 0 || flags == F_OK)
		return access(path, flags);
	if (stat(path, &st))
		return -1;
	mode = 0;
	if (euid == 0)
	{
		if (!S_ISREG(st.st_mode) || !(flags & X_OK) || (st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)))
			return 0;
		goto nope;
	}
	else if (euid == st.st_uid)
	{
		if (flags & R_OK)
			mode |= S_IRUSR;
		if (flags & W_OK)
			mode |= S_IWUSR;
		if (flags & X_OK)
			mode |= S_IXUSR;
	}
	else if (egid == st.st_gid)
	{
#if _lib_getgroups
	setgroup:
#endif
		if (flags & R_OK)
			mode |= S_IRGRP;
		if (flags & W_OK)
			mode |= S_IWGRP;
		if (flags & X_OK)
			mode |= S_IXGRP;
	}
	else
	{
#if _lib_getgroups
		register int	n;

		static int	ngroups = -2;
		static gid_t*	groups; 

		if (ngroups == -2)
		{
			if ((ngroups = getgroups(0, (gid_t*)0)) <= 0)
			{
#if defined(NGROUPS_MAX)
				ngroups = NGROUPS_MAX;
#elif defined(_SC_NGROUPS_MAX)
				ngroups = (int)sysconf(_SC_NGROUPS_MAX);
#elif defined(_POSIX_NGROUPS_MAX)
				ngroups = _POSIX_NGROUPS_MAX;
#else
				ngroups = 32;
#endif
			}
			if (!(groups = newof(0, gid_t, ngroups + 1, 0)))
				ngroups = -1;
			else
				ngroups = getgroups(ngroups, groups);
		}
		n = ngroups;
		while (--n >= 0)
			if (groups[n] == st.st_gid)
				goto setgroup;
#endif
		if (flags & R_OK)
			mode |= S_IROTH;
		if (flags & W_OK)
			mode |= S_IWOTH;
		if (flags & X_OK)
			mode |= S_IXOTH;
	}
	if ((st.st_mode & mode) == mode)
		return 0;
 nope:
	errno = EACCES;
	return -1;
#endif
}

#endif
