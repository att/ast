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
__STDPP__directive pragma pp:hide lseek
#else
#define lseek        ______lseek
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide lseek
#else
#undef  lseek
#endif

#include "FEATURE/syscall"

#ifdef	lseek3d

#if !_nosys_lseek64 && _typ_off64_t

typedef off64_t (*Seek64_f)(int, off64_t, int);

#undef	off_t
#undef	lseek

off64_t
lseek64(int fd, off64_t off, int op)
{
	off64_t		r;
	int		n;
	int		m;

	static Seek64_f	seekf;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_seek, 0, fd, off, op))
		return state.ret;
	mp = monitored();
#endif
	if (!seekf)
		seekf = (Seek64_f)sysfunc(SYS3D_lseek64);
	for (m = state.trap.size - 1; m >= 0; m--)
		if (MSG_MASK(MSG_seek) & state.trap.intercept[m].mask)
			break;
	if (m >= 0)
	{
		n = state.trap.size;
		state.trap.size = m;
		r = (*state.trap.intercept[m].call)(&state.trap.intercept[m], MSG_seek, SYS3D_lseek64, (void*)fd, (void*)&off, (void*)op, NiL, NiL, NiL) ? -1 : off;
		state.trap.size = n;
	}
	else
		r = (*seekf)(fd, off, op);
#if FS
	if (r != (off64_t)(-1))
	{
		if (mp)
			fscall(mp, MSG_seek, r, fd, off, op);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_seek))
				fscall(mp, MSG_seek, r, fd, off, op);
	}
#endif
	return r;
}

#endif

typedef off_t (*Seek_f)(int, off_t, int);

off_t
lseek3d(int fd, off_t off, int op)
{
	off_t		r;

	static Seek_f	seekf;

#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_seek, 0, fd, off, op))
		return state.ret;
	mp = monitored();
#endif
	if (sizeof(off_t) > sizeof(long))
	{
		if (!seekf)
			seekf = (Seek_f)sysfunc(SYS3D_lseek);
		r = (*seekf)(fd, off, op);
	}
	else if ((r = LSEEK(fd, off, op)) == -1)
		return -1;
#if FS
	if (mp)
		fscall(mp, MSG_seek, r, fd, off, op);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_seek))
			fscall(mp, MSG_seek, r, fd, off, op);
#endif
	return r;
}

#else

NoN(lseek)

#endif
