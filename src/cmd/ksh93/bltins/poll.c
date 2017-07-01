/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2007-2012 AT&T Intellectual Property          *
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
*               Roland Mainz <roland.mainz@nrubsig.org>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "defs.h"
#include "variables.h"
#include "lexstates.h"
#include "io.h"
#include "name.h"
#include "builtins.h"
#include "history.h"
#include "terminal.h"
#include <stdio.h>
#include <poll.h>
#include <tmx.h>
#include <stk.h>

#ifndef SH_DICT
#   define SH_DICT "libshell"
#endif

#define sh_contexttoshell(context)	((context)?((context)->shp):(NULL))

static
const char sh_optpoll[] =
"[-?\n@(#)$Id: poll (AT&T Labs Research) 2012-08-16 $\n]"
"[-author?Roland Mainz <roland.mainz@nrubsig.org>]"
"[-license?http://www.eclipse.org/org/documents/epl-v10.html]"
"[+NAME? poll - input/output multiplexing]"
"[+DESCRIPTION?The poll command provides applications with a "
	"mechanism for multiplexing input/output over a set of "
	"file descriptors. "
	"For each member of the (optionally sparse) indexed or "
	"associative compound or type (see \btypeset -T\b) array "
	"variable \bvar\b, poll examines the given file descriptor "
	"in the subscript \b.fd\b for the event(s) specified in "
	"the subscript \b.events\b. "
	"The poll command identifies those file descriptors on "
	"which an application can read or write data, or on which "
	"certain events have occurred.]"
"[+?The \bvar\b argument specifies an array of file descriptors to "
	"be examined and the events of interest for each file "
	"descriptor. "
	"It is a array of compound variables (or user-defined type) "
	"with one member for each open file descriptor of interest. "
	"The array's compound or type variable members contain the "
	"following subscripts:]{"
		"[+?\b.fd\b       # file descriptor]"
		"[+?\b.events\b   # compound variable of requested event(s)]"
		"[+?\b.revents\b  # compound variable of returned event(s)]"
	"}"
"[+?The \bfd\b variable specifies an open file descriptor and the "
	"\bevents\b and \brevents\b members are compound variables "
	"constructed from the following boolean member variables:]"
	"{ "
	"[+.pollin?('true'|'false') Data other than high priority "
		"data may be read without blocking. For STREAMS, this "
		"flag is set in \brevents\b even if the message "
		"is of zero length.]"
	"[+.pollrdnorm?('true'|'false') Normal data (priority band "
		"equals 0) may be read without blocking. For STREAMS, "
		"this flag is set in \brevents\b even if the message "
		"is of zero length.]"
	"[+.pollrdband?('true'|'false') Data from a non-zero "
		"priority band may be read without blocking. For "
		"STREAMS, this flag is set in \brevents\b even if the "
		"message is of zero length.]"
	"[+.pollpri?('true'|'false') High priority data may be "
		"received without blocking. For STREAMS, this flag is "
		"set in \brevents\b even if the message is of zero "
		"length.]"
	"[+.pollout?('true'|'false') Normal data (priority band "
		"equals 0) may be written without blocking.]"
	"[+.pollwrnorm?('true'|'false') The same as \bpollout\b.]"
	"[+.pollwrband?('true'|'false') Priority data (priority band "
		"> 0) may be written.  This event only examines bands "
		"that have been written to at least once.]"
	"[+.pollerr?('true'|'false') An error has occurred on the "
		"device or stream.  This flag is only valid in the "
		"\brevents\b compound variable; it is not used in the "
		"\bevents\b compound variable.]"
	"[+.pollhup?('true'|'false') A hangup has occurred on the "
		"stream. This event and \bpollout\b are mutually "
		"exclusive; a stream can never be writable if a "
		"hangup has occurred. "
		"However, this event and \bpollin\b, "
		"\bpollrdband\b, or \bpollpri\b are not "
		"mutually exclusive. This flag is only valid "
		"in the \brevents\b compound variable; it is not "
		"used in the \bevents\b compound variable.]"
	"[+.pollnval?('true'|'false') The specified fd value does "
		"not belong to an open file. "
		"This flag is only valid in the \brevents\b compound "
		"variable; it is not used in the \bevents\b "
		"compound variable.]"
   "}"
"]"

"[+?If the value fd is less than 0, events is ignored and "
	"revents is set to 0 in that entry on return from poll.]"

"[+?The results of the poll query are stored in the \brevents\b "
	"compound variable members in the \bvar\b structure. "
	"\bpoll*\b-variables are set in the \brevents\b compound "
	"variable to indicate which of the requested events are true. "
	"If none are true, the matching member in the \brevents\b "
	"compound variable will have the value of 'false' when the "
	"poll command returns. "
	"The \brevents\b compound variable members \bpollhup\b, "
	"\bpollerr\b, and \bpollnval\b are always set to 'true' in "
	"\brevents\b if the conditions they indicate are true; this "
	"occurs even though these flags were not present and/or set "
	"to 'true' in the \bevents\b compound variable.]"

"[+?If none of the defined events have occurred on any selected "
	"file descriptor, poll waits at least timeout milliseconds "
	"for an event to occur on any of the selected file "
	"descriptors. "
	"On a computer where millisecond timing accuracy is not "
	"available, timeout is rounded up to the nearest legal value "
	"available on that system. If the value timeout is 0, poll "
	"returns immediately. If the value of timeout is -1, poll "
	"blocks until a requested event occurs or until the call is "
	"interrupted.]"

"[+?The poll utility supports regular files, terminal and "
	"pseudo-terminal devices, STREAMS-based files, FIFOs, and "
	"pipes. The behavior of poll on elements of fds that refer "
	"to other types of file is unspecified.]"

"[+?The poll utility supports sockets.]"
#ifdef __SunOS
"[+?The poll utility may be used on Solaris on directory fds of "
	"/proc/$pid/ to get a \bpollhup='true'\b when the process quits.]"
#endif
"[+?A file descriptor for a socket that is listening for connections "
	"will indicate that it is ready for reading, once connections "
	"are available. A file descriptor for a socket that "
	"is connecting asynchronously will indicate that it is ready "
	"for writing, once a connection has been established.]"
 
"[+?Regular files always poll TRUE for reading and writing.]"

"[e:eventarray]:[fdcount?Upon successful completion, an indexed array "
	"of strings is returned which contains a list of array "
	"subscripts in the poll array which received events.]"
"[S!:pollsfio?Look into sfio streams for buffered information and set "
	"pollin/pollout to reflect sfio stream state.]"
"[R:pollttyraw?Put tty connections into raw mode when polling. The "
	"fd is returned to tty cooked mode before poll(1) exits.]"
"[t:timeout]:[seconds?Timeout in seconds. If the value timeout is 0, "
	"poll returns immediately. If the value of timeout is -1, "
	"poll blocks until a requested event occurs or until the "
	"call is interrupted.]"
"\n"
"\nvar\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Success.]"
        "[+>0?An error occurred.]"
"}"
"[+NOTES?]{"
	"[+?poll*-variables defined in \bevents\b will always appear "
	"in \brevents\b. This gives the script author control over "
	"which poll*-variables he can expect in \brevents\b.]"

	"[+?The \bpollinhup\b, \bpollnval\b and \bpollerr\b variables "
	"may appear in the \brevents\b compound variable even if "
	"they were not requested in \bevents\b.]"

	"[+?Using the value of variables in \brevents\b which are "
	"not set in \bevents\b can be done by putting a '-' suffix "
	"after the variable name, e.g. use "
	"\b${ar[x]].revents.pollhup-}\b to get the value of "
	"\bar[x]].revents.pollhup\b or an empty string if the variable "
	"was not set.]"

	"[+?Like \bpoll\b(2) it is legal to poll on the same fd in "
	"multiple entries, for exanple to listen for different events "
	"or to allow multiple callers to pool their poll lists "
	"together into one \bpoll\b(1) call.]"
"}"

/* quoting: ']' must be quoted as "]]" and '?' must be quoted as "//" */
"[+EXAMPLES?]{"
	"[+?The following example will wait for 10 seconds for input "
	"on fd 0, variable \bp[fd0]].revents.pollin\b will be 'true' "
	"or 'false' depening on whether the stream 0 is ready for "
	"reading:]{"
		"[+?compound -A p=( [fd0]]=( fd=0 events=( pollin='true' ) ) ) ; poll -t10 p ; print -v p]"
	"}"

	"[+?The following example will wait for 2 seconds for input "
	"on fd 0, and variables \bp[0]].revents.pollin\b and "
	"\bp[0]].revents.pollhup\b will be 'true' after polling ends "
	"because there is both input data available and the end of "
	"the stream was reached:]{"
		"[+?printf '\\n' | ksh -c 'compound -a p=( ( fd=0 events=( pollin=\"true\" pollhup=\"true\" ) ) ) ; poll -t2 p ; print -v p']"
	"}"
"}"

"[+SEE ALSO?\bopen\b(1),\btmpfile\b(1),\bdup\b(1),\bclose\b(1),\bpoll\b(2)]"
;


/* Like |nv_open()| but constructs variable name on the fly using |sfsprintf()| format */
static
Namval_t *nv_open_fmt(Dt_t *dict, int flags, const char *namefmt, ...)
{
	char 	varnamebuff[PATH_MAX];
	va_list	ap;

	va_start(ap, namefmt);
	vsnprintf(varnamebuff, sizeof(varnamebuff), namefmt, ap);
	va_end(ap);
	
	return nv_open(varnamebuff, dict, flags);
}

/* Name/value mapping table for POLL*-flags */
struct pollflagnamemap
{
	const int flag;
	const char *name;
};

static const
struct pollflagnamemap pfnm[]=
{
	{ POLLIN,	"pollin"	},
#ifdef POLLPRI
	{ POLLPRI,	"pollpri"	},
#endif
	{ POLLOUT,	"pollout"	},
#ifdef POLLRDNORM
	{ POLLRDNORM,	"pollrdnorm"	},
#endif
#ifdef POLLWRNORM
	{ POLLWRNORM,	"pollwrnorm"	},
#endif
#ifdef POLLRDBAND
	{ POLLRDBAND,	"pollrdband"	},
#endif
#ifdef POLLWRBAND
	{ POLLWRBAND,	"pollwrband"	},
#endif
#ifdef POLLMSG
	{ POLLMSG,	"pollmsg"	},
#endif
#ifdef POLLREMOVE
	{ POLLREMOVE,	"pollremove"	},
#endif
#ifdef POLLRDHUP
	{ POLLRDHUP,	"pollrdhup"	},
#endif
	{ POLLERR,	"pollerr"	},
	{ POLLHUP,	"pollhup"	},
	{ POLLNVAL,	"pollnval"	},
	{ 0,		NULL		},
};

/* structure to keep track of per array entry data */
struct pollstat
{
	/* name of array subscript */
	const char *array_subscript;

	/* |sfio| keeps track of sfio information */
	struct
	{
		Sfio_t	*sfd;
		ssize_t flags;
	} sfio;

	/*
	 * Bits in |eventvar_found| are POLL*-bits, set if matching
	 * ar[i].events.poll* var was found. We use this later to
	 * set the same ar[i].revents.poll* variable, regardless
	 * whether it was polled or not. This was done so the script
	 * author can control which poll* variables in the "revents"
	 * compound appear and which not.
	 */
	int eventvar_found;
};

/* poll on given |fds| data and retry after EINTR/EAGAIN while adjusting timeout */
static
int poll_loop(Shbltin_t* context, struct pollfd *fds, nfds_t nfds, int timeout)
{
/* nanoseconds to milliseconds */
#define TIME_NS2MS(t) ((t)/(1000UL*1000UL))
/* milliseconds to nanoseconds */
#define TIME_MS2NS(t) (((Time_t)(t))*(1000UL*1000UL))

	int n;

	/* We need two codepaths here:
	 * 1. timeout > 0:  we have to wait for |timeout| or events.
	 * 2. timeout <= 0: we have to wait forever (-1), return
	 *    immediately (0) or an event occurs.
	 */
	if (timeout > 0)
	{
		const Time_t starttime = tmxgettime();
		Time_t timeout_ns = TIME_MS2NS(timeout);

		do
		{
			while(((n = poll(fds, nfds, timeout)) < 0) &&
				((errno == EINTR) || (errno == EAGAIN)) &&
				(!context->sigset))
				errno=0;

			timeout_ns=timeout_ns-(tmxgettime()-starttime);
			timeout=TIME_NS2MS(timeout_ns);
		} while((timeout > 0) && (!context->sigset));
	}
	else
	{
		while(((n = poll(fds, nfds, timeout)) < 0) &&
			((errno == EINTR) || (errno == EAGAIN)) &&
			(!context->sigset))
			errno=0;
	}
	return n;
}


/* get ".poll*"-variables in "ar[i].events" and store data in |currpollfd| and |currps| */
static
bool get_compound_revents(Shell_t *shp, const char *parrayname, struct pollstat *currps, struct pollfd *currpollfd)
{
	const char	*subname=currps->array_subscript;
	Namval_t	*np;
	int		fd;
	int		pi;

	np = nv_open_fmt(shp->var_tree, NV_VARNAME|NV_NOFAIL|NV_NOADD, "%s[%s].fd", parrayname, subname);
	if (!np)
	{
		errormsg(SH_DICT, ERROR_ERROR, "missing pollfd %s[%s].fd", parrayname, subname);
		return false;
	}
	fd = (int)nv_getnum(np);
	nv_close(np);
	if ((fd < -1) || (fd > OPEN_MAX))
	{
		errormsg(SH_DICT, ERROR_ERROR, "invalid pollfd %s[%s].fd %d", parrayname, subname, fd);
		return false;
	}
	currpollfd->fd = fd;

	np = nv_open_fmt(shp->var_tree, NV_VARNAME|NV_COMVAR|NV_NOFAIL|NV_NOADD, "%s[%s].events", parrayname, subname);
	if (!np)
	{
		errormsg(SH_DICT, ERROR_ERROR, "missing pollfd %s[%s].events", parrayname, subname);
		return false;
	}
	nv_close(np);

	currpollfd->events=0;
	currpollfd->revents=0;
	currps->eventvar_found=0;
	for (pi=0 ; pfnm[pi].name != NULL ; pi++)
	{
		const char *s;

		np = nv_open_fmt(shp->var_tree, NV_VARNAME|NV_NOFAIL|NV_NOADD, "%s[%s].events.%s", parrayname, subname, pfnm[pi].name);
		if (!np)
			continue;

		currps->eventvar_found |= pfnm[pi].flag;
		s=nv_getval(np);
		if (s != NULL)
		{
			if (!strcmp(s, "true"))
				currpollfd->events |= pfnm[pi].flag;
			else if (!strcmp(s, "false"))
				;
			else
				errormsg(SH_DICT, ERROR_ERROR, "invalid boolean value % in variable %s[%s].events", s, parrayname, subname);
		}
		nv_close(np);
	}

	return true;
}

/* set ".poll*"-variables in "ar[i].revents" per data in |currpollfd| and |currps| */
static
void set_compound_revents(Shell_t *shp, const char *parrayname, struct pollstat *currps, struct pollfd *currpollfd)
{
	const char *subname=currps->array_subscript;
	Namval_t *np;
	int pi;

	np = nv_open_fmt(shp->var_tree, NV_VARNAME|NV_NOFAIL|NV_COMVAR, "%s[%s].revents", parrayname, subname);
	if (!np)
	{
		errormsg(SH_DICT, ERROR_ERROR, "could not create pollfd %s[%s].revents", parrayname, subname);
		return;
	}
	nv_setvtree(np); /* make "revents" really a compound variable */
	nv_close(np);

	for (pi=0 ; pfnm[pi].name != NULL ; pi++)
	{
		/*
		 * POLLHUP|POLLNVAL|POLLERR can always appear in |currpollfd->revents|
		 * even if we did not request them in |currpollfd->events|
		 */
		if ((currps->eventvar_found & pfnm[pi].flag) ||
			((currpollfd->revents & (POLLHUP|POLLNVAL|POLLERR)) & pfnm[pi].flag))
		{
			np = nv_open_fmt(shp->var_tree, NV_VARNAME|NV_NOFAIL, "%s[%s].revents.%s", parrayname, subname, pfnm[pi].name);
			if (!np)
				continue;

			nv_putval(np, ((currpollfd->revents & pfnm[pi].flag)?"true":"false"), 0);
			nv_close(np);
		}
	}
}

/* |main()| for poll(1) builtin */
extern
int b_poll(int argc, char *argv[], Shbltin_t* context)
{
	Shell_t		*shp = sh_contexttoshell(context);
	Namval_t	*np,
			*array_np,
			*array_np_sub;
	Sfio_t		*strstk		= NULL; /* stk object for memory allocations */
	const char	*parrayname,		/* name of array with poll data */
			*eventarrayname = NULL, /* name of array with indexes to results */
			*subname,		/* current subscript */
			*s;
	int		n;
	nfds_t		numpollfd = 0;		/* number of entries to poll */
	int		i,
			j;
	double		timeout		= -1.;
	char		buff[PATH_MAX*2+1];	/* fixme: theoretically enough to hold two variable names */
	bool		ttyraw		= false;/* put ttys into raw more when polling */
	bool		pollsfio	= true; /* should we ask sfio layer if it has data cached ? */
	int		pi;			/* index for |pfnm| */
	struct pollfd   *pollfd		= NULL,	/* data for poll(2) */
			*currpollfd;		/* current |pollfd| we are working on */
	struct pollstat *pollstat	= NULL,	/* context data from shell array */
			*currps;		/* current |pollstat| we are working on */
	int		retval		= 0;	/* return value of builtin */

	while (n = optget(argv, sh_optpoll)) switch (n)
	{
		case 't':
			errno = 0;
			timeout = strtod(opt_info.arg, (char **)NULL);	
			if (errno != 0)
				errormsg(SH_DICT, ERROR_system(1), "%s: invalid timeout", opt_info.arg);

			/* -t uses seconds */
			if (timeout >=0)
				timeout *= 1000.;
			break;
		case 'e':
			eventarrayname = opt_info.arg;
			break;
		case 'S':
			pollsfio=opt_info.num?true:false;
			break;
		case 'R':
			ttyraw=opt_info.num?true:false;
			break;
		case ':':
			errormsg(SH_DICT, ERROR_ERROR, "%s", opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
			break;
	}
	argc -= opt_info.index;
	argv += opt_info.index;
	if(argc!=1)
		errormsg(SH_DICT, ERROR_usage(2), optusage((char*)0));

	parrayname = argv[0];

	strstk = stkopen(0);
	if (!strstk)
		errormsg(SH_DICT, ERROR_system(1), e_nospace);

	array_np = nv_open(parrayname, shp->var_tree, NV_VARNAME|NV_NOFAIL|NV_NOADD);
	if (!array_np)
	{
		stkclose(strstk);
		errormsg(SH_DICT, ERROR_system(1), "cannot find array variable %s", parrayname);
	}
	if (!nv_isattr(array_np, NV_ARRAY))
	{
		nv_close(array_np);
		stkclose(strstk);
		errormsg(SH_DICT, ERROR_system(1), "variable %s is not an array", parrayname);
	}

	/*
	 * Count number of array elememts. We need to do it "manually"
	 * to handle sparse indexed and associative arrays
	 */
	nv_putsub(array_np, NULL, 0, ARRAY_SCAN);
	array_np_sub = array_np;
	do
	{
		if (!(subname=nv_getsub(array_np_sub)))
			break;
		numpollfd++;
	} while(array_np_sub && nv_nextsub(array_np_sub));

	/*
	 * Done with counting, now we need to allocate a work area big enough
	 */
	pollfd   = (struct pollfd   *)stkalloc(strstk, (sizeof(struct pollfd)   * numpollfd));
	pollstat = (struct pollstat *)stkalloc(strstk, (sizeof(struct pollstat) * numpollfd));
	if (!pollfd || !pollstat)
	{
		errormsg(SH_DICT, ERROR_ERROR, e_nospace);
		goto done_error;
	}

	/*
	 * Walk the array again and fetch the data we need...
	 */
	nv_putsub(array_np, NULL, 0, ARRAY_SCAN);
	array_np_sub = array_np;
	i = 0;
	do
	{
		if (!(subname=nv_getsub(array_np_sub)))
			break;

		pollstat[i].array_subscript=stkcopy(strstk, subname);
		if (!pollstat[i].array_subscript)
		{
			errormsg(SH_DICT, ERROR_ERROR, e_nospace);
			goto done_error;
		}

		if (!get_compound_revents(shp, parrayname, &pollstat[i], &pollfd[i]))
			goto done_error;

		i++;
	} while(array_np_sub && nv_nextsub(array_np_sub));

	nv_close(array_np);
	array_np=NULL;

	/*
	 * If sfio handles fds we need to check whether there are
	 * any data in the sfio buffers and remember this information
	 * so we can set { POLLIN, POLLOUT } on demand to reflect
	 * this information.
	 */
	if (pollsfio)
	{
		Sfio_t	**sfd;
		int	fd;
		int	num_sfd=0,
			active_sfd=0;

		sfd = (Sfio_t **)stkalloc(strstk, (sizeof(Sfio_t *) * (numpollfd+1)));
		if (!sfd)
		{
			errormsg(SH_DICT, ERROR_ERROR, e_nospace);
			goto done_error;
		}

		for (i=0 ; i < numpollfd ; i++)
		{
			currps=&pollstat[i];
			fd=pollfd[i].fd;
			
			currps->sfio.sfd=(fd>=0)?sh_fd2sfio(shp, fd):NULL;
			currps->sfio.flags=0;
			if (currps->sfio.sfd!=NULL)
			{
				/* Only add |currps->sfio.sfd| to the
				 * |sfd| array (list of |Sfio_t*|
				 * passed to |sfpoll()|) if it is not
				 * in that list yet. This prevents
				 * that we call |sfpoll()| on the same
				 * sfio stream multiple times (which
				 * can happen if pollfd contains the
				 * same fd multiple times (which is
				 * valid usage, for example if multiple
				 * consumers pool their pool lists in
				 * one poll call or listen to different
				 * sets of poll event flags)).
				 */
				for (j=0 ; j < num_sfd ; j++)
				{
					if (sfd[j]==currps->sfio.sfd)
						break;
				}
				if (j == num_sfd)
					sfd[num_sfd++]=currps->sfio.sfd;
			}
		}

		active_sfd = sfpoll(&sfd[0], num_sfd, 0);
		if (active_sfd > 0)
		{
			ssize_t sfpoll_flags;

			for (i=0 ; i < active_sfd ; i++)
			{
				sfpoll_flags=sfvalue(sfd[i]);

				/*
				 * We have to loop over all entries
				 * because single fd may be polled
				 * multiple times in different pollfd
				 * entries
				 */
				for (j=0 ; j < numpollfd ; j++)
				{
					if (pollstat[j].sfio.sfd == sfd[i])
						pollstat[j].sfio.flags=sfpoll_flags;
				}
			}
		}
	}

	/*
	 * Create --eventarray array on demand
	 */
	if (eventarrayname)
	{
		np = nv_open_fmt(shp->var_tree, NV_VARNAME|NV_ARRAY|NV_NOFAIL, "%s", eventarrayname);
		if (!np)
		{
			errormsg(SH_DICT, ERROR_ERROR, "could not create eventarray variable %s", eventarrayname);
			goto done_error;
		}

		nv_close(np);
	}

	/*
	 * Make sure we poll on "raw" tty to catch _every_ keystroke...
	 */
	if (ttyraw)
	{
		int fd;

		for (i=0 ; i < numpollfd ; i++)
		{
			fd=pollfd[i].fd;
			if ((fd >=0) && (shp->fdstatus[fd]&IOTTY))
				tty_raw(fd, 1);
		}
	}

	/*
	 * ... then poll for events...
	 */
	n = poll_loop(context, pollfd, numpollfd, timeout);

	/* 
	 * ... and restore the tty's to "cooked" mode
	 */
	if (ttyraw)
	{
		int fd;

		for (i=0 ; i < numpollfd ; i++)
		{
			fd=pollfd[i].fd;
			if ((fd >=0) && (shp->fdstatus[fd]&IOTTY))
				tty_cooked(fd);
		}
	}

	if (n < 0)
	{
		/* |ERROR_system(0)| won't quit the builtin */
		errormsg(SH_DICT, ERROR_system(0), "poll(2) failure");
		retval=1;
	}

	/*
	 * Write results back into the array
	 */
	for (i=0 ; i < numpollfd ; i++)
	{
		/* Adjust data in |pollfd[i]| to reflect sfio stream status (if requested) */
		if (pollsfio)
		{
			if ((pollfd[i].events & POLLIN)  && (pollstat[i].sfio.flags & SF_READ))
				pollfd[i].revents |= POLLIN;
			if ((pollfd[i].events & POLLOUT) && (pollstat[i].sfio.flags & SF_WRITE))
				pollfd[i].revents |= POLLOUT;
		}

		set_compound_revents(shp, parrayname, &pollstat[i], &pollfd[i]);

		/* Add array index to eventarray if this pollfd entry had any events */
		if (eventarrayname && pollfd[i].revents)
		{
			sfsprintf(buff, sizeof(buff), "%s+=( '%s' )", eventarrayname, pollstat[i].array_subscript);
			sh_trap(shp, buff, 0);
		}
	}
	
	goto done;

done_error:
	retval=1;
done:
	if (array_np)
		nv_close(array_np);
	if (strstk)
		stkclose(strstk);
	
	return(retval);
}
