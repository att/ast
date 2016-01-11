/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
 * convert delimited/terminated flat file from
 * row major to col major order
 */

static const char id[] = "\n@(#)$Id: r2c (AT&T Research) 1998-11-08 $\0\n";

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <ls.h>
#include <sfdcgzip.h>

typedef struct Col_s
{
	struct Col_s*	next;
	Sfio_t*		sp;
} Col_t;

static struct State_s
{
	Col_t*		cols;
	Col_t*		last;

	size_t		cache;
	size_t		window;

	int		delimiter;
	int		level;
	int		quote;
	int		terminator;
	int		test;
	int		verbose;
} state;

/*
 * flush output to op
 */

static int
flush(Sfio_t* op)
{
	register Col_t*	col;
	register size_t	n;

	if ((col = state.cols) && sfstrtell(col->sp))
	{
		do
		{
			n = sfstrtell(col->sp);
			if (sfwrite(op, sfstrseek(col->sp, 0, SEEK_SET), n) != n || sfsync(op))
			{
				error(ERROR_SYSTEM|2, "write error");
				return -1;
			}
		} while (col = col->next);
	}
	state.cache = state.window;
	return 0;
}

/*
 * convert one file from ip to op
 */

static int
r2c(const char* file, Sfio_t* ip, Sfio_t* op)
{
	register char*		s;
	register char*		e;
	register int		d;
	register int		q;
	register int		t;
	register Col_t*		col;
	register size_t		w;
	register size_t		n;
	register size_t		m;
	char*			b;

	w = state.cache;
	d = state.delimiter;
	q = state.quote;
	t = state.terminator;
	while (s = sfgetr(ip, t, 0))
	{
		n = sfvalue(ip);
		if (w < n)
		{
			if (n > state.window)
			{
				error(2, "%s: input record larger than window size", file);
				return -1;
			}
			if (flush(op))
				return -1;
		}
		w -= n;
		col = state.cols;
		for (;;)
		{
			if (!col)
			{
				if (!(col = newof(0, Col_t, 1, 0)) || !(col->sp = sfstropen()))
					error(ERROR_SYSTEM|3, "out of space [column]");
				if (!state.cols)
					state.cols = col;
				else if (state.last)
					state.last->next = col;
				state.last = col;
			}
			if (q && *s == q)
			{
				b = s;
				e = s + n;
				while (++b < e)
					if (*b == q)
					{
						b++;
						break;
					}
				m = b - s;
			}
			else if (e = memchr(s, d, n))
				m = e - s + 1;
			else
				m = n;
			if (sfwrite(col->sp, s, m) != m)
			{
				error(ERROR_SYSTEM|2, "%s: column write error");
				return -1;
			}
			if (!e)
				break;
			s += m;
			n -= m;
			col = col->next;
		}
	}
	if (sfgetr(ip, -1, 0))
	{
		error(2, "%s: last record incomplete", file);
		return -1;
	}
	return 0;
}

main(int argc, char** argv)
{
	char*		file;
	Sfio_t*		ip;

	error_info.id = "r2c";
	state.delimiter = ':';
	state.terminator = '\n';
	state.window = 4 * 1024 * 1024;
	state.level = 9;
	for (;;)
	{
		switch (optget(argv, "d:[delimiter]l#[compression-level]q:[quote]t:[terminator]vw#[window-size]T#[test-mask]"))
		{
		case 'd':
			state.delimiter = *opt_info.arg;
			continue;
		case 'l':
			state.level = opt_info.num;
			continue;
		case 'q':
			state.quote = *opt_info.arg;
			continue;
		case 't':
			state.delimiter = *opt_info.arg;
			continue;
		case 'v':
			state.verbose = 1;
			continue;
		case 'w':
			state.window = opt_info.num;
			continue;
		case 'T':
			state.test |= opt_info.num;
			continue;
		case '?':
			error(ERROR_USAGE|4, opt_info.arg);
			continue;
		case ':':
			error(2, opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	state.cache = state.window;
	if (state.level > 0 && sfdcgzip(sfstdout, state.level) < 0)
		error(3, "output compress discipline error");
	if (!*argv)
		r2c("/dev/stdin", sfstdin, sfstdout);
	else
		while (file = *argv++)
		{
			if (streq(file, "-"))
			{
				file = "/dev/stdin";
				ip = sfstdin;
			}
			else if (!(ip = sfopen(NiL, file, "r")))
			{
				error(ERROR_SYSTEM|2, "%s: cannot read", file);
				continue;
			}
			r2c(file, ip, sfstdout);
			if (ip != sfstdin)
				sfclose(ip);
		}
	flush(sfstdout);
	return error_info.errors != 0;
}
