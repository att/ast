/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2013 AT&T Intellectual Property          *
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
 * shared event daemon
 */

#define EVENT_MAJOR		1
#define EVENT_MINOR		0

static const char usage[] =
"[-?\n@(#)$Id: event (AT&T Research) 2013-10-25 $\n]"
USAGE_LICENSE
"[+NAME?event - shared event client and server]"
"[+DESCRIPTION?\bevent\b is a shared event client and server. Events are "
    "stored in a persistent database named by the \aname\a operand. Each "
    "event has a name, an expiration, and a binary status \braised\b or "
    "\bnot-raised\b. A non-existent event is \bnot-raised\b. Events may be "
    "raised, deleted, cleared, tested and waited for. If no \brequest\b "
    "operands are specified then requests are prompted for, with an "
    "\bEVENT>\b prompt, and read from the standard input. Multiple command "
    "line requests must be separated by \b:\b. In the following events "
    "operands are matched by \bksh\b(1) patterns. The client requests are:]"
    "{"
        "[+all \aconnection\a?Raise all pending events for the "
            "\aconnection\a. Mainly for debugging.]"
        "[+clear \aevent\a ...?Mark \aevent\a not-raised but do not "
            "delete from the database. This allows the events to be matched "
            "by patterns.]"
        "[+delete \aevent\a ...?Delete \aevent\a.]"
        "[+exit?Close the client connection.]"
        "[+hold [ \aevent\a ...]]?If \aevent\a operands are specified "
            "then clients are not notified about the those events until they "
            "are explicitly released by \brelease\b \aevent\a ... If no "
            "events are specified then all current and future events will be "
            "unconditionally held until a \brelease\b with no event "
            "operands.]"
        "[+info?List the server status pending events by client "
            "connection. The list is terminated by a \bdone\b message.]"
        "[+list [ \apattern\a ]]?Start an event dictionary scan and list "
            "the first event. If \apattern\a is specified then only events "
            "matching \apattern\a are listed.]"
        "[+next?List the next event in the \blist\b event scan. The list "
            "is terminated by a \bdone\b message.]"
        "[+quit?Equivalent to exit.]"
        "[+raise \aevent\a ...?Raise \aevent\a ...]"
        "[+release [ \aevent\a ...]]?If \aevent\a operands are specified "
            "then they are released from a previous \bhold\b \aevent\a ... "
            "If no \aevent\a operands are specified then any previous "
            "unconditional \bhold\b is turned off.]"
        "[+set \aoption\a ...?]"
        "[+stop?Terminate the server. Persistent data is preserved.]"
        "[+test \aevent\a?Determine the \aevent\a status.]"
        "[+wait \aevent\a?Wait for \aevent\a status to be \braised\b.]"
    "}"
"[+?The \b--cs\b, \b--expire\b, \b--initialize\b, and \b--log\b options "
    "apply to the initial service command, and the \b--expire\b, \b--log\b, "
    "\b--newer\b, \b--older\b, and \b--quiet\b options apply to client "
    "requests.]"
"[c:cs|connect-stream?Use \aconnect-stream\a instead of the default.]"
    ":[connect-stream:=/dev/tcp/local/event]"
"[e:expire?Set the current event expiration to the \bdate\b(1) or "
    "\bcron\b(1) expression \adate-expression\a.]:[date-expression]"
"[i:initialize?Initialize the service if it is not already running.]"
"[l!:log?Log server activity to \astate-name\a.log, where \astate-name\a "
    "is the state path name sans suffix.]"
"[n:newer?Match events newer than \adate\a. If \b--older\b is also "
    "specified then only event times > newer and < older match.]:[date]"
"[o:older?Match events older than \adate\a. If \b--newer\b is also "
    "specified then only event times > newer and < older match.]:[date]"
"[q:quiet?Suppress request confirmation messages.]"
"\n"
"\nname [ request [ options ] [ arg ... ] ] [ : request ... ]\n"
"\n"
"[+CAVEATS?Expirations, logging and the \bset\b request are not "
    "implemented yet.]"
"[+SEE ALSO?\bcoshell\b(1), \bcs\b(1), \bnmake\b(1), \bdbm\b(3), "
    "\bndbm\b(3), \bgdbm\b(3)]"
;

static const char	command[] = "event";

static const char	ident_key[] = "'//\t<(IDENT)>\t\\\\'";
static const char	ident_name[] = "EVEN";

#define IDENT_SWAP	0x01020304
#define IDENT_VERSION	((EVENT_MAJOR<<16)|(EVENT_MINOR))

#define EVENT(s)	(*((char*)(s))!=ident_key[0])
#define log		_log	/* gnu builtin? you've got to be kidding */

#include <ast.h>
#include <cdt.h>
#include <css.h>
#include <ctype.h>
#include <debug.h>
#include <error.h>
#include <namval.h>
#include <ls.h>
#include <regex.h>
#include <stdarg.h>
#include <swap.h>
#include <tok.h>
#include <tm.h>
#include <ast_ndbm.h>

#if !_use_ndbm

int
main(int argc, char** argv)
{
	NoP(argc);
	NoP(argv);
	error(3, "<ndbm.h> library required");
	return 1;
}

#else

#define KEY_MAX		64		/* max key length + 1			*/

struct Key_s; typedef struct Key_s Key_t;

#define DATA_clear	0x00000001	/* explicit clear			*/
#define DATA_hold	0x00000002	/* explicit hold			*/

typedef uint32_t Number_t;

typedef struct Data_s			/* event data				*/
{
	Number_t		expire;	/* expiration seconds since epoch	*/
	Number_t		time;	/* last raise time			*/
	Number_t		raise;	/* total # raise requests		*/
	Number_t		flags;	/* DATA_* flags				*/
} Data_t;

typedef struct Event_s			/* event bucket				*/
{
	Dtlink_t	link;		/* dictionary link			*/
	unsigned int	waiting;	/* # clients waiting			*/
	Data_t		data;		/* event persistent data		*/
	char		name[256];	/* event name				*/
} Event_t;

typedef struct Waiting_s		/* pending event bucket			*/
{
	Dtlink_t	link;		/* dictionary link			*/
	int		id;		/* wait id				*/
	Event_t*	event;		/* event bucket pointer			*/
} Waiting_t;

typedef struct Connection_s		/* client connection state		*/
{
	Dtlink_t	link;		/* list link				*/
	Csid_t		id;		/* connection id			*/
	Dt_t*		waiting;	/* pending events			*/
	datum		list;		/* list finger				*/
	int		fd;		/* connection fd			*/
	int		all;		/* list all vs. list match		*/
	int		code;		/* request exit code			*/
	int		quiet;		/* suppress response log messages	*/
	unsigned long	newer;		/* list --newer time			*/
	unsigned long	older;		/* list --older time			*/
	regex_t		re;		/* list request pattern			*/
} Connection_t;

typedef struct Request_s		/* static request info			*/
{
	const char*	name;		/* request name				*/
	int		index;		/* REQ_* index				*/
	int		min;		/* min #args				*/
	int		max;		/* max #args				*/
} Request_t;

typedef struct State_s			/* program state			*/
{
	Cssdisc_t	disc;		/* css discipline			*/
	Dtdisc_t	condisc;	/* connection dictionary discipline	*/
	Dtdisc_t	eventdisc;	/* event dictionary discipline		*/
	Dtdisc_t	waitdisc;	/* pending events dictionary discipline	*/
	unsigned int	active;		/* number of active clients		*/
	int		hold;		/* hold announcements			*/
	int		log;		/* log activity				*/
	int		major;		/* db major version			*/
	int		minor;		/* db major version			*/
	int		swap;		/* datum <=> native int32_ swap		*/
	unsigned long	expire;		/* expiration in seconds		*/
	DBM*		dbm;		/* dbm handle				*/
	Dt_t*		connections;	/* active connection list		*/
	Dt_t*		events;		/* outstanding events dictionary	*/
	char*		service;	/* service connect stream path		*/
	char*		path;		/* event db path			*/
	Sfio_t*		logf;		/* log file stream			*/
	Sfio_t*		usrf;		/* usr buffer stream			*/
	Sfio_t*		tmp;		/* tmp buffer stream			*/
	char*		cmd[1024];	/* request command argv			*/
	char		req[8 * 1024];	/* request buffer			*/
} State_t;

#define REQ_all			1
#define REQ_clear		2
#define REQ_delete		3
#define REQ_exit		4
#define REQ_hold		5
#define REQ_info		6
#define REQ_list		7
#define REQ_next		8
#define REQ_raise		9
#define REQ_release		10
#define REQ_set			11
#define REQ_stop		12
#define REQ_test		13
#define REQ_wait		14

static const Request_t	requests[] =
{
	"a*ll",		REQ_all,	1,	1,
	"c*lear",	REQ_clear,	1,	-1,
	"d*elete",	REQ_delete,	1,	-1,
	"e*xit",	REQ_exit,	0,	0,
	"hold",		REQ_hold,	0,	-1,
	"i*nfo",	REQ_info,	0,	0,
	"l*ist",	REQ_list,	0,	1,
	"n*ext",	REQ_next,	0,	0,
	"q*uit",	REQ_exit,	0,	0,
	"r*aise",	REQ_raise,	1,	-1,
	"release",	REQ_release,	0,	-1,
	"s*et",		REQ_set,	0,	0,
	"stop",		REQ_stop,	0,	0,
	"t*est",	REQ_test,	1,	1,
	"w*ait",	REQ_wait,	1,	1,
};

/*
 * generate a user response and log message
 */

static void
log(State_t* state, Connection_t* con, int type, const char* format, ...)
{
	va_list		ap;
	char*		s;
	int		n;

	va_start(ap, format);
	if (format)
		sfvprintf(state->tmp, format, ap);
	va_end(ap);
	if (type)
	{
		if (!(s = sfstruse(state->tmp)))
			error(ERROR_SYSTEM|3, "out of space");
		if (type != 'I' && state->log && state->logf)
			sfprintf(state->logf, "%s (%03d) %c %s\n", fmttime("%K", time(NiL)), con ? con->fd : 0, toupper(type), s);
		if (con && type != 'R' && type != 'S')
		{
			if (type != 'L' || !con->quiet)
				debug_printf(con->fd, "%c %s\n", toupper(type), s);
			if (type == 'W')
				con->code |= 1;
			else if (type == 'E')
				con->code |= 2;
		}
	}
}

/*
 * accept a new connection
 */

static int
acceptf(Css_t* css, Cssfd_t* fp, Csid_t* ip, char** av, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	con;

	if (!(con = newof(0, Connection_t, 1, 0)))
		return -1;
	fp->data = con;
	con->id = *ip;
	con->waiting = 0;
	con->fd = fp->fd;
	state->active++;
	dtinsert(state->connections, con);
	log(state, con, 'S', "accept connection -- %d active", state->active);
	return fp->fd;
}

/*
 * notify connections waiting on ep
 */

static int
notify(State_t* state, Event_t* ep)
{
	Connection_t*	cp;
	Waiting_t*	wp;
	char*		s;
	size_t		n;

	for (cp = (Connection_t*)dtfirst(state->connections); cp; cp = (Connection_t*)dtnext(state->connections, cp))
		if (cp->waiting && (wp = (Waiting_t*)dtmatch(cp->waiting, &ep)))
		{
			if (wp->id >= 0)
			{
				log(state, cp, 'x', "%d 0", wp->id);
				n = sfstrtell(state->usrf);
				if (!(s = sfstruse(state->usrf)))
					error(ERROR_SYSTEM|3, "out of space");
				write(cp->fd, s, n);
			}
			else if (!cp->quiet)
				log(state, cp, 'i', "%s raised", ep->name);
			n = ep->waiting == 1;
			dtdelete(cp->waiting, wp);
			if (n)
				break;
		}
	return 0;
}

/*
 * post pending event name for connection
 */

static int
post(State_t* state, Connection_t* con, const char* name, int id)
{
	Event_t*	ep;
	Waiting_t*	wp;

	if (!con->waiting && !(con->waiting = dtopen(&state->waitdisc, Dtset)))
	{
		error(ERROR_SYSTEM|3, "out of space [waiting]");
		return -1;
	}
	if (ep = dtmatch(state->events, name))
	{
		if (dtmatch(con->waiting, &ep))
			return 0;
	}
	else if (!(ep = newof(0, Event_t, 1, 0)))
	{
		error(ERROR_SYSTEM|3, "out of space [event]");
		return -1;
	}
	else
	{
		strcpy(ep->name, name);
		dtinsert(state->events, ep);
	}
	if (!(wp = newof(0, Waiting_t, 1, 0)))
	{
		error(ERROR_SYSTEM|3, "out of space [waiting]");
		return -1;
	}
	ep->waiting++;
	wp->id = id;
	wp->event = ep;
	dtinsert(con->waiting, wp);
	return 0;
}

/*
 * list server info/state
 */

static int
info(State_t* state, Connection_t* con, Css_t* css)
{
	Connection_t*	cp;
	Waiting_t*	wp;
	int		n;

	log(state, con, 'I', "info server='%s' version=%d.%d host=%s pid=%d uid=%d gid=%d", fmtident(usage), EVENT_MAJOR, EVENT_MINOR, csname(css->state, 0), getpid(), geteuid(), getegid());
	log(state, con, 'I', "info active=%d", state->active);
	for (cp = (Connection_t*)dtfirst(state->connections); cp; cp = (Connection_t*)dtnext(state->connections, cp))
		if (cp->waiting && (n = dtsize(cp->waiting)) > 0)
		{
			log(state, con, 0, "waiting connection=%d count=%d", cp->fd, n);
			for (wp = (Waiting_t*)dtfirst(cp->waiting); wp; wp = (Waiting_t*)dtnext(cp->waiting, wp))
				log(state, con, 0, " %s", wp->event->name);
			log(state, con, 'I', 0);
		}
	log(state, con, 'I', "done");
	return 0;
}

static int	request(State_t*, Connection_t*, int, int, char**, unsigned long, unsigned long);

/*
 * apply request r to one key
 */

static int
apply(State_t* state, Connection_t* con, int id, int index, datum key, datum val, Data_t* dat)
{
	Event_t*	e;
	int		n;

	switch (index)
	{
	case REQ_clear:
		dat->flags |= DATA_clear;
		dat->time = time(NiL);
		val.dptr = (void*)dat;
		val.dsize = sizeof(*dat);
		if (!(n = dbm_store(state->dbm, key, val, DBM_INSERT)) || n > 0 && !dbm_store(state->dbm, key, val, DBM_REPLACE))
			log(state, con, 'L', "%s cleared", key.dptr);
		else if (!dbm_error(state->dbm))
			log(state, con, 'W', "%s unchanged", key.dptr);
		else
		{
			dbm_clearerr(state->dbm);
			log(state, con, 'E', "%s io error", key.dptr);
		}
		break;
	case REQ_delete:
		if (!dbm_delete(state->dbm, key))
		{
			log(state, con, 'L', "%s deleted", key.dptr);
			return 1;
		}
		else if (!dbm_error(state->dbm))
			log(state, con, 'W', "%s not in db", key.dptr);
		else
		{
			dbm_clearerr(state->dbm);
			log(state, con, 'E', "%s io error", key.dptr);
		}
		break;
	case REQ_hold:
		dat->flags |= DATA_hold;
		dat->time = time(NiL);
		val.dptr = (void*)dat;
		val.dsize = sizeof(*dat);
		if (!(n = dbm_store(state->dbm, key, val, DBM_INSERT)) || n > 0 && !dbm_store(state->dbm, key, val, DBM_REPLACE))
			log(state, con, 'L', "%s held", key.dptr);
		else if (!dbm_error(state->dbm))
			log(state, con, 'W', "%s unchanged", key.dptr);
		else
		{
			dbm_clearerr(state->dbm);
			log(state, con, 'E', "%s io error", key.dptr);
		}
		break;
	case REQ_raise:
		dat->flags &= ~DATA_clear;
		dat->time = time(NiL);
		dat->raise++;
		val.dptr = (void*)dat;
		val.dsize = sizeof(*dat);
		if (!(n = dbm_store(state->dbm, key, val, DBM_INSERT)) || n > 0 && !dbm_store(state->dbm, key, val, DBM_REPLACE))
		{
			if (!state->hold && (e = (Event_t*)dtmatch(state->events, key.dptr)))
				notify(state, e);
			log(state, con, 'I', "%s raised", key.dptr);
		}
		else if (!dbm_error(state->dbm))
			log(state, con, 'W', "%s unchanged", key.dptr);
		else
		{
			dbm_clearerr(state->dbm);
			log(state, con, 'E', "%s io error", key.dptr);
		}
		break;
	case REQ_release:
		if (dat->flags & DATA_hold)
		{
			dat->flags &= ~DATA_hold;
			if (dat->raise)
			{
				val.dptr = (void*)dat;
				val.dsize = sizeof(*dat);
				if (!(n = dbm_store(state->dbm, key, val, DBM_INSERT)) || n > 0 && !dbm_store(state->dbm, key, val, DBM_REPLACE))
					log(state, con, 'L', "%s released", key.dptr);
				else if (!dbm_error(state->dbm))
					log(state, con, 'W', "%s unchanged", key.dptr);
				else
				{
					dbm_clearerr(state->dbm);
					log(state, con, 'E', "%s io error", key.dptr);
				}
				if (e = (Event_t*)dtmatch(state->events, key.dptr))
					notify(state, e);
			}
			else if (!dbm_delete(state->dbm, key))
				log(state, con, 'L', "%s deleted", key.dptr);
			else if (!dbm_error(state->dbm))
				log(state, con, 'W', "%s not in db", key.dptr);
			else
			{
				dbm_clearerr(state->dbm);
				log(state, con, 'E', "%s io error", key.dptr);
			}
		}
		break;
	case REQ_test:
		if (val.dptr && !(dat->flags & DATA_clear))
		{
			if (state->hold)
				log(state, con, 'W', "%s global hold", key.dptr);
			else if (dat->flags & DATA_hold)
				log(state, con, 'W', "%s explicit hold", key.dptr);
			else
				log(state, con, 'I', "%s raised", key.dptr);
		}
		else
			log(state, con, 'I', "%s not-raised", key.dptr);
		break;
	case REQ_wait:
		if (val.dptr && !state->hold && !(dat->flags & (DATA_clear|DATA_hold)))
			log(state, con, 'I', "%s raised", key.dptr);
		else if (post(state, con, key.dptr, id))
			return -1;
		break;
	}
	return 0;
}

/*
 * apply request r to args a
 */

static int
request(State_t* state, Connection_t* con, int id, int index, char** a, unsigned long older, unsigned long newer)
{
	char*			s;
	int			i;
	Event_t*		e;
	datum			key;
	datum			val;
	Data_t			dat;
	regex_t			re;
	char			buf[64];

	while (s = *a++)
		if (i = regcomp(&re, s, REG_SHELL|REG_AUGMENTED|REG_LEFT|REG_RIGHT))
		{
			regerror(i, &re, buf, sizeof(buf));
			log(state, con, 'E', "%s: %s", s, buf);
		}
		else if (regstat(&re)->re_info & REG_LITERAL)
		{
			if (!EVENT(s))
			{
				log(state, con, 'E', "%s invalid event name", s);
				return -1;
			}
			key.dptr = (void*)s;
			key.dsize = strlen(s) + 1;
			if (key.dsize >= sizeof(e->name))
				s[(key.dsize = sizeof(e->name)) - 1] = 0;
			val = dbm_fetch(state->dbm, key);
			if (val.dptr)
			{
				if (val.dsize > sizeof(dat))
					val.dsize = sizeof(dat);
				swapmem(state->swap, val.dptr, &dat, sizeof(dat));
			}
			else
				memset(&dat, 0, sizeof(dat));
			if (apply(state, con, id, index, key, val, &dat))
				return -1;
		}
		else
		{
		rescan:
			for (key = dbm_firstkey(state->dbm); key.dptr; key = dbm_nextkey(state->dbm))
				if (EVENT(key.dptr) && !regexec(&re, key.dptr, 0, NiL, 0))
				{
					val = dbm_fetch(state->dbm, key);
					if (val.dsize > sizeof(dat))
						val.dsize = sizeof(dat);
					swapmem(state->swap, val.dptr, &dat, val.dsize);
					if ((!older || dat.time < older) && (!newer || dat.time > newer))
					{
						if ((i = apply(state, con, id, index, key, val, &dat)) < 0)
							return -1;
						if (i > 0)
							goto rescan;
					}
				}
		}
	return 0;
}

/*
 * convert s to a date/time
 */

static unsigned long
date(State_t* state, Connection_t* con, const char* s)
{
	unsigned long	t;
	char*		e;
	datum		key;
	datum		val;
	Data_t		dat;

	key.dptr = (void*)s;
	key.dsize = strlen(s) + 1;
	val = dbm_fetch(state->dbm, key);
	if (val.dptr)
	{
		swapmem(state->swap, val.dptr, &dat, val.dsize);
		t = dat.time;
	}
	else
	{
		t = tmdate(s, &e, NiL);
		if (*e)
		{
			log(state, con, 'E', "%s: invalid date/time", s);
			t = 0;
		}
	}
	return t;
}

/*
 * service a request
 */

static int
actionf(register Css_t* css, register Cssfd_t* fp, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	con;
	char*			s;
	char*			t;
	char**			a;
	char**			q;
	Cssfd_t*		f;
	Request_t*		r;
	Event_t*		e;
	Waiting_t*		w;
	Connection_t*		x;
	int			n;
	int			err;
	int			id;
	unsigned long		older;
	unsigned long		newer;
	datum			key;
	datum			val;
	Data_t			data;
	char			buf[64];

	switch (fp->status)
	{
	case CS_POLL_CLOSE:
		if (con = (Connection_t*)fp->data)
			dtdelete(state->connections, con);
		return 0;
	case CS_POLL_READ:
		con = (Connection_t*)fp->data;
		if ((n = csread(css->state, fp->fd, state->req, sizeof(state->req), CS_LINE)) <= 0)
			return -1;
		state->req[--n] = 0;
		log(state, con, 'R', "%s", state->req);
		con->code = 0;
		if (tokscan(state->req, NiL, " %v ", state->cmd, elementsof(state->cmd) - 1) > 0)
		{
			id = -1;
			for (q = state->cmd; (s = *q) && (isalpha(*s) || *s == '_'); q++)
			{
				while (isalnum(*++s));
				if (*s != '=')
					break;
				if ((s - *q) == 2 && *(s - 1) == 'd' && *(s - 2) == 'i')
					id = (int)strtol(s + 1, NiL, 0);
			}
			s = *(a = q);
			if (!(r = (Request_t*)strpsearch(requests, elementsof(requests), sizeof(requests[0]), s, NiL)))
				log(state, con, 'E', "%s: unknown request", s);
			else
			{
				opt_info.index = 0;
				newer = older = 0;
				err = 0;
				sfstrseek(state->usrf, 0, SEEK_SET);
				for (;;)
				{
					switch (optget(a, usage))
					{
					case 'e':
						if (r->index == REQ_set)
						{
							state->expire = strelapsed(opt_info.arg, &t, 1);
							if (*t)
							{
								log(state, con, 'E', "%s: invalid elapsed time expression", opt_info.arg);
								err = 1;
								break;
							}
						}
						continue;
					case 'l':
						if (r->index == REQ_set)
							state->log = opt_info.num;
						continue;
					case 'n':
						newer = date(state, con, opt_info.arg);
						continue;
					case 'o':
						older = date(state, con, opt_info.arg);
						continue;
					case 'q':
						con->quiet = opt_info.num;
						continue;
					case '?':
					case ':':
						log(state, con, 'E', "%s: %s", s, opt_info.arg);
						err = 1;
						break;
					}
					break;
				}
				if (!err)
				{
					if (!*(a += opt_info.index))
					{
						if (newer || older)
						{
							a[0] = "*";
							a[1] = 0;
							n = 1;
						}
						else
							n = 0;
					}
					else
						n = a[1] ? 2 : 1;
					if (r->min && n < r->min)
						sfprintf(state->usrf, "E %s: at least %d argument%s expected\n", s, r->min, r->min == 1 ? "" : "s");
					else if (r->max > 0 && n > r->max)
						log(state, con, 'E', "%s: at most %d argument%s expected", s, r->max, r->max == 1 ? "" : "s");
					else if (r->min == r->max && n != r->max)
						log(state, con, 'E', "%s: %d argument%s expected", s, r->max, r->max == 1 ? "" : "s");
					else
						switch (r->index)
						{
						case REQ_all:
							n = (int)strtol(a[0], &t, 0);
							if (*t)
							{
								log(state, con, 'E', "%s: invalid numeric value", a[0]);
								break;
							}
							else if (!(f = cssfd(css, n, 0)) || !(x = (Connection_t*)f->data))
							{
								log(state, con, 'E', "%d: invalid connection index", n);
								break;
							}
							if (x->waiting)
							{
								n = x->quiet;
								x->quiet = 1;
								a = state->cmd;
								for (w = (Waiting_t*)dtfirst(x->waiting); w; w = (Waiting_t*)dtnext(x->waiting, w))
								{
									if (a >= &state->cmd[elementsof(state->cmd)-1])
									{
										*a = 0;
										if (request(state, x, -1, REQ_raise, a = state->cmd, older, newer))
											break;
									}
									*a++ = w->event->name;
									log(state, con, 'R', "%s %s", s, w->event->name);
								}
								if (a > state->cmd)
								{
									*a = 0;
									request(state, x, -1, REQ_raise, state->cmd, older, newer);
								}
								x->quiet = n;
							}
							log(state, con, 'I', "done");
							break;
						case REQ_clear:
						case REQ_delete:
						case REQ_raise:
						case REQ_test:
						case REQ_wait:
							if (request(state, con, id, r->index, a, older, newer))
								return -1;
							break;
						case REQ_exit:
							cssfd(css, fp->fd, CS_POLL_CLOSE);
							break;
						case REQ_info:
							info(state, con, css);
							break;
						case REQ_hold:
							if (!*a)
							{
								state->hold = 1;
								sfprintf(state->usrf, "I holding\n");
							}
							else
								if (request(state, con, id, r->index, a, older, newer))
									return -1;
							break;
						case REQ_list:
							con->all = 1;
							if (s = *a)
							{
								if (n = regcomp(&con->re, s, REG_SHELL|REG_AUGMENTED|REG_LEFT|REG_RIGHT))
								{
									regerror(n, &con->re, buf, sizeof(buf));
									log(state, con, 'E', "%s: %s", s, buf);
									break;
								}
								con->all = 0;
							}
							con->list = dbm_firstkey(state->dbm);
							if (!con->list.dptr)
							{
								log(state, con, 'I', "empty");
								break;
							}
							con->newer = newer;
							con->older = older;
							goto list;
						case REQ_next:
							if (!con->list.dptr)
							{
								log(state, con, 'W', "next: must execute list first");
								break;
							}
							for (;;)
							{
								con->list = dbm_nextkey(state->dbm);
								if (!con->list.dptr)
								{
									log(state, con, 'I', "done");
									break;
								}
						list:
								if (EVENT(con->list.dptr) && (con->all || !regexec(&con->re, con->list.dptr, 0, NiL, 0)))
								{
									val = dbm_fetch(state->dbm, con->list);
									if (val.dsize > sizeof(data))
										val.dsize = sizeof(data);
									swapmem(state->swap, val.dptr, &data, val.dsize);
									if ((!con->older || data.time < con->older) && (!con->newer || data.time > con->newer))
									{
										log(state, con, 'I', "event %s %s %d%s%s", con->list.dptr, fmttime("%K", data.time), data.raise, (data.flags & DATA_clear) ? " CLEAR" : "", (data.flags & DATA_hold) ? " HOLD" : "");
										break;
									}
								}
							}
							break;
						case REQ_release:
							if (!*a)
							{
								state->hold = 0;
								sfprintf(state->usrf, "I released\n");
								key = dbm_firstkey(state->dbm);
								while (key.dptr)
								{
									val = dbm_fetch(state->dbm, key);
									if (val.dsize > sizeof(data))
										val.dsize = sizeof(data);
									swapmem(state->swap, val.dptr, &data, val.dsize);
									if (!(data.flags & (DATA_clear|DATA_hold)) && (e = (Event_t*)dtmatch(state->events, key.dptr)))
										notify(state, e);
									key = dbm_nextkey(state->dbm);
								}
							}
							else
								if (request(state, con, id, r->index, a, older, newer))
									return -1;
							break;
						case REQ_set:
							break;
						case REQ_stop:
							exit(0);
							break;
						}
				}
			}
			if (sfstrtell(state->usrf))
			{
				if (id >= 0)
				{
					sfstrseek(state->usrf, 0, SEEK_SET);
					log(state, con, 'x', "%d %d", id, con->code);
				}
				n = sfstrtell(state->usrf);
				if (!(s = sfstruse(state->usrf)))
					error(ERROR_SYSTEM|3, "out of space");
				if (cswrite(css->state, fp->fd, s, n) != n)
					return -1;
			}
		}
		return 1;
	}
	return 0;
}

/*
 * handle exceptions
 */

static int
exceptf(Css_t* css, unsigned long op, unsigned long arg, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;

	switch (op)
	{
	case CSS_CLOSE:
		if (state->dbm)
		{
			dbm_close(state->dbm);
			state->dbm = 0;
		}
		return 0;
	case CSS_INTERRUPT:
		error(ERROR_SYSTEM|3, "%s: interrupt exit", fmtsignal(arg));
		return 0;
	case CSS_WAKEUP:
		error(2, "wakeup");
		return 0;
	}
	error(ERROR_SYSTEM|3, "poll error op=0x%08x arg=0x%08x", op, arg);
	return -1;
}

/*
 * free connection
 */

static void
confree(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	State_t*	state = (State_t*)((char*)disc - offsetof(State_t, condisc));
	Connection_t*	con = (Connection_t*)obj;

	NoP(dt);
	NoP(disc);
	state->active--;
	if (con->waiting)
		dtclose(con->waiting);
	log(state, con, 'S', "drop connection -- %d active", state->active);
	free(obj);
}

/*
 * free event
 */

static void
eventfree(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	NoP(dt);
	NoP(disc);
	free(obj);
}

/*
 * free connection pending event
 */

static void
waitfree(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	State_t*	state = (State_t*)((char*)disc - offsetof(State_t, waitdisc));
	Waiting_t*	p = (Waiting_t*)obj;

	NoP(dt);
	if (--p->event->waiting == 0)
		dtdelete(state->events, p->event);
	free(obj);
}

/*
 * open and verify event db
 */

static void
db(register State_t* state)
{
	datum		key;
	datum		val;
	Data_t		data;
	uint32_t	u4;

	if (!(state->dbm = dbm_open(state->path, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)))
		error(ERROR_SYSTEM|3, "%s: cannot open database for read/write", state->path);
	key.dptr = (void*)ident_key;
	key.dsize = sizeof(ident_key);
	val = dbm_fetch(state->dbm, key);
	if (val.dptr)
	{
		if (val.dsize != sizeof(data))
			error(3, "%s: invalid db -- data size %d, expected %d", state->path, val.dsize, sizeof(data));
		memcpy(&data, val.dptr, val.dsize);
		if (memcmp(&data.expire, ident_name, sizeof(data.expire)))
			error(3, "%s: %s: invalid db -- ident mismatch, expected %s", state->path, ident_key, ident_name);
		u4 = IDENT_SWAP;
		if (state->swap = swapop(&u4, &data.time, 4))
			swapmem(state->swap, &data, &data, sizeof(data));
	}
	else
	{
		val = dbm_firstkey(state->dbm);
		if (val.dptr)
			error(3, "%s: %s: invalid db -- ident entry expected", state->path, ident_key);
		memset(&data, 0, sizeof(data));
		memcpy(&data.expire, ident_name, sizeof(data.expire));
		data.time = IDENT_SWAP;
		data.raise = IDENT_VERSION;
		val.dptr = (void*)&data;
		val.dsize = sizeof(data);
		if (dbm_store(state->dbm, key, val, DBM_INSERT))
		{
			dbm_clearerr(state->dbm);
			error(3, "%s: %s: db initial ident entry store failed", state->path, ident_key);
		}
	}
	state->major = (data.raise >> 16) & 0xffff;
	state->minor = data.raise & 0xffff;
}

/*
 * client/server main
 */

int
main(int argc, char** argv)
{
	char*			s;
	Css_t*			css;

	char*			p = 0;
	int			server = 0;

	static State_t		state;

	NoP(argc);
	setlocale(LC_ALL, "");
	opt_info.argv = argv;
	state.log = 1;
	error_info.id = (char*)command;
	if (!(state.usrf = sfstropen()) || !(state.tmp = sfstropen()))
		error(3, "out of space [buf]");

	/*
	 * check the options
	 */

	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			p = opt_info.arg;
			continue;
		case 'e':
			state.expire = strelapsed(opt_info.arg, &s, 1);
			if (*s)
				error(2, "%s: invalid elapsed time expression", opt_info.arg);
			continue;
		case 'i':
			server = 1;
			continue;
		case 'l':
			state.log = opt_info.num;
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
	if (error_info.errors || !(state.path = *(argv += opt_info.index)))
		error(ERROR_USAGE|4, "%s", optusage(NiL));

	/*
	 * get the connect stream path
	 */

	if (s = strrchr(state.path, '/'))
		s++;
	else
		s = state.path;
	if (p)
		sfprintf(state.usrf, "%s/%s", p, s);
	else
		sfprintf(state.usrf, "/dev/tcp/local/%s/%s", error_info.id, s);
	if (!(state.service = strdup(sfstruse(state.usrf))))
		error(3, "out of space [service]");

	/*
	 * either server or client at this point
	 */

	if (server)
	{
		umask(S_IWOTH);
		db(&state);
		state.condisc.link = offsetof(Event_t, link);
		state.condisc.freef = confree;
		if (!(state.connections = dtopen(&state.condisc, Dtlist)))
			error(ERROR_SYSTEM|3, "out of space [connection dictionary]");
		state.eventdisc.link = offsetof(Event_t, link);
		state.eventdisc.key = offsetof(Event_t, name);
		state.eventdisc.freef = eventfree;
		if (!(state.events = dtopen(&state.eventdisc, Dtoset)))
			error(ERROR_SYSTEM|3, "out of space [event dictionary]");
		state.waitdisc.link = offsetof(Waiting_t, link);
		state.waitdisc.key = offsetof(Waiting_t, event);
		state.waitdisc.size = sizeof(Event_t*);
		state.waitdisc.freef = waitfree;
		state.disc.version = CSS_VERSION;
		state.disc.flags = CSS_DAEMON|CSS_LOG|CSS_ERROR|CSS_INTERRUPT|CSS_WAKEUP|CSS_PRESERVE;
		state.disc.timeout = 60 * 60 * 1000L;
		state.disc.acceptf = acceptf;
		state.disc.actionf = actionf;
		state.disc.errorf = errorf;
		state.disc.exceptf = exceptf;
		if (!(css = cssopen(state.service, &state.disc)))
			return 1;
		umask(S_IWOTH);
		error_info.id = css->id;
		if (state.log)
		{
			sfprintf(state.tmp, "%s.log", state.path);
			if (!(s = sfstruse(state.tmp)))
				error(ERROR_SYSTEM|3, "out of space");
			if (state.logf = sfopen(NiL, s, "a"))
				sfset(state.logf, SF_LINE, 1);
			else
				error(ERROR_SYSTEM|2, "%s: cannot append log file", s);
		}
		log(&state, 0, 'S', "start service %s", fmtident(usage));
		csspoll(CS_NEVER, 0);
		log(&state, 0, 'S', "stop service");
		return 1;
	}
	return csclient(&cs, -1, state.service, "event> ", argv + 1, CS_CLIENT_ARGV|CS_CLIENT_SEP);
}

#endif
