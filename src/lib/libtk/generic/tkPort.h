/*
 * tkPort.h --
 *
 *	This header file handles porting issues that occur because of
 *	differences between systems.  It reads in platform specific
 *	portability files.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * SCCS: @(#) tkPort.h 1.7 96/02/11 16:42:10
 */

#ifndef _TKPORT
#define _TKPORT

#if !defined(_WINIX) && (_UWIN || __CYGWIN__ || __EMX__)
#define _WINIX		1
#endif
#if !_WINIX && (defined(__WIN32__) || defined(_WIN32) || defined(WIN32))
#define	WIN_TCL		1
#endif

#ifndef _TK
#include "tk.h"
#endif
#ifndef _TCL
#include "tcl.h"
#endif

#if defined(WIN_TCL)
#   include "tkWinPort.h"
#else
#   if defined(MAC_TCL)
#	include "tkMacPort.h"
#   else
#	include "../unix/tkUnixPort.h"
#   endif
#endif

#endif /* _TKPORT */
