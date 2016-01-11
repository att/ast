/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
#pragma prototyped

/*
 * determine fixed record size by sampling data
 */

#include "pzlib.h"

typedef struct
{
	unsigned int	rep[4 * 1024];
	unsigned int	hit[UCHAR_MAX + 1];
} Fixed_t;

/*
 * determine fixed record size by sampling data
 * if buf!=0 then <buf,z> is used
 * otherwise data is peeked from io
 * return:
 *	>0 best guess from sample
 *	 0 could not determine
 *	<0 error
 */

ssize_t
pzfixed(Pz_t* pz, register Sfio_t* io, void* buf, size_t z)
{
	register unsigned char*		s;
	register Fixed_t*		xp;
	register unsigned int*		cp;
	register unsigned int		i;
	unsigned char*			t;
	unsigned int			j;
	unsigned int			k;
	unsigned int			n;
	unsigned int			m;
	unsigned int			max;
	unsigned long			f;
	unsigned long			g;
	Sfoff_t				siz;
	Error_f				trace;

	trace = pz && error_info.trace <= -2 ? pz->disc->errorf : 0;
	siz = pz && (pz->flags & PZ_POP) ? (Sfoff_t)0 : sfsize(io);
	if (buf)
		s = (unsigned char*)buf;
	else if (!(s = sfreserve(io, 8 * elementsof(xp->rep), 1)) && !(s = sfreserve(io, SF_UNBOUND, 1)))
		return -1;
	else
		z = sfvalue(io);
	if (trace)
		(*trace)(pz, pz->disc, -2, "pzfixed: siz=%I*d buf=%p z=%I*u", sizeof(siz), siz, buf, sizeof(z), z);

	/*
	 * first check for newline terminated
	 */

	if ((t = (unsigned char*)memchr((void*)s, '\n', z / 2)) && (n = t - s + 1) > 1)
	{
		if (siz > 0 && siz % n)
			n = 0;
		else
			for (i = n - 1; i < z; i += n)
				if (s[i] != '\n')
				{
					n = 0;
					break;
				}
		if (n && trace)
			(*trace)(pz, pz->disc, -2, "pzfixed: newline terminated %u byte records", n);
	}
	else
		n = 0;
	if (!n && (xp = newof(0, Fixed_t, 1, 0)))
	{
		if (trace)
			(*trace)(pz, pz->disc, -2, "pzfixed:   LEN      REP     BEST     FREQ");
		max = 0;
		for (i = 0; i < z; i++)
		{
			cp = xp->hit + s[i];
			m = i - *cp;
			*cp = i;
			if (m < elementsof(xp->rep))
			{
				if (m > max)
					max = m;
				xp->rep[m]++;
			}
		}
		n = 0;
		m = 0;
		f = ~0;
		for (i = max; i > 1; i--)
		{
			if ((siz <= 0 || !(siz % i)) && xp->rep[i] > xp->rep[n])
			{
				m++;
				g = 0;
				for (j = i; j < z - i; j += i)
					for (k = 0; k < i; k++)
						if (s[j + k] != s[j + k - i])
							g++;
				g = (((g * 100) / i) * 100) / xp->rep[i];
				if (trace)
					(*trace)(pz, pz->disc, -2, "pzfixed: %5d %8d %8ld %8ld%s", i, xp->rep[i], f, g, (g <= f) ? " *" : "");
				if (g <= f)
				{
					f = g;
					n = i;
				}
			}
		}
		if (m <= 1 && n <= 2 && siz > 0 && siz < 256)
			n = siz;
		free(xp);
	}

	/*
	 * release the peek data
	 */

	if (!buf)
		sfread(io, s, 0);
	return n;
}
