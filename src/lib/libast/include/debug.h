/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * common ast debug definitions
 * include after the ast headers
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <ast.h>
#include <error.h>

#if !defined(DEBUG) && _BLD_DEBUG
#define DEBUG		_BLD_DEBUG
#endif

#if DEBUG || _BLD_DEBUG

#define debug(x)	x
#define message(x)	do if (error_info.trace < 0) { error x; } while (0)
#define messagef(x)	do if (error_info.trace < 0) { errorf x; } while (0)

#define DEBUG_BEGTIME()		debug_elapsed(1)
#define DEBUG_GETTIME()		debug_elapsed(0)
#define DEBUG_ASSERT(p)		((p) ? 0 : (debug_fatal(__FILE__, __LINE__, #p),0))
#define DEBUG_COUNT(n)		((n) += 1)
#define DEBUG_TALLY(c,n,v)	((c) ? ((n) += (v)) : (n))
#define DEBUG_INCREASE(n)	((n) += 1)
#define DEBUG_DECREASE(n)	((n) -= 1)
#define DEBUG_DECLARE(t,v)	t v;
#define DEBUG_SET(n,v)		((n) = (v))
#define DEBUG_MESSAGE(m)	debug_printf(2,"%s:%d: %s\n",__FILE__,__LINE__,m)
#define DEBUG_PRINT(fd,s,v)	debug_printf((fd),(s),(v))
#define DEBUG_WRITE(fd,d,n)	write((fd),(d),(n))
#define DEBUG_TEMP(temp)	(temp) /* debugging stuff that should be removed */
#define DEBUG_BREAK		break
#define DEBUG_CONTINUE		continue
#define DEBUG_GOTO(label)	do { debug_printf(2, "%s:%d: goto %s\n", __FILE__, __LINE__, #label); goto label; } while(0)
#define DEBUG_RETURN(x)		do { debug_printf(2, "%s:%d: return %s\n", __FILE__, __LINE__, #x); return(x); } while(0)

#else

#define debug(x)
#define message(x)
#define messagef(x)

#define DEBUG_BEGTIME()
#define DEBUG_GETTIME()
#define DEBUG_ASSERT(p)
#define DEBUG_COUNT(n)
#define DEBUG_TALLY(c,n,v)
#define DEBUG_INCREASE(n)
#define DEBUG_DECREASE(n)
#define DEBUG_DECLARE(t,v)
#define DEBUG_SET(n,v)
#define DEBUG_MESSAGE(s)
#define DEBUG_PRINT(fd,s,v)
#define DEBUG_WRITE(fd,d,n)
#define DEBUG_TEMP(x)
#define DEBUG_BREAK		break
#define DEBUG_CONTINUE		continue
#define DEBUG_GOTO(label)	goto label
#define DEBUG_RETURN(x)		return(x)

#endif

#ifndef BREAK
#define BREAK			DEBUG_BREAK
#endif
#ifndef CONTINUE
#define CONTINUE		DEBUG_CONTINUE
#endif
#ifndef GOTO
#define GOTO(label)		DEBUG_GOTO(label)
#endif
#ifndef RETURN
#define RETURN(x)		DEBUG_RETURN(x)
#endif

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern double		debug_elapsed(int);
extern void		debug_indent(int);
extern void		debug_fatal(const char*, int, const char*);
#if 0 /* this prototype in <ast.h> for convenience */
extern ssize_t		debug_printf(int, const char*, ...);
#endif
extern ssize_t		debug_sprintf(char*, size_t, const char*, ...);
extern ssize_t		debug_vsprintf(char*, size_t, const char*, va_list);
extern ssize_t		debug_vprintf(int, const char*, va_list);
extern void		systrace(const char*);

#undef	extern

#endif
