/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2013 AT&T Intellectual Property          *
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
 * make support routines
 */

#include "make.h"

/*
 * stat() that checks for read access
 * if res!=0 then resolve(name,fd,mode) must be called
 */

int
rstat(char* name, Stat_t* st, int res)
{
	Time_t	t;

	if (internal.openfile)
	{
		internal.openfile = 0;
		close(internal.openfd);
	}
	while ((internal.openfd = open(name, O_RDONLY|O_BINARY|O_CLOEXEC)) < 0)
	{
		if (errno != EINTR)
		{
			if (errno == EACCES)
			{
				if (state.maxview && !state.fsview)
				{
					int		oerrno = errno;
					Stat_t		rm;

					if (!stat(name, st) && st->st_nlink > 1 && !st->st_size && !(st->st_mode & S_IPERM))
					{
						edit(internal.tmp, name, KEEP, ".../...", DELETE);
						if (!stat(sfstruse(internal.tmp), &rm) && rm.st_ino == st->st_ino)
						{
							/*
							 * not in this or any lower view
							 */

							errno = ENODEV;
							return -1;
						}
					}
					errno = oerrno;
				}
				if (!stat(name, st) && (st->st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)))
					goto found;
				error(1, "%s ignored -- not readable", name);
			}
			return -1;
		}
	}
	internal.openfile = name;
	if (res && resolve(internal.openfile, internal.openfd, O_RDONLY))
		return -1;
	if (fstat(internal.openfd, st))
		return -1;
#if _WINIX
	if (st->st_nlink > 1)
	{
		internal.openfile = 0;
		close(internal.openfd);
		internal.openfd = -1;
	}
#endif
 found:
	if (!tmxgetmtime(st))
	{
		if (S_ISREG(st->st_mode) || S_ISDIR(st->st_mode))
			error(1, "%s modify time must be after the epoch", name);
		tmxsetmtime(st, OLDTIME);
	}
	if (state.regress && (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)))
	{
		t = CURTIME;
		tmxsetmtime(st, t);
	}
	return 0;
}

/*
 * allocate a chunk of units
 * free units linked onto head
 */

void*
newchunk(char** head, register size_t unit)
{
#if __hppa || __hppa__ || hppa /* cc botches e arithmetic -- off by 8! */
	NoP(head);
	return newof(0, char, unit, 0);
#else
	register char*	p;
	register char*	e;
	register char**	x;
	int		n;
	void*		v;

	n = (4096 / unit) * unit;
	v = p = newof(0, char, n, 0);
	e = p + n - unit;
	x = head;
	while (((char*)(x = (char**)(*x = p += unit))) < e);
	return v;
#endif
}

/*
 * append a list q onto the end of list p
 * list p is surgically modified
 */

List_t*
append(List_t* p, List_t* q)
{
	register List_t*	t;

	if (t = p)
	{
		if (q)
		{
			while (t->next)
				t = t->next;
			t->next = q;
		}
		return p;
	}
	else return q;
}

/*
 * add rule r onto the front of list p
 * list p is not modified
 */

List_t*
cons(Rule_t* r, List_t* p)
{
	register List_t*	q;

	newlist(q);
	q->next = p;
	q->rule = r;
	return q;
}

/*
 * construct and return a copy of list p
 * the items in the list are not copied
 */

List_t*
listcopy(register List_t* p)
{
	register List_t*	q;
	register List_t*	r;
	register List_t*	t;

	if (!p)
		return 0;
	newlist(r);
	q = r;
	while (p)
	{
		q->rule = p->rule;
		if (p = p->next)
		{
			newlist(t);
			q = q->next = t;
		}
	}
	q->next = 0;
	return r;
}

/*
 * format time or convert to number
 */

char*
timefmt(const char* fmt, Time_t t)
{
	if (!t)
		t = 0;
	else if (t == NOTIME)
		t = 100000000;
	else if (t == OLDTIME)
		t = 200000000;
	else if (state.regress)
	{
		if (t < state.start)
			t = 300000000;
		else
			t = 400000000;
	}
	else
		return fmttmx((fmt && *fmt) ? fmt : "%s.%6N", t);
	return fmttmx((fmt && *fmt) ? fmt : "%s.%1N", t);
}

/*
 * convert numeric string time to Time_t
 */

Time_t
timenum(const char* s, char** p)
{
	Time_t		t;
	unsigned long	n;
	unsigned long	m;
	char*		e;

	if ((t = strtoull(s, &e, 10)) == (Time_t)(-1))
	{
		if (p)
			*p = (char*)s;
		return TMX_NOTIME;
	}
	n = 0;
	if (*e == '.')
	{
		m = 1000000000;
		while (isdigit(*++e))
			n += (*e - '0') * (m /= 10);
	}
	if (p)
		*p = e;
	return tmxsns(t, n);
}

/*
 * convert time t to a string for tracing
 */

char*
timestr(Time_t t)
{
	if (!t)
		return "not found";
	else if (t == NOTIME)
		return "not checked";
	else if (t == OLDTIME)
		return "really old";
	else if (!state.regress)
		return fmttmx("%?%K.%6N", t);
	else if (t < state.start)
		return "recent";
	else
		return "current";
}

/*
 * printext() value types
 */

#if _ast_intmax_long
#undef	strtoll
#define strtoll		strtol
#undef	strtoull
#define strtoull	strtoul
#endif

typedef union Value_u
{
	char**		p;
	char*		s;
	intmax_t	q;
	unsigned long	u;
	long		l;
	int		i;
	short		h;
	char		c;
	double		d;
} Value_t;

typedef struct Fmt_s
{
	Sffmt_t		fmt;
	char*		arg;
	int		all;
	Sfio_t*		tmp;
} Fmt_t;

/*
 * printf %! extension function
 */

static int
printext(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register Fmt_t*	fp = (Fmt_t*)dp;
	Value_t*	value = (Value_t*)vp;
	register char*	s;
	char*		txt;
	char*		e;
	Time_t		tm;

	if (fp->all)
	{
		s = fp->arg;
		fp->arg += strlen(s);
	}
	else if (!(s = getarg(&fp->arg, NiL)))
		return -1;
	if (dp->n_str > 0)
	{
		if (!fp->tmp)
			fp->tmp = sfstropen();
		sfprintf(fp->tmp, "%.*s", dp->n_str, dp->t_str);
		txt = sfstruse(fp->tmp);
	}
	else
		txt = 0;
	dp->flags |= SFFMT_VALUE;
	switch (dp->fmt)
	{
	case 'C':
		error(1, "%%%c: obsolete: use the %%c format", dp->fmt);
		dp->fmt = 'c';
		/*FALLTHROUGH*/
	case 'c':
		value->c = *s;
		break;
	case 'd':
		dp->size = sizeof(value->q);
		value->q = strtoll(s, NiL, 0);
		break;
	case 'F':
		dp->fmt = 'f';
		/*FALLTHROUGH*/
	case 'a':
	case 'A':
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
		dp->size = sizeof(value->d);
		value->d = strtod(s, NiL);
		break;
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		dp->size = sizeof(value->q);
		value->q = strtoull(s, NiL, 0);
		break;
	case 'p':
		value->p = (char**)strtol(s, NiL, 0);
		break;
	case 'S':
		error(1, "%%%c: obsolete: use the %%s format", dp->fmt);
		dp->fmt = 's';
		/*FALLTHROUGH*/
	case 's':
		value->s = s;
		if (txt)
		{
			if (streq(txt, "identifier"))
			{
				if (*s && !istype(*s, C_ID1))
					*s++ = '_';
				for (; *s; s++)
					if (!istype(*s, C_ID1|C_ID2))
						*s = '_';
			}
			else if (streq(txt, "invert"))
			{
				for (; *s; s++)
					if (isupper(*s))
						*s = tolower(*s);
					else if (islower(*s))
						*s = toupper(*s);
			}
			else if (streq(txt, "lower"))
			{
				for (; *s; s++)
					if (isupper(*s))
						*s = tolower(*s);
			}
			else if (streq(txt, "upper"))
			{
				for (; *s; s++)
					if (islower(*s))
						*s = toupper(*s);
			}
			else if (streq(txt, "variable"))
			{
				for (; *s; s++)
					if (!istype(*s, C_VARIABLE1|C_VARIABLE2))
						*s = '.';
			}
		}
		dp->size = -1;
		break;
	case 't':
	case 'T':
		if (!isdigit(*s) || ((tm = timenum(s, &e)), *e))
			tm = tmxdate(s, &e, TMX_NOW);
		if (*e || tm == TMX_NOTIME)
			tm = CURTIME;
		value->s = txt ? fmttmx(txt, tm) : timestr(tm);
		dp->fmt = 's';
		dp->size = -1;
		break;
	case 'Z':
		dp->fmt = 'c';
		value->c = 0;
		break;
	case '.':
		value->i = (int)strtol(s, NiL, 0);
		break;
	default:
		tmpname[0] = dp->fmt;
		tmpname[1] = 0;
		value->s = tmpname;
		error(2, "%%%c: unknown format", dp->fmt);
		dp->fmt = 's';
		dp->size = -1;
		break;
	}
	return 0;
}

/*
 * printf from args in argp into sp
 * all!=0 if %s gets all of argp
 * term is an sfputr terminator
 */

int
strprintf(Sfio_t* sp, const char* format, char* argp, int all, int term)
{
	int	n;
	int	i;
	Sfio_t*	tp;
	Fmt_t	fmt;

	memset(&fmt, 0, sizeof(fmt));
	fmt.fmt.version = SFIO_VERSION;
	tp = sfstropen();
	sfprintf(tp, "%s", format);
	stresc(fmt.fmt.form = sfstruse(tp));
	fmt.fmt.extf = printext;
	fmt.arg = argp;
	fmt.all = all;
	n = 0;
	while ((i = sfprintf(sp, "%!", &fmt)) >= 0)
	{
		n += i;
		if (fmt.arg <= argp || !*(argp = fmt.arg))
			break;
	}
	if (term != -1)
	{
		sfputc(sp, term);
		n++;
	}
	sfstrclose(tp);
	if (fmt.tmp)
		sfstrclose(fmt.tmp);
	return n;
}

/*
 * return next (possibly quoted) space-separated arg in *buf
 * *buf points past end of arg on return
 * the contents of buf are modified
 * if flags!=0 then it is set with metarule specific A_* flags
 */

char*
getarg(char** buf, register int* flags)
{
	register char*	s;
	register char*	t;
	register int	c;
	char*		a;
	char*		q;
	int		paren;

	if (flags)
		*flags &= ~(A_group|A_metarule|A_scope);
	for (s = *buf; isspace(*s); s++);
	if (flags && !(*flags & A_nooptions) && s[0] == '-' && s[1] == '-')
	{
		if (!s[2] || isspace(s[2]))
		{
			for (s += 2; isspace(*s); s++);
			*flags |= A_nooptions;
		}
		else
			*flags |= A_scope;
	}
	if (!*(a = t = s))
		return 0;
	paren = 0;
	for (;;)
	{
		switch (c = *s++)
		{
		case '\\':
			if (*s)
			{
				*t++ = c;
				c = *s++;
			}
			break;
		case '(':
			paren++;
			if (flags)
				*flags |= A_group;
			break;
		case ')':
			paren--;
			break;
		case '"':
		case '\'':
			if (!paren)
			{
				for (q = t; *s && *s != c; *t++ = *s++)
					if (*s == '\\' && *(s + 1))
						*t++ = *s++;
				if (*s)
					s++;
				*t = 0;
				t = q + stresc(q);
				continue;
			}
			break;
		case '%':
			if (flags && !(*flags & A_scope))
				*flags |= A_metarule;
			break;
		case '=':
			if (flags && !(*flags & (A_group|A_metarule)))
				*flags |= A_scope;
			break;
		case ',':
			if (flags)
				*flags |= A_group;
			break;
		default:
			if (paren || !isspace(c))
				break;
			/*FALLTHROUGH*/
		case 0:
			*t = 0;
			if (!c)
				s--;
			*buf = s;
			if (flags && !(*flags & (A_nooptions|A_scope)))
				*flags |= A_nooptions;
			return a;
		}
		*t++ = c;
	}
}

/*
 * list explanation
 */

void
explain(int level, ...)
{
	va_list	ap;

	va_start(ap, level);
	errorv(NiL, state.explain ? 0 : EXPTRACE, ap);
	va_end(ap);
}
