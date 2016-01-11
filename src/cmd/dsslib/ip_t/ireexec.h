/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * include this file to instantiate a private ire exec
 * parameterized on these macros
 *
 *	IREINIT		the re element integral type
 *	IRENEXT		the re advance function
 *	IREEXEC		the re exec function
 *
 * macros must be defined before include
 * macros undefined after include
 *
 * Glenn Fowler
 * AT&T Research
 */

/*
 * advance IREINT re until success:1 or failure:0
 */

static int
IRENEXT(Ire_t* ire, register Re_t* re, int must, IREINT* lp, IREINT* rp)
{
	register int	i;
	register int	j;
	register int	n;
	IREINT*		bp;
	IREINT*		cp;
	IREINT*		ep;

	if (!re)
		return !ire->right || lp >= rp;
	if ((rp - lp) < must)
		return 0;
	if (re->lo)
	{
		must -= re->lo;
		bp = lp + re->lo - 1;
	}
	else if (IRENEXT(ire, re->next, must, lp, rp))
		return 1;
	else
		bp = lp;
	if ((rp - bp) < must)
		return 0;
	if (!re->hi || re->hi > (rp - lp))
		ep = rp;
	else
		ep = lp + re->hi;
	if ((rp - ep) < must)
		ep = rp - must;
	for (cp = lp; cp < ep; cp++)
	{
		if (*cp == ire->group && ire->group)
		{
			j = *++cp;
			if (cp < bp)
			{
				bp += j + 1;
				ep += j + 1;
			}
			cp += j;
			j = 1 - j;
		}
		else
			j = 0;
		if (re->n)
		{
			while (j < 1)
			{
				n = cp[j++];
				for (i = 0; i < re->n; i++)
					if (n == re->id[i])
					{
						if (re->invert)
							return 0;
						goto hit;
					}
			}
			if (!re->invert)
				return 0;
		}
	hit:
		if (cp >= bp && IRENEXT(ire, re->next, must, cp + 1, rp))
			return 1;
	}
	return 0;
}

/*
 * IREINT ire exec
 */

static int
IREEXEC(Ire_t* ire, void* data, size_t size)
{
	register Re_t*	re = ire->re;
	IREINT*		lp = (IREINT*)data;
	IREINT*		rp = lp + size / sizeof(IREINT);
	int		left = ire->left;
	int		must = ire->must;

	do
	{
		if (IRENEXT(ire, re, must, lp, rp))
			return 1;
		if (*lp++ == ire->group && ire->group)
			lp += *lp + 1;
	} while (!left && lp < rp);
	return 0;
}

#undef	IREINT
#undef	IRENEXT
#undef	IREEXEC
