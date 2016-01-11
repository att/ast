/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
#ifndef _VCSFIO_H
#define _VCSFIO_H	1

#if _PACKAGE_ast
#include <vcodex.h>
#endif

/*	FILE I/O.
**
**	Written by Kiem-Phong Vo
*/

#if _SFIO_H /* let Sfio take care of I/O */
#define Vcsfio_t	Sfio_t
#define vcsfread	sfread
#define vcsfwrite	sfwrite
#define vcsfsync	sfsync
#define vcsfclose	sfclose
#else /* above functions will be explicitly provided */
#define Vcsfio_t	Void_t
#endif /*_SFIO_H*/

/* header processing modes */
#define VCSF_VCDIFF	000001	/* output RFC3284 VCDIFF header	*/
#define VCSF_PLAIN	000002	/* no header to be output	*/
#define VCSF_TRANS	000004	/* set trans on VC_DECODE	*/
#define VCSF_FREE	000010	/* free sfdt on disc pop	*/

/* application-defined function to process error messages */
typedef void		(*Vcsferror_f)_ARG_((const char*));

/* data passed to vcsfio() to initialize a stream */
typedef struct _vcsfdata_s
{	int		type;	/* various types of processing	*/
	char*		trans;	/* transformation specification	*/
	char*		window;	/* window specification		*/
	char*		source;	/* source file to delta against	*/
	Vcsferror_f	errorf;	/* to process errors		*/
} Vcsfdata_t;

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Vcsfio_t*	vcsfio _ARG_((Sfio_t*, Vcsfdata_t*, int));

#if !_SFIO_H
extern ssize_t		vcsfread _ARG_((Vcsfio_t*, Void_t*, size_t));
extern ssize_t		vcsfwrite _ARG_((Vcsfio_t*, const Void_t*, size_t));
extern int		vcsfsync _ARG_((Vcsfio_t*));
extern int		vcsfclose _ARG_((Vcsfio_t*));
#endif

#undef	extern

_END_EXTERNS_

#endif /*_VCSFIO_H*/
