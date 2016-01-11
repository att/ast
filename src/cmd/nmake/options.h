/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2012 AT&T Intellectual Property          *
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
 * make option attributes and flag mappings
 *
 * omitted letters are avilable for option flags:
 *
 *	abcdefghijklmnopqrstuvwxyz
 *      --------------------------
 *	ABCDEFGHIJK MNOPQRSTUV X  
 *	abcdefg ijkl no  rst vwx z
 */

#define OPT(o)		(o&((1<<9)-1))	/* get opt char from code	*/
#define Of		(1<<8)		/* cannot be set by flag	*/

#define Oa		(1<<9)		/* multiple string values added	*/
#define Ob		(1<<10)		/* boolean value		*/
#define Oi		(1<<11)		/* state.* sense inverted	*/
#define Om		(1<<13)		/* internal option		*/
#define On		(1<<14)		/* numeric value		*/
#define Oo		(1<<15)		/* flag sense opposite		*/
#define Op		(1L<<19)	/* .mo probe prerequisite	*/
#define Os		(1L<<16)	/* string value			*/
#define Ov		(1L<<17)	/* value is optional		*/
#define Ox		(1L<<18)	/* not expanded in $(-)		*/

#define OPT_COMPILE	(1L<<26)	/* compile into object		*/
#define OPT_DECLARE	(1L<<27)	/* compile declaration		*/
#define OPT_DEFAULT	(1L<<28)	/* := default value		*/
#define OPT_EXTERNAL	(1L<<29)	/* external by name		*/
#define OPT_READONLY	(1L<<30)	/* cannot reset til state.user	*/
#define OPT_SET		(1L<<31)	/* explicitly set		*/

#define OPT_accept	('A'|Ob)	/* accept existing targets	*/
#define OPT_alias	('a'|Ob)	/* directory aliasing enabled	*/
#define OPT_base	('b'|Ob|Ox)	/* compile base|global rules	*/
#define OPT_believe	('B'|On)	/* believe state from this level*/
#define OPT_byname	('o'|Oa|Os|Ox)	/* command line option by name	*/
#define OPT_compile	('c'|Ob|Ox)	/* force makefile compilation	*/
#define OPT_compatibility ('C'|Ob|Ox)	/* disable compatibility msgs	*/
#define OPT_corrupt	('X'|Os|Ov)	/* corrupt statefile action	*/
#define OPT_cross	('J'|Ob)	/* don't run gen'd executables	*/
#define OPT_debug	('d'|Oi|On)	/* debug trace level		*/
#define OPT_define	('D'|Op|Os|Ox)	/* passed to preprocessor	*/
#define OPT_errorid	('E'|Os)	/* append to error output id	*/
#define OPT_exec	('n'|Ob|Oo)	/* execute shell actions	*/
#define OPT_expandview	('x'|Ob)	/* expand paths if fsview!=0	*/
#define OPT_explain	('e'|Ob)	/* explain actions		*/
#define OPT_file	('f'|Oa|Os|Ox)	/* next arg is makefile		*/
#define OPT_force	('F'|Ob)	/* force targets to be updated	*/
#define OPT_global	('g'|Oa|Os|Ov)	/* next arg is global makefile	*/
#define OPT_ignore	('i'|Ob)	/* ignore shell action errors	*/
#define OPT_ignorelock	('K'|Ob)	/* ignore state file locking	*/
#define OPT_include	('I'|Os|Ox)	/* passed to preprocessor	*/
#define OPT_intermediate ('G'|Ob)	/* force intermediate targets	*/
#define OPT_jobs	('j'|On)	/* job concurrency level	*/
#define OPT_keepgoing	('k'|Ob)	/* do sibling prereqs on error	*/
#define OPT_list	('l'|Ob|Ox)	/* list info and don't make	*/
#define OPT_mam		('M'|Os)	/* generate mam			*/
#define OPT_never	('N'|Ob)	/* really - don't exec anything	*/
#define OPT_option	(101|Oa|Of|Os|Ox)/* define new option by name	*/
#define OPT_override	(102|Ob|Of|Ox)	/* override selected algorithms	*/
#define OPT_preprocess	('P'|Ob)	/* preprocess all makefiles	*/
#define OPT_questionable ('Q'|On)	/* enable questionable code	*/
#define OPT_readonly	('R'|Ob)	/* current vars|opts readonly	*/
#define OPT_readstate	('S'|On|Ov)	/* read state file on startup	*/
#define OPT_regress	('q'|Os|Ov)	/* output for regression test	*/
#define OPT_reread	(103|Ob|Of|Ox)	/* force re-read all makefiles	*/
#define OPT_ruledump	('r'|Ob|Ox)	/* dump rule definitions	*/
#define OPT_scan	(104|Ob|Of)	/* scan|check implicit prereqs	*/
#define OPT_serialize	('O'|Ob)	/* serialize concurrent output	*/
#define OPT_silent	('s'|Ob)	/* run silently			*/
#define OPT_strictview	('V'|Ob)	/* strict views			*/
#define OPT_targetcontext (105|Ob|Of)	/* expand in target dir context	*/
#define OPT_targetprefix (106|Os|Of)	/* source dir target prefix sep	*/
#define OPT_test	('T'|On)	/* enable test code		*/
#define OPT_tolerance	('z'|On)	/* time comparison tolerance	*/
#define OPT_touch	('t'|Ob)	/* touch out of date targets	*/
#define OPT_undef	('U'|Op|Os|Ox)	/* passed to preprocessor	*/
#define OPT_vardump	('v'|Ob|Ox)	/* dump variable definitions	*/
#define OPT_warn	('w'|Ob)	/* enable source file warnings	*/
#define OPT_writeobject	(107|Of|Os|Ov|Ox)/* write recompiled object	*/
#define OPT_writestate	(108|Of|Os|Ov|Ox)/* write state file on exit	*/

struct Option_s; typedef struct Option_s Option_t;

struct Option_s				/* option table entry		*/
{
	char*		name;		/* option name			*/
	unsigned long	flags;		/* O[a-z] and OPT_[A-Z]+ flags	*/
	char*		value;		/* overloaded value		*/
	char*		set;		/* call this on set		*/
	char*		description;	/* description			*/
	char*		arg;		/* arg name			*/
	Option_t*	next;		/* external option list link	*/
};

extern Option_t*	optflag(int);
extern void		optcheck(int);
extern void		optinit(void);
