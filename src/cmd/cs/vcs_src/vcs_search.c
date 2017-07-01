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
/*
 *	search.c
 *		cmptime()
 *		search_tag
 *		gettagbyspec()
 *		lookup_tagtime()
 *		lookup_tag_t()
 *		pattern2time()
 *		gettagbytime()
 *		gettagbyname()
 *		rs_readlink()
 */
#include "vcs_rscs.h"
#include <ls.h>
#include <tm.h>


int cmptime(s1, s2)
	const void*	s1;
	const void*	s2;
{
#define TAGTIME(s)	((*((rdirent_t**)s))->tag->stat.st_mtime)
	return (TAGTIME(s1) - TAGTIME(s2));
}

/*
 *	versions 	::	version
 *			 | 	version ; versions
 *			 | 	NULL
 *	version  	::	id 
 *			 |	id distance 
 *	id	 	::	string | date_spec
 *	date_spec 	:: 	[ "(" ] date [ ")" ]
 *	distance	::	[ "+" | "-" ] integer
 *	date		::	month day [ time | year]
 *	time		::	hh ":" mm [ ":" ss ]
 *
 *
 */
int search_tag(rf, ap, versions, domain, tpp, mode, dir)
	Sfio_t*	rf;
	attr_t*		ap;
	char*		versions;
	int		domain;
	tag_t**		tpp;
	int		mode;
	rdirent_t**	dir;
{
	register rdirent_t*	list;
	int			r;
	tag_t*			tp;

	if ((list = rs_dir(rf, ap)) == NULL)
		return (-1);
	if (dir)
		*dir = list;
	if (!versions)
		r = lookup_tag(list, rf, ap, versions, domain, &tp, 0);
	else
		r = gettagbyspec(list, versions, domain, &tp);
	
	if (r)
		return (r);

	if (R_ISLINK(tp) && (mode & G_LINK))
	{
		if ((tp = gettagbyname(list, tp->version, G_LINK, 0)) == NULL)
			return (-1);
	}
	*tpp = tp;
	return (0);
}



int gettagbyspec(list, versions, domain, tpp)
	register rdirent_t*	list;
	char*			versions;
	int			domain;
	tag_t**			tpp;
{
	char			buf[2048];
	register char*		s;
	char*			elist[MAXDEPTH+1];
	rdirent_t*		tbl[MAXDEPTH+1];
	register int		i;
	register rdirent_t*	entry;
	int			sort = 0;
	int			total;
	int			num;
	int			flag;
	int			index;
	int			distance;
	time_t			to;


	for (entry=list, i=0; entry; entry = entry->next, i++)
		tbl[i] = entry;
	total = i;

	strcpy(buf, versions);
	s = buf;
	elist[num=chop(s, elist, MAXDEPTH, ';')] = NULL;

	for (i = 0; i < num; i++)
	{
		distance = 0;
		if ((s = strchr(elist[i], '-')) || (s = strchr(elist[i], '+')))
		{
			flag = (*s == '+') ? 0 : 1;
			*s = '\0';
			s++;
			distance = (int)strtol(s, (char**)0, 0);
			if (!sort)
			{
				qsort((char *)tbl, total, sizeof(rdirent_t *), cmptime);
				sort++;
			}
		}
		if ((index = lookup_tag_t(tbl, total, elist[i], domain, 0)) < 0)
		{
			if (pattern2time(elist[i], &to) < 0)
				continue;
			if (!sort)
			{
				qsort((char *)tbl, total, sizeof(rdirent_t *), cmptime);
				sort++;
			}
			if ((index = lookup_tag_time(tbl, total, to, domain, 0)) < 0)
				continue;
		}
		if (!distance)
		{
			*tpp = tbl[index]->tag;
			return (0);
		}
		if (!flag) 
			index += distance;
		else
			index -= distance;
		if (index >=0 && index < total)
		{
			*tpp = tbl[index]->tag;
			return (0);
		}
	}
	return (-1);
}

int lookup_tag_time(tbl, total, to, domain, first)
	register rdirent_t**	tbl;
	int			total;
	time_t			to;
	int			domain;
	int			first;
{
	register int		i;
	int			r;

	r = -1;

	for (i = 0; i < total; i++)
	{
		if (tbl[i]->tag->stat.st_mtime > to)
			return (r);
		if (domain && tbl[i]->tag->domain != domain)
			continue;
		if (first)
			return (i);
		r =  i;
	}
	return (r);
}


		
/*
 *	mode = 0 	any;
 *	mode = 1	domain must match 
 */

int lookup_tag(entries, rf, ap, version, domain, tpp, mode)
	rdirent_t*		entries;	
	Sfio_t*		rf;
	attr_t*			ap;
	char*			version;
	int			domain;
	tag_t**			tpp;
	int			mode;
{
	register rdirent_t*	list;
	register rdirent_t*	p;
	register char*		v;
	rdirent_t*		sp;
	rdirent_t*		ep;
	time_t			mtime;
	int			cnt = 0;

	if (entries)
		list = entries;
	else if ((list = rs_dir(rf, ap)) == NULL)
		return (-1);

	if (version)
		v = version;
	else
		v = "LATEST";

	for (sp = NULL, ep = NULL, p = list; p; p = p->next)
	{
		if (strcmp(p->tag->version, v) == 0)
		{
			if (domain)
			{
				if (p->tag->domain == domain)
				{
					tpp = &(p->tag);
					return (0);
				}
			}
			else
			{
				cnt++;
				if (!sp)
					ep = sp = p;
				else
					ep = p;
			}
		}
	}
	if (mode)
		return (-1);

	switch (cnt)
	{
	case 0:
		if (strcmp(v, "BASE") == 0)
		{
			/* return the base tag */
			for (p =list; p; p = p->next)
			{
				if (ISBASE(p->tag))
				{
					tpp = &(p->tag);
					return (0);
				}
			}
			message((0, "can find the base"));
			return (-1);
		}
		else if (strcmp(v, "LATEST") == 0)
		{
			/* return the latest one */
			for (p = list, mtime = 0L; p; p = p->next)
			{
				if (p->tag->stat.st_mtime > mtime)
				{
					*tpp = p->tag;
					mtime = p->tag->stat.st_mtime;
				}
			}
			return (0);
		}
		else
			return (-1);
	case 1:
		*tpp = sp->tag;
		return (0);
	default:
		/*
	 	 *	return the latest one 
	 	 */
		for (p = sp, mtime = 0L; p != ep->next; p = p->next)
		{
			if (p->tag->stat.st_mtime > mtime)
			{
				*tpp = p->tag;
				mtime = p->tag->stat.st_mtime;
			}
		}
		return (0);
	}
}

		
/*
 *	mode = 0 	any;
 *	mode = 1	domain must match 
 */

int lookup_tag_t(tbl, total, version, domain, mode)
	register rdirent_t**	tbl;
	int			total;
	char*			version;
	int			domain;
	int			mode;
{
	register char*		v;
	register int		i;
	int			cnt = 0;
	int			r;


	if (version && *version)
		v = version;
	else
		v = "LATEST";

	for (i = 0; i < total; i++)
	{
		if (strcmp(tbl[i]->tag->version, v) == 0)
		{
			if (domain)
			{
				if (tbl[i]->tag->domain == domain)
					return (i); 
			}
			else
			{
				cnt++;
				r = i;
			}
		}
	}

	if (mode)
		return (-1);

	if (cnt)
		return (r);

	if (strcmp(v, "BASE") == 0 || strcmp(v, "base") == 0)
	{
		for (i = 0; i < total; i++)
		{
			if (ISBASE(tbl[i]->tag))
				return (i);
		}
		message((0, "can find the base"));
		return (-1);
	}
	else if (strcmp(v, "LATEST") == 0 || strcmp(v, "LATEST") == 0)
		return (total-1);
	else
		return (-1);

}
				
static char*  MONTH[12]=
{
	"Jan", "Feb", "Mar", 
	"Apr", "May", "Jun", 
	"Jul", "Aug", "Sep", 
	"Oct", "Nov", "Dec",
};



int pattern2time(pattern, to)
	char*		pattern;
	time_t*		to;
{
	char		buf[1024];
	register char*	s;
	int		dd, yy, hh, mm, ss;
	char		month[10];
	char		tbuf[128];
	int		flag = 0;
	int		mon;


	message((5, "pattern2time ++ [%s]", pattern)); 

#define LINIT()	(yy = 0, ss = 59, mm = 59, hh = 23)

	LINIT();

	if (*pattern == '(')
	{
		for (s=buf, pattern++; *pattern && *pattern != ')'; s++, pattern++)
		{
			*s = *pattern;
			if (*s == '/')
				flag++;
		}
		*s = '\0';
		s = buf;
	}
	else
	{
		s = pattern;
		if (strchr(s, '/'))
			flag++;
	}
	if (flag)
	{
		if ((sfsscanf(s, "%d/%d/%d", &mon, &dd, &yy)) != 3)
			return (-1);
		if (mon <= 12)
			strcpy(month, MONTH[--mon]);
		else if (dd <= 12)
		{
			strcpy(month, MONTH[--dd]);
			dd = mon;
		}
		else
			return (-1);
	}
	else if (
	   (LINIT(), sfsscanf(s, "%s %d %d %d:%d:%d", month, &dd, &yy, &hh, &mm, &ss) != 6) &&
           (LINIT(), sfsscanf(s, "%s %d %d:%d:%d", month, &dd, &hh, &mm, &ss) != 5) &&
	   (LINIT(), sfsscanf(s, "%s %d %d %d:%d", month, &dd, &yy, &hh, &mm) != 5) &&
	   (LINIT(), sfsscanf(s, "%s %d %d:%d", month, &dd, &hh, &mm) != 4) &&
	   (LINIT(), sfsscanf(s, "%s %d %d", month, &dd, &yy) != 3) &&
	   (LINIT(), sfsscanf(s, "%s %d", month, &dd) != 2))
	{
		message((5, "can't scan --")); 
		return (-1);
	}



	if (yy)
		sfsprintf(tbuf, sizeof(tbuf), "%s %d %d %d:%d:%d", month, dd, yy, hh, mm, ss);
	else
		sfsprintf(tbuf, sizeof(tbuf), "%s %d %d:%d:%d", month, dd, hh, mm, ss);

	message((3, "tbuf [from] %s", tbuf)); 
	*to = tmdate(tbuf, (char **) 0, (time_t *) 0);
	return (0);
}

tag_t* gettagbytime(list, mtime)
	rdirent_t*		list;
	register time_t		mtime;
{
	register rdirent_t*	ep;

	for(ep =list; ep; ep =ep->next)
	{
		if (ep->tag->stat.st_mtime == mtime)
			return (ep->tag);
	}
	return (NULL);
}

tag_t* gettagbyname(list, name, mode, level)
	rdirent_t*	list;
	char*		name;
	int		mode;
	int		level;
{
	register rdirent_t*	ep;

	if (level > MAXLINKS)
		return (NULL);

	
	for (ep = list; ep; ep = ep->next)
	{
		if (strcmp(ep->tag->version, name) == 0)
		{
			if (R_ISLINK(ep->tag) && (mode & G_LINK))
			{
				return (gettagbyname(list, ep->link, mode, level +1));
			}
			return (ep->tag);
		}
	}
	return (NULL);
}


char* rs_readlink(s)
	char*	s;
{
	register char*	t;

	if ((t = strrchr(s, CHRLINK)) == NULL)
		return (NULL);
	
	*t = '\0';
	return (++t);
}
