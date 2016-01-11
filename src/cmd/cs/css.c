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
 * css - multiplex multiple clients on one filter server
 */

static const char usage[] =
"[-?\n@(#)$Id: css (AT&T Research) 1998-05-01 $\n]"
USAGE_LICENSE
"[+NAME?css - multiplex multiple clients on one connect stream server]"
"[+DESCRIPTION?\bcss\b multiplexes multiple clients on one filter server."
"	A filter server is a process that reads lines from the standard input"
"	and writes result lines to the standard output. \aconnect-stream\a"
"	is the connect stream path by which the filter service will be known.]"

"[t:timeout?The service will exit after a \atime\a period of client"
"	inactivity.]:[time]"

"\n"
"\nconnect-stream command [ arg ... ]\n"
"\n"

"[+PROTOCOL?A filter service must follow a simple line oriented protocol. All"
"	client lines are split into arguments and a number is inserted in the"
"	second argument position. This number, followed by a space, must be"
"	placed at the beginning of each line written by the filter server for"
"	the given client request.]"

"[+SEE ALSO?\bcoshell\b(1), \bcs\b(1), \bss\b(1), \bcs\b(3)]"
;

#include <css.h>
#include <ctype.h>
#include <error.h>
#include <proc.h>

typedef struct
{
	Csid_t		id;
	int		service;
} Connection_t;

typedef struct
{
	Cssdisc_t	disc;
	Proc_t*		proc;
	Sfio_t*		tmp;
} State_t;

static char	buf[8 * 1024];

static int
acceptf(Css_t* css, Cssfd_t* fp, Csid_t* ip, char** av, Cssdisc_t* disc)
{
	register Connection_t*	con;

	NoP(av);
	if (!(con = newof(0, Connection_t, 1, 0)))
		return -1;
	fp->data = con;
	con->id = *ip;
	return fp->fd;
}

static int
actionf(register Css_t* css, register Cssfd_t* fp, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	con;
	register char*		s;
	register char*		t;
	int			n;
	int			i;

	switch (fp->status)
	{
	case CS_POLL_CLOSE:
		if (con = (Connection_t*)fp->data)
		{
			if (con->service)
				error(ERROR_SYSTEM|3, "service termination exit");
			free(con);
		}
		return 0;
	case CS_POLL_READ:
		con = (Connection_t*)fp->data;
		if ((n = csread(css->state, fp->fd, buf, sizeof(buf) - 1, CS_LINE)) <= 0)
		{
			if (con->service)
				error(ERROR_SYSTEM|3, "service termination exit");
			return -1;
		}
		buf[n] = 0;
		for (s = buf; isspace(*s); s++);
		if (con->service)
		{
			for (i = 0; isdigit(*s); i = i * 10 + *s++ - '0');
			for (; isspace(*s); s++);
			if (*s && cssfd(css, i, 0))
			{
				n -= s - buf;
				if (cswrite(css->state, i, s, n) != n)
					cssfd(css, i, CS_POLL_CLOSE);
			}
		}
		else if (*s == '!')
		{
			if ((n -= ++s - buf) > 0 && cswrite(css->state, state->proc->wfd, s, n) != n)
				return -1;
		}
		else
		{
			for (t = s; *t && !isspace(*t); t++);
			for (; isspace(*t); t++);
			if (*s == 'Q' && !*t)
			{
				if (con->id.uid == geteuid())
					error(3, "service quit exit");
			}
			else
			{
				n = sfprintf(state->tmp, "%-.*s%d %s", t - s, s, fp->fd, t);
				if (!(s = sfstruse(state->tmp)))
					return -1;
				if (cswrite(css->state, state->proc->wfd, s, n) != n)
					return -1;
			}
		}
		return 1;
	}
	return 0;
}

static int
exceptf(Css_t* css, unsigned long op, unsigned long arg, Cssdisc_t* disc)
{
	switch (op)
	{
	case CSS_INTERRUPT:
		error(ERROR_SYSTEM|3, "%s: interrupt exit", fmtsignal(arg));
		return 0;
	case CSS_DORMANT:
		error(2, "service dormant exit");
		exit(0);
	}
	error(ERROR_SYSTEM|3, "poll error op=0x%08x arg=0x%08x", op, arg);
	return -1;
}


int
main(int argc, char** argv)
{
	Css_t*		css;
	Cssfd_t*	fp;
	Connection_t*	con;
	char*		e;
	State_t		state;

	NoP(argc);
	error_info.id = "css";
	memset(&state, 0, sizeof(state));
	state.disc.version = CSS_VERSION;
	state.disc.flags = CSS_DAEMON|CSS_ERROR|CSS_INTERRUPT;
	state.disc.acceptf = acceptf;
	state.disc.actionf = actionf;
	state.disc.errorf = errorf;
	state.disc.exceptf = exceptf;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 't':
			state.disc.timeout = strelapsed(opt_info.arg, &e, 1);
			if (*e)
				error(3, "%s: invalid timeout value", opt_info.arg);
			state.disc.flags |= CSS_DORMANT;
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
	if (!argv[0] || !argv[1])
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(state.tmp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [tmp stream]");
	if (!(state.proc = procopen(argv[1], argv + 1, NiL, NiL, PROC_READ|PROC_WRITE)))
		error(ERROR_SYSTEM|3, "%s: cannot execute", argv[1]);
	if (!(css = cssopen(argv[0], &state.disc)))
		return 1;
	if (!(fp = cssfd(css, state.proc->rfd, CS_POLL_READ)))
		error(ERROR_SYSTEM|3, "%s: cannot poll output", argv[1]);
	if (!(con = newof(0, Connection_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	fp->data = con;
	con->service = 1;
	csspoll(CS_NEVER, 0);
	return 1;
}
