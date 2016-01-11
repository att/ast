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
 * cs - connect stream control
 */

static const char usage[] =
"[-?\n@(#)$Id: cs (AT&T Research) 2006-06-11 $\n]"
USAGE_LICENSE
"[+NAME?cs - connect stream control]"
"[+DESCRIPTION?\bcs\b displays, initiates, and terminates connect stream"
"	services, and displays the contents of connect stream message files."
"	If no options are spcified then the connect stream for \apath\a is"
"	opened. If the corresponding service is not running then it is"
"	initiated and the connection is attempted again. If \acommand\a is"
"	specified then it is executed with the standard input, standard"
"	output and standard error redirected to the \apath\a connect stream."
"	If \acommand\a is omitted then the \b/dev/\b equivalent path for"
"	the connect stream is listed on the standard output.]"

"[a:attribute?List the attribute \aname\a for each host. If \aname\a is \b-\b"
"	then all attributes are listed. The hostname attribute is listed"
"	without a label, all other attributes are listed as"
"	\alabel\a=\avalue\a]:[name]"
"[c:cat?Catenate messages in the named \apath\a operands. If \apath\a is"
"	omitted then the standard input is read.]"
"[d:debug?Set the debug trace level to \alevel\a. Higher levels produce"
"	more output.]#[level]"
"[f:continuous?Used with \b--cat\b to list messages on a \apath\a or standard"
"	input that is continuously updated.]"
"[h:hostenvironment?Lists \bHOSTNAME\b=\aname\a \bHOSTTYPE\b=\atype\a on the"
"	standard output for the named \ahost\a operand or the local host if"
"	\ahost\a is omitted. This is useful for \b.profile\b initialization.]"
"[i:interactive?Open an interactive connection to the connect stream. The"
"	service is initiated if it is not already running.]"
"[k:kill?Send \asignal\a to the server on the connect stream named by"
"	\apath\a. \asignal\a may be a signal name or signal number.]:[signal]"
"[l:list?List the active connect stream services on the standard output.]"
"[m:mount?List the active connect stream mount directories on the standard"
"	output.]"
"[p:process?List the active connect stream process ids on the standard output.]"
"[q:query?Open an interactive connection to the connect stream if a service"
"	is already running; fail otherwise.]"
"[r:raw?Raw mode \b--interactive\b connection.]"
"[s:iservice?List details for each active connect stream service on the"
"	standard output. The output format is similar to an \bls\b(1)"
"	\b--long\b listing, except the size field is the \btcp\b or \budp\b"
"	port number, and the service base name appears as a symbolic link"
"	to the network \b/proc\b path for the service process.]"
"[t:translate?\ahost\a name operands are translated to IP address dot notation"
"	and listed on the standard output. If \ahost\a is omitted then the"
"	standard input is read for host names, one per line.]"
"[C:call?Used with \b--cat\b to list only messages for the calls in"
"	\acall-list\a.]:[call-list]"
"[O:open-flags?Set optional \bcsopen\b(3) flags. Used by the \bcs\b(3) library"
"	to initiate remote connections.]:[flags]"
"[T:terse?Used with \b--cat\b to list terse messages for the calls in"
"	\acall-list\a]:[call-list]"

"\n"
"\n[ [ - ] host | path [ command ... ] ]\n"
"\n"

"[+DATA?Static information for hosts in the local network is in the file"
"	\b../share/lib/cs/local\b on \b$PATH\b. Each line in the \blocal\b"
"	file provides information for a single host. The syntax is:"
"	\ahost-name\a [ \aattribute\a=\avalue\a ... ]]. Attributes for the host"
"	\blocal\b are inherited by all hosts. Locally administered attributes"
"	may be added. \aattribute\a with predefined semantics are:]{"
"		[+addr?The host IP address in dot notation.]"
"		[+bias?The \bcoshell\b(1) multiplies the host load by \bbias\b"
"			to prioritize host availability. \bbias\b > 1 makes"
"			the host less likely to be chosen.]"
"		[+busy?\bcoshell\b(1) jobs running on a host that has remained"
"			busy for this amount of time are suspended until the"
"			host returns to idle status.]"
"		[+cpu?The number of cpus on the host as reported by"
"			\bpackage\b(1).]"
"		[+idle?The minimum interactive user idle time before"
"			\bcoshell\b(1) will schedule a job on the host.]"
"		[+pool?The \bcoshell\b(1) attempts to keep \bpool\b"
"			host connections active.]"
"		[+rating?The host rating as reported by \bpackage\b(1).]"
"		[+type?The host type as reported by \bpackage\b(1).]"
"}"
"[+FILES]{"
"	[+../share/lib/cs/local?Local host info list on \b$PATH\b.]"
"	[+../share/lib/ss/\ahost\a?Host status files on \b$PATH\b.]"
"}"

"[+SEE ALSO?\bcoshell\b(1), \bcss\b(1), \bpackage\b(1), \bss\b(1), \bcs\b(3)]"
;

#include <ast.h>
#include <coshell.h>
#include <cs.h>
#include <error.h>
#include <ftwalk.h>
#include <msg.h>
#include <proc.h>
#include <sig.h>
#include <tm.h>
#include <tok.h>
#include <debug.h>

#define LIST		(1<<0)
#define LIST_MOUNT	(1<<1)
#define LIST_PROCESS	(1<<2)
#define LIST_SERVICE	(1<<3)

#define MSG_LIST		(MSG_LIST_USER<<0)
#define MSG_LIST_CONTINUOUS	(MSG_LIST_USER<<1)

static struct
{
	int		list;		/* list flags			*/
	char*		local;		/* csname(0)			*/
} state;

/*
 * host address translation
 */

static void
address(const char* name)
{
	unsigned long	addr;

	if (addr = csaddr(name))
		sfprintf(sfstdout, "name=%s addr=%s host=%s user=%s flags=:%s%s%s%s%s\n"
			, csfull(addr)
			, csntoa(addr)
			, cs.host
			, cs.user
			, (cs.flags & CS_ADDR_LOCAL) ? "LOCAL:" : ""
			, (cs.flags & CS_ADDR_NUMERIC) ? "NUMERIC:" : ""
			, (cs.flags & CS_ADDR_SHARE) ? "SHARE:" : ""
			, (cs.flags & CS_ADDR_REMOTE) ? "REMOTE:" : ""
			, (cs.flags & (CS_ADDR_LOCAL|CS_ADDR_NUMERIC|CS_ADDR_SHARE|CS_ADDR_REMOTE)) ? "" : ":"
			);
	else
		sfprintf(sfstdout, "addr=\n");
}

/*
 * order by name
 */

static int
order(register Ftw_t* f1, register Ftw_t* f2)
{
	return f1->level == 3 ? strcoll(f1->name, f2->name) : 0;
}

/*
 * list the service mount directories
 */

#define PROC_OFF	4
#define SERVICE_COLS	37

static int
list(register Ftw_t* ftw)
{
	register char*	s;
	register char*	t;
	register char*	u;
	register char*	p;
	char*		port;
	char*		proc;
	int		mode;
	int		n;
	uid_t		uid;
	struct stat	st;

	static char	label[3][64];
	static char	qual_buf[64];
	static char	proc_buf[PATH_MAX + 1] = " -> ";
	static char	port_buf[PATH_MAX + 1];
	static char	serv_buf[PATH_MAX + 1];
	static char	time_buf[64];

	static Sfio_t*	sp;

	if (ftw->level > 0)
	{
		if (ftw->level > elementsof(label))
		{
			ftw->status = FTW_SKIP;
			mode = ftw->statb.st_mode & (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
			if (strmatch(ftw->name, "*-*-*-*"))
			{
				s = qual_buf;
				*s++ = '/';
				t = strrchr(ftw->name, '-');
				while (s < &qual_buf[sizeof(qual_buf) - 1] && (*s++ = *++t));
				*s = 0;
			}
			else
				qual_buf[0] = 0;
			if (!streq(label[1], "share"))
				mode |= (S_ISVTX|S_IXOTH);
			if (!sp && !(sp = sfstropen()))
				error(ERROR_SYSTEM|3, "out of space");
			sfprintf(sp, "%s/X%s", ftw->path, CS_MNT_TAIL);
			if (!(p = sfstruse(sp)))
				error(ERROR_SYSTEM|3, "out of space");
			s = p + ftw->pathlen + 1;
			*s = CS_MNT_PROCESS;
			if (pathgetlink(p, proc_buf + PROC_OFF, sizeof(proc_buf) - PROC_OFF) <= 0)
			{
				/*
				 * check for old single char cs mounts
				 */

				*(s + 1) = 0;
				if (pathgetlink(p, proc_buf + PROC_OFF, sizeof(proc_buf) - PROC_OFF) <= 0)
				{
					*s = CS_MNT_STREAM;
					remove(p);
					*(s + 1) = CS_MNT_TAIL[0];
					remove(p);
					return 0;
				}
			}
			proc = proc_buf;
			if (strncmp(proc + PROC_OFF, "/n/", 3))
				u = proc + PROC_OFF;
			else
			{
				t = proc + PROC_OFF + 3;
				if (u = strchr(t, '/'))
				{
					*u = 0;
					if (strcmp(t, state.local))
					{
						*u = '/';
						u = 0;
					}
					else
						*u = '/';
				}
			}
			if (u && (n = strtol(u + 6, NiL, 10)) && kill(n, 0) && errno == ESRCH)
			{
				remove(p);
				*s = CS_MNT_STREAM;
				remove(p);
				return 0;
			}
			if (state.list & LIST_SERVICE)
			{
				*s = CS_MNT_STREAM;
				if (pathgetlink(p, port_buf, sizeof(port_buf)) > 0 && (port = strrchr(port_buf, '/')))
					port++;
				else
					port = "";
				if (stat(p, &st))
					st.st_mtime = ftw->statb.st_mtime;
				tmfmt(time_buf, sizeof(time_buf), "%?%QL", &st.st_mtime);
				*s = CS_MNT_LOG;
				if (stat(p, &st))
					st = ftw->statb;
				*(s - 1) = 0;
				sfprintf(sfstdout, "%c%s  1 %-8s %-8s %7s %s %s%s%s\n", label[0][0], fmtmode(mode, 0) + 1, fmtuid(st.st_uid), (mode & S_IROTH) ? "other" : fmtgid(ftw->statb.st_gid), port, time_buf, label[2], qual_buf, proc);
			}
			else
			{
				n = sfprintf(sfstdout, "/dev/%s/%s/%s", label[0], label[1], label[2]);
				if (!(mode & S_IROTH))
				{
					if (!(mode & S_IRGRP))
					{
						n += sfprintf(sfstdout, "/user");
						if (ftw->statb.st_uid != geteuid())
							n += sfprintf(sfstdout, "=%s", fmtuid(ftw->statb.st_uid));
					}
					else
					{
						n += sfprintf(sfstdout, "/group");
						if (ftw->statb.st_gid != getegid())
							n += sfprintf(sfstdout, "=%s", fmtgid(ftw->statb.st_gid));
					}
				}
				if (*ftw->name == '-')
					n += sfprintf(sfstdout, "/trust");
				else
				{
					sfsprintf(port_buf, sizeof(port_buf) - 1, "%s/%s/%s/%s%s", CS_SVC_DIR, label[0], label[2], label[2], CS_SVC_SUFFIX);
					uid = strtol(ftw->name, NiL, 0);
					if (!pathpath(port_buf, "", PATH_ABSOLUTE|PATH_EXECUTE, serv_buf, sizeof(serv_buf)) || stat(serv_buf, &st) || st.st_uid != uid)
						n += sfprintf(sfstdout, "/trust=%s", fmtuid(uid));
				}
				if (qual_buf[0])
					n += sfprintf(sfstdout, "%s", qual_buf);
				if (u && streq(label[1], "share"))
					n += sfprintf(sfstdout, "/local");
				if (*label[0] == 't')
				{
					*s = CS_MNT_AUTH;
					if (access(p, F_OK))
						n += sfprintf(sfstdout, "/other");
				}
				if (state.list &= ~LIST)
				{
					if ((state.list ^ (state.list>>1)) == (state.list | (state.list>>1)))
						while (n++ < SERVICE_COLS)
							sfputc(sfstdout, ' ');
					if (state.list & LIST_PROCESS)
					{
						*(s - 1) = 0;
						sfprintf(sfstdout, " %s", u ? u : proc + PROC_OFF);
					}
					if (state.list & LIST_MOUNT)
					{
						*(s - 1) = 0;
						sfprintf(sfstdout, " %s", p);
					}
				}
				sfprintf(sfstdout, "\n");
			}
		}
		else
		{
			s = ftw->name;
			if (ftw->level == 2 && streq(s, "share") && streq(s, state.local))
				s = "local";
			strncpy(label[ftw->level - 1], s, elementsof(label[0]) - 1);
		}
	}
	return 0;
}

/*
 * list messages in sp
 * if continuous!=0 then act like tail -f
 */

static int
msgcat(register Sfio_t* sp, register int flags, unsigned long call, unsigned long terse)
{
	register long	n;
	Msg_call_t	msg;

	if (flags & MSG_LIST_CONTINUOUS)
		sfset(sfstdout, SF_LINE, 1);
	for (;;)
	{
		while ((n = msgrecv(sffileno(sp), &msg)) > 0)
			if (MSG_MASK(msg.call) & call)
				msglist(sfstdout, &msg, flags, terse);
		if (n < 0)
			return -1;
		if (!(flags & MSG_LIST_CONTINUOUS))
			return 0;
		sleep(2);
	}
}

int
main(int argc, char** argv)
{
	char**		ap;
	int		n;
	int		fd;
	int		hostenv = 0;
	int		initiate = CS_OPEN_READ;
	int		interactive = 0;
	int		msg = 0;
	int		clientflags = 0;
	int		remote = 0;
	int		translate = 0;
	unsigned long	call = ~0;
	unsigned long	terse = 0;
	char*		attr = 0;
	char*		host;
	char*		path;
	char*		proc;
	char*		sig = 0;
	char*		av[8];
	Sfio_t*		sp;
	char		buf[PATH_MAX + 1];
	char		tmp[PATH_MAX + 1];

	NoP(argc);
	setlocale(LC_ALL, "");
	error_info.id = "cs";
	debug(systrace(0));
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			attr = opt_info.arg;
			continue;
		case 'c':
			msg |= MSG_LIST;
			continue;
		case 'd':
			error_info.trace = -opt_info.num;
			continue;
		case 'f':
			msg |= MSG_LIST_CONTINUOUS;
			continue;
		case 'h':
			hostenv = 1;
			continue;
		case 'r':
			clientflags = CS_CLIENT_RAW;
			/*FALLTHROUGH*/
		case 'i':
			interactive = 1;
			msg |= MSG_LIST_ID;
			continue;
		case 'k':
			sig = opt_info.arg;
			continue;
		case 'l':
			state.list |= LIST;
			continue;
		case 'm':
			state.list |= LIST_MOUNT;
			continue;
		case 'p':
			state.list |= LIST_PROCESS;
			continue;
		case 'q':
			interactive = 1;
			initiate |= CS_OPEN_TEST;
			continue;
		case 's':
			state.list |= LIST_SERVICE;
			continue;
		case 't':
			translate = 1;
			continue;
		case 'C':
			call = msgsetmask(opt_info.arg);
			continue;
		case 'T':
			terse = msgsetmask(opt_info.arg);
			continue;
		case 'O':
			remote = 1;
			host = opt_info.arg;
			for (;;)
			{
				switch (*host++)
				{
				case 0:
					break;
				case CS_REMOTE_OPEN_AGENT:
					initiate |= CS_OPEN_AGENT;
					continue;
				case CS_REMOTE_OPEN_LOCAL:
					initiate |= CS_OPEN_LOCAL;
					continue;
				case CS_REMOTE_OPEN_NOW:
					initiate |= CS_OPEN_NOW;
					continue;
				case CS_REMOTE_OPEN_READ:
					continue;
				case CS_REMOTE_OPEN_SHARE:
					initiate |= CS_OPEN_SHARE;
					continue;
				case CS_REMOTE_OPEN_TEST:
					initiate |= CS_OPEN_TEST;
					continue;
				case CS_REMOTE_OPEN_TRUST:
					initiate |= CS_OPEN_TRUST;
					continue;
				default:
					error(2, "%c: unknown open flag", *(host - 1));
					break;
				}
				break;
			}
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (msg & MSG_LIST)
	{
		if (!(path = *argv++))
			sp = sfstdin;
		else if (*argv)
			error(ERROR_USAGE|4, "%s", optusage(NiL));
		else if (!(sp = sfopen(NiL, path, "r")))
			error(ERROR_SYSTEM|3, "%s: cannot read", path);
		return msgcat(sp, msg, call, terse) != 0;
	}
	if (translate)
	{
		if (*argv)
			while (host = *argv++)
				address(host);
		else
		{
			sfopen(sfstdin, NiL, "rt");
			while (host = sfgetr(sfstdin, '\n', 1))
				address(host);
		}
		return 0;
	}
	state.local = csname(0);
	if ((path = *argv++) && path[0] == '-' && !path[1])
	{
		path = *argv++;
		initiate |= CS_OPEN_TEST;
	}
	if (remote)
	{
		if (!path)
			return 1;
		if (initiate & CS_OPEN_AGENT)
		{
			register char*	s = path;
			register char*	t = tmp;
			register int	n = 0;

			/*
			 * get the unqualified-host connect stream path
			 */

			while (*t++ = *s)
				switch (*s++)
				{
				case '/':
					while (*s == '/')
						s++;
					n++;
					break;
				case '.':
					if (n == 3)
						for (t--; *s && *s != '/'; s++);
					break;
				}
			path = tmp;
		}
		if ((fd = csopen(path, initiate)) < 0)
			return 1;
		if (initiate & CS_OPEN_AGENT)
		{
			*cs.control = CS_MNT_AUTH;
			remote = !access(cs.mount, F_OK);
			sfprintf(sfstdout, "%s%s\n", cspath(fd, 0), remote ? ".A" : "");
			if (remote)
			{
				close(fd);
				sfsync(sfstdout);
				if (csauth(-1, cs.mount, NiL))
					return 1;
			}
		}
	}
	else if (sig)
	{
		if (path)
		{
			do
			{
				if ((fd = csopen(path, CS_OPEN_TEST)) < 0)
				{
					error(ERROR_SYSTEM|2, "%s: cannot open connect stream", path);
					continue;
				}
				close(fd);
				if (cs.flags & CS_ADDR_REMOTE)
					host = cs.host;
				else
				{
					*cs.control = CS_MNT_PROCESS;
					if (pathgetlink(cs.mount, buf, sizeof(buf)) <= 0)
					{
						error(ERROR_SYSTEM|2, "%s: cannot get service process mount", path);
						continue;
					}
					if (tokscan(buf, NiL, "/proc/%s", &proc) == 1)
						host = 0;
					else if (tokscan(buf, NiL, "/n/%s/proc/%s", &host, &proc) != 2)
					{
						error(2, "%s: %s: invalid service process mount", path, buf);
						continue;
					}
				}
				sfsprintf(tmp, sizeof(tmp), "-%s", sig);
				ap = av;
				if (host && !streq(host, state.local))
				{
					*ap++ = CS_REMOTE_SHELL;
					*ap++ = host;
					if (*cs.user)
					{
						*ap++ = "-l";
						*ap++ = cs.user;
					}
				}
				*ap++ = "kill";
				*ap++ = tmp;
				*ap++ = proc;
				*ap = 0;
				if (procclose(procopen(av[0], av, NiL, NiL, PROC_UID|PROC_GID|(*argv ? PROC_OVERLAY : 0))))
					error(ERROR_SYSTEM|2, "%s: cannot %s %s to kill server", path, av[0], host);
			} while (path = *argv++);
		}
	}
	else if (state.list & LIST)
	{
		if (path)
			error(1, "%s: argument not expected", path);
		ap = av;
		*ap++ = csvar(CS_VAR_LOCAL, 0);
		if (pathpath(csvar(CS_VAR_SHARE, 0), "", PATH_EXECUTE, tmp, sizeof(tmp)))
			*ap++ = tmp;
		*ap = 0;
		ftwalk((char*)av, list, FTW_MULTIPLE|FTW_PHYSICAL, order);
	}
	else if (attr)
	{
		if (!path)
		{
			while (proc = csattr(NiL, attr))
				sfputr(sfstdout, proc, '\n');
		}
		else
		{
			n = *argv != 0;
			hostenv = streq(attr, "-");
			terse = streq(attr, "name");
			do
			{
				if (proc = csattr(path, attr))
				{
					if (hostenv)
						sfputr(sfstdout, "name", '=');
					if (!terse && (hostenv || n))
						sfputr(sfstdout, path, ' ');
					sfputr(sfstdout, proc, '\n');
				}
				else if (streq(path, CS_HOST_SHARE) && (sp = csinfo(path, NiL)))
				{
					while (path = sfgetr(sp, '\n', 1))
						if (proc = csattr(path, attr))
						{
							if (hostenv)
								sfputr(sfstdout, "name", '=');
							if (!terse)
								sfputr(sfstdout, path, ' ');
							sfputr(sfstdout, proc, '\n');
						}
						else
							error(2, "%s: no host info", path);
					sfclose(sp);
				}
				else
					error(2, "%s: no host info", path);
			} while (path = *argv++);
		}
		return 0;
	}
	else if (hostenv)
	{
		if (!path || streq(path, "local"))
			path = state.local;
		proc = csattr(path, "type");
		sfprintf(sfstdout, "%s=%s %s=%s\n", CO_ENV_HOST, path, CO_ENV_TYPE, proc ? proc : "unknown");
		return 0;
	}
	else if (path)
	{
		if ((fd = csopen(path, initiate)) < 0)
			error(ERROR_SYSTEM|3, "%s: cannot open connect stream", path);
		if (state.list & LIST_MOUNT)
		{
			if (*argv)
				error(1, "%s: argument not expected", path);
			if (cs.flags & CS_ADDR_REMOTE)
				error(1, "%s: remote connect stream", path);
			else
			{
				*(cs.control - 1) = 0;
				sfputr(sfstdout, cs.mount, '\n');
			}
		}
		else if (interactive)
			return csclient(fd, path, "cs> ", NiL, clientflags) || error_info.errors;
		else
		{
			if (*argv)
			{
				close(0);
				close(1);
				if (dup(fd) != 0 || dup(fd) != 1)
					error(ERROR_SYSTEM|3, "%s: cannot redirect connect stream", path);
				close(fd);
				if (!(initiate & CS_OPEN_TEST) && csdaemon((1<<0)|(1<<1)))
					error(ERROR_SYSTEM|3, "%s: cannot dive into background", path);
				procopen(*argv, argv, NiL, NiL, PROC_OVERLAY|PROC_UID|PROC_GID);
				error(ERROR_SYSTEM|4, "%s: %s: cannot execute", path, *argv);
			}
			sfprintf(sfstdout, "%s\n", cspath(fd, 0));
		}
	}
	else if (interactive)
		error(3, "connect stream argument expected");
	else
		sfprintf(sfstdout, "%s\n", cspath(0, 0));
	return error_info.errors != 0;
}
