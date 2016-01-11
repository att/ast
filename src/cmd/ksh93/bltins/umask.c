/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * umask [-pS] [mask]
 *
 *   David Korn
 *   dgkorn@gmail.com
 *
 */

#include	<ast.h>	
#include	<sfio.h>	
#include	<error.h>	
#include	<ctype.h>	
#include	<ls.h>	
#include	<shell.h>	
#include	"builtins.h"
#ifndef SH_DICT
#   define SH_DICT	"libshell"
#endif

int	b_umask(int argc,char *argv[],Shbltin_t *context)
{
	register char *mask;
	register int flag = 0;
	register bool sflag=false, pflag=false;
	NOT_USED(context);
	while((argc = optget(argv,sh_optumask))) switch(argc)
	{
		case 'p':
			pflag = true;
			break;
		case 'S':
			sflag = true;
			break;
		case ':':
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(2), "%s",opt_info.arg);
			break;
	}
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	argv += opt_info.index;
	if(mask = *argv)
	{
		register int c;	
		if(isdigit(*mask))
		{
			while(c = *mask++)
			{
				if (c>='0' && c<='7')	
					flag = (flag<<3) + (c-'0');	
				else
					errormsg(SH_DICT,ERROR_exit(1),e_number,*argv);
			}
		}
		else
		{
			char *cp = mask;
			flag = umask(0);
			c = strperm(cp,&cp,~flag&0777);
			if(*cp)
			{
				umask(flag);
				errormsg(SH_DICT,ERROR_exit(1),e_format,mask);
			}
			flag = (~c&0777);
		}
		umask(flag);	
	}	
	else
	{
		char *prefix = pflag?"umask ":"";
		umask(flag=umask(0));
		if(sflag)
			sfprintf(sfstdout,"%s%s\n",prefix,fmtperm(~flag&0777));
		else
			sfprintf(sfstdout,"%s%0#4o\n",prefix,flag);
	}
	return(0);
}

