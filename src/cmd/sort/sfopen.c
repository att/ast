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
*                     Phong Vo <phongvo@gmail.com>                     *
*              Doug McIlroy <doug@research.bell-labs.com>              *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfopen file suffix match intercept
 * export SFOPEN_INTERCEPT with one or more suffix map entries
 *
 *	option<del>[no]verbose<del>
 *	program<del><pattern><del>
 *		input<del><pattern><del>command<del>
 *		output<del><pattern><del>command<del>
 *	...
 *
 * space between entries ignored
 * program patterns matched against error_info.id
 * io patterns matched against the file path
 * (literal) \0 in command expands to the file path
 * (literal) \<n> in command expands to the <n>th subexpression in the file path pattern match
 */

#define sfopen	_sfopen

#include <ast.h>
#include <ctype.h>
#include <error.h>

#undef	sfopen

struct Io_s; typedef struct Io_s Io_t;
struct Match_s; typedef struct Match_s Match_t;

struct Io_s
{
	Io_t*		next;
	char*		command;
	char*		pattern;
};

struct Match_s
{
	Match_t*	next;
	Io_t*		io[2];
	char*		pattern;
};

static struct State_s
{
	Match_t*	match;
	Sfio_t*		cmd;
	int		dump;
	int		init;
	int		verbose;
} state;

static Match_t*
initialize(void)
{
	register char*	s;
	register char*	t;
	register char*	v;
	register char*	x;
	register int	n;
	register int	d;
	Match_t*	mp;
	Match_t*	me;
	Match_t*	mv;
	Io_t*		io;
	Io_t*		ie[2];

	mp = me = 0;
	if ((s = getenv("SFOPEN_INTERCEPT")) && (s = strdup(s)))
		for (;;)
		{
			while (isspace(*s))
				s++;
			for (t = s; isalnum(*s); s++);
			if (s == t || !(d = *s))
				break;
			*s++ = 0;
			for (v = s; *s && *s != d; s++);
			if (*s)
				*s++ = 0;
			if (streq(t, "input") || streq(t, "output"))
			{
				if (mp)
				{
					for (x = s; *s && *s != d; s++);
					if (*s)
						*s++ = 0;
					if (!(io = newof(0, Io_t, 1, 0)))
						break;
					if (*v)
						io->pattern = v;
					if (*x)
						io->command = x;
					n = *t == 'o';
					if (ie[n])
						ie[n] = ie[n]->next = io;
					else
						ie[n] = mp->io[n] = io;
				}
			}
			else if (streq(t, "program"))
			{
				if (!(mv = newof(0, Match_t, 1, 0)))
					break;
				if (*v)
					mv->pattern = v;
				if (me)
					me = me->next = mv;
				else
					mp = mv;
				ie[0] = ie[1] = 0;
			}
			else if (streq(t, "option"))
			{
				if (*v == 'n' && *(v + 1) == 'o')
				{
					v += 2;
					n = 0;
				}
				else
					n = 1;
				if (streq(v, "dump"))
					state.dump = n;
				else if (streq(v, "verbose"))
					state.verbose = n;
			}
		}
	return mp;
}

Sfio_t*
sfopen(Sfio_t* f, const char* path, const char* mode)
{
	register Match_t*	mp;
	register Io_t*		io;
	register const char*	s;
	register ssize_t	r;
	register int		c;
	register int		n;
	register int		m;

	ssize_t			sub[20];

	if (path && error_info.id)
	{
		if (!state.init)
		{
			state.init = 1;
			if (state.cmd = sfstropen())
				state.match = initialize();
			if (state.dump)
			{
				sfprintf(sfstdout, "SFIO_INTERCEPT\n");
				if (state.dump)
					sfprintf(sfstdout, "	option	dump\n");
				if (state.verbose)
					sfprintf(sfstdout, "	option	verbose\n");
				for (mp = state.match; mp; mp = mp->next)
				{
					sfprintf(sfstdout, "	program	%s	\n", mp->pattern);
					for (io = mp->io[0]; io; io = io->next)
						sfprintf(sfstdout, "		input	%s	%s\n", io->pattern, io->command);
					for (io = mp->io[1]; io; io = io->next)
						sfprintf(sfstdout, "		output	%s	%s\n", io->pattern, io->command);
				}
			}
		}
		if (mp = state.match)
		{
			n = 0;
			s = mode;
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					break;
				case 'b':
					n = -1;
					break;
				case 'w':
				case '+':
					n = 1;
					continue;
				default:
					continue;
				}
				break;
			}
			if (n > 0 || n == 0 && !access(path, R_OK))
				do
				{
					if ((!mp->pattern || strmatch(error_info.id, mp->pattern)) && (io = mp->io[n]))
					{
						do
						{
					    		if (!io->pattern && !(m = 0) || (m = strgrpmatch(path, io->pattern, sub, elementsof(sub) / 2, STR_MAXIMAL|STR_LEFT|STR_RIGHT)))
								break;
						} while (io = io->next);
						if (io)
						{
							if (s = io->command)
							{
								m *= 2;
								r = 1;
								while (c = *s++)
								{
									if (c == '\\' && *s && (c = *s++) >= '0' && c <= '9')
									{
										c = 2 * (c - '0');
										if (c < m && (r = sub[c+1] - sub[c]))
											sfwrite(state.cmd, path + sub[c], r);
										r = 0;
									}
									else
										sfputc(state.cmd, c);
								}
								if (r)
									sfprintf(state.cmd, " %c %s", n == 1 ? '<' : '>', path);
								if (!(s = sfstruse(state.cmd)))
									return 0;
								if (state.verbose)
									error(0, "%s %s", error_info.id, s);
								if (f = sfpopen(f, s, mode))
									sfset(f, SF_SHARE, 0);
								return f;
							}
							break;
						}
					}
				} while (mp = mp->next);
		}
	}
	return _sfopen(f, path, mode);
}
