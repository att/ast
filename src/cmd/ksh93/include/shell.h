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

#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>

#define SH_VERSION 20120720

#include "ast.h"
#include "cdt.h"
#include "defs.h"
#include "fault.h"
#include "name.h"
#include "shcmd.h"
#include "stk.h"

typedef void (*Shinit_f)(Shell_t *, int);

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
// The "viraw" option no longer has any effect as its behavior is now always eanbled.
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
// #define SH_XARGS 34
#define SH_RC 35
#define SH_SHOWME 36
#define SH_LETOCTAL 37

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
    char shcomp;             // set when running shcomp
    short subshell;          // set for virtual subshell
    Stk_t *stk;              // stack pointer
    int pwdfd;               // file descriptor for pwd
    // Everything below this line is private and should not be touched by a plugin.
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
    Sfio_t *outpool;       // output stream pool
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
    checkpt_t *jmplist;   // longjmp return stack plus other data
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
    bool echo_universe_valid;
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
    Lex_t *lex_context;
    struct Shell_arg *arg_context;
    void *job_context;
    Pathcomp_t *pathlist;
    Pathcomp_t *defpathlist;
    Pathcomp_t *cdpathlist;
    char **argaddr;
    void *optlist;
    siginfo_ll_t **siginfo;
#if USE_SPAWN
    Spawnvex_t *vex;
    Spawnvex_t *vexp;
#endif  // USE_SPAWN
    struct sh_scoped global;
    checkpt_t checkbase;
    Shinit_f userinit;
    Shbltin_f bltinfun;
    Shbltin_t bltindata;
    char *cur_line;
    int offsets[10];
    Sfio_t **sftable;
    unsigned int *fdstatus;
    const char *pwd;
    checkpt_t *jmpbuffer;
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
};

// Flags for sh_parse.
#define SH_NL 1   // treat new-lines as ;
#define SH_EOF 2  // EOF causes syntax error

// Symbolic values for sh_iogetiop.
#define SH_IOCOPROCESS (-2)
#define SH_IOHISTFILE (-3)

// Symbolic value for sh_fdnotify.
#define SH_FDCLOSE (-1)

extern int sh_access(const char *, int);
extern int sh_chdir(const char *);
extern int sh_close(int);
extern void sh_delay(double);
extern int sh_dup(int);
extern int sh_exec(Shell_t *, const Shnode_t *, int);
extern int sh_fchdir(int);
extern int sh_fcntl(int, int, ...);
extern int (*sh_fdnotify(int (*)(int, int)))(int, int);
extern char *sh_fmtq(const char *);
extern char *sh_fmtqf(const char *, int, int);
extern Shell_t *sh_getinterp(void);
extern Shell_t *sh_init(int, char *[], Shinit_f);
extern Sfio_t *sh_iogetiop(int, int);
extern int sh_main(int, char *[], Shinit_f);
extern int sh_open(const char *, int, ...);
extern Shnode_t *sh_parse(Shell_t *, Sfio_t *, int);
extern int sh_pipe(int[]);
extern ssize_t sh_read(int, void *, size_t);
extern off_t sh_seek(int, off_t, int);
extern void sh_sigcheck(Shell_t *);
extern int sh_stat(const char *, struct stat *);
extern void sh_subfork(void);
extern mode_t sh_umask(mode_t);
extern int sh_waitsafe(void);
extern ssize_t sh_write(int, const void *, size_t);

extern Namval_t *sh_addbuiltin(Shell_t *, const char *, int (*)(int, char *[], Shbltin_t *),
                               void *);
extern int sh_eval(Shell_t *, Sfio_t *, int);
extern void sh_exit(Shell_t *, int);
extern Sfio_t *sh_fd2sfio(Shell_t *, int);
extern int sh_fun(Shell_t *, Namval_t *, Namval_t *, char *[]);
extern int sh_funscope(Shell_t *, int, char *[], int (*)(void *), void *, int);
extern Shscope_t *sh_getscope(Shell_t *, int, int);
extern void sh_menu(Shell_t *, Sfio_t *, int, char *[]);
extern Sfio_t *sh_pathopen(Shell_t *, const char *);
extern int sh_reinit(Shell_t *, char *[]);
extern int sh_run(Shell_t *, int, char *[]);
extern Shscope_t *sh_setscope(Shell_t *, Shscope_t *);
extern Sfdouble_t sh_strnum(Shell_t *, const char *, char **, int);
extern int sh_trap(Shell_t *, const char *, int);

#define sh_ptr(np) ((np)->nvshell)

//
// Direct access to sh is obsolete, use sh_getinterp() instead.
//
extern Shell_t sh;

#define SH_EXITSIG 0400               // signal exit bit
#define SH_EXITMASK (SH_EXITSIG - 1)  // normal exit status bits

#endif  // _SHELL_H
