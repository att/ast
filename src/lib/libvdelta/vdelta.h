/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#ifndef _VDELTA_H
#define _VDELTA_H	1

/*	Header for the vdelta library
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

/* standardize conventions */
#ifndef __KPV__
#define __KPV__		1

/* The symbol __STD_C indicates that the language is ANSI-C or C++ */
#ifndef __STD_C
#ifdef __STDC__
#define	__STD_C		1
#else
#if __cplusplus || c_plusplus
#define __STD_C		1
#else
#define __STD_C		0
#endif /*__cplusplus*/
#endif /*__STDC__*/
#endif /*__STD_C*/

/* For C++, extern symbols must be protected against name mangling */
#ifndef _BEGIN_EXTERNS_
#if __cplusplus || c_plusplus
#define _BEGIN_EXTERNS_	extern "C" {
#define _END_EXTERNS_	}
#else
#define _BEGIN_EXTERNS_
#define _END_EXTERNS_
#endif
#endif /*_BEGIN_EXTERNS_*/

/* _ARG_ simplifies function prototypes between K&R-C and more modern Cs */
#ifndef _ARG_
#if __STD_C
#define _ARG_(x)	x
#else
#define _ARG_(x)	()
#endif
#endif /*_ARG_*/

/* The type Void_t is properly defined so that Void_t* can address any type */
#ifndef Void_t
#if __STD_C
#define Void_t		void
#else
#define Void_t		char
#endif
#endif /*Void_t*/

/* The below are for DLLs on systems such as WINDOWS that only
** allows pointers across client ** and library code.
*/
#ifndef _PTR_
#if  _DLL_INDIRECT_DATA && !defined(_DLL)	/* building client code		*/
#define _ADR_ 		/* cannot export whole structs - data access via ptrs	*/
#define _PTR_	*
#else			/* library code or a normal system			*/
#define _ADR_	&	/* exporting whole struct is ok				*/
#define _PTR_ 
#endif
#endif /*_PTR_*/

#endif /*__KPV__*/

/* user-supplied functions to do io */
typedef struct _vddisc_s	Vddisc_t;
typedef int(*	Vdio_f)_ARG_((Void_t*, int, long, Vddisc_t*));
struct _vddisc_s
{	long	size;		/* total data size	*/
	Void_t*	data;		/* data array		*/
	Vdio_f	readf;		/* to read data		*/
	Vdio_f	writef;		/* to write data	*/
};

/* magic header for delta output */
#define VD_MAGIC	"\026\004\000\002"
#define VD_MAGIC_OLD	"vd02"

_BEGIN_EXTERNS_
extern long	vddelta _ARG_((Vddisc_t*,Vddisc_t*,Vddisc_t*));
extern long	vdupdate _ARG_((Vddisc_t*,Vddisc_t*,Vddisc_t*));
extern int	vdsqueeze _ARG_((Void_t*, int, Void_t*));
extern int	vdexpand _ARG_((Void_t*, int, Void_t*));
_END_EXTERNS_

#endif /*_VDELTA_H*/
