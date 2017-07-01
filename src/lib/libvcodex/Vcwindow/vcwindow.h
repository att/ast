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
#ifndef _VCWINDOW_H
#define _VCWINDOW_H	1

/* Window matching algorithms for delta compression of large files.
**
** Written by Kiem-Phong Vo
*/

typedef struct _vcwmatch_s	Vcwmatch_t;
typedef struct _vcwmethod_s	Vcwmethod_t;
typedef struct _vcwdisc_s	Vcwdisc_t;
typedef struct _vcwindow_s	Vcwindow_t;
typedef int	(*Vcwevent_f)_ARG_((Vcwindow_t*, int, Void_t*, Vcwdisc_t*));

struct _vcwmatch_s
{	int		type;	/* VCD_[SOURCE|TARGET]FILE		*/
	Sfoff_t		wpos;	/* position in file			*/
	ssize_t		wsize;	/* size of matching window		*/
	Void_t*		wdata;	/* window data				*/
	ssize_t		msize;	/* amount of data actually matched	*/
	int		more;	/* more subwindows to process		*/
};

struct _vcwmethod_s
{	Vcwmatch_t*	(*applyf)_ARG_((Vcwindow_t*, Void_t*, size_t, Sfoff_t));
	int		(*eventf)_ARG_((Vcwindow_t*, int));
	char*		name;
	char*		desc;
	char*		about;
}; 

struct _vcwdisc_s
{	Sfio_t*		srcf;	/* source file				*/
	Sfio_t*		tarf;	/* target file if any			*/
	Vcwevent_f	eventf;
};

struct _vcwindow_s
{	Vcwmethod_t*	meth;
	Vcwdisc_t*	disc;
	ssize_t		cmpsz;	/* size of result of last comp. attempt	*/
	Vcwmatch_t	match;	/* space to return the matching window	*/
	Void_t*		mtdata;
};

/* window events */
#define VCW_OPENING	0
#define VCW_CLOSING	1

#define vcwfeedback(vcw, sz)	((vcw)->cmpsz = (sz))
#define vcwapply(vcw, dt, dtsz, p) \
			(*(vcw)->meth->applyf)((vcw), (dt), (dtsz), (p))

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_vcodex && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Vcwmethod_t*	Vcwmirror;
extern Vcwmethod_t*	Vcwvote;
extern Vcwmethod_t*	Vcwprefix;

#undef	extern

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Vcwindow_t*	vcwopen _ARG_((Vcwdisc_t*, Vcwmethod_t*));
extern int		vcwclose _ARG_((Vcwindow_t*));
extern Vcwmethod_t*	vcwgetmeth _ARG_((char*));
extern int		vcwwalkmeth _ARG_((Vcwalk_f, Void_t*));

#undef	extern

_END_EXTERNS_

#endif /*_VCWINDOW_H*/
