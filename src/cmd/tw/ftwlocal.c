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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "ftwlocal.h"

#include <ls.h>

/*
 * return 1 if ftw is mounted on a local filesystem
 */

int
ftwlocal(Ftw_t* ftw)
{
#ifdef ST_LOCAL
	struct statvfs	fs;

	return statvfs(ftw->path, &fs) || (fs.f_flag & ST_LOCAL);
#else
	return !strgrpmatch(fmtfs(&ftw->statb), "([an]fs|samb)", NiL, 0, STR_LEFT|STR_ICASE);
#endif
}
