/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * ss - list local network system status
 */

static const char usage[] =
"[-?\n@(#)$Id: ss (AT&T Research) 1995-05-09 $\n]"
USAGE_LICENSE
"[+NAME?ss - list local network system status]"
"[+DESCRIPTION?\bss\b writes the system status of hosts on the local network"
"	to the standard output. The option order determines the listing sort"
"	keys, from left to right, hightest to lowest precedence. If one or"
"	more \ahost\a operands are given then only the status to those hosts"
"	is listed, otherwise the status for all hosts on the local network is"
"	listed.]"
"[+?Static information for hosts in the local network is in the file"
"	\b../share/lib/cs/local\b on \b$PATH\b; see \bcs\b(1) for details."
"	Dynamic status for each active host is maintained in the file"
"	\b../share/lib/ss/\b\ahost\a by a status daemon \assd\a running on"
"	each host. This information is updated once a minute, plus or minus"
"	a random part of a minute. \bss\b starts an \bssd\bs as necessary. An"
"	\bssd\b may be killed by removing the corresponding host status file.]"

"[c:cpu?Sort by cpu utilization.]"
"[i:idle?Sort by user idle time.]"
"[l:load?Sort by load average.]"
"[r:reverse?Reverse the sort order.]"
"[t:time?Sort by system up time.]"
"[u:users?Sort by number of active users.]"

"\n"
"\n[ host ... ]\n"
"\n"

"[+FILES]{"
"	[+../share/lib/cs/local?Local host info list on \b$PATH\b.]"
"	[+../share/lib/ss/\ahost\a?Host status files on \b$PATH\b.]"
"}"

"[+SEE ALSO?\bcs\b(1)]"
;

#include <ast.h>
#include <cs.h>
#include <dirent.h>
#include <error.h>

#define CPU		1
#define IDLE		2
#define LOAD		3
#define TIME		4
#define USERS		5

#define KEYS		4
#define SHIFT		3

#define SORT		((1<<SHIFT)-1)
#define MASK		((1<<(KEYS*SHIFT))-1)

typedef struct
{
	char*		name;
	char*		fail;
	CSSTAT		stat;
} Sys_t;

static struct
{
	Sys_t**		base;
	Sys_t**		end;
	Sys_t**		next;
	int		keys;
	int		rev;
}			state;


/*
 * enter system name into the system vector
 * alloc!=0 allocate space for name
 */

static void
enter(char* name, int alloc)
{
	register int	n;
	register Sys_t*	sp;

	if (state.next >= state.end)
	{
		n = state.end - state.base;
		if (!(state.base = newof(state.base, Sys_t*, n * 2, 0)))
			error(3, "out of space [vec %s]", n * 2);
		state.end = (state.next = state.base + n) + n;
	}
	if (!(*state.next = newof(0, Sys_t, 1, 0)))
		error(3, "out of space [sys]");
	sp = *state.next++;
	if (!alloc) sp->name = name;
	else if (!(sp->name = strdup(name)))
		error(3, "out of space [name]");
	if (!csaddr(sp->name))
		sp->fail = "*unknown host*";
	else if (csstat(sp->name, &sp->stat))
		sp->fail = "*no status*";
}

/*
 * sys sort
 */

static int
sort(const char* pa, const char* pb)
{
	register Sys_t*	a = (Sys_t*)pa;
	register Sys_t*	b = (Sys_t*)pb;
	register int	k;
	register long	n;

	for (k = state.keys; k; k >>= SHIFT)
		switch (k & SORT)
		{
		case CPU:
			if (n = (a->stat.pctusr + a->stat.pctsys) - (b->stat.pctusr + b->stat.pctsys))
				return(n * state.rev);
			break;
		case IDLE:
			if (a->stat.idle < b->stat.idle)
				return(state.rev);
			if (a->stat.idle > b->stat.idle)
				return(-state.rev);
			break;
		case LOAD:
			if (a->stat.load < b->stat.load)
				return(-state.rev);
			if (a->stat.load > b->stat.load)
				return(state.rev);
			break;
		case TIME:
			if (a->stat.up < b->stat.up)
				return(state.rev);
			if (a->stat.up > b->stat.up)
				return(-state.rev);
			break;
		case USERS:
			if (n = a->stat.users - b->stat.users)
				return(n * state.rev);
			break;
		}
	return strcoll(a->name, b->name) * state.rev;
}

int
main(int argc, char** argv)
{
	register Sys_t*		sp;
	register Sys_t**	sv;
	register struct dirent*	entry;
	int			n;
	DIR*			dirp;
	char			buf[PATH_MAX];

	NoP(argc);
	setlocale(LC_ALL, "");
	error_info.id = "ss";
	state.rev = 1;
	while (n = optget(argv, usage))
	{
		switch (n)
		{
		case 'c':
			n = CPU;
			break;
		case 'i':
			n = IDLE;
			break;
		case 'l':
			n = LOAD;
			break;
		case 'r':
			state.rev = -state.rev;
			continue;
		case 't':
			n = TIME;
			break;
		case 'u':
			n = USERS;
			break;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		state.keys = (state.keys >> SHIFT) | (n << (SHIFT*(KEYS-1)));
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (state.keys)
		while (!(state.keys & SORT))
			state.keys >>= SHIFT;
	if (!pathpath(CS_STAT_DIR, argv[0], PATH_EXECUTE, buf, sizeof(buf)))
		error(3, "%s: cannot find data directory", CS_STAT_DIR);
	if (!(state.next = state.base = newof(0, Sys_t*, n = 64, 0)))
		error(3, "out of space [vec %d]", n);
	state.end = state.base + n;
	if (*(argv += opt_info.index))
		while (*argv)
			enter(*argv++, 0);
	else if (!(dirp = opendir(buf)))
		error(ERROR_SYSTEM|3, "%s: cannot open", buf);
	else
	{
		while (entry = readdir(dirp))
			if (entry->d_name[0] != '.' && !streq(entry->d_name, "core"))
				enter(entry->d_name, 1);
		closedir(dirp);
	}
	if (n = state.next - state.base)
	{
		strsort((char**)state.base, n, sort);
		for (sv = state.base; sv < state.next; sv++)
			if ((sp = *sv)->fail)
				sfprintf(sfstdout, "%-12s %s\n", sp->name, sp->fail);
			else
				sfprintf(sfstdout, "%-12s%4s%7s,%3d user%s idle%7s, load%3d.%02d, %%usr%3d, %%sys%3d\n", sp->name, sp->stat.up < 0 ? "down" : "up", fmtelapsed(sp->stat.up < 0 ? -sp->stat.up : sp->stat.up, 1), sp->stat.users, sp->stat.users == 1 ? ", " : "s,", sp->stat.idle > sp->stat.up ? "%" : fmtelapsed(sp->stat.idle, 1), sp->stat.load / 100, sp->stat.load % 100, sp->stat.pctusr, sp->stat.pctsys);
	}
	return error_info.errors != 0;
}
