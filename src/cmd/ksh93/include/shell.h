/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
// David Korn
// AT&T Labs
//
// Interface definitions for shell command language.
//
#ifndef _SHELL_H
#define _SHELL_H 1

#define SH_VERSION 20120720

// Bit of a chicken and egg problem here. If we've already included fault.h then this typedef
// already exists and depending on the compiler defining it here may cause a warning or an error.
#ifndef _FAULT_H
typedef struct Shell_s Shell_t;
#endif

#include "ast.h"

#include "cdt.h"
#include "cmd.h"
#include "fault.h"
#include "shcmd.h"
#include "stk.h"

#ifdef _SH_PRIVATE
#include "name.h"
#else
#include "nval.h"
#endif  // _SH_PRIVATE
#if __STDC_VERSION__ >= 199901L
#include <stdint.h>
#endif

/// This is a macro that can be used to silence "unused parameter" warnings from the compiler for
/// functions which need to accept parameters they do not use because they need to be compatible
/// with an interface. It's similar to the Python idiom of doing `_ = param` at the top of a
/// function in the same situation.
#define UNUSED(expr)  \
    do {              \
        (void)(expr); \
    } while (0)

// Options.
#if __STDC_VERSION__ >= 199901L
typedef uint_fast64_t Shopt_t_data_t;
#else
typedef unsigned int Shopt_t_data_t;
#endif
typedef struct {
    Shopt_t_data_t v[(256 / 8) / sizeof(Shopt_t_data_t)];
} Shopt_t;

typedef void (*Shinit_f)(Shell_t *, int);
#ifndef SH_wait_f_defined
typedef int (*Shwait_f)(int, long, int);
#define SH_wait_f_defined
#endif

union Shnode_u;
typedef union Shnode_u Shnode_t;

#define SH_CFLAG 0
#define SH_HISTORY 1      // used also as a state
#define SH_ERREXIT 2      // used also as a state
#define SH_VERBOSE 3      // used also as a state
#define SH_MONITOR 4      // used also as a state
#define SH_INTERACTIVE 5  // used also as a state
#define SH_RESTRICTED 6
#define SH_XTRACE 7
#define SH_KEYWORD 8
#define SH_NOUNSET 9
#define SH_NOGLOB 10
#define SH_ALLEXPORT 11
#define SH_IGNOREEOF 13
#define SH_NOCLOBBER 14
#define SH_MARKDIRS 15
#define SH_BGNICE 16
#define SH_VI 17
#define SH_VIRAW 18
#define SH_TFLAG 19
#define SH_TRACKALL 20
#define SH_SFLAG 21
#define SH_NOEXEC 22
#define SH_GMACS 24
#define SH_EMACS 25
#define SH_PRIVILEGED 26
#define SH_SUBSHARE 27  // subshell shares state with parent
#define SH_NOLOG 28
#define SH_NOTIFY 29
#define SH_DICTIONARY 30
#define SH_PIPEFAIL 32
#define SH_GLOBSTARS 33
#define SH_XARGS 34
#define SH_RC 35
#define SH_SHOWME 36
#define SH_LETOCTAL 37

//
// Passed as flags to builtins in Nambltin_t struct when BLT_OPTIM is on.
//
#define SH_BEGIN_OPTIM 0x1
#define SH_END_OPTIM 0x2

// The following type is used for error messages.

// Error messages.
extern const char e_defpath[];
extern const char e_found[];
extern const char e_nospace[];
extern const char e_format[];
extern const char e_number[];
extern const char e_restricted[];
extern const char e_recursive[];
extern char e_version[];

typedef struct sh_scope {
    struct sh_scope *par_scope;
    int argc;
    char **argv;
    char *cmdname;
    char *filename;
    char *funname;
    int64_t lineno;
    Dt_t *var_tree;
    struct sh_scope *self;
} Shscope_t;

//
// Saves the state of the shell.
//
struct Shell_s {
    Shopt_t options;         // set -o options
    Dt_t *var_tree;          // for shell variables
    Dt_t *fun_tree;          // for shell functions
    Dt_t *alias_tree;        // for alias names
    Dt_t *bltin_tree;        // for builtin commands
    Shscope_t *topscope;     // pointer to top-level scope
    int inlineno;            // line number of current input file
    int exitval;             // most recent exit value
    unsigned char trapnote;  // set when trap/signal is pending
    char shcomp;             // set when runing shcomp
    short subshell;          // set for virtual subshell
    Stk_t *stk;              // stack poiter
    int pwdfd;               // file descriptor for pwd
#ifdef _SH_PRIVATE
    struct shared *gd;    // global data
    struct sh_scoped st;  // scoped information
    Sfio_t *heredocs;     // current here-doc temp file
    Sfio_t *funlog;       // for logging function definitions
    int **fdptrs;         // pointer to file numbers
    int savexit;
    char *lastarg;
    char *lastpath;    // last alsolute path found
    int path_err;      // last error on path search
    Dt_t *track_tree;  // for tracked aliases*/
    Dt_t *var_base;    // global level variables
    Dt_t *openmatch;
    Dt_t *namref_root;
    Namval_t *namespace;   // current active namespace*/
    Namval_t *last_table;  // last table used in last nv_open
    Namval_t *prev_table;  // previous table used in nv_open
    Namval_t *oldnp;       // last valid parent node
    Namval_t **nodelist;   // for decl commands
    Sfio_t *outpool;       // ouput stream pool
    long timeout;          // read timeout
    long curenv;           // current subshell number
    long jobenv;           // subshell number for jobs
    int infd;              // input file descriptor
    short nextprompt;      // next prompt is PS<nextprompt>
    short poolfiles;
    Namval_t *posix_fun;  // points to last name() function
    char *outbuff;        // pointer to output buffer
    char *errbuff;        // pointer to stderr buffer
    char *prompt;         // pointer to prompt string
    char *shname;         // shell name
    char *comdiv;         // points to sh -c argument
    char *prefix;         // prefix for compound assignment
    sigjmp_buf *jmplist;  // longjmp return stack
    char *fifo;           // fifo name for process sub
    int oldexit;
    pid_t bckpid;  // background process id
    pid_t cpid;
    pid_t spid;  // subshell process id
    pid_t pipepid;
    pid_t outpipepid;
    pid_t *procsub;  // pids for >() argument
    int nprocsub;    // number of pids in procsub
    int topfd;
    int errorfd;
    int savesig;
    unsigned char *sigflag;  // pointer to signal states
    char intrap;
    char login_sh;
    char lastbase;
    char forked;
    char binscript;
    char deftype;
    char funload;
    char used_pos;  // used postional parameter
    char universe;
    char winch;
    char inarith;           // set when in ((...))
    char indebug;           // set when in debug trap
    unsigned char ignsig;   // ignored signal in subshell
    unsigned char lastsig;  // last signal received
    char pathinit;          // pathinit called from subshell
    char comsub;            // set when in $() comsub
    char subshare;          // set when in ${..} comsub
    char toomany;           // set when out of fd's
    char instance;          // in set_instance
    char decomma;           // decimal_point=','
    char redir0;            // redirect of 0
    char intrace;           // set when trace expands PS4
    char *readscript;       // set before reading a script
    int subdup;             // bitmask for dups of 1
    int *inpipe;            // input pipe pointer
    int *outpipe;           // output pipe pointer
    int cpipe[3];
    int coutpipe;
    int inuse_bits;
    struct argnod *envlist;
    struct dolnod *arglist;
    int fn_depth;
    int fn_reset;
    int dot_depth;
    int hist_depth;
    int xargmin;
    int xargmax;
    int xargexit;
    int nenv;
    int lexsize;
    Sflong_t sigval;
    mode_t mask;
    Env_t *env;
    void *init_context;
    void *mac_context;
    void *lex_context;
    void *arg_context;
    void *job_context;
    void *pathlist;
    void *defpathlist;
    void *cdpathlist;
    char **argaddr;
    void *optlist;
    siginfo_ll_t **siginfo;
#if !_AST_no_spawnveg
    Spawnvex_t *vex;
    Spawnvex_t *vexp;
#endif
    struct sh_scoped global;
    struct checkpt checkbase;
    Shinit_f userinit;
    Shbltin_f bltinfun;
    Shbltin_t bltindata;
    char *cur_line;
    int offsets[10];
    Sfio_t **sftable;
    unsigned int *fdstatus;
    const char *pwd;
    void *jmpbuffer;
    void *mktype;
    Sfio_t *strbuf;
    Sfio_t *strbuf2;
    Dt_t *first_root;
    Dt_t *prefix_root;
    Dt_t *last_root;
    Dt_t *prev_root;
    Dt_t *fpathdict;
    Dt_t *typedict;
    Dt_t *inpool;
    Dt_t *transdict;
    char ifstable[256];
    unsigned long test;
    Shopt_t offoptions;
    Shopt_t glob_options;
    Namval_t *typeinit;
    Namfun_t nvfun;
    char *mathnodes;
    void *coshell;
    char *bltin_dir;
    char exittrap;
    char errtrap;
    char end_fn;
#endif  // _SH_PRIVATE
};

// Flags for sh_parse.
#define SH_NL 1   // treat new-lines as ;
#define SH_EOF 2  // EOF causes syntax error

// Symbolic values for sh_iogetiop.
#define SH_IOCOPROCESS (-2)
#define SH_IOHISTFILE (-3)

// Symbolic value for sh_fdnotify.
#define SH_FDCLOSE (-1)

#undef getenv  // -lshell provides its own

extern int sh_access(const char *, int);
extern Namval_t *sh_addbuiltin(const char *, int (*)(int, char *[], Shbltin_t *), void *);
extern Dt_t *sh_bltin_tree(void);
extern int sh_chdir(const char *);
extern int sh_close(int);
extern void sh_delay(double);
extern int sh_dup(int);
extern int sh_eval(Sfio_t *, int);
extern int sh_exec(Shell_t *, const Shnode_t *, int);
extern void sh_exit(int);
extern int sh_fchdir(int);
extern int sh_fcntl(int, int, ...);
extern Sfio_t *sh_fd2sfio(int);
extern int (*sh_fdnotify(int (*)(int, int)))(int, int);
extern char *sh_fmtq(const char *);
extern char *sh_fmtqf(const char *, int, int);
extern int sh_fun(Namval_t *, Namval_t *, char *[]);
extern int sh_funscope(int, char *[], int (*)(void *), void *, int);
extern Shell_t *sh_getinterp(void);
extern Shscope_t *sh_getscope(int, int);
extern Shell_t *sh_init(int, char *[], Shinit_f);
extern Sfio_t *sh_iogetiop(int, int);
extern int sh_main(int, char *[], Shinit_f);
extern void sh_menu(Sfio_t *, int, char *[]);
extern void sh_offoption(int);
extern bool sh_isoption(int);
extern void sh_onoption(int);
extern int sh_open(const char *, int, ...);
extern int sh_openmax(void);
extern void *sh_parse(Shell_t *, Sfio_t *, int);
extern Sfio_t *sh_pathopen(const char *);
extern int sh_pipe(int[]);
extern ssize_t sh_read(int, void *, size_t);
extern int sh_reinit(char *[]);
extern int sh_run(int, char *[]);
extern off_t sh_seek(int, off_t, int);
extern Shscope_t *sh_setscope(Shscope_t *);
extern void sh_sigcheck(Shell_t *);
extern int sh_stat(const char *, struct stat *);
extern Sfdouble_t sh_strnum(const char *, char **, int);
extern void sh_subfork(void);
extern int sh_trap(const char *, int);
extern mode_t sh_umask(mode_t);
extern void *sh_waitnotify(Shwait_f);
extern int sh_waitsafe(void);
extern ssize_t sh_write(int, const void *, size_t);

extern Namval_t *sh_addbuiltin_20120720(Shell_t *, const char *,
                                        int (*)(int, char *[], Shbltin_t *), void *);
extern Dt_t *sh_bltin_tree_20120720(Shell_t *);
extern int sh_eval_20120720(Shell_t *, Sfio_t *, int);
extern void sh_exit_20120720(Shell_t *, int);
extern Sfio_t *sh_fd2sfio_20120720(Shell_t *, int);
extern int sh_fun_20120720(Shell_t *, Namval_t *, Namval_t *, char *[]);
extern int sh_funscope_20120720(Shell_t *, int, char *[], int (*)(void *), void *, int);
extern Shscope_t *sh_getscope_20120720(Shell_t *, int, int);
extern void sh_offoption_20120720(Shell_t *, int);
extern bool sh_isoption_20120720(Shell_t *, int);
extern void sh_onoption_20120720(Shell_t *, int);
extern void sh_menu_20120720(Shell_t *, Sfio_t *, int, char *[]);
extern Sfio_t *sh_pathopen_20120720(Shell_t *, const char *);
extern int sh_reinit_20120720(Shell_t *, char *[]);
extern int sh_run_20120720(Shell_t *, int, char *[]);
extern Shscope_t *sh_setscope_20120720(Shell_t *, Shscope_t *);
extern Sfdouble_t sh_strnum_20120720(Shell_t *, const char *, char **, int);
extern int sh_trap_20120720(Shell_t *, const char *, int);
extern void *sh_waitnotify_20120720(Shwait_f, void *);

#define sh_ptr(np) ((np)->nvshell)

//
// Direct access to sh is obsolete, use sh_getinterp() instead.
//
extern Shell_t sh;

#include "shellapi.h"

#ifndef _AST_INTERCEPT
#if _lib_lseek64
#undef stat64
#define stat64(a, b) sh_stat(a, b)
#else
#undef stat
#define stat(a, b) sh_stat(a, b)
#endif
#endif  // !_AST_INTERCEPT
#ifndef _shtest_c
#ifndef _SH_PRIVATE
#undef access
#define access(a, b) sh_access(a, b)
#endif
#endif  // !_shtest_c
#ifndef _shio_h
#ifndef _AST_INTERCEPT
#undef chdir
#define chdir(a) sh_chdir(a)
#undef fchdir
#define fchdir(a) sh_fchdir(a)
#endif
#ifndef HIST_MAX
#if _lib_lseek64
#undef open64
#define open64 sh_open
#undef lseek64
#define lseek64(a, b, c) sh_seek(a, b, c)
#else
#undef open
#define open sh_open
#undef lseek
#define lseek(a, b, c) sh_seek(a, b, c)
#endif
#endif
#ifndef _SH_PRIVATE
#undef access
#define access(a, b) sh_access(a, b)
#undef close
#define close(a) sh_close(a)
#if SHELLAPI(20120720)
#undef exit
#define exit(a) sh_exit(sh_getinterp(), a)
#else
#undef exit
#define exit(a) sh_exit(a)
#endif
#undef fcntl
#define fcntl(a, b, c) sh_fcntl(a, b, c)
#undef pipe
#define pipe(a) sh_pipe(a)
#undef read
#define read(a, b, c) sh_read(a, b, c)
#undef write
#define write(a, b, c) sh_write(a, b, c)
#undef umask
#define umask(a) sh_umask(a)
#undef dup
#define dup sh_dup
#endif  // !_SH_PRIVATE
#endif  // !_shio_h

#define SH_SIGSET 4
#define SH_EXITSIG 0400               // signal exit bit
#define SH_EXITMASK (SH_EXITSIG - 1)  // normal exit status bits
#define SH_RUNPROG -1022              // needs to be negative and < 256

#endif  // _SHELL_H
