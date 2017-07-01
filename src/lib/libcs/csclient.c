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
 * client line by line interaction
 */

#include "cslib.h"

#include "FEATURE/termios"
#if !_lib_tcgetattr || !_lib_tcsetattr
#undef	_hdr_termios
#endif
#if _hdr_termios
#include <termios.h>
#endif

#if _hdr_termios

static struct State_s
{
	struct termios	new_term;	/* raw term for -r		*/
	struct termios	old_term;	/* original term for -r		*/
} state;

static void
restore(void)
{
	tcsetattr(0, TCSANOW, &state.old_term);
}

#endif

/*
 * if fd<0 then establish client connection to service
 * if argv specified then each is a command line to be executed
 * if CS_CLIENT_ARGV not set then additional commands read from stdin
 */

int
csclient(Cs_t* cs, int fd, const char* service, const char* prompt, char** argv, unsigned int flags)
{
	register int	i;
	char*		s;
	Sfio_t*		tmp;
	int		done;
	int		promptlen;
	int		timeout;
	ssize_t		n;
	int		sdf[2];
	Cspoll_t	fds[2];
	char		buf[8 * 1024];

	if (fd < 0 && (fd = csopen(cs, service, CS_OPEN_TEST)) < 0)
	{
		if (errno == ENOENT)
			error(3, "%s: server not running", service);
		else
			error(ERROR_SYSTEM|3, "%s: cannot open connect stream", service);
	}
#if _hdr_termios
	if (flags & CS_CLIENT_RAW)
	{
		tcgetattr(0, &state.old_term);
		atexit(restore);
		state.new_term = state.old_term;
		state.new_term.c_iflag &= ~(BRKINT|IGNPAR|PARMRK|INLCR|IGNCR|ICRNL);
		state.new_term.c_lflag &= ~(ECHO|ECHOK|ICANON|ISIG);
		state.new_term.c_cc[VTIME] = 0;
		state.new_term.c_cc[VMIN] = 1;
		tcsetattr(0, TCSANOW, &state.new_term);
	}
#endif
	sdf[0] = fd;
	sdf[1] = 1;
	if (argv && *argv)
	{
		fds[0].fd = 1;
		fds[0].events = CS_POLL_WRITE;
	}
	else
	{
		argv = 0;
		fds[0].fd = 0;
		fds[0].events = CS_POLL_READ;
	}
	fds[1].fd = fd;
	fds[1].events = CS_POLL_READ;
	done = 0;
	if (promptlen = (!argv && prompt && isatty(fds[0].fd) && isatty(1)) ? strlen(prompt) : 0)
		write(1, prompt, promptlen);
	timeout = CS_NEVER;
	tmp = 0;
	while (cspoll(cs, fds, elementsof(fds), timeout) > 0)
		for (i = 0; i < elementsof(fds); i++)
			if (fds[i].status & (CS_POLL_READ|CS_POLL_WRITE))
			{
				if (!i && argv)
				{
					if (!*argv)
					{
						if (flags & CS_CLIENT_ARGV)
						{
							if (done++)
								return 0;
							timeout = 500;
						}
						else
						{
							argv = 0;
							fds[0].fd = 0;
							fds[0].events = CS_POLL_READ;
						}
						continue;
					}
					if (!tmp && !(tmp = sfstropen()))
						error(ERROR_SYSTEM|3, "out of space");
					for (;;)
					{
						s = *argv++;
						if ((flags & CS_CLIENT_SEP) && *s == ':' && !*(s + 1))
							break;
						if (sfstrtell(tmp))
							sfputc(tmp, ' ');
						sfprintf(tmp, "%s", s);
						if (!(flags & CS_CLIENT_SEP) || !*argv)
							break;
					}
					sfputc(tmp, '\n');
					n = sfstrtell(tmp);
					s = sfstruse(tmp);
				}
				else if ((n = read(fds[i].fd, s = buf, sizeof(buf) - 1)) < 0)
					error(ERROR_SYSTEM|3, "/dev/fd/%d: read error", fds[i].fd);
				if (!n)
				{
					if (done++)
						return 0;
					if (!i)
						write(sdf[i], "quit\n", 5);
					continue;
				}
				if (!i)
				{
#if _hdr_termios
					register char*	u;
					register int	m;

					s[n] = 0;
					if ((u = strchr(s, 035)))
					{
						if ((m = u - s) > 0 && write(sdf[i], s, m) != m)
							error(ERROR_SYSTEM|3, "/dev/fd/%d: write error", sdf[i]);
						tcsetattr(0, TCSANOW, &state.old_term);
						if (promptlen)
							write(1, prompt, promptlen);
						if ((n = read(fds[i].fd, s = buf, sizeof(buf) - 1)) <= 0)
						{
							write(1, "\n", 1);
							return 0;
						}
						buf[n - 1] = 0;
						if (*u == 'q' || *u == 'Q')
							return 0;
						tcsetattr(0, TCSANOW, &state.new_term);
						if (*u)
							error(1, "%s: unknown command", u);
						continue;
					}
#endif
				}
				if (write(sdf[i], s, n) != n)
					error(ERROR_SYSTEM|3, "/dev/fd/%d: write error", sdf[i]);
				if (sdf[i] == 1 && promptlen)
					write(1, prompt, promptlen);
			}
	return error_info.errors != 0;
}

int
_cs_client(int fd, const char* service, const char* prompt, char** argv, unsigned int flags)
{
	return csclient(&cs, fd, service, prompt, argv, flags);
}
