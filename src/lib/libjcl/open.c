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
 * jcl handle open/close
 */

#include "jcllib.h"

Redirect_t	redirect[] =
{
	{ "SYSOUT",	1	},
	{ "SYSERR",	2	},
};

/*
 * open a handle to the jcl deck
 */

Jcl_t*
jclopen(Jcl_t* scope, const char* file, unsigned long flags, Jcldisc_t* disc)
{
	register Jcl_t*	jcl;
	Dt_t*		dt;
	Sfio_t*		sp;
	Vmalloc_t*	vm;
	Vmalloc_t*	vs;
	int		i;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)) || !(vs = vmopen(Vmdcheap, Vmbest, 0)))
	{
		nospace(NiL, disc);
		if (vm)
			vmclose(vm);
		return 0;
	}
	if (!(jcl = vmnewof(vm, 0, Jcl_t, 1, 0)))
	{
		nospace(NiL, disc);
		vmclose(vm);
		vmclose(vs);
		return 0;
	}
	jcl->id = "jcl";
	jcl->vs = jcl->vm = vm;
	jcl->vx = vs;
	jcl->disc = disc;
	jcl->flags = flags & JCL_INHERIT;
	jcl->main = (jcl->scope = scope) && scope->main->name ? scope->main : jcl;
	jcl->step = &jcl->current;
	for (i = 0; i < elementsof(redirect); i++)
		jcl->redirect[i] = dup(redirect[i].fd);
	if (!(jcl->cp = sfstropen()) || !(jcl->rp = sfstropen()) || !(jcl->tp = sfstropen()) || !(jcl->vp = sfstropen()) || !(jcl->xp = sfstropen()))
		goto mem;
	jcl->dddisc.link = offsetof(Jcldd_t, link);
	jcl->dddisc.key = offsetof(Jcldd_t, name);
	jcl->dddisc.size = -1;
	jcl->outdirdisc.link = offsetof(Jcldir_t, link);
	jcl->outdirdisc.key = offsetof(Jcldir_t, name);
	jcl->outdirdisc.size = 0;
	jcl->outputdisc.link = offsetof(Jcloutput_t, link);
	jcl->outputdisc.key = offsetof(Jcloutput_t, name);
	jcl->outputdisc.size = -1;
	jcl->rcdisc.link = offsetof(Rc_t, link);
	jcl->rcdisc.key = offsetof(Rc_t, name);
	jcl->rcdisc.size = 0;
	jcl->symdisc.link = offsetof(Jclsym_t, link);
	jcl->symdisc.key = offsetof(Jclsym_t, name);
	jcl->symdisc.size = 0;
	if (!(jcl->dd = dtnew(jcl->vm, &jcl->dddisc, Dtoset)) ||
	    !(jcl->ds = dtnew(jcl->vm, &jcl->dddisc, Dtoset)) ||
	    !(jcl->outdir = dtnew(jcl->vm, &jcl->outdirdisc, Dtoset)) ||
	    !(jcl->output = dtnew(jcl->vm, &jcl->outputdisc, Dtoset)) ||
	    !(jcl->rcs = dtnew(jcl->vm, &jcl->rcdisc, Dtoset)) ||
	    !(jcl->syms = dtnew(jcl->vm, &jcl->symdisc, Dtoset)) ||
	    !(jcl->ss = dtnew(jcl->vm, &jcl->symdisc, Dtoset)) ||
	    !(jcl->step->syms = dtnew(jcl->vm, &jcl->symdisc, Dtoset)))
		goto mem;
	jcl->step->dd = jcl->ds;
	jcl->step->syms = jcl->ss;
	if (!(flags & JCL_SCOPE))
	{
		if (!file || streq(file, "-") || streq(file, "/dev/stdin"))
		{
			file = 0;
			sp = sfstdin;
		}
		else if (!(file = jclfind(jcl, file, flags, 2, &sp)))
			goto bad;
		if (jclpush(jcl, sp, file, 0))
			goto bad;
		if (!jclstep(jcl))
			goto bad;
		dt = jcl->dd;
		jcl->dd = jcl->ds;
		jcl->ds = dt;
		dt = jcl->output;
		if (!jcl->name && error_info.line && jcl->disc->errorf)
			(*jcl->disc->errorf)(NiL, jcl->disc, 1, "no JOB or PROC statement");
	}
	jcl->vs = vs;
	return jcl;
 mem:
	nospace(jcl, NiL);
 bad:
	jclclose(jcl);
	return 0;
}

/*
 * close jcl handle
 */

int
jclclose(Jcl_t* jcl)
{
	int	i;
	int	r;

	if (!jcl)
		return -1;
	if ((r = jcl->rc) < 0)
		r = 256 + 6;
	sfsync(sfstdout);
	sfsync(sfstderr);
	for (i = 0; i < elementsof(redirect); i++)
		if (jcl->redirect[i] >= 0)
			dup2(jcl->redirect[i], redirect[i].fd);
	if (!jcl->scope && jclstats(sfstdout, jcl->flags, jcl->disc))
		r = -1;
	while (jclpop(jcl) > 0);
	if (jcl->cp)
		sfclose(jcl->cp);
	if (jcl->dp)
		sfclose(jcl->dp);
	if (jcl->rp)
		sfclose(jcl->rp);
	if (jcl->tp)
		sfclose(jcl->tp);
	if (jcl->vp)
		sfclose(jcl->vp);
	if (jcl->xp)
		sfclose(jcl->xp);
	vmclose(jcl->vx);
	vmclose(jcl->vm);
	return r;
}

/*
 * push an include file
 */

int
jclpush(register Jcl_t* jcl, Sfio_t* sp, const char* file, long line)
{
	register Include_t*	ip;

	if (!(ip = vmnewof(jcl->vm, 0, Include_t, 1, file ? strlen(file) : 0)))
	{
		nospace(jcl, NiL);
		return -1;
	}
	ip->sp = jcl->sp;
	jcl->sp = sp;
	ip->file = error_info.file;
	error_info.file = file ? strcpy(ip->path, file) : (char*)0;
	ip->line = error_info.line;
	error_info.line = 0;
	ip->prev = jcl->include;
	jcl->include = ip;
	if (file && (jcl->flags & (JCL_LISTJOBS|JCL_LISTSCRIPTS)))
		uniq(file, NiL, JCL_LISTSCRIPTS, jcl->disc);
	return 0;
}

/*
 * pop the top include file
 * return
 *	>0 some includes left
 *       0 last include popped
 *      <0 no includes left
 */

int
jclpop(Jcl_t* jcl)
{
	Include_t*	ip;

	if (!(ip = jcl->include))
		return -1;
	jcl->include = ip->prev;
	if (jcl->sp != sfstdin)
		sfclose(jcl->sp);
	jcl->sp = ip->sp;
	error_info.file = ip->file;
	error_info.line = ip->line;
	vmfree(jcl->vm, ip);
	return jcl->sp != 0;
}
