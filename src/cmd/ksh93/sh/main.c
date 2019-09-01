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
// UNIX shell
//
// S. R. Bourne
// Rewritten By David Korn
// AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "fcin.h"
#include "history.h"
#include "io.h"
#include "jobs.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shlex.h"
#include "shnodes.h"
#include "stk.h"
#include "terminal.h"
#include "variables.h"

// These routines are referenced by this module.
static_fn void exfile(Shell_t *, Sfio_t *, int);
static_fn void chkmail(Shell_t *shp, char *);

#ifndef environ
extern char **environ;
#endif

static struct stat lastmail;
static time_t mailtime;
static char beenhere = 0;

#ifdef PATH_BFPATH
#define PATHCOMP NULL
#else
#define PATHCOMP ""
#endif

//
// Search for file and exfile() it if it exists.
// Rreturn 1 if file found, 0 otherwise.
//
bool sh_source(Shell_t *shp, Sfio_t *iop, const char *file) {
    char *oid;
    char *nid;
    int fd;

    if (!file || !*file || (fd = path_open(shp, file, PATHCOMP)) < 0) {
        return false;
    }
    oid = error_info.id;
    nid = error_info.id = strdup(file);
    shp->st.filename = path_fullname(shp, stkptr(shp->stk, PATH_OFFSET));
    exfile(shp, iop, fd);
    error_info.id = oid;
    free(nid);
    return true;
}

int sh_main(int ac, char *av[], Shinit_f userinit) {
    int fdin = 0;
    char *name;
    Sfio_t *iop;
    Shell_t *shp;
    struct stat statb;
    int i;        /* set for restricted shell */
    bool rshflag; /* set for restricted shell */
    char *command;

    // Make sure we weren't started with several critical signals blocked from delivery.
    sh_sigaction(SIGALRM, SIG_UNBLOCK);
    sh_sigaction(SIGCHLD, SIG_UNBLOCK);
    sh_sigaction(SIGHUP, SIG_UNBLOCK);

    shp = sh_init(ac, av, userinit);
    set_debug_filename(av[0]);
    time(&mailtime);
    rshflag = sh_isoption(shp, SH_RESTRICTED);
    if (rshflag) sh_offoption(shp, SH_RESTRICTED);
    if (sigsetjmp(shp->jmpbuffer->buff, 0)) {
        // Begin script execution here.
        sh_reinit(shp, NULL);
        shp->gd->pid = getpid();
        shp->gd->ppid = getppid();
    }
    shp->fn_depth = shp->dot_depth = 0;
    command = error_info.id;
    // Set pidname '$$'.
    srand(shp->gd->pid & 0x7fff);
    if (nv_isnull(VAR_PS4)) nv_putval(VAR_PS4, e_traceprompt, NV_RDONLY);
    path_pwd(shp);
    iop = NULL;
    sh_onoption(shp, SH_BRACEEXPAND);
    if ((beenhere++) == 0) {
        sh_onstate(shp, SH_PROFILE);
        shp->lex_context->nonstandard = 0;
        if (shp->gd->ppid == 1) shp->login_sh++;
        if (shp->login_sh >= 2) sh_onoption(shp, SH_LOGIN_SHELL);
        // Decide whether shell is interactive.
        if (!sh_isoption(shp, SH_INTERACTIVE) && !sh_isoption(shp, SH_TFLAG) &&
            !sh_isoption(shp, SH_CFLAG) && sh_isoption(shp, SH_SFLAG) && tty_check(0) &&
            tty_check(STDERR_FILENO)) {
            sh_onoption(shp, SH_INTERACTIVE);
        }
        if (sh_isoption(shp, SH_INTERACTIVE)) {
            sh_onoption(shp, SH_BGNICE);
            sh_onoption(shp, SH_RC);
        }
        if (!sh_isoption(shp, SH_RC) &&
            (sh_isoption(shp, SH_BASH) && !sh_isoption(shp, SH_POSIX))) {
            sh_onoption(shp, SH_RC);
        }
        for (i = 0; i < elementsof(shp->offoptions.v); i++) {
            shp->options.v[i] &= ~shp->offoptions.v[i];
        }
        if (sh_isoption(shp, SH_INTERACTIVE)) {
#ifdef SIGXCPU
            sh_signal(SIGXCPU, (sh_sigfun_t)(SIG_DFL));
#endif  // SIGXCPU
#ifdef SIGXFSZ
            sh_signal(SIGXFSZ, (sh_sigfun_t)(SIG_DFL));
#endif  // SIGXFSZ
            sh_onoption(shp, SH_MONITOR);
        }
        job_init(shp, sh_isoption(shp, SH_LOGIN_SHELL));
        sh_source(shp, iop, INSTALL_PREFIX "/share/ksh/config.ksh");
        if (sh_isoption(shp, SH_LOGIN_SHELL)) {
            //  System profile.
            sh_source(shp, iop, e_sysprofile);
            if (!sh_isoption(shp, SH_NOUSRPROFILE) && !sh_isoption(shp, SH_PRIVILEGED)) {
                char **files = shp->gd->login_files;
                while ((name = *files++) && !sh_source(shp, iop, sh_mactry(shp, name))) {
                    ;  // empty loop
                }
            }
        }
        // Make sure PWD is set up correctly.
        path_pwd(shp);
        if (!sh_isoption(shp, SH_NOEXEC)) {
            if (!sh_isoption(shp, SH_NOUSRPROFILE) && !sh_isoption(shp, SH_PRIVILEGED) &&
                sh_isoption(shp, SH_RC)) {
#if SHOPT_BASH
                if (sh_isoption(shp, SH_BASH) && !sh_isoption(shp, SH_POSIX)) {
                    sh_source(shp, iop, e_bash_sysrc);
                    sh_source(
                        shp, iop,
                        shp->gd->rcfile ? shp->gd->rcfile : sh_mactry(shp, (char *)e_bash_rc));
                } else
#endif
                {
                    name = sh_mactry(shp, nv_getval(VAR_ENV));
                    if (name) name = *name ? strdup(name) : NULL;
                    if (!name || !strmatch(name, "?(.)/./*")) sh_source(shp, iop, e_sysrc);
                    if (name) {
                        sh_source(shp, iop, name);
                        free(name);
                    }
                }
            } else if (sh_isoption(shp, SH_INTERACTIVE) && sh_isoption(shp, SH_PRIVILEGED)) {
                sh_source(shp, iop, e_suidprofile);
            }
        }
        // Add enum type _Bool.
        i = 0;
        if (sh_isoption(shp, SH_XTRACE)) {
            i = 1;
            sh_offoption(shp, SH_XTRACE);
        }
        if (sh_isoption(shp, SH_NOEXEC)) {
            i |= 2;
            sh_offoption(shp, SH_NOEXEC);
        }
        if (sh_isoption(shp, SH_VERBOSE)) {
            i |= 4;
            sh_offoption(shp, SH_VERBOSE);
        }
        sh_trap(shp, "enum _Bool=(false true) ;", 0);
        if (i & 1) sh_onoption(shp, SH_XTRACE);
        if (i & 2) sh_onoption(shp, SH_NOEXEC);
        if (i & 4) sh_onoption(shp, SH_VERBOSE);
        shp->st.cmdname = error_info.id = command;
        sh_offstate(shp, SH_PROFILE);
        if (rshflag) sh_onoption(shp, SH_RESTRICTED);
        // Open input file if specified.
        if (shp->comdiv) {
        shell_c:
            iop = sfnew(NULL, shp->comdiv, strlen(shp->comdiv), 0, SF_STRING | SF_READ);
        } else {
            name = error_info.id;
            error_info.id = shp->shname;
            if (sh_isoption(shp, SH_SFLAG)) {
                fdin = 0;
            } else {
                char *sp;
                /* open stream should have been passed into shell */
                if (strmatch(name, e_devfdNN)) {
#if !__CYGWIN__
                    char *cp;
                    int type;
#endif
                    fdin = (int)strtol(name + 8, NULL, 10);
                    if (fstat(fdin, &statb) < 0) {
                        errormsg(SH_DICT, ERROR_system(1), e_open, name);
                        __builtin_unreachable();
                    }
#if !__CYGWIN__
                    //
                    // Try to undo effect of solaris 2.5+ change for argv for setuid scripts.
                    //
                    if (shp->st.repl_index > 0) av[shp->st.repl_index] = shp->st.repl_arg;
                    if (((type = sh_type(cp = av[0])) & SH_TYPE_SH) &&
                        (name = nv_getval(VAR_underscore)) &&
                        (!((type = sh_type(cp = name)) & SH_TYPE_SH))) {
                        av[0] = (type & SH_TYPE_LOGIN) ? cp : path_basename(cp);
                        // Exec to change $0 for ps.
                        execv(pathshell(), av);
                        // Exec failed.
                        shp->st.dolv[0] = av[0];
                    }
#endif
                    name = av[0];
                    sh_offoption(shp, SH_VERBOSE);
                    sh_offoption(shp, SH_XTRACE);
                } else {
                    int isdir = 0;
                    if ((fdin = sh_open(name, O_RDONLY, 0)) >= 0 &&
                        (fstat(fdin, &statb) < 0 || S_ISDIR(statb.st_mode))) {
                        sh_close(fdin);
                        isdir = 1;
                        fdin = -1;
                    } else {
                        shp->st.filename = path_fullname(shp, name);
                    }
                    sp = NULL;
                    if (fdin < 0 && !strchr(name, '/')) {
#ifdef PATH_BFPATH
                        if (path_absolute(shp, name, NULL)) {
                            sp = stkptr(shp->stk, PATH_OFFSET);
                        }
#else
                        sp = path_absolute(shp, name, NULL);
#endif
                        if (sp) {
                            fdin = sh_open(sp, O_RDONLY, 0);
                            if (fdin >= 0) shp->st.filename = path_fullname(shp, sp);
                        }
                    }
                    if (fdin < 0) {
                        if (isdir) errno = EISDIR;
                        error_info.id = av[0];
                        if (sp || errno != ENOENT) {
                            errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_open, name);
                            __builtin_unreachable();
                        }
                        // Try sh -c 'name "$@"'.
                        sh_onoption(shp, SH_CFLAG);
                        shp->comdiv = malloc(strlen(name) + 7);
                        name = stpcpy(shp->comdiv, name);
                        if (shp->st.dolc) stpcpy(name, " \"$@\"");
                        goto shell_c;
                    }
                    if (fdin == 0) fdin = sh_iomovefd(shp, fdin);
                }
                shp->readscript = shp->shname;
            }
            error_info.id = name;
            shp->comdiv--;
        }
    } else {
        fdin = shp->infd;
    }

    if (sh_isoption(shp, SH_INTERACTIVE)) sh_onstate(shp, SH_INTERACTIVE);
    nv_putval(VAR_IFS, (char *)e_sptbnl, NV_RDONLY);
    exfile(shp, iop, fdin);
    sh_done(shp, 0);
    // NOTREACHED
    // TODO: If this isn't reached it should probably be an abort() call.
    return 0;
}

//
// <iop> is not null when the input is a string.
// <fno> is the input file descriptor.
//
static_fn void exfile(Shell_t *shp, Sfio_t *iop, int fno) {
    time_t curtime;
    Shnode_t *t;
    int maxtry = IOMAXTRY, tdone = 0, execflags;
    int states, jmpval;
    checkpt_t buff;

    sh_pushcontext(shp, &buff, SH_JMPERREXIT);
    // Open input stream.
    nv_putval(VAR_sh_file, shp->st.filename, NV_NOFREE);
    if (!iop) {
        if (fno > 0) {
            int r;
            if (fno < 10 && ((r = sh_fcntl(fno, F_DUPFD, 10)) >= 10)) {
                shp->fdstatus[r] = shp->fdstatus[fno];
                sh_close(fno);
                fno = r;
            }
            (void)fcntl(fno, F_SETFD, FD_CLOEXEC);
            shp->fdstatus[fno] |= IOCLEX;
            iop = sh_iostream(shp, fno, fno);
        } else {
            iop = sfstdin;
        }
    } else {
        fno = -1;
    }
    shp->infd = fno;
    if (sh_isstate(shp, SH_INTERACTIVE)) {
        if (nv_isnull(VAR_PS1)) {
            nv_putval(VAR_PS1, (shp->gd->euserid ? e_stdprompt : e_supprompt), NV_RDONLY);
        }
        sh_sigdone(shp);
        if (sh_histinit(shp)) sh_onoption(shp, SH_HISTORY);
    } else {
        if (!sh_isstate(shp, SH_PROFILE)) {
            buff.mode = SH_JMPEXIT;
            sh_onoption(shp, SH_TRACKALL);
        }
        sh_offstate(shp, SH_INTERACTIVE);
        if (sh_isoption(shp, SH_MONITOR)) sh_onstate(shp, SH_MONITOR);
        sh_offstate(shp, SH_HISTORY);
        sh_offoption(shp, SH_HISTORY);
    }
    states = sh_getstate(shp);
    jmpval = sigsetjmp(buff.buff, 0);
    if (jmpval) {
        Sfio_t *top;
        sh_iorestore(shp, 0, jmpval);
        hist_flush(shp->gd->hist_ptr);
        sfsync(shp->outpool);
        shp->st.execbrk = shp->st.breakcnt = 0;
        // Check for return from profile or env file.
        if (sh_isstate(shp, SH_PROFILE) &&
            (jmpval == SH_JMPFUN || jmpval == SH_JMPEXIT || jmpval == SH_JMPERREXIT)) {
            sh_setstate(shp, states);
            goto done;
        }
        if (!sh_isoption(shp, SH_INTERACTIVE) || sh_isstate(shp, SH_FORKED) ||
            (jmpval > SH_JMPERREXIT && job_close(shp) >= 0)) {
            sh_offstate(shp, SH_INTERACTIVE);
            sh_offstate(shp, SH_MONITOR);
            goto done;
        }
        exitset(shp);
        // Skip over remaining input.
        top = fcfile();
        if (top) {
            while (fcget() > 0) {
                ;  // empty loop
            }
            fcclose();
            while ((top = sfstack(iop, SF_POPSTACK))) sfclose(top);
        }
        // Make sure that we own the terminal.
#ifdef SIGTSTP
        tcsetpgrp(job.fd, shp->gd->pid);
#endif  // SIGTSTP
    }
    // Error return here.
    sfclrerr(iop);
    sh_setstate(shp, states);
    shp->st.optindex = 1;
    opt_info.offset = 0;
    shp->st.loopcnt = 0;
    shp->trapnote = 0;
    shp->intrap = 0;
    error_info.line = 1;
    shp->inlineno = 1;
    shp->binscript = 0;
    shp->exittrap = 0;
    shp->errtrap = 0;
    shp->end_fn = 0;
    if (sfeof(iop)) goto eof_or_error;
    // Command loop.
    while (1) {
        shp->nextprompt = 1;
        sh_freeup(shp);
        stkset(shp->stk, NULL, 0);
        sh_offstate(shp, SH_STOPOK);
        sh_offstate(shp, SH_ERREXIT);
        sh_offstate(shp, SH_VERBOSE);
        sh_offstate(shp, SH_TIMING);
        sh_offstate(shp, SH_GRACE);
        sh_offstate(shp, SH_TTYWAIT);
        if (sh_isoption(shp, SH_VERBOSE)) sh_onstate(shp, SH_VERBOSE);
        sh_onstate(shp, SH_ERREXIT);
        // -eim  flags don't apply to profiles.
        if (sh_isstate(shp, SH_PROFILE)) {
            sh_offstate(shp, SH_INTERACTIVE);
            sh_offstate(shp, SH_ERREXIT);
            sh_offstate(shp, SH_MONITOR);
        }
        if (sh_isstate(shp, SH_INTERACTIVE) && !tdone) {
            char *mail;
#ifdef JOBS
            sh_offstate(shp, SH_MONITOR);
            if (sh_isoption(shp, SH_MONITOR)) sh_onstate(shp, SH_MONITOR);
            if (job.pwlist) {
                job_walk(shp, sfstderr, job_list, JOB_NFLAG, NULL);
                job_wait((pid_t)0);
            }
#endif  // JOBS
            if ((mail = nv_getval(VAR_MAILPATH)) || (mail = nv_getval(VAR_MAIL))) {
                time(&curtime);
                if ((curtime - mailtime) >= sh_mailchk) {
                    chkmail(shp, mail);
                    mailtime = curtime;
                }
            }
            if (shp->gd->hist_ptr) hist_eof(shp->gd->hist_ptr);
            // Sets timeout for command entry.
            shp->timeout = shp->st.tmout;
#if READ_TIMEOUT > 0
            if (shp->timeout <= 0 || shp->timeout > READ_TIMEOUT) shp->timeout = READ_TIMEOUT;
#endif
            shp->inlineno = 1;
            error_info.line = 1;
            shp->trapnote = 0;
            if (buff.mode == SH_JMPEXIT) {
                buff.mode = SH_JMPERREXIT;
#ifdef DEBUG
                errormsg(SH_DICT, ERROR_warn(0), "%d: mode changed to JMP_EXIT", getpid());
#endif
            }
        }
        errno = 0;
        if (tdone || !sfreserve(iop, 0, 0)) {
        eof_or_error:
            if (sh_isstate(shp, SH_INTERACTIVE) && !sferror(iop)) {
                if (--maxtry > 0 && sh_isoption(shp, SH_IGNOREEOF) && !sferror(sfstderr)) {
                    // It is theoretically possible for fno == -1 at this point. That would be bad.
                    assert(fno >= 0);
                    if ((shp->fdstatus[fno] & IOTTY)) {
                        sfclrerr(iop);
                        errormsg(SH_DICT, 0, e_logout);
                        continue;
                    }
                } else if (job_close(shp) < 0) {
                    continue;
                }
            }
            if (errno == 0 && sferror(iop) && --maxtry > 0) {
                sfclrlock(iop);
                sfclrerr(iop);
                continue;
            }
            goto done;
        }
        shp->exitval = shp->savexit;
        maxtry = IOMAXTRY;
        if (sh_isstate(shp, SH_INTERACTIVE) && shp->gd->hist_ptr) {
            job_wait((pid_t)0);
            hist_eof(shp->gd->hist_ptr);
            sfsync(sfstderr);
        }
        if (sh_isoption(shp, SH_HISTORY)) sh_onstate(shp, SH_HISTORY);
        job.waitall = job.curpgid = 0;
        error_info.flags |= ERROR_INTERACTIVE;
        t = sh_parse(shp, iop, 0);
        if (!sh_isstate(shp, SH_INTERACTIVE) && !sh_isoption(shp, SH_CFLAG)) {
            error_info.flags &= ~ERROR_INTERACTIVE;
        }
        shp->readscript = NULL;
        if (sh_isstate(shp, SH_INTERACTIVE) && shp->gd->hist_ptr) hist_flush(shp->gd->hist_ptr);
        sh_offstate(shp, SH_HISTORY);
        if (!t) continue;

        execflags = sh_state(SH_ERREXIT) | sh_state(SH_INTERACTIVE);

        shp->st.execbrk = 0;
        sh_exec(shp, t, execflags);
        if (shp->forked) {
            sh_offstate(shp, SH_INTERACTIVE);
            goto done;
        }
        // This is for sh -t.
        if (sh_isoption(shp, SH_TFLAG) && !sh_isstate(shp, SH_PROFILE)) tdone++;
    }
done:
    sh_popcontext(shp, &buff);
    if (sh_isstate(shp, SH_INTERACTIVE)) {
        sfputc(sfstderr, '\n');
        job_close(shp);
    }
    if (jmpval == SH_JMPSCRIPT) {
        siglongjmp(shp->jmplist->buff, jmpval);
    } else if (jmpval == SH_JMPEXIT || jmpval == SH_JMPERREXIT) {
        sh_done(shp, 0);
    }
    if (fno > 0) sh_close(fno);
    if (shp->st.filename) {
        free(shp->st.filename);
        shp->st.filename = NULL;
    }
}

// Prints out messages if files in list have been modified since last call.
static_fn void chkmail(Shell_t *shp, char *files) {
    char *cp, *sp, *qp;
    char save;
    struct argnod *arglist = NULL;
    int offset = stktell(shp->stk);
    char *savstak = stkptr(shp->stk, 0);
    struct stat statb;

    if (*(cp = files) == 0) return;
    sp = cp;
    do {
        // Skip to : or end of string saving first '?'.
        for (qp = NULL; *sp && *sp != ':'; sp++) {
            if ((*sp == '?' || *sp == '%') && qp == 0) qp = sp;
        }
        save = *sp;
        *sp = 0;
        // Change '?' to end-of-string.
        if (qp) *qp = 0;
        do {
            // See if time has been modified since last checked and the access
            // time <= the modification time.
            if (sh_stat(cp, &statb) >= 0 && statb.st_mtime >= mailtime &&
                statb.st_atime <= statb.st_mtime) {
                // Check for directory.
                if (!arglist && S_ISDIR(statb.st_mode)) {
                    // Generate list of directory entries.
                    path_complete(shp, cp, "/*", &arglist);
                } else {
                    // If the file has shrunk, or if the size is zero then
                    // don't print anything.
                    if (statb.st_size &&
                        (statb.st_ino != lastmail.st_ino || statb.st_dev != lastmail.st_dev ||
                         statb.st_size > lastmail.st_size)) {
                        // Save and restore $_.
                        char *save = shp->lastarg;
                        shp->lastarg = cp;
                        errormsg(SH_DICT, 0, sh_mactry(shp, qp ? qp + 1 : (char *)e_mailmsg));
                        shp->lastarg = save;
                    }
                    lastmail = statb;
                    break;
                }
            }
            if (arglist) {
                cp = arglist->argval;
                arglist = arglist->argchn.ap;
            } else {
                cp = 0;
            }
        } while (cp);
        if (qp) *qp = '?';
        *sp++ = save;
        cp = sp;
    } while (save);
    stkset(shp->stk, savstak, offset);
}

#undef EXECARGS
#undef PSTAT
#if _hdr_execargs && defined(pdp11)
#include <execargs.h>
#define EXECARGS 1
#endif

#if _lib_pstat && _hdr_sys_pstat
#include <sys/pstat.h>
#define PSTAT 1
#endif
