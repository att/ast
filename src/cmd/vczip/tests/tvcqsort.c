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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"vctest.h"

int	ilist[] = {3, 4, 4, 5, 1, 1, 2, 2, 7, 6};
int	isort[] = {1, 1, 2, 2, 3, 4, 4, 5, 6, 7};

int intcmp(Void_t* one, Void_t* two, Void_t* disc)
{
	int	i1 = *((int*)one), i2 = *((int*)two);
	return i1 - i2;
}

MAIN()
{
	int	i;

	vcqsort(ilist, sizeof(ilist)/sizeof(int), sizeof(int), intcmp, 0);
	for(i = 0; i < sizeof(ilist)/sizeof(int); ++i)
	{	if(ilist[i] != isort[i])
			terror("vcqsort() failed");
	}

	exit(0);
}
