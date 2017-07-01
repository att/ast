/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
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
 * C expression library private definitions
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _CX_PRIVATE_

struct Cxtable_s;

struct Cxbuf_s; typedef struct Cxbuf_s Cxbuf_t;
struct Cxcompile_s; typedef struct Cxcompile_s Cxcompile_t;
struct Cxdone_s; typedef struct Cxdone_s Cxdone_t;
struct Cxinclude_s; typedef struct Cxinclude_s Cxinclude_t;

#define _CX_PRIVATE_ \
	Cxexpr_t*		expr; \
	struct Cxinclude_s*	include; \
	struct Cxtable_s*	table; \
	Sfio_t*			op; \
	regdisc_t		redisc; \
	Cxcallout_f		deletef; \
	Cxcallout_f		getf; \
	Cxcallout_f		returnf; \
	Cxcallout_f		referencef; \
	Cxbuf_t*		top; \
	Cxbuf_t*		cvt; \
	char*			ccbuf; \
	char*			cvtbuf; \
	size_t			ccsiz; \
	size_t			cvtsiz; \
	int			evaluating; \
	int			index; \
	int			jump; \
	int			scoped; \
	int			view; \
	int			width; \
	Sfio_t*			tp;

#define _CX_CONSTRAINT_PRIVATE_ \
	regex_t*		re;

#define _CX_EDIT_PRIVATE_ \
	regex_t			re; \
	regdisc_t		redisc;

#define _CX_EXPR_PRIVATE_ \
	char**			files; \
	Vmalloc_t*		vm; \
	Cxdone_t*		done; \
	struct Cxoperand_s*	stack; \
	unsigned int		stacksize; \
	int			begun; \
	int			reclaim;

#define _CX_ITEM_PRIVATE_ \
	Dtlink_t		str2num; \
	Dtlink_t		num2str;

#define _CX_QUERY_PRIVATE_ \
	int			head; \
	Cxquery_t*		next; \
	Cxinstruction_t*	prog;

#define _CX_STATE_PRIVATE_ \
	Cxheader_t*		header; \
	unsigned int		initialized; \
	Dtdisc_t		codedisc; \
	Dtdisc_t		listdisc; \
	Dtdisc_t		namedisc; \
	unsigned char		ctype[UCHAR_MAX+1];

#include <ast.h>
#include <regex.h>

#include "cx.h"

#include <ctype.h>
#include <error.h>

#define CX_CVT		64

#define CX_PAREN	(((CX_OPERATORS>>CX_ATTR)+1)<<CX_ATTR)
#define CX_INTERNALS	(((CX_OPERATORS>>CX_ATTR)+2)<<CX_ATTR)

#define CX_ADDADD	(CX_ADD|CX_X2)
#define CX_SUBSUB	(CX_SUB|CX_X2)

#define CX_SCOPE	(CX_FLAGS>>1)

#define CX_VIEW_callouts	(1<<0)
#define CX_VIEW_constraints	(1<<1)
#define CX_VIEW_edits		(1<<2)
#define CX_VIEW_fields		(1<<3)
#define CX_VIEW_maps		(1<<4)
#define CX_VIEW_queries		(1<<5)
#define CX_VIEW_recodes		(1<<6)
#define CX_VIEW_types		(1<<7)
#define CX_VIEW_variables	(1<<8)

#define CXH			CX_HEADER_INIT
#define CXS			CX_SCHEMA_INIT
#define CXC(o,l,r,f,d)		CX_CALLOUT_INIT(o,l,r,f,d)
#define CXF(n,t,f,p,d)		CX_FUNCTION_INIT(n,t,f,p,d)
#define CXR(o,l,r,f,d)		CX_RECODE_INIT(o,l,r,f,d)
#define CXT(n,b,e,i,m,d)	CX_TYPE_INIT(n,b,e,i,m,d)
#define CXV(n,t,i,d)		CX_VARIABLE_INIT(n,t,i,d)

struct Cxbuf_s
{
	Cxbuf_t*		next;
	char*			data;
	size_t			size;
};

struct Cxcompile_s
{
	Cxcompile_t*		next;
	Cx_t*			cx;
	int			balanced;
	int			collecting;
	int			paren;
	int			reclaim;
	unsigned int		level;
	unsigned int		depth;
	unsigned int		pp;
	unsigned int		stacksize;
	Sfio_t*			ip;
	Sfio_t*			tp;
	Sfio_t*			xp;
	Vmalloc_t*		vm;
	Cxdone_t*		done;
	Cxtype_t*		type;
	struct Cxoperand_s*	stack;
};

struct Cxdone_s
{
	Cxdone_t*		next;
	Cxdone_f		donef;
	void*			data;
};

struct Cxinclude_s
{
	Cxinclude_t*		pop;
	Sfio_t*			sp;
	char*			base;
	char*			next;
	char*			last;
	char*			ofile;
	int			eof;
	int			final;
	int			head;
	int			interactive;
	int			newline;
	int			oline;
	int			prompt;
	int			retain;
	char			file[1];
};

typedef struct Cxtable_s
{
	unsigned char		opcode[UCHAR_MAX];
	unsigned char		comparison[CX_INTERNALS];
	unsigned char		logical[CX_INTERNALS];
	unsigned char		precedence[CX_INTERNALS];
} Cxtable_t;

extern int		cxinitmap(Cxmap_t*, Cxdisc_t*);

#endif
