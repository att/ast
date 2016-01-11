/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2011 AT&T Intellectual Property          *
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
*                             Pat Sullivan                             *
*                                                                      *
***********************************************************************/

/*
 * terminal interface
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H	1
#ifdef _hdr_termios
#   include	<termios.h>
#   ifdef __sgi	/* special hack to eliminate ^M problem */
#	ifndef ECHOCTL
#	    define ECHOCTL	ECHOE
#	endif /* ECHOCTL */
#	ifndef CNSUSP
#	    define CNSUSP	CNSWTCH
#	endif /* CNSUSP */
#   endif /* __sgi */
#else
#   ifdef _sys_termios
#	include	<sys/termios.h>
#	define _hdr_termios
#   endif /* _sys_termios */
#endif /* _hdr_termios */
#if !defined(TCSETS) && !defined(TCSANOW)
#undef	_hdr_termios
#undef	_sys_termios
#endif
#ifdef _hdr_termios
#   ifndef TCSANOW
#	define TCSANOW		TCSETS
#	define TCSADRAIN	TCSETSW
#	define TCSAFLUSH	TCSETSF
#	define tcgetattr(fd,tty)	ioctl(fd, TCGETS, tty)
#	define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#	define cfgetospeed(tp)		((tp)->c_cflag & CBAUD)
#   endif /* TCSANOW */
    /* the following is because of an ultrix bug */
#   if defined(TCSADFLUSH) && !defined(TCSAFLUSH)
#	define TCSAFLUSH	TCSADFLUSH
#   endif
#   undef TIOCGETC
#   undef _hdr_termio
#   undef _sys_termio
#   undef _hdr_sgtty
#   undef _sys_ioctl
#   undef _sys_bsdtty
#else
#   undef OLDTERMIO
#endif /* _hdr_termios */

#ifdef _hdr_termio
#   include	<termio.h>
#else
#   ifdef _sys_termio
#	include	<sys/termio.h>
#   define _hdr_termio 1
#   endif /* _sys_termio */
#endif /* _hdr_termio */
#if !defined(VEOL2) || !defined(TCGETA)
#undef	_hdr_termio
#undef	_sys_termio
#endif
#ifdef _hdr_termio
#   define termios termio
#   undef _hdr_sgtty
#   undef TIOCGETC
#   undef _sys_ioctl
#   define tcgetattr(fd,tty)		ioctl(fd, TCGETA, tty)
#   define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#endif /* _hdr_termio */

#ifdef _sys_bsdtty
#   include	<sys/bsdtty.h>
#endif /* _sys_bsdtty */

#ifdef _hdr_sgtty
#   include	<sgtty.h>
#   ifdef _sys_nttyio
#	ifndef LPENDIN
#	    include	<sys/nttyio.h>
#	endif /* LPENDIN */
#   endif /* _sys_nttyio */
#   define termios sgttyb
#   undef _sys_ioctl
#   ifdef TIOCSETN
#	undef TCSETAW
#   endif /* TIOCSETN */
#   ifdef _SELECT_
#	ifndef included_sys_time
#	    ifdef _sys_time
#		include	<sys/time.h>
#	    endif /* _sys_time */
#	    define included_sys_time
#	endif /* included_sys_time */
	extern const int tty_speeds[];
#   endif /* _SELECT_ */
#   ifdef TIOCGETP
#	define tcgetattr(fd,tty)		ioctl(fd, TIOCGETP, tty)
#	define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#   else
#	define tcgetattr(fd,tty)	gtty(fd, tty)
#	define tcsetattr(fd,action,tty)	stty(fd, tty)
#   endif /* TIOCGETP */
#endif /* _hdr_sgtty */

#ifndef TCSANOW
#   ifdef TCSETAW
#	define TCSANOW	TCSETA
#	ifdef u370
	/* delays are too long, don't wait for output to drain */
#	    define TCSADRAIN	TCSETA
#	else
#	   define TCSADRAIN	TCSETAW
#	endif /* u370 */
#	define TCSAFLUSH	TCSETAF
#   else
#	ifdef TIOCSETN
#	    define TCSANOW	TIOCSETN
#	    define TCSADRAIN	TIOCSETN
#	    define TCSAFLUSH	TIOCSETP
#	endif /* TIOCSETN */
#   endif /* TCSETAW */
#endif /* TCSANOW */
#endif /* _TERMINAL_H */

#ifndef _hdr_termios
#   define cfgetospeed(tp)	((tp)->c_cflag & CBAUD)
#endif /* _hdr_termios */
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
#   ifndef VEOL2
#	define RAWONLY	1
#   endif /* !VEOL2 */
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

#ifdef PROTO
    extern int	tty_alt(int);
    extern void tty_cooked(int);
    extern int	tty_get(int,struct termios*);
    extern int	tty_raw(int);
    extern int	tty_check(int);
#else
    extern int	tty_alt();
    extern void tty_cooked();
    extern int	tty_get();
    extern int	tty_raw();
    extern int	tty_check();
#endif /* PROTO */
extern int	tty_set();
