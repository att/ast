/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
 * troff to html definitions
 */

#ifndef _MM2HTML_H
#define _MM2HTML_H	1

#include <ast.h>
#include <ccode.h>
#include <ctype.h>
#include <debug.h>
#include <hash.h>

#define ARGS		512
#define STKS		512
#define MAXNAME		1024

#define COND_BLOCK	(1<<0)
#define COND_EL		(1<<1)
#define COND_IE		(1<<2)
#define COND_IF		(1<<3)
#define COND_KEPT	(1<<4)
#define COND_SKIP	(1<<5)

#define TAG_BREAK	(1<<0)
#define TAG_COPY	(1<<1)
#define TAG_DO		(1<<2)
#define TAG_PASS	(1<<3)
#define TAG_RAW		(1<<4)
#define TAG_STATIC	(1<<5)
#define TAG_STATIC_BODY	(1<<6)
#define TAG_TRACE_GET	(1<<7)
#define TAG_TRACE_SET	(1<<8)
#define TAG_TRIGGER	(1<<9)

typedef struct Arg_s
{
	int		argc;		/* number of args		*/
	char*		argv[ARGS];	/* arg values			*/
	Sfio_t*		sp;		/* arg string stream		*/
} Arg_t;

struct Tag_s;

typedef void (*Call_f)(struct Tag_s*, Arg_t*);

typedef struct Tag_s
{
	char*		name;		/* mm tag name			*/
	Call_f		call;		/* tag implementation		*/
	unsigned long	flags;		/* TAG_* flags			*/
	char*		body;		/* macro body			*/
	char*		file;		/* definition file		*/
	int		line;		/* definition line		*/
	int		size;		/* body size (0 if static)	*/
} Tag_t;

typedef struct Dir_s
{
	struct Dir_s*	next;		/* next in list			*/
	char*		name;		/* directory name		*/
} Dir_t;

typedef struct Num_s
{
	char*		name;		/* register name		*/
	char		value[64];	/* representation value		*/
	long		number;		/* numeric value		*/
	int		flags;		/* TAG_* flags (yes)		*/
	int		increment;	/* auto increment amount	*/
	char		format;		/* {0,1,a,A,i,I}		*/
	char		internal;	/* internal readonly		*/
} Num_t;

typedef struct
{
	char*		path;		/* last path on stream		*/
	Sfio_t*		sp;		/* io stream			*/
} Stream_t;

typedef struct Pushin_s
{
	unsigned char*	in;		/* previous state.in		*/
	char*		file;		/* previous error_info.file	*/
	int		line;		/* previous error_info.line	*/
	Arg_t*		mac;		/* previous state.mac		*/
	Arg_t*		tag;		/* previous state.tag		*/
	Arg_t		top;		/* new state.tag		*/
	Sfio_t*		ip;		/* input file stream		*/
	unsigned char*	buf;		/* input buffer			*/
	unsigned char*	end;		/* end of input buffer		*/
	Sfio_t*		loop;		/* loop body stream		*/
} Pushin_t;

typedef struct Trap_s
{
	struct Trap_s*	next;		/* next in list			*/
	char		name[1];	/* trap name			*/
} Trap_t;

typedef struct Value_s
{
	int		current;	/* current value		*/
	int		previous;	/* previous value		*/
} Value_t;

typedef struct Var_s
{
	char*		name;		/* variable name		*/
	char*		value;		/* variable value		*/
} Var_t;

typedef struct Env_s
{
	int		generation;	/* environment generation	*/

	int		c2;		/* no-br tag control character	*/
	int		cc;		/* tag control character	*/
	int		ce;		/* center state			*/
	int		dl;		/* output line length		*/
	int		dn;		/* output line count		*/
	int		nf;		/* output fill state		*/
	int		ss;		/* sub/super script state	*/

	Value_t		ft;		/* font index			*/
	Value_t		in;		/* left indent			*/
	Value_t		ll;		/* line length			*/
	Value_t		po;		/* page offset			*/
	Value_t		ps;		/* font size			*/
	Value_t		ti;		/* temporary indent		*/
	Value_t		vs;		/* vertical spacing		*/

	char*		name;		/* environment name		*/

} Env_t;

typedef struct Divert_s
{
	struct Divert_s*next;		/* next in list			*/
	Env_t		environment;	/* diversion environment	*/
	Env_t*		env;		/* previous state.env		*/
	Sfio_t*		sp;		/* diversion stream		*/
	Tag_t*		tag;		/* diversion tag		*/
} Divert_t;

typedef struct List_s
{
	int		dl;		/* dl code emitted		*/
	int		in;		/* data indent			*/
	int		ti;		/* label indent			*/
} List_t;

typedef struct State_s
{
	int		generation;	/* evnironment generation	*/
	Env_t*		env;		/* current environment		*/
	Env_t*		env_stack[STKS];/* environment stack		*/
	Env_t**		env_sp;		/* environment stack pointer	*/
	unsigned char	tag_stack[STKS];/* tag nest stack		*/
	unsigned char*	tag_top;	/* tag nest stack pointer	*/

	List_t*		list;		/* current list state		*/
	List_t		list_stack[STKS];/* current list stack		*/

	unsigned long	test;		/* test mask			*/

	int		groff;		/* groff extensions		*/
	int		groff_init;	/* groff extensions init	*/
	int		dl;		/* diversion line length	*/
	int		dn;		/* diversion line count		*/
	int		ec;		/* escape character		*/
	int		footer;		/* footer title emitted		*/
	int		head;		/* document has head		*/
	int		eo;		/* ec disabled			*/
	int		link;		/* current \h...\h'0' code	*/
	int		ln;		/* output line count		*/
	int		n;		/* last output line length	*/
	int		nl;		/* output line position		*/
	int		noline;		/* '\n' is not line terminator	*/
	int		pass;		/* pass state			*/
	int		pc;		/* title page number character	*/
	int		silent;		/* minimal error messages	*/
	int		t;		/* distance to next trap	*/
	int		verbose;	/* verbose messages		*/

	time_t		date;		/* global date			*/

	char*		address;	/* author address		*/
	char*		author;		/* document author		*/
	char*		background;	/* background image		*/
	char*		company;	/* author company		*/
	char*		corporation;	/* author corporation		*/
	char*		font[6];	/* font index to name map	*/
	char*		input;		/* first input file		*/
	char*		location;	/* author location		*/
	char*		logo;		/* logo/banner image		*/
	char*		mailto;		/* mail contact			*/
	char*		organization;	/* author organization		*/
	char*		package;	/* title prefix			*/
	char*		phone;		/* author phone			*/
	char*		title;		/* document title		*/
	char*		toolbar;	/* junk before </BODY>		*/

	Arg_t*		mac;		/* current macro args		*/
	Arg_t*		tag;		/* current tag args		*/
	Arg_t		top;		/* top tag arg data		*/

	Dir_t*		dirs;		/* include dir list		*/
	Dir_t*		hot;		/* hot text list		*/
	Dir_t*		macros;		/* macro packages		*/

	Divert_t*	divert;		/* diversion stack		*/

	Hash_table_t*	symbols;	/* symbol dictionary		*/

	Sfio_t*		arg;		/* arg buffer			*/
	Sfio_t*		nul;		/* ignored buffer		*/
	Sfio_t*		out;		/* output file pointer		*/
	Sfio_t*		ref;		/* reference buffer		*/
	Sfio_t*		req;		/* request buffer		*/
	Sfio_t*		tmp;		/* temporary buffer		*/

	unsigned char*	in;		/* input buffer pointer		*/

	unsigned char	ta[ARGS];	/* .ta stops			*/

	Pushin_t	in_stack[STKS];	/* input stream stack		*/
	Pushin_t*	in_top;		/* input stream stack top	*/
	Sfio_t*		out_stack[STKS];/* output stream stack		*/
	Sfio_t**	out_top;	/* output stream stack top	*/

	Tag_t*		define;		/* defining this macro		*/
	Tag_t*		end;		/* end collection with this tag	*/

	struct
	{
	int		level;		/* conditional nesting level	*/
	int		flags[STKS];	/* COND_* flags			*/
	}		cond;

	struct
	{
	char		trap[MAXNAME+4];/* inline trap invocation	*/
	int		center;		/* remaining center lines	*/
	int		count;		/* remaining input lines	*/
	int		dc;		/* check list indent		*/
	int		dd;		/* end of list item		*/
	int		dl;		/* start of list		*/
	int		dt;		/* start of list item		*/
	int		interrupt;	/* text interrupted by \c	*/
	int		right;		/* right justify `center'	*/
	int		text;		/* output text line count	*/
	}		it;

	struct
	{
	char*		file;		/* original file		*/
	int		line;		/* original line		*/
	}		original;

	Trap_t*		fini;		/* fini trap list		*/
	Trap_t*		trap;		/* normal trap list		*/

} State_t;

extern State_t		state;

#endif
