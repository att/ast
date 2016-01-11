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

#include	"defs.h"
#include	"builtins.h"
#include	"name.h"
#include	"edit.h"
#include	"variables.h"

#define FILTER_AMP	0x4000

Dtdisc_t        _Compdisc =
{
        offsetof(struct Complete,name), -1 , 0, 0, 0, nv_compare
};

static const char Options[] = "bdDfn";
static const char *Option_names[] =  {
	"bbashdefault",
	"ddefault",
	"Idirnames",
	"ffilenames",
	"nnospace",
	"pplusdirs",
	0
};

static const char Actions[] = "aABkbcfdDEeFgHhjrsoOZSuv";

static const char *Action_names[] =  {
	"aalias",
	"Aarrayvar",
	"Bbinding",
	"kkeyword",
	"bbuiltin",
	"ccommand",
	"ffile",
	"ddirectory",
	"Idisabled",
	"Jenabled",
	"eexport",
	"Ffunction",
	"ggroup",
	"Hhelptopic",
	"hhostname",
	"jjob",
	"rrunning",
	"sservice",
	"osetopt",
	"Oshopt",
	"Zsignal",
	"Sstopped",
	"uuser",
	"vvariable",
	0
};

static const char *Action_eval[] =  {
	"yalias | sed -e 's/=.*//'",
	"xtypeset +a",
	"",
	"k",
	"x'builtin'",
	"x(IFS=:;for i in $PATH;do cd \"$i\";for f in *;do [[ -x $f && ! -d $f ]] && print -r -- \"$f\";done;cd ~-;done)",
	"xfor _ in *; do [[ ! -d \"$_\" ]] && print -r -- \"$_\";done",
	"xfor _ in *; do [[ -d \"$_\" ]] && print -r -- \"$_\";done",
	"x'builtin' -n",
	"x'builtin'",
	"xtypeset +x",
	"xtypeset +f | sed -e 's/()//'",
	"xsed -e 's/:.*//' /etc/group", 
	"",
	"ygrep -v '^#' ${HOSTFILE:-/etc/hosts} | tr '\t ' '\n\n' | tr -s '\n' | cat",
	"xjobs -l",
	"xjobs -l | grep Running",
	"xgrep -v '^#' /etc/services | sed -e 's/[ \t].*//'",
	"xset -o | { read;cat;} | sed -e 's/ .*//'",
	"pprint interactive\nrestricted\nlogin_shell\n",
	"xkill -l",
	"xjobs -l | grep Stopped",
	"xsed -e 's/:.*//' /etc/passwd", 
	"xtypeset + | grep -v '^[ {}]' | grep -v namespace",
	0
};

static const char compgen_desc[] =
	"Complete possible matches for \aword\a according to the \aoptions\a, "
	"which may be an option accepted by \bcomplete\b with the exception of "
	"\b-p\b and \b-r\b, and write the matches to standard output.  When "
	"using the \b-F\b or \b-C\b options, the variables shell variables "
	"set by the programmable completion facilities, while available, will "
	"not have useful values.]"
	;
static const char complete_desc[] =
	"Specify how arguments to each \aname\a should be completed.  If "
	"the \b-p\b option is supplied, or if no options are supplied, "
	"existing completion specifications are printed to standard output in "
	"a way that allows them to be reused as input.  The \b-r\b option "
	"removes a completion specification for each \aname\a, or if no "
	"\aname\as are supplied, all completion specifications.]"
	;

static const char complete_opts[] =
"[p?Existing options for \aword\a are written to standard output in a format "
	"that can be used for reinput.]"
"[r?Removes the completion specification for each \aword\a.]"
;


static const char sh_optcomplete[] =
"+[-1c?\n@(#)$Id: \f (AT&T Research) 2014-04-21 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - command completion.]"
"[+DESCRIPTION?\fdescription\f]"
"[A]:[action?\aaction\a may be one of the following list of completions:]{"
	"[+alias?Equivalent to \b-a\b.]"
	"[+arrayvar?Array variable names.]"
	"[+binding?\breadline\b key binding names.]"
	"[+builtin?Equivalent to \b-b\b.]"
	"[+command?Equivalent to \b-c\b.]"
	"[+directory?Equivalent to \b-d\b.]"
	"[+disabled?Names of disabled shell builtins.]"
	"[+enabled?Names of enabled shell builtins.]"
	"[+export?Equivalent to \b-e\b.]"
	"[+file??Equivalent to \b-f\b.]"
	"[+function?Names of shell functions.]"
	"[+group?Equivalent to \b-g\b.]"
	"[+helptopic?Help topics as accepted by the \bhelp\b builtin.]"
	"[+hostname?Hostnames, as taken from the file specified by the "
		"\bHOSTFILE\b shell variable.]"
	"[+job?Equivalent to \b-j\b.]"
	"[+keyword?Equivalent to \b-k\b.]"
	"[+running?Names of running jobs if job control is active.]"
	"[+service?Equivalent to \b-s\b.]"
	"[+setopt?Valid option names as acceptered by the \bshopt\b builtin.]"
	"[+signal?Signal names.]"
	"[+stopped?Names of stopped jobs if job control is active.]"
	"[+user?Equivalent to \b-u\b.]"
	"[+variable?Equivalent to \b-v\b.]"
"}"
"[o]:[option?\aoption\a may be one of the following list of completions:]{"
	"[+bashdefault?perform \bbash\b default completions if there are no "
		"matches]"
	"[+default?Use default filename completion of there are no matches.]"
	"[+dirnames?Perform directory name completion if there are no matches.]"
	"[+filenames?]"
	"[+nospace?No space will be appended to words completed at the end "
		"of the line.]"
"}"
"[G]:[globpat?The filename expansion pattern \aglobpat\a is expanded to "
	"generate the possible completions.]"
"[W]:[wordlist?The awordlist\a is split using the characters in \bIFS\b "
	"as delimiters, and the resultant word is expanded.  The possible "
	"completions are the members of the resultant list which match the "
	"word being completed.]"
"[C]:[command?\acommand\a is executed in a subshell environment, and its "
	"output is used as the possible completions.]"
"[D?Use this completion as a default for commands that don't have a completion "
	"specified.]"
"[E?Use this completion when for a blank line.]"
	"specified.]"
"[F]:[function?The shell function \afunction\a is executed in the current "
	"environment.  When it finishes the possible list of completions are "
	"retrieved from the indexed array variable \bCOMPREPLY\b]"
"[X]:[pattern?\apattern\a is used to filter filename expansions.  It is "
	"applied to the list of possible completions generated by the "
	"preceding options and arguments, and each completion matching "
	"\apattern\a is removed from the list.  A leading \b!\b in \apattern\a "
	"negates the pattern.]"
"[P]:[prefix?\aprefix\a is prepended to each possible completion after all "
	"other options have been applied.]"
"[S]:[suffix?\asuffix\a is appended to each possible completion after all "
	"other options have been applied.]"
"[a?Generate the list of alias names.]"
"[b?Generate the list of builtins.]"
"[c?Generate the list of command names.]"
"[d?Generate the list of directory names.]"
"[e?Generate the list of exported variable names.]"
"[f?Generate the list of file names.]"
"[g?Generate the list of group names.]"
"[j?Generate the list of job names if job control is active.]"
"[k?Generate the list of shell reserved words.]"
"[s?Generate the list of service names.]"
"[u?Generate the list of user names.]"
"[v?Generate the list of variable names.]"
"\fxopts\f"
"\n"
"\n[word ...]\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?No errors occurred.]"
        "[+>0?An error occurred.]"
"}"

"[+SEE ALSO?\breadonly\b(1), \bexport\b(1)]"
;

static char action(const char *list[],const char *str)
{
	const char *cp;
	int n=0;
	for(cp=list[0]; cp; cp=list[++n]) 
	{
		if(strcmp(cp+1,str)==0)
			return(*cp);
	}
	return(0);
}

static int compgen_info(Opt_t* op, Sfio_t *out, const char *str, Optdisc_t *od)
{
	if(strcmp(str,"description")==0)
		sfputr(out,complete_desc,-1);
	return(0);
}

static int complete_info(Opt_t* op, Sfio_t *out, const char *str, Optdisc_t *od)
{
	if(strcmp(str,"description")==0)
		sfputr(out,compgen_desc,-1);
	else
		sfputr(out,complete_opts,-1);
	return(0);
}

static bool keywords(Sfio_t *out)
{
	register const Shtable_t *tp;
	for(tp=shtab_reserved; *tp->sh_name; tp++)
	{
		if(sfputr(out,tp->sh_name,'\n')<0)
			return(false);
	}
	return(true);
}

static bool evaluate(struct Complete *comp, Sfio_t *out, const char *str)
{
	Sfio_t *in = sfopen((Sfio_t*)0,str,"s");
	int n = sh_eval(comp->sh,in,0);
	return(n==0);
}

/* write wordlist to stack splitting on IFS, one word per line */
static gen_wordlist(Sfio_t *iop, const char *word)
{
	const char *ifs = nv_getval(IFSNOD);
	register char c, n=0;
	while((c = *word) && strchr(ifs,c))
		word++;
	while(c = *word++)
	{
		if(strchr(ifs,c))
		{
			if(n++)
				continue;
			c = '\n';
		}
		else
			n = 0;
		sfputc(iop,c);
	}
	if(n==0)
		sfputc(iop,'\n');
	
}

char **ed_pcomplete(struct Complete *comp, const char *line, const char *prefix, int index)
{
	Sfio_t		*tmp=sftmp(0), *saveout;
	int		i=0,c,complete=(index!=0);
	bool		negate=false, wordlist;
	size_t		len,tlen=0,plen=0,slen=0, wlen;
	char		**av, *cp, *str, *lastword;
	const char	*filter;
	Shell_t	*shp = comp->sh;
	while(Actions[c=i++])
	{
		if((1L<<c) > comp->action)
			break;
		if(comp->action&(1L<<c))
		{
			str = (char*)Action_eval[c];
			switch(*str++)
			{
			    case 'k':
				keywords(tmp);
				break;
			    case 'x':
				sfsync(sfstdout);
				saveout = sfswap(sfstdout,(Sfio_t*)0);
				sfswap(tmp,sfstdout);
				sh_trap(shp,str,0);
				sfsync(sfstdout);
				tmp = sfswap(sfstdout,(Sfio_t*)0);
				sfswap(saveout,sfstdout);
				break;
			    case 'y':
				stkseek(shp->stk,0);
				sfprintf(shp->stk,"{ %s ;} >&%d\n",str,sffileno(tmp));
				sfputc(shp->stk,0);
				sh_trap(shp,stkptr(shp->stk,0),0);
				sfseek(tmp,(Sfoff_t)0,SEEK_END);
				sfsync(tmp);
				stkseek(shp->stk,0);
				break;
			    case 'p':
				sfputr(tmp,str,'\n');
				break;
			    default:
				break;
			}
		}
	}
	if(comp->wordlist || comp->globpat)
	{
		if(comp->globpat)
			gen_wordlist(tmp,comp->globpat);
		if(comp->wordlist)
			gen_wordlist(tmp,comp->wordlist);
	}
	if(comp->fname && !comp->fun && !(comp->fun = nv_search(comp->fname,sh_subfuntree(shp,0),0)))
		errormsg(SH_DICT,ERROR_exit(1),"%s: function not found",comp->fname);
	if(comp->command || comp->fun)
	{
		char *cpsave;
		int csave;
		if(strcmp(comp->name," E")==0)
			complete = 1;
		if(complete)
		{
			_nv_unset(COMPREPLY,0);
			COMP_POINT->nvalue.s = index+1;
			COMP_LINE->nvalue.cp = line;
			cp = (char*)&line[index]-strlen(prefix);
			csave = *(cpsave=cp);
			while(--cp>=line)
			{
				if(isspace(*cp))
					break;
			}
			lastword = ++cp;
		}
		if(comp->fun)
		{
			Namarr_t	*ap;
			Namval_t	*np = COMPREPLY;
			int		n,spaces=0;
			if(!complete)
				errormsg(SH_DICT,ERROR_warn(0),"-F option may not work as you expect");
			_nv_unset(COMP_WORDS,NV_RDONLY);
			cp = (char*)line;
			if(strchr(" \t",*cp))
				cp++;
			n = 1;
			while(c = *cp++)
			{
				if(strchr(" \t",c))
				{
					if(spaces++==0)
						n++;
				}
				else if(spaces)
					spaces = 0;
			}
			COMP_CWORD->nvalue.s = n-1;
			stkseek(shp->stk,0);
			len = (n+1)*sizeof(char*) + strlen(line)+1;
			stkseek(shp->stk,len);
			av = (char**)stkptr(shp->stk,0);
			cp = (char*)&av[n+1];
			strcpy(cp,line);
			spaces = 0;
			while(*cp)
			{
				while(*cp && strchr(" \t",*cp))
				{
					*cp++ = 0;
					spaces++;
				}
				if(*cp==0)
				{
					if(spaces)
					{
						*--cp = ' ';
						*av++ = cp;
					}
					break;
				}
				spaces = 0;
				*av++ = cp;
				while(*cp && !strchr(" \t",*cp))
					cp++;
			}
			*av=0;
			av = (char**)stkptr(shp->stk,0);
			nv_setvec(COMP_WORDS,0,n,av);
			stkseek(shp->stk,0);
			*cpsave = 0;
			sfprintf(shp->stk,"%s \"%s\" \"%s\" \"%s\"\n\0",nv_name(comp->fun),comp->name,prefix,lastword); 
			*cpsave = csave;
			sfputc(shp->stk,0);
			str = stkptr(shp->stk,0);
			sh_trap(shp,str,0);
			stkseek(shp->stk,0);
			if((ap = nv_arrayptr(np)) && ap->nelem>0)
			{
				nv_putsub(np,(char*)0,0,ARRAY_SCAN);
				do
				{
					cp =  nv_getval(np);
					sfputr(tmp,cp,'\n');
				}
				while(nv_nextsub(np));
			}
			else if(cp = nv_getval(np))
				sfputr(tmp,cp,'\n');
		}
		if(comp->command)
		{
			if(!complete)
				errormsg(SH_DICT,ERROR_warn(0),"-C option may not work as you expect");
			stkseek(shp->stk,0);
			sfsync(tmp);
			*cpsave = 0;
			sfprintf(shp->stk,"(\"%s\" \"%s\" \"%s\" \"%s\") >&%d\n",comp->command,comp->name,prefix,lastword,sffileno(tmp));
			sfputc(shp->stk,0);
			*cpsave = csave;
			str = stkptr(shp->stk,0);
			sh_trap(shp,str,0);
			sfseek(tmp,(Sfoff_t)0,SEEK_END);
			stkseek(shp->stk,0);
		}
	}
	if(comp->prefix)
		plen = strlen(comp->prefix);
	if(comp->suffix)
		slen = strlen(comp->prefix);
	filter = comp->filter;
	if(comp->options&FILTER_AMP)
	{
		while(c = *filter++)
		{
			if(c=='\\' && *filter=='&')
				c = *filter++;
			else if(c=='&')
			{
				sfputr(shp->stk,prefix,-1);
				continue;
			}
			sfputc(shp->stk,c);
		}
		filter = stkfreeze(shp->stk,1);
	}
	if(filter && *filter=='!')
	{
		filter++;
		negate = true;
	}
	sfset(tmp,SF_WRITE,0);
	if(prefix)
	{
		if(*prefix=='\'' && prefix[1]=='\'')
			prefix+=2;
		else if(*prefix=='"' && prefix[1]=='"')
			prefix+=2;
		len = strlen(prefix);
	}
again:
	c = 0;
	sfseek(tmp,(Sfoff_t)0,SEEK_SET);
	while(str = sfgetr(tmp,'\n',0))
	{
		wlen = sfvalue(tmp)-1;
		if(prefix && memcmp(prefix,str,len))
			continue;
		if(filter)
		{
			str[wlen] = 0;
			i = strmatch(str,filter);
			str[wlen] = '\n';
			if( i ^ negate)
				continue;
		}
		c++;
		if(complete==1)
			tlen += wlen;
		else if(complete==2)
		{
			*av++ = cp;
			if(comp->prefix)
				memcpy(cp,comp->prefix,plen);
			memcpy(cp+=plen,str,wlen);
			cp += wlen;
			if(comp->suffix)
				memcpy(cp,comp->suffix,slen);
			cp += slen;
			*cp++ = 0;
		}
		else
		{
			if(comp->prefix)
				sfwrite(sfstdout,comp->prefix,plen);
			sfwrite(sfstdout,str,wlen);
			if(comp->suffix)
				sfwrite(sfstdout,comp->suffix,slen);
			sfputc(sfstdout,'\n');
		}
	}
	if(complete==2)
	{
		*av = 0;
		sfclose(tmp);
		return((char**)stkptr(shp->stk,0));
	}
	if(complete)
	{
		/* reserved space on stack and try again */
		len = 3;
		tlen = (c+1)*sizeof(char*)+len*c +1024; 
		stkseek(shp->stk,tlen);
		complete = 2;
		av = (char**)stkptr(shp->stk,0);
		cp = (char*)av + (c+1)*sizeof(char*);
		goto again;
	}
	sfclose(tmp);
	return(0);
}

static bool delete_and_add(const char *name, struct Complete *comp)
{
	struct Complete *old=0;
	Dt_t *compdict =  ((Edit_t*)(shgd->ed_context))->compdict;
	if(compdict && (old=(struct Complete*)dtmatch(compdict,name)))
	{
		dtdelete(compdict,old);
		free((void*)old);
	}
	else if(comp && !compdict)
		((Edit_t*)(shgd->ed_context))->compdict=compdict=dtopen(&_Compdisc, Dtoset);
	if(!comp && old)
		return(false);
	if(comp)
	{
		int	size = comp->name?strlen(comp->name)+1:0;
		int	n=size,p=0,s=0,f=0,w=0,g=0,c=0,fn=0;
		char	*cp;
		if(comp->prefix)
			size += (p=strlen(comp->prefix)+1);
		if(comp->suffix)
			size += (s=strlen(comp->suffix)+1);
		if(comp->filter)
			size += (f=strlen(comp->filter)+1);
		if(comp->wordlist)
			size += (w=strlen(comp->wordlist)+1);
		if(comp->globpat)
			size += (g=strlen(comp->globpat)+1);
		if(comp->command)
			size += (c=strlen(comp->command)+1);
		if(comp->fname)
			size += (fn=strlen(comp->fname)+1);
		old = malloc(sizeof(struct Complete) + size);
		*old = *comp;
		cp = (char*)(old+1);
		old->name = cp;
		memcpy(old->name,comp->name,n);
		cp += n;
		if(p)
			memcpy(old->prefix=cp,comp->prefix,p);
		cp += p;
		if(s)
			memcpy(old->suffix=cp,comp->suffix,s);
		cp += s;
		if(f)
			memcpy(old->filter=cp,comp->filter,f);
		cp += f;
		if(w)
			memcpy(old->wordlist=cp,comp->wordlist,w);
		cp += w;
		if(g)
			memcpy(old->globpat=cp,comp->globpat,g);
		cp += g;
		if(c)
			memcpy(old->command=cp,comp->command,c);
		if(fn)
			memcpy(old->fname=cp,comp->fname,fn);
		dtinsert(compdict,old);
	}
	return(true);
}

static const char *lquote(struct Complete *cp, const char *str)
{
	register int c;
	char	*sp;
	Sfio_t	*stakp;
	if(!(sp=strchr(str,'\'')))
		return(str);
	stakp = cp->sh->stk;
	stkseek(stakp,0);
	sfputc(stakp,'$');
	if(sp-str)
		sfwrite(stakp,str,sp-str);
	while(c = *sp++) 
	{
		if(c=='\'')
			sfputc(stakp,'\\');
		sfputc(stakp,c);
	}
	sfputc(stakp,0);
	return(stkptr(stakp,0));
}

static void print_out(struct Complete *cp,Sfio_t *out)
{
	int c,i=0,a;
	char *sp;
	sfputr(out,"complete",' ');
	while(Options[c=i++])
	{
		if(cp->options&(1<<c))
			sfprintf(out,"-o %s ",Option_names[c]+1);
	}
	while(Actions[c=i++])
	{
		if(a=cp->action&(1L<<c))
		{
			if(sp=strchr("abcdefgjksuv",Actions[c]))
			{
				sfputc(out,'-');
				sfputc(out,*sp);
				sfputc(out,' ');
			}
			else
				sfprintf(out,"-A %s ",Action_names[c]+1);
		}
		if((1L<<c) >= cp->action)
			break;
	}
	if(cp->globpat)
		sfprintf(out,"-G '%s' ", lquote(cp,cp->globpat));
	if(cp->wordlist)
		sfprintf(out,"-W %s ", cp->wordlist);
	if(cp->prefix)
		sfprintf(out,"-P '%s' ", lquote(cp,cp->prefix));
	if(cp->suffix)
		sfprintf(out,"-S '%s' ", lquote(cp,cp->suffix));
	if(cp->filter)
		sfprintf(out,"-X '%s' ", lquote(cp,cp->filter));
	if(cp->fname)
		sfprintf(out,"-F %s ", cp->fname);
	if(cp->command)
		sfprintf(out,"-C %s ", cp->command);
	if(*cp->name ==' ')
		sfprintf(out,"-%c\n", cp->name[1]);
	else
		sfputr(out,cp->name,'\n');
}

int b_complete(int argc, char *argv[], Shbltin_t *context)
{
	int n,r=0;
	bool complete=true,delete=false,print=(argc==1);
	bool empty=false;
	char *av[2];
	Optdisc_t disc;
	struct Complete comp;
	disc.version = OPT_VERSION;
	if(strcmp(argv[0],"compgen")==0)
	{
		complete  = false;
		optinit(&disc,compgen_info);
	}
	else
		optinit(&disc,complete_info);
	memset(&comp,0,sizeof(comp));
	comp.sh = context->shp;

	while((n = optget(argv,sh_optcomplete)))
	{
		switch(n)
		{
		    case 'A':
			if((n=action(Action_names,opt_info.arg))==0)
				errormsg(SH_DICT,ERROR_exit(1),"invalid -%c option name %s", 'A',opt_info.arg);
		    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		    case 'g': case 'j': case 'k': case 's': case 'u': case 'v':
		    case 'I': case 'J': case 'Z':
			n =  (strchr(Actions,n)-Actions);
			comp.action |= 1L<<n;
			if(Actions[n] == 'c')
			{
				/* c contains keywords, builtins and functs */
				const char *cp;
				for(cp="kbF";n= *cp; cp++)
				{
					n = (strchr(Actions,n)-Actions);
					comp.action |= 1L<<n;
				}
			}
			break;
		    case 'o':
			if((n=action(Option_names,opt_info.arg))==0)
				errormsg(SH_DICT,ERROR_exit(1),"invalid -%c option name %s", 'o',opt_info.arg);
			n =  (strchr(Options,n)-Options);
			comp.options |= 1<<n;
			break;
		    case 'G':
			comp.globpat = opt_info.arg;
			break;
		    case 'W':
			comp.wordlist = opt_info.arg;
			break;
		    case 'C':
			comp.command = opt_info.arg;
			break;
		    case 'F':
			comp.fname = opt_info.arg;
			break;
		    case 'S':
			comp.suffix = opt_info.arg;
			break;
		    case 'P':
			comp.prefix = opt_info.arg;
			break;
		    case 'X':
			comp.filter = opt_info.arg;
			if(strchr(comp.filter,'&'))
				comp.options |= FILTER_AMP;
			break;
		    case 'r':
			delete=true;
			break;
		    case 'p':
			print=true;
			break;
		    case 'D': case 'E':
			av[1] = 0;
			av[0] = n=='D'?" D":" E";
			empty = true;
			break;
		    case '?':
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
			return(-1);
		}
	}
	argv += opt_info.index;
	if(complete)
	{
		char *name;
		struct Complete *cp;
		Dt_t *compdict =  ((Edit_t*)(shgd->ed_context))->compdict;
		if(!empty && !argv[0])
		{
			if(!print && !delete)
				errormsg(SH_DICT,ERROR_usage(0), "complete requires command name");
			if(compdict)
			{
				struct Complete *cpnext;
				for(cp=(struct Complete*)dtfirst(compdict);cp;cp=cpnext)
				{
					cpnext = (struct Complete*)dtnext(compdict,cp);
					if(print)
						print_out(cp,sfstdout);
					else
						delete_and_add(cp->name,0);
				}
			}
		}
		if(empty)
			argv = av;
		while(name = *argv++)
		{
			if(print)
			{
				if(!compdict || !(cp = (struct Complete*)dtmatch(compdict,name)))
					r = 1;
				else
					print_out(cp,sfstdout);
			}
			else
			{
				comp.name = name;
				if(!delete_and_add(argv[0],delete?0:&comp))
					r=1;
			}
		}
	}
	else
	{
		comp.name = "";
		ed_pcomplete(&comp,"",argv[0],0);
	}
	return(r);
}

