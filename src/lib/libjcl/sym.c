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
 * jcl symbol support
 */

#include "jcllib.h"

/*
 * add symbol name=value to the current step
 * value==0 => name==name=value, got it?
 * no diagnostic if value==0 and name!=name=value
 */

Jclsym_t*
jclsym(Jcl_t* jcl, const char* name, const char* value, int flags)
{
	register Jclsym_t*	v;
	register int		n;
	int			imported;
	const char*		set;
	char*			e;
	Jclsym_t*		o;

	if (!(set = name))
		return 0;
	else if (value)
		n = strlen(name);
	else if (value = (const char*)strchr(name, '='))
		n = value++ - name;
	else
		return 0;
	if (name[0] == '%' && name[1] == '%')
	{
		name = (const char*)sfprints("%s%-.*s", JCL_AUTO, n - 2, name + 2);
		n = strlen(name);
		imported = (flags & JCL_SYM_SET) && (jcl->flags & JCL_IMPORT);
	}
	else
		imported = 0;
	if (!(v = vmnewof(jcl->vs, 0, Jclsym_t, 1, n + strlen(value) + 2)))
	{
		nospace(jcl, NiL);
		return 0;
	}
	memcpy(v->name, name, n);
	strcpy(v->value = (char*)(v + 1) + n + 1, value);
	v->flags = flags;
	if (o = (Jclsym_t*)dtsearch(jcl->step->syms, v))
	{
		if (imported && (o->flags & JCL_SYM_IMPORT) || (flags & JCL_SYM_SET) && (o->flags & JCL_SYM_READONLY))
		{
			vmfree(jcl->vs, v);
			v = o;
			goto export;
		}
		dtdelete(jcl->step->syms, o);
		vmfree(jcl->vs, o);
	}
	else if (imported && (e = getenv(name)))
	{
		vmfree(jcl->vs, v);
		if (!(v = vmnewof(jcl->vs, 0, Jclsym_t, 1, strlen(e) + 1)))
		{
			nospace(jcl, NiL);
			return 0;
		}
		strcpy(v->name, e);
		if (v->value = strchr(v->name, '='))
			*v->value++ = 0;
		v->flags |= JCL_SYM_IMPORT;
	}
	dtinsert(jcl->step->syms, v);
	if (flags & (JCL_SYM_EXPORT|JCL_SYM_READONLY))
	{
	export:
		if (jcl->flags & JCL_EXEC)
		{
			if (!(set = vmstrdup(jcl->vs, set)) || !setenviron(set))
			{
				nospace(jcl, NiL);
				return 0;
			}
		}
		else if (jcl->flags & JCL_VERBOSE)
			sfprintf(sfstdout, "export %s=%s\n", v->name, fmtquote(v->value, "\"", "\"", strlen(v->value), FMT_SHELL));
	}
	message((-2, "set       %s=%s", v->name, v->value));
	return v;
}
