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

#define USAGE "list [mount-point ...]"

int im_list_help(s)
	char*	s;
{
	printf("\t%s\n", USAGE);
	return (0);
}

int im_list(argc, argv)
	int 		argc;
	register char**	argv;
{
	register int	n;
	register char*	s;
	int		seq;
	int		fd;
	int		len;
	char*		mpoint;
	char		buf[1024];
	char		reply[1024];

	error_info.id = argv[0];
	memset(buf, 0, sizeof(buf));
	memset(reply, 0, sizeof(reply));

	opt_info.index = 1;
	while (n = optget(argv, "s:[server] "))
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
	case '?':
	case ':':
		printf(USAGE);
		return (1);
	}
	argc -= opt_info.index;
	argv += opt_info.index;

	if (argc == 0)
	{
		seq = -1;
		do
		{
			if (seq == -1)
				sfsprintf(buf, sizeof(buf), "m -0\n");
			else
				sfsprintf(buf, sizeof(buf), "m %d\n", seq);
			if (vcs_write(buf) <= 0 || vcs_read(reply, 1024) <= 0)
				return (-1);
			if (seq == -1)
			{
				if ((s = strstr(reply, "ok 0 ")) == NULL)
				{
					printf("%s", buf);
					return (-1);
				}
				s += 5;
				len = (int)strtol(s, (char**)0, 0);
			}
			else
				printmtmsg(reply);
			seq++;
		} while (seq < len);
		return (0);
	}
	else
	{
		while  (argc> 0)
		{
			mpoint = *argv;
			sfsprintf(buf, sizeof(buf), "m %s\n", mpoint);	
			if (vcs_write(buf) > 0 && vcs_read(reply, 1024) > 0)
				printmtmsg(reply);
			argc--;
			argv++;
		}
	}
	return (0);
}
