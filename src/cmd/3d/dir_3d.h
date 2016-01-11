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
/*
 * AT&T Research
 *
 * directory stream access library private definitions
 * handles 3d file system
 */

#ifndef _DIR3D_H
#define _DIR3D_H

#include "3d.h"

#if !_mem_DIR
#undef	_lib_opendir
#endif

#define DIRENT_ILLEGAL_ACCESS	1
#define DIR			DIRDIR

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide opendir _opendir __opendir
__STDPP__directive pragma pp:hide readdir _readdir __readdir readdir64 _readdir64 __readdir64
__STDPP__directive pragma pp:hide rewinddir _rewinddir __rewinddir rewinddir64 _rewinddir64 __rewinddir64
__STDPP__directive pragma pp:hide seekdir _seekdir __seekdir seekdir64 _seekdir64 __seekdir64
__STDPP__directive pragma pp:hide telldir _telldir __telldir telldir64 _telldir64 __telldir64
__STDPP__directive pragma pp:hide closedir _closedir __closedir
__STDPP__directive pragma pp:hide _getdents __getdents getdents64 _getdents64 __getdents64 getdirentries
#else
#define opendir		DIRopendir
#define _opendir	_DIRopendir
#define __opendir	__DIRopendir
#define readdir		DIRreaddir
#define _readdir	_DIRreaddir
#define __readdir	__DIRreaddir
#define readdir64	DIRreaddir64
#define _readdir64	_DIRreaddir64
#define __readdir64	__DIRreaddir64
#if _lib_rewinddir
#define rewinddir	DIRrewinddir
#define _rewinddir	_DIRrewinddir
#define __rewinddir	__DIRrewinddir
#define rewinddir64	DIRrewinddir64
#define _rewinddir64	_DIRrewinddir64
#define __rewinddir64	__DIRrewinddir64
#endif
#define seekdir		DIRseekdir
#define _seekdir	_DIRseekdir
#define __seekdir	__DIRseekdir
#define seekdir64	DIRseekdir64
#define _seekdir64	_DIRseekdir64
#define __seekdir64	__DIRseekdir64
#if _lib_telldir
#define telldir		DIRtelldir
#define _telldir	_DIRtelldir
#define __telldir	__DIRtelldir
#define telldir64	DIRtelldir64
#define _telldir64	_DIRtelldir64
#define __telldir64	__DIRtelldir64
#endif
#define closedir	DIRclosedir
#define _closedir	_DIRclosedir
#define __closedir	__DIRclosedir
#define _getdents	_DIRgetdents
#define __getdents	__DIRgetdents
#define getdents64	DIRgetdents64
#define _getdents64	_DIRgetdents64
#define __getdents64	__DIRgetdents64
#define getdirentries	DIRgetdirentries
#endif

#define _BLD_ast	1
#include <ast_dir.h>
#undef	_BLD_ast

#if _lib_readdir64 && _typ_struct_dirent64
#undef	dirent
#undef	DIRdirent
#define DIRdirent	dirent
#endif

#include <hash.h>

#if !defined(SYS3D_opendir)
extern DIR*		OPENDIR(const char*);
#endif
#if !defined(SYS3D_readdir)
extern struct dirent*	READDIR(DIR*);
#endif
#if !defined(SYS3D_seekdir)
extern void		SEEKDIR(DIR*, long);
#endif
#if !defined(SYS3D_telldir)
extern long		TELLDIR(DIR*);
#endif
#if !defined(SYS3D_rewinddir)
extern void		REWINDDIR(DIR*);
#endif
#if !defined(SYS3D_closedir)
extern void		CLOSEDIR(DIR*);
#endif

#undef	DIR

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide opendir _opendir __opendir
__STDPP__directive pragma pp:nohide readdir _readdir __readdir readdir64 _readdir64 __readdir64
__STDPP__directive pragma pp:nohide rewinddir _rewinddir __rewinddir rewinddir64 _rewinddir64 __rewinddir64
__STDPP__directive pragma pp:nohide seekdir _seekdir __seekdir seekdir64 _seekdir64 __seekdir64
__STDPP__directive pragma pp:nohide telldir _telldir __telldir telldir64 _telldir64 __telldir64
__STDPP__directive pragma pp:nohide closedir _closedir __closedir
__STDPP__directive pragma pp:nohide _getdents __getdents getdents64 _getdents64 __getdents64 getdirentries
#else
#undef	opendir
#undef	_opendir
#undef	__opendir
#undef	readdir
#undef	_readdir
#undef	__readdir
#undef	readdir64
#undef	_readdir64
#undef	__readdir64
#ifndef REWINDDIR
#if _lib_rewinddir
#undef	rewinddir
#undef	_rewinddir
#undef	__rewinddir
#undef	rewinddir64
#undef	_rewinddir64
#undef	__rewinddir64
#else
#define REWINDDIR(p)	rewinddir(p)
#define REWINDDIR64(p)	rewinddir64(p)
#endif
#endif
#undef	seekdir
#undef	_seekdir
#undef	__seekdir
#undef	seekdir64
#undef	_seekdir64
#undef	__seekdir64
#ifndef TELLDIR
#if _lib_telldir
#undef	telldir
#undef	_telldir
#undef	__telldir
#undef	telldir64
#undef	_telldir64
#undef	__telldir64
#else
#define TELLDIR64(p)	telldir64(p)
#endif
#endif
#undef	closedir
#undef	_closedir
#undef	__closedir
#undef	_getdents
#undef	__getdents
#undef	getdents64
#undef	_getdents64
#undef	__getdents64
#undef	getdirentries
#endif

#if _mem_DIR
#define CHEATDIR(p)	((p)->dir = *(p)->viewp->dirp)
#else
#define CHEATDIR(p)
#endif

typedef struct
{
	DIRDIR*		dirp;		/* system stream pointer	*/
	ino_t		opaque;		/* opaque inode number		*/
	int		level;		/* view level			*/
} Dir_physical_t;

typedef struct
{
	int		fd;		/* placeholder for dirfd()	*/
#if _mem_DIR
	DIRDIR		dir;		/* in case user checks *DIRDIR	*/
#endif
	Dir_physical_t*	viewp;		/* current directory in view	*/
	Dir_physical_t	view[TABSIZE];	/* dirp's in view		*/
	Hash_table_t*	overlay;	/* directory overlay hash	*/
	int		boundary;	/* return . at each view level	*/
} DIR;					/* directory stream descriptor	*/

extern int		closedir(DIR*);
extern DIR*		opendir(const char*);
#if _HUH_2008_11_21
extern struct dirent*	readdir(DIR*);
#endif
#if _lib_rewinddir && !defined(rewinddir)
extern void		rewinddir(DIR*);
#endif
extern void		seekdir(DIR*, long);
#if _lib_telldir && !defined(telldir)
extern long		telldir(DIR*);
#endif

#endif
