/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2012 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                       Jeff Korn <@google.com>                        *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "tksh.h"

static char **av;
static int ac;

static void tksh_userinit(Shell_t* shp, int subshell)
{
	char *end = av[0] + strlen(av[0]);
	int len = strlen(av[0]);
	char *args[2];
	Namval_t *np;

	if(np = nv_open("source",shp->alias_tree,NV_NOADD))
	{
		nv_unset(np);
		nv_close(np);
	}
	if (subshell < 0)
	{
		if(nv_open("tkloop",shp->fun_tree,NV_NOADD))
			sh_trap("tkloop", 0);
		return;
	}
	else if (subshell > 0)
	{
		TkshSubShell();
		return;
	}

#ifndef DO_TK
	/* sfsetbuf(sfstdout, NULL, 0); */
	args[0] = av[0]; args[1] = NULL;
	if ((len >= 4) && (strcmp(end-4, "tksh") == 0))
		/* b_tkinit(0, (char **) 0, (void *) 0); */
		b_tkinit(1, args, (Shbltin_t *) 0);
	else if ((len >= 6) && (strcmp(end-6, "tclksh") == 0))
                /* b_tclinit(0, (char **) 0, (void *) 0); */
                b_tclinit(1, args, (Shbltin_t *) 0);
	else
	{
		sh_addbuiltin("tclinit", b_tclinit, (void *) 0);
		sh_addbuiltin("tkinit", b_tkinit, (void *) 0);
	}
#else
	sh_addbuiltin("tkinit", b_tkinit, (void *) 0);
	if ((len >= 6) && (strcmp(end-6, "tclksh") == 0))
                b_tclinit(0, (char **) 0, (Shbltin_t *) 0);
	else
		sh_addbuiltin("tclinit", b_tclinit, (void *) 0);
#endif
}

int main(int argc, char *argv[])
{
	return (sh_main(ac=argc, av=argv, tksh_userinit));
}
