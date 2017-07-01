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
 * process status stream PSS_METHOD_kvm implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_kvm

NoN(pss_kvm)

#else

#include <kvm.h>
#if _sys_time
#include <sys/time.h>
#endif
#if _sys_param
#include <sys/param.h>
#endif
#if _sys_proc
#include <sys/proc.h>
#endif
#include <sys/sysctl.h>
#include <sys/tty.h>

#if !_mem_p_pid_extern_proc
#define extern_proc		proc
#endif

#ifndef SIDL
#ifdef	LSIDL
#define SIDL	LSIDL
#else
#define SIDL	1
#endif
#endif
#ifndef SRUN
#ifdef	LSRUN
#define SRUN	LSRUN
#else
#define SRUN	2
#endif
#endif
#ifndef SSLEEP
#ifdef	LSSLEEP
#define SSLEEP	LSSLEEP
#else
#define SSLEEP	3
#endif
#endif
#ifndef SSTOP
#ifdef	LSSTOP
#define SSTOP	LSSTOP
#else
#define SSTOP	4
#endif
#endif
#ifndef SZOMB
#ifdef	LSZOMB
#define SZOMB	LSZOMB
#else
#define SZOMB	5
#endif
#endif
#ifndef SDEAD
#ifdef	LSDEAD
#define SDEAD	LSDEAD
#else
#define SDEAD	6
#endif
#endif
#ifndef SONPROC
#ifdef	LSONPROC
#define SONPROC	LSONPROC
#else
#define SONPROC	7
#endif
#endif
#ifndef SSUSPENDED
#ifdef	LSSUSPENDED
#define SSUSPENDED	LSSUSPENDED
#else
#define SSUSPENDED	8
#endif
#endif

typedef struct State_s
{
	kvm_t*			kd;
	struct kinfo_proc*	kp;
	struct kinfo_proc*	ke;
	struct extern_proc*	pr;
	struct eproc*		px;
} State_t;

static int
kvm_init(Pss_t* pss)
{
	register State_t*	state;

	if (!(state = vmnewof(pss->vm, 0, State_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (!(state->kd = kvm_open(NiL, NiL, 0, O_RDONLY, NiL)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|1, "kvm open error");
		return -1;
	}
	pss->data = state;
	error(1, "%s: pss method is currently incomplete", pss->meth->name);
	return 1;
}

static int
kvm_done(Pss_t* pss)
{
	register State_t*	state = (State_t*)pss->data;

	kvm_close(state->kd);
	return 1;
}

static int
kvm_readf(Pss_t* pss, Pss_id_t pid)
{
	register State_t*	state = (State_t*)pss->data;
	int			count;

	if (pid)
	{
		if (!(state->kp = kvm_getprocs(state->kd, KERN_PROC_PID, pid, &count)))
			return -1;
		state->ke = state->kp + count;
	}
	else if (!state->kp)
	{
		if (!(state->kp = kvm_getprocs(state->kd, KERN_PROC_ALL, 0, &count)))
			return -1;
		state->ke = state->kp + count;
	}
	if (state->kp >= state->ke)
		return 0;
	pss->pid = state->kp->kp_proc.p_pid;
	state->pr = &state->kp->kp_proc;
	state->px = &state->kp->kp_eproc;
	state->kp++;
	return 1;
}

static int
kvm_part(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*	state = (State_t*)pss->data;

	pe->pid = state->pr->p_pid;
	pe->pgrp = state->px->e_pgid;
	pe->tty = state->px->e_tdev;
	pe->uid = state->px->e_ucred.cr_uid;
#if 0
	pe->sid = state->px->e_sess->s_sid;
#endif
	switch (state->pr->p_stat)
	{
	case SIDL:	pe->state = 'I'; break;
	case SRUN:	pe->state = 'R'; break;
	case SSLEEP:	pe->state = 'S'; break;
	case SSTOP:	pe->state = 'T'; break;
	case SZOMB:	pe->state = 'Z'; break;
	default:	pe->state = 'O'; break;
	}
	return 1;
}

static int
kvm_full(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*		state = (State_t*)pss->data;
	unsigned long			fields = pss->disc->fields & pss->meth->fields;
	char*				s;
	int				i;

	if (pe->state != PSS_ZOMBIE)
	{
		if (fields & PSS_args)
		{
			s = state->pr->p_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->args = s;
		}
		if (fields & PSS_command)
		{
			s = state->pr->p_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->command = s;
		}
	}
#if _mem_p_addr_extern_proc
	pe->addr = state->pr->p_addr;
#endif
#if _mem_p_wchan_extern_proc
	pe->wchan = state->pr->p_wchan;
#endif
	pe->flags = state->pr->p_flag;
	pe->nice = state->pr->p_nice;
	pe->ppid = state->px->e_ppid;
#if PSS_pri && _mem_p_usrpri_extern_proc
	pe->pri = state->pr->p_usrpri;
#endif
#ifdef FWIDTH
	pe->cpu = state->pr->p_pctcpu >> FWIDTH;
#endif
	pe->refcount = state->px->e_xccount;
	pe->rss = state->px->e_xrssize;
#if _mem_e_xsize_eproc
	pe->size = state->px->e_xsize;
#else
	pe->size = state->px->e_vm.vm_tsize + state->px->e_vm.vm_dsize + state->px->e_vm.vm_ssize;
#endif
#if _mem_p_starttime_extern_proc
	pe->start = state->pr->p_starttime.tv_sec;
#endif
	pe->time = state->pr->p_rtime.tv_sec;
	return 1;
}

static Pssmeth_t kvm_method =
{
	"kvm",
	"[-version?@(#)$Id: pss kvm (AT&T Research) 2008-01-31 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_all,
	kvm_init,
	kvm_readf,
	kvm_part,
	kvm_full,
	0,
	0,
	0,
	kvm_done
};

Pssmeth_t*	_pss_method = &kvm_method;

#endif
