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
	char	buf[1024], *s;
	int	n;
#ifdef DEBUG
	Sfio_t*	logf = sfopen(0,"LOG","a"); sfsetbuf(logf,NIL(Void_t*),0);
#endif

	alarm(10);
	if(argc > 1)
	{	/* coprocess only */
		while((s = sfreserve(sfstdin,SF_UNBOUND,0)) )
		{
#ifdef DEBUG
			sfwrite(logf, s, sfvalue(sfstdin));
#endif
			sfwrite(sfstdout, s, sfvalue(sfstdin));
		}
		return 0;
	}

	/* make coprocess */
	if(!(f = sfpopen(NIL(Sfio_t*), sfprints("%s -p",argv[0]), "r+")))
		terror("Opening for read/write");
	for(n = 0; n < 10; ++n)
	{	sfsprintf(buf,sizeof(buf),"Line %d",n);
		sfputr(f,buf,'\n');
		if(!(s = sfgetr(f,'\n',1)))
			terror("Did not read back line");
		if(strcmp(s,buf) != 0)
			terror("Input=%s, Expect=%s",s,buf);
	}

	if(sfputr(f,"123456789",'\n') != 10)
		terror("Bad write");

	if(sfread(f,buf,3) != 3)
		terror("Did not get data back");
	if(strncmp(s,"123",3) != 0)
		terror("Wrong data");

	if(sfwrite(f,"aaa",3) != 3 || sfputc(f,'\n') != '\n')
		terror("Fail on write");

	if(!(s = sfgetr(f,'\n',1)) )
		terror("Should have gotten 456789"); 
	if(strcmp(s,"456789") != 0)
		terror("Wrong data2");

	if(!(s = sfgetr(f,'\n',1)) )
		terror("Should have gotten aaa"); 
	if(strcmp(s,"aaa") != 0)
		terror("Wrong data3");

	sfclose(f);
	
	texit(0);
}
