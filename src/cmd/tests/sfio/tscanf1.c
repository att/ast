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

/* test to see sfvscanf still works ok with a discipline that
** returns one byte at a time.
*/

typedef struct _mydisc_s
{
	Sfdisc_t	disc;
	Sfio_t*		f;
} Mydisc_t;

#if __STD_C
static ssize_t oneread(Sfio_t* f, Void_t* buf, size_t size, Sfdisc_t* disc)
#else
static ssize_t oneread(f, buf, size, disc)
Sfio_t*		f;
Void_t*		buf;
size_t		size;
Sfdisc_t*	disc;
#endif
{
	Mydisc_t	*dc = (Mydisc_t*)disc;

	if(!f || size <= 0)
		return -1;
	return sfread(dc->f, buf, 1);
}

static Mydisc_t	Mydisc;

tmain()
{
	int		n;
	double		d;
	int		i;
	char		s[10];
	Sfio_t		*f, *str;

	if((n = sfsscanf("ten 10 10", "%s %d %lf", s, &i, &d)) != 3)
		terror("Bad scanning %d", n);
	if(strncmp("ten",s,3) != 0 || i != 10 || d != 10)
		terror("Bad scanned values ");

	if(!(f = sfnew((Sfio_t*)0, (Void_t*)0, (size_t)SF_UNBOUND, 0, SF_READ) ) )
		terror("Can't create stream f");
	if(!(str = sfopen(0, "ten 10 10", "s")) )
		terror("Can't open string stream str");
	Mydisc.disc.readf = oneread;
	Mydisc.f = str;
	sfdisc(f,&Mydisc.disc);

	s[0] = 0; i = 0; d = 0;
	if((n = sfscanf(f, "%s %d %lf", s, &i, &d)) != 3)
		terror("Bad scanning %d", n);
	if(strncmp("ten",s,3) != 0 || i != 10 || d != 10)
		terror("Bad scanned values2");

	return 0;
}
