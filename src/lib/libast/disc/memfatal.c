/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * install error message handler for fatal malloc exceptions
 */

#define _AST_API_IMPLEMENT	1

#include <ast.h>
#include <error.h>
#include <vmalloc.h>

#include "FEATURE/vmalloc"

#if _std_malloc

void
memfatal(void)
{
}

#else

/*
 * print message and fail on VM_BADADDR,VM_NOMEM
 */

static int
nomalloc(Vmalloc_t* region, int type, void* obj, Vmdisc_t* disc)
{
	Vmstat_t	st;

	NoP(disc);
	switch (type)
	{
	case VM_NOMEM:
		vmstat(region, &st);
		error(ERROR_SYSTEM|3, "storage allocator out of space on %zu byte request ( region %zu segments %zu busy %zu:%zu free %zu:%zu )", (size_t)obj, st.extent, st.n_seg, st.n_busy, st.s_busy, st.n_free, st.s_free);
		return -1;
#ifdef VM_BADADDR
	case VM_BADADDR:
		error(ERROR_SYSTEM|3, "invalid pointer %p passed to free or realloc", obj);
		return -1;
#endif
	}
	return 0;
}

void
memfatal(void)
{
	(void)memfatal_20130509(NiL);
}

#undef	_AST_API_IMPLEMENT

#include <ast_api.h>

/*
 * initialize the malloc fatal exception handler for disc
 */

int
memfatal_20130509(Vmdisc_t* disc)
{
	if (!disc)
	{
		malloc(0);
		if (!(disc = vmdisc(Vmregion, NiL)))
			return -1;
	}
	disc->exceptf = nomalloc;
	return 0;
}

#endif
