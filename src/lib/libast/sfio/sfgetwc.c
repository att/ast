/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"sfhdr.h"

/*	Read the next multibyte character from f and return the corresponding rune (wide char.)
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
int sfgetwc(Sfio_t* f)
#else
int sfgetwc(f)
Sfio_t*	f;
#endif
{
	uchar		*s;
	char		*e, *b;
	int		c, n, m, i;
	char		buf[32];
	SFMTXDECL(f); /* declare a local stream variable for multithreading */

	SFMTXENTER(f, -1);

	if(f->mode != SF_READ && _sfmode(f,SF_READ,0) < 0)
		SFMTXRETURN(f, -1);

	SFLOCK(f,0);

	if(SFRPEEK(f,s,n) <= 0)
	{	f->flags |= SF_ERROR;
		c = -1;
	}
	else if (!mbwide())
	{
		c = *s++;
		f->next = s;
	}
	else
	{
		wchar_t		w;
		SFMBDCLP(q)

		q = SFMBSTATE(f);
		c = mbchar(&w, s, n, q);
		if (!mberrno(q))
			f->next = s;
		else if (n < (m = mbmax()))
		{	for(i = 0; i < n; i++)
				buf[i] = *s++;
			for(;;)
			{	f->next = s;
				if(SFRPEEK(f,s,n) <= 0)
				{	f->flags |= SF_ERROR;
					break;
				}
				if(n > (m - i))
					n = m - i;
				for(; i < n; i++)
					buf[i] = *s++;
				e = buf + i;
				b = buf;
				c = mbchar(&w, b, n, q);
				if (!mberrno(q))
				{	f->next = s - (e - b);
					break;
				}
				if(i >= m || mberrno(q) == EILSEQ)
				{	f->flags |= SF_ERROR;
					break;
				}
			}
		}
	}
	SFOPEN(f,0);
	SFMTXRETURN(f, c);
}
