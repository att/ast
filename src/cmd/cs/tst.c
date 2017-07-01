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
 * cs service test
 */

static const char id[] = "@(#)$Id: cs.tst (AT&T Research) 1997-11-11 $\0\n";

#define TST_VERSION	"1.0"

#include <css.h>
#include <ctype.h>
#include <error.h>
#include <tm.h>

typedef struct
{
	Csid_t		id;
	char*		args;
} Connection_t;

static char	buf[1024];
static char	dat[1024];

static int
acceptf(Css_t* css, Cssfd_t* fp, Csid_t* ip, char** av, Cssdisc_t* disc)
{
	register Connection_t*	con;
	char*			s;
	Sfio_t*			sp;
	char**			ap;

	if (!(con = newof(0, Connection_t, 1, 0)))
		return -1;
	fp->data = con;
	con->id = *ip;
	con->args = 0;
	if ((ap = av) && (sp = sfstropen()))
	{
		while (s = *ap++)
		{
			if (ap > av + 1)
				sfputc(sp, ' ');
			sfputr(sp, s, -1);
		}
		con->args = strdup(sfstruse(sp));
		sfclose(sp);
	}
	return fp->fd;
}

static int
actionf(register Css_t* css, register Cssfd_t* fp, Cssdisc_t* disc)
{
	register Connection_t*	con;
	int			n;

	switch (fp->status)
	{
	case CS_POLL_CLOSE:
		if (con = (Connection_t*)fp->data)
		{
			if (con->args)
				free(con->args);
			free(con);
		}
		return 0;
	case CS_POLL_READ:
		con = (Connection_t*)fp->data;
		if ((n = csread(css->state, fp->fd, dat, sizeof(dat), CS_LINE)) <= 0)
			return -1;
		dat[--n] = 0;
		if (isalpha(dat[0]) && (dat[1] == 0 || isdigit(dat[1]))) switch (dat[0])
		{
		case 'd':
			error_info.trace = -(int)strtol(dat + 1, NiL, 0);
			n = sfsprintf(buf, sizeof(buf), "I debug level %d\n", -error_info.trace);
			break;
		case 'Q':
			exit(0);
			/*FALLTHROUGH*/
		case 'q':
			return -1;
		default:
			n = sfsprintf(buf, sizeof(buf), "E %s: unknown command\n", dat);
			break;
		}
		else
			n = sfsprintf(buf, sizeof(buf), "I [%s] server=%s version=%s %s=%s server.pid=%d pid=%d uid=%d gid=%d args=`%s'\n", fmttime(*dat ? dat : "%K", time(NiL)), csname(css->state, 0), TST_VERSION, CS_HOST_LOCAL, csntoa(css->state, con->id.hid), getpid(), con->id.pid, con->id.uid, con->id.gid, con->args);
		if (cswrite(css->state, fp->fd, buf, n) != n)
			return -1;
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
		error(2, "dormant exit");
		exit(0);
	case CSS_WAKEUP:
		error(2, "wakeup");
		return 0;
	}
	error(ERROR_SYSTEM|3, "poll error op=0x%08x arg=0x%08x", op, arg);
	return -1;
}

int
main(int argc, char** argv)
{
	Css_t*		css;
	Cssdisc_t	disc;

	NoP(argc);
	memset(&disc, 0, sizeof(disc));
	disc.version = CSS_VERSION;
	disc.flags = CSS_DAEMON|CSS_LOG|CSS_ERROR|CSS_DORMANT|CSS_INTERRUPT|CSS_WAKEUP;
	disc.timeout = 20 * 1000L;
	disc.wakeup = 4 * 1000L;
	disc.acceptf = acceptf;
	disc.actionf = actionf;
	disc.errorf = errorf;
	disc.exceptf = exceptf;
	if (!(css = cssopen(argv[1], &disc)))
		exit(1);
	error_info.id = css->id;
	csspoll(CS_NEVER, 0);
	exit(1);
}
