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

char* stamp2version(list, t)
	rdirent_t*	list;
	time_t		t;
{
	char			buf[15];
	register char*		s;
	register int		len;
	register rdirent_t*	et;
	int			seq = -1;
	int			num;

	/* mm.dd.yy:nn 		*/
	(void)tmform(buf, "%x", &t);	
	buf[2] = buf[5] = '.'; 

	if (list == NULL)
		return (memdup(buf, strlen(buf)+1));
		
	len = 8;

	for (et =list; et; et = et->next)
	{
		if (strncmp(et->tag->version, buf, 8) == 0)
		{
			if (strlen(et->tag->version) > 8)
			{
				s = et->tag->version + 9;
				num = (int)strtol(s, (char**)0, 0);
				if (num && num > seq)
					seq = num;
			}
			else
				seq = 0;
		}
	}
	seq++;
	if (seq)
	{
		s = buf + len;
		sfsprintf(s, sizeof(buf) - len, ":%d", seq);
	}
	return (memdup(buf, strlen(buf)+1));
}

int locking(fd)
	Sfio_t*	fd;
{
	return (0);
}


int unlocking(fd)
	Sfio_t*	fd;
{
	return (0);
}

int sfmark(sf)
{
	return (0);
}


int rollback(sf)
{
	return (0);
}

/*  chop(s,w,n,c) - break string into fields
 *  a common usage:
 *		elist[num=chop(s, elist, MAXDEPTH, '/')] = NULL;
 *  s is modified in place with '\0' replacing occurrences of separator char c.
 *  w is an array of n char pointers to receive addresses of the fields.  The
 *  return value gives the number of fields actually found; additional entries
 *  in w are given the address of a null string.
 */

int chop(s,w,n,c)
	register char *s;
	char *w[];
	int n;
	char c;
{
	register int i;

	for (i = 0; i < n && *s;)  
	{
		w[i++] = s;
		while (*s && *s != c)
			s++;
		if (!*s)
	    		break;
		*s++ = '\0';
	}
	while (i < n)
		w[--n] = s;
    	return i;
}


/* 
 * skip(w,n)
 *	skip empty string in w;
 *
 */

int skip(w, n)
	char *w[];
	register int n;
{
	register int i;
	register int j;

	for (i = 0, j = 0; i < n; i++)
		if (w[i][0]) w[j++] = w[i];
	return (j);
}
		



/*
 *	maintain domain table
 */

static char*	DomainTbl[MAXVCSDOMAIN+1];
static int	NumDomain = 0;
static char	MyDomain[20];
static int	MyDomainID = 0;

static void domaininit()
{
	register char*		s;
	register int		i;

	s = (char *)malloc(strlen(DOMAINS) + 1);
	strcpy(s, DOMAINS);
	DomainTbl[NumDomain = chop(s, DomainTbl, MAXVCSDOMAIN, ' ')] = NULL;
	/* first entry is reserved */
	for (i = NumDomain; i ; i--)
		DomainTbl[i] = DomainTbl[i-1];
	NumDomain++;
	DomainTbl[0] = DomainTbl[NumDomain] = NULL;
	
#if 0
	(void) getdomainname(MyDomain, sizeof(MyDomain));
#else
	if ((s = csfull(0)) && (s = strchr(s, '.')))
	{
		register char*	t = MyDomain;
		register char*	e = MyDomain + sizeof(MyDomain) - 1;

		while (t < e && (*t = *++s) && *t != '.')
			t++;
		*t = 0;
	}

#endif
	for (i = 1; i < NumDomain; i++)
	{
		if (strcmp(MyDomain, DomainTbl[i]) == 0)
		{
			MyDomainID = i;
			return;
		}
	}
	message((0, "current domain %s is not registered", MyDomain));
	MyDomainID = 0;
	return; 
}

int getmydomain()
{
	if (!NumDomain)
		domaininit();

	return (MyDomainID);
}


int getdomainbyname(s)
	register char*	s;
{
	register int	i;

	if (!NumDomain)
		domaininit();
	for (i = 0; i < NumDomain; i++)
	{
		if (strcmp(s, DomainTbl[i]) == 0)
			return i;
	}
	return (-1);
}


char* getdomainbyid(i)
	register int i;
{
	if (!NumDomain)
		domaininit();
	if (i >= NumDomain)
		return (NULL);

	return (DomainTbl[i]);
}


/*
 *	return permission for access the version marked by a marker
 *		0 : warnning
 *		1 : warnning & check_out file is read-ony 
 *		2 : checkout is prohibited 
 */

int permission(st, uid, gid)
	register struct stat* 	st;
	register uid_t		uid;
	register uid_t		gid;
{
	if (uid == st->st_uid)
		return (st->st_mode & S_IWUSR ? 0 :
			st->st_mode & S_IRUSR ? 1 : 2);
	else if (gid == st->st_gid)
		return (st->st_mode & S_IWGRP ? 0 :
			st->st_mode & S_IRGRP ? 1 : 2);
	else
		return (st->st_mode & S_IWOTH ? 0 :
			st->st_mode & S_IROTH ? 1 : 2);
}
