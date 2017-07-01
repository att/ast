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
 * return the next codex method
 * call with meth==0 to get the first method
 * plugins are included in the list
 */

#include <codex.h>
#include <dlldefs.h>

Codexmeth_t*
codexlist(Codexmeth_t* meth)
{
	register Codexmeth_t*	lp;
	Codexmeth_t*		np;
	Dllscan_t*		dls;
	Dllent_t*		dle;
	void*			dll;
	Codexlib_f		lib;

	if (!meth)
		return codexstate.first;
	if (!meth->next && !codexstate.scanned)
	{
		codexstate.scanned = 1;
		lp = meth;
		if (dls = dllsopen(codexstate.id, NiL, NiL))
		{
			while (dle = dllsread(dls))
				if (dll = dlopen(dle->path, RTLD_LAZY))
				{
					/* vcodex check works around obsolete vcodex<=>codex plugin */
					if ((lib = (Codexlib_f)dlllook(dll, "codex_lib")) && (np = (*lib)(dle->name)) && !streq(np->name, "vcodex"))
						for (lp = lp->next = np; lp->next; lp = lp->next);
					else
						dlclose(dll);
				}
				else
					message((-1, "%s: %s", dle->path, dlerror()));
			dllsclose(dls);
		}
	}
	return meth->next;
}
