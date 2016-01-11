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
	Sfio_t	*f;
	int	n, i;
	char	buf[50], *s, *string = "111\n222\n333";

	f = sfopen(NIL(Sfio_t*),string,"s");
	if(!(s = sfgetr(f,'\n',SF_STRING|SF_LOCKR)) || strcmp(s,"111") != 0)
		terror("sfgetr failed1");

	if(sfgetr(f,'\n',0) != NIL(char*))
		terror("sfgetr should have failed because of locking");
	sfread(f,s,1);

	if(!(s = sfgetr(f,'\n',SF_STRING)) || strcmp(s,"222") != 0)
		terror("sfgetr failed2");

	if((s = sfgetr(f,'\n',0)) != NIL(char*))
		terror("sfgetr should have failed because of partial record");

	if(!(s = sfgetr(f,0,SF_LASTR)) )
		terror("sfgetr should have succeeded getting partial record");

	/* test type == -1 and type == 1 modes */
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"111") != 0)
		terror("sfgetr failed in compatible mode");

	if(!(s = sfgetr(f,'\n',SF_STRING|SF_LOCKR)) || strcmp(s,"222") != 0)
		terror("sfgetr failed3");
	if(sfgetr(f,'\n',1) )
		terror("sfgetr should have failed due to locking");
	sfread(f,s,0);

	if(sfgetr(f,'\n',1) )
		terror("sfgetr should have failed because record is incomplete");
	
	if(!(s = sfgetr(f,0,-1)) || strcmp(s,"333") != 0)
		terror("sfgetr failed in getting last partial record");

	if(!(f = sftmp(0)) )
		terror("Can't open temporary stream");
	for(n = 0; n < 10; ++n) /* each record is 100 bytes */
	{	for(i = 0; i < 100; ++i)
			sfputc(f, 'a');
		sfputc(f,'\n');
	}
	sfseek(f,(Sfoff_t)0,0);
	sfsetbuf(f, buf, 50);
	if(!(s = sfgetr(f, '\n', 1)))
		terror("Can't get a record");

	n = 80;
	sfmaxr(n, 1); /* set maximum record size */
	if((i = sfmaxr(0, 0)) != n)
		terror("maxr is %d, expected %d", i, n);
	if((s = sfgetr(f, '\n', 1)) != 0)
		terror("Shouldn't have gotten a record");

	n = 0;
	sfmaxr(n, 1); /* no record size limit */
	if((i = sfmaxr(0, 0)) != n)
		terror("maxr is %d, expected %d", i, n);
	if(!(s = sfgetr(f, '\n', 1)))
		terror("Can't get a record");
	if(!(s = sfgetr(f, '\n', 1)))
		terror("Can't get a record");

	texit(0);
}
