/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2012 AT&T Intellectual Property          *
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
 * atx -- the at impersonator
 *
 *	cd AT_JOB_DIR
 *	$PWD/AT_EXEC_FILE $SHELL <job>
 */

static const char id[] = "\n@(#)$Id: atx (AT&T Research) 2012-02-29 $\0\n";

#include "at.h"

/*
 * prepend current date-time to buffer on fd==2
 * and drop the initial command label if any
 */

static ssize_t
stampwrite(int fd, const void* buf, size_t n)
{
	register char*		s;
	register int		i;
	register ssize_t	r;
	register ssize_t	z;

	r = 0;
	if (fd == 2 && (s = fmttime(AT_TIME_FORMAT, time(0))))
	{
		i = strlen(s);
		s[i++] = ' ';
		if ((z = write(fd, s, i)) < 0)
			r = -1;
		else
			r += z;
		for (s = (char*)buf; s < ((char*)buf + n - 1) && !isspace(*s); s++)
			if (*s == ':')
			{
				while (++s < ((char*)buf + n - 1) && isspace(*s));
				n -= s - (char*)buf;
				buf = (void*)s;
				break;
			}
	}
	if ((z = write(fd, buf, n)) < 0)
		r = -1;
	else if (r >= 0)
		r += z;
	return r;
}

int
main(int argc, char** argv)
{
	register int	n = 0;
	unsigned long	uid;
	unsigned long	gid;
	unsigned long	tid;
	struct stat	ds;
	struct stat	js;
	struct stat	xs;

	error_info.id = "atx";
	error_info.write = stampwrite;
	if (argc != 3 ||
	    ++n && lstat(".", &ds) ||
	    ++n && !AT_DIR_OK(&ds) ||
	    ++n && lstat(argv[2], &js) ||
	    ++n && !AT_JOB_OK(&ds, &js) ||
	    ++n && !S_ISREG(js.st_mode) ||
	    ++n && lstat(AT_EXEC_FILE, &xs) ||
	    ++n && !AT_EXEC_OK(&ds, &xs) ||

	    ++n && sfsscanf(argv[2], "%..36lu.%..36lu.%..36lu", &uid, &gid, &tid) != 3)
		error(3, "%s: command garbled [%d]", argc >= 3 ? argv[2] : (char*)0, n);
	if (setgid(gid))
		error(ERROR_SYSTEM|3, "%s %s group denied (gid=%u egid=%u => gid=%d)", argv[2], error_info.id, getgid(), getegid(), gid);
	if (setuid(uid))
		error(ERROR_SYSTEM|3, "%s %s user denied (uid=%u euid=%u => uid=%d)", argv[2], error_info.id, getuid(), geteuid(), uid);
	setsid();
	argv++;
	execvp(argv[0], argv);
	error(ERROR_SYSTEM|3, "%s: exec failed", argv[2]);
	return 1;
}
