/* : : generated from tty by iffe version 2013-11-14 : : */
#ifndef _def_tty_features
#define _def_tty_features 1

#if _mac__POSIX_VDISABLE
#undef _POSIX_VDISABLE
#endif
#include <termios.h>

#undef tcgetattr
#undef tcsetattr
#undef tcgetpgrp
#undef tcsetpgrp
#undef cfgetospeed
#ifndef TCSANOW
#define TCSANOW TCSETS
#define TCSADRAIN TCSETSW
#define TCSAFLUSH TCSETSF
#endif /* TCSANOW */
       /* The following corrects bugs in some implementations */
#if defined(TCSADFLUSH) && !defined(TCSAFLUSH)
#define TCSAFLUSH TCSADFLUSH
#endif /* TCSADFLUSH */
#undef TIOCGETC

/* set ECHOCTL if driver can echo control charaters as ^c */
#ifdef LCTLECH
#ifndef ECHOCTL
#define ECHOCTL LCTLECH
#endif /* !ECHOCTL */
#endif /* LCTLECH */
#ifdef LNEW_CTLECH
#ifndef ECHOCTL
#define ECHOCTL LNEW_CTLECH
#endif /* !ECHOCTL */
#endif /* LNEW_CTLECH */
#ifdef LNEW_PENDIN
#ifndef PENDIN
#define PENDIN LNEW_PENDIN
#endif /* !PENDIN */
#endif /* LNEW_PENDIN */

#endif
