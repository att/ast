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
#pragma prototyped

#include "vcs_rscs.h"
#include "vcs_cmd.h"

int	rserrno;
int	debug;			/* take away ? */

int
rscs_out(int argc, char** argv, char* ret, int* n)
{
	rfile_t			rf;
	version_t		vs;
	attr_t			attr;
	tag_t			tag;
	rdirent_t*		list;
	register struct stat*	st;

	if (argc < 2)
	{
		*n = sfsprintf(ret, *n, "E arguments error\n");
		return (-1);
	}
	rf.path = argv[1];
	vs.path = argv[1];
	vs.version = NULL;
	if (argc > 3)
	{
		vs.version = argv[2];
		if (argc > 4)
			vs.path = argv[3];
	}
	
	if ((rf.fd = sfopen(NULL, rf.path, "r")) == NULL)
	{
		*n = sfsprintf(ret, *n, "E %s cannot open for read\n", rf.path);
		return (-1);
	}
	if (get_attr(rf.fd, &attr))
	{
		*n = sfsprintf(ret, *n, "E %s not vcs\n", rf.path);
		sfclose(rf.fd);
		return (-1);
	}
	rf.ap = &attr;
	vs.tp = &tag;
	if (search_tag(rf.fd, rf.ap, vs.version, 0, &(vs.tp), G_LINK, &list))
	{
		*n = sfsprintf(ret, *n, "E version %s not existed\n", vs.version);
		return (-1);
	}
	(void) unlink(vs.path);
	if ((vs.fd = sfopen(NULL, vs.path, "w")) == NULL)
	{
		sfclose(rf.fd);
		*n = sfsprintf(ret, *n, "E %s cannot open for write\n", vs.path);
		return (-1);
	}
	if (checkout(&rf, &vs) < 0)
	{
		sfclose(rf.fd); sfclose(vs.fd);
		*n = sfsprintf(ret, *n, "E checkout\n");
		return (-1);
	}
	sfsync(vs.fd); 
	sfclose(rf.fd); sfclose(vs.fd);
	st = &(vs.tp->stat);
	(void) touch(vs.path, st->st_atime, st->st_mtime, 1);
	(void) chmod(vs.path, st->st_mode);
	(void) chown(vs.path, st->st_uid, st->st_gid);
	*n = sfsprintf(ret, *n, "I %s(%s) checkout\n", vs.path, vs.tp->version);
	return (0);
}

int
rscs_instances(int argc, char** argv, char* ret, int* n)
{
	register Sfio_t*	fd;
	attr_t			attr;
	register rdirent_t*	list;
	register char*		s;
	register int		len;
	register int		rsize;
	register char*		path;

	path = argv[1];

	if ((fd = sfopen(NULL, path, "r")) == NULL)
	{
		*n = sfsprintf(ret, *n, "E %s cannot open for read\n", path);
		return (-1);
	}
	if (get_attr(fd, &attr))
	{
		*n = sfsprintf(ret, *n, "E %s not vcs\n", path);
		sfclose(fd);
		return (-1);
	}
	if ((list = rs_dir(fd, &attr)) == NULL)
	{
		*n = sfsprintf(ret, *n, "I no entry\n");
		sfclose(fd);
		return (0);
	}
	
	sfclose(fd);

	s = ret; 
	rsize = *n;
	len = sfsprintf(s, rsize, "I <");
	s += len;
	rsize -= len;
	while(list != NULL)
	{
		len = sfsprintf(s, rsize, "%s ", list->tag->version);
		s += len;
		rsize -= len;
		list = list->next;
	}
	len = s - ret;
	ret[len-1] = '>';
	ret[len] = '\n';
	ret[len+1] = '\0';
	*n = len + 2;
	return (0);
}
