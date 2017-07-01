/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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

#include "format.h"

/*
 * pax delta format
 */

static Delta_format_t	data = { "94" };

Format_t	pax_delta_format =
{
	"delta",
	"delta94|vdelta",
	"vdelta difference/compression",
	DELTA_94,
	DELTA|DELTAIO|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(delta),
};

PAXLIB(delta)
