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

static int	Count = 0;
static Sfdisc_t	Disc;

static char	Rec[] = "0";

#if __STD_C
void count(Sfio_t* f, int type, void* data)
#else
void count(f, type, data)
Sfio_t* f;
int type;
void* data;
#endif
{
	int	fd = integralof(data);

	if(fd >= 0)
		Count += 1;
}

#define TEST_BUFSIZE	(1024*1024)

tmain()
{
	Sfio_t*	f;
	char*	s;
	ssize_t	siz;
	Sfoff_t	pos;
	Sfoff_t	nxt;
	int	pid;
	char*	buf;

	if(!(buf = (char*)malloc(TEST_BUFSIZE)))
		terror("malloc(%ld) failed", TEST_BUFSIZE);
	/* test to see if transforming to file is ok with sfwrite */
	memset(buf,1,TEST_BUFSIZE);
	if(!(f = sftmp(1024)) )
		terror("sftmp failed");
	if((siz = sfwrite(f,buf,TEST_BUFSIZE)) != TEST_BUFSIZE)
		terror("sfwrite failed with siz=%ld",siz);

	/* ast ed does this */
	if (!(f = sftmp(SF_BUFSIZE)))
		terror("sftmp");
	if (pos = sfseek(f, (Sfoff_t)0, SEEK_CUR))
		terror("top offset %I*d expected 0", sizeof(pos), pos);
	if ((siz = sfputr(f, Rec, 0)) != sizeof(Rec))
		terror("put record size %I*d expected %d",
			sizeof(siz), siz, sizeof(Rec));
	if ((nxt = sfseek(f, (Sfoff_t)0, SEEK_CUR)) != (pos + siz))
		terror("put record size %I*d offset %I*d expected %I*d",
			sizeof(siz), siz, sizeof(nxt), nxt,
			sizeof(nxt), nxt + sizeof(Rec));
	if ((pos = sfseek(f, (Sfoff_t)SF_BUFSIZE, SEEK_CUR)) != (nxt + SF_BUFSIZE))
		terror("skip block size %d offset %I*d expected %I*d",
			SF_BUFSIZE, sizeof(nxt), nxt, sizeof(nxt), nxt + SF_BUFSIZE);
	sfclose(f);

	/* let two run concurrently */
	if((pid = fork()) < 0)
		return 0;

	f = sftmp((size_t)SF_UNBOUND);

	sfputr(f,"1234",'\n');	/* write a string into it */
	sfseek(f,(Sfoff_t)0,0);		/* get back so we can read the string */
	s = sfreserve(f,SF_UNBOUND,0);
	if(sfvalue(f) != 5)
		terror("Get n=%d, expect n=5", sfvalue(f));

	sfseek(f,(Sfoff_t)10,1);	/* seek to extend buffer */
	if(s = sfreserve(f,SF_UNBOUND,0))
		terror("Get n=%d, expect n=0", sfvalue(f));

	sfset(f,SF_READ,0);	/* turn off read mode so stream is write only */

	sfseek(f,(Sfoff_t)(-10),1);	/* back 10 places to get space to write */
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || sfwrite(f,s,0) != 0)
		terror("Get n=%d, expect n > 0", sfvalue(f));
	strcpy(s,"5678\n");

	sfset(f,SF_READ,1);
	sfseek(f,(Sfoff_t)0,0);		/* read 1234\n5678\n */
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || sfread(f,s,0) != 0)
		terror("Get n=%d, expect n > 0", sfvalue(f));
	if(strncmp(s,"1234\n5678\n",10) != 0)
		terror("Get wrong string");
	sfclose(f);

	sfnotify(count);
	if(!(f = sftmp(0)) )
		terror("sftmp");
	if(Count != 1)
		terror("wrong count 1, count=%d", Count);
	sfclose(f);
	if(Count != 2)
		terror("wrong count 2 count=%d", Count);

	if(!(f = sftmp(8)) )
		terror("sftmp");
	if(Count != 2)
		terror("wrong count 2.2 count=%d", Count);
	sfdisc(f,&Disc);
	if(Count != 3)
		terror("wrong count 3 count=%d", Count);
	sfclose(f);
	if(Count != 4)
		terror("wrong count 4 count=%d", Count);

	if(!(f = sftmp(8)) )
		terror("sftmp");
	if(Count != 4)
		terror("wrong count 4.2 count=%d", Count);
	sfwrite(f,"0123456789",10);
	if(Count != 5)
		terror("wrong count 5 count=%d", Count);
	sfclose(f);
	if(Count != 6)
		terror("wrong count 6 count=%d", Count);

	if(!(f = sftmp(1024)) )
		terror("sftmp");
	sfwrite(f,"1234567890",10);
	sfseek(f,(Sfoff_t)0,0);
	if(sfsize(f) != 10)
		terror("Wrong size");
	sfdisc(f,SF_POPDISC);
	if(sfsize(f) != 10)
		terror("Wrong size");
	s = sfreserve(f,SF_UNBOUND,0);
	if(sfvalue(f) != 10 || strncmp(s,"1234567890",10) != 0)
		terror("did not create correct real file");

	if(pid != 0)
		wait(&pid);

	texit(0);
}
