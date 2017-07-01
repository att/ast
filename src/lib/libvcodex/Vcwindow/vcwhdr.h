/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
#ifndef _VCWHDR_H
#define _VCWHDR_H	1

#include	"vchdr.h"

#define NG_BYTE		4
#define H0		(1)
#define	H1		((Grint_t)0x101)
#define	H2		(H1*H1)
#define	H3		(H1*H2)
#define NGINIT(s,g)	(g = H3*s[0] + H2*s[1] + H1*s[2] + H0*s[3] )
#define NGNEXT(s,g)	(g = H1*(g - H3*s[-1]) + s[3])

#define NG_FREQ		(1<<13) /* this is the size of the alphabet	*/
				/* used in counting frequencies. 	*/
#define NGINDEX(g)	(g & (NG_FREQ-1))

#define NG_NSEG		(1<<7)	/* default # of segments per window	*/
#define NG_SIZE		(1<<10)	/* segment size to sum n-gram values	*/
#define NG_BITS		(1<<20)	/* # of bits used per n-gram value	*/
#define NGVALUE(g)	(g & (NG_BITS-1))

/* extra amount to add to windows to enhance matching */
#define VCWEXTRA(s)	((s)/4)

typedef unsigned int	Grint_t; /* n-gram integer value		*/

typedef struct _vcwfile_s
{	Sfio_t*		file;	/* the file to be searched for windows	*/
	Sfoff_t		size;	/* extent of file			*/
	int		done;	/* 0: undone, -1: error, 1: ok		*/

	Grint_t*	work;	/* signatures of a window to be matched	*/
	int		nwork;

	Grint_t*	sig;	/* signatures of each file index	*/
	Grint_t**	ssig;	/* sorted signatures			*/
	int		nsig;	/* number of signatures			*/

	int		nidx;	/* indices of where to search		*/
	int		idx[8];
} Vcwfile_t;

_BEGIN_EXTERNS_
extern int		vcwngfreq _ARG_((size_t*, Vcchar_t*, size_t));
extern double		vcwngmatch _ARG_((int*,size_t*,size_t,Vcchar_t*,size_t,size_t,double));
extern Grint_t		vcwngsig _ARG_((Vcchar_t*, size_t));

extern Vcwfile_t*	vcwfopen _ARG_((Sfio_t*));
extern void		vcwfclose _ARG_((Vcwfile_t*));
extern int		vcwfsearch _ARG_((Vcwfile_t*, Vcchar_t*, size_t));

extern Vcwmethod_t*	Vcwdecode; /* special windowing method for decoding */
_END_EXTERNS_

#endif /*_VCWHDR_H*/
