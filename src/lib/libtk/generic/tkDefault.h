/*
 * default.h --
 *
 *	This file defines the defaults for all options for all of
 *	the Tk widgets.
 *
 * Copyright (c) 1991-1994 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * SCCS: @(#) default.h 1.4 96/02/07 17:33:39
 */

#ifndef _TKDEFAULT
#define _TKDEFAULT

#if !defined(_WINIX) && (_UWIN || __CYGWIN__ || __EMX__)
#define _WINIX		1
#endif
#if !_WINIX && (defined(__WIN32__) || defined(_WIN32) || defined(WIN32))
#define	WIN_TCL		1
#endif

#if defined(WIN_TCL)
#   include "tkWinDefault.h"
#else
#   if defined(MAC_TCL)
#	include "tkMacDefault.h"
#   else
#	include "tkUnixDef.h"
#   endif
#endif

#endif /* _TKDEFAULT */
