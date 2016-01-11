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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * return the next character in the string s
 * \ character constants are expanded
 * *p is updated to point to the next character in s
 * *m is 1 if return value is wide
 */

#include <ast.h>
#include <ctype.h>
#include <ccode.h>

int
chrexp(register const char* s, char** p, int* m, register int flags)
{
	register const char*	t;
	register int		c;
	const char*		e;
	const char*		b;
	char*			r;
	int			n;
	int			x;
	wchar_t			d;
	Mbstate_t		q;
	bool			u;
	bool			w;

	u = w = 0;
	mbtinit(&q);
	for (;;)
	{
		b = s;
		switch (c = mbtchar(&d, s, MB_LEN_MAX, &q))
		{
		case 0:
			s = b;
			break;
		case '\\':
			b = s;
			switch (c = *s++)
			{
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				c -= '0';
				t = s + 2;
				while (s < t)
					switch (*s)
					{
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
						c = (c << 3) + *s++ - '0';
						break;
					default:
						t = s;
						break;
					}
				break;
			case 'a':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				c = CC_bel;
				break;
			case 'b':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				c = '\b';
				break;
			case 'c': /*DEPRECATED*/
			case 'C':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				if (c = *s)
				{
					s++;
					if (c == '\\')
					{
						c = chrexp(s - 1, &r, 0, flags);
						s = (const char*)r;
					}
					if (islower(c))
						c = toupper(c);
					c = ccmapc(c, CC_NATIVE, CC_ASCII);
					c ^= 0x40;
					c = ccmapc(c, CC_ASCII, CC_NATIVE);
				}
				break;
			case 'e': /*DEPRECATED*/
			case 'E':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				c = CC_esc;
				break;
			case 'f':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				c = '\f';
				break;
			case 'M':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				if (*s == '-')
				{
					s++;
					c = CC_esc;
				}
				break;
			case 'n':
				if (flags & FMT_EXP_NONL)
					continue;
				if (!(flags & FMT_EXP_LINE))
					goto noexpand;
				c = '\n';
				break;
			case 'r':
				if (flags & FMT_EXP_NOCR)
					continue;
				if (!(flags & FMT_EXP_LINE))
					goto noexpand;
				c = '\r';
				break;
			case 't':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				c = '\t';
				break;
			case 'v':
				if (!(flags & FMT_EXP_CHAR))
					goto noexpand;
				c = CC_vt;
				break;
			case 'u':
				u = 1;
			case 'w':
				t = s + 4;
				goto wex;
			case 'U':
				u = 1;
			case 'W':
				t = s + 8;
			wex:
				if (!(flags & FMT_EXP_WIDE))
					goto noexpand;
				w = 1;
				goto hex;
			case 'x':
				t = s + 2;
			hex:
				e = s;
				n = 0;
				c = 0;
				x = 0;
				while (!e || !t || s < t)
				{
					switch (*s)
					{
					case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						c = (c << 4) + *s++ - 'a' + 10;
						n++;
						continue;
					case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
						c = (c << 4) + *s++ - 'A' + 10;
						n++;
						continue;
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						c = (c << 4) + *s++ - '0';
						n++;
						continue;
					case '{':
					case '[':
						if (s != e)
							break;
						e = 0;
						s++;
						if (w && (*s == 'U' || *s == 'W') && *(s + 1) == '+')
							s += 2;
						continue;
					case '-':
						if (e)
							break;
						if (*(s + 1) != '}' && *(s + 1) != ']')
						{
							if (!*(s + 1) || *(s + 2) != '}' && *(s + 2) != ']')
								break;
							x = *(unsigned char*)(s + 1);
							s += 2;
						}
						else
						{
							x = -1;
							s++;
						}
						/*FALLTHROUGH*/
					case '}':
					case ']':
						if (!e)
							e = ++s;
						break;
					default:
						break;
					}
					break;
				}
				if (e)
				{
					if (n < 8 || n == 8 && c >= 0)
					{
						if (!w)
						{
							if (n > 2)
							{
								if (!(flags & FMT_EXP_WIDE))
									goto noexpand;
								w = 1;
							}
							else if (!(flags & FMT_EXP_CHAR))
								goto noexpand;
							else
								break;
						}
						if (!mbwide())
							w = 0;
						if (c <= 0x7f)
							break;
						if (u)
						{
							uint32_t	i = c;
							wchar_t		o;

							if (!utf32invalid(i) && utf32stowcs(&o, &i, 1) > 0)
							{
								c = o;
								break;
							}
						}
						else if (w || c <= ast.byte_max)
							break;
					}
					if (x)
					{
						c = x;
						w = 0;
						break;
					}
				}
				/*FALLTHROUGH*/
			case 0:
				goto noexpand;
			}
			break;
		default:
			if ((s - b) > 1)
				w = 1;
			break;
		noexpand:
			s = b;
			w = 0;
			c = '\\';
			break;
		}
		break;
	}
 done:
	if (m)
		*m = w;
	if (p)
		*p = (char*)s;
	return c;
}

int
chresc(register const char* s, char** p)
{
	return chrexp(s, p, NiL, FMT_EXP_CHAR|FMT_EXP_LINE|FMT_EXP_WIDE);
}
