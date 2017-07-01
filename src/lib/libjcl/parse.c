/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * jcl step parser from the description in
 * http://publibz.boulder.ibm.com/cgi-bin/bookmgr_OS390/BOOKS/iea2b600/
 */

#include "jcllib.h"

#include <error.h>
#include <tm.h>

#define DDLIB(s)	(streq((s),"JCLLIB")||streq((s),"JOBLIB")||streq((s),"PROCLIB"))
#define ID1(c)		(isalpha(c)||(c)=='_'||(c)=='$'||(c)=='@'||(c)=='#')
#define ID(c)		(isalnum(c)||(c)=='_'||(c)=='$'||(c)=='@'||(c)=='#')

static char		null_data[2];
static char*		null = &null_data[1];

static char		dummy[] = "/dev/null";

/* vendor SS in the standard namespace -- great */

#undef	SS
#undef	ST

static char		EQ[] = "=";
static char		END[] = "\\\\";
static char		SS[] = "//";
static char		ST[] = "*";

/*
 * common syntax error message
 */

static void
syntax(Jcl_t* jcl, int level, char* token, char* expected, char* type)
{
	if (jcl->disc->errorf && (level > 1 || (jcl->flags & JCL_WARN)))
	{
		if (expected)
			(*jcl->disc->errorf)(NiL, jcl->disc, level, "%s: %s expected", token ? token : "EOF", expected);
		else if (token)
			(*jcl->disc->errorf)(NiL, jcl->disc, level, "%s: unknown %s", token, type ? type : "keyword");
		else
			(*jcl->disc->errorf)(NiL, jcl->disc, level, "unexpected EOF");
	}
}

/*
 * push a string on the input token stream
 */

static int
push(Jcl_t* jcl, char* s)
{
	if (jcl->pushed >= elementsof(jcl->push))
	{
		if (jcl->disc->errorf)
			(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: too much pushback", s);
		return -1;
	}
	memcpy(&jcl->push[jcl->pushed++], &jcl->data, sizeof(Push_t));
	memset(&jcl->data, 0, sizeof(Push_t));
	jcl->data = s;
	return 0;
}

/*
 * push back one token or entire card if tok==0
 */

static void
xel(register Jcl_t* jcl, register char* tok)
{
	if (tok)
	{
		jcl->peekpeek = jcl->peek;
		jcl->peek = tok;
	}
	else
	{
		jcl->data = strcpy(jcl->card, jcl->record ? jcl->record : null);
		jcl->peek = jcl->peekpeek = 0;
		jcl->last = 0;
	}
}

/*
 * read the next input card
 */

static char*
card(register Jcl_t* jcl)
{
	register char*	s;
	size_t		n;

	for (;;)
	{
		if (s = jcl->peekcard)
		{
			jcl->peekcard = 0;
			break;
		}
		if (!jcl->sp)
			break;
		if (s = sfgetr(jcl->sp, '\n', 1))
		{
			error_info.line++;
			if (!jcl->canon && (n = sfvalue(jcl->sp)) > 1)
			{
				if (s[n - 1] == '\r')
					s[n - 1] = 0;
				if (n > CARD)
					s[CARD] = 0;
			}
			message((-6, "card    %4d  %s", error_info.line, s));
			break;
		}
		if (!jcl->include->prev || jclpop(jcl) <= 0)
			break;
	}
	return s;
}

/*
 * expand control-m variable s with leading %% stripped
 * pointer to next char in s returned
 */

static char*
autoexpand(register Jcl_t* jcl, register char* s, Sfio_t* sp)
{
	register char*	t;
	register char*	v;
	register int	c;
	int		o;
	Sfio_t*		tp;

	if (*s == '.')
		return s + 1;
	tp = 0;
	o = sfstrtell(jcl->vp);
	sfputr(jcl->vp, JCL_AUTO, -1);
	for (;;)
	{
		if (!(c = *s++) || c == '.')
		{
			s--;
			break;
		}
		else if (c == '%' && *s == '%')
		{
			if (*++s == '.')
			{
				s++;
				break;
			}
			if (!tp && !(tp = sfstropen()))
				nospace(jcl, NiL);
			s = autoexpand(jcl, s, tp);
			if (!(t = sfstruse(tp)))
				nospace(jcl, NiL);
			if (!ID(*t))
				break;
			sfputr(jcl->vp, t, -1);
			*t = 0;
		}
		else if (ID(c))
			sfputc(jcl->vp, c);
		else
		{
			s--;
			break;
		}
	}
	sfputc(jcl->vp, 0);
	t = sfstrseek(jcl->vp, o, SEEK_SET);
	if (jcl->flags & JCL_PARAMETERIZE)
		sfprintf(sp, "${%s}", t);
	else
	{
		if (jcl->flags & JCL_LISTAUTOEDITS)
			uniq(t + sizeof(JCL_AUTO) - 1, NiL, 0, jcl->disc);
		if (v = lookup(jcl, t, NiL, 0, DEFAULT|MUST))
			sfputr(sp, v, -1);
		else
			sfprintf(sp, "%%%%%s.", t);
	}
	if (tp)
	{
		sfputr(sp, sfstrbase(tp), -1);
		sfstrclose(tp);
	}
	return s;
}

/*
 * set *r to point to the next token in s
 * pointer to next char in s returned
 */

static char*
autotoken(register Jcl_t* jcl, register char* s, char** r, int set)
{
	char*	t;
	int	o;

	o = sfstrtell(jcl->tp);
	while (*s == ' ')
		s++;
	for (t = s; *s; s++)
		if (*s == ' ')
		{
			*s++ = 0;
			break;
		}
		else if (*s == '%' && *(s + 1) == '%')
		{
			if (set)
			{
				set = 0;
				s++;
			}
			else
			{
				if (s - t)
					sfwrite(jcl->tp, t, s - t);
				s = autoexpand(jcl, s + 2, jcl->tp);
				if (*s != '.')
				{
					sfputc(jcl->tp, 0);
					t = fmtbuf(sfstrtell(jcl->tp) - o);
					strcpy(t, sfstrseek(jcl->tp, o, SEEK_SET));
					break;
				}
				t = s + 1;
			}
		}
	*r = t;
	return s;
}

/*
 * set *p to point to the next number token value in s
 * pointer to next char in s returned
 */

static char*
autonumber(register Jcl_t* jcl, char* s, long* r)
{
	char*	t;
	char*	b;

	while (*s == ' ')
		s++;
	b = s;
	*r = strtol(s, &t, 10);
	if (s == t && jcl->disc->errorf)
		(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: invalid control-m numeric operand", b);
	return t;
}

#define NOOPERAND()	do { \
				if (p) \
				{ \
					p = sfstrseek(jcl->tp, o, SEEK_SET); \
					if (jcl->disc->errorf) \
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: %s: left operand not expected", f, p); \
				} \
			} while (0)

#define OPERAND()	do { \
				if (p) \
				{ \
					p = 0; \
					t = sfstrseek(jcl->tp, o, SEEK_SET); \
				} \
				else \
				{ \
					t = ""; \
					if (jcl->disc->errorf) \
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: left operand expected", f); \
				} \
			} while (0)

/*
 * expand control-m expression from s into sp
 * pointer to next char in s returned
 */

static char*
autoeval(register Jcl_t* jcl, register char* s, char** r)
{
	char*	b;
	char*	e;
	char*	f;
	char*	p;
	char*	t;
	char*	v;
	int	o;
	int	y;
	long	i;
	long	j;
	time_t	x;
	Tm_t*	tm;

	o = sfstrtell(jcl->tp);
	p = 0;
	for (;;)
	{
		while (*s == ' ')
			s++;
		if (!*s)
			break;
		if (*s == '%' && *(s + 1) == '%')
		{
			b = s += 2;
			if (*s == '$')
			{
				s++;
				y = 1;
			}
			else
				y = 0;
			for (f = s; ID(*s); s++);
			if (*s)
				*s++ = 0;
			switch (*f)
			{
			case 'C':
				if (streq(f, "CALCDATE") || streq(f, "CALCDTE"))
				{
					NOOPERAND();
					s = autotoken(jcl, s, &t, 0);
					s = autonumber(jcl, s, &i);
					x = tmscan(t, &e, y ? "%Y%m%d" : "%y%m%d", &v, NiL, 0);
					if (jcl->disc->errorf && (*e || *v))
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: %s: invalid control-m date operand", f, t);
					tm = tmmake(&x);
					tm->tm_mday += i;
					x = tmtime(tm, 0);
					sfputr(jcl->tp, fmttime(y ? "%Y%m%d" : "%y%m%d", x), -1);
					goto done;
				}
				break;
			case 'G':
				if (streq(f, "GREG"))
				{
					NOOPERAND();
					s = autotoken(jcl, s, &t, 0);
					x = tmscan(t, &e, y ? "%Y%j" : "%y%j", &v, NiL, 0);
					if (jcl->disc->errorf && (*e || *v))
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: %s: invalid control-m date operand", f, t);
					sfputr(jcl->tp, fmttime(y ? "%Y%m%d" : "%y%m%d", x), -1);
					goto done;
				}
				break;
			case 'J':
				if (streq(f, "JULIAN"))
				{
					NOOPERAND();
					s = autotoken(jcl, s, &t, 0);
					x = tmscan(t, &e, y ? "%Y%m%d" : "%y%m%d", &v, NiL, 0);
					if (jcl->disc->errorf && (*e || *v))
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: %s: invalid control-m date operand", f, t);
					sfputr(jcl->tp, fmttime(y ? "%Y%j" : "%y%j", x), -1);
					goto done;
				}
				break;
			case 'L':
				if (streq(f, "LEAP"))
				{
					NOOPERAND();
					s = autotoken(jcl, s, &t, 0);
					x = tmscan(t, &e, y ? "%Y%m%d" : "%y%m%d", &v, NiL, 0);
					if (jcl->disc->errorf && (*e || *v))
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: %s: invalid control-m date operand", f, t);
					sfprintf(jcl->tp, "%d", tmisleapyear((int)strtol(fmttime("%Y", x), NiL, 10)));
					goto done;
				}
				break;
			case 'M':
				if (streq(f, "MINUS"))
				{
					NOOPERAND();
					autonumber(jcl, t, &i);
					s = autonumber(jcl, s, &j);
					sfprintf(jcl->tp, "%ld", i - j);
					goto done;
				}
				break;
			case 'P':
				if (streq(f, "PLUS"))
				{
					OPERAND();
					autonumber(jcl, t, &i);
					s = autonumber(jcl, s, &j);
					sfprintf(jcl->tp, "%ld", i + j);
					goto done;
				}
				break;
			case 'S':
				if (streq(f, "SUBSTR"))
				{
					NOOPERAND();
					s = autotoken(jcl, s, &t, 0);
					s = autonumber(jcl, s, &i);
					s = autonumber(jcl, s, &j);
					while (--i > 0 && *t)
						t++;
					for (v = t; j-- > 0 && *v; v++);
					sfwrite(jcl->tp, t, v - t);
					goto done;
				}
				break;
			case 'W':
				if (streq(f, "WCALC"))
				{
					NOOPERAND();
					goto notyet;
				}
				else if (streq(f, "WEEK#"))
				{
					NOOPERAND();
					s = autotoken(jcl, s, &t, 0);
					x = tmscan(t, &e, y ? "%Y%m%d" : "%y%m%d", &v, NiL, 0);
					if (jcl->disc->errorf && (*e || *v))
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: %s: invalid control-m date operand", f, t);
					sfputr(jcl->tp, fmttime("%W", x), -1);
					goto done;
				}
				else if (streq(f, "WEEKDAY"))
				{
					NOOPERAND();
					s = autotoken(jcl, s, &t, 0);
					x = tmscan(t, &e, y ? "%Y%m%d" : "%y%m%d", &v, NiL, 0);
					if (jcl->disc->errorf && (*e || *v))
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: %s: invalid control-m date operand", f, t);
					sfputr(jcl->tp, fmttime("%w", x), -1);
					goto done;
				}
				break;
			}
			if (p)
				sfputc(jcl->tp, ' ');
			s = autoexpand(jcl, b, jcl->tp);
		}
		else
		{
			if (p)
				sfputc(jcl->tp, ' ');
			s = autotoken(jcl, s, &p, 0);
			sfputr(jcl->tp, p, -1);
		}
		sfputc(jcl->tp, 0);
		sfstrseek(jcl->tp, -1, SEEK_CUR);
		p = sfstrbase(jcl->tp) + o;
	}
 done:
	sfputc(jcl->tp, 0);
	*r = fmtbuf(sfstrtell(jcl->tp) - o);
	strcpy(*r, sfstrseek(jcl->tp, o, SEEK_SET));
	return s;
 notyet:
	if (jcl->disc->errorf)
		(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s: control-m function not implemented", f);
	return s;
}

#define CMD_DD		2
#define CMD_EXEC	4

/*
 * return the next token
 */

static char*
lex(register Jcl_t* jcl)
{
	register char*	s;
	register char*	t;
	register char*	p;
	register int	n;
	register int	a;
	register int	q;
	register int	x;
	register int	c;
	register int	m;
	char*		v;
	char*		w;
	int		j;
	int		e;
#if DEBUG
	long		line;
#endif

	if (s = jcl->peek)
	{
		jcl->peek = jcl->peekpeek;
		jcl->peekpeek = 0;
		goto token;
	}
 again:
	while (*jcl->data == 0)
	{
		if (jcl->pushed)
		{
			memcpy(&jcl->data, &jcl->push[--jcl->pushed], sizeof(Push_t));
			if (jcl->last == END)
			{
				s = END;
				goto token;
			}
		}
		else if (jcl->data != null)
		{
			jcl->data = null;
			s = END;
			goto token;
		}
		else
		{
			a = 0;
			e = 0;
			j = 0;
			n = 0;
			q = 0;
			x = 0;
			for (;;)
			{
				if (!(s = card(jcl)))
				{
					if (x && jcl->disc->errorf)
						(*jcl->disc->errorf)(NiL, jcl->disc, 2, "unexpected EOF in continuation");
					jcl->data = null;
					return 0;
				}
				if (!sfstrtell(jcl->cp))
					line = error_info.line;
				if (*s == '/' && (*(s + 1) == '*' || *(s + 1) == '/' && *(s + 2) == '*'))
				{
					if (*++s == '/')
						s++;
					if (*s == '*')
						s++;
					while (*s == ' ')
						s++;
					if (*s == '%' && *(s + 1) == '%')
					{
						for (t = s += 2; *s && *s != ' '; s++);
						if (*s)
							for (*s++ = 0; *s == ' '; s++);
						if (streq(t, "SET") && *s == '%' && *(s + 1) == '%')
						{
							for (s = autotoken(jcl, s, &v, 1); *s == ' '; s++);
							if (*s == '=')
							{
								while (*++s == ' ');
								s = autoeval(jcl, s, &w);
								jclsym(jcl, v, w, 0);
								t = 0;
							}
						}
						if (t && jcl->disc->errorf)
							(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: %s: not implemented", t - 2, s);
					}
					if (x < 0)
						break;
					continue;
				}
				if (*s != '/' || *(s + 1) != '/')
				{
					if (jcl->disc->errorf)
						(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%-.*s...: invalid card: // expected", 8, s);
					return 0;
				}
				m = sfvalue(jcl->sp) > CARD && !isspace(s[CARD-1]) && !jcl->canon;
				if (x)
				{
					if (*(s += 2))
					{
						if (!isspace(*s))
						{
							jcl->peekcard = s - 2;
							break;
						}
						while (isspace(*++s));
					}
					if (x < 1)
					{
						if (m)
							continue;
						break;
					}
					if (*s == 0)
					{
						x = 0;
						continue;
					}
				}
				p = 0;
				for (;;)
				{
					switch (c = *s++)
					{
					case 0:
						if (p)
						{
							s = p;
							p = 0;
							continue;
						}
						break;
					case '=':
						if (!q && *s == '\'' && *(s + 1) == '\'' && (*(s + 2) == ',' || isspace(*(s + 2)) || *(s + 2) == 0))
							s += 2;
						sfputc(jcl->cp, c);
						if (!q)
							switch (j)
							{
							case CMD_DD:
								if (*(s - 2) == 'P' && *(s - 3) == 'M' && *(s - 4) == 'A' && !ID(*(s - 5)) ||
								    *(s - 2) == 'H' && *(s - 3) == 'T' && *(s - 4) == 'A' && *(s - 5) == 'P' && !ID(*(s - 6)) ||
								    *(s - 2) == 'S' && *(s - 3) == 'Y' && *(s - 4) == 'S' && *(s - 5) == 'B' && *(s - 6) == 'U' && *(s - 7) == 'T' && !ID(*(s - 8)))
									e = 1;
								break;
							case CMD_EXEC:
								if (*(s - 2) == 'T' && *(s - 3) == 'C' && *(s - 4) == 'C' && *(s - 5) == 'A' && !ID(*(s - 6)) ||
								    *(s - 2) == 'M' && *(s - 3) == 'R' && *(s - 4) == 'A' && *(s - 5) == 'P' && !ID(*(s - 6)))
									e = 1;
								break;
							}
						continue;
					case '\'':
						if (!p)
						{
							if (!q)
								q = e ? -1 : 1;
							else if (*s == '\'')
							{
								s++;
								sfputc(jcl->cp, c);
							}
							else
								q = 0;
						}
						sfputc(jcl->cp, c);
						continue;
					case '&':
						if (p)
							sfputc(jcl->cp, c);
						else if (*s == '&')
						{
							s++;
							if (q > 0)
								sfputc(jcl->cp, c);
							sfputc(jcl->cp, c);
						}
						else if (!ID(*s))
							sfputc(jcl->cp, c);
						else
						{
							for (t = s; ID(*s); s++);
							c = *s;
							*s = 0;
							v = lookup(jcl, t, NiL, 0, MUST);
							*s = c;
							if (v)
							{
								if (c == '.')
									s++;
								p = s;
								s = v;
							}
							else
								for (t--; t < s; t++)
									sfputc(jcl->cp, *t);
						}
						continue;
					case '%':
						if (p || *s != '%' || *(s + 1) == '#')
							sfputc(jcl->cp, c);
						else
						{
							p = autotoken(jcl, s - 1, &v, 0);
							s = v;
						}
						continue;
					default:
						if (!q)
						{
							if (e && !ID(c))
								e = 0;
							if (isspace(c))
							{
								if ((x = sfstrtell(jcl->cp)) && *(t = sfstrbase(jcl->cp) + x - 1) == ' ')
									continue;
								if (!n && ++a >= 3)
									break;

								/*
								 * { CMD_DD IF } don't follow the norm
								 */

								if (a == 2)
								{
									/*
									 * a DD may override/augment DD in another procedures so
									 * variable expansion must be delayed until the other
									 * procedure context is active
									 */

									if (*t == 'D' && *(t - 1) == 'D' && *(t - 2) == ' ')
									{
										j = CMD_DD;
										if (memchr(sfstrbase(jcl->cp), '.', x))
											p = null;
									}
									else if (*t == 'C' && *(t - 1) == 'E' && *(t - 2) == 'X' && *(t - 3) == 'E' && *(t - 4) == ' ')
										j = CMD_EXEC;
									else if (*t == 'F' && *(t - 1) == 'I' && *(t - 2) == ' ')
										n = 1;
								}
								c = ' ';
							}
						}
						sfputc(jcl->cp, c);
						continue;
					}
					break;
				}
				if (q)
					x = 1;
				else
				{
					t = sfstrbase(jcl->cp);
					for (s = sfstrseek(jcl->cp, 0, SEEK_CUR); s > t && *(s - 1) == ' '; s--);
					sfstrseek(jcl->cp, s - t, SEEK_SET);
					if (a < 2 || s == t || *(s - 1) != ',')
					{
						if (!m)
							break;
						x = -1;
					}
					else
						x = 1;
				}
			}
			if (!(jcl->data = jcl->card = sfstruse(jcl->cp)))
				nospace(jcl, NiL);
			sfputr(jcl->rp, jcl->card, -1);
			if (!(jcl->record = sfstruse(jcl->rp)))
				nospace(jcl, NiL);
			message((-4, "record  %4d  %s", line, jcl->record));
		}
	}
	s = jcl->data;
	n = 0;
	q = 0;
	for (;;)
	{
		switch (*jcl->data++)
		{
		case 0:
			jcl->data--;
			if (q)
			{
				if (jcl->disc->errorf)
					(*jcl->disc->errorf)(NiL, jcl->disc, 2, "unbalanced '...'");
				return 0;
			}
			if (n)
			{
				if (jcl->disc->errorf)
					(*jcl->disc->errorf)(NiL, jcl->disc, 2, "unbalanced (...)");
				return 0;
			}
			goto token;
		case '/':
			if (s == jcl->card)
			{
				if (*jcl->data == '/')
				{
					jcl->data++;
					if (*jcl->data == '*')
					{
						jcl->data = null;
						goto again;
					}
					if (*jcl->data == ' ' || *jcl->data == 0)
					{
						xel(jcl, null);
						if (*jcl->data == ' ')
							jcl->data++;
						else
							xel(jcl, null);
					}
					s = SS;
					goto token;
				}
				else if (*jcl->data == '*')
				{
					jcl->data = null;
					goto again;
				}
			}
			continue;
		case '=':
			if (!n && !q && jcl->last != EQ)
			{
				if (*jcl->data == '\'' && *(jcl->data + 1) == '\'' && (*(jcl->data + 2) == ',' || isspace(*(jcl->data + 2)) || *(jcl->data + 2) == 0))
				{
					*(jcl->data - 1) = 0;
					jcl->data += 2;
					xel(jcl, null);
					if ((jcl->data - s) == 3)
					{
						s = EQ;
						goto token;
					}
				}
				else
				{
					if (*jcl->data == ',' || *jcl->data == ' ' || *jcl->data == 0)
						xel(jcl, null);
					if ((jcl->data - s) == 1)
					{
						s = EQ;
						goto token;
					}
				}
				xel(jcl, EQ);
				break;
			}
			continue;
		case '*':
			if (!n && !q && (jcl->data - s) == 1 && (*jcl->data == ',' || *jcl->data == ' ' || *jcl->data == 0))
			{
				s = ST;
				goto token;
			}
			continue;
		case '(':
			if (!q)
				n++;
			continue;
		case ')':
			if (!q)
			{
				if (n > 0)
					n--;
				else if (jcl->pushed && *jcl->data == 0)
					break;
			}
			continue;
		case ',':
			if (!n && !q)
			{
				if ((jcl->data - s) > 1)
					break;
				s = jcl->data;
			}
			continue;
		case ' ':
			if (!n && !q)
				break;
			continue;
		case '\'':
			if (!q)
				q = 1;
			else if (*jcl->data == '\'')
				jcl->data++;
			else
				q = 0;
			s++;
			t = jcl->data;
			while (--t >= s)
				*t = *(t - 1);
			continue;
		default:
			continue;
		}
		break;
	}
	*(jcl->data - 1) = 0;
 token:
	message((-8, "lex     %4d  %s", error_info.line, s));
	return jcl->last = s;
}

/*
 * eat the remainder of the card and its continuations
 */

static void
eat(Jcl_t* jcl)
{
	register char*	tok;

	if (jcl->last != END)
		while ((tok = lex(jcl)) && tok != END);
}

/*
 * parse a COND [sub]expression
 */

static Jclcond_t*
cond(register Jcl_t* jcl, char* b, char** p)
{
	register char*		s;
	register char*		t;
	register Jclcond_t*	x;
	register Jclcond_t*	y;
	char*			e;

	s = b;
	y = 0;
	for (;;)
	{
		if (*s == '(')
		{
			if (!(x = cond(jcl, s + 1, &e)))
				return 0;
			s = e;
		}
		else if (!(x = vmnewof(jcl->vs, NiL, Jclcond_t, 1, 0)))
		{
			nospace(jcl, NiL);
			return 0;
		}
		else
		{
			x->code = (short)strtol(s, &e, 10);
			if (*e == ',')
			{
				s = e + 2;
				switch (*(e + 1))
				{
				case 'E':
					switch (*s)
					{
					case 'Q':
						x->op = JCL_COND_EQ;
						break;
					default:
						goto bad;
					}
					break;
				case 'G':
					switch (*s)
					{
					case 'E':
						x->op = JCL_COND_GE;
						break;
					case 'T':
						x->op = JCL_COND_GT;
						break;
					default:
						goto bad;
					}
					break;
				case 'L':
					switch (*s)
					{
					case 'E':
						x->op = JCL_COND_LE;
						break;
					case 'T':
						x->op = JCL_COND_LT;
						break;
					default:
						goto bad;
					}
					break;
				case 'N':
					switch (*s)
					{
					case 'E':
						x->op = JCL_COND_NE;
						break;
					default:
						goto bad;
					}
					break;
				default:
					goto bad;
				}
				if (*++s == ',')
				{
					for (t = ++s; *s && *s != ',' && *s != ')'; s++);
					if (!(x->step = vmnewof(jcl->vs, 0, char, s - t, 1)))
					{
						nospace(jcl, NiL);
						return 0;
					}
					memcpy(x->step, t, s - t);
				}
			}
			else
			{
				for (t = s; *s && *s != ',' && *s != ')'; s++);
				if ((s - t) != 4)
					goto bad;
				if (strneq(t, "EVEN", 4))
					x->op = JCL_COND_EVEN;
				else if (strneq(t, "ONLY", 4))
					x->op = JCL_COND_ONLY;
				else
					goto bad;
			}
		}
		if (y)
			y->next = x;
		else
			y = x;
		if (*s == 0)
			break;
		if (*s == ')')
		{
			s++;
			break;
		}
		if (*s++ != ',')
			goto bad;
	}
	if (p)
		*p = s;
	else if (*s)
		goto bad;
	return y;
 bad:
	syntax(jcl, 2, b, "(CODE,LT|LE|EQ|NE|GE|GT[,STEP])|EVEN|ONLY", NiL);
	return 0;
}

/*
 * return next arg token
 * *p points to optional =val
 */

static char*
arg(Jcl_t* jcl, char** p)
{
	char*	tok;
	char*	val;

	if (!(tok = lex(jcl)))
		syntax(jcl, 2, NiL, NiL, NiL);
	else if (tok == END)
		return 0;
	else if ((val = lex(jcl)) == EQ)
		val = lex(jcl);
	else
	{
		xel(jcl, val);
		val = 0;
	}
	*p = val;
	return tok;
}

/*
 * return next parm from single token or (...,...,...) list *p
 * list modified in place and not restored
 */

char*
jclparm(char** p)
{
	register char*	s;
	register char*	t;
	register int	q;
	register int	n;
	register int	x;
	int		empty;
	char*		b;

	s = *p;
	if (*s == '(')
	{
		s++;
		x = -1;
	}
	else
		x = '\'';
	q = 0;
	n = 0;
	for (b = t = s; *s; s++)
		if (*s == x)
			q = !q;
		else if (!q)
		{
			if (*s == '(')
				n++;
			else if (*s == ')')
			{
				if (--n < 0)
					break;
			}
			else if (!n && *s == ',')
				break;
			*t++ = *s;
		}
	if (*s)
	{
		empty = *s == ',';
		*s++ = 0;
	}
	else
		empty = 0;
	*p = s;
	if (*b == ' ')
	{
		s = b;
		for (s = b; *++s == ' ';);
		for (t = s; ID(*s); s++);
		if (*s == '=')
			b = t;
	}
	if (!*b && !empty)
		return 0;
	return b;
}

/*
 * parse a DD statement
 */

static int
DD(register Jcl_t* jcl, register Jclstep_t* step, char* name)
{
	register char*		tok;
	register char*		op;
	register Jcldd_t*	dd;
	register Jclcat_t*	cat;
	char*			val;
	char*			s;
	Jcldd_t*		pd;
	Jclout_t*		out;
	Jcloutput_t*		output;
	int			i;
	int			n;
	int			d0;
	int			d1;
	int			d2;
	int			change;

	if (!step->name && !jcl->lastdd && !DDLIB(name))
	{
		if (jcl->disc->errorf && (jcl->flags & JCL_WARN))
			(*jcl->disc->errorf)(NiL, jcl->disc, 1, "//%s DD appears before EXEC", name);
		return -1;
	}
	change = 0;
	if (*name)
	{
		if (dd = (Jcldd_t*)dtmatch(step->dd, name))
		{
			jcl->lastdd = dd;
			change = 1;
		}
		else if (!(dd = vmnewof(jcl->vs, NiL, Jcldd_t, 1, 0)))
		{
			nospace(jcl, NiL);
			return -1;
		}
		else if (!(dd->name = stash(jcl, jcl->vs, name, 0)))
			return -1;
		else if (tok = strchr(dd->name, '.'))
		{
			dd->reference = tok - dd->name;
			dd->flags |= JCL_DD_REFERENCE;
		}
		else if (streq(name, "SYSIN"))
		{
			dd->flags |= JCL_DD_SYSIN;
			if (!dd->disp[0])
				dd->disp[0] = JCL_DISP_OLD;
		}
		else if (streq(name, "SYSOUT"))
		{
			dd->flags |= JCL_DD_SYSOUT;
			if (!dd->disp[0])
				dd->disp[0] = JCL_DISP_NEW;
		}
		else if (streq(name, "SYSERR"))
		{
			dd->flags |= JCL_DD_SYSERR;
			if (!dd->disp[0])
				dd->disp[0] = JCL_DISP_NEW;
		}
		else if (DDLIB(name))
			dd->flags |= JCL_DD_INCLUDE;
	}
	else if (!(dd = jcl->lastdd))
	{
		syntax(jcl, 2, name, "NAME", NiL);
		return -1;
	}
	d0 = '/';
	d1 = '/';
	d2 = '*';
	while (tok = arg(jcl, &val))
	{
		if (val && (val[0] == '*' && val[1] == '.' || val[0] == '(' && val[1] == '*' && val[2] == '.'))
		{
			op = val;
			if (*op == '(')
				op++;
			op += 2;
			if (streq(tok, "OUTPUT"))
			{
				for (;;)
				{
					if (*op == '*')
						op++;
					if (*op == '.')
						op++;
					for (val = op; *op && *op != ',' && *op != ')'; op++);
					if (n = *op)
						*op = 0;
					if (*val)
					{
						sfprintf(jcl->tp, "%s.%s", step->name, val);
						if (!(s = sfstruse(jcl->tp)))
							nospace(jcl, NiL);
						if (!(output = (Jcloutput_t*)dtmatch(jcl->output, s)))
							output = (Jcloutput_t*)dtmatch(jcl->output, val);
						if (output)
						{
							if (!(out = vmnewof(jcl->vs, NiL, Jclout_t, 1, 0)))
							{
								nospace(jcl, NiL);
								return -1;
							}
							out->output = output;
							out->next = dd->out;
							dd->out = out;
						}
						else if (jcl->disc->errorf && (jcl->flags & JCL_WARN))
							(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: OUTPUT not defined", val);
					}
					if (!n)
						break;
					*op++ = n;
					if (n != ',')
						break;
				}
				continue;
			}
			for (val = op; *op && *op != '.'; op++);
			if (*op)
			{
				*op = 0;
				if (!streq(val, step->name))
				{
					if (jcl->disc->errorf)
						(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: step back reference not supported", val);
					return -1;
				}
				*op++ = '.';
				val = op;
				op += strlen(op);
			}
			if (*--op == ')')
				*op = 0;
			else
				op = 0;
			pd = (Jcldd_t*)dtmatch(step->dd, val);
			if (op)
				*op = ')';
			if (!pd)
			{
				if (jcl->disc->errorf && (jcl->flags & JCL_WARN))
					(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: DD not defined", val);
				continue;
			}
		}
		else
			pd = 0;
		if (val && (streq(tok, "DSN") || streq(tok, "DSNAME")) || !val && (streq(tok, "DUMMY") && (val = dummy) && (dd->flags |= JCL_DD_DUMMY) || !jcl->pushed && tok != ST && (val = tok)))
		{
			if (dd->flags & JCL_DD_DUMMY)
			{
				dd->path = (char*)dummy;
				dd->cat = dd->last = 0;
			}
			else
			{
				if (pd)
					name = pd->path;
				else if (!(name = stash(jcl, jcl->vs, jclpath(jcl, val), 1)))
					return -1;
				if (!dd->path || change)
					dd->path = name;
				else if (!(cat = vmnewof(jcl->vs, NiL, Jclcat_t, 1, 0)))
				{
					nospace(jcl, NiL);
					return -1;
				}
				else
				{
					cat->path = name;
					if (dd->last)
						dd->last->next = cat;
					else
						dd->cat = cat;
					dd->last = cat;
				}
			}
		}
		else if (val)
		{
			if (streq(tok, "AVGREC"))
			{
				if (!(dd->recfm & (JCL_RECFM_F|JCL_RECFM_V)))
				{
					if (pd && (pd->recfm & (JCL_RECFM_F|JCL_RECFM_V)))
					{
						dd->recfm = pd->recfm;
						dd->lrecl = pd->lrecl;
					}
#if _2005_06_10__NOT_RELIABLE_
					else if (n = pd ? pd->space : dd->space)
					{
						dd->lrecl = n;
						dd->recfm |= JCL_RECFM_F;
					}
#endif
				}
			}
			else if (streq(tok, "DCB"))
			{
				if (pd)
				{
					memcpy(dd->disp, pd->disp, sizeof(dd->disp));
					dd->lrecl = pd->lrecl;
					dd->recfm = pd->recfm;
				}
				else if (push(jcl, val + (*val == '(')))
					return -1;
			}
			else if (streq(tok, "DDNAME"))
			{
				dd->flags |= JCL_DD_ALIAS;
				dd->path = stash(jcl, jcl->vs, pd ? ((pd->flags & JCL_DD_ALIAS) ? pd->path : pd->name) : val, 0);
			}
			else if (streq(tok, "DISP"))
			{
				if (pd)
					memcpy(dd->disp, pd->disp, sizeof(dd->disp));
				else if (tok = jclparm(&val))
				{
					if (streq(tok, "NEW"))
						dd->disp[0] = JCL_DISP_NEW;
					else if (streq(tok, "OLD"))
						dd->disp[0] = JCL_DISP_OLD;
					else if (streq(tok, "SHR"))
						dd->disp[0] = JCL_DISP_SHR;
					else if (streq(tok, "MOD"))
						dd->disp[0] = JCL_DISP_MOD;
					else if (*tok && jcl->disc->errorf && (jcl->flags & JCL_WARN))
						(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: unknown DISP", tok);
					for (i = 1; i < elementsof(dd->disp) && (tok = jclparm(&val)); i++)
					{
						if (streq(tok, "DELETE"))
							dd->disp[i] = JCL_DISP_DELETE;
						else if (streq(tok, "KEEP"))
							dd->disp[i] = JCL_DISP_KEEP;
						else if (streq(tok, "PASS"))
							dd->disp[i] = JCL_DISP_PASS;
						else if (streq(tok, "CATLG"))
							dd->disp[i] = JCL_DISP_CATLG;
						else if (streq(tok, "UNCATLG"))
							dd->disp[i] = JCL_DISP_UNCATLG;
						else if (*tok && jcl->disc->errorf && (jcl->flags & JCL_WARN))
							(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: unknown DISP", tok);
					}
				}
			}
			else if (streq(tok, "DLM"))
			{
				d0 = 0;
				if (d1 = val[0])
					d2 = val[1];
				else
					d2 = 0;
			}
			else if (streq(tok, "LRECL"))
				dd->lrecl = pd ? pd->lrecl : (int)strtol(val, NiL, 10);
			else if (streq(tok, "RECFM"))
			{
				if (pd)
					dd->recfm = pd->recfm;
				else
				{
					dd->recfm = 0;
					for (;;)
					{
						switch (*val++)
						{
						case 0:
							break;
						case 'A':
							dd->recfm |= JCL_RECFM_A;
							continue;
						case 'B':
							dd->recfm |= JCL_RECFM_B;
							continue;
						case 'D':
							dd->recfm |= JCL_RECFM_D;
							continue;
						case 'F':
							dd->recfm |= JCL_RECFM_F;
							continue;
						case 'M':
							dd->recfm |= JCL_RECFM_M;
							continue;
						case 'S':
							dd->recfm |= JCL_RECFM_S;
							continue;
						case 'U':
							dd->recfm |= JCL_RECFM_U;
							continue;
						case 'V':
							dd->recfm |= JCL_RECFM_V;
							continue;
						default:
							if (jcl->disc->errorf && (jcl->flags & JCL_WARN))
								(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%c: unknown RECFM", *(val - 1));
							continue;
						}
						break;
					}
				}
			}
			else if (streq(tok, "SPACE"))
			{
				if (pd)
				{
					dd->flags |= pd->flags & JCL_DD_DIR;
					dd->space = pd->space;
				}
				else
					for (n = 0;;)
					{
						switch (*val++)
						{
						case 0:
							break;
						case '(':
							n++;
							i = 0;
							continue;
						case ',':
							i++;
							continue;
						case ')':
							if (n == 2)
							{
								if (i == 2)
									dd->flags |= JCL_DD_DIR;
								break;
							}
							continue;
						default:
							if (!dd->space)
								dd->space = (int)strtol(val - 1, NiL, 10);
							continue;
						}
						break;
					}
			}
			else if (streq(tok, "SUBSYS"))
			{
				if (*val == '(' && push(jcl, val + 1))
					return -1;
			}
		}
		else if (tok == ST)
		{
			while (tok = card(jcl))
			{
				if (tok[0] == d1 && tok[1] == d2 || tok[0] == d0 && tok[1] == d1 && tok[2] == d2)
					break;
				sfputr(jcl->tp, tok, '\n');
			}
			if (!(s = sfstruse(jcl->tp)))
				nospace(jcl, NiL);
			if (!(dd->here = stash(jcl, jcl->vs, s, 0)))
				return -1;
			dd->dlm[0] = d1;
			dd->dlm[1] = d2;
			jcl->record = tok;
			xel(jcl, NiL);
			if (tok)
				xel(jcl, END);
		}
	}
#if 0
	if (dd->disp[0] == JCL_DISP_NEW && !dd->recfm && jcl->disc->errorf)
		(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: no DCB for NEW DD", dd->name);
#endif
	if ((dd->flags & JCL_DD_INCLUDE) && dd->path && jclinclude(jcl, dd->path, JCL_PROC, NiL))
		return -1;
	if (dd->reference)
	{
		if (dd->card)
			sfputr(jcl->tp, dd->card, -1);
		val = jcl->record;
		if (val[0] == '/' && val[1] == '/' && val[2] != ' ' && (op = strchr(val, '.')))
		{
			val = op + 1;
			sfputc(jcl->tp, '/');
			sfputc(jcl->tp, '/');
		}
		sfputr(jcl->tp, val, '\n');
		if (!(s = sfstruse(jcl->tp)))
			nospace(jcl, NiL);
		if (!(dd->card = stash(jcl, jcl->vs, s, 0)))
		{
			nospace(jcl, NiL);
			return -1;
		}
	}
	else if (dd != jcl->lastdd && (dd->path || dd->here))
	{
		if ((jcl->flags & JCL_MARKLENGTH) && dd->path && !dd->recfm && !dd->lrecl)
			marked(dd->path, dd, jcl->disc);
	}
	else if (!(dd->flags & JCL_DD_ALIAS))
		return 0;
	jcl->lastdd = dd;
	dtinsert(step->dd, dd);
	return 0;
}

/*
 * parse an OUTPUT statement
 */

static int
OUTPUT(register Jcl_t* jcl, register Jclstep_t* step, char* name)
{
	Jcloutput_t*	output;
	char*		s;
	char*		tok;
	char*		val;

	if (!*name)
	{
		syntax(jcl, 2, NiL, "NAME", NiL);
		return -1;
	}
	if (step->name)
	{
		sfprintf(jcl->tp, "%s.%s", step->name, name);
		if (!(name = sfstruse(jcl->tp)))
			nospace(jcl, NiL);
	}
	if (output = (Jcloutput_t*)dtmatch(jcl->output, name))
	{
		if (jcl->disc->errorf)
			(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: OUTPUT alread defined", name);
		return 0;
	}
	if (!(output = vmnewof(jcl->vs, NiL, Jcloutput_t, 1, 0)))
	{
		nospace(jcl, NiL);
		return 0;
	}
	else if (!(output->name = stash(jcl, jcl->vs, name, 0)))
		return 0;
	while (tok = arg(jcl, &val))
	{
		sfprintf(jcl->tp, ",%s", tok);
		if (val)
			sfprintf(jcl->tp, "=%s", val);
	}
	if (!(s = sfstruse(jcl->tp)))
		nospace(jcl, 0);
	if (!(output->parm = stash(jcl, jcl->vs, s, 0)))
		return 0;
	dtinsert(jcl->output, output);
	return 0;
}

static int	eval(Jcl_t*, char*, char**, int);

/*
 * return the next IF expression operand value
 */

static int
operand(Jcl_t* jcl, register char* s, char** e)
{
	register char*		t;
	register char*		u;
	register char*		v;
	Rc_t*			p;
	int			c;
	int			d;
	int			n;
	int			abend;
	int			run;
	int			rc;

	while (*s == ' ')
		s++;
	if ((n = *s == '!') && *++s == ' ')
		s++;
	if (*s == '(')
	{
		if ((rc = eval(jcl, s + 1, e, 99)) < 0)
			return rc;
		s = *e;
		while (*s == ' ')
			s++;
		if (*s++ != ')')
		{
			if (jcl->disc->errorf)
				(*jcl->disc->errorf)(NiL, jcl->disc, 2, "unbalanced (...)");
			return -1;
		}
	}
	else if (isdigit(*s))
	{
		rc = (int)strtol(s, e, 10);
		s = *e;
	}
	else if (*s == 0 || *s == ')')
		rc = 0;
	else
	{
		v = 0;
		for (t = s; ID(*s) || *s == '.' && (v = s); s++);
		if (v)
		{
			*v = 0;
			p = (Rc_t*)dtmatch(jcl->rcs, t);
			*v = '.';
			if (p)
			{
				run = 1;
				if (p->rc < 0)
				{
					abend = 1;
					rc = 0;
				}
				else
				{
					abend = 0;
					rc = p->rc;
				}
			}
			else
			{
				abend = 0;
				rc = 0;
				run = 0;
			}
			t = v + 1;
		}
		else
		{
			abend = jcl->abend;
			rc = jcl->rc;
			run = 1;
		}
		c = *s;
		*s = 0;
		if (!*t)
			/*ok*/;
		else if (streq(t, "ABEND"))
			rc = abend;
		else if (streq(t, "ABENDCC"))
			rc = 0;
		else if (streq(t, "FALSE"))
			rc = 0;
		else if (streq(t, "RUN"))
			rc = run;
		else if (streq(t, "TRUE"))
			rc = 1;
		else if (*t == 'S' && (s - t) == 5 && (rc = (int)strtol(t + 1, e, 10), !*e))
			/*OK*/;
		else if (*t == 'U' && (s - t) == 4 && (rc = (int)strtol(t + 1, e, 16), !*e))
			/*OK*/;
		else if (!streq(t, "RC"))
		{
			if (c)
			{
				u = s;
				while (*++u == ' ');
				if (*u == '=' || *u == '!' && *(u + 1) == '=')
				{
					if (n = *u == '!')
						u++;
					while (*++u == ' ');
					v = u;
					while (*++u && *u != ' ');
					d = *u;
					*u = 0;
					rc = streq(s, v) != n;
					*u = d;
					*s = c;
					s = u;
					goto done;
				}
			}
			if (jcl->disc->errorf)
				(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: unknown IF expression token", t);
			return -1;
		}
		*s = c;
	}
 done:
	if (n)
		rc = !rc;
	while (*s == ' ')
		s++;
	*e = s;
	return rc;
}

/*
 * evaluate an IF [sub]expression
 */

static int
eval(register Jcl_t* jcl, register char* s, char** e, int prec)
{
	register char*	t;
	int		a;
	int		op;
	int		b;
#if DEBUG
	int		o;

	static const char*	opname[] = { "NOP", "ONLY", "EVEN", "LT", "LE", "EQ", "NE", "GE", "GT", "OR", "AND" };
#endif

	if ((a = operand(jcl, s, e)) < 0)
		return -1;
	s = *e;
	for (;;)
	{
		if (*s == 0 || *s == ')')
			break;
		op = 0;
		t = s;
		switch (*s++)
		{
		case 'A':
			if (*s++ == 'N' && *s++ == 'D')
				op = JCL_COND_AND;
			break;
		case 'E':
			if (*s++ == 'Q')
				op = JCL_COND_EQ;
			break;
		case 'G':
			switch (*s)
			{
			case 'E':
				op = JCL_COND_GE;
				s++;
				break;
			case 'T':
				op = JCL_COND_GT;
				s++;
				break;
			}
			break;
		case 'L':
			switch (*s)
			{
			case 'E':
				op = JCL_COND_LE;
				s++;
				break;
			case 'T':
				op = JCL_COND_LT;
				s++;
				break;
			}
			break;
		case 'N':
			if (*s++ == 'E')
				op = JCL_COND_NE;
			break;
		case 'O':
			if (*s++ == 'R')
				op = JCL_COND_OR;
			break;
		case '&':
			op = JCL_COND_AND;
			break;
		case '=':
			op = JCL_COND_EQ;
			break;
		case '>':
			if (*s == '=')
			{
				op = JCL_COND_GE;
				s++;
			}
			else
				op = JCL_COND_GT;
			break;
		case '<':
			if (*s == '=')
			{
				op = JCL_COND_LE;
				s++;
			}
			else
				op = JCL_COND_LT;
			break;
		case '!':
			if (*s++ == '=')
				op = JCL_COND_NE;
			break;
		case '|':
			op = JCL_COND_OR;
			break;
		}
		if (!op || isupper(*t) && ID(*s))
		{
			if (jcl->disc->errorf)
				(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: operator expected in IF expression", t);
			return -1;
		}
		if (op > prec)
		{
			s = t;
			break;
		}
		if ((b = eval(jcl, s, e, op)) < 0)
			return -1;
		s = *e;
		o = a;
		switch (op)
		{
		case JCL_COND_AND:
			a = a && b;
			break;
		case JCL_COND_OR:
			a = a || b;
			break;
		case JCL_COND_LT:
			a = a < b;
			break;
		case JCL_COND_LE:
			a = a <= b;
			break;
		case JCL_COND_EQ:
			a = a == b;
			break;
		case JCL_COND_NE:
			a = a != b;
			break;
		case JCL_COND_GE:
			a = a >= b;
			break;
		case JCL_COND_GT:
			a = a > b;
			break;
		}
		message((-7, "eval          %2d %2s %2d => %d", o, opname[op], b, a));
	}
	while (*s == ' ')
		s++;
	*e = s;
	return a;
}

/*
 * parse and evaluate an IF expression
 * return:
 *	<0 fatal error
 *	 0 false
 *	>0 true
 */

static int
IF(register Jcl_t* jcl)
{
	register char*	s;
	char*		e;
	int		r;

	while (s = lex(jcl))
		if (s == END)
		{
			if (jcl->disc->errorf)
				(*jcl->disc->errorf)(NiL, jcl->disc, 2, "THEN expected");
			return -1;
		}
		else if (streq(s, "THEN"))
			break;
		else
			sfputr(jcl->tp, s, ' ');
	sfstrseek(jcl->tp, -1, SEEK_CUR);
	if (!(s = sfstruse(jcl->tp)))
		nospace(jcl, NiL);
	if ((r = eval(jcl, s, &e, 99)) < 0)
		return r;
	if (*e)
	{
		if (jcl->disc->errorf)
			(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: invalid IF expression", e);
		return -1;
	}
	message((-5, "if      %4d  %s == %d", jcl->ie->line, sfstrbase(jcl->tp), r));
	return r;
}

/*
 * main parse loop
 */

static int
parse(register Jcl_t* jcl, register Jclstep_t* step)
{
	register char*		tok;
	register char*		name;
	register char*		op;
	char*			val;
	Ie_t*			ie;
	Sfio_t*			sp;
	char*			t;
	char*			v;
	int			i;

	while (tok = lex(jcl))
	{
		if (tok != SS)
		{
			syntax(jcl, 2, tok, SS, NiL);
			return -1;
		}
		if (!(name = lex(jcl)))
		{
			syntax(jcl, 2, NiL, "NAME", NiL);
			return -1;
		}
		if (!(op = lex(jcl)))
		{
			syntax(jcl, 2, NiL, "OP", NiL);
			return -1;
		}
		if (!*op)
			/*NOP*/;
		else if (streq(op, "DD"))
		{
			if (DD(jcl, step, name))
				return -1;
		}
		else if (streq(op, "ELSE"))
		{
			if (step->name || jcl->vs == jcl->vm)
			{
				xel(jcl, NiL);
				break;
			}
			if (!jcl->ie)
			{
				if (jcl->disc->errorf)
					(*jcl->disc->errorf)(NiL, jcl->disc, 2, "no IF for ELSE");
				return -1;
			}
			if (jcl->ie->flags & IE_KEEP)
				jcl->ie->flags |= IE_SKIP;
			else
				jcl->ie->flags |= IE_KEEP;
			while (arg(jcl, &val));
		}
		else if (streq(op, "ENDIF"))
		{
			if (!jcl->ie)
			{
				if (jcl->disc->errorf)
					(*jcl->disc->errorf)(NiL, jcl->disc, 2, "no IF for ENDIF");
				return -1;
			}
			if (step->name || jcl->vs == jcl->vm)
			{
				xel(jcl, NiL);
				break;
			}
			jcl->ie = jcl->ie->prev;
			while (arg(jcl, &val));
		}
		else if (streq(op, "EXEC"))
		{
			if (step->name || jcl->vs == jcl->vm)
			{
				xel(jcl, NiL);
				break;
			}
			if (!(step->name = stash(jcl, jcl->vs, name, 0)))
				return -1;
			while (tok = arg(jcl, &val))
			{
				if (val && (streq(tok, "PGM") && (step->flags |= JCL_PGM) || streq(tok, "PROC")) || !val && !step->command && (val = tok))
				{
					if (!(step->command = stash(jcl, jcl->vs, val, 0)))
						return -1;
					if (!(step->flags & JCL_PGM))
						step->flags |= JCL_PROC;
				}
				else if (val)
				{
					if (streq(tok, "COND"))
					{
						if (*val && !(step->cond = cond(jcl, val, NiL)))
							return -1;
					}
					else if (streq(tok, "PARM"))
					{
						if (*val && !(step->parm = stash(jcl, jcl->vs, val, 0)))
							return -1;
					}
					else if (!jclsym(jcl, tok, val, 0))
						return -1;
				}
			}
		}
		else if (streq(op, "IF"))
		{
			if (step->name || jcl->vs == jcl->vm)
			{
				xel(jcl, NiL);
				break;
			}
			if (!jcl->ie && !(ie = jcl->iefree) || jcl->ie && !(ie = jcl->ie->next))
			{
				if (!(ie = vmnewof(jcl->vm, 0, Ie_t, 1, 0)))
				{
					nospace(jcl, 0);
					return -1;
				}
				if (ie->prev = jcl->ie)
					jcl->ie->next = ie;
				else
					jcl->iefree = ie;
			}
			ie->line = error_info.line;
			jcl->ie = ie;
			if ((i = IF(jcl)) < 0)
				return -1;
			ie->flags = ie->prev && (ie->prev->flags & (IE_KEEP|IE_SKIP)) != IE_KEEP ? IE_SKIP : i ? IE_KEEP : 0;
		}
		else if (streq(op, "INCLUDE"))
		{
			while (tok = arg(jcl, &val))
				if (val && streq(tok, "MEMBER") && (!(tok = jclfind(jcl, val, JCL_PROC, 2, &sp)) || jclpush(jcl, sp, tok, 0)))
					return -1;
		}
		else if (streq(op, "JCLLIB"))
		{
			while (tok = arg(jcl, &val))
				if (val && streq(tok, "ORDER"))
					while (tok = jclparm(&val))
						if (jclinclude(jcl, tok, JCL_PROC, NiL))
							return -1;
		}
		else if (streq(op, "JOB"))
		{
			if (!(jcl->name = stash(jcl, jcl->vm, name, 0)))
				return -1;
			while (tok = arg(jcl, &val))
				if (val)
				{
					if (streq(tok, "COND"))
					{
						if (*val && !(jcl->cond = cond(jcl, val, NiL)))
							return -1;
					}
					else if (!lookup(jcl, tok, val, 0, DEFAULT))
						return -1;
				}
		}
		else if (streq(op, "OUTPUT"))
		{
			if (OUTPUT(jcl, step, name))
				return -1;
		}
		else if (streq(op, "PEND"))
		{
			/*HERE check for PROC on line 1, check that this is last? */
		}
		else if ((i = streq(op, "PROC") ? DEFAULT : 0) || streq(op, "SET"))
		{
			if (i && jcl->name)
			{
				sfprintf(jcl->vp, "(PROC)%s", name);
				sfputr(jcl->tp, jcl->record, '\n');
				while (tok = card(jcl))
				{
					if (sfvalue(jcl->sp) > 10 && tok[0] == '/' && tok[1] == '/' && tok[2] != '*')
					{
						for (val = tok + 2; isspace(*val); val++);
						if (strneq(val, "PEND", 4) && (!val[4] || isspace(val[4])))
							break;
					}
					sfputr(jcl->tp, tok, '\n');
				}
				if (!(v = sfstruse(jcl->vp)) || !(t = sfstruse(jcl->tp)))
					nospace(jcl, NiL);
				if (!lookup(jcl, v, t, 0, 0))
					return -1;
			}
			else
			{
				if (i && !(jcl->name = stash(jcl, jcl->vm, name, 0)))
					return -1;
				while (tok = arg(jcl, &val))
					if (val)
					{
						if (*tok == '?' && *(tok + strlen(tok) - 1) == '?')
						{
							if (streq(tok + 1, "ABEND?"))
								jcl->abend = (int)strtol(val, NiL, 0);
							else if (streq(tok + 1, "RC?"))
								jcl->rc = (int)strtol(val, NiL, 0);
						}
						else if (!lookup(jcl, tok, val, 0, i))
							return -1;
					}
			}
		}
		else
			syntax(jcl, 1, op, NiL, "OP");
		eat(jcl);
	}
	return 0;
}

/*
 * return the next jcl job step
 */

Jclstep_t*
jclstep(register Jcl_t* jcl)
{
	register Jclstep_t*	step;
	register Jcldd_t*	dd;
	register Jclcat_t*	cat;
	Jcl_t*			scope;
	Sfio_t*			sp;
	char*			ofile;
	int			oline;
	int			i;

	errno = 0;
	for (;;)
	{
		step = jcl->step;
		if (jcl->vs != jcl->vm)
		{
			vmclear(jcl->vs);
			dtclear(jcl->ss);
		}
		memset(step, 0, sizeof(*step));
		step->dd = jcl->ds;
		step->syms = jcl->ss;
		dtclear(step->dd);
		step->vm = jcl->vs;
		jcl->lastdd = 0;
		if (!jcl->data)
			jcl->data = null;
		if (parse(jcl, step))
			break;
		if (!step->name && jcl->vs != jcl->vm)
			break;
		if (jcl->ie && (jcl->ie->flags & (IE_KEEP|IE_SKIP)) != IE_KEEP)
			continue;
		if (step->name)
			for (scope = jcl->scope; scope; scope = scope->scope)
				for (dd = (Jcldd_t*)dtfirst(scope->step->dd); dd; dd = (Jcldd_t*)dtnext(scope->step->dd, dd))
					if (dd->reference && strneq(dd->name, step->name, dd->reference) && dd->name[dd->reference] == '.')
					{
						if (!jcl->dp && !(jcl->dp = sfstropen()) || sfstrbuf(jcl->dp, dd->card, strlen(dd->card), 0))
						{
							nospace(jcl, 0);
							return 0;
						}
						sp = jcl->sp;
						jcl->sp = jcl->dp;
						oline = error_info.line;
						ofile = error_info.file;
						error_info.line = 0;
						error_info.file = "DD-addition/override";
						jcl->canon++;
						i = parse(jcl, step);
						jcl->canon--;
						error_info.line = oline;
						error_info.file = ofile;
						jcl->sp = sp;
						if (i)
							return 0;
					}
		if (jcl->flags & JCL_MARKLENGTH)
			for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
				if (dd->path && (dd->recfm & (JCL_RECFM_F|JCL_RECFM_V)) && dd->lrecl && !(dd->flags & JCL_DD_DIR))
				{
					if (!streq(dd->path, dummy))
					{
						dd->path = mark(dd->path, dd->recfm, dd->lrecl, jcl->disc);
						dd->flags |= JCL_DD_MARKED;
					}
					for (cat = dd->cat; cat; cat = cat->next)
						if (!streq(cat->path, dummy))
							cat->path = mark(cat->path, 0, dd->lrecl, jcl->disc);
				}
		return step;
	}
	if (jcl->disc->errorf && jcl->eof && jcl->ie)
		(*jcl->disc->errorf)(NiL, jcl->disc, 2, "IF on line %d has no ENDIF", jcl->ie->line);
	return 0;
}

/*
 * return >0 if condition is true
 */

int
jcleval(Jcl_t* jcl, register Jclcond_t* cond, int code)
{
	if (!cond)
		return !code;
	while (cond)
	{
		switch (cond->op)
		{
		case JCL_COND_ONLY:
			if (jcl->failed)
				return 0;
			break;
		case JCL_COND_LT:
			if (cond->code < code)
				return 0;
			break;
		case JCL_COND_LE:
			if (cond->code <= code)
				return 0;
			break;
		case JCL_COND_EQ:
			if (cond->code == code)
				return 0;
			break;
		case JCL_COND_NE:
			if (cond->code != code)
				return 0;
			break;
		case JCL_COND_GE:
			if (cond->code >= code)
				return 0;
			break;
		case JCL_COND_GT:
			if (cond->code > code)
				return 0;
			break;
		}
		cond = cond->next;
	}
	return 1;
}

/*
 * set step return code
 * rc<0 for abend
 */

int
jclrc(register Jcl_t* jcl, register Jclstep_t* step, int rc)
{
	register Rc_t*		rp;

	/*
	 * map unix signal codes to abend
	 */

	if (rc > 128 && rc < 192)
		rc -= 128;
	else if (rc > 256 && rc < 320)
		rc -= 256;
	if (step && step->name)
	{
		if (!(rp = (Rc_t*)dtmatch(jcl->rcs, step->name)))
		{
			if (!(rp = vmnewof(jcl->vm, 0, Rc_t, 1, strlen(step->name))))
			{
				nospace(jcl, NiL);
				return -1;
			}
			strcpy(rp->name, step->name);
			dtinsert(jcl->rcs, rp);
		}
		if (rc < 0 && rp->rc > rc || rc > 0 && rp->rc < rc)
			rp->rc = rc;
		if (!jcl)
			return rp->rc;
	}
	if (jcl)
	{
		if (rc < 0)
			jcl->abend++;
		if (rc < 0 && jcl->rc > rc || rc > 0 && jcl->rc < rc)
			jcl->rc = rc;
		return jcl->rc;
	}
	return 0;
}
