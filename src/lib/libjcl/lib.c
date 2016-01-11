/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
 * jcl private library support
 */

#include "jcllib.h"

#include <tm.h>

static char*
internal(register Jcl_t* jcl, const char* name)
{
	register char*	s;
	register char*	t;

	if (streq(name, "CYCLE"))
		return tmmake(NiL)->tm_mday <= 10 ? "10" : "31";
	else if (streq(name, "JOBNAME"))
		return jcl->name;
	else if (streq(name, "USERID"))
		return getlogin();
	else if (streq(name, "VDATE"))
	{
		for (t = s = fmttime("%h%y", time(NiL)); *t; t++)
			if (islower(*t))
				*t = toupper(*t);
		return s;
	}
	return 0;
}

/*
 * return value for &name. variable
 * if value!=0 then value is set
 * if (set&DEFAULT) then no set if already defined
 * if (set&MUST) then error message if not found
 * no provision for delete
 */

char*
lookup(register Jcl_t* jcl, const char* name, const char* value, int flags, int set)
{
	register Jclsym_t*	v;
	register Jcl_t*		scope;
	char*			s;
	char*			b;
	time_t			t;
	int			j;
	int			f;

	if (!value)
	{
		if (jcl->step->syms && (v = (Jclsym_t*)dtmatch(jcl->step->syms, name)))
			goto found;
		for (scope = jcl; scope; scope = scope->scope)
		{
			if (scope->scope && scope->scope->step->syms && (v = (Jclsym_t*)dtmatch(scope->scope->step->syms, name)))
				goto found;
			if (scope->syms && (v = (Jclsym_t*)dtmatch(scope->syms, name)))
				goto found;
		}
	}
	else if (!jcl->syms)
		return 0;
	else if ((v = (Jclsym_t*)dtmatch(jcl->syms, name)) && (!(flags & (JCL_SYM_EXPORT|JCL_SYM_SET)) || (v->flags & (JCL_SYM_EXPORT|JCL_SYM_SET))))
	{
		if (!(set & DEFAULT))
			v->value = stash(jcl, jcl->vm, value, 0);
		goto found;
	}
	if (!value)
	{
		if (strneq(name, JCL_AUTO, sizeof(JCL_AUTO) - 1))
		{
			b = (char*)name + sizeof(JCL_AUTO) - 1;
			if ((s = getenv(name)) || (s = internal(jcl, b)))
				return s;
			s = b;
			if (*s == '$')
			{
				s++;
				f = 1;
			}
			else
				f = 0;
			switch (*s)
			{
			case 'O':
				s++;
				t = jcl->disc->odate;
				break;
			case 'R':
				s++;
				t = jcl->disc->rdate;
				break;
			default:
				t = jcl->disc->date;
				break;
			}
			if (s[0] == 'J' && s[1] == 'U' && s[2] == 'L')
			{
				s += 3;
				j = 1;
			}
			else
				j = 0;
			switch (*s)
			{
			case 'C':
				if (!strcmp(s, "CENT"))
					return fmttime("%C", t);
				break;
			case 'D':
				if (!strcmp(s, "DATE"))
					return fmttime(f ? (j ? "%Y%j" : "%Y%m%d") : (j ? "%y%j" : "%y%m%d"), t);
				else if (!strcmp(s, "DAY"))
					return fmttime(j ? "%j" : "%d", t);
				break;
			case 'M':
				if (!strcmp(s, "MONTH"))
					return fmttime("%m", t);
				break;
			case 'W':
				if (!strcmp(s, "WDAY"))
					return fmttime("%u", t);
				else if (!strcmp(s, "WEEK"))
					return fmttime("%U", t);
				break;
			case 'Y':
				if (!strcmp(s, "YEAR"))
					return fmttime(f ? "%Y" : "%y", t);
				break;
			}
			switch (*b)
			{
			case 'B':
				if (!strncmp(b, "BLANK", 5))
				{
					if (!(j = (int)strtol(b, &s, 10)))
						j = 1;
					if (!*s)
					{
						s = fmtbuf(j + 1);
						memset(s, ' ', j);
						s[j] = 0;
						return s;
					}
				}
				break;
			case 'R':
				if (!strcmp(b, "RN"))
					return "1";
				break;
			case 'T':
				if (!strcmp(b, "TIME"))
					return fmttime("%H%M%S", t);
				break;
			}
			if ((set & MUST) && jcl->disc->errorf)
				(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%%%%%s operand value or %s environment value expected", b, name);
		}
		else if (s = internal(jcl, name))
			return s;
		else if ((set & MUST) && jcl->disc->errorf)
			(*jcl->disc->errorf)(NiL, jcl->disc, 1, "&%s: undefined variable", name);
		return 0;
	}
	if (!(v = vmnewof(jcl->vm, 0, Jclsym_t, 1, strlen(name))))
	{
		nospace(jcl, NiL);
		return 0;
	}
	strcpy(v->name, name);
	if (!(v->value = stash(jcl, jcl->vm, value, 0)))
		return 0;
	dtinsert(jcl->syms, v);
 found:
	if (jcl->flags & (JCL_PARAMETERIZE|JCL_LISTVARIABLES))
	{
		register char*	s;

		for (s = v->name; isalnum(*s) || *s == '_'; s++);
		if (!*s)
		{
			if (jcl->flags & JCL_LISTVARIABLES)
				uniq(v->name, NiL, 0, jcl->disc);
			if (jcl->flags & JCL_PARAMETERIZE)
			{
#if 0
				if (diff(v->name, v->value))
					sfprintf(sfstdout, "%s=%s\t# global\n", v->name, fmtquote(v->value, "$'", "'", strlen(v->value), 0));
#endif
				sfprintf(jcl->vp, "${%s}", v->name);
				if (!(s = sfstruse(jcl->vp)))
					nospace(jcl, NiL);
				return s;
			}
		}
	}
	return v->value;
}

/*
 * save string in vm
 * path==1 enables mvs => unix path conversion
 */

char*
stash(Jcl_t* jcl, Vmalloc_t* vm, const char* str, int path)
{
	char*	s;
	char*	t;
	char*	e;
	int	n;

	n = strlen(str);
	if (!(s = vmnewof(vm, NiL, char, n, 1)))
		nospace(jcl, NiL);
	strcpy(s, str);
	if (path && (*s != '$' || *(s + 1) != '(') && (t = strchr(s, '(')) && *(s + n - 1) == ')')
	{
		strtol(t + 1, &e, 10);
		if (e == (s + n - 1))
			*t = 0;
		else
		{
			*t = '/';
			*(s + n - 1) = 0;
		}
	}
	return s;
}

/*
 * out of space message
 */

void
nospace(Jcl_t* jcl, Jcldisc_t* disc)
{
	if (!disc)
		disc = jcl->disc;
	if (disc->errorf)
		(*disc->errorf)(jcl, disc, ERROR_SYSTEM|2, "out of space");
}
