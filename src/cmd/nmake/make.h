/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make common definitions
 */

#include <ast.h>
#include <ls.h>
#include <ctype.h>
#include <dirent.h>
#include <fs3d.h>
#include <glob.h>
#include <hash.h>
#include <swap.h>
#include <namval.h>
#include <error.h>
#include <coshell.h>
#include <times.h>
#include <tok.h>
#include <setjmp.h>
#include <sfdisc.h>
#include <tmx.h>

#if DEBUG
#define debug(x)	do if (error_info.trace < 0) { error x; } while (0)
#define	PANIC		(ERROR_PANIC|ERROR_SOURCE|ERROR_SYSTEM),__FILE__,__LINE__
#else
#define debug(x)
#define	PANIC		ERROR_PANIC
#endif

#define COMMENT		'#'		/* make comment char		*/
#define MARK_CONTEXT	'\002'		/* context mark -- not in input!*/
#define MARK_QUOTE	'\003'		/* quote mark -- not in input!	*/
#define SALT		'#'		/* preprocessor control char	*/
#define VIEWOFFSET	'0'		/* char offset for view==0	*/

#if _WINIX
#define FILE_SPACE	'\001'		/* file name space		*/
#else
#define FILE_SPACE	'?'		/* file name space		*/
#endif

#define ATTRNAME	'.'		/* prefix to name an attribute	*/
#define ATTRSET		'+'		/* prefix to set an attribute	*/
#define ATTRCLEAR	'-'		/* prefix to clear an attribute	*/

#define CMDTRACE	(-6)		/* coshell trace debug level	*/
#define EXPTRACE	(-5)		/* explanation trace debug lev	*/

#define EXPLAIN		(state.user&&(state.explain||error_info.trace<=EXPTRACE))

#undef	atoi
#undef	atol
#undef	bind
#undef	clrbit		/* netbsd has one in <sys/param.h> */
#undef	optinit
#undef	setbit		/* netbsd has one in <sys/param.h> */

#define bind		bindrule /* avoids possible socket clash */
#define canon(x)	((state.context&&iscontextp(x,&state.tmppchar))?state.tmppchar:(state.mam.statix?mamcanon(x):pathcanon(x,0,0)))
#define clrbit(v,b)	((v)&=~(1L<<(b)))
#define getar(name)	((Dir_t*)hashget(table.ar,(name)))
#define getbound(name)	((char*)hashget(table.bound,(name)))
#define getdir(id)	((Dir_t*)hashget(table.dir,(char*)(id)))
#define getfile(name)	((File_t*)hashget(table.file,(name)))
#define getold(name)	((char*)hashget(table.oldvalue,(name)))
#define getreg(name)	((int*)hashget(table.regress,(name)))
#define getrule(name)	((Rule_t*)hashget(table.rule,(name)))
#define getvar(name)	((Var_t*)hashget(table.var,(name)))
#define message(x)	do if (error_info.trace < 0) { error x; } while (0)
#define notfile(r)	(((r)->property&(P_attribute|P_functional|P_make|P_operator|P_state|P_use|P_virtual))||((r)->dynamic&D_scope)||(r)->semaphore||((r)->property&P_dontcare)&&((r)->dynamic&D_bound)&&!(r)->time)
#define oldname(r)	do{if(getbound(r->uname))putbound(0,0);if(r->dynamic&D_alias)r->dynamic&=~D_alias;else putrule(r->name,0);r->name=r->uname;r->uname=0;}while(0)
#define putbound(n,d)	hashput(table.bound,(char*)(n),(char*)(d))
#define putar(name,d)	hashput(table.ar,(name),(char*)(d))
#define putdir(id,d)	hashput(table.dir,(char*)(id),(char*)(d))
#define putfile(name,f)	hashput(table.file,(char*)(name),(char*)(f))
#define putold(name,v)	hashput(table.oldvalue,(char*)(name),(char*)(v))
#define putptr(f,p)	(internal.ptr=(char*)(p),sfwrite(f,&internal.ptr,sizeof(internal.ptr)))
#define putreg(name,r)	hashput(table.regress,(char*)(name),(char*)(r))
#define putrule(name,r)	hashput(table.rule,(char*)(name),(char*)(r))
#define putvar(name,v)	hashput(table.var,(char*)(name),(char*)(v))
#define reason(x)	do if(EXPLAIN)explain x;while(0)
#define ropen(f,m)	((f)==internal.openfile?(internal.openfile=0,internal.openfd):open(f,m))
#define rsfopen(f)	((f)==internal.openfile?(internal.openfile=0,sfnew(NiL,NiL,SF_UNBOUND,internal.openfd,SF_READ)):sfopen(NiL,f,"re"))
#define setbit(v,b)	((v)|=(1L<<(b)))
#define shquote		shellquote /* netbsd has one in <stdlib.h>! */
#define statetimeq	timestateq /* avoids statetime symbol truncation clash */
#define timefix(t)      t-=(t<state.tolerance)?t:state.tolerance
#define trap()		(state.caught?handle():0)
#define tstbit(v,b)	((v)&(1L<<(b)))
#define unviewname(s)	(*(s)='(')
#define viewable(r)	((r->property&P_state)&&*r->name=='(')
#define viewname(s,v)	(*(s)=VIEWOFFSET+(v))
#define zero(x)		memzero(&(x),sizeof(x))

#define isaltstate(s)	(nametype(s,NiL)&(NAME_altstate))
#define iscontext(s)	(nametype(s,NiL)&(NAME_context))
#define iscontextp(s,p)	(nametype(s,p)&(NAME_context))
#define isdynamic(s)	(nametype(s,NiL)&(NAME_dynamic|NAME_glob))
#define isglob(s)	(nametype(s,NiL)&(NAME_glob))
#define isintvar(s)	(nametype(s,NiL)&(NAME_intvar))
#define isstate(s)	(nametype(s,NiL)&(NAME_staterule|NAME_altstate|NAME_statevar))
#define isstatevar(s)	(nametype(s,NiL)&(NAME_statevar))

#define freelist(x)	do{if(x){(x)->rule=(Rule_t*)internal.freelists;internal.freelists=(char*)(x);}}while(0)
#define freerule(r)	do{zero(*r);*((char**)r)=internal.freerules;internal.freerules=(char*)(r);}while(0)
#define freevar(v)	do{(v)->property&=(V_free|V_import);*((char**)v)=internal.freevars;internal.freevars=(char*)(v);}while(0)

#define newlist(x)	do{if(x=(List_t*)internal.freelists){if(x->next){x=x->next;*((char**)internal.freelists)=(char*)x->next;}else internal.freelists=(char*)x->rule;}else x=(List_t*)newchunk(&internal.freelists,sizeof(List_t));}while(0)
#define newrule(r)	do{if(r=(Rule_t*)internal.freerules){internal.freerules=(*((char**)r));zero(*r);}else r=(Rule_t*)newchunk(&internal.freerules,sizeof(Rule_t));}while(0)
#define newvar(v)	do{if(v=(Var_t*)internal.freevars){internal.freevars=(*((char**)v));}else v=(Var_t*)newchunk(&internal.freevars,sizeof(Var_t));}while(0)

#if CHAR_MIN < 0
#define ctable		(ctypes-(CHAR_MIN)+1)
#else
#define ctable		(ctypes)
#endif

#define istype(c,t)	(ctable[c]&(t))
#define settype(c,t)	(ctable[c]|=(t))
#define unsettype(c,t)	(ctable[c]&=~(t))

#define NOTYET		0	/* don't know what to do yet		*/
#define UPDATE		1	/* rule in process of being updated	*/
#define MAKING		2	/* executing update action		*/
#define TOUCH		3	/* archive member to be touched		*/
#define EXISTS		4	/* rule already exists in desired state	*/
#define IGNORE		5	/* rule make failed but ignore errors	*/
#define FAILED		6	/* rule make failed			*/
#define OLDRULE		7	/* makefile compiler old rule mark	*/

#define RECOMPILE	1	/* recompile make object		*/
#define COMPILED	2	/* make object compiled (if necessary)	*/
#define SAVED		3	/* make state saved			*/

#define DELETE	NiL		/* delete path component in edit()	*/
#define KEEP	((char*)1)	/* keep path component in edit()	*/

#define NOTIME	TMX_NOTIME	/* not checked time			*/
#define OLDTIME	((Time_t)(1))	/* oldest valid time			*/
#define CURTIME	TMX_NOW		/* high resolution current time		*/
#define CURSECS	((Seconds_t)time(NiL)) /* seconds resolution time	*/

/*
 * VAR and RULE must not change -- the rest must be in sequence
 */

#define VAR		0	/* state var from var in staterule()	*/
#define CONSISTENT	VAR	/* consistency check bits		*/
#define RULE		1	/* state rule in staterule()		*/
#define PREREQS		2	/* alternate prereqs in staterule()	*/
#define STATERULES	2	/* last staterule() index		*/

#define MINVALUE	32	/* minimum variable value length	*/
#define MAXJOBS		128	/* maximum number concurrent jobs	*/
#define MAXNAME		1024	/* maximum file pathname length		*/
#define PCTGARBAGE	10	/* maximum state garbage percentage	*/

#define MAXVIEW		(sizeof(Flags_t)*CHAR_BIT) /* max view index	*/

#if BINDINDEX
#define MAXBIND		UCHAR_MAX/* maximum bind index			*/
#endif

#define A_clear		(1<<0)	/* assertion() .CLEAR			*/
#define A_copy		(1<<1)	/* assertion() .COPY			*/
#define A_delete	(1<<2)	/* assertion() .DELETE			*/
#define A_group		(1<<3)	/* assertion() metarule (...) grouping	*/
#define A_insert	(1<<4)	/* assertion() .INSERT			*/
#define A_metarule	(1<<5)	/* assertion() pattern metarule 	*/
#define A_negate	(1<<6)	/* assertion() -attribute		*/
#define A_nooptions	(1<<7)	/* assertion() no more option prereqs	*/
#define A_norhs		(1<<8)	/* assertion() empty rhs		*/
#define A_null		(1<<9)	/* assertion() .NULL			*/
#define A_scan		(1<<10)	/* assertion() .SCAN			*/
#define A_scope		(1<<11)	/* assertion() metarule scope prereqs	*/
#define A_special	(1<<12)	/* assertion() .SPECIAL			*/
#define A_target	(1<<13)	/* assertion() set P_target		*/

#define C_ID1		(1<<0)	/* istype() first identifier char	*/
#define C_ID2		(1<<1)	/* istype() remaining id chars		*/
#define C_MATCH		(1<<2)	/* istype() shell pattern match chars	*/
#define C_OPTVAL	(1<<3)	/* istype() option value separator	*/
#define C_SEP		(1<<4)	/* istype() token separator		*/
#define C_TERMINAL	(1<<5)	/* istype() terminal chars		*/
#define C_VARIABLE1	(1<<6)	/* istype() first variable name char	*/
#define C_VARIABLE2	(1<<7)	/* istype() remaining variable chars	*/

#define C_VARPOS1	(1<<8)	/* istype() var superimposed code 1	*/
#define C_VARPOS2	(1<<9)	/* istype() var superimposed code 2	*/
#define C_VARPOS3	(1<<10)	/* istype() var superimposed code 3	*/
#define C_VARPOS4	(1<<11)	/* istype() var superimposed code 4	*/
#define C_VARPOS5	(1<<12)	/* istype() var superimposed code 5	*/
#define C_VARPOS6	(1<<13)	/* istype() var superimposed code 6	*/
#define C_VARPOS7	(1<<14)	/* istype() var superimposed code 7	*/
#define C_VARPOS8	(1L<<15)/* istype() var superimposed code 8	*/

#define NAME_altstate	0x001	/* altername state rule name		*/
#define NAME_assignment	0x002	/* assignment				*/
#define NAME_context	0x004	/* context string			*/
#define NAME_dynamic	0x008	/* requires dynamic expand()		*/
#define NAME_glob	0x010	/* requires dynamic glob()		*/
#define NAME_identifier	0x020	/* identifier name			*/
#define NAME_intvar	0x040	/* internal variable name		*/
#define NAME_option	0x080	/* option				*/
#define NAME_path	0x100	/* path name				*/
#define NAME_staterule	0x200	/* state rule name			*/
#define NAME_statevar	0x400	/* state variable name			*/
#define NAME_variable	0x800	/* variable name			*/

#define ARG_ASSIGN	(1<<0)	/* command line assignment arg flag	*/
#define ARG_SCRIPT	(1<<1)	/* command line script arg flag		*/
#define ARG_TARGET	(1<<2)	/* command line target arg flag		*/

#define BIND_DOT	(1<<0)	/* bindfile in .			*/
#define BIND_FORCE	(1<<1)	/* force bindfile current time		*/
#define BIND_MAKEFILE	(1<<2)	/* bindfile using makefile dirs		*/
#define BIND_RULE	(1<<3)	/* force bindfile makerule		*/

#define MERGE_ALL	(1<<0)	/* merge everything			*/
#define MERGE_ASSOC	(1<<1)	/* pattern association merge		*/
#define MERGE_ATTR	(1<<2)	/* merge just attributes		*/
#define MERGE_BOUND	(1<<3)	/* MERGE_ALL but no bind		*/
#define MERGE_FORCE	(1<<4)	/* override attributes			*/
#define MERGE_SCANNED	(1<<5)	/* MERGE_ALL but no entries|scanned	*/

#define COMP_BASE	(1<<0)	/* base rules prereq			*/
#define COMP_DONTCARE	(1<<1)	/* optional include prereq		*/
#define COMP_FILE	(1<<2)	/* -f prereq				*/
#define COMP_GLOBAL	(1<<3)	/* -g prereq				*/
#define COMP_INCLUDE	(1<<4)	/* include prereq			*/
#define COMP_OPTIONS	(1<<5)	/* -[DIU]* prereq			*/
#define COMP_RULES	(1<<6)	/* from explicit rules statement	*/
#define COMP_NSEC	(1<<7)	/* next prereq nsec			*/

#define PREREQ_APPEND	1	/* addprereq append			*/
#define PREREQ_DELETE	2	/* addprereq delete			*/
#define PREREQ_INSERT	3	/* addprereq insert			*/
#define PREREQ_LENGTH	4	/* addprereq insert by length		*/

#define SCAN_IGNORE	1	/* .SCAN.IGNORE scan index		*/
#define SCAN_NULL	2	/* .SCAN.NULL scan index		*/
#define SCAN_STATE	3	/* .SCAN.STATE scan index		*/
#define SCAN_USER	8	/* first user defined scan index	*/
#define SCAN_MAX	UCHAR_MAX/* max scan index			*/

#define CO_ALWAYS	(CO_USER<<0)	/* always exec			*/
#define CO_DATAFILE	(CO_USER<<1)	/* job output to state.tmpdata	*/
#define CO_ERRORS	(CO_USER<<2)	/* job had errors		*/
#define CO_FOREGROUND	(CO_USER<<3)	/* wait for completion		*/
#define CO_KEEPGOING	(CO_USER<<4)	/* keep going on job error	*/
#define CO_LOCALSTACK	(CO_USER<<5)	/* stack local vars for action	*/
#define CO_PRIMARY	(CO_USER<<6)	/* primary prereq added		*/
#define CO_SEMAPHORES	(CO_USER<<7)	/* release semaphores		*/
#define CO_URGENT	(CO_USER<<8)	/* enter job at top of queue	*/

/*
 * rule.property flags
 */

#define P_accept	(1<<0)		/* ignore state conflicts	*/
#define P_after		(1<<1)		/* make after parent update	*/
#define P_always	(1<<2)		/* execute even if !state.exec	*/
#define P_archive	(1<<3)		/* rule bound to archive file	*/
#define P_attribute	(1<<4)		/* rule is an attribute		*/
#define P_before	(1<<5)		/* make before parent update	*/
#define P_command	(1<<6)		/* command target -- no pattern	*/
#define P_dontcare	(1<<7)		/* don't care if rule made	*/
#define P_force		(1<<8)		/* target is always out of date	*/
#define P_foreground	(1<<9)		/* run action in foreground	*/
#define P_functional	(1<<10)		/* associated w/functional var	*/
#define P_ignore	(1<<11)		/* parent to ignore this prereq	*/
#define P_immediate	(1<<12)		/* rule needs immediate action	*/
#define P_implicit	(1<<13)		/* force implicit prereq checks	*/
#define P_internal	(1<<14)		/* don't compile unless prereq	*/
#define P_joint		(1L<<15)	/* pseudo for joint targets	*/
#define P_make		(1L<<16)	/* make (not shell) action	*/
#define P_metarule	(1L<<17)	/* metarule			*/
#define P_multiple	(1L<<18)	/* multi prereq occurrences OK	*/

#define P_local		(1L<<19)	/* local affinity		*/

#define P_operator	(1L<<20)	/* rule is an operator		*/
#define P_parameter	(1L<<21)	/* rule bound to parameter file	*/
#define P_readonly	(1L<<22)	/* no user modifications	*/
#define P_repeat	(1L<<23)	/* make even if already made	*/
#define P_state		(1L<<24)	/* state atom			*/
#define P_staterule	(1L<<25)	/* staterule			*/
#define P_statevar	(1L<<26)	/* statevar			*/
#define P_target	(1L<<27)	/* rule is an explicit target	*/
#define P_terminal	(1L<<28)	/* terminal target or metarule	*/
#define P_use		(1L<<29)	/* rule is a .USE script	*/
#define P_virtual	(1L<<30)	/* target is not a file		*/

#define P_read		(1L<<31)	/* read action output		*/

#define P_failure	(P_after|P_before)	/* conjoined		*/

/*
 * rule.dynamic flags
 */

#define D_alias		(1<<0)		/* more than one unbound name	*/
#define D_aliaschanged	(1<<1)		/* alias changed		*/
#define D_bound		(1<<2)		/* rule has been bound		*/
#define D_built		(1<<3)		/* triggered action built target*/
#define D_cached	(1<<4)		/* post assertion info cached	*/
#define D_compiled	(1<<5)		/* rule has been compiled	*/
#define D_dynamic	(1<<6)		/* must do dynamic expansion	*/
#define D_entries	(1<<7)		/* scanned rule has entries	*/
#define D_garbage	(1<<8)		/* state file GC mark		*/
#define D_hasafter	(1<<9)		/* rule has after prereqs	*/
#define D_hasbefore	(1<<10)		/* rule has before prereqs	*/

#define D_membertoo	(1<<11)		/* D_member was also set	*/
#define D_hassemaphore	(1<<12)		/* has rule.semaphore prereq	*/
#define D_same		(1<<13)		/* target unchanged by action	*/

#define D_lower		(1<<14)		/* state from lower view	*/

#define D_source	(1L<<15)	/* .SOURCE directory		*/

#define D_member	(1L<<16)	/* rule bound to archive member	*/

#define D_global	(1L<<17)	/* global view if view==0	*/

#define D_regular	(1L<<18)	/* rule bound to regular file	*/
#define D_scanned	(1L<<19)	/* has been scanned		*/
#define D_select0	(1L<<20)	/* $(...) select bit 0		*/
#define D_select1	(1L<<21)	/* $(...) select bit 1		*/
#define D_triggered	(1L<<22)	/* rule action triggered	*/

#define D_index		(1L<<23)	/* load time index consistency	*/

#define D_bindindex	(1L<<24)	/* bind index table entry	*/

#define D_intermediate	(1L<<25)	/* intermediate pretend target	*/

#define D_hasscope	(1L<<26)	/* has D_scope prereqs		*/
#define D_scope		(1L<<27)	/* scoped var assignment	*/

#define D_hasmake	(1L<<28)	/* rule has .MAKE after prereqs	*/

#define D_context	(1L<<29)	/* ref may be diff dir context	*/

#define D_lowres	(1L<<30)	/* low resolution time		*/

#define D_CLEAROBJECT	(~(D_bindindex|D_built|D_compiled|D_context|D_dynamic|D_index|D_lower|D_lowres|D_scope))

#define M_bind		(1<<0)		/* bind recursion mark		*/
#define M_compile	(1<<1)		/* compilation mark		*/
#define M_directory	(1<<2)		/* bind directory mark		*/
#define M_generate	(1<<3)		/* prereq generation mark	*/
#define M_mark		(1<<4)		/* temporary mark		*/
#define M_metarule	(1<<5)		/* metarule closure mark	*/
#define M_scan		(1<<6)		/* scan recursion mark		*/
#define M_waiting	(1<<7)		/* waiting to complete mark	*/

/*
 * var.property flags
 */

#define V_compiled	(1<<0)		/* variable has been compiled	*/
#define V_frozen	(1<<1)		/* frozen in make object file	*/
#define V_functional	(1<<2)		/* make rule name before access	*/
#define V_import	(1<<3)		/* imported from environment	*/
#define V_local_D	(1<<4)		/* :T=D: localview reference	*/
#define V_oldvalue	(1<<5)		/* compile old value		*/
#define V_readonly	(1<<6)		/* only dynamic modifications	*/
#define V_retain	(1<<7)		/* retain value in state file	*/
#define V_scan		(1<<8)		/* scan for implicit state var	*/

#define V_local_E	(1<<9)		/* :T=E: localview reference	*/

#define V_auxiliary	(1<<10)		/* auxiliary &= assignment	*/
#define V_append	(1<<11)		/* cmd line += property or op	*/
#define V_free		(1<<12)		/* value may be freed		*/
#define V_builtin	(1<<13)		/* builtin V_functional		*/
#define V_scope		(1<<14)		/* scoped value			*/

#define V_restored	(1L<<15)	/* retained value stored	*/

#define V_CLEARSTATE	(V_free|V_restored)
#define V_CLEAROBJECT	(~(V_auxiliary|V_builtin|V_functional|V_import|V_retain|V_scan|V_scope))

/*
 * getval() flags
 */

#define VAL_AUXILIARY	(1<<0)		/* auxilliary value		*/
#define VAL_BRACE	(1<<1)		/* { }				*/
#define VAL_FILE	(1<<2)		/* !notfile(r)			*/
#define VAL_PRIMARY	(1<<3)		/* primary value		*/
#define VAL_UNBOUND	(1<<4)		/* unbound name			*/

/*
 * dumpjob() flags
 */

#define JOB_blocked	1		/* blocked jobs with prereqs	*/
#define JOB_status	2		/* job status list		*/

typedef struct dirent Dirent_t;
typedef struct stat Stat_t;

struct File_s; typedef struct File_s File_t;
struct Frame_s; typedef struct Frame_s Frame_t;
struct List_s; typedef struct List_s List_t;
struct Rule_s; typedef struct Rule_s Rule_t;

typedef uint32_t Flags_t;		/* flag bit vector		*/
typedef uint32_t Seconds_t;		/* seconds resolution time	*/

typedef struct Fileid_s			/* unique file id		*/
{
	long		dev;		/* device number		*/
	long		ino;		/* inode number			*/
} Fileid_t;

typedef struct Dir_s			/* scanned directory entry	*/
{
	char*		name;		/* directory name		*/
	Time_t		time;		/* modify time			*/
	unsigned char	archive;	/* directory is an archive	*/
	unsigned char	directory;	/* directory is a real directory*/
	unsigned char	ignorecase;	/* pox on dirs that ignore case	*/
	unsigned char	truncate;	/* names truncated to this	*/
} Dir_t;

struct File_s				/* file table entry		*/
{
	File_t*		next;		/* next in list			*/
	Dir_t*		dir;		/* directory containing file	*/
	Time_t		time;		/* modify time			*/
};

struct Frame_s				/* active target frame		*/
{
	Frame_t*	parent;		/* parent frame			*/
	Frame_t*	previous;	/* previous active frame	*/
	Rule_t*		target;		/* target in frame		*/
	List_t*		prereqs;	/* original prereqs		*/
	char*		action;		/* original action		*/
	char*		original;	/* original bound name		*/
	char*		primary;	/* metarule primary prereq name	*/
	char*		stem;		/* metarule stem		*/
	Flags_t		flags;		/* make() flags			*/

	struct
	{
	char*		name;		/* original target name		*/
	Frame_t*	frame;		/* original target frame	*/
	Time_t		time;		/* original target time		*/
	}		context;	/* context push/pop		*/
};

/*
 * statevar data, staterule sync time and unbound rule name -- shared in rule.u1
 */

#define event		u1.u_event
#define statedata	u1.u_data
#define uname		u1.u_uname
#define unbound(r)	((r)->uname?(r)->uname:(r)->name)

struct Rule_s				/* rule				*/
{
	char*		name;		/* rule name			*/
	Frame_t*	active;		/* active target frame		*/

	union
	{
	char*		u_uname;	/* unbound name			*/
	char*		u_data;		/* state value			*/
	Time_t		u_event;	/* state rule event time	*/
	}		u1;

	List_t*		prereqs;	/* prerequisites		*/
	char*		action;		/* update action		*/
	Time_t		time;		/* modify time			*/

	Flags_t		attribute;	/* external named attributes	*/
	Flags_t		dynamic;	/* dynamic properties		*/
	Flags_t		property;	/* stable properties		*/

	unsigned char	scan;		/* file scan strategy index	*/
	unsigned char	semaphore;	/* semaphore + count		*/
	unsigned char	status;		/* disposition			*/
	unsigned char	view;		/* view bind index		*/

	unsigned char	mark;		/* M_* marks			*/
	unsigned char	preview;	/* min prereq view		*/
	unsigned short	must;		/* cancel if == 0		*/

	unsigned long	complink;	/* compilation link		*/
	Flags_t		checked[STATERULES+1];	/* view state check	*/

#if BINDINDEX
	unsigned char	source;		/* source bind index		*/
#endif
};

typedef struct Internal_s		/* internal rule and list info	*/
{
	/*
	 * read/write rule attributes
	 */

	Rule_t*		accept;		/* .ACCEPT rule pointer		*/
	Rule_t*		after;		/* .AFTER rule pointer		*/
	Rule_t*		alarm;		/* .ALARM rule pointer		*/
	Rule_t*		always;		/* .ALWAYS rule pointer		*/
	Rule_t*		archive;	/* .ARCHIVE rule pointer	*/
	Rule_t*		attribute;	/* .ATTRIBUTE rule pointer	*/
	Rule_t*		before;		/* .BEFORE rule pointer		*/
	Rule_t*		command;	/* .COMMAND rule pointer	*/
	Rule_t*		dontcare;	/* .DONTCARE rule pointer	*/
	Rule_t*		force;		/* .FORCE rule pointer		*/
	Rule_t*		foreground;	/* .FOREGROUND rule pointer	*/
	Rule_t*		functional;	/* .FUNCTIONAL rule pointer	*/
	Rule_t*		freeze;		/* .FREEZE rule pointer		*/
	Rule_t*		ignore;		/* .IGNORE rule pointer		*/
	Rule_t*		immediate;	/* .IMMEDIATE rule pointer	*/
	Rule_t*		implicit;	/* .IMPLICIT rule pointer	*/
	Rule_t*		insert;		/* .INSERT rule pointer		*/
	Rule_t*		joint;		/* .JOINT rule pointer		*/
	Rule_t*		local;		/* .LOCAL rule pointer		*/
	Rule_t*		make;		/* .MAKE rule pointer		*/
	Rule_t*		making;		/* .MAKING rule pointer		*/
	Rule_t*		multiple;	/* .MULTIPLE rule pointer	*/
	Rule_t*		op;		/* .OPERATOR rule pointer	*/
	Rule_t*		parameter;	/* .PARAMETER rule pointer	*/
	Rule_t*		read;		/* .READ rule pointer		*/
	Rule_t*		readonly;	/* .READONLY rule pointer	*/
	Rule_t*		regular;	/* .REGULAR rule pointer	*/
	Rule_t*		repeat;		/* .REPEAT rule pointer		*/
	Rule_t*		run;		/* .RUN rule pointer		*/
	Rule_t*		semaphore;	/* .SEMAPHORE rule pointer	*/
	Rule_t*		source;		/* .SOURCE rule pointer		*/
	Rule_t*		state;		/* .STATE rule pointer		*/
	Rule_t*		sync;		/* .SYNC rule pointer		*/
	Rule_t*		terminal;	/* .TERMINAL rule pointer	*/
	Rule_t*		use;		/* .USE rule pointer		*/
	Rule_t*		virt;		/* .VIRTUAL rule pointer	*/
	Rule_t*		wait;		/* .WAIT rule pointer		*/

	/*
	 * readonly rule attributes
	 */
	 
	Rule_t*		active;		/* .ACTIVE rule pointer		*/
	Rule_t*		bound;		/* .BOUND rule pointer		*/
	Rule_t*		built;		/* .BUILT rule pointer		*/
	Rule_t*		entries;	/* .ENTRIES rule pointer	*/
	Rule_t*		exists;		/* .EXISTS rule pointer		*/
	Rule_t*		failed;		/* .FAILED rule pointer		*/
	Rule_t*		file;		/* .FILE rule pointer		*/
	Rule_t*		global;		/* .GLOBAL rule pointer		*/
	Rule_t*		member;		/* .MEMBER rule pointer		*/
	Rule_t*		notyet;		/* .NOTYET rule pointer		*/
	Rule_t*		scanned;	/* .SCANNED rule pointer	*/
	Rule_t*		staterule;	/* .STATERULE rule pointer	*/
	Rule_t*		statevar;	/* .STATEVAR rule pointer	*/
	Rule_t*		target;		/* .TARGET rule pointer		*/
	Rule_t*		triggered;	/* .TRIGGERED rule pointer	*/

	/*
	 * special rules and names
	 */

	Rule_t*		args;		/* .ARGS rule pointer		*/
	Rule_t*		bind;		/* .BIND rule pointer		*/
	Rule_t*		clear;		/* .CLEAR rule pointer		*/
	Rule_t*		copy;		/* .COPY rule pointer		*/
	Rule_t*		delete;		/* .DELETE rule pointer		*/
	Rule_t*		dot;		/* . rule pointer		*/
	Rule_t*		empty;		/* "" rule pointer		*/
	Rule_t*		error;		/* error intercept rule pointer	*/
	Rule_t*		exports;	/* .EXPORT rule pointer		*/
	Rule_t*		globalfiles;	/* .GLOBALFILES rule pointer	*/
	Rule_t*		include;	/* .INCLUDE rule pointer	*/
	Rule_t*		internal;	/* .INTERNAL rule pointer	*/
	Rule_t*		main;		/* .MAIN rule pointer		*/
	Rule_t*		makefiles;	/* .MAKEFILES rule pointer	*/
	Rule_t*		metarule;	/* .METARULE rule pointer	*/
	Rule_t*		null;		/* .NULL rule pointer		*/
	Rule_t*		preprocess;	/* .PREPROCESS rule pointer	*/
	Rule_t*		query;		/* .QUERY rule pointer		*/
	Rule_t*		rebind;		/* .REBIND rule pointer		*/
	Rule_t*		reset;		/* .RESET rule pointer		*/
	Rule_t*		retain;		/* .RETAIN rule pointer		*/
	Rule_t*		scan;		/* .SCAN rule pointer		*/
	Rule_t*		script;		/* .SCRIPT rule pointer		*/
	Rule_t*		serialize;	/* - rule pointer		*/
	Rule_t*		special;	/* .SPECIAL rule pointer	*/
	Rule_t*		tmplist;	/* .TMPLIST rule pointer	*/
	Rule_t*		unbind;		/* .UNBIND rule pointer		*/
	Rule_t*		view;		/* .VIEW rule pointer		*/

	/*
	 * pattern association rules
	 */

	Rule_t*		append_p;	/* .APPEND. rule pointer	*/
	Rule_t*		assert_p;	/* .ASSERT. rule pointer	*/
	Rule_t*		assign_p;	/* .ASSIGN. rule pointer	*/
	Rule_t*		attribute_p;	/* .ATTRIBUTE. rule pointer	*/
	Rule_t*		bind_p;		/* .BIND. rule pointer		*/
	Rule_t*		dontcare_p;	/* .DONTCARE. rule pointer	*/
	Rule_t*		insert_p;	/* .INSERT. rule pointer	*/
	Rule_t*		require_p;	/* .REQUIRE. rule pointer	*/
	Rule_t*		source_p;	/* .SOURCE. rule pointer	*/

	/*
	 * miscellaneous internal info
	 */

	Sfio_t*		met;		/* metarule expansion buffer	*/
	Sfio_t*		nam;		/* name generation buffer	*/
	Sfio_t*		tmp;		/* very temporary work buffer	*/
	Sfio_t*		val;		/* initial getval return buffer	*/
	Sfio_t*		wrk;		/* very temporary work buffer	*/

	char*		freelists;	/* free lists list		*/
	char*		freerules;	/* free rules list		*/
	char*		freevars;	/* free variables list		*/

	char*		issource;	/* internal.source* match pat	*/
	char*		openfile;	/* bind()-scan() optimization	*/
	char*		pwd;		/* PWD value			*/
	char*		ptr;		/* temporary for sfstrptr()	*/

	int		openfd;		/* bind()-scan() optimization	*/
	int		pwdlen;		/* strlen(internal.pwd)		*/
} Internal_t;

typedef struct External_s		/* external engine name info	*/
{
	/*
	 * names of variables defined by engine, init, or environment
	 */

	char*		args;		/* candidate args file name(s)	*/
	char*		convert;	/* makefile converter patterns	*/
	char*		file;		/* main input makefile name	*/
	char*		files;		/* candidate makefile name(s)	*/
	char*		import;		/* explicit env override vars	*/
	char*		lib;		/* related file lib directory	*/
	char*		make;		/* program path name		*/
	char*		nproc;		/* # jobs for compatibility	*/
	char*		old;		/* old program path name	*/
	char*		pwd;		/* pwd name			*/
	char*		rules;		/* candidate rules file name(s)	*/
	char*		skip;		/* order directory skip pattern	*/
	char*		version;	/* engine version stamp		*/
	char*		viewdot;	/* . view dir list		*/
	char*		viewnode;	/* view node dir list		*/

	/*
	 * infrequently used engine interface names
	 */

	char*		compdone;	/* made after makefile compiled	*/
	char*		compinit;	/* made before makefile compiled*/
	char*		done;		/* made just before exit	*/
	char*		init;		/* made before first user target*/
	char*		interrupt;	/* made on first interrupt	*/
	char*		jobdone;	/* made when each job done	*/
	char*		makedone;	/* made after done		*/
	char*		makeinit;	/* made after before init	*/
	char*		makeprompt;	/* made just before each prompt	*/
	char*		makerun;	/* made just before each job	*/
	char*		mamname;	/* external mam atom name	*/
	char*		mamaction;	/* external mam action		*/
	char*		order;		/* :W=[OPR]: favorites		*/

	/*
	 * related file suffixes
	 */

	char*		lock;		/* make lock file suffix	*/
	char*		object;		/* make object file suffix	*/
	char*		source;		/* make source file suffix	*/
	char*		state;		/* make state file suffix	*/
	char*		tmp;		/* make temporary file suffix	*/
} External_t;

typedef struct Tables_s			/* hash table pointers		*/
{
	Hash_table_t*	ar;		/* archives dir info by name	*/
	Hash_table_t*	bound;		/* directory of bound file	*/
	Hash_table_t*	dir;		/* directories and archives	*/
	Hash_table_t*	file;		/* files from scanned dirs	*/
	Hash_table_t*	oldvalue;	/* old variable values		*/
	Hash_table_t*	regress;	/* regression path maps		*/
	Hash_table_t*	rule;		/* rule names			*/
	Hash_table_t*	var;		/* variable names		*/
} Tables_t;

#define BIND_EXISTS	(1<<0)		/* statefile loaded		*/
#define BIND_LOADED	(1<<1)		/* statefile loaded		*/

typedef struct Binding_s		/* binding info			*/
{
#if BINDINDEX
	Rule_t*		path;		/* path name component		*/
#else
	char*		path;		/* path name component		*/
#endif
	char*		root;		/* path root			*/
	short		pathlen;	/* path length			*/
	short		rootlen;	/* root length			*/
	unsigned char	flags;		/* BIND_* flags			*/
	unsigned char	map;		/* external index map		*/
} Binding_t;

typedef struct Label_s			/* resume label			*/
{
	jmp_buf		label;
} Label_t;

typedef struct Mam_s			/* mam state			*/
{
	Sfdisc_t	disc;		/* output discipline -- first!	*/

	int		hold;		/* output hold nest level	*/
	int		level;		/* next error() message level	*/
	int		parent;		/* mam parent label		*/
	int		rootlen;	/* strlen(state.mam.root)	*/

	char*		label;		/* instruction label		*/
	char*		options;	/* option string		*/
	char*		root;		/* names relative to this root	*/
	char*		type;		/* mam type name		*/

	Sfio_t*		out;		/* output stream pointer	*/

	unsigned char	dynamic;	/* dynamic mam			*/
	unsigned char	regress;	/* regression mam		*/
	unsigned char	statix;		/* static mam			*/

	unsigned char	dontcare;	/* emit dontcare rules too	*/
	unsigned char	port;		/* emit porting hints		*/
} Mam_t;

typedef struct State_s			/* program state		*/
{
	unsigned char	accept;		/* accept all existing targets	*/
	unsigned char	alias;		/* enable directory aliasing	*/
	unsigned char	base;		/* compile base|global rules	*/
	unsigned char	caught;		/* a signal was caught		*/
	unsigned char	compile;	/* make object compile state	*/
	unsigned char	compileonly;	/* only compile (force)		*/
	unsigned char	compatibility;	/* disable compatibility msgs	*/
	unsigned char	cross;		/* don't run gen'd executables	*/
	unsigned char	exec;		/* execute shell actions	*/
	unsigned char	expandall;	/* expanding $(...)		*/
	unsigned char	expandview;	/* expand paths if fsview!=0	*/
	unsigned char	explain;	/* explain reason for actions	*/
	unsigned char	explicitrules;	/* explicit rules statement	*/
	unsigned char	finish;		/* in finish()			*/
	unsigned char	force;		/* force target updates		*/
	unsigned char	forceread;	/* force makefiles to be read	*/
	unsigned char	forcescan;	/* force implicit prereq scan	*/
	unsigned char	fsview;		/* file system handles views	*/
	unsigned char	fullscan;	/* scan for impl state prereqs	*/
	unsigned char	global;		/* reading global rules		*/
	unsigned char	ignore;		/* ignore sh action errors	*/
	unsigned char	ignorelock;	/* ignore state lock		*/
	unsigned char	import;		/* import var def precedence	*/
	unsigned char	init;		/* engine initialization	*/
	unsigned char	intermediate;	/* force intermediate targets	*/
	unsigned char	interpreter;	/* in interpreter main loop	*/
	unsigned char	keepgoing;	/* continue w/ sibling prereqs	*/
	unsigned char	list;		/* list readable definitions	*/
	unsigned char	localview;	/* automatics to local view	*/
#if BINDINDEX
	unsigned char	logical;	/* emit logical pathnames	*/
#endif
	unsigned char	never;		/* really - don't exec anything	*/
	unsigned char	op;		/* currently parsing operator	*/
	unsigned char	override;	/* override explicit rules	*/
	unsigned char	pushed;		/* --global state		*/
	unsigned char	push_global;	/* --global state		*/
	unsigned char	push_user;	/* --global state		*/
	unsigned char	preprocess;	/* preprocess all makefiles	*/
	unsigned char	reading;	/* currently reading makefile	*/
	unsigned char	readonly;	/* current vars|opts readonly	*/
	unsigned char	ruledump;	/* dump rule information	*/
	unsigned char	savestate;	/* must save state variables	*/
	unsigned char	scan;		/* scan|check implicit prereqs	*/
	unsigned char	serialize;	/* serialize concurrent output	*/
	unsigned char	silent;		/* run silently			*/
	unsigned char	strictview;	/* strict views			*/
	unsigned char	targetcontext;	/* expand in target dir context	*/
	unsigned char	touch;		/* touch out of date targets	*/
	unsigned char	user;		/* user activities started	*/
	unsigned char	val;		/* internal.val in use		*/
	unsigned char	vardump;	/* dump variable information	*/
	unsigned char	virtualdot;	/* fsview . is virtual		*/
	unsigned char	waiting;	/* waiting for job completion	*/
	unsigned char	warn;		/* enable source file warnings	*/

	int		argc;		/* global argc			*/
	int		believe;	/* believe state from this level*/
	int		errors;		/* keepgoing error count	*/
	int		interrupt;	/* interrupt causing exit	*/
	int		jobs;		/* sh action concurrency level	*/
	int		pid;		/* make pid			*/
	int		readstate;	/* state files to this view ok	*/
	int		reread;		/* input makefile reread count	*/
	int		stateview;	/* state file view index	*/
	int		tabstops;	/* tab stops for makefile parse	*/
	int		targetview;	/* most recent active targ view	*/
	int		tolerance;	/* time comparison tolerance	*/
	int		unwind;		/* make() dontcare unwind level	*/

	Flags_t		questionable;	/* questionable code enable bits*/
	Flags_t		test;		/* test code enable bits	*/

	Time_t		start;		/* start time of this make	*/

	char*		corrupt;	/* corrupt state file action	*/
	char*		errorid;	/* error message id		*/
	char*		hold;		/* hold error trap		*/
	char*		loading;	/* loading this object file	*/
	char*		makefile;	/* first makefile name		*/
	char*		objectfile;	/* make object file name	*/
	char*		regress;	/* output for regression test	*/
	char*		rules;		/* base rules base name		*/
	char*		statefile;	/* state variable file name	*/
	char*		targetprefix;	/* target prefix dir separator	*/
	char*		tmppchar;	/* macro char* temporary	*/
	char*		tmpfile;	/* temporary file name		*/
	char*		writeobject;	/* 0:nowrite or object file def	*/
	char*		writestate;	/* 0:nowrite or state file def	*/

	int*		argf;		/* global argv ARG_* flags	*/

	char**		argv;		/* global argv			*/

	Dir_t*		archive;	/* .SCAN archive		*/

	Sfio_t*		context;	/* localview() target context	*/

	Frame_t*	frame;		/* current target frame		*/

#if BINDINDEX
	Binding_t	source[MAXBIND+1];/* source bind table		*/
	int		maxsource;	/* max source bind index	*/
#endif
	Binding_t	view[MAXVIEW+1];/* view bind table		*/
	unsigned int	maxview;	/* max view bind index		*/

	int (*compnew)(const char*, char*, void*); /* new compile rule	*/
	void*		comparg;	/* compnew handle		*/

	Label_t		resume;		/* if interpreter!=0		*/

	Mam_t		mam;		/* mam state			*/

	Coshell_t*	coshell;	/* coshell handle		*/

	Sfio_t*		io[11];		/* print/read streams		*/
} State_t;

typedef struct Var_s			/* variable			*/
{
	char*		name;		/* name				*/
	char*		value;		/* value			*/
	Flags_t		property;	/* static and dynamic		*/
	size_t		length;		/* maximum length of value	*/
	char*		(*builtin)(char**);	/* builtin function	*/
} Var_t;

struct List_s				/* rule cons cell		*/
{
	List_t*		next;		/* next in list			*/
	Rule_t*		rule;		/* list item			*/
};

/*
 * make globals
 */

extern External_t	external;	/* external engine names	*/
extern Internal_t	internal;	/* internal rule and list info	*/
extern State_t		state;		/* engine state			*/
extern Tables_t		table;		/* hash table pointers		*/

extern char		null[];		/* null string			*/
extern char		tmpname[];	/* temporary name buffer	*/

extern short		ctypes[];	/* internal character types	*/

extern char*		idname;		/* interface id name		*/
extern char*		initdynamic;	/* dynamic initialization	*/
extern char*		initstatic;	/* static initialization	*/
extern char*		version;	/* program version stamp	*/

/*
 * make routines
 */

extern File_t*		addfile(Dir_t*, char*, Time_t);
extern void		addprereq(Rule_t*, Rule_t*, int);
extern List_t*		append(List_t*, List_t*);
extern int		apply(Rule_t*, char*, char*, char*, Flags_t);
extern void		argcount(void);
extern void		arscan(Rule_t*);
extern void		artouch(char*, char*);
extern char*		arupdate(char*);
extern Rule_t*		associate(Rule_t*, Rule_t*, char*, List_t**);
extern Var_t*		auxiliary(char*, int);
extern Rule_t*		bind(Rule_t*);
extern void		bindattribute(Rule_t*);
extern Rule_t*		bindfile(Rule_t*, char*, int);
extern Rule_t*		bindstate(Rule_t*, char*);
extern int		block(int);
extern void		candidates(void);
extern char*		call(Rule_t*, char*);
extern Rule_t*		catrule(char*, char*, char*, int);
extern char*		colonlist(Sfio_t*, char*, int, int);
extern void		compile(char*, char*);
extern int		complete(Rule_t*, List_t*, Time_t*, Flags_t);
extern void		compref(Rule_t*, int);
extern List_t*		cons(Rule_t*, List_t*);
extern void		dirscan(Rule_t*);
extern void		drop(void);
extern void		dump(Sfio_t*, int);
extern void		dumpaction(Sfio_t*, const char*, char*, const char*);
extern void		dumpjobs(int, int);
extern void		dumpregress(Sfio_t*, const char*, const char*, char*);
extern void		dumprule(Sfio_t*, Rule_t*);
extern void		dumpvar(Sfio_t*, Var_t*);
extern void		dynamic(Rule_t*);
extern void		edit(Sfio_t*, char*, char*, char*, char*);
extern void		expand(Sfio_t*, char*);
extern void		explain(int, ...);
extern long		expr(Sfio_t*, char*);
extern Sfio_t*		fapply(Rule_t*, char*, char*, char*, Flags_t);
extern void		finish(int);
extern int		forcescan(const char*, char* v, void*);
extern char*		getarg(char**, int*);
extern void		getop(Sfio_t*, char*, int);
extern char*		getval(char*, int);
extern char**		globv(glob_t*, char*);
extern int		handle(void);
extern int		hasattribute(Rule_t*, Rule_t*, Rule_t*);
extern int		hasafter(Rule_t*, Flags_t);
extern void		immediate(Rule_t*);
extern void		initcode(void);
extern void		inithash(void);
extern void		initrule(void);
extern void		initscan(int);
extern void		inittrap(void);
extern void		initview(void);
extern void		initwakeup(int);
extern void		interpreter(char*);
extern int		isoption(const char*);
extern int		nametype(const char*, char**);
extern List_t*		joint(Rule_t*);
extern List_t*		listcopy(List_t*);
extern void		listops(Sfio_t*, int);
extern int		load(Sfio_t*, const char*, int, int);
extern int		loadable(Sfio_t*, Rule_t*, int);
extern void		localvar(Sfio_t*, Var_t*, char*, int);
extern char*		localview(Rule_t*);
extern void		lockstate(int);
extern int		make(Rule_t*, Time_t*, char*, Flags_t);
extern int		makeafter(Rule_t*, Flags_t);
extern int		makebefore(Rule_t*);
extern Rule_t*		makerule(char*);
extern void		maketop(Rule_t*, int, char*);
extern char*		mamcanon(char*);
extern ssize_t		mamerror(int, const void*, size_t);
extern char*		mamname(Rule_t*);
extern void		mampop(Sfio_t*, Rule_t*, Flags_t);
extern int		mampush(Sfio_t*, Rule_t*, Flags_t);
extern Sfio_t*		mamout(Rule_t*);
extern char*		maprule(char*, Rule_t*);
extern void		merge(Rule_t*, Rule_t*, int);
extern void		mergestate(Rule_t*, Rule_t*);
extern void		metaclose(Rule_t*, Rule_t*, int);
extern void		metaexpand(Sfio_t*, char*, char*);
extern Rule_t*		metaget(Rule_t*, Frame_t*, char*, Rule_t**);
extern Rule_t*		metainfo(int, char*, char*, int);
extern int		metamatch(char*, char*, char*);
extern Rule_t*		metarule(char*, char*, int);
extern void		negate(Rule_t*, Rule_t*);
extern void*		newchunk(char**, size_t);
extern void		newfile(Rule_t*, char*, Time_t);
extern char*		objectfile(void);
extern void		parentage(Sfio_t*, Rule_t*, char*);
extern int		parse(Sfio_t*, char*, char*, Sfio_t*);
extern char*		parsefile(void);
extern char*		pathname(char*, Rule_t*);
extern void		poplocal(void*);
extern int		prereqchange(Rule_t*, List_t*, Rule_t*, List_t*);
extern void		punt(int);
extern void*		pushlocal(void);
extern void		readcheck(void);
extern void		readclear(void);
extern void		readenv(void);
extern int		readfile(char*, int, char*);
extern void		readstate(void);
extern void		rebind(Rule_t*, int);
extern void		remdup(List_t*);
extern void		remtmp(int);
extern int		resolve(char*, int, int);
extern int		rstat(char*, Stat_t*, int);
extern void		rules(char*);
extern Rule_t*		rulestate(Rule_t*, int);
extern void		savestate(void);
extern List_t*		scan(Rule_t*, Time_t*);
extern int		scanargs(int, char**, int*);
extern int		set(char*, int, Sfio_t*);
extern Var_t*		setvar(char*, char*, int);
extern void		shquote(Sfio_t*, char*);
extern Rule_t*		source(Rule_t*);
extern int		special(Rule_t*);
extern char*		statefile(void);
extern Rule_t*		staterule(int, Rule_t*, char*, int);
extern Time_t		statetime(Rule_t*, int);
extern int		statetimeq(Rule_t*, Rule_t*);
extern int		strprintf(Sfio_t*, const char*, char*, int, int);
extern void		terminate(void);
extern char*		timefmt(const char*, Time_t);
extern Time_t		timenum(const char*, char**);
extern char*		timestr(Time_t);
extern void		trigger(Rule_t*, Rule_t*, char*, Flags_t);
extern int		unbind(const char*, char*, void*);
extern Dir_t*		unique(Rule_t*);
extern void		unparse(int);
extern Var_t*		varstate(Rule_t*, int);
extern void		wakeup(Seconds_t, List_t*);
