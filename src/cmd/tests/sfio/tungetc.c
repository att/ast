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
	char	*str, *alpha, *s;
	char	buf[128];
	int	n;

	str = "0123456789";
	alpha = "abcdefghijklmnop";

	if(!(f = sfopen((Sfio_t*)0,alpha,"s")))
		terror("Opening stream");

	for(n = 9; n >= 0; --n)
		if(sfungetc(f,n+'0') != n+'0')
			terror("Ungetc");

	if(!(s = sfreserve(f,SF_UNBOUND,0)) || sfvalue(f) != 10)
		terror("Peek stream1");
	if(strncmp(s,str,10) != 0)
		terror("Bad data1");

	if(!(s = sfreserve(f,SF_UNBOUND,0)) || sfvalue(f) != (ssize_t)strlen(alpha))
		terror("Peek stream2");
	if(strncmp(s,alpha,strlen(alpha)) != 0)
		terror("Bad data2");

	sfseek(f,(Sfoff_t)0,0);
	for(n = 9; n >= 0; --n)
		if(sfungetc(f,n+'0') != n+'0')
			terror("Ungetc2");
	if(sfgetc(f) != '0')
		terror("Sfgetc");
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,SF_UNBOUND,0)) || sfvalue(f) != (ssize_t)strlen(alpha))
		terror("Peek stream3");
	if(strncmp(s,alpha,strlen(alpha)) != 0)
		terror("Bad data2");

	sfseek(f,(Sfoff_t)0,0);
	if(sfungetc(f,'0') != '0')
		terror("Ungetc3");

	strcpy(buf,"0123456789\n");
	if(!(f = sfopen(f,buf,"s+")) )
		terror("Reopening  string");
	if(sfungetc(f,'\n') != '\n')
		terror("Can't unget new-line2");
	if(sfungetc(f,'d') != 'd')
		terror("Can't unget d");
	if(sfungetc(f,'c') != 'c')
		terror("Can't unget c");
	if(sfungetc(f,'\n') != '\n')
		terror("Can't unget new-line");
	if(sfungetc(f,'b') != 'b')
		terror("Can't unget b");
	if(sfungetc(f,'a') != 'a')
		terror("Can't unget a");

	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"ab") != 0)
		terror("Did not get ab");
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"cd") != 0)
		terror("Did not get cd");
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"0123456789") != 0)
		terror("Did not get 0123456789");

	texit(0);
}
