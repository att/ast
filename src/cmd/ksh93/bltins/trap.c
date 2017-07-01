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
 * trap  [-p]  action sig...
 * kill  [-l] [sig...]
 * kill  [-s sig] pid...
 *
 *   David Korn
 *   dgkorn@gmail.com
 *
 */

#include	"defs.h"
#include	"jobs.h"
#include	"builtins.h"

#define L_FLAG	1
#define S_FLAG	2
#define Q_FLAG	JOB_QFLAG
#define QQ_FLAG	JOB_QQFLAG

static int	sig_number(Shell_t*,const char*);

int	b_trap(int argc,char *argv[],Shbltin_t *context)
{
	register char *arg = argv[1];
	register int sig, clear;
	register bool pflag=false, dflag=false, aflag=false, lflag=false;
	register Shell_t *shp = context->shp;
	NOT_USED(argc);
	while (sig = optget(argv, sh_opttrap)) switch (sig)
	{
	    case 'a':
		aflag = true;
		break;
	    case 'p':
		pflag=true;
		break;
	    case 'l':
		lflag = true;
		break;
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
		break;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		return(2);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s", optusage((char*)0));
	if(pflag && aflag)
		errormsg(SH_DICT,ERROR_usage(2),"-a and -p are mutually exclusive");
	if(lflag)
	{
		sh_siglist(shp,sfstdout,-1);;
		return(0);
	}
	if(arg = *argv)
	{
		char *action = arg;
		if(!dflag && !pflag)
		{
			/* first argument all digits or - means clear */
			while(isdigit(*arg))
				arg++;
			clear = (arg!=action && *arg==0);
			if(!clear)
			{
				++argv;
				if(*action=='-' && action[1]==0)
					clear++;
				/*
				 * NOTE: 2007-11-26: workaround for tests/signal.sh
				 * if function semantics can be worked out then it
				 * may merit a -d,--default option
				 */
				else if(*action=='+' && action[1]==0 && shp->st.self == &shp->global)
				{
					clear++;
					dflag = true;
				}
			}
			if(!argv[0])
				errormsg(SH_DICT,ERROR_exit(1),e_condition);
		}
		while(arg = *argv++)
		{
			sig = sig_number(shp,arg);
			if(sig<0)
			{
				errormsg(SH_DICT,2,e_trap,arg);
				return(1);
			}
			/* internal traps */
			if(sig&SH_TRAP)
			{
				char **trap = (shp->st.otrap?shp->st.otrap:shp->st.trap);
				sig &= ~SH_TRAP;
				if(sig>SH_DEBUGTRAP)
				{
					errormsg(SH_DICT,2,e_trap,arg);
					return(1);
				}
				if(pflag)
				{
					if(arg=trap[sig])
						sfputr(sfstdout,arg,'\n');
					continue;
				}
				shp->st.otrap = 0;
				arg = shp->st.trap[sig];
				shp->st.trap[sig] = 0;
				if(!clear && *action)
				{
					char *cp = action;
					if(aflag)
					{
						size_t off=stktell(shp->stk);
						sfprintf(shp->stk,"%s;%s%c",cp,arg,0);
						cp = stkptr(shp->stk,off);
						stkseek(shp->stk,off);
					}
					shp->st.trap[sig] = strdup(cp);
				}
				if(sig == SH_DEBUGTRAP)
				{
					if(shp->st.trap[sig])
						shp->trapnote |= SH_SIGTRAP;
					else
						shp->trapnote = 0;
				}
				if(arg)
					free(arg);
				continue;
			}
			if(sig>=shp->gd->sigmax)
			{
				errormsg(SH_DICT,2,e_trap,arg);
				return(1);
			}
			else if(pflag)
			{
				char **trapcom = (shp->st.otrapcom?shp->st.otrapcom:shp->st.trapcom);
				if(arg=trapcom[sig])
					sfputr(sfstdout,arg,'\n');
			}
			else if(clear)
			{
				sh_sigclear(shp,sig);
				if(dflag) 
					signal(sig,SIG_DFL);
			}
			else
			{
				if(sig >= shp->st.trapmax)
					shp->st.trapmax = sig+1;
				arg = shp->st.trapcom[sig];
				shp->st.trapcom[sig] = Empty;
				sh_sigtrap(shp,sig);
				if(!(shp->sigflag[sig]&SH_SIGOFF))
				{
					char *cp = action;
					if(aflag && arg && arg!=Empty)
					{
						size_t off=stktell(shp->stk);
						sfprintf(shp->stk,"%s;%s%c",cp,arg,0);
						cp = stkptr(shp->stk,off);
						stkseek(shp->stk,off);
						
					}
					shp->st.trapcom[sig] =  strdup(cp);
				}
				if(arg && arg != Empty)
					free(arg);
			}
		}
	}
	else /* print out current traps */
		sh_siglist(shp,sfstdout,-2);
	return(0);
}

int	b_kill(int argc,char *argv[],Shbltin_t *context)
{
	register char *signame;
	register int sig=SIGTERM, flag=0, n;
	register Shell_t *shp = context->shp;
	int usemenu = 0;
	NOT_USED(argc);
	while((n = optget(argv,sh_optkill))) switch(n)
	{
		case ':':
			if((signame=argv[opt_info.index++]) && (sig=sig_number(shp,signame+1))>=0)
				goto endopts;
			opt_info.index--;
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			break;
		case 'n':
			sig = (int)opt_info.num;
			goto endopts;
		case 's':
			flag |= S_FLAG;
			signame = opt_info.arg;
			goto endopts;
		case 'L':
			usemenu = -1;
		case 'l':
			flag |= L_FLAG;
			break;
		case 'q':
			flag |= Q_FLAG;
			shp->sigval = opt_info.num;
			if((int)shp->sigval != shp->sigval) 
				errormsg(SH_DICT,ERROR_exit(1), "%lld - too large for sizeof(integer)", shp->sigval);
			break;
		case 'Q':
			flag |= Q_FLAG|QQ_FLAG;
			shp->sigval = opt_info.num;
			if((int)shp->sigval <0) 
				errormsg(SH_DICT,ERROR_exit(1), "%lld - Q must be unsigned", shp->sigval);
			break;
		case '?':
			shp->sigval = 0;
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
			break;
	}
endopts:
	argv += opt_info.index;
	if(*argv && strcmp(*argv,"--")==0 && strcmp(*(argv-1),"--")!=0)
		argv++;
	if(error_info.errors || flag==(L_FLAG|S_FLAG) || (!(*argv) && !(flag&L_FLAG)))
	{
		shp->sigval = 0;
		errormsg(SH_DICT,ERROR_usage(2),"%s", optusage((char*)0));
	}
	/* just in case we send a kill -9 $$ */
	sfsync(sfstderr);
	if(flag&L_FLAG)
	{
		if(!(*argv))
			sh_siglist(shp,sfstdout,usemenu);
		else while(signame = *argv++)
		{
			if(isdigit(*signame))
				sh_siglist(shp,sfstdout,((int)strtol(signame, (char**)0, 10)&0177)+1);
			else
			{
				if((sig=sig_number(shp,signame))<0)
				{
					shp->exitval = 2;
					shp->sigval = 0;
					errormsg(SH_DICT,ERROR_exit(1),e_nosignal,signame);
				}
				sfprintf(sfstdout,"%d\n",sig);
			}
		}
		return(shp->exitval);
	}
	if(flag&S_FLAG)
	{
		if((sig=sig_number(shp,signame)) < 0 || sig >= shp->gd->sigmax)
		{
			shp->exitval = 2;
			errormsg(SH_DICT,ERROR_exit(1),e_nosignal,signame);
		}
	}
	if(job_walk(shp,sfstdout,job_kill,sig|(flag&(Q_FLAG|QQ_FLAG)),argv))
		shp->exitval = 1;
	shp->sigval = 0;
	return(shp->exitval);
}

/*
 * Given the name or number of a signal return the signal number
 */

static int sig_number(Shell_t *shp,const char *string)
{
	const Shtable_t	*tp;
	register int	n,o,sig=0;
	char		*last, *name;
	if(isdigit(*string))
	{
		n = (int)strtol(string,&last,10);
		if(*last)
			n = -1;
	}
	else
	{
		register int c;
		o = stktell(shp->stk);
		do
		{
			c = *string++;
			if(islower(c))
				c = toupper(c);
			sfputc(shp->stk,c);
		}
		while(c);
		stkseek(shp->stk,o);
		if(memcmp(stkptr(shp->stk,o),"SIG",3)==0)
		{
			sig = 1;
			o += 3;
			if(isdigit(*stkptr(shp->stk,o)))
			{
				n = (int)strtol(stkptr(shp->stk,o),&last,10);
				if(!*last)
					return(n);
			}
		}
		tp = sh_locate(stkptr(shp->stk,o),(const Shtable_t*)shtab_signals,sizeof(*shtab_signals));
		n = tp->sh_number;
		if(sig==1 && (n>=(SH_TRAP-1) && n < (1<<SH_SIGBITS)))
		{
			/* sig prefix cannot match internal traps */
			n = 0;
			tp = (Shtable_t*)((char*)tp + sizeof(*shtab_signals));
			if(strcmp(stkptr(shp->stk,o),tp->sh_name)==0)
				n = tp->sh_number;
		}
		if((n>>SH_SIGBITS)&SH_SIGRUNTIME)
			n = shp->gd->sigruntime[(n&((1<<SH_SIGBITS)-1))-1];
		else
		{
			n &= (1<<SH_SIGBITS)-1;
			if(n < SH_TRAP)
				n--;
		}
		if(n<0 && shp->gd->sigruntime[1] && (name=stkptr(shp->stk,o)) && *name++=='R' && *name++=='T')
		{
			if(name[0]=='M' && name[1]=='I' && name[2]=='N' && name[3]=='+')
			{
				if((sig=(int)strtol(name+4,&name,10)) >= 0 && !*name)
					n = shp->gd->sigruntime[SH_SIGRTMIN] + sig;
			}
			else if(name[0]=='M' && name[1]=='A' && name[2]=='X' && name[3]=='-')
			{
				if((sig=(int)strtol(name+4,&name,10)) >= 0 && !*name)
					n = shp->gd->sigruntime[SH_SIGRTMAX] - sig;
			}
			else if((sig=(int)strtol(name,&name,10)) > 0 && !*name)
				n = shp->gd->sigruntime[SH_SIGRTMIN] + sig - 1;
			if(n<shp->gd->sigruntime[SH_SIGRTMIN] || n>shp->gd->sigruntime[SH_SIGRTMAX])
				n = -1;
		}
	}
	return(n);
}

