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
#include "mnt_imount.h"

#include <ctype.h>

#define USAGE 	"mount [-f mount_file]\
 [mount-point prot:/[user[:pass]@]host[:port][/remote-path]]"

char*	getpass();

/*
 *	prot:/user:pass@host:port/remote-path
 */

char* checkfs(data, buf, size)
	char*	data;
	char*	buf;
	int	size;
{
	register char*	s;
	register char*	t;
	char	psbuf[1024];
	char*	pass;

	if (!(s = strchr(data, '-')) || s[1] != '@') {
		strcpy(buf, data);
		return(buf);
	}
	t = strchr( s, '/' );
	if( t )  *t = '\0';
	sfsprintf( psbuf, sizeof(psbuf), "passwd (%s): ", data );
	if( t )  *t = '/';
	pass = getpass(psbuf);
	*s = '\0';
	sfsprintf( buf, size, "%s%s%s", data, pass, s+1 );
	*s = '-';
	return( buf );
}


int im_mount_help(s)
	char*	s;
{
	printf("\t%s # mount a remote file server\n", USAGE);
	return (0);
}

int im_mount(argc, argv)
	int		argc;
	register char**	argv;
{
	register char*	s;
	register int	n;
	char		buf[1024 * 2];
	char		reply[1024 * 2];
	char		mfsbuf[1024 * 2];
	int		fd;
	char*		mpoint;
	char*		mfs;
	FILE*		mfd = NULL;

	error_info.id = argv[0];
	memset(buf, 0, sizeof(buf));
	memset(reply, 0, sizeof(reply));
	memset(mfsbuf, 0, sizeof(mfsbuf));

	opt_info.index = 1;
	while (n = optget(argv, "s:[server]f:[mount-file] [mount-point prot,host,user,pass,time:remote-path]"))
	 switch (n)
	{
	case 's':
		s = opt_info.arg;
		if ((fd = csopen(s, CS_OPEN_READ)) < 0)
		{
			printf("cannot connect cs server %s\n", s);
			return (-1);
		}
		istate.cs_svc = strdup(s);
		istate.fd = fd;
		break;
	case 'f':
		if ((mfd = fopen(opt_info.arg, "r")) == NULL)
		{
			printf("cannot connect file %s\n", opt_info.arg);
			return(1);
		}
		break;
	case '?':
	case ':':
		printf("%s\n", USAGE);
		return (1);
	}


	
	argv += opt_info.index;
	argc -= opt_info.index;

	if (mfd == NULL)
	{
		if (argc < 2)
		{
			printf("%s\n", USAGE);
			return (1);
		}
		mpoint = argv[0];
		if ((mfs = checkfs(argv[1], buf, sizeof(buf))) == NULL)
		{
			printf("ERROR: %s\n", USAGE);
			return (1);
		}
		if (callmount(fd, mpoint, mfs, reply) == 0)
		{
			printmtmsg(reply);
			return (0);
		}
		else
		{
			printf("ERROR: %s", reply);
			return (1);
		}
	}

	/* mfd != NULL */

	while((fgets(buf, 2048, mfd)))
	{
		if ((s = strchr(buf, '#')) != NULL)
			*s = '\0';
		for (s = buf; *s && isspace(*s); s++);
		if (*s == '\0')
			continue;
		mpoint = s;
		for (; *s && !isspace(*s); s++);
		if (*s == '\0')
			continue;
		*s = '\0';
		for (s++; *s && isspace(*s); s++);
		if (*s == '\0')
			continue;
		mfs = s;
		for (s; *s && !isspace(*s); s++);
		*s = '\0';
		if ((mfs = checkfs(mfs, mfsbuf, sizeof(mfsbuf))) == NULL)
		{
			printf("ERROR: %s\n", USAGE);
			continue;
		}
		if (callmount(fd, mpoint, mfs, reply) == 0)
		{
			printmtmsg(reply);
			continue;
		}
	}
	return(0);
}
	

int callmount(fd, mpoint, mfs, reply)
	int	fd;
	char*	mpoint;
	char*	mfs;
	char*	reply;
{
	char	buf[1024];

	memset(buf, 0, sizeof(buf));

	if (mfs == NULL)
		sfsprintf(buf, sizeof(buf), "m - %s\n", mpoint);
	else
		sfsprintf(buf, sizeof(buf), "m %s %s\n", mfs, mpoint);
	if (vcs_write(buf) <= 0 || vcs_read(reply, 1024) <= 0)
		return (-1);
	if (strstr(reply, "ok ") != NULL)
		(void)add_entry(mpoint, buf);
	return (0);
}

