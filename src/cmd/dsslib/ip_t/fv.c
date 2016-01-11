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
 * unsigned fixed vector arithmetic
 */

#include <fv.h>

#undef	_BLD_DEBUG

int
fvcmp(int n, const unsigned char* a, const unsigned char* b)
{
	register int	i;

	for (i = 0; i < n; i++)
		if (a[i] < b[i])
			return -1;
		else if (a[i] > b[i])
			return 1;
	return 0;
}

int
fvset(int n, unsigned char* r, long v)
{
	register int	i;

	i = n;
	while (i--)
	{
		r[i] = v;
		v >>= 8;
	}
	return v ? -1 : 0;
}

int
fvior(int n, unsigned char* r, const unsigned char* a, const unsigned char* b)
{
	register int	i;

	for (i = 0; i < n; i++)
		r[i] = a[i] | b[i];
	return 0;
}

int
fvxor(int n, unsigned char* r, const unsigned char* a, const unsigned char* b)
{
	register int	i;

	for (i = 0; i < n; i++)
		r[i] = a[i] ^ b[i];
	return 0;
}

int
fvand(int n, unsigned char* r, const unsigned char* a, const unsigned char* b)
{
	register int	i;

	for (i = 0; i < n; i++)
		r[i] = a[i] & b[i];
	return 0;
}

int
fvodd(int n, const unsigned char* a)
{
	return a[n-1] & 1;
}

int
fvlsh(int n, unsigned char* r, const unsigned char* a, int v)
{
	register int	i;
	register int	b;
	register int	g;

	if (g = v / 8)
	{
		for (i = 0; i < n - g; i++)
			r[i] = r[i + g];
		for (; i < n; i++)
			r[i] = 0;
	}
	if (b = v % 8)
	{
		for (i = 0; i < n - 1; i++)
			r[i]  = (r[i] << b) | (r[i + 1] >> (8 - b));
		r[i] <<= b;
	}
	return 0;
}

int
fvrsh(int n, unsigned char* r, const unsigned char* a, int v)
{
	register int	i;
	register int	b;
	register int	g;

	if (g = v / 8)
	{
		for (i = n - 1; i >= g; i--)
			r[i] = r[i - g];
		for (; i >= 0; i--)
			r[i] = 0;
	}
	if (b = v % 8)
	{
		for (i = n - 1; i > 0; i--)
			r[i]  = (r[i] >> b) | (r[i - 1] << (8 - b));
		r[i] >>= b;
	}
	return 0;
}

int
fvnot(int n, unsigned char* r, const unsigned char* a)
{
	register int	i;

	for (i = 0; i < n; i++)
		r[i]  = ~a[i];
	return 0;
}

int
fvadd(int n, unsigned char* r, const unsigned char* a, const unsigned char* b)
{
	register int	i;
	register int	c;
	register int	t;

	c = 0;
	i = n;
	while (i--)
	{
		c = (t = a[i] + b[i] + c) > 0xff;
		r[i] = t;
	}
	return -c;
}

int
fvsub(int n, unsigned char* r, const unsigned char* a, const unsigned char* b)
{
	register int	i;
	register int	c;
	register int	t;

	c = 0;
	i = n;
	while (i--)
	{
		c = (t = a[i] - b[i] - c) < 0;
		r[i] = t;
	}
#if _BLD_DEBUG
	sfprintf(sfstdout, "fvsub ( %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ) - ( %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ) = ( %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ) [%d]\n",
		a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15],
		b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15],
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15],
		-c);
#endif
	return -c;
}

int
fvmpy(int n, unsigned char* r, const unsigned char* a, const unsigned char* b)
{
	register int	i;
	register int	j;
	register int	m;
	register int	c;
	register int	t;

	fvset(n, r, 0);
	for (i = 0; i < n - 1 && !a[i]; i++);
	while (i < n)
	{
		m = a[i++];
		c = 0;
		j = n;
		while (j--)
		{
			c = (t = m * b[j] + c) >> 8;
			r[j] += t;
		}
		if (c)
			return -1;
	}
	return 0;
}

int
fvdiv(int n, unsigned char* r, unsigned char* m, const unsigned char* a, const unsigned char* b)
{
	register int	i;
	register int	j;
	register int	k;
	register int	s;
	register int	x;
	register int	y;
	unsigned char*	d;
	unsigned char*	t;
	unsigned char*	u;
	unsigned char*	v;

	for (j = 0; j < n && !b[j]; j++);
#if _BLD_DEBUG
	sfprintf(sfstdout, "fvdiv ( %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ) / ( %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ) [%d]\n",
		a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15],
		b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15],
		j);
#endif
	if (j == n)
		return -1;
	if (!(d = newof(0, unsigned char, 3 * n, 0)))
		return -1;
	fvcpy(n, u = d + n, a);
	v = u + n;
	fvset(n, r, 0);
	for (;;)
	{
		for (i = 0; i < n && !u[i]; i++);
		if (i > j)
			break;
		x = u[i];
		y = b[j];
		if (x >= y)
		{
			if (j == 0)
				break;
			for (s = 0; x >= (y << (s + 1)); s++);
			for (;;)
			{
				for (k = 0; k < i; k++)
					d[k] = 0;
				for (x = j; x < n; x++)
				{
					d[k - 1] |= b[x] >> (8 - s);
					d[k] = b[x] << s; 
					k++;
				}
				while (k < n)
					d[k++] = 0;
				if (!fvsub(n, v, u, d))
					break;
				if (!s--)
					goto done;
			}
			r[i] |= 1<<s;
		}
		else if (i == (n - 1))
			break;
		else
		{
			for (s = 0; x < (y >> (s + 1)); s++);
			for (;;)
			{
				for (k = 0; k < i; k++)
					d[k] = 0;
				d[k] = 0;
				for (x = j; x < n; x++)
				{
					d[k] |= b[x] >> s;
					if (++k < n)
						d[k] = b[x] << (8 - s); 
				}
				while (++k < n)
					d[k] = 0;
				if (!fvsub(n, v, u, d))
					break;
				if (++s >= 8)
					goto done;
			}
			r[i + 1] |= 1<<(8 - s);
		}
		t = u;
		u = v;
		v = t;
	}
 done:
	free(d);
	fvcpy(n, m, u);
#if _BLD_DEBUG
	sfprintf(sfstdout, "fvdiv ( %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ) R ( %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x )\n",
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15],
		m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
#endif
	return 0;
}

char*
fmtfv(int n, const unsigned char* a, int x, int c, int g)
{
	register int	i;
	register int	p;
	char*		s;
	char*		o;
	unsigned char*	b;
	unsigned char*	d;
	unsigned char*	r;
	int		m;

	static const char	digit[] = "0123456789abcdef";

	if (x != 8 && x != 10 && x != 16)
		x = 10;
	m = n * 3 + 4;
	if (g > 0)
		m += n / g;
	s = fmtbuf(m) + m;
	*--s = 0;
	o = s;
	p = c ? g : 0;
	if (b = newof(0, unsigned char, 3 * n, 0))
	{
		fvcpy(n, b, a);
		fvset(n, d = b + n, x);
		r = d + n;
		for (;;)
		{
			i = n;
			while (--i && !b[i]);
			if (!i && !b[i])
				break;
			fvdiv(n, b, r, b, d);
			*--s = digit[r[n - 1]];
			if (!--p)
			{
				p = g;
				*--s = c;
			}
		}
		free(b);
	}
	if (*s == c)
		s++;
	if (s == o)
		*--s = '0';
	else if (x == 8)
		*--s = '0';
	else if (x == 16)
	{
		*--s = 'x';
		*--s = '0';
	}
	return s;
}

int
strfv(int n, unsigned char* r, const char* s, char** e, int b, int d)
{
	register int	c;
	register int	i;
	register int	t;
	register int	x;

	static unsigned char	dig[256];

	if (!dig[0])
	{
		memset(dig, 0xff, sizeof(dig));
		for (i = '0'; i <= '9'; i++)
			dig[i] = i - '0';
		for (i = 'A'; i <= 'F'; i++)
			dig[i] = 10 + i - 'A';
		for (i = 'a'; i <= 'f'; i++)
			dig[i] = 10 + i - 'a';
	}
	fvset(n, r, 0);
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s != '0')
	{
		if (!b)
			b = 10;
	}
	else if (*(s + 1) == 'x' || *(s + 1) == 'X')
	{
		if (!b || b == 16)
		{
			s += 2;
			b = 16;
		}
	}
	else if (!b)
		b = *(s + 1) >= '0' && *(s + 1) <= '9' ? 8 : 10;
	while (c = *s++)
	{
		if (c == d)
			continue;
		else if ((c = dig[c]) == 0xff)
		{
			c = 0;
			break;
		}
		x = 0;
		i = n;
		while (i--)
		{
			t = r[i] * b + x;
			x = t >> 8;
			r[i] = t;
		}
		i = n;
		while (i--)
		{
			c = (t = r[i] + c) > 0xff;
			r[i] = t;
			if (!c)
				break;
		}
		if (c)
			break;
	}
	if (e)
		*e = (char*)s - 1;
	return -c;
}

#undef	fvcpy

int
fvcpy(int n, unsigned char* r, const unsigned char* a)
{
	memcpy(r, a, n);
	return 0;
}

/*
 * return the minimum prefix of a limited to m bits
 */

unsigned char*
fvplo(int z, int m, unsigned char* r, const unsigned char* a)
{
	int		i;
	int		n;

	if (m)
	{
		fvcpy(z, r, a);
		m = z * 8 - m;
		n = m / 8;
		m -= n * 8;
		n++;
		for (i = 1; i < n; i++)
			r[z - i] = 0;
		if (m)
			r[z - n] &= ~((1<<m) - 1);
	}
	else
		fvset(z, r, 0);
	return r;
}

/*
 * return the maximum prefix of a limited to m bits
 */

unsigned char*
fvphi(int z, int m, unsigned char* r, const unsigned char* a)
{
	int		i;
	int		n;

	if (m)
	{
		fvcpy(z, r, a);
		m = z * 8 - m;
		n = m / 8;
		m -= n * 8;
		n++;
		for (i = 1; i < n; i++)
			r[z - i] = 0xFF;
		if (m)
			r[z - n] |= ((1<<m) - 1);
	}
	else
		for (i = 0; i < z; i++)
			r[i] = 0xFF;
	return r;
}
