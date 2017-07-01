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
 * echo [arg...]
 * print [-nrps] [-f format] [-u filenum] [arg...]
 * printf  format [arg...]
 *
 *   David Korn
 *   AT&T Labs
 */

#include	"defs.h"
#include	<error.h>
#include	"io.h"
#include	"name.h"
#include	"history.h"
#include	"builtins.h"
#include	"streval.h"
#include	<tmx.h>
#include	<ccode.h>

union types_t
{
	unsigned char	c;
	short		h;
	int		i;
	long		l;
	Sflong_t	ll;
	Sfdouble_t	ld;
	double		d;
	float		f;
	char		*s;
	int		*ip;
	char		**p;
};

struct printf
{
	Sffmt_t		hdr;
	int		argsize;
	int		intvar;
	char		**nextarg;
	char		*lastarg;
	char		cescape;
	char		err;
	Shell_t		*sh;
};

struct printmap
{
	size_t		size;
	char		*name;
	char		map[3];
};

static const struct printmap  Pmap[] =
{
	3,	"csv",			"#q",
	3,	"ere",			"R",
	4,	"html",			"H",
	17,	"nounicodeliterals",	"0q",
	7,	"pattern",		"P",
	15,	"unicodeliterals",	"+q",
	3,	"url",			"#H",
	0,	0,			0,
};


static int		extend(Sfio_t*,void*, Sffmt_t*);
static const char   	preformat[] = "";
static char		*genformat(Shell_t*,char*);
static int		fmtvecho(Shell_t*, const char*, struct printf*);
static ssize_t		fmtbase64(Shell_t*,Sfio_t*, char*, const char*, int);

struct print
{
	Shell_t         *sh;
	const char	*options;
	char		raw;
	char		echon;
};

static char* 	nullarg[] = { 0, 0 };

int    B_echo(int argc, char *argv[],Shbltin_t *context)
{
	static char bsd_univ;
	struct print prdata;
	prdata.options = sh_optecho+5;
	prdata.raw = prdata.echon = 0;
	prdata.sh = context->shp;
	NOT_USED(argc);
	/* This mess is because /bin/echo on BSD is different */
	if(!prdata.sh->universe)
	{
		register char *universe;
		if(universe=astconf("UNIVERSE",0,0))
			bsd_univ = (strcmp(universe,"ucb")==0);
		prdata.sh->universe = 1;
	}
	if(!bsd_univ)
		return(b_print(0,argv,(Shbltin_t*)&prdata));
	prdata.options = sh_optecho;
	prdata.raw = 1;
	while(argv[1] && *argv[1]=='-')
	{
		if(strcmp(argv[1],"-n")==0)
			prdata.echon = 1;
#if !SHOPT_ECHOE
		else if(strcmp(argv[1],"-e")==0)
			prdata.raw = 0;
		else if(strcmp(argv[1],"-ne")==0 || strcmp(argv[1],"-en")==0)
		{
			prdata.raw = 0;
			prdata.echon = 1;
		}
#endif /* SHOPT_ECHOE */
		else
			break;
		argv++;
	}
	return(b_print(0,argv,(Shbltin_t*)&prdata));
}

int    b_printf(int argc, char *argv[],Shbltin_t *context)
{
	struct print prdata;
	NOT_USED(argc);
	memset(&prdata,0,sizeof(prdata));
	prdata.sh = context->shp;
	prdata.options = sh_optprintf;
	return(b_print(-1,argv,(Shbltin_t*)&prdata));
}

static int infof(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	const struct printmap *pm;
	char c='%';
	for(pm=Pmap;pm->size>0;pm++)
		sfprintf(sp, "[+%c(%s)q?Equivalent to %%%s.]",c,pm->name,pm->map);
	return(1);
}

/*
 * argc==0 when called from echo
 * argc==-1 when called from printf
 */

int    b_print(int argc, char *argv[], Shbltin_t *context)
{
	register Sfio_t *outfile;
	register int exitval=0,n, fd = 1;
	register Shell_t *shp = context->shp;
	const char *options, *msg = e_file+4;
	char *format = 0, *fmttype=0;
	int sflag = 0, nflag=0, rflag=0, vflag=0;
	Namval_t *vname=0;
	Optdisc_t disc;
	disc.version = OPT_VERSION;
	disc.infof = infof;
	opt_info.disc = &disc;
	if(argc>0)
	{
		options = sh_optprint;
		nflag = rflag = 0;
		format = 0;
	}
	else
	{
		struct print *pp = (struct print*)context;
		shp = pp->sh;
		options = pp->options;
		if(argc==0)
		{
			nflag = pp->echon;
			rflag = pp->raw;
			argv++;
			goto skip;
		}
	}
	while((n = optget(argv,options))) switch(n)
	{
		case 'n':
			nflag++;
			break;
		case 'p':
			fd = shp->coutpipe;
			msg = e_query;
			break;
		case 'f':
			format = opt_info.arg;
			break;
		case 's':
			/* print to history file */
			if(!sh_histinit((void*)shp))
				errormsg(SH_DICT,ERROR_system(1),e_history);
			outfile = shp->gd->hist_ptr->histfp;
			fd = sffileno(outfile);
			sh_onstate(shp,SH_HISTORY);
			sflag++;
			break;
		case 'e':
			rflag = 0;
			break;
		case 'r':
			rflag = 1;
			break;
		case 'u':
			if(opt_info.arg[0]=='p' && opt_info.arg[1]==0)
			{
				fd = shp->coutpipe;
				msg = e_query;
				break;
			}
			fd = (int)strtol(opt_info.arg,&opt_info.arg,10);
			if(*opt_info.arg)
				fd = -1;
			else if(!sh_iovalidfd(shp,fd))
				fd = -1;
			else if(!(shp->inuse_bits&(1<<fd)) && (sh_inuse(shp,fd) || (shp->gd->hist_ptr && fd==sffileno(shp->gd->hist_ptr->histfp))))

				fd = -1;
			break;
		case 'j':
			fmttype = "json";
		case 'v':
			if(argc < 0)
			{
				if(!(vname = nv_open(opt_info.arg, shp->var_tree,NV_VARNAME|NV_NOARRAY)))
					errormsg(SH_DICT,2, "Cannot create variable %s", opt_info.arg);
			}
			else
				vflag='v';
			break;
		case 'C':
			vflag='C';
			break;
		case ':':
			/* The following is for backward compatibility */
#if OPT_VERSION >= 19990123
			if(strcmp(opt_info.name,"-R")==0)
#else
			if(strcmp(opt_info.option,"-R")==0)
#endif
			{
				rflag = 1;
				if(error_info.errors==0)
				{
					argv += opt_info.index+1;
					/* special case test for -Rn */
					if(strchr(argv[-1],'n'))
						nflag++;
					if(*argv && strcmp(*argv,"-n")==0)
					{

						nflag++;
						argv++;
					}
					goto skip2;
				}
			}
			else
				errormsg(SH_DICT,2, "%s", opt_info.arg);
			break;
		case '?':
			errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
			break;
	}
	argv += opt_info.index;
	if(error_info.errors || (argc<0 && !(format = *argv++)))
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if(vflag && format)
		errormsg(SH_DICT,ERROR_usage(2),"-%c and -f are mutually exclusive",vflag);
skip:
	if(format)
		format = genformat(shp,format);
	/* handle special case of '-' operand for print */
	if(argc>0 && *argv && strcmp(*argv,"-")==0 && strcmp(argv[-1],"--"))
		argv++;
	if(vname)
	{
		if(!shp->strbuf2)
			shp->strbuf2 = sfstropen();
		outfile = shp->strbuf2;
		goto printv;
	}
skip2:
	if(fd < 0)
	{
		errno = EBADF;
		n = 0;
	}
	else if(sflag)
		n = IOREAD|IOWRITE|IOSEEK;
	else if(!(n=shp->fdstatus[fd]))
		n = sh_iocheckfd(shp,fd,fd);
	if(!(n&IOWRITE))
	{
		/* don't print error message for stdout for compatibility */
		if(fd==1)
			return(1);
		errormsg(SH_DICT,ERROR_system(1),msg);
	}
	if(!sflag && !(outfile=shp->sftable[fd]))
	{
		sh_onstate(shp,SH_NOTRACK);
		n = SF_WRITE|((n&IOREAD)?SF_READ:0);
		shp->sftable[fd] = outfile = sfnew(NIL(Sfio_t*),shp->outbuff,IOBSIZE,fd,n);
		sh_offstate(shp,SH_NOTRACK);
		sfpool(outfile,shp->outpool,SF_WRITE);
	}
	/* turn off share to guarantee atomic writes for printf */
	n = sfset(outfile,SF_SHARE|SF_PUBLIC,0);
printv:
	if(format)
	{
		/* printf style print */
		Sfio_t *pool;
		struct printf pdata;
		memset(&pdata, 0, sizeof(pdata));
		pdata.sh = shp;
		pdata.hdr.version = SFIO_VERSION;
		pdata.hdr.extf = extend;
		pdata.nextarg = argv;
		sh_offstate(shp,SH_STOPOK);
		pool=sfpool(sfstderr,NIL(Sfio_t*),SF_WRITE);
		do
		{
			if(shp->trapnote&SH_SIGSET)
				break;
			pdata.hdr.form = format;
			sfprintf(outfile,"%!",&pdata);
		} while(*pdata.nextarg && pdata.nextarg!=argv);
		if(pdata.nextarg == nullarg && pdata.argsize>0)
			sfwrite(outfile,stkptr(shp->stk,stktell(shp->stk)),pdata.argsize);
		if(sffileno(outfile)!=sffileno(sfstderr))
			sfsync(outfile);
		sfpool(sfstderr,pool,SF_WRITE);
		exitval = pdata.err;
	}
	else if(vflag)
	{
		while(*argv)
		{
			fmtbase64(shp,outfile,*argv++,fmttype,vflag=='C');
			if(!nflag)
				sfputc(outfile,'\n');
		}
	}
	else
	{
		/* echo style print */
		if(nflag && !argv[0])
			sfsync((Sfio_t*)0);
		else if(sh_echolist(shp,outfile,rflag,argv) && !nflag)
			sfputc(outfile,'\n');
	}
	if(vname)
		nv_putval(vname, sfstruse(outfile),0);
	else if(sflag)
	{
		hist_flush(shp->gd->hist_ptr);
		sh_offstate(shp,SH_HISTORY);
	}
	else if(n&SF_SHARE)
	{
		sfset(outfile,SF_SHARE|SF_PUBLIC,1);
		sfsync(outfile);
	}
	return(exitval);
}

/*
 * echo the argument list onto <outfile>
 * if <raw> is non-zero then \ is not a special character.
 * returns 0 for \c otherwise 1.
 */

int sh_echolist(Shell_t *shp,Sfio_t *outfile, int raw, char *argv[])
{
	register char	*cp;
	register int	n;
	struct printf pdata;
	pdata.cescape = 0;
	pdata.err = 0;
	while(!pdata.cescape && (cp= *argv++))
	{
		if(!raw  && (n=fmtvecho(shp,cp,&pdata))>=0)
		{
			if(n)
				sfwrite(outfile,stkptr(shp->stk,stktell(shp->stk)),n);
		}
		else
			sfputr(outfile,cp,-1);
		if(*argv)
			sfputc(outfile,' ');
		sh_sigcheck(shp);
	}
	return(!pdata.cescape);
}

/*
 * modified version of stresc for generating formats
 */
static char strformat(char *s)
{
        register char*  t;
        register int    c;
        char*           b;
        char*           p;
#if SHOPT_MULTIBYTE && defined(FMT_EXP_WIDE)
	int		w;
#endif

        b = t = s;
        for (;;)
        {
                switch (c = *s++)
                {
                    case '\\':
			if(*s==0)
				break;
#if SHOPT_MULTIBYTE && defined(FMT_EXP_WIDE)
                        c = chrexp(s - 1, &p, &w, FMT_EXP_CHAR|FMT_EXP_LINE|FMT_EXP_WIDE);
#else
                        c = chresc(s - 1, &p);
#endif
                        s = p;
#if SHOPT_MULTIBYTE
#if defined(FMT_EXP_WIDE)
			if(c<0) /* conversion failed => empty string */
				continue;
			if(w)
			{
				t += mbconv(t, c);
				continue;
			}
#else
			if(c>UCHAR_MAX && mbwide())
			{
				t += mbconv(t, c);
				continue;
			}
#endif /* FMT_EXP_WIDE */
#endif /* SHOPT_MULTIBYTE */
			if(c=='%')
				*t++ = '%';
			else if(c==0)
			{
				*t++ = '%';
				c = 'Z';
			}
                        break;
                    case 0:
                        *t = 0;
                        return(t - b);
                }
                *t++ = c;
        }
}


static char *genformat(Shell_t * shp,char *format)
{
	register char *fp;
	stkseek(shp->stk,0);
	sfputr(shp->stk,preformat,-1);
	sfputr(shp->stk,format,-1);
	fp = (char*)stkfreeze(shp->stk,1);
	strformat(fp+sizeof(preformat)-1);
	return(fp);
}

static char *fmthtml(Shell_t *shp,const char *string, int flags)
{
	register const char *cp = string;
	register int c, offset = stktell(shp->stk);
	if(!(flags&SFFMT_ALTER))
	{
		while(c= *(unsigned char*)cp++)
		{
#if SHOPT_MULTIBYTE
			register int s;
			if((s=mbsize(cp-1)) > 1)
			{
				cp += (s-1);
				continue;
			}
#endif /* SHOPT_MULTIBYTE */
			if(c=='<')
				sfputr(shp->stk,"&lt",';');
			else if(c=='>')
				sfputr(shp->stk,"&gt",';');
			else if(c=='&')
				sfputr(shp->stk,"&amp",';');
			else if(c=='"')
				sfputr(shp->stk,"&quot",';');
			else if(c=='\'')
				sfputr(shp->stk,"&apos",';');
			else if(c==' ')
				sfputr(shp->stk,"&nbsp",';');
			else if(!isprint(c) && c!='\n' && c!='\r')
				sfprintf(shp->stk,"&#%X;",CCMAPC(c,CC_NATIVE,CC_ASCII));
			else
				sfputc(shp->stk,c);
		}
	}
	else
	{
		while(c= *(unsigned char*)cp++)
		{
			if(strchr("!*'();@&+$,#[]<>~.\"{}|\\-`^% ",c) || (!isprint(c) && c!='\n' && c!='\r'))
				sfprintf(stkstd,"%%%02X",CCMAPC(c,CC_NATIVE,CC_ASCII));
			else
				sfputc(shp->stk,c);
		}
	}
	sfputc(shp->stk,0);
	return(stkptr(shp->stk,offset));
}

static ssize_t fmtbase64(Shell_t *shp, Sfio_t *iop, char *string, const char *fmt,int alt)
{
	char			*cp;
	Sfdouble_t		d;
	ssize_t			size;
	Namval_t		*np = nv_open(string, shp->var_tree, NV_VARNAME|NV_NOASSIGN|NV_NOADD);
	Namarr_t		*ap;
	static union types_t	number;
	if(!np || nv_isnull(np))
	{
		if(sh_isoption(shp,SH_NOUNSET))
			errormsg(SH_DICT,ERROR_exit(1),e_notset,string);
		return(0);
	}
	if(nv_isattr(np,NV_INTEGER) && !nv_isarray(np))
	{
		d = nv_getnum(np);
		if(nv_isattr(np,NV_DOUBLE))
		{
			if(nv_isattr(np,NV_LONG))
			{
				size = sizeof(Sfdouble_t);
				number.ld = d;
			}
			else if(nv_isattr(np,NV_SHORT))
			{
				size = sizeof(float);
				number.f = (float)d;
			}
			else
			{
				size = sizeof(double);
				number.d = (double)d;
			}
		}
		else
		{
			if(nv_isattr(np,NV_LONG))
			{
				size =  sizeof(Sflong_t);
				number.ll = (Sflong_t)d;
			}
			else if(nv_isattr(np,NV_SHORT))
			{
				size =  sizeof(short);
				number.h = (short)d;
			}
			else
			{
				size =  sizeof(short);
				number.i = (int)d; 
			}
		}
#if 1
		return(sfwrite(iop, (void*)&number, size));
#else
		if(sz)
			*sz = size;
		return((void*)&number);
#endif
	}
	if(nv_isattr(np,NV_BINARY))
#if 1
	{
		Namfun_t *fp;
		for(fp=np->nvfun; fp;fp=fp->next)
		{
			if(fp->disc && fp->disc->writef)
				break;
		}
		if(fp)
			return (*fp->disc->writef)(np, iop, 0, fp); 		
		else
		{
			int n = nv_size(np);
			if(nv_isarray(np))
			{
				nv_onattr(np,NV_RAW);
				cp = nv_getval(np);
				nv_offattr(np,NV_RAW);
			}
			else
				cp = (char*)np->nvalue.cp;
			if((size = n)==0)
				size = strlen(cp);
			size = sfwrite(iop, cp, size);
			return(n?n:size);
		}
	}
	else if(nv_isarray(np) && (ap=nv_arrayptr(np)) && array_elem(ap) && (ap->flags&(ARRAY_UNDEF|ARRAY_SCAN)))
	{
		Namval_t *nspace = shp->namespace;
		if(*string=='.' && memcmp(string,".sh.",4))
			shp->namespace = shp->last_table;
		nv_outnode(np,iop,(alt?-1:0),0);
		sfputc(iop,')');
		shp->namespace = nspace;
		return(sftell(iop));
	}
	else
	{
		Namval_t *nspace = shp->namespace;
		if(alt==1 && nv_isvtree(np))
			nv_onattr(np,NV_EXPORT);
		if(fmt && memcmp(fmt,"json",4)==0)
			nv_onattr(np,NV_JSON);
		if(*string=='.')
			shp->namespace = 0;
		cp = nv_getval(np);
		if(*string=='.')
			shp->namespace = nspace;
		if(alt==1)
			nv_offattr(np,NV_EXPORT);
		else if(fmt && memcmp(fmt,"json",4)==0)
			nv_offattr(np,NV_JSON);
		if(!cp)
			return(0);
		size = strlen(cp);
		return(sfwrite(iop,cp,size));
	}
#else
		nv_onattr(np,NV_RAW);
	cp = nv_getval(np);
	if(nv_isattr(np,NV_BINARY))
		nv_offattr(np,NV_RAW);
	if((size = nv_size(np))==0)
		size = strlen(cp);
	if(sz)
		*sz = size;
	return((void*)cp);
#endif
}

static int varname(const char *str, ssize_t n)
{
	register int c,dot=1,len=1;
	if(n < 0)
	{
		if(*str=='.')
			str++;
		n = strlen(str);
	}
	for(;n > 0; n-=len)
	{
#ifdef SHOPT_MULTIBYTE
		len = mbsize(str);
		c = mbchar(str);
#else
		c = *(unsigned char*)str++;
#endif
		if(dot && !(isalpha(c)||c=='_'))
			break;
		else if(dot==0 && !(isalnum(c) || c=='_' || c == '.'))
			break;
		dot = (c=='.');
	}
	return(n==0);
}

static const char *mapformat(Sffmt_t *fe)
{
	const struct printmap *pm = Pmap;
	while(pm->size>0)
	{
		if(pm->size==fe->n_str && memcmp(pm->name,fe->t_str,fe->n_str)==0)
			return(pm->map);
		pm++;
	}
	return(0);
}

static int extend(Sfio_t* sp, void* v, Sffmt_t* fe)
{
	char*		lastchar = "";
	register int	neg = 0;
	Sfdouble_t	d;
	Sfdouble_t	longmin = LDBL_LLONG_MIN;
	Sfdouble_t	longmax = LDBL_LLONG_MAX;
	int		format = fe->fmt;
	int		n;
	int		fold = fe->base;
	union types_t*	value = (union types_t*)v;
	struct printf*	pp = (struct printf*)fe;
	Shell_t		*shp = pp->sh;
	register char*	argp = *pp->nextarg;
	char		*w,*s;

	if(fe->n_str>0 && (format=='T'||format=='Q') && varname(fe->t_str,fe->n_str) && (!argp || varname(argp,-1)))
	{
		if(argp)
			pp->lastarg = argp;
		else
			argp = pp->lastarg;
		if(argp)
		{
			sfprintf(pp->sh->strbuf,"%s.%.*s%c",argp,fe->n_str,fe->t_str,0);
			argp = sfstruse(pp->sh->strbuf);
		}
	}
	else
		pp->lastarg = 0;
	fe->flags |= SFFMT_VALUE;
	if(!argp || format=='Z')
	{
		switch(format)
		{
		case 'c':
			value->c = 0;
			fe->flags &= ~SFFMT_LONG;
			break;
		case 'q':
			format = 's';
			/* FALL THROUGH */
		case 's':
		case 'H':
		case 'B':
		case 'P':
		case 'R':
		case 'Z':
		case 'b':
			fe->fmt = 's';
			fe->size = -1;
			fe->base = -1;
			value->s = "";
			fe->flags &= ~SFFMT_LONG;
			break;
		case 'a':
		case 'e':
		case 'f':
		case 'g':
		case 'A':
		case 'E':
		case 'F':
		case 'G':
                        if(SFFMT_LDOUBLE)
				value->ld = 0.;
			else
				value->d = 0.;
			break;
		case 'n':
			value->ip = &pp->intvar;
			break;
		case 'Q':
			value->ll = 0;
			break;
		case 'T':
			fe->fmt = 'd';
			value->ll = tmxgettime();
			break;
		default:
			if(!strchr("DdXxoUu",format))
				errormsg(SH_DICT,ERROR_exit(1),e_formspec,format);
			fe->fmt = 'd';
			value->ll = 0;
			break;
		}
	}
	else
	{
		switch(format)
		{
		case 'p':
			value->p = (char**)strtol(argp,&lastchar,10);
			break;
		case 'n':
		{
			Namval_t *np;
			np = nv_open(argp,shp->var_tree,NV_VARNAME|NV_NOASSIGN|NV_NOARRAY);
			_nv_unset(np,0);
			nv_onattr(np,NV_INTEGER);
			if (np->nvalue.lp = new_of(int32_t,0))
				*np->nvalue.lp = 0;
			nv_setsize(np,10);
			if(sizeof(int)==sizeof(int32_t))
				value->ip = (int*)np->nvalue.lp;
			else
			{
				int32_t sl = 1;
				value->ip = (int*)(((char*)np->nvalue.lp) + (*((char*)&sl) ? 0 : sizeof(int)));
			}
			nv_close(np);
			break;
		}
		case 'q':
			if(fe->n_str)
			{
				const char *fp = mapformat(fe);
				if(fp)
				{
					if (!isalpha(*fp))
						switch (*fp++)
						{
						case '#':
							fe->flags |= SFFMT_ALTER;
							break;
						case '+':
							fe->flags |= SFFMT_SIGN;
							break;
						case '0':
							fe->flags |= SFFMT_ZERO;
							break;
						}
					format = *fp;
				}
			}
		case 'b':
		case 's':
		case 'B':
		case 'H':
		case 'P':
		case 'R':
			fe->fmt = 's';
			fe->size = -1;
			if(format=='s' && fe->base>=0)
			{
				value->p = pp->nextarg;
				pp->nextarg = nullarg;
			}
			else
			{
				fe->base = -1;
				value->s = argp;
			}
			fe->flags &= ~SFFMT_LONG;
			break;
		case 'c':
			if(mbwide() && (n = mbsize(argp)) > 1)
			{
				fe->fmt = 's';
				fe->size = n;
				value->s = argp;
			}
			else if(fe->base >=0)
				value->s = argp;
			else
				value->c = *argp;
			fe->flags &= ~SFFMT_LONG;
			break;
		case 'o':
		case 'x':
		case 'X':
		case 'u':
		case 'U':
			longmax = LDBL_ULLONG_MAX;
		case '.':
			if(fe->size==2 && strchr("bcsqHPRQTZ",*fe->form))
			{
				value->ll = ((unsigned char*)argp)[0];
				break;
			}
		case 'd':
		case 'D':
		case 'i':
			switch(*argp)
			{
			case '\'':
			case '"':
				w = argp + 1;
				if(mbwide() && mbsize(w) > 1)
					value->ll = mbchar(w);
				else
					value->ll = *(unsigned char*)w++;
				if(w[0] && (w[0] != argp[0] || w[1]))
				{
					errormsg(SH_DICT,ERROR_warn(0),e_charconst,argp);
					pp->err = 1;
				}
				break;
			default:
				d = sh_strnum(shp,argp,&lastchar,0);
				if(d<longmin)
				{
					errormsg(SH_DICT,ERROR_warn(0),e_overflow,argp);
					pp->err = 1;
					d = longmin;
				}
				else if(d>longmax)
				{
					errormsg(SH_DICT,ERROR_warn(0),e_overflow,argp);
					pp->err = 1;
					d = longmax;
				}
				value->ll = (Sflong_t)d;
				if(lastchar == *pp->nextarg)
				{
					value->ll = *argp;
					lastchar = "";
				}
				break;
			}
			if(neg)
				value->ll = -value->ll;
			fe->size = sizeof(value->ll);
			break;
		case 'a':
		case 'e':
		case 'f':
		case 'g':
		case 'A':
		case 'E':
		case 'F':
		case 'G':
			d = sh_strnum(shp,*pp->nextarg,&lastchar,0);
			switch(*argp)
			{
			    case '\'':
			    case '"':
				d = ((unsigned char*)argp)[1];
				if(argp[2] && (argp[2] != argp[0] || argp[3]))
				{
					errormsg(SH_DICT,ERROR_warn(0),e_charconst,argp);
					pp->err = 1;
				}
				break;
			    default:
				d = sh_strnum(shp,*pp->nextarg,&lastchar,0);
				break;
			}
                        if(SFFMT_LDOUBLE)
			{
				value->ld = d;
				fe->size = sizeof(value->ld);
			}
			else
			{
				value->d = d;
				fe->size = sizeof(value->d);
			}
			break;
		case 'Q':
			value->ll = (Sflong_t)strelapsed(*pp->nextarg,&lastchar,1);
			break;
		case 'T':
			value->ll = (Sflong_t)tmxdate(*pp->nextarg,&lastchar,TMX_NOW);
			break;
		default:
			value->ll = 0;
			fe->fmt = 'd';
			fe->size = sizeof(value->ll);
			errormsg(SH_DICT,ERROR_exit(1),e_formspec,format);
			break;
		}
		if (format == '.')
			value->i = value->ll;
		if(*lastchar)
		{
			errormsg(SH_DICT,ERROR_warn(0),e_argtype,format);
			pp->err = 1;
		}
		pp->nextarg++;
	}
	switch(format)
	{
	case 'Z':
		fe->fmt = 'c';
		fe->base = -1;
		value->c = 0;
		break;
	case 'b':
		if((n=fmtvecho(shp,value->s,pp))>=0)
		{
			if(pp->nextarg == nullarg)
			{
				pp->argsize = n;
				return -1;
			}
			value->s = stkptr(shp->stk,stktell(shp->stk));
			fe->size = n;
		}
		break;
	case 'B':
		if(!shp->strbuf2)
			shp->strbuf2 = sfstropen();
		fe->size = fmtbase64(shp,shp->strbuf2,value->s, fe->n_str?fe->t_str:0, (fe->flags&SFFMT_ALTER)!=0);
		value->s = sfstruse(shp->strbuf2);
		fe->flags |= SFFMT_SHORT;
		break;
	case 'H':
		value->s = fmthtml(shp,value->s, fe->flags);
		break;
	case 'q':
		value->s = sh_fmtqf(value->s, fe->flags, fold);
		break;
	case 'P':
		s = fmtmatch(value->s);
		if(!s || *s==0)
			errormsg(SH_DICT,ERROR_exit(1),e_badregexp,value->s);
		value->s = s;
		break;
	case 'R':
		s = fmtre(value->s);
		if(!s || *s==0)
			errormsg(SH_DICT,ERROR_exit(1),e_badregexp,value->s);
		value->s = s;
		break;
	case 'Q':
		if (fe->n_str>0)
		{
			fe->fmt = 'd';
			fe->size = sizeof(value->ll);
		}
		else
		{
			value->s = fmtelapsed(value->ll, 1);
			fe->fmt = 's';
			fe->size = -1;
		}
		break;
	case 'T':
		if(fe->n_str>0)
		{
			n = fe->t_str[fe->n_str];
			fe->t_str[fe->n_str] = 0;
			value->s = fmttmx(fe->t_str, value->ll);
			fe->t_str[fe->n_str] = n;
		}
		else value->s = fmttmx(NIL(char*), value->ll);
		fe->fmt = 's';
		fe->size = -1;
		break;
	}
	return 0;
}

/*
 * construct System V echo string out of <cp>
 * If there are not escape sequences, returns -1
 * Otherwise, puts null terminated result on stack, but doesn't freeze it
 * returns length of output.
 */

static int fmtvecho(Shell_t *shp,const char *string, struct printf *pp)
{
	register const char *cp = string, *cpmax;
	register int c;
	register int offset = stktell(shp->stk);
#if SHOPT_MULTIBYTE
	int chlen;
	if(mbwide())
	{
		while(1)
		{
			if ((chlen = mbsize(cp)) > 1)
				/* Skip over multibyte characters */
				cp += chlen;
			else if((c= *cp++)==0 || c == '\\')
				break;
		}
	}
	else
#endif /* SHOPT_MULTIBYTE */
	while((c= *cp++) && (c!='\\'));
	if(c==0)
		return(-1);
	c = --cp - string;
	if(c>0)
		sfwrite(shp->stk,(void*)string,c);
	for(; c= *cp; cp++)
	{
#if SHOPT_MULTIBYTE
		if (mbwide() && ((chlen = mbsize(cp)) > 1))
		{
			sfwrite(shp->stk,cp,chlen);
			cp +=  (chlen-1);
			continue;
		}
#endif /* SHOPT_MULTIBYTE */
		if( c=='\\') switch(*++cp)
		{
			case 'E':
				c = ('a'==97?'\033':39); /* ASCII/EBCDIC */
				break;
			case 'a':
				c = '\a';
				break;
			case 'b':
				c = '\b';
				break;
			case 'c':
				pp->cescape++;
				pp->nextarg = nullarg;
				goto done;
			case 'f':
				c = '\f';
				break;
			case 'n':
				c = '\n';
				break;
			case 'r':
				c = '\r';
				break;
			case 'v':
				c = '\v';
				break;
			case 't':
				c = '\t';
				break;
			case '\\':
				c = '\\';
				break;
			case '0':
				c = 0;
				cpmax = cp + 4;
				while(++cp<cpmax && *cp>='0' && *cp<='7')
				{
					c <<= 3;
					c |= (*cp-'0');
				}
			default:
				cp--;
		}
		sfputc(shp->stk,c);
	}
done:
	c = stktell(shp->stk)-offset;
	sfputc(shp->stk,0);
	stkseek(shp->stk,offset);
	return(c);
}
