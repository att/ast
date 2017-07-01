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
 * UNIX shell
 *
 * S. R. Bourne
 * Rewritten by David Korn
 * AT&T Labs
 *
 */

#include	"defs.h"
#include	"path.h"
#include	"builtins.h"
#include	"terminal.h"
#include	"edit.h"
#include	"jobs.h"
#include	"FEATURE/poll"
#if SHOPT_KIA
#   include	"shlex.h"
#   include	"io.h"
#endif /* SHOPT_KIA */
#if SHOPT_PFSH
#   define PFSHOPT	"P"
#else
#   define PFSHOPT
#endif
#if SHOPT_BASH
#   define BASHOPT	"\374"
#else
#   define BASHOPT
#endif
#define HFLAG        "H"

#define SORT		1
#define PRINT		2

static	char		*null;
static  pid_t		*procsub;

/* The following order is determined by sh_optset */
static  const char optksh[] =  PFSHOPT BASHOPT "DircabefhkmnpstuvxBCGEl" HFLAG;
static const int flagval[]  =
{
#if SHOPT_PFSH
	SH_PFSH,
#endif
#if SHOPT_BASH
	SH_POSIX,
#endif
	SH_DICTIONARY, SH_INTERACTIVE, SH_RESTRICTED, SH_CFLAG,
	SH_ALLEXPORT, SH_NOTIFY, SH_ERREXIT, SH_NOGLOB, SH_TRACKALL,
	SH_KEYWORD, SH_MONITOR, SH_NOEXEC, SH_PRIVILEGED, SH_SFLAG, SH_TFLAG,
	SH_NOUNSET, SH_VERBOSE,  SH_XTRACE, SH_BRACEEXPAND, SH_NOCLOBBER,
	SH_GLOBSTARS, SH_RC, SH_LOGIN_SHELL, SH_HISTEXPAND,
	0 
};

#define NUM_OPTS	(sizeof(flagval)/sizeof(*flagval))

typedef struct _arg_
{
	Shell_t		*sh;
	struct dolnod	*argfor; /* linked list of blocks to be cleaned up */
	struct dolnod	*dolh;
	char flagadr[NUM_OPTS+1];
#if SHOPT_KIA
	char	*kiafile;
#endif /* SHOPT_KIA */
} Arg_t;

static int 		arg_expand(Shell_t*,struct argnod*,struct argnod**,int);
static void 		sh_argset(Arg_t*, char *[]);

#define SORT_numeric	01
#define SORT_reverse	02
struct Node
{
	union	Value 	vp;
	int		index;
	unsigned char	bits;
	Namval_t	*nodes[1];
};

struct Sort
{
	Shell_t		*shp;
	Namval_t	*np;
	union Value	*vp;
	Dt_t		*root;
	int		cur;
	int		nelem;
	char		*flags;
	char		*name;
	struct Node	*nodes;
	struct Node	**nptrs;
	char		*keys[1];
};

static struct Sort *Sp;

static int arraysort(const char *s1, const char *s2)
{
	struct Sort *sp = Sp;
	Shell_t	*shp = sp->shp;
	int r=0, cur=sp->cur;
	Namval_t *np1, *np2;
	Namfun_t	fun;
	char		*cp;
	memset(&fun,0,sizeof(Namfun_t));
	if(!(np1 = ((struct Node*)s1)->nodes[sp->cur]))
	{
		sfprintf(shp->strbuf,"%s[%i].%s%c",sp->name,((struct Node*)s1)->index,sp->keys[sp->cur],0);
		cp = sfstruse(shp->strbuf);
		np1 = nv_create(cp,sp->root,NV_VARNAME|NV_NOFAIL|NV_NOADD|NV_NOSCOPE,&fun);
		((struct Node*)s1)->nodes[sp->cur] = np1;
	}
	if(!(np2 = ((struct Node*)s2)->nodes[sp->cur]))
	{
		sfprintf( shp->strbuf,"%s[%i].%s%c",sp->name,((struct Node*)s2)->index,sp->keys[sp->cur],0);
		cp = sfstruse(shp->strbuf);
		np2 = nv_create(cp,sp->root,NV_VARNAME|NV_NOFAIL|NV_NOADD|NV_NOSCOPE,&fun);
		((struct Node*)s2)->nodes[sp->cur] = np2;
	}
	if(sp->flags[sp->cur]&SORT_numeric)
	{
		Sfdouble_t d1 = np1?nv_getnum(np1):0;
		Sfdouble_t d2 = np2?nv_getnum(np2):0;
		if(d2<d1)
			r = 1;
		else if(d2>d1)
			r = -1;
	}
	else if(np1 && np2)
	{
		char *sp1 = nv_getval(np1);
		char *sp2 = nv_getval(np2);
		r = strcoll(sp1,sp2);
		
	}
	else if(np1)
		r = 1;
	else if(np2)
		r = -1;
	if(sp->flags[sp->cur]&SORT_reverse)
		r = -r;
	if(r==0 && sp->keys[++sp->cur])
		r = arraysort(s1,s2);
	sp->cur = cur;
	return(r);
}

static int alphasort(const char *s1, const char *s2)
{
	struct Sort	*sp = Sp;
	int		r = 0;
	char		*sp1, *sp2;
	nv_putsub(sp->np, NULL,((struct Node*)s1)->index,0);
	sp1 = nv_getval(sp->np);
	nv_putsub(sp->np, NULL,((struct Node*)s2)->index,0);
	sp2 = nv_getval(sp->np);
	r = strcoll(sp1,sp2);
	if(sp->flags[0]&SORT_reverse)
		r = -r;
	return(r);
}

static int numsort(const char *s1, const char *s2)
{
	struct Sort	*sp = Sp;
	Sfdouble_t	d1,d2;
	int		r=0;
	nv_putsub(sp->np, NULL,((struct Node*)s1)->index,0);
	d1 = nv_getnum(sp->np);
	nv_putsub(sp->np, NULL,((struct Node*)s2)->index,0);
	d2 = nv_getnum(sp->np);
	if(d2<d1)
		r = 1;
	else if(d2>d1)
		r = -1;
	if(sp->flags[0]&SORT_reverse)
		r = -r;
	return(r);
}

/* ======== option handling	======== */

void *sh_argopen(Shell_t *shp)
{
	void *addr = newof(0,Arg_t,1,0);
	Arg_t *ap = (Arg_t*)addr;
	ap->sh = shp;
	return(addr);
}

static int infof(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	Shell_t *shp = sh_getinterp();
#if SHOPT_BASH
	extern const char sh_bash1[], sh_bash2[];
	if(strcmp(s,"bash1")==0) 
	{
		if(sh_isoption(shp,SH_BASH))
			sfputr(sp,sh_bash1,-1);
	}
	else if(strcmp(s,"bash2")==0)
	{
		if(sh_isoption(shp,SH_BASH))
			sfputr(sp,sh_bash2,-1);
	}
	else if(*s==':' && sh_isoption(shp,SH_BASH))
		sfputr(sp,s,-1);
	else
#endif
	if(*s!=':')
		sfputr(sp,sh_set,-1);
	return(1);
}

/*
 *  This routine turns options on and off
 *  The options "PDicr" are illegal from set command.
 *  The -o option is used to set option by name
 *  This routine returns the number of non-option arguments
 */
int sh_argopts(int argc,register char *argv[], void *context)
{
	Shell_t		*shp = (Shell_t*)context;
	register int	n,o;
	register Arg_t	*ap = (Arg_t*)(shp->arg_context);
	Lex_t		*lp = (Lex_t*)(shp->lex_context);
	Shopt_t		newflags;
	int setflag=0, action=0, trace=(int)sh_isoption(shp,SH_XTRACE);
	Namval_t *np = NIL(Namval_t*);
	const char *sp;
	char *keylist=0;
	int verbose,f,unsetnp=0;
	Optdisc_t disc;
	newflags=shp->options;
	memset(&disc, 0, sizeof(disc));
	disc.version = OPT_VERSION;
	disc.infof = infof;
	opt_info.disc = &disc;

	if(argc>0)
		setflag = 4;
	else
		argc = -argc;
	while((n = optget(argv,setflag?sh_optset:sh_optksh)))
	{
		o=0;
		f=*opt_info.option=='-' && (opt_info.num || opt_info.arg);
		switch(n)
		{
	 	    case 'A':
			np = nv_open(opt_info.arg,shp->var_tree,NV_NOASSIGN|NV_ARRAY|NV_VARNAME);
			if(f)
				unsetnp=1;
			continue;
	 	    case 'K':
			keylist = opt_info.arg;
			continue;
#if SHOPT_BASH
		    case 'O':	/* shopt options, only in bash mode */
			if(!sh_isoption(shp,SH_BASH))
				errormsg(SH_DICT,ERROR_exit(1), e_option, opt_info.name);
#endif
		    case 'o':	/* set options */
		    byname:
			if(!opt_info.arg||!*opt_info.arg||*opt_info.arg=='-')
			{
				action = PRINT;
				/* print style: -O => shopt options
				 * bash => print unset options also, no heading
				 */
				verbose = (f?PRINT_VERBOSE:PRINT_NO_HEADER)|
					  (n=='O'?PRINT_SHOPT:0)|
					  (sh_isoption(shp,SH_BASH)?PRINT_ALL|PRINT_NO_HEADER:0)|
					  ((opt_info.arg&&(!*opt_info.arg||*opt_info.arg=='-'))?(PRINT_TABLE|PRINT_NO_HEADER):0);
				continue;
			}
			o = sh_lookopt(opt_info.arg,&f);
			if(o<=0
				|| (!sh_isoption(shp,SH_BASH) && (o&SH_BASHEXTRA))
				|| ((!sh_isoption(shp,SH_BASH) || n=='o') && (o&SH_BASHOPT))

				|| (setflag && (o&SH_COMMANDLINE)))
			{
				errormsg(SH_DICT,2, e_option, opt_info.arg);
				error_info.errors++;
			}
			o &= 0xff;
			if(sh_isoption(shp,SH_RESTRICTED) && !f && o==SH_RESTRICTED)
				errormsg(SH_DICT,ERROR_exit(1), e_restricted, opt_info.arg);
			break;
#if SHOPT_BASH
		    case -1:	/* --rcfile */
			shp->gd->rcfile = opt_info.arg;
			continue;
		    case -2:	/* --noediting */
			if (!f)
			{
				off_option(&newflags,SH_VI);
				off_option(&newflags,SH_EMACS);
				off_option(&newflags,SH_GMACS);
			}
			continue;
		    case -3:	/* --profile */
			n = 'l';
			goto skip;
		    case -4:	/* --posix */
			/* mask lower 8 bits to find char in optksh string */
			n&=0xff;
			goto skip;
		    case -5:	/* --version */
			sfputr(sfstdout, "ksh bash emulation, version ",-1);
			np = nv_open("BASH_VERSION",shp->var_tree,0);
			sfputr(sfstdout, nv_getval(np),-1);
			np = nv_open("MACHTYPE",shp->var_tree,0);
			sfprintf(sfstdout, " (%s)\n", nv_getval(np));
			sh_exit(shp,0);
#endif
		    case -6:	/* --default */
			{
				register const Shtable_t *tp;
				for(tp=shtab_options; o = tp->sh_number; tp++)
					if(!(o&SH_COMMANDLINE) && is_option(&newflags,o&0xff))
						off_option(&newflags,o&0xff);
			}
		    	continue;
	 	    case -7:
			f = 0;
		    	goto byname;
	 	    case 'D':
			on_option(&newflags,SH_NOEXEC);
			goto skip;
		    case 'T':
			if (opt_info.num)
				shp->test |= opt_info.num;
			else
				shp->test = 0;
		    	continue;
		    case 's':
			if(setflag)
			{
				action = SORT;
				continue;
			}
#if SHOPT_KIA
			goto skip;
		    case 'R':
			if(setflag)
				n = ':';
			else
			{
				ap->kiafile = opt_info.arg;
				n = 'n';
			}
			/*FALLTHROUGH*/
#endif /* SHOPT_KIA */
#if SHOPT_REGRESS
			goto skip;
		    case 'I':
			continue;
#endif /* SHOPT_REGRESS */
		    skip:
		    default:
			if(sp=strchr(optksh,n))
				o = flagval[sp-optksh];
			break;
		    case ':':
			if(opt_info.name[0]=='-'&&opt_info.name[1]=='-')
			{
				opt_info.arg = argv[opt_info.index-1] + 2;
				f = 1;
				goto byname;
			}
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			continue;
		    case '?':
			errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
			return(-1);
		}
		if(f)
		{
			if(o==SH_VI || o==SH_EMACS || o==SH_GMACS)
			{
				off_option(&newflags,SH_VI);
				off_option(&newflags,SH_EMACS);
				off_option(&newflags,SH_GMACS);
			}
			on_option(&newflags,o);
			off_option(&shp->offoptions,o);
		}
		else
		{
			if(o==SH_XTRACE)
				trace = 0;
			off_option(&newflags,o);
			if(setflag==0)
				on_option(&shp->offoptions,o);
		}
	}
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage(NIL(char*)));
	/* check for '-' or '+' argument */
	if((sp=argv[opt_info.index]) && sp[1]==0 && (*sp=='+' || *sp=='-') &&
		strcmp(argv[opt_info.index-1],"--"))
	{
		opt_info.index++;
		off_option(&newflags,SH_XTRACE);
		off_option(&newflags,SH_VERBOSE);
		trace = 0;
	}
	if(trace)
		sh_trace(shp,argv,1);
	argc -= opt_info.index;
	argv += opt_info.index;
	if(action==PRINT)
		sh_printopts(shp,newflags,verbose,0);
	if(setflag)
	{
		if(action==SORT)
		{
			int	(*sortfn)(const char*,const char*) = strcoll;
			Namarr_t	*arp;
			union Value	*args;
			unsigned char	*bits;
			if(argc>0)
				strsort(argv,argc,sortfn);
			else if(np && (arp=nv_arrayptr(np)) && (args = nv_aivec(np,&bits)))
			{
				struct Sort	*sp;
				char		*cp;
				int		i, c, keys=0;
				size_t		nodesize;
				if(keylist)
				{
					for(cp=keylist;c= *cp; cp++)
					{
						if(c==',')
							keys++;
					}
					keys++;
				}
				else
					keylist = Empty;
				arp->nelem = nv_aipack(arp);;
				cp = nv_name(np);
				c = strlen(cp);
				nodesize = sizeof(struct Node)+(keys-1)*sizeof(Namval_t*);
				sp = (struct Sort*)malloc(sizeof(struct Sort)+strlen(keylist)+(sizeof(char*)+1)*keys+(arp->nelem+1)*(nodesize+sizeof(void*))+c+3);
				sp->shp = shp;
				sp->np = np;
				if(!(sp->root = shp->last_root))
					sp->root = shp->var_tree;
				sp->vp = args;
				sp->cur = 0;
				sp->nodes = (struct Node*)&sp->keys[keys+2];
				memset(sp->nodes, 0, arp->nelem*nodesize);
				sp->nptrs = (struct Node**)((char*)sp->nodes+arp->nelem*nodesize);
				sp->flags = (char*)&sp->nptrs[arp->nelem+1];
				memset(sp->flags,0,keys+1);
				sp->name = sp->flags + keys+1;
				memcpy(sp->name,cp,c+1);
				sp->keys[0] = sp->name+c+1;
				strcpy(sp->keys[0],keylist);
				cp = (char*)sp->nodes;
				for(c=0; c < arp->nelem; c++)
				{
					if(keylist!=Empty && *keylist!=':')
					{
						((struct Node*)cp)->index = strtol(args[c].np->nvname,NULL,10);
						((struct Node*)cp)->bits = bits[c];
					}
					else
						((struct Node*)cp)->index = c;
					((struct Node*)cp)->vp = args[c];
					sp->nptrs[c] = (struct Node*)cp;
					cp += nodesize;
				}
				if(!(cp = sp->keys[0]))
					cp = keylist;
				for(keys=0;c= *cp; cp++)
				{
					if(c==',')
					{
						*cp++ = 0;
						sp->keys[++keys] = cp;
						sp->flags[keys] = 0;
					}
					else if(c==':')
					{
					again:
						*cp++ = 0;
						if((c= *cp) == 'r')
						{
							sp->flags[keys] |= SORT_reverse; 
							c = cp[1];
						}
						else if(c=='n')
						{
							sp->flags[keys] |= SORT_numeric; 
							c = cp[1];
						}
						if(c=='n' || c=='r')
							goto again;
						 
					}
				}
				sp->keys[++keys] = 0;
				Sp = sp;
				if(sp->keys[0] && *sp->keys[0])
					sortfn = arraysort;
				else if(sp->flags[0]&SORT_numeric)
					sortfn = numsort;
				else
					sortfn = alphasort;
				strsort((char**)sp->nptrs,arp->nelem,sortfn);
				cp = (char*)sp->nodes;
				for(c=0; c < arp->nelem; c++)
				{
					i = (char*)sp->nptrs[c]-(char*)&sp->nodes[0];
					if(i/nodesize !=c)
					{
						args[c] = ((struct Node*)(cp+i))->vp;
						bits[c] = ((struct Node*)(cp+i))->bits;
					}
				}
				free(sp);
				nv_close(np);
				np = 0;
			}
			else
				strsort(shp->st.dolv+1,shp->st.dolc,sortfn);
		}
		if(np)
		{
			if(unsetnp)
				nv_unset(np);
			nv_setvec(np,0,argc,argv);
			nv_close(np);
		}
		else if(argc>0 || ((sp=argv[-1]) && strcmp(sp,"--")==0))
			sh_argset(ap,argv-1);
	}
	else if(is_option(&newflags,SH_CFLAG))
	{
		if(!(shp->comdiv = *argv++))
		{
			errormsg(SH_DICT,2,e_cneedsarg);
			errormsg(SH_DICT,ERROR_usage(2),optusage(NIL(char*)));
		}
		argc--;
	}
	/* handling SH_INTERACTIVE and SH_PRIVILEGED has been moved to
	 * sh_applyopts(), so that the code can be reused from b_shopt(), too
	 */
	sh_applyopts(shp,newflags);
#if SHOPT_KIA
	if(ap->kiafile)
	{
		if(!argv[0])
			errormsg(SH_DICT,ERROR_usage(2),"-R requires scriptname");
		if(!(lp->kiafile=sfopen(NIL(Sfio_t*),ap->kiafile,"w+")))
			errormsg(SH_DICT,ERROR_system(3),e_create,ap->kiafile);
		if(!(lp->kiatmp=sftmp(2*SF_BUFSIZE)))
			errormsg(SH_DICT,ERROR_system(3),e_tmpcreate);
		sfputr(lp->kiafile,";vdb;CIAO/ksh",'\n');
		lp->kiabegin = sftell(lp->kiafile);
		lp->entity_tree = dtopen(&_Nvdisc,Dtbag);
		lp->scriptname = strdup(sh_fmtq(argv[0]));
		lp->script=kiaentity(lp,lp->scriptname,-1,'p',-1,0,0,'s',0,"");
		lp->fscript=kiaentity(lp,lp->scriptname,-1,'f',-1,0,0,'s',0,"");
		lp->unknown=kiaentity(lp,"<unknown>",-1,'p',-1,0,0,'0',0,"");
		kiaentity(lp,"<unknown>",-1,'p',0,0,lp->unknown,'0',0,"");
		lp->current = lp->script;
		ap->kiafile = 0;
	}
#endif /* SHOPT_KIA */
	return(argc);
}

/* apply new options */

void sh_applyopts(Shell_t* shp,Shopt_t newflags)
{
	/* cannot set -n for interactive shells since there is no way out */
	if(sh_isoption(shp,SH_INTERACTIVE))
		off_option(&newflags,SH_NOEXEC);
	if(is_option(&newflags,SH_PRIVILEGED))
		on_option(&newflags,SH_NOUSRPROFILE);
	if(!sh_isstate(shp,SH_INIT) && is_option(&newflags,SH_PRIVILEGED) != sh_isoption(shp,SH_PRIVILEGED) || sh_isstate(shp,SH_INIT) && is_option(&((Arg_t*)shp->arg_context)->sh->offoptions,SH_PRIVILEGED) && shp->gd->userid!=shp->gd->euserid)
	{
		if(!is_option(&newflags,SH_PRIVILEGED))
		{
			setuid(shp->gd->userid);
			setgid(shp->gd->groupid);
			if(shp->gd->euserid==0)
			{
				shp->gd->euserid = shp->gd->userid;
				shp->gd->egroupid = shp->gd->groupid;
			}
		}
		else if((shp->gd->userid!=shp->gd->euserid && setuid(shp->gd->euserid)<0) ||
			(shp->gd->groupid!=shp->gd->egroupid && setgid(shp->gd->egroupid)<0) ||
			(shp->gd->userid==shp->gd->euserid && shp->gd->groupid==shp->gd->egroupid))
				off_option(&newflags,SH_PRIVILEGED);
	}
#if SHOPT_BASH
	on_option(&newflags,SH_CMDHIST);
	on_option(&newflags,SH_CHECKHASH);
	on_option(&newflags,SH_EXECFAIL);
	on_option(&newflags,SH_EXPAND_ALIASES);
	on_option(&newflags,SH_HISTAPPEND);
	on_option(&newflags,SH_INTERACTIVE_COMM);
	on_option(&newflags,SH_LITHIST);
	on_option(&newflags,SH_NOEMPTYCMDCOMPL);

	if(!is_option(&newflags,SH_XPG_ECHO) && sh_isoption(shp,SH_XPG_ECHO))
		astconf("UNIVERSE", 0, "ucb");
	if(is_option(&newflags,SH_XPG_ECHO) && !sh_isoption(shp,SH_XPG_ECHO))
		astconf("UNIVERSE", 0, "att");
	if(!is_option(&newflags,SH_PHYSICAL) && sh_isoption(shp,SH_PHYSICAL))
		astconf("PATH_RESOLVE", 0, "metaphysical");
	if(is_option(&newflags,SH_PHYSICAL) && !sh_isoption(shp,SH_PHYSICAL))
		astconf("PATH_RESOLVE", 0, "physical");
	if(is_option(&newflags,SH_HISTORY2) && !sh_isoption(shp,SH_HISTORY2))
	{
		sh_onstate(shp,SH_HISTORY);
                sh_onoption(shp,SH_HISTORY);
	}
	if(!is_option(&newflags,SH_HISTORY2) && sh_isoption(shp,SH_HISTORY2))
	{
		sh_offstate(shp,SH_HISTORY);
		sh_offoption(shp,SH_HISTORY);
	}
#endif
	shp->options = newflags;
}

/*
 * returns the value of $-
 */
char *sh_argdolminus(void *context)
{
	Shell_t	*shp = (Shell_t*)context;
	register Arg_t *ap = (Arg_t*)shp->arg_context;
	register const char *cp=optksh;
	register char *flagp=ap->flagadr;
	while(cp< &optksh[NUM_OPTS])
	{
		int n = flagval[cp-optksh];
		if(sh_isoption(shp,n))
			*flagp++ = *cp;
		cp++;
	}
	*flagp = 0;
	return(ap->flagadr);
}

/*
 * set up positional parameters 
 */
static void sh_argset(Arg_t *ap,char *argv[])
{
	sh_argfree(ap->sh,ap->dolh,0);
	ap->dolh = sh_argcreate(argv);
	/* link into chain */
	ap->dolh->dolnxt = ap->argfor;
	ap->argfor = ap->dolh;
	ap->sh->st.dolc = ap->dolh->dolnum-1;
	ap->sh->st.dolv = ap->dolh->dolval;
}

/*
 * free the argument list if the use count is 1
 * If count is greater than 1 decrement count and return same blk
 * Free the argument list if the use count is 1 and return next blk
 * Delete the blk from the argfor chain
 * If flag is set, then the block dolh is not freed
 */
struct dolnod *sh_argfree(Shell_t *shp, struct dolnod *blk,int flag)
{
	register struct dolnod*	argr=blk;
	register struct dolnod*	argblk;
	register Arg_t *ap = (Arg_t*)shp->arg_context;
	if(argblk=argr)
	{
		if((--argblk->dolrefcnt)==0)
		{
			argr = argblk->dolnxt;
			if(flag && argblk==ap->dolh)
				ap->dolh->dolrefcnt = 1;
			else
			{
				/* delete from chain */
				if(ap->argfor == argblk)
					ap->argfor = argblk->dolnxt;
				else
				{
					for(argr=ap->argfor;argr;argr=argr->dolnxt)
						if(argr->dolnxt==argblk)
							break;
					if(!argr)
						return(NIL(struct dolnod*));
					argr->dolnxt = argblk->dolnxt;
					argr = argblk->dolnxt;
				}
				free((void*)argblk);
			}
		}
	}
	return(argr);
}

/*
 * grab space for arglist and copy args
 * The strings are copied after the argment vector
 */
struct dolnod *sh_argcreate(register char *argv[])
{
	register struct dolnod *dp;
	register char **pp=argv, *sp;
	register int 	n;
	register size_t size=0;
	/* count args and number of bytes of arglist */
	while(sp= *pp++)
		size += strlen(sp);
	n = (pp - argv)-1;
	dp=new_of(struct dolnod,n*sizeof(char*)+size+n);
	dp->dolrefcnt=1;	/* use count */
	dp->dolnum = n;
	dp->dolnxt = 0;
	pp = dp->dolval;
	sp = (char*)dp + sizeof(struct dolnod) + n*sizeof(char*);
	while(n--)
	{
		*pp++ = sp;
		sp = strcopy(sp, *argv++) + 1;
	}
	*pp = NIL(char*);
	return(dp);
}

/*
 *  used to set new arguments for functions
 */
struct dolnod *sh_argnew(Shell_t *shp,char *argi[], struct dolnod **savargfor)
{
	register Arg_t *ap = (Arg_t*)shp->arg_context;
	register struct dolnod *olddolh = ap->dolh;
	*savargfor = ap->argfor;
	ap->dolh = 0;
	ap->argfor = 0;
	sh_argset(ap,argi);
	return(olddolh);
}

/*
 * reset arguments as they were before function
 */
void sh_argreset(Shell_t *shp,struct dolnod *blk, struct dolnod *afor)
{
	register Arg_t *ap = (Arg_t*)shp->arg_context;
	while(ap->argfor=sh_argfree(shp,ap->argfor,0));
	ap->argfor = afor;
	if(ap->dolh = blk)
	{
		shp->st.dolc = ap->dolh->dolnum-1;
		shp->st.dolv = ap->dolh->dolval;
	}
}

/*
 * increase the use count so that an sh_argset will not make it go away
 */
struct dolnod *sh_arguse(Shell_t* shp)
{
	register struct dolnod *dh;
	register Arg_t *ap = (Arg_t*)shp->arg_context;
	if(dh=ap->dolh)
		dh->dolrefcnt++;
	return(dh);
}

/*
 *  Print option settings on standard output
 *  if mode is inclusive or of PRINT_*
 *  if <mask> is set, only options with this mask value are displayed
 */
void sh_printopts(Shell_t *shp,Shopt_t oflags,register int mode, Shopt_t *mask)
{
	register const Shtable_t *tp;
	const char *name;
	int on;
	int value;
	if(!(mode&PRINT_NO_HEADER))
		sfputr(sfstdout,sh_translate(e_heading),'\n');
	if(mode&PRINT_TABLE)
	{
		size_t	w;
		int	c;
		int	r;
		int	i;

		c = 0;
		for(tp=shtab_options; value=tp->sh_number; tp++)
		{
			if(mask && !is_option(mask,value&0xff))
				continue;
			name = tp->sh_name;
			if(name[0] == 'n' && name[1] == 'o' && name[2] != 't')
				name += 2;
			if(c<(w=strlen(name)))
				c = w;
		}
		c += 4;
		if((w = ed_window()) < (2*c))
			w = 2*c;
		r = w / c;
		i = 0;
		for(tp=shtab_options; value=tp->sh_number; tp++)
		{
			if(mask && !is_option(mask,value&0xff))
				continue;
			on = !!is_option(&oflags,value);
			value &= 0xff;
			name = tp->sh_name;
			if(name[0] == 'n' && name[1] == 'o' && name[2] != 't')
			{
				name += 2;
				on = !on;
			}
			if(++i>=r)
			{
				i = 0;
				sfprintf(sfstdout, "%s%s\n", on ? "" : "no", name);
			}
			else
				sfprintf(sfstdout, "%s%-*s", on ? "" : "no", on ? c : (c-2), name);
		}
		if(i)
			sfputc(sfstdout,'\n');
		return;
	}
#if SHOPT_RAWONLY
	on_option(&oflags,SH_VIRAW);
#endif
	if(!(mode&(PRINT_ALL|PRINT_VERBOSE))) /* only print set options */
	{
		if(mode&PRINT_SHOPT)
			sfwrite(sfstdout,"shopt -s",3);
		else
			sfwrite(sfstdout,"set --default",13);
	}
	for(tp=shtab_options; value=tp->sh_number; tp++)
	{
		if(mask && !is_option(mask,value&0xff))
			continue;
		if(sh_isoption(shp,SH_BASH))
		{
			if (!(mode&PRINT_SHOPT) != !(value&SH_BASHOPT))
				continue;
		}
		else if (value&(SH_BASHEXTRA|SH_BASHOPT))
			continue;
		on = !!is_option(&oflags,value);
		name = tp->sh_name;
		if(name[0] == 'n' && name[1] == 'o' && name[2] != 't')
		{
			name += 2;
			on = !on;
		}
		if(mode&PRINT_VERBOSE)
		{
			sfputr(sfstdout,name,' ');
			sfnputc(sfstdout,' ',24-strlen(name));
			sfputr(sfstdout,on ? sh_translate(e_on) : sh_translate(e_off),'\n');
		}
		else if(mode&PRINT_ALL) /* print unset options also */
		{
			if(mode&PRINT_SHOPT)
				sfprintf(sfstdout, "shopt -%c %s\n",
					on?'s':'u',
					name);
			else
				sfprintf(sfstdout, "set %co %s\n",
					on?'-':'+',
					name);
		}
		else if(!(value&SH_COMMANDLINE) && is_option(&oflags,value&0xff))
			sfprintf(sfstdout," %s%s%s",(mode&PRINT_SHOPT)?"":"--",on?"":"no",name);
	}
	if(!(mode&(PRINT_VERBOSE|PRINT_ALL)))
		sfputc(sfstdout,'\n');
}

/*
 * build an argument list
 */
char **sh_argbuild(Shell_t *shp,int *nargs, const struct comnod *comptr,int flag)
{
	register struct argnod	*argp;
	struct argnod *arghead=0;
	shp->xargmin = 0;
	{
		register const struct comnod	*ac = comptr;
		register int n;
		/* see if the arguments have already been expanded */
		if(!ac->comarg)
		{
			*nargs = 0;
			return(&null);
		}
		else if(!(ac->comtyp&COMSCAN))
		{
			register struct dolnod *ap = (struct dolnod*)ac->comarg;
			*nargs = ap->dolnum;
			return(ap->dolval+ap->dolbot);
		}
		shp->lastpath = 0;
		procsub = shp->procsub;
		*nargs = 0;
		if(ac)
		{
			if(ac->comnamp == SYSLET)
				flag |= ARG_LET;
			argp = ac->comarg;
			while(argp)
			{
				n = arg_expand(shp,argp,&arghead,flag);
				if(n>1)
				{
					if(shp->xargmin==0)
						shp->xargmin = *nargs;
					shp->xargmax = *nargs+n;
				}
				*nargs += n;
				argp = argp->argnxt.ap;
			}
			argp = arghead;
		}
		if(procsub)
			*procsub = 0;
	}
	{
		register char	**comargn;
		register int	argn;
		register char	**comargm;
		argn = *nargs;
		/* allow room to prepend args */
		argn += 1;

		comargn=(char**)stkalloc(shp->stk,(unsigned)(argn+1)*sizeof(char*));
		comargm = comargn += argn;
		*comargn = NIL(char*);
		if(!argp)
		{
			/* reserve an extra null pointer */
			*--comargn = 0;
			return(comargn);
		}
		while(argp)
		{
			struct argnod *nextarg = argp->argchn.ap;
			argp->argchn.ap = 0;
			*--comargn = argp->argval;
			if(!(argp->argflag&ARG_RAW))
				sh_trim(*comargn);
			if(!(argp=nextarg) || (argp->argflag&ARG_MAKE))
			{
				if((argn=comargm-comargn)>1)
					strsort(comargn,argn,strcoll);
				comargm = comargn;
			}
		}
		shp->last_table = 0;
		return(comargn);
	}
}

#if _pipe_socketpair && !_socketpair_devfd
#   define sh_pipe(a)	sh_rpipe(a)
#endif

struct argnod *sh_argprocsub(Shell_t *shp,struct argnod *argp)
{
	/* argument of the form <(cmd) or >(cmd) */
	register struct argnod *ap;
	int nn, monitor, fd, pv[3];
	int subshell = shp->subshell;
	pid_t pid0;
	ap = (struct argnod*)stkseek(shp->stk,ARGVAL);
	ap->argflag |= ARG_MAKE;
	ap->argflag &= ~ARG_RAW;
	fd = argp->argflag&ARG_RAW;
	if(fd==0 && shp->subshell)
		sh_subtmpfile(shp);
#if SHOPT_DEVFD
	sfwrite(shp->stk,e_devfdNN,8);
	pv[2] = 0;
	sh_pipe(pv);
	sfputr(shp->stk,fmtbase((long)pv[fd],10,0),0);
#else
	pv[0] = -1;
	shp->fifo = pathtemp(0,0,0,"ksh.fifo",0);
	mkfifo(shp->fifo,S_IRUSR|S_IWUSR);
	sfputr(shp->stk,shp->fifo,0);
#endif /* SHOPT_DEVFD */
	ap = (struct argnod*)stkfreeze(shp->stk,0);
	shp->inpipe = shp->outpipe = 0;
	if(monitor = (sh_isstate(shp,SH_MONITOR)!=0))
		sh_offstate(shp,SH_MONITOR);
	shp->subshell = 0;
#if SHOPT_DEVFD
#    ifdef SPAWN_cwd
	if(shp->vex || (shp->vex = (void*)spawnvex_open(0)))
		spawnvex_add((Spawnvex_t*)shp->vex,pv[fd],pv[fd],0,0);
	else
#    endif /* SPAWN_cwd */
		fcntl(pv[fd],F_SETFD,0);
	shp->fdstatus[pv[fd]] &= ~IOCLEX;
#endif /* SHOPT_DEVFD */
	pid0=shp->procsub?*shp->procsub:0; 
	if(fd)
	{
		if(!shp->procsub)
			shp->procsub = procsub = newof(0,pid_t,shp->nprocsub=4,0);
		else if((nn=procsub-shp->procsub) >= shp->nprocsub)
		{
			shp->nprocsub += 3;
			shp->procsub = newof(shp->procsub,pid_t,shp->nprocsub,0);
			procsub = shp->procsub + nn;
		}
		if(pid0)
			*shp->procsub = 0;
		shp->inpipe = pv;
		sh_exec(shp,(Shnode_t*)argp->argchn.ap,(int)sh_isstate(shp,SH_ERREXIT));
		if(pid0)
			*shp->procsub = pid0;
		*procsub++ = job.lastpost;
	}
	else
	{
		shp->outpipe = pv;
		sh_exec(shp,(Shnode_t*)argp->argchn.ap,(int)sh_isstate(shp,SH_ERREXIT));
	}
	shp->subshell = subshell;
	if(monitor)
		sh_onstate(shp,SH_MONITOR);
#if SHOPT_DEVFD
	sh_close(pv[1-fd]);
	sh_iosave(shp,-pv[fd], shp->topfd, (char*)0);
#else
	free(shp->fifo);
	shp->fifo = 0;
#endif /* SHOPT_DEVFD */
	return(ap);
}

/* Argument expansion */
static int arg_expand(Shell_t *shp,register struct argnod *argp, struct argnod **argchain,int flag)
{
	register int count = 0;
	argp->argflag &= ~ARG_MAKE;
	if(*argp->argval==0 && (argp->argflag&ARG_EXP))
	{
		struct argnod *ap;
		ap = sh_argprocsub(shp,argp);
		ap->argchn.ap = *argchain;
		*argchain = ap;
		count++;
	}
	else
	if(!(argp->argflag&ARG_RAW))
	{
#if SHOPT_OPTIMIZE
		struct argnod *ap;
		sh_stats(STAT_ARGEXPAND);
		if(flag&ARG_OPTIMIZE)
			argp->argchn.ap=0;
		if(ap=argp->argchn.ap)
		{
			sh_stats(STAT_ARGHITS);
			count = 1;
			ap->argchn.ap = *argchain;
			ap->argflag |= ARG_RAW;
			ap->argflag &= ~ARG_EXP;
			*argchain = ap;
		}
		else
#endif /* SHOPT_OPTIMIZE */
		count = sh_macexpand(shp,argp,argchain,flag);
	}
	else
	{
		argp->argchn.ap = *argchain;
		*argchain = argp;
		argp->argflag |= ARG_MAKE;
		count++;
	}
	return(count);
}

