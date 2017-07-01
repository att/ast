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
 * jcl file search
 */

#include "jcllib.h"

#include <ctype.h>
#include <ls.h>
#include <regex.h>

#define directory(p,s)	(stat((p),(s))>=0&&S_ISDIR((s)->st_mode))
#define REGULAR(p,s)	(stat((p),(s))>=0&&(S_ISREG((s)->st_mode)||streq(p,"/dev/null")))

static struct				/* directory list state		*/
{
	Dirlist_t	dirs;		/* global directory list	*/
} state;

/*
 * return >0 if path is a regular file
 */

static int
regular(Jcl_t* jcl, char* path, struct stat* st)
{
	register char*		s;
	register char*		t;
	register Include_t*	ip;

	if (REGULAR(path, st))
		goto found;
	if (s = strrchr(path, '/'))
		s++;
	else
		s = path;
	while (*s && !isupper(*s))
		s++;
	if (*s)
	{
		t = sfprints("%s", path);
		for (s = t + (s - path); *s; s++)
			if (isupper(*s))
				*s = tolower(*s);
		if (REGULAR(t, st))
		{
			strcpy(path, t);
			goto found;
		}
	}
	return 0;
 found:

	/*
	 * catch recursive includes
	 */

	do
	{
		for (ip = jcl->include; ip; ip = ip->prev)
			if (streq(path, ip->path))
				return 0;
	} while (jcl = jcl->scope);
	return 1;
}

/*
 * expand ${...} in name
 * return expanded value returned in jcl->vp
 * jcl->tp may be clobbered
 */

char*
expand(Jcl_t* jcl, const char* name, int flags)
{
	register char*	s;
	register char*	t;
	register int	c;
	char*		b;
	char*		v;
	int		i;
	size_t		n;
	size_t		o;
	size_t		p;
	size_t		r;
	Sfio_t*		vp;

	if (jcl)
		for (s = (char*)name; *s; s++)
			if (*s == '$' && *(s + 1) == '{')
			{
				p = sfstrtell(jcl->tp);
				if (n = s - (char*)name)
					sfwrite(jcl->tp, name, n);
				while (c = *s++)
					if (c == '$' && *s == '{')
					{
						b = s;
						o = sfstrtell(jcl->tp);
						s++;
						if (*s == '%' && *(s + 1) == '%')
						{
							s += 2;
							sfputr(jcl->tp, JCL_AUTO, -1);
						}
						while ((c = *s++) && c != ':' && c != '}')
							sfputc(jcl->tp, c);
						sfputc(jcl->tp, 0);
						v = sfstrseek(jcl->tp, o, SEEK_SET);
						if (isdigit(*v) && !*(v + 1))
						{
							if (t = matched(*v - '0', &n, jcl->disc))
								sfwrite(jcl->tp, t, n);
						}
						else if (t = lookup(jcl, v, NiL, flags, DEFAULT))
						{
							if ((flags & JCL_SYM_SET) && jcl->vp != jcl->xp)
							{
								vp = jcl->vp;
								jcl->vp = jcl->xp;
								t = expand(jcl, t, flags);
								sfputr(jcl->tp, t, -1);
								jcl->vp = vp;
							}
							else
								sfputr(jcl->tp, t, -1);
						}
						else if (((flags & JCL_SYM_EXPORT) || !(flags & JCL_SYM_SET)) && (t = getenv(v)))
							sfputr(jcl->tp, t, -1);
						else if (flags & JCL_SYM_SET)
						{
							sfputc(jcl->tp, '$');
							s = b;
							continue;
						}
						while (c == ':')
						{
							regex_t		re;
							regmatch_t	match[10];

							c = *s;
							if (c == 's' && s++ || c != '/')
							{
								if (!(i = regcomp(&re, s, REG_AUGMENTED|REG_DELIMITED|REG_LENIENT|REG_NULL)))
								{
									s += re.re_npat;
									if (!(i = regsubcomp(&re, s, NiL, 0, 0)))
										s += re.re_npat;
								}
								if (!i && (*s == ':' || *s == '}'))
								{
									s++;
									r = sfstrtell(jcl->tp);
									sfputc(jcl->tp, 0);
									v = sfstrseek(jcl->tp, o, SEEK_SET);
									if (!(i = regexec(&re, v, elementsof(match), match, 0)) && !(i = regsubexec(&re, v, elementsof(match), match)))
									{
										sfputr(jcl->tp, re.re_sub->re_buf, -1);
										regfree(&re);
									}
									else if (i != REG_NOMATCH)
										regfatal(&re, 2, i);
									else
									{
										sfstrseek(jcl->tp, r, SEEK_SET);
										regfree(&re);
									}
								}
								else
								{
									if (i)
										regfatalpat(&re, 2, i, s);
									while (*s && *s++ != '}');
								}
							}
							else if (jcl->disc->errorf)
								(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%-.*s: unknown edit op", s - t, t);
							c = *(s - 1);
						}
					}
					else
						sfputc(jcl->tp, c);
				sfputc(jcl->tp, 0);
				s = sfstrseek(jcl->tp, p, SEEK_SET);
				message((-7, "expand    %s => %s", name, s));
				sfputr(jcl->vp, s, -1);
				if (!(s = sfstruse(jcl->vp)))
					nospace(jcl, NiL);
				return s;
			}
	return (char*)name;
}

/*
 * append dir to jclfind() include list
 */

int
jclinclude(Jcl_t* jcl, const char* dir, unsigned long flags, Jcldisc_t* disc)
{
	register Dirlist_t*	lp;
	register Dir_t*		dp;
	struct stat		st;

	if (dir && *(dir = (const char*)expand(jcl, dir, 0)) && !streq(dir, ".") && directory(dir, &st))
	{
		lp = jcl ? &jcl->dirs : &state.dirs;
		for (dp = lp->head; dp; dp = dp->next)
			if (streq(dir, dp->dir))
				return 0;
		if (!(dp = oldof(0, Dir_t, 1, strlen(dir))))
		{
			nospace(jcl, disc);
			return -1;
		}
		strcpy(dp->dir, dir);
		dp->next = 0;
		dp->flags = flags;
		if (lp->tail)
			lp->tail = lp->tail->next = dp;
		else
			lp->head = lp->tail = dp;
	}
	return 0;
}

/*
 * check the jclinclude() directories
 */

static char*
search(Jcl_t* jcl, const char* dir, const char* name, unsigned long flags, struct stat* st)
{
	register char*		s;
	register Dir_t*		dp;
	Jcl_t*			top;

	if (!dir || !(flags & JCL_STANDARD) || strchr(name, '/'))
	{
		if (dir && *name != '/')
			sfprintf(jcl->tp, "%s/", dir);
		sfprintf(jcl->vp, "%s", name);
		if (!(s = sfstruse(jcl->vp)))
			nospace(jcl, NiL);
		if (regular(jcl, s, st))
			return s;
	}
	if (*name != '/')
	{
		top = jcl;
		dp = jcl->dirs.head;
		for (;;)
		{
			for (; dp; dp = dp->next)
				if (flags & dp->flags)
				{
					if (dir && *dp->dir != '/')
						sfprintf(top->tp, "%s/", dir);
					sfprintf(top->tp, "%s/%s", dp->dir, name);
					if (!(s = sfstruse(top->tp)))
						nospace(jcl, NiL);
					if (regular(top, s, st))
						return s;
				}
			if (!jcl)
				break;
			dp = (jcl = jcl->scope) ? jcl->dirs.head : state.dirs.head;
		}
	}
	return 0;
}

/*
 * return path to name using jclinclude() list
 * path allocated in jcl->vs
 * !(flags&JCL_STANDARD) checks . and dir of including file
 * level>0 is not found error message level
 * { jcl->tp jcl->vp jcl->xp } may be clobbered
 */

char*
jclfind(Jcl_t* jcl, const char* name, unsigned long flags, int level, Sfio_t** spp)
{
	register char*		s;
	struct stat		st;

	if (flags & JCL_PROC)
	{
		sfprintf(jcl->vp, "(PROC)%s", name);
		if (!(s = sfstruse(jcl->vp)))
			nospace(jcl, NiL);
		if (s = lookup(jcl, s, NiL, 0, 0))
		{
			if (spp && !(*spp = sfstropen()) || sfstrbuf(*spp, s, strlen(s), 0))
			{
				if (*spp)
					sfclose(*spp);
				nospace(jcl, NiL);
				return 0;
			}
			s = (char*)name;
			goto save;
		}
	}
	name = (const char*)expand(jcl, jclpath(jcl, name), 0);

	/*
	 * check the unadorned path first
	 * this handles . and absolute paths
	 */

	if (s = search(jcl, NiL, name, flags, &st))
		goto found;
	if (*name != '/')
	{
		/*
		 * check the directory of the including file
		 * on the assumption that error_info.file is properly stacked
		 */

		if (!(flags & JCL_STANDARD) && error_info.file && (s = strrchr(error_info.file, '/')))
		{
			sfprintf(jcl->tp, "%-.*s%s", s - error_info.file + 1, error_info.file, name);
			if (!(s = sfstruse(jcl->tp)))
				nospace(jcl, NiL);
			if (regular(jcl, s, &st))
				goto found;
		}
	}
	if (flags & JCL_CREATE)
	{
		if (spp)
			*spp = 0;
		s = (char*)name;
		goto save;
	}
	if (level && jcl->disc->errorf)
		(*jcl->disc->errorf)(NiL, jcl->disc, ERROR_SYSTEM|level, "%s: not found", name);
	return 0;
 found:
	if (spp && !(*spp = sfopen(NiL, s, "r")) && level > 0 && jcl->disc->errorf)
		(*jcl->disc->errorf)(NiL, jcl->disc, ERROR_SYSTEM|level, "%s: cannot read", s);
	message((-4, "find      %s => %s", name, s));
 save:
	return stash(jcl, jcl->vs, s, 0);
}
