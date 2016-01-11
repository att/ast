/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * at interface definitions
 */

#ifndef _AT_H
#define _AT_H

#include <ast.h>
#include <coshell.h>
#include <css.h>
#include <ctype.h>
#include <error.h>
#include <tm.h>

#ifndef AT_STRICT
#define AT_STRICT	3
#endif

#if _WINIX

#undef	csstat
#define csstat(s,h,p)	(memset(p,0,sizeof(*(p))),0)

#define AT_DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IXGRP|S_IXOTH|S_IRGRP|S_IROTH)
#define AT_JOB_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define AT_RUN_MODE	(AT_JOB_MODE|S_IXUSR)

#if AT_STRICT > 1

#define AT_EXEC_OK(d,x)	(S_ISREG((x)->st_mode)&&((x)->st_mode&(S_IXUSR|S_IXGRP|S_IWOTH|S_IXOTH))==(S_IXUSR|S_IXGRP|S_IXOTH))
#define AT_DIR_OK(d)	(S_ISDIR((d)->st_mode)&&(((d)->st_mode)&S_IPERM)==AT_DIR_MODE)
#define AT_JOB_OK(d,j)	((d)->st_uid==(j)->st_uid&&S_ISREG((j)->st_mode)&&(((j)->st_mode)&S_IPERM)==AT_JOB_MODE)
#define AT_OLD_OK(d,j)	((d)->st_uid==(j)->st_uid&&S_ISREG((j)->st_mode)&&(((j)->st_mode)&(S_IPERM&~(AT_JOB_MODE^AT_RUN_MODE)))==AT_JOB_MODE)
#define AT_RUN_OK(d,j)	((d)->st_uid==(j)->st_uid&&S_ISREG((j)->st_mode)&&(((j)->st_mode)&S_IPERM)==AT_RUN_MODE&&(unsigned long)(j)->st_mtime<=NOW)

#else

#define AT_EXEC_OK(d,x)	(S_ISREG((x)->st_mode))
#define AT_DIR_OK(d)	(S_ISDIR((d)->st_mode))
#define AT_JOB_OK(d,j)	(S_ISREG((j)->st_mode)&&(d)->st_uid==(j)->st_uid)
#define AT_OLD_OK(d,j)	(AT_JOB_OK(d,j))
#define AT_RUN_OK(d,j)	(AT_JOB_OK(d,j)&&(unsigned long)(j)->st_mtime<=NOW)

#endif

#else

#define AT_DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IXGRP|S_IXOTH)
#define AT_JOB_MODE	(S_IRUSR|S_IWUSR)
#define AT_RUN_MODE	(AT_JOB_MODE|S_IXUSR)

#define AT_EXEC_OK(d,x)	(S_ISREG((x)->st_mode)&&((x)->st_mode&(S_IXUSR|S_IWGRP|S_IXGRP|S_IWOTH|S_IXOTH))==(S_IXUSR|S_IXGRP|S_IXOTH))
#define AT_DIR_OK(d)	(S_ISDIR((d)->st_mode)&&(((d)->st_mode)&S_IPERM)==AT_DIR_MODE)
#define AT_JOB_OK(d,j)	((d)->st_uid==(j)->st_uid&&S_ISREG((j)->st_mode)&&(((j)->st_mode)&S_IPERM)==AT_JOB_MODE)
#define AT_OLD_OK(d,j)	((d)->st_uid==(j)->st_uid&&S_ISREG((j)->st_mode)&&(((j)->st_mode)&(S_IPERM&~(AT_JOB_MODE^AT_RUN_MODE)))==AT_JOB_MODE)
#define AT_RUN_OK(d,j)	((d)->st_uid==(j)->st_uid&&S_ISREG((j)->st_mode)&&(((j)->st_mode)&S_IPERM)==AT_RUN_MODE&&(unsigned long)(j)->st_mtime<=NOW)

#endif

#define AT_SERVICE	"/dev/tcp/local/at"

#ifndef AT_JOB_DIR
#define AT_JOB_DIR	"lib/at/jobs"
#endif

#define AT_CRON_DIR	"../../../cron"
#define AT_ALLOW_FILE	"at.allow"
#define AT_DENY_FILE	"at.deny"

#define AT_EXEC_FILE	"../atx"
#define AT_QUEUE_FILE	"../../queues"
#define AT_LOG_FILE	"../../log"

#define AT_TIME_FORMAT	"%Y-%m-%d+%H:%M:%S"

#define AT_ACCESS	'a'
#define AT_ADMIN	'A'
#define AT_DEBUG	'D'
#define AT_INFO		'i'
#define AT_JOB		'j'
#define AT_LABEL	'h'
#define AT_LIST		'l'
#define AT_LOG		'L'
#define AT_MAIL		'm'
#define AT_QUEUE	'q'
#define AT_QUIT		'Q'
#define AT_REMOVE	'r'
#define AT_STATUS	'p'
#define AT_TIME		't'
#define AT_UPDATE	'U'
#define AT_USER		'u'
#define AT_VERSION	'v'

#ifndef NOW
#define NOW		(unsigned long)time((time_t*)0)
#endif

#endif
