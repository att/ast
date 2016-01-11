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
#ifndef _VCDHDR_H
#define _VCDHDR_H	1

#include	"vchdr.h"
#include	"vcdelta.h"

#define COPYMIN		(4)	/* min size for a COPY 		*/

/* assumed max sizes for various instruction types. The current Vcdiff
   implementation only allows add+copy and copy+add instructions.
*/
#define VCD_ADDMAX	256	/* absolute max for a single ADD	*/
#define VCD_COPYMAX	256	/* absolute max for a single COPY	*/
#define VCD_ADDCOPY	16	/* absolute max for a merged ADD	*/
#define VCD_COPYADD	16	/* absolute max for a merged COPY	*/

typedef struct _vcdiff_s	Vcdiff_t;

typedef struct _vcdcache_s	Vcdcache_t;
typedef struct _vcdsave_s	Vcdsave_t;

typedef struct _vcdsize_s	Vcdsize_t;
typedef struct _vcdindex_s	Vcdindex_t;

/* Tables used to compute merged ADD+COPY and COPY+ADD instructions.  */
struct _vcdsize_s
{	ssize_t		add;			/* max single ADD size	*/
	ssize_t		copy[VCD_ADDR];		/* max single COPY size	*/
	ssize_t		add1[VCD_ADDR];		/* max ADD size in A+C	*/
	ssize_t		copy2[VCD_ADDR];	/* max COPY size in A+C	*/
	ssize_t		copy1[VCD_ADDR];	/* max COPY size in C+A	*/
	ssize_t		add2[VCD_ADDR];		/* max ADD size in C+A	*/
};
struct _vcdindex_s
{	Vcchar_t	add[VCD_ADDMAX];
	Vcchar_t	copy[VCD_ADDR][VCD_COPYMAX];
	Vcchar_t	addcopy[VCD_ADDCOPY][VCD_ADDR][VCD_COPYADD];
	Vcchar_t	copyadd[VCD_ADDR][VCD_COPYADD][VCD_ADDCOPY];
};

/* address caches to code COPY addresses */
struct _vcdcache_s
{	ssize_t		s_near;		/* size of near cache		*/
	ssize_t*	c_near;		/* near address cache		*/
	ssize_t		n;		/* index in near cache		*/
	ssize_t		s_same;		/* size of same cache		*/
	ssize_t*	c_same;		/* exact match cache		*/
};

/* to store a saved instruction */
struct _vcdsave_s
{	ssize_t		dtsz;		/* size of ADD or COPY		*/
	ssize_t		addr;		/* the address to code		*/
	ssize_t		mode;		/* address encoding mode	*/
};

/* the Vcdiff/undiff handle */
struct _vcdiff_s
{	Vclzparse_t	vcpa;

	Vcdcache_t*	cache;		/* address caches		*/
	Vcdtable_t*	table;		/* code table for compression	*/
	Vcdsave_t*	save;		/* saved instruction		*/

	Vcdsize_t*	size;
	Vcdindex_t*	index;

	Vcio_t*		data;		/* unmatched data		*/
	Vcio_t*		inst;		/* instruction data		*/
	Vcio_t*		addr;		/* COPY address data		*/

	unsigned int	flags;		/* control flags		*/
};

#define VCDINIT(vcd) \
	( (vcd)->cache = NIL(Vcdcache_t*), \
	  (vcd)->table = NIL(Vcdtable_t*), \
	  (vcd)->save = NIL(Vcdsave_t*), \
	  (vcd)->size = NIL(Vcdsize_t*), \
	  (vcd)->index = NIL(Vcdindex_t*), \
	  (vcd)->data = NIL(Vcio_t*), \
	  (vcd)->inst = NIL(Vcio_t*), \
	  (vcd)->addr = NIL(Vcio_t*) \
	)

_BEGIN_EXTERNS_ /* private functions */
extern Vcdsize_t	_Vcdsize;
extern Vcdindex_t	_Vcdindex;
extern Vcdtable_t*	_Vcdtbl;

extern Vcdcache_t*	vcdkaopen _ARG_((ssize_t, ssize_t));
extern void		vcdkaclear _ARG_((Vcdcache_t*));
extern void		vcdkaclose _ARG_((Vcdcache_t*));
extern ssize_t		vcdkasetaddr _ARG_((Vcdcache_t*, ssize_t, ssize_t, ssize_t*));
extern ssize_t		vcdkagetaddr _ARG_((Vcdcache_t*, Vcio_t*, ssize_t, ssize_t));

extern Vcdtable_t*	vcdbmtable();
extern int		vcdbmputinst _ARG_((Vclzparse_t*, ssize_t, ssize_t, ssize_t, ssize_t, int));

extern void		_vcdtblinit _ARG_((void));
_END_EXTERNS_

#endif /*_VDELHDR_H*/
