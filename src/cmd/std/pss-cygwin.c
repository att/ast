/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * process status stream PSS_METHOD_cygwin implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_cygwin

NoN(pss_cygwin)

#else

#ifndef PR_HZ
#define PR_HZ			100
#endif
#ifndef PRNODEV
#define PRNODEV			((Pss_dev_t)(-1))
#endif
#ifndef PID_ZOMBIE
#define PID_ZOMBIE		0
#endif

#undef	PR_CTIME
#define PR_CTIME(p)		(((p)->rusage_children.ru_utime.tv_sec+(p)->rusage_children.ru_stime.tv_sec)*PR_HZ+((((p)->rusage_children.ru_utime.tv_usec+(p)->rusage_children.ru_stime.tv_usec)*PR_HZ)/1000000))
#undef	PR_START
#define PR_START(p)		((p)->start_time)
#undef	PR_TIME
#define PR_TIME(p)		(((p)->rusage_self.ru_utime.tv_sec+(p)->rusage_self.ru_stime.tv_sec)*PR_HZ+((((p)->rusage_self.ru_utime.tv_usec+(p)->rusage_self.ru_stime.tv_usec)*PR_HZ)/1000000))

#undef	PSS_addr
#define PSS_addr	0
#undef	PSS_sched
#define PSS_sched	0
#undef	PSS_cpu
#define PSS_cpu		0
#undef	PSS_nice
#define PSS_nice	0
#undef	PSS_pri
#define PSS_pri		0
#undef	PSS_proc
#define PSS_proc	0
#undef	PSS_refcount
#define PSS_refcount	0
#undef	PSS_tgrp
#define PSS_tgrp	0
#undef	PSS_wchan
#define PSS_wchan	0

static int
cygwin_init(Pss_t* pss)
{
	cygwin_internal(CW_LOCK_PINFO, 1000);
	return 1;
}

static int
cygwin_done(Pss_t* pss)
{
	cygwin_internal(CW_UNLOCK_PINFO);
	return 1;
}

static int
cygwin_read(Pss_t* pss, Pss_id_t pid)
{
	if (pid)
		pss->pid = pid;
	else
	{
		if (!pss->data)
			pss->pid = 0;
		else if (!(pss->pid = ((struct external_pinfo*)pss->data)->pid))
		{
			pss->data = 0;
			return 0;
		}
		pss->pid |= CW_NEXTPID;
	}
	return 1;
}

static int
cygwin_part(register Pss_t* pss, register Pssent_t* pe)
{
	register struct external_pinfo*	pr;

	if (!(pr = (struct external_pinfo*)cygwin_internal(CW_GETPINFO, pss->pid)) || !pr->pid)
		return 0;
	pss->data = (void*)pr;
	pe->gid = pr->gid;
	pe->pgrp = pr->pgid;
	pe->sid = pr->sid;
	pe->tty = pr->ctty == PRNODEV ? PSS_NODEV : pr->ctty;
	pe->uid = pr->uid;
	if (pr->process_state & (PID_EXITED|PID_ZOMBIE))
		pe->state = PSS_ZOMBIE;
	else if (pr->process_state & PID_STOPPED)
		pe->state = 'S';
	else if (pr->process_state & PID_TTYIN)
		pe->state = 'I';
	else if (pr->process_state & PID_TTYOU)
		pe->state = 'O';
	else
		pe->state = ' ';
	return 1;
}

static int
cygwin_full(register Pss_t* pss, register Pssent_t* pe)
{
	register struct external_pinfo*	pr = (struct external_pinfo*)pss->data;
	unsigned long			fields = pss->disc->fields & pss->meth->fields;
	char*				s;
	int				i;

	if (pe->state != PSS_ZOMBIE)
	{
		if (fields & PSS_args)
		{
			cygwin_conv_to_posix_path(pr->progname, pss->buf);
			s = pss->buf;
			if ((i = strlen(s)) > 4 && !strcasecmp(s + i - 4, ".exe"))
				*(s + i - 4) = 0;
			pe->args = s;
		}
		if (fields & PSS_command)
		{
			cygwin_conv_to_posix_path(pr->progname, pss->buf);
			if (s = strrchr(pss->buf, '/'))
				s++;
			else
				s = pss->buf;
			if ((i = strlen(s)) > 4 && !strcasecmp(s + i - 4, ".exe"))
				*(s + i - 4) = 0;
			pe->command = s;
		}
	}
	pe->flags = pr->process_state;
	pe->pid = pr->pid;
	pe->npid = pr->dwProcessId;
	pe->ppid = pr->ppid;
	pe->rss = pr->rusage_self.ru_maxrss;
	pe->size = pr->rusage_self.ru_idrss;
	pe->start = PR_START(pr);
	pe->time = PR_TIME(pr);
	return 1;
}

static char*
cygwin_ttyname(Pss_t* pss, register Pssent_t* pe)
{
	if (pe->tty < 0)
		return "?";
	if (pe->tty == (Pss_dev_t)TTY_CONSOLE)
		return "con";
	sfsprintf(pss->buf, sizeof(pss->buf), "%I*d", sizeof(pe->tty), pe->tty);
	return pss->buf;
}

static Pss_dev_t
cygwin_ttymap(Pss_t* pss, Pss_dev_t dev)
{
	return minor(dev);
}

static Pssmeth_t cygwin_method =
{
	"cygwin",
	"[-version?@(#)$Id: pss cygwin (AT&T Research) 2003-02-01 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_all,
	cygwin_init,
	cygwin_read,
	cygwin_part,
	cygwin_full,
	0,
	cygwin_ttyname,
	cygwin_ttymap,
	cygwin_done
};

Pssmeth_t*	_pss_method = &cygwin_method;

#endif
