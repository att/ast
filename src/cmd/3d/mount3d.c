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

int
mount3d(const char* aspc, char* path, int mode, void* a4, void* a5, void* a6)
{
	char*			spc = (char*)aspc;
	register Fs_t*		fs;
	register char*		sp;
	register int		r;
	Table_t*		tab;
	int			size = 0;
	int			oerrno;
	unsigned long		x;

	if ((!spc || !*spc) && (!path || !*path) && !mode) return(0);
	if (mode & FS3D_ALL)
	{
		initialize();
		switch (mode & FS3D_ALL)
		{
		case FS3D_VERSION:
			tab = &state.vmap;
			break;
		case FS3D_VIEW:
			tab = &state.vpath;
			break;
		default:
			tab = 0;
			break;
		}
		if (mode & FS3D_GET)
		{
			size = FS3D_SIZEOF(mode);
			if ((sp = spc) && (!*sp || *sp == '-' && !*(sp + 1) || *sp == '/' && *(sp + 1) == '#'))
			{
				r = -1;
				oerrno = errno;
				errno = 0;
				if (*sp && *sp++ == '/') sp++;
				if (!*sp)
				{
					if ((r = mapdump(NiL, NiL, 0)) >= 0 && r < size)
					{
						mapdump(NiL, path, 0);
						r = 0;
					}
				}
				else
				{
					x = getkey(sp, sp + strlen(sp), 0);
					for (fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
						if (x == fs->key)
						{
							if (fs->get && (r = (*fs->get)(fs, NiL, sp = state.key.next, 0)) >= 0 && r < size)
							{
								if ((r = (*fs->get)(fs, path, sp, 0)) > 0 && path[r - 1] == ' ') r--;
								path[r] = 0;
								r = 0;
							}
							break;
						}
				}
				if (r >= 0) errno = oerrno;
				else if (!errno) errno = ENODEV;
			}
			else if (!spc)
			{
				if ((r = mapdump(tab, NiL, 0)) >= 0 && r < size)
				{
					mapdump(tab, path, 0);
					r = 0;
				}
			}
			else if (tab == &state.vmap)
			{
				register Map_t*	map;

				if (!(pathreal(spc, P_PATHONLY|P_ABSOLUTE|P_NOSLASH, NiL)))
					r = -1;
				else if (!(map = search(tab, state.path.name, strlen(state.path.name), NiL, 0)))
				{
					if ((r = 0) < size)
						path[r] = 0;
				}
				else if ((r = T_VALSIZE(map)) < size)
				{
					strncpy(path, map->val, r);
					path[r] = 0;
					r = 0;
				}
			}
			else if (!pathreal(spc, P_LSTAT, NiL)) r = -1;
#if 1
			else
			{
				r = strlen(state.path.name);
				if (r > 1)
				{
					if (state.path.name[r - 1] == '.' && state.path.name[r - 2] == '/')
					{
						if (!(r -= 2))
							r = 1;
						state.path.name[r] = 0;
					}
				}
				if (++r <= size)
				{
					strcpy(path, state.path.name);
					r = 0;
				}
			}
#else
			else if ((r = strlen(state.path.name)) < size)
			{
				strcpy(path, state.path.name);
				r = 0;
			}
#endif
			return(r);
		}
		else if (tab && spc) return(mapset(tab, path, 0, spc, 0));
	}
	return(MOUNT(spc, path ? pathreal(path, 0, NiL) : (char*)0, mode, a4, a5, a6));
}
