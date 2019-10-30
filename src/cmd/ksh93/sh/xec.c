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
// UNIX shell parse tree executer.
//
// David Korn
// AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

#if _hdr_stdlib
#include <stdlib.h>
#elif _hdr_malloc
#include <malloc.h>
#endif

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "cdt.h"
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
#include "shcmd.h"
#include "shnodes.h"
#include "shtable.h"
#include "stk.h"
#include "streval.h"
#include "terminal.h"
#include "test.h"
#include "variables.h"

#if USE_SPAWN
#include "spawnvex.h"
#endif

#define SH_NTFORK SH_TIMING

extern int nice(int);
#if USE_SPAWN
static_fn pid_t sh_ntfork(Shell_t *, const Shnode_t *, char *[], int *, int);
#endif  // USE_SPAWN

static_fn void sh_funct(Shell_t *, Namval_t *, int, char *[], struct argnod *, int);
static_fn void coproc_init(Shell_t *, int pipes[]);

static Timer_t *timeout;
static char nlock;
static char pipejob;
static int restorefd;
#if USE_SPAWN
static int restorevex;
#endif

struct funenv {
    Namval_t *node;
    struct argnod *env;
    Namval_t **nref;
};

#if USE_SPAWN
static_fn int io_usevex(struct ionod *iop) {
    struct ionod *first = iop;
    for (; iop; iop = iop->ionxt) {
        if ((iop->iofile & IODOC) && !(iop->iofile & IOQUOTE) && iop != first) return 0;
    }
    return IOUSEVEX;
}
#endif  // USE_SPAWN
#if 1
#undef IOUSEVEX
#define IOUSEVEX 0
#endif

// ======== command execution ========

#if !has_dev_fd
static_fn void fifo_check(void *handle) {
    Shell_t *shp = handle;
    pid_t pid = getppid();
    if (pid == 1) {
        unlink(shp->fifo);
        sh_done(shp, 0);
    }
}
#endif  // !has_dev_fd

#if _lib_getrusage

// Use getrusage() rather than times() since the former typically has higher resolution.
#include <sys/resource.h>

static_fn void get_cpu_times(struct timeval *tv_usr, struct timeval *tv_sys) {
    struct rusage usage_self, usage_child;

    getrusage(RUSAGE_SELF, &usage_self);
    getrusage(RUSAGE_CHILDREN, &usage_child);
    timeradd(&usage_self.ru_utime, &usage_child.ru_utime, tv_usr);
    timeradd(&usage_self.ru_stime, &usage_child.ru_stime, tv_sys);
}

#else  // _lib_getrusage

static_fn void get_cpu_times(struct timeval *tv_usr, struct timeval *tv_sys) {
    struct timeval tv1, tv2;
    double dtime;
    long clk_tck = sysconf(_SC_CLK_TCK);
    struct tms cpu_times;
    times(&cpu_times);

    dtime = (double)cpu_times.tms_utime / clk_tck;
    tv1.tv_sec = dtime / 60;
    tv1.tv_usec = 1000000 * (dtime - tv1.tv_sec);
    dtime = (double)cpu_times.tms_cutime / clk_tck;
    tv2.tv_sec = dtime / 60;
    tv2.tv_usec = 1000000 * (dtime - tv2.tv_sec);
    timeradd(&tv1, &tv2, tv_usr);

    dtime = (double)cpu_times.tms_stime / clk_tck;
    tv1.tv_sec = dtime / 60;
    tv1.tv_usec = 1000000 * (dtime - tv1.tv_sec);
    dtime = (double)cpu_times.tms_cstime / clk_tck;
    tv2.tv_sec = dtime / 60;
    tv2.tv_usec = 1000000 * (dtime - tv2.tv_sec);
    timeradd(&tv1, &tv2, tv_sys);
}

#endif  // _lib_getrusage

static_fn struct timeval clock_t_delta(int clk_tck, clock_t after, clock_t before) {
    struct timeval tv_after, tv_before, tv;

    double dtime = (double)after / clk_tck;
    tv_after.tv_sec = floor(dtime);
    tv_after.tv_usec = 1000000 * (dtime - tv_after.tv_sec);
    dtime = (double)before / clk_tck;
    tv_before.tv_sec = floor(dtime);
    tv_before.tv_usec = 1000000 * (dtime - tv_before.tv_sec);

    timersub(&tv_after, &tv_before, &tv);
    return tv;
}

static inline double timeval_to_double(struct timeval tv) {
    return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
}

//
// The following two functions allow command substituion for non-builtins to use a pipe and to wait
// for the pipe to close before restoring to a temp file.
//
static int subpipe[3], subdup, tsetio, usepipe;

static_fn void iousepipe(Shell_t *shp) {
    int i;
    int fd = sffileno(sfstdout);

    if (!sh_iovalidfd(shp, fd)) abort();
    if (usepipe) {
        usepipe++;
        sh_iounpipe(shp);
    }
    sh_rpipe(subpipe);
    usepipe++;
    if (shp->comsub != 1) {
        subpipe[2] = sh_fcntl(subpipe[1], F_DUPFD, 10);
        sh_close(subpipe[1]);
        return;
    }
    subpipe[2] = sh_fcntl(fd, F_DUPFD_CLOEXEC, 10);
    if (!sh_iovalidfd(shp, subpipe[2])) abort();
    shp->fdstatus[subpipe[2]] = shp->fdstatus[1];

    close(fd);
    i = fcntl(subpipe[1], F_DUPFD, fd);
    assert(i != -1);  // it should be impossible for the fcntl() to fail

    shp->fdstatus[1] = shp->fdstatus[subpipe[1]] & ~IOCLEX;
    sh_close(subpipe[1]);
    subdup = shp->subdup;
    if (subdup) {
        for (i = 0; i < 10; i++) {
            if (subdup & (1 << i)) {
                sh_close(i);
                (void)fcntl(1, F_DUPFD, i);  // this can't fail
                shp->fdstatus[i] = shp->fdstatus[1];
            }
        }
    }
}

void sh_iounpipe(Shell_t *shp) {
    int n;
    char buff[SF_BUFSIZE];
    int fd = sffileno(sfstdout);

    if (!usepipe) return;
    --usepipe;
    if (shp->comsub > 1) {
        sh_close(subpipe[2]);
        while (read(subpipe[0], buff, sizeof(buff)) > 0) {
            ;  // empty loop
        }
        goto done;
    }

    close(fd);
    n = fcntl(subpipe[2], F_DUPFD, fd);
    assert(n != -1);  // it should be impossible for the fcntl() to fail

    shp->fdstatus[1] = shp->fdstatus[subpipe[2]];
    if (subdup) {
        for (n = 0; n < 10; n++) {
            if (subdup & (1 << n)) {
                sh_close(n);
                (void)fcntl(1, F_DUPFD, n);  // this can't fail so we don't check the return value
                shp->fdstatus[n] = shp->fdstatus[1];
            }
        }
    }
    shp->subdup = 0;
    sh_close(subpipe[2]);
    if (usepipe == 0) {
        while (1) {
            while (job.waitsafe && job.savesig == SIGCHLD) {
                if (!vmbusy()) {
                    job.in_critical++;
                    job_reap(SIGCHLD);
                    job.in_critical--;
                    break;
                }
                sh_delay(1);
            }
            if ((n = read(subpipe[0], buff, sizeof(buff))) == 0) break;
            if (n > 0) {
                sfwrite(sfstdout, buff, n);
            } else if (errno != EINTR) {
                break;
            }
        }
    }
done:
    sh_close(subpipe[0]);
    subpipe[0] = -1;
    tsetio = 0;
    usepipe = 0;
}

//
// Print time <t> in h:m:s format with precision <p>.
//
static_fn void l_time(Sfio_t *outfile, struct timeval *tv, int precision) {
    int hr = tv->tv_sec / (60 * 60);
    int min = (tv->tv_sec / 60) % 60;
    int sec = tv->tv_sec % 60;
    int frac = tv->tv_usec;

    // Scale fraction from micro to milli, centi, or deci second according to precision.
    for (int n = 3 + (3 - precision); n > 0; --n) frac /= 10;

    if (hr) sfprintf(outfile, "%dh", hr);
    if (precision) {
        sfprintf(outfile, "%dm%d%c%0*ds", min, sec, getdecimal(), precision, frac);
    } else {
        sfprintf(outfile, "%dm%ds", min, sec);
    }
}

#define TM_REAL_IDX 0
#define TM_USR_IDX 1
#define TM_SYS_IDX 2

static_fn void p_time(Shell_t *shp, Sfio_t *out, const char *format, struct timeval tm[3]) {
    int c, n, offset = stktell(shp->stk);
    const char *first;
    struct timeval tv_cpu_sum;
    struct timeval *tvp;
    Stk_t *stkp = shp->stk;

    for (first = format; *format; format++) {
        c = *format;
        if (c != '%') continue;
        bool l_modifier = false;
        int precision = 3;

        sfwrite(stkp, first, format - first);
        c = *++format;
        if (c == '\0') {
            // If a lone percent is the last character of the format pretend
            // the user had written `%%` for a literal percent.
            sfwrite(stkp, "%", 1);
            first = format + 1;
            break;
        } else if (c == '%') {
            first = format;
            continue;
        }

        if (c >= '0' && c <= '9') {
            precision = (c > '3') ? 3 : (c - '0');
            c = *++format;
        }

        if (c == 'P') {
            struct timeval tv_real = tm[TM_REAL_IDX];
            struct timeval tv_cpu;
            timeradd(&tm[TM_USR_IDX], &tm[TM_SYS_IDX], &tv_cpu);

            double d = timeval_to_double(tv_real);
            if (d) d = 100.0 * timeval_to_double(tv_cpu) / d;
            sfprintf(stkp, "%.*f", precision, d);
            first = format + 1;
            continue;
        }

        if (c == 'l') {
            l_modifier = true;
            c = *++format;
        }

        if (c == 'R') {
            tvp = &tm[TM_REAL_IDX];
        } else if (c == 'U') {
            tvp = &tm[TM_USR_IDX];
        } else if (c == 'S') {
            tvp = &tm[TM_SYS_IDX];
        } else if (c == 'C') {
            timeradd(&tm[TM_USR_IDX], &tm[TM_SYS_IDX], &tv_cpu_sum);
            tvp = &tv_cpu_sum;
        } else {
            errormsg(SH_DICT, ERROR_exit(0), e_badtformat, c);
            continue;
        }

        if (l_modifier) {
            l_time(stkp, tvp, precision);
        } else {
            // Scale fraction from micro to milli, centi, or deci second according to precision.
            int frac = tvp->tv_usec;
            for (int n = 3 + (3 - precision); n > 0; --n) frac /= 10;
            sfprintf(stkp, "%d.%0*d", tvp->tv_sec, precision, frac);
        }
        first = format + 1;
    }

    if (format > first) sfwrite(stkp, first, format - first);
    sfputc(stkp, '\n');
    n = stktell(stkp) - offset;
    sfwrite(out, stkptr(stkp, offset), n);
    stkseek(stkp, offset);
}

//
// Clear argument pointers that point into the stack.
//
static_fn int xec_p_arg(Shell_t *, struct argnod *, int);
static_fn int xec_p_switch(Shell_t *, struct regnod *);

static_fn int xec_p_comarg(Shell_t *shp, struct comnod *com) {
    Namval_t *np = com->comnamp;
    int n = xec_p_arg(shp, com->comset, ARG_ASSIGN);
    if (com->comarg && (com->comtyp & COMSCAN)) n += xec_p_arg(shp, com->comarg, 0);
    if (com->comstate && np) {
        // Call builtin to cleanup state.
        Shbltin_t *bp = &shp->bltindata;
        void *save_ptr = bp->ptr;
        void *save_data = bp->data;
        bp->bnode = np;
        bp->vnode = com->comnamq;
        bp->ptr = nv_context(np);
        bp->data = com->comstate;
        // Was bp->flags = SH_END_OPTIM but no builtin actually uses the flags structure member
        // and it's companion symbols, SH_BEGIN_OPTIM, isn't used anywhere.
        bp->flags = 0;
        funptr(np)(0, NULL, bp);
        bp->ptr = save_ptr;
        bp->data = save_data;
    }
    com->comstate = NULL;
    if (com->comarg && !np) n++;
    return n;
}

extern void sh_optclear(Shell_t *, void *);

static_fn int sh_tclear(Shell_t *shp, Shnode_t *t) {
    if (!t) return 0;

    int n;
    switch (t->tre.tretyp & COMMSK) {
        case TTIME:
        case TPAR: {
            return sh_tclear(shp, t->par.partre);
        }
        case TCOM: {
            return xec_p_comarg(shp, (struct comnod *)t);
        }
        case TSETIO:
        case TFORK: {
            return sh_tclear(shp, t->fork.forktre);
        }
        case TIF: {
            n = sh_tclear(shp, t->if_.iftre);
            n += sh_tclear(shp, t->if_.thtre);
            n += sh_tclear(shp, t->if_.eltre);
            return n;
        }
        case TWH: {
            n = 0;
            if (t->wh.whinc) n = sh_tclear(shp, (Shnode_t *)(t->wh.whinc));
            n += sh_tclear(shp, t->wh.whtre);
            n += sh_tclear(shp, t->wh.dotre);
            return n;
        }
        case TLST:
        case TAND:
        case TORF:
        case TFIL: {
            n = sh_tclear(shp, t->lst.lstlef);
            return n + sh_tclear(shp, t->lst.lstrit);
        }
        case TARITH: {
            return xec_p_arg(shp, t->ar.arexpr, ARG_ARITH);
        }
        case TFOR: {
            n = sh_tclear(shp, t->for_.fortre);
            return n + sh_tclear(shp, (Shnode_t *)t->for_.forlst);
        }
        case TSW: {
            n = xec_p_arg(shp, t->sw.swarg, 0);
            return n + xec_p_switch(shp, t->sw.swlst);
        }
        case TFUN: {
            n = sh_tclear(shp, t->funct.functtre);
            return n + sh_tclear(shp, (Shnode_t *)t->funct.functargs);
        }
        case TTST: {
            if ((t->tre.tretyp & TPAREN) == TPAREN) {
                return sh_tclear(shp, t->lst.lstlef);
            }
            n = xec_p_arg(shp, &(t->lst.lstlef->arg), 0);
            if (t->tre.tretyp & TBINARY) n += xec_p_arg(shp, &(t->lst.lstrit->arg), 0);
            return n;
        }
        default: { return 0; }
    }
}

static_fn int xec_p_arg(Shell_t *shp, struct argnod *arg, int flag) {
    while (arg) {
        if (strlen(arg->argval) || (arg->argflag == ARG_RAW)) {
            arg->argchn.ap = NULL;
        } else if (flag == 0) {
            sh_tclear(shp, (Shnode_t *)arg->argchn.ap);
        } else {
            sh_tclear(shp, ((struct fornod *)arg->argchn.ap)->fortre);
        }
        arg = arg->argnxt.ap;
    }
    return 0;
}

static_fn int xec_p_switch(Shell_t *shp, struct regnod *reg) {
    int n = 0;
    while (reg) {
        n += xec_p_arg(shp, reg->regptr, 0);
        n += sh_tclear(shp, reg->regcom);
        reg = reg->regnxt;
    }
    return n;
}

static_fn void out_pattern(Sfio_t *iop, const char *cp, int n) {
    int c;

    do {
        switch (c = *cp) {
            case 0: {
                if (n < 0) return;
                c = n;
                break;
            }
            case '\n': {
                sfputr(iop, "$'\\n", '\'');
                continue;
            }
            case '\\': {
                if (!(c = *++cp)) c = '\\';
            }
            // FALLTHRU
            case ' ':
            case '<':
            case '>':
            case ';':
            case '$':
            case '`':
            case '\t': {
                sfputc(iop, '\\');
                break;
            }
            default: { break; }
        }
        sfputc(iop, c);
    } while (*cp++);
}

static_fn void out_string(Sfio_t *iop, const char *cp, int c, int quoted) {
    if (quoted) {
        int n = stktell(stkstd);
        cp = sh_fmtq(cp);
        if (iop == stkstd && cp == stkptr(stkstd, n)) {
            *stkptr(stkstd, stktell(stkstd) - 1) = c;
            return;
        }
    }
    sfputr(iop, cp, c);
}

struct Level {
    Namfun_t namfun;
    short maxlevel;
};

//
// This is for a debugger but it hasn't been tested yet. If a debug script sets .sh.level it should
// set up the scope as if you were executing in that level.
//
static_fn void put_level(Namval_t *np, const void *val, nvflag_t flags, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    Shscope_t *sp;
    struct Level *lp = (struct Level *)fp;
    int16_t level, oldlevel = (int16_t)nv_getnum(np);

    nv_putv(np, val, flags, fp);
    if (!val) {
        fp = nv_stack(np, NULL);
        if (fp && !fp->nofree) free(fp);
        return;
    }
    level = nv_getnum(np);
    if (level < 0 || level > lp->maxlevel) {
        nv_putv(np, (char *)&oldlevel, NV_INT16, fp);
        // Perhaps this should be an error.
        return;
    }
    if (level == oldlevel) return;
    sp = sh_getscope(shp, level, SEEK_SET);
    if (sp) {
        sh_setscope(shp, sp);
        error_info.id = sp->cmdname;
    }
}

static const Namdisc_t level_disc = {.dsize = sizeof(struct Level), .putval = put_level};

static_fn struct Level *init_level(Shell_t *shp, int level) {
    struct Level *lp = calloc(1, sizeof(struct Level));

    lp->maxlevel = level;
    _nv_unset(VAR_sh_level, 0);
    nv_onattr(VAR_sh_level, NV_INT16 | NV_NOFREE);
    shp->last_root = nv_dict(VAR_sh);
    nv_putval(VAR_sh_level, (char *)&lp->maxlevel, NV_INT16);
    lp->namfun.disc = &level_disc;
    nv_disc(VAR_sh_level, &lp->namfun, DISC_OP_FIRST);
    return lp;
}

//
// Write the current command on the stack and make it available as .sh.command.
//
int sh_debug(Shell_t *shp, const char *trap, const char *name, const char *subscript,
             char *const argv[], int flags) {
    Stk_t *stkp = shp->stk;
    struct sh_scoped savst;
    Namval_t *np = VAR_sh_command;
    char *sav = stkptr(stkp, 0);
    int n = 4, offset = stktell(stkp);
    const char *cp = "+=( ";
    Sfio_t *iop = stkstd;
    short level;

    if (shp->indebug) return 0;
    shp->indebug = 1;
    if (name) {
        sfputr(iop, name, -1);
        if (subscript) {
            sfputc(iop, '[');
            out_string(iop, subscript, ']', 1);
        }
        if (!(flags & ARG_APPEND)) cp += 1, n -= 1;
        if (!(flags & ARG_ASSIGN)) n -= 2;
        sfwrite(iop, cp, n);
    }
    if (*argv && !(flags & ARG_RAW)) out_string(iop, *argv++, ' ', 0);
    n = (flags & ARG_ARITH);
    while ((cp = *argv++)) {
        if ((flags & ARG_EXP) && argv[1] == 0) {
            out_pattern(iop, cp, ' ');
        } else {
            out_string(iop, cp, ' ', n ? 0 : (flags & (ARG_RAW | ARG_NOGLOB)) || *argv);
        }
    }
    if (flags & ARG_ASSIGN) {
        sfputc(iop, ')');
    } else if (iop == stkstd) {
        *stkptr(stkp, stktell(stkp) - 1) = 0;
    }
    STORE_VT(np->nvalue, const_cp, stkfreeze(stkp, 1));
    // Now setup .sh.level variable.
    shp->st.lineno = error_info.line;
    level = shp->fn_depth + shp->dot_depth;
    shp->last_root = nv_dict(VAR_sh);
    if (!VAR_sh_level->nvfun || !VAR_sh_level->nvfun->disc ||
        nv_isattr(VAR_sh_level, NV_INT16 | NV_NOFREE) != (NV_INT16 | NV_NOFREE)) {
        init_level(shp, level);
    } else {
        nv_putval(VAR_sh_level, (char *)&level, NV_INT16);
    }
    savst = shp->st;
    shp->st.trap[SH_DEBUGTRAP] = 0;
    n = sh_trap(shp, trap, 0);
    STORE_VT(np->nvalue, const_cp, NULL);
    shp->indebug = 0;
    if (shp->st.cmdname) error_info.id = shp->st.cmdname;
    nv_putval(VAR_sh_file, shp->st.filename, NV_NOFREE);
    nv_putval(VAR_sh_fun, shp->st.funname, NV_NOFREE);
    shp->st = savst;
    if (sav != stkptr(stkp, 0)) {
        stkset(stkp, sav, 0);
    } else {
        stkseek(stkp, offset);
    }
    return n;
}

//
// Returns true when option -<c> is specified.
//
static_fn bool checkopt(char *argv[], int c) {
    char *cp;

    while ((cp = *++argv)) {
        if (*cp == '+') continue;
        if (*cp != '-' || cp[1] == '-') break;
        if (strchr(++cp, c)) return 1;
        if (*cp == 'h' && cp[1] == 0 && *++argv == 0) break;
    }
    return false;
}

static_fn void free_list(struct openlist *olist) {
    struct openlist *item, *next;

    for (item = olist; item; item = next) {
        next = item->next;
        free(item);
    }
}

//
// Set ${.sh.name} and ${.sh.subscript}.
// Set _ to reference for ${.sh.name}[$.sh.subscript].
//
static_fn nvflag_t set_instance(Shell_t *shp, Namval_t *nq, Namval_t *node, struct Namref *nr) {
    char *sp = NULL;
    char *cp;
    Namarr_t *ap;
    Namval_t *np;

    if (!nv_isattr(nq, NV_MINIMAL | NV_EXPORT | NV_ARRAY) && (np = nq->nvenv) && nv_isarray(np)) {
        nq = np;
    } else if (nv_isattr(nq, NV_MINIMAL) == NV_MINIMAL && !nv_type(nq) &&
               (np = nv_typeparent(nq))) {
        nq = np;
    }
    cp = nv_name(nq);
    memset(nr, 0, sizeof(*nr));
    nr->np = nq;
    nr->root = shp->var_tree;
    nr->table = shp->last_table;
    if (!nr->table && shp->namespace) nr->table = shp->namespace;
    shp->instance = 1;
    ap = nv_arrayptr(nq);
    if (ap) {
        sp = nv_getsub(nq);
        if (sp) nr->sub = strdup(sp);
    }
    shp->instance = 0;
    if (shp->var_tree != shp->var_base && !nv_search_namval(nq, nr->root, NV_NOSCOPE)) {
        nr->root = shp->namespace ? nv_dict(shp->namespace) : shp->var_base;
    }
    nv_putval(VAR_sh_name, cp, NV_NOFREE);
    memcpy(node, VAR_underscore, sizeof(*node));
    STORE_VT(VAR_underscore->nvalue, nrp, nr);
    nv_setattr(VAR_underscore, NV_REF | NV_NOFREE);
    VAR_underscore->nvfun = NULL;
    VAR_underscore->nvenv = NULL;
    if (ap && nr->sub) {
        nv_putval(VAR_sh_subscript, nr->sub, NV_NOFREE);
        return ap->flags & ARRAY_SCAN;
    }
    return 0;
}

static_fn void unset_instance(Namval_t *nq, Namval_t *node, struct Namref *nr, nvflag_t mode) {
    UNUSED(nq);

    STORE_VT(VAR_underscore->nvalue, nrp, FETCH_VT(node->nvalue, nrp));
    nv_setattr(VAR_underscore, node->nvflag);
    VAR_underscore->nvfun = node->nvfun;
    if (nr->sub) {
        nv_putsub(nr->np, nr->sub, 0, mode);
        free(nr->sub);
    }
    _nv_unset(VAR_sh_name, 0);
    _nv_unset(VAR_sh_subscript, 0);
}

static_fn Sfio_t *openstream(Shell_t *shp, struct ionod *iop, int *save) {
    Sfio_t *sp;
    int savein;
    int fd = sh_redirect(shp, iop, 3);

    savein = dup(0);
    if (fd == 0) fd = savein;
    sp = sfnew(NULL, NULL, SF_UNBOUND, fd, SF_READ);
    close(0);
    if (sh_open("/dev/null", O_RDONLY | O_CLOEXEC) != 0) abort();
    shp->offsets[0] = -1;
    shp->offsets[1] = 0;
    *save = savein;
    return sp;
}

static_fn Namval_t *enter_namespace(Shell_t *shp, Namval_t *nsp) {
    Namval_t *path = nsp, *fpath = nsp, *onsp = shp->namespace;
    Dt_t *root = NULL;
    Dt_t *oroot = NULL;
    char *val;

    if (nsp) {
        if (!nv_istable(nsp)) {
            nsp = NULL;
        } else if (nv_dict(nsp)->view != shp->var_base) {
            return onsp;
        }
    }
    if (!nsp && !onsp) return NULL;
    if (onsp == nsp) return nsp;
    if (onsp) {
        oroot = nv_dict(onsp);
        if (!nsp) {
            path = nv_search(VAR_PATH->nvname, oroot, NV_NOSCOPE);
            fpath = nv_search(VAR_FPATH->nvname, oroot, NV_NOSCOPE);
        }
        if (shp->var_tree == oroot) {
            shp->var_tree = shp->var_tree->view;
            oroot = shp->var_base;
        }
    }
    if (nsp) {
        if (shp->var_tree == shp->var_base) {
            shp->var_tree = nv_dict(nsp);
        } else {
            for (root = shp->var_tree; root->view != oroot; root = root->view) {
                ;  // empty loop
            }
            dtview(root, nv_dict(nsp));
        }
    }
    shp->namespace = nsp;
    if (path && (path = nv_search(VAR_PATH->nvname, shp->var_tree, NV_NOSCOPE)) &&
        (val = nv_getval(path))) {
        nv_putval(path, val, NV_RDONLY);
    }
    if (fpath && (fpath = nv_search(VAR_FPATH->nvname, shp->var_tree, NV_NOSCOPE)) &&
        (val = nv_getval(fpath))) {
        nv_putval(fpath, val, NV_RDONLY);
    }
    return onsp;
}

__attribute__((noreturn)) static_fn void forked_child(Shell_t *shp, const Shnode_t *t, int flags,
                                                      int type, bool no_fork, char *com0,
                                                      char **com, pid_t parent, int topfd,
                                                      int vexi) {
#if !USE_SPAWN
    UNUSED(vexi);
#endif
    // This is the FORKED branch (child) of execute.
    volatile int jmpval;
    checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
    struct ionod *iop;
    int rewrite = 0;
    if (no_fork) sh_sigreset(shp, 2);
    sh_pushcontext(shp, buffp, SH_JMPEXIT);
    jmpval = sigsetjmp(buffp->buff, 0);
    if (jmpval) goto done;
    if ((type & FINT) && !sh_isstate(shp, SH_MONITOR)) {
        // Default std input for &.
        sh_signal(SIGINT, (sh_sigfun_t)(SIG_IGN));
        sh_signal(SIGQUIT, (sh_sigfun_t)(SIG_IGN));
        shp->sigflag[SIGINT] = SH_SIGOFF;
        shp->sigflag[SIGQUIT] = SH_SIGOFF;
        if (!shp->st.ioset) {
            if (sh_close(0) >= 0) {
                int tmp_fd = sh_open("/dev/null", O_RDONLY, 0);
                assert(tmp_fd == 0);
            }
        }
    }
    sh_offstate(shp, SH_MONITOR);
    // Pipe in or out.
    if ((type & FAMP) && sh_isoption(shp, SH_BGNICE)) nice(4);

#if !has_dev_fd
    if (shp->fifo && (type & (FPIN | FPOU))) {
        int fn, fd = (type & FPIN) ? 0 : 1;
        Timer_t *fifo_timer = sh_timeradd(500, 1, fifo_check, shp);
        fn = sh_open(shp->fifo, fd ? O_WRONLY : O_RDONLY);
        timerdel(fifo_timer);
        sh_iorenumber(shp, fn, fd);
        sh_close(fn);
        sh_delay(.001);
        unlink(shp->fifo);
        free(shp->fifo);
        shp->fifo = NULL;
        type &= ~(FPIN | FPOU);
    }
#endif  // !has_dev_fd
    if (type & FPIN) {
        sh_iorenumber(shp, shp->inpipe[0], 0);
        if (!(type & FPOU) || (type & FCOOP)) sh_close(shp->inpipe[1]);
    }
    if (type & FPOU) {
        sh_iorenumber(shp, shp->outpipe[1], 1);
        sh_pclose(shp->outpipe);
    }
    if ((type & COMMSK) != TCOM) {
        error_info.line = t->fork.forkline - shp->st.firstline;
    }
    if (shp->topfd) sh_iounsave(shp);
    topfd = shp->topfd;
    if (com0 && (iop = t->tre.treio)) {
        for (; iop; iop = iop->ionxt) {
            if (iop->iofile & IOREWRITE) rewrite = 1;
        }
    }
    sh_redirect(shp, t->tre.treio, 1 | IOUSEVEX);
    if (rewrite) {
        job_lock();
        while ((parent = fork()) < 0) _sh_fork(shp, parent, 0, NULL);
        if (parent) {
            job.toclear = 0;
            job_post(shp, parent, 0);
            job_wait(parent);
            sh_iorestore(shp, topfd, SH_JMPCMD);
#if USE_SPAWN
            if (shp->vexp->cur > vexi) sh_vexrestore(shp, vexi);
#endif
            sh_done(shp, (shp->exitval & SH_EXITSIG) ? (shp->exitval & SH_EXITMASK) : 0);
        }
        job_unlock();
    }
    if ((type & COMMSK) != TCOM) {
        // Don't clear job table for out pipes so that jobs comand can be used in a
        // pipeline.
        if (!no_fork && !(type & FPOU)) job_clear(shp);
        sh_exec(shp, t->fork.forktre, flags | sh_state(SH_NOFORK) | sh_state(SH_FORKED));
    } else if (com0) {
        sh_offoption(shp, SH_ERREXIT);
        sh_freeup(shp);
        path_exec(shp, com0, com, t->com.comset);
    }
done:
    sh_popcontext(shp, buffp);
    if (jmpval > SH_JMPEXIT) siglongjmp(shp->jmplist->buff, jmpval);
    sh_done(shp, 0);
}

int sh_exec(Shell_t *shp, const Shnode_t *t, int flags) {
    sh_sigcheck(shp);

    if (!t) return shp->exitval;
    if (shp->st.execbrk) return shp->exitval;
    if (sh_isoption(shp, SH_NOEXEC)) return shp->exitval;

    Stk_t *stkp = shp->stk;
    int type = flags;
    char *com0 = NULL;
    int errorflg = (type & sh_state(SH_ERREXIT)) | (flags & ARG_OPTIMIZE);
    int execflg = (type & sh_state(SH_NOFORK));
    int execflg2 = (type & sh_state(SH_FORKED));
    int mainloop = (type & sh_state(SH_INTERACTIVE));
#if USE_SPAWN
    int ntflag = (type & sh_state(SH_NTFORK));
#else
    int ntflag = 0;
#endif
    int topfd = shp->topfd;
    char *sav = stkptr(stkp, 0);
    char *cp = NULL;
    char **com = NULL;
    char *comn;
    int argn;
    int skipexitset = 0;
#if USE_SPAWN
    int vexi = shp->vexp->cur;
#else
    int vexi = 0;
#endif
    pid_t *procsub = NULL;
    volatile int was_interactive = 0;
    volatile int was_errexit = sh_isstate(shp, SH_ERREXIT);
    volatile int was_monitor = sh_isstate(shp, SH_MONITOR);
    volatile int echeck = 0;

    if (flags & sh_state(SH_INTERACTIVE)) {
        if (pipejob == 2) job_unlock();
        nlock = 0;
        pipejob = 0;
        job.curpgid = 0;
        job.curjobid = 0;
        flags &= ~sh_state(SH_INTERACTIVE);
    }
    sh_offstate(shp, SH_ERREXIT);
    sh_offstate(shp, SH_DEFPATH);
    if (was_errexit & flags) sh_onstate(shp, SH_ERREXIT);
    if (was_monitor & flags) sh_onstate(shp, SH_MONITOR);
    type = t->tre.tretyp;
    if (!shp->intrap) shp->oldexit = shp->exitval;
    shp->exitval = 0;
    shp->lastsig = 0;
    shp->lastpath = NULL;
    if (shp->exittrap || shp->errtrap) execflg = 0;
    switch (type & COMMSK) {
        case TCOM: {
            struct argnod *argp;
            char *trap;
            Namval_t *np, *nq, *last_table;
            struct ionod *io;
            int command = 0;
            nvflag_t nvflags = NV_ASSIGN;
            shp->bltindata.invariant = type >> (COMBITS + 2);
            shp->bltindata.pwdfd = shp->pwdfd;
            type &= (COMMSK | COMSCAN);
            sh_stats(STAT_SCMDS);
            error_info.line = t->com.comline - shp->st.firstline;
#if USE_SPAWN
            spawnvex_add(shp->vex, SPAWN_frame, 0, 0, 0);
#endif
            com = sh_argbuild(shp, &argn, &(t->com), flags & ARG_OPTIMIZE);
            procsub = shp->procsub;
            shp->procsub = NULL;
            echeck = 1;
            if (t->tre.tretyp & COMSCAN) {
                argp = t->com.comarg;
                if (argp && *com && !(argp->argflag & ARG_RAW)) sh_sigcheck(shp);
            }
            np = t->com.comnamp;
            nq = t->com.comnamq;
            if (np && shp->namespace && nq != shp->namespace &&
                nv_isattr(np, NV_BLTIN | NV_INTEGER | BLT_SPC) != (NV_BLTIN | BLT_SPC)) {
                Namval_t *mp;
                mp = sh_fsearch(shp, com[0], 0);
                if (mp) {
                    nq = shp->namespace;
                    np = mp;
                }
            }
            com0 = com[0];
            shp->xargexit = 0;
            while (np == SYSCOMMAND) {
                int n = b_command(0, com, &shp->bltindata);
                if (n == 0) break;
                command += n;
                np = NULL;
                if (!(com0 = *(com += n))) break;
                np = nv_bfsearch(com0, shp->bltin_tree, &nq, &cp);
            }
            if (shp->xargexit) {
                shp->xargmin -= command;
                shp->xargmax -= command;
            } else {
                shp->xargmin = 0;
            }
            argn -= command;
            if (np && is_abuiltin(np)) {
                if (!command) {
                    Namval_t *mp;
                    if (shp->namespace && (mp = sh_fsearch(shp, np->nvname, 0))) {
                        np = mp;
                    } else {
                        np = dtsearch(shp->fun_tree, np);
                    }
                }
            }
            if (com0) {
                if (!np && !strchr(com0, '/')) {
                    Dt_t *root = command ? shp->bltin_tree : shp->fun_tree;
                    np = nv_bfsearch(com0, root, &nq, &cp);
                    if (shp->namespace && !nq && !cp) np = sh_fsearch(shp, com0, 0);
                }
                comn = com[argn - 1];
            }
            io = t->tre.treio;
        tryagain:
            shp->envlist = argp = t->com.comset;
            if (shp->envlist) {
                if (argn == 0 ||
                    (np && (nv_isattr(np, BLT_DCL) || (!command && nv_isattr(np, BLT_SPC))))) {
                    Namval_t *tp = NULL;
                    if (argn) {
                        if (checkopt(com, 'A')) {
                            nvflags |= NV_ARRAY;
                        } else if (checkopt(com, 'a')) {
                            nvflags |= NV_IARRAY;
                        }
                    }
                    if (np) nvflags |= NV_UNJUST;
                    if (np == SYSLOCAL) {
                        if (!nv_getval(VAR_sh_fun)) {
                            errormsg(SH_DICT, ERROR_exit(1), "%s: can only be used in a function",
                                     com0);
                            __builtin_unreachable();
                        }
                        if (!shp->st.var_local) {
                            sh_scope(shp, NULL, 0);
                            shp->st.var_local = shp->var_tree;
                        }
                    }
                    if (np == SYSTYPESET || (np && FETCH_VT(np->nvalue, shbltinp) ==
                                                       FETCH_VT(SYSTYPESET->nvalue, shbltinp))) {
                        if (np != SYSTYPESET) {
                            shp->typeinit = np;
                            tp = nv_type(np);
                        }
                        if (checkopt(com, 'C')) nvflags |= NV_COMVAR;
                        if (checkopt(com, 'S')) nvflags |= NV_STATIC;
                        if (checkopt(com, 'm')) nvflags |= NV_MOVE;
                        if (checkopt(com, 'n')) {
                            nvflags |= NV_NOREF;
                        } else if (argn >= 3 && checkopt(com, 'T')) {
                            if (shp->namespace) {
                                char *sp, *xp;
                                if (!shp->strbuf2) shp->strbuf2 = sfstropen();
                                sfprintf(shp->strbuf2, "%s%s%c", NV_CLASS, nv_name(shp->namespace),
                                         0);
                                char *p = sfstruse(shp->strbuf2);
                                assert(p);
                                shp->prefix = strdup(p);
                                xp = shp->prefix + strlen(NV_CLASS);
                                for (sp = xp + 1; sp;) {
                                    sp = strchr(sp, '.');
                                    if (sp) *sp = 0;
                                    nv_open(shp->prefix, shp->var_base, NV_VARNAME);
                                    if (sp) *sp++ = '.';
                                }
                            } else {
                                shp->prefix = NV_CLASS;
                            }
                            nvflags |= NV_TYPE;
                        }
                        if ((shp->fn_depth && !shp->prefix) || np == SYSLOCAL) {
                            nvflags |= NV_NOSCOPE;
                        }
                    } else if (np == SYSEXPORT) {
                        nvflags |= NV_EXPORT;
                    }
                    if (nvflags & (NV_EXPORT | NV_NOREF)) {
                        nvflags |= NV_IDENT;
                    } else {
                        nvflags |= NV_VARNAME;
                    }
                    if (np && nv_isattr(np, BLT_DCL)) nvflags |= NV_DECL;
                    if (t->com.comtyp & COMFIXED) ((Shnode_t *)t)->com.comtyp &= ~COMFIXED;
                    shp->nodelist = sh_setlist(shp, argp, nvflags, tp);
                    if (np == shp->typeinit) shp->typeinit = NULL;
                    shp->envlist = argp;
                    argp = NULL;
                }
            }
            last_table = shp->last_table;
            shp->last_table = NULL;
            if ((io || argn)) {
                Shbltin_t *bp = NULL;
                static char *argv[2] = {NULL, NULL};
                int tflags = 1;
                if (np && nv_isattr(np, BLT_DCL)) tflags |= 2;
                if (argn == 0) {
                    // Fake 'true' built-in.
                    np = SYSTRUE;
                    argv[0] = nv_name(np);
                    com = argv;
                }
                // set +x doesn't echo.
                else if ((t->tre.tretyp & FSHOWME) && sh_isoption(shp, SH_SHOWME)) {
                    int ison = sh_isoption(shp, SH_XTRACE);
                    if (!ison) sh_onoption(shp, SH_XTRACE);
                    sh_trace(shp, com - command, tflags);
                    if (io) sh_redirect(shp, io, SH_SHOWME | IOHERESTRING);
                    if (!ison) sh_offoption(shp, SH_XTRACE);
                    break;
                } else if ((np != SYSSET) && sh_isoption(shp, SH_XTRACE)) {
                    sh_trace(shp, com - command, tflags);
                }
                trap = shp->st.trap[SH_DEBUGTRAP];
                if (trap) {
                    int n = sh_debug(shp, trap, NULL, NULL, com, ARG_RAW);
                    if (n == 255 && shp->fn_depth + shp->dot_depth) {
                        np = SYSRETURN;
                        argn = 1;
                        com[0] = np->nvname;
                        com[1] = 0;
                        io = 0;
                        argp = NULL;
                    } else if (n == 2) {
                        break;
                    }
                }
                if (io) sfsync(shp->outpool);
                shp->lastpath = NULL;
                if (!np && !strchr(com0, '/')) {
                    if (path_search(shp, com0, NULL, 1)) {
                        error_info.line = t->com.comline - shp->st.firstline;
                        if (!shp->namespace || !(np = sh_fsearch(shp, com0, 0))) {
                            np = nv_search(com0, shp->fun_tree, 0);
                        }
                        if (!np || !FETCH_VT(np->nvalue, ip)) {
                            Namval_t *mp = nv_search(com0, shp->bltin_tree, 0);
                            if (mp) np = mp;
                        } else if ((t->com.comtyp & COMFIXED) && nv_type(np)) {
                            ((Shnode_t *)t)->com.comtyp &= ~COMFIXED;
                            goto tryagain;
                        }
                    } else {
                        if ((np = nv_search(com0, shp->track_tree, 0)) &&
                            !nv_isattr(np, NV_NOALIAS) && FETCH_VT(np->nvalue, const_cp)) {
                            np = nv_search(nv_getval(np), shp->bltin_tree, 0);
                        } else {
                            np = NULL;
                        }
                    }
                }
                if (np && pipejob == 2) {
                    if (shp->comsub == 1 && np && is_abuiltin(np) && *np->nvname == '/') {
                        np = NULL;
                    } else {
                        job_unlock();
                        nlock--;
                        pipejob = 1;
                    }
                }
                // Check for builtins.
                if (np && is_abuiltin(np)) {
                    volatile int scope = 0, share = 0;
                    volatile void *save_ptr;
                    volatile void *save_data;
                    int jmpval, save_prompt;
                    int was_nofork = execflg ? sh_isstate(shp, SH_NOFORK) : 0;
                    checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
                    volatile unsigned long was_vi = 0, was_emacs = 0, was_gmacs = 0;
#if !O_SEARCH
                    struct stat statb;
#endif  // O_SEARCH
                    bp = &shp->bltindata;
                    save_ptr = bp->ptr;
                    save_data = bp->data;
#if !O_SEARCH
                    memset(&statb, 0, sizeof(struct stat));
#endif  // O_SEARCH
                    if (strchr(nv_name(np), '/')) {
                        // Disable editors for built-in versions of commands on PATH.
                        was_vi = sh_isoption(shp, SH_VI);
                        was_emacs = sh_isoption(shp, SH_EMACS);
                        was_gmacs = sh_isoption(shp, SH_GMACS);
                        sh_offoption(shp, SH_VI);
                        sh_offoption(shp, SH_EMACS);
                        sh_offoption(shp, SH_GMACS);
                    }
                    if (execflg) sh_onstate(shp, SH_NOFORK);
                    sh_pushcontext(shp, buffp, SH_JMPCMD);
                    jmpval = sigsetjmp(buffp->buff, 1);
                    if (jmpval == 0) {
                        if (!(nv_isattr(np, BLT_ENV))) error_info.flags |= ERROR_SILENT;
                        errorpush(&buffp->err, 0);
                        if (io) {
                            struct openlist *item;
                            if (np == SYSEXEC) {
                                type = 1 + !com[1];
                            } else {
                                type = (execflg && !shp->subshell && !shp->st.trapcom[0]);
                            }
                            shp->redir0 = 1;
                            sh_redirect(shp, io,
                                        type | (FETCH_VT(np->nvalue, shbltinp) == b_source
                                                    ? 0
                                                    : IOHERESTRING | IOUSEVEX));
                            for (item = buffp->olist; item; item = item->next) item->strm = NULL;
                        }
                        if (!nv_isattr(np, BLT_ENV) && !nv_isattr(np, BLT_SPC)) {
                            if (!shp->pwd) {
                                path_pwd(shp);
#if !O_SEARCH
                            } else if (shp->pwdfd >= 0) {
                                fstat(shp->pwdfd, &statb);
                            } else if (shp->pwd) {
                                stat(e_dot, &statb);
#endif  // O_SEARCH
                            }
                            sfsync(NULL);
                            share = sfset(sfstdin, SF_SHARE, 0);
                            sh_onstate(shp, SH_STOPOK);
                            sfpool(sfstderr, NULL, SF_WRITE);
                            sfset(sfstderr, SF_LINE, 1);
                            save_prompt = shp->nextprompt;
                            shp->nextprompt = 0;
                        }
                        if (argp) {
                            scope++;
                            sh_scope(shp, argp, 0);
                        }
                        opt_info.index = opt_info.offset = 0;
                        opt_info.disc = NULL;
                        error_info.id = *com;
                        if (argn) shp->exitval = 0;
                        shp->bltinfun = funptr(np);
                        bp->bnode = np;
                        bp->vnode = nq;
                        bp->ptr = nv_context(np);
                        bp->data = t->com.comstate;
                        bp->sigset = 0;
                        bp->notify = 0;
                        bp->flags = ((flags & ARG_OPTIMIZE) != 0);
                        if (shp->subshell && nv_isattr(np, BLT_NOSFIO)) sh_subtmpfile(shp);
                        if (execflg && !shp->subshell && !shp->st.trapcom[0] &&
                            !shp->st.trap[SH_ERRTRAP] && shp->fn_depth == 0 &&
                            !nv_isattr(np, BLT_ENV)) {
                            // Do close-on-exec.
                            int fd;
                            for (fd = 0; fd < shp->gd->lim.open_max; fd++) {
                                if ((shp->fdstatus[fd] & IOCLEX) && fd != shp->infd &&
                                    (fd != shp->pwdfd)) {
                                    sh_close(fd);
                                }
                            }
                        }
                        if (argn) {
                            shp->exitval = (*shp->bltinfun)(argn, com, bp);
                            sfsync(NULL);
                        }
                        if (error_info.flags & ERROR_INTERACTIVE) tty_check(STDERR_FILENO);
                        ((Shnode_t *)t)->com.comstate = shp->bltindata.data;
                        bp->data = (void *)save_data;
                        if (shp->exitval && errno == EINTR && shp->lastsig) {
                            shp->exitval = SH_EXITSIG | shp->lastsig;
                        } else if (!nv_isattr(np, BLT_EXIT)) {
                            shp->exitval &= SH_EXITMASK;
                        }
                    } else {
                        struct openlist *item;
                        for (item = buffp->olist; item; item = item->next) {
                            if (item->strm) {
                                sfclrlock(item->strm);
                                if (shp->gd->hist_ptr && item->strm == shp->gd->hist_ptr->histfp) {
                                    hist_close(shp->gd->hist_ptr);
                                } else {
                                    sfclose(item->strm);
                                }
                            }
                        }
                        if (shp->bltinfun && (error_info.flags & ERROR_NOTIFY)) {
                            (*shp->bltinfun)(-2, com, bp);
                            sfsync(NULL);
                        }
                        // Failure on special built-ins fatal.
                        if (jmpval <= SH_JMPCMD && (!nv_isattr(np, BLT_SPC) || command)) {
                            jmpval = 0;
                        }
                    }
#if USE_SPAWN
                    if (np != SYSEXEC && shp->vex->cur) {
#if 1
                        spawnvex_apply(shp->vex, 0, SPAWN_RESET | SPAWN_FRAME);
#else
                        int fd;
                        spawnvex_apply(shp->vex, 0, SPAWN_RESET | SPAWN_FRAME);
                        if (shp->comsub && (fd = sffileno(sfstdout)) != 1 && fd >= 0)
                            spawnvex_add(shp->vex, fd, 1, 0, 0);
#endif
                    }
#endif
                    bp->bnode = NULL;
                    if (bp->ptr != nv_context(np)) np->nvfun = (Namfun_t *)bp->ptr;
                    if (execflg && !was_nofork) sh_offstate(shp, SH_NOFORK);
                    if (!(nv_isattr(np, BLT_ENV))) {
#if O_SEARCH
                        while ((sh_fchdir(shp->pwdfd) < 0) && errno == EINTR) errno = 0;
#else   // O_SEARCH
                        if (shp->pwd || (shp->pwdfd >= 0)) {
                            struct stat stata;
                            stat(e_dot, &stata);
                            // Restore directory changed.
                            if (statb.st_ino != stata.st_ino || statb.st_dev != stata.st_dev) {
                                // Chdir for directories on HSM/tapeworms may take minutes.
                                int err = errno;
                                if (shp->pwdfd >= 0) {
                                    while ((fchdir(shp->pwdfd) < 0) && errno == EINTR) {
                                        errno = err;
                                    }
                                } else {
                                    while ((chdir(shp->pwd) < 0) && errno == EINTR) errno = err;
                                }
                            }
                        }
#endif  // O_SEARCH
                        sh_offstate(shp, SH_STOPOK);
                        if (share & SF_SHARE) sfset(sfstdin, SF_PUBLIC | SF_SHARE, 1);
                        sfset(sfstderr, SF_LINE, 0);
                        sfpool(sfstderr, shp->outpool, SF_WRITE);
                        sfpool(sfstdin, NULL, SF_WRITE);
                        shp->nextprompt = save_prompt;
                    }
                    sh_popcontext(shp, buffp);
                    errorpop(&buffp->err);
                    error_info.flags &= ~(ERROR_SILENT | ERROR_NOTIFY);
                    shp->bltinfun = 0;
                    if (buffp->olist) free_list(buffp->olist);
                    if (was_vi) {
                        sh_onoption(shp, SH_VI);
                    } else if (was_emacs) {
                        sh_onoption(shp, SH_EMACS);
                    } else if (was_gmacs) {
                        sh_onoption(shp, SH_GMACS);
                    }
                    if (scope) sh_unscope(shp);
                    bp->ptr = (void *)save_ptr;
                    bp->data = (void *)save_data;
                    // Don't restore for subshell exec.
                    if ((shp->topfd > topfd) && !(shp->subshell && np == SYSEXEC)) {
                        sh_iorestore(shp, topfd, jmpval);
                    }
#if USE_SPAWN
                    if (shp->vexp->cur > vexi) sh_vexrestore(shp, vexi);
#endif
                    shp->redir0 = 0;
                    if (jmpval) siglongjmp(shp->jmplist->buff, jmpval);
                    if (shp->exitval >= 0) goto setexit;
                    np = NULL;
                    type = 0;
                }
                // Check for functions.
                if (!command && np && nv_isattr(np, NV_FUNCTION)) {
                    volatile int indx;
                    int jmpval = 0;
                    checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
                    Namval_t node;
                    Namval_t *namespace = NULL;
                    struct Namref nr;
                    nvflag_t mode;
                    struct slnod *slp;
                    if (!FETCH_VT(np->nvalue, ip)) {
                        indx = path_search(shp, com0, NULL, 0);
                        if (indx == 1) {
                            if (shp->namespace) {
                                np = sh_fsearch(shp, com0, 0);
                            } else {
                                np = nv_search(com0, shp->fun_tree, NV_NOSCOPE);
                            }
                        }

                        assert(np);  // Coverity CID#253708
                        if (!FETCH_VT(np->nvalue, ip)) {
                            if (indx == 1) {
                                errormsg(SH_DICT, ERROR_exit(0), e_defined, com0);
                                shp->exitval = ERROR_NOEXEC;
                            } else {
                                errormsg(SH_DICT, ERROR_exit(0), e_found, "function");
                                shp->exitval = ERROR_NOENT;
                            }
                            goto setexit;
                        }
                    }
                    // Increase refcnt for unset.
                    slp = (struct slnod *)np->nvenv;
                    sh_funstaks(slp->slchild, 1);
                    stklink(slp->slptr);
                    if (nq) {
                        Namval_t *mp = NULL;
                        if (nv_isattr(np, NV_STATICF) && (mp = nv_type(nq))) nq = mp;
                        shp->last_table = last_table;
                        mode = set_instance(shp, nq, &node, &nr);
                    }
                    if (io) {
                        indx = shp->topfd;
                        sh_pushcontext(shp, buffp, SH_JMPCMD);
                        jmpval = sigsetjmp(buffp->buff, 0);
                    }
                    if (jmpval == 0) {
                        if (io) indx = sh_redirect(shp, io, execflg | IOUSEVEX);
                        if (*np->nvname == '.') {
                            char *ep;
                            bool type = 0;
                            cp = np->nvname + 1;
                            if (strncmp(cp, "sh.type.", 8) == 0) {
                                cp += 8;
                                type = true;
                            }
                            ep = strrchr(cp, '.');
                            if (ep) {
                                if (type) {
                                    while (--ep > cp && *ep != '.') {
                                        ;  // empty loop
                                    }
                                }
                                *ep = 0;
                                namespace = nv_search(cp - 1, shp->var_base, NV_NOSCOPE);
                                *ep = '.';
                            }
                        }
                        namespace = enter_namespace(shp, namespace);
                        sh_funct(shp, np, argn, com, t->com.comset, (flags & ~ARG_OPTIMIZE));
                    }
                    enter_namespace(shp, namespace);
#if USE_SPAWN
                    spawnvex_apply(shp->vex, 0, SPAWN_RESET | SPAWN_FRAME);
                    if (shp->vexp->cur > vexi) sh_vexrestore(shp, vexi);
#endif
                    if (io) {
                        if (buffp->olist) free_list(buffp->olist);
                        sh_popcontext(shp, buffp);
                        sh_iorestore(shp, indx, jmpval);
                    }
                    if (nq) unset_instance(nq, &node, &nr, mode);
                    sh_funstaks(slp->slchild, -1);
                    stkclose(slp->slptr);
                    if (jmpval > SH_JMPFUN || (io && jmpval > SH_JMPIO)) {
                        siglongjmp(shp->jmplist->buff, jmpval);
                    }
                    goto setexit;
                }
            } else if (!io) {
#if USE_SPAWN
                spawnvex_apply(shp->vex, 0, SPAWN_RESET | SPAWN_FRAME);
#endif
            setexit:
                exitset(shp);
                break;
            }
        }
        // FALLTHRU
        case TFORK: {
            pid_t parent;
            bool no_fork;
            int jobid;
            int pipes[3];
#if USE_SPAWN
            bool unpipe = false;
#endif

            if (shp->subshell) {
                sh_subtmpfile(shp);
                if ((type & (FAMP | TFORK)) == (FAMP | TFORK)) {
                    if (shp->comsub && !(shp->fdstatus[1] & IONOSEEK)) {
                        iousepipe(shp);
#if USE_SPAWN
                        unpipe = true;
#endif  // USE_SPAWN
                    }
                    sh_subfork();
                }
            }
            no_fork =
                !ntflag && !(type & (FAMP | FPOU)) && !shp->subshell &&
                !(shp->st.trapcom[SIGINT] && *shp->st.trapcom[SIGINT]) && !shp->st.trapcom[0] &&
                !shp->st.trap[SH_ERRTRAP] && shp->jmplist->mode != SH_JMPEVAL &&
                (execflg2 ||
                 (execflg && shp->fn_depth == 0 && !(pipejob && sh_isoption(shp, SH_PIPEFAIL))));
            if (sh_isstate(shp, SH_PROFILE) || shp->dot_depth) {
                // Disable foreground job monitor.
                if (!(type & FAMP)) {
                    sh_offstate(shp, SH_MONITOR);
#if has_dev_fd
                } else if (!(type & FINT)) {
                    sh_offstate(shp, SH_MONITOR);
#endif  // has_dev_fd
                }
            }
            if (no_fork) {
                job.parent = parent = 0;
            } else {
                if (((type & (FAMP | FINT)) == (FAMP | FINT)) &&
                    (job.maxjob = nv_getnum(VAR_JOBMAX)) > 0) {
                    while (job.numbjob >= job.maxjob) {
                        job_lock();
                        job_reap(0);
                        job_unlock();
                    }
                }
                nv_getval(VAR_RANDOM);
                restorefd = shp->topfd;
#if USE_SPAWN
                restorevex = shp->vexp->cur;
#endif
                if (type & FCOOP) {
                    pipes[2] = 0;
                    coproc_init(shp, pipes);
                }

#if USE_SPAWN
                if (com) {
                    parent = sh_ntfork(shp, t, com, &jobid, ntflag);
                } else {
                    parent = sh_fork(shp, type, &jobid);
                }

                if (parent < 0) {
#if USE_SPAWN
                    if (shp->comsub == 1 && usepipe && unpipe) sh_iounpipe(shp);
#endif  // USE_SPAWN
                    break;
                }
#else   // USE_SPAWN
                parent = sh_fork(shp, type, &jobid);
#endif  // USE_SPAWN
            }
            job.parent = parent;
            if (!job.parent) {
                forked_child(shp, t, flags, type, no_fork, com0, com, parent, topfd, vexi);
                __builtin_unreachable();
            }
            // This is the parent branch of fork. It may or may not wait for the child.
            if (pipejob == 2) {
                pipejob = 1;
                nlock--;
                job_unlock();
            }
            if (shp->subshell) shp->spid = parent;
            if (type & FPCL) sh_close(shp->inpipe[0]);
            if (type & (FCOOP | FAMP)) {
                shp->bckpid = parent;
            } else if (!(type & (FAMP | FPOU))) {
                if (!sh_isoption(shp, SH_MONITOR)) {
                    if (!(shp->sigflag[SIGINT] & (SH_SIGFAULT | SH_SIGOFF))) {
                        sh_sigtrap(shp, SIGINT);
                    }
                    shp->trapnote |= SH_SIGIGNORE;
                }
                if (shp->pipepid) {
                    shp->pipepid = parent;
                } else {
                    job_wait(parent);
                    if (parent == shp->spid) shp->spid = 0;
                }
                if (shp->topfd > topfd) sh_iorestore(shp, topfd, 0);
#if USE_SPAWN
                if (shp->vexp->cur > vexi) sh_vexrestore(shp, vexi);
#endif
                if (usepipe && tsetio && subdup) sh_iounpipe(shp);
                if (!sh_isoption(shp, SH_MONITOR)) {
                    shp->trapnote &= ~SH_SIGIGNORE;
                    if (shp->exitval == (SH_EXITSIG | SIGINT)) kill(getpid(), SIGINT);
                }
            }
            if (type & FAMP) {
                if (sh_isstate(shp, SH_PROFILE) || sh_isstate(shp, SH_INTERACTIVE)) {
                    /* print job number */
#ifdef JOBS
                    sfprintf(sfstderr, "[%d]\t%d\n", jobid, parent);
#else   // JOBS
                    sfprintf(sfstderr, "%d\n", parent);
#endif  // JOBS
                }
            }
            break;
        }
        case TSETIO: {
            // Don't create a new process, just save and restore io-streams.
            pid_t pid;
            int jmpval, waitall;
            int simple = (t->fork.forktre->tre.tretyp & COMMSK) == TCOM;
            checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
            if (shp->subshell) execflg = 0;
            sh_pushcontext(shp, buffp, SH_JMPIO);
            if (type & FPIN) {
                was_interactive = sh_isstate(shp, SH_INTERACTIVE);
                sh_offstate(shp, SH_INTERACTIVE);
                shp->pipepid = simple;
                sh_iosave(shp, 0, shp->topfd, NULL);
                sh_iorenumber(shp, shp->inpipe[0], 0);
                // if read end of pipe is a simple command treat as non-sharable to improve
                // performance.
                if (simple) sfset(sfstdin, SF_PUBLIC | SF_SHARE, 0);
                waitall = job.waitall;
                job.waitall = 0;
                pid = job.parent;
            } else {
                error_info.line = t->fork.forkline - shp->st.firstline;
            }
            jmpval = sigsetjmp(buffp->buff, 0);
            if (jmpval == 0) {
                if (shp->comsub) tsetio = 1;
                sh_redirect(shp, t->fork.forkio, execflg);
                (t->fork.forktre)->tre.tretyp |= t->tre.tretyp & FSHOWME;
                t = t->fork.forktre;
#if SHOPT_BASH
                if ((t->tre.tretyp & COMMSK) == TCOM && sh_isoption(shp, SH_BASH) &&
                    !sh_isoption(shp, SH_LASTPIPE)) {
                    Shnode_t *tt = stkalloc(shp->stk, sizeof(Shnode_t));
                    tt->par.partyp = type = TPAR;
                    tt->par.partre = (Shnode_t *)t;
                    t = tt;
                }
#endif  // SHOPT_BASH
                sh_exec(shp, t, flags & ~simple);
            } else {
                sfsync(shp->outpool);
            }
            sh_popcontext(shp, buffp);
            sh_iorestore(shp, buffp->topfd, jmpval);
#if USE_SPAWN
            if (shp->vexp->cur > vexi) sh_vexrestore(shp, buffp->vexi);
#endif
            if (buffp->olist) free_list(buffp->olist);
            if (type & FPIN) {
                job.waitall = waitall;
                type = shp->exitval;
                if (!(type & SH_EXITSIG)) {
                    // Wait for remainder of pipline.
                    if (shp->pipepid > 1 && shp->comsub != 1) {
                        job_wait(shp->pipepid);
                        type = shp->exitval;
                    } else {
                        job_wait(waitall ? pid : 0);
                    }
                    if (type || !sh_isoption(shp, SH_PIPEFAIL)) shp->exitval = type;
                }
                shp->pipepid = 0;
                shp->st.ioset = 0;
                if (simple && was_errexit) {
                    echeck = 1;
                    sh_onstate(shp, SH_ERREXIT);
                }
            }
            if (jmpval > SH_JMPIO) siglongjmp(shp->jmplist->buff, jmpval);
            break;
        }
        case TPAR: {
            echeck = 1;
            flags &= ~ARG_OPTIMIZE;
            if (!shp->subshell && !shp->st.trapcom[0] && !shp->st.trap[SH_ERRTRAP] &&
                (flags & sh_state(SH_NOFORK))) {
                char *savsig;
                int nsig, jmpval;
                checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
                shp->st.otrapcom = NULL;

                nsig = shp->st.trapmax;
                if (nsig > 0 || shp->st.trapcom[0]) {
                    int trapcom_size = (shp->st.trapmax + 1) * sizeof(char *);
                    savsig = malloc(trapcom_size);
                    memcpy(savsig, shp->st.trapcom, trapcom_size);
                    shp->st.otrapcom = (char **)savsig;
                }
                sh_sigreset(shp, 0);
                sh_pushcontext(shp, buffp, SH_JMPEXIT);
                jmpval = sigsetjmp(buffp->buff, 0);
                if (jmpval == 0) sh_exec(shp, t->par.partre, flags);
                sh_popcontext(shp, buffp);
                if (jmpval > SH_JMPEXIT) siglongjmp(shp->jmplist->buff, jmpval);
                if (shp->exitval > 256) shp->exitval -= 128;
                sh_done(shp, 0);
            } else if (((type = t->par.partre->tre.tretyp) & FAMP) && ((type & COMMSK) == TFORK)) {
                pid_t pid;
                sfsync(NULL);
                while ((pid = fork()) < 0) _sh_fork(shp, pid, 0, 0);
                if (pid == 0) {
                    sh_exec(shp, t->par.partre, flags);
                    shp->st.trapcom[0] = 0;
                    sh_offoption(shp, SH_INTERACTIVE);
                    sh_done(shp, 0);
                }
            } else {
                sh_subshell(shp, t->par.partre, flags, 0);
            }
            break;
        }
        case TFIL: {
            // This code sets up a pipe. All elements of the pipe are started by the parent. The
            // last element executes in current environment.
            int pvo[3];  // old pipe for multi-stage
            int pvn[3];  // current set up pipe
            int savepipe = pipejob;
            int savelock = nlock;
            int showme = t->tre.tretyp & FSHOWME;
            int n, waitall, savewaitall = job.waitall;
            int savejobid = job.curjobid;
            int *exitval = NULL, *saveexitval = job.exitval;
            pid_t savepgid = job.curpgid;
            job.exitval = NULL;
            job.curjobid = 0;
            if (shp->subshell) {
                sh_subtmpfile(shp);
                if (shp->comsub == 1 && !(shp->fdstatus[1] & IONOSEEK)) iousepipe(shp);
            }
            shp->inpipe = pvo;
            shp->outpipe = pvn;
            pvo[1] = -1;
            if (sh_isoption(shp, SH_PIPEFAIL)) {
                const Shnode_t *tn = t;
                job.waitall = 2;
                job.curpgid = 0;
                while ((tn = tn->lst.lstrit) && tn->tre.tretyp == TFIL) job.waitall++;
                exitval = job.exitval = stkalloc(shp->stk, job.waitall * sizeof(int));
                memset(exitval, 0, job.waitall * sizeof(int));
            } else {
                job.waitall |= !pipejob && sh_isstate(shp, SH_MONITOR);
            }
            job_lock();
            nlock++;
            do {
                // Create the pipe.
                sh_pipe(pvn);
                // Execute out part of pipe no wait.
                (t->lst.lstlef)->tre.tretyp |= showme;
                type = sh_exec(shp, t->lst.lstlef, errorflg);
                // Close out-part of pipe.
                sh_close(pvn[1]);
                pipejob = 1;
                // Save the pipe stream-ids.
                pvo[0] = pvn[0];
                // Pipeline all in one process group.
                t = t->lst.lstrit;
            }
            // Repeat until end of pipeline.
            while (!type && t->tre.tretyp == TFIL);
            shp->inpipe = pvn;
            shp->outpipe = NULL;
            pipejob = 2;
            waitall = job.waitall;
            job.waitall = 0;
            if (type == 0) {
                // Execute last element of pipeline in the current process.
                ((Shnode_t *)t)->tre.tretyp |= showme;
                sh_exec(shp, t, flags);
            } else {
                // Execution failure, close pipe.
                sh_pclose(pvn);
            }
            if (pipejob == 2) job_unlock();
            if ((pipejob = savepipe) && nlock < savelock) pipejob = 1;
            n = shp->exitval;
            job.waitall = waitall;
            if (job.waitall) {
                if (sh_isstate(shp, SH_MONITOR)) {
                    job_wait(0);
                } else {
                    shp->intrap++;
                    job_wait(0);
                    shp->intrap--;
                }
            }
            if (n == 0 && exitval) {
                while (exitval <= --job.exitval) {
                    if (*job.exitval) {
                        n = *job.exitval;
                        break;
                    }
                }
            }
            shp->exitval = n;
#ifdef SIGTSTP
            if (!pipejob && sh_isstate(shp, SH_MONITOR) && sh_isoption(shp, SH_INTERACTIVE)) {
                tcsetpgrp(JOBTTY, shp->gd->pid);
            }
#endif  // SIGTSTP
            job.curpgid = savepgid;
            job.exitval = saveexitval;
            job.waitall = savewaitall;
            job.curjobid = savejobid;
            break;
        }
        case TLST: {
            // A list of commands are executed here.
            do {
                sh_exec(shp, t->lst.lstlef, errorflg | (flags & ARG_OPTIMIZE));
                t = t->lst.lstrit;
            } while (t->tre.tretyp == TLST);
            sh_exec(shp, t, flags);
            break;
        }
        case TAND: {
            if (type & TTEST) skipexitset++;
            if (sh_exec(shp, t->lst.lstlef, (flags & ARG_OPTIMIZE)) == 0) {
                sh_exec(shp, t->lst.lstrit, flags);
            }
            break;
        }
        case TORF: {
            if (type & TTEST) skipexitset++;
            if (sh_exec(shp, t->lst.lstlef, flags & ARG_OPTIMIZE) != 0) {
                sh_exec(shp, t->lst.lstrit, flags);
            }
            break;
        }
        case TFOR: {  // for and select
            char **args;
            int nargs;
            Namval_t *np;
            int flag = errorflg | ARG_OPTIMIZE;
            struct dolnod *argsav = NULL;
            struct comnod *tp;
            char *trap;
            char *nullptr = NULL;
            int nameref, refresh = 1;
            char *av[5];
            int jmpval = shp->jmplist->mode;
            checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
            void *optlist = shp->optlist;
            shp->optlist = NULL;
            sh_tclear(shp, t->for_.fortre);
            sh_pushcontext(shp, buffp, jmpval);
            jmpval = sigsetjmp(buffp->buff, 0);
            if (jmpval) goto endfor;
            error_info.line = t->for_.forline - shp->st.firstline;
            if (!(tp = t->for_.forlst)) {
                args = shp->st.dolv + 1;
                nargs = shp->st.dolc;
                argsav = sh_arguse(shp);
            } else {
                args = sh_argbuild(shp, &argn, tp, 0);
                nargs = argn;
            }
            np = nv_open(t->for_.fornam, shp->var_tree, NV_NOARRAY | NV_VARNAME | NV_NOREF);
            nameref = nv_isref(np) != 0;
            shp->st.loopcnt++;
            cp = *args;
            while (cp && shp->st.execbrk == 0) {
                if (t->tre.tretyp & COMSCAN) {
                    char *val;
                    int save_prompt;
                    // Reuse.
                    if (refresh) {
                        sh_menu(shp, sfstderr, nargs, args);
                        refresh = 0;
                    }
                    save_prompt = shp->nextprompt;
                    shp->nextprompt = 3;
                    shp->timeout = 0;
                    shp->exitval =
                        sh_readline(shp, &nullptr, NULL, 0, 1, (size_t)0, 1000 * shp->st.tmout);
                    shp->nextprompt = save_prompt;
                    if (shp->exitval || sfeof(sfstdin) || sferror(sfstdin)) {
                        shp->exitval = 1;
                        break;
                    }
                    if (!(val = nv_getval(sh_scoped(shp, VAR_REPLY)))) {
                        continue;
                    } else {
                        if (*(cp = val) == 0) {
                            refresh++;
                            goto check;
                        }
                        while ((type = *cp++)) {
                            if (type < '0' && type > '9') break;
                        }
                        if (type != 0) {
                            type = nargs;
                        } else {
                            type = (int)strtol(val, NULL, 10) - 1;
                        }
                        if (type < 0 || type >= nargs) {
                            cp = "";
                        } else {
                            cp = args[type];
                        }
                    }
                }
                if (nameref) {
                    nv_offattr(np, NV_REF | NV_TABLE);
                } else if (nv_isattr(np, NV_ARRAY)) {
                    nv_putsub(np, NULL, 0L, 0);
                }
                nv_putval(np, cp, 0);
                if (nameref) {
                    nv_setref(np, NULL, NV_VARNAME);
                    nv_onattr(np, NV_TABLE);
                }
                trap = shp->st.trap[SH_DEBUGTRAP];
                if (trap) {
                    av[0] = (t->tre.tretyp & COMSCAN) ? "select" : "for";
                    av[1] = t->for_.fornam;
                    av[2] = "in";
                    av[3] = cp;
                    av[4] = 0;
                    sh_debug(shp, trap, NULL, NULL, av, 0);
                }
                sh_exec(shp, t->for_.fortre, flag);
                flag &= ~ARG_OPTIMIZE;
                if (t->tre.tretyp & COMSCAN) {
                    if ((cp = nv_getval(sh_scoped(shp, VAR_REPLY))) && *cp == 0) refresh++;
                } else {
                    cp = *++args;
                }
            check:
                if (shp->st.breakcnt < 0) shp->st.execbrk = (++shp->st.breakcnt != 0);
            }
            if (nameref) nv_offattr(np, NV_TABLE);
        endfor:
            sh_popcontext(shp, buffp);
            sh_tclear(shp, t->for_.fortre);
            sh_optclear(shp, optlist);
            if (jmpval) siglongjmp(shp->jmplist->buff, jmpval);
            if (shp->st.breakcnt > 0) shp->st.execbrk = (--shp->st.breakcnt != 0);
            shp->st.loopcnt--;
            sh_argfree(shp, argsav);
            nv_close(np);
            break;
        }
        case TWH: {  // while and until
            volatile int r = 0;
            int first = ARG_OPTIMIZE;
            Shnode_t *tt = t->wh.whtre;
            Sfio_t *iop = NULL;
            int savein;
            int jmpval = shp->jmplist->mode;
            checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
            void *optlist = shp->optlist;
            shp->optlist = NULL;
            sh_tclear(shp, t->wh.whtre);
            sh_tclear(shp, t->wh.dotre);
            sh_pushcontext(shp, buffp, jmpval);
            jmpval = sigsetjmp(buffp->buff, 0);
            if (jmpval) goto endwhile;
            if (type == TWH && tt->tre.tretyp == TCOM && !tt->com.comarg && tt->com.comio) {
                iop = openstream(shp, tt->com.comio, &savein);
                if (tt->com.comset) sh_setlist(shp, tt->com.comset, NV_IDENT | NV_ASSIGN, 0);
            }
            shp->st.loopcnt++;
            while (shp->st.execbrk == 0) {
                if (iop) {
                    if (!(shp->cur_line = sfgetr(iop, '\n', SF_STRING))) break;
                } else if ((sh_exec(shp, tt, first) == 0) != (type == TWH)) {
                    break;
                }
                r = sh_exec(shp, t->wh.dotre, first | errorflg);
                if (shp->st.breakcnt < 0) shp->st.execbrk = (++shp->st.breakcnt != 0);
                // This is for the arithmetic for.
                if (shp->st.execbrk == 0 && t->wh.whinc) {
                    sh_exec(shp, (Shnode_t *)t->wh.whinc, first);
                }
                first = 0;
                errorflg &= ~ARG_OPTIMIZE;
                shp->offsets[0] = -1;
                shp->offsets[1] = 0;
            }
        endwhile:
            sh_popcontext(shp, buffp);
            sh_tclear(shp, t->wh.whtre);
            sh_tclear(shp, t->wh.dotre);
            sh_optclear(shp, optlist);
            if (jmpval) siglongjmp(shp->jmplist->buff, jmpval);
            if (shp->st.breakcnt > 0) shp->st.execbrk = (--shp->st.breakcnt != 0);
            shp->st.loopcnt--;
            shp->exitval = r;
            if (iop) {
                int err = errno;
                sfclose(iop);
                while (close(0) < 0 && errno == EINTR) errno = err;
                int dup_fd = dup(savein);  // it has to return zero
                assert(dup_fd == 0);
                shp->cur_line = NULL;
            }
            break;
        }
        case TARITH: {  // (( expression ))
            char *trap;
            char *arg[4];
            error_info.line = t->ar.arline - shp->st.firstline;
            arg[0] = "((";
            if (!(t->ar.arexpr->argflag & ARG_RAW)) {
                arg[1] = sh_macpat(shp, t->ar.arexpr, (flags & ARG_OPTIMIZE) | ARG_ARITH);
            } else {
                arg[1] = t->ar.arexpr->argval;
            }
            arg[2] = "))";
            arg[3] = 0;
            trap = shp->st.trap[SH_DEBUGTRAP];
            if (trap) {
                sh_debug(shp, trap, NULL, NULL, arg, ARG_ARITH);
            }
            if (sh_isoption(shp, SH_XTRACE)) {
                sh_trace(shp, NULL, 0);
                sfprintf(sfstderr, "((%s))\n", arg[1]);
            }
            if (t->ar.arcomp) {
                shp->exitval = !arith_exec((Arith_t *)t->ar.arcomp);
            } else {
                shp->exitval = !sh_arith(shp, arg[1]);
            }
            break;
        }
        case TIF: {
            if (sh_exec(shp, t->if_.iftre, flags & ARG_OPTIMIZE) == 0) {
                sh_exec(shp, t->if_.thtre, flags);
            } else if (t->if_.eltre) {
                sh_exec(shp, t->if_.eltre, flags);
            } else {
                shp->exitval = 0;  // force zero exit for if-then-fi
            }
            break;
        }
        case TSW: {
            Shnode_t *tt = (Shnode_t *)t;
            char *trap, *r = sh_macpat(shp, tt->sw.swarg, flags & ARG_OPTIMIZE);
            error_info.line = t->sw.swline - shp->st.firstline;
            t = (Shnode_t *)(tt->sw.swlst);
            trap = shp->st.trap[SH_DEBUGTRAP];
            if (trap) {
                char *av[4];
                av[0] = "case";
                av[1] = r;
                av[2] = "in";
                av[3] = 0;
                sh_debug(shp, trap, NULL, NULL, av, 0);
            }
            while (t) {
                struct argnod *rex = (struct argnod *)t->reg.regptr;
                while (rex) {
                    char *s;
                    if (rex->argflag & ARG_MAC) {
                        s = sh_macpat(shp, rex, (flags & ARG_OPTIMIZE) | ARG_EXP | ARG_CASE);
                        while (*s == '\\' && s[1] == 0) s += 2;
                    } else {
                        s = rex->argval;
                    }
                    type = (rex->argflag & ARG_RAW);
                    if ((type && strcmp(r, s) == 0) || (!type && strmatch(r, s))) {
                        do {
                            sh_exec(shp, t->reg.regcom,
                                    (t->reg.regflag ? (flags & sh_state(SH_ERREXIT)) : flags));
                        } while (t->reg.regflag && (t = (Shnode_t *)t->reg.regnxt));
                        t = 0;
                        break;
                    } else {
                        rex = rex->argnxt.ap;
                    }
                }
                if (t) t = (Shnode_t *)t->reg.regnxt;
            }
            break;
        }
        case TTIME: {  // time the command
            const char *format = e_timeformat;
            struct timeval ta, tb, tm[3];
            struct timeval before_usr, before_sys, after_usr, after_sys;

            if (type != TTIME) {
                sh_exec(shp, t->par.partre, flags & ARG_OPTIMIZE);
                shp->exitval = !shp->exitval;
                break;
            }
            gettimeofday(&tb, NULL);
            get_cpu_times(&before_usr, &before_sys);
            if (t->par.partre) {
                if (shp->subshell && shp->comsub == 1) sh_subfork();
                long timer_on = sh_isstate(shp, SH_TIMING);
                job.waitall = 1;
                sh_onstate(shp, SH_TIMING);
                sh_exec(shp, t->par.partre, flags & ARG_OPTIMIZE);
                if (!timer_on) sh_offstate(shp, SH_TIMING);
                job.waitall = 0;
            }
            get_cpu_times(&after_usr, &after_sys);
            gettimeofday(&ta, NULL);
            timersub(&ta, &tb, &tm[TM_REAL_IDX]);  // calculate elapsed real-time
            timersub(&after_usr, &before_usr, &tm[TM_USR_IDX]);
            timersub(&after_sys, &before_sys, &tm[TM_SYS_IDX]);

            if (t->par.partre) {
                Namval_t *np = nv_open("TIMEFORMAT", shp->var_tree, NV_NOADD);
                if (np) {
                    format = nv_getval(np);
                    nv_close(np);
                }
                if (!format) format = e_timeformat;
            } else {
                format = strchr(format + 1, '\n') + 1;
            }
            if (format && *format) p_time(shp, sfstderr, sh_translate(format), tm);
            break;
        }
        case TFUN: {
            Namval_t *np = NULL;
            struct slnod *slp;
            char *fname = ((struct functnod *)t)->functnam;
            Namval_t *npv = 0, *mp;
            cp = strrchr(fname, '.');
            if (t->tre.tretyp == TNSPACE) {
                Namval_t *oldnspace = NULL;
                int offset = stktell(stkp);
                nvflag_t nvflags = NV_NOARRAY | NV_VARNAME;
                char *sp, *xp;
                sfputc(stkp, '.');
                sfputr(stkp, fname, 0);
                xp = stkptr(stkp, offset);
                for (sp = xp + 1; sp;) {
                    sp = strchr(sp, '.');
                    if (sp) *sp = 0;
                    np = nv_open(xp, shp->var_tree, nvflags);
                    if (sp) *sp++ = '.';
                }
                if (!nv_istable(np)) {
                    Dt_t *root = dtopen(&_Nvdisc, Dtoset);
                    dtuserdata(root, shp, 1);
                    nv_mount(np, NULL, root);
                    STORE_VT(np->nvalue, const_cp, Empty);
                    dtview(root, shp->var_base);
                }
                oldnspace = enter_namespace(shp, np);
                sh_exec(shp, t->for_.fortre, nvflags | sh_state(SH_ERREXIT));
                enter_namespace(shp, oldnspace);
                break;
            }
            // Look for discipline functions.
            error_info.line = t->funct.functline - shp->st.firstline;
            // Function names cannot be special builtin.
            if (cp || shp->prefix) {
                int offset = stktell(stkp);
                if (shp->prefix) {
                    cp = shp->prefix;
                    shp->prefix = NULL;
                    npv = nv_open(cp, shp->var_tree, NV_NOARRAY | NV_VARNAME);
                    shp->prefix = cp;
                    cp = fname;
                } else {
                    sfwrite(stkp, fname, cp++ - fname);
                    sfputc(stkp, 0);
                    npv = nv_open(stkptr(stkp, offset), shp->var_tree, NV_NOARRAY | NV_VARNAME);
                }
                offset = stktell(stkp);
                sfprintf(stkp, "%s.%s%c", nv_name(npv), cp, 0);
                fname = stkptr(stkp, offset);
            } else if ((mp = nv_search(fname, shp->bltin_tree, 0)) && nv_isattr(mp, BLT_SPC)) {
                errormsg(SH_DICT, ERROR_exit(1), e_badfun, fname);
                __builtin_unreachable();
            }
            if (shp->namespace && !shp->prefix && *fname != '.') {
                np = sh_fsearch(shp, fname, NV_ADD | NV_NOSCOPE);
            }
            if (!np) {
                np = nv_open(fname, sh_subfuntree(shp, true), NV_NOARRAY | NV_VARNAME | NV_NOSCOPE);
            }
            if (npv) {
                if (!shp->mktype) cp = nv_setdisc(npv, cp, np, (Namfun_t *)npv);
                if (!cp) {
                    errormsg(SH_DICT, ERROR_exit(1), e_baddisc, fname);
                    __builtin_unreachable();
                }
            }
            if (FETCH_VT(np->nvalue, rp)) {
                struct Ufunction *rp = FETCH_VT(np->nvalue, rp);
                slp = (struct slnod *)np->nvenv;
                sh_funstaks(slp->slchild, -1);
                stkclose(slp->slptr);
                if (rp->sdict) {
                    Namval_t *nq;
                    shp->last_root = rp->sdict;
                    for (mp = dtfirst(rp->sdict); mp; mp = nq) {
                        _nv_unset(mp, NV_RDONLY);
                        nq = dtnext(rp->sdict, mp);
                        nv_delete(mp, rp->sdict, 0);
                    }
                    dtclose(rp->sdict);
                    rp->sdict = NULL;
                }
                if (shp->funload) {
                    if (!shp->fpathdict) free(FETCH_VT(np->nvalue, rp));
                    STORE_VT(np->nvalue, rp, NULL);
                }
            }
            if (!FETCH_VT(np->nvalue, rp)) {
                struct Ufunction *rp =
                    calloc(1, sizeof(struct Ufunction) + (shp->funload ? sizeof(Dtlink_t) : 0));
                STORE_VT(np->nvalue, rp, rp);
            }
            if (t->funct.functstak) {
                static Dtdisc_t _Rpdisc = {.key = offsetof(struct Ufunction, fname),
                                           .size = -1,
                                           .link = sizeof(struct Ufunction)};
                struct functnod *fp;
                struct comnod *ac = t->funct.functargs;
                slp = t->funct.functstak;
                sh_funstaks(slp->slchild, 1);
                stklink(slp->slptr);
                np->nvenv = (Namval_t *)slp;
                nv_funtree(np) = (int *)(t->funct.functtre);
                FETCH_VT(np->nvalue, rp)->hoffset = t->funct.functloc;
                FETCH_VT(np->nvalue, rp)->lineno = t->funct.functline;
                FETCH_VT(np->nvalue, rp)->nspace = shp->namespace;
                FETCH_VT(np->nvalue, rp)->fname = NULL;
                FETCH_VT(np->nvalue, rp)->argv = ac ? ((struct dolnod *)ac->comarg)->dolval + 1 : 0;
                FETCH_VT(np->nvalue, rp)->argc = ac ? ((struct dolnod *)ac->comarg)->dolnum : 0;
                FETCH_VT(np->nvalue, rp)->fdict = shp->fun_tree;
                fp = (struct functnod *)(slp + 1);
                if (fp->functtyp == (TFUN | FAMP)) FETCH_VT(np->nvalue, rp)->fname = fp->functnam;
                nv_setsize(np, fp->functline);
                nv_offattr(np, NV_FPOSIX);
                if (shp->funload) {
                    struct Ufunction *rp = FETCH_VT(np->nvalue, rp);
                    rp->np = np;
                    if (!shp->fpathdict) shp->fpathdict = dtopen(&_Rpdisc, Dtobag);
                    if (shp->fpathdict) {
                        dtuserdata(shp->fpathdict, shp, 1);
                        dtinsert(shp->fpathdict, rp);
                    }
                }
            } else {
                _nv_unset(np, 0);
            }
            if (type & FPOSIX) {
                nv_onattr(np, NV_FUNCTION | NV_FPOSIX);
            } else {
                nv_onattr(np, NV_FUNCTION);
            }
            if (type & FPIN) nv_onattr(np, NV_FTMP);
            if (type & FOPTGET) nv_onattr(np, NV_OPTGET);
            if (type & FSHVALUE) nv_onattr(np, NV_SHVALUE);
            break;
        }
        case TTST: {  // new test compound command
            int n;
            char *left;
            int negate = (type & TNEGATE) != 0;
            if (type & TTEST) skipexitset++;
            error_info.line = t->tst.tstline - shp->st.firstline;
            echeck = 1;
            if ((type & TPAREN) == TPAREN) {
                sh_exec(shp, t->lst.lstlef, flags & ARG_OPTIMIZE);
                n = !shp->exitval;
            } else {
                bool traceon = 0;
                char *right;
                char *trap;
                char *argv[6];
                int savexit = shp->savexit;
                n = type >> TSHIFT;
                left = sh_macpat(shp, &(t->lst.lstlef->arg), flags & ARG_OPTIMIZE);
                if (type & TBINARY) {
                    right = sh_macpat(
                        shp, &(t->lst.lstrit->arg),
                        ((n == TEST_PEQ || n == TEST_PNE) ? ARG_EXP : 0) | (flags & ARG_OPTIMIZE));
                }
                shp->savexit = savexit;
                trap = shp->st.trap[SH_DEBUGTRAP];
                if (trap) {
                    argv[0] = (type & TNEGATE) ? ((char *)e_tstbegin) : "[[";
                }
                if (sh_isoption(shp, SH_XTRACE)) {
                    traceon = sh_trace(shp, NULL, 0);
                    sfwrite(sfstderr, e_tstbegin, (type & TNEGATE ? 5 : 3));
                }
                if (type & TUNARY) {
                    if (traceon) sfprintf(sfstderr, "-%c %s", n, sh_fmtq(left));
                    if (trap) {
                        char unop[3];
                        unop[0] = '-';
                        unop[1] = n;
                        unop[2] = 0;
                        argv[1] = unop;
                        argv[2] = left;
                        argv[3] = "]]";
                        argv[4] = 0;
                        sh_debug(shp, trap, NULL, NULL, argv, 0);
                    }
                    n = test_unop(shp, n, left);
                } else if (type & TBINARY) {
                    char *op;
                    int pattern = 0;
                    if (trap || traceon) op = (char *)(shtab_testops + (n & 037) - 1)->sh_name;
                    type >>= TSHIFT;
                    if (type == TEST_PEQ || type == TEST_PNE) pattern = ARG_EXP;
                    if (trap) {
                        argv[1] = left;
                        argv[2] = op;
                        argv[3] = right;
                        argv[4] = "]]";
                        argv[5] = 0;
                        sh_debug(shp, trap, NULL, NULL, argv, pattern);
                    }
                    n = test_binop(shp, n, left, right);
                    if (traceon) {
                        sfprintf(sfstderr, "%s %s ", sh_fmtq(left), op);
                        if (pattern) {
                            out_pattern(sfstderr, right, -1);
                        } else {
                            sfputr(sfstderr, sh_fmtq(right), -1);
                        }
                    }
                }
                if (traceon) sfwrite(sfstderr, e_tstend, 4);
            }
            shp->exitval = ((!n) ^ negate);
            if (!skipexitset) exitset(shp);
            break;
        }
        default: { break; }
    }

    if (procsub && *procsub) {
        pid_t pid;
        int exitval = shp->exitval;
        while ((pid = *procsub++)) job_wait(pid);
        shp->exitval = exitval;
    }
    if (shp->trapnote || (shp->exitval && sh_isstate(shp, SH_ERREXIT) && t && echeck)) {
        sh_chktrap(shp);
    }
    // Set $_.
    if (mainloop && com0) {
        // Store last argument here if it fits.
        static char lastarg[PATH_MAX];
        if (sh_isstate(shp, SH_FORKED)) sh_done(shp, 0);
        if (shp->lastarg != lastarg && shp->lastarg) free(shp->lastarg);
        // Although this node is marked as NV_NOFREE, it should get free'd above when $_ is
        // reset
        nv_onattr(VAR_underscore, NV_NOFREE);
        if (strlen(comn) < sizeof(lastarg)) {
            shp->lastarg = strcpy(lastarg, comn);
        } else {
            shp->lastarg = strdup(comn);
        }
    }
    if (!skipexitset) exitset(shp);
    if (!(flags & ARG_OPTIMIZE)) {
        if (sav != stkptr(stkp, 0)) {
            stkset(stkp, sav, 0);
        } else if (stktell(stkp)) {
            stkseek(stkp, 0);
        }
    }
    if (shp->trapnote & SH_SIGSET) sh_exit(shp, SH_EXITSIG | shp->lastsig);
    if (was_interactive) sh_onstate(shp, SH_INTERACTIVE);
    if (was_monitor && sh_isoption(shp, SH_MONITOR)) sh_onstate(shp, SH_MONITOR);
    if (was_errexit) sh_onstate(shp, SH_ERREXIT);

    return shp->exitval;
}

//
// Print out the command line if set -x is on.
//
bool sh_trace(Shell_t *shp, char *argv[], int nl) {
    char *cp;
    int bracket = 0;
    int decl = (nl & 2);
    nl &= ~2;
    if (sh_isoption(shp, SH_XTRACE)) {
        // Make this trace atomic.
        sfset(sfstderr, SF_SHARE | SF_PUBLIC, 0);
        if (!(cp = nv_getval(sh_scoped(shp, VAR_PS4)))) {
            cp = "+ ";
        } else {
            shp->intrace = 1;
            sh_offoption(shp, SH_XTRACE);
            cp = sh_mactry(shp, cp);
            sh_onoption(shp, SH_XTRACE);
            shp->intrace = 0;
        }
        if (*cp) sfputr(sfstderr, cp, -1);
        if (argv) {
            char *argv0 = *argv;
            nl = (nl ? '\n' : -1);
            // Don't quote [ and [[.
            cp = argv[0];
            if (*cp == '[' && (!cp[1] || (!cp[2] && cp[1] == '['))) {
                sfputr(sfstderr, cp, *++argv ? ' ' : nl);
                bracket = 1;
            }
            while ((cp = *argv++)) {
                if (bracket == 0 || *argv || *cp != ']') cp = sh_fmtq(cp);
                if (decl && shp->prefix && cp != argv0 && *cp != '-') {
                    if (*cp == '.' && cp[1] == 0) {
                        cp = shp->prefix;
                    } else {
                        sfputr(sfstderr, shp->prefix, '.');
                    }
                }
                sfputr(sfstderr, cp, *argv ? ' ' : nl);
            }
            sfset(sfstderr, SF_SHARE | SF_PUBLIC, 1);
        }
        return true;
    }
    return false;
}

//
// This routine creates a subshell by calling fork().
// If ((flags&COMASK)==TCOM), then fork() is permitted.
// If fork fails, the shell sleeps for exponentially longer periods
//   and tries again until a limit is reached.
// SH_FORKLIM is the max period between forks - power of 2 usually.
// Currently shell tries after 2,4,8,16, and 32 seconds and then quits
// Failures cause the routine to error exit.
// Parent links to here-documents are removed by the child.
// Traps are reset by the child.
// The process-id of the child is returned to the parent, 0 to the child.
//
static_fn void timed_out(void *handle) {
    UNUSED(handle);
    timeout = NULL;
}

//
// Called by parent and child after fork by sh_fork().
//
pid_t _sh_fork(Shell_t *shp, pid_t parent, int flags, int *jobid) {
    static long forkcnt = 1000L;
    pid_t curpgid = job.curpgid;
    pid_t postid = (flags & FAMP) ? 0 : curpgid;
    int sig, nochild;

    if (parent < 0) {
        sh_sigcheck(shp);
        forkcnt *= 2;
        if (forkcnt > 1000L * SH_FORKLIM) {
            forkcnt = 1000L;
            errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_nofork);
            __builtin_unreachable();
        }
        timeout = sh_timeradd(forkcnt, 0, timed_out, NULL);
        nochild = job_wait((pid_t)1);
        if (timeout) {
            if (nochild) {
                pause();
            } else if (forkcnt > 1000L) {
                forkcnt /= 2;
            }
            timerdel(timeout);
            timeout = NULL;
        }
        return -1;
    }
    forkcnt = 1000L;
    if (parent) {
        int myjob, waitall = job.waitall;
        shp->gd->nforks++;
        if (job.toclear) job_clear(shp);
        job.waitall = waitall;
#ifdef JOBS
        // First process defines process group.
        if (sh_isstate(shp, SH_MONITOR)) {
            // errno==EPERM means that an earlier processes completed.  Make parent the job group
            // id.
            if (postid == 0) job.curpgid = job.jobcontrol ? parent : getpid();
            if (job.jobcontrol || (flags & FAMP)) {
                if (setpgid(parent, job.curpgid) < 0 && errno == EPERM) setpgid(parent, parent);
            }
        }
#endif  // JOBS
        if (!sh_isstate(shp, SH_MONITOR) && job.waitall && postid == 0) job.curpgid = parent;
        if (flags & FCOOP) shp->cpid = parent;
        if (!postid && job.curjobid && (flags & FPOU)) postid = job.curpgid;
        if (!postid && (flags & (FAMP | FINT)) == (FAMP | FINT)) postid = 1;
        myjob = job_post(shp, parent, postid);
        if (job.waitall && (flags & FPOU)) {
            if (!job.curjobid) job.curjobid = myjob;
            if (job.exitval) job.exitval++;
        }
        if (flags & FAMP) job.curpgid = curpgid;
        if (jobid) *jobid = myjob;
        if (shp->comsub == 1 && usepipe) {
            if (!tsetio || !subdup) {
#if USE_SPAWN
                if (shp->vexp->cur > restorevex) sh_vexrestore(shp, restorevex);
#endif
                if (shp->topfd > restorefd) sh_iorestore(shp, restorefd, 0);
                sh_iounpipe(shp);
            }
        }
        return parent;
    }
    shp->outpipepid = ((flags & FPOU) ? getpid() : 0);
    // This is the child process.
    if (shp->trapnote & SH_SIGTERM) sh_exit(shp, SH_EXITSIG | SIGTERM);
    shp->gd->nforks = 0;
    timerdel(NULL);
#ifdef JOBS
    if (!job.jobcontrol && !(flags & FAMP)) sh_offstate(shp, SH_MONITOR);
    if (sh_isstate(shp, SH_MONITOR)) {
        parent = getpid();
        if (postid == 0) job.curpgid = parent;
        while (setpgid(0, job.curpgid) < 0 && job.curpgid != parent) job.curpgid = parent;
#ifdef SIGTSTP
        if (job.curpgid == parent && !(flags & FAMP)) tcsetpgrp(job.fd, job.curpgid);
#endif  // SIGTSTP
    }
#ifdef SIGTSTP
    if (job.jobcontrol) {
        sh_signal(SIGTTIN, (sh_sigfun_t)(SIG_DFL));
        sh_signal(SIGTTOU, (sh_sigfun_t)(SIG_DFL));
        sh_signal(SIGTSTP, (sh_sigfun_t)(SIG_DFL));
    }
#endif  // SIGTSTP
    job.jobcontrol = 0;
#endif  // JOBS
    job.toclear = 1;
    shp->login_sh = 0;
    sh_offoption(shp, SH_LOGIN_SHELL);
    sh_onstate(shp, SH_FORKED);
    sh_onstate(shp, SH_NOLOG);
    if (shp->fn_reset) shp->fn_depth = shp->fn_reset = 0;
    // Reset remaining signals to parent except for those `lost' by trap.
    if (!(flags & FSHOWME)) sh_sigreset(shp, 2);
    shp->subshell = 0;
    shp->comsub = 0;
    shp->spid = 0;
    if ((flags & FAMP) && shp->coutpipe > 1) sh_close(shp->coutpipe);
    sig = shp->savesig;
    shp->savesig = 0;
    if (sig > 0) kill(getpid(), sig);
    sh_sigcheck(shp);
    usepipe = 0;
    return 0;
}

pid_t sh_fork(Shell_t *shp, int flags, int *jobid) {
    pid_t parent;
    sigset_t set, oset;

    if (!shp->pathlist) path_get(shp, "");
    sfsync(NULL);
    shp->trapnote &= ~SH_SIGTERM;
    job_fork(-1);
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, &oset);
    while (_sh_fork(shp, parent = fork(), flags, jobid) < 0) {
        ;  // empty loop
    }
    sh_stats(STAT_FORKS);
#if USE_SPAWN
    if (parent == 0 && shp->vex) {
        spawnvex_apply(shp->vex, 0, 0);
        spawnvex_apply(shp->vexp, 0, SPAWN_RESET);
    }
#endif  // USE_SPAWN
    sigprocmask(SIG_SETMASK, &oset, NULL);
    job_fork(parent);
    return parent;
}

struct Tdata {
    Shell_t *sh;
    Namval_t *tp;
    void *extra[2];
};

//
// Add exports from previous scope to the new scope.
//
static_fn void local_exports(Namval_t *np, void *data) {
    Shell_t *shp = ((struct Tdata *)data)->sh;
    Namval_t *mp;
    char *cp;

    if (nv_isarray(np)) nv_putsub(np, NULL, 0, 0);
    cp = nv_getval(np);
    if (cp && (mp = nv_search(nv_name(np), shp->var_tree, NV_ADD | NV_NOSCOPE)) && nv_isnull(mp)) {
        nv_putval(mp, cp, 0);
        nv_setattr(mp, np->nvflag);
    }
}

//
// This routine executes .sh.math functions from within ((...))).
//
Sfdouble_t sh_mathfun(Shell_t *shp, void *fp, int nargs, Sfdouble_t *arg) {
    // The initialization of this var isn't really needed because it is indirectly modified by the
    // sh_funscope() call below. But this stops lint tools from complaining that we're returning
    // garbage from an uninitialized var.
    Sfdouble_t d = 0.0;
    Namval_t node, *mp, *nref[9], **nr = nref;
    char *argv[2];
    struct funenv funenv;
    int i;

    Namval_t *np = fp;
    funenv.node = np;
    funenv.nref = nref;
    funenv.env = NULL;
    memcpy(&node, VAR_sh_value, sizeof(node));
    VAR_sh_value->nvfun = NULL;
    VAR_sh_value->nvenv = NULL;
    nv_setattr(VAR_sh_value, NV_LDOUBLE | NV_NOFREE);
    STORE_VT(VAR_sh_value->nvalue, sfdoublep, NULL);
    for (i = 0; i < nargs; i++) {
        *nr++ = mp = nv_namptr(shp->mathnodes, i);
        STORE_VT(mp->nvalue, sfdoublep, arg++);
    }
    *nr = 0;
    STORE_VT(VAR_sh_value->nvalue, sfdoublep, &d);
    argv[0] = np->nvname;
    argv[1] = NULL;
    sh_funscope(shp, 1, argv, 0, &funenv, 0);
    while ((mp = *nr++)) STORE_VT(mp->nvalue, sfdoublep, NULL);
    VAR_sh_value->nvfun = node.nvfun;
    nv_setattr(VAR_sh_value, node.nvflag);
    VAR_sh_value->nvenv = node.nvenv;
    STORE_VT(VAR_sh_value->nvalue, sfdoublep, FETCH_VT(node.nvalue, sfdoublep));
    return d;
}

static_fn void sh_funct(Shell_t *shp, Namval_t *np, int argn, char *argv[], struct argnod *envlist,
                        int execflg) {
    struct funenv fun;
    char *fname = nv_getval(VAR_sh_fun);
    struct Level *lp = (struct Level *)(VAR_sh_level->nvfun);
    int level, pipepid = shp->pipepid;

    shp->pipepid = 0;
    sh_stats(STAT_FUNCT);
    if (!lp->namfun.disc) lp = init_level(shp, 0);
    if ((struct sh_scoped *)shp->topscope != shp->st.self) sh_setscope(shp, shp->topscope);
    level = lp->maxlevel = shp->dot_depth + shp->fn_depth + 1;
    STORE_VT(VAR_sh_level->nvalue, i16, lp->maxlevel);
    shp->st.lineno = error_info.line;
    FETCH_VT(np->nvalue, rp)->running += 2;
    if (nv_isattr(np, NV_FPOSIX) && !sh_isoption(shp, SH_BASH)) {
        int loopcnt = shp->st.loopcnt;
        shp->posix_fun = np;
        shp->st.funname = nv_name(np);
        shp->last_root = nv_dict(VAR_sh);
        nv_putval(VAR_sh_fun, nv_name(np), NV_NOFREE);
        opt_info.index = opt_info.offset = 0;
        error_info.errors = 0;
        shp->st.loopcnt = 0;
        if (argn == 1) {
            // The vast majority of the time this is called with argn == 1 so optimize that case.
            // The first arg is ignored and could be anything.
            char *source_argv[3] = {"sh_funct", argv[0], NULL};
            b_source(2, source_argv, &shp->bltindata);
        } else {
            char **source_argv = malloc((argn + 2) * sizeof(char *));
            source_argv[0] = "sh_funct";
            source_argv[argn + 1] = NULL;
            memcpy(source_argv + 1, argv, argn * sizeof(char *));
            b_source(argn + 1, source_argv, &shp->bltindata);
            free(source_argv);
        }
        shp->st.loopcnt = loopcnt;
    } else {
        fun.env = envlist;
        fun.node = np;
        fun.nref = NULL;
        sh_funscope(shp, argn, argv, 0, &fun, execflg);
    }
    if (level-- != nv_getnum(VAR_sh_level)) {
        Shscope_t *sp = sh_getscope(shp, 0, SEEK_END);
        sh_setscope(shp, sp);
    }
    lp->maxlevel = level;
    STORE_VT(VAR_sh_level->nvalue, i16, lp->maxlevel);
    shp->last_root = nv_dict(VAR_sh);
    nv_putval(VAR_sh_fun, fname, NV_NOFREE);
    nv_putval(VAR_sh_file, shp->st.filename, NV_NOFREE);
    shp->pipepid = pipepid;
    FETCH_VT(np->nvalue, rp)->running -= 2;
    if (FETCH_VT(np->nvalue, rp) && FETCH_VT(np->nvalue, rp)->running == 1) {
        FETCH_VT(np->nvalue, rp)->running = 0;
        _nv_unset(np, NV_RDONLY);
    }
}

//
// External interface to execute a function without arguments. <np> is the function node. If <nq> is
// not-null, then sh.name and sh.subscript will be set.
//
int sh_fun(Shell_t *shp, Namval_t *np, Namval_t *nq, char *argv[]) {
    int offset;
    char *base;
    Namval_t node;
    struct Namref nr;
    nvflag_t mode;
    char *prefix = shp->prefix;
    int n = 0;
    char *av[3];

    Fcin_t save;
    fcsave(&save);
    offset = stktell(shp->stk);
    if (offset > 0) base = stkfreeze(shp->stk, 0);
    shp->prefix = NULL;
    if (!argv) {
        argv = av + 1;
        argv[1] = 0;
    }
    argv[0] = nv_name(np);
    while (argv[n]) n++;
    if (nq) mode = set_instance(shp, nq, &node, &nr);
    if (is_abuiltin(np)) {
        int jmpval;
        checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
        Shbltin_t *bp = &shp->bltindata;
        sh_pushcontext(shp, buffp, SH_JMPCMD);
        jmpval = sigsetjmp(buffp->buff, 1);
        if (jmpval == 0) {
            bp->bnode = np;
            bp->ptr = nv_context(np);
            errorpush(&buffp->err, 0);
            error_info.id = argv[0];
            opt_info.index = opt_info.offset = 0;
            opt_info.disc = NULL;
            shp->exitval = 0;
            shp->exitval = (funptr(np))(n, argv, bp);
        }
        sh_popcontext(shp, buffp);
        if (jmpval > SH_JMPCMD) siglongjmp(shp->jmplist->buff, jmpval);
    } else {
        sh_funct(shp, np, n, argv, NULL, sh_isstate(shp, SH_ERREXIT));
    }
    if (nq) unset_instance(nq, &node, &nr, mode);
    fcrestore(&save);
    if (offset > 0) stkset(shp->stk, base, offset);
    shp->prefix = prefix;
    return shp->exitval;
}

//
// Set up pipe for cooperating process.
//
static_fn void coproc_init(Shell_t *shp, int pipes[]) {
    int outfd;
    if (shp->coutpipe >= 0 && shp->cpid) {
        errormsg(SH_DICT, ERROR_exit(1), e_copexists);
        __builtin_unreachable();
    }
    shp->cpid = 0;
    if (shp->cpipe[0] <= 0 || shp->cpipe[1] <= 0) {
        // First co-process.
        sh_pclose(shp->cpipe);
        sh_pipe(shp->cpipe);
        outfd = shp->cpipe[1];
        if (outfd < 10) {
            int fd = sh_fcntl(shp->cpipe[1], F_DUPFD_CLOEXEC, 10);
            if (fd >= 10) {
                shp->fdstatus[fd] = (shp->fdstatus[outfd] & ~IOCLEX);
                sh_close(outfd);
                shp->fdstatus[outfd] = IOCLOSE;
                shp->cpipe[1] = fd;
            }
        }
        shp->fdptrs[shp->cpipe[0]] = shp->cpipe;
    }
    shp->outpipe = shp->cpipe;
    sh_pipe(shp->inpipe = pipes);
    shp->coutpipe = shp->inpipe[1];
    shp->fdptrs[shp->coutpipe] = &shp->coutpipe;
}

#if USE_SPAWN

static_fn void sigreset(Shell_t *shp, int mode) {
    char *trap;
    int sig = shp->st.trapmax;

    while (sig-- > 0) {
#ifdef SIGCLD
#if SIGCLD != SIGCHLD
        if (sig == SIGCLD) continue;
#endif
#endif
        if (sig == SIGCHLD) continue;
        if (shp->sigflag[sig] & SH_SIGOFF) return;
        trap = shp->st.trapcom[sig];
        if (trap && *trap == 0) {
            sh_signal(sig, mode ? (sh_sigfun_t)sh_fault : (sh_sigfun_t)(SIG_IGN));
        }
    }
}

//
// A combined fork/exec for systems with slow or non-existent fork().
//
static_fn pid_t sh_ntfork(Shell_t *shp, const Shnode_t *t, char *argv[], int *jobid, int flag) {
    static pid_t spawnpid;
    static int savetype;
    static int savejobid;
    checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
    int otype = 0, jmpval, jobfork = 0, lineno = shp->st.firstline;
    volatile int scope = 0, sigwasset = 0;
    char **arge, *path;
    volatile pid_t grp = 0;
    Pathcomp_t *pp;

    if (flag) {
        otype = savetype;
        savetype = 0;
    }
    sh_pushcontext(shp, buffp, SH_JMPCMD);
    errorpush(&buffp->err, ERROR_SILENT);
    jmpval = sigsetjmp(buffp->buff, 0);
    if (jmpval == 0) {
        if ((otype & FINT) && !sh_isstate(shp, SH_MONITOR)) {
            sh_signal(SIGQUIT, (sh_sigfun_t)(SIG_IGN));
            sh_signal(SIGINT, (sh_sigfun_t)(SIG_IGN));
        }
        spawnpid = -1;
        if (t->com.comio) {
            shp->errorfd = error_info.fd;
#if 1
            sh_redirect(shp, t->com.comio, io_usevex(t->com.comio));
#else
            sh_redirect(shp, t->com.comio, 0);
#endif
        }
        error_info.id = *argv;
        if (t->com.comset) {
            scope++;
            sh_scope(shp, t->com.comset, 0);
        }
        if (!strchr(path = argv[0], '/')) {
            Namval_t *np;
            np = nv_search(path, shp->track_tree, 0);
            if (np && !nv_isattr(np, NV_NOALIAS) && FETCH_VT(np->nvalue, const_cp)) {
                path = nv_getval(np);
            } else if (path_absolute(shp, path, NULL)) {
                path = stkptr(shp->stk, PATH_OFFSET);
                stkfreeze(shp->stk, 0);
            } else {
                pp = path_get(shp, path);
                while (pp) {
                    if (pp->len == 1 && *pp->name == '.') break;
                    pp = pp->next;
                }
                if (!pp) path = 0;
            }
        } else if (sh_isoption(shp, SH_RESTRICTED)) {
            errormsg(SH_DICT, ERROR_exit(1), e_restricted, path);
            __builtin_unreachable();
        }
        if (!path) {
            spawnpid = -1;
            goto fail;
        }
        arge = sh_envgen(shp);
        // Restore firstline in case LINENO was exported.
        shp->st.firstline = lineno;
        shp->exitval = 0;
#ifdef JOBS
        if (sh_isstate(shp, SH_MONITOR) && (job.jobcontrol || (otype & FAMP))) {
            if ((otype & FAMP) || job.curpgid == 0) {
                grp = 1;
            } else {
                grp = job.curpgid;
            }
        }
#endif  // JOBS

        sfsync(NULL);
        sigreset(shp, 0);  // set signals to ignore
        sigwasset++;
        // Find first path that has a library component.
        for (pp = path_get(shp, argv[0]); pp && !pp->lib; pp = pp->next) {
            ;  // empty loop
        }
        job_fork(-1);
        jobfork = 1;
        spawnpid = path_spawn(shp, path, argv, arge, pp, (grp << 1) | 1);
        if (spawnpid < 0 && errno == ENOEXEC) {
            char *devfd;
            int fd = open(path, O_RDONLY);
            argv[-1] = argv[0];
            argv[0] = path;
            if (fd >= 0) {
                struct stat statb;
                sfprintf(shp->strbuf, "/dev/fd/%d", fd);
                if (stat(devfd = sfstruse(shp->strbuf), &statb) >= 0) argv[0] = devfd;
            }
            if (!shp->gd->shpath) shp->gd->shpath = pathshell();
            spawnpid = path_spawn(shp, shp->gd->shpath, &argv[-1], arge, pp, (grp << 1) | 1);
            if (fd >= 0) sh_close(fd);
            argv[0] = argv[-1];
        }
    fail:
        if (jobfork && spawnpid < 0) job_fork(0);
        if (spawnpid < 0) {
            switch (errno = shp->path_err) {
                case ENOENT: {
                    errormsg(SH_DICT, ERROR_system(ERROR_NOENT), e_found + 4);
                    __builtin_unreachable();
                }
                default: {
                    errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_exec + 4);
                    __builtin_unreachable();
                }
            }
        }
    } else {
        exitset(shp);
    }
    sh_popcontext(shp, buffp);
    if (buffp->olist) free_list(buffp->olist);
    if (sigwasset) sigreset(shp, 1); /* restore ignored signals */
    if (scope) {
        sh_unscope(shp);
        if (jmpval == SH_JMPSCRIPT) {
            sh_setlist(shp, t->com.comset, NV_EXPORT | NV_IDENT | NV_ASSIGN, 0);
        }
    }
    if (t->com.comio && (jmpval || spawnpid <= 0)) {
        sh_iorestore(shp, buffp->topfd, jmpval);
#if USE_SPAWN
        if (shp->vexp->cur > buffp->vexi) sh_vexrestore(shp, buffp->vexi);
#endif
    }
    if (jmpval > SH_JMPCMD) siglongjmp(shp->jmplist->buff, jmpval);
    if (spawnpid > 0) {
        _sh_fork(shp, spawnpid, otype, jobid);
        job_fork(spawnpid);
#ifdef JOBS
        if (grp == 1) job.curpgid = spawnpid;
#ifdef SIGTSTP
        if (grp > 0 && !(otype & FAMP)) {
            while (tcsetpgrp(job.fd, job.curpgid) < 0 && job.curpgid != spawnpid) {
                job.curpgid = spawnpid;
            }
        }
#endif  // SIGTSTP
#endif  // JOBS
        savejobid = *jobid;
        if (otype) return 0;
    }
    return spawnpid;
}

#endif  // USE_SPAWN

//
// This routine is used to execute the given function <fun> in a new scope. If <fun> is NULL, then
// arg points to a structure containing a pointer to a function that will be executed in the current
// environment.
//
int sh_funscope(Shell_t *shp, int argn, char *argv[], int (*fun)(void *), void *arg, int execflg) {
    UNUSED(argn);
    char *trap;
    int nsig;
    struct dolnod *argsav = NULL;
    struct dolnod *saveargfor;
    struct sh_scoped savst, *prevscope;
    struct argnod *envlist = NULL;
    int isig, jmpval;
    volatile int r = 0;
    int n;
    char **savsig;
    struct funenv *fp = NULL;
    checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
    Namval_t *nspace = shp->namespace;
    Dt_t *last_root = shp->last_root;
    Shopt_t options;

    options = shp->options;
    if (shp->fn_depth == 0) {
        shp->glob_options = shp->options;
    } else {
        shp->options = shp->glob_options;
    }
    prevscope = shp->st.self;
    *prevscope = shp->st;
    sh_offoption(shp, SH_ERREXIT);
    shp->st.prevst = prevscope;
    memset(&savst, 0, sizeof(savst));
    shp->st.self = &savst;
    shp->topscope = (Shscope_t *)shp->st.self;
    shp->st.opterror = shp->st.optchar = 0;
    shp->st.optindex = 1;
    shp->st.loopcnt = 0;
    if (!fun) {
        fp = (struct funenv *)arg;
        shp->st.real_fun = FETCH_VT((fp->node)->nvalue, rp);
        envlist = fp->env;
    }
    prevscope->save_tree = shp->var_tree;
    n = dtvnext(prevscope->save_tree) != (shp->namespace ? shp->var_base : 0);
    sh_scope(shp, envlist, 1);
    if (n) {
        struct Tdata tdata;
        memset(&tdata, 0, sizeof(tdata));
        tdata.sh = shp;
        // Eliminate parent scope.
        nv_scan(prevscope->save_tree, local_exports, &tdata, NV_EXPORT, NV_EXPORT | NV_NOSCOPE);
    }
    shp->st.save_tree = shp->var_tree;
    if (!fun) {
        if (nv_isattr(fp->node, NV_TAGGED)) {
            sh_onoption(shp, SH_XTRACE);
        } else {
            sh_offoption(shp, SH_XTRACE);
        }
    }
    shp->st.cmdname = argv[0];
    // Save trap table.
    nsig = shp->st.trapmax;
    if (nsig > 0 || shp->st.trapcom[0]) {
        savsig = malloc(nsig * sizeof(char *));
        // Contents of shp->st.trapcom may change
        for (isig = 0; isig < nsig; ++isig) {
            savsig[isig] = shp->st.trapcom[isig] ? strdup(shp->st.trapcom[isig]) : NULL;
        }
    }
    sh_sigreset(shp, 0);
    argsav = sh_argnew(shp, argv, &saveargfor);
    sh_pushcontext(shp, buffp, SH_JMPFUN);
    errorpush(&buffp->err, 0);
    error_info.id = argv[0];
    shp->st.var_local = shp->var_tree;
    if (!fun) {
        shp->st.filename = FETCH_VT(fp->node->nvalue, rp)->fname;
        shp->st.funname = nv_name(fp->node);
        shp->last_root = nv_dict(VAR_sh);
        nv_putval(VAR_sh_file, shp->st.filename, NV_NOFREE);
        nv_putval(VAR_sh_fun, shp->st.funname, NV_NOFREE);
    }
    if ((execflg & sh_state(SH_NOFORK))) shp->end_fn = 1;
    jmpval = sigsetjmp(buffp->buff, 0);
    if (jmpval == 0) {
        if (shp->fn_depth++ > MAXDEPTH) {
            shp->toomany = 1;
            siglongjmp(shp->jmplist->buff, SH_JMPERRFN);
        } else if (fun) {
            r = (*fun)(arg);
        } else {
            char **args = shp->st.real_fun->argv;
            Namval_t *np, *nq, **nref;
            nref = fp->nref;
            if (nref) {
                shp->last_root = NULL;
                for (r = 0; args[r]; r++) {
                    np = nv_search(args[r], shp->var_tree, NV_NOSCOPE | NV_ADD);
                    if (!np || !*nref) continue;

                    nq = *nref++;
                    struct Namref *nrp = calloc(1, sizeof(struct Namref));
                    STORE_VT(np->nvalue, nrp, nrp);
                    if (nv_isattr(nq, NV_LDOUBLE) == NV_LDOUBLE) {
                        nrp->np = nq;
                    } else {
                        // TODO: Figure out where the assignment to nq->nvalue.ldp is occurring.
                        // This used to be the sole use of `pointerof()` which simply did what
                        // we're now doing. But when spelled out like this it is obvious there
                        // is a problem.
                        nrp->np = (void *)((uintptr_t)*FETCH_VT(nq->nvalue, sfdoublep));
                        nv_onattr(nq, NV_LDOUBLE);
                    }
                    nv_onattr(np, NV_REF | NV_NOFREE);
                }
            }
            sh_exec(shp, (Shnode_t *)(nv_funtree((fp->node))), execflg | SH_ERREXIT);
            r = shp->exitval;
        }
    }
    if (shp->topscope != (Shscope_t *)shp->st.self) sh_setscope(shp, shp->topscope);
    if (--shp->fn_depth == 1 && jmpval == SH_JMPERRFN) {
        errormsg(SH_DICT, ERROR_exit(1), e_toodeep, argv[0]);
        __builtin_unreachable();
    }
    sh_popcontext(shp, buffp);
    sh_unscope(shp);
    shp->namespace = nspace;
    shp->var_tree = prevscope->save_tree;
    sh_argreset(shp, argsav, saveargfor);
    trap = shp->st.trapcom[0];
    shp->st.trapcom[0] = 0;
    sh_sigreset(shp, 1);
    shp->st = *prevscope;
    shp->topscope = (Shscope_t *)prevscope;
    nv_getval(sh_scoped(shp, VAR_IFS));
    shp->end_fn = 0;
    if (nsig) {
        for (isig = 0; isig < nsig; ++isig) {
            if (shp->st.trapcom[isig]) {
                free(shp->st.trapcom[isig]);
            }
        }
        memcpy(shp->st.trapcom, savsig, nsig * sizeof(char *));
        free(savsig);
    }
    shp->trapnote = 0;
    shp->options = options;
    shp->last_root = last_root;
    if (jmpval == SH_JMPSUB) siglongjmp(shp->jmplist->buff, jmpval);
    if (trap) {
        sh_trap(shp, trap, 0);
        free(trap);
    }
    if (jmpval) r = shp->exitval;
    if (!sh_isstate(shp, SH_IOPROMPT) && r > SH_EXITSIG &&
        ((r & SH_EXITMASK) == SIGINT || ((r & SH_EXITMASK) == SIGQUIT))) {
        kill(getpid(), r & SH_EXITMASK);
    }
    if (jmpval > SH_JMPFUN) {
        sh_chktrap(shp);
        siglongjmp(shp->jmplist->buff, jmpval);
    }
    return r;
}

//
// Given stream <iop> compile and execute.
//
int sh_eval(Shell_t *shp, Sfio_t *iop, int mode) {
    Shnode_t *t;
    struct slnod *saveslp = shp->st.staklist;
    int jmpval;
    checkpt_t *pp = shp->jmplist;
    checkpt_t *buffp = stkalloc(shp->stk, sizeof(checkpt_t));
    static Sfio_t *io_save;
    volatile bool traceon = false;
    volatile int lineno = 0;
    int binscript = shp->binscript;
    char comsub = shp->comsub;
    Sfio_t *iosaved = io_save;

    io_save = iop;  // preserve correct value across longjmp
    shp->binscript = 0;
    shp->comsub = 0;
#define SH_TOPFUN 0x8000  // this is a temporary tksh hack
    if (mode & SH_TOPFUN) {
        mode ^= SH_TOPFUN;
        shp->fn_reset = 1;
    }
    sh_pushcontext(shp, buffp, SH_JMPEVAL);
    buffp->olist = pp->olist;
    jmpval = sigsetjmp(buffp->buff, 0);
    while (jmpval == 0) {
        if (mode & SH_READEVAL) {
            lineno = shp->inlineno;
            traceon = sh_isoption(shp, SH_XTRACE);
            if (traceon) sh_offoption(shp, SH_XTRACE);
        }
        t = sh_parse(shp, iop, (mode & (SH_READEVAL | SH_FUNEVAL)) ? mode & SH_FUNEVAL : SH_NL);
        if (!(mode & SH_FUNEVAL) || !sfreserve(iop, 0, 0)) {
            if (!(mode & SH_READEVAL)) sfclose(iop);
            io_save = 0;
            mode &= ~SH_FUNEVAL;
        }
        mode &= ~SH_READEVAL;
        if (!sh_isoption(shp, SH_VERBOSE)) sh_offstate(shp, SH_VERBOSE);
        if ((mode & ~SH_FUNEVAL) && shp->gd->hist_ptr) {
            hist_flush(shp->gd->hist_ptr);
            mode = sh_state(SH_INTERACTIVE);
        }
        sh_exec(shp, t,
                sh_isstate(shp, SH_ERREXIT) | sh_isstate(shp, SH_NOFORK) | (mode & ~SH_FUNEVAL));
        if (!io_save) break;
    }
    sh_popcontext(shp, buffp);
    shp->binscript = binscript;
    shp->comsub = comsub;
    if (traceon) sh_onoption(shp, SH_XTRACE);
    if (lineno) shp->inlineno = lineno;
    if (io_save) sfclose(io_save);
    io_save = iosaved == iop ? 0 : iosaved; /* io_save is static so assignment is meaningful. */
    sh_freeup(shp);
    shp->st.staklist = saveslp;
    shp->fn_reset = 0;
    if (jmpval > SH_JMPEVAL) siglongjmp(shp->jmplist->buff, jmpval);
    return shp->exitval;
}

int sh_run(Shell_t *shp, int argn, char *argv[]) {
    struct dolnod *dp;
    struct comnod *t = stkalloc(shp->stk, sizeof(struct comnod));
    int savtop = stktell(shp->stk);
    char *savptr = stkfreeze(stkstd, 0);
    Shbltin_t bltindata;

    bltindata = shp->bltindata;
    memset(t, 0, sizeof(struct comnod));
    dp = stkalloc(shp->stk,
                  sizeof(struct dolnod) + ARG_SPARE * sizeof(char *) + argn * sizeof(char *));
    dp->dolnum = argn;
    dp->dolbot = ARG_SPARE;
    memcpy(dp->dolval + ARG_SPARE, argv, (argn + 1) * sizeof(char *));
    t->comarg = (struct argnod *)dp;
    if (!strchr(argv[0], '/')) {
        t->comnamp = nv_bfsearch(argv[0], shp->fun_tree, (Namval_t **)&t->comnamq, NULL);
    }
    argn = sh_exec(shp, (Shnode_t *)t, sh_isstate(shp, SH_ERREXIT));
    shp->bltindata = bltindata;
    if (savptr != stkptr(shp->stk, 0)) {
        stkset(shp->stk, savptr, savtop);
    } else {
        stkseek(shp->stk, savtop);
    }
    return argn;
}
