/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*              Doug McIlroy <doug@research.bell-labs.com>              *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * generate test data
 */

static const char id[] = "\n@(#)$Id: gen (AT&T Research) 1998-05-11 $\0\n";

#include <ast.h>
#include <error.h>

static unsigned char	hit[UCHAR_MAX+1];
static unsigned char	data[UCHAR_MAX+1];

#define ALPHA		62
#define BINARY		(UCHAR_MAX+1)
#define NUMERIC		10

#define RAND()		(state.seed=(state.seed*0x63c63cd9L+1)&0xffffffff)
#define ROUND(n,s)	((n)=(((n)+(s)-1)/(s))*(s))

typedef struct
{
	unsigned long	min;
	unsigned long	max;
} Range_t;

static struct
{
	unsigned char*	buf;
	int		charset;
	int		part;
	Range_t		range[UCHAR_MAX+1];
	unsigned long	seed;
} state;

static int
gen(register int m)
{
	int	c;

	if (state.part >= state.range['d'].min)
	{
		state.part = 0;
		RAND();
	}
	c = data[(state.seed >> state.part) % m];
	state.part += CHAR_BIT;
	return c;
}

main(int argc, char** argv)
{
	register unsigned int	c;
	register unsigned int	i;
	register unsigned int	k;
	register unsigned long	n;
	int			newline = 0;
	char*			e;

	error_info.id = "gen";
	state.range['d'].min = sizeof(state.seed) * CHAR_BIT;
	state.range['r'].min = 0x00001000;
	state.range['s'].min = 0x00100000;
	state.seed = 0x12345678;
	state.charset = ALPHA;
	i = 0;
	e = "0123456789";
	while (c = *e++)
	{
		hit[c] = 1;
		data[i++] = c;
	}
	e = "abcdefghijklmnopqrstuvwxyz";
	while (c = *e++)
	{
		hit[c] = 1;
		data[i++] = c;
	}
	e = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	while (c = *e++)
	{
		hit[c] = 1;
		data[i++] = c;
	}
#if 0
	e = "`-=[]\\{}|;':\",./<>?~!@#$%^&*()_+";
	while (c = *e++)
	{
		hit[c] = 1;
		data[i++] = c;
	}
#endif
	for (c = 0; c <= elementsof(data); c++)
		if (!hit[c])
			data[i++] = c;
	while (c = optget(argv, "bd:[dup[:max]]f:[float-key[:max]]i:[int-key[:max]]k:[key]l:[lcm]p:[prefix[:max]]r:[record:[max]]s:[size]"))
		switch (c)
		{
		case 'b':
			state.charset = BINARY;
			break;
		case 'd':
		case 'f':
		case 'i':
		case 'k':
		case 'l':
		case 'n':
		case 'p':
		case 'r':
		case 's':
			state.range[c].min = strton(opt_info.arg, &e, NiL, 0);
			if (*e)
				state.range[c].max = strton(e + 1, NiL, NiL, 0);
			break;
		case '?':
			error(ERROR_USAGE|4, opt_info.arg);
			break;
		case ':':
			error(2, opt_info.arg);
			break;
		}
	if (state.range['l'].min)
		ROUND(state.range['s'].min, state.range['l'].min);
	if (state.range['r'].max)
	{
		newline = 1;
		state.range['r'].min--;
		if (state.range['r'].max < state.range['r'].min)
			state.range['r'].max = state.range['r'].min;
		if (state.range['r'].max <= (state.range['r'].min + 1))
			state.range['r'].max = 1;
		else
			state.range['r'].max -= state.range['r'].min + 1;
	}
	else
		state.range['r'].max = 1;
	if (state.range['p'].min)
	{
		if (state.range['p'].min > state.range['r'].min)
			state.range['p'].min = state.range['r'].min;
		if (state.range['p'].max < state.range['p'].min)
			state.range['p'].max = state.range['p'].min;
		if (state.range['p'].max > state.range['r'].min + state.range['r'].max)
			state.range['p'].max = state.range['r'].min + state.range['r'].max;
		if (state.range['p'].max <= (state.range['p'].min + 1))
			state.range['p'].max = 1;
		else
			state.range['p'].max -= state.range['p'].min + 1;
	}
	if (state.range['f'].min)
	{
		if (state.range['f'].min >= state.range['r'].min)
			state.range['f'].min = state.range['r'].min - 1;
		if (state.range['f'].max < state.range['f'].min)
			state.range['f'].max = state.range['f'].min;
		if (state.range['f'].max > state.range['r'].min + state.range['r'].max)
			state.range['f'].max = state.range['r'].min + state.range['r'].max;
		if (state.range['f'].max <= (state.range['f'].min + 1))
			state.range['f'].max = 1;
		else
			state.range['f'].max -= state.range['f'].min + 1;
	}
	if (state.range['i'].min)
	{
		if (state.range['i'].min >= state.range['r'].min)
			state.range['i'].min = state.range['r'].min - 1;
		if (state.range['i'].max < state.range['i'].min)
			state.range['i'].max = state.range['i'].min;
		if (state.range['i'].max > state.range['r'].min + state.range['r'].max)
			state.range['i'].max = state.range['r'].min + state.range['r'].max;
		if (state.range['i'].max <= (state.range['i'].min + 1))
			state.range['i'].max = 1;
		else
			state.range['i'].max -= state.range['i'].min + 1;
	}
	if (!(state.buf = newof(0, unsigned char,
			state.range['f'].min + state.range['f'].max + 1 +
			state.range['i'].min + state.range['i'].max + 1 +
			state.range['k'].min + state.range['k'].max +
			state.range['p'].min + state.range['p'].max +
			2 * (state.range['r'].min + state.range['r'].max),
			1)))
		error(ERROR_SYSTEM|3, "out of space [buf]");
	n = state.range['s'].min;
	do
	{
		i = 0;
		if (state.range['f'].min)
		{
			k = state.range['f'].min + (RAND() % state.range['f'].max) - 6;
			state.buf[i++] = '.';
			while (i < k)
				state.buf[i++] = gen(NUMERIC);
			state.buf[i++] = 'e';
			state.buf[i++] = "-+"[RAND() % 2];
			state.buf[i++] = gen(NUMERIC);
			state.buf[i++] = gen(NUMERIC);
			state.buf[i++] = ':';
		}
		if (state.range['i'].min)
		{
			k = state.range['i'].min + (RAND() % state.range['i'].max);
			while (i < k)
				state.buf[i++] = gen(NUMERIC);
			state.buf[i++] = ':';
		}
		if (state.range['p'].min)
		{
			k = state.range['p'].min + (RAND() % state.range['p'].max);
			c = gen(state.charset);
			while (i < k)
				state.buf[i++] = c;
		}
		k = state.range['k'].min ? state.range['k'].min : state.range['r'].min + (RAND() % state.range['r'].max);
		while (i < k)
			state.buf[i++] = gen(state.charset);
		if (state.range['k'].min)
		{
			k = state.range['r'].min + (RAND() % state.range['r'].max);
			while (i < k)
				state.buf[i++] = ' ';
		}
		if (newline)
			state.buf[i++] = '\n';
		if (i > n)
		{
			i = n;
			if (newline)
				state.buf[n - 1] = '\n';
		}
		sfwrite(sfstdout, state.buf, i);
	} while (n -= i);
	exit(0);
}
