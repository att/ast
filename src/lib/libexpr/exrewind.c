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
 * AT&T Research
 *
 * expression library
 */

#include "exlib.h"

int
exrewind(Expr_t* ex)
{
	register int	n;

	if (ex->linewrap)
	{
		exerror("too much pushback");
		return -1;
	}
	if (!ex->input->pushback && !(ex->input->pushback = oldof(0, char, sizeof(ex->line), 3)))
	{
		exnospace();
		return -1;
	}
	if (n = ex->linep - ex->line)
		memcpy(ex->input->pushback, ex->line, n);
	if (ex->input->peek)
	{
		ex->input->pushback[n++] = ex->input->peek;
		ex->input->peek = 0;
	}
	ex->input->pushback[n++] = ' ';
	ex->input->pushback[n] = 0;
	ex->input->pp = ex->input->pushback;
	ex->input->nesting = ex->nesting;
	setcontext(ex);
	return 0;
}

void
exstatement(Expr_t* ex)
{
	ex->nesting = ex->input->nesting;
	setcontext(ex);
}
