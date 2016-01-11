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
	Sfio_t	*f1, *f2;
	char*	s;
	Sfoff_t	p;
	char	buf[1024];
	int	r, w;

	if(!(f1 = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w")) )
		terror("Can't open f1");
	if(!(f1 = sfopen(f1, tstfile("sf", 0), "a+")) )
		terror("Can't open f1");

	if(!(f2 = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "a+")) )
		terror("Can't open f2");

	if(sfwrite(f1,"012345678\n",10) != 10 || sfsync(f1) < 0)
		terror("Writing to f1");
	if((p = sftell(f1)) != 10)
		terror("Bad sftell1 %ld",p);

	if(sfwrite(f2,"abcdefghi\n",10) != 10 || sfsync(f2) < 0)
		terror("Writing to f2");
	if((p = sftell(f2)) != 20)
		terror("Bad sftell2");

	if((p = sfseek(f1,(Sfoff_t)0,0)) != 0)
		terror("Bad seek");
	if(!(s = sfgetr(f1,'\n',1)) )
		terror("Bad getr1");
	if(strcmp(s,"012345678") != 0)
		terror("Bad input1");

	if((p = sftell(f1)) != 10)
		terror("Bad sftell3");

	if(sfwrite(f1,"012345678\n",10) != 10 || sfsync(f1) < 0)
		terror("Writing to f1");
	if((p = sftell(f1)) != 30)
		terror("Bad sftell4");

	if((p = sfseek(f2,(Sfoff_t)10,0)) != 10)
		terror("Bad seek");
	if(!(s = sfgetr(f2,'\n',1)) )
		terror("Bad getr2");
	if(strcmp(s,"abcdefghi") != 0)
		terror("Bad input2");

	if(!(s = sfgetr(f2,'\n',1)) )
		terror("Bad getr3");
	if(strcmp(s,"012345678") != 0)
		terror("Bad input3");

	if(!(f1 = sfopen(f1, tstfile("sf", 0), "w")) )
		terror("Can't open file to write");
	for(r = 0; r < 1024; ++r)
		buf[r] = 'a';
	if((w = sfwrite(f1,buf,1024)) != 1024)
		terror("writing w=%d", w);
	if(!(f1 = sfopen(f1, tstfile("sf", 0), "a")) )
		terror("Can't open file to append");
	sfseek(f1,(Sfoff_t)0,0);
	if((w = sfwrite(f1,buf,64)) != 64)
		terror("writing w=%d", w);
	if((r = (int)sftell(f1)) != (1024+64) )
		terror("seek position wrong s=%d", r);

	texit(0);
}
