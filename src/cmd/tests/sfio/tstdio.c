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
#if _PACKAGE_ast
#include	"sftest.h"
#include 	<stdio.h>
#else
#include	"sftest.h"
#include	"../Stdio_s/stdio.h"
#endif

/* test compliance of certain stdio behaviors */
tmain()
{
	FILE	*f, *f2;
	long	s1, s2;
	int	i, k, fd;
	char	buf[128*1024], rbuf[1024], *sp;

	if(argc > 1)
	{	if(sfwrite(sfstdout,argv[1],strlen(argv[1])) != strlen(argv[1]))
			terror("Can't write to stdout");
		sfsync(sfstdout);
		return 0;
	}

	/* test for shared streams and seek behavior */
	if(!(f = fopen(tstfile("sf", 0),"w+")) )
		terror("Opening file to read&write");

	/* change stdout to a dup of fileno(f) */
	fd = dup(1); close(1); dup(fileno(f));

	/* write something to the dup file descriptor */
	system(sfprints("%s 0123456789", argv[0]));

	/* change stdout back */
	close(1); dup(fd); close(fd);

	/* this fseek should reset the stream back to where we can read */
	fseek(f, 0L, SEEK_SET);

	/* see if data is any good */
	fread(buf, sizeof(buf), 1, f);
	for(i = 0; i < 10; ++i)
		if(buf[i] != '0'+i)
			terror("Bad data0");

	/* construct a bunch of lines and out put to f */
	sp = buf;
	for(k = 0; k < sizeof(buf)/10; ++k)
	{	for(i = 0; i < 9; ++i)
			*sp++ = '0' + i;
		*sp++ = '\n';
	}

	/* write out a bunch of thing */
	fseek(f, 0L, SEEK_SET);
	if(fwrite(buf, sizeof(buf), 1, f) != 1)
		terror("Writing data");

	if((fd = dup(fileno(f))) < 0)
		terror("Can't dup file descriptor");
	if(!(f2 = fdopen(fd, "r")) )
		terror("Can't create stream");

	/* read a few bytes from this dup stream */
	fseek(f2, 0L, SEEK_SET);
	rbuf[0] = 0;
	if(fread(rbuf, 1, 7, f2) != 7)
		terror("Bad read");
	for(i = 0; i < 7; ++i)
		if(rbuf[i] != '0'+i)
			terror("Bad data1");

	if((s2 = ftell(f2)) != 7)
		terror("Bad tell location in f2");

	/* now seek way off on f */
	fseek(f, 1005L, SEEK_SET);
	rbuf[0] = 0;
	fread(rbuf, 5, 1, f);
	for(i = 5; i < 9; ++i)
		if(rbuf[i-5] != '0'+i)
			terror("Bad data2");
	if(rbuf[i-5] != '\n')
		terror("Bad data: did not get new-line");
	if((s1 = ftell(f)) != 1010)
		terror("Bad location in f: s1=%lld", (Sflong_t)s1);

	fseek(f, 0L, SEEK_CUR); /* switch mode so we can write */
	if(fputc('x',f) < 0)
		terror("fputc failed");
	if(fflush(f) < 0)
		terror("fflush failed");
	if((s1 = ftell(f)) != 1011)
		terror("Bad location in f: s1=%lld", (Sflong_t)s1);
	fseek(f, -1L, SEEK_CUR); /* set the seek location in the file descriptor */

	fflush(f2); /* assuming POSIX conformance and to set seek location to 1010 */
	if((s2 = ftell(f2)) != 1010)
		terror("Bad location in f2: s2=%lld", (Sflong_t)s2);

	fread(rbuf, 10, 1, f2);
	if(rbuf[0] != 'x')
		terror("Didn't get x");
	for(i = 1; i < 9; ++i)
		if(rbuf[i] != '0'+i)
			terror("Bad data3");
	if(rbuf[i] != '\n')
		terror("Did not get new-line");

	texit(0);
}
