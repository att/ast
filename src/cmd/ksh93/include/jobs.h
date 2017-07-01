/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
#ifndef JOB_NFLAG
/*
 *	Interface to job control for shell
 *	written by David Korn
 *
 */

#define JOBTTY	2

#include	<ast.h>
#include	<sfio.h>
#ifndef SIGINT
#   include	<signal.h>
#endif /* !SIGINT */
#include	"FEATURE/options"
#include	"FEATURE/externs"
#if	_hdr_aso
#   include	<aso.h>
#else
#   define ASO_LOCK	1
#   define ASO_UNLOCK	2
#   define asolock(a,b,c)
#endif /* _hdr_aso */

#if SHOPT_COSHELL
#   include	<coshell.h>
#   define	COPID_BIT	(1L<<30)
    struct cosh
    {
	struct cosh	*next;
	Coshell_t	*coshell;
	Cojob_t		*cojob;
	char		*name;
	short		id;
    };

    extern pid_t	sh_copid(struct cosh*);
    extern char  	*sh_pid2str(Shell_t*,pid_t);
#endif /* SHOPT_COSHELL */

#undef JOBS
#if defined(SIGCLD) && !defined(SIGCHLD)
#   define SIGCHLD	SIGCLD
#endif
#ifdef SIGCHLD
#   define JOBS	1
#   include	"terminal.h"
#   ifdef FIOLOOKLD
	/* Ninth edition */
	extern int tty_ld, ntty_ld;
#	define OTTYDISC	tty_ld
#	define NTTYDISC	ntty_ld
#   endif	/* FIOLOOKLD */
#else
#   undef SIGTSTP
#   undef SH_MONITOR
#   define SH_MONITOR	0
#   define job_set(x)
#   define job_reset(x)
#endif

struct process
{
	struct process *p_nxtjob;	/* next job structure */
	struct process *p_nxtproc;	/* next process in current job */
	Shell_t		*p_shp;		/* shell that posted the job */
	char		*p_curdir;	/* current direcory at job start */
#if SHOPT_COSHELL
	Cojob_t		*p_cojob;	/* coshell job */
#endif /* SHOPT_COSHELL */
	int		*p_exitval;	/* place to store the exitval */
	int		p_wstat;
	pid_t		p_pid;		/* process id */
	pid_t		p_pgrp;		/* process group */
	pid_t		p_fgrp;		/* process group when stopped */
	short		p_job;		/* job number of process */
	unsigned short	p_exit;		/* exit value or signal number */
	unsigned short	p_exitmin;	/* minimum exit value for xargs */
	unsigned short	p_flag;		/* flags - see below */
	long		p_env;		/* subshell environment number */
#ifdef JOBS
	off_t		p_name;		/* history file offset for command */
	struct termios	p_stty;		/* terminal state for job */
#endif /* JOBS */
};

struct jobs
{
	struct process	*pwlist;	/* head of process list */
	int		*exitval;	/* pipe exit values */
	pid_t		curpgid;	/* current process gid id */
	pid_t		parent;		/* set by fork() */
	pid_t		mypid;		/* process id of shell */
	pid_t		mypgid;		/* process group id of shell */
	pid_t		mytgid;		/* terminal group id of shell */
	pid_t		lastpost;	/* last job posted */
	int		curjobid;
	unsigned int	in_critical;	/* >0 => in critical region */
	int		savesig;	/* active signal */
	int		numpost;	/* number of posted jobs */
	int		numbjob;	/* number of background jobs */
	short		fd;		/* tty descriptor number */
	short		maxjob;		/* must reap after maxjob if > 0 */
#ifdef JOBS
	int		suspend;	/* suspend character */
	int		linedisc;	/* line dicipline */
#endif /* JOBS */
	char		jobcontrol;	/* turned on for real job control */
	char		waitsafe;	/* wait will not block */
	char		waitall;	/* wait for all jobs in pipe */
	char		toclear;	/* job table needs clearing */
	int		asol;		/* used for asolock */
	unsigned char	*freejobs;	/* free jobs numbers */
#if SHOPT_COSHELL
	struct cosh	*colist;	/* coshell job list */
#endif /* SHOPT_COSHELL */
};

/* flags for joblist */
#define JOB_LFLAG	1
#define JOB_NFLAG	2
#define JOB_PFLAG	4
#define JOB_NLFLAG	8
#define JOB_QFLAG	0x100
#define JOB_QQFLAG	0x200

extern struct jobs job;

#ifdef JOBS

#if !_std_malloc
#   include <vmalloc.h>
#   ifdef vmlocked
#	define vmbusy()	vmlocked(Vmregion)
#   else
#	if VMALLOC_VERSION >= 20130509L
#	    define vmbusy()	(vmstat(Vmregion,0)!=0)
#	else
#	    if VMALLOC_VERSION >= 20070911L
#		define vmbusy()	(vmstat(0,0)!=0)
#	    endif
#	endif
#   endif
#endif
#ifndef vmbusy
#   define vmbusy()	0
#endif

#if _hdr_aso

#define job_lock()	asoincint(&job.in_critical)
#define job_unlock()	\
	do { \
		int	_sig; \
		if (asogetint(&job.in_critical) == 1 && (_sig = job.savesig) && !vmbusy()) \
			job_reap(_sig); \
		asodecint(&job.in_critical); \
	} while(0)

#else

#define job_lock()	(job.in_critical++)
#define job_unlock()	\
	do { \
		int	_sig; \
		if (!--job.in_critical && (_sig = job.savesig)) \
		{ \
			if (!job.in_critical++ && !vmbusy()) \
				job_reap(_sig); \
			job.in_critical--; \
		} \
	} while(0)

#endif

extern const char	e_jobusage[];
extern const char	e_done[];
extern const char	e_running[];
extern const char	e_coredump[];
extern const char	e_no_proc[];
extern const char	e_no_job[];
extern const char	e_badpid[];
extern const char	e_jobsrunning[];
extern const char	e_nlspace[];
extern const char	e_access[];
extern const char	e_terminate[];
extern const char	e_no_jctl[];
extern const char	e_signo[];
#ifdef SIGTSTP
   extern const char	e_no_start[];
#endif /* SIGTSTP */
#ifdef NTTYDISC
   extern const char	e_newtty[];
   extern const char	e_oldtty[];
#endif /* NTTYDISC */
#endif	/* JOBS */

/*
 * The following are defined in jobs.c
 */

extern void	job_clear(Shell_t*);
extern void	job_bwait(char**);
extern int	job_walk(Shell_t*,Sfio_t*,int(*)(struct process*,int),int,char*[]);
extern int	job_kill(struct process*,int);
extern bool	job_wait(pid_t);
extern int	job_post(Shell_t*,pid_t,pid_t);
extern void	*job_subsave(void);
extern void	job_subrestore(Shell_t*,void*);
extern void	job_chldtrap(Shell_t*, const char*,int);
#ifdef JOBS
	extern void	job_init(Shell_t*,int);
	extern int	job_close(Shell_t*);
	extern int	job_list(struct process*,int);
	extern int	job_terminate(struct process*,int);
	extern int	job_switch(struct process*,int);
	extern void	job_fork(pid_t);
	extern bool	job_reap(int);
#else
#	define job_init(s,flag)
#	define job_close(s)	(0)
#	define job_fork(p)
#endif	/* JOBS */


#endif /* !JOB_NFLAG */
