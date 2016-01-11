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
__STDPP__directive pragma pp:hide statfs
#else
#define statfs        ______statfs
#endif
#define _def_syscall_3d	1

#include "3d.h"

#if defined(statfs3d) && ( _sys_statfs || _sys_vfs || _sys_mount && _mem_f_files_statfs )

#include <cs_lib.h>

#if _sys_statfs
#include <sys/statfs.h>
#else
#if _sys_vfs
#include <sys/vfs.h>
#define _vfs_statfs	1
#else
#if _sys_mount
#if _lib_getmntinfo
#include <sys/param.h>		/* expect some macro redefinitions here */
#endif
#include <sys/mount.h>
#endif
#endif
#endif

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide statfs
#else
#undef  statfs
#endif

#include "FEATURE/syscall"

#if _vfs_statfs
#define VFS	(fs)
#else
#define VFS	(&vfs)
#endif

#if ARG3D_statfs == 4

int
statfs3d(const char* path, struct statfs* fs, int size, int type)
{
	register char*	sp;
#if FS
	Mount_t*	mp;
#if !_vfs_statfs
	struct statvfs	vfs;
#endif

	if (!fscall(NiL, MSG_statfs, 0, path, VFS, size, type))
	{
#if !_vfs_statfs
		if (!state.ret)
		{
			memset(fs, 0, sizeof(*fs));
			fs->f_bsize = vfs.f_bsize;
			fs->f_blocks = vfs.f_blocks;
			fs->f_bfree = vfs.f_bfree;
			fs->f_files = vfs.f_files;
			fs->f_ffree = vfs.f_ffree;
		}
#endif
		return state.ret;
	}
	mp = monitored();
#endif
	if (!(sp = pathreal(path, 0, NiL)))
		return -1;
	if (STATFS(sp, fs, size, type))
		return -1;
#if FS
#if !_vfs_statfs
	if (mp || state.global)
	{
		memset(&vfs, 0, sizeof(vfs));
		vfs.f_bsize = fs->f_bsize;
		vfs.f_blocks = fs->f_blocks;
		vfs.f_bfree = fs->f_bfree;
		vfs.f_files = fs->f_files;
		vfs.f_ffree = fs->f_ffree;
	}
#endif
	if (mp)
		fscall(mp, MSG_statfs, 0, state.path.name, VFS, size, type);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_statfs))
			fscall(mp, MSG_statfs, 0, state.path.name, VFS, size, type);
#endif
	return 0;
}

#else

int
statfs3d(const char* path, struct statfs* fs)
{
	register char*	sp;
#if FS
	Mount_t*	mp;
#if !_vfs_statfs
	struct statvfs	vfs;
#endif

	if (!fscall(NiL, MSG_statfs, 0, path, VFS))
	{
#if !_vfs_statfs
		if (!state.ret)
		{
			memset(fs, 0, sizeof(*fs));
			fs->f_bsize = vfs.f_bsize;
			fs->f_blocks = vfs.f_blocks;
			fs->f_bfree = vfs.f_bfree;
			fs->f_files = vfs.f_files;
			fs->f_ffree = vfs.f_ffree;
		}
#endif
		return state.ret;
	}
	mp = monitored();
#endif
	if (!(sp = pathreal(path, 0, NiL)))
		return -1;
	if (STATFS(sp, fs))
		return -1;
#if FS
#if !_vfs_statfs
	if (mp || state.global)
	{
		memset(&vfs, 0, sizeof(vfs));
		vfs.f_bsize = fs->f_bsize;
		vfs.f_blocks = fs->f_blocks;
		vfs.f_bfree = fs->f_bfree;
		vfs.f_files = fs->f_files;
		vfs.f_ffree = fs->f_ffree;
	}
#endif
	if (mp)
		fscall(mp, MSG_statfs, 0, state.path.name, VFS);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_statfs))
			fscall(mp, MSG_statfs, 0, state.path.name, VFS);
#endif
	return 0;
}

#endif

#else

NoN(statfs)

#endif
