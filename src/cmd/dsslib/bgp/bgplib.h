/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * bgp library implementation header
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _BGPLIB_H
#define _BGPLIB_H	1

#include <dsslib.h>
#include <tm.h>
#include <align.h>

#include "bgp.h"

#ifndef truncof
#define truncof(x,y)		((x)&~((y)-1))
#endif

#define BGPFILE(f)		((Bgp_t*)(f)->dss->data)
#define BGPDATA(p)		BGPFILE(DSSRECORD(p)->file)

#define BGP_FIXED		offsetof(Bgproute_t,data)
#define BGP_PREFIX_FIXED	(offsetof(Bgproute_t,prefixv6)-offsetof(Bgproute_t,agg_addr32))

typedef struct Prefix_s			/*  ip prefix			*/
{
	uint32_t	addr;
	unsigned char	bits;		
} Prefix_t;

typedef struct Bgp_s			/* method handle		*/
{
	Sfio_t*		tmp;
	Cxtype_t*	type_as16path;
	Cxtype_t*	type_as32path;
	Cxtype_t*	type_cluster;
	Cxtype_t*	type_community;
	Cxtype_t*	type_extended;
	Cxtype_t*	type_ipv4addr;
	Cxtype_t*	type_ipv4prefix;
	Cxtype_t*	type_ipv6addr;
	Cxtype_t*	type_ipv6prefix;
	Cxtype_t*	type_label;
} Bgp_t;

#define BGP_METHOD_ANONYMIZE		0x0001

#define BGPVEC(s,r,t,p,m,v,e,d) \
	do { \
		if (m > (v)->maxsize) \
		{ \
			int	n; \
			(s)->size = roundof((s)->size, sizeof(t)); \
			n = ((s)->size >= (s)->temp) ? \
				0 : \
				((s)->temp - (s)->size) / sizeof(t); \
			if (m > n) \
			{ \
				if ((d)->errorf) \
					(*(d)->errorf)(NiL, d, 1, "%s length %d truncated to %d", e, m, n); \
				m = n; \
			} \
			(v)->offset = (s)->size; \
			(v)->maxsize = m; \
			(s)->size += m * sizeof(t); \
		} \
		(v)->size = m; \
		p = (t*)((r)->data+(v)->offset); \
	} while (0)

#define BGPPERM(s,r,t,p,m,x,e,d) \
	do { \
		int	a; \
		int	n; \
		a = roundof((s)->size, sizeof(t)); \
		n = m * sizeof(t) + x; \
		if ((a + n) <= (s)->temp) \
		{ \
			(s)->size = a + n; \
			memset((r)->data + a, 0, n); \
			p = (t*)((r)->data + a); \
		} \
		else \
		{ \
			if ((d)->errorf) \
				(*(d)->errorf)(NiL, d, 1, "out of space for %s size %d", e, n); \
			p = 0; \
		} \
	} while (0)

#define BGPTEMP(s,r,t,p,m,x,e,d) \
	do { \
		int	a; \
		int	n; \
		n = m * sizeof(t) + x; \
		if ((a = (int)(s)->temp - n) >= 0) \
		{ \
			(s)->temp = truncof(a, ALIGN_BOUND1); \
			memset((r)->data + (s)->temp, 0, n); \
			p = (t*)((r)->data + (s)->temp); \
		} \
		else \
		{ \
			if ((d)->errorf) \
				(*(d)->errorf)(NiL, d, 1, "out of space for %s size %d", e, n); \
			p = 0; \
		} \
	} while (0)

#define bgp_first_format	(&bgp_fixed_format)
#define bgp_cisco_next		(&bgp_ciscov6_format)
#define bgp_ciscov6_next	(&bgp_mrt_anonymize_format)
#define bgp_fixed_next		(&bgp_cisco_format)
#define bgp_mrt_anonymize_next	(&bgp_mrt_format)
#define bgp_mrt_next		(&bgp_ipma_format)
#define bgp_ipma_next		(&bgp_table_format)
#define bgp_table_next		0

#define bgp_method		_dss_bgp_method
#define bgp_formats		_dss_bgp_formats
#define bgp_cisco_format	_dss_bgp_cisco_format
#define bgp_ciscov6_format	_dss_bgp_ciscov6_format
#define bgp_fixed_format	_dss_bgp_fixed_format
#define bgp_mrt_anonymize_format _dss_bgp_mrt_anonymize_format
#define bgp_mrt_format		_dss_bgp_mrt_format
#define bgp_ipma_format		_dss_bgp_ipma_format
#define bgp_table_format	_dss_bgp_table_format

extern Dssformat_t*		bgp_formats;
extern Dssformat_t		bgp_cisco_format;
extern Dssformat_t		bgp_ciscov6_format;
extern Dssformat_t		bgp_fixed_format;
extern Dssformat_t		bgp_mrt_format;
extern Dssformat_t		bgp_mrt_anonymize_format;
extern Dssformat_t		bgp_ipma_format;
extern Dssformat_t		bgp_table_format;

#endif
