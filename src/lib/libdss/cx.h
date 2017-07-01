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
 * C expression library interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _CX_H
#define _CX_H

#include <ast.h>
#include <math.h>
#include <dt.h>
#include <vmalloc.h>

#define CX_ID		"cx"
#define CX_VERSION	20110811L

/* Cx_t.flags */

#define CX_DEBUG	(1<<0)			/* debug trace		*/
#define CX_QUIET	(1<<1)			/* no non-fatal messages*/
#define CX_REGRESS	(1<<2)			/* regression output	*/
#define CX_TRACE	(1<<3)			/* algorithm trace	*/
#define CX_VALIDATE	(1<<4)			/* validate constraints	*/
#define CX_VERBOSE	(1<<5)			/* verbose feedback	*/

#define CX_BALANCED	(1<<6)			/* cx input () balanced	*/
#define CX_INCLUDE	(1<<7)			/* include cxpush()	*/
#define CX_FLAGS	(1L<<8)			/* first caller flag	*/

/* _CX_HEADER_.flags */

#define CX_NORMALIZED	0x00000001
#define CX_INITIALIZED	0x00000002
#define CX_REFERENCED	0x00000004
#define CX_DEPRECATED	0x00000008
#define CX_VISITED	0x00000010
#define CX_IGNORECASE	0x00000020
#define CX_SCHEMA	0x00000040

/* Cxmember_t.flags */

#define CX_VIRTUAL	0x0001

/* Cxpart_t.flags */

#define CX_ALL		0x0001

/* Cxtype_t.representation */

#define CX_void		0
#define CX_number	1
#define CX_string	2
#define CX_pointer	3
#define CX_reference	4
#define CX_buffer	5		/* allocated separately		*/
#define CX_type		6
#define CX_bool		7

/* Cxformat_t.flags */

#define CX_STRING	0x0001
#define CX_INTEGER	0x0002
#define CX_UNSIGNED	0x0004
#define CX_FLOAT	0x0008
#define CX_BUFFER	0x0010
#define CX_BINARY	0x0020
#define CX_MULTIPLE	0x0100
#define CX_FREE		0x0200
#define CX_NUL		0x0400
#define CX_VARIABLE	0x0800
#define CX_QUOTEALL	0x1000
#define CX_LONG		0x2000

#define CX_ASSIGN	01
#define CX_X2		02
#define CX_UNARY	04
#define CX_ATTR		3

#define CX_OPNAME	"0+&/>~<~%*!|-^CDeGLnpRJSst"

#define CX_NOP		(( 0))

#define CX_ADD		(( 1<<CX_ATTR))
#define CX_AND		(( 2<<CX_ATTR))
#define CX_DIV		(( 3<<CX_ATTR))
#define CX_GT		(( 4<<CX_ATTR))
#define CX_INV		(( 5<<CX_ATTR)|CX_UNARY)
#define CX_LT		(( 6<<CX_ATTR))
#define CX_MAT		(( 7<<CX_ATTR))
#define CX_MOD		(( 8<<CX_ATTR))
#define CX_MPY		(( 9<<CX_ATTR))
#define CX_NOT		((10<<CX_ATTR)|CX_UNARY)
#define CX_OR		((11<<CX_ATTR))
#define CX_SUB		((12<<CX_ATTR))
#define CX_XOR		((13<<CX_ATTR))

#define CX_CALL		((14<<CX_ATTR)|CX_UNARY)
#define CX_DEL		((15<<CX_ATTR))
#define CX_END		((16<<CX_ATTR))
#define CX_GET		((17<<CX_ATTR)|CX_UNARY)
#define CX_LOG		((18<<CX_ATTR)|CX_UNARY)
#define CX_NUM		((19<<CX_ATTR))
#define CX_POP		((20<<CX_ATTR))
#define CX_REF		(CX_AND|CX_UNARY)
#define CX_RET		((21<<CX_ATTR)|CX_UNARY)
#define CX_SC0		((22<<CX_ATTR))
#define CX_SC1		(CX_SC0|CX_X2)
#define CX_SET		((23<<CX_ATTR)|CX_ASSIGN)
#define CX_STR		((24<<CX_ATTR))
#define CX_TST		((25<<CX_ATTR))
#define CX_OPERATORS	((26<<CX_ATTR))

#define CX_CAST		(CX_SET|CX_UNARY)
#define CX_EQ		(CX_SET|CX_X2)
#define CX_ANDAND	(CX_AND|CX_X2)
#define CX_OROR		(CX_OR|CX_X2)
#define CX_LSH		(CX_LT|CX_X2)
#define CX_RSH		(CX_GT|CX_X2)
#define CX_UPLUS	(CX_ADD|CX_UNARY)
#define CX_UMINUS	(CX_SUB|CX_UNARY)

#define CX_MATCH	(CX_MAT|CX_ASSIGN)
#define CX_NOMATCH	(CX_MAT)

#define CX_GE		(CX_GT|CX_ASSIGN)
#define CX_LE		(CX_LT|CX_ASSIGN)
#define CX_NE		((CX_NOT&~CX_UNARY)|CX_ASSIGN)

#define CX_CTYPE_ALPHA	(1<<0)
#define CX_CTYPE_DIGIT	(1<<1)
#define CX_CTYPE_FLOAT	(1<<2)
#define CX_CTYPE_SPACE	(1<<3)

#define CXMIN(a,b)	(((a)<(b))?(a):(b))
#define CXMAX(a,b)	(((a)>(b))?(a):(b))

#define CXINTEGER(n)	((Cxinteger_t)(n))
#define CXUNSIGNED(n)	((Cxunsigned_t)CXINTEGER(n))

#define CXDETAILS(d,f,t,v) \
	((d)?(d):((d)=(f)&&(f)->details?(f)->details:(t)->format.details?(t)->format.details:(v)))

#define CX_HEADER_INIT	{{0},0}
#define CX_SCHEMA_INIT	{{0},CX_SCHEMA}
#define CX_CALLOUT_INIT(op,type1,type2,callout,description) \
	{0,description,CX_HEADER_INIT,op,(Cxtype_t*)type1,(Cxtype_t*)type2,callout},
#define CX_FUNCTION_INIT(name,type,function,prototype,description) \
	{name,description,CX_HEADER_INIT,function,(Cxtype_t*)type,prototype},
#define CX_RECODE_INIT(op,type1,type2,recode,description) \
	{0,description,CX_HEADER_INIT,op,(Cxtype_t*)type1,(Cxtype_t*)type2,recode},
#define CX_TYPE_INIT(name,base,external,internal,match,description) \
	{name,description,CX_HEADER_INIT,(Cxtype_t*)base,0,external,internal,0,0,0,0,CX_HEADER_INIT,match},
#define CX_VARIABLE_INIT(name,type,index,description) \
	{name,description,CX_HEADER_INIT,0,(Cxtype_t*)type,0,index},

#define cxrepresentation(t)	((t)->representation)
#define cxisbool(t)		(cxrepresentation(t)==CX_bool)
#define cxisbuffer(t)		(cxrepresentation(t)==CX_buffer)
#define cxislogical(t)		(cxrepresentation(t)==CX_bool||cxrepresentation(t)==CX_number)
#define cxisnumber(t)		(cxrepresentation(t)==CX_number)
#define cxisstring(t)		(cxrepresentation(t)==CX_string)
#define cxisvoid(t)		(cxrepresentation(t)==CX_void)

#define cxsize(t,v)	((cxisstring(t)||cxisbuffer(t))?((v)->buffer.size):0)

#if ! _ast_fltmax_double
#undef	strtod
#define strtod(a,b)	strtold(a,b)
#undef	strntod
#define strntod(a,b,c)	strntold(a,b,c)
#endif

typedef _ast_fltmax_t Cxnumber_t;
typedef      uint32_t Cxflags_t;
typedef      intmax_t Cxinteger_t;
typedef     uintmax_t Cxunsigned_t;

struct Cx_s; typedef struct Cx_s Cx_t;
struct Cxarray_s; typedef struct Cxarray_s Cxarray_t;
struct Cxconstraint_s; typedef struct Cxconstraint_s Cxconstraint_t;
struct Cxdisc_s; typedef struct Cxdisc_s Cxdisc_t;
struct Cxedit_s; typedef struct Cxedit_s Cxedit_t;
struct Cxexpr_s; typedef struct Cxexpr_s Cxexpr_t;
struct Cxformat_s; typedef struct Cxformat_s Cxformat_t;
struct Cxinstruction_s; typedef struct Cxinstruction_s Cxinstruction_t;
struct Cxitem_s; typedef struct Cxitem_s Cxitem_t;
struct Cxlib_s; typedef struct Cxlib_s Cxlib_t;
struct Cxmatch_s; typedef struct Cxmatch_s Cxmatch_t;
struct Cxmap_s; typedef struct Cxmap_s Cxmap_t;
struct Cxmember_s; typedef struct Cxmember_s Cxmember_t;
struct Cxop_s; typedef struct Cxop_s Cxop_t;
struct Cxoperand_s; typedef struct Cxoperand_s Cxoperand_t;
struct Cxpart_s; typedef struct Cxpart_s Cxpart_t;
struct Cxquery_s; typedef struct Cxquery_s Cxquery_t;
struct Cxreference_s; typedef struct Cxreference_s Cxreference_t;
struct Cxstate_s; typedef struct Cxstate_s Cxstate_t;
struct Cxstructure_s; typedef struct Cxstructure_s Cxstructure_t;
struct Cxtype_s; typedef struct Cxtype_s Cxtype_t;
struct Cxvariable_s; typedef struct Cxvariable_s Cxvariable_t;

struct Cxop_s				/* callout/recode op		*/
{
	int		code;		/* op code			*/
	Cxtype_t*	type1;		/* operand 1 type		*/
	Cxtype_t*	type2;		/* operand 2 type		*/
};

#define _CX_HEADER_ \
	const char*	name;		/* key name			*/ \
	const char*	description;	/* description			*/

#define _CX_HEADER_LINK_ \
	Dtlink_t	link;		/* dictionary link		*/ \
	uint16_t	flags;		/* cx header flags		*/ \
	uint16_t	index;		/* member index			*/

#define _CX_NAME_HEADER_ \
	_CX_HEADER_ \
	struct \
	{ \
	_CX_HEADER_LINK_ \
	}		header;

#define _CX_LIST_HEADER_ \
	_CX_HEADER_ \
	struct \
	{ \
	_CX_HEADER_LINK_ \
	Dtlink_t	list; \
	}		header;

#define _CX_CODE_HEADER_ \
	_CX_NAME_HEADER_ \
	Cxop_t		op;		/* operator			*/

typedef struct Cxheader_s		/* name/description header	*/
{
	_CX_HEADER_
} Cxheader_t;

typedef struct Cxnameheader_s		/* name key dict element header	*/
{
	_CX_NAME_HEADER_
} Cxnameheader_t;

typedef struct Cxlistheader_s		/* list key dict element header	*/
{
	_CX_LIST_HEADER_
} Cxlistheader_t;

typedef struct Cxcodeheader_s		/* code key dict element header	*/
{
	_CX_CODE_HEADER_
} Cxcodeheader_t;

typedef struct Cxbuffer_s		/* buffer type			*/
{
	void*		data;		/* data pointer			*/
	uint32_t	size;		/* data size			*/
	uint32_t	elements;	/* sizeof() elements		*/
} Cxbuffer_t;

typedef struct Cxstring_s		/* string type			*/
{
	char*		data;		/* data pointer			*/
	size_t		size;		/* data size			*/
} Cxstring_t;

typedef union Cxvalue_u			/* fundamental types		*/
{
	Cxbuffer_t	buffer;		/* sized buffer			*/
	Cxnumber_t	number;		/* long/double number		*/
	void*		pointer;	/* generic pointer		*/
	Cxstring_t	string;		/* 0-terminated string		*/
	Cxtype_t*	type;		/* type				*/
	Cxvariable_t*	variable;	/* variable reference		*/
} Cxvalue_t;

typedef int	(*Cxcallout_f)	(Cx_t*, Cxinstruction_t*, Cxoperand_t*, 
				 Cxoperand_t*, Cxoperand_t*, void*, Cxdisc_t*);
typedef int	(*Cxconstraint_f)(Cx_t*, Cxvalue_t*, void*, Cxdisc_t*);
typedef int	(*Cxdone_f)	(Cx_t*, void*, Cxdisc_t*);
typedef ssize_t	(*Cxexternal_f)	(Cx_t*, Cxtype_t*, const char*, Cxformat_t*, 
				 Cxvalue_t*, char*, size_t, Cxdisc_t*);
typedef int	(*Cxfunction_f)	(Cx_t*, Cxvariable_t*, Cxoperand_t*, 
				 Cxoperand_t*, int, void*, Cxdisc_t*);
typedef void*	(*Cxinit_f)	(void*, Cxdisc_t*);
typedef ssize_t	(*Cxinternal_f)	(Cx_t*, Cxtype_t*, const char*, Cxformat_t*, 
				 Cxoperand_t*, const char*, size_t, Vmalloc_t*, 
				 Cxdisc_t*);
typedef Cxlib_t*(*Cxload_f)	(const char*, Cxdisc_t*);
typedef char*	(*Cxlocation_f)	(Cx_t*, void*, Cxdisc_t*);
typedef void*	(*Cxmatchcomp_f)(Cx_t*, Cxtype_t*, Cxtype_t*, Cxvalue_t*, Cxdisc_t*);
typedef int	(*Cxmatchexec_f)(Cx_t*, void*, Cxtype_t*, Cxvalue_t*, Cxdisc_t*);
typedef int	(*Cxmatchfree_f)(Cx_t*, void*, Cxdisc_t*);
typedef char*	(*Cxnum2str_f)	(Cx_t*, Cxunsigned_t, Cxdisc_t*);
typedef int	(*Cxquery_f)	(Cx_t*, Cxexpr_t*, void*, Cxdisc_t*);
typedef int	(*Cxrecode_f)	(Cx_t*, Cxexpr_t*, Cxinstruction_t*, 
				 Cxinstruction_t*, Cxinstruction_t*, void*, 
				 Cxdisc_t*);
typedef int	(*Cxstr2num_f)	(Cx_t*, const char*, size_t, Cxunsigned_t*, Cxdisc_t*);

struct Cxoperand_s			/* expression operand		*/
{
	Cxtype_t*	type;		/* type				*/
	int		refs;		/* reference count		*/
	Cxvalue_t	value;		/* value			*/
};

struct Cxitem_s				/* map item			*/
{
	Cxitem_t*	next;		/* next item			*/
	const char*	name;		/* item name			*/
	Cxunsigned_t	mask;		/* local mask			*/
	Cxunsigned_t	value;		/* item value			*/
	Cxmap_t*	map;		/* optional map on value match	*/
#ifdef _CX_ITEM_PRIVATE_
	_CX_ITEM_PRIVATE_
#endif
};

struct Cxedit_s				/* edit list element		*/
{
	_CX_NAME_HEADER_
	Cxedit_t*	next;		/* next in list			*/
	Cxinit_f	initf;		/* called at cxaddedit()	*/
	Cxnum2str_f	num2strf;	/* num=>str function		*/
	Cxstr2num_f	str2numf;	/* str=>num function		*/
	void*		data;		/* private data			*/
#ifdef _CX_EDIT_PRIVATE_
	_CX_EDIT_PRIVATE_
#endif
};

struct Cxpart_s				/* map part			*/
{
	Cxpart_t*	next;		/* next part			*/
	Cxunsigned_t	shift;		/* local shift			*/
	Cxunsigned_t	mask;		/* local mask			*/
	Cxitem_t*	item;		/* item list			*/
	Cxflags_t	flags;		/* flags			*/
	Cxtype_t*	type;		/* item value type		*/
	Cxedit_t*	num2str;	/* num=>str edit list		*/
	Cxedit_t*	str2num;	/* str=>num edit list		*/
	Cxedit_t*	edit;		/* str=>str edit list		*/
};

struct Cxmap_s				/* str<=>num map		*/
{
	_CX_NAME_HEADER_
	Cxunsigned_t	shift;		/* global shift			*/
	Cxunsigned_t	mask;		/* global mask			*/
	Cxpart_t*	part;		/* part list			*/
	Dt_t*		str2num;	/* str=>num dict		*/
	Dt_t*		num2str;	/* pure value num=>str dict	*/
	Cxmap_t*	map;		/* indirect reference		*/
};

struct Cxconstraint_s			/* value constraints		*/
{
	_CX_NAME_HEADER_
	Cxinit_f	initf;		/* called at cxaddconstraint()	*/
	Cxconstraint_f	constraintf;	/* external constraint function	*/
	Cxvalue_t*	def;		/* default value		*/
	Cxvalue_t*	min;		/* numeric minimum		*/
	Cxvalue_t*	max;		/* numeric maximum		*/
	const char*	expression;	/* expression on ``.''		*/
	const char*	pattern;	/* string match pattern		*/
	void*		data;		/* private data			*/
#ifdef _CX_CONSTRAINT_PRIVATE_
	_CX_CONSTRAINT_PRIVATE_
#endif
};

struct Cxformat_s			/* format info			*/
{
	const char*	description;	/* external details description	*/
	char*		details;	/* default external details	*/
	unsigned short	flags;		/* flags			*/
	short		width;		/* width in bytes		*/
	short		print;		/* print width hint		*/
	short		base;		/* base				*/
	short		fill;		/* fill character		*/
	short		code;		/* code set			*/
	short		delimiter;	/* delimiter			*/
	short		escape;		/* escape			*/
	short		quotebegin;	/* quotebegin			*/
	short		quoteend;	/* quoteend			*/
	short		fixedpoint;	/* fixed point width		*/
	short		unused;		/* not used			*/
	Cxmap_t*	map;		/* str<=>num map		*/
	Cxconstraint_t*	constraint;	/* value constraints		*/
};

struct Cxreference_s			/* member reference		*/
{
	Cxreference_t*	next;		/* submember			*/
	Cxvariable_t*	variable;	/* member variable		*/
	Cxmember_t*	member;		/* member info			*/
};

struct Cxarray_s			/* array info			*/
{
	Cxvariable_t*	variable;	/* variable size value		*/
	size_t		size;		/* fixed or max size		*/
	short		delimiter;	/* value delimiter		*/
};

struct Cxstructure_s			/* structure info		*/
{
	Cxvariable_t*	parent;		/* parent structure		*/
	Cxvariable_t*	members;	/* member list (children)	*/
	Cxvariable_t*	next;		/* next member (sibling)	*/
	size_t		size;		/* size				*/
	int		level;		/* structure level		*/
};

struct Cxvariable_s			/* variable info		*/
{
	_CX_LIST_HEADER_
	Cxfunction_f	function;	/* pointer if function		*/
	Cxtype_t*	type;		/* value type			*/
	const char*	prototype;	/* (proto)type name		*/
	unsigned long	index;		/* caller defined index		*/
	Cxformat_t	format;		/* format info			*/
	void*		data;		/* caller defined data		*/
	Cxreference_t*	reference;	/* member reference list	*/
	Cxtype_t*	member;		/* member type			*/
	Cxarray_t*	array;		/* array info			*/
	Cxstructure_t*	structure;	/* structure info		*/
};

struct Cxmatch_s			/* type match info		*/
{
	_CX_NAME_HEADER_
	Cxmatchcomp_f	compf;		/* match pattern compile	*/
	Cxmatchexec_f	execf;		/* match pattern exec		*/
	Cxmatchfree_f	freef;		/* match pattern free		*/
};

struct Cxmember_s			/* type member info		*/
{
	Cxcallout_f	getf;		/* get member value		*/
	Cxcallout_f	setf;		/* set member value		*/
	Dt_t*		members;	/* Cxvariable_t member dict	*/
	Cxflags_t	flags;		/* CX_* member flags		*/
};

struct Cxtype_s				/* type info			*/
{
	_CX_NAME_HEADER_
	Cxtype_t*	base;		/* base type			*/
	Cxinit_f	initf;		/* called at cxaddtype()	*/
	Cxexternal_f	externalf;	/* internal => external		*/
	Cxinternal_f	internalf;	/* external => internal		*/
	unsigned short	representation;	/* fundamental type index	*/
	unsigned short	index;		/* caller defined index		*/
	unsigned short	size;		/* sizeof() size in bytes	*/
	unsigned short	element;	/* element size in bytes	*/
	Cxformat_t	format;		/* format defaults		*/
	Cxmatch_t*	match;		/* match info			*/
	Cxmember_t*	member;		/* member info			*/
	Cxtype_t**	generic;	/* generic implementation table	*/
	Cxtype_t*	fundamental;	/* fundamental type		*/
	void*		data;		/* private data			*/
};

typedef struct Cxcallout_s		/* op code callout		*/
{
	_CX_CODE_HEADER_
	Cxcallout_f	callout;	/* callout function		*/
} Cxcallout_t;

typedef struct Cxrecode_s		/* recode callout		*/
{
	_CX_CODE_HEADER_
	Cxrecode_f	recode;		/* callout function		*/
} Cxrecode_t;

struct Cxinstruction_s			/* parsed instruction		*/
{
	int		op;		/* op code			*/
	int		pp;		/* stack push(>0) pop(<0) count	*/
	Cxtype_t*	type;		/* return type			*/
	Cxvalue_t	data;		/* optional data		*/
	Cxcallout_f	callout;	/* callout function		*/
};

struct Cxdisc_s				/* user discipline		*/
{
	unsigned long	version;	/* interface version		*/
	Error_f		errorf;		/* error function		*/
	Cxload_f	loadf;		/* library load function	*/
	Cxlocation_f	locationf;	/* input location function	*/
	const char*	ps1;		/* primary prompt		*/
	const char*	ps2;		/* secondary prompt		*/
	const char*	map;		/* map file			*/
};

struct Cxquery_s			/* query			*/
{
	_CX_NAME_HEADER_
	Cxquery_f	beg;		/* called before first eval	*/
	Cxquery_f	sel;		/* select current data		*/
	Cxquery_f	act;		/* act on selected data		*/
	Cxquery_f	end;		/* called after last eval	*/
	const char*	method;		/* caller specific method match	*/
	Cxquery_f	ref;		/* compile-time reference	*/
#ifdef _CX_QUERY_PRIVATE_
	_CX_QUERY_PRIVATE_
#endif
};

struct Cxmeth_s; typedef struct Cxmeth_s Cxmeth_t;

struct Cxlib_s				/* Cxdisc_t.loadf library info	*/
{
	_CX_NAME_HEADER_
	const char**	libraries;	/* library list			*/
	Cxmeth_t*	meth;		/* caller method		*/
	Cxtype_t*	types;		/* type table			*/
	Cxcallout_t*	callouts;	/* callout table		*/
	Cxrecode_t*	recodes;	/* recode table			*/
	Cxmap_t**	maps;		/* map table			*/
	Cxquery_t*	queries;	/* query table			*/
	Cxconstraint_t*	constraints;	/* constraint table		*/
	Cxedit_t*	edits;		/* edit table			*/
	Cxvariable_t*	functions;	/* function table		*/

	void*		pad[7];		/* pad for future expansion	*/

	/* the remaining are set by Cxdisc_t.loadf			*/

	const char*	path;		/* library path name		*/
};

struct Cxexpr_s				/* compiled expression node	*/
{
	Cxexpr_t*	parent;		/* parent			*/
	Cxexpr_t*	group;		/* group			*/
	Cxexpr_t*	next;		/* next sibling			*/
	Cxexpr_t*	pass;		/* pass branch			*/
	Cxexpr_t*	fail;		/* fail branch			*/
	Cxquery_t*	query;		/* query callouts		*/
	const char*	file;		/* output file			*/
	Sfio_t*		op;		/* output stream for file	*/
	void*		data;		/* query private data		*/
	char**		argv;		/* query argv			*/
	Cxunsigned_t	queried;	/* # records queried		*/
	Cxunsigned_t	selected;	/* # records selected		*/
#ifdef _CX_EXPR_PRIVATE_
	_CX_EXPR_PRIVATE_
#endif
};

struct Cxstate_s			/* cx library global state	*/
{
	Dt_t*		libraries;	/* Dsslib_t dictionary (ouch)	*/
	Dt_t*		methods;	/* Cxnameheader_t dictionary	*/
	Dt_t*		types;		/* Cxtype_t dictionary		*/
	Dt_t*		callouts;	/* Cxcallout_t dictionary	*/
	Dt_t*		recodes;	/* Cxrecode_t dictionary	*/
	Dt_t*		maps;		/* Cxmap_t dictionary		*/
	Dt_t*		queries;	/* Cxquery_t dictionary		*/
	Dt_t*		constraints;	/* Cxconstraint_t dictionary	*/
	Dt_t*		edits;		/* Cxedit_t dictionary		*/
	Dt_t*		variables;	/* Cxvariable_t dictionary	*/
	Cxtype_t*	type_bool;	/* bool fundamental type	*/
	Cxtype_t*	type_buffer;	/* buffer fundamental type	*/
	Cxtype_t*	type_number;	/* number fundamental type	*/
	Cxtype_t*	type_reference;	/* reference fundamental type	*/
	Cxtype_t*	type_string;	/* string fundamental type	*/
	Cxtype_t*	type_type_t;	/* type				*/
	Cxtype_t*	type_void;	/* void fundamental type	*/
#ifdef _CX_STATE_PRIVATE_
	_CX_STATE_PRIVATE_
#endif
};

struct Cx_s				/* interface handle		*/
{
	const char*	id;		/* interface id			*/
	Vmalloc_t*	vm;		/* handle memory		*/
	Vmalloc_t*	em;		/* eval memory			*/
	Vmalloc_t*	rm;		/* record memory		*/
	Cxflags_t	flags;		/* CX_* flags			*/
	Cxflags_t	test;		/* test mask			*/
	int		eof;		/* input at eof			*/
	int		error;		/* error occurred		*/
	int		interactive;	/* interactive input		*/
	Cxstate_t*	state;		/* global state			*/
	Cxdisc_t*	disc;		/* user discipline		*/
	Sfio_t*		buf;		/* tmp buffer stream		*/
	void*		caller;		/* caller defined handle	*/
	Dt_t*		variables;	/* sorted variable symbol table	*/
	Dt_t*		fields;		/* order variable field list	*/
	Dt_t*		types;		/* Cxtype_t dictionary		*/
	Dt_t*		callouts;	/* Cxcallout_t dictionary	*/
	Dt_t*		recodes;	/* Cxrecode_t dictionary	*/
	Dt_t*		maps;		/* Cxmap_t dictionary		*/
	Dt_t*		queries;	/* Cxquery_t dictionary		*/
	Dt_t*		constraints;	/* Cxconstraint_t dictionary	*/
	Dt_t*		edits;		/* Cxedit_t dictionary		*/
	Cx_t*		scope;		/* next scope			*/
	unsigned char*	ctype;		/* ctype table			*/
#ifdef _CX_PRIVATE_
	_CX_PRIVATE_
#endif
};

#define cxinit(d,e)	(memset(d,0,sizeof(Cxdisc_t)),(d)->version=CX_VERSION,(d)->errorf=(Error_f)(e),cxstate(d))

#if _BLD_cx && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Cxstate_t*	cxstate(Cxdisc_t*);

extern Cx_t*		cxopen(Cxflags_t, Cxflags_t, Cxdisc_t*);
extern Cx_t*		cxscope(Cx_t*, Cx_t*, Cxflags_t, Cxflags_t, Cxdisc_t*);
extern int		cxclose(Cx_t*);

extern void*		cxpush(Cx_t*, const char*, Sfio_t*, const char*, ssize_t, Cxflags_t);
extern int		cxpop(Cx_t*, void*);
extern ssize_t		cxtell(Cx_t*);
extern Cxexpr_t*	cxcomp(Cx_t*);
extern int		cxbeg(Cx_t*, Cxexpr_t*, const char*);
extern int		cxeval(Cx_t*, Cxexpr_t*, void*, Cxoperand_t*);
extern int		cxend(Cx_t*, Cxexpr_t*);
extern int		cxlist(Cx_t*, Cxexpr_t*, Sfio_t*);
extern int		cxfree(Cx_t*, Cxexpr_t*);
extern int		cxcast(Cx_t*, Cxoperand_t*, Cxvariable_t*, Cxtype_t*, void*, const char*);
extern size_t		cxsizeof(Cx_t*, Cxvariable_t*, Cxtype_t*, Cxvalue_t*);
extern ssize_t		cxmembers(Cx_t*, Cxtype_t*, const char*, Cxformat_t*, Cxvalue_t*, char*, size_t, Cxdisc_t*);

extern char*		cxcontext(Cx_t*);
extern char*		cxlocation(Cx_t*, void*);

extern void		cxcodetrace(Cx_t*, const char*, Cxinstruction_t*, unsigned int, Cxoperand_t*, Cxoperand_t*);
extern char*		cxcodename(int);
extern char*		cxopname(int, Cxtype_t*, Cxtype_t*);

extern int		cxaddcallout(Cx_t*, Cxcallout_t*, Cxdisc_t*);
extern int		cxaddconstraint(Cx_t*, Cxconstraint_t*, Cxdisc_t*);
extern int		cxaddedit(Cx_t*, Cxedit_t*, Cxdisc_t*);
extern int		cxaddmap(Cx_t*, Cxmap_t*, Cxdisc_t*);
extern int		cxaddquery(Cx_t*, Cxquery_t*, Cxdisc_t*);
extern int		cxaddrecode(Cx_t*, Cxrecode_t*, Cxdisc_t*);
extern int		cxaddtype(Cx_t*, Cxtype_t*, Cxdisc_t*);
extern int		cxaddvariable(Cx_t*, Cxvariable_t*, Cxdisc_t*);

extern Cxtype_t*	cxattr(Cx_t*, const char*, char**, Cxformat_t*, Cxdisc_t*);
extern Cxcallout_f	cxcallout(Cx_t*, int, Cxtype_t*, Cxtype_t*, Cxdisc_t*);
extern Cxconstraint_t*	cxconstraint(Cx_t*, const char*, Cxdisc_t*);
extern Cxedit_t*	cxedit(Cx_t*, const char*, Cxdisc_t*);
extern Cxvariable_t*	cxfunction(Cx_t*, const char*, Cxdisc_t*);
extern Cxmap_t*		cxmap(Cx_t*, const char*, Cxdisc_t*);
extern Cxquery_t*	cxquery(Cx_t*, const char*, Cxdisc_t*);
extern Cxrecode_f	cxrecode(Cx_t*, int, Cxtype_t*, Cxtype_t*, Cxdisc_t*);
extern Cxtype_t*	cxtype(Cx_t*, const char*, Cxdisc_t*);
extern Cxvariable_t*	cxvariable(Cx_t*, const char*, Cxtype_t*, Cxdisc_t*);

extern int 		cxnum2str(Cx_t*, Cxformat_t*, Cxunsigned_t, char**);
extern int		cxstr2num(Cx_t*, Cxformat_t*, const char*, size_t, Cxunsigned_t*);

extern int		cxsub(Cx_t*, Cxedit_t*, Cxoperand_t*);
extern int		cxsuball(Cx_t*, Cxpart_t*, Cxoperand_t*);

extern char*		cxcvt(Cx_t*, const char*, size_t);

extern int		cxatfree(Cx_t*, Cxexpr_t*, Cxdone_f, void*);

#undef	extern

#endif
