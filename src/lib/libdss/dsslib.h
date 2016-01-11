/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
 * dss library/method implementation header
 * this header is exported to the method libraries
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _DSSLIB_H
#define _DSSLIB_H	1

#include <ast.h>
#include <ctype.h>
#include <dt.h>
#include <regex.h>
#include <swap.h>
#include <tag.h>

#define Dssdisc_s	Cxdisc_s		/* interchangeable	*/

struct Dssdisc_s;
struct Dssfile_s;
struct Dssrecord_s;

typedef int (*Dssread_f)(struct Dssfile_s*, struct Dssrecord_s*, struct Dssdisc_s*);
typedef int (*Dsswrite_f)(struct Dssfile_s*, struct Dssrecord_s*, struct Dssdisc_s*);
typedef Sfoff_t (*Dssseek_f)(struct Dssfile_s*, Sfoff_t, struct Dssdisc_s*);

#define _DSS_FILE_PRIVATE_ \
	Dssrecord_t		record; \
	Dssfile_t*		next; \
	Dssread_f		readf; \
	Dsswrite_f		writef; \
	Dssseek_f		seekf; \
	size_t			skip; \
	Dssflags_t		ident; \
	void*			data;

#define _DSS_FORMAT_PRIVATE_ \
	Dssformat_t*		next; \
	void*			data;

#define _DSS_METH_PRIVATE_ \
	void*			data; \
	int			reference; \
	int			flags;

#define _DSS_STATE_PRIVATE_ \
	Sfdisc_t		compress_preferred; \
	Dssmeth_t*		global; \
	unsigned int		initialized; \
	unsigned int		scanned;

#include "cxlib.h"
#include "dss.h"

#define DSS_TELL		((Sfoff_t)(-1))

typedef struct Dsstagdisc_s
{
	Tagdisc_t		tagdisc;
	Dssdisc_t*		disc;
	Dssmeth_t*		meth;
} Dsstagdisc_t;

/*
 * library private globals
 */

#define dss_con_beg		_dss_con_beg
#define dss_con_dat		_dss_con_dat

#define dss_map_beg		_dss_map_beg
#define dss_map_dat		_dss_map_dat
#define dss_map_end		_dss_map_end

#define dss_tags		_dss_tags

extern Tags_t	dss_tags[];

#if _BLD_dss && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

/*
 * these are exported for the method libraries
 */

extern Tags_t*		dss_con_beg(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
extern int		dss_con_dat(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
extern int		dss_con_end(Tag_t*, Tagframe_t*, Tagdisc_t*);

extern Tags_t*		dss_map_beg(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
extern int		dss_map_dat(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
extern int		dss_map_end(Tag_t*, Tagframe_t*, Tagdisc_t*);

extern Dssrecord_t*	dss_no_fread(Dssfile_t*, Dssrecord_t*, Dssdisc_t*);
extern int		dss_no_fwrite(Dssfile_t*, Dssrecord_t*, Dssdisc_t*);
extern Sfoff_t		dss_no_fseek(Dssfile_t*, Sfoff_t, Dssdisc_t*);

#undef	extern

#endif
