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


/*	Test multiple processes reading/writing from same file
**	descriptor.
*/
tmain()
{
	char*	s;

	if(argc > 1)
	{	if(strcmp(argv[1],"-r") == 0)	/* doing sfgetr */
		{	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line2") != 0)
				terror("Coprocess getr did not get Line2");
			if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line3") != 0)
				terror("Coprocess getr did not get Line3");
		}
		else	/* doing sfmove */
		{	Sfio_t*	f = sfopen(NIL(Sfio_t*),NIL(char*),"swr");
			if(!f)
				terror("Can't open string stream");
			if(sfmove(sfstdin,f,(Sfoff_t)2,'\n') != 2)
				terror("Coprocess sfmove failed");
			sfseek(f,(Sfoff_t)0,0);
			if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"Line2") != 0)
				terror("Coprocess move did not get Line2");
			if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"Line3") != 0)
				terror("Coprocess move did not get Line3");
		}
		texit(0);
	}

	if(sfopen(sfstdout, tstfile("sf", 0), "w") != sfstdout )
		terror("Opening file");
	if(sfputr(sfstdout,"Line1",'\n') < 0 ||
	   sfputr(sfstdout,"Line2",'\n') < 0 ||
	   sfputr(sfstdout,"Line3",'\n') < 0 ||
	   sfputr(sfstdout,"Line4",'\n') < 0)
		terror("Writing data");
	sfopen(sfstdout,"/dev/null","w");

	/* testing coprocess calling sfgetr */
	if(sfopen(sfstdin, tstfile("sf", 0),"r") != sfstdin)
		terror("Opening to read");
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line1") != 0)
		terror("Did not get Line1 for sfgetr");
	sfsync(sfstdin);
	system(sfprints("%s -r",argv[0]));
	sfseek(sfstdin, (Sfoff_t)lseek(sffileno(sfstdin), (off_t)0, 1), 0);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line4") != 0)
		terror("Did not get Line4 for sfgetr");

	/* testing coprocess calling sfmove */
	if(sfopen(sfstdin, tstfile("sf", 0), "r") != sfstdin)
		terror("Opening to read");
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line1") != 0)
		terror("Did not get Line1 for sfmove");
	sfsync(sfstdin);
	system(sfprints("%s -m",argv[0]));
	sfseek(sfstdin, (Sfoff_t)lseek(sffileno(sfstdin), (off_t)0, 1), 0);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line4") != 0)
		terror("Did not get Line4 for sfmove");

	/* testing the head program */
#ifdef HEAD
	if(sfopen(sfstdin, tstfile("sf", 0), "r") != sfstdin)
		terror("Opening to read");
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line1") != 0)
		terror("Did not get Line1 for head");
	sfsync(sfstdin);
	system("head -2 > /dev/null"); /* for testing the head program */
	sfseek(sfstdin, (Sfoff_t)lseek(sffileno(sfstdin), (off_t)0, 1), 0);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line4") != 0)
		terror("Did not get Line4 for head");
#endif

	texit(0);
}
