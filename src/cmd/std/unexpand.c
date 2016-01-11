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
 * expand.c and unexpand.c
 * They can be linked together
 * Written by David Korn
 * Sat Oct  8 13:47:13 EDT 1994
 */

static const char expand_usage[] =
"[-?@(#)$Id: expand (AT&T Research) 1999-06-17 $\n]"
USAGE_LICENSE
"[+NAME?expand - convert tabs to spaces]"
"[+DESCRIPTION?\bexpand\b writes the contents of each given file "
	"to standard output with tab characters replaced with one or "
	"more space characters needed to pad to the next tab stop. Each "
	"backspace character copied to standard output causes the column "
	"position to be decremented by 1 unless already in the first column.]"  "[+?If no \afile\a is given, or if the \afile\a is \b-\b, \bexpand\b "
        "copies from standard input.   The start of the file is defined "
        "as the current offset.]"
"[i:initial? Only convert initial tabs (those that precede all non "
	"space or tab characters) on each line to spaces.]"
"[t:tabs]:[tablist:=8?\atablist\a is a comma or space separated list "
	"of positive integers that specifies the tab stops.  If only one "
	"tab stop is specified, then tabs will be set at that many "
	"column positions apart.  Otherwise, the value in \atablist\a "
	"must be in ascending order and the tab stops will be set to "
	"these positions.  In the event of \bexpand\b having to process "
	"tab characters beyond the last specified tab stop, each tab "
	"character is replaced by a single tab.]"
"\n"
"\n[file ...]\n"
"\n"
"[+EXIT STATUS]{"
	"[+0?All files expanded successfully.]"
	"[+>0?One or more files failed to open or could not be read.]"
"}"
"[+SEE ALSO?\bunexpand\b(1), \bpr\b(1)]"
;

static const char unexpand_usage[] =
"[-?@(#)$Id: unexpand (AT&T Research) 1999-06-07 $\n]"
USAGE_LICENSE
"[+NAME?unexpand - convert spaces to tabs]"
"[+DESCRIPTION?\bunexpand\b writes the contents of each given file "
	"to standard output with strings of two or more space and "
	"tab characters at the beginning of each line converted "
	"to as many tabs as possible followed by as many spaces needed "
	"to fill the same number of column positions.  By default, "
	"tabs are set at every 8th column.  Each backspace character copied "
	"to standard output causes the column position to be decremented by 1 "
	"unless already in the first column.]"  
"[+?If no \afile\a is given, or if the \afile\a is \b-\b, \bunexpand\b "
        "copies from standard input.   The start of the file is defined "
        "as the current offset.]"
"[a:all?Convert all strings of two or more spaces or tabs, not just "
	"initial ones.]"
"[t:tabs]:[tablist:=8?\atablist\a is a comma or space separated list "
	"of positive integers that specifies the tab stops.  If only one "
	"tab stop is specified, then tabs will be set at that many "
	"column positions apart.  Otherwise, the value in \atablist\a "
	"must be in ascending order and the tab stops will be set to "
	"these positions.  This option implies the \b-a\b option.]"
"\n"
"\n[file ...]\n"
"\n"
"[+EXIT STATUS?]{"
	"[+0?All files unexpanded successfully.]"
	"[+>0?One or more files failed to open or could not be read.]"
"}"
"[+SEE ALSO?\bexpand\b(1), \bpr\b(1)]"
;


#include	<cmd.h>

#define S_BS	1
#define S_TAB	2
#define S_NL	3
#define S_EOF	4
#define S_SPACE	5

static int *gettabs(const char *arg, int *ntab)
{
	register const char *cp=arg;
	register int c,n=1,old= -1;
	int *tablist;
	while(c= *cp++)
	{
		if(c==' ' || c=='\t' || c==',')
			n++;
	}
	tablist = newof(NiL,int,n,0);
	n=0;
	while(1)
	{
		cp=arg;
		while((c= *cp) && c==' ' || c=='\t' || c==',')
			cp++;
		if(c==0)
			break;
		tablist[n] = strtol(cp,(char**)&arg,10)-1;
		if(cp==arg)
			error(ERROR_exit(1),"%c - invalid character in tablist",*cp);
		if(tablist[n] <= old)
			error(ERROR_exit(1),"tab stops must be increasing");
		old = tablist[n++];
	}
	*ntab=n;
	return(tablist);
}

static int expand(Sfio_t *in, Sfio_t *out, int tablist[], int tabmax, int type,int initial)
{
	static char state[256];
	register int n=0;
	register char *cp, *first, *buff;
	register int savec;
	register char *cpend;
	state[0] = S_EOF;
	state['\b'] = S_BS;
	state['\n'] = S_NL;
	if(type)
		state['\t'] = S_TAB;
	else
		state[' '] = S_SPACE;
	errno=0;
	while(buff = cp = sfreserve(in,SF_UNBOUND,0))
	{
		first = cp-n;
		cpend = cp + sfvalue(in);
		if(state[n= *(unsigned char*)(cpend-1)]==0 || (n==' '&&type==0))
			cpend[-1] = 0; /* put in sentinal */
		savec = n; 
		while(1)
		{
			while((n=state[*(unsigned char*)cp++])==0);
			switch(n)
			{
			    case S_SPACE:
				if(tabmax==0 || cp==first+1)
				{
					register int i,tabspace = tablist[0];
					n = 1;
					cp -= 1;
					if(tabmax==0)
						tabspace -= (cp-first)%tabspace;
					while(cp[n]==' ')
						n++;
					i = cp-buff;
					/* check for end of buffer */
					if(cp[n]==0 && cp+n+1==cpend && savec==' ')
					{
						/* keep grabbing spaces */
						register int c;
						if(i)
							sfwrite(out,buff, i);
						i=0;
						n++;
						while((c=sfgetc(in))==' ')
							n++;
						if(c!=EOF)
							sfungetc(in,c);
						buff = cp+n;
					}
					cp += n;
					if(n >= tabspace || cp+n>cpend)
					{
						if(i)
						{
							sfwrite(out,buff, i);
							i=0;
						}
						buff = cp;
						while(n >= tabspace)
						{
							sfputc(out,'\t');
							n -= tabspace;
							if(++i < tabmax)
								tabspace = tablist[i]-tablist[i-1];
							else if(tabmax<=1)
								tabspace = tablist[0];
							else
								break;
							
						}
						if(n>0)
							sfnputc(out,' ',n);
					}
				}
				if(tabmax)
					state[' '] = 0;
				break;
			    case S_TAB:
				sfwrite(out,buff, (cp-1)-buff);
				n = (cp-1)-first;
				if(tabmax==1)
					n = tablist[0]*((n+tablist[0])/tablist[0])-n;
				else
				{
					register int i=0;
					while(1)
					{
						if(i>=tabmax)
							n=1;
						else if(tablist[i++]<=n)
							continue;
						else
							n = tablist[i-1]-n;
						break;
					}
				}
				if(n<=1)
					sfputc(out,' ');
				else
				{
					sfnputc(out,' ',n);
					first -= (n-1);
				}
				buff = cp;
				if(initial)
					state[' '] = 0;
				break;
			    case S_BS:
				if((first+=2) > cp)
					first = cp;
				break;
			    case S_EOF:
				if(cp==cpend)
				{
					/* end of buffer */
					cp[-1] = savec;
				}
				break;
			    case S_NL:
				first = cp;
				if(type==0)
					state[' '] = S_SPACE;
				else
					state['\t'] = S_TAB;
				break;
			}
			if(cp >= cpend)
			{
				if((n=cp-buff)>0)
					sfwrite(out, buff, n);
				n = cp-first;
				break;
			}
		}
	}
	return(errno);
}

int
main(int argc, char** argv)
{
	register int n,type;
	register char *cp;
	register Sfio_t *fp;
	int ntabs= -1;
	int tabstop=8;
	int initial=0;
	int *tablist=&tabstop;
	const char *usage;

	if (cp = strrchr(argv[0], '/')) cp++;
	else cp = argv[0];
	error_info.id = cp;
	type = *cp == 'e';
	if(type)
		usage = expand_usage;
	else
		usage = unexpand_usage;
	while (n = optget(argv, usage)) switch (n)
	{
	    case 't':
		tablist =  gettabs(opt_info.arg, &ntabs);
		break;
	    case 'i':
		initial = 1;
		break;
	    case 'a':
		if(ntabs < 0)
			ntabs = 0;
		break;
	    case ':':
		cp = argv[opt_info.index]+1;
		if(*cp>='0' && *cp<='9')
			tablist = gettabs(cp, &ntabs);
		else
			error(2, "%s", opt_info.arg);
		break;
	    case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(ntabs<0)
		ntabs=1;
	else if(ntabs==1)
		tablist[0] += 1;
	if(error_info.errors)
		error(ERROR_usage(2), "%s", optusage((char*)0));
	if(cp= *argv)
		argv++;
	do
	{
		if (!cp || streq(cp,"-"))
			fp = sfstdin;
		else if (!(fp = sfopen(NiL, cp, "r")))
		{
			error(ERROR_system(0), "%s: cannot open", cp);
			error_info.errors = 1;
			continue;
		}
		if(expand(fp,sfstdout,tablist,ntabs,type,initial)<0)
			error(ERROR_system(1), "failed");
		if (fp != sfstdin)
			sfclose(fp);
	} while (cp = *argv++);
	return(error_info.errors);
}
