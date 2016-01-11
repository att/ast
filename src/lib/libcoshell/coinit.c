/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * return job initialization commands
 */

#if _WIN32
#undef	_BLD_DLL
#define _BLD_DLL	1
#endif

#include "colib.h"

#include <ctype.h>
#include <fs3d.h>
#include <ls.h>

static void
exid(Sfio_t* sp, const char* pre, const char* name, const char* pos)
{
	int	c;

	sfputr(sp, pre, -1);
	if ((c = *name++) && c != '=')
	{
		if (isdigit(c))
			sfputc(sp, '_');
		do
		{
			if (!isalnum(c))
				c = '_';
			sfputc(sp, c);
		} while ((c = *name++) && c != '=');
	}
	else
		sfputc(sp, '_');
	sfputr(sp, pos, -1);
}

/*
 * add n to the export list
 * old!=0 formats in old style
 * coex!=0 for CO_ENV_EXPORT
 * if n prefixed by % then coquote conversion enabled
 */

static void
putexport(Coshell_t* co, Sfio_t* sp, char* n, int old, int coex, int flags)
{
	int		cvt;
	char*		v;
	Coexport_t*	ex;

	if (cvt = *n == '%')
		n++;

	/*
	 * currently limited to valid identifer env var names
	 */

	if (!co->export || !dtmatch(co->export, n))
	{
		if (old)
			cvt = 0;
		if ((v = getenv(n)) && *v || coex && ((flags & CO_EXPORT) || co->export && dtsize(co->export) > 0))
		{
			if (!old)
				sfprintf(sp, "\\\n");
			exid(sp, " ", n, "='");
			if (coex && (flags & CO_EXPORT))
				v = "(*)";
			if (v)
				coquote(sp, v, cvt);
			if (coex && !(flags & CO_EXPORT))
			{
				v = v ? ":" : "";
				for (ex = (Coexport_t*)dtfirst(co->export); ex; ex = (Coexport_t*)dtnext(co->export, ex))
				{
					sfprintf(sp, "%s%s", v, ex->name);
					exid(sp, v, ex->name, "");
					v = ":";
				}
			}
			sfputc(sp, '\'');
			if (old)
				exid(sp, "\nexport ", n, "\n");
		}
	}
}

/*
 * return job initialization commands
 */

char*
coinitialize(Coshell_t* co, int flags)
{
	register char*	s;
	int		n;
	int		m;
	int		old;
	int		sync;
	char*		t;
	long		p;
	Coexport_t*	ex;
	Sfio_t*		sp;
	Sfio_t*		tp;
	struct stat	st;

	sync = co->init.sync;
	co->init.sync = 0;

	/*
	 * pwd
	 */

	if (stat(".", &st))
		return 0;
	if (!state.pwd || st.st_ino != co->init.pwd_ino || st.st_dev != co->init.pwd_dev)
	{
		co->init.pwd_dev = st.st_dev;
		co->init.pwd_ino = st.st_ino;
		if (state.pwd)
			free(state.pwd);
		if (!(state.pwd = getcwd(NiL, 0)))
		{
			if (errno != EINVAL || !(state.pwd = newof(0, char, PATH_MAX, 0)))
				return 0;
			if (!getcwd(state.pwd, PATH_MAX))
			{
				free(state.pwd);
				state.pwd = 0;
				return 0;
			}
		}
		if (!(flags & CO_INIT))
			sync = 1;
	}

	/*
	 * umask
	 */

	umask(n = umask(co->init.mask));
	if (co->init.mask != n)
	{
		co->init.mask = n;
		if (!(flags & CO_INIT))
			sync = 1;
	}
	if (!co->init.script || sync)
	{
		/*
		 * co_export[] vars
		 */

		if (!(sp = sfstropen()))
			return 0;
		tp = 0;
		old = !(flags & (CO_KSH|CO_SERVER));
		if (!old)
			sfprintf(sp, "export");
		if (sync)
		{
			if (flags & CO_EXPORT)
				s = "(*)";
			else
			{
				for (n = 0; s = co_export[n]; n++)
					putexport(co, sp, s, old, !n, flags);
				s = getenv(co_export[0]);
			}
			if (s)
			{
				if (*s == '(')
				{
					register char**	ep = environ;
					register char*	e;
					char*		v;
					char*		es;
					char*		xs;

					if (v = strchr(s, ':'))
						*v = 0;
					while (e = *ep++)
						if ((t = strsubmatch(e, s, 1)) && (*t == '=' || !*t && (t = strchr(e, '='))))
						{
							m = (int)(t - e);
							if (!strneq(e, "PATH=", 5) && !strneq(e, "_=", 2))
							{
								for (n = 0; xs = co_export[n]; n++)
								{
									es = e;
									while (*xs && *es == *xs)
									{
										es++;
										xs++;
									}
									if (*es == '=' && !*xs)
										break;
								}
								if (!xs)
								{
									if (!old)
										sfprintf(sp, "\\\n");
									exid(sp, " ", e, "='");
									coquote(sp, e + m + 1, 0);
									sfputc(sp, '\'');
									if (old)
										exid(sp, "\nexport ", e, "\n");
								}
							}
						}
					if (v)
					{
						*v++ = ':';
						s = v;
					}
				}
				if (*s)
					for (;;)
					{
						if (t = strchr(s, ':'))
							*t = 0;
						putexport(co, sp, s, old, 0, 0);
						if (!(s = t))
							break;
						*s++ = ':';
					}
			}
			if (co->export)
				for (ex = (Coexport_t*)dtfirst(co->export); ex; ex = (Coexport_t*)dtnext(co->export, ex))
				{
					if (!old)
						sfprintf(sp, "\\\n");
					exid(sp, " ", ex->name, "='");
					coquote(sp, ex->value, 0);
					sfputc(sp, '\'');
					if (old)
						exid(sp, "\nexport ", ex->name, "\n");
				}
		}

		/*
		 * PATH
		 */

		if (!old)
			sfprintf(sp, "\\\n");
		sfprintf(sp, " PATH='");
		n = PATH_MAX;
		if (!(t = sfstrrsrv(sp, n)))
		{
		bad:
			sfstrclose(sp);
			if (tp)
				sfstrclose(tp);
			return 0;
		}
		t += n / 2;
		if (!(flags & CO_CROSS) && !pathpath("ignore", NiL, PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE, t, n / 2) && pathpath("bin/ignore", "", PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE, t, n / 2))
		{
			*strrchr(t, '/') = 0;
			sfputc(sp, ':');
			coquote(sp, t, !old);
			sfputc(sp, ':');
			s = pathbin();
		}
		else
		{
			s = pathbin();
			if (!(flags & CO_CROSS))
			{
				if (!sync && (*s == ':' || *s == '.' && *(s + 1) == ':'))
				{
					sfstrseek(sp, 0, SEEK_SET);
					goto done;
				}
				sfputc(sp, ':');
			}
		}
		for (;;)
		{
			if (*s == ':')
				s++;
			else if (*s == '.' && *(s + 1) == ':')
				s += 2;
			else
				break;
		}
		if (!(flags & CO_CROSS))
			tp = 0;
		else if (!(tp = sfstropen()))
			goto bad;
		else
		{
			while (n = *s++)
			{
				if (n == ':')
				{
					while (*s == ':')
						s++;
					if (!*s)
						break;
					if (*s == '.')
					{
						if (!*(s + 1))
							break;
						if (*(s + 1) == ':')
						{
							s++;
							continue;
						}
					}
				}
				sfputc(tp, n);
			}
			if (!(s = costash(tp)))
				goto bad;
		}
		coquote(sp, s, !old);
		if (tp)
			sfstrclose(tp);
		sfputc(sp, '\'');
		if (old)
			sfprintf(sp, "\nexport PATH");
		sfputc(sp, '\n');
		if (sync)
		{
			/*
			 * VPATH
			 */

			p = (int)sfstrtell(sp);
			sfprintf(sp, "vpath ");
			n = PATH_MAX;
			if (fs3d(FS3D_TEST))
				for (;;)
				{
					if (!(t = sfstrrsrv(sp, n)))
						goto bad;
					if ((m = mount(NiL, t, FS3D_GET|FS3D_ALL|FS3D_SIZE(n), NiL)) > 0)
						m = n;
					else
					{
						if (!m)
							sfstrseek(sp, strlen(t), SEEK_CUR);
						break;
					}
				}
			else
			{
				m = 0;
				sfprintf(sp, "- /#option/2d");
			}
			if (m)
				sfstrseek(sp, p, SEEK_SET);
			else
				sfprintf(sp, " 2>/dev/null || :\n");
			sfprintf(sp, "umask 0%o\ncd '%s'\n", co->init.mask, state.pwd);
		}
	done:
		if (!(flags & CO_SERVER))
		{
			sfprintf(sp, "%s%s=%05d${!%s-$$}\n", old ? "" : "export ", CO_ENV_TEMP, getpid(), (flags & CO_OSH) ? "" : ":");
			if (old)
				sfprintf(sp, "export %s\n", CO_ENV_TEMP);
		}
		sfputc(sp, 0);
		n = (int)sfstrtell(sp);
		if (co->vm)
		{
			if (co->init.script)
				vmfree(co->vm, co->init.script);
			if (!(co->init.script = vmnewof(co->vm, 0, char, n, 1)))
				goto bad;
		}
		else
		{
			if (co->init.script)
				free(co->init.script);
			if (!(co->init.script = newof(0, char, n, 1)))
				goto bad;
		}
		memcpy(co->init.script, sfstrbase(sp), n);
		sfstrclose(sp);
	}
	else if (!co->init.script)
	{
		if (co->init.script = co->vm ? vmnewof(co->vm, 0, char, 1, 0) : newof(0, char, 1, 0))
			*co->init.script = 0;
	}
	return co->init.script;
}

/*
 * return generic job initialization commands
 */

char*
coinit(int flags)
{
	if (!state.generic)
	{
		if (!(state.generic = newof(0, Coshell_t, 1, 0)))
			return 0;
		state.generic->init.sync = 1;
	}
	return coinitialize(state.generic, flags);
}
