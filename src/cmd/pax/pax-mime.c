/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
 * pax mime format
 */

#include "format.h"

#define MIME_HEADER	1024

typedef struct Mime_s
{
	size_t		fill;		/* last member filler size	*/
	size_t		length;		/* separator magic length	*/
	char		magic[1];	/* separator magic		*/
} Mime_t;

static int
mime_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	register Mime_t*	mime;
	char*			s;
	char*			t;
	size_t			n;

	s = state.tmp.buffer;
	if (paxread(pax, ap, s, (off_t)0, (off_t)MIME_HEADER, 0) <= 0)
		return 0;
	paxunread(pax, ap, s, MIME_HEADER);
	if (s[0] != '-' || s[1] != '-' || !(t = (char*)memchr(s, '\n', MIME_HEADER - 2)) || (t - s + 8) >= MIME_HEADER || strncasecmp(t + 1, "Content", 7))
		return 0;
	n = t - s + 1;
	if (t > s && *(t - 1) == '\r')
		t--;
	n = t - s;
	if (!(mime = newof(0, Mime_t, 1, n)))
	{
		nospace();
		return -1;
	}
	ap->data = mime;
	mime->length = n;
	memcpy(mime->magic, s, mime->length);
	message((-1, "mime magic `%s'", mime->magic));
	paxread(pax, ap, NiL, (off_t)0, n, 0);
	return 1;
}

static int
mime_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
mime_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	register Mime_t*	mime = (Mime_t*)ap->data;
	register char*		s;
	register char*		t;
	register char*		v;
	off_t			m;
	off_t			b;
	size_t			n;
	int			loop;

	if (paxread(pax, ap, s = state.tmp.buffer, mime->length + 2, mime->length + 2, 1) <= 0 || memcmp(s, mime->magic, mime->length))
		error(3, "%s: corrupt %s format member header -- separator not found", ap->name, ap->format->name);
	else if (*(s += mime->length) == '-' && *(s + 1) == '-')
	{
		while (paxread(pax, ap, s, 1, 1, 1) > 0 && *s != '\n');
		return 0;
	}
	else if (*s == '\n')
		paxunread(pax, ap, s + 1, 1);
	else if (*s != '\r' && *(s + 1) != '\n')
		error(3, "%s: corrupt %s format member header -- separator line not found", ap->name, ap->format->name);
	f->name = 0;
	for (;;)
	{
		for (t = (s = state.tmp.buffer) + state.buffersize - 1; s < t; s++)
			if (paxread(pax, ap, s, 1, 1, 1) <= 0)
				error(3, "%s: unexpected %s format EOF", ap->name, ap->format->name);
			else if (*s == '\n')
			{
				if (s > state.tmp.buffer && *(s - 1) == '\r')
					s--;
				*s = 0;
				break;
			}
		s = state.tmp.buffer;
		if (!*s)
			break;
		if (strncasecmp(s, "content-", 8))
			error(3, "%s: corrupt %s format member header", ap->name, ap->format->name);
		if (t = strchr(s, ':'))
			s = t + 1;
		while (isspace(*s))
			s++;
		for (;;)
		{
			for (t = s; *s && *s != ';' && *s != '='; s++);
			if (!(n = s - t))
				break;
			if (*s == '=')
			{
				if (*++s == '"')
					for (v = ++s; *s && *s != '"'; s++);
				else
					for (v = s; *s && *s != ';'; s++);
			}
			else
				v = s;
			m = s - v;
			if (*s)
				*s++ = 0;
			for (; *s == ';' || isspace(*s); s++);
			if (!f->name && n == 4 && !strncasecmp(t, "name", 4) || n == 8 && !strncasecmp(t, "filename", 8))
				f->name = paxstash(pax, &ap->stash.head, v, m);
		}
	}
	if (!f->name)
	{
		if (s = strrchr(ap->name, '/'))
			s++;
		else
			s = ap->name;
		f->name = paxstash(pax, &ap->stash.head, s, strlen(s) + 16);
		sfsprintf(f->name, ap->stash.head.size, "%s-%d", s, ap->entries + 1);
	}
	if (!ap->io->seekable)
		seekable(ap);
	f->st->st_size = 0;
	loop = 0;
	b = paxseek(pax, ap, 0, SEEK_CUR, 0);
	while (s = paxget(pax, ap, 0, &m))
	{
		if (m < mime->length)
		{
			if (loop++)
				error(3, "%s: corrupt %s format member header [too short]", ap->name, ap->format->name);
			paxseek(pax, ap, -m, SEEK_CUR, 0);
			paxsync(pax, ap, 0);
			continue;
		}
		v = s;
		for (t = s + m - mime->length; s = memchr(s, '-', t - s); s++)
			if (!memcmp(s, mime->magic, mime->length))
			{
				paxseek(pax, ap, b, SEEK_CUR, 0);
				paxsync(pax, ap, 0);
				if (s > v && *(s - 1) == '\n')
				{
					mime->fill++;
					if (s > (v + 1) && *(s - 2) == '\r')
						mime->fill++;
				}
				f->st->st_size += (s - v) - mime->fill;
				f->st->st_mtime = NOW;
				f->st->st_mode = X_IFREG|X_IRUSR|X_IRGRP|X_IROTH;
				f->st->st_uid = state.uid;
				f->st->st_gid = state.gid;
				f->st->st_dev = 0;
				f->st->st_ino = 0;
				f->st->st_nlink = 1;
				IDEVICE(f->st, 0);
				f->linktype = NOLINK;
				f->linkpath = 0;
				f->uidname = 0;
				f->gidname = 0;
				return 1;
			}
		paxseek(pax, ap, -(off_t)mime->length, SEEK_CUR, 0);
		paxsync(pax, ap, 0);
		f->st->st_size += m;
	}
	return 0;
}

Format_t	pax_mime_format =
{
	"mime",
	0,
	"encapsulated mime",
	0,
	ARCHIVE|DOS|NOHARDLINKS|IN,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(mime),
	0,
	mime_done,
	mime_getprologue,
	mime_getheader,
};

PAXLIB(mime)
