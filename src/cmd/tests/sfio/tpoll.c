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
#if _sys_socket
#include	<sys/socket.h>
#endif

tmain()
{
	Sfio_t	*f, *g, *str, *fr, *fw, *sf[2];
	int	c;
	char	*s;
	int	fd[2];

	if(argc > 1)
	{	while((s = sfgetr(sfstdin, '\n', 1)) )
		{	sfputr(sfstdout, s, '\n');
			sfsync(sfstdout);
		}
		texit(0);
	}

	if(!(str = sfopen(NIL(Sfio_t*),"abc","s")) )
		terror("Opening string stream");

	if(pipe(fd) < 0)
		terror("pipe failed");

	if(!(fr = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,
			 fd[0],SF_READ)) )
		terror("Opening read pipe stream");
	if(!(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,
			 fd[1],SF_WRITE)) )
		terror("Opening write pipe stream");

	sf[0] = fr;
	sf[1] = str;
	if((c = sfpoll(sf,2,0)) != 1 || sf[0] != str)
		terror("Only str should be available c=%d",c);

	sf[0] = fr;
	if(sfpoll(sf,1,0) != 0 )
		terror("Pipe stream should not be ready");

	sfputc(fw,'a'); sfsync(fw);
	sf[0] = fr;
	if(sfpoll(sf,1,0) != 1 )
		terror("Pipe read should be ready");
	if((c = sfgetc(fr)) != 'a')
		terror("Didn't get back right data");

	sf[0] = fr;
	sf[1] = str;
	if(sfpoll(sf,2,0) != 1 || sf[0] != str)
		terror("Only str should be available2");

	sf[0] = fw;
	sf[1] = str;
	if(sfpoll(sf,2,0) != 2)
		terror("Both str&pipe write should be available");

	if(pipe(fd) < 0)
		terror("Can't create pipe");

	if(!(fr = sfnew(fr,NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ)) )
		terror("Can't create stream");

	if(write(fd[1],"0123456789",10) != 10)
		terror("Can't write to pipe");

	if(sfpoll(&fr,1,1000) != 1)
		terror("Data should be available");

	s = sfprints("%s 1", argv[0]);
	if(!(f = sfpopen(0, s, "w+")) )
		terror("Can't create read/write process");

	/* this write does not flush yet */
	if(sfwrite(f, "abc\n",4) != 4)
		terror("Writing to pipe");

	if(sfpoll(&f, 1, 0) != 1)
		terror("Poll should succeed");
	if(sfvalue(f)&SF_READ) /* data has not been flushed to the child yet */
		terror("Read should not be ready");
	if(!(sfvalue(f)&SF_WRITE) )
		terror("Write should be ready");

	if(sfsync(f) < 0) /* now flush data to the child process */
		terror("Bad sync");
	if(sfpoll(&f, 1, 1000) != 1)
		terror("Poll should succeed2");
	if(!(sfvalue(f)&SF_READ) ) /* the child should have read and rewritten */
		terror("Read should be ready");
	if(!(sfvalue(f)&SF_WRITE) )
		terror("Write should be ready");
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s, "abc") != 0)
		terror("Bad read");

#if _lib_socketpair
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) != 0)
		terror("socketpair failed");
	if(!(f = sfnew(0,NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ|SF_WRITE)) )
		terror("Can't create stream with socket file descriptor");
	if(!(g = sfnew(0,NIL(Void_t*),(size_t)SF_UNBOUND,fd[1],SF_READ|SF_WRITE)) )
		terror("Can't create stream with socket file descriptor");

	/* turn off write-capability for f */
	sfset(f,SF_WRITE,0);

	sf[0] = f;
	sf[1] = g;
	if(sfpoll(sf,2,0) != 1)
		terror("Exactly one stream should be ready!");
	if(sf[0] != g)
		terror("Stream g should be ready");
	if(sfvalue(g)&SF_READ)
		terror("Read should not be ready for g");
	if(!(sfvalue(g)&SF_WRITE) )
		terror("Write should be ready for g");

	if(sfwrite(g, "abc\n", 4) != 4  || sfsync(g) < 0)
		terror("Writing to g socket");
	if(sfpoll(sf, 2, 0) != 2)
		terror("Poll should succeed with both streams");
	if(!(sfvalue(f)&SF_READ) )
		terror("Read should be ready for f");

	if(sfgetc(f) != 'a' )
		terror("sfgetc failed");

	/* turn back on write-capability for f */
	sfset(f,SF_WRITE,1);

	if(sfwrite(f,"def\n",4) != 4 || sfsync(f) < 0)
		terror("Writing to f socket");

	if(sfpoll(sf, 2, 0) != 2)
		terror("Poll should succeed for both streams");
	if(!sfvalue(f)&SF_READ)
		terror("Read should be ready for f");
	if(!sfvalue(g)&SF_READ)
		terror("Read should be ready for g");

	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"bc") != 0)
		terror("f gets wrong data");

	if(!(s = sfgetr(g,'\n',1)) || strcmp(s,"def") != 0)
		terror("g gets wrong data");

	if(sfpoll(sf, 2, 0) != 2)
		terror("Poll should succeed for both streams");
	if(sfvalue(f)&SF_READ)
		terror("Read should not be ready for f");
	if(sfvalue(g)&SF_READ)
		terror("Read should not be ready for g");
#endif

	texit(0);
}
