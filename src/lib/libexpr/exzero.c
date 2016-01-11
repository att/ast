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
 * expression library support
 */

#include "exlib.h"

/*
 * return 0 value for type
 */

Extype_t
exzero(int type)
{
	Extype_t	v;

	switch (type)
	{
	case FLOATING:
		v.floating = 0.0;
		break;
	case INTEGER:
	case UNSIGNED:
		v.integer = 0;
		break;
	case STRING:
		v.string = expr.nullstring;
		break;
	}
	return v;
}
