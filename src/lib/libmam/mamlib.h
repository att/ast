/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * AT&T Bell Laboratories
 *
 * make abstract machine library private definitions
 */

#ifndef _MAMLIB_H
#define _MAMLIB_H

struct frame				/* target stack frame		*/
{
	struct frame*	next;		/* next in list			*/
	struct frame*	prev;		/* prev in list			*/
	struct rule*	rule;		/* rule for this frame		*/
};

#define _MAM_RULE_PRIVATE		/* rule private			*/ \
	struct block*	atail;		/* action tail			*/

#define _MAM_PROC_PRIVATE		/* proc private			*/ \
	struct frame*	bp;		/* proc frame base pointer	*/ \
	struct frame*	fp;		/* proc frame pointer		*/ \
	struct proc*	next;		/* next in list of all procs	*/ \
	struct proc*	stail;		/* sibling tail			*/

#define _MAM_MAM_PRIVATE		/* mam private			*/ \
	struct proc*	procs;		/* list of all procs		*/

#include <mam.h>

#endif
