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
 * AT&T Research
 *
 * generate a temp file / name
 *
 *	[<dir>/][<pfx>]<bas>.<suf>
 *
 * length(<pfx>)<=5
 * length(<bas>)==chars-to-make-BASE-length-name
 * length(<suf>)==3
 *
 *	pathtmp(a,b,c,d)	pathtemp(a,L_tmpnam,b,c,0)
 *	tmpfile()		char*p=pathtemp(0,0,0,"tf",&sp);
 *				remove(p);
 *				free(p)
 *	tmpnam(0)		static char p[L_tmpnam];
 *				pathtemp(p,sizeof(p),0,"tn",0)
 *	tmpnam(p)		pathtemp(p,L_tmpnam,0,"tn",0)
 *	tempnam(d,p)		pathtemp(0,d,p,0)
 *	mktemp(p)		pathtemp(0,0,p,0)
 *
 * if buf==0 then space is malloc'd
 * buf size is size
 * dir and pfx may be 0
 * / as pfx trailing char creates a directory instead of a file
 * if pfx contains trailing X's then it is a mktemp(3) template
 * otherwise only first 5 chars of pfx are used
 * if fdp!=0 then the path is opened O_EXCL and *fdp is the open fd
 * malloc'd space returned by successful pathtemp() calls
 * must be freed by the caller
 *
 * generated names are pseudo-randomized to avoid both
 * collisions and predictions (same alg in sfio/sftmp.c)
 *
 * / as first pfx char provides tmp file generation control
 * 0 returned for unknown ops
 *
 *	/cycle		dir specifies TMPPATH cycle control
 *		automatic	(default) cycled with each tmp file
 *		manual		cycled by application with dir=(nil)
 *		(nil)		cycle TMPPATH
 *	/check		dir==0 disables access() check when fdp==0
 *	/prefix		dir specifies the default prefix (default ast)
 *	/private	private file/dir modes
 *	/public		public file/dir modes
 *	/seed		dir specifies pseudo-random generator seed
 *			0 or "0" to re-initialize
 *	/TMPPATH	dir overrides the env value
 *	/TMPDIR		dir overrides the env value
 */

#include <ast.h>
#include <ls.h>
#include <tv.h>
#include <tm.h>
#include <error.h>

#define ATTEMPT		16

#define BASE		14
#define SUFF		3

#define TMP_ENV		"TMPDIR"
#define TMP_PATH_ENV	"TMPPATH"
#define TMP1		"/tmp"
#define TMP2		"/usr/tmp"

#define VALID(d)	(*(d)&&!eaccess(d,W_OK|X_OK))

static struct Tmp_s
{
	mode_t		mode;
	char**		vec;
	char**		dir;
	uintmax_t	key;
	uintmax_t	rng;
	pid_t		pid;
	int		manual;
	int		nocheck;
	int		seed;
	char*		pfx;
	char*		tmpdir;
	char*		tmppath;
} tmp = { S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH };

char*
pathtemp(char* buf, size_t len, const char* dir, const char* pfx, int* fdp)
{
	register char*		d;
	register char*		b;
	register char*		s;
	register char*		x;
	uintmax_t		key;
	int			m;
	int			n;
	int			l;
	int			r;
	int			z;
	int			attempt;
	int			directory;
	Tv_t			tv;
	char			keybuf[16];

	if (pfx && pfx[0] == '/' && pfx[1])
	{
		pfx++;
		if (dir && !*dir)
			dir = 0;
		if (streq(pfx, "check"))
		{
			tmp.nocheck = !dir;
			return (char*)pfx;
		}
		else if (streq(pfx, "cycle"))
		{
			if (!dir)
			{
				tmp.manual = 1;
				if (tmp.dir && !*tmp.dir++)
					tmp.dir = tmp.vec;
			}
			else
				tmp.manual = streq(dir, "manual");
			return (char*)pfx;
		}
		else if (streq(pfx, "prefix"))
		{
			if (tmp.pfx)
				free(tmp.pfx);
			tmp.pfx = dir ? strdup(dir) : (char*)0;
			return (char*)pfx;
		}
		else if (streq(pfx, "private"))
		{
			tmp.mode = S_IRUSR|S_IWUSR;
			return (char*)pfx;
		}
		else if (streq(pfx, "public"))
		{
			tmp.mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
			return (char*)pfx;
		}
		else if (streq(pfx, "seed"))
		{
			tmp.key = (tmp.seed = (tmp.rng = dir ? (uintmax_t)strtoul(dir, NiL, 0) : (uintmax_t)1) != 0)? (uintmax_t)0x63c63cd9L : 0;
			return (char*)pfx;
		}
		else if (streq(pfx, TMP_ENV))
		{
			if (tmp.vec)
			{
				free(tmp.vec);
				tmp.vec = 0;
			}
			if (tmp.tmpdir)
				free(tmp.tmpdir);
			tmp.tmpdir = dir ? strdup(dir) : (char*)0;
			return (char*)pfx;
		}
		else if (streq(pfx, TMP_PATH_ENV))
		{
			if (tmp.vec)
			{
				free(tmp.vec);
				tmp.vec = 0;
			}
			if (tmp.tmppath)
				free(tmp.tmppath);
			tmp.tmppath = dir ? strdup(dir) : (char*)0;
			return (char*)pfx;
		}
		return 0;
	}
	if (tmp.seed)
		tv.tv_nsec = 0;
	else
		tvgettime(&tv);
	if (!(d = (char*)dir) || *d && eaccess(d, W_OK|X_OK))
	{
		if (!tmp.vec)
		{
			if ((x = tmp.tmppath) || (x = getenv(TMP_PATH_ENV)))
			{
				n = 2;
				s = x;
				while (s = strchr(s, ':'))
				{
					s++;
					n++;
				}
				if (!(tmp.vec = newof(0, char*, n, strlen(x) + 1)))
					return 0;
				tmp.dir = tmp.vec;
				x = strcpy((char*)(tmp.dir + n), x);
				*tmp.dir++ = x;
				while (x = strchr(x, ':'))
				{
					*x++ = 0;
					if (!VALID(*(tmp.dir - 1)))
						tmp.dir--;
					*tmp.dir++ = x;
				}
				if (!VALID(*(tmp.dir - 1)))
					tmp.dir--;
				*tmp.dir = 0;
			}
			else
			{
				if (((d = tmp.tmpdir) || (d = getenv(TMP_ENV))) && !VALID(d))
					d = 0;
				if (!(tmp.vec = newof(0, char*, 2, d ? (strlen(d) + 1) : 0)))
					return 0;
				if (d)
					*tmp.vec = strcpy((char*)(tmp.vec + 2), d);
			}
			tmp.dir = tmp.vec;
		}
		if (!(d = *tmp.dir++))
		{
			tmp.dir = tmp.vec;
			d = *tmp.dir++;
		}
		if (!d && (!*(d = astconf("TMP", NiL, NiL)) || eaccess(d, W_OK|X_OK)) && eaccess(d = TMP1, W_OK|X_OK) && eaccess(d = TMP2, W_OK|X_OK))
			return 0;
	}
	if (!len)
		len = PATH_MAX;
	len--;
	if (!(b = buf) && !(b = newof(0, char, len, 1)))
		return 0;
	z = 0;
	if (!pfx && !(pfx = tmp.pfx))
		pfx = "ast";
	m = strlen(pfx);
	if (directory = pfx[m - 1] == '/')
		m--;
	if (buf && dir && (buf == (char*)dir && (buf + strlen(buf) + 1) == (char*)pfx || buf == (char*)pfx && !*dir) && !strcmp((char*)pfx + m + 1, "XXXXX"))
	{
		d = (char*)dir;
		len = m += strlen(d) + 8;
		l = 3;
		r = 3;
	}
	else if (*pfx && pfx[m - 1] == 'X')
	{
		for (l = m; l && pfx[l - 1] == 'X'; l--);
		r = m - l;
		m = l;
		l = r / 2;
		r -= l;
	}
	else if ((x = strchr(pfx, '.')) && (x - pfx) < 5)
	{
		if (m > 5)
			m = 5;
		l = BASE - SUFF - m;
		r = SUFF;
	}
	else
	{
		if (m > 5)
			m = 5;
		l = BASE - SUFF - m - 1;
		r = SUFF;
		z = '.';
	}
	x = b + len;
	s = b;
	if (d)
	{
		while (s < x && (n = *d++))
			*s++ = n;
		if (s < x && s > b && *(s - 1) != '/')
			*s++ = '/';
	}
	if ((x - s) > m)
		x = s + m;
	while (s < x && (n = *pfx++))
	{
		if (n == '/' || n == '\\' || n == z)
			n = '_';
		*s++ = n;
	}
	*s = 0;
	len -= (s - b);
	for (attempt = 0; attempt < ATTEMPT; attempt++)
	{
		if (!tmp.rng || !tmp.seed && (attempt || tmp.pid != getpid()))
		{	
			register int	r;

			/*
			 * get a quasi-random coefficient
			 */

			tmp.pid = getpid();
			tmp.rng = (uintmax_t)tmp.pid * ((uintmax_t)time(NiL) ^ (((uintmax_t)integralof(&attempt)) >> 3) ^ (((uintmax_t)integralof(tmp.dir)) >> 3));
			if (!tmp.key)
				tmp.key = (tmp.rng >> (sizeof(tmp.rng) * 4)) | ((tmp.rng & 0xffff) << (sizeof(tmp.rng) * 4));
			tmp.rng ^= tmp.key;

			/*
			 * Knuth vol.2, page.16, Thm.A
			 */

			if ((r = (tmp.rng - 1) & 03))
				tmp.rng += 4 - r;
		}

		/*
		 * generate a pseudo-random name
		 */

		key = tmp.rng * tmp.key + tv.tv_nsec;
		if (!tmp.seed)
			tvgettime(&tv);
		tmp.key = tmp.rng * key + tv.tv_nsec;
		sfsprintf(keybuf, sizeof(keybuf), "%07.7.62I*u%07.7.62I*u", sizeof(key), key, sizeof(tmp.key), tmp.key);
		sfsprintf(s, len, "%-.*s%s%-.*s", l, keybuf, z ? "." : "", r, keybuf + sizeof(keybuf) / 2);
		if (fdp)
		{
			if (directory)
			{
				if (!mkdir(b, tmp.mode|S_IXUSR))
				{
					if ((n = open(b, O_SEARCH)) >= 0)
					{
						*fdp = n;
						return b;
					}
					rmdir(b);
				}
			}
			else if ((n = open(b, O_CREAT|O_RDWR|O_EXCL|O_TEMPORARY, tmp.mode)) >= 0)
			{
				*fdp = n;
				return b;
			}
			switch (errno)
			{
			case EACCES:
				if (access(b, F_OK))
					goto nope;
				break;
			case ENOTDIR:
			case EROFS:
				goto nope;
			}
		}
		else if (tmp.nocheck)
			return b;
		else if (access(b, F_OK))
			switch (errno)
			{
			case ENOENT:
				return b;
			}
	}
 nope:
	if (!buf)
		free(b);
	return 0;
}
