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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * open a new coshell
 */

#include "colib.h"

#include <namval.h>
#include <proc.h>
#include <sfdisc.h>
#include <tok.h>

static const Namval_t	options[] =
{
	"cross",	CO_CROSS,
	"debug",	CO_DEBUG,
	"devfd",	CO_DEVFD,
	"ignore",	CO_IGNORE,
	"orphan",	CO_ORPHAN,
	"silent",	CO_SILENT,
	"separate",	CO_SEPARATE,
	"service",	CO_SERVICE,
	0,		0
};

Costate_t		state = { "libcoshell:coshell" };

/*
 * called when ident sequence hung
 */

static void
hung(int sig)
{
	NoP(sig);
	close(sffileno(state.current->msgfp));
}

/*
 * close all open coshells
 */

static void
clean(void)
{
	coclose(NiL);
}

#ifdef SIGCONT

/*
 * pass job control signals to the coshell and self
 */

static void
stop(int sig)
{
	cokill(NiL, NiL, sig);
	signal(sig, SIG_DFL);
	sigunblock(sig);
	kill(getpid(), sig);
	cokill(NiL, NiL, SIGCONT);
	signal(sig, stop);
}

#endif

/*
 * called by stropt() to set options
 */

static int
setopt(void* handle, register const void* p, int n, const char* v)
{
	Coshell_t*	co = (Coshell_t*)handle;
	Coservice_t*	cs;
	char*		s;
	char**		a;

	NoP(v);
	if (p)
	{
		if (n)
		{
			co->flags |= ((Namval_t*)p)->value;
			if (((Namval_t*)p)->value == CO_SERVICE && v && (cs = vmnewof(co->vm, 0, Coservice_t, 1, 2 * strlen(v))))
			{
				a = cs->argv;
				*a++ = s = cs->path = cs->name = (char*)(cs + 1);
				while (*s = *v++)
					if (*s++ == ':')
					{
						*(s - 1) = 0;
						if (*v == '-')
						{
							v++;
							if (*v == '-')
								v++;
						}
						if (strneq(v, "command=", 8))
							cs->path = s + 8;
						else if (strneq(v, "state=", 6))
							cs->db = s + 6;
						else if (strneq(v, "db=", 3))
							cs->db = s + 3;
						else if (a < &cs->argv[elementsof(cs->argv)-2] && *v && *v != ':')
						{
							*a++ = s;
							*s++ = '-';
							*s++ = '-';
						}
					}
				if (cs->db)
					*a++ = cs->db;
				*a = 0;
				cs->next = co->service;
				co->service = cs;
			}
		}
		else
			co->mask |= ((Namval_t*)p)->value;
	}
	return 0;
}

Coshell_t*
coopen(const char* path, int flags, const char* attributes)
{
	register Coshell_t*	co;
	register char*		s;
	register int		i;
	char*			t;
	int			n;
	Proc_t*			proc;
	Cojob_t*		cj;
	Vmalloc_t*		vm;
	Sfio_t*			sp;
	Sig_handler_t		handler;
	int			pio[4];
	long			ops[5];
	char			devfd[16];
	char			evbuf[sizeof(CO_ENV_MSGFD) + 8];
	char*			av[8];
	char*			ev[2];

	static char*	sh[] = { 0, 0, "ksh", "sh", "/bin/sh" };

	if (!state.type && (!(s = getenv(CO_ENV_TYPE)) || !(state.type = strdup(s))))
		state.type = "";
	if ((flags & CO_ANY) && (co = state.coshells))
		return co;
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)) || !(co = vmnewof(vm, 0, Coshell_t, 1, 0)))
	{
		if (vm)
			vmclose(vm);
		errormsg(state.lib, ERROR_LIBRARY|2, "out of space");
		return 0;
	}
	co->vm = vm;
	co->index = ++state.index;
	stropt(getenv(CO_ENV_OPTIONS), options, sizeof(*options), setopt, co);
	if (attributes)
		stropt(attributes, options, sizeof(*options), setopt, co);
	co->flags |= ((flags | CO_DEVFD) & ~co->mask);
	if (co->flags & CO_SEPARATE)
	{
		co->flags &= ~CO_SEPARATE;
		co->mode |= CO_MODE_SEPARATE;
	}
	co->flags |= CO_INIT;
	if (co->mode & CO_MODE_SEPARATE)
	{
		flags = 0;
		proc = 0;
	}
	else
	{
		for (i = 0; i < elementsof(pio); i++)
			pio[i] = -1;
		if (pipe(&pio[0]) < 0 || pipe(&pio[2]) < 0)
		{
			errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot allocate pipes");
			goto bad;
		}
		if (flags & CO_SHELL)
			for (i = 0; i < elementsof(pio); i++)
				if (pio[i] < 10 && (n = fcntl(pio[i], F_DUPFD, 10)) >= 0)
				{
					close(pio[i]);
					pio[i] = n;
				}
		co->cmdfd = pio[1];
		co->gsmfd = pio[3];
		if (!(co->msgfp = sfnew(NiL, NiL, 256, pio[2], SF_READ)))
		{
			errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot allocate message stream");
			goto bad;
		}
		sfdcslow(co->msgfp);
		ops[0] = PROC_FD_DUP(pio[0], 0, PROC_FD_PARENT);
		ops[1] = PROC_FD_CLOSE(pio[1], PROC_FD_CHILD);
		ops[2] = PROC_FD_CLOSE(pio[2], PROC_FD_CHILD);
		ops[3] = PROC_FD_CLOSE(pio[3], PROC_FD_PARENT);
		ops[4] = 0;
		sfsprintf(devfd, sizeof(devfd), "/dev/fd/%d", pio[0]);
		flags = !access(devfd, F_OK);
	}
	sh[0] = (char*)path;
	sh[1] = getenv(CO_ENV_SHELL);
	for (i = 0; i < elementsof(sh); i++)
		if ((s = sh[i]) && *s && (s = strdup(s)))
		{
			if ((n = tokscan(s, NiL, " %v ", av, elementsof(av) - 1)) > 0)
			{
				if (t = strrchr(s = av[0], '/'))
					av[0] = t + 1;
				if (flags || (co->flags & CO_DEVFD) && strmatch(s, "*ksh*"))
					av[n++] = devfd;
				av[n] = 0;
				sfsprintf(evbuf, sizeof(evbuf), "%s=%d", CO_ENV_MSGFD, co->gsmfd);
				ev[0] = evbuf;
				ev[1] = 0;
				if ((co->mode & CO_MODE_SEPARATE) || (proc = procopen(s, av, ev, ops, (co->flags & (CO_SHELL|CO_ORPHAN)) ? (PROC_ORPHAN|PROC_DAEMON|PROC_IGNORE) : (PROC_DAEMON|PROC_IGNORE))))
				{
					if (!state.sh)
						state.sh = strdup(s);
					free(s);
					if (proc)
					{
						co->pid = proc->pid;
						procfree(proc);
					}
					break;
				}
			}
			free(s);
		}
	if (i >= elementsof(sh))
	{
		errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot execute");
		goto bad;
	}
	if (!(co->mode & CO_MODE_SEPARATE))
	{
		/*
		 * send the shell identification sequence
		 */

		if (!(sp = sfstropen()))
		{
			errormsg(state.lib, ERROR_LIBRARY|2, "out of buffer space");
			goto bad;
		}
		sfprintf(sp, "#%05d\n%s='", 0, CO_ENV_ATTRIBUTES);
		if (t = getenv(CO_ENV_ATTRIBUTES))
		{
			coquote(sp, t, 0);
			if (attributes)
				sfprintf(sp, ",");
		}
		if (attributes)
			coquote(sp, attributes, 0);
		sfprintf(sp, "'\n");
		sfprintf(sp, coident, CO_ENV_MSGFD, pio[3], CO_ENV_MSGFD, CO_ENV_MSGFD, CO_ENV_MSGFD);
		i = sfstrtell(sp);
		sfstrseek(sp, 0, SEEK_SET);
		sfprintf(sp, "#%05d\n", i - 7);
		i = write(co->cmdfd, sfstrbase(sp), i) != i;
		sfstrclose(sp);
		if (i)
		{
			errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "cannot write initialization message");
			goto nope;
		}
		state.current = co;
		handler = signal(SIGALRM, hung);
		i = alarm(30);
		if (!(s = sfgetr(co->msgfp, '\n', 1)))
		{
			if (errno == EINTR)
				errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "identification message read timeout");
			goto nope;
		}
		alarm(i);
		signal(SIGALRM, handler);
		if (co->flags & CO_DEBUG)
			errormsg(state.lib, 2, "coshell %d shell path %s identification \"%s\"", co->index, state.sh, s);
		switch (*s)
		{
		case 'o':
			co->flags |= CO_OSH;
			/*FALLTHROUGH*/
		case 'b':
			s = cobinit;
			break;
		case 'k':
			co->flags |= CO_KSH;
			s = cokinit;
			break;
		case 'i':	/* NOTE: 'i' is obsolete */
		case 's':
			co->flags |= CO_SERVER;
			co->pid = 0;
			for (;;)
			{
				if (t = strchr(s, ','))
					*t = 0;
				if (streq(s, CO_OPT_ACK))
					co->mode |= CO_MODE_ACK;
				else if (streq(s, CO_OPT_INDIRECT))
					co->mode |= CO_MODE_INDIRECT;
				if (!(s = t))
					break;
				s++;
			}
			if (!(co->mode & CO_MODE_INDIRECT))
				wait(NiL);
			break;
		default:
			goto nope;
		}
		if (s)
		{
			if (!(cj = coexec(co, s, 0, NiL, NiL, NiL)) || cowait(co, cj, -1) != cj)
			{
				errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "initialization message exec error");
				goto nope;
			}
			co->total = 0;
			co->user = 0;
			co->sys = 0;
		}
	}
	co->flags &= ~CO_INIT;
	fcntl(pio[1], F_SETFD, FD_CLOEXEC);
	fcntl(pio[2], F_SETFD, FD_CLOEXEC);
	co->next = state.coshells;
	state.coshells = co;
	if (!(co->flags & CO_SHELL))
	{
#ifdef SIGCONT
#ifdef SIGTSTP
		signal(SIGTSTP, stop);
#endif
#ifdef SIGTTIN
		signal(SIGTTIN, stop);
#endif
#ifdef SIGTTOU
		signal(SIGTTOU, stop);
#endif
#endif
		if (!state.init)
		{
			state.init = 1;
			atexit(clean);
		}
	}
	return co;
 bad:
	n = errno;
	if (co->msgfp)
	{
		sfclose(co->msgfp);
		pio[2] = -1;
	}
	for (i = 0; i < elementsof(pio); i++)
		if (pio[i] >= 0)
			close(pio[i]);
	coclose(co);
	errno = n;
	return 0;
 nope:
	i = errno;
	if (!(s = sh[1]) || (s = (t = strrchr(s, '/')) ? (t + 1) : s) && !strmatch(s, "?(k)sh") && !streq(s, CO_ID))
		error(2, "export %s={ksh,sh,%s}", CO_ENV_SHELL, CO_ID);
	coclose(co);
	errno = i;
	return 0;
}

/*
 * set coshell attributes
 */

int
coattr(Coshell_t* co, const char* attributes)
{
	return 0;
}
