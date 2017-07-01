/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * jcl deck interpreter
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "jcl.h"

#include <tm.h>

static const char usage[] =
"[-1s1I?\n@(#)$Id: jcl (AT&T Research) 2013-03-04 $\n]"
USAGE_LICENSE
"[+NAME?jcl - jcl deck interpreter]"
"[+DESCRIPTION?\bjcl\b interprets the JCL decks named by the \afile\a "
    "operands. The standard input is read if no \afile\a operands are "
    "specified. If \b--map\b is not specified then \b--map=" JCL_MAPFILE "\b "
    "is assumed. The \aname\a=\avalue\a operands override any values "
    "specified JCL decks.]"
"[+?control-M scheduler auto edit variable \b%%\b\aname\a values may be "
    "initialized by exporting \b" JCL_AUTO "\b\aname\a=\avalue\a or by "
    "\b%%\b\aname\a=\avalue\a on the command line.]"
"[d:debug?Set the debug trace level. Higher levels produce more "
    "output.]#[level]"
"[g:global?Like \b--map\b=\afile\a except that the default " JCL_MAPFILE
    ", if it exists, is still read after the command "
    "line options are processed.]:[file]"
"[i:import?Environment variable definitions take precedence over "
    "corresponding \b--map\b file definitions.]"
"[I:include?Search \adirectory\a for DSN names. More than one "
    "\b--include\b option may be specified.]:[directory]"
"[k:marklength?Mark fixed length record file names by appending "
    "\b%\b\alrecl\a to the names.]"
"[l:list?List the referenced \aitem\as. Multiple \b--list\b options may "
    "be specified. \aitem\a may be:]:[item]"
    "{"
        "[a:autoedits?List the %%\aname\a autoedit variables.]"
        "[c:counts?List the \aitem\a dup counts too.]"
        "[d:dd?List the \add\a name for \binputs\b and \boutputs\b.]"
        "[e:execs?List the EXEC job and command names.]"
        "[i:inputs?List the DD input data file paths.]"
        "[j:jobs?List main job names.]"
        "[o:outputs?List the DD output data file paths.]"
        "[p:programs?List the EXEC PGM=\aprogram\a program names.]"
        "[P:parms?List the EXEC PGM=\aprogram\a PARMs.]"
        "[s:scripts?List the EXEC [PROC=]]\ascript\a script names.]"
        "[v:variables?List the &\aname\a variables.]"
    "}"
"[m:map?Read the dataset file path prefix map \afile\a. Each line in "
    "\afile\a contains an operation followed by 0 or more space separated "
    "fields. \b#\b in the first column denotes a comment; comments and blank "
    "lines are ignored. If \afile\a is \b-\b then \b/dev/null\b is assumed. "
    "If \afile\a is not found and contains no \b/\b then "
    "\b../lib/jcl/\b\afile\a is searched for on \b$PATH\b. If no \b--map\b "
    "option is specified then the default " JCL_MAPFILE ", if it exists, is "
    "read after the command line options are processed. The operations "
    "are:]:[file]"
    "{"
        "[+export \aname\a=\avalue ...?Set export variable values. "
            "Export variable expansions are preserved in the generated "
            "scripts.]"
        "[+map \aprefix\a \amap\a [\asuffix\a]]?Dataset paths are mapped "
            "by doing a longest prefix match on the prefixes. The matching "
            "dataset prefix is replaced by \amap\a, and \asuffix\a, if "
            "specified, is appended. The prefix \b\"\"\b matches when no "
            "other prefix matches. Leading \b*.\b and trailing \b.*\b match "
            "any \b.\b-separated component. \b${\b\an\a\b}\b in \amap\a "
            "expands to the component matched by the \an\a-th \b*\b in "
            "\aprefix\a, counting from 1, left to right.]"
        "[+set --[no]]\aoption\a[=\avalue]] ...?Set command line "
            "options.]"
        "[+set %%\aname\a=\avalue ...?Set auto edit variable values.]"
        "[+set \aname\a=\avalue ...?Set variable values.]"
    "}"
"[n!:exec?Enable command execution. \b--noexec\b disables.]"
"[N:never?Disable all command and recursive script execution.]"
"[p:parameterize?Parameterize simple &\aname\a variable references by "
    "substituting the \bsh\b(1) equivalent ${\aname\a}.]"
"[r:resolve?Resolve each operand as a path name using the \b--map\b "
    "files and print the resulting path on the standard output, one path per "
    "line.]"
"[s:subdir?Execute each job in a separate subdirectory named "
    "\ajobname\a.\ayy-mm-dd.n\a]]. \an\a starts at \b1\b and is incremeted "
    "by \b1\b for each run of \ajobname\a within the same day.]"
"[t:subtmp?Place temporary files in \b.\b and do not remove on exit.]"
"[v:verbose?For \b--noexec\b the equivalent \bsh\b(1) script is listed "
    "on the standard output. For \b--exec\b verbose execution output, like "
    "start and completion times, is enabled.]"
"[w:warn?Enable verbose warning messages.]"
"[x:trace?Enable command execution trace.]"
"[O:odate?Set the control-M original date to \adate\a.]:[date:=now]"
"[R:rdate?Set the control-M current run date to \adate\a.]:[date:=now]"
"[S:date?Set the control-M system date to \adate\a.]:[date:=now]"
"\n"
"\n[ name=value ... ] [ file ... ]\n"
"\n"
"[+FILES]"
    "{"
        "[+../lib/jcl/prefix?Default \b--map\b file, found on \b$PATH\b.]"
    "}"
"[+SEE ALSO?\bsh\b(1)]"
;

#include <tm.h>

struct Map_s; typedef struct Map_s Map_t;

struct Map_s
{
	Map_t*		next;
	char*		arg;
	int		map;
};

typedef struct State_s
{
	Jcldisc_t	disc;
	int		resolve;
	Map_t*		map;
	Map_t*		lastmap;
} State_t;

static int
optset(Jcl_t* jcl, int c, Jcldisc_t* disc)
{
	State_t*	state = (State_t*)disc;
	unsigned long	f;
	char*		s;
	time_t*		t;
	Map_t*		p;

	switch (c)
	{
	case 'd':
		error_info.trace = -opt_info.number;
		return 1;
	case 'g':
	case 'm':
		if (!(p = newof(0, Map_t, 1, 0)))
		{
			error(ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		p->arg = opt_info.arg;
		p->map = c == 'm';
		if (!state->map)
			state->map = p;
		else
			state->lastmap->next = p;
		state->lastmap = p;
		return 1;
	case 'i':
		f = JCL_IMPORT;
		break;
	case 'I':
		jclinclude(NiL, opt_info.arg, JCL_JOB|JCL_PROC, disc);
		return 1;
	case 'k':
		f = JCL_MARKLENGTH;
		break;
	case 'l':
		switch (opt_info.num)
		{
		case 'a':
			jcl->flags |= JCL_LISTAUTOEDITS;
			break;
		case 'c':
			jcl->flags |= JCL_LISTCOUNTS;
			break;
		case 'd':
			jcl->flags |= JCL_LISTDD;
			break;
		case 'e':
			jcl->flags |= JCL_LISTEXEC;
			break;
		case 'i':
			jcl->flags |= JCL_LISTINPUTS;
			break;
		case 'j':
			jcl->flags |= JCL_LISTJOBS;
			break;
		case 'o':
			jcl->flags |= JCL_LISTOUTPUTS;
			break;
		case 'p':
			jcl->flags |= JCL_LISTPROGRAMS;
			break;
		case 'P':
			jcl->flags |= JCL_LISTPARMS;
			break;
		case 's':
			jcl->flags |= JCL_LISTSCRIPTS;
			break;
		case 'v':
			jcl->flags |= JCL_LISTVARIABLES;
			break;
		}
		return 1;
	case 'n':
		jcl->roflags |= JCL_EXEC;
		if (!opt_info.number)
			jcl->flags &= ~JCL_EXEC;
		return 1;
	case 'N':
		jcl->roflags |= JCL_RECURSE;
		if (!opt_info.number)
			jcl->flags &= ~JCL_RECURSE;
		return 1;
	case 'O':
		t = &jcl->disc->odate;
	date:
		*t = tmdate(opt_info.arg, &s, NiL);
		if (*s)
		{
			error(2, "%s: %s: invalid date", opt_info.name, opt_info.arg);
			return 0;
		}
		return 1;
	case 'p':
		f = JCL_PARAMETERIZE;
		break;
	case 'r':
		((State_t*)disc)->resolve = !!opt_info.number;
		return 1;
	case 'R':
		t = &jcl->disc->rdate;
		goto date;
	case 's':
		f = JCL_SUBDIR;
		break;
	case 'S':
		t = &jcl->disc->date;
		goto date;
	case 't':
		f = JCL_SUBTMP;
		break;
	case 'v':
		f = JCL_VERBOSE;
		break;
	case 'w':
		f = JCL_WARN;
		break;
	case 'x':
		f = JCL_TRACE;
		break;
	case ':':
		error(2, "%s", opt_info.arg);
		return 0;
	case '?':
		error(ERROR_USAGE|4, "%s", opt_info.arg);
		return 0;
	}
	if ((jcl->roflags & (JCL_MAPPED|f)) != (JCL_MAPPED|f))
	{
		if (!(jcl->roflags & JCL_MAPPED))
			jcl->roflags |= f;
		if (opt_info.number)
			jcl->flags |= f;
		else
			jcl->flags &= ~f;
	}
	return 1;
}

int
main(int argc, char** argv)
{
	char*		s;
	char*		t;
	Jcl_t*		jcl;
	Map_t*		p;
	int		c;
	State_t		state;

	error_info.id = "jcl";
	memset(&state, 0, sizeof(state));
	jclinit(&state.disc, errorf);
	state.disc.usage = usage;
	state.disc.optsetf = optset;
	if (jcl = jclopen(NiL, NiL, JCL_EXEC|JCL_JOB|JCL_RECURSE|JCL_STANDARD|JCL_SCOPE, &state.disc))
	{
		while ((c = optget(argv, usage)) && optset(jcl, c, &state.disc));
		argv += opt_info.index;
		if (error_info.errors)
			error(ERROR_USAGE|4, "%s", optusage(NiL));
		while (jclsym(jcl, *argv, NiL, JCL_SYM_READONLY))
			argv++;
		c = 0;
		while (p = state.map)
		{
			state.map = state.map->next;
			if (jclmap(jcl, p->arg, &state.disc))
				return 1;
			c |= p->map;
			free(p);
		}
		if (!c && jclmap(jcl, NiL, &state.disc))
			return 1;
		if (state.resolve)
		{
			while (s = *argv++)
				if (t = jclpath(jcl, s))
					sfprintf(sfstdout, "%s\n", t);
				else
					error(2, "%s: cannot resolve", s);
		}
		else
		{
			jcl->step->command = *argv;
			while (!jclrun(jcl) && jcl->step->command && (jcl->step->command = *++argv));
		}
	}
	if (!(c = jclclose(jcl)) && error_info.errors)
		c = 1;
	return c;
}
