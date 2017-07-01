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
 * make - maintain and update programs
 *
 * history
 *
 *   1970's                   pre-make           (research)
 *                                .
 *                                .
 *   1976                       make             S. I. Feldman
 *                              .  .
 *                            .     .
 *                          .        .
 *                        .           .
 *   1978        augmented-make        .         E. G. Bradford
 *                    . . .             .
 *                  .   .   .           .
 *                .     .     .         .
 *              .       .       .       .
 *   1979     build     .         .     .        V. B. Erickson, J. F. Pellegrin
 *              .       .           .   .
 *              .       .             . .
 *   1984       .       .            V8-make     S. I. Feldman
 *              .       .             . .
 *              .       .           .   .
 *              .       .         .     .
 *              .       .       .       .
 *              .       .     .         .
 *              .       .   .           .
 *              .       . .             .
 *   1985       .     nmake             .        G. S. Fowler
 *              .     . .               .
 *              .   .   .               .
 *              . .     .               .
 *   1986    LC-nmake   .               .        J. E. Shopiro, M. S. Freeland
 *              . .     .               . 
 *              .   .   .               mk       A. G. Hume
 *              .     . .             . .
 *   1987       .     nmake         .   .        G. S. Fowler
 *              .       .         .     .
 *            (1.4)     .       .       .
 *                .     .     .         .
 *                  .   .   .           .
 *                    . . .             .
 *   1988             nmake             .        G. S. Fowler
 *                      .               .
 *                    (2.0)             .
 *                      .               .
 *   1990             (2.1)             .
 *                      .               .
 *   1991             (2.2)             .
 *                      .               .
 *   1992             (2.3)             .
 *                      .               .
 *   1993             (3.0)             .
 *                      .               .
 *   1994             (3.1)             .
 *                      .               .
 *   1995             (3.2)             .	AT&T Bell Labs
 *                      .               .
 *   1996             (3.3)             .	AT&T Research (Lucent split)
 *                      .               .
 *   1997             (3.4)             .	AT&T Research
 *                      .               .
 *   1998             (3.5)             .	AT&T Research
 *                      .               .
 *   1999             (3.6)             .	AT&T Research
 *                      .               .
 *   2000             (4.0)             .	AT&T Research
 *                      .               .
 *   2001             (4.1)             .	AT&T Research
 *   2002             (4.2)             .	AT&T Research
 *   2003             (4.3)             .	AT&T Research
 *   2004             (4.4)             .	AT&T Research
 *   2005             (5.0)             .	AT&T Research
 *   2006             (5.1)             .	AT&T Research
 *   2007             (5.2)             .	AT&T Research
 *   2008             (5.3)             .	AT&T Research
 *   2009             (5.4)             .	AT&T Research
 *   2010             (5.5)             .	AT&T Research
 *   2011             (5.6)             .	AT&T Research
 *   2012             (5.7)             .	AT&T Research
 *
 * command line arguments are of three types
 *
 *	make [ [ option ] [ script ] [ target ] ... ]
 *
 * options are described in options.h
 *
 * debug trace levels are controlled by error_info.trace
 * debug level n enables tracing for all levels less than or equal to n
 * levels 4 and higher are conditionally compiled with DEBUG!=0
 * if debug level < 20 then all levels are disabled during early bootstrap
 * if debug level = 1 then all levels are disabled until after .INIT is made
 *
 *	level	trace
 *	  0	no trace
 *	  1	basic make trace
 *	  2	major actions
 *	  3	option settings, make object files
 *	  4	state variables
 *	  5	directory and archive scan, some OPT_explain'ations
 *	  6	coshell commands and messages
 *	  7	makefile input lines
 *	  8	lexical analyzer states
 *	  9	metarules applied
 *	 10	variable expansions
 *	 11	bindfile() trace
 *	 12     files added to hash
 *	 13	new atoms
 *	 14	hash table usage
 *		...
 *	 20	bootstrap trace
 *
 * state.questionable registry (looks like it should go but not sure)
 *
 *	0x00000001 *temporary*
 *	0x00000002 *temporary*
 *	0x00000004 *temporary* disable bind_p "- ..." and "... ..." virtual logic
 *	0x00000008 1996-10-11 meta rhs prefix dir check even if rhs has prefix
 *	0x00000010 1994-01-01 old out of date if prereq in higher view
 *	0x00000020 1994-01-01 old scan advance logic
 *	0x00000040 2001-10-20 foiled alias still allows bind (!1994-08-11)
 *	0x00000080 1994-08-11 don't catrule() ../.. dir prefixes in bindfile()
 *	0x00000100 1994-10-01 metaclose() recursion even if rule.action!=0
 *	0x00000200 1994-11-11 disable generate() % transformation check
 *	0x00000400 1995-01-19 forceread even if global
 *	0x00000800 1995-07-17 D_global if not P_target in bindfile()
 *	0x00001000 1995-07-17 less agressive aliasing in bindfile()
 *	0x00002000 1995-07-17 less agressive putbound() in bindalias()
 *	0x00004000 1995-11-11 P_target && D_dynamic does not imply generated
 *	0x00008000 1996-02-29 :P=D: returns at most one dir per item
 *	0x00010000 1996-10-11 don't alias check path suffixes in bindfile()
 *	0x00020000 1998-11-11 don't force require_p make() recheck
 *	0x00040000 1999-09-07 generate() intermediates only if (sep & LT)
 *	0x00080000 1999-11-19 disable touch steady state loop
 *	0x00100000 2000-02-14 apply metarule even if dir on unbound lhs
 *	0x00200000 2000-09-08 0x00100000 implied if !P_implicit
 *	0x00400000 2001-02-14 allow P_archive to break job deadlock
 *	0x00800000 2001-10-05 disable early r->status=FAILED when errors!=0
 *	0x01000000 2002-10-02 $(>) doesn't check min(rule.time,state.time)
 *	0x02000000 2002-12-04 don't inhibit .UNBIND of M_bind rules
 *	0x04000000 2004-01-20 don't include triggered time==0 targets in :T=F: 
 *	0x08000000 2004-10-01 metaget does not assume % target for ... : % .NULL
 *	0x10000000 2007-01-08 :P=D: alias check
 *	0x20000000 2007-06-15 disable :W: . alias check
 *	0x40000000 2007-06-20 don't include intermediate makefile dirs in :W:
 *	0x80000000 2007-08-28 don't force reassoc bindalias()
 *
 * state.test registry (conditionally compiled with DEBUG!=0)
 *
 *	0x00000001 *temporary*
 *	0x00000002 *temporary*
 *	0x00000004 *temporary*
 *	0x00000008 bindfile() directory prune trace
 *	0x00000010 expand() trace
 *	0x00000020 force external archive table of contents generation
 *	0x00000040 bindalias() trace
 *	0x00000080 scan state trace
 *	0x00000100 statetime() trace
 *	0x00000200 staterule() view trace
 *	0x00000400 more staterule() view trace
 *	0x00000800 mergestate() trace
 *	0x00001000 job status trace
 *	0x00002000 dump Vmheap stats on exit
 *	0x00004000 dump atom address with name
 *	0x00008000 force state file garbage collection
 *	0x00010000 alarm status trace
 *	0x00020000 unused
 *	0x00040000 set failed state|metarule event to staterule event
 *	0x00080000 replace ' ' with '?' instead of FILE_SPACE
 *	0x00100000 scan action trace
 */

#include "make.h"
#include "options.h"

#include <sig.h>
#include <stak.h>
#include <vecargs.h>
#include <vmalloc.h>

#define settypes(c,t)	for(s=c;*s;settype(*s++,t))

#ifndef PATHCHECK
#define PATHCHECK	IDNAME
#endif

/*
 * global initialization -- external engine names are defined in initrule()
 */

Internal_t		internal;	/* internal rules and vars	*/
State_t			state;		/* program state		*/
Tables_t		table;		/* hash table info		*/
char			null[] = "";	/* null string			*/
char			tmpname[MAXNAME];/* temporary name buffer	*/
short			ctypes[UCHAR_MAX+1];/* istype() character types	*/

/*
 * user error message intercept
 */

static int
intercept(Sfio_t* sp, int level, int flags)
{
	register Rule_t*	r;
	char*			m;
	char*			s;
	char*			t;
	char*			e;
	int			n;
	int			i;
	Sfio_t*			tmp;

	NoP(sp);
	NoP(flags);
	if ((state.mam.level = level) > 0 && !state.hold && (r = internal.error) && (r->property & (P_functional|P_target)) == (P_functional|P_target) && !state.compileonly && !state.interrupt && (m = stakptr(0)) && (n = staktell()) > 0)
	{
		/*
		 * make the error trap
		 */

		state.hold = m;
		while (*m && *m != ':')
			m++;
		while (isspace(*++m));
		n -= m - state.hold;
		tmp = sfstropen();
		sfprintf(tmp, "%d %-.*s", level, n, m);
		s = sfstruse(tmp);

		/*
		 * return [ level | - ] [ message ]
		 * level is new level or - to retain old
		 * omitted message means it has been printed
		 */

		if (t = call(r, s))
		{
			i = strtol(t, &e, 0);
			if (e > t)
			{
				t = e;
				level = i;
			}
			else if (*t == '-')
				t++;
			while (isspace(*t))
				t++;
		}
		if (!t || !*t)
		{
			level |= ERROR_OUTPUT;
			message((-1, "suppressed %s message: `%-.*s'", level == 1 ? "warning" : "error", n, m));
		}
		sfstrclose(tmp);
		state.hold = 0;
	}
	return level;
}

/*
 * make entry point
 */

int
main(int argc, char** argv)
{
	register char*		s;
	register Rule_t*	r;
	register List_t*	p;
	int			i;
	int			args;
	int			trace;
	char*			t;
	char*			buf;
	char*			tok;
	Var_t*			v;
	Stat_t			st;
	Stat_t			ds;
	Sfio_t*			tmp;

	/*
	 * initialize dynamic globals
	 */

	version = strdup(fmtident(version));
	setlocale(LC_ALL, "");
	error_info.id = idname;
	error_info.version = version;
	error_info.exit = finish;
	error_info.auxilliary = intercept;
	if (pathcheck(PATHCHECK, error_info.id, NiL))
		return 1;
	error(-99, "startup");
	settypes("*?[]", C_MATCH);
	settypes("+-|=", C_OPTVAL);
	settypes(" \t\n", C_SEP);
	settype(0, C_SEP);
	settypes(" \t\v\n:+&=;\"\\", C_TERMINAL);
	settype(0, C_TERMINAL);
	settypes("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", C_ID1|C_ID2|C_VARIABLE1|C_VARIABLE2);
	settypes(".", C_VARIABLE1|C_VARIABLE2);
	settypes("0123456789", C_ID2|C_VARIABLE2);

	/*
	 * close garbage fd's -- we'll be tidy from this point on
	 * 3 may be /dev/tty on some systems
	 * 0..9 for user redirection in shell
	 * 10..19 left open by bugs in some shells
	 * error_info.fd interited from parent
	 * any close-on-exec fd's must have been done on our behalf
	 */

	i = 3;
	if (isatty(i))
		i++;
	for (; i < 20; i++)
		if (i != error_info.fd && !fcntl(i, F_GETFD, 0))
			close(i);

	/*
	 * allocate the very temporary buffer streams
	 */

	internal.met = sfstropen();
	internal.nam = sfstropen();
	internal.tmp = sfstropen();
	internal.val = sfstropen();
	internal.wrk = sfstropen();
	tmp = sfstropen();
	sfstrrsrv(tmp, 2 * MAXNAME);

	/*
	 * initialize the code and hash tables
	 */

	initcode();
	inithash();

	/*
	 * set the default state
	 */

	state.alias = 1;
	state.exec = 1;
	state.global = 1;
	state.init = 1;
#if DEBUG
	state.intermediate = 1;
#endif
	state.io[0] = sfstdin;
	state.io[1] = sfstdout;
	state.io[2] = sfstderr;
	state.jobs = 1;
	state.pid = getpid();
	state.readstate = MAXVIEW;
	state.scan = 1;
	state.start = CURTIME;
	state.stateview = -1;
	state.tabstops = 8;
	state.targetview = -1;
#if BINDINDEX
	state.view[0].path = makerule(".");
#else
	state.view[0].path = ".";
#endif
	state.view[0].pathlen = 1;
	state.writeobject = state.writestate = "-";

	/*
	 * pwd initialization
	 *
	 * for project management, if . is group|other writeable
	 * then change umask() for similar write protections
	 */

	buf = sfstrbase(tmp);
	internal.pwd = (s = getcwd(buf, MAXNAME)) ? strdup(s) : strdup(".");
	internal.pwdlen = strlen(internal.pwd);
	if (stat(".", &st))
		error(3, "cannot stat .");
	if (S_ISDIR(st.st_mode) && (st.st_mode & (S_IWGRP|S_IWOTH)))
		umask(umask(0) & ~(st.st_mode & (S_IWGRP|S_IWOTH)));

	/*
	 * set some variable default values
	 */

	hashclear(table.var, HASH_ALLOCATE);
	setvar(external.make, argv[0], V_import);
	t = "lib/make";
	setvar(external.lib, strdup((s = pathpath(t, argv[0], PATH_EXECUTE, buf, SF_BUFSIZE)) ? s : t), V_import);
	setvar(external.pwd, internal.pwd, V_import);
	setvar(external.version, version, V_import);
	hashset(table.var, HASH_ALLOCATE);

	/*
	 * read the environment
	 */

	readenv();
	if (v = getvar(external.nproc))
		state.jobs = (int)strtol(v->value, NiL, 0);
	if ((v = getvar(external.pwd)) && !streq(v->value, internal.pwd))
	{
		if (!stat(v->value, &st) && !stat(internal.pwd, &ds) && st.st_ino == ds.st_ino && st.st_dev == ds.st_dev)
		{
			free(internal.pwd);
			internal.pwd = strdup(v->value);
			internal.pwdlen = strlen(v->value);
		}
		else
		{
			v->property &= ~V_import;
			v->property |= V_free;
			v->value = strdup(internal.pwd);
		}
	}

	/*
	 * initialize the internal rule pointers
	 */

	initrule();

	/*
	 * read the static initialization script
	 */

	sfputr(tmp, initstatic, -1);
	parse(NiL, sfstruse(tmp), "initstatic", NiL);

	/*
	 * check and read the args file
	 */

	if (s = colonlist(tmp, external.args, 1, ' '))
	{
		i = fs3d(0);
		tok = tokopen(s, 1);
		while (s = tokread(tok))
			if (vecargs(vecfile(s), &argc, &argv) >= 0)
				break;
			else if (errno != ENOENT)
				error(1, "cannot read args file %s", s);
		tokclose(tok);
		fs3d(i);
	}
	state.argf = newof(0, int, argc, 0);

	/*
	 * set the command line options
	 * read the command line assignments
	 * mark the command line scripts and targets
	 */

	state.init = 0;
	state.readonly = 1;
	state.argv = argv;
	state.argc = argc;
	if ((args = scanargs(state.argc, state.argv, state.argf)) < 0)
		return 1;
	state.readonly = 0;
	state.init = 1;
	if (state.base)
		state.readstate = 0;
	if (state.compileonly)
	{
		state.forceread = 1;
		state.virtualdot = 0;
	}

	/*
	 * tone down the bootstrap noise
	 */

	if ((trace = error_info.trace) == -1)
		error_info.trace = 0;

	/*
	 * check explicit environment overrides
	 */

	if (s = colonlist(tmp, external.import, 1, ' '))
	{
		tok = tokopen(s, 1);
		while (s = tokread(tok))
		{
			if (i = *s == '!')
				s++;
			if (v = getvar(s))
			{
				if (i)
					v->property &= ~V_import;
				else
					v->property |= V_readonly;
			}
		}
		tokclose(tok);
	}

	/*
	 * set up the traps
	 */

	inittrap();

	/*
	 * announce the version
	 */

	if (error_info.trace < 0)
	{
		errno = 0;
		error(error_info.trace, "%s [%d %s]", version, state.pid, timestr(state.start));
	}

	/*
	 * initialize the views
	 */

	state.global = 0;
	state.user = 1;
	initview();

	/*
	 * check for mam
	 */

	if (state.mam.out)
	{
		if (!state.mam.statix || *state.mam.label)
			error_info.write = mamerror;
		if (state.mam.regress || state.regress)
		{
			sfprintf(state.mam.out, "%sinfo mam %s %05d\n", state.mam.label, state.mam.type, state.mam.parent);
			if (state.mam.regress)
				sfprintf(state.mam.out, "%sinfo start regression\n", state.mam.label);
		}
		else
		{
			sfprintf(state.mam.out, "%sinfo mam %s %05d 1994-07-17 %s\n", state.mam.label, state.mam.type, state.mam.parent, version);
			if (!state.mam.statix || *state.mam.label)
			{
				sfprintf(state.mam.out, "%sinfo start %lu\n", state.mam.label, CURTIME);
				if (!state.mam.root || streq(state.mam.root, internal.pwd))
					sfprintf(state.mam.out, "%sinfo pwd %s\n", state.mam.label, internal.pwd);
				else
					sfprintf(state.mam.out, "%sinfo pwd %s %s\n", state.mam.label, state.mam.root, mamname(makerule(internal.pwd)));
				buf = sfstrbase(tmp);
				if (state.fsview && !mount(NiL, buf, FS3D_GET|FS3D_ALL|FS3D_SIZE(sfstrsize(tmp)), NiL))
					sfprintf(state.mam.out, "%sinfo view %s\n", state.mam.label, buf);
			}
		}
	}

	/*
	 * read the dynamic initialization script
	 */

	if ((i = error_info.trace) > -20)
		error_info.trace = 0;
	sfputr(tmp, initdynamic, -1);
	parse(NiL, sfstruse(tmp), "initdynamic", NiL);
	error_info.trace = i;
	state.user = 0;
	state.init = 0;

	/*
	 * read the explicit makefiles
	 * readfile() handles the base and global rules
	 *
	 * NOTE: internal.tmplist is used to handle the effects of
	 *	 load() on internal list pointers
	 */

	compref(NiL, 0);
	if (p = internal.makefiles->prereqs)
	{
		p = internal.tmplist->prereqs = listcopy(p);
		for (; p; p = p->next)
			readfile(p->rule->name, COMP_FILE, NiL);
		freelist(internal.tmplist->prereqs);
		internal.tmplist->prereqs = 0;
	}

	/*
	 * if no explicit makefiles then try external.{convert,files}
	 */

	if (!state.makefile)
	{
		int	sep;
		Sfio_t*	exp;
		Sfio_t*	imp;

		exp = 0;
		imp = sfstropen();
		sep = 0;
		s = 0;
		if (*(t = getval(external.convert, VAL_PRIMARY)))
		{
			sfputr(tmp, t, 0);
			sfstrrsrv(tmp, MAXNAME);
			tok = tokopen(sfstrbase(tmp), 0);
			while (s = tokread(tok))
			{
				if (!exp)
					exp = sfstropen();
				if (t = colonlist(exp, s, 0, ' '))
				{
					t = tokopen(t, 0);
					while (s = tokread(t))
					{
						if (readfile(s, COMP_INCLUDE|COMP_DONTCARE, NiL))
							break;
						if (sep)
							sfputc(imp, ',');
						else
							sep = 1;
						sfputr(imp, s, -1);
					}
					tokclose(t);
					if (s)
						break;
				}
				if (!(s = tokread(tok)))
					break;
			}
			tokclose(tok);
			sfstrseek(tmp, 0, SEEK_SET);
		}
		if (!s && (s = colonlist(tmp, external.files, 1, ' ')))
		{
			tok = tokopen(s, 1);
			while (s = tokread(tok))
			{
				if (readfile(s, COMP_INCLUDE|COMP_DONTCARE, NiL))
					break;
				if (sep)
					sfputc(imp, ',');
				else
					sep = 1;
				sfputr(imp, s, -1);
			}
			tokclose(tok);
		}
		if (!s)
		{
			/*
			 * this readfile() pulls in the default base rules
			 * that might resolve any delayed self-documenting
			 * options in optcheck()
			 */

			if (readfile("-", COMP_FILE, NiL))
				optcheck(1);
			if (*(s = sfstruse(imp)))
				error(state.errorid ? 1 : 3, "a makefile must be specified when %s omitted", s);
			else
				error(state.errorid ? 1 : 3, "a makefile must be specified");
		}
		sfstrclose(imp);
		if (exp)
			sfstrclose(exp);
	}

	/*
	 * validate external command line options
	 */

	optcheck(1);

	/*
	 * check for listing of variable and rule definitions
	 */

	if (state.list)
	{
		dump(sfstdout, 0);
		return 0;
	}

	/*
	 * check if makefiles to be compiled
	 */

	if (state.compile && !state.virtualdot && state.writeobject)
	{
		/*
		 * make the compinit trap
		 */

		if (r = getrule(external.compinit))
		{
			state.reading = 1;
			maketop(r, P_dontcare|P_foreground, NiL);
			state.reading = 0;
		}
		if (state.exec && state.objectfile)
		{
			message((-2, "compiling makefile input into %s", state.objectfile));
			compile(state.objectfile, NiL);
		}

		/*
		 * make the compdone trap
		 */

		if (r = getrule(external.compdone))
		{
			state.reading = 1;
			maketop(r, P_dontcare|P_foreground, NiL);
			state.reading = 0;
		}
	}

	/*
	 * makefile read cleanup
	 */

	if (state.compileonly)
		return 0;
	compref(NiL, 0);
	sfstrclose(tmp);
	state.compile = COMPILED;
	if (state.believe)
	{
		if (!state.maxview)
			state.believe = 0;
		else if (state.fsview)
			error(3, "%s: option currently works in 2d only", optflag(OPT_believe)->name);
	}

	/*
	 * read the state file
	 */

	readstate();

	/*
	 * place the command line targets in internal.args
	 */

	if (internal.main->dynamic & D_dynamic)
		dynamic(internal.main);
	internal.args->prereqs = p = 0;
	for (i = args; i < state.argc; i++)
		if (state.argf[i] & ARG_TARGET)
		{
			List_t*		q;

			q = cons(makerule(state.argv[i]), NiL);
			if (p)
				p = p->next = q;
			else
				internal.args->prereqs = p = q;
		}

	/*
	 * the engine bootstrap is complete -- start user activities
	 */

	state.user = 1;

	/*
	 * make the makeinit trap
	 */

	if (r = getrule(external.makeinit))
		maketop(r, P_dontcare|P_foreground, NiL);

	/*
	 * read the command line scripts
	 */

	for (i = args; i < state.argc; i++)
		if (state.argf[i] & ARG_SCRIPT)
		{
			state.reading = 1;
			parse(NiL, state.argv[i], "command line script", NiL);
			state.reading = 0;
		}

	/*
	 * freeze the parameter files and candidate state variables
	 */

	state.user = 2;
	candidates();

	/*
	 * make the init trap
	 */

	if (r = getrule(external.init))
		maketop(r, P_dontcare|P_foreground, NiL);

	/*
	 * internal.args default to internal.main
	 */

	if (!internal.args->prereqs && internal.main->prereqs)
		internal.args->prereqs = listcopy(internal.main->prereqs);

	/*
	 * turn up the volume again
	 */

	if (!error_info.trace)
		error_info.trace = trace;

	/*
	 * make the prerequisites of internal.args
	 */

	if (internal.args->prereqs)
		while (internal.args->prereqs)
		{
			/*
			 * we explicitly allow internal.args modifications
			 */

			r = internal.args->prereqs->rule;
			internal.making->prereqs = internal.args->prereqs;
			internal.args->prereqs = internal.args->prereqs->next;
			internal.making->prereqs->next = 0;
			maketop(r, 0, NiL);
		}
	else if (state.makefile)
		error(3, "%s: a main target must be specified", state.makefile);

	/*
	 * finish up
	 */

	finish(0);
	return 0;
}

/*
 * clean up and exit
 */

void
finish(int n)
{
	Rule_t*		r;
	int		i;

	/*
	 * old error intercept
	 */

	if (!state.hold && (r = internal.error) && (r->property & (P_target|P_functional)) == P_target)
	{
		state.hold = null;
		if (n && error_info.errors && !state.compileonly && !state.interrupt)
		{
			if (r->status == NOTYET)
				maketop(r, P_dontcare|P_foreground, NiL);
			state.hold = 0;
			if (r->status == EXISTS)
			{
				r->status = NOTYET;
				return;
			}
		}
	}

	/*
	 * children exit without cleanup
	 */

	if (getpid() != state.pid)
		_exit(n);
	unparse(0);
	switch (state.finish)
	{

	case 0:
		/*
		 * disable listing and wait for any jobs to finish
		 */

		state.finish++;
		alarm(0);
		state.list = 0;
		message((-1, "%s cleanup", state.interrupt ? "interrupt" : n > 0 ? "error" : "normal"));
		complete(NiL, NiL, NiL, 0);
		/*FALLTHROUGH*/

	case 1:
		/*
		 * make the done trap
		 */

		state.finish++;
		if (!state.compileonly && (r = getrule(external.done)))
			maketop(r, P_dontcare, NiL);
		/*FALLTHROUGH*/

	case 2:
		/*
		 * make the makedone trap
		 */

		state.finish++;
		if (!state.compileonly && (r = getrule(external.makedone)))
			maketop(r, P_dontcare, NiL);
		/*FALLTHROUGH*/

	case 3:
		/*
		 * wait for any job(s) to finish
		 */

		state.finish++;
		complete(NiL, NiL, NiL, 0);
		/*FALLTHROUGH*/

	case 4:
		/*
		 * put all jobs in foreground and save state
		 */

		state.finish++;
		state.jobs = 0;
		savestate();
		/*FALLTHROUGH*/

	case 5:
		/*
		 * clean up temporaries
		 */

		state.finish++;
		remtmp(1);
		/*FALLTHROUGH*/

	case 6:
		/*
		 * drop the coshell
		 */
		
		state.finish++;
		drop();
		/*FALLTHROUGH*/

	case 7:
		/*
		 * dump
		 */

		state.finish++;
		if (state.test & 0x00002000)
		{
			Vmstat_t	vs;

			vmstat(Vmheap, &vs);
			error(0, "vm region %zu segments %zu busy %zu:%zu free %zu:%zu", vs.extent, vs.n_seg, vs.n_busy, vs.s_busy, vs.n_free, vs.s_free);
		}
		dump(sfstdout, error_info.trace <= -14);
		/*FALLTHROUGH*/

	case 8:
		/*
		 * final output
		 */

		state.finish++;
		if (state.errors && state.keepgoing && !n)
			n = 1;
		if (state.mam.out)
		{
			if (state.mam.regress)
				sfprintf(state.mam.out, "%sinfo finish regression\n", state.mam.label);
			else if (state.mam.dynamic || *state.mam.label)
				sfprintf(state.mam.out, "%sinfo finish %lu %d\n", state.mam.label, CURTIME, n);
		}
		for (i = 0; i < elementsof(state.io); i++)
			if (state.io[i] != sfstdin && state.io[i] != sfstdout && state.io[i] != sfstderr)
				sfclose(state.io[i]);
		if (state.errors && state.keepgoing)
			error(2, "*** %d action%s failed", state.errors, state.errors == 1 ? null : "s");
		message((-1, "%s exit", state.interrupt ? "interrupt" : n ? "error" : "normal"));
		break;

	}
	if (state.interrupt)
	{
		n = 3;
		signal(state.interrupt, SIG_DFL);
		kill(getpid(), state.interrupt);
		pause();
	}
	exit(n);
}
