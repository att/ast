/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
 * identify encoding given first block of encoded data
 * not all encodings are expected to self-identify
 */

#include <codex.h>

Codexmeth_t*
codexid(const void* head, size_t headsize, char* name, size_t namesize)
{
	register Codexmeth_t*	meth;

	for (meth = codexlist(NiL); meth; meth = codexlist(meth))
		if (meth->identf && (*meth->identf)(meth, head, headsize, name, namesize))
			return meth;
	return 0;
}
