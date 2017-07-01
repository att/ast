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
 * Glenn Fowler
 * AT&T Research
 */

static const char id[] = "\n@(#)$Id: pcmp (AT&T Research) 1998-11-01 $\0\n";

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <ls.h>
#include <pzip.h>
#include <zlib.h>

static struct
{
	int		test;
	int		level;
	int		verbose;
} state;

/*
 * return the compressed size of buffer f of fsize bytes
 */

static size_t
zip(unsigned char* f, size_t fsize, unsigned char* t, size_t tsize)
{
	unsigned long	used;

	used = tsize;
	if (compress2(t, &used, f, fsize, state.level) != Z_OK)
		error(3, "gzip compress failed");
	return used;
}

/*
 * return the difference size of buffer f of fsize bytes
 */

static size_t
dif(unsigned char* f, size_t fsize, unsigned char* t, size_t tsize, size_t cols)
{
	unsigned char*	p;
	unsigned char*	e;
	size_t		n;
	size_t		i;

	static Sfio_t*	dp;
	static Sfio_t*	vp;

	if (!dp && !(dp = sfstropen()) || !vp && !(vp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [tmp streams]");
	p = (unsigned char*)memcpy(t, f, cols);
	sfwrite(dp, p, cols);
	n = 1;
	for (e = f + fsize; f < e; f += cols)
	{
		for (i = 0; i < cols; i++)
			if (p[i] != f[i])
			{
				sfputu(dp, n);
				sfputu(dp, i + 1);
				sfputc(vp, p[i] = f[i]);
				n = 0;
				while (++i < cols)
					if (p[i] != f[i])
					{
						sfputu(dp, i + 1);
						sfputc(vp, p[i] = f[i]);
					}
				sfputu(dp, 0);
				break;
			}
		n++;
	}
	sfputu(dp, n);
	sfputu(dp, 0);
	fsize = sfstrtell(vp);
	sfputu(dp, fsize);
	sfwrite(dp, sfstrseek(vp, 0, SEEK_SET), fsize);
	fsize = sfstrtell(dp);
	return zip((unsigned char*)sfstrseek(dp, 0, SEEK_SET), fsize, t, tsize);
}

main(int argc, char** argv)
{
	size_t		cols;
	size_t		rows;
	size_t		pct;
	size_t		z;
	size_t		d;
	size_t		n;
	size_t		i;
	unsigned int	h;
	unsigned int	m;
	size_t		datsize;
	size_t		bufsize;
	unsigned char*	dat;
	unsigned char*	buf;
	unsigned char*	end;
	unsigned char*	s;
	unsigned char*	p;

	error_info.id = "pcmp";
	state.level = 6;
	cols = 100;
	rows = 1000;
	for (;;)
	{
		switch (optget(argv, "c#[cols]l#[compression-level]r#[rows]vT#[test-mask]"))
		{
		case 'c':
			cols = opt_info.num;
			continue;
		case 'l':
			state.level = opt_info.num;
			continue;
		case 'p':
			pct = opt_info.num;
			continue;
		case 'r':
			rows = opt_info.num;
			continue;
		case 'v':
			state.verbose = 1;
			continue;
		case 'T':
			state.test |= opt_info.num;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || *argv)
		error(ERROR_USAGE|4, "%s", optusage(NiL));

	/*
	 * set up the workspace
	 */

	datsize = rows * cols;
	if (!(dat = newof(0, unsigned char, datsize, 0)))
		error(ERROR_SYSTEM|3, "out of space [dat]");
	bufsize = 4 * datsize;
	if (!(buf = newof(0, unsigned char, bufsize, 0)))
		error(ERROR_SYSTEM|3, "out of space [buf]");
	end = dat + datsize;

	/*
	 * loop over the probabilities
	 */

	h = (unsigned int)time(NiL) ^ (unsigned int)getpid();
	for (n = 0; n < 100; n++)
	{
		m = (UINT_MAX / 100) * n;
		memset(dat, 040, cols);
		for (s = p = dat; s < end; p = s, s += cols)
			for (i = 0; i < cols; i++)
				if ((h = h * 0x63c63cd9L + 0x9c39c33dL) < m)
					s[i] = 040 | (h & 0177);
				else
					s[i] = p[i];
		d = dif(dat, datsize, buf, bufsize, cols);
		z = zip(dat, datsize, buf, bufsize);
		sfprintf(sfstdout, "%2d %5u %5u\n", n, d, z);
		if (d > z)
			break;
	}
	return 0;
}
