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
 * html to rtf definitions
 */

#ifndef _HTML2RTF_H
#define _HTML2RTF_H	1

#include <ast.h>
#include <ctype.h>
#include <hash.h>

#define twips(x)	((x)*2)

#define FONTSIZE	12		/* default font size		*/

#define ENT_SPACE	(1<<0)		/* translates to trailing space	*/

#define STK_HEADING	(1<<0)		/* element contains <Hn>	*/
#define STK_NOEND	(1<<1)		/* don't call </x>		*/
#define STK_TAG		(1<<8)		/* first tag specific bit	*/

#define TAG_UNBALANCED	(1<<0)		/* </x> without <x> ok		*/
#define TAG_IGNORE	(1<<1)		/* ignore element text		*/

typedef struct Attribute_s
{
	char*		name;		/* attribute name		*/
	char*		value;		/* optional attribute value	*/
} Attribute_t;

struct Tag_s;

typedef int (*Call_f)(struct Tag_s*, Attribute_t*);

typedef struct Entity_s
{
	const char*	name;		/* entity name			*/
	const char*	value;		/* entity value			*/
	unsigned long	flags;		/* ENT_* flags			*/
} Entity_t;

typedef struct Tag_s
{
	const char*	name;		/* html tag name		*/
	Call_f		start;		/* tag start			*/
	Call_f		end;		/* optional tag end		*/
	void*		data;		/* optional data		*/
	unsigned long	flags;		/* TAG_* flags			*/
} Tag_t;

typedef struct
{
	int		tags;		/* # tags in rendering		*/
	Tag_t*		tag[1];		/* rendering tags		*/
} Render_t;

typedef union Data_u
{
	Sfio_t*		io;
	int		number;
	void*		pointer;
	char*		string;
} Data_t;

typedef struct Stack_s
{
	Tag_t*		tag;		/* active tag			*/
	unsigned int	line;		/* line				*/
	unsigned int	flags;		/* STK_* flags			*/
	Data_t		data[8];	/* tag specific data		*/
} Stack_t;

typedef struct State_s
{
	int		center;		/* center nesting level		*/
	int		fontsize;	/* current font size		*/
	int		hanging;	/* current left hanging indent	*/
	int		indent;		/* current left indent		*/
	int		pre;		/* <PRE> nesting level		*/
	int		sep;		/* output control needs sep	*/
	int		verbose;	/* verbose messages		*/

	char*		project;	/* project file name		*/
	char*		prefix;		/* label prefix			*/

	Sfio_t*		in;		/* input file pointer		*/
	Sfio_t*		nul;		/* ignored buffer		*/
	Sfio_t*		out;		/* output file pointer		*/
	Sfio_t*		tmp;		/* temporary buffer		*/

	Stack_t*	sp;		/* tag stack pointer		*/
	Stack_t*	sp_max;		/* highest sp			*/
	Stack_t*	sp_min;		/* lowest sp			*/

	Hash_table_t*	entities;	/* entity dictionary		*/
	Hash_table_t*	files;		/* project file dictionary	*/
	Hash_table_t*	tags;		/* tag dictionary		*/

} State_t;

extern State_t		state;

#endif
