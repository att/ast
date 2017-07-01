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
 * pax zoo format
 *
 * header layout snarfed from the public domain unzoo.c
 */

#include <paxlib.h>
#include <codex.h>
#include <sum.h>
#include <swap.h>
#include <tm.h>

#define MAGIC		0xfdc4a7dc
#define SUM		"sum-crc-0xa001"

typedef struct Ar_s
{
	Codexdisc_t	codexdisc;
	Pax_t*		pax;
	Paxarchive_t*	ap;
	char		method[64];
	unsigned int	index;
	unsigned long	checksum;

	unsigned long	head;		/* next member header offset	*/
	unsigned long	data;		/* current member data offset	*/
	unsigned char	majver;		/* major version		*/
	unsigned char	minver;		/* minor version		*/
} Ar_t;

static int
zoo_done(Pax_t* pax, register Paxarchive_t* ap)
{
	register Ar_t*	ar = (Ar_t*)ap->data;

	if (!ar)
		return -1;
	free(ar);
	ap->data = 0;
	return 0;
}

static int
zoo_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*		ar;

	if (size < 34 || swapget(3, buf + 20, 4) != MAGIC)
		return 0;
	if (!(ar = newof(0, Ar_t, 1, 0)))
	{
		if (ar)
			free(ar);
		return paxnospace(pax);
	}
	ar->pax = pax;
	ar->ap = ap;
	ar->head = swapget(3, buf + 24, 4);
	ar->majver = buf[32];
	ar->minver = buf[33];
	ap->data = ar;
	if (ar->head > 34 && size < 42)
	{
		zoo_done(pax, ap);
		return paxcorrupt(pax, ap, NiL, "unexpected EOF");
	}
	codexinit(&ar->codexdisc, pax->errorf);
	return 1;
}

static int
zoo_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	register unsigned char*	buf;
	register char*		s;
	Tm_t			tm;
	long			n;
	int			i;
	int			mode;
	int			dosdate;
	int			dostime;
	int			doszone;
	int			ext;
	int			extdir;
	int			extname;
	int			system;
	char			name[14];

	for (;;)
	{
		if (paxseek(pax, ap, ar->head, SEEK_SET, 1) != ar->head ||
		    !(buf = paxget(pax, ap, 38, NiL)) ||
		    swapget(3, buf, 4) != MAGIC)
			break;
		if (!(ar->head = swapget(3, buf+6, 4)))
			return 0;
		if (buf[30])
			continue;
		ar->data = swapget(3, buf+10, 4);
		dosdate = swapget(3, buf+14, 2);
		dostime = swapget(3, buf+16, 2);
		ar->checksum = swapget(3, buf+18, 2);
		f->uncompressed = swapget(3, buf+20, 4);
		f->st->st_size = swapget(3, buf+24, 4);
		ar->majver = buf[28];
		ar->minver = buf[29];
		switch (ar->index = buf[5])
		{
		case 0:
			sfsprintf(ar->method, sizeof(ar->method), "copy|%s", SUM);
			break;
		case 1:
			sfsprintf(ar->method, sizeof(ar->method), "lzd+SIZE=%I*u|%s", sizeof(f->uncompressed), f->uncompressed, SUM);
			s = "lzd";
			break;
		case 2:
			sfsprintf(ar->method, sizeof(ar->method), "lzh-8k-s1+SIZE=%I*u|%s", sizeof(f->uncompressed), f->uncompressed, SUM);
			break;
		default:
			*ar->method = 0;
			break;
		}
		if (paxread(pax, ap, name, sizeof(name) - 1, 0, 1) != (sizeof(name) - 1))
			break;
		name[sizeof(name) - 1] = 0;
		if (buf[4] == 2)
		{
			if (!(buf = paxget(pax, ap, 5, NiL)))
				break;
			ext = swapget(3, buf+0, 2);
			doszone = buf[2];
		}
		else
		{
			ext = 0;
			doszone = 127;
		}
		if (ext > 0)
		{
			if (!(buf = paxget(pax, ap, 1, NiL)))
				break;
			extname = buf[0];
		}
		else
			extname = 0;
		if (ext > 1)
		{
			if (!(buf = paxget(pax, ap, 1, NiL)))
				break;
			extdir = buf[0];
		}
		else
			extdir = 0;
		if (n = extdir + extname)
		{
			if (!extname)
				n += (i = strlen(name));
			f->name = paxstash(pax, &ap->stash.head, NiL, n + 1);
			if (!extname)
			{
				memcpy(f->name + extdir, name, i);
				f->name[extdir + i] = 0;
			}
			else if (paxread(pax, ap, f->name + extdir, extname, 0, 1) != extname)
				break;
			else
				f->name[extdir + extname + 1] = 0;
			if (extdir)
			{	
				if (paxread(pax, ap, f->name, extdir, 0, 1) != extdir)
					break;
				f->name[extdir - 1] = '/';
			}
		}
		else
			f->name = paxstash(pax, &ap->stash.head, name, 0);
		if (ext > n+2)
		{
			if (!(buf = paxget(pax, ap, 2, NiL)))
				break;
			system = swapget(3, buf, 2);
		}
		else
			system = 0;
		if (ext > n+4)
		{
			if (!(buf = paxget(pax, ap, 3, NiL)))
				break;
			mode = (buf[2] << 16) | (buf[1] << 8) | buf[0];
		}
		else
			mode = 0;
		if (ext > n+7 && !(buf = paxget(pax, ap, 3, NiL)))
			break;
		f->linkpath = 0;
		f->st->st_dev = 0;
		f->st->st_ino = 0;
		if (system == 0 || system == 2)
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
		tm.tm_year = ((dosdate >>  9) & 0x7f) + 80;
		tm.tm_mon =  ((dosdate >>  5) & 0x0f) - 1;
		tm.tm_mday = ((dosdate	    ) & 0x1f);
		tm.tm_hour = ((dostime >> 11) & 0x1f);
		tm.tm_min =  ((dostime >>  5) & 0x3f);
		tm.tm_sec =  ((dostime	    ) & 0x1f) * 2;
		if (doszone < 127)
			tm.tm_sec += 15 * 60 * doszone;
		else if (doszone > 127)
			tm.tm_sec += 15 * 60 * (doszone - 256);
		f->st->st_mtime = f->st->st_ctime = f->st->st_atime = tmtime(&tm, TM_LOCALZONE);
		return 1;
	}
	return paxcorrupt(pax, ap, f, NiL);
}

static int
zoo_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
{
	register Ar_t*	ar = (Ar_t*)ap->data;
	Sfio_t*		sp;
	ssize_t		n;
	int		r;
	int		pop;
	Codexdata_t	sum;

	if (fd < 0 || !f->st->st_size)
		return 1;
	if (paxseek(pax, ap, ar->data, SEEK_SET, 0) != ar->data)
		return paxcorrupt(pax, ap, f, "cannot seek to data");
	if (!*ar->method || ar->majver > 2 || ar->majver == 2 && ar->minver > 1)
	{
		(*pax->errorf)(NiL, pax, 2, "%s: %s: cannot extract %s method=%d version=%d.%d data", ap->name, f->name, ap->format->name, ar->index, ar->majver, ar->minver);
		return -1;
	}
	r = -1;
	if (sp = paxpart(pax, ap, f->st->st_size))
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
				if (paxdata(pax, ap, f, fd, pax->buf, n))
					break;
			}
			codexpop(sp, pop);
		}
	}
	return r;
}

Paxformat_t	pax_zoo_format =
{
	"zoo",
	0,
	"zoo archive",
	0,
	PAX_ARCHIVE|PAX_DOS|PAX_NOHARDLINKS|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(zoo),
	0,
	zoo_done,
	zoo_getprologue,
	zoo_getheader,
	zoo_getdata,
};

PAXLIB(zoo)
