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
 * include this file to instantiate tuple cx externalf/internalf
 * parameterized on these macros
 *
 *	ITLINT		the tuple element integral type
 *	ITLEXTERNAL	convert to external
 *	ITLINTERNAL	convert to internal
 *
 * macros must be defined before include
 * macros undefined after include
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <ctype.h>

/*
 * pretty print an ITLINT tuple list from value->buffer into buf
 */

ssize_t
ITLEXTERNAL(Cx_t* cx, Cxtype_t* type, int dots, int tuple, int group, const char* details, Cxformat_t** formats, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	ITLINT*		ap;
	Sfio_t*		sp;
	char*		sep;
	char*		bet;
	char*		s;
	char*		t;
	ITLINT		v;
	int		def;
	int		fmt;
	int		b;
	int		i;
	int		j;
	int		k;
	int		m;
	int		n;
	int		g;

	if (j = value->buffer.size / sizeof(ITLINT))
	{
		sep = (char*)CXDETAILS(details, formats[0], type, "");
		if (!(fmt = *sep++))
		{
			fmt = 'd';
			sep = ",";
			def = 1;
		}
		else if (fmt == 's')
			def = 1;
		else
			def = sep == type->format.details;
		if (!*sep)
			sep = ",";
		bet = ":";
		sp = cx->buf;
		ap = (ITLINT*)value->buffer.data;
		i = 0;
		j -= tuple - 1;
		g = j + 1 + !group;
		while (i < j)
		{
			if (i > g)
			{
				g = j + 1;
				sfprintf(sp, "}");
				t = sep;
			}
			else if (g == (j + 1) && ap[i] == (ITLINT)(~0) && i < (j - 1))
			{
				i++;
				if (ap[i])
				{
					g = i + ap[i];
					sfprintf(sp, "%s", sep);
					t = "{";
				}
				else
					t = sep;
				i++;
			}
			else
				t = sep;
			for (k = 0; k < tuple; i++, k++, t = bet)
			{
				sfprintf(sp, "%s", t);
				if (def && formats && formats[k] && formats[k]->map && !cxnum2str(cx, formats[k], (Cxinteger_t)ap[i], &s))
					sfprintf(sp, "%s", s);
				else if (ap[i])
					switch (fmt)
					{
					case 'd':
						sfprintf(sp, "%d", ap[i]);
						break;
					case 'o':
						sfprintf(sp, "0%o", ap[i]);
						break;
					case 'x':
						sfprintf(sp, "0x%0*x", sizeof(ITLINT) * 2, ap[i]);
						break;
					case '.':
						n = 8;
						goto dotted;
					case '1':
					case '2':
					case '4':
					case '8':
						n = (n - '0') * 8;
					dotted:
						v = ap[i];
						m = (1 << n) - 1;
						b = sizeof(ITLINT) * 8;
						if ((b / n) == 2 && !((v >> n) & m))
							b = n;
						while ((b -= n) >= 0)
							sfprintf(sp, "%u%s", (v >> b) & m, b ? "." : "");
						break;
					default:
						if (dots)
						{
							n = dots * 8;
							goto dotted;
						}
						sfprintf(sp, "%u", ap[i]);
						break;
					}
				else
					sfputc(sp, '0');
			}
		}
		if (g < (j + 1))
			sfputc(sp, '}');
		i = sfstrtell(sp);
		if (!(s = sfstruse(sp)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		if (j = strlen(sep))
		{
			s += j;
			i -= j;
		}
	}
	else
	{
		s = "";
		i = 0;
	}
	if ((i + 1) > size)
		return i + 1;
	memcpy(buf, s, i + 1);
	return i;
}

/*
 * parse an ITLINT tuple list from buf into value->buffer
 * return the number of bytes consumed from buf
 */

ssize_t
ITLINTERNAL(Cx_t* cx, Cxvalue_t* value, int dots, int tuple, int group, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	register char*	s;
	register char*	e;
	register int	t;
	char*		p;
	Sfio_t*		sp;
	ITLINT*		vp;
	ITLINT		n;
	ITLINT		m;
	size_t		o;
	size_t		z;

	sp = cx->buf;
	o = 0;
	t = 0;
	s = (char*)buf;
	e = s + size;
	if (!dots)
		dots = 1;
	dots *= 8;
	m = 1;
	m = (m << dots) - 1;
	for (; s < e; s++)
		if (*s == ':' || isdigit(*s))
		{
			t++;
			if (*s == ':')
				n = 0;
			else
			{
				n = strntol(s, e - s, &p, 0);
				for (;;)
				{
					s = p;
					if (s >= e || *s != '.')
						break;
					n = (n << dots) | (strntol(s, e - s, &p, 0) & m);
				}
			}
			sfwrite(sp, &n, sizeof(n));
			if (group && n == (ITLINT)(~0))
			{
				n = 0;
				sfwrite(sp, &n, sizeof(n));
				n = (ITLINT)(~0);
				sfwrite(sp, &n, sizeof(n));
			}
			if (s < e && *s != ':')
			{
				s--;
				n = 0;
				while (t++ < tuple)
					sfwrite(sp, &n, sizeof(n));
				t = 0;
			}
		}
		else if (*s == '{')
		{
			if (group)
			{
				n = (ITLINT)(~0);
				sfwrite(sp, &n, sizeof(n));
				o = sfstrtell(sp) / sizeof(n);
				sfwrite(sp, &n, sizeof(n));
			}
		}
		else if (isalpha(*s) || *s == '?')
			break;
	if (z = sfstrtell(sp) / sizeof(n))
	{
		if (!vm)
			vm = Vmregion;
		if (!(vp = vmnewof(vm, 0, ITLINT, z, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		memcpy(vp, sfstrseek(sp, 0, SEEK_SET), z * sizeof(n));
		if (o)
			vp[o] = z - o;
	}
	else
		vp = 0;
	value->buffer.data = vp;
	value->buffer.size = z * sizeof(n);
	return s - (char*)buf;
}

#undef	ITLINT
#undef	ITLEXTERNAL
#undef	ITLINTERNAL
