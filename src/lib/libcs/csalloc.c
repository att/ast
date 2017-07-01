/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * allocate cs state context
 */

#include "cslib.h"

Cs_t*
csalloc(Csdisc_t* disc)
{
	register Cs_t*	state;

	if (state = newof(0, Cs_t, 1, 0))
	{
		state->id = cs.id;
		state->disc = disc ? disc : cs.disc;
#if _stream_peek
		state->nostream = -1;
#endif
#if _socket_peek
		state->nosocket = -1;
#endif
	}
	return state;
}
