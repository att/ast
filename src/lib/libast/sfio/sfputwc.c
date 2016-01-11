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

/*	Write out a rune (wide char) as a multibyte char on f.
**
**	Written by Kiem-Phong Vo.
*/

/*
 * we use an almost empty discipline to keep the stream mb state
 * the discpline is identified by the private _sfmbexcept address
 */

typedef struct Sfmbstate_s
{
	Sfdisc_t	disc;
	Mbstate_t	mbs;
} Sfmbstate_t;

#if __STD_C
static int _sfmbexcept(Sfio_t* f, int type, Void_t* arg, Sfdisc_t* disc)
#else
static int _sfmbexcept(f, op, arg, disc)
Sfio_t*		f;
int		op;
Void_t*		arg;
Sfdisc_t*	disc;
#endif
{
	if (type == SF_DPOP || type == SF_FINAL)
		free(disc);
	return 0;
}

#if __STD_C
Mbstate_t* _sfmbstate(Sfio_t* f)
#else
Mbstate_t* _sfmbstate(f)
Sfio_t*		f;
#endif
{
	Sfdisc_t*	disc;
	Sfmbstate_t*	mbs;

	for (disc = f->disc; disc; disc = disc->disc)
		if (disc->exceptf == _sfmbexcept)
			return &((Sfmbstate_t*)disc)->mbs;
	if (mbs = newof(0, Sfmbstate_t, 1, 0))
	{
		mbs->disc.exceptf = _sfmbexcept;
		sfdisc(f, &mbs->disc);
	}
	return &mbs->mbs;
}

#if __STD_C
int sfputwc(Sfio_t* f, int w)
#else
int sfputwc(f,w)
Sfio_t*		f;	/* write a portable ulong to this stream */
int		w;	/* the unsigned value to be written */
#endif
{
	reg uchar	*s;
	reg char	*b;
	int		n, m;
	char		buf[32];
	SFMTXDECL(f);

	SFMTXENTER(f, -1);

	if(f->mode != SF_WRITE && _sfmode(f,SF_WRITE,0) < 0)
		SFMTXRETURN(f, -1);
	SFLOCK(f,0);

	n = mbtconv(buf, w, SFMBSTATE(f));

	if(n > 8 || SFWPEEK(f,s,m) < n)
		n = SFWRITE(f,(Void_t*)s,n); /* write the hard way */
	else
	{	b = buf;
		switch(n)
		{
		case 8 : *s++ = *b++;
		case 7 : *s++ = *b++;
		case 6 : *s++ = *b++;
		case 5 : *s++ = *b++;
		case 4 : *s++ = *b++;
		case 3 : *s++ = *b++;
		case 2 : *s++ = *b++;
		case 1 : *s++ = *b++;
		}
		f->next = s;
	}

	SFOPEN(f,0);
	SFMTXRETURN(f, (int)n);
}
