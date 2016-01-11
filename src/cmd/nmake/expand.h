/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2011 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * make variable expansion long name interface and data
 * in a header to allow an external old->new syntax converter
 *
 * edit op names are mapped to the interpreted <op><sep><arg> forms
 */

#ifndef ED_EDIT

typedef struct
{
	unsigned short		type;		/* {ED_OP,ED_QUAL}	*/
	unsigned short		op;		/* old op name letter	*/
	unsigned short		arg;		/* old op value char 1	*/
	unsigned short		aux;		/* old op value char 2	*/
} Edit_cmd_t;

typedef struct
{
	const char*		name;		/* option name		*/
	const Edit_cmd_t	cmd;		/* cmd info		*/
} Edit_opt_t;

typedef struct
{
	const char*		name;		/* operator name	*/
	const Edit_opt_t*	options;	/* operator option map	*/
	const Edit_cmd_t	cmd;		/* cmd info		*/
} Edit_map_t;

typedef struct
{
	const char*		old;		/* old op		*/
	const char*		xxx;		/* new op		*/
	int			len;		/* len(old)==len(xxx)	*/
} Edit_xxx_t;

#define LT		(1<<0)
#define EQ		(1<<1)
#define GT		(1<<2)
#define NOT		(1<<3)
#define HAT		(1<<4)
#define MAT		(1<<5)

#define NE		(NOT|EQ)
#define LE		(LT|EQ)
#define GE		(GT|EQ)
#define CMP		(1<<6)

#define ED_COPY		1
#define ED_EDIT		2
#define ED_OP		3
#define ED_QUAL		4

#define ED_AUXILLIARY	(CMP<<0)
#define ED_FORCE	(CMP<<1)
#define ED_JOIN		(CMP<<5)
#define ED_LITERAL	(CMP<<2)
#define ED_LONG		(CMP<<3)
#define ED_NOBIND	(CMP<<4)
#define ED_NOWAIT	(CMP<<6)
#define ED_PARTS	(CMP<<7)
#define ED_PRIMARY	(CMP<<8)

/*
 * old -> new ops not captured by the tables
 */

static const Edit_xxx_t		editxxx[] =
{
	"A>",		"N>",		2,
	0
};

static const Edit_opt_t		editbind[] =
{
	"force",		{ ED_QUAL,ED_FORCE	},
	"nowait",		{ ED_QUAL,ED_NOWAIT	},
	0
};

static const Edit_opt_t		editbound[] =
{
	"",			{ ED_OP,   0,'B'	},
	"directory",		{ ED_OP,   0,'D'	},
	0
};

static const Edit_opt_t		editgenerate[] =
{
	"target",		{ ED_QUAL,LT		},
	0
};

static const Edit_opt_t		editintersect[] =
{
	"literal",		{ ED_QUAL,LT		},
	0
};

static const Edit_opt_t		editlist[] =
{
	"",			{ ED_QUAL,EQ		},
	"first",		{ ED_QUAL,EQ		},
	"sort",			{ ED_QUAL,LT		},
	"reverse",		{ ED_QUAL,GT		},
	0
};

static const Edit_opt_t		editmatch[] =
{
	"prereqs",		{ ED_QUAL,GT		},
	"re",			{ ED_OP,   'M'		},
	0
};

static const Edit_opt_t		editread[] =
{
	"raw",			{ ED_OP,   0,'X'	},
	0
};

static const Edit_opt_t		editsort[] =
{
	"numeric",		{ ED_QUAL,EQ		},
	"reverse",		{ ED_QUAL,GT		},
	"unique",		{ ED_QUAL,NOT		},
	0
};

static const Edit_opt_t		editstate[] =
{
	"",			{ ED_OP,   0,'S'	},
	"assign",		{ ED_OP,   0,'E'	},
	"define",		{ ED_OP,   0,'D'	},
	"prereqs",		{ ED_OP,   0,'S','P'	},
	"rule",			{ ED_OP,   0,'S','R'	},
	"variable",		{ ED_OP,   0,'S','V'	},
	0
};

static const Edit_opt_t		edittime[] =
{
	"event",		{ ED_OP,   0,'E'	},
	"prereqs",		{ ED_OP,   0,'C'	},
	"rule",			{ ED_OP,   0,'R'	},
	"state",		{ ED_OP,   0,'S'	},
	"wall",			{ ED_OP,   0,'W'	},
	0
};

static const Edit_opt_t		editview[] =
{
	"",			{ ED_OP,   0,'L'	},
	"all",			{ ED_OP,   0,'L','*'	},
	"subdirectory",		{ ED_OP,   0,'S'	},
	"top",			{ ED_OP,   0,'L','!'	},
	0
};

static const Edit_opt_t		editwrite[] =
{
	"append",		{ ED_OP,   0,'A'	},
	"numeric",		{ ED_OP,   0,'N'	},
	"raw",			{ ED_OP,   0,'X'	},
	0
};

static const Edit_map_t		editmap[] =
{
	"absolute",		0,		{ ED_OP,  'P','A'	},
	"access",		0,		{ ED_OP,  'P','X'	},
	"alias",		0,		{ ED_OP,  'P','Z'	},
	"archive_update",	0,		{ ED_OP,  'T','A'	},
	"associate",		0,		{ ED_OP,  'A','<'	},
	"attribute",		0,		{ ED_OP,  'A'		},
	"base",			0,		{ ED_EDIT,'B',1		},
	"bind",			0,		{ ED_OP,  'T','*'	},
	"bound",		editbound,	{ ED_OP,  'P'		},
	"break",		0,		{ ED_OP,  'K'		},
	"canon",		0,		{ ED_OP,  'P','C'	},
	"cross",		0,		{ ED_OP,  'X'		},
	"directory",		0,		{ ED_EDIT,'D',1		},
	"expr",			0,		{ ED_COPY,'E'		},
	"format",		0,		{ ED_OP,  'F'		},
	"generate",		editgenerate,	{ ED_OP,  'G'		},
	"generated",		0,		{ ED_OP,  'T','G'	},
	"hash",			0,		{ ED_OP,  'P','H'	},
	"id",			0,		{ ED_OP,  'P','I'	},
	"index",		0,		{ ED_OP,  'O'		},
	"intersect",		editintersect,	{ ED_OP,  'I'		},
	"isrule",		0,		{ ED_OP,  'T','Q'	},
	"isvariable",		0,		{ ED_OP,  'T','Q','V'	},
	"join",			0,		{ ED_QUAL,ED_JOIN	},
	"list",			editlist,	{ ED_OP,  'L'		},
	"match",		editmatch,	{ ED_OP,  'N'		},
	"metarule",		0,		{ ED_OP,  'T','S','M'	},
	"native",		0,		{ ED_OP,  'P','N'	},
	"nobase",		0,		{ ED_EDIT,'B',0		},
	"nodirectory",		0,		{ ED_EDIT,'D',0		},
	"nosuffix",		0,		{ ED_EDIT,'S',0		},
	"not",			0,		{ ED_QUAL,NOT		},
	"parentage",		0,		{ ED_OP,  'T','M'	},
	"parse",		0,		{ ED_OP,  'R'		},
	"physical",		0,		{ ED_OP,  'T','P'	},
	"probe",		0,		{ ED_OP,  'P','P'	},
	"quote",		0,		{ ED_OP,  'Q'		},
	"read",			editread,	{ ED_OP,  'T','I'	},
	"relative",		0,		{ ED_OP,  'P','R'	},
	"rule",			0,		{ ED_OP,  'T','S','A'	},
	"sort",			editsort,	{ ED_OP,  'H'		},
	"state",		editstate,	{ ED_OP,  'T'		},
	"substitute",		0,		{ ED_COPY,'C'		},
	"suffix",		0,		{ ED_EDIT,'S',1		},
	"test",			0,		{ ED_COPY,'Y'		},
	"time",			edittime,	{ ED_OP,  'T','Z'	},
	"timecmp",		0,		{ ED_OP,  'T'		},
	"unbound",		0,		{ ED_OP,  'P','U'	},
	"unstate",		0,		{ ED_OP,  'T','U'	},
	"view",			editview,	{ ED_OP,  'P'		},
	"write",		editwrite,	{ ED_OP,  'T','O'	},
};

#endif
