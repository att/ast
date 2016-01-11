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
/*
 * Fault handling routines
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#include	"defs.h"
#include	<fcin.h>
#include	"io.h"
#include	"history.h"
#include	"shlex.h"
#include	"variables.h"
#include	"jobs.h"
#include	"path.h"
#include	"builtins.h"
#include	"ulimit.h"

#define abortsig(sig)	(sig==SIGABRT || sig==SIGBUS || sig==SIGILL || sig==SIGSEGV)

struct Siginfo
{
	siginfo_t	info;
	struct Siginfo	*next;
	struct Siginfo	*last;
};

static char	indone;
static int	cursig = -1;

#if !_std_malloc
#   include	<vmalloc.h>
#endif
#if  defined(VMFL) && (VMALLOC_VERSION>=20031205L)
    /*
     * This exception handler is called after vmalloc() unlocks the region
     */
    static int malloc_done(Vmalloc_t* vm, int type, Void_t* val, Vmdisc_t* dp)
    {
	Shell_t *shp = sh_getinterp();
	dp->exceptf = 0;
	sh_exit(shp,SH_EXITSIG);
	return(0);
    }
#endif

static int	notify_builtin(Shell_t *shp, int sig)
{
	int action = 0;
#ifdef ERROR_NOTIFY
	if(error_info.flags&ERROR_NOTIFY)
		action = (*shp->bltinfun)(-sig,(char**)0,(void*)0);
	if(action>0)
		return(action);
#endif
	if(shp->bltindata.notify)
		shp->bltindata.sigset = 1;
	return(action);
}

#ifdef _lib_sigaction
   static void set_trapinfo(Shell_t *shp, int sig, siginfo_t *info)
   {
	sigset_t	set,oset;
	if(info)
	{
		struct Siginfo *jp,*ip;
		ip = malloc(sizeof(struct Siginfo));
		ip->next = 0;
		memcpy(&ip->info,info,sizeof(siginfo_t));
		if(!(jp=(struct Siginfo*)shp->siginfo[sig]))
		{
			ip->last = ip;
			shp->siginfo[sig] = (void*)ip;
		}
		else
		{
			jp->last->next = ip;
			jp->last = ip;
		}
	}
   }
#endif

/*
 * Most signals caught or ignored by the shell come here
*/
#ifdef _lib_sigaction
void	sh_fault(register int sig, siginfo_t *info, void *context)
#else
void	sh_fault(register int sig)
#endif
{
	int			saved_errno=errno; /* many platforms do not save/restore errno for signal handlers */
	register Shell_t	*shp = sh_getinterp();
	register int 		flag=0;
	register char		*trap;
	register struct checkpt	*pp = (struct checkpt*)shp->jmplist;
	int	action=0;
	if(sig==SIGCHLD)
		sfprintf(sfstdout,"childsig\n");
#ifdef SIGWINCH
	if(sig==SIGWINCH)
		shp->winch = 1;
#endif  /* SIGWINCH */
	trap = shp->st.trapcom[sig];
if(sig==SIGBUS)
{
	signal(sig,SIG_DFL);
	sigrelease(sig);
	kill(getpid(),sig);
}
	if(shp->savesig)
	{
		/* critical region, save and process later */
		if(!(shp->sigflag[sig]&SH_SIGIGNORE))
			shp->savesig = sig;
		goto done;
	}
	/* handle ignored signals */
	if(trap && *trap==0)
		goto done;
	flag = shp->sigflag[sig]&~SH_SIGOFF;
	if(!trap)
	{
		if(sig==SIGINT && (shp->trapnote&SH_SIGIGNORE))
			goto done;
		if(flag&SH_SIGIGNORE)
		{
			if(shp->subshell)
				shp->ignsig = sig;
			goto done;
		}
		if(flag&SH_SIGDONE)
		{
			void *ptr=0;
			if(shp->bltinfun)
				action = notify_builtin(shp,sig);
			if((flag&SH_SIGINTERACTIVE) && sh_isstate(shp,SH_INTERACTIVE) && !sh_isstate(shp,SH_FORKED) && ! shp->subshell)
			{
				/* check for TERM signal between fork/exec */
				if(sig==SIGTERM && job.in_critical)
					shp->trapnote |= SH_SIGTERM;
				goto done;
			}
			shp->lastsig = sig;
			sigrelease(sig);
			if(pp->mode != SH_JMPSUB)
			{
				if(pp->mode < SH_JMPSUB)
					pp->mode = shp->subshell?SH_JMPSUB:SH_JMPFUN;
				else
					pp->mode = SH_JMPEXIT;
			}
			if(shp->subshell)
				sh_exit(shp,SH_EXITSIG);
			if(sig==SIGABRT || (abortsig(sig) && (ptr = malloc(1))))
			{
				if(ptr)
					free(ptr);
				sh_done(shp,sig);
			}
			/* mark signal and continue */
			shp->trapnote |= SH_SIGSET;
			if(sig < shp->gd->sigmax)
				shp->sigflag[sig] |= SH_SIGSET;
#if  defined(VMFL) && (VMALLOC_VERSION>=20031205L)
			if(abortsig(sig))
			{
				/* abort inside malloc, process when malloc returns */
				/* VMFL defined when using vmalloc() */
				Vmdisc_t* dp = vmdisc(Vmregion,0);
				if(dp)
					dp->exceptf = malloc_done;
			}
#endif
			goto done;
		}
	}
	errno = 0;
	if(pp->mode==SH_JMPCMD || (pp->mode==1 && shp->bltinfun) && !(flag&SH_SIGIGNORE))
		shp->lastsig = sig;
	if(trap)
	{
		/*
		 * propogate signal to foreground group
		 */
#ifdef _lib_sigaction
		set_trapinfo(shp,sig,info);
#endif
		if(sig==SIGHUP && job.curpgid)
			killpg(job.curpgid,SIGHUP);
		flag = SH_SIGTRAP;
	}
	else
	{
		shp->lastsig = sig;
		flag = SH_SIGSET;
#ifdef SIGTSTP
		if(sig==SIGTSTP)
		{
			shp->trapnote |= SH_SIGTSTP;
			if(pp->mode==SH_JMPCMD && sh_isstate(shp,SH_STOPOK))
			{
				sigrelease(sig);
				sh_exit(shp,SH_EXITSIG);
				goto done;
			}
		}
#endif /* SIGTSTP */
	}
	if(shp->bltinfun)
		action = notify_builtin(shp,sig);
	if(action>0)
		goto done;
	shp->trapnote |= flag;
#ifdef AST_SERIAL_RESTART
	if(flag&(SH_SIGSET|SH_SIGTRAP))
		astserial(AST_SERIAL_RESTART, AST_SERIAL_except);
#endif
	if(sig < shp->gd->sigmax)
		shp->sigflag[sig] |= flag;
	if(pp->mode==SH_JMPCMD && sh_isstate(shp,SH_STOPOK))
	{
		if(action<0)
			goto done;
		sigrelease(sig);
		sh_exit(shp,SH_EXITSIG);
	}
done:
	errno = saved_errno;
}

/*
 * initialize signal handling
 */
void sh_siginit(void *ptr)
{
	Shell_t	*shp = (Shell_t*)ptr;
	register int sig, n;
	register const struct shtable2	*tp = shtab_signals;
	sig_begin();
	/* find the largest signal number in the table */
#if defined(SIGRTMIN) && defined(SIGRTMAX)
	if ((n = SIGRTMIN) > 0 && (sig = SIGRTMAX) > n && sig < SH_TRAP)
	{
		shp->gd->sigruntime[SH_SIGRTMIN] = n;
		shp->gd->sigruntime[SH_SIGRTMAX] = sig;
	}
#endif /* SIGRTMIN && SIGRTMAX */
	n = SIGTERM;
	while(*tp->sh_name)
	{
		sig = (tp->sh_number&((1<<SH_SIGBITS)-1));
		if (!(sig-- & SH_TRAP))
		{
			if ((tp->sh_number>>SH_SIGBITS) & SH_SIGRUNTIME)
				sig = shp->gd->sigruntime[sig];
			if(sig>n && sig<SH_TRAP)
				n = sig;
		}
		tp++;
	}
	shp->gd->sigmax = ++n;
	shp->st.trapcom = (char**)calloc(n,sizeof(char*));
	shp->sigflag = (unsigned char*)calloc(n,sizeof(char));
	shp->gd->sigmsg = (char**)calloc(n,sizeof(char*));
	shp->siginfo = (void**)calloc(sizeof(void*),shp->gd->sigmax);
	for(tp=shtab_signals; sig=tp->sh_number; tp++)
	{
		n = (sig>>SH_SIGBITS);
		if((sig &= ((1<<SH_SIGBITS)-1)) > (shp->gd->sigmax))
			continue;
		sig--;
		if(n&SH_SIGRUNTIME)
			sig = shp->gd->sigruntime[sig];
		if(sig>=0)
		{
			shp->sigflag[sig] = n;
			if(*tp->sh_name)
				shp->gd->sigmsg[sig] = (char*)tp->sh_value;
		}
	}
}

/*
 * Turn on trap handler for signal <sig>
 */
void	sh_sigtrap(Shell_t *shp,register int sig)
{
	register int flag;
	void (*fun)(int);
	shp->st.otrapcom = 0;
	if(sig==0)
		sh_sigdone(shp);
	else if(!((flag=shp->sigflag[sig])&(SH_SIGFAULT|SH_SIGOFF)))
	{
		/* don't set signal if already set or off by parent */
		if((fun=signal(sig,sh_fault))==SIG_IGN) 
		{
			signal(sig,SIG_IGN);
			flag |= SH_SIGOFF;
		}
		else
		{
			flag |= SH_SIGFAULT;
			if(sig==SIGALRM && fun!=SIG_DFL && fun!=(sh_sigfun_t)sh_fault)
				signal(sig,fun);
		}
		flag &= ~(SH_SIGSET|SH_SIGTRAP);
		shp->sigflag[sig] = flag;
	}
}

/*
 * set signal handler so sh_done is called for all caught signals
 */
void	sh_sigdone(Shell_t *shp)
{
	register int 	flag, sig = shgd->sigmax;
	shp->sigflag[0] |= SH_SIGFAULT;
	while(--sig>=0)
	{
		flag = shp->sigflag[sig];
		if((flag&(SH_SIGDONE|SH_SIGIGNORE|SH_SIGINTERACTIVE)) && !(flag&(SH_SIGFAULT|SH_SIGOFF)))
			sh_sigtrap(shp,sig);
	}
}

/*
 * Restore to default signals
 * Free the trap strings if mode is non-zero
 * If mode>1 then ignored traps cause signal to be ignored 
 */
void	sh_sigreset(Shell_t *shp,register int mode)
{
	register char	*trap;
#ifdef SIGRTMIN
	register int 	flag, sig=SIGRTMIN;
#else
	register int 	flag, sig=shp->st.trapmax;
#endif
	while(sig-- > 0)
	{
		if(trap=shp->st.trapcom[sig])
		{
			flag  = shp->sigflag[sig]&~(SH_SIGTRAP|SH_SIGSET);
			if(*trap)
			{
				if(mode)
					free(trap);
				shp->st.trapcom[sig] = 0;
			}
			else if(sig && mode>1)
			{
				if(sig!=SIGCHLD)
					signal(sig,SIG_IGN);
				flag &= ~SH_SIGFAULT;
				flag |= SH_SIGOFF;
			}
			shp->sigflag[sig] = flag;
		}
	}
	for(sig=SH_DEBUGTRAP-1;sig>=0;sig--)
	{
		if(trap=shp->st.trap[sig])
		{
			if(mode)
				free(trap);
			shp->st.trap[sig] = 0;
		}
		
	}
	shp->st.trapcom[0] = 0;
	if(mode)
		shp->st.trapmax = 0;
	shp->trapnote=0;
}

/*
 * free up trap if set and restore signal handler if modified
 */
void	sh_sigclear(Shell_t *shp,register int sig)
{
	register int flag = shp->sigflag[sig];
	register char *trap;
	shp->st.otrapcom=0;
	if(!(flag&SH_SIGFAULT))
		return;
	flag &= ~(SH_SIGTRAP|SH_SIGSET);
	if(trap=shp->st.trapcom[sig])
	{
		if(!shp->subshell)
			free(trap);
		shp->st.trapcom[sig]=0;
	}
	shp->sigflag[sig] = flag;
}

/*
 * check for traps
 */

void	sh_chktrap(Shell_t* shp)
{
	register int 	sig=shp->st.trapmax;
	register char *trap;
	int count=0;
	if(!(shp->trapnote&~SH_SIGIGNORE))
		sig=0;
	if(sh.intrap)
		return;
	shp->trapnote &= ~SH_SIGTRAP;
	if(shp->winch)
	{
		int rows=0, cols=0;
		int32_t v;
		astwinsize(2,&rows,&cols);
		if(v = cols)
			nv_putval(COLUMNS, (char*)&v, NV_INT32|NV_RDONLY);
		if(v = rows)
			nv_putval(LINES, (char*)&v, NV_INT32|NV_RDONLY);
		shp->winch = 0;
	}
	/* execute errexit trap first */
	if(sh_isstate(shp,SH_ERREXIT) && shp->exitval)
	{
		int	sav_trapnote = shp->trapnote;
		shp->trapnote &= ~SH_SIGSET;
		if(shp->st.trap[SH_ERRTRAP])
		{
			trap = shp->st.trap[SH_ERRTRAP];
			shp->st.trap[SH_ERRTRAP] = 0;
			sh_trap(shp,trap,0);
			shp->st.trap[SH_ERRTRAP] = trap;
		}
		shp->trapnote = sav_trapnote;
		if(sh_isoption(shp,SH_ERREXIT))
		{
			struct checkpt	*pp = (struct checkpt*)shp->jmplist;
			pp->mode = SH_JMPEXIT;
			sh_exit(shp,shp->exitval);
		}
	}
	if(!shp->sigflag)
		return;
	if(shp->sigflag[SIGALRM]&SH_SIGALRM)
		sh_timetraps(shp);
	while(--sig>=0)
	{
		if(sig==cursig)
			continue;
		if((shp->sigflag[sig]&SH_SIGTRAP) || shp->siginfo && shp->siginfo[sig])
		{
			shp->sigflag[sig] &= ~SH_SIGTRAP;
			if(sig==SIGCHLD)
			{
				job_chldtrap(shp,shp->st.trapcom[SIGCHLD],1);
				continue;
			}
			if(trap=shp->st.trapcom[sig])
			{
				struct Siginfo  *ip=0, *ipnext;
retry:
				if(shp->siginfo)
				{
					do ip = shp->siginfo[sig];
					while (asocasptr(&shp->siginfo[sig],ip,0)!=ip);
				}
			again:
#ifdef _lib_sigaction
				if(ip)
				{
					sh_setsiginfo(&ip->info);
					ipnext = ip->next;
				}
				else
					continue;
#endif
				cursig = sig;
 				sh_trap(shp,trap,0);
				count++;
#ifdef _lib_sigaction
				if(ip)
				{
					free(ip);
					if(ip=ipnext)
						goto again;
				}
#endif
				if(shp->siginfo[sig])
					goto retry;
				cursig = -1;
 			}
		}
		if(sig==1 && count)
		{
			count=0;
			sig = shp->st.trapmax;
		}
	}
}


/*
 * exit the current scope and jump to an earlier one based on pp->mode
 */
void sh_exit_20120720(Shell_t *shp,register int xno)
{
	register struct checkpt	*pp = (struct checkpt*)shp->jmplist;
	register int		sig=0;
	register Sfio_t*	pool;
	shp->exitval=xno;
	if(xno==SH_EXITSIG)
		shp->exitval |= (sig=shp->lastsig);
	if(pp && pp->mode>1)
		cursig = -1;
	if(shp->procsub)
		*shp->procsub = 0;
#ifdef SIGTSTP
	if((shp->trapnote&SH_SIGTSTP) && job.jobcontrol)
	{
		/* ^Z detected by the shell */
		shp->trapnote = 0;
		shp->sigflag[SIGTSTP] = 0;
		if(!shp->subshell && sh_isstate(shp,SH_MONITOR) && !sh_isstate(shp,SH_STOPOK))
			return;
		if(sh_isstate(shp,SH_TIMING))
			return;
		/* Handles ^Z for shell builtins, subshells, and functs */
		shp->lastsig = 0;
		sh_onstate(shp,SH_MONITOR);
		sh_offstate(shp,SH_STOPOK);
		shp->trapnote = 0;
		shp->forked = 1;
		if(!shp->subshell && (sig=sh_fork(shp,0,NIL(int*))))
		{
			job.curpgid = 0;
			job.parent = (pid_t)-1;
			job_wait(sig);
			shp->forked = 0;
			job.parent = 0;
			shp->sigflag[SIGTSTP] = 0;
			/* wait for child to stop */
			shp->exitval = (SH_EXITSIG|SIGTSTP);
			/* return to prompt mode */
			pp->mode = SH_JMPERREXIT;
		}
		else
		{
			if(shp->subshell)
				sh_subfork();
			/* child process, put to sleep */
			sh_offstate(shp,SH_STOPOK);
			sh_offstate(shp,SH_MONITOR);
			shp->sigflag[SIGTSTP] = 0;
			/* stop child job */
			killpg(job.curpgid,SIGTSTP);
			/* child resumes */
			job_clear(shp);
			shp->exitval = (xno&SH_EXITMASK);
			return;
		}
	}
#endif /* SIGTSTP */
	/* unlock output pool */
	sh_offstate(shp,SH_NOTRACK);
	if(!(pool=sfpool(NIL(Sfio_t*),shp->outpool,SF_WRITE)))
		pool = shp->outpool; /* can't happen? */
	sfclrlock(pool);
#ifdef SIGPIPE
	if(shp->lastsig==SIGPIPE)
		sfpurge(pool);
#endif /* SIGPIPE */
	sfclrlock(sfstdin);
	if(!pp)
		sh_done(shp,sig);
	shp->intrace = 0;
	shp->prefix = 0;
	shp->mktype = 0;
	if(job.in_critical)
		job_unlock();
	if(pp->mode == SH_JMPSCRIPT && !pp->prev) 
		sh_done(shp,sig);
	if(pp->mode)
		siglongjmp(pp->buff,pp->mode);
}

#undef sh_exit
void sh_exit(register int xno)
{
	Shell_t *shp = sh_getinterp();
	register struct checkpt *pp = (struct checkpt*)shp->jmplist;
	if(pp && pp->mode == SH_JMPIO && xno!=ERROR_NOEXEC)
		pp->mode = SH_JMPERREXIT;
	sh_exit_20120720(shp,xno);
}

static void array_notify(Namval_t *np, void *data)
{
	Namarr_t	*ap = nv_arrayptr(np);
	NOT_USED(data);
	if(ap && ap->fun)
		(*ap->fun)(np, 0, NV_AFREE);
}

/*
 * This is the exit routine for the shell
 */

void sh_done(void *ptr, register int sig)
{
	Shell_t	*shp = (Shell_t*)ptr;
	register char *t;
	register int savxit = shp->exitval;
	shp->trapnote = 0;
	indone=1;
	if(sig)
		savxit = SH_EXITSIG|sig;
	if(shp->userinit)
		(*shp->userinit)(shp, -1);
	if(shp->st.trapcom && (t=shp->st.trapcom[0]))
	{
		shp->st.trapcom[0]=0; /*should free but not long */
		shp->oldexit = savxit;
		sh_trap(shp,t,0);
		savxit = shp->exitval;
	}
	else
	{
		/* avoid recursive call for set -e */
		sh_offstate(shp,SH_ERREXIT);
		sh_chktrap(shp);
	}
	if(shp->var_tree)
		nv_scan(shp->var_tree,array_notify,(void*)0,NV_ARRAY,NV_ARRAY);
	sh_freeup(shp);
#if SHOPT_ACCT
	sh_accend();
#endif	/* SHOPT_ACCT */
	if(mbwide()||sh_isoption(shp,SH_EMACS)||sh_isoption(shp,SH_VI)||sh_isoption(shp,SH_GMACS))
		tty_cooked(-1);
#ifdef JOBS
	if((sh_isoption(shp,SH_INTERACTIVE) && shp->login_sh) || (!sh_isoption(shp,SH_INTERACTIVE) && (sig==SIGHUP)))
		job_walk(shp,sfstderr,job_terminate,SIGHUP,NIL(char**));
#endif	/* JOBS */
	job_close(shp);
	if(shp->var_tree && nv_search("VMTRACE", shp->var_tree,0))
		strmatch((char*)0,(char*)0);
	sfsync((Sfio_t*)sfstdin);
	sfsync((Sfio_t*)shp->outpool);
	sfsync((Sfio_t*)sfstdout);
	if(savxit&SH_EXITSIG)
		sig = savxit&SH_EXITMASK;
	if(sig)
	{
		/* generate fault termination code */
		if(RLIMIT_CORE!=RLIMIT_UNKNOWN)
		{
#ifdef _lib_getrlimit
			struct rlimit rlp;
			getrlimit(RLIMIT_CORE,&rlp);
			rlp.rlim_cur = 0;
			setrlimit(RLIMIT_CORE,&rlp);
#else
			vlimit(RLIMIT_CORE,0);
#endif
		}
		signal(sig,SIG_DFL);
		sigrelease(sig);
		kill(getpid(),sig);
		pause();
	}
#if SHOPT_KIA
	if(sh_isoption(shp,SH_NOEXEC))
		kiaclose((Lex_t*)shp->lex_context);
#endif /* SHOPT_KIA */
	if(shp->pwdfd >=0)
		close(shp->pwdfd);
	exit(savxit&SH_EXITMASK);
}

/*
 * synthesize signal name for sig in buf
 * pfx!=0 prepends SIG to default signal number
 */
static char *sig_name(Shell_t *shp,int sig, char* buf, int pfx)
{
	register int	i;

	i = 0;
	if(sig>shp->gd->sigruntime[SH_SIGRTMIN] && sig<shp->gd->sigruntime[SH_SIGRTMAX])
	{
		buf[i++] = 'R';
		buf[i++] = 'T';
		buf[i++] = 'M';
		if(sig>shp->gd->sigruntime[SH_SIGRTMIN]+(shp->gd->sigruntime[SH_SIGRTMAX]-shp->gd->sigruntime[SH_SIGRTMIN])/2)
		{
			buf[i++] = 'A';
			buf[i++] = 'X';
			buf[i++] = '-';
			sig = shp->gd->sigruntime[SH_SIGRTMAX]-sig;
		}
		else
		{
			buf[i++] = 'I';
			buf[i++] = 'N';
			buf[i++] = '+';
			sig = sig-shp->gd->sigruntime[SH_SIGRTMIN];
		}
	}
	else if(pfx)
	{
		buf[i++] = 'S';
		buf[i++] = 'I';
		buf[i++] = 'G';
	}
	i += sfsprintf(buf+i, 8, "%d", sig);
	buf[i] = 0;
	return buf;
}

static const char trapfmt[] = "trap -- %s %s\n";
/*
 * if <flag> is positive, then print signal name corresponding to <flag>
 * if <flag> is zero, then print all signal names
 * if <flag> is -1, then print all signal names in menu format
 * if <flag> is <-1, then print all traps
 */
void sh_siglist(register Shell_t *shp,Sfio_t *iop,register int flag)
{
	register const struct shtable2	*tp;
	register int sig;
	register char *sname;
	char name[10];
	const char *names[SH_TRAP];
	const char *traps[SH_DEBUGTRAP+1];
	tp=shtab_signals;
	if(flag<=0)
	{
		/* not all signals may be defined, so initialize */
		for(sig=shp->gd->sigmax-1; sig>=0; sig--)
			names[sig] = 0;
		for(sig=SH_DEBUGTRAP; sig>=0; sig--)
			traps[sig] = 0;
	}
	for(; *tp->sh_name; tp++)
	{
		sig = tp->sh_number&((1<<SH_SIGBITS)-1);
		if (((tp->sh_number>>SH_SIGBITS) & SH_SIGRUNTIME) && (sig = shp->gd->sigruntime[sig-1]+1) == 1)
			continue;
		if(sig==flag)
		{
			sfprintf(iop,"%s\n",tp->sh_name);
			return;
		}
		else if(sig&SH_TRAP)
			traps[sig&~SH_TRAP] = (char*)tp->sh_name;
		else if(sig-- && sig < elementsof(names))
			names[sig] = (char*)tp->sh_name;
	}
	if(flag > 0)
		sfputr(iop, sig_name(shp,flag-1,name,0), '\n');
	else if(flag<-1)
	{
		/* print the traps */
		register char *trap,**trapcom;
		sig = shp->st.trapmax;
		/* use parent traps if otrapcom is set (for $(trap)  */
		trapcom = (shp->st.otrapcom?shp->st.otrapcom:shp->st.trapcom);
		while(--sig >= 0)
		{
			if(!(trap=trapcom[sig]))
				continue;
			if(sig >= shp->gd->sigmax || !(sname=(char*)names[sig]))
				sname = sig_name(shp,sig,name,1);
			sfprintf(iop,trapfmt,sh_fmtq(trap),sname);
		}
		for(sig=SH_DEBUGTRAP; sig>=0; sig--)
		{
			if(!(trap=shp->st.otrap?shp->st.otrap[sig]:shp->st.trap[sig]))
				continue;
			sfprintf(iop,trapfmt,sh_fmtq(trap),traps[sig]);
		}
	}
	else
	{
		/* print all the signal names */
		for(sig=1; sig < shp->gd->sigmax; sig++)
		{
			if(!(sname=(char*)names[sig]))
			{
				sname = sig_name(shp,sig,name,1);
				if(flag)
					sname = stkcopy(shp->stk,sname);
			}
			if(flag)
				names[sig] = sname;
			else
				sfputr(iop,sname,'\n');
		}
		if(flag)
		{
			names[sig] = 0;
			sh_menu(shp,iop,shp->gd->sigmax,(char**)names+1);
		}
	}
}

/*
 * parse and execute the given trap string, stream or tree depending on mode
 * mode==0 for string, mode==1 for stream, mode==2 for parse tree
 */

int sh_trap_20120720(Shell_t *shp,const char *trap, int mode)
{
	int	jmpval, savxit = shp->exitval;
	int	was_history = sh_isstate(shp,SH_HISTORY);
	int	was_verbose = sh_isstate(shp,SH_VERBOSE);
	int	staktop = stktell(shp->stk);
	char	*savptr = stkfreeze(shp->stk,0);
	char	ifstable[256];
	struct	checkpt buff;
	Fcin_t	savefc;
	fcsave(&savefc);
	memcpy(ifstable,shp->ifstable,sizeof(ifstable));
	sh_offstate(shp,SH_HISTORY);
	sh_offstate(shp,SH_VERBOSE);
	shp->intrap++;
	sh_pushcontext(shp,&buff,SH_JMPTRAP);
	jmpval = sigsetjmp(buff.buff,0);
	if(jmpval == 0)
	{
		if(mode==2)
			sh_exec(shp,(Shnode_t*)trap,sh_isstate(shp,SH_ERREXIT));
		else
		{
			Sfio_t *sp;
			if(mode)
				sp = (Sfio_t*)trap;
			else
				sp = sfopen(NIL(Sfio_t*),trap,"s");
			sh_eval(shp,sp,0);
		}
	}
	else if(indone)
	{
		if(jmpval==SH_JMPSCRIPT)
			indone=0;
		else
		{
			if(jmpval==SH_JMPEXIT)
				savxit = shp->exitval;
			jmpval=SH_JMPTRAP;
		}
	}
	sh_popcontext(shp,&buff);
	shp->intrap--;
	sfsync(shp->outpool);
	if(!shp->indebug && jmpval!=SH_JMPEXIT && jmpval!=SH_JMPFUN)
		shp->exitval=savxit;
	stkset(shp->stk,savptr,staktop);
	fcrestore(&savefc);
	memcpy(shp->ifstable,ifstable,sizeof(ifstable));
	if(was_history)
		sh_onstate(shp,SH_HISTORY);
	if(was_verbose)
		sh_onstate(shp,SH_VERBOSE);
	exitset(shp);
	if(jmpval>SH_JMPTRAP && (((struct checkpt*)shp->jmpbuffer)->prev || ((struct checkpt*)shp->jmpbuffer)->mode==SH_JMPSCRIPT))
		siglongjmp(*shp->jmplist,jmpval);
	return(shp->exitval);
}

#undef sh_trap
int sh_trap(const char *trap, int mode)
{
	Shell_t	*shp = sh_getinterp();
	return(sh_trap_20120720(shp,trap,mode));
}

#ifdef _lib_sigaction
#   undef signal
    sh_sigfun_t sh_signal(int sig ,sh_sigfun_t func)
    {
	struct sigaction sigin, sigout;
	memset(&sigin,0,sizeof(struct sigaction));
	if(func== SIG_DFL || func== SIG_IGN)
		sigin.sa_handler = func;
	else
	{
		sigin.sa_sigaction = (void (*)(int, siginfo_t*,void*))func;
		sigin.sa_flags = SA_SIGINFO;
	}
	sigaction(sig,&sigin,&sigout);
	sigunblock(sig);
	return((sh_sigfun_t)sigout.sa_sigaction);
    }
    
#endif

