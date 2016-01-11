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

#if !defined(ACL3D) || !defined(SETACL)

#define aclent_t	char

#endif

#define MSG_facl	MSG_INIT(MSG_control, 051131, MSG_VAR_FILE)

int
facl3d(int fd, int cmd, int cnt, aclent_t* buf)
{
	register int	r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_facl, 0, fd, cmd, cnt, buf))
		return(state.ret);
	mp = monitored();
#endif
	r = FACL(fd, cmd, cnt, buf);
#if FS
	if (r >= 0)
	{
		if (mp)
			fscall(mp, MSG_facl, 0, fd, cmd, cnt, buf);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_facl))
				fscall(mp, MSG_facl, 0, fd, cmd, cnt, buf);
	}
#endif
	return(r);
}
