/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2012 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

#ifndef _terminal_
#define _terminal_	1

/*
 * terminal interface
 * complicated by the fact that there are so many variations
 * This will use POSIX <termios.h> interface where available
 */

#   include	<termios.h>

#   undef tcgetattr
#   undef tcsetattr
    /* The following corrects bugs in some implementations */
#   if defined(TCSADFLUSH) && !defined(TCSAFLUSH)
#	define TCSAFLUSH	TCSADFLUSH
#   endif /* TCSADFLUSH */
#   undef TIOCGETC
#   if SHOPT_OLDTERMIO  /* use both termios and termio */
#	    include	<termio.h>
#   endif /* SHOPT_OLDTERMIO */


/* set ECHOCTL if driver can echo control charaters as ^c */
#ifdef LCTLECH
#   ifndef ECHOCTL
#	define ECHOCTL	LCTLECH
#   endif /* !ECHOCTL */
#endif /* LCTLECH */
#ifdef LNEW_CTLECH
#   ifndef ECHOCTL
#	define ECHOCTL  LNEW_CTLECH
#   endif /* !ECHOCTL */
#endif /* LNEW_CTLECH */
#ifdef LNEW_PENDIN
#   ifndef PENDIN
#	define PENDIN LNEW_PENDIN
#  endif /* !PENDIN */
#endif /* LNEW_PENDIN */
#ifndef ECHOCTL
#   ifndef VEOL
#	define RAWONLY	1
#   endif /* !VEOL */
#endif /* !ECHOCTL */

#ifdef _sys_filio
#   ifndef FIONREAD
#	include	<sys/filio.h>
#   endif /* FIONREAD */
#endif /* _sys_filio */
/* set FIORDCHK if you can check for characters in input queue */
#ifdef FIONREAD
#   ifndef FIORDCHK
#	define FIORDCHK	FIONREAD
#   endif /* !FIORDCHK */
#endif /* FIONREAD */

extern int	tty_alt(int);
extern void	tty_cooked(int);
extern int	tty_get(int,struct termios*);
extern int	tty_raw(int,int);
extern int	tty_check(int);
extern int	tty_set(int, int, struct termios*);
extern int	sh_ioctl(int,int,void*,int);
#define ioctl(a,b,c)	sh_ioctl(a,b,c,sizeof(c))

extern int	sh_tcgetattr(int,struct termios*);
extern int	sh_tcsetattr(int,int,struct termios*);
#   define tcgetattr(a,b)	sh_tcgetattr(a,b)
#   define tcsetattr(a,b,c)	sh_tcsetattr(a,b,c)

#endif /* _terminal_ */
