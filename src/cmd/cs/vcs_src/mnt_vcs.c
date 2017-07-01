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

int vcs_write(buf)
	char*	buf;
{
	int	ret;

	if ((istate.fd == -1) && ((istate.fd = csopen(istate.cs_svc, CS_OPEN_READ)) < 0))
	{
		printf("cannot connect cs server %s\n", istate.cs_svc);
		return (-1);
	}
	if ((ret = write(istate.fd, buf, strlen(buf))) < 0)
	{
		printf("cannot write to cs server %s\n", istate.cs_svc);
		istate.fd = -1;
		return (-1);
	}
	return (ret);
}

int vcs_read(buf, bufsize)
	char*	buf;
	int	bufsize;
{
	int	ret;
	Cs_poll_t	ack;
	if ((istate.fd == -1) && ((istate.fd = csopen(istate.cs_svc, CS_OPEN_READ)) < 0))
	{
		printf("cannot connect cs server %s\n", istate.cs_svc);
		return (-1);
	}
	ack.fd = istate.fd;
	ack.events = CS_POLL_READ;
	if (cspoll(&ack, 1, -1) != 1)
	{
		printf("no msg from server\n");
		close(istate.fd);
		istate.fd = -1;
		return (-1);
	}
	ret = read(istate.fd, buf, bufsize);
	if (ret <= 0)
	{
		printf("read error\n");
		close(istate.fd);
		istate.fd = -1;
	}
	else
		buf[ret] = '\0';
	return (ret);	
}

int im_vcs_help(s)
	register char*	s;
{
	if (strcmp(s, "cs") == 0)
		printf("\tcs commands\n");
	else if (strcmp(s, "version") == 0)
		printf("\tversion # display server's ID\n");
	else if (strcmp(s, "kill") == 0)
		printf("\tkill # kill server \n");
	else if (strcmp(s, "connect") == 0)
		printf("\tconnect [server] # connect server \n");
	else if (strcmp(s, "log") == 0)
		printf("\tlog [file] # log server messages\n");
	return (0);
}

int im_vcs_main(argc, argv)
	int	argc;
	char**	argv;
{
	register char*	s;
	register char*	cmd;
	char		reply[1024];
	int		fd;
	char		buf[2048];
	int		len;

	cmd = error_info.id = argv[0];
	opt_info.index = 1;

	memset(buf, 0, sizeof(buf));
	memset(reply, 0, sizeof(reply));
	argc -= opt_info.index;
	argv += opt_info.index;

	if(strcmp(cmd, "kill") == 0)
	{
		sfsprintf(buf, 2048, "%s\n", cmd);
		(void)vcs_write(buf);
		close(istate.fd);
		istate.fd = -1;
		return (0);
	}
	else if ((strcmp(cmd, "connect") == 0) || (strncmp(cmd, "version", 4) == 0))
	{
		if (argc > 0)
		{
			s = *argv;
			if ((fd = csopen(s, CS_OPEN_READ)) < 0)
			{
				printf("cannot connect cs server %s\n", s);
				return (-1);
			}
			istate.cs_svc = strdup(s);
			istate.fd = fd;
			argc--;
			argv++;
		}
		cmd = "version";
	}
	else if (strcmp(cmd, "log") == 0)
	{
		cmd = "r";
	}
	else if (strcmp(cmd, "cs") == 0)
	{
		cmd = NULL;

	}
	
	if (cmd != NULL)
	{
		len = sfsprintf(buf, 2048, "%s ", cmd);
		s = buf + len;
	}
	else
		s = buf;

	if (argc > 0)
	{
		argv[argc] = NULL;
		arr2str(s, argv, ' ');
	}
	strcat(buf, "\n");
	if (vcs_write(buf) <=0 || vcs_read(reply, 1024) <=0)
	{
		printf("cannot connect cs server\n");
		return (-1);
	}
	if (strncmp(reply, "I ", 2) == 0)
		s = reply + 2;
	else
		s = reply;
	printf("%s", s);
	return (0);
}


void printmtmsg(buf)
	char*	buf;
{
	register char*	url;
	register char*	mnt;

	if( !strncmp( buf, "I 0 ok 0 ", 9 ) &&
	    (url = strtok( buf+9, " \t\r\n" )) &&
	    (mnt = strtok( NULL, " \t\r\n" )) ) {
		printf("%s -> %s\n", mnt, url);
	} else {
		printf( "%s", buf );
	}
}
