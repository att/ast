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
 * <recfmt.h> test harness
 */

static const char usage[] =
"[-?\n@(#)$Id: rec (AT&T Research) 2005-11-01 $\n]"
USAGE_LICENSE
"[+NAME?rec - <recfmt.h> test harness]"
"[+DESCRIPTION?\brec\b is a <recfmt.h> test harness. It converts the "
    "\aformat\a operand to a format descriptor using \brecstr\b(3) and lists "
    "the \bfmtrec\b(3) canonical format string on the standard output. If a "
    "\afile\a operand is specified then a line containing the number of "
    "records and the sum of all record lengths is printed. If \afile\a is "
    "\b-\b then the standard input is read.]"
"[c:count?Count the number of records.]"
"[l:length?List the length of each record.]"
"[r:reserve?List underlying sfreserve() sizes and offsets.]"

"\n"
"\nformat [ file ]\n"
"\n"

"[+SEE ALSO?\brecfmt\b(3)]"
;

#include <ast.h>
#include <error.h>
#include <ls.h>
#include <recfmt.h>

/*
 * process argv as in sort(1)
 */

int
main(int argc, char** argv)
{
	Recfmt_t	f;
	char*		s;
	char*		e;
	char*		b;
	char*		path;
	ssize_t		m;
	ssize_t		z;
	Sfoff_t		o;
	Sfoff_t		r;
	Sfoff_t		t;
	Sfio_t*		sp;
	struct stat	st;

	int		count = 0;
	int		length = 0;
	int		reserve = 0;

	NoP(argc);
	error_info.id = "rec";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			count = !!opt_info.num;
			continue;
		case 'l':
			length = !!opt_info.num;
			continue;
		case 'r':
			reserve = !!opt_info.num;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (!*argv || *(argv + 1) && *(argv + 2) || error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	f = recstr(*argv++, &e);
	if (!(path = *argv))
		sp = 0;
	else if (streq(path, "-"))
		sp = sfstdin;
	else if (!(sp = sfopen(NiL, path, "r")))
		error(ERROR_SYSTEM|3, "%s: cannot open", path);
	sfprintf(sfstdout, "%s", fmtrec(f, 0));
	if (*e)
		sfprintf(sfstdout, " [%s]", e);
	if (sp && RECTYPE(f) == REC_method)
	{
		e = "";
		switch (REC_M_INDEX(f))
		{
		case REC_M_data:
			if (s = sfreserve(sp, SF_UNBOUND, SF_LOCKR))
			{
				z = sfvalue(sp);
				if (fstat(sffileno(sp), &st) || st.st_size < z)
					st.st_size = 0;
				f = recfmt(s, z, st.st_size);
				sfread(sp, s, 0);
			}
			break;
		case REC_M_path:
			f = ((s = strrchr(path, '%')) && !strchr(++s, '/')) ? recstr(s, &e) : REC_N_TYPE();
			break;
		default:
			f = REC_N_TYPE();
			break;
		}
		sfprintf(sfstdout, " => %s", fmtrec(f, 0));
		if (*e)
			sfprintf(sfstdout, " [%s]", e);
	}
	sfprintf(sfstdout, "\n");
	if (sp && (count || length))
	{
		r = 0;
		t = 0;
		b = s = e = 0;
		m = SF_UNBOUND;
		for (;;)
		{
			if (s >= e)
			{
				if (b)
					sfread(sp, b, s - b);
				o = sftell(sp);
				if (!(b = sfreserve(sp, m, SF_LOCKR)))
					break;
				m = SF_UNBOUND;
				s = b;
				e = s + sfvalue(sp);
				if (reserve)
					sfprintf(sfstdout, "reserve %d at %I*d\n", e - s, sizeof(o), o);
			}
			z = reclen(f, s, e - s);
			if (z > 0)
			{
				if (length)
					sfprintf(sfstdout, "%I*d\n", sizeof(z), z);
				r++;
				t += z;
				if (z <= (e - s))
					s += z;
				else
				{
					if (b)
					{
						sfread(sp, b, e - b);
						b = 0;
					}
					o = sftell(sp);
					if (reserve)
						sfprintf(sfstdout, "skip %I*d to complete %I*d at %I*d\n", sizeof(z), z - (e - s), sizeof(z), z, sizeof(o), o);
					z -= e - s;
					if (!sfreserve(sp, z, 0))
					{
						if (length && (z = sfvalue(sp)))
							sfprintf(sfstdout, "partial %I*d\n", sizeof(z), z);
						break;
					}
					s = e;
				}
			}
			else if (z < 0)
			{
				sfprintf(sfstdout, "recfmt FAILED\n");
				break;
			}
			else
			{
				if (reserve)
					sfprintf(sfstdout, "recfmt 0 with %d remaining in buffer\n", e - s);
				if (b)
				{
					sfread(sp, b, s - b);
					b = 0;
				}
				m = -(e - s + 1);
				e = s;
			}
		}
		if (count)
			sfprintf(sfstdout, "%I*d %I*d\n", sizeof(r), r, sizeof(t), t);
	}
	return 0;
}
