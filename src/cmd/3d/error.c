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
 * Glenn Fowler
 * AT&T Research
 *
 * error and message formatter
 *
 *	level is the error level
 *	level >= error_info.core!=0 dumps core
 *	level >= ERROR_FATAL calls error_info.exit
 *	level < 0 is for debug tracing
 *
 * NOTE: non-sfio version just for 3d
 */

#include "3d.h"

#if DEBUG

static ssize_t
fs3d_write(int fd, const void* buf, size_t n)
{
	return(WRITE(fd, buf, n));
}

#undef error_info
#define error_info	_error_info_

Error_info_t	_error_info_ = { 2, exit, fs3d_write };
Error_info_t*	_error_infop_ = &_error_info_;
Error_info_t*	_error_data_ = &_error_info_;

/*
 * print a name, converting unprintable chars
 */

static void
print(char** buf, char* end, register char* name, char* delim)
{
	register char*	s = *buf;
	register char*	e = end;
	register int	c;

	while (c = *name++)
	{
		if (c & 0200)
		{
			c &= 0177;
			if (s >= e) break;
			*s++ = '?';
		}
		if (c < ' ')
		{
			c += 'A' - 1;
			if (s >= e) break;
			*s++ = '^';
		}
		if (s >= e) break;
		*s++ = c;
	}
	while (*delim)
	{
		if (s >= e) break;
		*s++ = *delim++;
	}
	*buf = s;
}

void
errorv(const char* lib, int level, va_list ap)
{
	register int	n;
	int		fd;
	int		flags;
	char*		b;
	char*		e;
	char*		format;
	char		buf[4096];

	int		line;
	char*		file;

	static int	intercepted;

	if (intercepted++)
	{
		intercepted--;
		return;
	}
	if (level > 0)
	{
		flags = level & ~ERROR_LEVEL;
		level &= ERROR_LEVEL;
	}
	else flags = 0;
	if ((fd = fsfd(&state.fs[FS_option])) <= 0 || level < error_info.trace || lib && (error_info.clear & ERROR_LIBRARY) || level < 0 && error_info.mask && !(error_info.mask & (1<<(-level - 1))))
	{
		if (level >= ERROR_FATAL) (*error_info.exit)(level - 1);
		intercepted--;
		return;
	}
	if (error_info.trace < 0) flags |= ERROR_LIBRARY|ERROR_SYSTEM;
	flags |= error_info.set;
	flags &= ~error_info.clear;
	if (!lib) flags &= ~ERROR_LIBRARY;
	e = (b = buf) + elementsof(buf) - 1;
	file = error_info.id;
	if (flags & ERROR_USAGE)
	{
		bprintf(&b, e, (flags & ERROR_NOID) ? "       " : "Usage: ");
		if (file) print(&b, e, file, " ");
	}
	else if (level && !(flags & ERROR_NOID))
	{
		if (file) print(&b, e, file, (flags & ERROR_LIBRARY) ? " " : ": ");
		if (flags & ERROR_LIBRARY)
			bprintf(&b, e, "[%s library]: ", lib);
	}
	if (level > 0 && error_info.line > 0)
	{
		if (error_info.file && *error_info.file)
			bprintf(&b, e, "\"%s\", ", error_info.file);
		bprintf(&b, e, "line %d: ", error_info.line);
	}
	switch (level)
	{
	case 0:
		break;
	case ERROR_WARNING:
		error_info.warnings++;
		bprintf(&b, e, "warning: ");
		break;
	case ERROR_PANIC:
		error_info.errors++;
		bprintf(&b, e, "panic: ");
		break;
	default:
		if (level < 0)
		{
			if (error_info.trace < -1) bprintf(&b, e, "debug%d:%s", level, level > -10 ? " " : "");
			else bprintf(&b, e, "debug: ");
			for (n = 0; n < error_info.indent; n++)
			{
				*b++ = ' ';
				*b++ = ' ';
			}
		}
		else error_info.errors++;
		break;
	}
	if (flags & ERROR_OUTPUT) fd = va_arg(ap, int);
	if (flags & ERROR_SOURCE)
	{
		/*
		 * source ([version], file, line) message
		 */

		file = va_arg(ap, char*);
		line = va_arg(ap, int);
		if (error_info.version) bprintf(&b, e, "(%s: %s, line %d) ", error_info.version, file, line);
		else bprintf(&b, e, "(%s, line %d) ", file, line);
	}
	format = va_arg(ap, char*);
	bvprintf(&b, e, format, ap);
	if (!(flags & ERROR_PROMPT))
	{
		if ((flags & ERROR_SYSTEM) && errno && errno != error_info.last_errno)
		{
			n = state.in_2d;
			state.in_2d = 1;
			bprintf(&b, e, " [%s]", fmterror(errno));
			state.in_2d = n;
			if (error_info.set & ERROR_SYSTEM) errno = 0;
			error_info.last_errno = (level >= 0) ? 0 : errno;
		}
		*b++ = '\n';
	}
	*b = 0;
	if (error_info.write) (*error_info.write)(fd, buf, b - buf);
	if (level >= error_info.core && error_info.core)
	{
		signal(SIGQUIT, SIG_DFL);
		kill(getpid(), SIGQUIT);
		pause();
	}
	if (level >= ERROR_FATAL) (*error_info.exit)(level - ERROR_FATAL + 1);
	intercepted--;
}

void
error(int level, ...)
{
	va_list	ap;

	va_start(ap, level);
	errorv(NiL, level, ap);
	va_end(ap);
}

int
errormsg(const char* dictionary, int level, ...)
{
	va_list	ap;

	va_start(ap, level);
	errorv(dictionary, level, ap);
	va_end(ap);
	return 0;
}

int
errorf(void* handle, void* discipline, int level, ...)
{
	va_list	ap;

	va_start(ap, level);
	errorv(discipline ? *((char**)handle) : (char*)handle, (discipline || level < 0) ? level : (level | ERROR_LIBRARY), ap);
	va_end(ap);
	return 0;
}

#else

NoN(error)

#endif
