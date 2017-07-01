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
 * pax rar format
 */

#include <paxlib.h>
#include <codex.h>
#include <swap.h>
#include <tm.h>

#define SUM			"sum-crc-0xedb88320-init-done"

typedef struct Ar_s
{
	Codexdisc_t		codexdisc;
	Pax_t*			pax;
	Paxarchive_t*		ap;
	Sfio_t*			sum;
	char			method[128];
	unsigned long		checksum;
	int			solid;

	unsigned short		level;
} Ar_t;

static int
rar_done(Pax_t* pax, register Paxarchive_t* ap)
{
	register Ar_t*	ar = (Ar_t*)ap->data;

	if (!ar)
		return -1;
	if (ar->sum)
		sfdisc(ar->sum, SF_POPDISC);
	free(ar);
	ap->data = 0;
	return 0;
}

static int
rar_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*		ar;

	if (size < 7 || buf[0] != 0x52 || buf[1] != 0x61 || buf[2] != 0x72 || buf[3] != 0x21 || buf[4] != 0x1a || buf[5] != 0x07 || buf[6] != 0x00)
		return 0;
	if (!(ar = newof(0, Ar_t, 1, 0)))
	{
		if (ar)
			free(ar);
		return paxnospace(pax);
	}
	ap->data = ar;
	ar->pax = pax;
	ar->ap = ap;
	codexinit(&ar->codexdisc, pax->errorf);
	ar->codexdisc.passphrase = pax->passphrase;
	if (!(ar->sum = codexnull()) || codex(ar->sum, SUM, CODEX_ENCODE, &ar->codexdisc, NiL) <= 0)
	{
		ar->sum = 0;
		rar_done(pax, ap);
		return -1;
	}
	return 1;
}

static int
rar_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	register unsigned char*	buf;
	char*			s;
	Tm_t			tm;
	unsigned short		checksum;
	unsigned short		flags;
	unsigned short		type;
	unsigned short		size;
	unsigned long		data;
	unsigned long		dostime;
	int			i;
	char*			msg;
	Codexdata_t		sum;

	msg = 0;
	for (;;)
	{
		if (!(buf = (unsigned char*)paxget(pax, ap, -7, NiL)))
			return 0;
		checksum = swapget(3, buf+0, 2);
		type = buf[2];
		flags = swapget(3, buf+3, 2);
		if (flags & 0x0008)
			ar->solid = 1;
		if ((size = swapget(3, buf+5, 2)) < 7)
			break;
		if (!(size -= 7))
			continue;
		sfsync(ar->sum);
		sfwrite(ar->sum, buf+2, 5);
		if (!(buf = (unsigned char*)paxget(pax, ap, size, NiL)))
		{
			msg = "unexpected EOF";
			break;
		}
		data = (flags & 0x8000) ? swapget(3, buf+0, 4) : 0;
		if (type != 0x74)
		{
			if (data && paxseek(pax, ap, data, SEEK_CUR, 0) < 0)
				break;
			continue;
		}
		sfwrite(ar->sum, buf, size);
		codexdata(ar->sum, &sum);
		if (paxchecksum(pax, ap, NiL, checksum, sum.num & 0xffff))
			return -1;
		f->uncompressed = swapget(3, buf+4, 4);
		if (flags & 0x0100)
		{
			data += swapget(3, buf+25, 4) << 32;
			f->uncompressed += swapget(3, buf+29, 4);
		}
		f->st->st_size = data;
		ar->checksum = swapget(3, buf+9, 4);
		dostime = swapget(3, buf+13, 4);
		i = 0;
		if (flags & 0x0004)
			i += sfsprintf(ar->method, sizeof(ar->method), "crypt-rar-%u%s+SIZE=%I*u|", buf[17], ar->solid ? "-RETAIN" : "", sizeof(f->st->st_size), f->st->st_size);
		if ((ar->level = buf[18]) == 0x30)
			i += sfsprintf(ar->method + i, sizeof(ar->method) - i, "copy");
		else
			i += sfsprintf(ar->method + i, sizeof(ar->method) - i, "rar-%u%s+SIZE=%I*u", buf[17], ar->solid ? "-RETAIN" : "", sizeof(f->uncompressed), f->uncompressed);
		sfsprintf(ar->method + i, sizeof(ar->method) - i, "|%s", SUM);
		i = swapget(3, buf+19, 2);
		f->name = paxstash(pax, &ap->stash.head, NiL, i + 1);
		memcpy(f->name, buf + ((flags & 0x0100) ? 33 : 25), i);
		f->name[i] = 0;
		f->linkpath = 0;
		f->st->st_dev = 0;
		f->st->st_ino = 0;
		i = swapget(3, buf+21, 4);
		if (buf[8] <= 2)
		{
			if (i & 0x10)
				i = X_IFDIR|X_IRUSR|X_IWUSR|X_IXUSR|X_IRGRP|X_IWGRP|X_IXGRP|X_IROTH|X_IWOTH|X_IXOTH;
			else if (i & 0x01)
				i = X_IFREG|X_IRUSR|X_IRGRP|X_IROTH;
			else
			{
				i = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IWGRP|X_IROTH|X_IWOTH;
				if ((s = strrchr(f->name, '.')) && (s[1]=='e' || s[1]=='E') && (s[1]=='x' || s[1]=='X') && (s[1]=='e' || s[1]=='E'))
					i |= X_IXUSR|X_IXGRP|X_IXOTH;
			}
		}
		if ((flags & 0x00e0) == 0x00e0 && (i & X_IFMT) != X_IFDIR)
		{
			i = X_IFDIR|(i & ~X_IFMT)|X_IRUSR|X_IWUSR|X_IXUSR;
			if (i & (X_IROTH|X_IWOTH))
				i |= X_IROTH|X_IXOTH;
			else
			{
				i &= ~(X_IROTH|X_IXOTH);
				if (i & (X_IRGRP|X_IWGRP))
					i |= X_IRGRP|X_IXGRP;
				else
					i &= ~(X_IRGRP|X_IXGRP);
			}
		}
		f->st->st_mode = i & pax->modemask;
		f->st->st_uid = pax->uid;
		f->st->st_gid = pax->gid;
		f->st->st_nlink = 1;
		IDEVICE(f->st, 0);
		memset(&tm, 0, sizeof(tm));
		tm.tm_year  = ((dostime >> (16+9)) & 0x7f) + 80;
		tm.tm_mon   = ((dostime >> (16+5)) & 0x0f) - 1;
		tm.tm_mday  = ((dostime >> (16+0)) & 0x1f);
		tm.tm_hour  = ((dostime >>     11) & 0x1f);
		tm.tm_min   = ((dostime >>      5) & 0x3f);
		tm.tm_sec   = ((dostime          ) & 0x1f) * 2;
		f->st->st_mtime = f->st->st_ctime = f->st->st_atime = tmtime(&tm, TM_LOCALZONE);
		return 1;
	}
	return paxcorrupt(pax, ap, f, msg);
}

static int
rar_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
{
	register Ar_t*	ar = (Ar_t*)ap->data;
	Sfio_t*		sp;
	off_t		pos;
	ssize_t		n;
	int		r;
	int		pop;
	Codexdata_t	sum;

	if (!f->st->st_size)
		return 1;
	pos = paxseek(pax, ap, 0, SEEK_CUR, 0) + f->st->st_size;
	r = -1;
	if (fd < 0 && !ar->solid)
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
					if (codexdata(sp, &sum) <= 0)
						(*pax->errorf)(NiL, pax, 2, "%s: %s: checksum discipline error", ap->name, f->name);
					else if (!paxchecksum(pax, ap, f, ar->checksum, sum.num))
						r = 1;
					break;
				}
				if (fd >= 0 && paxdata(pax, ap, f, fd, pax->buf, n))
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

Paxformat_t	pax_rar_format =
{
	"rar",
	0,
	"rar archive",
	0,
	PAX_ARCHIVE|PAX_DOS|PAX_NOHARDLINKS|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(rar),
	0,
	rar_done,
	rar_getprologue,
	rar_getheader,
	rar_getdata,
};

PAXLIB(rar)
