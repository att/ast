/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2012 AT&T Intellectual Property          *
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
/*
 *	tag.c
 *
 */

#include "vcs_rscs.h"

/*
 *	Always point to next tag after call 
 */

tag_t*	get_tag(f, tp)
	Sfio_t*	f;
	register tag_t*	tp;
{
	register char *s;
	int	len;

	s = (char *)tp + sizeof(int);
	(void) memset((char *)tp, 0, sizeof(tag_t));
	if (!sfread(f, (char *)&(tp->length), sizeof(int)) || (len = tp->length - sizeof(int)) && sfread(f, (char *)s, len) != len)
		return (NULL);

	if (tp->type & LOG)
	{
		tp->del = WHERE(f);
		ADVANCE(f, tp->dsize);
	}
	
	return (tp);
}



int new_tag(tp, sp, v, dsize, domain, type)
	register tag_t*	tp;
	struct stat*	sp;
	char*		v;
	int		dsize;
	int		domain;
	int		type;
{
	memset((char *)tp, 0, sizeof(tag_t));
	tp->type = type;
	tp->dsize = dsize;
	tp->stat = *sp;
	tp->stat.st_ctime = cs.time;
	tp->domain = (domain ? domain : getmydomain());
	strcpy(tp->version, v);
	tp->length = sizeof(tag_t) - MAXVID + strlen(v) + 1;
	return (tp->length);
}

int keycmp(tp1, tp2)
	register tag_t*	tp1;
	register tag_t*	tp2;
{
	int	n;

	if ((n = strcmp(tp1->version, tp2->version)))
		return (n);

	return (tp1->domain - tp2->domain);
}
