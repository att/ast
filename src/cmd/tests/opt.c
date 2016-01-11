/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <debug.h>

#if OPT_VERSION >= 20000101L
#define NEW	1
#else
#define NEW	0
#endif

#if !NEW
#define name	option
#endif

typedef struct Info_s Info_t;

struct Info_s
{
	Info_t*		next;
	char*		name;
	char*		value;
};

static Info_t*		info;

#if NEW
static int
infof(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	Info_t*		ip;

	for (ip = info; ip; ip = ip->next)
		if (streq(s, ip->name))
			return sfprintf(sp, "%s", ip->value);
	if (*s == ':')
		return sfprintf(sp, "%s", *(s + 1) == 'n' ? "" : (s + 2));
	if (streq(s, "options"))
		return sfprintf(sp, "[Z:zoom?Do it as fast as possible.]\fmore#1\f\fmore#2\f[B:boom?Dump into \afile\a.]:[file]");
	if (streq(s, "zero"))
		return sfprintf(sp, "[+yabba?dabba][+doo?aroni]");
	if (streq(s, "more#1"))
		return sfprintf(sp, "[C:cram?Cram as much as possible.]\fmore#3\f");
	if (streq(s, "more#2"))
		return sfprintf(sp, "\fmore#4\f[D:dump?Dump as much as possible.]");
	if (streq(s, "more#3"))
		return sfprintf(sp, "[K:kill?kill all processes.]");
	if (streq(s, "more#4"))
		return sfprintf(sp, "[F:fudge?Fudge the statistics to satisfy everyone.]");
	if (streq(s, "more#5"))
		return sfprintf(sp, "\bred\b, \borange\b, \byellow\b, \bgreen\b, \bblue\b, \bindigo\b, \bviolet\b");
	if (streq(s, "more#6"))
		return sfprintf(sp, "\bred\b");
	return sfprintf(sp, "<* %s info ok *>", s);
}

static char*
translate(const char* locale, const char* id, const char* catalog, const char* msg)
{
	register int	c;
	register int	i;
	register char*	s;
	register char*	t;
	register char*	e;
	register char*	r;

	static char	buf[8 * 1024];

	static char	rot[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMabcdefghijklmnopqrstuvwxyzabcdefghijklm";

	sfprintf(sfstdout, "id=%s catalog=%s text=\"%s\"\n", id, catalog, msg);
	i = !catalog;
	s = (char*)msg;
	t = buf;
	e = buf + sizeof(buf) - 1;
	while ((c = *s++) && t < e)
	{
		switch (c)
		{
		case '\\':
			if (t < e)
				*t++ = c;
			if (!(c = *s++))
				s--;
			break;
		case '%':
			do
			{
				if (t >= e)
					break;
				*t++ = c;
				if (!(c = *s++))
				{
					s--;
					break;
				}
			} while (!isalpha(c) || (!islower(c) || c == 'h' || c == 'l') && (t < e) && isalpha(*s));
			break;
		case '\b':
			do
			{
				if (t >= e)
					break;
				*t++ = c;
				if (!(c = *s++))
				{
					s--;
					break;
				}
			} while (c != '\b');
			break;
		default:
			if (r = strchr(rot, c))
			{
				c = *(r + 13);
				if (i)
					c = isupper(c) ? tolower(c) : toupper(c);
			}
			break;
		}
		*t++ = c;
	}
	*t = 0;
	return streq(buf, msg) ? (char*)msg : buf;
}
#endif

int
main(int argc, char** argv)
{
	int		n;
	int		ext;
	int		ostr;
	int		str;
	int		loop;
	Info_t*		ip;
	char*		s;
	char*		command;
	char*		usage;
	char**		extra;
	char**		oargv;
#if NEW
	Optdisc_t	disc;
#endif

	error_info.id = "opt";
	setlocale(LC_ALL, "");
	error(-1, "test");
	extra = 0;
	ext = 0;
	str = 0;
	while (command = *(argv + 1))
	{
		if (*command == '=' && (s = strchr(command + 1, '=')))
		{
			argv++;
			*s++ = 0;
			command++;
			if (ip = newof(0, Info_t, 1, 0))
			{
				ip->name = command;
				ip->value = s;
				ip->next = info;
				info = ip;
			}
		}
		else if (streq(command, "-"))
		{
			argv++;
			str = NEW;
		}
		else if (streq(command, "-+"))
		{
			argv++;
#if NEW
			ast.locale.set |= (1<<AST_LC_MESSAGES);
			error_info.translate = translate;
#endif
		}
		else if (streq(command, "+") && *(argv + 2))
		{
			ext += 2;
			argv += 2;
			if (!extra)
				extra = argv;
		}
		else
			break;
	}
	if (!(command = *++argv) || !(usage = *++argv))
		error(ERROR_USAGE|4, "[ - | + usage ... ] command-name usage-string [ arg ... ]");
	argv += str;
	error_info.id = command;
#if NEW
	memset(&disc, 0, sizeof(disc));
	disc.version = OPT_VERSION;
	disc.infof = infof;
	opt_info.disc = &disc;
#else
	memset(&opt_info, 0, sizeof(opt_info));
#endif
	loop = strncmp(usage, "[-1c", 4) ? 0 : 3;
	oargv= argv;
	ostr = str;
	for (;;)
	{
		for (;;)
		{
			if (!str)
			{
				if (!(n = optget(argv, usage)))
					break;
			}
			else if (!(n = optstr(*argv, usage)))
			{
				if (!*++argv)
					break;
				continue;
			}
			if (loop)
				sfprintf(sfstdout, "[%d] ", loop);
			if (n == '?')
			{
				sfprintf(sfstdout, "return=%c option=%s name=%s num=%I*d\n", n, opt_info.option, opt_info.name, sizeof(opt_info.number), opt_info.number);
				error(ERROR_USAGE|4, "%s", opt_info.arg);
			}
			else if (n == ':')
			{
				sfprintf(sfstdout, "return=%c option=%s name=%s num=%I*d", n, opt_info.option, opt_info.name, sizeof(opt_info.number), opt_info.number);
				if (!opt_info.option[0])
					sfprintf(sfstdout, " str=%s", argv[opt_info.index - 1]);
				sfputc(sfstdout, '\n');
				error(2, "%s", opt_info.arg);
			}
			else if (n > 0)
				sfprintf(sfstdout, "return=%c option=%s name=%s arg%-.1s=%s num=%I*d\n", n, opt_info.option, opt_info.name, &opt_info.assignment, opt_info.arg, sizeof(opt_info.number), opt_info.number);
			else
				sfprintf(sfstdout, "return=%d option=%s name=%s arg%-.1s=%s num=%I*d\n", n, opt_info.option, opt_info.name, &opt_info.assignment, opt_info.arg, sizeof(opt_info.number), opt_info.number);
			if (extra)
			{
				for (n = 0; n < ext; n += 2)
					optget(NiL, extra[n]);
				extra = 0;
			}
		}
		if (!str && *(argv += opt_info.index))
			while (command = *argv++)
			{
				if (loop)
					sfprintf(sfstdout, "[%d] ", loop);
				sfprintf(sfstdout, "argument=%d value=\"%s\"\n", ++str, command);
			}
		if (--loop <= 0)
			break;
		argv = oargv;
		str = ostr;
		opt_info.index = 0;
	}
	return error_info.errors != 0;
}
