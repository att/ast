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
 * UNIX shell parse tree executer
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#include	"defs.h"
#include	<fcin.h>
#include	"variables.h"
#include	"path.h"
#include	"name.h"
#include	"io.h"
#include	"shnodes.h"
#include	"jobs.h"
#include	"test.h"
#include	"builtins.h"
#include	"FEATURE/time"
#include	"FEATURE/externs"
#include	"FEATURE/locale"
#include	"streval.h"

#if !_std_malloc
#   include	<vmalloc.h>
#endif

#if     _lib_vfork
#   include     <ast_vfork.h>
#else
#   define vfork()      fork()
#endif

#define SH_NTFORK	SH_TIMING
#define NV_BLTPFSH	NV_ARRAY

#if _lib_nice
    extern int	nice(int);
#endif /* _lib_nice */
#if SHOPT_SPAWN
    static pid_t sh_ntfork(Shell_t*,const Shnode_t*,char*[],int*,int);
#endif /* SHOPT_SPAWN */

static void	sh_funct(Shell_t *,Namval_t*, int, char*[], struct argnod*,int);
static bool	trim_eq(const char*, const char*);
static void	coproc_init(Shell_t*, int pipes[]);

static void	*timeout;
static char	nlock;
static char	pipejob;
static char	nopost;
static int	restorefd;
static int	restorevex;

struct funenv
{
	Namval_t	*node;
	struct argnod	*env;
	Namval_t	**nref;
};

static int io_usevex(struct ionod *iop)
{
	struct ionod *first = iop;
	for(;iop;iop=iop->ionxt)
	{
		if((iop->iofile&IODOC) && !(iop->iofile&IOQUOTE) && iop!=first)
			return(0);
	}
	return(IOUSEVEX);
}
#if 1
#undef IOUSEVEX
#define IOUSEVEX	0
#endif

/* ========	command execution	========*/

#if !SHOPT_DEVFD
    static void fifo_check(void *handle)
    {
	Shell_t	*shp = (Shell_t*)handle;
	pid_t pid = getppid();
	if(pid==1)
	{
		unlink(shp->fifo);
		sh_done(shp,0);
	}
    }
#endif /* !SHOPT_DEVFD */

/*
 * The following two functions allow command substituion for non-builtins
 * to use a pipe and to wait for the pipe to close before restoring to a
 * temp file.
 */
static int      subpipe[3],subdup,tsetio,usepipe;

static bool iousepipe(Shell_t *shp)
{
	int fd=sffileno(sfstdout),i,err=errno;
	if(usepipe)
	{
		usepipe++;
		sh_iounpipe(shp);
	}
	if(sh_rpipe(subpipe) < 0)
		return(false);
	usepipe++;
	if(shp->comsub!=1)
	{
		subpipe[2] = sh_fcntl(subpipe[1],F_DUPFD,10);
		sh_close(subpipe[1]);
		return(true);
	}
	subpipe[2] = sh_fcntl(fd,F_dupfd_cloexec,10);
	shp->fdstatus[subpipe[2]] = shp->fdstatus[1];
	while(close(fd)<0 && errno==EINTR)
		errno = err;
	fcntl(subpipe[1],F_DUPFD,fd);
	shp->fdstatus[1] = shp->fdstatus[subpipe[1]]&~IOCLEX;
	sh_close(subpipe[1]);
	if(subdup=shp->subdup) for(i=0; i < 10; i++)
	{
		if(subdup&(1<<i))
		{
			sh_close(i);
			fcntl(1,F_DUPFD,i);
			shp->fdstatus[i] = shp->fdstatus[1];
		}
	}
	return(true);
}

void sh_iounpipe(Shell_t *shp)
{
	int fd=sffileno(sfstdout),n,err=errno;
	char buff[SF_BUFSIZE];
	if(!usepipe)
		return;
	--usepipe;
	if(shp->comsub>1)
	{
		sh_close(subpipe[2]);
		while(read(subpipe[0],buff,sizeof(buff))>0);
		goto done;
	}
	while(close(fd)<0 && errno==EINTR)
		errno = err;
	fcntl(subpipe[2], F_DUPFD, fd);
	shp->fdstatus[1] = shp->fdstatus[subpipe[2]];
	if(subdup) for(n=0; n < 10; n++)
	{
		if(subdup&(1<<n))
		{
			sh_close(n);
			fcntl(1, F_DUPFD, n);
			shp->fdstatus[n] = shp->fdstatus[1];
		}
	}
	shp->subdup = 0;
	sh_close(subpipe[2]);
	if(usepipe==0) while(1)
	{
		while(job.waitsafe && job.savesig==SIGCHLD)
		{
			if(!vmbusy())
			{
				job.in_critical++;
				job_reap(SIGCHLD);
				job.in_critical--;
				break;
			}
			sh_delay(1);
		}
		if((n = read(subpipe[0],buff,sizeof(buff)))==0)
			break;
		if(n>0)
			sfwrite(sfstdout,buff,n);
		else if(errno!=EINTR)
			break;
	}
done:
	sh_close(subpipe[0]);
	subpipe[0] = -1;
	tsetio = 0;
	usepipe = 0;
}

/*
 * print time <t> in h:m:s format with precision <p>
 */
static void     l_time(Sfio_t *outfile,register clock_t t,int p)
{
	register int  min, sec, frac;
	register int hr;
	if(p)
	{
		frac = t%shgd->lim.clk_tck;
		frac = (frac*100)/shgd->lim.clk_tck;
	}
	t /= shgd->lim.clk_tck;
	sec = t%60;
	t /= 60;
	min = t%60;
	if(hr=t/60)
		sfprintf(outfile,"%dh",hr);
	if(p)
		sfprintf(outfile,"%dm%d%c%0*ds",min,sec,GETDECIMAL(0),p,frac);
	else
		sfprintf(outfile,"%dm%ds",min,sec);
}

static int p_time(Shell_t *shp, Sfio_t *out, const char *format, clock_t *tm)
{
	int		c,p,l,n,offset = stktell(shp->stk);
	const char	*first;
	double		d;
	Stk_t		*stkp = shp->stk;
	for(first=format ; c= *format; format++)
	{
		if(c!='%')
			continue;
		sfwrite(stkp, first, format-first);
		n = l = 0;
		p = 3;
		if((c= *++format) == '%')
		{
			first = format;
			continue;
		}
		if(c>='0' && c <='9')
		{
			p = (c>'3')?3:(c-'0');
			c = *++format;
		}
		else if(c=='P')
		{
			if(d=tm[0])
				d = 100.*(((double)(tm[1]+tm[2]))/d);
			p = 2;
			goto skip;
		}
		if(c=='l')
		{
			l = 1;
			c = *++format;
		}
		if(c=='U')
			n = 1;
		else if(c=='S')
			n = 2;
		else if(c!='R')
		{
			stkseek(stkp,offset);
			errormsg(SH_DICT,ERROR_exit(0),e_badtformat,c);
			return(0);
		}
		d = (double)tm[n]/shp->gd->lim.clk_tck;
	skip:
		if(l)
			l_time(stkp, tm[n], p);
		else
			sfprintf(stkp,"%.*f",p, d);
		first = format+1;
	}
	if(format>first)
		sfwrite(stkp,first, format-first);
	sfputc(stkp,'\n');
	n = stktell(stkp)-offset;
	sfwrite(out,stkptr(stkp,offset),n);
	stkseek(stkp,offset);
	return(n);
}

#if SHOPT_OPTIMIZE
/*
 * clear argument pointers that point into the stack
 */
static int p_arg(Shell_t*,struct argnod*,int);
static int p_switch(Shell_t*,struct regnod*);
static int p_comarg(Shell_t *shp,register struct comnod *com)
{
	Namval_t *np=com->comnamp;
	int n = p_arg(shp,com->comset,ARG_ASSIGN);
	if(com->comarg && (com->comtyp&COMSCAN))
		n+= p_arg(shp,com->comarg,0);
	if(com->comstate  && np)
	{
		/* call builtin to cleanup state */
		Shbltin_t *bp = &shp->bltindata;
		void  *save_ptr = bp->ptr;
		void  *save_data = bp->data;
		bp->bnode = np;
		bp->vnode = com->comnamq;
		bp->ptr = nv_context(np);
		bp->data = com->comstate;
		bp->flags = SH_END_OPTIM;
		((Shbltin_f)funptr(np))(0,(char**)0, bp);
		bp->ptr = save_ptr;
		bp->data = save_data;
	}
	com->comstate = 0;
	if(com->comarg && !np)
		n++;
	return(n);
}

extern void sh_optclear(Shell_t*, void*);

static int sh_tclear(Shell_t *shp, register Shnode_t *t)
{
	int n=0;
	if(!t)
		return(0);
	switch(t->tre.tretyp&COMMSK)
	{
		case TTIME:
		case TPAR:
			return(sh_tclear(shp,t->par.partre)); 
		case TCOM:
			return(p_comarg(shp,(struct comnod*)t));
		case TSETIO:
		case TFORK:
			return(sh_tclear(shp,t->fork.forktre));
		case TIF:
			n=sh_tclear(shp,t->if_.iftre);
			n+=sh_tclear(shp,t->if_.thtre);
			n+=sh_tclear(shp,t->if_.eltre);
			return(n);
		case TWH:
			if(t->wh.whinc)
				n=sh_tclear(shp,(Shnode_t*)(t->wh.whinc));
			n+=sh_tclear(shp,t->wh.whtre);
			n+=sh_tclear(shp,t->wh.dotre);
			return(n);
		case TLST:
		case TAND:
		case TORF:
		case TFIL:
			n=sh_tclear(shp,t->lst.lstlef);
			return(n+sh_tclear(shp,t->lst.lstrit));
		case TARITH:
			return(p_arg(shp,t->ar.arexpr,ARG_ARITH));
		case TFOR:
			n=sh_tclear(shp,t->for_.fortre);
			return(n+sh_tclear(shp,(Shnode_t*)t->for_.forlst));
		case TSW:
			n=p_arg(shp,t->sw.swarg,0);
			return(n+p_switch(shp,t->sw.swlst));
		case TFUN:
			n=sh_tclear(shp,t->funct.functtre);
			return(n+sh_tclear(shp,(Shnode_t*)t->funct.functargs));
		case TTST:
			if((t->tre.tretyp&TPAREN)==TPAREN)
				return(sh_tclear(shp,t->lst.lstlef)); 
			else
			{
				n=p_arg(shp,&(t->lst.lstlef->arg),0);
				if(t->tre.tretyp&TBINARY)
					n+=p_arg(shp,&(t->lst.lstrit->arg),0);
			}
	}
	return(n);
}

static int p_arg(Shell_t *shp,register struct argnod *arg,int flag)
{
	while(arg)
	{
		if(strlen(arg->argval) || (arg->argflag==ARG_RAW))
			arg->argchn.ap = 0;
		else if(flag==0)
			sh_tclear(shp,(Shnode_t*)arg->argchn.ap);
		else
			sh_tclear(shp,((struct fornod*)arg->argchn.ap)->fortre);
		arg = arg->argnxt.ap;
	}
	return(0);
}

static int p_switch(Shell_t *shp,register struct regnod *reg)
{
	int n=0;
	while(reg)
	{
		n+=p_arg(shp,reg->regptr,0);
		n+=sh_tclear(shp,reg->regcom);
		reg = reg->regnxt;
	}
	return(n);
}
#   define OPTIMIZE_FLAG	(ARG_OPTIMIZE)
#   define OPTIMIZE		(flags&OPTIMIZE_FLAG)
#else
#   define OPTIMIZE_FLAG	(0)
#   define OPTIMIZE		(0)
#   define sh_tclear(x,y)
#endif /* SHOPT_OPTIMIZE */

static void out_pattern(Sfio_t *iop, register const char *cp, int n)
{
	register int c;
	do
	{
		switch(c= *cp)
		{
		    case 0:
			if(n<0)
				return;
			c = n;
			break;
		    case '\n':
			sfputr(iop,"$'\\n",'\'');
			continue;
		    case '\\':
			if (!(c = *++cp))
				c = '\\';
			/*FALLTHROUGH*/
		    case ' ':
		    case '<': case '>': case ';':
		    case '$': case '`': case '\t':
			sfputc(iop,'\\');
			break;
		}
		sfputc(iop,c);
	}
	while(*cp++);
}

static void out_string(Sfio_t *iop, register const char *cp, int c, int quoted)
{
	if(quoted)
	{
		int n = stktell(stkstd);
		cp = sh_fmtq(cp);
		if(iop==stkstd && cp==stkptr(stkstd,n))
		{
			*stkptr(stkstd,stktell(stkstd)-1) = c;
			return;
		}
	}
	sfputr(iop,cp,c);
}

struct Level
{
	Namfun_t	hdr;
	short		maxlevel;
};

/*
 * this is for a debugger but it hasn't been tested yet
 * if a debug script sets .sh.level it should set up the scope
 *  as if you were executing in that level
 */ 
static void put_level(Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	Shell_t		*shp = sh_ptr(np);
	Shscope_t	*sp;
	struct Level *lp = (struct Level*)fp;
	int16_t level, oldlevel = (int16_t)nv_getnum(np);
	nv_putv(np,val,flags,fp);
	if(!val)
	{
		fp = nv_stack(np, NIL(Namfun_t*));
		if(fp && !fp->nofree)
			free((void*)fp);
		return;
	}
	level = nv_getnum(np);
	if(level<0 || level > lp->maxlevel)
	{
		nv_putv(np, (char*)&oldlevel, NV_INT16, fp);
		/* perhaps this should be an error */
		return;
	}
	if(level==oldlevel)
		return;
	if(sp = sh_getscope(shp,level,SEEK_SET))
	{
		sh_setscope(shp,sp);
		error_info.id = sp->cmdname;
		
	}
}

static const Namdisc_t level_disc = {  sizeof(struct Level), put_level };

static struct Level *init_level(Shell_t *shp,int level)
{
	struct Level *lp = newof(NiL,struct Level,1,0);
	lp->maxlevel = level;
	_nv_unset(SH_LEVELNOD,0);
	nv_onattr(SH_LEVELNOD,NV_INT16|NV_NOFREE);
	shp->last_root = nv_dict(DOTSHNOD);
	nv_putval(SH_LEVELNOD,(char*)&lp->maxlevel,NV_INT16);
	lp->hdr.disc = &level_disc;
	nv_disc(SH_LEVELNOD,&lp->hdr,NV_FIRST);
	return(lp);
}

/*
 * write the current command on the stack and make it available as .sh.command
 */
int sh_debug(Shell_t *shp, const char *trap, const char *name, const char *subscript, char *const argv[], int flags)
{
	Stk_t			*stkp=shp->stk;
	struct sh_scoped	savst;
	Namval_t		*np = SH_COMMANDNOD;
	char			*sav = stkptr(stkp,0);
	int			n=4, offset=stktell(stkp);
	const char		*cp = "+=( ";
	Sfio_t			*iop = stkstd;
	short			level;
	if(shp->indebug)
		return(0);
	shp->indebug = 1;
	if(name)
	{
		sfputr(iop,name,-1);
		if(subscript)
		{
			sfputc(iop,'[');
			out_string(iop,subscript,']',1);
		}
		if(!(flags&ARG_APPEND))
			cp+=1, n-=1;
		if(!(flags&ARG_ASSIGN))
			n -= 2;
		sfwrite(iop,cp,n);
	}
	if(*argv && !(flags&ARG_RAW))
		out_string(iop, *argv++,' ', 0);
	n = (flags&ARG_ARITH);
	while(cp = *argv++)
	{
		if((flags&ARG_EXP) && argv[1]==0)
			out_pattern(iop, cp,' ');
		else
			out_string(iop, cp,' ',n?0: (flags&(ARG_RAW|ARG_NOGLOB))||*argv);
	}
	if(flags&ARG_ASSIGN)
		sfputc(iop,')');
	else if(iop==stkstd)
		*stkptr(stkp,stktell(stkp)-1) = 0;
	np->nvalue.cp = stkfreeze(stkp,1);
	/* now setup .sh.level variable */
	shp->st.lineno = error_info.line;
	level  = shp->fn_depth+shp->dot_depth;
	shp->last_root = nv_dict(DOTSHNOD);
	if(!SH_LEVELNOD->nvfun || !SH_LEVELNOD->nvfun->disc || nv_isattr(SH_LEVELNOD,NV_INT16|NV_NOFREE)!=(NV_INT16|NV_NOFREE))
		init_level(shp,level);
	else
		nv_putval(SH_LEVELNOD,(char*)&level,NV_INT16);
	savst = shp->st;
	shp->st.trap[SH_DEBUGTRAP] = 0;
	n = sh_trap(shp,trap,0);
	np->nvalue.cp = 0;
	shp->indebug = 0;
	if(shp->st.cmdname)
		error_info.id = shp->st.cmdname;
	nv_putval(SH_PATHNAMENOD,shp->st.filename,NV_NOFREE);
	nv_putval(SH_FUNNAMENOD,shp->st.funname,NV_NOFREE);
	shp->st = savst;
	if(sav != stkptr(stkp,0))
		stkset(stkp,sav,0);
	else
		stkseek(stkp,offset);
	return(n);
}

/*
 * returns true when option -<c> is specified
 */
static bool checkopt(char *argv[], int c)
{
	char *cp;
	while(cp = *++argv)
	{
		if(*cp=='+')
			continue;
		if(*cp!='-' || cp[1]=='-')
			break;
		if(strchr(++cp,c))
			return(1);
		if(*cp=='h' && cp[1]==0 && *++argv==0)
			break;
	}
	return(false);
}

static void free_list(struct openlist *olist)
{
	struct openlist *item,*next;
	for(item=olist;item;item=next)
	{
		next = item->next;
		free((void*)item);
	}
}

/*
 * set ${.sh.name} and ${.sh.subscript}
 * set _ to reference for ${.sh.name}[$.sh.subscript]
 */
static int set_instance(Shell_t *shp,Namval_t *nq, Namval_t *node, struct Namref *nr)
{
	char		*sp=0,*cp;
	Namarr_t	*ap;
	Namval_t	*np;
	if(!nv_isattr(nq,NV_MINIMAL|NV_EXPORT|NV_ARRAY) && (np=(Namval_t*)nq->nvenv) && nv_isarray(np))
		nq = np;
	else if(nv_isattr(nq,NV_MINIMAL)==NV_MINIMAL && !nv_type(nq) && (np=nv_typeparent(nq)))
		nq = np;
	cp = nv_name(nq);
	memset(nr,0,sizeof(*nr));
	nr->np = nq;
	nr->root = shp->var_tree;
	nr->table = shp->last_table;
#if SHOPT_NAMESPACE
	if(!nr->table && shp->namespace)
		nr->table = shp->namespace;
#endif /* SHOPT_NAMESPACE */
	shp->instance = 1;
	if((ap=nv_arrayptr(nq)) && (sp = nv_getsub(nq)))
		sp = strdup(sp);
	shp->instance = 0;
	if(shp->var_tree!=shp->var_base && !nv_search((char*)nq,nr->root,HASH_BUCKET|HASH_NOSCOPE))
	{
#if SHOPT_NAMESPACE
		nr->root = shp->namespace?nv_dict(shp->namespace):shp->var_base;
#else
		nr->root = shp->var_base;
#endif /* SHOPT_NAMESPACE */
	}
	nv_putval(SH_NAMENOD, cp, NV_NOFREE);
	memcpy(node,L_ARGNOD,sizeof(*node));
	L_ARGNOD->nvalue.nrp = nr;
	L_ARGNOD->nvflag = NV_REF|NV_NOFREE;
	L_ARGNOD->nvfun = 0;
	L_ARGNOD->nvenv = 0;
	if(sp)
	{
		nv_putval(SH_SUBSCRNOD,nr->sub=sp,NV_NOFREE);
		return(ap->flags&ARRAY_SCAN);
	}
	return(0);
}

static void unset_instance(Namval_t *nq, Namval_t *node, struct Namref *nr,long mode)
{
	L_ARGNOD->nvalue.nrp = node->nvalue.nrp;
	L_ARGNOD->nvflag = node->nvflag;
	L_ARGNOD->nvfun = node->nvfun;
	if(nr->sub)
	{
		nv_putsub(nr->np, nr->sub, 0, mode);
		free((void*)nr->sub);
	}
	_nv_unset(SH_NAMENOD,0);
	_nv_unset(SH_SUBSCRNOD,0);
}

#if SHOPT_COSHELL
static uintmax_t	coused;
/*
 * print out function definition
 */
static void print_fun(register Namval_t* np, void *data)
{
	register char *format;
	NOT_USED(data);
	if(!is_afunction(np) || !np->nvalue.ip)
		return;
	if(nv_isattr(np,NV_FPOSIX))
		format="%s()\n{ ";
	else
		format="function %s\n{ ";
	sfprintf(sfstdout,format,nv_name(np));
	sh_deparse(sfstdout,(Shnode_t*)(nv_funtree(np)),0);
	sfwrite(sfstdout,"}\n",2);
}

static void *sh_coinit(Shell_t *shp,char **argv)
{
	struct cosh	*csp = job.colist;
	const char 	*name = argv?argv[0]:0;
	int  		id, xopen=1;
	if(!name)
		return(0);
	if(*name=='-')
	{
		name++;
		xopen=0;
	}
	nv_open(name,shp->var_tree,NV_IDENT|NV_NOADD);
	while(csp)
	{
		if(strcmp(name,csp->name)==0)
		{
			if(xopen)
			{
				coattr(csp->coshell,argv[1]);
				return((void*)csp);
			}
			coclose(csp->coshell);
			return(0);
		}
		csp = csp->next;
	}
	if(!xopen)
		errormsg(SH_DICT,ERROR_exit(1),"%s: unknown namespace",name);
	environ[0][2]=0;
	csp = newof(0,struct cosh,1,strlen(name)+1);
	if(!(csp->coshell = coopen(NULL,CO_SHELL|CO_SILENT,argv[1])))
	{
		free((void*)csp);
		errormsg(SH_DICT,ERROR_exit(1),"%s: unable to create namespace",name);
	}
	csp->coshell->data = (void*)csp;
	csp->name = (char*)(csp+1);
	strcpy(csp->name,name);
	for(id=0; coused&(1LL<<id); id++);
	coused |= (1LL<<id);
	csp->id = id;
	csp->next = job.colist;
	job.colist = csp;
	return((void*)csp);
}

bool sh_coaddfile(Shell_t *shp, char *name)
{
	Namval_t *np = dtmatch(shp->inpool,name);
	if(!np)
	{
		np = (Namval_t*)stkalloc(shp->stk,sizeof(Dtlink_t)+sizeof(char*));
		np->nvname = name;
		(Namval_t*)dtinsert(shp->inpool,np);
		shp->poolfiles++;
		return(true);
	}
	return(false);
}

static int sh_coexec(Shell_t *shp,const Shnode_t *t, int filt)
{
	struct cosh	*csp = ((struct cosh*)shp->coshell);
	Cojob_t		*cjp;
	char		*str,*trap,host[PATH_MAX];
	int		lineno,sig,trace = sh_isoption(shp,SH_XTRACE);
	int		verbose = sh_isoption(shp,SH_VERBOSE);
	sh_offoption(shp,SH_XTRACE);
	sh_offoption(shp,SH_VERBOSE);
	if(!shp->strbuf2)
		shp->strbuf2 = sfstropen();
	sfswap(shp->strbuf2,sfstdout);
	sh_trap(shp,"typeset -p\nprint cd \"$PWD\"\nprint .sh.dollar=$$\nprint umask $(umask)",0);
	for(sig=shp->st.trapmax;--sig>0;)
	{
		if((trap=shp->st.trapcom[sig]) && *trap==0)
			sfprintf(sfstdout,"trap '' %d\n",sig);
	}
	if(t->tre.tretyp==TFIL)
		lineno = ((struct forknod*)t->lst.lstlef)->forkline;
	else
		lineno = t->fork.forkline;
	if(filt)
	{
		if(gethostname(host,sizeof(host)) < 0)
			errormsg(SH_DICT,ERROR_system(1),e_pipe);
		if(shp->inpipe[2]>=20000)
			sfprintf(sfstdout,"command exec < /dev/tcp/%s/%d || print -u2 'cannot create pipe'\n",host,shp->inpipe[2]);
		sfprintf(sfstdout,"command exec > /dev/tcp/%s/%d || print -u2 'cannot create pipe'\n",host,shp->outpipe[2]);
		if(filt==3)
			t = t->fork.forktre;
	}
	else
		t = t->fork.forktre;
	nv_scan(shp->fun_tree, print_fun, (void*)0,0, 0);
	if(1)
	{
		Dt_t *top = shp->var_tree;
		sh_scope(shp,(struct argnod*)0,0);
		shp->inpool = dtopen(&_Nvdisc,Dtset);
		sh_exec(shp,t,filt==1||filt==2?SH_NOFORK:0);
		if(shp->poolfiles)
		{
			Namval_t *np;
			sfprintf(sfstdout,"[[ ${.sh} == *pool* ]] && .sh.pool.files=(\n");
			for(np=(Namval_t*)dtfirst(shp->inpool);np;np=(Namval_t*)dtnext(shp->inpool,np))
			{
				sfprintf(sfstdout,"\t%s\n",sh_fmtq(np->nvname));
			}
			sfputr(sfstdout,")",'\n');
			;
		}
		dtclose(shp->inpool);
		shp->inpool = 0;
		shp->poolfiles = 0;
		sh_unscope(shp);
		shp->var_tree = top;
	}
	sfprintf(sfstdout,"typeset -f .sh.pool.init && .sh.pool.init\n");
	sfprintf(sfstdout,"LINENO=%d\n",lineno);
	if(trace)
		sh_onoption(shp,SH_XTRACE);
	if(verbose)
		sh_onoption(shp,SH_VERBOSE);
	sh_trap(shp,"set +o",0);
	sh_deparse(sfstdout,t,filt==1||filt==2?FALTPIPE:0);
	sfputc(sfstdout,0);
	sfswap(shp->strbuf2,sfstdout);
	str = sfstruse(shp->strbuf2);
	if(cjp=coexec(csp->coshell,str,0,NULL,NULL,NULL))
	{
		csp->cojob = cjp;
		cjp->local = shp->coshell;
		if(filt)
		{
			if(filt>1)
				sh_coaccept(shp,shp->inpipe,1);
			sh_coaccept(shp,shp->outpipe,0);
			if(filt > 2)
			{
				shp->coutpipe = shp->inpipe[1];
				shp->fdptrs[shp->coutpipe] = &shp->coutpipe;
			}
		}
		return(sh_copid(csp));
	}
	return(-1);
}
#endif /*SHOPT_COSHELL*/

#if SHOPT_FILESCAN
    static Sfio_t *openstream(Shell_t *shp, struct ionod *iop, int *save)
    {
	int err=errno,savein, fd = sh_redirect(shp,iop,3);
	Sfio_t	*sp;
	savein = dup(0);
	if(fd==0)
		fd = savein;
	sp = sfnew(NULL,NULL,SF_UNBOUND,fd,SF_READ);
	while(close(0)<0 && errno==EINTR)
		errno = err;
	open(e_devnull,O_RDONLY|O_cloexec);
	shp->offsets[0] = -1;
	shp->offsets[1] = 0;
	*save = savein;
	return(sp);
    }
#endif /* SHOPT_FILESCAN */

#if SHOPT_NAMESPACE
static Namval_t *enter_namespace(Shell_t *shp, Namval_t *nsp)
{
	Namval_t	*path=nsp, *fpath=nsp, *onsp=shp->namespace;
	Dt_t		*root=0,*oroot=0;
	char		*val;
	if(nsp)
	{
		if(!nv_istable(nsp))
			nsp = 0;
		else if(nv_dict(nsp)->view!=shp->var_base)
			return(onsp);
	}
	if(!nsp && !onsp)
		return(0);
	if(onsp == nsp) 
		return(nsp);
	if(onsp)
	{
		oroot = nv_dict(onsp);
		if(!nsp)
		{
			path = nv_search(PATHNOD->nvname,oroot,HASH_NOSCOPE);
			fpath = nv_search(FPATHNOD->nvname,oroot,HASH_NOSCOPE);
		}
		if(shp->var_tree==oroot)
		{
			shp->var_tree = shp->var_tree->view;
			oroot = shp->var_base;
		}
	}
	if(nsp)
	{
		if(shp->var_tree==shp->var_base)
			shp->var_tree = nv_dict(nsp);
		else
		{
			for(root=shp->var_tree; root->view!=oroot;root=root->view);
			dtview(root,nv_dict(nsp));
		}
	}
	shp->namespace = nsp;
	if(path && (path = nv_search(PATHNOD->nvname,shp->var_tree,HASH_NOSCOPE)) && (val=nv_getval(path)))
		nv_putval(path,val,NV_RDONLY);
	if(fpath && (fpath = nv_search(FPATHNOD->nvname,shp->var_tree,HASH_NOSCOPE)) && (val=nv_getval(fpath)))
		nv_putval(fpath,val,NV_RDONLY);
	return(onsp);
}
#endif /* SHOPT_NAMESPACE */

int sh_exec(register Shell_t *shp,register const Shnode_t *t, int flags)
{
	Stk_t			*stkp = shp->stk;
	int			unpipe=0;
	sh_sigcheck(shp);
	if(t && !shp->st.execbrk && !sh_isoption(shp,SH_NOEXEC))
	{
		register int 	type = flags;
		register char	*com0 = 0;
		int 		errorflg = (type&sh_state(SH_ERREXIT))|OPTIMIZE;
		int 		execflg = (type&sh_state(SH_NOFORK));
		int 		execflg2 = (type&sh_state(SH_FORKED));
		int 		mainloop = (type&sh_state(SH_INTERACTIVE));
#if SHOPT_AMP || SHOPT_SPAWN
		int		ntflag = (type&sh_state(SH_NTFORK));
#else
		int		ntflag = 0;
#endif
		int		topfd = shp->topfd;
		char 		*sav=stkptr(stkp,0);
		char		*cp=0, **com=0, *comn;
		int		argn;
		int 		skipexitset = 0;
#ifdef SPAWN_cwd
		int		vexi = shp->vexp->cur;
#endif
		pid_t		*procsub = 0;
		volatile int	was_interactive = 0;
		volatile int	was_errexit = sh_isstate(shp,SH_ERREXIT);
		volatile int	was_monitor = sh_isstate(shp,SH_MONITOR);
		volatile int	echeck = 0;
		if(flags&sh_state(SH_INTERACTIVE))
		{
			if(pipejob==2)
				job_unlock();
			nlock = 0;
			pipejob = 0;
			job.curpgid = 0;
			job.curjobid = 0;
			flags &= ~sh_state(SH_INTERACTIVE);
		}
		sh_offstate(shp,SH_ERREXIT);
		sh_offstate(shp,SH_DEFPATH);
		if(was_errexit&flags)
			sh_onstate(shp,SH_ERREXIT);
		if(was_monitor&flags)
			sh_onstate(shp,SH_MONITOR);
		type = t->tre.tretyp;
		if(!shp->intrap)
			shp->oldexit=shp->exitval;
		shp->exitval=0;
		shp->lastsig = 0;
		shp->lastpath = 0;
		switch(type&COMMSK)
		{
		    case TCOM:
		    {
			register struct argnod	*argp;
			char		*trap;
			Namval_t	*np, *nq, *last_table;
			struct ionod	*io;
			int		command=0, flgs=NV_ASSIGN;
			shp->bltindata.invariant = type>>(COMBITS+2);
			shp->bltindata.pwdfd = shp->pwdfd;
			type &= (COMMSK|COMSCAN);
			sh_stats(STAT_SCMDS);
			error_info.line = t->com.comline-shp->st.firstline;
			spawnvex_add(shp->vex,SPAWN_frame,0,0,0);
			com = sh_argbuild(shp,&argn,&(t->com),OPTIMIZE);
			procsub = shp->procsub;
			shp->procsub = 0;
			echeck = 1;
			if(t->tre.tretyp&COMSCAN)
			{
				argp = t->com.comarg;
				if(argp && *com && !(argp->argflag&ARG_RAW))
					sh_sigcheck(shp);
			}
			np = (Namval_t*)(t->com.comnamp);
			nq = (Namval_t*)(t->com.comnamq);
#if SHOPT_NAMESPACE
			if(np && shp->namespace && nq!=shp->namespace && nv_isattr(np,NV_BLTIN|NV_INTEGER|BLT_SPC)!=(NV_BLTIN|BLT_SPC))
			{
				Namval_t *mp;
				if(mp = sh_fsearch(shp,com[0],0))
				{
					nq = shp->namespace;
					np = mp;
				}
			}
#endif /* SHOPT_NAMESPACE */
			com0 = com[0];
			shp->xargexit = 0;
			while(np==SYSCOMMAND)
			{
				register int n = b_command(0,com,&shp->bltindata);
				if(n==0)
					break;
				command += n;
				np = 0;
				if(!(com0= *(com+=n)))
					break;
				np = nv_bfsearch(com0, shp->bltin_tree, &nq, &cp); 
			}
			if(shp->xargexit)
			{
				shp->xargmin -= command;
				shp->xargmax -= command;
			}
			else
				shp->xargmin = 0;
			argn -= command;
#if SHOPT_COSHELL
			if(argn && shp->inpool)
			{
				if(io=t->tre.treio)
					sh_redirect(shp,io,0);
				if(!np || !is_abuiltin(np) || *np->nvname=='/' || np==SYSCD)
				{
					char **argv, *sp;
					for(argv=com+1; sp= *argv; argv++)
					{
						if(sp && *sp && *sp!='-')
							sh_coaddfile(shp,*argv);
					}
					break;
				}
				if(np->nvalue.bfp!=SYSTYPESET->nvalue.bfp)
					break;
			}
			if(t->tre.tretyp&FAMP)
			{
				shp->coshell = sh_coinit(shp,com);
				com0 = 0;
				break;
			}
#endif /* SHOPT_COSHELL */
			if(np && is_abuiltin(np))
			{
				if(!command)
				{
					Namval_t *mp;
#if SHOPT_NAMESPACE
					if(shp->namespace && (mp=sh_fsearch(shp,np->nvname,0)))
						np = mp;
					else
#endif /* SHOPT_NAMESPACE */
					np = dtsearch(shp->fun_tree,np);
				}
#if SHOPT_PFSH
				if(sh_isoption(shp,SH_PFSH) && nv_isattr(np,NV_BLTINOPT) && !nv_isattr(np,NV_BLTPFSH)) 
				{
					if(path_xattr(shp,np->nvname,(char*)0))
					{
						dtdelete(shp->bltin_tree,np);
						np = 0;
					}
					else
						nv_onattr(np,NV_BLTPFSH);
					
				}
#endif /* SHOPT_PFSH */
			}
			if(com0)
			{
				if(!np && !strchr(com0,'/'))
				{
					Dt_t *root = command?shp->bltin_tree:shp->fun_tree;
					np = nv_bfsearch(com0, root, &nq, &cp); 
#if SHOPT_NAMESPACE
					if(shp->namespace && !nq && !cp)
						np = sh_fsearch(shp,com0,0);
#endif /* SHOPT_NAMESPACE */
				}
				comn = com[argn-1];
			}
			io = t->tre.treio;
tryagain:
			if(shp->envlist = argp = t->com.comset)
			{
				if(argn==0 || (np && (nv_isattr(np,BLT_DCL)||(!command && nv_isattr(np,BLT_SPC)))))
				{
					Namval_t *tp=0;
					if(argn)
					{
						if(checkopt(com,'A'))
							flgs |= NV_ARRAY;
						else if(checkopt(com,'a'))
							flgs |= NV_IARRAY;
					}
					if(np)
						flgs |= NV_UNJUST;
#if SHOPT_BASH
					if(np==SYSLOCAL)
					{
						if(!nv_getval(SH_FUNNAMENOD))
							errormsg(SH_DICT,ERROR_exit(1),"%s: can only be used in a function",com0);
						if(!shp->st.var_local)
						{
							sh_scope(shp,(struct argnod*)0,0);
							shp->st.var_local = shp->var_tree;
						}
			
					}
#endif /* SHOPT_BASH */
					if(np==SYSTYPESET ||  (np && np->nvalue.bfp==SYSTYPESET->nvalue.bfp))
					{
						if(np!=SYSTYPESET)
						{
							shp->typeinit = np;
							tp = nv_type(np);
						}
						if(checkopt(com,'C'))
							flgs |= NV_COMVAR;
						if(checkopt(com,'S'))
							flgs |= NV_STATIC;
						if(checkopt(com,'m'))
							flgs |= NV_MOVE;
						if(checkopt(com,'n'))
							flgs |= NV_NOREF;
						else if(argn>=3 && checkopt(com,'T'))
						{
#if SHOPT_NAMESPACE
							if(shp->namespace)
							{
								char	*sp,*xp;
								if(!shp->strbuf2)
									shp->strbuf2 = sfstropen();
								sfprintf(shp->strbuf2,"%s%s%c",NV_CLASS,nv_name(shp->namespace),0);
								shp->prefix = strdup(sfstruse(shp->strbuf2));
								xp = shp->prefix+strlen(NV_CLASS);
								for(sp=xp+1;sp;)
								{
									if(sp = strchr(sp,'.'))
										*sp = 0;
									nv_open(shp->prefix,shp->var_base,NV_VARNAME);
									if(sp)
										*sp++ = '.';
								}
							}
							else
#endif /* SHOPT_NAMESPACE */
							shp->prefix = NV_CLASS;
							flgs |= NV_TYPE;
			
						}
						if((shp->fn_depth && !shp->prefix) || np==SYSLOCAL)
							flgs |= NV_NOSCOPE;
					}
					else if(np==SYSEXPORT)
						flgs |= NV_EXPORT;
					if(flgs&(NV_EXPORT|NV_NOREF))
						flgs |= NV_IDENT;
					else
						flgs |= NV_VARNAME;
#if 0
					if(OPTIMIZE)
						flgs |= NV_TAGGED;
#endif
					if(np && nv_isattr(np,BLT_DCL))
						flgs |= NV_DECL;
					if(t->com.comtyp&COMFIXED)
						((Shnode_t*)t)->com.comtyp &= ~COMFIXED;
					shp->nodelist = sh_setlist(shp,argp,flgs,tp);
					if(np==shp->typeinit)
						shp->typeinit = 0;
					shp->envlist = argp;
					argp = NULL;
				}
			}
			last_table = shp->last_table;
			shp->last_table = 0;
			if((io||argn))
			{
				Shbltin_t *bp=0;
				static char *argv[1];
				int tflags = 1;
				if(np &&  nv_isattr(np,BLT_DCL))
					tflags |= 2;
				if(argn==0)
				{
					/* fake 'true' built-in */
					np = SYSTRUE;
					*argv = nv_name(np);
					com = argv;
				}
				/* set +x doesn't echo */
				else if((t->tre.tretyp&FSHOWME) && sh_isoption(shp,SH_SHOWME))
				{
					int ison = sh_isoption(shp,SH_XTRACE);
					if(!ison)
						sh_onoption(shp,SH_XTRACE);
					sh_trace(shp,com-command,tflags);
					if(io)
						sh_redirect(shp,io,SH_SHOWME|IOHERESTRING);
					if(!ison)
						sh_offoption(shp,SH_XTRACE);
					break;
				}
				else if((np!=SYSSET) && sh_isoption(shp,SH_XTRACE))
					sh_trace(shp,com-command,tflags);
				if(trap=shp->st.trap[SH_DEBUGTRAP])
				{
					int n = sh_debug(shp,trap,(char*)0,(char*)0, com, ARG_RAW);
					if(n==255 && shp->fn_depth+shp->dot_depth)
					{
						np = SYSRETURN;
						argn = 1;
						com[0] = np->nvname;
						com[1] = 0;
						io = 0;
						argp = 0;
					}
					else if(n==2)
						break;
				}
				if(io)
					sfsync(shp->outpool);
				shp->lastpath = 0;
				if(!np  && !strchr(com0,'/'))
				{
					if(path_search(shp,com0,NIL(Pathcomp_t**),1))
					{
						error_info.line = t->com.comline-shp->st.firstline;
#if SHOPT_NAMESPACE
						if(!shp->namespace || !(np=sh_fsearch(shp,com0,0)))
#endif /* SHOPT_NAMESPACE */
							np=nv_search(com0,shp->fun_tree,0);
						if(!np || !np->nvalue.ip)
						{
							Namval_t *mp=nv_search(com0,shp->bltin_tree,0);
							if(mp)
								np = mp;
						}
						else if((t->com.comtyp&COMFIXED) && nv_type(np))
						{
							((Shnode_t*)t)->com.comtyp &= ~COMFIXED;
							goto tryagain;
						}
					}
					else
					{
						if((np=nv_search(com0,shp->track_tree,0)) && !nv_isattr(np,NV_NOALIAS) && np->nvalue.cp)
							np=nv_search(nv_getval(np),shp->bltin_tree,0);
						else
							np = 0;
					}
				}
				if(np && pipejob==2)
				{
					if(shp->comsub==1 && np && is_abuiltin(np) && *np->nvname=='/')
						np = 0;
					else
					{
						job_unlock();
						nlock--;
						pipejob = 1;
					}
				}
				/* check for builtins */
				if(np && is_abuiltin(np))
				{
					volatile int scope=0, share=0;
					volatile void *save_ptr;
					volatile void *save_data;
					int jmpval, save_prompt;
					int was_nofork = execflg?sh_isstate(shp,SH_NOFORK):0;
					struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
					volatile unsigned long was_vi=0, was_emacs=0, was_gmacs=0;
#ifndef O_SEARCH
					struct stat statb;
#endif
					bp = &shp->bltindata;
					save_ptr = bp->ptr;
					save_data = bp->data;
#ifndef O_SEARCH
					memset(&statb, 0, sizeof(struct stat));
#endif
					if(strchr(nv_name(np),'/'))
					{
						/*
						 * disable editors for built-in
						 * versions of commands on PATH
						 */
						was_vi = sh_isoption(shp,SH_VI);
						was_emacs = sh_isoption(shp,SH_EMACS);
						was_gmacs = sh_isoption(shp,SH_GMACS);
						sh_offoption(shp,SH_VI);
						sh_offoption(shp,SH_EMACS);
						sh_offoption(shp,SH_GMACS);
					}
					if(execflg)
						sh_onstate(shp,SH_NOFORK);
					sh_pushcontext(shp,buffp,SH_JMPCMD);
					jmpval = sigsetjmp(buffp->buff,1);
					if(jmpval == 0)
					{
						if(!(nv_isattr(np,BLT_ENV)))
							error_info.flags |= ERROR_SILENT;
						errorpush(&buffp->err,0);
						if(io)
						{
							struct openlist *item;
							if(np==SYSLOGIN)
								type=1;
							else if(np==SYSEXEC)
								type=1+!com[1];
							else
								type = (execflg && !shp->subshell && !shp->st.trapcom[0]);
							shp->redir0 = 1;
							sh_redirect(shp,io,type|(np->nvalue.bfp==(Nambfp_f)b_dot_cmd?0:IOHERESTRING|IOUSEVEX));
							for(item=buffp->olist;item;item=item->next)
								item->strm=0;
						}
						if(!nv_isattr(np,BLT_ENV) && !nv_isattr(np,BLT_SPC))
						{
							if(!shp->pwd)
								path_pwd(shp,0);
#ifndef O_SEARCH
							else if (shp->pwdfd>=0)
								fstat(shp->pwdfd,&statb);
							else if (shp->pwd)
								stat(e_dot,&statb);
#endif
							sfsync(NULL);
							share = sfset(sfstdin,SF_SHARE,0);
							sh_onstate(shp,SH_STOPOK);
							sfpool(sfstderr,NIL(Sfio_t*),SF_WRITE);
							sfset(sfstderr,SF_LINE,1);
							save_prompt = shp->nextprompt;
							shp->nextprompt = 0;
						}
						if(argp)
						{
							scope++;
							sh_scope(shp,argp,0);
						}
						opt_info.index = opt_info.offset = 0;
						opt_info.disc = 0;
						error_info.id = *com;
						if(argn)
							shp->exitval = 0;
						shp->bltinfun = (Shbltin_f)funptr(np);
						bp->bnode = np;
						bp->vnode = nq;
						bp->ptr = nv_context(np);
						bp->data = t->com.comstate;
						bp->sigset = 0;
						bp->notify = 0;
						bp->flags = (OPTIMIZE!=0);
						if(shp->subshell && nv_isattr(np,BLT_NOSFIO))
							sh_subtmpfile(shp);
						if(execflg && !shp->subshell &&
							!shp->st.trapcom[0] && !shp->st.trap[SH_ERRTRAP] && shp->fn_depth==0 && !nv_isattr(np,BLT_ENV))
						{
							/* do close-on-exec */
							int fd;
							for(fd=0; fd < shp->gd->lim.open_max; fd++)
								if((shp->fdstatus[fd]&IOCLEX)&&fd!=shp->infd && (fd!=shp->pwdfd))
									sh_close(fd);
						}
						if(argn)
							shp->exitval = (*shp->bltinfun)(argn,com,(void*)bp);
						if(error_info.flags&ERROR_INTERACTIVE)
							tty_check(ERRIO);
						((Shnode_t*)t)->com.comstate = shp->bltindata.data;
						bp->data = (void*)save_data;
						if(shp->exitval && errno==EINTR && shp->lastsig)
							shp->exitval = SH_EXITSIG|shp->lastsig;
						else if(!nv_isattr(np,BLT_EXIT) && shp->exitval!=SH_RUNPROG)
							shp->exitval &= SH_EXITMASK;
					}
					else
					{
						struct openlist *item;
						for(item=buffp->olist;item;item=item->next)
						{
							if(item->strm)
							{
								sfclrlock(item->strm);
								if(shp->gd->hist_ptr && item->strm == shp->gd->hist_ptr->histfp)
									hist_close(shp->gd->hist_ptr);
								else
									sfclose(item->strm);
							}
						}
						if(shp->bltinfun && (error_info.flags&ERROR_NOTIFY))
							(*shp->bltinfun)(-2,com,(void*)bp);
						/* failure on special built-ins fatal */
						if(jmpval<=SH_JMPCMD  && (!nv_isattr(np,BLT_SPC) || command))
							jmpval=0;
					}
					if(np!=SYSEXEC && shp->vex->cur)
#if 1
						spawnvex_apply(shp->vex, 0, SPAWN_RESET|SPAWN_FRAME);
#else
					{
						int fd;
						spawnvex_apply(shp->vex, 0, SPAWN_RESET|SPAWN_FRAME);
						if(shp->comsub && (fd=sffileno(sfstdout))!=1 && fd>=0)
							spawnvex_add(shp->vex,fd,1,0,0);
					}
#endif
					if(bp)
					{
						bp->bnode = 0;
						if( bp->ptr!= nv_context(np))
							np->nvfun = (Namfun_t*)bp->ptr;
					}
					if(execflg && !was_nofork)
						sh_offstate(shp,SH_NOFORK);
					if(!(nv_isattr(np,BLT_ENV)))
					{
#ifdef O_SEARCH
						while((fchdir(shp->pwdfd) < 0) && errno==EINTR)
							errno = 0;
#else
						if(shp->pwd || (shp->pwdfd >= 0))
						{
							struct stat stata;
							stat(e_dot,&stata);
							/* restore directory changed */
							if(statb.st_ino!=stata.st_ino || statb.st_dev!=stata.st_dev)
							{
								/* chdir for directories on HSM/tapeworms may take minutes */
								int err=errno;
								if(shp->pwdfd >= 0)
								{
									 while((fchdir(shp->pwdfd) < 0) && errno==EINTR)
										errno = err;
								}
								else
								{
									 while((chdir(shp->pwd) < 0) && errno==EINTR)
										errno = err;
								}
							}
						}
#endif /* O_SEARCH */
						sh_offstate(shp,SH_STOPOK);
						if(share&SF_SHARE)
							sfset(sfstdin,SF_PUBLIC|SF_SHARE,1);
						sfset(sfstderr,SF_LINE,0);
						sfpool(sfstderr,shp->outpool,SF_WRITE);
						sfpool(sfstdin,NIL(Sfio_t*),SF_WRITE);
						shp->nextprompt = save_prompt;
					}
					sh_popcontext(shp,buffp);
					errorpop(&buffp->err);
					error_info.flags &= ~(ERROR_SILENT|ERROR_NOTIFY);
					shp->bltinfun = 0;
					if(buffp->olist)
						free_list(buffp->olist);
					if(was_vi)
						sh_onoption(shp,SH_VI);
					else if(was_emacs)
						sh_onoption(shp,SH_EMACS);
					else if(was_gmacs)
						sh_onoption(shp,SH_GMACS);
					if(scope)
						sh_unscope(shp);
					bp->ptr = (void*)save_ptr;
					bp->data = (void*)save_data;
					/* don't restore for subshell exec */
					if((shp->topfd>topfd) && !(shp->subshell && np==SYSEXEC))
						sh_iorestore(shp,topfd,jmpval);
					if(shp->vexp->cur>vexi)
						sh_vexrestore(shp,vexi);
					shp->redir0 = 0;
					if(jmpval)
						siglongjmp(*shp->jmplist,jmpval);
#if 0
					if(flgs&NV_STATIC)
						((Shnode_t*)t)->com.comset = 0;
#endif
					if(shp->exitval >=0)
						goto setexit;
					np = 0;
					type=0;
				}
				/* check for functions */
				if(!command && np && nv_isattr(np,NV_FUNCTION))
				{
					volatile int indx;
					int jmpval=0;
					struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
#if SHOPT_NAMESPACE
					Namval_t node, *namespace=0;
#else
					Namval_t node;
#endif /* SHOPT_NAMESPACE */
					struct Namref	nr;
					long		mode;
					register struct slnod *slp;
					if(!np->nvalue.ip)
					{
						indx = path_search(shp,com0,NIL(Pathcomp_t**),0);
						if(indx==1)
						{
#if SHOPT_NAMESPACE
							if(shp->namespace)
								np = sh_fsearch(shp,com0,0);
							else
#endif /* SHOPT_NAMESPACE */
							np = nv_search(com0,shp->fun_tree,HASH_NOSCOPE);
						}
						
						if(!np->nvalue.ip)
						{
							if(indx==1)
							{
								errormsg(SH_DICT,ERROR_exit(0),e_defined,com0);
								shp->exitval = ERROR_NOEXEC;
							}
							else
							{
								errormsg(SH_DICT,ERROR_exit(0),e_found,"function");
								shp->exitval = ERROR_NOENT;
							}
							goto setexit;
						}
					}
					/* increase refcnt for unset */
					slp = (struct slnod*)np->nvenv;
					sh_funstaks(slp->slchild,1);
					stklink(slp->slptr);
					if(nq)
					{
						Namval_t *mp=0;
						if(nv_isattr(np,NV_STATICF) && (mp=nv_type(nq)))
							nq = mp;
						shp->last_table = last_table;
						mode = set_instance(shp,nq,&node,&nr);
					}
					if(io)
					{
						indx = shp->topfd;
						sh_pushcontext(shp,buffp,SH_JMPCMD);
						jmpval = sigsetjmp(buffp->buff,0);
					}
					if(jmpval == 0)

					{
						if(io)
							indx = sh_redirect(shp,io,execflg|IOUSEVEX);
#if SHOPT_NAMESPACE
						if(*np->nvname=='.')
						{
							char *ep;
							bool type=0;
							cp = np->nvname+1;
							if(memcmp(cp,"sh.type.",8)==0)
							{
								cp += 8;
								type = true;
							}
							if(ep = strrchr(cp,'.'))
							{
								if(type)
									while(--ep>cp && *ep!='.');
								*ep = 0;
								namespace = nv_search(cp-1,shp->var_base,HASH_NOSCOPE);
								*ep = '.';
							}
						}
						namespace = enter_namespace(shp,namespace);
#endif /* SHOPT_NAMESPACE */
						sh_funct(shp,np,argn,com,t->com.comset,(flags&~OPTIMIZE_FLAG));
					}
#if SHOPT_NAMESPACE
					enter_namespace(shp,namespace);
#endif /* SHOPT_NAMESPACE */
					spawnvex_apply(shp->vex, 0, SPAWN_RESET|SPAWN_FRAME);
					if(shp->vexp->cur>vexi)
						sh_vexrestore(shp,vexi);
					if(io)
					{
						if(buffp->olist)
							free_list(buffp->olist);
						sh_popcontext(shp,buffp);
						sh_iorestore(shp,indx,jmpval);
					}
					if(nq)
						unset_instance(nq,&node,&nr,mode);
					sh_funstaks(slp->slchild,-1);
					stkclose(slp->slptr);
					if(jmpval>SH_JMPFUN || (io && jmpval>SH_JMPIO))
						siglongjmp(*shp->jmplist,jmpval);
					goto setexit;
				}
			}
			else if(!io)
			{
				spawnvex_apply(shp->vex, 0, SPAWN_RESET|SPAWN_FRAME);
			setexit:
				exitset(shp);
				break;
			}
		    }
		    case TFORK:
		    {
			register pid_t parent;
			int no_fork,jobid;
			int pipes[3];
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				sh_exec(shp,t->fork.forktre,0);
				break;
			}
#endif /* SHOPT_COSHELL */
			if(shp->subshell)
			{
				sh_subtmpfile(shp);
				if((type&(FAMP|TFORK))==(FAMP|TFORK))
				{
					if(shp->comsub && !(shp->fdstatus[1]&IONOSEEK))
					{
						unpipe = iousepipe(shp);
						sh_subfork();
					}
				}
			}
			no_fork = !ntflag && !(type&(FAMP|FPOU)) && !shp->subshell &&
			    !(shp->st.trapcom[SIGINT] && *shp->st.trapcom[SIGINT]) &&
			    !shp->st.trapcom[0] && !shp->st.trap[SH_ERRTRAP] &&
				((struct checkpt*)shp->jmplist)->mode!=SH_JMPEVAL &&
				(execflg2 || (execflg && shp->fn_depth==0 &&
				!(pipejob && sh_isoption(shp,SH_PIPEFAIL))
			    ));
			if(sh_isstate(shp,SH_PROFILE) || shp->dot_depth)
			{
				/* disable foreground job monitor */
				if(!(type&FAMP))
					sh_offstate(shp,SH_MONITOR);
#if SHOPT_DEVFD
				else if(!(type&FINT))
					sh_offstate(shp,SH_MONITOR);
#endif /* SHOPT_DEVFD */
			}
			if(no_fork)
				job.parent=parent=0;
			else
			{
				if(((type&(FAMP|FINT))==(FAMP|FINT)) && (job.maxjob=nv_getnum(JOBMAXNOD))>0)
				{
					while(job.numbjob >= job.maxjob)
					{
						job_lock();
						job_reap(0);
						job_unlock();
					}
				}
				nv_getval(RANDNOD);
				restorefd = shp->topfd;
#ifdef SPAWN_cwd
				restorevex = shp->vexp->cur;
#endif
				if(type&FCOOP)
				{
					pipes[2] = 0;
#if SHOPT_COSHELL
					if(shp->coshell)
					{
						if(shp->cpipe[0]<0 || shp->cpipe[1] < 0)
						{
							sh_copipe(shp,shp->outpipe=shp->cpipe,0);
							shp->fdptrs[shp->cpipe[0]] = shp->cpipe;
						}
						sh_copipe(shp,shp->inpipe=pipes,0);
						parent = sh_coexec(shp,t,3);
						shp->cpid = parent;
						jobid = job_post(shp,parent,0);
						goto skip;
					}
#endif /* SHOPT_COSHELL */
					coproc_init(shp,pipes);
				}
#if SHOPT_COSHELL
				if((type&(FAMP|FINT)) == (FAMP|FINT))
				{
					if(shp->coshell)
					{
						parent = sh_coexec(shp,t,0);
						jobid = job_post(shp,parent,0);
						goto skip;
					}
				}
#endif /* SHOPT_COSHELL */
#if SHOPT_AMP
				if((type&(FAMP|FINT)) == (FAMP|FINT))
					parent = sh_ntfork(shp,t,com,&jobid,ntflag);
				else
					parent = sh_fork(shp,type,&jobid);
				if(parent<0)
				{
					if(shp->comsub==1 && usepipe && unpipe)
						sh_iounpipe(shp);
					break;
				}
#else
#if SHOPT_SPAWN
#   ifdef _lib_fork
				if(com)
					parent = sh_ntfork(shp,t,com,&jobid,ntflag);
				else
					parent = sh_fork(shp,type,&jobid);
#   else
				if((parent = sh_ntfork(shp,t,com,&jobid,ntflag))<=0)
					break;
#   endif /* _lib_fork */
				if(parent<0)
				{
					if(shp->comsub==1 && usepipe && unpipe)
						sh_iounpipe(shp);
					break;
				}
#else
				parent = sh_fork(shp,type,&jobid);
#endif /* SHOPT_SPAWN */
#endif
			}
#if SHOPT_COSHELL
		skip:
#endif /* SHOPT_COSHELL */
			if(job.parent=parent)
			/* This is the parent branch of fork
			 * It may or may not wait for the child
			 */
			{
				if(pipejob==2)
				{
					pipejob = 1;
					nlock--;
					job_unlock();
				}
				if(shp->subshell)
					shp->spid = parent;
				if(type&FPCL)
					sh_close(shp->inpipe[0]);
				if(type&(FCOOP|FAMP))
					shp->bckpid = parent;
				else if(!(type&(FAMP|FPOU)))
				{
					if(!sh_isoption(shp,SH_MONITOR))
					{
						if(!(shp->sigflag[SIGINT]&(SH_SIGFAULT|SH_SIGOFF)))
							sh_sigtrap(shp,SIGINT);
						shp->trapnote |= SH_SIGIGNORE;
					}
					if(shp->pipepid)
						shp->pipepid = parent;
					else
					{
						job_wait(parent);
						if(parent==shp->spid)
							shp->spid = 0;
					}
					if(shp->topfd > topfd)
						sh_iorestore(shp,topfd,0);
#ifdef SPAWN_cwd
					if(shp->vexp->cur > vexi)
						sh_vexrestore(shp,vexi);
#endif
					if(usepipe && tsetio &&  subdup)
						sh_iounpipe(shp);
					if(!sh_isoption(shp,SH_MONITOR))
					{
						shp->trapnote &= ~SH_SIGIGNORE;
						if(shp->exitval == (SH_EXITSIG|SIGINT))
							kill(getpid(),SIGINT);
					}
				}
				if(type&FAMP)
				{
					if(sh_isstate(shp,SH_PROFILE) || sh_isstate(shp,SH_INTERACTIVE))
					{
						/* print job number */
#ifdef JOBS
#   if SHOPT_COSHELL
						sfprintf(sfstderr,"[%d]\t%s\n",jobid,sh_pid2str(shp,parent));
#   else
						sfprintf(sfstderr,"[%d]\t%d\n",jobid,parent);
#   endif /* SHOPT_COSHELL */
#else
						sfprintf(sfstderr,"%d\n",parent);
#endif /* JOBS */
					}
				}
				break;
			}
			else
			/*
			 * this is the FORKED branch (child) of execute
			 */
			{
				volatile int jmpval;
				struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
				struct ionod *iop;
				int	rewrite=0;
				if(no_fork)
					sh_sigreset(shp,2);
				sh_pushcontext(shp,buffp,SH_JMPEXIT);
				jmpval = sigsetjmp(buffp->buff,0);
				if(jmpval)
					goto done;
				if((type&FINT) && !sh_isstate(shp,SH_MONITOR))
				{
					/* default std input for & */
					signal(SIGINT,SIG_IGN);
					signal(SIGQUIT,SIG_IGN);
					shp->sigflag[SIGINT] = SH_SIGOFF;
					shp->sigflag[SIGQUIT] = SH_SIGOFF;
					if(!shp->st.ioset)
					{
						if(sh_close(0)>=0)
							sh_open(e_devnull,O_RDONLY,0);
					}
				}
				sh_offstate(shp,SH_MONITOR);
				/* pipe in or out */
#ifdef _lib_nice
				if((type&FAMP) && sh_isoption(shp,SH_BGNICE))
					nice(4);
#endif /* _lib_nice */
#if !SHOPT_DEVFD
				if(shp->fifo && (type&(FPIN|FPOU)))
				{
					int	fn,fd = (type&FPIN)?0:1;
					void	*fifo_timer=sh_timeradd(500,1,fifo_check,(void*)shp);
					fn = sh_open(shp->fifo,fd?O_WRONLY:O_RDONLY);
					timerdel(fifo_timer);
					sh_iorenumber(shp,fn,fd);
					sh_close(fn);
					sh_delay(.001);
					unlink(shp->fifo);
					free(shp->fifo);
					shp->fifo = 0;
					type &= ~(FPIN|FPOU);
				}
#endif /* !SHOPT_DEVFD */
				if(type&FPIN)
				{
#if SHOPT_COSHELL
					if(shp->inpipe[2]>20000)
						sh_coaccept(shp,shp->inpipe,0);
#endif /* SHOPT_COSHELL */
					sh_iorenumber(shp,shp->inpipe[0],0);
					if(!(type&FPOU) || (type&FCOOP))
						sh_close(shp->inpipe[1]);
				}
				if(type&FPOU)
				{
#if SHOPT_COSHELL
					if(shp->outpipe[2]>20000)
						sh_coaccept(shp,shp->outpipe,1);
#endif /* SHOPT_COSHELL */
					sh_iorenumber(shp,shp->outpipe[1],1);
					sh_pclose(shp->outpipe);
				}
				if((type&COMMSK)!=TCOM)
					error_info.line = t->fork.forkline-shp->st.firstline;
				if(shp->topfd)
					sh_iounsave(shp);
				topfd = shp->topfd;
				if(com0 && (iop=t->tre.treio))
				{
					for(;iop;iop=iop->ionxt)
					{
						if(iop->iofile&IOREWRITE)
							rewrite = 1;
					}
				}
				sh_redirect(shp,t->tre.treio,1|IOUSEVEX);
				if(rewrite)
				{
					job_lock();
					while((parent = vfork()) < 0)
						_sh_fork(shp,parent, 0, (int*)0);
					if(parent)
					{
						job.toclear = 0;
						job_post(shp,parent,0);
						job_wait(parent);
						sh_iorestore(shp,topfd,SH_JMPCMD);
						if(shp->vexp->cur>vexi)
							sh_vexrestore(shp,vexi);
						sh_done(shp,(shp->exitval&SH_EXITSIG)?(shp->exitval&SH_EXITMASK):0);

					}
					job_unlock();
				}
				if((type&COMMSK)!=TCOM)
				{
					/* don't clear job table for out
					   pipes so that jobs comand can
					   be used in a pipeline
					 */
					if(!no_fork && !(type&FPOU))
						job_clear(shp);
					sh_exec(shp,t->fork.forktre,flags|sh_state(SH_NOFORK)|sh_state(SH_FORKED));
				}
				else if(com0)
				{
					sh_offoption(shp,SH_ERREXIT);
					sh_freeup(shp);
					path_exec(shp,com0,com,t->com.comset);
				}
			done:
				sh_popcontext(shp,buffp);
				if(jmpval>SH_JMPEXIT)
					siglongjmp(*shp->jmplist,jmpval);
				sh_done(shp,0);
			}
		    }

		    case TSETIO:
		    {
		    /*
		     * don't create a new process, just
		     * save and restore io-streams
		     */
			pid_t	pid;
			int 	jmpval, waitall;
			int 	simple = (t->fork.forktre->tre.tretyp&COMMSK)==TCOM;
			struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				sh_redirect(shp,t->fork.forkio,0);
				sh_exec(shp,t->fork.forktre,0);
				break;
			}
#endif /*SHOPT_COSHELL */
			if(shp->subshell)
				execflg = 0;
			sh_pushcontext(shp,buffp,SH_JMPIO);
			if(type&FPIN)
			{
				was_interactive = sh_isstate(shp,SH_INTERACTIVE);
				sh_offstate(shp,SH_INTERACTIVE);
				shp->pipepid = simple;
#if 0
				sh_vexsave(shp,0,shp->inpipe[0],0,0);
#else
				sh_iosave(shp,0,shp->topfd,(char*)0);
				sh_iorenumber(shp,shp->inpipe[0],0);
#endif
				/*
				 * if read end of pipe is a simple command
				 * treat as non-sharable to improve performance
				 */
				if(simple)
					sfset(sfstdin,SF_PUBLIC|SF_SHARE,0);
				waitall = job.waitall;
				job.waitall = 0;
				pid = job.parent;
			}
			else
				error_info.line = t->fork.forkline-shp->st.firstline;
			jmpval = sigsetjmp(buffp->buff,0);
			if(jmpval==0)
			{
				if(shp->comsub)
					tsetio = 1;
				sh_redirect(shp,t->fork.forkio,execflg);
				(t->fork.forktre)->tre.tretyp |= t->tre.tretyp&FSHOWME;
				t = t->fork.forktre;
				if((t->tre.tretyp&COMMSK)==TCOM && sh_isoption(
shp,SH_BASH) && !sh_isoption(shp,SH_LASTPIPE))
				{

					Shnode_t *tt = (Shnode_t*)stkalloc(shp->stk,sizeof(Shnode_t));
					tt->par.partyp = type = TPAR;
					tt->par.partre = (Shnode_t*)t;
					t = tt;
				}
				sh_exec(shp,t,flags&~simple);
			}
			else
				sfsync(shp->outpool);
			sh_popcontext(shp,buffp);
			sh_iorestore(shp,buffp->topfd,jmpval);
			if(shp->vexp->cur>vexi)
				sh_vexrestore(shp,buffp->vexi);
			if(buffp->olist)
				free_list(buffp->olist);
			if(type&FPIN)
			{
				job.waitall = waitall;
				type = shp->exitval;
				if(!(type&SH_EXITSIG))
				{
					/* wait for remainder of pipline */
					if(shp->pipepid>1 && shp->comsub!=1)
					{
						job_wait(shp->pipepid);
						type = shp->exitval;
					}
					else
						job_wait(waitall?pid:0);
					if(type || !sh_isoption(shp,SH_PIPEFAIL))
						shp->exitval = type;
				}
				if(shp->comsub==1 && usepipe && unpipe)
					sh_iounpipe(shp);
				shp->pipepid = 0;
				shp->st.ioset = 0;
				if(simple && was_errexit)
				{
					echeck = 1;
					sh_onstate(shp,SH_ERREXIT);
				}
			}
			if(jmpval>SH_JMPIO)
				siglongjmp(*shp->jmplist,jmpval);
			break;
		    }

		    case TPAR:
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				sh_exec(shp,t->par.partre,0);
				break;
			}
#endif /* SHOPT_COSHELL */
			echeck = 1;
			flags &= ~OPTIMIZE_FLAG;
			if(!shp->subshell && !shp->st.trapcom[0] && !shp->st.trap[SH_ERRTRAP] && (flags&sh_state(SH_NOFORK)))
			{
				char *savsig;
				int nsig,jmpval;
				struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
				shp->st.otrapcom = 0;
				if((nsig=shp->st.trapmax*sizeof(char*))>0 || shp->st.trapcom[0])
				{
					nsig += sizeof(char*);
					memcpy(savsig=malloc(nsig),(char*)&shp->st.trapcom[0],nsig);
					shp->st.otrapcom = (char**)savsig;
				}
				sh_sigreset(shp,0);
				sh_pushcontext(shp,buffp,SH_JMPEXIT);
				jmpval = sigsetjmp(buffp->buff,0);
				if(jmpval==0)
					sh_exec(shp,t->par.partre,flags);
				sh_popcontext(shp,buffp);
				if(jmpval > SH_JMPEXIT)
					siglongjmp(*shp->jmplist,jmpval);
				if(shp->exitval > 256)
					shp->exitval -= 128;
				sh_done(shp,0);
			}
			else if(((type=t->par.partre->tre.tretyp)&FAMP) && ((type&COMMSK)==TFORK))
			{
				pid_t	pid;
				sfsync(NIL(Sfio_t*));
				while((pid=fork())< 0)
					_sh_fork(shp,pid,0,0);
				if(pid==0)
				{
					sh_exec(shp,t->par.partre,flags);
					shp->st.trapcom[0]=0;
					sh_done(shp,0);
				}
			}
			else
				sh_subshell(shp,t->par.partre,flags,0);
			break;

		    case TFIL:
		    {
		    /*
		     * This code sets up a pipe.
		     * All elements of the pipe are started by the parent.
		     * The last element executes in current environment
		     */
			int	pvo[3];	/* old pipe for multi-stage */
			int	pvn[3];	/* current set up pipe */
			int	savepipe = pipejob;
			int	savelock = nlock;
			int	showme = t->tre.tretyp&FSHOWME;
			int	n,waitall,savewaitall=job.waitall;
			int	savejobid = job.curjobid;
			int	*exitval=0,*saveexitval = job.exitval;
			pid_t	savepgid = job.curpgid;
#if SHOPT_COSHELL
			int	copipe=0;
			Shnode_t	*tt;
#endif /* SHOPT_COSHELL */
			job.exitval = 0;
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				do
				{
					sh_exec(shp,t->lst.lstlef, 0);
					t = t->lst.lstrit;
					if(flags && (t->tre.tretyp!=TFIL || !(t->lst.lstlef->tre.tretyp&FALTPIPE)))
						goto coskip1;
				}
				while(t->tre.tretyp==TFIL);
				sh_exec(shp,t,0);
			coskip1:
				break;
			}
			pvo[2] = pvn[2] = 0;
#endif /* SHOPT_COSHELL */
			job.curjobid = 0;
			if(shp->subshell)
			{
				sh_subtmpfile(shp);
				if(shp->comsub==1 && !(shp->fdstatus[1]&IONOSEEK))
					iousepipe(shp);
			}
			shp->inpipe = pvo;
			shp->outpipe = pvn;
			pvo[1] = -1;
			if(sh_isoption(shp,SH_PIPEFAIL))
			{
				const Shnode_t* tn=t;
				job.waitall = 2;
				job.curpgid = 0;
				while((tn=tn->lst.lstrit) && tn->tre.tretyp==TFIL)
					job.waitall++;
				exitval = job.exitval = (int*)stkalloc(shp->stk,job.waitall*sizeof(int));
				memset(exitval,0,job.waitall*sizeof(int));
			}
			else
				job.waitall |= !pipejob && sh_isstate(shp,SH_MONITOR);
			job_lock();
			nlock++;
			do
			{
				/* create the pipe */
#if SHOPT_COSHELL
				tt = t->lst.lstrit;
				if(shp->coshell && !showme)
				{
					if(t->lst.lstlef->tre.tretyp&FALTPIPE)
					{
						sh_copipe(shp,pvn,0);
						type = sh_coexec(shp,t,1+copipe);
						pvn[1] = -1;
						pipejob=1;
						if(type>0)
						{
							job_post(shp,type,0);
							type = 0;
						}
						copipe = 1;
						pvo[0] = pvn[0];
						while(tt->tre.tretyp==TFIL && tt->lst.lstlef->tre.tretyp&FALTPIPE)
							tt = tt->lst.lstrit;
						t = tt;
						continue;
					}
					else if(tt->tre.tretyp==TFIL && tt->lst.lstlef->tre.tretyp&FALTPIPE)
					{
						sh_copipe(shp,pvn,0);
						pvo[2] = pvn[2];
						copipe = 0;
						goto coskip2;
					}
				}
#endif /* SHOPT_COSHELL */
				sh_pipe(pvn);
#if SHOPT_COSHELL
				pvn[2] = 0;
			coskip2:
#endif /* SHOPT_COSHELL */
				/* execute out part of pipe no wait */
				(t->lst.lstlef)->tre.tretyp |= showme;
				type = sh_exec(shp,t->lst.lstlef, errorflg);
				/* close out-part of pipe */
				sh_close(pvn[1]);
				pipejob=1;
				/* save the pipe stream-ids */
				pvo[0] = pvn[0];
				/* pipeline all in one process group */
				t = t->lst.lstrit;
			}
			/* repeat until end of pipeline */
			while(!type && t->tre.tretyp==TFIL);
			shp->inpipe = pvn;
			shp->outpipe = 0;
			pipejob = 2;
			waitall = job.waitall;
			job.waitall = 0;
			if(type == 0)
			{
				/*
				 * execute last element of pipeline
				 * in the current process
				 */
				((Shnode_t*)t)->tre.tretyp |= showme;
				sh_exec(shp,t,flags);
			}
			else
				/* execution failure, close pipe */
				sh_pclose(pvn);
			if(pipejob==2)
				job_unlock();
			if((pipejob = savepipe) && nlock<savelock)
				pipejob = 1;
			n = shp->exitval;
			if(job.waitall = waitall)
			{
				if(sh_isstate(shp,SH_MONITOR))
					job_wait(0);
				else
				{
					shp->intrap++;
					job_wait(0);
					shp->intrap--;
				}
			}
			if(n==0 && exitval)
			{
				while(exitval <= --job.exitval)
				{
					if(*job.exitval)
					{
						n = *job.exitval;
						break;
					}
				}
			}
			shp->exitval = n;
#ifdef SIGTSTP
			if(!pipejob && sh_isstate(shp,SH_MONITOR) && sh_isoption(shp,SH_INTERACTIVE))
				tcsetpgrp(JOBTTY,shp->gd->pid);
#endif /*SIGTSTP */
			job.curpgid = savepgid;
			job.exitval = saveexitval;
			job.waitall = savewaitall;
			job.curjobid = savejobid;
			break;
		    }

		    case TLST:
		    {
			/*  a list of commands are executed here */
			do
			{
				sh_exec(shp,t->lst.lstlef,errorflg|OPTIMIZE);
				t = t->lst.lstrit;
			}
			while(t->tre.tretyp == TLST);
			sh_exec(shp,t,flags);
			break;
		    }

		    case TAND:
#if SHOPT_COSHELL
			if(shp->inpool)
			{
			andor:
				sh_exec(shp,t->lst.lstlef,0);
				sh_exec(shp,t->lst.lstrit,0);
				break;
			}
#endif /* SHOPT_COSHELL */
			if(type&TTEST)
				skipexitset++;
			if(sh_exec(shp,t->lst.lstlef,OPTIMIZE)==0)
				sh_exec(shp,t->lst.lstrit,flags);
			break;

		    case TORF:
#if SHOPT_COSHELL
			if(shp->inpool)
				goto andor;
#endif /* SHOPT_COSHELL */
			if(type&TTEST)
				skipexitset++;
			if(sh_exec(shp,t->lst.lstlef,OPTIMIZE)!=0)
				sh_exec(shp,t->lst.lstrit,flags);
			break;

		    case TFOR: /* for and select */
		    {
			register char **args;
			register int nargs;
			register Namval_t *np;
			int flag = errorflg|OPTIMIZE_FLAG;
			struct dolnod	*argsav=0;
			struct comnod	*tp;
			char *trap, *nullptr = 0;
			int nameref, refresh=1;
			char *av[5];
#if SHOPT_COSHELL
			int poolfiles;
#endif /* SHOPT_COSHELL */
#if SHOPT_OPTIMIZE
			int  jmpval = ((struct checkpt*)shp->jmplist)->mode;
			struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
			void *optlist = shp->optlist;
			shp->optlist = 0;
			sh_tclear(shp,t->for_.fortre);
			sh_pushcontext(shp,buffp,jmpval);
			jmpval = sigsetjmp(buffp->buff,0);
			if(jmpval)
				goto endfor;
#endif /* SHOPT_OPTIMIZE */
			error_info.line = t->for_.forline-shp->st.firstline;
			if(!(tp=t->for_.forlst))
			{
				args=shp->st.dolv+1;
				nargs = shp->st.dolc;
				argsav=sh_arguse(shp);
			}
			else
			{
				args=sh_argbuild(shp,&argn,tp,0);
				nargs = argn;
			}
			np = nv_open(t->for_.fornam, shp->var_tree,NV_NOASSIGN|NV_NOARRAY|NV_VARNAME|NV_NOREF);
			nameref = nv_isref(np)!=0;
			shp->st.loopcnt++;
			cp = *args;
			while(cp && shp->st.execbrk==0)
			{
				if(t->tre.tretyp&COMSCAN)
				{
					char *val;
					int save_prompt;
					/* reuse register */
					if(refresh)
					{
						sh_menu(shp,sfstderr,nargs,args);
						refresh = 0;
					}
					save_prompt = shp->nextprompt;
					shp->nextprompt = 3;
					shp->timeout = 0;
					shp->exitval=sh_readline(shp,&nullptr,(void*)0,0,1,(size_t)0,1000*shp->st.tmout);
					shp->nextprompt = save_prompt;
					if(shp->exitval||sfeof(sfstdin)||sferror(sfstdin))
					{
						shp->exitval = 1;
						break;
					}
					if(!(val=nv_getval(sh_scoped(shp,REPLYNOD))))
						continue;
					else
					{
						if(*(cp=val) == 0)
						{
							refresh++;
							goto check;
						}
						while(type = *cp++)
							if(type < '0' && type > '9')
								break;
						if(type!=0)
							type = nargs;
						else
							type = (int)strtol(val, (char**)0, 10)-1;
						if(type<0 || type >= nargs)
							cp = "";
						else
							cp = args[type];
					}
				}
				if(nameref)
					nv_offattr(np,NV_REF|NV_TABLE);
				else if(nv_isattr(np, NV_ARRAY))
					nv_putsub(np,NIL(char*),0L,0);
				nv_putval(np,cp,0);
				if(nameref)
				{
					nv_setref(np,(Dt_t*)0,NV_VARNAME);
					nv_onattr(np,NV_TABLE);
				}
				if(trap=shp->st.trap[SH_DEBUGTRAP])
				{
					av[0] = (t->tre.tretyp&COMSCAN)?"select":"for";
					av[1] = t->for_.fornam;
					av[2] = "in";
					av[3] = cp;
					av[4] = 0;
					sh_debug(shp,trap,(char*)0,(char*)0,av,0);
				}
#if SHOPT_COSHELL
				if(shp->inpool)
				{
					poolfiles = shp->poolfiles;
					sh_exec(shp,t->for_.fortre,0);
					if(poolfiles==shp->poolfiles)
						break;
				}
#endif /* SHOPT_COSHELL */
				sh_exec(shp,t->for_.fortre,flag);
				flag &= ~OPTIMIZE_FLAG;
				if(t->tre.tretyp&COMSCAN)
				{
					if((cp=nv_getval(sh_scoped(shp,REPLYNOD))) && *cp==0)
						refresh++;
				}
				else
					cp = *++args;
			check:
				if(shp->st.breakcnt<0)
					shp->st.execbrk = (++shp->st.breakcnt !=0);
			}
			if(nameref)
				nv_offattr(np,NV_TABLE);
#if SHOPT_OPTIMIZE
		endfor:
			sh_popcontext(shp,buffp);
			sh_tclear(shp,t->for_.fortre);
			sh_optclear(shp,optlist);
			if(jmpval)
				siglongjmp(*shp->jmplist,jmpval);
#endif /*SHOPT_OPTIMIZE */
			if(shp->st.breakcnt>0)
				shp->st.execbrk = (--shp->st.breakcnt !=0);
			shp->st.loopcnt--;
			sh_argfree(shp,argsav,0);
			nv_close(np);
			break;
		    }

		    case TWH: /* while and until */
		    {
			volatile int 	r=0;
			int first = OPTIMIZE_FLAG;
			Shnode_t *tt = t->wh.whtre;
#if SHOPT_FILESCAN
			Sfio_t *iop=0;
			int savein;
#endif /*SHOPT_FILESCAN*/
#if SHOPT_OPTIMIZE
			int  jmpval = ((struct checkpt*)shp->jmplist)->mode;
			struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
			void *optlist = shp->optlist;
#endif /* SHOPT_OPTIMIZE */
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				int poolfiles;
#   if SHOPT_FILESCAN
				if(type==TWH && tt->tre.tretyp==TCOM && !tt->com.comarg && tt->com.comio)
				{
					sh_redirect(shp,tt->com.comio,0);
					break;
				}
#   endif /* SHOPT_FILESCAN */
				sh_exec(shp,tt,0);
				do
				{
					if((sh_exec(shp,tt,0)==0)!=(type==TWH))
						break;
					poolfiles = shp->poolfiles;
					sh_exec(shp,t->wh.dotre,0);
					if(t->wh.whinc)
						sh_exec(shp,(Shnode_t*)t->wh.whinc,0);
				}
				while(poolfiles != shp->poolfiles);
				break;
			}
#endif /*SHOPT_COSHELL */
#if SHOPT_OPTIMIZE
			shp->optlist = 0;
			sh_tclear(shp,t->wh.whtre);
			sh_tclear(shp,t->wh.dotre);
			sh_pushcontext(shp,buffp,jmpval);
			jmpval = sigsetjmp(buffp->buff,0);
			if(jmpval)
				goto endwhile;
#endif /* SHOPT_OPTIMIZE */
#if SHOPT_FILESCAN
			if(type==TWH && tt->tre.tretyp==TCOM && !tt->com.comarg && tt->com.comio)
			{
				iop = openstream(shp,tt->com.comio,&savein);
				if(tt->com.comset)
					sh_setlist(shp,tt->com.comset,NV_IDENT|NV_ASSIGN,0);
			}
#endif /*SHOPT_FILESCAN */
			shp->st.loopcnt++;
			while(shp->st.execbrk==0)
			{
#if SHOPT_FILESCAN
				if(iop)
				{
					if(!(shp->cur_line=sfgetr(iop,'\n',SF_STRING)))
						break;
				}
				else
#endif /*SHOPT_FILESCAN */
				if((sh_exec(shp,tt,first)==0)!=(type==TWH))
					break;
				r = sh_exec(shp,t->wh.dotre,first|errorflg);
				if(shp->st.breakcnt<0)
					shp->st.execbrk = (++shp->st.breakcnt !=0);
				/* This is for the arithmetic for */
				if(shp->st.execbrk==0 && t->wh.whinc)
					sh_exec(shp,(Shnode_t*)t->wh.whinc,first);
				first = 0;
				errorflg &= ~OPTIMIZE_FLAG;
#if SHOPT_FILESCAN
				shp->offsets[0] = -1;
				shp->offsets[1] = 0;
#endif /*SHOPT_FILESCAN */
			}
#if SHOPT_OPTIMIZE
		endwhile:
			sh_popcontext(shp,buffp);
			sh_tclear(shp,t->wh.whtre);
			sh_tclear(shp,t->wh.dotre);
			sh_optclear(shp,optlist);
			if(jmpval)
				siglongjmp(*shp->jmplist,jmpval);
#endif /*SHOPT_OPTIMIZE */
			if(shp->st.breakcnt>0)
				shp->st.execbrk = (--shp->st.breakcnt !=0);
			shp->st.loopcnt--;
			shp->exitval= r;
#if SHOPT_FILESCAN
			if(iop)
			{
				int err=errno;
				sfclose(iop);
				while(close(0)<0 && errno==EINTR)
					errno = err;
				dup(savein);
				shp->cur_line = 0;
			}
#endif /*SHOPT_FILESCAN */
			break;
		    }
		    case TARITH: /* (( expression )) */
		    {
			register char *trap;
			char *arg[4];
			error_info.line = t->ar.arline-shp->st.firstline;
			arg[0] = "((";
			if(!(t->ar.arexpr->argflag&ARG_RAW))
				arg[1] = sh_macpat(shp,t->ar.arexpr,OPTIMIZE|ARG_ARITH);
			else
				arg[1] = t->ar.arexpr->argval;
			arg[2] = "))";
			arg[3] = 0;
			if(trap=shp->st.trap[SH_DEBUGTRAP])
				sh_debug(shp,trap,(char*)0, (char*)0, arg, ARG_ARITH);
			if(sh_isoption(shp,SH_XTRACE))
			{
				sh_trace(shp,NIL(char**),0);
				sfprintf(sfstderr,"((%s))\n",arg[1]);
			}
			if(t->ar.arcomp)
				shp->exitval  = !arith_exec((Arith_t*)t->ar.arcomp);
			else
				shp->exitval = !sh_arith(shp,arg[1]);
			break;
		    }

		    case TIF:
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				sh_exec(shp,t->if_.thtre,0);
				if(t->if_.eltre)
					sh_exec(shp,t->if_.eltre, 0);
				break;
			}
#endif /*SHOPT_COSHELL */
			if(sh_exec(shp,t->if_.iftre,OPTIMIZE)==0)
				sh_exec(shp,t->if_.thtre,flags);
			else if(t->if_.eltre)
				sh_exec(shp,t->if_.eltre, flags);
			else
				shp->exitval=0; /* force zero exit for if-then-fi */
			break;

		    case TSW:
		    {
			Shnode_t *tt = (Shnode_t*)t;
			char *trap, *r = sh_macpat(shp,tt->sw.swarg,OPTIMIZE);
			error_info.line = t->sw.swline-shp->st.firstline;
			t= (Shnode_t*)(tt->sw.swlst);
			if(trap=shp->st.trap[SH_DEBUGTRAP])
			{
				char *av[4];
				av[0] = "case";
				av[1] = r;
				av[2] = "in";
				av[3] = 0;
				sh_debug(shp,trap, (char*)0, (char*)0, av, 0);
			}
			while(t)
			{
				register struct argnod	*rex=(struct argnod*)t->reg.regptr;
#if SHOPT_COSHELL
				if(shp->inpool)
				{
					sh_exec(shp,t->reg.regcom,0);
					continue;
				}
#endif /*SHOPT_COSHELL */
				while(rex)
				{
					register char *s;
					if(rex->argflag&ARG_MAC)
					{
						s = sh_macpat(shp,rex,OPTIMIZE|ARG_EXP|ARG_CASE);
						while(*s=='\\' && s[1]==0)
							s+=2;
					}
					else
						s = rex->argval;
					type = (rex->argflag&ARG_RAW);
					if((type && strcmp(r,s)==0) ||
						(!type && (strmatch(r,s)
						|| trim_eq(r,s))))
					{
						do	sh_exec(shp,t->reg.regcom,(t->reg.regflag?(flags&sh_state(SH_ERREXIT)):flags));
						while(t->reg.regflag &&
							(t=(Shnode_t*)t->reg.regnxt));
						t=0;
						break;
					}
					else
						rex=rex->argnxt.ap;
				}
				if(t)
					t=(Shnode_t*)t->reg.regnxt;
			}
			break;
		    }

		    case TTIME:
		    {
			/* time the command */
			struct tms before,after;
			const char *format = e_timeformat;
			clock_t at, tm[3];
#ifdef timeofday
			struct timeval tb,ta;
#else
			clock_t bt;
#endif	/* timeofday */
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				if(t->par.partre)
					sh_exec(shp,t->par.partre,0);
				break;
			}
#endif /*SHOPT_COSHELL */
			if(type!=TTIME)
			{
				sh_exec(shp,t->par.partre,OPTIMIZE);
				shp->exitval = !shp->exitval;
				break;
			}
			if(t->par.partre)
			{
				long timer_on;
				if(shp->subshell && shp->comsub==1)
					sh_subfork();
				timer_on = sh_isstate(shp,SH_TIMING);
#ifdef timeofday
				timeofday(&tb);
				times(&before);
#else
				bt = times(&before);
#endif	/* timeofday */
				job.waitall = 1;
				sh_onstate(shp,SH_TIMING);
				sh_exec(shp,t->par.partre,OPTIMIZE);
				if(!timer_on)
					sh_offstate(shp,SH_TIMING);
				job.waitall = 0;
			}
			else
			{
#ifndef timeofday
				bt = 0;
#endif	/* timeofday */
				before.tms_utime = before.tms_cutime = 0;
				before.tms_stime = before.tms_cstime = 0;
			}
#ifdef timeofday
			times(&after);
			timeofday(&ta);
			at = shp->gd->lim.clk_tck*(ta.tv_sec-tb.tv_sec);
			at +=  ((shp->gd->lim.clk_tck*(((1000000L/2)/shp->gd->lim.clk_tck)+(ta.tv_usec-tb.tv_usec)))/1000000L);
#else
			at = times(&after) - bt;
#endif	/* timeofday */
			tm[0] = at;
			if(t->par.partre)
			{
				Namval_t *np = nv_open("TIMEFORMAT",shp->var_tree,NV_NOADD);
				if(np)
				{
					format = nv_getval(np);
					nv_close(np);
				}
				if(!format)
					format = e_timeformat;
			}
			else
				format = strchr(format+1,'\n')+1;
			tm[1] = after.tms_utime - before.tms_utime;
			tm[1] += after.tms_cutime - before.tms_cutime;
			tm[2] = after.tms_stime - before.tms_stime;
			tm[2] += after.tms_cstime - before.tms_cstime;
			if(format && *format)
				p_time(shp,sfstderr,sh_translate(format),tm);
			break;
		    }
		    case TFUN:
		    {
			register Namval_t *np=0;
			register struct slnod *slp;
			register char *fname = ((struct functnod*)t)->functnam;
			register Namval_t *npv=0,*mp;
			cp = strrchr(fname,'.');
#if SHOPT_COSHELL
			if(shp->inpool)
			{
				sh_exec(shp,t->funct.functtre,0);
				break;
			}
#endif /* SHOPT_COSHELL */
#if SHOPT_NAMESPACE
			if(t->tre.tretyp==TNSPACE)
			{
				Dt_t *root;
				Namval_t *oldnspace = shp->namespace;
				int offset = stktell(stkp);
				int	flag=NV_NOASSIGN|NV_NOARRAY|NV_VARNAME;
				char	*sp,*xp;
				sfputc(stkp,'.');
				sfputr(stkp,fname,0);
				xp=stkptr(stkp,offset);
				for(sp=xp+1;sp;)
				{
					if(sp = strchr(sp,'.'))
						*sp = 0;
					np = nv_open(xp,shp->var_tree,flag);
					if(sp)
						*sp++ = '.';
				}
				offset = stktell(stkp);
				if(nv_istable(np))
					root = nv_dict(np);
				else
				{
					root = dtopen(&_Nvdisc,Dtoset);
					dtuserdata(root,shp,1);
					nv_mount(np, (char*)0, root);
					np->nvalue.cp = Empty;
					dtview(root,shp->var_base);
				}
				oldnspace = enter_namespace(shp,np);
				sh_exec(shp,t->for_.fortre,flag|sh_state(SH_ERREXIT));
				enter_namespace(shp,oldnspace);
				break;
			}
#endif /* SHOPT_NAMESPACE */
			/* look for discipline functions */
			error_info.line = t->funct.functline-shp->st.firstline;
			/* Function names cannot be special builtin */
			if(cp || shp->prefix)
			{
				int offset = stktell(stkp);
				if(shp->prefix)
				{
					cp = shp->prefix;
					shp->prefix = 0;
					npv = nv_open(cp,shp->var_tree,NV_NOASSIGN|NV_NOARRAY|NV_VARNAME);
					shp->prefix = cp;
					cp = fname;
				}
				else
				{
					sfwrite(stkp,fname,cp++-fname);
					sfputc(stkp,0);
					npv = nv_open(stkptr(stkp,offset),shp->var_tree,NV_NOASSIGN|NV_NOARRAY|NV_VARNAME);
				}
				offset = stktell(stkp);
				sfprintf(stkp,"%s.%s%c",nv_name(npv),cp,0);
				fname = stkptr(stkp,offset);
			}
			else if((mp=nv_search(fname,shp->bltin_tree,0)) && nv_isattr(mp,BLT_SPC))
				errormsg(SH_DICT,ERROR_exit(1),e_badfun,fname);
#if SHOPT_NAMESPACE
			if(shp->namespace && !shp->prefix && *fname!='.')
				np = sh_fsearch(shp,fname,NV_ADD|HASH_NOSCOPE);
			if(!np)
#endif /* SHOPT_NAMESPACE */
			np = nv_open(fname,sh_subfuntree(shp,1),NV_NOASSIGN|NV_NOARRAY|NV_VARNAME|NV_NOSCOPE);
			if(npv)
			{
				if(!shp->mktype)
					cp = nv_setdisc(npv,cp,np,(Namfun_t*)npv);
				if(!cp)
					errormsg(SH_DICT,ERROR_exit(1),e_baddisc,fname);
			}
			if(np->nvalue.rp)
			{
				struct Ufunction *rp = np->nvalue.rp;
				slp = (struct slnod*)np->nvenv;
				sh_funstaks(slp->slchild,-1);
				stkclose(slp->slptr);
				if(rp->sdict)
				{
					Namval_t *nq;
					shp->last_root = rp->sdict;
					for(mp=(Namval_t*)dtfirst(rp->sdict);mp;mp=nq)
					{
						_nv_unset(mp,NV_RDONLY);
						nq = dtnext(rp->sdict,mp);
						nv_delete(mp,rp->sdict,0);
					}
					dtclose(rp->sdict);
					rp->sdict = 0;
				}
				if(shp->funload)
				{
					if(!shp->fpathdict)
						free((void*)np->nvalue.rp);
					np->nvalue.rp = 0;
				}
			}
			if(!np->nvalue.rp)
			{
				np->nvalue.rp = new_of(struct Ufunction,shp->funload?sizeof(Dtlink_t):0);
				memset((void*)np->nvalue.rp,0,sizeof(struct Ufunction));
			}
			if(t->funct.functstak)
			{
				static Dtdisc_t		_Rpdisc =
				{
				        offsetof(struct Ufunction,fname), -1, sizeof(struct Ufunction) 
				};
				struct functnod *fp;
				struct comnod *ac = t->funct.functargs;
				slp = t->funct.functstak;
				sh_funstaks(slp->slchild,1);
				stklink(slp->slptr);
				np->nvenv = (char*)slp;
				nv_funtree(np) = (int*)(t->funct.functtre);
				np->nvalue.rp->hoffset = t->funct.functloc;
				np->nvalue.rp->lineno = t->funct.functline;
				np->nvalue.rp->nspace = shp->namespace;
				np->nvalue.rp->fname = 0;
				np->nvalue.rp->argv = ac?((struct dolnod*)ac->comarg)->dolval+1:0;
				np->nvalue.rp->argc = ac?((struct dolnod*)ac->comarg)->dolnum:0;
				np->nvalue.rp->fdict = shp->fun_tree;
				fp = (struct functnod*)(slp+1);
				if(fp->functtyp==(TFUN|FAMP))
					np->nvalue.rp->fname = fp->functnam;
				nv_setsize(np,fp->functline);
				nv_offattr(np,NV_FPOSIX);
				if(shp->funload)
				{
					struct Ufunction *rp = np->nvalue.rp;
					rp->np = np;
					if(!shp->fpathdict)
						shp->fpathdict = dtopen(&_Rpdisc,Dtobag);
					if(shp->fpathdict)
					{
						dtuserdata(shp->fpathdict,shp,1);
						dtinsert(shp->fpathdict,rp);
					}
				}
			}
			else
				_nv_unset(np,0);
			if(type&FPOSIX)
				nv_onattr(np,NV_FUNCTION|NV_FPOSIX);
			else
				nv_onattr(np,NV_FUNCTION);
			if(type&FPIN)
				nv_onattr(np,NV_FTMP);
			if(type&FOPTGET)
				nv_onattr(np,NV_OPTGET);
			if(type&FSHVALUE)
				nv_onattr(np,NV_SHVALUE);
			break;
		    }

		    /* new test compound command */
		    case TTST:
		    {
			register int n;
			register char *left;
			int negate = (type&TNEGATE)!=0;
#if SHOPT_COSHELL
			if(shp->inpool)
				break;
#endif /* SHOPT_COSHELL */
			if(type&TTEST)
				skipexitset++;
			error_info.line = t->tst.tstline-shp->st.firstline;
			echeck = 1;
			if((type&TPAREN)==TPAREN)
			{
				sh_exec(shp,t->lst.lstlef,OPTIMIZE);
				n = !shp->exitval;
			}
			else
			{
				register bool traceon=0;
				register char *right;
				register char *trap;
				char *argv[6];
				int savexit = shp->savexit;
				n = type>>TSHIFT;
				left = sh_macpat(shp,&(t->lst.lstlef->arg),OPTIMIZE);
				if(type&TBINARY)
					right = sh_macpat(shp,&(t->lst.lstrit->arg),((n==TEST_PEQ||n==TEST_PNE)?ARG_EXP:0)|OPTIMIZE);
				shp->savexit = savexit;
				if(trap=shp->st.trap[SH_DEBUGTRAP])
					argv[0] = (type&TNEGATE)?((char*)e_tstbegin):"[[";
				if(sh_isoption(shp,SH_XTRACE))
				{
					traceon = sh_trace(shp,NIL(char**),0);
					sfwrite(sfstderr,e_tstbegin,(type&TNEGATE?5:3));
				}
				if(type&TUNARY)
				{
					if(traceon)
						sfprintf(sfstderr,"-%c %s",n,sh_fmtq(left));
					if(trap)
					{
						char unop[3];
						unop[0] = '-';
						unop[1] = n;
						unop[2] = 0;
						argv[1] = unop;
						argv[2] = left;
						argv[3] = "]]";
						argv[4] = 0;
						sh_debug(shp,trap,(char*)0,(char*)0,argv, 0);
					}
					n = test_unop(shp,n,left);
				}
				else if(type&TBINARY)
				{
					char *op;
					int pattern = 0;
					if(trap || traceon)
						op = (char*)(shtab_testops+(n&037)-1)->sh_name;
					type >>= TSHIFT;
					if(type==TEST_PEQ || type==TEST_PNE)
						pattern=ARG_EXP;
					if(trap)
					{
						argv[1] = left;
						argv[2] = op;
						argv[3] = right;
						argv[4] = "]]";
						argv[5] = 0;
						sh_debug(shp,trap,(char*)0,(char*)0,argv, pattern);
					}
					n = test_binop(shp,n,left,right);
					if(traceon)
					{
						sfprintf(sfstderr,"%s %s ",sh_fmtq(left),op);
						if(pattern)
							out_pattern(sfstderr,right,-1);
						else
							sfputr(sfstderr,sh_fmtq(right),-1);
					}
				}
				if(traceon)
					sfwrite(sfstderr,e_tstend,4);
			}
			shp->exitval = ((!n)^negate); 
			if(!skipexitset)
				exitset(shp);
			break;
		    }
		}
		if(procsub && *procsub)
		{
			pid_t pid;
			int exitval = shp->exitval;
			while(pid = *procsub++)
				job_wait(pid);
			shp->exitval = exitval;
		}
		if(shp->trapnote&SH_SIGALRM)
		{
			shp->trapnote &= ~SH_SIGALRM;
			sh_timetraps(shp);
		}
		if(shp->trapnote || (shp->exitval && sh_isstate(shp,SH_ERREXIT)) &&
			t && echeck) 
			sh_chktrap(shp);
		/* set $_ */
		if(mainloop && com0)
		{
			/* store last argument here if it fits */
			static char	lastarg[32];
			if(sh_isstate(shp,SH_FORKED))
				sh_done(shp,0);
			if(shp->lastarg!= lastarg && shp->lastarg)
				free(shp->lastarg);
			if(strlen(comn) < sizeof(lastarg))
			{
				nv_onattr(L_ARGNOD,NV_NOFREE);
				shp->lastarg = strcpy(lastarg,comn);
			}
			else
			{
				nv_offattr(L_ARGNOD,NV_NOFREE);
				shp->lastarg = strdup(comn);
			}
		}
		if(!skipexitset)
			exitset(shp);
#if SHOPT_COSHELL
		if(!shp->inpool && !(OPTIMIZE))
#else
		if(!(OPTIMIZE))
#endif /* SHOPT_COSHELL */
		{
			if(sav != stkptr(stkp,0))
				stkset(stkp,sav,0);
			else if(stktell(stkp))
				stkseek(stkp,0);
		}
		if(shp->trapnote&SH_SIGSET)
			sh_exit(shp,SH_EXITSIG|shp->lastsig);
		if(was_interactive)
			sh_onstate(shp,SH_INTERACTIVE);
		if(was_monitor && sh_isoption(shp,SH_MONITOR))
			sh_onstate(shp,SH_MONITOR);
		if(was_errexit)
			sh_onstate(shp,SH_ERREXIT);
	}
	return(shp->exitval);
}

/*
 * test for equality with second argument trimmed
 * returns 1 if r == trim(s) otherwise 0
 */

static bool trim_eq(register const char *r,register const char *s)
{
	register char c;
	while(c = *s++)
	{
		if(c=='\\')
			c = *s++;
		if(c && c != *r++)
			return(0);
	}
	return(*r==0);
}

/*
 * print out the command line if set -x is on
 */

bool sh_trace(Shell_t *shp,register char *argv[], register int nl)
{
	register char *cp;
	register int bracket = 0;
	int decl = (nl&2);
	nl &= ~2;
	if(sh_isoption(shp,SH_XTRACE))
	{
		/* make this trace atomic */
		sfset(sfstderr,SF_SHARE|SF_PUBLIC,0);
		if(!(cp=nv_getval(sh_scoped(shp,PS4NOD))))
			cp = "+ ";
		else
		{
			shp->intrace = 1;
			sh_offoption(shp,SH_XTRACE);
			cp = sh_mactry(shp,cp);
			sh_onoption(shp,SH_XTRACE);
			shp->intrace = 0;
		}
		if(*cp)
			sfputr(sfstderr,cp,-1);
		if(argv)
		{
			char *argv0 = *argv;
			nl = (nl?'\n':-1);
			/* don't quote [ and [[ */
			if(*(cp=argv[0])=='[' && (!cp[1] || !cp[2]&&cp[1]=='['))  
			{
				sfputr(sfstderr,cp,*++argv?' ':nl);
				bracket = 1;
			}
			while(cp = *argv++)
			{
				if(bracket==0 || *argv || *cp!=']')
					cp = sh_fmtq(cp);
				if(decl && shp->prefix && cp!=argv0 && *cp!='-')
				{
					if(*cp=='.' && cp[1]==0)
						cp = shp->prefix;
					else
						sfputr(sfstderr,shp->prefix,'.');
				}
				sfputr(sfstderr,cp,*argv?' ':nl);
			}
			sfset(sfstderr,SF_SHARE|SF_PUBLIC,1);
		}
		return(true);
	}
	return(false);
}

/*
 * This routine creates a subshell by calling fork() or vfork()
 * If ((flags&COMASK)==TCOM), then vfork() is permitted
 * If fork fails, the shell sleeps for exponentially longer periods
 *   and tries again until a limit is reached.
 * SH_FORKLIM is the max period between forks - power of 2 usually.
 * Currently shell tries after 2,4,8,16, and 32 seconds and then quits
 * Failures cause the routine to error exit.
 * Parent links to here-documents are removed by the child
 * Traps are reset by the child
 * The process-id of the child is returned to the parent, 0 to the child.
 */

static void timed_out(void *handle)
{
	NOT_USED(handle);
	timeout = 0;
}


/*
 * called by parent and child after fork by sh_fork()
 */
pid_t _sh_fork(Shell_t *shp,register pid_t parent,int flags,int *jobid)
{
	static long forkcnt = 1000L;
	pid_t	curpgid = job.curpgid;
	pid_t	postid = (flags&FAMP)?0:curpgid;
	int	sig,nochild;
	if(parent<0)
	{
		sh_sigcheck(shp);
		if((forkcnt *= 2) > 1000L*SH_FORKLIM)
		{
			forkcnt=1000L;
			errormsg(SH_DICT,ERROR_system(ERROR_NOEXEC),e_nofork);
		}
		timeout = (void*)sh_timeradd(forkcnt, 0, timed_out, NIL(void*));
		nochild = job_wait((pid_t)1);
		if(timeout)
		{
			if(nochild)
				pause();
			else if(forkcnt>1000L)
				forkcnt /= 2;
			timerdel(timeout);
			timeout = 0;
		}
		return(-1);
	}
	forkcnt = 1000L;
	if(parent)
	{
		int myjob,waitall=job.waitall;
		shp->gd->nforks++;
		if(job.toclear)
			job_clear(shp);
		job.waitall = waitall;
#ifdef JOBS
		/* first process defines process group */
		if(sh_isstate(shp,SH_MONITOR))
		{
			/*
			 * errno==EPERM means that an earlier processes
			 * completed.  Make parent the job group id.
			 */
			if(postid==0)
				job.curpgid = job.jobcontrol?parent:getpid();
			if(job.jobcontrol || (flags&FAMP))
			{
				if(setpgid(parent,job.curpgid)<0 && errno==EPERM)
					setpgid(parent,parent);
			}
		}
#endif /* JOBS */
		if(!sh_isstate(shp,SH_MONITOR) && job.waitall && postid==0)
			job.curpgid = parent;
		if(flags&FCOOP)
			shp->cpid = parent;
		if(!postid && job.curjobid && (flags&FPOU))
			postid = job.curpgid;
		if(!postid && (flags&(FAMP|FINT)) == (FAMP|FINT))
			postid = 1;
		myjob = job_post(shp,parent,postid);
		if(postid==1)
			postid = 0;
		if(job.waitall && (flags&FPOU))
		{
			if(!job.curjobid)
				job.curjobid = myjob;
			if(job.exitval)
				job.exitval++;
		}
		if(flags&FAMP)
			job.curpgid = curpgid;
		if(jobid)
			*jobid = myjob;
		if(shp->comsub==1 && usepipe)
		{
			if(!tsetio || !subdup)
			{
#ifdef SPAWN_cwd
				if(shp->vexp->cur>restorevex)
					sh_vexrestore(shp,restorevex);
#endif
				if(shp->topfd > restorefd)
					sh_iorestore(shp,restorefd,0);
				sh_iounpipe(shp);
			}
		}
		return(parent);
	}
#if !_std_malloc
	vmtrace(-1);
#endif
	shp->outpipepid = ((flags&FPOU)?getpid():0);
	/* This is the child process */
	if(shp->trapnote&SH_SIGTERM)
		sh_exit(shp,SH_EXITSIG|SIGTERM);
	shp->gd->nforks=0;
	timerdel(NIL(void*));
#ifdef JOBS
	if(!job.jobcontrol && !(flags&FAMP))
		sh_offstate(shp,SH_MONITOR);
	if(sh_isstate(shp,SH_MONITOR))
	{
		parent = getpid();
		if(postid==0)
			job.curpgid = parent;
		while(setpgid(0,job.curpgid)<0 && job.curpgid!=parent)
			job.curpgid = parent;
#   ifdef SIGTSTP
		if(job.curpgid==parent &&  !(flags&FAMP))
			tcsetpgrp(job.fd,job.curpgid);
#   endif /* SIGTSTP */
	}
#   ifdef SIGTSTP
	if(job.jobcontrol)
	{
		signal(SIGTTIN,SIG_DFL);
		signal(SIGTTOU,SIG_DFL);
		signal(SIGTSTP,SIG_DFL);
	}
#   endif /* SIGTSTP */
	job.jobcontrol = 0;
#endif /* JOBS */
	job.toclear = 1;
	shp->login_sh = 0;
	sh_offoption(shp,SH_LOGIN_SHELL);
	sh_onstate(shp,SH_FORKED);
	sh_onstate(shp,SH_NOLOG);
	if (shp->fn_reset)
		shp->fn_depth = shp->fn_reset = 0;
#if SHOPT_ACCT
	sh_accsusp();
#endif	/* SHOPT_ACCT */
	/* Reset remaining signals to parent */
	/* except for those `lost' by trap   */
	if(!(flags&FSHOWME))
		sh_sigreset(shp,2);
	shp->subshell = 0;
	shp->comsub = 0;
	shp->spid = 0;
	if((flags&FAMP) && shp->coutpipe>1)
		sh_close(shp->coutpipe);
	sig = shp->savesig;
	shp->savesig = 0;
	if(sig>0)
		kill(getpid(),sig);
	sh_sigcheck(shp);
	usepipe=0;
	return(0);
}

pid_t sh_fork(Shell_t *shp,int flags, int *jobid)
{
	register pid_t parent;
	sigset_t set,oset;
	if(!shp->pathlist)
		path_get(shp,"");
	sfsync(NIL(Sfio_t*));
	shp->trapnote &= ~SH_SIGTERM;
	job_fork(-1);
	sigfillset(&set);
	sigprocmask(SIG_BLOCK,&set,&oset);
	while(_sh_fork(shp,parent=fork(),flags,jobid) < 0);
	sh_stats(STAT_FORKS);
#ifdef SPAWN_cwd
	if(parent==0 && shp->vex)
	{
		spawnvex_apply(shp->vex,0,0);
		spawnvex_apply(shp->vexp,0,SPAWN_RESET);
	}
#endif /* SPAWN_cwd */
	sigprocmask(SIG_SETMASK,&oset,(sigset_t*)0);
	job_fork(parent);
	return(parent);
}

struct Tdata
{
        Shell_t         *sh;
        Namval_t        *tp;
	void		*extra[2];
};

/*
 * add exports from previous scope to the new scope
 */
static void  local_exports(register Namval_t *np, void *data)
{
	Shell_t			*shp = ((struct Tdata*)data)->sh;
	register Namval_t	*mp;
	register char		*cp;
	if(nv_isarray(np))
		nv_putsub(np,NIL(char*),0,0);
	if((cp = nv_getval(np)) && (mp = nv_search(nv_name(np), shp->var_tree, NV_ADD|HASH_NOSCOPE)) && nv_isnull(mp))
		nv_putval(mp, cp, 0);
}

/*
 * This routine executes .sh.math functions from within ((...)))
*/
Sfdouble_t sh_mathfun(Shell_t *shp,void *fp, int nargs, Sfdouble_t *arg)
{
	Sfdouble_t	d;
	Namval_t	node,*mp,*np, *nref[9], **nr=nref;
	char		*argv[2];
	struct funenv	funenv;
	int		i;
	np = (Namval_t*)fp;
	funenv.node = np;
	funenv.nref = nref; 
	funenv.env = 0;
	memcpy(&node,SH_VALNOD,sizeof(node));
	SH_VALNOD->nvfun = 0;
	SH_VALNOD->nvenv = 0;
	SH_VALNOD->nvflag = NV_LDOUBLE|NV_NOFREE;
	SH_VALNOD->nvalue.ldp = 0;
	for(i=0; i < nargs; i++)	
	{
		*nr++ = mp = nv_namptr(shp->mathnodes,i);
		mp->nvalue.ldp = arg++;
	}
	*nr = 0;
	SH_VALNOD->nvalue.ldp = &d;
	argv[0] =  np->nvname;
	argv[1] = 0;
	sh_funscope(shp,1,argv,0,&funenv,0);
	while(mp= *nr++)
		mp->nvalue.ldp = 0;
	SH_VALNOD->nvfun = node.nvfun;
	SH_VALNOD->nvflag = node.nvflag;
	SH_VALNOD->nvenv = node.nvenv;
	SH_VALNOD->nvalue.ldp = node.nvalue.ldp;
	return(d);
}

static void sh_funct(Shell_t *shp,Namval_t *np,int argn, char *argv[],struct argnod *envlist,int execflg)
{
	struct funenv fun;
	char *fname = nv_getval(SH_FUNNAMENOD);
	struct Level	*lp =(struct Level*)(SH_LEVELNOD->nvfun);
	int		level, pipepid=shp->pipepid;
	shp->pipepid = 0;
	sh_stats(STAT_FUNCT);
	if(!lp->hdr.disc)
		lp = init_level(shp,0);
	if((struct sh_scoped*)shp->topscope != shp->st.self)
		sh_setscope(shp,shp->topscope);
	level = lp->maxlevel = shp->dot_depth + shp->fn_depth+1;
	SH_LEVELNOD->nvalue.s = lp->maxlevel;
	shp->st.lineno = error_info.line;
	np->nvalue.rp->running  += 2;
	if(nv_isattr(np,NV_FPOSIX) && !sh_isoption(shp,SH_BASH))
	{
		char *save;
		int loopcnt = shp->st.loopcnt;
		shp->posix_fun = np;
		save = argv[-1];
		argv[-1] = 0;
		shp->st.funname = nv_name(np);
		shp->last_root = nv_dict(DOTSHNOD);
		nv_putval(SH_FUNNAMENOD, nv_name(np),NV_NOFREE);
		opt_info.index = opt_info.offset = 0;
		error_info.errors = 0;
		shp->st.loopcnt = 0;
		b_dot_cmd(argn+1,argv-1,&shp->bltindata);
		shp->st.loopcnt = loopcnt;
		argv[-1] = save;
	}
	else
	{
		fun.env = envlist;
		fun.node = np;
		fun.nref = 0;
		sh_funscope(shp,argn,argv,0,&fun,execflg);
	}
	if(level-- != nv_getnum(SH_LEVELNOD))
	{
		Shscope_t *sp = sh_getscope(shp,0,SEEK_END);
		sh_setscope(shp,sp);
	}
	lp->maxlevel = level;
	SH_LEVELNOD->nvalue.s = lp->maxlevel;
	shp->last_root = nv_dict(DOTSHNOD);
#if 0
	nv_putval(SH_FUNNAMENOD,shp->st.funname,NV_NOFREE);
#else
	nv_putval(SH_FUNNAMENOD,fname,NV_NOFREE);
#endif
	nv_putval(SH_PATHNAMENOD,shp->st.filename,NV_NOFREE);
	shp->pipepid = pipepid;
	np->nvalue.rp->running  -= 2;
	if(np->nvalue.rp && np->nvalue.rp->running==1)
	{
		np->nvalue.rp->running = 0;
		_nv_unset(np, NV_RDONLY);
	}
}

/*
 * external interface to execute a function without arguments
 * <np> is the function node
 * If <nq> is not-null, then sh.name and sh.subscript will be set
 */
int sh_fun_20120720(Shell_t *shp,Namval_t *np, Namval_t *nq, char *argv[])
{
	register int offset;
	register char *base;
	Namval_t node;
	struct Namref	nr;
	long		mode;
	char		*prefix = shp->prefix;
	int n=0;
	char *av[3];
	Fcin_t save;
	fcsave(&save);
	if((offset=stktell(shp->stk))>0)
		base=stkfreeze(shp->stk,0);
	shp->prefix = 0;
	if(!argv)
	{
		argv = av+1;
		argv[1]=0;
	}
	argv[0] = nv_name(np);
	while(argv[n])
		n++;
	if(nq)
		mode = set_instance(shp,nq,&node, &nr);
	if(is_abuiltin(np))
	{
		int jmpval;
		struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
		Shbltin_t *bp = &shp->bltindata;
		sh_pushcontext(shp,buffp,SH_JMPCMD);
		jmpval = sigsetjmp(buffp->buff,1);
		if(jmpval == 0)
		{
			bp->bnode = np;
			bp->ptr = nv_context(np);
			errorpush(&buffp->err,0);
			error_info.id = argv[0];
			opt_info.index = opt_info.offset = 0;
			opt_info.disc = 0;
			shp->exitval = 0;
			shp->exitval = ((Shbltin_f)funptr(np))(n,argv,bp);
		}
		sh_popcontext(shp,buffp);
		if(jmpval>SH_JMPCMD)
			siglongjmp(*shp->jmplist,jmpval);
	}
	else
		sh_funct(shp,np,n,argv,(struct argnod*)0,sh_isstate(shp,SH_ERREXIT));
	if(nq)
		unset_instance(nq, &node, &nr, mode);
	fcrestore(&save);
	if(offset>0)
		stkset(shp->stk,base,offset);
	shp->prefix = prefix;
	return(shp->exitval);
}
#undef sh_fun
int sh_fun(Namval_t *np, Namval_t *nq, char *argv[])
{
	return(sh_fun_20120720(sh_getinterp(),np,nq,argv));
}

/*
 * This dummy routine is called by built-ins that do recursion
 * on the file system (chmod, chgrp, chown).  It causes
 * the shell to invoke the non-builtin version in this case
 */
int cmdrecurse(int argc, char* argv[], int ac, char* av[])
{
	NOT_USED(argc);
	NOT_USED(argv[0]);
	NOT_USED(ac);
	NOT_USED(av[0]);
	return(SH_RUNPROG);
}

/*
 * set up pipe for cooperating process 
 */
static void coproc_init(Shell_t *shp, int pipes[])
{
	int outfd;
	if(shp->coutpipe>=0 && shp->cpid)
		errormsg(SH_DICT,ERROR_exit(1),e_pexists);
	shp->cpid = 0;
	if(shp->cpipe[0]<=0 || shp->cpipe[1]<=0)
	{
		/* first co-process */
		sh_pclose(shp->cpipe);
		sh_pipe(shp->cpipe);
		if((outfd=shp->cpipe[1]) < 10) 
		{
		        int fd=sh_fcntl(shp->cpipe[1],F_dupfd_cloexec,10);
			if(fd>=10)
			{
			        shp->fdstatus[fd] = (shp->fdstatus[outfd]&~IOCLEX);
				sh_close(outfd);
			        shp->fdstatus[outfd] = IOCLOSE;
				shp->cpipe[1] = fd;
			}
		}
		shp->fdptrs[shp->cpipe[0]] = shp->cpipe;
	}
	shp->outpipe = shp->cpipe;
	sh_pipe(shp->inpipe=pipes);
	shp->coutpipe = shp->inpipe[1];
	shp->fdptrs[shp->coutpipe] = &shp->coutpipe;
}

#if SHOPT_SPAWN


#if SHOPT_AMP || !defined(_lib_fork)

/*
 * create a shell script consisting of t->fork.forktre and execute it
 */
static int run_subshell(Shell_t *shp,const Shnode_t *t,pid_t grp)
{
	static const char prolog[] = "(print $(typeset +A);set; typeset -p; print .sh.dollar=$$;set +o)";
	register int i, fd, trace = sh_isoption(shp,SH_XTRACE);
	int pin,pout;
	pid_t pid;
	char *arglist[3], *envlist[2], devfd[12], *cp;
	Sfio_t *sp = sftmp(0);
	envlist[0] = "_=" SH_ID;
	envlist[1] = 0;
	arglist[0] = error_info.id?error_info.id:shp->shname;
	if(*arglist[0]=='-')
		arglist[0]++;
	arglist[1] = devfd;
	strncpy(devfd,e_devfdNN,sizeof(devfd));
	arglist[2] = 0;
	sfstack(sfstdout,sp);
	if(trace)
		sh_offoption(shp,SH_XTRACE);
	sfwrite(sfstdout,"typeset -A -- ",14);
	sh_trap(shp,prolog,0);
	nv_scan(shp->fun_tree, print_fun, (void*)0,0, 0);
	if(shp->st.dolc>0)
	{
		/* pass the positional parameters */
		char **argv = shp->st.dolv+1;
		sfwrite(sfstdout,"set --",6);
		while(*argv)
			sfprintf(sfstdout," %s",sh_fmtq(*argv++));
		sfputc(sfstdout,'\n');
	}
	pin = (shp->inpipe?shp->inpipe[1]:0);
	pout = (shp->outpipe?shp->outpipe[0]:0);
	for(i=3; i < 10; i++)
	{
		if(shp->fdstatus[i]&IOCLEX && i!=pin && i!=pout)
		{
			sfprintf(sfstdout,"exec %d<&%d\n",i,i);
			fcntl(i,F_SETFD,0);
		}
	}
	sfprintf(sfstdout,"LINENO=%d\n",t->fork.forkline);
	if(trace)
	{
		sfwrite(sfstdout,"set -x\n",7);
		sh_onoption(shp,SH_XTRACE);
	}
	sfstack(sfstdout,NIL(Sfio_t*));
	sh_deparse(sp,t->fork.forktre,0);
	sfseek(sp,(Sfoff_t)0,SEEK_SET);
	fd = sh_dup(sffileno(sp));
	cp = devfd+8;
	if(fd>9)
		*cp++ = '0' + (fd/10);
	*cp++ = '0' + fd%10;
	*cp = 0;
	sfclose(sp);
	sfsync(NIL(Sfio_t*));
	if(!shp->gd->shpath)
		shp->gd->shpath = pathshell();
	pid = spawnveg(shp->shpath,arglist,envlist,grp);
	sh_close(fd);
	for(i=3; i < 10; i++)
	{
		if(shp->fdstatus[i]&IOCLEX && i!=pin && i!=pout)
			fcntl(i,F_SETFD,FD_CLOEXEC);
	}
	if(pid <=0)
		errormsg(SH_DICT,ERROR_system(ERROR_NOEXEC),e_exec,arglist[0]);
	return(pid);
}
#endif /* !_lib_fork */

static void sigreset(Shell_t *shp,int mode)
{
	register char   *trap;
	register int sig=shp->st.trapmax;
	while(sig-- > 0)
	{
#ifdef SIGCLD
#   if SIGCLD!=SIGCHLD
		if(sig==SIGCLD)
			continue;
#   endif
#endif
		if(sig==SIGCHLD)
			continue;
		if(shp->sigflag[sig]&SH_SIGOFF)
			return;
		if((trap=shp->st.trapcom[sig]) && *trap==0)
			signal(sig,mode?(sh_sigfun_t)sh_fault:SIG_IGN);
	}
}

/*
 * A combined fork/exec for systems with slow or non-existent fork()
 */
static pid_t sh_ntfork(Shell_t *shp,const Shnode_t *t,char *argv[],int *jobid,int flag)
{
	static pid_t	spawnpid;
	static int	savetype;
	static int	savejobid;
	struct checkpt	*buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
	int		otype=0, jmpval,jobfork=0, lineno=shp->st.firstline;
	volatile int	jobwasset=0, scope=0, sigwasset=0;
	char		**arge, *path;
	volatile pid_t	grp = 0;
	Pathcomp_t	*pp;
	if(flag)
	{
		otype = savetype;
		savetype=0;
	}
#   if SHOPT_AMP || !defined(_lib_fork)
	if(!argv)
	{
		register Shnode_t *tchild = t->fork.forktre;
		int optimize=0;
		otype = t->tre.tretyp;
		savetype = otype;
		spawnpid = 0;
#	ifndef _lib_fork
		if((tchild->tre.tretyp&COMMSK)==TCOM)
		{
			Namval_t *np = (Namval_t*)(tchild->com.comnamp);
			if(np)
			{
				path = nv_name(np);
				if(!nv_isattr(np,BLT_ENV))
					np=0;
				else if(strcmp(path,"echo")==0 || memcmp(path,"print",5)==0)
					np=0;
			}
			else if(!tchild->com.comarg)
				optimize=1;
			else if(tchild->com.comtyp&COMSCAN)
			{
				if(tchild->com.comarg->argflag&ARG_RAW)
					path = tchild->com.comarg->argval;
				else
					path = 0;
			}
			else
				path = ((struct dolnod*)tchild->com.comarg)->dolval[ARG_SPARE];
			if(!np && path && !nv_search(path,shp->fun_tree,0))
				optimize=1;
		}
#	endif
		sh_pushcontext(shp,buffp,SH_JMPIO);
		jmpval = sigsetjmp(buffp->buff,0);
		{
			if((otype&FINT) && !sh_isstate(shp,SH_MONITOR))
			{
				signal(SIGQUIT,SIG_IGN);
				signal(SIGINT,SIG_IGN);
				if(!shp->st.ioset)
				{
					sh_iosave(shp,0,buffp->topfd,(char*)0);
					sh_iorenumber(shp,sh_open(e_devnull,O_RDONLY),0);
				}
			}
			if(otype&FPIN)
			{
				int fd = shp->inpipe[1];
#if 0
				sh_vexsave(shp,0,shp->inpipe[0],0,0);
#else
				sh_iosave(shp,0,buffp->topfd,(char*)0);
				sh_iorenumber(shp,shp->inpipe[0],0);
#endif
			}
			if(otype&FPOU)
			{
#if SHOPT_COSHELL
					if(shp->outpipe[2] > 20000)
						sh_coaccept(shp,shp->outpipe,1);
#endif /* SHOPT_COSHELL */
#if 0
				sh_vexsave(shp,1,shp->outpipe[1],0,0);
#else
				sh_iosave(shp,1,buffp->topfd,(char*)0);
				sh_iorenumber(shp,sh_dup(shp->outpipe[1]),1);
#endif
			}
	
			if(t->fork.forkio)
				sh_redirect(shp,t->fork.forkio,0);
			if(optimize==0)
			{
#ifdef JOBS
				if(sh_isstate(shp,SH_MONITOR) && (job.jobcontrol || (otype&FAMP)))
				{
					if((otype&FAMP) || job.curpgid==0)
						grp = 1;
					else
						grp = job.curpgid;
				}
#endif /* JOBS */
				spawnpid = run_subshell(shp,t,grp);
			}
			else
			{
				sh_exec(shp,tchild,SH_NTFORK);
				if(jobid)
					*jobid = savejobid;
			}
		}
		sh_popcontext(shp,buffp);
		if((otype&FINT) && !sh_isstate(shp,SH_MONITOR))
		{
			signal(SIGQUIT,sh_fault);
			signal(SIGINT,sh_fault);
		}
		if(t->fork.forkio || otype)
		{
			sh_iorestore(shp,buffp->topfd,jmpval);
			if(shp->vexp->cur>buffp->vexi)
				sh_vexrestore(shp,buffp->vexi);
		}
		if(optimize==0)
		{
			if(spawnpid>0)
				_sh_fork(shp,spawnpid,otype,jobid);
			if(grp>0 && !(otype&FAMP))
			{
				while(tcsetpgrp(job.fd,job.curpgid)<0 && job.curpgid!=spawnpid)
					job.curpgid = spawnpid;
			}
		}
		savetype=0;
		if(jmpval>SH_JMPIO)
			siglongjmp(*shp->jmplist,jmpval);
		if(spawnpid<0 && (otype&FCOOP))
		{
			sh_close(shp->coutpipe);
			sh_close(shp->cpipe[1]);
			shp->cpipe[1] = -1;
			shp->coutpipe = -1;
		}
		shp->exitval = 0;
		return(spawnpid);
	}
#   endif /* !_lib_fork */
	sh_pushcontext(shp,buffp,SH_JMPCMD);
	errorpush(&buffp->err,ERROR_SILENT);
	jmpval = sigsetjmp(buffp->buff,0);
	if(jmpval == 0)
	{
		if((otype&FINT) && !sh_isstate(shp,SH_MONITOR))
		{
			signal(SIGQUIT,SIG_IGN);
			signal(SIGINT,SIG_IGN);
		}
		spawnpid = -1;
		if(t->com.comio)
		{
			shp->errorfd = error_info.fd;
#if 1
			sh_redirect(shp,t->com.comio,io_usevex(t->com.comio));
#else
			sh_redirect(shp,t->com.comio,0);
#endif
		}
		error_info.id = *argv;
		if(t->com.comset)
		{
			scope++;
			sh_scope(shp,t->com.comset,0);
		}
		if(!strchr(path=argv[0],'/')) 
		{
			Namval_t *np;
			if((np=nv_search(path,shp->track_tree,0)) && !nv_isattr(np,NV_NOALIAS) && np->nvalue.cp)
				path = nv_getval(np);
			else if(path_absolute(shp,path,NIL(Pathcomp_t*)))
			{
				path = stkptr(shp->stk,PATH_OFFSET);
				stkfreeze(shp->stk,0);
			}
			else
			{
				pp=path_get(shp,path);
				while(pp)
				{
					if(pp->len==1 && *pp->name=='.')
						break;
					pp = pp->next;
				}
				if(!pp)
					path = 0;
			}
		}
		else if(sh_isoption(shp,SH_RESTRICTED))
			errormsg(SH_DICT,ERROR_exit(1),e_restricted,path);
		if(!path)
		{
			spawnpid = -1;
			goto fail;
		}
		arge = sh_envgen(shp);
		/* restore firstline in case LINENO was exported */
		shp->st.firstline = lineno;
		shp->exitval = 0;
#ifdef JOBS
		if(sh_isstate(shp,SH_MONITOR) && (job.jobcontrol || (otype&FAMP)))
		{
			if((otype&FAMP) || job.curpgid==0)
				grp = 1;
			else
				grp = job.curpgid;
		}
#endif /* JOBS */

		sfsync(NIL(Sfio_t*));
		sigreset(shp,0);	/* set signals to ignore */
		sigwasset++;
	        /* find first path that has a library component */
		for(pp=path_get(shp,argv[0]); pp && !pp->lib ; pp=pp->next);
		job_fork(-1);
		jobfork = 1;
		spawnpid = path_spawn(shp,path,argv,arge,pp,(grp<<1)|1);
		if(spawnpid < 0 && errno==ENOEXEC)
		{
			char *devfd;
			int fd = open(path,O_RDONLY);
			argv[-1] = argv[0];
			argv[0] = path;
			if(fd>=0)
			{
				struct stat statb;
				sfprintf(shp->strbuf,"/dev/fd/%d",fd);
				if(stat(devfd=sfstruse(shp->strbuf),&statb)>=0)
					argv[0] =  devfd;
			}
			if(!shp->gd->shpath)
				shp->gd->shpath = pathshell();
			spawnpid = path_spawn(shp,shp->gd->shpath,&argv[-1],arge,pp,(grp<<1)|1);
			if(fd>=0)
				sh_close(fd);
			argv[0] = argv[-1];
		}
	fail:
		if(jobfork && spawnpid<0) 
			job_fork(0);
		if(spawnpid < 0) switch(errno=shp->path_err)
		{
		    case ENOENT:
			errormsg(SH_DICT,ERROR_system(ERROR_NOENT),e_found+4);
		    default:
			errormsg(SH_DICT,ERROR_system(ERROR_NOEXEC),e_exec+4);
		}
	}
	else
		exitset(shp);
	sh_popcontext(shp,buffp);
	if(buffp->olist)
		free_list(buffp->olist);
	if(sigwasset)
		sigreset(shp,1);	/* restore ignored signals */
	if(scope)
	{
		sh_unscope(shp);
		if(jmpval==SH_JMPSCRIPT)
			sh_setlist(shp,t->com.comset,NV_EXPORT|NV_IDENT|NV_ASSIGN,0);
	}
	if(t->com.comio && (jmpval || spawnpid<=0))
	{
		sh_iorestore(shp,buffp->topfd,jmpval);
#ifdef SPAWN_cwd
		if(shp->vexp->cur > buffp->vexi)
			sh_vexrestore(shp,buffp->vexi);
#endif

	}
	if(jmpval>SH_JMPCMD)
		siglongjmp(*shp->jmplist,jmpval);
	if(spawnpid>0)
	{
		_sh_fork(shp,spawnpid,otype,jobid);
		job_fork(spawnpid);
#ifdef JOBS
		if(grp==1)
			job.curpgid = spawnpid;
#   ifdef SIGTSTP
		if(grp>0 && !(otype&FAMP))
		{
			while(tcsetpgrp(job.fd,job.curpgid)<0 && job.curpgid!=spawnpid)
				job.curpgid = spawnpid;
		}
#   endif /* SIGTSTP */
#endif /* JOBS */
		savejobid = *jobid;
		if(otype)
			return(0);
	}
	return(spawnpid);
}

#   ifdef _was_lib_fork
#	define _lib_fork	1
#   endif
#   ifndef _lib_fork
	pid_t fork(void)
	{
		errormsg(SH_DICT,ERROR_exit(3),e_notimp,"fork");
		return(-1);
	}
#   endif /* _lib_fork */
#endif /* SHOPT_SPAWN */

/*
 * This routine is used to execute the given function <fun> in a new scope
 * If <fun> is NULL, then arg points to a structure containing a pointer
 *  to a function that will be executed in the current environment.
 */
int sh_funscope_20120720(Shell_t *shp,int argn, char *argv[],int(*fun)(void*),void *arg,int execflg)
{
	register char		*trap;
	register int		nsig;
	struct dolnod		*argsav=0,*saveargfor;
	struct sh_scoped	savst, *prevscope = shp->st.self;
	struct argnod		*envlist=0;
	int			jmpval;
	volatile int		r = 0;
	int			n;
	char 			*savstak;
	struct funenv		*fp = 0;
	struct checkpt	*buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
	Namval_t		*nspace = shp->namespace;
	Dt_t			*last_root = shp->last_root;
	Shopt_t			options;
	options = shp->options;
	if(shp->fn_depth==0)
		shp->glob_options =  shp->options;
	else
		shp->options = shp->glob_options;
#if 0
	shp->st.lineno = error_info.line;
#endif
	*prevscope = shp->st;
	sh_offoption(shp,SH_ERREXIT);
	shp->st.prevst = prevscope;
	shp->st.self = &savst;
	shp->topscope = (Shscope_t*)shp->st.self;
	shp->st.opterror = shp->st.optchar = 0;
	shp->st.optindex = 1;
	shp->st.loopcnt = 0;
	if(!fun)
	{
		fp = (struct funenv*)arg;
		shp->st.real_fun = (fp->node)->nvalue.rp;
		envlist = fp->env;
	}
	prevscope->save_tree = shp->var_tree;
	n = dtvnext(prevscope->save_tree)!= (shp->namespace?shp->var_base:0);
	sh_scope(shp,envlist,1);
	if(n)
	{
		struct Tdata tdata;
		memset(&tdata,0,sizeof(tdata));
		tdata.sh = shp;
		/* eliminate parent scope */
		nv_scan(prevscope->save_tree, local_exports,&tdata, NV_EXPORT, NV_EXPORT|NV_NOSCOPE);
	}
	shp->st.save_tree = shp->var_tree;
	if(!fun)
	{
		if(nv_isattr(fp->node,NV_TAGGED))
			sh_onoption(shp,SH_XTRACE);
		else
			sh_offoption(shp,SH_XTRACE);
	}
	shp->st.cmdname = argv[0];
	/* save trap table */
	if((nsig=shp->st.trapmax*sizeof(char*))>0 || shp->st.trapcom[0])
	{
		nsig += sizeof(char*);
		memcpy(savstak=stkalloc(shp->stk,nsig),(char*)&shp->st.trapcom[0],nsig);
	}
	sh_sigreset(shp,0);
	argsav = sh_argnew(shp,argv,&saveargfor);
	sh_pushcontext(shp,buffp,SH_JMPFUN);
	errorpush(&buffp->err,0);
	error_info.id = argv[0];
	shp->st.var_local = shp->var_tree;
	if(!fun)
	{
		shp->st.filename = fp->node->nvalue.rp->fname;
		shp->st.funname = nv_name(fp->node);
		shp->last_root = nv_dict(DOTSHNOD);
		nv_putval(SH_PATHNAMENOD,shp->st.filename,NV_NOFREE);
		nv_putval(SH_FUNNAMENOD,shp->st.funname,NV_NOFREE);
	}
	jmpval = sigsetjmp(buffp->buff,0);
	if(jmpval == 0)
	{
		if(shp->fn_depth++ > MAXDEPTH)
		{
			shp->toomany = 1;
			siglongjmp(*shp->jmplist,SH_JMPERRFN);
		}
		else if(fun)
			r= (*fun)(arg);
		else
		{
			char		**args = shp->st.real_fun->argv;
			Namval_t	*np, *nq, **nref;
			if(nref=fp->nref)
			{
				shp->last_root = 0;
				for(r=0; args[r]; r++)
				{
					np = nv_search(args[r],shp->var_tree,HASH_NOSCOPE|NV_ADD);
					if(np && (nq=*nref++))
					{
						np->nvalue.nrp = newof(0,struct Namref,1,0);
						if(nv_isattr(nq,NV_LDOUBLE)==NV_LDOUBLE)
							np->nvalue.nrp->np = nq;
						else
						{
							np->nvalue.nrp->np = (Namval_t*)pointerof((Sflong_t)(*nq->nvalue.ldp));
							nv_onattr(nq,NV_LDOUBLE);
						}
						nv_onattr(np,NV_REF|NV_NOFREE);
					}
				}
			}
			sh_exec(shp,(Shnode_t*)(nv_funtree((fp->node))),execflg|SH_ERREXIT);
			r = shp->exitval;
		}
	}
	if(shp->topscope != (Shscope_t*)shp->st.self)
		sh_setscope(shp,shp->topscope);
	if(--shp->fn_depth==1 && jmpval==SH_JMPERRFN)
		errormsg(SH_DICT,ERROR_exit(1),e_toodeep,argv[0]);
	sh_popcontext(shp,buffp);
	sh_unscope(shp);
	shp->namespace = nspace;
	shp->var_tree = (Dt_t*)prevscope->save_tree;
	sh_argreset(shp,argsav,saveargfor);
	trap = shp->st.trapcom[0];
	shp->st.trapcom[0] = 0;
	sh_sigreset(shp,1);
	shp->st = *prevscope;
	shp->topscope = (Shscope_t*)prevscope;
	nv_getval(sh_scoped(shp,IFSNOD));
	if(nsig)
		memcpy((char*)&shp->st.trapcom[0],savstak,nsig);
	shp->trapnote=0;
	if(nsig)
		stkset(shp->stk,savstak,0);
	shp->options = options;
	shp->last_root = last_root;
	if(jmpval == SH_JMPSUB)
		siglongjmp(*shp->jmplist,jmpval);
	if(trap)
	{
		sh_trap(shp,trap,0);
		free(trap);
	}
	if(jmpval)
		r=shp->exitval;
	if(!sh_isstate(shp,SH_IOPROMPT) && r>SH_EXITSIG && ((r&SH_EXITMASK)==SIGINT || ((r&SH_EXITMASK)==SIGQUIT)))
		kill(getpid(),r&SH_EXITMASK);
	if(jmpval > SH_JMPFUN)
	{
		sh_chktrap(shp);
		siglongjmp(*shp->jmplist,jmpval);
	}
	return(r);
}

#undef sh_funscope
int sh_funscope(int argn, char *argv[],int(*fun)(void*),void *arg,int execflg)
{
	return(sh_funscope_20120720(sh_getinterp(),argn,argv,fun,arg,execflg));
}

/*
 * Given stream <iop> compile and execute
 */
int sh_eval_20120720(Shell_t *shp,register Sfio_t *iop, int mode)
{
	register Shnode_t *t;
	struct slnod *saveslp = shp->st.staklist;
	int jmpval;
	struct checkpt *pp = (struct checkpt*)shp->jmplist;
	struct checkpt *buffp = (struct checkpt*)stkalloc(shp->stk,sizeof(struct checkpt));
	static Sfio_t *io_save;
	volatile bool traceon=false;
	volatile int lineno=0;
	int binscript=shp->binscript;
	char comsub = shp->comsub;
	Sfio_t *iosaved = io_save;
	io_save = iop; /* preserve correct value across longjmp */
	shp->binscript = 0;
	shp->comsub = 0;
#define SH_TOPFUN	0x8000	/* this is a temporary tksh hack */
	if (mode & SH_TOPFUN)
	{
		mode ^= SH_TOPFUN;
		shp->fn_reset = 1;
	}
	sh_pushcontext(shp,buffp,SH_JMPEVAL);
	buffp->olist = pp->olist;
	jmpval = sigsetjmp(buffp->buff,0);
	while(jmpval==0)
	{
		if(mode&SH_READEVAL)
		{
			lineno = shp->inlineno;
			if(traceon=sh_isoption(shp,SH_XTRACE))
				sh_offoption(shp,SH_XTRACE);
		}
		t = (Shnode_t*)sh_parse(shp,iop,(mode&(SH_READEVAL|SH_FUNEVAL))?mode&SH_FUNEVAL:SH_NL);
		if(!(mode&SH_FUNEVAL) || !sfreserve(iop,0,0))
		{
			if(!(mode&SH_READEVAL))
				sfclose(iop);
			io_save = 0;
			mode &= ~SH_FUNEVAL;
		}
		mode &= ~SH_READEVAL;
		if(!sh_isoption(shp,SH_VERBOSE))
			sh_offstate(shp,SH_VERBOSE);
		if((mode&~SH_FUNEVAL) && shp->gd->hist_ptr)
		{
			hist_flush(shp->gd->hist_ptr);
			mode = sh_state(SH_INTERACTIVE);
		}
		sh_exec(shp,t,sh_isstate(shp,SH_ERREXIT)|sh_isstate(shp,SH_NOFORK)|(mode&~SH_FUNEVAL));
		if(!io_save)
			break;
	}
	sh_popcontext(shp,buffp);
	shp->binscript = binscript;
	shp->comsub = comsub;
	if(traceon)
		sh_onoption(shp,SH_XTRACE);
	if(lineno)
		shp->inlineno = lineno;
	if(io_save)
		sfclose(io_save);
	if((io_save=iosaved) == iop)
		io_save = 0;
	sh_freeup(shp);
	shp->st.staklist = saveslp;
	shp->fn_reset = 0;
	if(jmpval>SH_JMPEVAL)
		siglongjmp(*shp->jmplist,jmpval);
	return(shp->exitval);
}

#undef sh_eval
int sh_eval(register Sfio_t *iop, int mode)
{
	return(sh_eval_20120720(sh_getinterp(),iop,mode));
}

int sh_run_20120720(Shell_t *shp,int argn, char *argv[])
{
	register struct dolnod	*dp;
	register struct comnod	*t = (struct comnod*)stkalloc(shp->stk,sizeof(struct comnod));
	int			savtop = stktell(shp->stk);
	char			*savptr = stakfreeze(0);
	Opt_t			*op, *np = optctx(0, 0);
	Shbltin_t		bltindata;
	bltindata = shp->bltindata;
	op = optctx(np, 0);
	memset(t, 0, sizeof(struct comnod));
	dp = (struct dolnod*)stkalloc(shp->stk,(unsigned)sizeof(struct dolnod) + ARG_SPARE*sizeof(char*) + argn*sizeof(char*));
	dp->dolnum = argn;
	dp->dolbot = ARG_SPARE;
	memcpy(dp->dolval+ARG_SPARE, argv, (argn+1)*sizeof(char*));
	t->comarg = (struct argnod*)dp;
	if(!strchr(argv[0],'/'))
		t->comnamp = (void*)nv_bfsearch(argv[0],shp->fun_tree,(Namval_t**)&t->comnamq,(char**)0);
	argn=sh_exec(shp,(Shnode_t*)t,sh_isstate(shp,SH_ERREXIT));
	optctx(op,np);
	shp->bltindata = bltindata;
	if(savptr!=stkptr(shp->stk,0))
		stkset(shp->stk,savptr,savtop);
	else
		stkseek(shp->stk,savtop);
	return(argn);
}

#undef sh_run
int sh_run(int argn, char *argv[])
{
	return(sh_run_20120720(sh_getinterp(),argn,argv));
}

