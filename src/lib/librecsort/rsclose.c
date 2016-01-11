/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"rshdr.h"

/*	Close a sorting context
**
**	Written by Kiem-Phong Vo (07/18/96)
*/

#if __STD_C
int rsclose(Rs_t* rs)
#else
int rsclose(rs)
Rs_t*	rs;
#endif
{
	reg int	rv;

	while (rsdisc(rs, NIL(Rsdisc_t*), RS_POP));
	if ((rv = RSNOTIFY(rs,RS_CLOSE,0,0,rs->disc)) < 0)
		return rv;

	if(rs->vm)
		vmclose(rs->vm);
	if(rs->methdata)
		vmfree(Vmheap,rs->methdata);
	vmfree(Vmheap,rs);

	return 0;
}
