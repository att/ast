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

/*	Discipline event notification.
*/

#if __STD_C
int rsnotify(Rs_t* rs, int op, Void_t* data, Void_t* arg, reg Rsdisc_t* disc)
#else
int rsnotify(rs, op, data, arg, disc)
Rs_t*		rs;
int		op;
Void_t*		data;
Void_t*		arg;
reg Rsdisc_t*	disc;
#endif
{
	int	r;

	r = 0;
	if (rs->events & op)
		for (; disc; disc = disc->disc)
			if ((disc->events & op) &&
			    (r = (*disc->eventf)(rs, op, data, arg, disc)))
				break;
	return r;
}
