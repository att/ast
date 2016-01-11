/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
#include "vcs_rscs.h"
#include "vcs_cmd.h"

tag_t*	getmarker();

int rs_marker(rp, from, to, st, expire, force)
	register rfile_t*	rp;
	register char*		from;
	register char*		to;
	register struct stat*	st;
	time_t			expire;
	int			force;
{
	rdirent_t*		dir;
	tag_t*			tp;
	tag_t*			dump;
	tag_t			tag;
	char			buf[1024];

	tp = NULL;
	if (from && *from && search_tag(rp->fd, rp->ap, from, 0, &tp, G_LINK, &dir))
	{
		message((0, "Version %s not found", from));
		return (-1);
	}

	if (!gettagbyspec(dir, to, 0, &dump))
	{
		message((0, "Version %s existed", to));
		return (-1);
	}

	sfsprintf(buf, sizeof(buf), "%s%c%s", to, CHRMARKER, (tp ? tp->version : NULL));
	
	if (!force && (dump = getmarker(dir, buf)))
	{
		if (!R_ISMARKER(dump))
		{
			message((0, "Version %s existed", buf));
			return (-1);
		}
		if (dump->stat.st_ino != st->st_ino)
		{
			message((0, "Marker %s existed", buf));
			return (-1);
		}
	}
	tp = &tag;
	new_tag(tp, st, buf, 0, 0, LOG | MARKER);
	tp->stat.st_mtime = expire;
	(void) sfseek(rp->fd, 0L, 2);
	sfwrite(rp->fd, (char *)tp, tp->length);
	replica(rp->path, NULL, tp);
	return (0);
}


tag_t* getmarker(dir, s)
	rdirent_t*	dir;
	register char*	s;
{
	register rdirent_t*	ep;

	for (ep=dir; ep ; ep = ep->next)
	{
		if (strcmp(ep->tag->version, s) == 0)
			return (ep->tag);
	}
	return (NULL);
}

tag_t* getmarkerbyfrom(dir, s)
	rdirent_t*	dir;
	register char*	s;
{
	register rdirent_t*	ep;
	register char*		t;
	register rdirent_t*	r;

	r = NULL;
	for (ep = dir; ep; ep = ep->next)
	{
		if (!R_ISMARKER(ep->tag))
			continue;
		if ((t = strrchr(ep->tag->version, CHRMARKER)) == NULL)
			continue;

		if (!(*++t))
		{
			if (!r || r->tag->stat.st_ctime < ep->tag->stat.st_ctime)
				r = ep;
			continue;
		}

		if (strcmp(t, s) == 0)
		{
			if (!r || r->tag->stat.st_ctime < ep->tag->stat.st_ctime)
				r = ep;
		}

	}
	if (r)
		return (r->tag);
	else
		return (NULL);
}

tag_t* getmarkerbyto(dir, s)
	rdirent_t*	dir;
	register char*	s;
{
	register rdirent_t*	ep;
	register rdirent_t*	r;

	r = NULL;

	for (ep = dir; ep; ep = ep->next)
	{
		if (!R_ISMARKER(ep->tag))
			continue;
		if (markermatch(ep->tag->version, s))
		{
			if (!r || r->tag->stat.st_ctime < ep->tag->stat.st_ctime)
				r = ep;
		}

	}
	if (r)
		return (r->tag);
	else
		return (NULL);
}

			
				

	
int markermatch(marker, s)
	char*	marker;
	char*	s;
{
	register char*	t;
	int		i;

	for (i = 1, t = marker; *t && *t != CHRMARKER; t++, i++);
	return *t && strncmp(marker, s, i) == 0 && s[i] == '\0';
}
