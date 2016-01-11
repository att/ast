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
 * read [-ACprs] [-q format] [-d delim] [-u filenum] [-t timeout] [-n n] [-N n] [name...]
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#include	<ast.h>
#include	<error.h>
#include	"defs.h"
#include	"variables.h"
#include	"lexstates.h"
#include	"io.h"
#include	"name.h"
#include	"builtins.h"
#include	"history.h"
#include	"terminal.h"
#include	"edit.h"

#define	R_FLAG	1	/* raw mode */
#define	S_FLAG	2	/* save in history file */
#define	A_FLAG	4	/* read into array */
#define N_FLAG	8	/* fixed size read at most */
#define NN_FLAG	0x10	/* fixed size read exact */
#define V_FLAG	0x20	/* use default value */
#define	C_FLAG	0x40	/* read into compound variable */
#define D_FLAG	8	/* must be number of bits for all flags */
#define	SS_FLAG	0x80	/* read .csv format file */

struct read_save
{
	char	**argv;
	char	*prompt;
	short	fd;
	short	plen;
	int	flags;
	int	mindex;
	ssize_t	len;
        long	timeout;
};

struct Method
{
	char	*name;
	void	*fun;
};

static int json2sh(Shell_t *shp, Sfio_t *in, Sfio_t *out)
{
	int	c, state=0, lastc=0, level=0, line=1;
	size_t	here, offset = stktell(shp->stk);
	char	*start;
	bool	isname, isnull=false;
	while((c = sfgetc(in)) > 0)
	{
		if(c=='\n')
			line++;
		if(state==0)
		{
			switch(c)
			{
			    case '\t': case ' ':
				if(lastc==' ' || lastc=='\t')
					continue;
				break;
			    case ',':
				continue;
			    case '[': case '{':
				c = '(';
				level++;
				break;
			    case ']': case '}':
				c = ')';
				level--;
				break;
			    case '"':
				state = 1;
				isname = true;
				sfputc(shp->stk,c);
				continue;
			}
			sfputc(out,c);
			if(level==0)
				break;
		}
		else if(state==1)
		{
			if(c=='"' && lastc != '\\') 
				state=2;
			else if(state==1 && !isalnum(c) && c!='_')
				isname = false;
			sfputc(shp->stk,c);
		}
		else if(state==2)
		{
			char *last;
			if(c==' ' || c == '\t')
				continue;
			if(c==':')
			{
				int len;
				while((c = sfgetc(in)) &&  isblank(c));
				sfungetc(in,c);
				if(!strchr("[{,\"",c))
				{
					if(isdigit(c) || c=='.' || c =='-')
						sfwrite(out,"float ",6);
					else if(c=='t' || c=='f')
						sfwrite(out,"bool ",5);
					else if(c=='n')
					{
						char buff[4];
						isnull = true;
						sfread(in,buff,4);
						sfwrite(out,"typeset  ",8);
					}
				}
				start = stkptr(shp->stk,offset);
				here = stktell(shp->stk);
				if(isname && !isalpha(*(start+1)) && c!='_')
					isname = false;
				len = here-offset-2;
				if(!isname)
				{
					char *sp;
					sfwrite(out,".[",2);
					for(sp=start+1;len-->0;sp++)
					{
						if(*sp=='$')
						{
							if(sp>start+1)
								sfwrite(out,start,sp-start);
							sfputc(out,'\\');
							sfputc(out,'$');
							start = sp;
						}
					}
					len = (sp- start)-1;
				}
				sfwrite(out,start+1, len);
				if(!isname)
					sfputc(out,']');
				if(isnull)
					isnull = false;
				else
					sfputc(out,'=');
				stkseek(shp->stk,offset);
				if(c=='{')
					c = ' ';
			}
			if(c==',' || c=='\n' || c== '}' || c==']' || c=='{')
			{
				start = stkptr(shp->stk,offset);
				here = stktell(shp->stk);
				if(here>1)
				{
					*stkptr(shp->stk,here-1) = 0;
					stresc(start+1);
					sfputr(out,sh_fmtq(start+1),-1);
					stkseek(shp->stk,offset);
				}
				if(c=='{')
					sfputc(out,'=');
				else
					sfputc(out,' ');
				if(c=='}' || c==']' || c=='{')
					sfungetc(in,c);
			}
			c = ' ';
			state = 0;
		}
		lastc = c;
	}
	return(0);
}

static struct Method methods[] = {
	"json",	json2sh,
	"ksh",	0,
	0,	0
};

int	b_read(int argc,char *argv[], Shbltin_t *context)
{
	Sfdouble_t sec;
	register char *name=0;
	register int r, flags=0, fd=0;
	register Shell_t *shp = context->shp;
	ssize_t	len=0;
	long timeout = 1000*shp->st.tmout;
	int save_prompt, fixargs=context->invariant;
	struct read_save *rp;
	int mindex=0;
	char *method = 0;
	void *readfn = 0;
	static char default_prompt[3] = {ESC,ESC};
	rp = (struct read_save*)(context->data);
	if(argc==0)
	{
		if(rp)
			free((void*)rp);
		return(0);
	}
	if(rp)
	{
		flags = rp->flags;
		timeout = rp->timeout;
		fd = rp->fd;
		argv = rp->argv;
		name = rp->prompt;
		mindex = rp->mindex;
		r = rp->plen;
		goto bypass;
	}
	while((r = optget(argv,sh_optread))) switch(r)
	{
	    case 'A':
		flags |= A_FLAG;
		break;
	    case 'C':
		flags |= C_FLAG;
		method = "ksh";
		break;
	    case 't':
		sec = sh_strnum(shp,opt_info.arg, (char**)0,1);
		timeout = sec ? 1000*sec : 1;
		break;
	    case 'd':
		if(opt_info.arg && *opt_info.arg!='\n')
		{
			char *cp = opt_info.arg;
			flags &= ((1<<D_FLAG+1)-1);
			flags |= (mbchar(cp)<< D_FLAG+1) | (1<<D_FLAG);
		}
		break;
	    case 'p':
		if(shp->cpipe[0]<=0 || *opt_info.arg!='-' && (!strmatch(opt_info.arg,"+(\\w)") || isdigit(*opt_info.arg)))
			name = opt_info.arg;
		else
		{
			/* for backward compatibility */
			fd = shp->cpipe[0];
			argv--;
			argc--;
		}
		break;
	    case 'm':
		method = opt_info.arg;
		flags |= C_FLAG;
		break;
	    case 'n': case 'N':
		flags &= ((1<<D_FLAG)-1);
		flags |= (r=='n'?N_FLAG:NN_FLAG);
		len = opt_info.num;
		break;
	    case 'r':
		flags |= R_FLAG;
		break;
	    case 's':
		/* save in history file */
		flags |= S_FLAG;
		break;
	    case 'S':
		flags |= SS_FLAG;
		break;
	    case 'u':
		if(opt_info.arg[0]=='p' && opt_info.arg[1]==0)
		{
			if((fd = shp->cpipe[0])<=0)
				errormsg(SH_DICT,ERROR_exit(1),e_query);
			break;
		}
		fd = (int)strtol(opt_info.arg,&opt_info.arg,10);
		if(*opt_info.arg)
			fd = -1;
		else if(!sh_iovalidfd(shp,fd))
			fd = -1;
		else if(!(shp->inuse_bits&(1<<fd)) && (sh_inuse(shp,fd) || (shp->gd->hist_ptr && fd==sffileno(shp->gd->hist_ptr->histfp))))
		break;
	    case 'v':
		flags |= V_FLAG;
		break;
	    case ':':
		if(shp->cpipe[0]>0 && strcmp(opt_info.arg,"-p: prompt argument expected")==0)
		{
			fd = shp->cpipe[0];
			break;
		}
		errormsg(SH_DICT,2, "%s", opt_info.arg);
		break;
	    case '?':
		errormsg(SH_DICT,ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2), "%s", optusage((char*)0));
	if(method)
	{
		for(mindex=0; methods[mindex].name; mindex++)
		{
			if(strcmp(method,methods[mindex].name)==0)
				break;
		}
		if(!methods[mindex].name)
			errormsg(SH_DICT,ERROR_system(1),"%s method not supported",method);
	}
		
	if(!((r=shp->fdstatus[fd])&IOREAD)  || !(r&(IOSEEK|IONOSEEK)))
		r = sh_iocheckfd(shp,fd,fd);
	if(fd<0 || !(r&IOREAD))
		errormsg(SH_DICT,ERROR_system(1),e_file+4);
	/* look for prompt */
	if(!name && *argv && (name=strchr(*argv,'?')))
		name++;
	if(name && (r&IOTTY))
		r = strlen(name)+1;
	else
		r = 0;
	if(argc==fixargs && (rp=newof(NIL(struct read_save*),struct read_save,1,0)))
	{
		context->data = (void*)rp;
		rp->fd = fd;
		rp->flags = flags;
		rp->timeout = timeout;
		rp->argv = argv;
		rp->prompt = name;
		rp->plen = r;
		rp->len = len;
		rp->mindex = mindex;
	}
bypass:
	shp->prompt = default_prompt;
	if(r && (shp->prompt=(char*)sfreserve(sfstderr,r,SF_LOCKR)))
	{
		memcpy(shp->prompt,name,r);
		sfwrite(sfstderr,shp->prompt,r-1);
	}
	shp->timeout = 0;
	save_prompt = shp->nextprompt;
	shp->nextprompt = 0;
	readfn = (flags&C_FLAG)?methods[mindex].fun:0;
	r=sh_readline(shp,argv,readfn,fd,flags,len,timeout);
	shp->nextprompt = save_prompt;
	if(r==0 && (r=(sfeof(shp->sftable[fd])||sferror(shp->sftable[fd]))))
	{
		if(fd == shp->cpipe[0] && errno!=EINTR)
			sh_pclose(shp->cpipe);
	}
	return(r);
}

struct timeout
{
	Shell_t	*shp;
	Sfio_t	*iop;
};

/*
 * here for read timeout
 */
static void timedout(void *handle)
{
	struct timeout *tp = (struct timeout*)handle;
	sfclrlock(tp->iop);
	sh_exit(tp->shp,1);
}

/*
 * This is the code to read a line and to split it into tokens
 *  <names> is an array of variable names
 *  <fd> is the file descriptor
 *  <flags> is union of -A, -r, -s, and contains delimiter if not '\n'
 *  <timeout> is number of milli-seconds until timeout
 */
int sh_readline(register Shell_t *shp,char **names, void *readfn, volatile int fd, int flags,ssize_t size,long timeout)
{
	register ssize_t	c;
	register unsigned char	*cp;
	register Namval_t	*np;
	register char		*name, *val;
	register Sfio_t		*iop;
	Namfun_t		*nfp;
	char			*ifs;
	unsigned char		*cpmax;
	unsigned char		*del;
	char			was_escape = 0;
	char			use_stak = 0;
	volatile char		was_write = 0;
	volatile char		was_share = 1;
	volatile int		keytrap;
	int			rel, wrd;
	long			array_index = 0;
	void			*timeslot=0;
	int			delim = '\n';
	int			jmpval=0;
	int			binary;
	int			oflags=NV_ASSIGN|NV_VARNAME;
	char			inquote = 0;
	struct	checkpt		buff;
	Edit_t			*ep = (struct edit*)shp->gd->ed_context;
	Namval_t		*nq = 0;
	if(!(iop=shp->sftable[fd]) && !(iop=sh_iostream(shp,fd,fd)))
		return(1);
	sh_stats(STAT_READS);
	if(names && (name = *names))
	{
		Namval_t *mp;
		if(val= strchr(name,'?'))
			*val = 0;
		if(flags&C_FLAG)
			oflags |= NV_ARRAY;
		np = nv_open(name,shp->var_tree,oflags);
		if(np && nv_isarray(np) && (mp=nv_opensub(np)))
			np = mp;
		if((flags&V_FLAG) && shp->gd->ed_context)
			((struct edit*)shp->gd->ed_context)->e_default = np;
		if(flags&A_FLAG)
		{
			Namarr_t *ap;
			flags &= ~A_FLAG;
			array_index = 1;
			if((ap=nv_arrayptr(np)) && !ap->fun)
				ap->nelem++;
			nv_unset(np);
			if((ap=nv_arrayptr(np)) && !ap->fun)
				ap->nelem--;
			nv_putsub(np,NIL(char*),0L,0);
		}
		else if(flags&C_FLAG)
		{
			char *sp =  np->nvenv;
			if(strchr(name,'['))
				nq = np;
			delim = -1;
			nv_unset(np);
			if(!nv_isattr(np,NV_MINIMAL))
				np->nvenv = sp;
			nv_setvtree(np);
			*(void**)(np->nvfun+1) = readfn;
		}
		else
			name = *++names;
		if(val)
			*val = '?';
	}
	else
	{
		name = 0;
		if(dtvnext(shp->var_tree) || shp->namespace)
                	np = nv_open(nv_name(REPLYNOD),shp->var_tree,0);
		else
			np = REPLYNOD;
	}
	keytrap =  ep?ep->e_keytrap:0;
	if(size || (flags>>D_FLAG))	/* delimiter not new-line or fixed size read */
	{
		if((shp->fdstatus[fd]&IOTTY) && !keytrap)
			tty_raw(sffileno(iop),1);
		if(!(flags&(N_FLAG|NN_FLAG)))
		{
			delim = ((unsigned)flags)>>(D_FLAG+1);
			ep->e_nttyparm.c_cc[VEOL] = delim;
			ep->e_nttyparm.c_lflag |= ISIG;
			tty_set(sffileno(iop),TCSADRAIN,&ep->e_nttyparm);
		}
	}
	binary = nv_isattr(np,NV_BINARY);
	if(!binary && !(flags&(N_FLAG|NN_FLAG)))
	{
		Namval_t *mp;
		/* set up state table based on IFS */
		ifs = nv_getval(mp=sh_scoped(shp,IFSNOD));
		if((flags&R_FLAG) && shp->ifstable['\\']==S_ESC)
			shp->ifstable['\\'] = 0;
		else if(!(flags&R_FLAG) && shp->ifstable['\\']==0)
			shp->ifstable['\\'] = S_ESC;
		if(delim>0)
			shp->ifstable[delim] = S_NL;
		if(delim!='\n')
		{
			if(ifs && strchr(ifs,'\n'))
				shp->ifstable['\n'] = S_DELIM;
			else
				shp->ifstable['\n'] = 0;
			nv_putval(mp, ifs, NV_RDONLY);
		}
		shp->ifstable[0] = S_EOF;
		if((flags&SS_FLAG))
		{
			shp->ifstable['"'] = S_QUOTE;
			shp->ifstable['\r'] = S_ERR;
		}
	}
	sfclrerr(iop);
	for(nfp=np->nvfun; nfp; nfp = nfp->next)
	{
		if(nfp->disc && nfp->disc->readf)
		{
			Namval_t *mp;
			if(nq)
				mp = nq;
			else
				mp = nv_open(name,shp->var_tree,oflags|NV_NOREF);
			if((c=(*nfp->disc->readf)(mp,iop,delim,nfp))>=0)
				return(c);
		}
	}
	if(binary && !(flags&(N_FLAG|NN_FLAG)))
	{
		flags |= NN_FLAG;
		size = nv_size(np);
	}
	was_write = (sfset(iop,SF_WRITE,0)&SF_WRITE)!=0;
	if(sffileno(iop)==0)
		was_share = (sfset(iop,SF_SHARE,shp->redir0!=2)&SF_SHARE)!=0;
	if(timeout || (shp->fdstatus[fd]&(IOTTY|IONOSEEK)))
	{
		sh_pushcontext(shp,&buff,1);
		jmpval = sigsetjmp(buff.buff,0);
		if(jmpval)
			goto done;
		if(timeout)
		{
			struct timeout tmout;
			tmout.shp = shp;
			tmout.iop = iop;
	                timeslot = (void*)sh_timeradd(timeout,0,timedout,(void*)&tmout);
		}
	}
	if(flags&(N_FLAG|NN_FLAG))
	{
		char buf[256],*var=buf,*cur,*end,*up,*v;
		/* reserved buffer */
		if((c=size)>=sizeof(buf))
		{
			if(!(var = (char*)malloc(c+1)))
				sh_exit(shp,1);
			end = var + c;
		}
		else
			end = var + sizeof(buf) - 1;
		up = cur = var;
		if((sfset(iop,SF_SHARE,1)&SF_SHARE) && sffileno(iop)!=0)
			was_share = 1;
		if(size==0)
		{
			cp = sfreserve(iop,0,0);
			c = 0;
		}
		else
		{
			ssize_t	m;
			int	f;
			for (;;)
			{
				c = size;
				if(keytrap)
				{
					cp = 0;
					f = 0;
					m = 0;
					while(c-->0 && (buf[m]=ed_getchar(ep,0)))
						m++;
					if(m>0)
						cp = (unsigned char*)buf;
				}
				else
				{
					f = 1;
					if(cp = sfreserve(iop,c,SF_LOCKR))
						m = sfvalue(iop);
					else if(flags&NN_FLAG)
					{
						c = size;
						m = (cp = sfreserve(iop,c,0)) ? sfvalue(iop) : 0;
						f = 0;
					}
					else
					{
						c = sfvalue(iop);
						m = (cp = sfreserve(iop,c,SF_LOCKR)) ? sfvalue(iop) : 0;
					}
				}
				if(m>0 && (flags&N_FLAG) && !binary && (v=memchr(cp,'\n',m)))
				{
					*v++ = 0;
					m = v-(char*)cp;
				}
				if((c=m)>size)
					c = size;
				if(c>0)
				{
					if(c > (end-cur))
					{
						ssize_t	cx = cur - var, ux = up - var;
						m = (end - var) + (c - (end - cur));
						if (var == buf)
						{
							v = (char*)malloc(m+1);
							var = memcpy(v, var, cur - var);
						}
						else
							var = newof(var, char, m, 1);
						end = var + m;
						cur = var + cx;
						up = var + ux;
					}
					if(cur!=(char*)cp)
						memcpy((void*)cur,cp,c);
					if(f)
						sfread(iop,cp,c);
					cur += c;
#if SHOPT_MULTIBYTE
					if(!binary && mbwide())
					{
						int	x;
						int	z;

						mbinit();
						*cur = 0;
						x = z = 0;
						while (up < cur && (z = mbsize(up)) > 0)
						{
							up += z;
							x++;
						}
						if((size -= x) > 0 && (up >= cur || z < 0) && ((flags & NN_FLAG) || z < 0 || m > c))
							continue;
					}
#endif
				}
#if SHOPT_MULTIBYTE
				if(!binary && mbwide() && (up == var || (flags & NN_FLAG) && size))
					cur = var;
#endif
				*cur = 0;
				if(c>=size || (flags&N_FLAG) || m==0)
				{
					if(m)
						sfclrerr(iop);
					break;
				}
				size -= c;
			}
		}
		if(timeslot)
			timerdel(timeslot);
		if(binary && !((size=nv_size(np)) && nv_isarray(np) && c!=size))
		{
			int optimize = !np->nvfun || !nv_hasdisc(np,&OPTIMIZE_disc);
			if(optimize && (c==size) && np->nvalue.cp && !nv_isarray(np))
				memcpy((char*)np->nvalue.cp,var,c);
			else
			{
				Namval_t *mp;
				if(var==buf)
					var = memdup(var,c+1);
				nv_putval(np,var,NV_RAW);
				nv_setsize(np,c);
				if(!nv_isattr(np,NV_IMPORT|NV_EXPORT)  && (mp=(Namval_t*)np->nvenv))
					nv_setsize(mp,c);
			}
		}
		else
		{
			nv_putval(np,var,0);
			if(var!=buf)
				free((void*)var);
		}
		goto done;
	}
	else if(cp = (unsigned char*)sfgetr(iop,delim,0))
		c = sfvalue(iop);
	else if(cp = (unsigned char*)sfgetr(iop,delim,-1))
	{
		c = sfvalue(iop)+1;
		if(!sferror(iop) && sfgetc(iop) >=0)
			errormsg(SH_DICT,ERROR_exit(1),e_overlimit,"line length");
	}
	if(timeslot)
		timerdel(timeslot);
	if((flags&S_FLAG) && !shp->gd->hist_ptr)
	{
		sh_histinit((void*)shp);
		if(!shp->gd->hist_ptr)
			flags &= ~S_FLAG;
	}
	if(cp)
	{
		cpmax = cp + c;
#if SHOPT_CRNL
		if(delim=='\n' && c>=2 && cpmax[-2]=='\r')
			cpmax--;
#endif /* SHOPT_CRNL */
		if(*(cpmax-1) != delim)
			*(cpmax-1) = delim;
		if(flags&S_FLAG)
			sfwrite(shp->gd->hist_ptr->histfp,(char*)cp,c);
		c = shp->ifstable[*cp++];
#if !SHOPT_MULTIBYTE
		if(!name && (flags&R_FLAG)) /* special case single argument */
		{
			/* skip over leading blanks */
			while(c==S_SPACE)
				c = shp->ifstable[*cp++];
			/* strip trailing delimiters */
			if(cpmax[-1] == '\n')
				cpmax--;
			if(cpmax>cp)
			{
				while((c=shp->ifstable[*--cpmax])==S_DELIM || c==S_SPACE);
				cpmax[1] = 0;
			}
			else
				*cpmax =0;
			if(nv_isattr(np, NV_RDONLY))
			{
				errormsg(SH_DICT,ERROR_warn(0),e_readonly, nv_name(np));
				jmpval = 1;
			}
			else
				nv_putval(np,(char*)cp-1,0);
			goto done;
		}
#endif /* !SHOPT_MULTIBYTE */
	}
	else
		c = S_NL;
	shp->nextprompt = 2;
	rel= stktell(shp->stk);
	/* val==0 at the start of a field */
	val = 0;
	del = 0;
	while(1)
	{
		switch(c)
		{
#if SHOPT_MULTIBYTE
		   case S_MBYTE:
			if(val==0)
				val = (char*)(cp-1);
			if(sh_strchr(ifs,(char*)cp-1,cpmax-cp+1)>=0)
			{
				c = mbsize((char*)cp-1);
				if(name)
					cp[-1] = 0;
				if(c>1)
					cp += (c-1);
				c = S_DELIM;
			}
			else
				c = 0;
			continue;
#endif /*SHOPT_MULTIBYTE */
		    case S_QUOTE:
			c = shp->ifstable[*cp++];
			if(inquote && c==S_QUOTE)
				c = -1;
			else
				inquote = !inquote;
			if(val)
			{
				sfputr(shp->stk,val,-1);
				use_stak = 1;
				*val = 0;
			}
			if(c== -1)
			{
				sfputc(shp->stk,'"');
				c = shp->ifstable[*cp++];
			}
			continue;
		    case S_ESC:
			/* process escape character */
			if((c = shp->ifstable[*cp++]) == S_NL)
				was_escape = 1;
			else
				c = 0;
			if(val)
			{
				sfputr(shp->stk,val,-1);
				use_stak = 1;
				was_escape = 1;
				*val = 0;
			}
			continue;

		    case S_ERR:
			cp++;
		    case S_EOF:
			/* check for end of buffer */
			if(val && *val)
			{
				sfputr(shp->stk,val,-1);
				use_stak = 1;
			}
			val = 0;
			if(cp>=cpmax)
			{
				c = S_NL;
				break;
			}
			/* eliminate null bytes */
			c = shp->ifstable[*cp++];
			if(!name && val && (c==S_SPACE||c==S_DELIM||c==S_MBYTE))
				c = 0;
			continue;
		    case S_NL:
			if(was_escape)
			{
				was_escape = 0;
				if(cp = (unsigned char*)sfgetr(iop,delim,0))
					c = sfvalue(iop);
				else if(cp=(unsigned char*)sfgetr(iop,delim,-1))
					c = sfvalue(iop)+1;
				if(cp)
				{
					if(flags&S_FLAG)
						sfwrite(shp->gd->hist_ptr->histfp,(char*)cp,c);
					cpmax = cp + c;
					c = shp->ifstable[*cp++];
					val=0;
					if(!name && (c==S_SPACE || c==S_DELIM || c==S_MBYTE))
						c = 0;
					continue;
				}
			}
			c = S_NL;
			break;

		    case S_SPACE:
			/* skip over blanks */
			while((c=shp->ifstable[*cp++])==S_SPACE);
			if(!val)
				continue;
#if SHOPT_MULTIBYTE
			if(c==S_MBYTE)
			{
				if(sh_strchr(ifs,(char*)cp-1,cpmax-cp+1)>=0)
				{
					if((c = mbsize((char*)cp-1))>1)
						cp += (c-1);
					c = S_DELIM;
				}
				else
					c = 0;
			}
#endif /* SHOPT_MULTIBYTE */
			if(c!=S_DELIM)
				break;
			/* FALL THRU */

		    case S_DELIM:
			if(!del)
				del = cp - 1;
			if(name)
			{
				/* skip over trailing blanks */
				while((c=shp->ifstable[*cp++])==S_SPACE);
				break;
			}
			/* FALL THRU */

		    case 0:
			if(val==0 || was_escape)
			{
				val = (char*)(cp-1);
				was_escape = 0;
			}
			/* skip over word characters */
			wrd = -1;
			while(1)
			{
				while((c=shp->ifstable[*cp++])==0)
					if(!wrd)
						wrd = 1;
				if(inquote)
				{
					if(c==S_QUOTE)
					{
						if(shp->ifstable[*cp]==S_QUOTE)
						{
							if(val)
							{
								sfwrite(shp->stk,val,cp-(unsigned char*)val);
								use_stak = 1;
							}
							val = (char*)++cp;
						}
						else
							break;
					}
					if(c && c!=S_EOF)
					{
						if(c==S_NL)
						{
							if(val)
							{
								sfwrite(shp->stk,val,cp-(unsigned char*)val);
								use_stak=1;
							}
							if(cp = (unsigned char*)sfgetr(iop,delim,0))
								c = sfvalue(iop);
							else if(cp = (unsigned char*)sfgetr(iop,delim,-1))
								c = sfvalue(iop)+1;
							val = (char*)cp;
						}
						continue;
					}
				}
				if(!del&&c==S_DELIM)
					del = cp - 1;
				if(name || c==S_NL || c==S_ESC || c==S_EOF || c==S_MBYTE)
					break;
				if(wrd<0)
					wrd = 0;
			}
			if(wrd>0)
				del = (unsigned char*)"";
			if(c!=S_MBYTE)
				cp[-1] = 0;
			continue;
		}
		/* assign value and advance to next variable */
		if(!val)
			val = "";
		if(use_stak)
		{
			sfputr(shp->stk,val,0);
			val = stkptr(shp->stk,rel);
		}
		if(!name && *val)
		{
			/* strip off trailing space delimiters */
			register unsigned char	*vp = (unsigned char*)val + strlen(val);
			while(shp->ifstable[*--vp]==S_SPACE);
			if(vp==del)
			{
				if(vp==(unsigned char*)val)
					vp--;
				else
					while(shp->ifstable[*--vp]==S_SPACE);
			}
			vp[1] = 0;
		}
		if(nv_isattr(np, NV_RDONLY))
		{
			errormsg(SH_DICT,ERROR_warn(0),e_readonly, nv_name(np));
			jmpval = 1;
		}
		else
			nv_putval(np,val,0);
		val = 0;
		del = 0;
		if(use_stak)
		{
			stkseek(shp->stk,rel);
			use_stak = 0;
		}
		if(array_index)
		{
			nv_putsub(np, NIL(char*), array_index++,0);
			if(c!=S_NL)
				continue;
			name = *++names;
		}
		while(1)
		{
			if(sh_isoption(shp,SH_ALLEXPORT)&&!strchr(nv_name(np),'.') && !nv_isattr(np,NV_EXPORT))
			{
				nv_onattr(np,NV_EXPORT);
				sh_envput(shp,np);
			}
			if(name)
			{
				nv_close(np);
				np = nv_open(name,shp->var_tree,NV_NOASSIGN|NV_VARNAME);
				name = *++names;
			}
			else
				np = 0;
			if(c!=S_NL)
				break;
			if(!np)
				goto done;
			if(nv_isattr(np, NV_RDONLY))
			{
				errormsg(SH_DICT,ERROR_warn(0),e_readonly, nv_name(np));
				jmpval = 1;
			}
			else
				nv_putval(np, "", 0);
		}
	}
done:
	if(timeout || (shp->fdstatus[fd]&(IOTTY|IONOSEEK)))
		sh_popcontext(shp,&buff);
	if(was_write)
		sfset(iop,SF_WRITE,1);
	if(!was_share)
		sfset(iop,SF_SHARE,0);
	nv_close(np);
	if((shp->fdstatus[fd]&IOTTY) && !keytrap)
		tty_cooked(sffileno(iop));
	if(flags&S_FLAG)
		hist_flush(shp->gd->hist_ptr);
	if(jmpval > 1)
		siglongjmp(*shp->jmplist,jmpval);
	return(jmpval);
}

