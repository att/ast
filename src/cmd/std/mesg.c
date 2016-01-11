/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
/*
 * mesg.c
 * Written by David Korn
 * Thu Aug 15 15:25:29 EDT 1996
 */

static const char usage[] =
"[-?\n@(#)$Id: mesg (AT&T Research) 1999-04-28 $\n]"
USAGE_LICENSE
"[+NAME?mesg - permit or deny messages to the terminal]"
"[+DESCRIPTION?\bmesg\b controls whether other users are allowed to send"
"	messages via \bwrite\b(1), \btalk\b(1) or other commands to the"
"	terminal device. The terminal device affected is determined by"
"	searching for the first terminal in the sequence of devices"
"	associated with standard input, standard output and standard error,"
"	respectively. With no arguments, \bmesg\b reports the current state"
"	without changing it. Processes with appropriate privileges may be"
"	able to send messages to the terminal independent of the"
"	current state.]"

"\n"
"\n[ y | n ]\n"
"\n"

"[+OPERANDS?The following operands are supported:]{"
"	[+y?Grant permission to other users to send messages to the terminal.]"
"	[+n?Deny permission to other users to send messages to the terminal.]"
"}"
"[+SEE ALSO?\btalk\b(1), \bwrite\b(1)]"
;

#include	<cmd.h>
#include	<ls.h>

static int mesg(int  mode)
{
	struct stat statb;
	char *tty = ttyname(2);
	if(!tty)
		tty = ttyname(0);
	if(!tty)
		tty = ttyname(1);
	if(!tty)
		error(ERROR_exit(1),"cannot find terminal");
	if(stat(tty,&statb)<0)
		error(ERROR_system(1),"%s: cannot stat",tty);
	switch(mode)
	{
	    case 'n':
	    case 'y':
		if(mode=='y')
			statb.st_mode |= S_IWGRP;
		else
			statb.st_mode &= ~S_IWGRP;
		if(chmod(tty, statb.st_mode) < 0)
			error(ERROR_system(1),"%s: cannot stat",tty);
		break;
	    case 0:
		sfprintf(sfstdout,"%c\n",(statb.st_mode&S_IWGRP)?'y':'n');
	}
	return((statb.st_mode&S_IWGRP)==0);
}

int
main(int argc, char *argv[])
{
	register int n;

	NoP(argc);
	error_info.id = "mesg";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	n = 0;
	if(error_info.errors || (*argv && (n= **argv) !='y' && n!='n'))
		error(ERROR_usage(2), "%s", optusage(NiL));
	return mesg(n);
}
