/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2013 AT&T Intellectual Property          *
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
 * partitioned fixed record zip library private definitions
 */

#ifndef _PZLIB_H
#define _PZLIB_H	1

#include <ast.h>
#include <dt.h>
#include <ctype.h>
#include <error.h>
#include <tv.h>

#define PZ_VERSION_SPLIT	19990811L	/* splitf added		*/

#define state		_pz_info	/* library private state	*/
#define pznospace	_pz_nospace
#define pzsdeflate	_pz_sdeflate
#define pzsinflate	_pz_sinflate
#define pzssplit	_pz_ssplit

#define _(s)		ERROR_translate(0,0,0,s)

struct Pz_s;

typedef struct Pzelt_s			/* window row order element	*/
{
	Dtlink_t	link;		/* dictionary link		*/
	unsigned char	buf[1];		/* row data			*/
} Pzelt_t;

typedef struct Pzrs_s			/* read stream state		*/
{
	size_t		hi;		/* num hi frequency rows in buf	*/
	size_t		lo;		/* rem lo frequency reps in pat	*/
} Pzrs_t;

typedef struct Pzws_s			/* write stream state		*/
{
	unsigned char*	bp;		/* hi freq buf ptr		*/
	unsigned char*	vp;		/* lo freq code val ptr		*/
	unsigned char*	ve;		/* lo freq code val end		*/
	unsigned char*	pb;		/* partial row buffer		*/

	unsigned char*	sp;		/* PZ_SORT partial buffer	*/

	size_t		pc;		/* partial row count		*/
	size_t		rep;		/* lo freq repeat count		*/
	size_t		row;		/* lo freq row count		*/
	size_t		sz;		/* PZ_SORT partial buffer rem	*/

	Pzelt_t*	se;		/* PZ_SORT partial element	*/

	Sfio_t*		io;		/* output sync stream		*/
} Pzws_t;

typedef struct Pzss_s			/* split stream state		*/
{
	char*		match;		/* strmatch() file pattern	*/
	unsigned long	flags;		/* PZ_SPLIT_* flags		*/
	size_t*		data;		/* split data			*/
	size_t		size;		/* split data size		*/
} Pzss_t;

#define	_PZ_PART_PRIVATE_ \
	Dtlink_t	link;		/* dictionary by name link	*/ \
	void*		discdata;	/* discipline specific data	*/

#define _PZ_PRIVATE_ \
	Dtdisc_t	partdisc;	/* partition discipline		*/ \
	Dt_t*		partdict;	/* partition dictionary by name	*/ \
	Pzpart_t*	mainpart;	/* the main partition		*/ \
	Pzpart_t*	freepart;	/* free partition		*/ \
	char*		partname;	/* specific part name		*/ \
	size_t		mrow;		/* max row size			*/ \
	size_t		mwin;		/* max window size		*/ \
	struct								   \
	{								   \
	size_t		count;		/* prefix record count		*/ \
	int		skip;		/* skip prefix on decompress	*/ \
	int		terminator;	/* prefix record terminator	*/ \
	char*		data;		/* prefix data			*/ \
	}		prefix;		/* header prefix info		*/ \
	struct								   \
	{								   \
	Dt_t*		order;		/* order dictionary		*/ \
	Dt_t*		free;		/* free list dictionary		*/ \
	Dtdisc_t	orderdisc;	/* order discipline		*/ \
	Dtdisc_t	freedisc;	/* free list discipline		*/ \
	}		sort;		/* PZ_SORT info			*/ \
	Sfio_t*		oip;		/* original input stream	*/ \
	Sfio_t*		oop;		/* original output stream	*/ \
	Sfio_t*		tmp;		/* temp string stream 		*/ \
	Sfio_t*		str;		/* sfstrtmp() string stream	*/ \
	Sfio_t*		det;		/* processed options stream	*/ \
	Sfio_t*		pin;		/* partition input stream	*/ \
	char*		headoptions;	/* header options		*/ \
	char*		options;	/* current options		*/ \
	Pzrs_t		rs;		/* read stream state		*/ \
	Pzws_t		ws;		/* read stream state		*/ \
	Pzss_t		split;		/* split state			*/ \
	Tv_t		start;		/* elapsed start time		*/ \
	void*		discdata;	/* discipline specific data	*/

#include <pzip.h>

typedef struct Pzdllpz_s		/* initialized pz list for dll	*/
{
	struct Pzdllpz_s*next;		/* next in list			*/
	Pz_t*		pz;		/* pz initialized for dll	*/
} Pzdllpz_t;

typedef struct Pzdll_s			/* loaded dlls			*/
{
	struct Pzdll_s*	next;		/* next in list			*/
	char*		name;		/* name				*/
	void*		dll;		/* handle			*/
	const char*	usage;		/* optget() usage		*/
	Pzinit_f	initf;		/* initialization function	*/
	Pzdllpz_t*	pzs;		/* initialized pz list		*/
} Pzdll_t;

typedef struct
{
	const char*	id;		/* library id			*/
	Pzdll_t*	dll;		/* loaded dll list		*/
} Pzstate_t;

#define PZ_MAJOR	2		/* current format major version	*/
#define PZ_MINOR	1		/* current format minor version	*/

#define PZ_SPLIT_DEFLATE	0x0001	/* split deflate enabled	*/
#define PZ_SPLIT_HEADER		0x0002	/* split header already written	*/
#define PZ_SPLIT_INFLATE	0x0004	/* split inflate enabled	*/
#define PZ_SPLIT_PART		0x0008	/* currently splitting a part	*/

#define PZ_DELAY	0x01000000	/* delay unknown error		*/
#define PZ_HANDLE	0x02000000	/* sfdcpzip() sp arg is Pz_t	*/
#define PZ_UNKNOWN	0x04000000	/* unknown input row size	*/
#define PZ_MAINONLY	0x08000000	/* mainpart only in header	*/
#define PZ_ROWONLY	0x10000000	/* grab row size from prt file	*/
#define PZ_AGAIN	0x20000000	/* path arg is Pz_t		*/
#define PZ_POP		0x40000000	/* pop pz->io on pzclose()	*/
#define PZ_ERROR	0x80000000	/* make pzclose() return error	*/

#define PZ_INTERNAL	(PZ_HANDLE|PZ_UNKNOWN|PZ_MAINONLY|PZ_ROWONLY|PZ_AGAIN|PZ_POP|PZ_ERROR)

#define PZ_MARK_TAIL	1		/* tail marker			*/
#define PZ_MARK_PART	2		/* tail marker			*/

#define PZ_HDR_ARR(x)	((x)>=0x41&&(x)<=0x5a)	/* header array op	*/
#define PZ_HDR_BUF(x)	((x)>=0x61&&(x)<=0x7a)	/* header buffer op	*/

#define PZ_HDR_comment	0x63		/* header comment string	*/
#define PZ_HDR_options	0x64		/* header options string	*/
#define PZ_HDR_fix	0x46		/* header fix array		*/
#define PZ_HDR_grp	0x47		/* header grp array		*/
#define PZ_HDR_map	0x4d		/* header map array		*/
#define PZ_HDR_part	0x50		/* header partition		*/
#define PZ_HDR_prefix	0x70		/* header prefix data		*/
#define PZ_HDR_split	0x53		/* split { major minor }	*/

#define PZGETZ(pz,ip,z,i) \
		if ((z = sfgetc(ip)) & SF_MORE)				   \
		{							   \
			z &= (SF_MORE - 1);				   \
			while ((i = sfgetc(ip)) & SF_MORE)		   \
			{						   \
				if (i < 0)				   \
				{					   \
					if (pz->disc->errorf)		   \
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: input truncated", pz->path); \
					return -1;			   \
				}					   \
				z = (z << SF_UBITS) | (i & (SF_MORE - 1)); \
			}						   \
			z = (z << SF_UBITS) | i;			   \
		}

#define PZGETP(pz,ip,z,i,a) \
		if (!(z = sfgetc(ip)))					   \
			a;						   \
		if (z & SF_MORE)					   \
		{							   \
			z &= (SF_MORE - 1);				   \
			while ((i = sfgetc(ip)) & SF_MORE)		   \
			{						   \
				if (i < 0)				   \
				{					   \
					if (pz->disc->errorf)		   \
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: input truncated", pz->path); \
					return -1;			   \
				}					   \
				z = (z << SF_UBITS) | (i & (SF_MORE - 1)); \
			}						   \
			z = (z << SF_UBITS) | i;			   \
		}

extern Pzstate_t	state;

extern int		pznospace(Pz_t*);
extern int		pzsdeflate(Pz_t*, Sfio_t*);
extern int		pzsinflate(Pz_t*, Sfio_t*);
extern int		pzssplit(Pz_t*);

#endif
