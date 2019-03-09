/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
/*
 * getconf - get configuration values
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ast.h"
#include "ast_errorf.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

static const char usage[] =
    "[-?\n@(#)$Id: getconf (AT&T Research) 2013-12-01 $\n]" USAGE_LICENSE
    "[+NAME?getconf - get configuration values]"
    "[+DESCRIPTION?\bgetconf\b displays the system configuration value for "
    "\aname\a. If \aname\a is a filesystem specific variable then the value "
    "is determined relative to \apath\a or the current directory if \apath\a "
    "is omitted. If \avalue\a is specified then \bgetconf\b attempts to "
    "change the process local value to \avalue\a. \b-\b may be used in place "
    "of \apath\a when it is not relevant. If \apath\a is \b=\b then the the "
    "\avalue\a is cached and used for subsequent tests in the calling and "
    "all child processes. Only \bwritable\b variables may be set; "
    "\breadonly\b variables cannot be changed.]"
    "[+?The current value for \aname\a is written to the standard output. If "
    "\aname\a is valid but undefined then \bundefined\b is written to the "
    "standard output. If \aname\a is invalid or an error occurs in "
    "determining its value, then a diagnostic written to the standard error "
    "and \bgetconf\b exits with a non-zero exit status.]"
    "[+?More than one variable may be set or queried by providing the "
    "\aname\a \apath\a \avalue\a 3-tuple for each variable, specifying \b-\b "
    "for \avalue\a when querying.]"
    "[+?If no operands are specified then all known variables are written in "
    "\aname\a=\avalue\a form to the standard output, one per line. Only one "
    "of \b--call\b, \b--name\b or \b--standard\b may be specified.]"
    "[+?This implementation uses the \bastgetconf\b(3) string interface to "
    "the native \bsysconf\b(2), \bconfstr\b(2), \bpathconf\b(2), "
    "\bfpathconf\b(2), and \bsysinfo\b(2) system calls. If \bgetconf\b on "
    "\b$PATH\b is not the default native \bgetconf\b, named by \b$(getconf "
    "GETCONF)\b, then \bastgetconf\b(3) checks only \bast\b specific "
    "extensions and the native system calls; invalid options and/or names "
    "not supported by \bastgetconf\b(3) cause the \bgetconf\b on \b$PATH\b "
    "to be executed.]"

    "[a:all?Call the native \bgetconf\b(1) with option \b-a\b.]"
    "[b:base?List base variable name sans call and standard prefixes.]"
    "[c:call?Display variables with call prefix that matches \aRE\a. The "
    "call prefixes are:]:[RE]"
    "{"
    "[+CS?\bconfstr\b(2)]"
    "[+PC?\bpathconf\b(2)]"
    "[+PC?\bfpathconf\b(2)]"
    "[+SC?\bsysconf\b(2)]"
    "[+SI?\bsysinfo\b(2)]"
    "[+XX?Constant value.]"
    "}"
    "[d:defined?Only display defined values when no operands are specified.]"
    "[f:descriptor?If \apath\a is \b-\b then use file descriptor \afd\a.]#[fd]"
    "[l:lowercase?List variable names in lower case.]"
    "[n:name?Display variables with name that match \aRE\a.]:[RE]"
    "[p:portable?Display the named \bwritable\b variables and values in a "
    "form that can be directly executed by \bsh\b(1) to set the values. If "
    "\aname\a is omitted then all \bwritable\b variables are listed.]"
    "[q:quote?\"...\" quote values.]"
    "[r:readonly?Display the named \breadonly\b variables in "
    "\aname\a=\avalue\a form. If \aname\a is omitted then all \breadonly\b "
    "variables are listed.]"
    "[s:standard?Display variables with standard prefix that matches \aRE\a. "
    "Use the \b--table\b option to view all standard prefixes, including "
    "local additions. The standard prefixes available on all systems "
    "are:]:[RE]"
    "{"
    "[+AES]"
    "[+AST]"
    "[+C]"
    "[+GNU]"
    "[+POSIX]"
    "[+SVID]"
    "[+XBS5]"
    "[+XOPEN]"
    "[+XPG]"
    "}"
    "[t:table?Display the internal table that contains the name, standard, "
    "standard section, and system call symbol prefix for each variable.]"
    "[w:writable?Display the named \bwritable\b variables in "
    "\aname\a=\avalue\a form. If \aname\a is omitted then all \bwritable\b "
    "variables are listed.]"
    "[v:specification?Call the native \bgetconf\b(1) with option \b-v\b "
    "\aname\a.]:[name]"

    "\n"
    "\n[ name [ path [ value ] ] ... ]\n"
    "\n"

    "[+ENVIRONMENT]"
    "{"
    "[+_AST_FEATURES?Process local writable values that are "
    "different from the default are stored in the \b_AST_FEATURES\b "
    "environment variable. The \b_AST_FEATURES\b value is a "
    "space-separated list of \aname\a \apath\a \avalue\a 3-tuples, "
    "where \aname\a is the system configuration name, \apath\a is "
    "the corresponding path, \b-\b if no path is applicable, and "
    "\avalue\a is the system configuration value. \b_AST_FEATURES\b "
    "is an implementation detail of process inheritance; it may "
    "change or vanish in the future; don't rely on it.]"
    "}"
    "[+SEE ALSO?\bpathchk\b(1), \bconfstr\b(2), \bpathconf\b(2), "
    "\bfpathconf\b(2), \bsysconf\b(2), \bastgetconf\b(3)]";

typedef struct Path_s {
    const char *path;
    int len;
} Path_t;

int b_getconf(int argc, char **argv, Shbltin_t *context) {
    char *name = NULL;
    char *path;
    char *value;
    const char *s;
    const char *t;
    char *pattern;
    char *native;
    char *cmd;
    Path_t *e;
    Path_t *p;
    int flags;
    int n;
    int i;
    int m;
    int q;
    char **oargv;
    char buf[PATH_MAX];
    Path_t std[64];
    struct stat st0;
    struct stat st1;

    static const char empty[] = "-";
    static const Path_t equiv[] = {{"/bin", 4}, {"/usr/bin", 8}};

    if (cmdinit(argc, argv, context, 0)) return -1;
    oargv = argv;
    native = astconf("GETCONF", NULL, NULL);
    if (*native != '/') native = 0;
    flags = 0;
    pattern = 0;
    while ((n = optget(argv, usage))) {
        switch (n) {
            case 'a':
                if (native) goto defer;
                break;
            case 'b':
                flags |= ASTCONF_base;
                break;
            case 'c':
                flags |= ASTCONF_matchcall;
                pattern = opt_info.arg;
                break;
            case 'd':
                flags |= ASTCONF_defined;
                break;
            case 'l':
                flags |= ASTCONF_lower;
                break;
            case 'n':
                flags |= ASTCONF_matchname;
                pattern = opt_info.arg;
                break;
            case 'p':
                flags |= ASTCONF_parse;
                break;
            case 'q':
                flags |= ASTCONF_quote;
                break;
            case 'r':
                flags |= ASTCONF_read;
                break;
            case 's':
                flags |= ASTCONF_matchstandard;
                pattern = opt_info.arg;
                break;
            case 't':
                flags |= ASTCONF_table;
                break;
            case 'v':
                if (native) goto defer;
                break;
            case 'w':
                flags |= ASTCONF_write;
                break;
            case ':':
                if (native) goto defer;
                error(2, "%s", opt_info.arg);
                break;
            case '?':
                error(ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
        }
    }
    argv += opt_info.index;
    name = *argv;
    if (!name) {
        path = 0;
    } else if (!strcmp(name, empty)) {
        name = NULL;
        path = *++argv;
        if (path) {
            argv++;
            path = 0;
        }
    }
    if (error_info.errors || (!name && *argv)) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (!name) {
        astconflist(sfstdout, path, flags, pattern);
    } else {
        if (native) flags |= (ASTCONF_system | ASTCONF_error);
        do {
            if (!(path = *++argv)) {
                path = 0;
                value = 0;
            } else {
                if (!strcmp(path, empty)) {
                    path = 0;
                    flags = 0;
                }
                if ((value = *++argv) && !strcmp(value, empty)) {
                    value = 0;
                    flags = 0;
                }
            }
            s = astgetconf(name, path, value, flags, errorf);
            if (error_info.errors) break;
            if (!s) {
                if (native) goto defer;
                error(2, "%s: unknown name", name);
                break;
            }
            if (!value) {
                if (flags & ASTCONF_write) {
                    sfputr(sfstdout, name, ' ');
                    sfputr(sfstdout, path ? path : empty, ' ');
                }
                sfputr(sfstdout, s, '\n');
            }
        } while (*argv && (name = *++argv));
    }
    return error_info.errors != 0;

defer:
    /*
     * defer to argv[0] if absolute and it exists
     */
    cmd = oargv[0];
    if (cmd && *cmd == '/' && !access(cmd, X_OK)) goto found;

    /*
     * defer to the first getconf on $PATH that is also on the standard PATH
     */

    e = std;
    s = astconf("PATH", NULL, NULL);
    q = !stat(equiv[0].path, &st0) && !stat(equiv[1].path, &st1) && st0.st_ino == st1.st_ino &&
        st0.st_dev == st1.st_dev;
    m = 0;
    do {
        for (t = s; *s && *s != ':'; s++) {
            ;
        }
        if ((n = s - t) && *t == '/') {
            if (q) {
                for (i = 0; i < 2; i++) {
                    if (n != equiv[i].len || strncmp(t, equiv[i].path, n)) continue;
                    if (m & (i + 1)) {
                        t = 0;
                    } else {
                        m |= (i + 1);
                        if (!(m & (!i + 1))) {
                            m |= (!i + 1);
                            e->path = t;
                            e->len = n;
                            e++;
                            if (e >= &std[elementsof(std)]) break;
                            t = equiv[!i].path;
                            n = equiv[!i].len;
                        }
                    }
                }
            }
            if (t) {
                e->path = t;
                e->len = n;
                e++;
            }
        }
        while (*s == ':') s++;
    } while (*s && e < &std[elementsof(std)]);
    if (e < &std[elementsof(std)]) {
        e->len = strlen(e->path = "/usr/sbin");
        if (++e < &std[elementsof(std)]) {
            e->len = strlen(e->path = "/sbin");
            e++;
        }
    }
    s = getenv("PATH");
    if (s) {
        do {
            for (t = s; *s && *s != ':'; s++) {
                ;
            }
            n = s - t;
            if (n && *t == '/') {
                for (p = std; p < e; p++) {
                    if (p->len == n && !strncmp(t, p->path, n)) {
                        sfsprintf(buf, sizeof(buf), "%-*.*s/%s", n, n, t, error_info.id);
                        if (!access(buf, X_OK)) {
                            cmd = buf;
                            goto found;
                        }
                    }
                }
            }
            while (*s == ':') s++;
        } while (*s);
    }

    /*
     * defer to the first getconf on the standard PATH
     */

    for (p = std; p < e; p++) {
        sfsprintf(buf, sizeof(buf), "%-*.*s/%s", p->len, p->len, p->path, error_info.id);
        if (!access(buf, X_OK)) {
            cmd = buf;
            goto found;
        }
    }

    /*
     * out of deferrals
     */

    if (name) {
        error(4, "%s: unknown name -- no native getconf(1) to defer to", name);
    } else {
        error(4, "no native getconf(1) to defer to");
    }

    return 2;

found:

    /*
     * don't blame us for crappy diagnostics
     */

    oargv[0] = cmd;
    n = bltin_run(context, argc, oargv);
    if (n >= EXIT_NOEXEC) error(ERROR_SYSTEM | 2, "%s: exec error [%d]", cmd, n);
    return n;
}
