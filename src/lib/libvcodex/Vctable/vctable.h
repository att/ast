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
#ifndef _VCTABLE_H
#define _VCTABLE_H	1

/* Types and functions related to the table transform
**
** Written by Binh Dao Vo and Kiem-Phong Vo
*/

#define VCTBL_RLE	0001	/* use run-length entropy (default)	*/
#define VCTBL_CEE	0002	/* use conditional entropy		*/
#define VCTBL_LEFT	0010	/* left to right dependency (default) 	*/
#define VCTBL_RIGHT	0020	/* right to left dependency		*/
#define VCTBL_SINGLE	0100	/* single predictor			*/

typedef struct _vctblcolumn_s /* transform specification for each column	*/
{	ssize_t		index;		/* column index			*/
	ssize_t		pred1;		/* <0 if self compressing	*/
	ssize_t		pred2;		/* >=0 if supporting pred1	*/
} Vctblcolumn_t;

typedef struct _vctblplan_s /* transform plan for all columns		*/
{	ssize_t		ncols;		/* # of columns	or row size	*/
	Vctblcolumn_t*	trans;		/* the plan to transform data	*/
	Vcodex_t*	zip;		/* to compress a plan encoding	*/
	ssize_t		train;		/* size of training data	*/
	ssize_t		dtsz;		/* last data size compressed	*/
	ssize_t		cmpsz;		/* and result			*/
	int		good;		/* training deemed good		*/
} Vctblplan_t;

_BEGIN_EXTERNS_
extern Vcmethod_t*	Vctable;	/* fixed-length table transform	*/
extern Vcmethod_t*	Vcrtable;	/* flat file table transform	*/

extern Vctblplan_t*	vctblmakeplan _ARG_((const Void_t*, size_t, size_t, int));
extern void		vctblfreeplan _ARG_((Vctblplan_t*));
extern ssize_t		vctblencodeplan _ARG_((Vctblplan_t*, Void_t**));
extern Vctblplan_t*	vctbldecodeplan _ARG_((Void_t*, size_t));
_END_EXTERNS_

#endif /*_VCTABLE_H*/
