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
 *	imount:  generic interface to Ifs internet service
 *
 */

#include "mnt_imount.h"

#include <ctype.h>

int im_mount();
int im_mount_help();
int im_umount();
int im_umount_help();
int im_restart();
int im_restart_help();
int im_vcs_main();
int im_vcs_help();
int im_list();
int im_list_help();
int im_touch();
int im_touch_help();
int im_help();
int im_help_help();
int im_bye();
int im_system();

typedef int 	(*func_t)();
struct command_t 
{
	int		len;
	char*		name;
	func_t		main;
	func_t		help;
	char*		man;
};

#define CMD_MAX		15
#define CMD_MASTER	"imount"
#define ENTRY(s,f,h)		{sizeof(s)-1, s, f, h, NULL}

struct command_t  COMMAND[CMD_MAX] =
{
	ENTRY("mount",	im_mount,	im_mount_help),		/* 0 */
	ENTRY("umount",	im_umount,	im_umount_help), 	/* 1 */
	ENTRY("list",	im_list,	im_list_help),		/* 2 */
	ENTRY("connect",im_vcs_main,	im_vcs_help),		/* 3 */
	ENTRY("kill",	im_vcs_main,	im_vcs_help),		/* 4 */
	ENTRY("reset",	im_restart,	im_restart_help), 	/* 5 */
	ENTRY("version",im_vcs_main,	im_vcs_help),		/* 6 */
	ENTRY("rm", 	im_touch,	im_touch_help),		/* 7 */
	ENTRY("log",	im_vcs_main,	im_vcs_help),		/* 8 */
	ENTRY("cs",	im_vcs_main,	im_vcs_help),		/* 9 */
	ENTRY("!",	im_system,	NULL),			/* 10 */
	ENTRY("sh", 	im_system,	NULL),			/* 11 */
	ENTRY("help",	im_help,	im_help_help),		/* 12 */
	ENTRY("quit",	im_bye,		NULL),			/* 13 */
	ENTRY("exit", 	im_bye,		NULL),			/* 14 */
};

#define cmdeq(s,i,l)	(strncmp(s, COMMAND[i].name, (l<COMMAND[i].len?l:COMMAND[i].len)) == 0)

struct istate_t istate=
{
	"/dev/tcp/local/vcs/user",	/* default cs server */
	-1,				/* fd */
	NULL,				/* hash table */
};

main(argc, argv)
	int	argc;
	char**	argv;
{
	register char*	cmd;
	register int	i;
	register char*	s;
	int		ret;
	char		buf[1024];
	char*		elist[20 + 1];
	int		nlist;
	int		n;
	int		fd;
	int		len;
	int		xjade = 0;
	
        if (cmd = strrchr(argv[0], '/')) cmd++;
        else cmd = argv[0];

	if (strcmp(cmd, CMD_MASTER))
	{
		if (*cmd == 'i') cmd++;
		len = strlen(cmd);
		for (i=0; i < CMD_MAX; i++)
		{
			if (cmdeq(cmd, i, len))
			{
				argv[0] = cmd;
				ret = (*COMMAND[i].main)(argc, argv);
				exit(ret);
			}
		}
	}
	error_info.id = cmd;

	while (n = optget(argv, "hxs:[cs_server]"))
	 switch (n)
	{
	case 'h':
		elist[0] = argv[0];
		im_help(1, elist);
		break;
	case 's':
		s = opt_info.arg;
		if ((fd = csopen(s, CS_OPEN_READ)) < 0)
			printf("cannot connect cs server %s\n", s);
		else
		{
			istate.cs_svc = strdup(s);
			istate.fd = fd;
		}
		break;
	case 'x':
		xjade = 1;
		break;
	case '?':
	case ':':
		printf("im [-s server]\n");
		break;
	}

        if (error_info.errors)
                error(ERROR_USAGE|4, optusage(NULL));

#define PROMPT()	{printf("[%s]> ", (istate.fd > 0 ? istate.cs_svc : "IM")); if (xjade) printf("\n"); fflush(stdout);}

	PROMPT();
	while (s = gets(buf))
	{
		if (*s == '\0')
		{
			PROMPT();
			continue;
		}
		nlist = mkargv(s, elist, 20);
		cmd = elist[0];
		len = strlen(cmd);
		for (i=0; i < CMD_MAX; i++)
		{
			if (cmdeq(cmd, i, len))
			{
				(void) (*COMMAND[i].main)(nlist, elist);
				break;
			}
		}
		if (i == CMD_MAX)
			printf("command not found\n");
		PROMPT();
	}
}
		
int mkargv(s, w, n)
	register char*	s;
	char**		w;
	int		n;
{
	register int	i;
	
	for(i = 0; i < n && *s; )
	{
		while (*s && isspace(*s))	
			s++;
		if (!*s)
			break;
		w[i++] = s;
		while (*s && !isspace(*s))
			s++;
		if(!*s)
			break;
		*s++ = '\0';
	}
	while (i < n)
		w[--n] = s;
	return i;
}
		
int im_help_help(cmd)
	char*	cmd;
{
	printf("\t%s command\n", cmd);
	return (0);
	
}

int im_help(argc, argv)
	int	argc;
	char**	argv;
{
	register int	i;
	register char*	s;
	int		len;

	if (argc == 1)
	{
		for (i=0; i < CMD_MAX; i++)
		{
			if (COMMAND[i].help != NULL)
				(void) (*COMMAND[i].help)(COMMAND[i].name);
			else
				printf("\t%s\n", COMMAND[i].name);
		}
		return (0);
	}

	while((--argc) > 0)
	{
		s = *(++argv);
		len = strlen(s);
		for (i=0; i < CMD_MAX; i++)
		{
			if (cmdeq(s, i, len))
			{
				if (COMMAND[i].help != NULL)
					(void) (*COMMAND[i].help)(COMMAND[i].name);
				else
					printf("\t%s\n", COMMAND[i].name);
				break;
			}
		}
		if (i==CMD_MAX)
			printf("command %s not undefined\n", s);
	}
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

int im_bye(argc, argv)
	int	argc;
	char**	argv;
{
	exit(0);
	return -1;
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

int im_system(argc, argv)
	int		argc;
	register char**	argv;
{
	register char*	s;
	char		buf[2048];
	int		len;

	s = *argv;
	s++;
	if (*s != '\0')
	{
		len = sfsprintf(buf, 2048, "%s ", s);
		s = buf + len;
	}
	else
		s = buf;
	argc--;
	argv++;
	if (argc > 0)
	{
		argv[argc] = NULL;
		arr2str(s, argv, ' ');
	}
	return(system(buf));
}
	
		
int arr2str(s,arr, c)
	register char 	*s;
	char 		*arr[];
	char 		c;
{
        register int 	i;
	register char* 	t;

        for (i=0; arr[i]; i++)
        {
		if (i) *s++ = c;
		for (t = arr[i]; *t; t++, s++)
			*s = *t;
	}
	*s = '\0';
        return(0);
}
