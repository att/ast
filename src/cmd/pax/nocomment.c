/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
 * nocomment -- strip C comments
 */

#include <ast.h>
#include <ctype.h>

#if 0
static int line_sfputc(int line, Sfio_t* sp, int c)
{
	sfprintf(sp, "<<<C:%d>>>", line);
	return sfputc(sp, c);
}
static int line_sfputr(int line, Sfio_t* sp, const char* buf, int op)
{
	sfprintf(sp, "<<<R:%d>>>", line);
	return sfputr(sp, buf, op);
}
#undef	sfputc
#define sfputc(a,b)	line_sfputc(__LINE__,a,b)
#undef	sfputr
#define sfputr(a,b,c)	line_sfputr(__LINE__,a,b,c)
#endif

#define SYNC()	do							\
		{							\
			if (line > prev && line > directive)		\
			{						\
				if (sfsprintf(buf, sizeof(buf), "\n#%s %d\n", sync <= 0 ? "line" : "", line + 1) <= line - prev)	\
				{					\
					sfputr(op, buf, -1);		\
					prev = line;			\
				}					\
				else while (prev < line)		\
				{					\
					prev++;				\
					sfputc(op, '\n');		\
				}					\
				data = 0;				\
			}						\
		} while (0)

#define DATA(c)	do							\
		{							\
			SYNC();						\
			data = 1;					\
			sfputc(op, c);					\
		} while (0)

/*
 * get next token on ip
 */

static char*
token(register Sfio_t* ip)
{
	register int	c;
	register char*	s;

	static char	buf[1024];

	s = buf;
	for (;;)
	{
		switch (c = sfgetc(ip))
		{
		case EOF:
			if (s > buf)
				break;
			return 0;
		case '\\':
		case '/':
		case '\n':
			sfungetc(ip, c);
			break;
		default:
			if (isspace(c))
			{
				if (s > buf)
				{
					sfungetc(ip, c);
					break;
				}
			}
			else if (s < &buf[sizeof(buf)-1])
				*s++ = c;
			continue;
		}
		break;
	}
	*s = 0;
	return buf;
}

/*
 * uncomment ip into op
 */

off_t
nocomment(register Sfio_t* ip, Sfio_t* op)
{
	register int	c = 0;
	register int	p;
	register int	data = 0;
	int		sync = 0;
	int		formals = 0;
	unsigned long	line = 0;
	unsigned long	prev = 0;
	unsigned long	directive = 0;
	Sfio_t*		pp = 0;
	off_t		count;
	int		quote;
	int		n;
	char*		s;
	char		buf[PATH_MAX];

	count = sftell(op);
	for (;;)
	{
	next:
		p = c;
		c = sfgetc(ip);
	check:
		switch (c)
		{
		case EOF:
			if (line != prev)
				sfputc(op, '\n');
			goto done;
		case 0:
			sfputc(op, c);
			sfmove(ip, op, SF_UNBOUND, -1);
			goto done;
		case '\\':
			DATA(c);
			switch (c = sfgetc(ip))
			{
			case EOF:
				goto done;
			case '\n':
				directive++;
				prev = ++line;
				sfputc(op, c);
				c = 0;
				break;
			default:
				sfputc(op, c);
				break;
			}
			break;
		case '\f':
		case '\t':
		case '\v':
			c = ' ';
			/*FALLTHROUGH*/
		case ' ':
			if (data) switch (p)
			{
			case ' ':
			case '\n':
			case '(':
			case '[':
			case ']':
			case '{':
			case '}':
			case ';':
			case ':':
			case '!':
			case '<':
			case '>':
			case '|':
			case ',':
			case '?':
			case '*':
				break;
			case ')':
				if (directive == line && formals)
				{
					formals = 0;
					sfputc(op, ' ');
				}
				break;
			default:
				switch (c = sfgetc(ip))
				{
				case '\n':
				case ')':
				case '[':
				case ']':
				case '{':
				case '}':
				case ';':
				case ':':
				case '=':
				case '!':
				case '<':
				case '>':
				case '|':
				case ',':
				case '?':
					break;
				case '(':
					if (directive == line && formals)
					{
						formals = 0;
						sfputc(op, ' ');
					}
					break;
				case '*':
					if (p == '/' || p == '=')
						sfputc(op, ' ');
					break;
				case '+':
				case '-':
				case '&':
					if (p == c || p == '=')
						sfputc(op, ' ');
					break;
				default:
					sfputc(op, ' ');
					break;
				}
				p = ' ';
				goto check;
			}
			break;
		case '\n':
			data = 0;
			line++;
			break;
		case '#':
			SYNC();
			directive = line;
			formals = 1;
			if (sync >= 0 && !data)
			{
				if (s = token(ip))
				{
					if (isdigit(*s))
					{
						sync = 1;
						directive = 0;
						line = strtol(s, NiL, 10);
						if (s = sfgetr(ip, '\n', 1))
						{
							while (isspace(*s))
								s++;
							if (*s == '"')
							{
								sfprintf(op, "# %d %s\n", line, s);
								prev = line;
							}
						}
						break;
					}
					if (!sync)
					{
						if (!pp && line <= 24 && streq(s, "pragma"))
						{
							if ((s = token(ip)) && streq(s, "prototyped"))
							{
								sync = -1;
								if (c = sffileno(ip))
								{
									n = dup(0);
									close(0);
									dup(c);
								}
								sfseek(ip, (Sfoff_t)(-1), SEEK_CUR);
								sfsync(ip);
								if (pp = sfpopen(NiL, "proto -fns", "r"))
									ip = pp;
								if (c)
								{
									close(0);
									dup(n);
								}
								c = 0;
								break;
							}
							DATA('#');
							sfputr(op, "pragma", ' ');
							sfputr(op, s, -1);
							c = 0;
							break;
						}
						if (!streq(s, "ident") && !streq(s, "line") && !streq(s, "pragma"))
							sync = -1;
					}
					DATA('#');
					sfputr(op, s, -1);
					c = 0;
				}
				else sync = -1;
			}
			if (c) DATA(c);
			else data = 1;
			break;
		case '/':
			switch (c = sfgetc(ip))
			{
			case EOF:
				goto done;
			case '/':
				for (;;) switch (p = c, c = sfgetc(ip))
				{
				case EOF:
					goto done;
				case '\n':
					if (p == '\\')
					{
						sfputc(op, p);
						sfputc(op, c);
						prev = ++line;
						break;
					}
					p = ' ';
					c = '\n';
					goto check;
				}
				break;
			case '*':
				for (;;) switch (p = c, c = sfgetc(ip))
				{
				case EOF:
					goto done;
				case '\n':
					directive++;
					line++;
					break;
				case '*':
					for (;;)
					{
						switch (c = sfgetc(ip))
						{
						case EOF:
							goto done;
						case '\n':
							directive++;
							line++;
							break;
						case '/':
							p = c = ' ';
							goto check;
						case '*':
							continue;
						}
						break;
					}
					break;
				}
				break;
			default:
				p = '/';
				DATA(p);
				goto check;
			}
			break;
		case '"':
		case '\'':
			DATA(c);
			quote = c;
			for (;;)
			{
				switch (c = sfgetc(ip))
				{
				case EOF:
					goto done;
				case '\\':
					sfputc(op, c);
					switch (c = sfgetc(ip))
					{
					case EOF:
						goto done;
					case '\n':
						directive++;
						prev = ++line;
						break;
					}
					break;
				case '"':
				case '\'':
					if (c == quote)
					{
						sfputc(op, c);
						goto next;
					}
					break;
				case '\n':
					p = ' ';
					goto check;
				}
				sfputc(op, c);
			}
			break;
		default:
			DATA(c);
			break;
		}
	}
 done:
	count = count < 0 ? 0 : (sferror(ip) || sferror(op)) ? -1 : (sftell(op) - count);
	if (pp) sfclose(pp);
	return count;
}
