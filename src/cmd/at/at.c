/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
 * at -- bundle request and send to AT_SERVICE
 */

static const char usage[] =
"[-?\n@(#)$Id: at (AT&T Research) 2000-05-09 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - run commands at specified time(s)]"
"[+DESCRIPTION?\b\f?\f\b is the command interface to the \bat\b daemon."
"	It submits commands to be executed at a future time, lists"
"	queue status, and controls the daemon.]"
"[+?Options that refer to specific jobs interpret the operands as job ids,"
"	otherwise if the \b--time\b option is not specified then the operands"
"	are interpreted as the start time. If \atime\a is not specified then"
"	the command is scheduled to be executed immediately, subject to the"
"	queue constraints.]"
"[+?If \b--time\b is specified then the first non-option argument is the"
"	command to be executed, otherwise the command to be executed is read"
"	from the standard input. The command job id is written to the standard"
"	output after the command has been successfully submitted to the"
"	daemon.]"

"[a:access?Check queue access and exit.]"
"[f:file?\afile\a is a script to be run at the specified time.]:[file]"
"[h:label|heading?Set the job label to \astring\a.]:[string]"
"[i:info?List queue information and exit.]"
"[l:list?List queue jobs and exit.]"
"[m:mail?Send mail when the command completes.]"
"[n!:exec?Execute the command. \b--noexec\b shows what would be done but"
"	does not execute.]"
"[p:status?List detailed queue status.]"
"[q:queue?Send request to \aqueue\a. Standard queues are:]:[queue]{"
"	[+a?The \bat\b(1) queue.]"
"	[+b?The \bbatch\b(1) queue.]"
"	[+c?The \bcron\b(1) queue.]"
"}"
"[r:remove?Remove command named by the \ajob\a ... operands from the queue.]"
"[s:service?Connect to the \bcs\b(1) service \apath\a.]:"
"	[path:=/dev/tcp/local/at]"
"[t:time|date?Schedule the command to be run at \atime\a. Most common"
"	date formats are accepted. The keyword \bevery\b can be combined"
"	with date parts to specify repeat executions, e.g.,"
"	\bevery Monday\b.]:[time:=now]"
"[u:user?List the per user queue information. Requires sufficient privilege"
"	to view the status of other users.]"
"[A:admin?Enable administrative actions. Requires sufficient privilege.]"
"[D:debug?Enable \bat\b daemon debug tracing to \afile\a at debug level"
"	\alevel\a. Higher levels produce more output. Requires sufficient"
"	privilege.]:[[level]][file]]]"
"[L:log?Write the log file path name on the standard output and exit. The log"
"	file is renamed with a \b.old\b suffix at the beginning of each month"
"	and a new log file started.]"
"[Q:quit?Terminate the \bat\b daemon. Requires sufficient privilege.]"
"[U:update?Causes the \bat\b daemon to do a schedule update and re-read"
"	the queue definition file if it has changed from the last time"
"	it was read. If \aqueuedef\a is specified then it is interpreted as"
"	a queue definition file line. See \bQUEUE DEFINITIONS\b below."
"	Requires sufficient privilege.]:?[queuedef]"
"[+QUEUE DEFINITIONS?(\aNOTE\a: the \bnroff\b(1) style syntax is taken from"
"	\bX/Open\b). The queue definition file defines queue attributes,"
"	one queue per line. Lines starting with \b#\b are comments. The format"
"	of a definition line is: \aname\a.[\anumber\a\battribute\b]]..., where"
"	\aname\a is a single letter queue name and \anumber\a applies to the"
"	following single character \battribute\b. If \anumber\a is omitted"
"	then it defaults to \b1\b. The default queues are: \ba.4j1n2u\b,"
"	\bb.2j2n90w2u\b, \bc.h8j2u60w\b. Per-queue user access may be"
"	specified by appending a space separated user names after the queue"
"	attributes. If the first list element is \b+\b then the list"
"	specifies users allowed to use the queue; othewise it specifies users"
"	denied access to the queue. If no user list is specified then queue"
"	access is controlled by the global files described in"
"	\bQUEUE ACCESS\b below. The attributes are:]{"
"		[+h?The job environment is initialized to contain at least the"
"			\bHOME\b, \bLOGNAME\b, \bUSER\b, \bPATH\b and"
"			\bSHELL\b of the submitting user. The jobs are also"
"			run in the user \bHOME\b directory.]"
"		[+j?The total number of running jobs for all queues is limited"
"			to \anumber\a.]"
"		[+l?No new jobs will be run until the load average is smaller"
"			than \anumber\a. This only works on systems where the"
"			load average is easily determined.]"
"		[+n?The default \bnice\b(1) priority is set to \anumber\a.]"
"		[+u?The per-user running job limit is set to \anumber\a.]"
"		[+w?At least \anumber\a seconds will elapse before the"
"			next job from the queue is run.]"
"	}"
"[+QUEUE ACCESS?The user \broot\b may submit jobs to all queues. If a queue"
"	definition does not specify a user access list then the queue access"
"	is controlled by the default access files in this order:]{"
"		[+(1)?If the directory \b/usr/lib/cron\b does not exist then"
"			job access is granted to all users.]"
"		[+(2)?If the file \b/usr/lib/cron/at.allow\b exists then"
"			access is granted only to user names listed in this"
"			file, one name per line.]"
"		[+(3)?If the file \b/usr/lib/cron/at.deny\b exists then"
"			access is denied to user names listed in this file,"
"			one name per line.]"
"		[+(4)?Otherwise access is denied to all users but \broot\b.]"
"	}"

"\n"
"\n[ job ... | time ... ]\n"
"\n"

"[+FILES]{"
"	[+/usr/lib/at/queuedefs?The default queue definition file.]"
"	[+/usr/lib/cron/at.(allow|deny)?The default queue access files.]"
"}"
"[+SEE ALSO?\bbatch\b(1), \bcrontab\b(1), \bnice\b(1), \bsh\b(1)]"
;

#include "at.h"

#include <fs3d.h>

#define ACCESS		(1<<0)
#define ADMIN		(1<<1)
#define EXEC		(1<<2)
#define INFO		(1<<3)
#define JOB		(1<<4)
#define LIST		(1<<5)
#define LOG		(1<<6)
#define MAIL		(1<<7)
#define QUIT		(1<<8)
#define REMOVE		(1<<9)
#define STATUS		(1<<10)
#define TRACE		(1<<11)
#define UPDATE		(1<<12)
#define USER		(1<<13)
#define VERSION		(1<<14)

int
main(int argc, char** argv)
{
	register int		c;
	register char*		s;
	register Sfio_t*	sp;
	register Sfio_t*	tp;
	char*			e;
	char*			t;
	int			n;
	int			skip;
	time_t			now;
	char			buf[PATH_MAX];

	int			flags = 0;
	int			interval = 1;
	int			op = EXEC;
	char*			file = 0;
	char*			label = 0;
	char*			queue = 0;
	char*			service = AT_SERVICE;
	char*			date = 0;
	char*			every = 0;
	char*			trace = 0;
	char*			update = 0;
	time_t			start = 0;

	static char		que[2];

	if (!(s = strrchr(*argv, '/')))
		s = *argv;
	else if (t = strchr(++s, '_'))
		s = t + 1;
	if (*s == 'b')
	{
		error_info.id = "batch";
		*(queue = que) = *s;
		op |= MAIL;
		date = "now";
	}
	else
		error_info.id = "at";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			op |= ACCESS;
			continue;
		case 'f':
			file = opt_info.arg;
			continue;
		case 'h':
			label = opt_info.arg;
			continue;
		case 'i':
			op |= INFO;
			continue;
		case 'l':
			op |= LIST;
			continue;
		case 'm':
			op |= MAIL;
			continue;
		case 'n':
			op &= ~EXEC;
			continue;
		case 'p':
			op |= STATUS;
			continue;
		case 'q':
			queue = opt_info.arg;
			continue;
		case 'r':
			op |= REMOVE;
			continue;
		case 's':
			service = opt_info.arg;
			continue;
		case 't':
			date = opt_info.arg;
			continue;
		case 'u':
			op |= USER;
			continue;
		case 'v':
			op |= VERSION;
			continue;
		case 'A':
			op |= ADMIN;
			continue;
		case 'D':
			op |= TRACE;
			trace = opt_info.arg;
			continue;
		case 'L':
			op |= LOG;
			continue;
		case 'Q':
			op |= QUIT;
			continue;
		case 'U':
			op |= UPDATE;
			update = opt_info.arg;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(sp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [message]");
	sfprintf(sp, "#00000\n#");
	if (op & ADMIN)
		sfprintf(sp, "%c", AT_ADMIN);
	if (queue)
		sfprintf(sp, "%c%s ", AT_QUEUE, queue);
	switch (op & (ACCESS|INFO|LIST|LOG|QUIT|REMOVE|STATUS|TRACE|UPDATE|USER|VERSION))
	{
	case ACCESS:
		sfputc(sp, AT_ACCESS);
		break;
	case INFO:
		sfputc(sp, AT_INFO);
		break;
	case LIST:
		sfputc(sp, AT_LIST);
		break;
	case LOG:
		sfputc(sp, AT_LOG);
		break;
	case QUIT:
		sfputc(sp, AT_QUIT);
		flags |= CS_OPEN_TEST;
		break;
	case REMOVE:
		sfputc(sp, AT_REMOVE);
		break;
	case STATUS:
		sfputc(sp, AT_STATUS);
		break;
	case TRACE:
		sfprintf(sp, "%c%s", AT_DEBUG, trace);
		break;
	case UPDATE:
		sfprintf(sp, "%c%s", AT_UPDATE, update ? update : "");
		break;
	case USER:
		sfputc(sp, AT_USER);
		break;
	case VERSION:
		sfputc(sp, AT_VERSION);
		break;
	default:
		if (op & ~(EXEC|MAIL))
			error(3, "only one of -[aglrsuvLQU] may be specified");
		op |= JOB;
		if (file && !sfopen(sfstdin, file, "r"))
			error(ERROR_SYSTEM|3, "%s: cannot read", file);
		if (!fs3d(FS3D_TEST) || !(s = pathpath("3d", NiL, PATH_ABSOLUTE|PATH_EXECUTE, buf, sizeof(buf))))
			s = pathshell();
		if (op & MAIL)
			sfputc(sp, AT_MAIL);
		skip = sfstrtell(sp) + 1;
		sfprintf(sp, "%c00000 %s ", AT_JOB, s);
		if (date || *argv)
		{
			if (!(tp = sfstropen()))
				error(ERROR_SYSTEM|3, "out of space [time]");
			sfputr(tp, "+0 ", -1);
			if (date)
				sfputr(tp, date, -1);
			else
				for (s = *argv;;)
				{
					for (n = ' '; c = *s++; n = c)
						if (!isspace(c) || n != (c = ' '))
							sfputc(tp, c);
					if (!(s = *++argv))
						break;
					if (n != ' ')
						sfputc(tp, ' ');
				}
			if (!(s = sfstruse(tp)))
				error(ERROR_SYSTEM|3, "out of space");
			s += 3;
			if (strneq(s, "cron ", n = 5) || strneq(s, "each ", n = 5) || strneq(s, "every ", n = 6) || strneq(s, "repeat ", n = 7))
			{
				every = s + n;
				t = s;
				*t++ = '+';
				*t++ = '0';
				while (*t != ' ')
					*t++ = ' ';
				if (*++t == '+' && isdigit(*(t + 1)))
				{
					interval = strtol(t, NiL, 0);
					while (*t && *t != ' ')
						*t++ = ' ';
				}
				else if (isdigit(*t))
				{
					interval = strtol(t, &e, 0);
					if (!isalpha(*e))
						interval = 1;
					else
						while (*t && *t != ' ')
							*t++ = ' ';
				}
				while (*every == ' ')
					every++;
			}
			now = time(NiL);
			start = tmdate(s, &t, &now);
			if (!every && (unsigned long)start <= (unsigned long)now)
				start = tmdate(s - 3, &t, &now);
			if (*t)
				error(3, "%s: invalid time specification", s);
			if (s = label)
			{
				sfputc(sp, AT_LABEL);
				while ((c = *s++) && !isspace(c))
					if (isalnum(c))
						sfputc(sp, c);
				sfputc(sp, ' ');
			}
			sfprintf(sp, "%c%lu", AT_TIME, start);
			if (s = every)
				sfprintf(sp, " +%d %s", interval, s);
		}
		break;
	}
	if (op & JOB)
	{
		if (!(s = coinit(CO_EXPORT|CO_SERVER)))
			error(ERROR_SYSTEM|3, "cannot generate initialization script");
		sfprintf(sp, "\n%s\n", s);
		s = sfstrbase(sp);
		sfsprintf(s + skip, 6, "%05lu ", sfstrtell(sp) - 7);
		s[skip + 5] = ' ';
		if (s = *argv++)
		{
			sfprintf(sp, "%s", s);
			while (s = *argv++)
				sfprintf(sp, " %s", s);
		}
		else	
			sfmove(sfstdin, sp, SF_UNBOUND, -1);
		sfputc(sp, '\n');
	}
	else
	{
		if (s = *argv++)
		{
			if (streq(s, "-") && !*argv)
				s = "*";
			sfprintf(sp, " %s", s);
			while (s = *argv++)
				sfprintf(sp, "|%s", s);
		}
		sfputc(sp, '\n');
	}
	n = sfstrtell(sp);
	if (!(s = sfstruse(sp)))
		error(ERROR_SYSTEM|3, "out of space");
	sfsprintf(s, 7, "#%05d\n", n - 7);
	s[6] = '\n';
	if (op & EXEC)
	{
		if ((op = csopen(&cs, service, 0)) < 0)
			error(ERROR_SYSTEM|3, "%s: cannot open service", service);
		if (cswrite(&cs, op, s, n) != n)
			error(ERROR_SYSTEM|3, "%s: service write error", service);
		while ((n = read(op, buf, sizeof(buf))) > 0)
			if (write(1, buf, n) != n)
				error(ERROR_SYSTEM|3, "write error");
		if (n < 0)
			error(ERROR_SYSTEM|3, "%s: service read error", service);
	}
	else
	{
		if (op & JOB)
		{
			sfprintf(sfstdout, "submit job to run on %s", fmttime(NiL, start));
			if (every)
				sfprintf(sfstdout, " and repeat every +%d %s", interval, every);
			sfprintf(sfstdout, "\n");
		}
		sfputr(sfstdout, s, -1);
	}
	return 0;
}
