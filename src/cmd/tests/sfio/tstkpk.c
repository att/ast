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

tmain()
{
	Sfio_t*	f;
	char		buf[1024];
	char*		s;
	int		fd[2];

	close(0);
	if(pipe(fd) < 0 || fd[0] != 0)
		terror("Making pipe");

	strcpy(buf,"1234567890");
	if(!(f = sfopen(NIL(Sfio_t*),buf,"s")))
		terror("Opening string stream");

	if(!sfstack(f,sfstdin))
		terror("Stacking");

	if(write(fd[1],"ab",2) != 2)
		terror("Writing ab to pipe");
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || sfvalue(f) != 2)
		terror("Peeking size1 = %d but should be 2", sfvalue(f));
	sfread(f,s,0);
	if(strncmp(s,"ab",2) != 0)
		terror("Wrong data1");

	if(write(fd[1],"cd",2) != 2)
		terror("Writing cd to pipe");
	close(fd[1]);
	if(!(s = sfreserve(f,4,0)) )
		terror("Peeking size2 = %d but should be 4", sfvalue(f));
	if(strncmp(s,"abcd",4) != 0)
		terror("Wrong data2");

	if(!(s = sfreserve(f,10,0)) )
		terror("Peeking size3 = %d but should be 10", sfvalue(f));
	if(strncmp(s,"1234567890",10) != 0)
		terror("Wrong data3");

	texit(0);
}
