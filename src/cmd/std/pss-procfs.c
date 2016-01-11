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
 * process status stream PSS_METHOD_procfs implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_procfs

NoN(pss_procfs)

#else

#if !defined(pr_clname) && !_mem_pr_clname_prpsinfo
#undef	PSS_sched
#define PSS_sched			0
#endif

#if !defined(pr_gid) && !_mem_pr_gid_prpsinfo
#undef	PSS_gid
#define PSS_gid				0
#endif

#if !defined(pr_lttydev) && !_mem_pr_lttydev_prpsinfo
#undef	_mem_pr_lttydev_prpsinfo
#define _mem_pr_lttydev_prpsinfo	0
#endif

#if !_mem_pr_npid_prpsinfo
#if !defined(pr_ntpid) && !_mem_pr_ntpid_prpsinfo
#undef	PSS_npid
#define PSS_npid			0
#else
#define pr_npid				pr_ntpid
#endif
#endif

#if !defined(pr_pgrp) && !_mem_pr_pgrp_prpsinfo
#if _mem_pr_pgid_prpsinfo
#undef	_mem_pr_pgrp_prpsinfo
#define _mem_pr_pgrp_prpsinfo		1
#define pr_pgrp				pr_pgid
#else
#undef	PSS_pgrp
#define PSS_pgrp			0
#endif
#endif

#if !defined(pr_psargs) && !_mem_pr_psargs_prpsinfo
#undef	_mem_pr_psargs_prpsinfo
#define _mem_pr_psargs_prpsinfo		0
#endif

#if !defined(pr_refcount) && !_mem_pr_refcount_prpsinfo
#undef	PSS_refcount
#define PSS_refcount			0
#endif

#if !defined(pr_rssize) && !_mem_pr_rssize_prpsinfo
#undef	PSS_rss
#define PSS_rss				0
#endif

#if !defined(pr_sonproc) && !_mem_pr_sonproc_prpsinfo
#undef	PSS_proc
#define PSS_proc			0
#endif

#if !defined(pr_sid) && !_mem_pr_sid_prpsinfo
#undef	PSS_sid
#define PSS_sid				0
#define pr_sid				pr_tgrp
#endif

#if !defined(pr_tgrp) && !_mem_pr_tgrp_prpsinfo
#undef	PSS_tgrp
#define PSS_tgrp			0
#define pr_tgrp				pr_pgrp
#endif

typedef struct State_s
{
	struct prpsinfo		pr;
	DIR*			dir;
} State_t;

static int
procfs_init(Pss_t* pss)
{
	register State_t*	state;
	int			fd;

	sfsprintf(pss->buf, sizeof(pss->buf), _PS_path_num, (unsigned long)1, _PS_status);
	if ((fd = open(pss->buf, O_RDONLY|O_BINARY)) < 0)
		return -1;
	close(fd);
	if (!(state = vmnewof(pss->vm, 0, State_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (!(state->dir = opendir(_PS_dir)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|1, "%s: cannot open", _PS_dir);
		return -1;
	}
	pss->data = state;
	return 1;
}

static int
procfs_done(Pss_t* pss)
{
	register State_t*	state = (State_t*)pss->data;

	closedir(state->dir);
	return 1;
}

static int
procfs_read(Pss_t* pss, Pss_id_t pid)
{
	register State_t*	state = (State_t*)pss->data;
	struct dirent*		ent;
	char*			e;

	if (pid)
		pss->pid = pid;
	else
		do
		{
			if (!(ent = readdir(state->dir)))
				return 0;
			pss->pid = (Pss_id_t)strtol(ent->d_name, &e, 10);
		} while (*e);
	return 1;
}

static int
procfs_part(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*		state = (State_t*)pss->data;
	register struct prpsinfo*	pr = &state->pr;
	register int			fd;
	int				n;
#if defined(_PS_scan_binary) || defined(_PS_scan_format)
	struct stat			st;
#endif

	sfsprintf(pss->buf, sizeof(pss->buf), _PS_path_num, (unsigned long)pss->pid, _PS_status);
	if ((fd = open(pss->buf, O_RDONLY|O_BINARY)) < 0)
	{
		if (pss->disc->errorf && ((pss->disc->flags & PSS_VERBOSE) || errno != ENOENT))
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%lu: cannot stat process", pss->pid);
		return 0;
	}
#ifdef _PS_scan_format
	if ((n = read(fd, pss->buf, sizeof(pss->buf))) <= 0 || fstat(fd, &st))
	{
		n = -1;
		errno = EINVAL;
	}
	else
	{
		memset(pr, sizeof(*pr), 0);
		n = sfsscanf(pss->buf, _PS_scan_format, _PS_scan_args(pr));
		if (n < _PS_scan_count)
		{
			register char*	s;

			for (s = pss->buf; *s; s++)
				if (*s == '(')
				{
					for (; *s && *s != ')'; s++)
						if (isspace(*s))
							*s = '_';
					break;
				}
			n = sfsscanf(pss->buf, _PS_scan_format, _PS_scan_args(pr));
			if (n < _PS_scan_count)
				error(1, "%lu: scan count %d, expected at least %d", (unsigned long)pss->pid, n, _PS_scan_count);
		}
#ifdef _PS_scan_fix
		_PS_scan_fix(pr, pe);
#endif
#ifdef _PS_task
		sfsprintf(pss->buf, sizeof(pss->buf), _PS_path_num, (unsigned long)pss->pid, _PS_task);
		(void)stat(pss->buf, &st);
#endif
		pr->pr_uid = st.st_uid;
		pr->pr_gid = st.st_gid;
		pr->pr_nice = pr->pr_priority - 15;
		pr->pr_size /= 1024;
#if _mem_pr_rssize_prpsinfo
		pr->pr_rssize /= 1024;
#endif
#ifdef _PS_scan_boot
		if (!pss->boot)
		{
			register char*	s;
			Sfio_t*		fp;

			pss->boot = 1;
			if (fp = sfopen(NiL, "/proc/stat", "r"))
			{
				while (s = sfgetr(fp, '\n', 0))
					if (strneq(s, "btime ", 6))
					{
						pss->boot = strtol(s + 6, NiL, 10);
						break;
					}
				sfclose(fp);
			}
			if (!(pss->hz = (int)strtol(astconf("CLK_TCK", NiL, NiL), NiL, 0)))
				pss->hz = PR_HZ;
		}
		pr->pr_start = pss->boot + pr->pr_start / pss->hz;
#endif
	}
#else
#ifdef _PS_scan_binary
	n = read(fd, pr, sizeof(*pr)) == sizeof(*pr) ? 1 : -1;
#else
	n = ioctl(fd, PIOCPSINFO, pr);
#endif
#endif
	close(fd);
	if (n < 0)
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%lu: cannot get process info", pss->pid);
		return 0;
	}
#if _UTS
	if (pr->pr_pri > 99)
		pr->pr_pri = 99;
	else if (pr->pr_pri < 0)
		pr->pr_pri = 0;
	pr->pr_ttydev = makedev((pr->pr_ttydev >> 18) & ((1<<14)-1), pr->pr_ttydev & ((1<<18)-1));
#endif
#if PSS_gid
	pe->gid = pr->pr_gid;
#endif
#if PSS_pgrp
	pe->pgrp = pr->pr_pgrp;
#endif
	pe->pid = pr->pr_pid;
#if PSS_sid
	pe->sid = pr->pr_sid;
#endif
	pe->state = PR_ZOMBIE(pr) ? PSS_ZOMBIE : pr->pr_sname;
#if PSS_tgrp
	pe->tgrp = pr->pr_tgrp;
#endif
	pe->tty = (pr->pr_ttydev == (Pss_dev_t)PRNODEV) ? PSS_NODEV : pr->pr_ttydev;
	pe->uid = pr->pr_uid;
	return 1;
}

static int
procfs_full(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*		state = (State_t*)pss->data;
	register struct prpsinfo*	pr = &state->pr;
	unsigned long			fields = pss->disc->fields & pss->meth->fields;
	char*				s;
	int				i;

	if (pe->state != PSS_ZOMBIE)
	{
		if (fields & PSS_args)
		{
#if _mem_pr_psargs_prpsinfo
			s = pr->pr_psargs;
#else
#ifdef _PS_args
			s = "<unknown>";
			sfsprintf(pss->buf, sizeof(pss->buf), _PS_path_num, pe->pid, _PS_args);
			if ((i = open(pss->buf, O_RDONLY|O_BINARY)) >= 0)
			{
				int	n;

				n = read(i, pss->buf, sizeof(pss->buf) - 1);
				close(i);
				if (n > 0)
				{
					s = pss->buf;
					for (i = 0; i < n; i++)
						if (!s[i])
							s[i] = ' ';
					s[i] = 0;
				}
			}
#else
			s = pr->pr_fname;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
#endif
#endif
			pe->args = s;
		}
		if (fields & PSS_command)
		{
			s = pr->pr_fname;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->command = s;
		}
	}
	pe->addr = (void*)pr->pr_addr;
#if _mem_pr_clname_prpsinfo
	pe->sched = pr->pr_clname;
#endif
	pe->cpu = PR_CPU(pr);
	pe->flags = pr->pr_flag;
	pe->nice = pr->pr_nice;
#if _mem_pr_npid_prpsinfo
	pe->npid = pr->pr_npid;
#else
#if _mem_pr_ntpid_prpsinfo
	pe->npid = pr->pr_ntpid;
#endif
#endif
	pe->ppid = pr->pr_ppid;
	pe->pri = pr->pr_pri;
#if _mem_pr_sonproc_prpsinfo
	pe->proc = pr->pr_sonproc;
#endif
#if _mem_pr_refcount_prpsinfo
	pe->refcount = pr->pr_refcount;
#endif
#if _mem_pr_rssize_prpsinfo
	pe->rss = pr->pr_rssize;
#endif
	pe->size = pr->pr_size;
	pe->start = PR_START(pr);
	pe->time = PR_TIME(pr);
	pe->wchan = (void*)pr->pr_wchan;
	return 1;
}

static Pssmeth_t procfs_method =
{
	"/proc",
	"[-version?@(#)$Id: pss /proc (AT&T Research) 2011-12-13 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_all,
	procfs_init,
	procfs_read,
	procfs_part,
	procfs_full,
	0,
	0,
	0,
	procfs_done
};

Pssmeth_t*	_pss_method = &procfs_method;

#endif
