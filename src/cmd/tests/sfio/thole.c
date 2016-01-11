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

/*	Test for /dev/null and hole-preserving code */

tmain()
{
	Sfio_t*		null;
	Sfio_t*		f;
	int		k, n;
	static char	buf[256*1024], b[256*1024];

	if(!(null = sfopen(NIL(Sfio_t*),"/dev/null","w")) )
		terror("Opening /dev/null");

	sfsetbuf(null,NIL(char*),(size_t)SF_UNBOUND);

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w+")) )
		terror("Creating %s", tstfile("sf", 0));
	sfwrite(f,"1234",4);
	sfseek(f,(Sfoff_t)1,0);
	sfsync(f);

	sfsetfd(null,-1);
	sfsetfd(null,sffileno(f));
	sfsync(null);

	sfseek(f,(Sfoff_t)0,0);
	if(sfread(f,buf,4) != 4 || strncmp(buf,"1234",4) != 0)
		terror("Bad data");

	for(k = 0; k < sizeof(buf); ++k)
		buf[k] = 1;
	for(k = sizeof(buf)/4; k < sizeof(buf)/2; ++k) /* make a big hole */
		buf[k] = 0;

	if(!(f = sfopen(f, tstfile("sf", 0), "w+")) )
		terror("Creating %s", tstfile("sf", 0));
	n = sizeof(buf)-127;
	if(sfwrite(f,buf,n) != n)
		terror("Writing large buffer");
	sfseek(f,(Sfoff_t)0,0);
	if(sfread(f,b,n) != n)
		terror("Reading large buffer");
	for(k = 0; k < n; ++k)
		if(b[k] != buf[k])
			terror("Bad data");

	texit(0);
}
