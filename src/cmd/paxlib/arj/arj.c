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
 * pax arj format
 */

#include <paxlib.h>
#include <codex.h>
#include <swap.h>
#include <tm.h>

#define MAGIC			0xea60

#define SUM			"sum-crc-0xedb88320-init-done"

typedef struct Ar_s
{
	Codexdisc_t		codexdisc;
	Pax_t*			pax;
	Paxarchive_t*		ap;
	Sfio_t*			sum;
	char			method[64];
	unsigned int		index;
	unsigned long		checksum;

	unsigned char		system;
	unsigned char		type;
} Ar_t;

static int
arj_done(Pax_t* pax, register Paxarchive_t* ap)
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
arj_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*	ar;
	register char*	s;
	int		n;
	int		r;
	uint32_t	checksum;
	Codexdata_t	sum;

	if (size < 4 || swapget(3, buf, 2) != MAGIC || size < ((n = swapget(3, buf+2, 2)) + 8) || n > 2600)
		return 0;
	if (!(ar = newof(0, Ar_t, 1, 0)))
		return paxnospace(pax);
	ar->pax = pax;
	ar->ap = ap;
	ap->data = ar;
	codexinit(&ar->codexdisc, pax->errorf);
	r = -1;
	if (!(ar->sum = codexnull()) || codex(ar->sum, SUM, CODEX_DECODE, &ar->codexdisc, NiL) <= 0)
	{
		ar->sum = 0;
		goto bad;
	}
	sfwrite(ar->sum, buf + 4, n);
	codexdata(ar->sum, &sum);
	checksum = swapget(3, buf + 4 + n, 4);
	if (checksum != sum.num)
	{
		r = 0;
		goto bad;
	}
	if (paxseek(pax, ap, n + 8, SEEK_SET, 0) < 0)
		goto bad;
	while ((s = paxget(pax, ap, 2, NiL)) && (n = swapget(3, s, 2)))
		if (paxseek(pax, ap, n + 2, SEEK_CUR, 0) < 0)
			goto bad;
	return 1;
 bad:
	arj_done(pax, ap);
	return r;
}

static int
arj_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	register unsigned char*	buf;
	char*			s;
	Tm_t			tm;
	unsigned long		dostime;
	long			n;
	int			mode;
	uint32_t		checksum;
	Codexdata_t		sum;

	for (;;)
	{
		if (!(buf = (unsigned char*)paxget(pax, ap, -4, NiL)) || !(n = swapget(3, buf+2, 2)))
			return 0;
		if (n < 32 || n > 2600 || swapget(3, buf, 2) != MAGIC || !(buf = (unsigned char*)paxget(pax, ap, n + 4, NiL)))
			break;
		sfsync(ar->sum);
		sfwrite(ar->sum, buf, n);
		codexdata(ar->sum, &sum);
		checksum = swapget(3, buf + n, 4);
		if (checksum != sum.num)
			break;
		ar->system = buf[3];
		ar->type = buf[4];
		dostime = swapget(3, buf+8, 4);
		f->st->st_size = swapget(3, buf+12, 4);
		f->uncompressed = swapget(3, buf+16, 4);
		switch (ar->index = buf[5])
		{
		case 0:
			sfsprintf(ar->method, sizeof(ar->method), "copy|%s", SUM);
			break;
		case 1:
		case 2:
		case 3:
			sfsprintf(ar->method, sizeof(ar->method), "lzh-26624-s1+SIZE=%I*u|%s", sizeof(f->uncompressed), f->uncompressed, SUM);
			break;
		default:
			*ar->method = 0;
			break;
		}
		ar->checksum = swapget(3, buf+20, 4);
		mode = swapget(3, buf+26, 2);
		s = (char*)buf + buf[0];
		f->name = paxstash(pax, &ap->stash.head, NiL, strlen(s) + 1);
		strcpy(f->name, s);
		while ((s = paxget(pax, ap, 2, NiL)) && (n = swapget(3, s, 2)))
			if (paxseek(pax, ap, n + 4, SEEK_CUR, 0) < 0)
				goto bad;
		f->linkpath = 0;
		f->st->st_dev = 0;
		f->st->st_ino = 0;
		if (ar->system != 3)
		{
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
		}
		f->st->st_mode = mode & pax->modemask;
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
 bad:
	return paxcorrupt(pax, ap, f, NiL);
}

static int
arj_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
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
		if (!*ar->method || (pop = codex(sp, ar->method, CODEX_DECODE, &ar->codexdisc, NiL)) < 0)
			(*pax->errorf)(NiL, pax, 2, "%s: %s: cannot decode method %d", ap->name, f->name, ar->index);
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

Paxformat_t	pax_arj_format =
{
	"arj",
	0,
	"arj archive",
	0,
	PAX_ARCHIVE|PAX_DOS|PAX_NOHARDLINKS|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(arj),
	0,
	arj_done,
	arj_getprologue,
	arj_getheader,
	arj_getdata,
};

PAXLIB(arj)
