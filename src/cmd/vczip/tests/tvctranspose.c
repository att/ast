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

#define NCOLS	1600
#define NROWS	2000

MAIN()
{
	int		i, j;
	Vcchar_t	matrix[NROWS][NCOLS], trans[NCOLS][NROWS], *tr;
	Vcodex_t	*vc;

	for(i = 0; i < NROWS; ++i)
	for(j = 0; j < NCOLS; ++j)
	{	matrix[i][j] = 'a' + (i+j)%26;
		trans[j][i] = 'a' + (i+j)%26;
	}

	if(!(vc = vcopen(0, Vctranspose, "0", 0, VC_ENCODE)) )
		terror("Cannot open Vctranspose handle");

	vcsetmtarg(vc, "columns", (Void_t*)1600, 2);
	if((i = vcapply(vc, matrix, sizeof(matrix), &tr)) != sizeof(matrix) )
		terror("Vctranspose failed");

	if(memcmp(&trans[0][0], tr, sizeof(trans)) != 0)
		terror("Bad data");

	exit(0);
}
