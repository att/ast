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
// test expression
// [ expression ]
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "io.h"
#include "name.h"
#include "sfio.h"
#include "shcmd.h"
#include "shtable.h"
#include "stk.h"
#include "terminal.h"
#include "test.h"
#include "tmx.h"

#ifdef S_ISSOCK
#if _pipe_socketpair
#if _socketpair_shutdown_mode
#define isapipe(f, p)                                                     \
    (test_stat(f, p) >= 0 &&                                              \
     (S_ISFIFO((p)->st_mode) || (S_ISSOCK((p)->st_mode) && (p)->st_ino && \
                                 ((p)->st_mode & (S_IRUSR | S_IWUSR)) != (S_IRUSR | S_IWUSR))))
#else  // _socketpair_shutdown_mode
#define isapipe(f, p) \
    (test_stat(f, p) >= 0 && (S_ISFIFO((p)->st_mode) || (S_ISSOCK((p)->st_mode) && (p)->st_ino)))
#endif  // _socketpair_shutdown_mode
#else   //_pipe_socketpair
#define isapipe(f, p) \
    (test_stat(f, p) >= 0 && (S_ISFIFO((p)->st_mode) || (S_ISSOCK((p)->st_mode) && (p)->st_ino)))
#endif  //_pipe_socketpair
#define isasock(f, p) (test_stat(f, p) >= 0 && S_ISSOCK((p)->st_mode))
#else  // S_ISSOCK
#define isapipe(f, p) (test_stat(f, p) >= 0 && S_ISFIFO((p)->st_mode))
#define isasock(f, p) (0)
#endif  // S_ISSOCK

#define permission(a, f) (sh_access(a, f) == 0)
static_fn time_t test_time(const char *, const char *);
static_fn int test_stat(const char *, struct stat *);
static_fn int test_mode(const char *);

// Single char string compare.
#define c_eq(a, c) (*a == c && *(a + 1) == 0)
// Two character string compare.
#define c2_eq(a, c1, c2) (*a == c1 && *(a + 1) == c2 && *(a + 2) == 0)

struct test {
    Shell_t *sh;
    int ap;
    int ac;
    char **av;
};

static_fn char *nxtarg(struct test *, int);
static_fn int eval_expr(Shell_t *shp, struct test *, int);
static_fn int eval_e3(Shell_t *shp, struct test *);

static_fn int test_strmatch(Shell_t *shp, const char *str, const char *pat) {
    int match[2 * (MATCH_MAX + 1)], n;
    int c, m = 0;
    const char *cp = pat;
    while ((c = *cp++)) {
        if (c == '(') m++;
        if (c == '\\' && *cp) cp++;
    }
    if (m) {
        m++;
    } else {
        match[0] = 0;
    }
    if (m > elementsof(match) / 2) m = elementsof(match) / 2;
    n = strgrpmatch(str, pat, (ssize_t *)match, m,
                    STR_GROUP | STR_MAXIMAL | STR_LEFT | STR_RIGHT | STR_INT);
    if (m == 0 && n == 1) match[1] = (int)strlen(str);
    if (n) sh_setmatch(shp, str, -1, n, match, 0);
    return n;
}

int b_test(int argc, char *argv[], Shbltin_t *context) {
    struct test tdata;
    char *cp = argv[0];
    bool negate;
    Shell_t *shp = context->shp;
    int jmpval = 0, result = 0;
    checkpt_t buff;

    memset(&buff, 0, sizeof(buff));
    tdata.sh = context->shp;
    tdata.av = argv;
    tdata.ap = 1;

    sh_pushcontext(shp, &buff, 1);
    jmpval = sigsetjmp(buff.buff, 0);

    // According to POSIX, test builtin should always return value > 1 on error
    if (jmpval) {
        result = 2;
        goto done;
    }

    if (c_eq(cp, '[')) {
        cp = argv[--argc];
        if (!c_eq(cp, ']')) {
            errormsg(SH_DICT, ERROR_exit(2), e_missing, "']'");
            __builtin_unreachable();
        }
    }

    // According to POSIX, test builtin should return 1 if expression is missing
    if (argc <= 1) {
        result = 1;
        goto done;
    }

    cp = argv[1];
    if (c_eq(cp, '(') && argc <= 6 && c_eq(argv[argc - 1], ')')) {
        // Special case  ( binop ) to conform with standard.
        if (!(argc == 4 && sh_lookup(cp = argv[2], shtab_testops))) {
            cp = (++argv)[1];
            argc -= 2;
        }
    }
    negate = c_eq(cp, '!');
    if (negate && c_eq(argv[2], '(') && argc <= 7 && c_eq(argv[argc - 1], ')')) {
        int i;
        for (i = 2; i < argc; i++) tdata.av[i] = tdata.av[i + 1];
        tdata.av[i] = 0;
        argc -= 2;
    }
    // Posix portion for test.
    switch (argc) {
        case 5: {
            if (!negate) break;
            argv++;
        }
        // FALLTHRU
        case 4: {
            int op = sh_lookup(cp = argv[2], shtab_testops);
            if (op & TEST_BINOP) break;
            if (!op) {
                if (argc == 5) break;
                if (negate && cp[0] == '-' && cp[2] == 0) {
                    result = (test_unop(tdata.sh, cp[1], argv[3]) != 0);
                    goto done;
                } else if (argv[1][0] == '-' && argv[1][2] == 0) {
                    result = !test_unop(tdata.sh, argv[1][1], cp);
                    goto done;
                } else if (negate && c_eq(argv[2], '!')) {
                    result = (*argv[3] == 0);
                    goto done;
                }
                errormsg(SH_DICT, ERROR_exit(2), e_badop, cp);
                __builtin_unreachable();
            }
            result = test_binop(tdata.sh, op, argv[1], argv[3]) ^ (argc != 5);
            goto done;
        }
        case 3: {
            if (negate) {
                result = (*argv[2] != 0);
                goto done;
            }
            if (cp[0] == '-' && strlen(cp) == 2) {
                result = !test_unop(tdata.sh, cp[1], argv[2]);
                goto done;
            }
            break;
        }
        case 2: {
            result = (*cp == 0);
            goto done;
        }
        default: { break; }
    }
    tdata.ac = argc;
    result = !eval_expr(shp, &tdata, 0);

done:
    sh_popcontext(shp, &buff);
    return result;
}

//
// Evaluate a test expression..
// Flag is 0 on outer level.
// Flag is 1 when in parenthesis.
// Flag is 2 when evaluating -a.
//
static_fn int eval_expr(Shell_t *shp, struct test *tp, int flag) {
    int r;
    char *p;

    r = eval_e3(shp, tp);
    while (tp->ap < tp->ac) {
        p = nxtarg(tp, 0);
        // Check for -o and -a.
        if (flag && c_eq(p, ')')) {
            tp->ap--;
            break;
        }
        if (*p == '-' && *(p + 2) == 0) {
            if (*++p == 'o') {
                if (flag == 2) {
                    tp->ap--;
                    break;
                }
                r |= eval_expr(shp, tp, 3);
                continue;
            } else if (*p == 'a') {
                r &= eval_expr(shp, tp, 2);
                continue;
            }
        }
        if (flag == 0) break;
        errormsg(SH_DICT, ERROR_exit(2), e_badsyntax);
        __builtin_unreachable();
    }
    return r;
}

static_fn char *nxtarg(struct test *tp, int mt) {
    if (tp->ap >= tp->ac) {
        if (mt) {
            tp->ap++;
            return 0;
        }
        errormsg(SH_DICT, ERROR_exit(2), e_argument);
        __builtin_unreachable();
    }
    return tp->av[tp->ap++];
}

static_fn int eval_e3(Shell_t *shp, struct test *tp) {
    char *arg, *cp;
    int op;
    char *binop;

    arg = nxtarg(tp, 0);
    if (c_eq(arg, '!')) return !eval_e3(shp, tp);
    if (c_eq(arg, '(')) {
        op = eval_expr(shp, tp, 1);
        cp = nxtarg(tp, 0);
        if (!cp || !c_eq(cp, ')')) {
            errormsg(SH_DICT, ERROR_exit(2), e_missing, "')'");
            __builtin_unreachable();
        }
        return op;
    }
    cp = nxtarg(tp, 1);
    if (cp != 0 && (c_eq(cp, '=') || c2_eq(cp, '!', '='))) goto skip;
    if (c2_eq(arg, '-', 't')) {
        if (cp) {
            long l = strtol(cp, &binop, 10);
            if (*binop) return 0;
            if (l > INT_MAX || l < INT_MIN) return 0;
            op = l;
            if (shp->subshell && op == STDOUT_FILENO) return 0;
            return tty_check(op);
        }
        // Test -t with no arguments.
        tp->ap--;
        if (shp->subshell) return 0;
        return tty_check(STDOUT_FILENO);
    }
    if (*arg == '-' && arg[2] == 0) {
        op = arg[1];
        if (!cp) {
            // For backward compatibility with new flags.
            if (op == 0 || !strchr(test_opchars + 10, op)) return 1;
            errormsg(SH_DICT, ERROR_exit(2), e_argument);
            __builtin_unreachable();
        }
        if (strchr(test_opchars, op)) return test_unop(tp->sh, op, cp);
    }
    if (!cp) {
        tp->ap--;
        return *arg != 0;
    }

skip:
    op = sh_lookup(binop = cp, shtab_testops);
    if (!(op & TEST_BINOP)) cp = nxtarg(tp, 0);
    if (!op) {
        errormsg(SH_DICT, ERROR_exit(2), e_badop, binop);
        __builtin_unreachable();
    }
    if (op == TEST_AND || op == TEST_OR) tp->ap--;
    return test_binop(tp->sh, op, arg, cp);
}

int test_unop(Shell_t *shp, int op, const char *arg) {
    struct stat statb;
    int f;

    switch (op) {
        case 'r': {
            return permission(arg, R_OK);
        }
        case 'w': {
            return permission(arg, W_OK);
        }
        case 'x': {
            return permission(arg, X_OK);
        }
        case 'd': {
            return test_stat(arg, &statb) >= 0 && S_ISDIR(statb.st_mode);
        }
        case 'c': {
            return test_stat(arg, &statb) >= 0 && S_ISCHR(statb.st_mode);
        }
        case 'b': {
            return test_stat(arg, &statb) >= 0 && S_ISBLK(statb.st_mode);
        }
        case 'f': {
            return test_stat(arg, &statb) >= 0 && S_ISREG(statb.st_mode);
        }
        case 'u': {
            return test_mode(arg) & S_ISUID;
        }
        case 'g': {
            return test_mode(arg) & S_ISGID;
        }
        case 'k': {
#ifdef S_ISVTX
            return test_mode(arg) & S_ISVTX;
#else   // S_ISVTX
            return 0;
#endif  // S_ISVTX
        }
        case 'L':
        case 'h': {  // -h is undocumented, and hopefully will disappear
            if (*arg == 0 || arg[strlen(arg) - 1] == '/' || lstat(arg, &statb) < 0) return 0;
            return S_ISLNK(statb.st_mode);
        }
        case 'S': {
            return isasock(arg, &statb);
        }
        case 'N': {
            return test_stat(arg, &statb) >= 0 && tmxgetmtime(&statb) > tmxgetatime(&statb);
        }
        case 'p': {
            return isapipe(arg, &statb);
        }
        case 'n': {
            return *arg != 0;
        }
        case 'z': {
            return *arg == 0;
        }
        case 's': {
            sfsync(sfstdout);
        }
        // TODO: Is this supposed to FALLTHRU?
        // FALLTHRU
        case 'O':
        case 'G': {
            if (*arg == 0 || test_stat(arg, &statb) < 0) return 0;
            if (op == 's') {
                return statb.st_size > 0;
            } else if (op == 'O') {
                return statb.st_uid == shp->gd->userid;
            }
            return statb.st_gid == shp->gd->groupid;
        }
        case 'a':
        case 'e': {
            if (strncmp(arg, "/dev/", 5) == 0 && sh_open(arg, O_NONBLOCK)) return 1;
            return permission(arg, F_OK);
        }
        case 'o': {
            f = 1;
            if (*arg == '?') return sh_lookopt(arg + 1, &f) > 0;
            op = sh_lookopt(arg, &f);
            return op && (f == (sh_isoption(shp, op) != false));
        }
        case 't': {
            char *last;
            long l = strtol(arg, &last, 10);
            if (*last) return 0;
            if (l > INT_MAX || l < INT_MIN) return 0;
            op = (int)l;
            if (shp->subshell && op == STDOUT_FILENO) return 0;
            return tty_check(op);
        }
        case 'v':
        case 'R': {
            Namval_t *np;
            Namarr_t *ap;
            int isref;
            if (!(np = nv_open(arg, shp->var_tree, NV_VARNAME | NV_NOFAIL | NV_NOADD | NV_NOREF))) {
                return 0;
            }
            isref = nv_isref(np);
            if (op == 'R') return isref;
            if (isref) {
                if (FETCH_VT(np->nvalue, const_cp)) {
                    np = nv_refnode(np);
                } else {
                    return 0;
                }
            }
            ap = nv_arrayptr(np);
            if (ap) return nv_arrayisset(np, ap);
            return !nv_isnull(np) || nv_isattr(np, NV_INTEGER);
        }
        default: {
            static char a[3] = "-?";
            a[1] = op;
            errormsg(SH_DICT, ERROR_exit(2), e_badop, a);
            __builtin_unreachable();
        }
    }
}

int test_binop(Shell_t *shp, int op, const char *left, const char *right) {
    double lnum = 0.0;
    double rnum = 0.0;

    if (op & TEST_ARITH) {
        if (*left == '0') {
            while (*left == '0') left++;
            if (!isdigit(*left)) left--;
        }
        if (*right == '0') {
            while (*right == '0') right++;
            if (!isdigit(*right)) right--;
        }
        lnum = sh_arith(shp, left);
        rnum = sh_arith(shp, right);
    }
    switch (op) {
        // Op must be one of the following values.
        case TEST_AND:
        case TEST_OR: {
            return *left != 0;
        }
        case TEST_PEQ: {
            return test_strmatch(shp, left, right);
        }
        case TEST_PNE: {
            return !test_strmatch(shp, left, right);
        }
        case TEST_SGT: {
            return strcoll(left, right) > 0;
        }
        case TEST_SLT: {
            return strcoll(left, right) < 0;
        }
        case TEST_SEQ: {
            return strcmp(left, right) == 0;
        }
        case TEST_SNE: {
            return strcmp(left, right) != 0;
        }
        case TEST_EF: {
            return test_inode(left, right);
        }
        case TEST_NT: {
            return test_time(left, right) > 0;
        }
        case TEST_OT: {
            return test_time(left, right) < 0;
        }
        case TEST_EQ: {
            return lnum == rnum;
        }
        case TEST_NE: {
            return lnum != rnum;
        }
        case TEST_GT: {
            return lnum > rnum;
        }
        case TEST_LT: {
            return lnum < rnum;
        }
        case TEST_GE: {
            return lnum >= rnum;
        }
        case TEST_LE: {
            return lnum <= rnum;
        }
        case TEST_REP: {
            // The ksh93u release treated this as a silent failure. That is, `test foo =~ foo` would
            // simply return zero (false) regardless of the operands and whether the condition is
            // true. We now alert the user that the `=~` binop is not supported by the POSIX test
            // command. See https://github.com/att/ast/issues/1152.
            errormsg(SH_DICT, ERROR_exit(2), e_test_no_pattern);
            __builtin_unreachable();
        }
        default: {
            // This requires that all binops be enumerated above. If we haven't done so that is a
            // bug. Alternatively, data corruption has caused us to be called with an unknown binop.
            // Either way don't pretend everything is okay by returning 0 (false) like ksh93u did.
            errormsg(SH_DICT, ERROR_ERROR, e_op_unhandled, op);
            abort();
        }
    }
}

//
// Returns the modification time of f1 - modification time of f2.
//
static_fn time_t test_time(const char *file1, const char *file2) {
    Time_t t1, t2;
    struct stat statb1, statb2;
    int r = test_stat(file2, &statb2);

    if (test_stat(file1, &statb1) < 0) return r < 0 ? 0 : -1;
    if (r < 0) return 1;
    t1 = tmxgetmtime(&statb1);
    t2 = tmxgetmtime(&statb2);
    if (t1 > t2) return 1;
    if (t1 < t2) return -1;
    return 0;
}

//
// Return true if inode of two files are the same.
//
int test_inode(const char *file1, const char *file2) {
    struct stat stat1, stat2;

    if (test_stat(file1, &stat1) >= 0 && test_stat(file2, &stat2) >= 0) {
        if (stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino) return 1;
    }
    return 0;
}

//
// This version of access checks against effective uid/gid.
//
int sh_access(const char *name, int mode) {
    Shell_t *shp = sh_getinterp();
    struct stat statb;

    if (*name == 0) return -1;
    if (sh_isdevfd(name)) return sh_ioaccess((int)strtol(name + 8, NULL, 10), mode);
    // Can't use access function for execute permission with root.
    if (mode == X_OK && shp->gd->euserid == 0) goto skip;
    if (shp->gd->userid == shp->gd->euserid && shp->gd->groupid == shp->gd->egroupid) {
        return access(name, mode);
    }
    // Swap the real uid to effective, check access then restore. First swap real and effective gid,
    // if different.
    if (shp->gd->groupid == shp->gd->euserid ||
        setregid(shp->gd->egroupid, shp->gd->groupid) == 0) {
        // Next swap real and effective uid, if needed.
        if (shp->gd->userid == shp->gd->euserid ||
            setreuid(shp->gd->euserid, shp->gd->userid) == 0) {
            mode = access(name, mode);
            // Restore ids.
            if (shp->gd->userid != shp->gd->euserid) {
                if (setreuid(shp->gd->userid, shp->gd->euserid) < 0) {
                    // Restoring real user id failed, exit.
                    error(ERROR_system(1), "setreuid(%d, %d) failed", shp->gd->userid,
                          shp->gd->euserid);
                    __builtin_unreachable();
                }
            }
            if (shp->gd->groupid != shp->gd->egroupid) {
                if (setregid(shp->gd->groupid, shp->gd->egroupid) < 0) {
                    // Restoring real group id failed, exit.
                    error(ERROR_system(1), "setregid(%d, %d) failed", shp->gd->groupid,
                          shp->gd->egroupid);
                    __builtin_unreachable();
                }
            }
            return mode;
        } else if (shp->gd->groupid != shp->gd->egroupid) {
            if (setregid(shp->gd->groupid, shp->gd->egroupid) < 0) {
                // Restoring real group id failed, exit.
                error(ERROR_system(1), "setregid(%d, %d) failed", shp->gd->groupid,
                      shp->gd->egroupid);
                __builtin_unreachable();
            }
        }
    }

skip:
    if (test_stat(name, &statb) == 0) {
        if (mode == F_OK) {
            return mode;
        } else if (shp->gd->euserid == 0) {
            if (!S_ISREG(statb.st_mode) || mode != X_OK) return 0;
            // Root needs execute permission for someone.
            mode = (S_IXUSR | S_IXGRP | S_IXOTH);
        } else if (shp->gd->euserid == statb.st_uid) {
            mode <<= 6;
        } else if (shp->gd->egroupid == statb.st_gid) {
            mode <<= 3;
        } else {
            // On some systems you can be in several groups.
            static int maxgroups = 0;
            gid_t *groups;
            int n;
            if (maxgroups == 0) {  // first time
                maxgroups = getgroups(0, NULL);
                // Pre-POSIX system.  This should be a can't happen situation.
                if (maxgroups == -1) maxgroups = sysconf(_SC_NGROUPS_MAX);
            }
            groups = stkalloc(shp->stk, (maxgroups + 1) * sizeof(gid_t));
            n = getgroups(maxgroups, groups);
            while (--n >= 0) {
                if (groups[n] == statb.st_gid) {
                    mode <<= 3;
                    break;
                }
            }
        }
        if (statb.st_mode & mode) return 0;
    }
    return -1;
}

//
// Return the mode bits of file <file>. If <file> is null, then the previous stat buffer is used.
// The mode bits are zero if the file doesn't exist.
//
static_fn int test_mode(const char *file) {
    struct stat statb;

    statb.st_mode = 0;
    if (file && (*file == 0 || test_stat(file, &statb) < 0)) return 0;
    return statb.st_mode;
}

/*
 * do an fstat() for /dev/fd/n, otherwise stat()
 */
static_fn int test_stat(const char *name, struct stat *buff) {
    if (*name == 0) {
        errno = ENOENT;
        return -1;
    }
    if (sh_isdevfd(name)) return fstat((int)strtol(name + 8, NULL, 10), buff);
    return sh_stat(name, buff);
}
