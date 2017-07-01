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
 */

static const char usage[] =
"[-?\n@(#)$Id: mbb (AT&T Research) 2000-05-09 $\n]"
USAGE_LICENSE
"[+NAME?mbb - message bulletin board server]"
"[+DESCRIPTION?\bmbb\b is a message bulletin board server. Each \bmbb\b"
"	instance is a session. Each session supports up to 64 message"
"	channels, labeled from 0 to 63. A client connects to \bmbb\b and"
"	provides a mask of channels that it is interested in. All subsequent"
"	messages in the mask are sent to the client. Channel 0 is reserved"
"	for service control messages.]"
"[+?A message is a newline terminated line with length 8K bytes max"
"	that does not contain the ASCII NUL character. A message must"
"	be prefixed by its channel number. The server changes this to"
"	\achannel\a.\aid\a where \aid\a is the server assigned client"
"	identification number. Messages with invalid or missing channel"
"	numbers are silently rejected.]"

"[b:backlog?Every \an\ath client message with length > 1 will be treated as"
"	if only half the data were sent. This should kick in the message"
"	backlog logic.]#[n]"
"[d:debug?Set the debug trace level to \alevel\a. Higher levels produce more"
"	output.]#[level]"
"[t:timeout?The service will exit after a \atime\a period of client"
"	inactivity. The default is to run until the system crashes.]:[time]"

"\n"
"\nconnect-stream\n"
"\n"

"[+PROTOCOL?Channel 0 is for service control messages. The server and clients"
"	may send messages on channel 0, with the exception that client"
"	messages on channel 0 are not sent to the other clients. The control"
"	messages are:]{"
"		[+0 listen \amask\a?[client]] The client is interested in"
"			channel numbers in the bitmask \amask\a. The default"
"			\amask\a is \b0xfffffffffffffffe\b; i.e., all but"
"			channel 0.]"
"		[+0 join \aid\a?[server]] Client with server assigned id"
"			number \aid\a has joined the session.]"
"		[+0 drop \aid\a?[server]] Client with server assigned id"
"			number \aid\a has dropped from the session.]"
"	}"

"[+SEE ALSO?\bcoshell\b(1), \bcs\b(1), \bss\b(1), \bcs\b(3)]"
;

#include <css.h>
#include <ctype.h>
#include <debug.h>
#include <error.h>
#include <ls.h>
#include <stdarg.h>

typedef struct Log_s
{
	char			name[2];	/* file name		*/
	Sfio_t*			sp;		/* r/w stream		*/
	Sfoff_t			offset;		/* next write offset	*/
	size_t			blocked;	/* blocked connections	*/
} Log_t;

typedef struct Connection_s
{
	struct Connection_s*	next;
	Cssfd_t*		fp;
	Sfulong_t		mask;
	Sfoff_t			blocked[2];
} Connection_t;

typedef struct State_s
{
	Cssdisc_t		disc;
	Connection_t*		all;
	Sfio_t*			tmp;
	Log_t			logs[2];
	int			backlog;
	int			count;
	int			log;
	int			logged;
} State_t;

#define ALL			(-1)
#define CHUNK			(4 * 1024)
#define HOG			(4 * 1024 * 1024)

#define CHAN_DEFAULT		(((Sfulong_t)~0)^1)
#define CHAN_VALID(c)		((c)>=0&&(c)<=63)
#define CHAN_MASK(c)		(((Sfulong_t)1)<<(c))

static char			buf[8 * 1024];

static ssize_t
data(register State_t* state, register Connection_t* to, char* s, size_t n, int force)
{
	if (!force && n > 1 && state->backlog && ++state->count >= state->backlog)
	{
		message((-1, "[%d] %d backlog", __LINE__, to->fp->fd));
		state->count = 0;
		n /= 2;
	}
	return write(to->fp->fd, s, n);
}

static int
note(Css_t* css, register Connection_t* to, int log, char* s, size_t n, int force, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	ssize_t			z;

	if ((force || to->blocked[log] < 0) && (z = data(state, to, s, n, force)) != n)
	{
		if (!force && !state->logged)
		{
			state->logged = 1;
			if (!state->logs[log].sp)
			{
				state->logs[log].name[0] = '0' + log;
				remove(state->logs[log].name);
				if (!(state->logs[log].sp = sfopen(NiL, state->logs[log].name, "r+")))
					error(ERROR_SYSTEM|3, "%s: cannot create message log", state->logs[log].name);
				message((-1, "[%d] %s: create log", __LINE__, state->logs[log].name));
			}
			message((-1, "[%d] %s: %d log", __LINE__, state->logs[log].name, to->fp->fd));
			if (sfwrite(state->logs[log].sp, s, n) != n)
				error(ERROR_SYSTEM|3, "%s: log file write error", state->logs[log].name);
			if ((state->logs[log].offset += n) >= HOG && !state->logs[!log].sp)
				state->log = !log;
		}
		if (to->blocked[log] < 0)
		{
			message((-1, "[%d] %s: block", __LINE__, state->logs[log].name));
			state->logs[log].blocked++;
		}
		to->blocked[log] = state->logs[log].offset - n + z;
		message((-1, "[%d] %s: %d offset %I*d", __LINE__, state->logs[log].name, to->fp->fd, sizeof(to->blocked[log]), to->blocked[log]));
		cssfd(css, to->fp->fd, CS_POLL_READ|CS_POLL_WRITE);
		return 0;
	}
	if (to->blocked[log] >= 0)
	{
		message((-1, "[%d] %s: %d unblock", __LINE__, state->logs[log].name, to->fp->fd));
		to->blocked[log] = -1;
		if (!--state->logs[log].blocked)
		{
			sfclose(state->logs[log].sp);
			state->logs[log].sp = 0;
			state->logs[log].offset = 0;
			remove(state->logs[log].name);
			message((-1, "[%d] %s: clear", __LINE__, state->logs[log].name));
		}
	}
	return 1;
}

static int
dump(Css_t* css, register Connection_t* con, int log, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	char*			s;
	size_t			n;
	int			r;

	n = state->logs[log].offset - con->blocked[log];
	if (n > CHUNK)
		n = CHUNK;
	if (sfseek(state->logs[log].sp, con->blocked[log], SEEK_SET) != con->blocked[log])
		error(ERROR_SYSTEM|3, "%s: cannot seek to %I*d", state->logs[log].name, sizeof(con->blocked[log]), con->blocked[log]);
	message((-1, "[%d] %s reserve n %I*d offset %I*d", __LINE__, state->logs[log].name, sizeof(n), n, sizeof(con->blocked[log]), con->blocked[log]));
	if (!(s = sfreserve(state->logs[log].sp, n, 0)))
		error(ERROR_SYSTEM|3, "%s: cannot reserve %d at %I*d", state->logs[log].name, sizeof(n), n, sizeof(con->blocked[log]), con->blocked[log]);
	r = note(css, con, log, s, n, 1, disc);
	if (state->logs[log].sp && sfseek(state->logs[log].sp, state->logs[log].offset, SEEK_SET) != state->logs[log].offset)
		error(ERROR_SYSTEM|3, "%s: cannot seek to %I*d", state->logs[log].name, sizeof(state->logs[log].offset), state->logs[log].offset);
	return r;
}

static int
post(Css_t* css, Cssdisc_t* disc, Connection_t* from, register Connection_t* to, int channel, const char* format, ...)
{
	State_t*		state = (State_t*)disc;
	char*			s;
	ssize_t			n;
	Sfulong_t		m;
	va_list			ap;

	sfprintf(state->tmp, "%d", channel);
	if (from)
		sfprintf(state->tmp, ".%d", from->fp->fd);
	sfputc(state->tmp, ' ');
	va_start(ap, format);
	sfvprintf(state->tmp, format, ap);
	va_end(ap);
	sfputc(state->tmp, '\n');
	n = sfstrtell(state->tmp);
	if (!(s = sfstruse(state->tmp)))
		error(ERROR_SYSTEM|3, "out of space");
	m = CHAN_MASK(channel);
	state->logged = 0;
	if (!to)
	{
		for (to = state->all; to; to = to->next)
			if ((to->mask & m) && to != from)
				note(css, to, state->log, s, n, 0, disc);
	}
	else if (to->mask & m)
		note(css, to, state->log, s, n, 0, disc);
	return 0;
}

static void
drop(Css_t* css, Connection_t* con, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	cp;
	register Connection_t*	pp;

	pp = 0;
	for (cp = state->all; cp; pp = cp, cp = cp->next)
		if (cp == con)
		{
			if (pp)
				pp->next = cp->next;
			else
				state->all = cp->next;
			cp->fp->data = 0;
			free(cp);
			post(css, disc, cp, NiL, 0, "drop");
			break;
		}
}

static int
acceptf(Css_t* css, Cssfd_t* fp, Csid_t* ip, char** av, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	con;
	int			i;

	NoP(ip);
	NoP(av);
	if (!(con = newof(0, Connection_t, 1, 0)))
		return -1;
	fp->data = con;
	con->fp = fp;
	con->mask = CHAN_DEFAULT;
	for (i = 0; i < elementsof(state->logs); i++)
		con->blocked[i] = -1;
	con->next = state->all;
	state->all = con;
	post(css, disc, con, NiL, 0, "join");
	return fp->fd;
}

static int
actionf(register Css_t* css, register Cssfd_t* fp, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	con;
	register char*		s;
	char*			e;
	int			n;
	int			c;
	Sfulong_t		m;
	Sfulong_t		o;

	switch (fp->status)
	{
	case CS_POLL_CLOSE:
		if (!(con = (Connection_t*)fp->data))
			return -1;
		drop(css, con, disc);
		return 0;
	case CS_POLL_READ:
		if (!(con = (Connection_t*)fp->data))
			return -1;
		if ((n = csread(css->state, fp->fd, buf, sizeof(buf) - 1, CS_LINE)) <= 0)
		{
			drop(css, con, disc);
			return -1;
		}
		if (n > 0 && buf[n - 1] == '\n')
			n--;
		buf[n] = 0;
		for (s = buf; isspace(*s); s++);
		c = (int)strtol(s, &e, 0);
		if (CHAN_VALID(c) && e > s)
		{
			s = e;
			if (*s == '.')
				while (isdigit(*++s));
			for (; isspace(*s); s++);
			if (c == 0)
			{
				for (e = s; *s && !isspace(*s); s++);
				if (*s)
					for (*s++ = 0; isspace(*s); s++);
				switch (*e)
				{
				case 'm':
					if (!strcmp(e, "mask"))
					{
						o = con->mask;
						if (*s)
						{
							m = strtoull(s, &e, 0);
							if (e > s)
								con->mask = m;
						}
						post(css, disc, con, con, 0, "mask 0x%I*x 0x%I*x", sizeof(con->mask), con->mask, sizeof(o), o);
					}
					break;
				case 'q':
					/* might want privilege check here */
					if (!strcmp(e, "quit"))
						exit(0);
					break;
				}
			}
			else
				post(css, disc, con, NiL, c, "%s", s);
		}
		return 1;
	case CS_POLL_WRITE:
		if (!(con = (Connection_t*)fp->data))
			return -1;
		if ((con->blocked[!state->log] < 0 || dump(css, con, !state->log, disc)) && con->blocked[state->log] >= 0)
			dump(css, con, state->log, disc);
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
	char*		e;
	State_t		state;

	NoP(argc);
	error_info.id = "mbb";
	memset(&state, 0, sizeof(state));
	state.disc.version = CSS_VERSION;
	state.disc.flags = CSS_DAEMON|CSS_ERROR|CSS_INTERRUPT|CSS_LOG;
	state.disc.acceptf = acceptf;
	state.disc.actionf = actionf;
	state.disc.errorf = errorf;
	state.disc.exceptf = exceptf;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'b':
			state.backlog = opt_info.num;
			continue;
		case 'd':
			error_info.trace = -opt_info.num;
			continue;
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
	if (!argv[0] || argv[1])
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(state.tmp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [tmp stream]");
	if (!cssopen(argv[0], &state.disc))
		return 1;
	umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	csspoll(CS_NEVER, 0);
	return 1;
}
