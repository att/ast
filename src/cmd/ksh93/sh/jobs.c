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
//
//  Job control for UNIX Shell
//
//   David Korn
//   AT&T Labs
//
//  Written October, 1982
//  Rewritten April, 1988
//  Revised January, 1992
//
#include "config_ast.h"  // IWYU pragma: keep

#include "defs.h"

#include <wait.h>
#include "aso.h"
#include "history.h"
#include "io.h"
#include "jobs.h"
#include "path.h"
#include "variables.h"

#if !defined(WCONTINUED) || !defined(WIFCONTINUED) || defined(__APPLE__)
#undef WCONTINUED
#define WCONTINUED 0
#undef WIFCONTINUED
#define WIFCONTINUED(wstat) (0)
#endif

#define NJOB_SAVELIST 4

//
// Temporary hack to get W* macros to work.
//
#undef wait
#define wait ______wait
//
// This struct saves a link list of processes that have non-zero exit status,
// have had $! saved, but haven't been waited for.
//
struct jobsave {
    struct jobsave *next;
    pid_t pid;
    unsigned short exitval;
    unsigned short env;
};

static struct jobsave *job_savelist;
static int njob_savelist;
static struct process *pwfg;
static int jobfork;
static siginfo_t Siginfo;

pid_t pid_fromstring(char *str) {
    pid_t pid;
    char *last;
    errno = 0;
    if (sizeof(pid) == sizeof(Sflong_t)) {
        pid = (pid_t)strtoll(str, &last, 10);
    } else {
        pid = (pid_t)strtol(str, &last, 10);
    }
    if (errno == ERANGE || *last) errormsg(SH_DICT, ERROR_exit(1), "%s: invalid process id", str);
    return pid;
}

static void init_savelist(void) {
    struct jobsave *jp;
    while (njob_savelist < NJOB_SAVELIST) {
        jp = newof(0, struct jobsave, 1, 0);
        jp->next = job_savelist;
        job_savelist = jp;
        njob_savelist++;
    }
}

struct back_save {
    int count;
    struct jobsave *list;
    struct back_save *prev;
};

#define BYTE(n) (((n) + CHAR_BIT - 1) / CHAR_BIT)
#define MAXMSG 25
#define SH_STOPSIG (SH_EXITSIG << 1)

#ifdef VSUSP
#ifndef CNSUSP
#ifdef _POSIX_VDISABLE
#define CNSUSP _POSIX_VDISABLE
#else
#define CNSUSP 0
#endif  // _POSIX_VDISABLE
#endif  // CNSUSP
#ifndef CSWTCH
#ifdef CSUSP
#define CSWTCH CSUSP
#else
#define CSWTCH ('z' & 037)
#endif  // CSUSP
#endif  // CSWTCH
#endif  // VSUSP

// Process states.
#define P_EXITSAVE 01
#define P_STOPPED 02
#define P_NOTIFY 04
#define P_SIGNALLED 010
#define P_STTY 020
#define P_DONE 040
#define P_COREDUMP 0100
#define P_DISOWN 0200
#define P_FG 0400
#define P_BG 01000

static int job_chksave(pid_t, long);
static struct process *job_bypid(pid_t);
static struct process *job_byjid(int);
static char *job_sigmsg(Shell_t *, int);
static int job_alloc(Shell_t *);
static void job_free(int);
static struct process *job_unpost(Shell_t *, struct process *, int);
static void job_unlink(struct process *);
static void job_prmsg(Shell_t *, struct process *);
static struct process *freelist;
static char beenhere;
static struct process dummy;
static char by_number;
static Sfio_t *outfile;
static pid_t lastpid;
static struct back_save bck;

#ifdef JOBS
static void job_set(struct process *);
static void job_reset(struct process *);
static struct process *job_byname(char *);
static struct process *job_bystring(char *);
static struct termios my_stty;  // terminal state for shell
static char *job_string;
#else
extern const char e_coredump[];
#endif  // JOBS

#ifdef SIGTSTP
static void job_unstop(struct process *);
static void job_fgrp(struct process *, int);
#else
#define job_unstop(pw)
#undef CNSUSP
#endif  // SIGTSTP

#ifndef OTTYDISC
#undef NTTYDISC
#endif  // OTTYDISC

#ifdef JOBS

typedef int (*Waitevent_f)(int, long, int);

static void setsiginfo(Shell_t *shp, siginfo_t *info, struct process *pw) {
    info->si_signo = SIGCHLD;
    info->si_uid = shp->gd->userid;
    info->si_pid = pw->p_pid;
    info->si_code = CLD_EXITED;
    info->si_value.sival_ptr = 0;
    if (WIFSTOPPED(pw->p_wstat)) {
        info->si_code = CLD_STOPPED;
        info->si_value.sival_ptr = pointerof(WSTOPSIG(pw->p_wstat));
    } else if (WIFCONTINUED(pw->p_wstat)) {
        info->si_code = CLD_CONTINUED;
    } else if (WIFSIGNALED(pw->p_wstat)) {
        info->si_value.sival_ptr = pointerof(WTERMSIG(pw->p_wstat));
        info->si_code = WTERMCORE(pw->p_wstat) ? CLD_DUMPED : CLD_KILLED;
    } else {
        info->si_value.sival_ptr = pointerof(WEXITSTATUS(pw->p_wstat));
    }
    sh_setsiginfo(info);
}

void job_chldtrap(Shell_t *shp, const char *trap, int lock) {
    struct process *pw, *pwnext;
    pid_t bckpid;
    int oldexit, trapnote, sorc;

    trapnote = shp->trapnote;
    job_lock();
    for (pw = job.pwlist; pw; pw = pwnext) {
        pwnext = pw->p_nxtjob;
        sorc = WIFSTOPPED(pw->p_wstat) || WIFCONTINUED(pw->p_wstat);
        if ((pw->p_flag & (P_BG | P_DONE)) != (P_BG | P_DONE) && !sorc) continue;
        setsiginfo(shp, &Siginfo, pw);
        if (!sorc) pw->p_flag &= ~P_BG;
        bckpid = shp->bckpid;
        oldexit = shp->savexit;
        shp->bckpid = pw->p_pid;
        shp->savexit = pw->p_exit;
        if (pw->p_flag & P_SIGNALLED) shp->savexit |= SH_EXITSIG;
        sh_trap(shp, trap, 0);
        if (!sorc) {
            job.numbjob--;
            if (pw->p_pid == bckpid) job_unpost(shp, pw, 0);
        }
        shp->savexit = oldexit;
        shp->bckpid = bckpid;
    }
    shp->trapnote = trapnote;
    job_unlock();
}

//
// Return next on link list of jobsave free list.
//
static struct jobsave *jobsave_create(pid_t pid) {
    struct jobsave *jp = job_savelist;

    job_chksave(pid, -1);
    if (++bck.count > shgd->lim.child_max) job_chksave(0, -1);
    if (jp) {
        njob_savelist--;
        job_savelist = jp->next;
    } else {
        jp = newof(0, struct jobsave, 1, 0);
    }
    if (jp) {
        jp->pid = pid;
        jp->next = bck.list;
        bck.list = jp;
        jp->exitval = 0;
    }
    return jp;
}

#if SHOPT_COSHELL
pid_t sh_copid(struct cosh *csp) { return (COPID_BIT | (csp->id << 16) | csp->cojob->id); }

char *sh_pid2str(Shell_t *shp, pid_t pid) {
    struct cosh *csp = 0;
    if (pid & COPID_BIT) {
        int id = (pid >> 16) & 0x3f;
        for (csp = job.colist; csp; csp = csp->next) {
            if (csp->id == id) break;
        }
    }
    if (csp) {
        sfprintf(shp->strbuf, "%s.%d%c", csp->name, pid & 0xff, 0);
    } else {
        sfprintf(shp->strbuf, "%d%c", pid, 0);
    }
    return sfstruse(shp->strbuf);
}

int job_cowalk(int (*fun)(struct process *, int), int arg, char *name) {
    Shell_t *shp = sh_getinterp();
    struct cosh *csp;
    struct process *pw, *pwnext;
    pid_t val;
    int r = 0;
    size_t n;
    char *cp = strchr(name, '.');

    if (!cp) {
        n = strlen(name);
    } else {
        n = cp - name;
    }
    for (csp = (struct cosh *)job.colist; csp; csp = csp->next) {
        if (strncmp(name, csp->name, n) == 0 && csp->name[n] == 0) break;
    }
    if (!csp) errormsg(SH_DICT, ERROR_exit(1), e_jobusage, name);
    if (cp) {
        n = pid_fromstring(cp + 1);
        val = (csp->id << 16) | n | COPID_BIT;
    }
    job_reap(SIGCHLD);
    for (n = 0, pw = job.pwlist; pw; pw = pwnext) {
        pwnext = pw->p_nxtjob;
        if ((cp && val == pw->p_pid) || (pw->p_cojob && pw->p_cojob->local == (void *)csp)) {
            if (fun) {
                if (pw->p_flag & P_DONE) continue;
                r |= (*fun)(pw, arg);
            } else {
                job_wait(-pw->p_pid);
            }
            n++;
        }
    }
    if (!n) {
        shp->exitval = fun ? 1 : ERROR_NOENT;
    } else if (fun) {
        shp->exitval = r;
    }
    return r;
}

#endif  // SHOPT_COSHELL

//
// This is the SIGCLD interrupt routine.
//
static void job_waitsafe(int sig, siginfo_t *info, void *context) {
    int saved_errno = errno;
    if (job.in_critical || vmbusy()) {
        job.savesig = sig;
        job.waitsafe++;
    } else {
        job_reap(sig);
    }
    errno = saved_errno;
}

//
// Reap one job. // When called with sig==0, it does a blocking wait.
//
bool job_reap(int sig) {
    Shell_t *shp = sh_getinterp();
    pid_t pid;
    struct process *pw;
    struct process *px;
    int flags;
    struct jobsave *jp;
    bool nochild = false;
    int oerrno, wstat;
    Waitevent_f waitevent = shp->gd->waitevent;
    static int wcontinued = WCONTINUED;
#if SHOPT_COSHELL
    Cojob_t *cjp;
    int cojobs;
    long cotimeout = sig ? 0 : -1;

    for (pw = job.pwlist; pw; pw = pw->p_nxtjob) {
        if (pw->p_cojob && !(pw->p_flag & P_DONE)) break;
    }
    cojobs = (pw != 0);
    pid = 0;
#endif /* SHOPT_COSHELL */
    if (vmbusy()) {
        char *s;

        errormsg(SH_DICT, ERROR_warn(0), "vmbusy() inside job_reap() -- should not happen");
        s = getenv("_AST_KSH_VMBUSY");
        if (s) {
            switch (*s) {
                case 'a': {
                    abort();
                    break;
                }
                case 'p': {
                    pause();
                    break;
                }
            }
        }
    }
#ifdef DEBUG
    if (sfprintf(sfstderr, "ksh: job line %4d: reap pid=%d critical=%d signal=%d\n", __LINE__,
                 getpid(), job.in_critical, sig) <= 0)
        write(2, "waitsafe\n", 9);
    sfsync(sfstderr);
#endif /* DEBUG */
    job.savesig = 0;
    if (sig) {
        flags = WNOHANG | WUNTRACED | wcontinued;
    } else {
        flags = WUNTRACED | wcontinued;
    }
    shp->gd->waitevent = 0;
    oerrno = errno;
    while (1) {
        if (!(flags & WNOHANG) && !shp->intrap && job.pwlist) {
            sh_onstate(shp, SH_TTYWAIT);
            if (waitevent && (*waitevent)(-1, -1L, 0)) flags |= WNOHANG;
        }
#if SHOPT_COSHELL
        if (cojobs) {
            cjp = cowait(0, 0, cotimeout);
            if (cjp) {
                struct cosh *csp;
                csp = (struct cosh *)(cjp->coshell->data);
                csp->cojob = cjp;
                pid = sh_copid(csp);
                if (cjp->status < 256) {
                    wstat = cjp->status << 8;
                } else {
                    wstat = cjp->status - 256;
                }
                cotimeout = 0;
                goto cojob;
            } else if (copending(0) == 0) {
                cojobs = 0;
            }
            cotimeout = 0;
        }
#endif  // SHOPT_COSHELL
        pid = waitpid((pid_t)-1, &wstat, flags);
        sh_offstate(shp, SH_TTYWAIT);
#if SHOPT_COSHELL
    cojob:
#endif  // SHOPT_COSHELL

        // Some systems (linux 2.6) may return EINVAL when there are no continued children.
        if (pid < 0 && errno == EINVAL && (flags & WCONTINUED)) {
            pid = waitpid((pid_t)-1, &wstat, flags &= ~WCONTINUED);
        }
        sh_sigcheck(shp);
        if (pid < 0 && errno == EINTR && (sig || job.savesig)) {
            errno = 0;
            continue;
        }
        if (pid <= 0) break;
        if (pid == shp->spid) shp->spid = 0;
        if (wstat == 0) job_chksave(pid, -1);
        flags |= WNOHANG;
        job.waitsafe++;
        jp = 0;
        pw = job_bypid(pid);
        if (pw) pw->p_wstat = wstat;
        if (shp->st.trapcom[SIGCHLD] && (WIFSTOPPED(wstat) || WIFCONTINUED(wstat))) {
            shp->sigflag[SIGCHLD] |= SH_SIGTRAP;
        }
        lastpid = pid;
        if (!pw) {
#ifdef DEBUG
            sfprintf(sfstderr,
                     "ksh: job line %4d: reap pid=%d critical=%d unknown job pid=%d pw=%x\n",
                     __LINE__, getpid(), job.in_critical, pid, pw);
#endif /* DEBUG */
            if (WIFCONTINUED(wstat) && wcontinued) continue;
            pw = &dummy;
            pw->p_exit = 0;
            pw->p_pgrp = 0;
            pw->p_exitmin = 0;
            pw->p_wstat = wstat;
            if (job.toclear) job_clear(shp);
            jp = jobsave_create(pid);
            jp->env = shp->curenv;
            pw->p_flag = 0;
            lastpid = pw->p_pid = pid;
            px = 0;
            if (jp && WIFSTOPPED(wstat)) {
                jp->exitval = SH_STOPSIG;
                continue;
            }
        }
#ifdef SIGTSTP
        else {
            px = job_byjid(pw->p_job);
        }
        if (WIFCONTINUED(wstat) && wcontinued)
            pw->p_flag &= ~(P_NOTIFY | P_SIGNALLED | P_STOPPED);
        else if (WIFSTOPPED(wstat)) {
            pw->p_flag |= (P_NOTIFY | P_SIGNALLED | P_STOPPED);
            pw->p_exit = WSTOPSIG(wstat);
            if (pw->p_pgrp && pw->p_pgrp == job.curpgid && sh_isstate(shp, SH_STOPOK))
                kill(getpid(), pw->p_exit);
            if (px) {
                // Move to top of job list.
                job_unlink(px);
                px->p_nxtjob = job.pwlist;
                job.pwlist = px;
            }
            continue;
        } else
#endif  // SIGTSTP
        {
            // Check for coprocess completion.
            if (pid == shp->cpid) {
                sh_close(shp->coutpipe);
                sh_close(shp->cpipe[1]);
                shp->cpipe[1] = -1;
                shp->coutpipe = -1;
            } else if (shp->subshell) {
                sh_subjobcheck(pid);
            }

            pw->p_flag &= ~(P_STOPPED | P_SIGNALLED);
            if (WIFSIGNALED(wstat)) {
                pw->p_flag |= (P_DONE | P_NOTIFY | P_SIGNALLED);
                if (WTERMCORE(wstat)) pw->p_flag |= P_COREDUMP;
                pw->p_exit = WTERMSIG(wstat);
                // If process in current jobs terminates from an interrupt,
                // propogate to parent shell.
                if (pw->p_pgrp && pw->p_pgrp == job.curpgid && pw->p_exit == SIGINT &&
                    sh_isstate(shp, SH_STOPOK)) {
                    pw->p_flag &= ~P_NOTIFY;
                    sh_offstate(shp, SH_STOPOK);
                    kill(getpid(), SIGINT);
                    sh_onstate(shp, SH_STOPOK);
                }
            } else {
                pw->p_flag |= (P_DONE | P_NOTIFY);
                pw->p_exit = pw->p_exitmin;
                if (WEXITSTATUS(wstat) > pw->p_exitmin) pw->p_exit = WEXITSTATUS(wstat);
            }
            if ((pw->p_flag & P_DONE) && (pw->p_flag & P_BG)) {
                if (shp->st.trapcom[SIGCHLD]) {
                    shp->sigflag[SIGCHLD] = SH_SIGTRAP;
                } else {
                    pw->p_flag &= ~P_BG;
                }
            }
            if (pw->p_pgrp == 0) pw->p_flag &= ~P_NOTIFY;
        }
        if (jp && pw == &dummy) {
            jp->exitval = pw->p_exit;
            if (pw->p_flag & P_SIGNALLED) jp->exitval |= SH_EXITSIG;
        }
#ifdef DEBUG
        sfprintf(sfstderr,
                 "ksh: job line %4d: reap pid=%d critical=%d job %d with pid %d flags=%o complete "
                 "with status=%x exit=%d\n",
                 __LINE__, getpid(), job.in_critical, pw->p_job, pid, pw->p_flag, wstat,
                 pw->p_exit);
        sfsync(sfstderr);
#endif  // DEBUG
        // Only top-level process in job should have notify set.
        if (px && pw != px) pw->p_flag &= ~P_NOTIFY;
        if (pid == pw->p_fgrp && pid == tcgetpgrp(JOBTTY)) {
            px = job_byjid((int)pw->p_job);
            for (; px && (px->p_flag & P_DONE); px = px->p_nxtproc) {
                ;  // empty loop
            }
            if (!px && sh_isoption(shp, SH_INTERACTIVE)) tcsetpgrp(JOBTTY, job.mypid);
        }
        if (!shp->st.trapcom[SIGCHLD] && (pw->p_flag & P_DONE) && (pw->p_flag & P_BG) &&
            !(pw->p_flag & P_NOTIFY)) {
            pw->p_flag &= ~P_BG;
            if (pw != pwfg) job_unpost(shp, pw, 1);
        }
    }
    if (errno == ECHILD) {
        errno = oerrno;
        job.numbjob = 0;
        nochild = true;
    }
    shp->gd->waitevent = waitevent;
    if (sh_isoption(shp, SH_NOTIFY) && sh_isstate(shp, SH_TTYWAIT)) {
        outfile = sfstderr;
        job_list(pw, JOB_NFLAG | JOB_NLFLAG);
        job_unpost(shp, pw, 1);
        sfsync(sfstderr);
    }
    shp->trapnote |= (shp->sigflag[SIGCHLD] & SH_SIGTRAP);
    if (!sig && (shp->trapnote & SH_SIGTRAP)) {
        int c = job.in_critical;
        job.in_critical = 0;
        job_chldtrap(shp, shp->st.trapcom[SIGCHLD], 1);
        job.in_critical = c;
    }
    return nochild;
}

//
// Initialize job control. If lflag is set the switching driver
// message will not print.
//
void job_init(Shell_t *shp, int lflag) {
    int ntry = 0;
    job.fd = JOBTTY;
    signal(SIGCHLD, job_waitsafe);
#if defined(SIGCLD) && (SIGCLD != SIGCHLD)
    signal(SIGCLD, job_waitsafe);
#endif
    if (njob_savelist < NJOB_SAVELIST) init_savelist();
    if (!sh_isoption(shp, SH_INTERACTIVE)) return;
        // Use new line discipline when available.
#ifdef NTTYDISC
#ifdef FIOLOOKLD
    if ((job.linedisc = ioctl(JOBTTY, FIOLOOKLD, 0)) < 0) return;
#else
    if (ioctl(JOBTTY, TIOCGETD, &job.linedisc) != 0) return;
#endif /* FIOLOOKLD */
    if (job.linedisc != NTTYDISC && job.linedisc != OTTYDISC) {
        // No job control when running with MPX.
        sh_onoption(shp, SH_VIRAW);
        return;
    }
    if (job.linedisc == NTTYDISC) job.linedisc = -1;
#endif /* NTTYDISC */

    job.mypgid = getpgrp();
#ifdef SIGTSTP
    // Wait until we are in the foreground.
    while ((job.mytgid = tcgetpgrp(JOBTTY)) != job.mypgid) {
        if (job.mytgid <= 0) return;
        // Stop this shell until continued.
        signal(SIGTTIN, SIG_DFL);
        kill(shp->gd->pid, SIGTTIN);
        // Resumes here after continue tries again.
        if (ntry++ > IOMAXTRY) {
            errormsg(SH_DICT, 0, e_no_start);
            return;
        }
    }
#endif  // SIGTTIN

#ifdef NTTYDISC
    // Set the line discipline.
    if (job.linedisc >= 0) {
        int linedisc = NTTYDISC;
#ifdef FIOPUSHLD
        tty_get(JOBTTY, &my_stty);
        if (ioctl(JOBTTY, FIOPOPLD, 0) < 0) return;
        if (ioctl(JOBTTY, FIOPUSHLD, &linedisc) < 0) {
            ioctl(JOBTTY, FIOPUSHLD, &job.linedisc);
            return;
        }
        tty_set(JOBTTY, TCSANOW, &my_stty);
#else
        if (ioctl(JOBTTY, TIOCSETD, &linedisc) != 0) return;
#endif  // FIOPUSHLD
        if (lflag == 0) {
            errormsg(SH_DICT, 0, e_newtty);
        } else {
            job.linedisc = -1;
        }
    }
#endif  // NTTYDISC

#ifdef SIGTSTP
    // Make sure that we are a process group leader.
    setpgid(0, shp->gd->pid);
#if defined(SA_NOCLDSTOP) || defined(SA_NOCLDWAIT)
#if !defined(SA_NOCLDSTOP)
#define SA_NOCLDSTOP 0
#endif
#if !defined(SA_NOCLDWAIT)
#define SA_NOCLDWAIT 0
#endif
    sigflag(SIGCHLD, SA_NOCLDSTOP | SA_NOCLDWAIT, 0);
#endif  // SA_NOCLDSTOP || SA_NOCLDWAIT
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    shp->sigflag[SIGTTIN] = SH_SIGOFF;
    shp->sigflag[SIGTTOU] = SH_SIGOFF;
    // The shell now handles ^Z.
    signal(SIGTSTP, sh_fault);
    tcsetpgrp(JOBTTY, shp->gd->pid);
#ifdef CNSUSP
    // Set the switch character.
    tty_get(JOBTTY, &my_stty);
    job.suspend = (unsigned)my_stty.c_cc[VSUSP];
    if (job.suspend == (unsigned char)CNSUSP) {
        my_stty.c_cc[VSUSP] = CSWTCH;
        tty_set(JOBTTY, TCSAFLUSH, &my_stty);
    }
#endif  // CNSUSP
    sh_onoption(shp, SH_MONITOR);
    job.jobcontrol++;
    job.mypid = shp->gd->pid;
#endif  // SIGTSTP
    return;
}

//
// See if there are any stopped jobs. Restore tty driver and pgrp.
//
int job_close(Shell_t *shp) {
    struct process *pw;
    int count = 0, running = 0;
    if (!job.jobcontrol) {
        return 0;
    } else if (getpid() != job.mypid) {
        return 0;
    }
    job_lock();
    if (!tty_check(0)) beenhere++;
    for (pw = job.pwlist; pw; pw = pw->p_nxtjob) {
        if (!(pw->p_flag & P_STOPPED)) {
            if (!(pw->p_flag & P_DONE)) running++;
            continue;
        }
        if (beenhere) killpg(pw->p_pgrp, SIGTERM);
        count++;
    }
    if (beenhere++ == 0 && job.pwlist) {
        if (count) {
            errormsg(SH_DICT, 0, e_terminate);
            return -1;
        } else if (running && shp->login_sh) {
            errormsg(SH_DICT, 0, e_jobsrunning);
            return -1;
        }
    }
    job_unlock();
#ifdef SIGTSTP
    if (setpgid(0, job.mypgid) >= 0) tcsetpgrp(job.fd, job.mypgid);
#endif  // SIGTSTP
#ifdef NTTYDISC
    if (job.linedisc >= 0) {
        // Restore old line discipline.
#ifdef FIOPUSHLD
        tty_get(job.fd, &my_stty);
        if (ioctl(job.fd, FIOPOPLD, 0) < 0) return (0);
        if (ioctl(job.fd, FIOPUSHLD, &job.linedisc) < 0) {
            job.linedisc = NTTYDISC;
            ioctl(job.fd, FIOPUSHLD, &job.linedisc);
            return 0;
        }
        tty_set(job.fd, TCSAFLUSH, &my_stty);
#else
        if (ioctl(job.fd, TIOCSETD, &job.linedisc) != 0) return (0);
#endif  // FIOPUSHL
        errormsg(SH_DICT, 0, e_oldtty);
    }
#endif  // NTTYDISC
#ifdef CNSUSP
    if (job.suspend == CNSUSP) {
        tty_get(job.fd, &my_stty);
        my_stty.c_cc[VSUSP] = CNSUSP;
        tty_set(job.fd, TCSAFLUSH, &my_stty);
    }
#endif /* CNSUSP */
    job.jobcontrol = 0;
    return 0;
}

static void job_set(struct process *pw) {
    Shell_t *shp = pw->p_shp;
    // Save current terminal state.
    tty_get(job.fd, &my_stty);
    if (pw->p_flag & P_STTY) {
        // Restore terminal state for job.
        tty_set(job.fd, TCSAFLUSH, &pw->p_stty);
    }
#ifdef SIGTSTP
    if ((pw->p_flag & P_STOPPED) || tcgetpgrp(job.fd) == shp->gd->pid) {
        tcsetpgrp(job.fd, pw->p_fgrp);
    }
    // If job is stopped, resume it in the background.
    if (!shp->forked) job_unstop(pw);
    shp->forked = 0;
#endif  // SIGTSTP
}

static void job_reset(struct process *pw) {
    Shell_t *shp = pw->p_shp;

    // Save the terminal state for current job.
#ifdef SIGTSTP
    job_fgrp(pw, tcgetpgrp(job.fd));
    if (sh_isoption(shp, SH_INTERACTIVE) && tcsetpgrp(job.fd, job.mypid) != 0) return;
#endif  // SIGTSTP
    // Force the following tty_get() to do a tcgetattr() unless fg.
    if (!(pw->p_flag & P_FG)) tty_set(-1, 0, NULL);
    if (pw && (pw->p_flag & P_SIGNALLED) && pw->p_exit != SIGHUP) {
        if (tty_get(job.fd, &pw->p_stty) == 0) pw->p_flag |= P_STTY;
        // Restore terminal state for job.
        tty_set(job.fd, TCSAFLUSH, &my_stty);
    }
    beenhere = 0;
}
#endif  // JOBS

//
// `wait` built-in command.
//
void job_bwait(char **jobs) {
    char *jp;
    struct process *pw;
    pid_t pid;

    if (*jobs == 0) {
        job_wait((pid_t)-1);
    } else {
        while (*jobs) {
            jp = *jobs++;
#ifdef JOBS
            if (*jp == '%') {
                job_lock();
                pw = job_bystring(jp);
                job_unlock();
                if (pw) {
                    pid = pw->p_pid;
                } else {
                    return;
                }
            }
#if SHOPT_COSHELL
            else if (isalpha(*jp)) {
                job_cowalk(NULL, 0, jp);
                return;
            }
#endif  // SHOPT_COSHELL
            else
#endif  // JOBS
                pid = pid_fromstring(jp);
            job_wait(-pid);
        }
    }
}

#ifdef JOBS
//
// Execute function <fun> for each job.
//
int job_walk(Shell_t *shp, Sfio_t *file, int (*fun)(struct process *, int), int arg,
             char *joblist[]) {
    struct process *pw;
    int r = 0;
    char *jobid, **jobs = joblist;
    struct process *px;
    job_string = 0;
    outfile = file;
    by_number = 0;
    job_lock();
    pw = job.pwlist;
#if SHOPT_COSHELL
    job_waitsafe(SIGCHLD, (siginfo_t *)0, (void *)0);
#endif  // SHOPT_COSHELL
    if (jobs == NULL) {
        // Do all jobs.
        for (; pw; pw = px) {
            px = pw->p_nxtjob;
            if (pw->p_env != shp->jobenv) continue;
            if ((*fun)(pw, arg)) r = 2;
        }
    } else if (*jobs == NULL) {  // current job
        // Skip over non-stop jobs.
        while (pw && (pw->p_env != shp->jobenv || pw->p_pgrp == 0)) pw = pw->p_nxtjob;
        if ((*fun)(pw, arg)) r = 2;
    } else {
        while (*jobs) {
            job_string = jobid = *jobs++;
            if (*jobid == 0) errormsg(SH_DICT, ERROR_exit(1), e_jobusage, job_string);
#if SHOPT_COSHELL
            if (isalpha(*jobid)) {
                r = job_cowalk(fun, arg, jobid);
                by_number = 0;
                job_unlock();
                return r;
            }
#endif  // SHOPT_COSHELL
            if (*jobid == '%') {
                pw = job_bystring(jobid);
            } else {
                int pid = pid_fromstring(jobid);
                if (!(pw = job_bypid(pid))) {
                    pw = &dummy;
                    pw->p_shp = shp;
                    pw->p_pid = pid;
                    pw->p_pgrp = pid;
                }
                by_number = 1;
            }
            if (!pw || (*fun)(pw, arg)) r = 2;
            by_number = 0;
        }
    }
    job_unlock();
    return r;
}

//
// Send signal <sig> to background process group if not disowned.
//
int job_terminate(struct process *pw, int sig) {
    if (pw->p_pgrp && !(pw->p_flag & P_DISOWN)) job_kill(pw, sig);
    return 0;
}

//
// List the given job.
// Flag JOB_LFLAG for long listing.
// Flag JOB_NFLAG for list only jobs marked for notification.
// Flag JOB_PFLAG for process id(s) only.
//
int job_list(struct process *pw, int flag) {
    Shell_t *shp = sh_getinterp();
    struct process *px = pw;
    int n;
    const char *msg;
    int msize;
    char *dir = 0, *home = nv_getval(HOME);
    size_t len = home ? strlen(home) : 0;

    if (!pw || pw->p_job <= 0) return (1);
    if (pw->p_env != shp->jobenv) return (0);
    if ((flag & JOB_NFLAG) && (!(px->p_flag & P_NOTIFY) || px->p_pgrp == 0)) return (0);
    if ((flag & JOB_PFLAG)) {
#if SHOPT_COSHELL
        sfprintf(outfile, "%s\n", sh_pid2str(shp, px->p_pgrp ? px->p_pgrp : px->p_pid));
#else
        sfprintf(outfile, "%d\n", px->p_pgrp ? px->p_pgrp : px->p_pid);
#endif  // SHOPT_COSHELL
        return 0;
    }
    if ((px->p_flag & P_DONE) && job.waitall && !(flag & JOB_LFLAG)) return (0);
    job_lock();
    n = px->p_job;
    if (px == job.pwlist) {
        msize = '+';
    } else if (px == job.pwlist->p_nxtjob) {
        msize = '-';
    } else {
        msize = ' ';
    }
    if (flag & JOB_NLFLAG) sfputc(outfile, '\n');
    sfprintf(outfile, "[%d] %c ", n, msize);
    do {
        if (px && px->p_curdir) dir = px->p_curdir;
        n = 0;
        if (flag & JOB_LFLAG) {
#if SHOPT_COSHELL
            sfprintf(outfile, "%s\t", sh_pid2str(shp, px->p_pid));
#else
            sfprintf(outfile, "%d\t", px->p_pid);
#endif  // SHOPT_COSHELL
        }
        if (px->p_flag & P_SIGNALLED) {
            msg = job_sigmsg(shp, (int)(px->p_exit));
        } else if (px->p_flag & P_NOTIFY) {
            msg = sh_translate(e_done);
            n = px->p_exit;
        } else {
            msg = sh_translate(e_running);
        }
        px->p_flag &= ~P_NOTIFY;
        sfputr(outfile, msg, -1);
        msize = (int)strlen(msg);
        if (n) {
            sfprintf(outfile, "(%d)", (int)n);
            msize += (3 + (n > 10) + (n > 100));
        }
        if (px->p_flag & P_COREDUMP) {
            msg = sh_translate(e_coredump);
            sfputr(outfile, msg, -1);
            msize += (int)strlen(msg);
        }
        sfnputc(outfile, ' ', MAXMSG > msize ? MAXMSG - msize : 1);
        if (flag & JOB_LFLAG)
            px = px->p_nxtproc;
        else {
            while ((px = px->p_nxtproc)) px->p_flag &= ~P_NOTIFY;
            px = NULL;
        }
        if (!px) {
            char *pwd;
            if (dir && (pwd = nv_getval(PWDNOD)) && !strcmp(dir, pwd)) dir = 0;
            hist_list(shgd->hist_ptr, outfile, pw->p_name, dir ? '\n' : 0, ";");
            if (dir) {
                char *tilde = "";
                if ((!strncmp(dir, home, len) && dir[len] == '/') || dir[len] == 0) {
                    tilde = "~";
                    dir += len;
                }
                sfprintf(outfile, "  (wd: %s%s)\n", tilde, dir);
                dir = 0;
            }
        } else {
            sfputr(outfile, e_nlspace, -1);
        }
    } while (px);
    job_unlock();
    return 0;
}

//
// Get the process group given the job number.
// This routine returns the process group number or -1.
//
static struct process *job_bystring(char *ajob) {
    struct process *pw = job.pwlist;
    int c;
    const char *msg;
    if (*ajob++ != '%') return (NULL);
    c = *ajob;
    if (isdigit(c)) {
        pw = job_byjid((int)strtol(ajob, (char **)0, 10));
    } else if (c == '+' || c == '%') {
        ;  // do nothing
    } else if (c == '-') {
        if (pw) pw = job.pwlist->p_nxtjob;
    } else {
        pw = job_byname(ajob);
    }
    if (pw && pw->p_flag) return (pw);
    msg = sh_translate(e_no_job);
    sfprintf(sfstderr, "%s: %s\n", ajob - 1, msg);
    return NULL;
}

//
// Kill a job or process.
//

int job_kill(struct process *pw, int sig) {
    Shell_t *shp;
    pid_t pid;
    int r;
    const char *msg;
    int qflag = sig & JOB_QFLAG;
#ifdef SIGTSTP
    bool stopsig;
#endif
    union sigval sig_val;
    if (pw == 0) goto error;
    shp = pw->p_shp;
    if (sig & JOB_QQFLAG)
        sig_val.sival_ptr = pointerof(shp->sigval);
    else
        sig_val.sival_int = (int)shp->sigval;
    sig &= ~(JOB_QFLAG | JOB_QQFLAG);
#ifdef SIGTSTP
    stopsig = (sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU);
#else
#define stopsig 1
#endif  // SIGTSTP
    job_lock();
    errno = ECHILD;
    pid = pw->p_pid;
#if SHOPT_COSHELL
    if (pw->p_cojob) {
        r = cokill(pw->p_cojob->coshell, pw->p_cojob, sig);
    } else
#endif  // SHOPT_COSHELL
        if (by_number) {
        if (pid == 0 && job.jobcontrol) r = job_walk(shp, outfile, job_kill, sig, (char **)0);
#ifdef SIGTSTP
        if (sig == SIGSTOP && pid == shp->gd->pid && shp->gd->ppid == 1) {
            // Can't stop login shell.
            errno = EPERM;
            r = -1;
        } else {
            if (pid >= 0) {
                if (qflag) {
                    if (pid == 0) goto no_sigqueue;
                    r = sigqueue(pid, sig, sig_val);
                    if (r < 0 && errno == EAGAIN) {
                        asoyield();
                        r = -2;
                    }
                } else {
                    r = kill(pid, sig);
                }
                if (r >= 0 && !stopsig) {
                    if (pw->p_flag & P_STOPPED) pw->p_flag &= ~(P_STOPPED | P_SIGNALLED);
                    if (sig && !qflag) kill(pid, SIGCONT);
                }
            } else {
                if (qflag) goto no_sigqueue;
                if ((r = killpg(-pid, sig)) >= 0 && !stopsig) {
                    job_unstop(job_bypid(pw->p_pid));
                    if (sig) killpg(-pid, SIGCONT);
                }
            }
        }
#else
        if (pid >= 0) {
            if (qflag) {
                r = sigqueue(pid, sig, sig_val);
                if (r < 0 &&errno = EAGAIN) {
                    asoyield();
                    r = -2;
                }
            } else {
                r = kill(pid, sig);
            }
        } else {
            if (qflag) goto no_sigqueue;
            r = killpg(-pid, sig);
        }
#endif  // SIGTSTP
    } else {
        if (qflag) goto no_sigqueue;
        pid = pw->p_pgrp;
        if (pid) {
            r = killpg(pid, sig);
#ifdef SIGTSTP
            if (r >= 0 && (sig == SIGHUP || sig == SIGTERM || sig == SIGCONT)) job_unstop(pw);
#endif  // SIGTSTP
#if 0
			if(r>=0)
				sh_delay(.05);
#endif
        }
        while (pw && pw->p_pgrp == 0 && (r = kill(pw->p_pid, sig)) >= 0) {
#ifdef SIGTSTP
            if (sig == SIGHUP || sig == SIGTERM) kill(pw->p_pid, SIGCONT);
#endif  // SIGTSTP
            pw = pw->p_nxtproc;
        }
    }
    if (r < 0 && job_string) {
    error:
        if (pw && by_number) {
            msg = sh_translate(e_no_proc);
        } else {
            msg = sh_translate(e_no_job);
        }
        if (errno == EPERM) msg = sh_translate(e_access);
        sfprintf(sfstderr, "kill: %s: %s\n", job_string, msg);
        r = 1;
    }
    sh_delay(.001);
    job_unlock();
    if (r == -2) r = 2;
    return r;

no_sigqueue:
    msg = sh_translate("queued signals require positive pids");
    sfprintf(sfstderr, "kill: %s: %s\n", job_string, msg);
    return 1;
}

//
// Get process structure from first letters of jobname.
//
static struct process *job_byname(char *name) {
    struct process *pw = job.pwlist;
    struct process *pz = 0;
    int *flag = 0;
    char *cp = name;
    int offset;

    if (!shgd->hist_ptr) return (NULL);
    if (*cp == '?') cp++, flag = &offset;
    for (; pw; pw = pw->p_nxtjob) {
        if (hist_match(shgd->hist_ptr, pw->p_name, cp, flag) >= 0) {
            if (pz) errormsg(SH_DICT, ERROR_exit(1), e_jobusage, name - 1);
            pz = pw;
        }
    }
    return pz;
}

#else
#define job_set(x)
#define job_reset(x)
#endif  // JOBS

//
// Initialize the process posting array.
//
void job_clear(Shell_t *shp) {
    struct process *pw, *px;
    struct process *pwnext;
    int j = BYTE(shp->gd->lim.child_max);
    struct jobsave *jp, *jpnext;

    job_lock();
    for (pw = job.pwlist; pw; pw = pwnext) {
        pwnext = pw->p_nxtjob;
        while (pw) {
            px = pw;
            pw = pw->p_nxtproc;
            free(px);
        }
    }
    for (jp = bck.list; jp; jp = jpnext) {
        jpnext = jp->next;
        free(jp);
    }
    bck.list = 0;
    if (njob_savelist < NJOB_SAVELIST) init_savelist();
    job.pwlist = NULL;
    job.numpost = 0;
    job.numbjob = 0;
    job.waitall = 0;
    job.curpgid = 0;
    job.toclear = 0;
    if (!job.freejobs) job.freejobs = (unsigned char *)malloc((unsigned)(j + 1));
    while (j >= 0) job.freejobs[j--] = 0;
    job_unlock();
}

//
// Put the process <pid> on the process list and return the job number.
// If non-zero, <join> is the process id of the job to join.
//
int job_post(Shell_t *shp, pid_t pid, pid_t join) {
    struct process *pw;
    History_t *hp = shp->gd->hist_ptr;
    int val, bg = 0;

    shp->jobenv = shp->curenv;
    if (job.toclear) {
        job_clear(shp);
        return 0;
    }
    job_lock();
    job.lastpost = pid;
    if (join == 1) {
        join = 0;
        bg = P_BG;
        job.numbjob++;
    }
    if (njob_savelist < NJOB_SAVELIST) init_savelist();
    pw = job_bypid(pid);
    if (pw) job_unpost(shp, pw, 0);
    if (join) {
        pw = job_bypid(join);
        if (pw) {
            val = pw->p_job;
        } else {
            val = job.curjobid;
        }
        // If job to join is not first move it to front.
        if (val && (pw = job_byjid(val)) != job.pwlist) {
            job_unlink(pw);
            pw->p_nxtjob = job.pwlist;
            job.pwlist = pw;
        }
    }
    pw = freelist;
    if (pw) {
        freelist = pw->p_nxtjob;
    } else {
        pw = calloc(1, sizeof(struct process));
    }
    pw->p_flag = 0;
    pw->p_curdir = 0;
    job.numpost++;
    pw->p_exitval = job.exitval;
#if SHOPT_COSHELL
    pw->p_cojob = 0;
    if (shp->coshell && (pid & COPID_BIT)) {
        pw->p_cojob = ((struct cosh *)shp->coshell)->cojob;
        job.curpgid = sh_isstate(shp, SH_MONITOR) ? pid : 0;
    }
#endif /* SHOPT_COSHELL */
    pw->p_shp = shp;
    pw->p_env = shp->curenv;
    pw->p_pid = pid;
    if (!shp->outpipe || shp->cpid == pid) pw->p_flag = P_EXITSAVE;
    pw->p_exitmin = shp->xargexit;
    pw->p_exit = 0;
    if (sh_isstate(shp, SH_MONITOR)) {
        if (killpg(job.curpgid, 0) < 0 && errno == ESRCH) job.curpgid = pid;
        pw->p_fgrp = job.curpgid;
    } else {
        pw->p_fgrp = 0;
    }
    pw->p_pgrp = pw->p_fgrp;
#ifdef DEBUG
    sfprintf(
        sfstderr,
        "ksh: job line %4d: post pid=%d critical=%d job=%d pid=%d pgid=%d savesig=%d join=%d\n",
        __LINE__, getpid(), job.in_critical, pw->p_job, pw->p_pid, pw->p_pgrp, job.savesig, join);
    sfsync(sfstderr);
#endif  // DEBUG
#ifdef JOBS
    if (hp && !sh_isstate(shp, SH_PROFILE)) {
        pw->p_name = hist_tell(shgd->hist_ptr, (int)hp->histind - 1);
    } else {
        pw->p_name = -1;
    }
#endif  // JOBS
    if ((val = job_chksave(pid, pw->p_env)) >= 0 && !jobfork) {
        pw->p_exit = val;
        if (pw->p_exit == SH_STOPSIG) {
            pw->p_flag |= (P_SIGNALLED | P_STOPPED);
            pw->p_exit = 0;
        } else if (pw->p_exit >= SH_EXITSIG) {
            pw->p_flag |= P_DONE | P_SIGNALLED;
            pw->p_exit &= SH_EXITMASK;
        } else {
            pw->p_flag |= (P_DONE | P_NOTIFY);
        }
    }
    if (bg) {
        if (pw->p_flag & P_DONE) {
            job.numbjob--;
        } else {
            pw->p_flag |= P_BG;
        }
    }
    lastpid = 0;
    if (join && job.pwlist) {
        // Join existing current job.
        pw->p_nxtjob = job.pwlist->p_nxtjob;
        pw->p_job = job.pwlist->p_job;
        do {
            pw->p_nxtproc = job.pwlist;
        } while (asocasptr(&job.pwlist, pw->p_nxtproc, pw) != pw->p_nxtproc);
    } else {
        // Create a new job.
        while ((pw->p_job = job_alloc(shp)) < 0) job_wait((pid_t)1);
        pw->p_nxtproc = 0;
        do {
            pw->p_nxtjob = job.pwlist;
        } while (asocasptr(&job.pwlist, pw->p_nxtjob, pw) != pw->p_nxtjob);
        pw->p_curdir = path_pwd(shp, 0);
        if (pw->p_curdir) pw->p_curdir = strdup(pw->p_curdir);
    }
    job_unlock();
    return pw->p_job;
}

//
// Returns a process structure give a process id.
//
static struct process *job_bypid(pid_t pid) {
    struct process *pw, *px;
    for (pw = job.pwlist; pw; pw = pw->p_nxtjob)
        for (px = pw; px; px = px->p_nxtproc) {
            if (px->p_pid == pid) return (px);
        }
    return NULL;
}

//
// Return a pointer to a job given the job id.
//
static struct process *job_byjid(int jobid) {
    struct process *pw;
    for (pw = job.pwlist; pw; pw = pw->p_nxtjob) {
        if (pw->p_job == jobid) break;
    }
    return pw;
}

//
// Print a signal message.
//
static void job_prmsg(Shell_t *shp, struct process *pw) {
    if (pw->p_exit != SIGINT && pw->p_exit != SIGPIPE) {
        const char *msg, *dump;
        msg = job_sigmsg(shp, (int)(pw->p_exit));
        msg = sh_translate(msg);
        if (pw->p_flag & P_COREDUMP) {
            dump = sh_translate(e_coredump);
        } else {
            dump = "";
        }
        if (sh_isstate(shp, SH_INTERACTIVE)) {
            sfprintf(sfstderr, "%s%s\n", msg, dump);
        } else {
            errormsg(SH_DICT, 2, "%d: %s%s", pw->p_pid, msg, dump);
        }
    }
}

//
// Wait for process pid to complete. If pid < -1, then wait can be interrupted,
// -pid is waited for (wait builtin).
//
// pid=0 to unpost all done processes
// pid=1 to wait for at least one process to complete
// pid=-1 to wait for all runing processes
//
bool job_wait(pid_t pid) {
    Shell_t *shp = sh_getinterp();
    struct process *pw = 0, *px;
    int jobid = 0;
    bool nochild = true;
    char intr = 0;

    if (pid < 0) {
        pid = -pid;
        intr = 1;
    }
    job_lock();
    if (pid == 0) {
        if (!job.waitall || !job.curjobid || !(pw = job_byjid(job.curjobid))) {
            job_unlock();
            goto done;
        }
        jobid = pw->p_job;
        job.curjobid = 0;
        if (!(pw->p_flag & (P_DONE | P_STOPPED))) job_reap(job.savesig);
    }
    if (pid > 1) {
        if (pid == shp->spid) shp->spid = 0;
        if (!(pw = job_bypid(pid))) {
            // Check to see whether job status has been saved.
            if ((shp->exitval = job_chksave(pid, shp->curenv)) < 0) shp->exitval = ERROR_NOENT;
            exitset(shp);
            job_unlock();
            return nochild;
        } else if (intr && pw->p_env != shp->curenv) {
            shp->exitval = ERROR_NOENT;
            job_unlock();
            return nochild;
        }
        jobid = pw->p_job;
        if (!intr) pw->p_flag &= ~P_EXITSAVE;
        if (pw->p_pgrp && job.parent != (pid_t)-1) job_set(job_byjid(jobid));
    }
    pwfg = pw;
#ifdef DEBUG
    sfprintf(sfstderr, "ksh: job line %4d: wait pid=%d critical=%d job=%d pid=%d waitall=%d\n",
             __LINE__, getpid(), job.in_critical, jobid, pid, job.waitall);
    if (pw) {
        sfprintf(sfstderr, "ksh: job line %4d: wait pid=%d critical=%d flags=%o\n", __LINE__,
                 getpid(), job.in_critical, pw->p_flag);
    }
#endif  // DEBUG
    errno = 0;
    if (shp->coutpipe >= 0 && lastpid && shp->cpid == lastpid) {
        sh_close(shp->coutpipe);
        sh_close(shp->cpipe[1]);
        shp->cpipe[1] = shp->coutpipe = -1;
    }
    while (1) {
        if (job.waitsafe) {
            for (px = job.pwlist; px; px = px->p_nxtjob) {
                if (px != pw && (px->p_flag & P_NOTIFY)) {
                    if (sh_isoption(shp, SH_NOTIFY)) {
                        outfile = sfstderr;
                        job_list(px, JOB_NFLAG | JOB_NLFLAG);
                        sfsync(sfstderr);
                    } else if (!sh_isoption(shp, SH_INTERACTIVE) && (px->p_flag & P_SIGNALLED)) {
                        job_prmsg(shp, px);
                        px->p_flag &= ~P_NOTIFY;
                    }
                }
            }
        }
        if (pw && (pw->p_flag & (P_DONE | P_STOPPED))) {
#ifdef SIGTSTP
            if (pw->p_flag & P_STOPPED) {
                pw->p_flag |= P_EXITSAVE;
                if (sh_isoption(shp, SH_INTERACTIVE) && !sh_isstate(shp, SH_FORKED)) {
                    if (pw->p_exit != SIGTTIN && pw->p_exit != SIGTTOU) break;

                    tcsetpgrp(JOBTTY, pw->p_pgrp);
                    killpg(pw->p_pgrp, SIGCONT);
                } else {  // ignore stop when non-interactive
                    pw->p_flag &= ~(P_NOTIFY | P_SIGNALLED | P_STOPPED | P_EXITSAVE);
                }
            } else
#endif /* SIGTSTP */
            {
                if (pw->p_flag & P_SIGNALLED) {
                    pw->p_flag &= ~P_NOTIFY;
                    job_prmsg(shp, pw);
                } else if (pw->p_flag & P_DONE) {
                    pw->p_flag &= ~P_NOTIFY;
                }
                if (pw->p_job == jobid) {
                    px = job_byjid(jobid);
                    /* last process in job */
                    if (px != pw) px = 0;
                    if (px) {
                        shp->exitval = px->p_exit;
                        if (px->p_flag & P_SIGNALLED) shp->exitval |= SH_EXITSIG;
                        if (intr) px->p_flag &= ~P_EXITSAVE;
                    }
                }
                px = job_unpost(shp, pw, 1);
                if (!px || !job.waitall) break;
                pw = px;
                continue;
            }
        }
        sfsync(sfstderr);
        job.waitsafe = 0;
        nochild = job_reap(job.savesig);
        if (job.waitsafe) continue;
        if (nochild) break;
        if (shp->sigflag[SIGALRM] & SH_SIGTRAP) sh_timetraps(shp);
        if ((intr && (shp->trapnote & (SH_SIGSET | SH_SIGTRAP))) || (pid == 1 && !intr)) break;
    }
    if (intr && (shp->trapnote & (SH_SIGSET | SH_SIGTRAP))) shp->exitval = 1;
    pwfg = 0;
    if (pid == 1) {
        if (nochild) {
            for (pw = job.pwlist; pw; pw = px) {
                px = pw->p_nxtjob;
                pw->p_flag |= P_DONE;
                job_unpost(shp, pw, 1);
            }
        }
        job_unlock();
        return nochild;
    }
    job_unlock();
    exitset(shp);
    if (pid == 0) goto done;
    if (pw->p_pgrp) {
        job_reset(pw);
        // Propogate keyboard interrupts to parent.
        if ((pw->p_flag & P_SIGNALLED) && pw->p_exit == SIGINT &&
            !(shp->sigflag[SIGINT] & SH_SIGOFF)) {
            kill(getpid(), SIGINT);
        }
#ifdef SIGTSTP
        else if ((pw->p_flag & P_STOPPED) && pw->p_exit == SIGTSTP) {
            job.parent = 0;
            kill(getpid(), SIGTSTP);
        }
#endif /* SIGTSTP */
    } else {
        if (pw->p_pid == tcgetpgrp(JOBTTY)) {
            if (pw->p_pgrp == 0) pw->p_pgrp = pw->p_pid;
            job_reset(pw);
        }
        tty_set(-1, 0, NULL);
    }
done:
    if (!job.waitall && sh_isoption(shp, SH_PIPEFAIL)) return (nochild);
    if (!shp->intrap) {
        job_lock();
        for (pw = job.pwlist; pw; pw = px) {
            px = pw->p_nxtjob;
            job_unpost(shp, pw, 0);
        }
        job_unlock();
    }
    return nochild;
}

//
// Move job to foreground if bgflag == 'f'.
// Move job to background if bgflag == 'b'.
// Disown job if bgflag == 'd'.
//
int job_switch(struct process *pw, int bgflag) {
    Shell_t *shp = sh_getinterp();
    const char *msg;

    job_lock();
    if (!pw || !(pw = job_byjid((int)pw->p_job))) {
        job_unlock();
        return true;
    }
    if (bgflag == 'd') {
        for (; pw; pw = pw->p_nxtproc) pw->p_flag |= P_DISOWN;
        job_unlock();
        return false;
    }
#ifdef SIGTSTP
    if (bgflag == 'b') {
        sfprintf(outfile, "[%d]\t", (int)pw->p_job);
        shp->bckpid = pw->p_pid;
        pw->p_flag |= P_BG;
        msg = "&";
    } else {
        job_unlink(pw);
        pw->p_nxtjob = job.pwlist;
        job.pwlist = pw;
        msg = "";
    }
    hist_list(shgd->hist_ptr, outfile, pw->p_name, '&', ";");
    sfputr(outfile, msg, '\n');
    sfsync(outfile);
    if (bgflag == 'f') {
        if (!(pw = job_unpost(shp, pw, 1))) {
            job_unlock();
            return true;
        }
        job.waitall = 1;
        pw->p_flag |= P_FG;
        pw->p_flag &= ~P_BG;
        job_wait(pw->p_pid);
        job.waitall = 0;
    } else if (pw->p_flag & P_STOPPED) {
        job_unstop(pw);
    }
#endif  // SIGTSTP
    job_unlock();
    return false;
}

#ifdef SIGTSTP
//
// Set the foreground group associated with a job.
//
static void job_fgrp(struct process *pw, int newgrp) {
    for (; pw; pw = pw->p_nxtproc) pw->p_fgrp = newgrp;
}

//
// Turn off STOP state of a process group and send CONT signals.
//
static void job_unstop(struct process *px) {
    struct process *pw;
    int num = 0;
    for (pw = px; pw; pw = pw->p_nxtproc) {
        if (pw->p_flag & P_STOPPED) {
            num++;
            pw->p_flag &= ~(P_STOPPED | P_SIGNALLED | P_NOTIFY);
        }
    }
    if (num != 0) {
        if (px->p_fgrp != px->p_pgrp) killpg(px->p_fgrp, SIGCONT);
        killpg(px->p_pgrp, SIGCONT);
    }
}
#endif  // SIGTSTP

//
// Remove a job from table. If all the processes have not completed, unpost
// first non-completed process. Otherwise the job is removed and job_unpost
// returns NULL. pwlist is reset if the first job is removed. If <notify> is
// non-zero, then jobs with pending notifications are unposted.
//
static struct process *job_unpost(Shell_t *shp, struct process *pwtop, int notify) {
    struct process *pw;

    // Make sure all processes are done.
#ifdef DEBUG
    sfprintf(sfstderr, "ksh: job line %4d: drop pid=%d critical=%d pid=%d env=%d\n", __LINE__,
             getpid(), job.in_critical, pwtop->p_pid, pwtop->p_env);
    sfsync(sfstderr);
#endif  // DEBUG
    pwtop = pw = job_byjid((int)pwtop->p_job);
    if (!pw || (pw->p_flag & P_BG)) return (pw);
    for (; pw && (pw->p_flag & P_DONE) && (notify || !(pw->p_flag & P_NOTIFY) || pw->p_env);
         pw = pw->p_nxtproc) {
        ;  // empty loop
    }
    if (pw) return pw;
    if (pwtop->p_job == job.curjobid) return (0);
    // All processes complete, unpost job.
    if (pwtop) job_unlink(pwtop);
    for (pw = pwtop; pw; pw = pw->p_nxtproc) {
        if (pw && pw->p_exitval) *pw->p_exitval = pw->p_exit;
        // Save the exit status for background jobs.
        if ((pw->p_flag & P_EXITSAVE) || pw->p_pid == shp->spid) {
            struct jobsave *jp;
            // Save status for future wait.
            jp = jobsave_create(pw->p_pid);
            if (jp) {
                jp->env = pw->p_env;
                jp->exitval = pw->p_exit;
                if (pw->p_flag & P_SIGNALLED) jp->exitval |= SH_EXITSIG;
            }
            pw->p_flag &= ~P_EXITSAVE;
        }
        pw->p_flag &= ~P_DONE;
        job.numpost--;
        pw->p_nxtjob = freelist;
        if (pw->p_curdir) free(pw->p_curdir);
        freelist = pw;
    }
    pwtop->p_pid = 0;
#ifdef DEBUG
    sfprintf(sfstderr, "ksh: job line %4d: free pid=%d critical=%d job=%d\n", __LINE__, getpid(),
             job.in_critical, pwtop->p_job);
    sfsync(sfstderr);
#endif  // DEBUG
    if (pwtop) job_free((int)pwtop->p_job);
    return NULL;
}

//
// Unlink a job form the job list.
//
static void job_unlink(struct process *pw) {
    struct process *px;
    if (pw == job.pwlist) {
        job.pwlist = pw->p_nxtjob;
        job.curpgid = 0;
        return;
    }
    for (px = job.pwlist; px; px = px->p_nxtjob) {
        if (px->p_nxtjob == pw) {
            px->p_nxtjob = pw->p_nxtjob;
            return;
        }
    }
}

//
// Get an unused job number.
//
static int job_alloc(Shell_t *shp) {
    int j = 0;
    unsigned mask = 1;
    unsigned char *freeword;
    int jmax = BYTE(shgd->lim.child_max);

    // Skip to first word with a free slot. Freejobs is a bit vector, 0 is unused.
    for (j = 0; job.freejobs[j] == UCHAR_MAX; j++) {
        ;  // empty loop
    }
    if (j >= jmax) {
        struct process *pw;
        for (j = 1; j < shgd->lim.child_max; j++) {
            if ((pw = job_byjid(j)) && !job_unpost(shp, pw, 0)) break;
        }
        j /= CHAR_BIT;
        if (j >= jmax) return -1;
    }
    freeword = &job.freejobs[j];
    j *= CHAR_BIT;
    for (j++; mask & (*freeword); j++, mask <<= 1) {
        ;  // empty loop
    }
    *freeword |= mask;
    return j;
}

//
// Return a job number.
//
static void job_free(int n) {
    int j = (--n) / CHAR_BIT;
    unsigned mask;
    n -= j * CHAR_BIT;
    mask = 1 << n;
    job.freejobs[j] &= ~mask;
}

static char *job_sigmsg(Shell_t *shp, int sig) {
    static char signo[40];

    if (sig < shgd->sigmax && shgd->sigmsg[sig]) return (shgd->sigmsg[sig]);
#if defined(SIGRTMIN) && defined(SIGRTMAX)
    if (sig >= shp->gd->sigruntime[SH_SIGRTMIN] && sig <= shp->gd->sigruntime[SH_SIGRTMAX]) {
        static char sigrt[20];
        if (sig >
            shp->gd->sigruntime[SH_SIGRTMIN] +
                (shp->gd->sigruntime[SH_SIGRTMAX] - sig <= shp->gd->sigruntime[SH_SIGRTMIN]) / 2) {
            sfsprintf(sigrt, sizeof(sigrt), "SIGRTMAX-%d", shp->gd->sigruntime[SH_SIGRTMAX] - sig);
        } else {
            sfsprintf(sigrt, sizeof(sigrt), "SIGRTMIN+%d", sig - shp->gd->sigruntime[SH_SIGRTMIN]);
        }
        return sigrt;
    }
#endif

    sfsprintf(signo, sizeof(signo), sh_translate(e_signo), sig);
    return signo;
}

//
// See whether exit status has been saved and delete it.
// If pid==0, then oldest saved process is deleted.
// If pid is not found a -1 is returned.
//
static int job_chksave(pid_t pid, long env) {
    struct jobsave *jp = bck.list, *jpold = 0;
    int r = -1;
    int count = bck.count;
    struct back_save *bp = &bck;

again:
    while (jp && count-- > 0) {
        if (jp->pid == pid) break;
        if (pid == 0 && !jp->next) break;
        jpold = jp;
        jp = jp->next;
    }
    if (!jp && pid && (bp = bp->prev)) {
        count = bp->count;
        jp = bp->list;
        jpold = 0;
        goto again;
    }
    if (jp && (env < 0 || jp->env == env)) {
        r = 0;
        if (pid) r = jp->exitval;
        if (jpold) {
            jpold->next = jp->next;
        } else {
            bp->list = jp->next;
        }
        bp->count--;
        if (njob_savelist < NJOB_SAVELIST) {
            njob_savelist++;
            jp->next = job_savelist;
            job_savelist = jp;
        } else {
            free(jp);
        }
    }
    return r;
}

void *job_subsave(void) {
    struct back_save *bp = calloc(1, sizeof(struct back_save));
    job_lock();
    *bp = bck;
    bp->prev = bck.prev;
    bck.count = 0;
    bck.list = 0;
    bck.prev = bp;
    job_unlock();
    return (void *)bp;
}

void job_subrestore(Shell_t *shp, void *ptr) {
    struct jobsave *jp, *jpnext;
    struct back_save *bp = (struct back_save *)ptr;
    struct process *pw, *px, *pwnext;
    struct jobsave *end = NULL;
    int i = 0;

    job_lock();
    for (jp = bck.list; jp; jp = jpnext, i++) {
        if (!(jpnext = jp->next) || jp == jpnext) end = jp;
        if (i >= shgd->lim.child_max) break;
    }
    if (end) {
        end->next = bp->list;
    } else {
        bck.list = bp->list;
    }
    bck.count += bp->count;
    bck.prev = bp->prev;
    while (bck.count > shgd->lim.child_max) job_chksave(0, -1);
    for (pw = job.pwlist; pw; pw = pwnext) {
        pwnext = pw->p_nxtjob;
        if (pw->p_env != shp->curenv || pw->p_pid == shp->pipepid) continue;
        for (px = pw; px; px = px->p_nxtproc) px->p_flag |= P_DONE;
        job_unpost(shp, pw, 0);
    }

    free(bp);
    job_unlock();
}

int sh_waitsafe(void) { return (job.waitsafe); }

void job_fork(pid_t parent) {
#ifdef DEBUG
    sfprintf(sfstderr, "ksh: job line %4d: fork pid=%d critical=%d parent=%d\n", __LINE__, getpid(),
             job.in_critical, parent);
#endif  // DEBUG
    switch (parent) {
        case -1: {
            job_lock();
            jobfork++;
            break;
        }
        case 0: {
            jobfork = 0;
            job_unlock();
            job.waitsafe = 0;
            job.in_critical = 0;
            break;
        }
        default: {
            job_chksave(parent, -1);
            jobfork = 0;
            job_unlock();
            break;
        }
    }
}
