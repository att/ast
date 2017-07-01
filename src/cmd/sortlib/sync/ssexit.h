/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * ibm dfsort user exit function interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _SSEXIT_H
#define _SSEXIT_H	1

#include <ast.h>
#include <error.h>
#include <recsort.h>

#define SS_EXIT_FIRST		0	/* the first exit record	*/
#define SS_EXIT_MOST		4	/* the middle exit records	*/
#define SS_EXIT_LAST		8	/* the last exit record		*/

#define SS_EXIT_ACCEPT		0	/* accept this record		*/
#define SS_EXIT_DELETE		4	/* delete this record		*/
#define SS_EXIT_CLOSE		8	/* don't call this exit again	*/
#define SS_EXIT_INSERT		12	/* insert this record		*/
#define SS_EXIT_TERMINATE	16	/* terminate the sort		*/
#define SS_EXIT_REPLACE		20	/* replace this record		*/

#ifndef _SS_H
typedef int (*Ssexit_f)(Rsobj_t*, Rsobj_t*, void**);
#endif

#endif
