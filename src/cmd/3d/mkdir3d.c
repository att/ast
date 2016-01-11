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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide mkdir
#else
#define mkdir        ______mkdir
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide mkdir
#else
#undef  mkdir
#endif

#include "FEATURE/syscall"

/*
 * mkdir() will always create a directory on the highest layer
 * mkdir() will create intermediate directories if they virtually exist
 * mkdir() will do a chdir() if a virtual dot directory is created
 */

int
mkdir3d(const char* path, mode_t mode)
{
	register char*	sp;
	register char*	cp;
	register int	r;
	char		buf[PATH_MAX + 1];
	char		tmp[PATH_MAX + 1];
#if FS
	Mount_t*	mp;
#endif

	if (state.real)
	{
#if FS
		mp = 0;
#endif
		sp = (char*)path;
	}
#if FS
	else if (!fscall(NiL, MSG_mkdir, 0, path, mode))
		return(state.ret);
#endif
	else
	{
#if FS
		mp = monitored();
#endif
		if (!(sp = pathreal(path, P_PATHONLY|P_NOOPAQUE, NiL)))
			return(-1);
	}
	if (state.path.level || state.level && (sp == state.pwd || streq(sp, state.pwd)) || (r = MKDIR(sp, mode)) && errno == ENOENT)
	{
		/*
		 * copy canonicalized pathname into buf
		 */

		if (*sp != '/')
			sp = state.path.name;
		for (cp = buf; *cp = *sp++; cp++);
		if (!state.real)
		{
			/*
			 * create intermediate dirs if they exist in lower view
			 */

			char*	a;
			char*	b;
			Path_t	save;
			int	oerrno;

			a = tmp;
			b = buf;
			while (*a++ = *b++);
			if (b = strrchr(tmp, '/'))
				*b = 0;
			oerrno = errno;
			save = state.path;
			for (;;)
				if (!pathnext(tmp, NiL, NiL) || ACCESS(tmp, F_OK))
				{
					state.path = save;
					errno = oerrno;
					return -1;
				}
			state.path = save;
			errno = oerrno;
		}

		/*
		 * ok to create intermediate dirs
		 */

		do
		{
			while (*--cp != '/');
			if (cp <= buf) return(-1);
			*cp = 0;
			r = !pathreal(buf, 0, NiL) || !streq(buf, state.path.name);
			*cp = '/';
		} while (r);
		*cp = '/';
		do
		{
			r = *++cp;
			if (r == 0 || r == '/')
			{
				*cp = 0;
				sp = pathreal(buf, P_PATHONLY, NiL);
				*cp = r;
				if (!sp) return(-1);
				if (sp == state.dot) sp = state.pwd;
				if (r = MKDIR(sp, r ? (mode|S_IWUSR|S_IXUSR) : mode)) return(r);
#if FS
				if (fsmonitored(state.path.name) && !LSTAT(state.path.name, &state.path.st))
					fscall(state.path.monitor, MSG_mkdir, 0, state.path.name, state.path.st.st_mode);
#endif
				if (sp == state.pwd || streq(sp, state.pwd))
				{
					state.level = 0;
					CHDIR(state.pwd);
				}
			}
		} while (*cp);
	}
#if FS
	if (!r)
	{
		if (mp) fscall(mp, MSG_mkdir, 0, path, mode);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_mkdir))
				fscall(mp, MSG_mkdir, 0, path, mode);
	}
#endif
	return(r);
}

/*
 * 3d internal mkdir to skip pathreal(path, ...)
 */

int
fs3d_mkdir(const char* path, mode_t mode)
{
	int	r;

	state.real++;
	r = mkdir(path, mode);
	state.real--;
	return(r);
}
