/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * ps -- list process status
 *
 * fall back to /bin/ps if no support -- and you better match their args!
 */

#define FIELDS_default	"pid,tty,time,comm"
#define FIELDS_c	"pid,class,pri,tty,time,comm"
#define FIELDS_f	"user,pid,ppid,start,tty,time,cmd"
#define FIELDS_j	"pid,pgrp,sid,tty,time,comm"
#define FIELDS_l	"flags,state,user,pid,ppid,pri,nice,size,rss,wchan,tty,time,cmd"

static const char usage[] =
"[-1o?\n@(#)$Id: ps (AT&T Research) 2011-12-13 $\n]"
USAGE_LICENSE
"[+NAME?ps - report process status]"
"[+DESCRIPTION?\bps\b lists process information subject to the appropriate"
"	privilege. If \apid\a arguments are specified then only those"
"	processes are listed, otherwise all processes with the effective"
"	user id and controlling terminal of the caller are listed. The options"
"	may alter this default behavior.]"
"[+?The listings are sorted by <\bUID,START,PID\b>. Options taking list"
"	arguments accept either space or comma separators.]"

"[a:interactive?List all processes associated with terminals.]"
"[B!:branch?Print tree branch prefixes for \bcommand\b and \bargs\b."
"	Implied by \b--children\b, \b--parents\b, and \b--tree\b.]"
"[c:class?Equivalent to \b--fields=" FIELDS_c "\b.]"
"[C:children?Display the process tree hierarchy, including the children"
"	of all selected processes, in the \bCMD\b field list.]"
"[d:no-session?List all processes except session leaders.]"
"[D:define?Define \akey\a with optional \avalue\a. \avalue\a will be expanded"
"	when \b%(\b\akey\a\b)\b is specified in \b--format\b. \akey\a may"
"	override internal \b--format\b identifiers.]:[key[=value]]]"
"[e|A:all?List all processes.]"
"[E!:escape?Escape non-printing characters in \bcommand\b and \bargs\b.]"
"[f:full?Equivalent to \b--fields=" FIELDS_f "\b.]"
"[F:format?Append to the listing format string (if \b--format\b is specified"
"	then \b--fields\b and all options that modify \b--fields\b are"
"	ignored.) The \bdf\b(1), \bls\b(1) and \bpax\b(1) commands also have"
"	\b--format\b options in this same style. \aformat\a follows"
"	\bprintf\b(3) conventions, except that \bsfio\b(3) inline ids are used"
"	instead of arguments:"
"	%[#-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\aheading\a]])\achar\a."
"	If \b#\b is specified then the internal width and precision are used."
"	If \achar\a is \bs\b then the string form of the item is listed,"
"	otherwise the corresponding numeric form is listed. If \achar\a is"
"	\bq\b then the string form of the item is $'...' quoted if it contains"
"	space or non-printing characters. If \awidth\a is omitted then the"
"	default width is assumed. \aheading\a overrides the default"
"	heading for \aid\a. Supported \aid\as"
"	are:]:[format]{\fformats\f}"
"[g:pgrps|process-groups?List processes with group leaders in the \apgrp\a"
"	list.]:[pgrp...]"
"[G:groups?List processes with real group id names or numbers in the \agroup\a"
"	list.]:[group...]"
"[h!:heading?Output a heading line.]"
"[j:jobs?Equivalent to \b--fields=" FIELDS_j "\b.]"
"[l:long?Equivalent to \b--fields=" FIELDS_l "\b.]"
"[L:leaders?List session leaders.]"
"[n:namelist?Specifies an alternate system namelist \afile\a. Ignored by"
"	this implementation.]"
"[N:default?Equivalent to \b--fields=" FIELDS_default "\b. This is the"
"	format when \b--fields\b is not specified.]"
"[o:fields?(\b--format\b is more general.) List information according to"
"	\akey\a. Multiple \b--fields\b options may be specified; the"
"	resulting format is a left-right ordered list with duplicate entries"
"	deleted from the right. The default width can be overriden by"
"	appending \a+width\a to \akey\a, and the default \alabel\a can be"
"	overridden by appending \a=label\a to \akey\a. The keys, labels and"
"	widths are listed under \b--format\b.]:[key[+width]][=label]]...]"
"[p:pids?List processes in the \apid\a list.]:[pid...]"
"[P:parents?Display the process tree hierarchy, including the parents"
"	of all selected processes, in the \bCMD\b field list.]"
"[r|R:recursive?Recursively list the children of all selected processes.]"
"[s:sessions?List processes with session leaders in the \asid\a list.]:[sid...]"
"[t:terminals|ttys?List processes with controlling terminals in the \atty\a"
"	list.]:[tty...]"
"[T:tree|forest?Display the process tree hierarchy, including the parents and"
"	children of all selected processes, in the \bCMD\b field list.]"
"[u|U:users?List processes with real user id names or numbers in the \auser\a"
"	list.]:[user...]"
"[v:verbose?List verbose error messages for inaccessible processes.]"
"[w:wide?Ignored by this implementation.]"
"[x:detached?List all processes not associated with terminals.]"
"[X:hex?List numeric entries in hexadecimal notation.]"

"\n"
"\n[ pid ... ]\n"
"\n"

"[+SEE ALSO?\bdf\b(1), \bkill\b(1), \bls\b(1), \bnice\b(1), \bpax\b(1),"
"	\bps\b(1), \bsh\b(1), \btop\b(1)]"
;

#include <ast.h>
#include <ast_dir.h>
#include <cdt.h>
#include <ctype.h>
#include <dirent.h>
#include <error.h>
#include <ls.h>
#include <pss.h>
#include <sfdisc.h>
#include <tm.h>

#if !_mem_st_rdev_stat
#define st_rdev			st_dev
#endif

#define KEY_environ		(-1)
#define KEY_alias		0

#define KEY_addr		1
#define KEY_class		2
#define KEY_cmd			3
#define KEY_comm		4
#define KEY_cpu			5
#define KEY_etime		6
#define KEY_flags		7
#define KEY_gid			8
#define KEY_group		9
#define KEY_job			10
#define KEY_nice		11
#define KEY_npid		12
#define KEY_pgrp		13
#define KEY_pid			14
#define KEY_ppid		15
#define KEY_pri			16
#define KEY_proc		17
#define KEY_refcount		18
#define KEY_rss			19
#define KEY_sid			20
#define KEY_size		21
#define KEY_start		22
#define KEY_state		23
#define KEY_tgrp		24
#define KEY_time		25
#define KEY_tty			26
#define KEY_uid			27
#define KEY_user		28
#define KEY_wchan		29

typedef struct Key_s			/* format key			*/
{
	char*		name;		/* key name			*/
	char*		head;		/* heading name			*/
	char*		desc;		/* description			*/
	unsigned long	field;		/* pss field			*/
	short		index;		/* index			*/
	short		width;		/* field width			*/
	unsigned long	maxval;		/* max value if !=0		*/
	unsigned char	hex;		/* optional hex output		*/
	unsigned char	already;	/* already specified		*/
	short		cancel;		/* cancel this if specified	*/
	short		prec;		/* field precision		*/
	short		disable;	/* macro being expanded		*/
	unsigned char	skip;		/* skip this			*/
	char*		macro;		/* macro value			*/
	const char*	sep;		/* next field separator		*/
	Dtlink_t	hashed;		/* hash link			*/
	struct Key_s*	next;		/* format link			*/
} Key_t;

typedef struct List_s			/* pid list			*/
{
	struct List_s*	next;		/* next in list			*/
	char**		argv;		/* , separated string vector	*/
	int		argc;		/* elementsof(argv)		*/
} List_t;

typedef struct Ps_s			/* process state		*/
{
	Dtlink_t	hashed;		/* pid hash link		*/
	Dtlink_t	sorted;		/* sorted link			*/
	Pssent_t*	ps;		/* ps info			*/
	struct Ps_s*	children;	/* child list			*/
	struct Ps_s*	lastchild;	/* end of children		*/
	struct Ps_s*	sibling;	/* sibling list			*/
	struct Ps_s*	root;		/* (partial) root list		*/
	char*		user;		/* user name			*/
	Pss_id_t	pid;		/* pid				*/
	int		level;		/* process tree level		*/
	int		seen;		/* already seen on chain	*/
	int		shown;		/* list state			*/
} Ps_t;

typedef struct State_s			/* program state		*/
{
	int		children;	/* recursively list all children*/
	int		escape;		/* escape { command args }	*/
	int		heading;	/* output heading		*/
	int		parents;	/* recursively list all parents	*/
	int		tree;		/* list proc tree		*/
	int		hex;		/* output optional hex key form	*/
	int		width;		/* output width			*/
	unsigned long	now;		/* current time			*/
	Key_t*		fields;		/* format field list		*/
	List_t*		pids;		/* pid vectors			*/
	char*		format;		/* sfkeyprintf() format		*/
	Key_t*		lastfield;	/* end of format list		*/
	Dt_t*		keys;		/* format keys			*/
	Dt_t*		bypid;		/* procs by pid			*/
	Dt_t*		byorder;	/* procs by pid			*/
	Ps_t*		pp;		/* next proc info slot		*/
	Pss_t*		pss;		/* ps stream			*/
	Pssdisc_t	pssdisc;	/* ps stream discipline		*/
	Sfio_t*		mac;		/* temporary string stream	*/
	Sfio_t*		tmp;		/* temporary string stream	*/
	Sfio_t*		wrk;		/* temporary string stream	*/
	char		branch[1024];	/* process tree branch		*/
	char		buf[1024];	/* work buffer			*/
} State_t;

static Key_t	keys[] =
{
	{
		0
	},
	{
		"addr",
		"ADDR",
		"Physical address.",
		PSS_addr,
		KEY_addr,
		8
	},
	{
		"class",
		"CLS",
		"Scheduling class.",
		PSS_sched,
		KEY_class,
		3,
	},
	{
		"cmd",
		"CMD",
		"Command path with arguments.",
		PSS_args,
		KEY_cmd,
		-32, 0,
		0,0,0,
		KEY_comm
	},
	{
		"comm",
		"COMMAND",
		"Command file base name.",
		PSS_command,
		KEY_comm,
		-24, 0,
		0,0,0,
		KEY_cmd
	},
	{
		"cpu",
		"%CPU",
		"Cpu percent usage.",
		PSS_cpu,
		KEY_cpu,
		4
	},
	{
		"etime",
		"ELAPSED",
		"Elapsed time since start.",
		0,
		KEY_etime,
		7
	},
	{
		"flags",
		"F",
		"State flags (octal and additive).",
		PSS_flags,
		KEY_flags,
		3
	},
	{
		"gid",
		"GROUP",
		"Numeric group id.",
		PSS_gid,
		KEY_gid,
		8, 0,
		0,0,0,
		KEY_group
	},
	{
		"group",
		"GROUP",
		"Group id name.",
		PSS_gid,
		KEY_group,
		8, 0,
		0,0,0,
		KEY_gid
	},
	{
		"job",
		"JOB",
		"Job id.",
		PSS_job,
		KEY_job,
		5, PID_MAX,
		1,0,0
	},
	{
		"nice",
		"NI",
		"Adjusted scheduling priority.",
		PSS_nice,
		KEY_nice,
		4
	},
	{
		"npid",
		"NPID",
		"Native process id.",
		PSS_npid,
		KEY_npid,
		5, 0,
		1,
	},
	{
		"pgrp",
		"PGRP",
		"Process group id.",
		PSS_pgrp,
		KEY_pgrp,
		5, PID_MAX,
		1,0,0
	},
	{
		"pid",
		"PID",
		"Process id.",
		PSS_pid,
		KEY_pid,
		5, PID_MAX,
		1,0,0
	},
	{
		"ppid",
		"PPID",
		"Parent process id.",
		PSS_ppid,
		KEY_ppid,
		5, PID_MAX,
		1
	},
	{
		"pri",
		"PRI",
		"Scheduling priority.",
		PSS_pri,
		KEY_pri,
		3
	},
	{
		"processor",
		"PROC",
		"Assigned processor.",
		PSS_proc,
		KEY_proc,
		3
	},
	{
		"refcount",
		"REFS",
		"Reference count.",
		PSS_refcount,
		KEY_refcount,
		4, 0,
		1,
	},
	{
		"rss",
		"RSS",
		"Resident page set size in kilobytes.",
		PSS_rss,
		KEY_rss,
		5
	},
	{
		"sid",
		"SID",
		"Session id.",
		PSS_sid,
		KEY_sid,
		5, PID_MAX,
		1
	},
	{
		"size",
		"SIZE",
		"Virtual memory size in kilobytes.",
		PSS_size,
		KEY_size,
		6
	},
	{
		"start",
		"START",
		"Start time.",
		PSS_start,
		KEY_start,
		8
	},
	{
		"state",
		"S",
		"Basic state.",
		PSS_state,
		KEY_state,
		1
	},
	{
		"tgrp",
		"TGRP",
		"Terminal group id.",
		PSS_tgrp,
		KEY_tgrp,
		5, PID_MAX,
		1,
	},
	{
		"time",
		"TIME",
		"usr+sys time.",
		PSS_time,
		KEY_time,
		6
	},
	{
		"tty",
		"TT",
		"Controlling terminal base name.",
		PSS_tty,
		KEY_tty,
		-7,
	},
	{
		"uid",
		"USER",
		"Numeric user id.",
		PSS_uid,
		KEY_uid,
		8, 0,
		0,0,0,
		KEY_user
	},
	{
		"user",
		"USER",
		"User id name.",
		PSS_uid,
		KEY_user,
		8, 0,
		0,0,0,
		KEY_uid
	},
	{
		"wchan",
		"WCHAN",
		"Wait address.",
		PSS_wchan,
		KEY_wchan,
		8
	},

	/* aliases after this point */

	{ "args",	0,	0,	0,	KEY_cmd			},
	{ "command",	0,	0,	0,	KEY_cmd			},
	{ "f",		0,	0,	0,	KEY_flags		},
	{ "jid",	0,	0,	0,	KEY_job			},
	{ "ntpid",	0,	0,	0,	KEY_npid		},
	{ "pcpu",	0,	0,	0,	KEY_cpu			},
	{ "pgid",	0,	0,	0,	KEY_pgrp		},
	{ "proc",	0,	0,	0,	KEY_proc		},
	{ "psr",	0,	0,	0,	KEY_proc		},
	{ "rgroup",	0,	0,	0,	KEY_group		},
	{ "ruser",	0,	0,	0,	KEY_user		},
	{ "s",		0,	0,	0,	KEY_state		},
	{ "sess",	0,	0,	0,	KEY_sid			},
	{ "stime",	0,	0,	0,	KEY_start		},
	{ "tid",	0,	0,	0,	KEY_tgrp		},
	{ "vsz",	0,	0,	0,	KEY_size		},

};

static const char	fields_default[] = FIELDS_default;
static const char	newline[] = "\n";
static const char	space[] = " ";

static State_t		state;

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register int	i;

	if (streq(s, "formats"))
		for (i = 1; i < elementsof(keys); i++)
		{
			sfprintf(sp, "[+%s?", keys[i].name);
			if (keys[i].head)
				sfprintf(sp, "%s The title string is \b%s\b and the default width is %d.%s]", keys[i].desc, keys[i].head, keys[i].width, (!keys[i].field || (state.pss->meth->fields & keys[i].field)) ? "" : " Not available on this system.");
			else
				sfprintf(sp, "Equivalent to \b%s\b.]", keys[keys[i].index].name);
		}
	return 0;
}

/*
 * return device id given tty base name
 */

static int
ttyid(const char* name)
{
	return pssttydev(state.pss, name);
}

/*
 * sfkeyprintf() lookup
 * handle==0 for heading
 */

static int
key(void* handle, register Sffmt_t* fp, const char* arg, char** ps, Sflong_t* pn)
{
	register Ps_t*		pp = (Ps_t*)handle;
	register char*		s = 0;
	register Sflong_t	n = 0;
	register Key_t*		kp;
	register int		i;
	int			j;
	unsigned long		u;

	static char		sbuf[2];

	if (!fp->t_str)
		return 0;
	if (!(kp = (Key_t*)dtmatch(state.keys, fp->t_str)))
	{
		if (*fp->t_str != '$')
		{
			error(3, "%s: unknown format key", fp->t_str);
			return 0;
		}
		if (!(kp = newof(0, Key_t, 1, strlen(fp->t_str) + 1)))
			error(ERROR_SYSTEM|3, "out of space");
		kp->name = strcpy((char*)(kp + 1), fp->t_str);
		kp->macro = getenv(fp->t_str + 1);
		kp->index = KEY_environ;
		kp->disable = 1;
		dtinsert(state.keys, kp);
	}
	if (!kp->head && (state.pss->meth->fields & keys[kp->index].field))
		kp = &keys[kp->index];
	if (kp->macro && !kp->disable)
	{
		kp->disable = 1;
		sfkeyprintf(state.mac, handle, kp->macro, key, NiL);
		if (!(s = sfstruse(state.mac)))
			error(ERROR_SYSTEM|3, "out of space");
		kp->disable = 0;
	}
	else if (!pp)
	{
		if (!(state.pss->meth->fields & kp->field))
		{
			error(1, "%s: not available on this system", kp->name);
			return -1;
		}
		state.pssdisc.fields |= kp->field;
		if (fp->flags & SFFMT_ALTER)
		{
			if (kp->maxval)
			{
				for (i = 1; kp->maxval /= 10; i++);
				if (kp->width < 0)
				{
					i = -i;
					if (kp->width > i)
						kp->width = i;
				}
				else if (kp->width < i)
					kp->width = i;
			}
			if ((fp->width = kp->width) < 0)
			{
				fp->width = -fp->width;
				fp->flags |= SFFMT_LEFT;
			}
			fp->precis = fp->width;
		}
		kp->width = fp->width;
		if (fp->flags & SFFMT_LEFT)
			kp->width = -kp->width;
		fp->fmt = 's';
		*ps = arg && (arg = (const char*)strdup(arg)) ? (char*)arg : kp->head;
	}
	else
	{
		if ((fp->flags & SFFMT_ALTER) && (fp->width = kp->width) < 0)
		{
			fp->width = -fp->width;
			fp->flags |= SFFMT_LEFT;
		}
		switch (kp->index)
		{
		case KEY_addr:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			n = (long)pp->ps->addr;
			goto number;
		case KEY_class:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			s = pp->ps->sched;
			break;
		case KEY_cmd:
			s = pp->ps->args;
			goto branch;
		case KEY_comm:
			s = pp->ps->command;
		branch:
			if (!s)
				s = "<defunct>";
			if ((j = pp->level) > 0)
			{
				for (i = 0, j--; i < j; i++)
					sfputr(state.wrk, state.branch[i] ? " |  " : "    ", -1);
				sfputr(state.wrk, " \\_ ", -1);
				i = sfstrtell(state.wrk);
				sfputr(state.wrk, s, -1);
				if (!(s = sfstruse(state.wrk)))
					error(ERROR_SYSTEM|3, "out of space");
				if (state.escape)
					fmtesc(s + i);
			}
			break;
		case KEY_cpu:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			n = pp->ps->cpu;
			goto percent;
		case KEY_etime:
			if (fp->fmt == 's')
				s = fmtelapsed(state.now - (unsigned long)pp->ps->start, 1);
			else
				n = pp->ps->start;
			break;
		case KEY_flags:
			n = pp->ps->flags & PSS_FLAGS;
			goto number;
		case KEY_group:
			if (fp->fmt == 's')
			{
				s = fmtgid(pp->ps->gid);
				break;
			}
			/*FALLTHROUGH*/
		case KEY_gid:
			n = pp->ps->gid;
			goto number;
		case KEY_nice:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			n = pp->ps->nice;
			goto number;
		case KEY_npid:
			n = pp->ps->npid;
			goto number;
		case KEY_pgrp:
			n = pp->ps->pgrp;
			goto number;
		case KEY_pid:
			n = pp->ps->pid;
			goto number;
		case KEY_ppid:
			n = pp->ps->ppid;
			break;
		case KEY_pri:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			n = pp->ps->pri;
			goto number;
		case KEY_refcount:
			n = pp->ps->refcount;
			goto number;
		case KEY_rss:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			n = pp->ps->rss;
			goto number;
		case KEY_sid:
			n = pp->ps->sid;
			goto number;
		case KEY_size:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			n = pp->ps->size;
			goto number;
		case KEY_start:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			if (fp->fmt == 's')
			{
				u = pp->ps->start;
				s = fmttime((state.now - u) >= (24 * 60 * 60) ? "%y-%m-%d" : "%H:%M:%S", u);
			}
			else
				n = pp->ps->start;
			break;
		case KEY_state:
			*(s = sbuf) = pp->ps->state;
			*(s + 1) = 0;
			break;
		case KEY_tgrp:
			n = pp->ps->tgrp;
			goto number;
		case KEY_time:
			if (fp->fmt == 's')
				s = fmtelapsed(pp->ps->time, 1);
			else
				n = pp->ps->time;
			break;
		case KEY_tty:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			s = pssttyname(state.pss, pp->ps);
			if (kp->prec && (i = strlen(s) - kp->prec) > 0)
			{
				if (s[0] == 'p' && s[1] == 't')
				{
					if (s[2] == 'y')
						s += 3;
					else
						s += 2;
				}
				else if (s[0] == 't' && s[1] == 't' && s[2] == 'y')
					s += 3;
				else
					s += i;
			}
			break;
		case KEY_user:
			if (fp->fmt == 's')
			{
				s = pp->user;
				break;
			}
			/*FALLTHROUGH*/
		case KEY_uid:
			n = pp->ps->uid;
			goto number;
		case KEY_wchan:
			if (pp->ps->state == PSS_ZOMBIE)
				goto zombie;
			n = (long)pp->ps->wchan;
			goto number;
		default:
			return 0;
		zombie:
			s = "";
			break;
		percent:
			sfprintf(state.tmp, "%%%I*d", sizeof(n), n);
			if (!(s = sfstruse(state.tmp)))
				error(ERROR_SYSTEM|3, "out of space");
			break;
		number:
			if (state.hex)
			{
				fp->fmt = 'x';
				if (!kp->hex)
					fp->flags |= SFFMT_ZERO;
			}
			break;
		}
		if (s)
			*ps = s;
		else
			*pn = n;
	}
	return 1;
}

/*
 * ps a single proc
 */

static void
ps(Ps_t* pp)
{
	register Key_t*		kp;
	register Pssent_t*	pr;
	register char*		s;
	register int		i;
	register long		n;
	unsigned long		u;
	int			j;
	char			sbuf[2];

	pp->shown = 1;
	if (state.format)
	{
		sfkeyprintf(sfstdout, pp, state.format, key, NiL);
		return;
	}
	pr = pp->ps;
	for (kp = state.fields; kp; kp = kp->next)
	{
		switch (kp->index)
		{
		case KEY_addr:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			n = (long)pr->addr;
			goto hex;
		case KEY_class:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			s = pr->sched;
			goto string;
		case KEY_cmd:
			s = pr->args;
			goto branch;
		case KEY_comm:
			s = pr->command;
		branch:
			if (!s)
				s = "<defunct>";
			if ((j = pp->level) > 0)
			{
				for (i = 0, j--; i < j; i++)
					sfputr(sfstdout, state.branch[i] ? " |  " : "    ", -1);
				sfputr(sfstdout, " \\_ ", -1);
			}
			if (state.escape)
				s = fmtesc(s);
			goto string;
		case KEY_cpu:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			n = pr->cpu;
			goto percent;
		case KEY_etime:
			s = fmtelapsed(state.now - (unsigned long)pr->start, 1);
			goto string;
		case KEY_flags:
			n = pr->flags & PSS_FLAGS;
			goto octal;
		case KEY_gid:
			n = pr->gid;
			goto number;
		case KEY_group:
			s = fmtgid(pr->gid);
			goto string;
		case KEY_nice:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			n = pr->nice;
			goto number;
		case KEY_npid:
			n = pr->npid;
			goto number;
		case KEY_pgrp:
			n = pr->pgrp;
			goto number;
		case KEY_pid:
			n = pr->pid;
			goto number;
		case KEY_ppid:
			n = pr->ppid;
			goto number;
		case KEY_pri:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			n = pr->pri;
			goto number;
		case KEY_refcount:
			n = pr->refcount;
			goto number;
		case KEY_rss:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			n = pr->rss;
			goto number;
		case KEY_sid:
			n = pr->sid;
			goto number;
		case KEY_size:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			n = pr->size;
			goto number;
		case KEY_start:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			u = pr->start;
			s = fmttime((state.now - u) >= (24 * 60 * 60) ? "%y-%m-%d" : "%H:%M:%S", u);
			goto string;
		case KEY_state:
			*(s = sbuf) = pr->state;
			*(s + 1) = 0;
			goto string;
		case KEY_tgrp:
			n = pr->tgrp;
			goto number;
		case KEY_time:
			s = fmtelapsed(pr->time, 1);
			goto string;
		case KEY_tty:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			s = pssttyname(state.pss, pr);
			if (kp->prec && (i = strlen(s) - kp->prec) > 0)
			{
				if (s[0] == 'p' && s[1] == 't')
				{
					if (s[2] == 'y')
						s += 3;
					else
						s += 2;
				}
				else if (s[0] == 't' && s[1] == 't' && s[2] == 'y')
					s += 3;
				else
					s += i;
			}
			goto string;
		case KEY_uid:
			n = pr->uid;
			goto number;
		case KEY_user:
			s = pp->user;
			goto string;
		case KEY_wchan:
			if (pr->state == PSS_ZOMBIE)
				goto zombie;
			n = (long)pr->wchan;
			goto hex;
		}
		s = "????";
	string:
		if (kp->width == kp->prec)
			sfprintf(sfstdout, "%0*s%s", kp->width, s, kp->sep);
		else
			sfprintf(sfstdout, "%*.*s%s", kp->width, kp->prec, s, kp->sep);
		continue;
	zombie:
		s = "-";
		goto string;
	percent:
		sfprintf(state.tmp, "%%%ld", n);
		if (!(s = sfstruse(state.tmp)))
			error(ERROR_SYSTEM|3, "out of space");
		goto string;
	number:
		if (!state.hex || !kp->hex)
		{
			sfprintf(sfstdout, "%*ld%s", kp->width, n, kp->sep);
			continue;
		}
	hex:
		if (kp->hex)
			sfprintf(sfstdout, "%*lx%s", kp->width, n, kp->sep);
		else
			sfprintf(sfstdout, "%0*lx%s", kp->width, n, kp->sep);
		continue;
	octal:
		sfprintf(sfstdout, "%*lo%s", kp->width, n, kp->sep);
		continue;
	}
}

/*
 * ps() a process and its children
 */

static void
kids(register Ps_t* pp, int level)
{
	if (state.tree)
	{
		pp->level = level;
		ps(pp);
		if (level > 0)
			state.branch[level - 1] = pp->sibling != 0;
		if (level < elementsof(state.branch) - 1)
			level++;
	}
	else
		ps(pp);
	for (pp = pp->children; pp; pp = pp->sibling)
		kids(pp, level);
}

/*
 * ps() the selected procs
 */

static void
list(void)
{
	register Ps_t*	pp;
	register Ps_t*	xp;
	register Ps_t*	zp;
	Ps_t*		rp;

	if (state.children || state.parents)
	{
		/*
		 * list the child/parent branches of selected processes
		 */

		if (state.parents)
			for (pp = (Ps_t*)dtfirst(state.byorder); pp; pp = (Ps_t*)dtnext(state.byorder, pp))
				if (pp->ps->pss & (PSS_EXPLICIT|PSS_MATCHED))
				{
					xp = pp;
					do
					{
						xp->ps->pss |= PSS_PARENT;
					} while ((xp = (Ps_t*)dtmatch(state.bypid, &xp->ps->ppid)) && !xp->ps->pss);
				}
		if (state.children)
			for (pp = (Ps_t*)dtfirst(state.byorder); pp; pp = (Ps_t*)dtnext(state.byorder, pp))
				if (!(pp->ps->pss & (PSS_EXPLICIT|PSS_MATCHED|PSS_PARENT)) && !pp->seen)
				{
					xp = pp;
					do
					{
						xp->seen = 1;
						if (xp->ps->pss & (PSS_ANCESTOR|PSS_EXPLICIT|PSS_MATCHED))
						{
							xp->ps->pss |= PSS_ANCESTOR;
							xp = pp;
							do
							{
								xp->ps->pss |= PSS_PARENT;
							} while ((xp = (Ps_t*)dtmatch(state.bypid, &xp->ps->ppid)) && !xp->ps->pss);
							break;
						}
					} while (xp->ps->ppid != xp->ps->pid && (xp = (Ps_t*)dtmatch(state.bypid, &xp->ps->ppid)));
				}
		rp = zp = 0;
		for (pp = (Ps_t*)dtfirst(state.byorder); pp; pp = (Ps_t*)dtnext(state.byorder, pp))
			if (pp->ps->pss)
			{
				if (pp->ps->ppid != pp->ps->pid && (xp = (Ps_t*)dtmatch(state.bypid, &pp->ps->ppid)) && (xp->ps->pss & (PSS_EXPLICIT|PSS_MATCHED|PSS_PARENT)))
				{
					xp->ps->pss |= PSS_CHILD;
					if (xp->lastchild)
						xp->lastchild = xp->lastchild->sibling = pp;
					else
						xp->children = xp->lastchild = pp;
				}
				else if (zp)
					zp = zp->root = pp;
				else
					rp = zp = pp;
			}
		for (pp = rp; pp; pp = pp->root)
			kids(pp, 0);
	}
	else
	{
		/*
		 * list by order
		 */

		for (pp = (Ps_t*)dtfirst(state.byorder); pp; pp = (Ps_t*)dtnext(state.byorder, pp))
			if (!pp->shown)
			{
				pp->shown = 1;
				pp->level = 0;
				ps(pp);
			}
	}
}

/*
 * finalize the field formats and optionally list the heading
 */

static void
head(void)
{
	register int	n;
	register Key_t*	kp;

	if (state.fields)
	{
		while (state.fields->skip)
			state.fields = state.fields->next;
		kp = state.fields;
		while (kp->next)
		{
			if (!kp->next->skip)
				kp = kp->next;
			else if (!(kp->next = kp->next->next))
			{
				state.lastfield = kp;
				break;
			}
		}
		n = 0;
		for (kp = state.fields; kp; kp = kp->next)
		{
			if ((kp->prec = kp->width) < 0)
				kp->prec = -kp->prec;
			if (*kp->head)
				n = 1;
		}
		kp = state.lastfield;
		if (kp->width < 0)
			kp->width = 0;
		if (kp->index == KEY_cmd)
			kp->prec = 80;
		if (n && state.heading)
		{
			for (kp = state.fields; kp; kp = kp->next)
				sfprintf(sfstdout, "%*s%s", kp->width, kp->head, kp->sep);
			sfputc(sfstdout, '\n');
		}
	}
	else
	{
		sfkeyprintf(state.heading ? sfstdout : state.wrk, NiL, state.format, key, NiL);
		sfstrseek(state.wrk, 0, SEEK_SET);
	}
}

/*
 * order procs by <uid,start,pid>
 */

static int
byorder(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	register Ps_t*	pa = (Ps_t*)a;
	register Ps_t*	pb = (Ps_t*)b;
	register int	i;

	NoP(dt);
	NoP(disc);
	if (i = strcmp(pa->user, pb->user))
		return i;
	if (pa->ps->pgrp < pb->ps->pgrp)
		return -1;
	if (pa->ps->pgrp > pb->ps->pgrp)
		return 1;
	if (i = (pa->ps->pgrp == pa->ps->pid) - (pb->ps->pgrp == pb->ps->pid))
		return i;
	if (pa->ps->start < pb->ps->start)
		return -1;
	if (pa->ps->start > pb->ps->start)
		return 1;
	if (pa->ps->pid < pb->ps->pid)
		return -1;
	if (pa->ps->pid > pb->ps->pid)
		return 1;
	return 0;
}

/*
 * add the procs in the pid list
 */

static void
addpid(Pssent_t* pe, register char* s)
{
	register char*		t;
	register Ps_t*		pp;
	register int		c;
	char*			e;
	Pss_id_t		pid;
	long			n;

	do
	{
		if (s)
		{
			for (; isspace(*s) || *s == ','; s++);
			for (t = s; *s && !isspace(*s) && *s != ','; s++);
			c = *s;
			*s = 0;
			if (!*t)
				break;
			errno = 0;
			n = strtol(t, &e, 0);
// deleting check for PID_MAX because on linux this relies on kernel instance
// and the system will reject bad pids any way
//			if (errno || n <= 0 || n > PID_MAX || *e)
			if (errno || n <= 0 || *e)
			{
				error(2, "%s: invalid pid", t);
				continue;
			}
			pid = n;
			pe = pssread(state.pss, pid);
		}
		if (pe)
		{
			if (!(pp = state.pp) && !(state.pp = pp = newof(0, Ps_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space");
			pp->user = fmtuid(pe->uid);
			pp->pid = pe->pid;
			pp->ps = pe;
			if (!dtsearch(state.byorder, pp))
			{
				if (!(pp->ps = psssave(state.pss, pe)))
					break;
				dtinsert(state.byorder, pp);
				state.pp = 0;
			}
		}
	} while (s && (*s++ = c));
}

/*
 * add the ids in s into state.pssdisc.match
 * getid!=0 translates alnum to id number
 */

static void
addid(register char* s, int index, int (*getid)(const char*))
{
	register char*	t;
	register int	c;
	char*		e;
	long		n;
	unsigned long	field;
	Pssmatch_t*	mp;
	Pssdata_t*	dp;

	if (!((field = keys[index].field) & PSS_match))
		error(3, "%s: cannot match on this field", keys[index].name);
	for (mp = state.pssdisc.match; mp && mp->field != field; mp = mp->next);
	if (!mp)
	{
		if (!(mp = newof(0, Pssmatch_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space");
		mp->next = state.pssdisc.match;
		mp->field = field;
		state.pssdisc.match = mp;
	}
	do
	{
		for (; isspace(*s) || *s == ','; s++);
		for (t = s; *s && !isspace(*s) && *s != ','; s++);
		if ((c = s - t) >= sizeof(state.buf))
			c = sizeof(state.buf) - 1;
		memcpy(state.buf, t, c);
		(t = state.buf)[c] = 0;
		if (!*t)
			break;
		if (isdigit(*t))
		{
			n = strtol(t, &e, 10);
			if (*e)
			{
				error(1, "%s: invalid %s", t, keys[index].name);
				continue;
			}
		}
		else if (!getid || (n = (*getid)(t)) < 0)
		{
			error(1, "%s: invalid %s", t, keys[index].name);
			continue;
		}
		if (!(dp = newof(0, Pssdata_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space");
		dp->next = mp->data;
		mp->data = dp;
		dp->data = n;
	} while (*s++);
}

/*
 * add the format key in s into state.fields
 */

static void
addkey(const char* k, int ignore)
{
	register char*	s = (char*)k;
	register char*	t;
	register int	c;
	register Key_t*	kp;
	register Key_t*	ap;
	char*		e;
	int		w;

	if (streq(s, "?"))
	{
		sfprintf(sfstdout, "%-8s %-8s %s\n", "KEY", "HEADING", "DESCRIPTION");
		for (kp = keys + 1; kp < keys + elementsof(keys); kp++)
		{
			ap = kp->head ? kp : (keys + kp->index);
			sfprintf(sfstdout, "%-8s %-8s %s%s%s\n", kp->name, ap->head, ap->desc, ap == kp ? "" : " [alias]", (!ap->field || (state.pss->meth->fields & ap->field)) ? "" : " [not available]");
		}
		exit(0);
	}
	do
	{
		for (; isspace(*s) || *s == ','; s++);
		for (t = s; *s && !isspace(*s) && *s != ',' && *s != '=' && *s != ':' && *s != '+'; s++);
		if ((c = s - t) >= sizeof(state.buf))
			c = sizeof(state.buf) - 1;
		memcpy(state.buf, t, c);
		(t = state.buf)[c] = 0;
		if (!*t)
			break;
		if (*s == ':' || *s == '+')
		{
			c = (int)strtol(s + 1, &e, 10);
			s = e;
		}
		else
			c = 0;
		if (!(kp = (Key_t*)dtmatch(state.keys, t)))
		{
			error(2, "%s: unknown format key", t);
			continue;
		}

		/*
		 * aliases have Key_t.head == 0
		 */

		if (!kp->head)
			kp = keys + kp->index;

		/*
		 * adjust the width field
		 */

		if (*s == '=')
		{
			for (t = ++s; *s && !isspace(*s) && *s != ','; s++);
			w = s - t;
			if (!(kp->head = newof(0, char, w, 1)))
				error(ERROR_SYSTEM|3, "out of space");
			memcpy(kp->head, t, w);
			if (w < c)
				w = c;
			if (w > kp->width)
				kp->width = w;
		}
		else if (c >= strlen(kp->head))
			kp->width = c;
		if (!(state.pss->meth->fields & kp->field))
		{
			if (!ignore)
				error(1, "%s: not available on this system", kp->name);
			continue;
		}

		/*
		 * except for width and head adjustments
		 * we ignore keys already specified to let
		 * shell aliases work with least suprise
		 */

		if (!kp->already)
		{
			kp->already = keys[kp->cancel].already = keys[kp->cancel].skip = 1;
			if (state.lastfield)
				state.lastfield = state.lastfield->next = kp;
			else
				state.fields = state.lastfield = kp;
			kp->sep = space;
			if (kp->maxval)
			{
				for (c = 1; kp->maxval /= 10; c++);
				if (c >= kp->width)
					kp->width = c;
			}
			state.pssdisc.fields |= kp->field;
		}
	} while (*s != '=' && *s++);
}

/*
 * add pid vector for subsequent poppids()
 */

static void
pushpids(void* argv, int argc)
{
	register List_t*	p;

	if (!(p = newof(0, List_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	p->argv = (char**)argv;
	p->argc = argc;
	p->next = state.pids;
	state.pids = p;
}

/*
 * pop state.pids by calling addpid()
 */

static void
poppids(void)
{
	register List_t*	p;
	register int		i;
	unsigned long		flags;

	flags = state.pssdisc.flags;
	state.pssdisc.flags |= PSS_VERBOSE;
	while (p = state.pids)
	{
		state.pids = p->next;
		if (i = p->argc)
			while (--i >= 0)
				addpid(NiL, p->argv[i]);
		else
			addpid(NiL, (char*)p->argv);
		free(p);
	}
	state.pssdisc.flags = flags;
}

int
main(int argc, register char** argv)
{
	register int	n;
	register char*	s;
	Sfio_t*		fmt;
	Key_t*		kp;
	Ps_t*		pp;
	Pssent_t*	pe;
	Dtdisc_t	kd;
	Dtdisc_t	nd;
	Dtdisc_t	sd;
	Optdisc_t	od;
	struct stat	st;

	NoP(argc);
	error_info.id = "ps";
	setlocale(LC_ALL, "");
	state.now = time((time_t*)0);
	state.escape = 1;
	state.heading = 1;
	if (!(fmt = sfstropen()) || !(state.tmp = sfstropen()) || !(state.wrk = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space");

	/*
	 * set up the disciplines
	 */

	pssinit(&state.pssdisc, argv[0], errorf);
	optinit(&od, optinfo);
	memset(&kd, 0, sizeof(kd));
	kd.key = offsetof(Key_t, name);
	kd.size = -1;
	kd.link = offsetof(Key_t, hashed);
	memset(&nd, 0, sizeof(nd));
	nd.key = offsetof(Ps_t, pid);
	nd.size = sizeof(Pss_id_t);
	nd.link = offsetof(Ps_t, hashed);
	memset(&sd, 0, sizeof(sd));
	sd.link = offsetof(Ps_t, sorted);
	sd.comparf = byorder;

	/*
	 * open the ps stream
	 */

	if (!(state.pss = pssopen(&state.pssdisc)) || !state.pss->meth->fields)
	{
		s = "/bin/ps";
		if (!streq(argv[0], s) && !eaccess(s, X_OK))
		{
			argv[0] = s;
			error(1, "falling back to %s", s);
			execv(s, argv);
			exit(EXIT_NOTFOUND);
		}
		if (!state.pss)
		{
			state.pssdisc.flags |= PSS_VERBOSE;
			pssopen(&state.pssdisc);
		}
		error(3, "process status access error");
	}

	/*
	 * initialize the format key table
	 */

	if (!(state.keys = dtopen(&kd, Dtset)) || !(state.bypid = dtopen(&nd, Dtset)) || !(state.byorder = dtopen(&sd, Dtset)))
		error(ERROR_SYSTEM|3, "out of space");
	for (n = 1; n < elementsof(keys); n++)
		dtinsert(state.keys, keys + n);

	/*
	 * grab the options
	 */

	n = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			state.pssdisc.flags |= PSS_ATTACHED|PSS_NOLEADER;
			continue;
		case 'c':
			addkey(FIELDS_c, 1);
			continue;
		case 'd':
			state.pssdisc.flags |= PSS_NOLEADER;
			continue;
		case 'e':
		case 'A':
			state.pssdisc.flags |= PSS_ALL;
			continue;
		case 'f':
			addkey(FIELDS_f, 1);
			continue;
		case 'h':
			state.heading = opt_info.num;
			continue;
		case 'g':
			addid(opt_info.arg, KEY_pgrp, NiL);
			continue;
		case 'j':
			addkey(FIELDS_j, 1);
			continue;
		case 'l':
			addkey(FIELDS_l, 1);
			continue;
		case 'n':
			continue;
		case 'o':
			addkey(opt_info.arg, 0);
			continue;
		case 'p':
			pushpids(opt_info.arg, 0);
			continue;
		case 'r':
		case 'R':
			state.children = opt_info.num;
			continue;
		case 's':
			addid(opt_info.arg, KEY_sid, NiL);
			continue;
		case 't':
			addid(opt_info.arg, KEY_tty, ttyid);
			continue;
		case 'u':
			addid(opt_info.arg, KEY_uid, struid);
			continue;
		case 'v':
			state.pssdisc.flags |= PSS_VERBOSE;
			continue;
		case 'w':
			/* ignored by this implementation */
			continue;
		case 'x':
			state.pssdisc.flags |= PSS_DETACHED;
			continue;
		case 'B':
			n = !opt_info.num;
			continue;
		case 'C':
			state.tree = state.children = opt_info.num;
			continue;
		case 'D':
			if (s = strchr(opt_info.arg, '='))
				*s++ = 0;
			if (*opt_info.arg == 'n' && *(opt_info.arg + 1) == 'o')
			{
				opt_info.arg += 2;
				s = 0;
			}
			if (!(kp = (Key_t*)dtmatch(state.keys, opt_info.arg)))
			{
				if (!s)
					continue;
				if (!(kp = newof(0, Key_t, 1, strlen(opt_info.arg) + 1)))
					error(ERROR_SYSTEM|3, "out of space");
				kp->name = strcpy((char*)(kp + 1), opt_info.arg);
				dtinsert(state.keys, kp);
			}
			if (kp->macro = s)
				stresc(s);
			continue;
		case 'E':
			state.escape = opt_info.num;
			continue;
		case 'F':
			if (sfstrtell(fmt))
				sfputc(fmt, ' ');
			sfputr(fmt, opt_info.arg, -1);
			continue;
		case 'G':
			addid(opt_info.arg, KEY_group, strgid);
			continue;
		case 'L':
			state.pssdisc.flags |= PSS_LEADER;
			continue;
		case 'N':
			addkey(fields_default, 1);
			continue;
		case 'P':
			state.tree = state.parents = opt_info.num;
			continue;
		case 'T':
			state.tree = state.parents = state.children = opt_info.num;
			continue;
		case 'X':
			state.hex = !state.hex;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (n)
		state.tree = 0;
	if (sfstrtell(fmt))
	{
		sfputc(fmt, '\n');
		if (!(state.format = sfstruse(fmt)))
			error(ERROR_SYSTEM|3, "out of space");
	}
	else
	{
		sfclose(fmt);
		fmt = 0;
		if (!state.fields)
			addkey(fields_default, 1);
	}
	head();

	/*
	 * add each proc by name
	 */

	if (*argv)
		pushpids(argv, argc);
	if (state.children || state.parents || !state.pids)
	{
		if (state.children || state.parents)
			state.pssdisc.flags |= PSS_UNMATCHED;
		if (!state.pids && !state.pssdisc.match && !(state.pssdisc.flags & (PSS_ALL|PSS_ATTACHED|PSS_DETACHED|PSS_LEADER|PSS_NOLEADER)))
		{
			state.pssdisc.flags |= PSS_UID;
			state.pssdisc.uid = geteuid();
			for (n = 0; n <= 2; n++)
				if (isatty(n) && !fstat(n, &st))
				{
					state.pssdisc.flags |= PSS_TTY;
					state.pssdisc.tty = st.st_rdev;
					break;
				}
		}
		poppids();
		while (pe = pssread(state.pss, PSS_SCAN))
			addpid(pe, NiL);
		if (state.children || state.parents)
			for (pp = (Ps_t*)dtfirst(state.byorder); pp; pp = (Ps_t*)dtnext(state.byorder, pp))
				dtinsert(state.bypid, pp);
	}
	else
		poppids();

	/*
	 * list the procs
	 */

	if (state.fields)
		state.lastfield->sep = newline;
	list();
	pssclose(state.pss);
	return error_info.errors != 0;
}
