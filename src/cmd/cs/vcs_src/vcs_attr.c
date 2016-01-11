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
/*
 *	return attr of a sfile 
 */


#include "vcs_rscs.h"

int get_attr(f, ap)
	Sfio_t*		f;
	register attr_t*	ap;
{
	if (sfread(f, (char *)ap, sizeof(attr_t)) != sizeof(attr_t) || !ISRSCS(ap))
	{
		rserrno = NOTRSCS;
		return (-1);
	}

	return (0);
}


