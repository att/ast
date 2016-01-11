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
 * <debug.h> support
 */

#include <ast.h>
#include <aso.h>
#include <error.h>
#include <debug.h>
#include <sig.h>

static int	indent = 0;

/*
 * stripped down printf -- only { %c %[I*|l|ll|z][dopux] %s }
 */

#define SIZEOF(x)	(int)sizeof(x)
#define VASIGNED(a,n)	(((n)>SIZEOF(long))?va_arg(a,int64_t):((n)?va_arg(a,long):va_arg(a,int)))
#define VAUNSIGNED(a,n)	(((n)>SIZEOF(unsigned long))?va_arg(a,uint64_t):((n)?va_arg(a,unsigned long):va_arg(a,unsigned int)))

ssize_t
debug_vsprintf(char* buf, size_t siz, register const char* format, va_list ap)
{
	register int		c;
	register char*		p;
	register char*		e;
	int64_t			n;
	uint64_t		u;
	int			w;
	int			x;
	int			l;
	int			f;
	int			g;
	int			r;
	int			i;
	char*			s;
	char*			b;
	char*			t;
	void*			v;
	wchar_t*		y;
	char			num[32];
	char			typ[32];

	static char	digits[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLNMOPQRSTUVWXYZ@_";

	i = 1;
	p = buf;
	e = buf + siz;
	for (;;)
	{
		switch (c = *format++)
		{
		case 0:
			goto done;
		case '%':
			if (*format == '-')
			{
				format++;
				l = 1;
			}
			else
				l = 0;
			if (*format == '0')
			{
				format++;
				f = l ? ' ' : '0';
			}
			else
				f = ' ';
			if ((c = *format) == '*')
			{
				format++;
				w = va_arg(ap, int);
			}
			else
			{
				w = 0;
				while (c >= '0' && c <= '9')
				{
					w = w * 10 + c - '0';
					c = *++format;
				}
			}
			x = r = 0;
			if (c == '.')
			{
				if ((c = *++format) == '*')
				{
					format++;
					x = va_arg(ap, int);
				}
				else
					while (c >= '0' && c <= '9')
					{
						x = x * 10 + c - '0';
						c = *++format;
					}
				if (c == '.')
				{
					if ((c = *++format) == '*')
					{
						format++;
						r = va_arg(ap, int);
					}
					else
						while (c >= '0' && c <= '9')
						{
							r = r * 10 + c - '0';
							c = *++format;
						}
				}
			}
			if (c == '(')
			{
				t = typ;
				while (c = *++format)
				{
					if (c == ')')
					{
						format++;
						break;
					}
					if (t < &typ[SIZEOF(typ)-1])
						*t++ = c;
				}
				*t = 0;
				t = typ;
			}
			else
				t = 0;
			n = 0;
			g = 0;
			b = num;
			for (;;)
			{
				switch (c = *format++)
				{
				case 'I':
					if (*format == '*')
					{
						format++;
						n = va_arg(ap, int);
					}
					continue;
				case 'l':
					n = n ? SIZEOF(int64_t) : SIZEOF(long);
					continue;
				case 'L':
					n = SIZEOF(int64_t);
					continue;
				case 't':
					n = SIZEOF(ptrdiff_t);
					continue;
				case 'z':
					n = SIZEOF(size_t);
					continue;
				}
				break;
			}
			switch (c)
			{
			case 0:
				break;
			case 'c':
				*b++ = va_arg(ap, int);
				break;
			case 'd':
				n = VASIGNED(ap, n);
				if (n < 0)
				{
					g = '-';
					n = -n;
				}
				u = n;
				goto dec;
			case 'o':
				u = VAUNSIGNED(ap, n);
				do *b++ = (char)(u & 07) + '0'; while (u >>= 3);
				break;
			case 's':
				if (!t)
				{
					s = va_arg(ap, char*);
					if (!s)
						s = "(null)";
#if 0
					if (n)
					{
						y = (wchar_t*)s;
						r = (int)wcslen(y);
						if (l && x && r > x)
							r = x;
						s = fmtbuf(4 * r + 1);
						if (!WideCharToMultiByte(CP_ACP, 0, y, r, s, 4 * r + 1, NULL, NULL))
							s = "(BadUnicode)";
					}
#endif
				}
#if 0
				else if (streq(t, "foo"))
				{
					/* foo-spcific format */
					continue;
				}
#endif
				else
					s = t;
				if (w || l && x)
				{
					if (!l || !x)
						x = (int)strlen(s);
					if (!w)
						w = x;
					n = w - x;
					if (l)
					{
						while (w-- > 0)
						{
							if (p >= e)
								goto done;
							if (!(*p = *s++))
								break;
							p++;
						}
						while (n-- > 0)
						{
							if (p >= e)
								goto done;
							*p++ = f;
						}
						continue;
					}
					while (n-- > 0)
					{
						if (p >= e)
							goto done;
						*p++ = f;
					}
				}
				for (;;)
				{
					if (p >= e)
						goto done;
					if (!(*p = *s++))
						break;
					p++;
				}
				continue;
			case 'u':
				u = VAUNSIGNED(ap, n);
			dec:
				if (r <= 0 || r >= SIZEOF(digits))
					r = 10;
				do *b++ = digits[u % r]; while (u /= r);
				break;
			case 'p':
				v = va_arg(ap, void*);
				if (u = (uint64_t)((char*)v - (char*)0))
				{
					if (SIZEOF(v) == 4)
						u &= 0xffffffff;
					g = 'x';
					w = 10;
					f = '0';
					l = 0;
				}
				goto hex;
			case 'x':
				u = VAUNSIGNED(ap, n);
			hex:
				do *b++ = ((c = (char)(u & 0xf)) >= 0xa) ? c - 0xa + 'a' : c + '0'; while (u >>= 4);
				break;
			case 'z':
				u = VAUNSIGNED(ap, n);
				do *b++ = ((c = (char)(u & 0x1f)) >= 0xa) ? c - 0xa + 'a' : c + '0'; while (u >>= 5);
				break;
			default:
				if (p >= e)
					goto done;
				*p++ = c;
				continue;
			}
			if (w)
			{
				if (g == 'x')
					w -= 2;
				else
					if (g) w -= 1;
				n = w - (b - num);
				if (!l)
				{
					if (g && f != ' ')
					{
						if (g == 'x')
						{
							if (p >= e)
								goto done;
							*p++ = '0';
							if (p >= e)
								goto done;
							*p++ = 'x';
						}
						else if (p >= e)
							goto done;
						else
							*p++ = g;
						g = 0;
					}
					while (n-- > 0)
					{
						if (p >= e)
							goto done;
						*p++ = f;
					}
				}
			}
			if (g == 'x')
			{
				if (p >= e)
					goto done;
				*p++ = '0';
				if (p >= e)
					goto done;
				*p++ = 'x';
			}
			else if (g)
			{
				if (p >= e)
					goto done;
				*p++ = g;
			}
			while (b > num)
			{
				if (p >= e)
					goto done;
				*p++ = *--b;
			}
			if (w && l)
				while (n-- > 0)
				{
					if (p >= e)
						goto done;
					*p++ = f;
				}
			continue;
		case ':':
			if (p >= e)
				goto done;
			*p++ = c;
			if (i && *format == ' ')
			{
				if ((p + i) < e)
					for (i = 0; i < indent; i++)
						*p++ = ' ';
				i = 0;
			}
			continue;
		default:
			if (p >= e)
				goto done;
			*p++ = c;
			continue;
		}
		break;
	}
 done:
	if (p < e)
		*p = 0;
	return (ssize_t)(p - buf);
}

ssize_t
debug_sprintf(char* buf, size_t siz, const char* format, ...)
{
	ssize_t		n;
	va_list		ap;

	va_start(ap, format);
	n = debug_vsprintf(buf, siz, format, ap);
	va_end(ap);
	return n;
}

ssize_t
debug_vprintf(int fd, const char* format, va_list ap)
{
	ssize_t		n;
	char		buf[4*1024];

	n = debug_vsprintf(buf, sizeof(buf), format, ap);
	return write(fd, buf, n);
}

ssize_t
debug_printf(int fd, const char* format, ...)
{
	ssize_t		n;
	va_list		ap;

	va_start(ap, format);
	n = debug_vprintf(fd, format, ap);
	va_end(ap);
	return n;
}

void
debug_fatal(const char* file, int line, const char* text)
{
	char*		s;
#ifdef SIG_BLOCK
	sigset_t	ss;
	sigfillset(&ss);
	sigprocmask(SIG_BLOCK, &ss, NiL);
#endif
	debug_printf(2, "%s:%d: assertion failed: %s\n", file, line, text);
	if (s = getenv("DEBUG_OPTIONS"))
		switch (*s)
		{
		case 'p':
			pause();
			break;
		default:
			abort();
			break;
		}
	exit(4);
}

void
debug_indent(int n)
{
	if (n > 0)
		asoincint(&indent);
	else if (n < 0 && indent > 0)
		asodecint(&indent);
	else
		indent = 0;
}

#if _sys_times

#include <times.h>
#include <sys/resource.h>

double
debug_elapsed(int set)
{	
	double		tm;
	struct rusage	ru;

	static double	prev;

	getrusage(RUSAGE_SELF, &ru);
	tm = (double)ru.ru_utime.tv_sec  + (double)ru.ru_utime.tv_usec/1000000.0;
	if (set)
		return prev = tm;
	return tm - prev;
}

#else

double
debug_elapsed(int set)
{
	return 0;
}

#endif
