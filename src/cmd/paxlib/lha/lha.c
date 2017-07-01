/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * pax lha format
 */

#include <paxlib.h>
#include <codex.h>
#include <ctype.h>
#include <swap.h>
#include <tm.h>

#define SUM			"sum-crc-0xa001"

typedef struct Ar_s
{
	Codexdisc_t		codexdisc;
	Pax_t*			pax;
	Paxarchive_t*		ap;
	char			method[64];
	unsigned short		checksum;

	unsigned char		check;
	unsigned char		system;
	unsigned char		minor_version;
} Ar_t;

static int
lha_method(register unsigned char* s)
{
	return (s[0] == '-' && isalnum(s[1]) && isalnum(s[2]) && isalnum(s[3]) && s[4] == '-') ? 0 : -1;
}

static int
lha_done(Pax_t* pax, register Paxarchive_t* ap)
{
	register Ar_t*	ar = (Ar_t*)ap->data;

	if (!ar)
		return -1;
	free(ar);
	ap->data = 0;
	return 0;
}

static int
lha_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*	ar;
	int		n;
	int		h;
	int		i;

	if (size < 1 || size < (n = buf[0] + 2) || n < 22 || lha_method(buf+2))
		return 0;
	for (h = 0, i = 2; i < n; i++)
		h += buf[i];
	if ((h & 0xff) != buf[1])
		return 0;
	if (!(ar = newof(0, Ar_t, 1, 0)))
	{
		if (ar)
			free(ar);
		return paxnospace(pax);
	}
	ar->pax = pax;
	ar->ap = ap;
	codexinit(&ar->codexdisc, pax->errorf);
	ar->method[0] = 'l';
	ar->method[1] = 'z';
	ar->method[2] = 'h';
	ar->method[3] = '-';
	ap->data = ar;
	return 1;
}

static int
lha_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*	ar = (Ar_t*)ap->data;
	register unsigned char*	hdr;
	char*			s;
	Tm_t			tm;
	unsigned long		dostime;
	long			n;
	int			h;
	int			i;
	int			mode;
	unsigned char		ss[2];

	if (paxread(pax, ap, ss, sizeof(ss), 0, 0) != sizeof(ss) || !(hdr = paxget(pax, ap, n = ss[0], NiL)) || lha_method(hdr))
		return 0;
	for (h = 0, i = 0; i < n; i++)
		h += hdr[i];
	if ((h & 0xff) != ss[1])
		return paxcorrupt(pax, ap, NiL, "header checksum error");
	f->st->st_size = swapget(3, hdr+5, 4);
	f->uncompressed = swapget(3, hdr+9, 4);
	ar->method[4] = hdr[1];
	ar->method[5] = hdr[2];
	ar->method[6] = hdr[3];
	dostime = swapget(3, hdr+13, 4);
	mode = swapget(3, hdr+17, 2);
	i = hdr[19];
	f->name = paxstash(pax, &ap->stash.head, NiL, i + 1);
	memcpy(f->name, hdr+20, i);
	f->name[i] = 0;
	hdr += i + 20;
	n -= i;
	if (n >= 24)
	{
		ar->checksum = swapget(3, hdr+0, 2);
		ar->system = hdr[2];
		ar->minor_version = hdr[3];
		ar->check = 1;
		hdr += 4;
	}
	else if (n == 22)
	{
		ar->checksum = swapget(3, hdr+0, 2);
		ar->system = 0;
		ar->check = 1;
		hdr += 2;
	}
	else if (n == 20)
	{
		ar->system = 0;
		ar->check = 0;
	}
	else
		return paxcorrupt(pax, ap, NiL, NiL);
	n = 7;
	n += sfsprintf(ar->method + n, sizeof(ar->method) - n, "+SIZE=%I*u", sizeof(f->uncompressed), f->uncompressed);
	if (ar->check)
		n += sfsprintf(ar->method + n, sizeof(ar->method) - n, "|%s", SUM);
	if (ar->system == 'U')
	{
		f->st->st_mtime = swapget(3, hdr+0, 4);
		f->st->st_mode = swapget(3, hdr+4, 2) & pax->modemask;
		f->st->st_uid = swapget(3, hdr+6, 2);
		f->st->st_gid = swapget(3, hdr+8, 2);
	}
	else
	{
		memset(&tm, 0, sizeof(tm));
		tm.tm_year  = ((dostime >> (16+9)) & 0x7f) + 80;
		tm.tm_mon   = ((dostime >> (16+5)) & 0x0f) - 1;
		tm.tm_mday  = ((dostime >> (16+0)) & 0x1f);
		tm.tm_hour  = ((dostime >>     11) & 0x1f);
		tm.tm_min   = ((dostime >>      5) & 0x3f);
		tm.tm_sec   = ((dostime          ) & 0x1f) * 2;
		f->st->st_mtime = tmtime(&tm, TM_LOCALZONE);
		if (mode & 0x10)
			mode = X_IFDIR|X_IRUSR|X_IWUSR|X_IXUSR|X_IRGRP|X_IWGRP|X_IXGRP|X_IROTH|X_IWOTH|X_IXOTH;
		else if (mode & 0x01)
			mode = X_IFREG|X_IRUSR|X_IRGRP|X_IROTH;
		else
		{
			mode = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IWGRP|X_IROTH|X_IWOTH;
			if ((s = strrchr(f->name, '.')) && (s[1]=='e' || s[1]=='E') && (s[1]=='x' || s[1]=='X') && (s[1]=='e' || s[1]=='E'))
				mode |= X_IXUSR|X_IXGRP|X_IXOTH;
		}
		f->st->st_mode = mode & pax->modemask;
	}
	f->linkpath = 0;
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_uid = pax->uid;
	f->st->st_gid = pax->gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->st->st_ctime = f->st->st_atime = f->st->st_mtime;
	return 1;
}

static int
lha_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
{
	register Ar_t*	ar = (Ar_t*)ap->data;
	Sfio_t*		sp;
	off_t		pos;
	ssize_t		n;
	int		r;
	int		pop;
	Codexdata_t	sum;

	if (!(n = f->st->st_size))
		return 1;
	pos = paxseek(pax, ap, 0, SEEK_CUR, 0) + f->st->st_size;
	r = -1;
	if (fd < 0)
		r = 1;
	else if (sp = paxpart(pax, ap, f->st->st_size))
	{
		if ((pop = codex(sp, ar->method, CODEX_DECODE, &ar->codexdisc, NiL)) < 0)
			(*pax->errorf)(NiL, pax, 2, "%s: %s: cannot decode method %s", ap->name, f->name, ar->method);
		else
		{
			for (;;)
			{
				if ((n = sfread(sp, pax->buf, sizeof(pax->buf))) < 0)
				{
					(*pax->errorf)(NiL, pax, 2, "%s: %s: unexpected EOF", ap->name, f->name);
					break;
				}
				else if (n == 0)
				{
					if (ar->check)
					{
						if (codexdata(sp, &sum) <= 0)
							(*pax->errorf)(NiL, pax, 2, "%s: %s: checksum discipline error", ap->name, f->name);
						else if (!paxchecksum(pax, ap, f, ar->checksum, sum.num & 0xffff))
							r = 1;
					}
					break;
				}
				if (paxdata(pax, ap, f, fd, pax->buf, n))
					break;
			}
			codexpop(sp, pop);
		}
	}
	if (paxseek(pax, ap, pos, SEEK_SET, 0) != pos)
	{
		(*pax->errorf)(NiL, pax, 2, "%s: %s: cannot seek past %s format data", ap->name, f->name, ap->format->name);
		r = -1;
	}
	return r;
}

Paxformat_t	pax_lha_format =
{
	"lha",
	"lharc",
	"lha archive",
	0,
	PAX_ARCHIVE|PAX_DOS|PAX_NOHARDLINKS|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(lha),
	0,
	lha_done,
	lha_getprologue,
	lha_getheader,
	lha_getdata,
};

PAXLIB(lha)
