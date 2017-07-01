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
 * David Korn
 * AT&T Labs
 *
 * Shell interface private definitions
 *
 */
#ifndef defs_h_defined
#define defs_h_defined

#include	<ast.h>
#include	<sfio.h>
#include	<error.h>
#include	"FEATURE/externs"
#include	"FEATURE/options"
#include	<cdt.h>
#include	<history.h>
#include	"fault.h"
#include	"argnod.h"
#include	"name.h"
#include	<ctype.h>

#ifndef pointerof
#define pointerof(x)		((void*)((char*)0+(x)))
#endif

#define Empty			((char*)(e_sptbnl+3))

#define	env_change()		(++ast.env_serial)
#if SHOPT_ENV
#   include	<env.h>
#else
#   define Env_t		void
#   define sh_envput(e,p)	env_change()
#   define env_delete(e,p)	env_change()
#endif

extern char*	sh_getenv(const char*);
extern char*	sh_setenviron(const char*);

/*
 * note that the first few fields have to be the same as for
 * Shscoped_t in <shell.h>
 */


struct sh_scoped
{
	struct sh_scoped *prevst;	/* pointer to previous state */
	int		dolc;
	char		**dolv;
	char		*cmdname;
	char		*filename;
	char		*funname;
	int64_t		lineno;
	Dt_t		*save_tree;	/* var_tree for calling function */
	struct sh_scoped *self;		/* pointer to copy of this scope*/
	Dt_t		*var_local;	/* local level variables for name() */
	struct slnod	*staklist;	/* link list of function stacks */
	int		states;
	int		breakcnt;
	int		execbrk;
	int		loopcnt;
	int		firstline;
	int32_t		optindex;
	int32_t		optnum;
	int32_t		tmout;		/* value for TMOUT */ 
	short		optchar;
	short		opterror;
	int		ioset;
	unsigned short	trapmax;
	char		*trap[SH_DEBUGTRAP+1];
	char		**otrap;
	char		**trapcom;
	char		**otrapcom;
	void		*timetrap;
	struct Ufunction *real_fun;	/* current 'function name' function */
};

struct limits
{
	long		arg_max;	/* max arg+env exec() size */
	int		open_max;	/* maximum number of file descriptors */
	int		clk_tck;	/* number of ticks per second */
	int		child_max;	/* maxumum number of children */
	int		ngroups_max;	/* maximum number of process groups */
	unsigned char	posix_version;	/* posix version number */
	unsigned char	posix_jobcontrol;/* non-zero for job control systems */
	unsigned char	fs3d;		/* non-zero for 3-d file system */
};

#ifndef SH_wait_f_defined
    typedef int (*Shwait_f)(int, long, int);
#   define     SH_wait_f_defined
#endif


struct shared
{
	struct limits	lim;
	uid_t		userid;
	uid_t		euserid;
	gid_t		groupid;
	gid_t		egroupid;
	pid_t		pid;
	pid_t		ppid;
	unsigned char	sigruntime[2];
	Namval_t	*bltin_nodes;
	Namval_t	*bltin_cmds;
	History_t	*hist_ptr;
	char		*shpath;
	char		*user;
	char		**sigmsg;
	char		*rcfile;
	char		**login_files;
	void		*ed_context;
	void		*init_context;
	void		*job_context;
	int		*stats;
	int		bltin_nnodes;	/* number of bltins nodes */ 
	int		sigmax;
	int		nforks;
	int		shtype;
	Shwait_f	waitevent;
};

#define _SH_PRIVATE \
	struct shared	*gd;		/* global data */ \
	struct sh_scoped st;		/* scoped information */ \
	Sfio_t		*heredocs;	/* current here-doc temp file */ \
	Sfio_t		*funlog;	/* for logging function definitions */ \
	int		**fdptrs;	/* pointer to file numbers */ \
	int		savexit; \
	char		*lastarg; \
	char		*lastpath;	/* last alsolute path found */ \
	int		path_err;	/* last error on path search */ \
	Dt_t		*track_tree;	/* for tracked aliases*/ \
	Dt_t		*var_base;	/* global level variables */ \
	Dt_t		*openmatch; \
	Dt_t		*namref_root; \
	Namval_t	*namespace;	/* current active namespace*/ \
	Namval_t	*last_table;	/* last table used in last nv_open  */ \
	Namval_t	*prev_table;	/* previous table used in nv_open  */ \
	Namval_t	*oldnp;		/* last valid parent node  */ \
	Namval_t	**nodelist;	/* for decl commands */ \
	Sfio_t		*outpool;	/* ouput stream pool */ \
	long		timeout;	/* read timeout */ \
	long		curenv;		/* current subshell number */ \
	long		jobenv;		/* subshell number for jobs */ \
	int		infd;		/* input file descriptor */ \
	short		nextprompt;	/* next prompt is PS<nextprompt> */ \
	short		poolfiles; \
	Namval_t	*posix_fun;	/* points to last name() function */ \
	char		*outbuff;	/* pointer to output buffer */ \
	char		*errbuff;	/* pointer to stderr buffer */ \
	char		*prompt;	/* pointer to prompt string */ \
	char		*shname;	/* shell name */ \
	char		*comdiv;	/* points to sh -c argument */ \
	char		*prefix;	/* prefix for compound assignment */ \
	sigjmp_buf	*jmplist;	/* longjmp return stack */ \
	char		*fifo;		/* fifo name for process sub */ \
	int		oldexit; \
	pid_t		bckpid;		/* background process id */ \
	pid_t		cpid; \
	pid_t		spid; 		/* subshell process id */ \
	pid_t		pipepid; \
	pid_t		outpipepid; \
	pid_t		*procsub;	/* pids for >() argument */ \
	int		nprocsub;	/* number of pids in procsub */ \
	int		topfd; \
	int		errorfd; \
	int		savesig; \
	unsigned char	*sigflag;	/* pointer to signal states */ \
	char		intrap; \
	char		login_sh; \
	char		lastbase; \
	char		forked;	\
	char		binscript; \
	char		deftype; \
	char		funload; \
	char		used_pos;	/* used postional parameter */\
	char		universe; \
	char		winch; \
	char		inarith; 	/* set when in ((...)) */ \
	char		indebug; 	/* set when in debug trap */ \
	unsigned char	ignsig;		/* ignored signal in subshell */ \
	unsigned char	lastsig;	/* last signal received */ \
	char		pathinit;	/* pathinit called from subshell */ \
	char		comsub;		/* set when in $() comsub */ \
	char		subshare;	/* set when in ${..} comsub */ \
	char		toomany;	/* set when out of fd's */ \
	char		instance;	/* in set_instance */ \
	char		decomma;	/* decimal_point=',' */ \
	char		redir0;		/* redirect of 0 */ \
	char		intrace;	/* set when trace expands PS4 */ \
	char		*readscript;	/* set before reading a script */ \
	int		subdup;		/* bitmask for dups of 1 */ \
	int		*inpipe;	/* input pipe pointer */ \
	int		*outpipe;	/* output pipe pointer */ \
	int		cpipe[3]; \
	int		coutpipe; \
	int		inuse_bits; \
	struct argnod	*envlist; \
	struct dolnod	*arglist; \
	int		fn_depth; \
	int		fn_reset; \
	int		dot_depth; \
	int		hist_depth; \
	int		xargmin; \
	int		xargmax; \
	int		xargexit; \
	int		nenv; \
	int		lexsize; \
	Sflong_t	sigval; \
	mode_t		mask; \
	Env_t		*env; \
	void		*init_context; \
	void		*mac_context; \
	void		*lex_context; \
	void		*arg_context; \
	void		*job_context; \
	void		*pathlist; \
	void		*defpathlist; \
	void		*cdpathlist; \
	char		**argaddr; \
	void		*optlist; \
	void		**siginfo; \
	Spawnvex_t	*vex; \
	Spawnvex_t	*vexp; \
	struct sh_scoped global; \
	struct checkpt	checkbase; \
	Shinit_f	userinit; \
	Shbltin_f	bltinfun; \
	Shbltin_t	bltindata; \
	char		*cur_line; \
	int		offsets[10]; \
	Sfio_t		**sftable; \
	unsigned int	*fdstatus; \
	const char	*pwd; \
	void		*jmpbuffer; \
	void		*mktype; \
	Sfio_t		*strbuf; \
	Sfio_t		*strbuf2; \
	Dt_t		*first_root; \
	Dt_t		*prefix_root; \
	Dt_t		*last_root; \
	Dt_t		*prev_root; \
	Dt_t		*fpathdict; \
	Dt_t		*typedict; \
	Dt_t		*inpool; \
	Dt_t		*transdict; \
	char		ifstable[256]; \
	unsigned long	test; \
	Shopt_t		offoptions; \
	Shopt_t		glob_options; \
	Namval_t	*typeinit; \
	Namfun_t	nvfun; \
	char		*mathnodes; \
	void		*coshell; \
	char		*bltin_dir; \
	struct Regress_s*regress;

#include	<shell.h>

#include	"shtable.h"
#include	"regress.h"

#if !defined(F_DUPFD_CLOEXEC)
#undef	F_dupfd_cloexec
#define F_dupfd_cloexec		(-99)
#endif

/* error exits from various parts of shell */
#define	NIL(type)	((type)0)

#define new_of(type,x)	((type*)malloc((unsigned)sizeof(type)+(x)))

#define exitset(shp)	(shp->savexit=shp->exitval)

#ifndef SH_DICT
#define SH_DICT		(void*)e_dict
#endif

#ifndef SH_CMDLIB_DIR
#define SH_CMDLIB_DIR	"/opt/ast/bin"
#endif

/* states */
/* low numbered states are same as options */
#define SH_NOFORK	0	/* set when fork not necessary */
#define	SH_FORKED	7	/* set when process has been forked */
#define	SH_PROFILE	8	/* set when processing profiles */
#define SH_NOALIAS	9	/* do not expand non-exported aliases */
#define SH_NOTRACK	10	/* set to disable sftrack() function */
#define SH_STOPOK	11	/* set for stopable builtins */
#define SH_GRACE	12	/* set for timeout grace period */
#define SH_TIMING	13	/* set while timing pipelines */
#define SH_DEFPATH	14	/* set when using default path */
#define SH_INIT		15	/* set when initializing the shell */
#define SH_TTYWAIT	16	/* waiting for keyboard input */ 
#define	SH_FCOMPLETE	17	/* set for filename completion */
#define	SH_PREINIT	18	/* set with SH_INIT before parsing options */
#define SH_COMPLETE	19	/* set for command completion */
#define SH_IOPROMPT	20	/* set when prompting */

#define SH_BASH			41
#define SH_BRACEEXPAND		42
#define SH_POSIX		46
#define SH_MULTILINE    	47

#define SH_NOPROFILE		78
#define SH_NOUSRPROFILE		79
#define SH_LOGIN_SHELL		67
#define SH_COMMANDLINE		0x100
#define SH_BASHEXTRA		0x200
#define SH_BASHOPT		0x400

#define SH_ID			"ksh"	/* ksh id */
#define SH_STD			"sh"	/* standard sh id */

/* defines for sh_type() */

#define SH_TYPE_SH		001
#define SH_TYPE_KSH		002
#define SH_TYPE_BASH		004
#define SH_TYPE_LOGIN		010
#define SH_TYPE_PROFILE		020
#define SH_TYPE_RESTRICTED	040

#if SHOPT_BASH
/*
 *  define for all the bash options
 */
#   define SH_CDABLE_VARS	51
#   define SH_CDSPELL		52
#   define SH_CHECKHASH		53
#   define SH_CHECKWINSIZE	54
#   define SH_CMDHIST		55
#   define SH_DOTGLOB		56
#   define SH_EXECFAIL		57
#   define SH_EXPAND_ALIASES	58
#   define SH_EXTGLOB		59
#   define SH_HOSTCOMPLETE	63
#   define SH_HUPONEXIT		64
#   define SH_INTERACTIVE_COMM	65
#   define SH_LITHIST		66
#   define SH_MAILWARN		68
#   define SH_NOEMPTYCMDCOMPL	69
#   define SH_NOCASEGLOB	70
#   define SH_NULLGLOB		71
#   define SH_PHYSICAL		45
#   define SH_PROGCOMP		72
#   define SH_PROMPTVARS	73
#   define SH_RESTRICTED2	74
#   define SH_SHIFT_VERBOSE	75
#   define SH_SOURCEPATH	76
#   define SH_XPG_ECHO		77
#   define SH_LASTPIPE		78
#endif

#define SH_HISTAPPEND		60
#define SH_HISTEXPAND		43
#define SH_HISTORY2		44
#define SH_HISTREEDIT		61
#define SH_HISTVERIFY		62

#ifndef PIPE_BUF
#   define PIPE_BUF		512
#endif

#if SHOPT_PFSH && ( !_lib_getexecuser || !_lib_free_execattr )
#   undef SHOPT_PFSH
#endif

#define MATCH_MAX		64

#define SH_READEVAL		0x4000	/* for sh_eval */
#define SH_FUNEVAL		0x10000	/* for sh_eval for function load */

extern struct shared	*shgd;
extern void		sh_outname(Shell_t*,Sfio_t*,char*, int);
extern void		sh_applyopts(Shell_t*,Shopt_t);
extern char 		**sh_argbuild(Shell_t*,int*,const struct comnod*,int);
extern struct dolnod	*sh_argfree(Shell_t *, struct dolnod*,int);
extern struct dolnod	*sh_argnew(Shell_t*,char*[],struct dolnod**);
extern void 		*sh_argopen(Shell_t*);
extern struct argnod	*sh_argprocsub(Shell_t*,struct argnod*);
extern void 		sh_argreset(Shell_t*,struct dolnod*,struct dolnod*);
extern Namval_t		*sh_assignok(Namval_t*,int);
extern struct dolnod	*sh_arguse(Shell_t*);
extern char		*sh_checkid(char*,char*);
extern void		sh_chktrap(Shell_t*);
extern void		sh_deparse(Sfio_t*,const Shnode_t*,int);
extern int		sh_debug(Shell_t *shp,const char*,const char*,const char*,char *const[],int);
extern int 		sh_echolist(Shell_t*,Sfio_t*, int, char**);
extern struct argnod	*sh_endword(Shell_t*,int);
extern char 		**sh_envgen(Shell_t*);
#if SHOPT_ENV
extern void 		sh_envput(Shell_t*, Namval_t*);
#endif
extern void 		sh_envnolocal(Namval_t*,void*);
extern Sfdouble_t	sh_arith(Shell_t*,const char*);
extern void		*sh_arithcomp(Shell_t *,char*);
extern pid_t 		sh_fork(Shell_t*,int,int*);
extern pid_t		_sh_fork(Shell_t*,pid_t, int ,int*);
extern char 		*sh_mactrim(Shell_t*,char*,int);
extern int 		sh_macexpand(Shell_t*,struct argnod*,struct argnod**,int);
extern bool		sh_macfun(Shell_t*,const char*,int);
extern void 		sh_machere(Shell_t*,Sfio_t*, Sfio_t*, char*);
extern void 		*sh_macopen(Shell_t*);
extern char 		*sh_macpat(Shell_t*,struct argnod*,int);
extern Sfdouble_t	sh_mathfun(Shell_t*, void*, int, Sfdouble_t*);
extern int		sh_outtype(Shell_t*, Sfio_t*);
extern char 		*sh_mactry(Shell_t*,char*);
extern int		sh_mathstd(const char*);
extern void		sh_printopts(Shell_t*,Shopt_t,int,Shopt_t*);
extern int 		sh_readline(Shell_t*,char**,void*,volatile int,int,ssize_t,long);
extern Sfio_t		*sh_sfeval(char*[]);
extern char		*sh_fmtj(const char*);
extern char		*sh_fmtstr(const char*,int);
extern void		sh_setmatch(Shell_t*,const char*,int,int,int[],int);
extern Dt_t		*sh_subaliastree(Shell_t*,int);
extern void             sh_scope(Shell_t*, struct argnod*, int);
extern Namval_t		*sh_scoped(Shell_t*, Namval_t*);
extern Namval_t		**sh_setlist(Shell_t*,struct argnod*, int, Namval_t*);
extern void 		sh_sigclear(Shell_t*,int);
extern void		sh_sigdone(Shell_t*);
extern void 		sh_sigreset(Shell_t*,int);
extern void		sh_sigtrap(Shell_t*,int);
extern void		sh_siglist(Shell_t*,Sfio_t*,int);
extern Dt_t		*sh_subfuntree(Shell_t*,int);
extern void		sh_subjobcheck(pid_t);
extern int		sh_subsavefd(int);
extern void		sh_subtmpfile(Shell_t*);
extern char 		*sh_substitute(Shell_t*,const char*,const char*,char*);
extern void		sh_timetraps(Shell_t*);
extern const char	*_sh_translate(const char*);
extern bool		sh_trace(Shell_t*,char*[],int);
extern void		sh_trim(char*);
extern int		sh_type(const char*);
extern void             sh_unscope(Shell_t*);
extern void		sh_utol(const char*, char*);
extern int 		sh_whence(char**,int);
#if SHOPT_COSHELL
   extern bool		sh_coaddfile(Shell_t*,char*);
   extern int		sh_copipe(Shell_t*, int[], int);
   extern int		sh_coaccept(Shell_t*,int[],int);
#endif /* SHOPT_COSHELL */
#if SHOPT_NAMESPACE
    extern Namval_t	*sh_fsearch(Shell_t*,const char *,int);
#endif /* SHOPT_NAMESPACE */
extern int		sh_diropenat(Shell_t*, int, const char*);

#ifndef ERROR_dictionary
#   define ERROR_dictionary(s)	(s)
#endif
#define sh_translate(s)	_sh_translate(ERROR_dictionary(s))

#define WBITS		(sizeof(Shopt_t_data_t)*8)
#define WMASK		(0xff)

#define is_option(s,x)	((bool)(((s)->v[((x)&WMASK)/WBITS] &  (1ULL << ((x) % WBITS)))?true:false))
#define on_option(s,x)	((void)((s)->v[((x)&WMASK)/WBITS] |=  (1ULL << ((x) % WBITS))))
#define off_option(s,x)	((void)((s)->v[((x)&WMASK)/WBITS] &= ~(1ULL << ((x) % WBITS))))

#undef sh_isoption
#undef sh_onoption
#undef sh_offoption
#define sh_isoption(shp,x)	is_option(&(shp)->options,(x))
#define sh_onoption(shp,x)	on_option(&(shp)->options,(x))
#define sh_offoption(shp,x)	off_option(&(shp)->options,(x))


#define sh_state(x)	( 1<<(x))
#define	sh_isstate(shp,x)	((shp)->st.states&sh_state(x))
#define	sh_onstate(shp,x)	((shp)->st.states |= sh_state(x))
#define	sh_offstate(shp,x)	((shp)->st.states &= ~sh_state(x))
#define	sh_getstate(shp)	((shp)->st.states)
#define	sh_setstate(shp,x)	((shp)->st.states = (x))

#define sh_sigcheck(shp) do{if(shp->trapnote&SH_SIGSET)sh_exit((shp),SH_EXITSIG);} while(0)

extern int32_t		sh_mailchk;
extern const char	e_dict[];

/* sh_printopts() mode flags -- set --[no]option by default */

#define PRINT_VERBOSE	0x01	/* option on|off list		*/
#define PRINT_ALL	0x02	/* list unset options too	*/
#define PRINT_NO_HEADER	0x04	/* omit listing header		*/
#define PRINT_SHOPT	0x08	/* shopt -s|-u			*/
#define PRINT_TABLE	0x10	/* table of all options		*/

#ifdef SHOPT_STATS
    /* performance statistics */
#   define	STAT_ARGHITS	0
#   define	STAT_ARGEXPAND	1
#   define	STAT_COMSUB	2
#   define	STAT_FORKS	3
#   define	STAT_FUNCT	4
#   define	STAT_GLOBS	5
#   define	STAT_READS	6
#   define	STAT_NVHITS	7
#   define	STAT_NVOPEN	8
#   define	STAT_PATHS	9
#   define	STAT_SVFUNCT	10
#   define	STAT_SCMDS	11
#   define	STAT_SPAWN	12
#   define	STAT_SUBSHELL	13
    extern const Shtable_t shtab_stats[];
#   define sh_stats(x)	(shgd->stats[(x)]++)
#else
#   define sh_stats(x)
#endif /* SHOPT_STATS */
extern const Shtable_t shtab_siginfo[];


#endif
