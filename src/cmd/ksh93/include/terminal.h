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
#ifndef _TERMINAL_H
#define _TERMINAL_H 1

//
// Terminal interface is complicated by the fact that there are so many variations. This will use
// POSIX <termios.h> interface where available.
//
#include <termios.h>

#undef tcgetattr
#undef tcsetattr
// The following corrects bugs in some implementations.
#if defined(TCSADFLUSH) && !defined(TCSAFLUSH)
#define TCSAFLUSH TCSADFLUSH
#endif  // TCSADFLUSH
#undef TIOCGETC

// Set ECHOCTL if driver can echo control charaters as ^c.
#ifdef LCTLECH
#ifndef ECHOCTL
#define ECHOCTL LCTLECH
#endif  // !ECHOCTL
#endif  // LCTLECH
#ifdef LNEW_CTLECH
#ifndef ECHOCTL
#define ECHOCTL LNEW_CTLECH
#endif  // !ECHOCTL
#endif  // LNEW_CTLECH
#ifdef LNEW_PENDIN
#ifndef PENDIN
#define PENDIN LNEW_PENDIN
#endif  // !PENDIN
#endif  // LNEW_PENDIN
#ifndef ECHOCTL
#ifndef VEOL
#endif  // !VEOL
#endif  // !ECHOCTL

#if _hdr_sys_filio
#include <sys/filio.h>
#endif  // _sys_filio

// This symbol is used by the CLI editor modes to signal an invalid character.
// Why it is U+DFFF (the last char in the low-surrogate range) is a mystery.
// There are other Unicode codepoints that would seem more appropriate.
// For example, U+FFFF or U+FFFE.
#define MARKER 0xdfff  // must be an invalid character

extern void tty_cooked(int);
extern int tty_get(int, struct termios *);
extern int tty_raw(int, int);
extern int tty_check(int);
extern int tty_set(int, int, struct termios *);

extern int sh_tcgetattr(int, struct termios *);
extern int sh_tcsetattr(int, int, struct termios *);
#define tcgetattr(a, b) sh_tcgetattr(a, b)
#define tcsetattr(a, b, c) sh_tcsetattr(a, b, c)

#endif  // _TERMINAL_H
