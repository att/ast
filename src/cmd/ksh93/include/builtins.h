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
#ifndef _BUILTINS_H
#define _BUILTINS_H 1

#include <stdbool.h>

#include "cdt.h"
#include "name.h"
#include "option.h"
#include "shtable.h"

// These definitions must be kept in sync with `shtab_builtins[]` in
// src/cmd/ksh93/data/builtins.c
#define SYSEXEC (shgd->bltin_cmds + 0)
#define SYSSET (shgd->bltin_cmds + 1)
// #define SYSCOLON (shgd->bltin_cmds + 2)
#define SYSTRUE (shgd->bltin_cmds + 3)
#define SYSCOMMAND (shgd->bltin_cmds + 4)
// #define SYSCD (shgd->bltin_cmds + 5)
#define SYSBREAK (shgd->bltin_cmds + 6)
#define SYSCONT (shgd->bltin_cmds + 7)
#define SYSTYPESET (shgd->bltin_cmds + 8)
#define SYSTEST (shgd->bltin_cmds + 9)
// #define SYSBRACKET (shgd->bltin_cmds + 10)
#define SYSLET (shgd->bltin_cmds + 11)
#define SYSEXPORT (shgd->bltin_cmds + 12)
#define SYSDOT (shgd->bltin_cmds + 13)
#define SYSRETURN (shgd->bltin_cmds + 14)
#define SYSENUM (shgd->bltin_cmds + 15)
// #define SYSDECLARE (shgd->bltin_cmds + 16)
#define SYSLOCAL (shgd->bltin_cmds + 17)

// This structure is used by `typeset`, `export`, `readonly`.
struct tdata {
    Shell_t *sh;
    Namval_t *tp;
    const char *wctname;
    Sfio_t *outfile;
    char *prefix;
    char *tname;
    char *help;
    char aflag;
    bool pflag;
    int argnum;
    nvflag_t scanmask;
    Dt_t *scanroot;
    char **argnam;
    int indent;
    int noref;
};

// This structure is used by `echo`, `print`, `printf`.
struct print {
    int fd;
    bool raw;
    bool verbose;
    bool vals_are_vars;
    bool write_to_hist;
    bool no_newline;
    const char *format;
    const char *fmt_type;
    const char *msg;
    Namval_t *var_name;
    Sfio_t *outfile;
};

extern int sh_print(char **argv, Shell_t *shp, struct print *pp);
extern int setall(char **, nvflag_t, Dt_t *, struct tdata *);
extern void print_scan(Sfio_t *file, nvflag_t flag, Dt_t *root, bool omit_attrs, struct tdata *tp);
extern void builtin_print_help(Shell_t *shp, const char *cmd);
extern void builtin_unknown_option(Shell_t *shp, const char *cmd, const char *opt);
extern void builtin_missing_argument(Shell_t *shp, const char *cmd, const char *opt);
extern void builtin_usage_error(Shell_t *shp, const char *cmd, const char *fmt, ...);

// Entry points for shell special builtins.
extern int b_alias(int, char *[], Shbltin_t *);
extern int b_break(int, char *[], Shbltin_t *);
extern int b_continue(int, char *[], Shbltin_t *);
extern int b_enum(int, char *[], Shbltin_t *);
extern int b_exec(int, char *[], Shbltin_t *);
extern int b_exit(int, char *[], Shbltin_t *);
extern int b_export(int, char *[], Shbltin_t *);
extern int b_eval(int, char *[], Shbltin_t *);
extern int b_return(int, char *[], Shbltin_t *);
extern int b_true(int, char *[], Shbltin_t *);
extern int b_false(int, char *[], Shbltin_t *);
extern int b_readonly(int, char *[], Shbltin_t *);
extern int b_set(int, char *[], Shbltin_t *);
extern int b_shift(int, char *[], Shbltin_t *);
extern int b_source(int, char *[], Shbltin_t *);
extern int b_trap(int, char *[], Shbltin_t *);
extern int b_typeset(int, char *[], Shbltin_t *);
extern int b_unset(int, char *[], Shbltin_t *);
extern int b_unalias(int, char *[], Shbltin_t *);

// The following are for job control.
#if defined(SIGCLD) || defined(SIGCHLD)
extern int b_jobs(int, char *[], Shbltin_t *);
extern int b_kill(int, char *[], Shbltin_t *);
#ifdef SIGTSTP
extern int b_bg(int, char *[], Shbltin_t *);
extern int b_fg(int, char *[], Shbltin_t *);
extern int b_disown(int, char *[], Shbltin_t *);
#endif  // SIGTSTP
#endif

// The following utilities are built-in because of side-effects.
extern int b_builtin(int, char *[], Shbltin_t *);
extern int b_cd(int, char *[], Shbltin_t *);
extern int b_command(int, char *[], Shbltin_t *);
extern int b_getopts(int, char *[], Shbltin_t *);
extern int b_hist(int, char *[], Shbltin_t *);
extern int b_let(int, char *[], Shbltin_t *);
extern int b_read(int, char *[], Shbltin_t *);
extern int b_ulimit(int, char *[], Shbltin_t *);
extern int b_umask(int, char *[], Shbltin_t *);
extern int b_wait(int, char *[], Shbltin_t *);
extern int b_whence(int, char *[], Shbltin_t *);

extern int b_print(int, char *[], Shbltin_t *);
extern int b_printf(int, char *[], Shbltin_t *);
extern int b_pwd(int, char *[], Shbltin_t *);
extern int b_sleep(int, char *[], Shbltin_t *);
extern int b_test(int, char *[], Shbltin_t *);
extern int b_times(int, char *[], Shbltin_t *);
extern int b_echo(int, char *[], Shbltin_t *);
extern int b_complete(int, char *[], Shbltin_t *);

// Used by `b_whence()` and `b_command()`.
#define WHENCE_P_FLAG (1 << 0)
#define WHENCE_V_FLAG (1 << 1)
#define WHENCE_A_FLAG (1 << 2)
#define WHENCE_F_FLAG (1 << 3)
#define WHENCE_X_FLAG (1 << 4)
#define WHENCE_Q_FLAG (1 << 5)
#define WHENCE_T_FLAG (1 << 6)
int whence(Shell_t *, char **, int);

extern const char e_badfun[];
extern const char e_baddisc[];
extern const char e_nofork[];
extern const char e_nosignal[];
extern const char e_nolabels[];
extern const char e_notimp[];
extern const char e_nosupport[];
extern const char e_badbase[];
extern const char e_overlimit[];

extern const char e_eneedsarg[];
extern const char e_oneoperand[];
extern const char e_toodeep[];
extern const char e_badname[];
extern const char e_badsyntax[];
extern const char e_histopen[];
extern const char e_condition[];
extern const char e_badrange[];
extern const char e_trap[];
extern const char e_direct[];
extern const char e_defedit[];
extern const char e_cneedsarg[];
extern const char e_defined[];

// For option parsing.
extern const char sh_set[];
extern const char sh_optbreak[];
extern const char sh_optbuiltin[];
extern const char sh_optcd[];
extern const char sh_optcommand[];
extern const char sh_optcont[];
extern const char sh_optdot[];
extern const char sh_optecho[];
extern const char sh_opteval[];
extern const char sh_optexec[];
extern const char sh_optexit[];
extern const char sh_optexport[];
extern const char sh_optgetopts[];
extern const char sh_optbg[];
extern const char sh_optcomplete[];
extern const char sh_optcut[];
extern const char sh_optdisown[];
extern const char sh_optenum[];
extern const char sh_optenum_type[];
extern const char sh_optfg[];
extern const char sh_opthist[];
extern const char sh_optjobs[];
extern const char sh_optkill[];
extern const char sh_optksh_mini[];
extern const char sh_optlet[];
extern const char sh_optprint[];
extern const char sh_optprintf[];
extern const char sh_optpwd[];
extern const char sh_optread[];
extern const char sh_optreadonly[];
extern const char sh_optreturn[];
extern const char sh_optset[];
extern const char sh_optshcomp[];
extern const char sh_optshift[];
extern const char sh_optsleep[];
extern const char sh_optsync[];
extern const char sh_opttest[];
extern const char sh_opttrap[];
extern const char sh_opttype[];
extern const char sh_opttypeset[];
extern const char sh_optulimit[];
extern const char sh_optumask[];
extern const char sh_optunalias[];
extern const char sh_optwait[];
extern const char sh_optunset[];
extern const char sh_optwhence[];
extern const char sh_opttimes[];
// Optional builtins from src/lib/libcmd.
extern const char sh_optbasename[];
extern const char sh_optcat[];
extern const char sh_optchmod[];
extern const char sh_optcmp[];
extern const char sh_optcut[];
extern const char sh_opthead[];
extern const char sh_optdirname[];
extern const char sh_optlogname[];
extern const char sh_optmkdir[];
extern const char sh_optsync[];
extern const char sh_optuname[];
extern const char sh_optwc[];

extern const char e_dict[];

#endif  // _BUILTINS_H
