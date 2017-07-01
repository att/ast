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

/*
 * return the expression for name or sym coerced to type
 */

Exnode_t*
exexpr(Expr_t* ex, const char* name, Exid_t* sym, int type)
{
	if (ex)
	{
		if (!sym)
			sym = name ? (Exid_t*)dtmatch(ex->symbols, name) : &ex->main;
		if (sym && sym->lex == PROCEDURE && sym->value)
		{
			if (type != DELETE)
				return excast(ex, sym->value->data.procedure.body, type, NiL, 0);
			exfreenode(ex, sym->value);
			sym->lex = NAME;
			sym->value = 0;
		}
	}
	return 0;
}
