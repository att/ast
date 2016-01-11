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

#include "3d.h"

#ifdef ftruncate3d

#if !_nosys_ftruncate64 && _typ_off64_t

#undef	off_t
#undef	ftruncate

typedef int (*Real_f)(int, off64_t);

int
ftruncate64(int fd, off64_t size)
{
	static Real_f	realf;
	int		r;
	int		n;
	int		m;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_ftruncate, 0, fd, size))
		return state.ret;
	mp = monitored();
#endif
	if (!realf)
		realf = (Real_f)sysfunc(SYS3D_ftruncate64);
	for (m = state.trap.size - 1; m >= 0; m--)
		if (MSG_MASK(MSG_truncate) & state.trap.intercept[m].mask)
			break;
	if (m >= 0)
	{
		n = state.trap.size;
		state.trap.size = m;
		r = (*state.trap.intercept[m].call)(&state.trap.intercept[m], MSG_truncate, SYS3D_ftruncate64, (void*)fd, (void*)&size, NiL, NiL, NiL, NiL);
		state.trap.size = n;
	}
	else
		r = (*realf)(fd, size);
#if FS
	if (!r)
	{
		if (mp)
			fscall(mp, MSG_ftruncate, 0, fd, size);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_ftruncate))
				fscall(mp, MSG_ftruncate, 0, fd, size);
	}
#endif
	return r;
}

#endif

int
ftruncate3d(int fd, off_t size)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_ftruncate, 0, fd, size))
		return state.ret;
	mp = monitored();
#endif
	if (FTRUNCATE(fd, size))
		return -1;
#if FS
	if (mp)
		fscall(mp, MSG_ftruncate, 0, fd, size);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_ftruncate))
			fscall(mp, MSG_ftruncate, 0, fd, size);
#endif
	return 0;
}

#else

NoN(ftruncate)

#endif
