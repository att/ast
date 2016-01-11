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
#include "vcs_replica.h"
#include <dirent.h>
#include <tm.h>

char* getrdir(rp, buf, bufsize)
	register char*	rp;
	char*		buf;
	int		bufsize;
{
	register char*	s;

	strcpy(buf, rp);
	if (!(s = pathcanon(buf, bufsize, PATH_PHYSICAL|PATH_EXISTS)))
		return 0;
	*s++ = '/';
	strcpy(s, REPL_DIR);
	return buf;
}

int replica_creat(path, sf)
	char*		path;
	Sfio_t*		sf;
{
	register char*		rf;
	register char*		rd;
	char			dirbuf[1024];
	register DIR*		dir;
	register struct dirent*	rca;
	register char*		s;
	register char*		e;
	Sfio_t*			fd;
	int			cnt = 0;
	struct stat		st;

	if (((rd = getrdir(path, dirbuf, sizeof(dirbuf))) == NULL) || (dir = opendir(rd)) == NULL)
		return (-1);

	e = rd + sizeof(dirbuf);
	for(s = rd; *s; s++);

	if ((rf = strrchr(path, '/')))
		rf++;
	else
		rf = path;
	
	
	while((rca = readdir(dir)))
	{
		/*
		 * skip all entries staring with ``.''
		 */

		if (rca->d_name[0] == '.')
			continue;

		/*
		 * skip regular file 
		 */
		sfsprintf(s, e - s, "/%s", rca->d_name);
		if (stat(dirbuf, &st) || !S_ISDIR(st.st_mode))
			continue;

		sfsprintf(s, e - s, "/%s/%s", rca->d_name, rf);
		if ((fd = sfopen(NULL, dirbuf, "w")))
		{
			sfseek(sf, 0L, 0);
			sfmove(sf, fd, -1, -1);
			sfclose(fd);
			cnt++;
		}
	}
	if (cnt)
	{
		/* send message to the server */
		

	}

	return (cnt);

}

int replica(path, df, tp)
	char*		path;
	Sfio_t*		df;
	tag_t*		tp;
{
	register char*		rf;
	register char*		rd;
	char			dirbuf[1024];
	register DIR*		dir;
	register struct dirent*	rca;
	register char*		s;
	register char*		e;
	Sfio_t*			fd;
	int			cnt = 0;
	struct stat		st;


	if (((rd = getrdir(path, dirbuf, sizeof(dirbuf))) == NULL) || (dir = opendir(rd)) == NULL)
		return (-1);

	e = rd + sizeof(dirbuf) - 1;
	for(s = rd; *s; s++);

	if ((rf = strrchr(path, '/')))
		rf++;
	else
		rf = path;
	
	
	while((rca = readdir(dir)))
	{
		/*
		 * skip all entries staring with ``.''
		 */

		if (rca->d_name[0] == '.')
			continue;

		/*
		 * skip regular file 
		 */
		sfsprintf(s, e - s, "/%s", rca->d_name);
		if (stat(dirbuf, &st) || !S_ISDIR(st.st_mode))
			continue;
		sfsprintf(s, e - s, "/%s/%s.%d", rca->d_name, rf, cs.time);
		if ((fd = sfopen(NULL, dirbuf, "a")))
		{
			sfwrite(fd,(char *)tp,tp->length); 
			if (df)
			{
				sfseek(df, 0L, 0); 
				sfmove(df,fd,-1,-1); 
			}
			sfclose(fd);
			cnt++;
		}
	}
	if (cnt)
	{
		/* send message to the server */
		

	}

	return (cnt);

}
