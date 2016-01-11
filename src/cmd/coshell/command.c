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
 * AT&T Bell Laboratories
 *
 * remote coshell server command interface support
 */

#include "service.h"

#define NOARG	(((int)1)<<(CHAR_BIT*sizeof(int)-2))

static struct
{
	int*	pass;
	int	fd;
} svc;

/*
 * send command message to fd and read ack
 * 0 returned after the last command
 */

static int
cmdsend(int fd, int c, register char* s, register int n, int* code)
{
	int	i;
	int	m;
	int	x;
	CSPOLL	fds[5];
	char	cmd[1024];

	if (cswrite(fd, s, n) != n)
		error(ERROR_SYSTEM|3, "message write error");
	if (c == 'Q') return(0);
	s = cmd;
	if (((n = read(fd, s, sizeof(cmd) - 1)) == 4 || n > 6) && s[0] == 'a' && isdigit(s[2]))
	{
		if (n == 4) n = read(fd, s, sizeof(cmd) - 1);
		else
		{
			n -= 4;
			s += 4;
		}
	}
	if (n < 6 || !isdigit(*(s += 4)))
	{
		if (code) *code = EXIT_NOEXEC;
		return(0);
	}
	if (code && (n = (int)strtol(s, NiL, 10))) *code = n;
	if (state.indirect.con)
	{
		n = 0;
		fds[n].fd = state.indirect.con;
		fds[n++].events = CS_POLL_READ|CS_POLL_CONNECT;
		if (fds[n].fd = state.indirect.err)
			fds[n++].events = CS_POLL_READ;
		fds[n].fd = state.indirect.out;
		fds[n++].events = CS_POLL_READ;
		i = n;
		while (cspoll(fds, n, 0) > 0)
			for (m = 0; m < n; m++)
				if (fds[m].status & CS_POLL_CONNECT)
				{
					if (n < elementsof(fds) && csrecv(fds[m].fd, NiL, &x, 1) == 1)
					{
						svc.pass[x] = 0;
						fds[n].fd = x;
						fds[n].status = 0;
						fds[n++].events = CS_POLL_READ;
					}
				}
				else if (fds[m].status & CS_POLL_READ)
				{
					if (x = svc.pass[fds[m].fd])
					{
						if ((c = read(fds[m].fd, cmd, sizeof(cmd))) > 0)
							cswrite(x, cmd, c);
						else
						{
							close(fds[m].fd);
							fds[m].events = 0;
						}
					}
					else if (csread(fds[m].fd, cmd, 7, CS_EXACT) == 7 && cmd[0] == '#')
						svc.pass[fds[m].fd] = (int)strtol(cmd + 1, NiL, 10);
					else
					{
						close(fds[m].fd);
						fds[m].events = 0;
					}
				}
		while (--i > n)
			if (fds[i].events)
				close(fds[i].fd);
	}
	return(1);
}

#ifdef SIGCONT

/*
 * send a job control kill message to fd
 */

static void
cmdkill(int fd, int sig)
{
	int	n;
	char	buf[32];

	n = sfsprintf(buf, sizeof(buf), "#%05d\nk 0 %d\n", 0, sig);
	sfsprintf(buf, 7, "#%05d\n", n - 7);
	cmdsend(fd, 'Q', buf, n, NiL);
}

/*
 * pass job control signals to the server and self
 */

static void
cmdstop(int sig)
{
	cmdkill(svc.fd, sig);
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
	cmdkill(svc.fd, SIGCONT);
	signal(sig, cmdstop);
}

#endif

/*
 * command loop on server stream fd
 * if ap!=0 then it is an argv of commands
 * otherwise the commands are read from stdin
 * exit code returned
 */

int
command(int fd, char** ap)
{
	register char*	s;
	register int	n;
	int		c;
	int		m;
	int		helped = 0;
	int		code = 0;
	int		prompt;
	char*		t;
	char*		arg;
	char*		att;

#ifdef SIGCONT
	svc.fd = fd;
#ifdef SIGTSTP
	signal(SIGTSTP, cmdstop);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, cmdstop);
#endif
#ifdef SIGTTOU
	signal(SIGTTOU, cmdstop);
#endif
#endif
	if (state.indirect.con)
	{
		if (!(svc.pass = newof(0, int, (int)strtol(astconf("OPEN_MAX", NiL, NiL), NiL, 0), 0)))
			error(3, "out of space [pass]");
		svc.pass[state.indirect.err] = 2;
		svc.pass[state.indirect.out] = 1;
	}
	prompt = !ap && isatty(0);
	for (;;)
	{
		if (!ap)
		{
			if (prompt) error(ERROR_PROMPT, "%s> ", CO_ID);
			if (!(s = sfgetr(sfstdin, '\n', 1))) break;
			while (isspace(*s)) s++;
			if (c = *s) s++;
		}
		else if (!(s = *ap++)) break;
		else if (*s == '-' && s[1])
		{
			c = *++s;
			s++;
		}
		if (c == 'q') break;
		while (isspace(*s)) s++;
		switch (c)
		{
		case 0:
		case '#':
			continue;
		case 'a':
		case 'c':
		case 'l':
		case 'o':
			if (!*s && ap && *ap) s = *ap++;
			sfprintf(state.string, "#%05d\nS %c - 0 ", 0, c);
			if (!s || !*s) sfprintf(state.string, "%s\n", (char*)0);
			else sfprintf(state.string, "(%d:%s)\n", strlen(s), s);
			break;
		case 'd':
		case 'f':
		case 'g':
		case 'j':
		case 'k':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'Q':
			if (!*s && ap && *ap && **ap != '-') s = *ap++;
			while (isspace(*s)) s++;
			m = isalpha(*s) ? *s++ : '-';
			while (isspace(*s)) s++;
			if (isdigit(*s))
			{
				n = strtol(s, &t, 0);
				s = t;
			}
			else n = *s ? *s : NOARG;
			sfprintf(state.string, "#%05d\nS %c %c %d ", 0, c, m, n);
			if (!s || !*s) sfprintf(state.string, "%s\n", (char*)0);
			else sfprintf(state.string, "(%d:%s)\n", strlen(s), s);
			break;
		case 'r':
		case 'R':
			if (!*s && ap && *ap) s = *ap++;
			m = 0;
			att = s;
			for (;;)
			{
				if (!(n = *s++))
				{
					s--;
					break;
				}
				if (n == '\\')
				{
					if (*s) s++;
				}
				else if (!m)
				{
					if (n == '\'' || n == '"') m = n;
					else if (isspace(*s))
					{
						*s++ = 0;
						while (isspace(*s)) s++;
						break;
					}
				}
				else if (n == m) m = 0;
			}
			sfprintf(state.string, "#%05d\ne %d %d %s %s %s",
				0,
				1,
				c == 'r' ? CO_SILENT : 0,
				NiL,
				NiL, 
				NiL);
			if (!att[0] || att[0] == '-' && !att[1]) sfprintf(state.string, " %s", NiL);
			else sfprintf(state.string, " (%d:%s)", strlen(att), att);
			arg = coinit(CO_SERVER);
			sfprintf(state.string, " (%d:%s)", strlen(arg), arg);
			t = state.buf;
			if (!*(arg = s) && (!ap || !*ap)) arg = "hostname 2>/dev/null || uname -n";
			t += sfsprintf(t, state.buf + state.buflen - t, "%s", arg);
			if (ap)
			{
				while (s = *ap++)
					t += sfsprintf(t, state.buf + state.buflen - t, " %s", s);
				ap--;
			}
			sfprintf(state.string, " (%d:%s)\n", t - state.buf, state.buf);
			break;
		default:
			if (ap) error(3, "%c: unknown command -- use h for help", c);
			error(2, "%c: unknown command", c);
			if (helped) continue;
			/*FALLTHROUGH*/
		case 'h':
		case '?':
			helped++;
			error(0, "\
\n\
	a host ...	change host shell attributes\n\
	c host ...	close host shell\n\
	d [level]	set debug trace stderr [and level]\n\
	f [fd]		internal fd status [drop CON fd]\n\
	g		global state\n\
	h		help\n\
	j		job status\n\
	k [ckst] job	terminate [continue|kill|stop|term] job\n\
	l host		list matching host names\n\
	o host ...	open host shell\n\
	q		quit\n\
	r host [cmd]	run cmd [hostname] on host\n\
	s [aelopst]	shell status [attr|every|long|open|pid|sched|temp]\n\
	t		shell and user connection totals\n\
	u		user connection status\n\
	v		server version\n\
	Q		kill server and quit\n\
");
			continue;
		}
		n = sfstrtell(state.string);
		sfstrseek(state.string, 0, SEEK_SET);
		sfprintf(state.string, "#%05d\n", n - 7);
		if (!cmdsend(fd, c, sfstrseek(state.string, 0, SEEK_SET), n, &code)) break;
	}
	return(code);
}

/*
 * return file mode given fd
 * if must!=0 then 0 returned if fstat fails
 */

static char*
ffmtmode(int fd, int must)
{
	char*		p;
	struct stat	st;

	if (fstat(fd, &st)) return(must ? (char*)0 : (char*)"**ERROR** ");
	p = fmtmode(st.st_mode ? st.st_mode : (S_IRUSR|S_IWUSR), 0);
	if (!st.st_mode)
#ifdef S_ISSOCK
		*p = 's';
#else
		*p = 'p';
#endif
	return(p);
}

/*
 * do CO_server op for user fd
 */

void
server(int fd, int op, int sub, int arg, char* dat)
{
	register char*		s;
	register int		n;
	register Connection_t*	con;
	int			u;
	int			hdr;
	char*			t;
	char*			v;
	Cojob_t*		jp;
	Coshell_t*		sp;
	Coshell_t**		sv;
	Coattr_t		attr;
	Coshell_t		tot;
	char			num[4][12];

	con = state.con;
	hdr = 0;
	switch (op)
	{
	case 0:
		break;
	case 'a':
		if (dat)
		{
			t = tokopen(dat, 1);
			while (s = tokread(t))
				if (!search(SET, s, NiL, NiL))
					error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: invalid host name", s);
			tokclose(t);
		}
		break;
	case 'c':
		if (dat)
		{
			t = tokopen(dat, 1);
			while (s = tokread(t))
			{
				if (!(sp = search(GET, s, NiL, NiL))) error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: not found", s);
				else if (!sp->fd) error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: host not open", s);
				else shellclose(sp, con[fd].info.user.fds[2]);
			}
			tokclose(t);
		}
		break;
	case 'd':
		if (arg == 't')
		{
			cs.flags |= CS_TEST;
			break;
		}
		else if (arg == 'T')
		{
			cs.flags &= ~CS_TEST;
			break;
		}
		arg = (arg == NOARG) ? error_info.trace : -arg;
		message((error_info.trace, "level=%d output=%s", -arg, dat ? dat : cspath(state.indirect.con ? 2 : con[fd].info.user.fds[2], 0)));
		error_info.trace = arg;
		if (dat)
		{
			if ((n = open(dat, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) >= 0)
			{
				close(2);
				dup(n);
				close(n);
			}
		}
		else if (!state.indirect.con)
		{
			close(2);
			dup(con[fd].info.user.fds[2]);
		}
		break;
	case 'f':
		if (arg == NOARG)
		{
			sfprintf(state.string, "CON  TYPE  INFO\n");
			for (n = 0; n <= state.fdtotal; n++)
				switch (con[n].type)
				{
				case ANON:
					sfprintf(state.string, "%3d  anon\n", n);
					break;
				case DEST:
					sfprintf(state.string, "%3d  dest\n", n);
					break;
				case IDENT:
					sfprintf(state.string, "%3d  ident\n", n);
					break;
				case MESG:
					sfprintf(state.string, "%3d  mesg  %s %s\n", n, ffmtmode(n, 0), state.mesg);
					break;
				case PASS:
					if (jp = con[n].info.pass.job) sfprintf(state.string, "%3d  pass  %-3d %s %d%s\n", n, con[n].info.pass.fd, jp->shell->name, jp->pid, con[n].info.pass.serialize ? " serialize" : "");
					else sfprintf(state.string, "%3d  pass  %-3d zombie\n", n, con[n].info.pass.fd);
					break;
				case POLL:
					sfprintf(state.string, "%3d  poll  %s %s\n", n, ffmtmode(n, 0), state.service);
					break;
				case PUMP:
					sfprintf(state.string, "%3d  pump  %s %s\n", n, ffmtmode(n, 0), state.pump);
					break;
				case SHELL:
					sfprintf(state.string, "%3d  shell %s\n", n, con[n].info.shell->name);
					break;
				case USER:
					if (con[n].info.user.fds[0] == n) sfprintf(state.string, "%3d  query %-3d %-3d %s\n", n, con[n].info.user.fds[1], con[n].info.user.fds[2], (sp = con[n].info.user.home) ? sp->name : "");
					else sfprintf(state.string, "%3d  user  %-3d %-3d %-3d %s\n", n, con[n].info.user.fds[0], con[n].info.user.fds[1], con[n].info.user.fds[2], (sp = con[n].info.user.home) ? sp->name : "");
					break;
				case UCMD:
					t = "ucmd";
					goto path;
				case UERR:
					t = "uerr";
					goto path;
				case UOUT:
					t = "uout";
					goto path;
				case 0:
					t = "open";
				path:
					if (v = ffmtmode(n, 1)) sfprintf(state.string, "%3d  %s  %s %s\n", n, t, v, cspath(n, 0));
					else if (con[n].type) sfprintf(state.string, "%3d  %s  error\n", n, t);
					break;
				}
		}
		else if (arg >= state.fdtotal || !con[arg].type) error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "cannot drop CON %d", arg);
		else drop(arg);
		break;
	case 'g':
		/*
		 * string values
		 */

		sfprintf(state.string, "\n");
		sfprintf(state.string, "   version  %s\n", state.version);
		sfprintf(state.string, "      mesg  %s\n", state.mesg);
		if (state.identify)
			sfprintf(state.string, "  identify  %s\n", fmtesc(state.identify));
		if (state.migrate)
			sfprintf(state.string, "   migrate  %s\n", state.migrate);
		if (state.profile && *state.profile)
			sfprintf(state.string, "   profile  %s\n", state.profile);
		sfprintf(state.string, "      pump  %s\n", state.pump);
		sfprintf(state.string, "    remote  %s\n", state.remote);
		sfprintf(state.string, "   service  %s\n", state.service);
		sfprintf(state.string, "     shell  %s\n", state.sh);

		/*
		 * readonly values
		 */

		sfprintf(state.string, "\n");

		sfprintf(state.string, "    access  %-7s", state.access < cs.time ? "EXPIRED" : fmtelapsed(state.access - cs.time, 1));
		sfprintf(state.string, "  joblimit  %-6d ", state.joblimit);
		sfprintf(state.string, "       pid  %-6d ", getpid());
		sfprintf(state.string, "       sys  %-6s\n", fmtelapsed(state.sys, CO_QUANT));


		sfprintf(state.string, "     clock  %-6s ", fmtelapsed(cs.time - state.clock, 1));
		sfprintf(state.string, "      jobs  %-6d ", state.jobs);
		sfprintf(state.string, "      real  %-6s ", fmtelapsed(state.real, 1));
		sfprintf(state.string, "      user  %-6s\n", fmtelapsed(state.user, CO_QUANT));

		sfprintf(state.string, "      cmds  %-6d ", state.cmds);
		sfprintf(state.string, "   jobwait  %-6d ", state.jobwait);
		sfprintf(state.string, "   running  %-6d ", state.running);
		sfprintf(state.string, "     users  %-6d\n", state.users);

		sfprintf(state.string, "   connect  %-6d ", state.connect);
		sfprintf(state.string, "      open  %-6d ", state.open);
		sfprintf(state.string, "    shells  %-6d ", state.shells);
		sfprintf(state.string, "    wakeup  %-6s\n", fmtelapsed(state.wakeup, 1000));

		sfprintf(state.string, "   fdtotal  %-6d ", state.fdtotal);
		sfprintf(state.string, "  override  %-6d ", state.override);
		sfprintf(state.string, " shellwait  %-6d\n", state.shellwait);

		/*
		 * global values
		 */

		sfprintf(state.string, "\n");

		sfprintf(state.string, "      busy  %-6s ", fmtelapsed(state.busy, 1));
		sfprintf(state.string, "     grace  %-6s ", fmtelapsed(state.grace, 1));
		sfprintf(state.string, "    percpu  %-6d ", state.percpu);
		sfprintf(state.string, "   peruser  %-6d\n", state.peruser);

		sfprintf(state.string, "     debug  %-6d ", -error_info.trace);
		sfprintf(state.string, "   maxidle  %-6s ", fmtelapsed(state.maxidle, 1));
		sfprintf(state.string, "   perhost  %-6d ", state.perhost);
		sfprintf(state.string, "      pool  %-6d\n", state.pool);

		sfprintf(state.string, "   disable  %-6s ", fmtelapsed(state.disable, 1));
		sfprintf(state.string, "   maxload  %-6s ", fmtfloat(state.maxload));
		sfprintf(state.string, " perserver  %-6d\n", state.perserver);

		/*
		 * done
		 */

		sfprintf(state.string, "\n");
		break;
	case 'j':
		for (jp = state.job; jp <= state.jobmax; jp++)
			if (jp->pid)
			{
				switch (jp->pid)
				{
				case QUEUE:
					t = "QUEUE";
					break;
				case START:
					t = "START";
					break;
				case WARP:
					t = "WARP";
					break;
				default:
					sfsprintf(t = num[0], sizeof(num[0]), "%d", jp->pid);
					break;
				}
				if (!hdr)
				{
					hdr = 1;
					sfprintf(state.string, "JOB USR RID   PID   TIME HOST             LABEL\n");
				}
				v = con[jp->fd].info.user.attr.label;
				sfprintf(state.string, "%3d%4d%4d%6s%7s%c%-16s %s%s%s", jp - state.job, jp->fd, jp->rid, t, fmtelapsed(cs.time - jp->start, 1), jp->ref <= 0 ? '*' : jp->lost ? (jp->lost > cs.time ? '?' : '!') : ' ', jp->shell->name, v, *v ? " " : "", jp->label);
				if (jp->sig) sfprintf(state.string, " [%s]", fmtsignal(jp->sig));
				sfputc(state.string, '\n');
			}
		break;
	case 'k':
		if (arg == NOARG)
			error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "job id required");
		else if ((jp = state.job + arg) > state.jobmax || !jp->pid)
			error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%d: invalid job id", arg);
		else switch (sub)
		{
#ifdef SIGCONT
		case 'c':
			jobkill(jp, SIGCONT);
			break;
#endif
		case 'k':
			jobkill(jp, SIGKILL);
			break;
#ifdef SIGSTOP
		case 's':
			jobkill(jp, SIGSTOP);
			break;
#endif
		case 't':
		case '-':
			jobkill(jp, SIGTERM);
			break;
		default:
			error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%c: invalid signal id", sub);
			break;
		}
		break;
	case 'l':
		sv = state.shellv;
		sp = state.shell;
		if (dat && *dat) attributes(dat, &attr, NiL);
		else attr = con[fd].info.user.attr;
		do
		{
			if (match(sp, &attr, 0))
				*sv++ = sp;
		} while ((sp = sp->next) != state.shell);
		if (n = sv - state.shellv)
		{
			*sv = 0;
			strsort((char**)(sv = state.shellv), n, byrank);
			if (dat && (attr.global.set & SETPOOL) && attr.global.pool > 0 && attr.global.pool < n) sv[attr.global.pool] = 0;
			while (sp = *sv++)
				sfprintf(state.string, "%s\n", sp->name);
		}
		break;
	case 'o':
		if (dat)
		{
			t = tokopen(dat, 1);
			while (s = tokread(t))
			{
				if (!(sp = search(NEW, s, NiL, NiL))) error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: invalid host name", s);
				else if (sp->fd) error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: host already open", s);
				else shellopen(sp, con[fd].info.user.fds[2]);
			}
			tokclose(t);
		}
		break;
	case 's':
		sv = state.shellv;
		sp = state.shell;
		do
		{
			if (sub == 'a' || sub == 'e' || sp->fd || (sub == 'l' || sub == 's') && sp->temp && !(sp->flags & IGN)) *sv++ = sp;
		} while ((sp = sp->next) != state.shell);
		if (n = sv - state.shellv)
		{
			*sv = 0;
			strsort((char**)(sv = state.shellv), n, sub == 'a' || sub == 'e' || sub == 'p' ? byname : sub == 't' ? bytemp : byrank);
			switch (sub)
			{
			case 'a':
				sfprintf(state.string, "HOST         CONF\n%-12.12s busy=%s grace=%s name=%s percpu=%d perserver=%d peruser=%d pool=%d\n", CS_HOST_LOCAL, fmtelapsed(state.busy, 1), fmtelapsed(state.grace, 1), state.shell->name, state.percpu, state.perserver, state.peruser, state.pool);
				break;
			case 'o':
			case 's':
			case 't':
				sfprintf(state.string, "CON OPEN USERS     UP CONNECT UPDATE OVERRIDE   IDLE   TEMP     RANK HOST\n");
				break;
			case 'p':
				sfprintf(state.string, "PID    HOST\n%-6d %s\n", getpid(), cs.cs);
				break;
			default:
				sfprintf(state.string, "CON JOBS TOTAL   USER    SYS   IDLE CPU  LOAD RATING  BIAS TYPE     HOST\n");
				break;
			}
			while (sp = *sv++)
			{
				switch (sub)
				{
				case 'a':
					sfprintf(state.string, "%-12.12s rating=%s", sp->name, fmtfloat(sp->rating));
					if (sp->flags & IGN)
						sfprintf(state.string, " ignore=1");
					if (sp->bias != BIAS)
						sfprintf(state.string, " bias=%s", fmtfloat(sp->bias));
					if (sp->cpu != 1)
						sfprintf(state.string, " cpu=%d", sp->cpu);
					if (sp->scale != sp->cpu)
						sfprintf(state.string, " scale=%d", sp->scale);
					if (sp->idle)
						sfprintf(state.string, " idle=%s", fmtelapsed(sp->idle, 1));
					if (sp->type[0] != '*' || sp->type[1])
						sfprintf(state.string, " type=%s", sp->type);
					if (sp->access)
						sfprintf(state.string, " access=%s", sp->access);
					if (sp->bypass)
						sfprintf(state.string, " bypass=%s", sp->bypass);
					if (sp->shell[0])
						sfprintf(state.string, " shell=%s", sp->shell);
					sfprintf(state.string, " %s\n", sp->misc);
					break;
				case 'p':
					if (sp->fd > 0) sfprintf(state.string, "%-6d %s\n", sp->pid, sp->name);
					break;
				default:
					if (sp->fd < 0) t = "+";
					else if (sp->fd > 0) sfsprintf(t = num[0], sizeof(num[0]), "%d", sp->fd);
					else t = (sp->temp && !(sp->flags & IGN)) ? "-" : "@";
					if (sub == 'o' || sub == 's' || sub == 't') sfprintf(state.string, "%3s%c%4d%6d%7s%8s%7s%7s/%d%7s%7s%9s %s\n"
					, t
					, sp->fd > 0 && state.con[sp->fd].error ? '?' : ' '
					, sp->open
					, sp->stat.users
					, sp->stat.up < 0 ? "DOWN" : fmtelapsed(sp->stat.up, 1)
					, sp->start ? fmtelapsed(cs.time - sp->start, 1) : ""
					, fmtelapsed(sp->update > cs.time ? sp->update - cs.time : 0, 1)
					, fmtelapsed(sp->override > cs.time ? sp->override - cs.time : 0, 1)
					, sp->home
					, fmtelapsed(sp->idle, 1)
					, fmtfloat(sp->temp >> (CHAR_BIT * sizeof(sp->temp) / 2))
					, fmtfloat(sp->rank)
					, sp->name
					);
					else sfprintf(state.string, "%3s %4d%6d%7s%7s%7s%c%3d%6s%7s%6s %-8.8s %s\n"
					, t
					, sp->running
					, sp->total
					, fmtelapsed(sp->user, CO_QUANT)
					, fmtelapsed(sp->sys, CO_QUANT)
					, fmtelapsed(sp->stat.up < 0 ? -sp->stat.up : sp->stat.idle, 1)
					, sp->stat.up < 0 ? '-' : (sp->mode & SHELL_DISABLE) ? '$' : (sp->mode & SHELL_DENIED) ? '!' : sp->idle && sp->stat.idle < sp->idle ? (sp->home ? '~' : '*') : ' '
					, sp->cpu
					, fmtfloat(sp->stat.load / sp->scale)
					, fmtfloat(sp->rating)
					, fmtfloat(sp->bias)
					, sp->type
					, sp->name
					);
					break;
				}
			}
		}
		break;
	case 't':
		memzero(&tot, sizeof(tot));
		sfprintf(state.string, "SHELLS  USERS    JOBS  CMDS     UP   REAL   USER    SYS     CPU  LOAD RATING\n");
		tot.running = state.running;
		sp = state.shell;
		do
		{
			if (sp->fd)
			{
				tot.fd++;
				tot.cpu += sp->cpu;
				tot.stat.load += sp->cpu * sp->stat.load / sp->scale;
				tot.rating += sp->cpu * sp->rating;
			}
		} while ((sp = sp->next) != state.shell);
		if (tot.cpu)
		{
			tot.stat.load /= tot.cpu;
			tot.rating /= tot.cpu;
		}
		for (n = u = 0; n <= state.fdtotal; n++)
			if (con[n].type == USER) u++;
		if (state.running)
		{
			state.real += cs.time - state.clock;
			state.clock = cs.time;
		}
		sfsprintf(num[0], sizeof(num[0]), "%d/%d", tot.fd, state.shells);
		sfsprintf(num[1], sizeof(num[1]), "%d/%d", u, state.users);
		sfsprintf(num[2], sizeof(num[2]), "%d/%d", tot.running, state.jobs);
		sfsprintf(num[3], sizeof(num[3]), "%d/%d+%d", tot.cpu, state.pool, state.override);
		sfprintf(state.string, "%6s%7s%8s%6d%7s%7s%7s%7s%8s%6s%7s\n", num[0], num[1], num[2], state.cmds, fmtelapsed(cs.time - state.start, 1), fmtelapsed(state.real, 1), fmtelapsed(state.user, CO_QUANT), fmtelapsed(state.sys, CO_QUANT), num[3], fmtfloat(tot.stat.load), fmtfloat(tot.rating));
		break;
	case 'u':
		for (n = 0; n <= state.fdtotal; n++)
			if (con[n].type == USER)
			{
				if (!hdr)
				{
					hdr = 1;
					sfprintf(state.string, "CON   PID JOBS TOTAL TTY                          LABEL\n");
				}
				sfprintf(state.string, "%3d%6d%5d%6d %-28s %s\n", n, con[n].info.user.pid, con[n].info.user.running, con[n].info.user.total, cspath(con[n].info.user.fds[2], 0), con[n].info.user.attr.label);
			}
		break;
	case 'v':
		sfprintf(state.string, "%s\n", state.version);
		break;
	case 'Q':
		exit(arg);
	default:
		error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%c: unknown server op", op);
		break;
	}
	if ((n = sfstrtell(state.string)) > 0)
		cswrite(con[fd].info.user.fds[1], sfstrseek(state.string, 0, SEEK_SET), n);
}
