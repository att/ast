/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
 * find and load a pzip dll
 */

#include "pzlib.h"

#include <dlldefs.h>

int
pzlib(register Pz_t* pz, register const char* name, int ignore)
{
	register Pzdll_t*	dll;
	register Pzdllpz_t*	pzs;
	char*			id;
	char			buf[64];
	char			path[PATH_MAX];

	if (id = strrchr(state.id, ':'))
		id++;
	else
		id = (char*)state.id;

	/*
	 * see if the dll is already loaded
	 */

	for (dll = state.dll; dll && !streq(dll->name, name); dll = dll->next);
	if (!dll)
	{
		/*
		 * add to the list and open
		 */

		if (!(dll = newof(0, Pzdll_t, 1, strlen(name) + 1)))
			return -1;
		dll->name = strcpy((char*)(dll + 1), name);
		dll->next = state.dll;
		state.dll = dll;
		if (ignore)
			return 0;
		if (!(dll->dll = dllplugin(id, dll->name, NiL, PZ_PLUGIN_VERSION, NiL, RTLD_LAZY, path, sizeof(path))))
		{
			if (pz->disc && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: %s", dll->name, dlerror());
			return -1;
		}

		/*
		 * get the initialization function
		 */

		sfsprintf(buf, sizeof(buf), "%s_init", id);
		if (!(dll->initf = (Pzinit_f)dlllook(dll->dll, buf)))
		{
			if (pz->disc && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: %s: initialization function not found in library", path, buf);
			return -1;
		}
	}

	/*
	 * see if pz already initialized
	 */

	if (dll->initf)
	{
		for (pzs = dll->pzs; pzs && pzs->pz != pz; pzs = pzs->next);
		if (!pzs)
		{
			if (!(pzs = newof(0, Pzdllpz_t, 1, 0)))
				return -1;
			pzs->pz = pz;
			pzs->next = dll->pzs;
			dll->pzs = pzs;
			if (!(dll->usage = pzinit(pz, dll->name, dll->initf)))
				return -1;
		}
	}
	return 0;
}
