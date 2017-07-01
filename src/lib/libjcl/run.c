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
 * jcl program exec and script loop
 */

#include "jcllib.h"

#include <coshell.h>
#include <errno.h>
#include <ls.h>
#include <tm.h>
#include <tmx.h>

/*
 * if name contains invalid export id chars then
 * return a new name with those chars converted to '_<XX>_'
 */

static char*
fmtexport(const char* name)
{
	register int		c;
	register const char*	s;
	register char*		t;
	register int		n;
	char*			b;

	s = name;
	c = *s++;
	if (isalpha(c) || c == '_')
		do
		{
			if (!(c = *s++))
				return (char*)name;
		} while (isalnum(c) || c == '_');
	n = 1;
	while (c = *s++)
		if (!isalnum(c) && c != '_')
			n++;
	t = b = fmtbuf(strlen(name) + 4 * n + 1);
	for (s = name; c = *s++;)
		if (!isalnum(c) && c != '_')
			t += sfsprintf(t, 5, "_%02X_", c);
		else
			*t++ = c;
	*t = 0;
	return b;
}

/*
 * output dd dsname with &path => /tmp file map
 */

static char*
dsn(Jcl_t* jcl, Jcldd_t* dd, const char* path, int mark)
{
	char*	s;

	if (mark && dd->disp[0] == JCL_DISP_MOD && !(dd->flags & JCL_DD_DIR))
		sfprintf(jcl->vp, "+");
	if (*path == '&')
	{
		if (*++path == '&')
			path++;
		sfprintf(jcl->vp, jcl->tmp);
	}
	sfprintf(jcl->vp, "%s", fmtquote(path, "\"", "\"", strlen(path), FMT_SHELL|FMT_PARAM));
	if (!(s = sfstruse(jcl->vp)))
		nospace(jcl, NiL);
	return s;
}

/*
 * create dd dir if it doesn't exist
 */

static void
checkdir(Jcl_t* jcl, Jcldd_t* dd)
{
	register char*	s;
	Jcldir_t*	dir;

	if (!dtmatch(jcl->outdir, dd->path))
	{
		if (dir = vmnewof(jcl->vm, NiL, Jcldir_t, 1, strlen(dd->path)))
		{
			strcpy(dir->name, dd->path);
			dtinsert(jcl->outdir, dir);
		}
		s = dsn(jcl, dd, dd->path, 0);
		sfprintf(jcl->tp, "[[ ! -d %s && ! -f %s ]] && mkdir -p %s\n", s, s, s);
	}
}

/*
 * execution loop
 */

int
jclrun(Jcl_t* scope)
{
	register Jcl_t*		jcl;
	register Jclstep_t*	step;
	register Jcldd_t*	dd;
	register Jclcat_t*	cat;
	register Jclsym_t*	sym;
	register char*		s;
	char*			arg;
	char*			t;
	Coshell_t*		co;
	Cojob_t*		cj;
	Jcldd_t*		xx;
	Jcldd_t*		std[4];
	Dtlink_t		k;
	Jcl_t*			top;
	int			code;
	int			del;
	int			n;
	int			i;
	double			pct;
	double			real;
	double			user;
	double			sys;
	Time_t			start;
	unsigned long		flags;
	char			subdir[64];

	if (!(jcl = jclopen(scope, scope->step->command, scope->flags|scope->step->flags, scope->disc)))
		return -1;
	if (jcl->flags & JCL_EXEC)
	{
		if (!(co = coopen(pathshell(), (jcl->flags & (JCL_EXEC|JCL_TRACE)) == (JCL_EXEC|JCL_TRACE) ? CO_ANY : (CO_ANY|CO_SILENT), NiL)))
		{
			if (jcl->disc->errorf)
				(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: cannot connect to coshell", jcl->name);
			jclclose(jcl);
			return -1;
		}
		start = tmxgettime();
		user = co->user;
		sys = co->sys;
	}
	code = 0;
	if (jcl->name && (!scope || !scope->scope || (scope->scope->flags & JCL_SCOPE)))
	{
		if (jcl->flags & (JCL_LISTJOBS|JCL_LISTSCRIPTS))
			uniq(jcl->name, NiL, JCL_LISTJOBS, jcl->disc);
		top = jcl;
		if (!(jcl->flags & JCL_EXEC))
		{
			if (jcl->flags & JCL_VERBOSE)
				sfprintf(sfstdout, ": JOB %s\nexport %sJOBNAME=%s\ncode=0\n", jcl->name, JCL_AUTO, jcl->name);
			if (jcl->flags & JCL_TRACE)
				sfprintf(sfstderr, "+ : JOB %s\n", jcl->name);
		}
		if (jcl->flags & JCL_SUBDIR)
		{
			if (jcl->flags & JCL_EXEC)
			{
				t = fmttime("%y-%m-%d", time(NiL));
				n = 0;
				for (;;)
				{
					sfsprintf(subdir, sizeof(subdir), "%s.%s.%d", jcl->name, t, ++n);
					if (!mkdir(subdir, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IXOTH))
						break;
					if (errno != EEXIST)
					{
						if (jcl->disc->errorf)
							(*jcl->disc->errorf)(NiL, jcl->disc, ERROR_SYSTEM|2, "%s: cannot create job subdirectory", subdir);
						jclclose(jcl);
						return -1;
					}
				}
				if (chdir(subdir))
				{
					if (jcl->disc->errorf)
						(*jcl->disc->errorf)(NiL, jcl->disc, ERROR_SYSTEM|2, "%s: cannot run in job subdirectory", subdir);
					jclclose(jcl);
					return -1;
				}
				sfsync(sfstdout);
				sfsync(sfstderr);
				for (i = 0; i < elementsof(redirect); i++)
					if (jcl->redirect[i] > 0)
					{
						close(redirect[i].fd);
						if ((n = open(redirect[i].file, O_CREAT|O_TRUNC|O_WRONLY|O_APPEND|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != redirect[i].fd)
						{
							close(n);
							dup2(jcl->redirect[i], redirect[i].fd);
						}
					}
				sfset(sfstdout, SF_LINE, 1);
				sfset(sfstderr, SF_LINE, 1);
				if (jcl->flags & JCL_TRACE)
					sfprintf(sfstderr, "+ mkdir %s\n+ cd %s\n", subdir, subdir);
				if (jcl->flags & JCL_VERBOSE)
					sfprintf(sfstdout, "STARTED AT %s\n", fmttime("%K", time(NiL)));
			}
			else if (jcl->flags & JCL_VERBOSE)
			{
				sfprintf(sfstdout, "n=0\nt=$(date +%%y-%%m-%%d)\nwhile	:\ndo	d=%s.$t.$((++n))\n	[[ -d $d ]] || break\ndone\nmkdir $d && cd $d || exit 1\n", jcl->name);
				sfputr(sfstdout, "exec > SYSOUT 2> SYSERR\nTIMEFORMAT='USAGE CPU=%P%% REAL=%R USR=%U SYS=%S'\ntime {\ndate +'STARTED AT %K'", '\n');
			}
		}
	}
	else
		for (top = scope; !top->name && !(top->flags & JCL_SCOPE) && top->scope; top = top->scope);
	while (step = jclstep(jcl))
	{
		for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
			if (dd->flags & JCL_DD_ALIAS)
			{
				dd->flags &= ~JCL_DD_ALIAS;
				xx = 0;
				t = fmtbuf(n = strlen(step->name) + strlen(dd->path) + 2);
				sfsprintf(t, n, "%s.%s", step->name, dd->path);
				for (scope = jcl; scope; scope = scope->scope)
					if ((xx = (Jcldd_t*)dtmatch(scope->step->dd, t)) ||
					    (xx = (Jcldd_t*)dtmatch(scope->step->dd, dd->path)))
						break;
				if (!xx)
				{
					if (jcl->disc->errorf)
						(*jcl->disc->errorf)(NiL, jcl->disc, 2, "%s: DD not defined", dd->path);
					code = -1;
					goto bad;
				}
				k = dd->link;
				s = dd->name;
				*dd = *xx;
				dd->link = k;
				dd->name = s;
			}
		if (!jcl->tmp)
		{
			for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
			{
				if (dd->path && *dd->path == '&')
					break;
				for (cat = dd->cat; cat; cat = cat->next)
					if (cat->path && *cat->path == '&')
						break;
			}
			if (dd)
			{
				if (jcl->flags & JCL_SUBTMP)
					sfprintf(jcl->vp, "tmp.%s.%lu.", jcl->main->name, (unsigned long)getpid());
				else
				{
					if (jcl->flags & JCL_EXEC)
					{
						if (!(s = getenv("TMPDIR")))
							s = "/tmp";
						sfprintf(jcl->vp, "%s/job.%s.%lu.", s, jcl->main->name, (unsigned long)getpid());
					}
					else
						sfprintf(jcl->vp, "${TMPDIR:-/tmp}/job.%s.$$.", jcl->main->name);
				}
				if (!(s = sfstruse(jcl->vp)))
					nospace(jcl, NiL);
				jcl->tmp = stash(jcl, jcl->vm, s, 0);
				for (s = jcl->tmp; *s; s++)
					if (*s == '@')
						*s = '-';
				if ((jcl->flags & (JCL_VERBOSE|JCL_EXEC|JCL_SUBTMP)) == JCL_VERBOSE)
					sfprintf(sfstdout, "trap 'code=$?; rm -rf %s*; exit $code' 0 1 2\n", jcl->tmp);
			}
		}
		jcl->steps++;
		if (jcl->flags & (JCL_LISTEXEC|JCL_LISTINPUTS|JCL_LISTJOBS|JCL_LISTOUTPUTS|JCL_LISTPROGRAMS|JCL_LISTSCRIPTS))
		{
			if ((jcl->flags & JCL_LISTPROGRAMS) && (step->flags & JCL_PGM))
				uniq(step->command, (jcl->flags & JCL_LISTPARMS) && step->parm ? step->parm : NiL, JCL_LISTPROGRAMS, jcl->disc);
			else if ((jcl->flags & (JCL_LISTJOBS|JCL_LISTSCRIPTS)) && !(step->flags & JCL_PGM) && (s = jclfind(jcl, step->command, 0, 0, NiL)))
				uniq(s, NiL, JCL_LISTSCRIPTS, jcl->disc);
			else if (jcl->flags & JCL_LISTEXEC)
				uniq(step->command, NiL, JCL_LISTEXEC, jcl->disc);
			if (jcl->flags & (JCL_LISTINPUTS|JCL_LISTOUTPUTS))
			{
				for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
				{
					flags = (dd->disp[0] == JCL_DISP_NEW || dd->disp[0] == JCL_DISP_MOD) ? JCL_LISTOUTPUTS : JCL_LISTINPUTS;
					if (dd->path && *dd->path != '&')
					{
						if (jcl->flags & JCL_LISTDD)
							uniq(dd->name, dd->path, flags, jcl->disc);
						else
							uniq(dd->path, NiL, flags, jcl->disc);
					}
					for (cat = dd->cat; cat; cat = cat->next)
						if (*cat->path != '&')
						{
							if (jcl->flags & JCL_LISTDD)
								uniq(dd->name, cat->path, flags, jcl->disc);
							else
								uniq(cat->path, NiL, flags, jcl->disc);
						}
				}
			}
		}
		if (!jcleval(jcl, jcl->cond, code))
			break;
		if (jcleval(jcl, step->cond, code))
		{
			if (step->flags & JCL_PGM)
			{
				std[0] = std[1] = std[2] = std[3] = 0;
				sfprintf(jcl->tp, ": %s\n", step->name);
				for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
					switch (dd->flags & (JCL_DD_DIR|JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE))
					{
					case JCL_DD_DIR:
						if (dd->disp[0] == JCL_DISP_NEW || dd->disp[0] == JCL_DISP_MOD)
							checkdir(jcl, dd);
						break;
					case JCL_DD_SYSIN:
						if (dd->path)
							std[0] = dd;
						if (dd->here)
							std[3] = dd;
						break;
					case JCL_DD_SYSOUT:
						std[1] = dd;
						break;
					case JCL_DD_SYSERR:
						std[2] = dd;
						break;
					default:
						if (!(dd->flags & JCL_DD_DUMMY) && (jcl->flags & (JCL_EXEC|JCL_VERBOSE)) && dd->path && *dd->path != '&' && dd->disp[0] == JCL_DISP_NEW && (*dd->path != '$' || *(dd->path + 1) != '(') && (s = strrchr(dd->path, '/')))
						{
							*s = 0;
							checkdir(jcl, dd);
							*s = '/';
						}
						break;
					}
				for (dd = (Jcldd_t*)dtfirst(jcl->dd); dd; dd = (Jcldd_t*)dtnext(jcl->dd, dd))
					if ((dd->flags & (JCL_DD_MARKED|JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE)) == JCL_DD_MARKED && (dd->disp[0] == JCL_DISP_NEW || dd->disp[0] == JCL_DISP_MOD))
					{
						s = dsn(jcl, dd, dd->path, 0);
						sfprintf(jcl->tp, "[[ -e %s ]] || > %s\n", s, s);
					}
				for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
					if ((dd->flags & (JCL_DD_MARKED|JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE)) == JCL_DD_MARKED && (dd->disp[0] == JCL_DISP_NEW || dd->disp[0] == JCL_DISP_MOD))
					{
						s = dsn(jcl, dd, dd->path, 0);
						sfprintf(jcl->tp, "[[ -e %s ]] || > %s\n", s, s);
					}
				if ((dd = std[0]) && dd->cat)
				{
					std[0] = std[3] = 0;
					sfprintf(jcl->tp, "cat %s", dsn(jcl, dd, dd->path, 0));
					for (cat = dd->cat; cat; cat = cat->next)
						sfprintf(jcl->tp, " \\\n\t%s", dsn(jcl, dd, cat->path, 0));
					sfprintf(jcl->tp, " |\n");
				}
				if (jcl->flags & JCL_EXEC)
					sfprintf(jcl->tp, "%sJOBNAME=%s ", JCL_AUTO, top->name);
				sfprintf(jcl->tp, "%s=%s", fmtexport("STEP"), step->name);
				s = " \\\n";
				for (dd = (Jcldd_t*)dtfirst(jcl->dd); dd; dd = (Jcldd_t*)dtnext(jcl->dd, dd))
					if (!(dd->flags & JCL_DD_REFERENCE))
					{
						sfprintf(jcl->tp, "%s%s=%s", s, fmtexport(dd->name), dsn(jcl, dd, dd->path, 1));
						for (cat = dd->cat; cat; cat = cat->next)
							sfprintf(jcl->tp, "'\n\t'%s", dsn(jcl, dd, cat->path, 1));
					}
				for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
					if (dd->path && !(dd->flags & (JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE)))
					{
						sfprintf(jcl->tp, "%s%s=%s", s, fmtexport(dd->name), dsn(jcl, dd, dd->path, 1));
						for (cat = dd->cat; cat; cat = cat->next)
							sfprintf(jcl->tp, "'\n\t'%s", dsn(jcl, dd, cat->path, 1));
						s = " \\\n";
					}
				del = 0;
				sfprintf(jcl->tp, "%sDDIN='", s);
				s = "";
				for (dd = (Jcldd_t*)dtfirst(jcl->dd); dd; dd = (Jcldd_t*)dtnext(jcl->dd, dd))
					if (!(dd->flags & (JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE)) && dd->disp[0] != JCL_DISP_NEW && dd->disp[0] != JCL_DISP_MOD)
					{
						sfprintf(jcl->tp, "%s%s", s, fmtexport(dd->name));
						s = " ";
					}
				for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
					if (!(dd->flags & (JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE)) && dd->disp[0] != JCL_DISP_NEW && dd->disp[0] != JCL_DISP_MOD)
					{
						sfprintf(jcl->tp, "%s%s", s, fmtexport(dd->name));
						s = " ";
					}
				sfprintf(jcl->tp, "' \\\nDDOUT='");
				s = "";
				for (dd = (Jcldd_t*)dtfirst(jcl->dd); dd; dd = (Jcldd_t*)dtnext(jcl->dd, dd))
					if (!(dd->flags & (JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE)) && (dd->disp[0] == JCL_DISP_NEW || dd->disp[0] == JCL_DISP_MOD))
					{
						sfprintf(jcl->tp, "%s%s", s, fmtexport(dd->name));
						s = " ";
					}
				for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
				{
					if (!(dd->flags & (JCL_DD_SYSIN|JCL_DD_SYSOUT|JCL_DD_SYSERR|JCL_DD_REFERENCE)) && (dd->disp[0] == JCL_DISP_NEW || dd->disp[0] == JCL_DISP_MOD))
					{
						sfprintf(jcl->tp, "%s%s", s, fmtexport(dd->name));
						s = " ";
					}
					if (dd->disp[1] == JCL_DISP_DELETE)
						del |= 1;
					if (dd->disp[2] == JCL_DISP_DELETE)
						del |= 2;
				}
				sfprintf(jcl->tp, "' \\\n%s", step->command);
				if (s = step->parm)
				{
					if (*s == '(')
					{
						arg = strcpy(fmtbuf(strlen(s) + 1), s);
						while (s = jclparm(&arg))
							sfprintf(jcl->vp, ",%s", s);
						if (!(s = sfstruse(jcl->vp)))
							nospace(jcl, NiL);
						if (*s == ',')
							s++;
						for (t = s + strlen(s); t > s && *--t ==  ' ';);
						if (*t == ')')
							*++t = 0;
						else if (*t == ',')
							*t = 0;
					}
					sfprintf(jcl->tp, " %s", fmtquote(s, "$'", "'", strlen(s), FMT_SHELL));
				}
				for (sym = (Jclsym_t*)dtfirst(step->syms); sym; sym = (Jclsym_t*)dtnext(step->syms, sym))
					sfprintf(jcl->tp, " %s=%s", fmtquote(sym->name, "'", "'", strlen(sym->name), FMT_SHELL), fmtquote(sym->value, "$'", "'", strlen(sym->value), 0));
				if (dd = std[0])
				{
					sfprintf(jcl->tp, " < %s", dsn(jcl, dd, dd->path, 0));
					std[3] = 0;
				}
				if (dd = std[1])
					sfprintf(jcl->tp, " > %s", dsn(jcl, dd, dd->path, 0));
				if (dd = std[2])
					sfprintf(jcl->tp, " 2> %s", dsn(jcl, dd, dd->path, 0));
				if (dd = std[3])
					sfprintf(jcl->tp, " <<%s\n%s%s", fmtquote(dd->dlm, "'", "'", strlen(dd->dlm), 1), dd->here, dd->dlm);
				sfprintf(jcl->tp, "\ncode=$?\n");
				if (del && !(jcl->flags & JCL_SUBTMP))
				{
					if (del & 1)
					{
						sfprintf(jcl->tp, "if (( ! $code ))\nthen\n\trm -rf");
						for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
							if (dd->disp[1] == JCL_DISP_DELETE)
								sfprintf(jcl->tp, " %s", dsn(jcl, dd, dd->path, 0));
					}
					if (del & 2)
					{
						if (del & 1)
							sfprintf(jcl->tp, "\nelse\n");
						else
							sfprintf(jcl->tp, "\nif (( $code ))\nthen\n");
						sfprintf(jcl->tp, "\trm -rf");
						for (dd = (Jcldd_t*)dtfirst(step->dd); dd; dd = (Jcldd_t*)dtnext(step->dd, dd))
							if (dd->disp[2] == JCL_DISP_DELETE)
								sfprintf(jcl->tp, " %s", dsn(jcl, dd, dd->path, 0));
					}
					sfprintf(jcl->tp, "\nfi\n");
				}
				if (jcl->flags & JCL_EXEC)
					sfprintf(jcl->tp, "exit $code\n");
				if (!(s = sfstruse(jcl->tp)))
					nospace(jcl, NiL);
			}
			else if ((jcl->flags & (JCL_VERBOSE|JCL_EXEC)) == JCL_VERBOSE)
				sfprintf(sfstdout, ": %s PROC %s\n", step->name, step->command);
			else if ((jcl->flags & (JCL_TRACE|JCL_EXEC)) == (JCL_TRACE|JCL_EXEC))
				sfprintf(sfstderr, "+ : %s PROC %s\n", step->name, step->command);
			if (step->flags & JCL_PGM)
			{
				if ((jcl->flags & (JCL_VERBOSE|JCL_EXEC)) == JCL_VERBOSE)
					sfputr(sfstdout, s, -1);
				if (sfsync(sfstdout))
				{
					if (jcl->disc->errorf)
						(*jcl->disc->errorf)(NiL, jcl->disc, 2, "write error");
					code = -1;
					break;
				}
				if (!(jcl->flags & JCL_EXEC))
					code = 0;
				else if (!(cj = coexec(co, s, CO_APPEND, redirect[0].file, redirect[1].file, NiL)) || !(cj = cowait(co, cj, -1)))
					code = 127;
				else
					code = cj->status;
			}
			else if (jcl->flags & JCL_RECURSE)
				code = jclrun(jcl);
			else
				code = 0;
			if (code)
				jcl->failed++;
			else
				jcl->passed++;
			if (jclrc(jcl, step, code) < 0 && ((jcl->flags & JCL_EXEC) || !(jcl->flags & JCL_LIST)))
				break;
		}
	}
 bad:
	if (jcl == top)
	{
		if (jcl->tmp && (jcl->flags & (JCL_EXEC|JCL_SUBTMP)) == JCL_EXEC)
		{
			sfprintf(jcl->tp, "rm -rf %s*\n", jcl->tmp);
			if (!(s = sfstruse(jcl->tp)))
				nospace(jcl, NiL);
			if (cj = coexec(co, s, CO_APPEND, redirect[0].file, redirect[1].file, NiL))
				cowait(co, cj, -1);
		}
		if (jcl->flags & JCL_GDG)
		{
			s = "gdgupdate";
			if (!(jcl->flags & JCL_EXEC))
				sfputr(sfstdout, s, '\n');
			else if (cj = coexec(co, s, CO_APPEND, redirect[0].file, redirect[1].file, NiL))
				cowait(co, cj, -1);
		}
		if ((jcl->flags & (JCL_SUBDIR|JCL_VERBOSE)) == (JCL_SUBDIR|JCL_VERBOSE))
		{
			if (jcl->flags & JCL_EXEC)
				sfprintf(sfstdout, "COMPLETED AT %s\n", fmttime("%K", time(NiL)));
			else
				sfputr(sfstdout, "date +'COMPLETED AT %K'\n}", '\n');
		}
		if ((jcl->flags & (JCL_EXEC|JCL_SUBDIR)) == (JCL_EXEC|JCL_SUBDIR) && chdir("..") && jcl->disc->errorf)
			(*jcl->disc->errorf)(NiL, jcl->disc, ERROR_SYSTEM|2, "%s: cannot chdir to job subdirectory parent", subdir);
		if ((jcl->flags & (JCL_EXEC|JCL_VERBOSE)) == (JCL_EXEC|JCL_VERBOSE))
		{
			start = tmxgettime() - start;
			user = (co->user - user) / CO_QUANT;
			sys = (co->sys - sys) / CO_QUANT;
			if (pct = (real = (double)start / 1e9))
				pct = 100.0 * ((user + sys) / pct);
			sfprintf(sfstdout, "USAGE CPU=%.2f%% REAL=%.2f USR=%.2f SYS=%.2f\n", pct, real, user, sys);
		}
	}
	code = jclclose(jcl);
	if (scope)
		jclrc(scope, NiL, code);
	return code;
}
