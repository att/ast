/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
#include	"sftest.h"

/* test to see if files created in atexit functions work ok */

void ae()
{
	Sfio_t*	f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w");

	if(!f)
		terror("Can't create file");

	if(sfwrite(f,"1234\n",5) != 5)
		terror("Can't write to file");

	tcleanup();
}

#if __STD_C
main(int argc, char** argv)
#else
main(argc, argv)
int	argc;
char**	argv;
#endif
{
	Sfio_t* f;

	if(argc <= 1) /* atexit function registered after some sfio access */
	{	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 1), "w")) )
			terror("Can't create file");
		if(sfwrite(f,"1234\n",5) != 5)
			terror("Can't write to file");

		atexit(ae);

		system(sfprints("%s 1",argv[0]));
	}
	else /* atexit function registered before some sfio access */
	{	atexit(ae);

		if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 1), "w")) )
			terror("Can't create file");
		if(sfwrite(f,"1234\n",5) != 5)
			terror("Can't write to file");
	}

	texit(0);
}
