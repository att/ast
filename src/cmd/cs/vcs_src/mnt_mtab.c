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

struct entry_t
{
	char*	mpoint;
	char*	vcscmd;
};

typedef struct entry_t	entry_t;

int im_restart_help(s)
	char* s;
{
	printf("\t%s # reset mount points\n", s);
	return (0);
}

int add_entry(mpoint, cmd)
	char*	mpoint;
	char*	cmd;
{
	register entry_t*	entry;

	if (mpoint == NULL || cmd == NULL)
			return (-1);	
	entry = (entry_t *)malloc(sizeof(entry_t));
	entry->mpoint = strdup(mpoint);
	entry->vcscmd = strdup(cmd);
	if (istate.mtab == NULL)
		istate.mtab = hashalloc((HASHTABLE *)0, 0);
	(void)hashput(istate.mtab, entry->mpoint, (char *)entry);
	return (0);
}

#define hashrm(tbl, name)	hashlook(tbl, name, HASH_DELETE, (char *) 0)
int rm_entry(mpoint)
	register char*	mpoint;
{
	register entry_t* entry;
	
	if (istate.mtab == NULL)
		return (1);
	if ((entry = (entry_t *)hashrm(istate.mtab, mpoint)) != NULL)
	{
		free(entry->mpoint);
		free(entry->vcscmd);
		free((char *)entry);
		return (0);
	}
	return (1);
}

int im_restart(argc, argv)
	int	argc;
	char**  argv;
{
	register char*	s;
	register int	n;
	int		fd;
	int 		do_call();

	error_info.id = argv[0];
	opt_info.index = 1;
	while (n = optget(argv, "s:[server]"))
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
		printf("reboot");
		return (1);
	}
	(void) hashwalk(istate.mtab, 0, do_call, 0);
	return(0);
}

int do_call(mpoint, value, dump)
	char*	mpoint;
	caddr_t value;
	caddr_t dump;
{
	register char*	vcscmd;
	char		reply[1024 *2];

	if (istate.mtab == NULL)
		return (1);

	memset(reply, 0, sizeof(reply));

	if (value != NULL && (vcscmd = ((entry_t *)value)->vcscmd) != NULL)
	{
		if (vcs_write(vcscmd) > 0 &&
		    vcs_read(reply, 1024 * 2) > 0)
		{
			printmtmsg(reply);
			return (0);
		}
		printf("%s cannot mount", mpoint);
		return (-1);
	}
	return (-1);
}

