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
 * some systems may pull in <ast_common.h> and its <ast_map.h>
 * which we are in the process of generating ... this prevents it
 */

int
main()
{
	printf("#pragma prototyped\n");
	printf("\n");
	printf("/*\n");
	printf(" * prototypes provided for standard interfaces hijacked\n");
	printf(" * by ast and mapped to _ast_* but already prototyped\n");
	printf(" * unmapped in native headers included by <ast_std.h>\n");
	printf(" */\n");
	printf("\n");
	return 0;
}
