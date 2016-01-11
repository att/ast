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
#ifndef _VCDELTA_H
#define _VCDELTA_H	1

/* Types and functions related to delta compression as defined by
** the IETF RFC3284 Proposed Standard.
**
** Written by Kiem-Phong Vo
*/

/* bits in the delta indicator byte */
#define	VCD_DATACOMPRESS	(1<<0)	/* compressed unmatched data	*/
#define	VCD_INSTCOMPRESS	(1<<1)	/* compressed instructions	*/
#define	VCD_ADDRCOMPRESS	(1<<2)	/* compressed COPY addrs	*/

/* delta instruction types */
#define	VCD_NOOP	0	
#define VCD_ADD		1	/* data immediately follow		*/
#define VCD_RUN		2	/* a run of a single byte		*/
#define VCD_COPY	3	/* copy data from some earlier address	*/
#define VCD_BYTE	4	/* Vcinst_t.mode is the byte encoded	*/

/* Address modes are limited to 16 (VCD_ADDR). Of these, VCD_SELF
   and VCD_HERE are reserved. The remaining modes are s+n < 16 where
   s*256 is the size of "same address" cache and n is the size of the
   "near address" cache.
*/
#define VCD_ADDR	16	/* the maximum number of address modes	*/
#define VCD_SELF	0	/* COPY addr is coded as itself		*/
#define VCD_HERE	1	/* COPY addr is offset from current pos	*/

/* buffer size requirement for encoding/decoding code tables */
#define VCD_TBLSIZE	(6*256 + 64)

typedef struct _vcdinst_s	Vcdinst_t;	/* instruction type	*/
typedef struct _vcdcode_s	Vcdcode_t;	/* a pair of insts	*/
typedef struct _vcdtable_s	Vcdtable_t;	/* entire code table	*/

struct _vcdinst_s
{	Vcchar_t	type;	/* COPY, RUN, ADD, NOOP, BYTE		*/
	Vcchar_t	size;	/* if 0, size coded separately		*/
	Vcchar_t	mode;	/* address mode for COPY		*/
};

struct _vcdcode_s
{	Vcdinst_t	inst1;
	Vcdinst_t	inst2;
};

struct _vcdtable_s
{	Vcchar_t	s_near;		/* size of near address cache	*/
	Vcchar_t	s_same;		/* size of same address cache	*/
	Vcdcode_t	code[256];	/* codes -> instructions	*/
};

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern ssize_t		vcdputtable _ARG_((Vcdtable_t*, Void_t*, size_t));
extern int		vcdgettable _ARG_((Vcdtable_t*, Void_t*, size_t));

#undef	extern

_END_EXTERNS_

#endif /*_VCDELTA_H*/
