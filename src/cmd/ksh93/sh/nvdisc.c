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
 * AT&T Labs
 *
 */

#include        "defs.h"
#include        "variables.h"
#include        "builtins.h"
#include        "path.h"

static void assign(Namval_t*,const char*,int,Namfun_t*);

int nv_compare(Dt_t* dict, Void_t *sp, Void_t *dp, Dtdisc_t *disc)
{
	if(sp==dp)
		return(0);
	return(strcmp((char*)sp,(char*)dp));
}

/*
 * call the next getval function in the chain
 */
char *nv_getv(Namval_t *np, register Namfun_t *nfp)
{
	Shell_t	*shp = sh_ptr(np);
	register Namfun_t	*fp;
	register char *cp;
	if((fp = nfp) != NIL(Namfun_t*) && !nv_local)
		fp = nfp = nfp->next;
	nv_local=0;
	for(; fp; fp=fp->next)
	{
		if(!fp->disc || (!fp->disc->getnum && !fp->disc->getval))
			continue;
		if(!nv_isattr(np,NV_NODISC) || fp==(Namfun_t*)nv_arrayptr(np))
			break;
	}
	if(fp && fp->disc->getval)
		cp = (*fp->disc->getval)(np,fp);
	else if(fp && fp->disc->getnum)
	{
		sfprintf(shp->strbuf,"%.*Lg",12,(*fp->disc->getnum)(np,fp));
		cp = sfstruse(shp->strbuf);
	}
	else
	{
		nv_local=1;
		cp = nv_getval(np);
	}
	return(cp);
}

/*
 * call the next getnum function in the chain
 */
Sfdouble_t nv_getn(Namval_t *np, register Namfun_t *nfp)
{
	register Namfun_t	*fp;
	register Sfdouble_t	d=0;
	Shell_t			*shp = sh_ptr(np);
	char *str;
	if((fp = nfp) != NIL(Namfun_t*) && !nv_local)
		fp = nfp = nfp->next;
	nv_local=0;
	for(; fp; fp=fp->next)
	{
		if(!fp->disc || (!fp->disc->getnum && !fp->disc->getval))
			continue;
		if(!fp->disc->getnum && nv_isattr(np,NV_INTEGER))
			continue;
		if(!nv_isattr(np,NV_NODISC) || fp==(Namfun_t*)nv_arrayptr(np))
			break;
	}
	if(fp && fp->disc && fp->disc->getnum)
		d = (*fp->disc->getnum)(np,fp);
	else if(nv_isattr(np,NV_INTEGER))
	{
		nv_local = 1;
		d =  nv_getnum(np);
	}
	else
	{
		if(fp && fp->disc && fp->disc->getval)
			str = (*fp->disc->getval)(np,fp);
		else
			str = nv_getv(np,fp?fp:nfp);
		if(str && *str)
		{
			if(nv_isattr(np,NV_LJUST|NV_RJUST) || (*str=='0' && !(str[1]=='x'||str[1]=='X')))
			{
				while(*str=='0')
					str++;
			}
			d = sh_arith(shp,str);
		}
	}
	return(d);
}

/*
 * call the next assign function in the chain
 */
void nv_putv(Namval_t *np, const char *value, int flags, register Namfun_t *nfp)
{
	register Namfun_t	*fp, *fpnext;
	Namarr_t		*ap;
	if((fp=nfp) != NIL(Namfun_t*) && !nv_local)
		fp = nfp = nfp->next;
	nv_local=0;
	if(flags&NV_NODISC)
		fp = 0;
	for(; fp; fp=fpnext)
	{
		fpnext = fp->next;
		if(!fp->disc || !fp->disc->putval)
		{
			if(!value && (!(ap=nv_arrayptr(np)) || ap->nelem==0))
			{
				if(fp->disc || !(fp->nofree&1))
					nv_disc(np,fp,NV_POP);
				if(!(fp->nofree&1))
					free((void*)fp);
			}
			continue;
		}
		if(!nv_isattr(np,NV_NODISC) || fp==(Namfun_t*)nv_arrayptr(np))
			break;
	}
	if(!value && (flags&NV_TYPE) && fp && fp->disc->putval==assign)
		fp = 0;
	if(fp && fp->disc->putval)
		(*fp->disc->putval)(np,value, flags, fp);
	else
	{
		nv_local=1;
		if(value)
			nv_putval(np, value, flags);
		else
			_nv_unset(np, flags&(NV_RDONLY|NV_EXPORT));
	}
}

#define	LOOKUPS		0
#define	ASSIGN		1
#define	APPEND		2
#define	UNASSIGN	3
#define	LOOKUPN		4
#define BLOCKED		((Namval_t*)&nv_local)

struct	vardisc
{
	Namfun_t	fun;
	Namval_t	*disc[5];
};

struct blocked
{
	struct blocked	*next;
	Namval_t	*np;
	int		flags;
	void		*sub;
	int		isub;
};

static struct blocked	*blist;

#define isblocked(bp,type)	((bp)->flags & (1<<(type)))
#define block(bp,type)		((bp)->flags |= (1<<(type)))
#define unblock(bp,type)	((bp)->flags &= ~(1<<(type)))

/*
 * returns pointer to blocking structure
 */
static struct blocked *block_info(Namval_t *np, struct blocked *pp)
{
	register struct blocked	*bp;
	void			*sub=0;
	int			isub=0;
	if(nv_isarray(np) && (isub=nv_aindex(np)) < 0)
		sub = nv_associative(np,(const char*)0,NV_ACURRENT);
	for(bp=blist ; bp; bp=bp->next)
	{
		if(bp->np==np && bp->sub==sub && bp->isub==isub)
			return(bp);
	}
	if(pp)
	{
		pp->np = np;
		pp->flags = 0;
		pp->isub = isub;
		pp->sub = sub;
		pp->next = blist;
		blist = pp;
	}
	return(pp);
}

static void block_done(struct blocked *bp)
{
	blist = bp = bp->next;
	if(bp && (bp->isub>=0 || bp->sub))
		nv_putsub(bp->np, bp->sub,(bp->isub<0?0:bp->isub),ARRAY_SETSUB);
}

/*
 * free discipline if no more discipline functions
 */
static void chktfree(register Namval_t *np, register struct vardisc *vp)
{
	register int n;
	for(n=0; n< sizeof(vp->disc)/sizeof(*vp->disc); n++)
	{
		if(vp->disc[n])
			break;
	}
	if(n>=sizeof(vp->disc)/sizeof(*vp->disc))
	{
		/* no disc left so pop */
		Namfun_t *fp;
		if((fp=nv_stack(np, NIL(Namfun_t*))) && !(fp->nofree&1))
			free((void*)fp);
	}
}

/*
 * This function performs an assignment disc on the given node <np>
 */
static void	assign(Namval_t *np,const char* val,int flags,Namfun_t *handle)
{
	Shell_t		*shp = sh_ptr(np);
	int		type = (flags&NV_APPEND)?APPEND:ASSIGN;
	register	struct vardisc *vp = (struct vardisc*)handle;
	register	Namval_t *nq =  vp->disc[type];
	struct blocked	block, *bp = block_info(np, &block);
	Namval_t	node;
	union Value	*up = np->nvalue.up;
	Namval_t	*tp, *nr;
	if(val && (tp=nv_type(np)) && (nr=nv_open(val,shp->var_tree,NV_VARNAME|NV_ARRAY|NV_NOADD|NV_NOFAIL)) && tp==nv_type(nr)) 
	{
		char *sub = nv_getsub(np);
		_nv_unset(np,0);
		if(sub)
		{
			nv_putsub(np, sub, 0,ARRAY_ADD);
			nv_putval(np,nv_getval(nr), 0);
		}
		else
			nv_clone(nr,np,0);
		goto done;
	}
	if(val || isblocked(bp,type))
	{
		if(!nq || isblocked(bp,type))
		{
			nv_putv(np,val,flags,handle);
			goto done;
		}
		node = *SH_VALNOD;
		if(!nv_isnull(SH_VALNOD))
		{
			nv_onattr(SH_VALNOD,NV_NOFREE);
			_nv_unset(SH_VALNOD,0);
		}
		if(flags&NV_INTEGER)
			nv_onattr(SH_VALNOD,(flags&(NV_LONG|NV_DOUBLE|NV_EXPNOTE|NV_HEXFLOAT|NV_SHORT)));
		nv_putval(SH_VALNOD, val, (flags&NV_INTEGER)?flags:NV_NOFREE);
	}
	else
		nq =  vp->disc[type=UNASSIGN];
	if(nq && !isblocked(bp,type))
	{
		int bflag=0;
		block(bp,type);
		if (type==APPEND && (bflag= !isblocked(bp,LOOKUPS)))
			block(bp,LOOKUPS);
		sh_fun(shp,nq,np,(char**)0);
		unblock(bp,type);
		if(bflag)
			unblock(bp,LOOKUPS);
		if(!vp->disc[type])
			chktfree(np,vp);
	}
	if(nv_isarray(np))
		np->nvalue.up = up;
	if(val)
	{
		register char *cp;
		Sfdouble_t d;
		if(nv_isnull(SH_VALNOD))
			cp=0;
		else if(flags&NV_INTEGER)
		{
			d = nv_getnum(SH_VALNOD);
			cp = (char*)(&d);
			flags |= (NV_LONG|NV_DOUBLE);
			flags &= ~NV_SHORT;
		}
		else
			cp = nv_getval(SH_VALNOD);
		if(cp)
			nv_putv(np,cp,flags|NV_RDONLY,handle);
		_nv_unset(SH_VALNOD,0);
		/* restore everything but the nvlink field */
		memcpy(&SH_VALNOD->nvname,  &node.nvname, sizeof(node)-sizeof(node.nvlink));
	}
	else if(sh_isstate(shp,SH_INIT) || np==SH_FUNNAMENOD)
	{
		/* don't free functions during reinitialization */
		nv_putv(np,val,flags,handle);
	}
	else if(!nq || !isblocked(bp,type))
	{
		Dt_t *root = sh_subfuntree(shp,1);
		Namval_t	*pp=0;
		int n;
		Namarr_t *ap;
		block(bp,type);
		nv_disc(np,handle,NV_POP);
		if(!nv_isattr(np,NV_MINIMAL))
			pp = (Namval_t*)np->nvenv;
		nv_putv(np, val, flags, handle);
		if(shp->subshell)
			goto done;
		if(pp && nv_isarray(pp))
			goto done;
		if(nv_isarray(np) && (ap=nv_arrayptr(np)) && ap->nelem>0)
			goto done;
		for(n=0; n < sizeof(vp->disc)/sizeof(*vp->disc); n++)
		{
			if((nq=vp->disc[n]) && !nv_isattr(nq,NV_NOFREE))
			{
				_nv_unset(nq,0);
				dtdelete(root,nq);
			}
		}
		unblock(bp,type);
		if(!(handle->nofree&1))
			free(handle);
	}
done:
	if(bp== &block)
		block_done(bp);
	if(nq && nq->nvalue.rp->running==1)
	{
		nq->nvalue.rp->running=0;
		_nv_unset(nq,0);
	}
}

/*
 * This function executes a lookup disc and then performs
 * the lookup on the given node <np>
 */
static char*	lookup(Namval_t *np, int type, Sfdouble_t *dp,Namfun_t *handle)
{
	Shell_t			*shp = sh_ptr(np);
	register struct vardisc	*vp = (struct vardisc*)handle;
	struct blocked		block, *bp = block_info(np, &block);
	register Namval_t	*nq = vp->disc[type];
	register char		*cp=0;
	Namval_t		node;
	union Value		*up = np->nvalue.up;
	if(nq && !isblocked(bp,type))
	{
		node = *SH_VALNOD;
		if(!nv_isnull(SH_VALNOD))
		{
			nv_onattr(SH_VALNOD,NV_NOFREE);
			_nv_unset(SH_VALNOD,0);
		}
		if(type==LOOKUPN)
		{
			nv_onattr(SH_VALNOD,NV_DOUBLE|NV_INTEGER);
			nv_setsize(SH_VALNOD,10);
		}
		block(bp,type);
		sh_fun(shp,nq,np,(char**)0);
		unblock(bp,type);
		if(!vp->disc[type])
			chktfree(np,vp);
		if(type==LOOKUPN)
		{
			cp = (char*)(SH_VALNOD->nvalue.cp);
			*dp = nv_getnum(SH_VALNOD);
		}
		else if(cp = nv_getval(SH_VALNOD))
			cp = stkcopy(stkstd,cp);
		_nv_unset(SH_VALNOD,NV_RDONLY);
		if(!nv_isnull(&node))
		{
			/* restore everything but the nvlink field */
			memcpy(&SH_VALNOD->nvname,  &node.nvname, sizeof(node)-sizeof(node.nvlink));
		}
	}
	if(nv_isarray(np))
		np->nvalue.up = up;
	if(!cp)
	{
		if(type==LOOKUPS)
			cp = nv_getv(np,handle);
		else
			*dp = nv_getn(np,handle);
	}
	if(bp== &block)
		block_done(bp);
	if(nq && nq->nvalue.rp && nq->nvalue.rp->running==1)
	{
		nq->nvalue.rp->running=0;
		_nv_unset(nq,0);
	}
	return(cp);
}

static char*	lookups(Namval_t *np, Namfun_t *handle)
{
	return(lookup(np,LOOKUPS,(Sfdouble_t*)0,handle));
}

static Sfdouble_t lookupn(Namval_t *np, Namfun_t *handle)
{
	Sfdouble_t	d;
	lookup(np,LOOKUPN, &d ,handle);
	return(d);
}


/*
 * Set disc on given <event> to <action>
 * If action==np, the current disc is returned
 * A null return value indicates that no <event> is known for <np>
 * If <event> is NULL, then return the event name after <action>
 * If <event> is NULL, and <action> is NULL, return the first event
 */
char *nv_setdisc(register Namval_t* np,register const char *event,Namval_t *action,register Namfun_t *fp)
{
	register struct vardisc *vp = (struct vardisc*)np->nvfun;
	register int type;
	char *empty = "";
	while(vp)
	{
		if(vp->fun.disc && (vp->fun.disc->setdisc || vp->fun.disc->putval == assign))
			break;
		vp = (struct vardisc*)vp->fun.next;
	}
	if(vp && !vp->fun.disc)
		vp = 0;
	if(np == (Namval_t*)fp)
	{
		register const char *name;
		register int getname=0;
		/* top level call, check for get/set */
		if(!event)
		{
			if(!action)
				return((char*)nv_discnames[0]);
			getname=1;
			event = (char*)action;
		}
		for(type=0; name=nv_discnames[type]; type++)
		{
			if(strcmp(event,name)==0)
				break;
		}
		if(getname)
		{
			event = 0;
			if(name && !(name = nv_discnames[++type]))
				action = 0;
		}
		if(!name)
		{
			for(fp=(Namfun_t*)vp; fp; fp=fp->next)
			{
				if(fp->disc && fp->disc->setdisc)
					return((*fp->disc->setdisc)(np,event,action,fp));
			}
		}
		else if(getname)
			return((char*)name);
	}
	if(!fp)
		return(NIL(char*));
	if(np != (Namval_t*)fp)
	{
		/* not the top level */
		while(fp = fp->next)
		{
			if(fp->disc && fp->disc->setdisc)
				return((*fp->disc->setdisc)(np,event,action,fp));
		}
		return(NIL(char*));
	}
	/* Handle GET/SET/APPEND/UNSET disc */
	if(vp && vp->fun.disc->putval!=assign)
		vp = 0;
	if(!vp)
	{
		Namdisc_t	*dp;
		if(action==np)
			return((char*)action);
		if(!(vp = newof(NIL(struct vardisc*),struct vardisc,1,sizeof(Namdisc_t))))
			return(0);
		dp = (Namdisc_t*)(vp+1);
		vp->fun.disc = dp;
		memset(dp,0,sizeof(*dp));
		dp->dsize = sizeof(struct vardisc);
		dp->putval = assign;
		if(nv_isarray(np) && !nv_arrayptr(np))
			nv_putsub(np,(char*)0, 1,0);
		nv_stack(np, (Namfun_t*)vp);
	}
	if(action==np)
	{
		action = vp->disc[type];
		empty = 0;
	}
	else if(action)
	{
		Namdisc_t *dp = (Namdisc_t*)vp->fun.disc;
		if(type==LOOKUPS)
			dp->getval = lookups;
		else if(type==LOOKUPN)
			dp->getnum = lookupn;
		vp->disc[type] = action;
	}
	else
	{
		struct blocked *bp;
		action = vp->disc[type];
		vp->disc[type] = 0;
		if(!(bp=block_info(np,(struct blocked*)0)) || !isblocked(bp,UNASSIGN))
			chktfree(np,vp);
	}
	return(action?(char*)action:empty);
}

/*
 * Set disc on given <event> to <action>
 * If action==np, the current disc is returned
 * A null return value indicates that no <event> is known for <np>
 * If <event> is NULL, then return the event name after <action>
 * If <event> is NULL, and <action> is NULL, return the first event
 */
static char *setdisc(register Namval_t* np,register const char *event,Namval_t *action,register Namfun_t *fp)
{
	register Nambfun_t *vp = (Nambfun_t*)fp;
	register int type,getname=0;
	register const char *name;
	const char **discnames = vp->bnames;
	/* top level call, check for discipline match */
	if(!event)
	{
		if(!action)
			return((char*)discnames[0]);
		getname=1;
		event = (char*)action;
	}
	for(type=0; name=discnames[type]; type++)
	{
		if(strcmp(event,name)==0)
			break;
	}
	if(getname)
	{
		event = 0;
		if(name && !(name = discnames[++type]))
			action = 0;
	}
	if(!name)
		return(nv_setdisc(np,event,action,fp));
	else if(getname)
		return((char*)name);
	/* Handle the disciplines */
	if(action==np)
		action = vp->bltins[type];
	else if(action)
	{
		Namval_t *tp = nv_type(np);
		if(tp && (np = (Namval_t*)vp->bltins[type]) && nv_isattr(np,NV_STATICF))
			errormsg(SH_DICT,ERROR_exit(1),e_staticfun,name,tp->nvname);
		vp->bltins[type] = action;
	}
	else
	{
		action = vp->bltins[type];
		vp->bltins[type] = 0;
	}
	return(action?(char*)action:"");
}

static void putdisc(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	Shell_t	*shp = sh_ptr(np);
	nv_putv(np,val,flag,fp);
	if(!val && !(flag&NV_NOFREE))
	{
		register Nambfun_t *vp = (Nambfun_t*)fp;
		register int i;
		for(i=0; vp->bnames[i]; i++)
		{
			register Namval_t *mp;
			if((mp=vp->bltins[i]) && !nv_isattr(mp,NV_NOFREE))
			{
				if(is_abuiltin(mp))
				{
					if(mp->nvfun && !nv_isattr(mp,NV_NOFREE))
						free((void*)mp->nvfun);
					dtdelete(shp->bltin_tree,mp);
					free((void*)mp);
				}
			}
		}
		nv_disc(np,fp,NV_POP);
		if(!(fp->nofree&1))
			free((void*)fp);
			
	}
}

static const Namdisc_t Nv_bdisc	= {   0, putdisc, 0, 0, setdisc };

Namfun_t *nv_clone_disc(register Namfun_t *fp, int flags)
{
	register Namfun_t	*nfp;
	register int		size;
	if(!fp->disc && !fp->next && (fp->nofree&1))
		return(fp);
	if(!(size=fp->dsize) && (!fp->disc || !(size=fp->disc->dsize)))
		size = sizeof(Namfun_t);
	if(!(nfp=newof(NIL(Namfun_t*),Namfun_t,1,size-sizeof(Namfun_t))))
		return(0);
	memcpy(nfp,fp,size);
	nfp->nofree &= ~1;
	nfp->nofree |= (flags&NV_RDONLY)?1:0;
	return(nfp);
}

bool nv_adddisc(Namval_t *np, const char **names, Namval_t **funs)
{
	register Nambfun_t *vp;
	register int n=0;
	register const char **av=names;
	if(av)
	{
		while(*av++)
			n++;
	}
	if(!(vp = newof(NIL(Nambfun_t*),Nambfun_t,1,n*sizeof(Namval_t*))))
		return(false);
	vp->fun.dsize = sizeof(Nambfun_t)+n*sizeof(Namval_t*);
	vp->fun.nofree |= 2;
	vp->num = n;
	if(funs)
		memcpy((void*)vp->bltins, (void*)funs,n*sizeof(Namval_t*));
	else while(n>=0)
		vp->bltins[n--] = 0;
	vp->fun.disc = &Nv_bdisc;
	vp->bnames = names; 
	nv_stack(np,&vp->fun);
	return(true);
}

/*
 * push, pop, clne, or reorder disciplines onto node <np>
 * mode can be one of
 *    NV_FIRST:  Move or push <fp> to top of the stack or delete top
 *    NV_LAST:	 Move or push <fp> to bottom of stack or delete last
 *    NV_POP:	 Delete <fp> from top of the stack
 *    NV_CLONE:  Replace fp with a copy created my malloc() and return it
 */
Namfun_t *nv_disc(register Namval_t *np, register Namfun_t* fp, int mode)
{
	Shell_t	*shp = sh_ptr(np);
	Namfun_t *lp, **lpp;
	if(nv_isref(np))
		return(0);
	if(mode==NV_CLONE && !fp)
		return(0);
	if(fp)
	{
		fp->subshell = shp->subshell;
		if((lp=np->nvfun)==fp)
		{
			if(mode==NV_CLONE)
			{
				lp = nv_clone_disc(fp,0);
				return(np->nvfun=lp);
			}
			if(mode==NV_FIRST || mode==0)
				return(fp);
			np->nvfun = lp->next;
			if(mode==NV_POP)
				return(fp);
			if(mode==NV_LAST && (lp->next==0 || lp->next->disc==0))
				return(fp);
		}
		/* see if <fp> is on the list already */
		lpp = &np->nvfun;
		if(lp)
		{
			while(lp->next && lp->next->disc)
			{
				if(lp->next==fp)
				{
					if(mode==NV_LAST && fp->next==0)
						return(fp);
					if(mode==NV_CLONE)
					{
						fp = nv_clone_disc(fp,0);
						lp->next = fp;
						return(fp);
					}
					lp->next = fp->next;
					if(mode==NV_POP)
						return(fp);
					if(mode!=NV_LAST)
						break;
				}
				lp = lp->next;
			}
			if(mode==NV_LAST && lp->disc)
				lpp = &lp->next;
		}
		if(mode==NV_POP)
			return(0);
		/* push */
		nv_offattr(np,NV_NODISC);
		if(mode==NV_LAST)
		{
			if(lp && !lp->disc)
				fp->next = lp;
			else
				fp->next = 0;
		}
		else
		{
			if((fp->nofree&1) && *lpp)
				fp = nv_clone_disc(fp,0);
			fp->next = *lpp;
		}
		*lpp = fp;
	}
	else
	{
		if(mode==NV_FIRST)
			return(np->nvfun);
		else if(mode==NV_LAST)
			for(lp=np->nvfun; lp; fp=lp,lp=lp->next);
		else if(fp = np->nvfun)
			np->nvfun = fp->next;
	}
	return(fp);
}

/*
 * returns discipline pointer if discipline with specified functions
 * is on the discipline stack
 */
Namfun_t *nv_hasdisc(Namval_t *np, const Namdisc_t *dp)
{
	register Namfun_t *fp;
	for(fp=np->nvfun; fp; fp = fp->next)
	{
		if(fp->disc== dp)
			return(fp);
	}
	return(0);
}

struct notify
{
	Namfun_t	hdr;
	char		**ptr;
};

static void put_notify(Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	struct notify *pp = (struct notify*)fp;
	nv_putv(np,val,flags,fp);
	nv_stack(np,fp);
	nv_stack(np,(Namfun_t*)0);
	*pp->ptr = 0;
	if(!(fp->nofree&1))
		free((void*)fp);
}

static const Namdisc_t notify_disc  = {  0, put_notify };

bool nv_unsetnotify(Namval_t *np, char **addr)
{
	register Namfun_t *fp;
	for(fp=np->nvfun;fp;fp=fp->next)
	{
		if(fp->disc->putval==put_notify && ((struct notify*)fp)->ptr==addr)
		{
			nv_stack(np,fp);
			nv_stack(np,(Namfun_t*)0);
			if(!(fp->nofree&1))
				free((void*)fp);
			return(true);
		}
	}
	return(false);
}

bool nv_setnotify(Namval_t *np, char **addr)
{
	struct notify *pp = newof(0,struct notify, 1,0);
	if(!pp)
		return(false);
	pp->ptr = addr;
	pp->hdr.disc = &notify_disc;
	nv_stack(np,&pp->hdr);
	return(true);
}

static void *newnode(const char *name)
{
	register size_t s;
	register Namval_t *np = newof(0,Namval_t,1,s=strlen(name)+1);
	if(np)
	{
		np->nvname = (char*)np+sizeof(Namval_t);
		memcpy(np->nvname,name,s);
	}
	return((void*)np);
}

/*
 * clone a numeric value
 */
static void *num_clone(register Namval_t *np, void *val)
{
	register int size;
	void *nval;
	if(!val)
		return(0);
	if(nv_isattr(np,NV_DOUBLE)==NV_DOUBLE)
	{
		if(nv_isattr(np,NV_LONG))
			size = sizeof(Sfdouble_t);
		else if(nv_isattr(np,NV_SHORT))
			size = sizeof(float);
		else
			size = sizeof(double);
	}
	else
	{
		if(nv_isattr(np,NV_LONG))
			size = sizeof(Sflong_t);
		else if(nv_isattr(np,NV_SHORT))
		{
			if(nv_isattr(np,NV_INT16P|NV_DOUBLE)==NV_INT16P)
				size = sizeof(short);
			else
				return((void*)np->nvalue.ip);
		}
		else
			size = sizeof(int32_t);
	}
	if(!(nval = malloc(size)))
		return(0);
	memcpy(nval,val,size);
	return(nval);
}

void clone_all_disc( Namval_t *np, Namval_t *mp, int flags)
{
	register Namfun_t *fp, **mfp = &mp->nvfun, *nfp, *fpnext;
	for(fp=np->nvfun; fp;fp=fpnext)
	{
		fpnext = fp->next;
		if(!fpnext && (flags&NV_COMVAR) && fp->disc && fp->disc->namef)
			return;
		if((fp->nofree&2) && (flags&NV_NODISC))
			nfp = 0;
		if(fp->disc && fp->disc->clonef)
			nfp = (*fp->disc->clonef)(np,mp,flags,fp);
		else	if(flags&NV_MOVE)
			nfp = fp;
		else
			nfp = nv_clone_disc(fp,flags);
		if(!nfp)
			continue;
		nfp->next = 0;
		*mfp = nfp;
		mfp = &nfp->next;
	}
}

/*
 * clone <mp> from <np> flags can be one of the following
 * NV_APPEND - append <np> onto <mp>
 * NV_MOVE - move <np> to <mp>
 * NV_NOFREE - mark the new node as nofree
 * NV_NODISC - discplines with funs non-zero will not be copied
 * NV_COMVAR - cloning a compound variable
 */
int nv_clone(Namval_t *np, Namval_t *mp, int flags)
{
	Namfun_t	*fp, *fpnext;
	const char	*val = mp->nvalue.cp;
	unsigned short	flag = mp->nvflag;
	size_t		size = nv_size(mp);
	mp->nvshell = np->nvshell;
	for(fp=mp->nvfun; fp; fp=fpnext)
	{
		fpnext = fp->next;
		if(!fpnext && (flags&NV_COMVAR) && fp->disc && fp->disc->namef)
			break;
		if(!(fp->nofree&1))
			free((void*)fp);
	}
	mp->nvfun = fp;
	if(fp=np->nvfun)
	{
		Shell_t	*shp = (Shell_t*)np->nvshell;
		Namval_t *last_table = shp->last_table;
		if(nv_isattr(mp,NV_EXPORT|NV_MINIMAL) == (NV_EXPORT|NV_MINIMAL))
		{
			mp->nvenv = 0;
			nv_offattr(mp,NV_MINIMAL);
		}
		if(!(flags&NV_COMVAR) && !nv_isattr(np,NV_MINIMAL) && np->nvenv && !(nv_isattr(mp,NV_MINIMAL)))
			mp->nvenv = np->nvenv;
		mp->nvflag &= NV_MINIMAL;
	        mp->nvflag |= np->nvflag&~(NV_ARRAY|NV_MINIMAL|NV_NOFREE);
		flag = mp->nvflag;
		clone_all_disc(np, mp, flags);
		shp->last_table = last_table;
	}
	if(flags&NV_APPEND)
		return(1);
	if(nv_size(mp) == size)
	        nv_setsize(mp,nv_size(np));
	if(mp->nvflag == flag)
	        mp->nvflag = (np->nvflag&~(NV_MINIMAL))|(mp->nvflag&NV_MINIMAL);
		if(nv_isattr(np,NV_EXPORT))
			mp->nvflag |= (np->nvflag&NV_MINIMAL);
	if(mp->nvalue.cp==val && !nv_isattr(np,NV_INTEGER))
	{
		if(np->nvalue.cp && np->nvalue.cp!=Empty && (flags&NV_COMVAR) && !(flags&NV_MOVE))
		{
			if(size)
				mp->nvalue.cp = (char*)memdup(np->nvalue.cp,size);
			else
			        mp->nvalue.cp = strdup(np->nvalue.cp);
			nv_offattr(mp,NV_NOFREE);
		}
		else if((np->nvfun || !nv_isattr(np,NV_ARRAY)) && !(mp->nvalue.cp = np->nvalue.cp))
			nv_offattr(mp,NV_NOFREE);
	}
	mp->nvshell = np->nvshell;
	if(flags&NV_MOVE)
	{
		if(nv_isattr(np,NV_INTEGER))
			mp->nvalue.ip = np->nvalue.ip;
		np->nvfun = 0;
		np->nvalue.cp = 0;
		if(!nv_isattr(np,NV_MINIMAL) || nv_isattr(mp,NV_EXPORT))
		{
			mp->nvenv = np->nvenv;
			if(nv_isattr(np,NV_MINIMAL))
			{
				np->nvenv = 0;
				np->nvflag = NV_EXPORT;
			}
			else
				np->nvflag = 0;
		}
		else
			np->nvflag &= NV_MINIMAL;
		return(1);
	}
	else if((flags&NV_ARRAY) && !nv_isattr(np,NV_MINIMAL))
		mp->nvenv = np->nvenv;
	if(nv_isattr(np,NV_INTEGER) && mp->nvalue.ip!=np->nvalue.ip && np->nvalue.cp!=Empty)
	{
		mp->nvalue.ip = (int*)num_clone(np,(void*)np->nvalue.ip);
		nv_offattr(mp,NV_NOFREE);
	}
	else if((flags&NV_NOFREE) && !nv_arrayptr(np))
	        nv_onattr(np,NV_NOFREE);
	return(1);
}

/*
 *  The following discipline is for copy-on-write semantics
 */
static char* clone_getv(Namval_t *np, Namfun_t *handle)
{
	return(np->nvalue.np?nv_getval(np->nvalue.np):0);
}

static Sfdouble_t clone_getn(Namval_t *np, Namfun_t *handle)
{
	return(np->nvalue.np?nv_getnum(np->nvalue.np):0);
}

static void clone_putv(Namval_t *np,const char* val,int flags,Namfun_t *handle)
{
	Shell_t	*shp = sh_ptr(np);
	Namfun_t *dp = nv_stack(np,(Namfun_t*)0);
	Namval_t *mp = np->nvalue.np;
	if(!shp->subshell)
		free((void*)dp);
	if(val)
		nv_clone(mp,np,NV_NOFREE);
	np->nvalue.cp = 0;
	nv_putval(np,val,flags);
}

static const Namdisc_t clone_disc =
{
	0,
	clone_putv,
	clone_getv,
	clone_getn
};

Namval_t *nv_mkclone(Namval_t *mp)
{
	Shell_t *shp = sh_ptr(mp);
	Namval_t *np;
	Namfun_t *dp;
	np = newof(0,Namval_t,1,0);
	np->nvflag = mp->nvflag;
	np->nvsize = mp->nvsize;
	np->nvname = mp->nvname;
	np->nvshell = mp->nvshell;
	np->nvalue.np = mp;
	np->nvflag = mp->nvflag;
	dp = newof(0,Namfun_t,1,0);
	dp->disc = &clone_disc;
	nv_stack(np,dp);
	dtinsert(nv_dict(shp->namespace),np);
	return(np);
}

Namval_t *nv_search(const char *name, Dt_t *root, int mode)
{
	Shell_t *shp = dtuserdata(root,0,0);
	register Namval_t *np;
	register Dt_t *dp = 0;
	if(mode&HASH_NOSCOPE)
		dp = dtview(root,0);
	if(mode&HASH_BUCKET)
	{
		Namval_t *mp = (void*)name;
		if(!(np = dtsearch(root,mp)) && (mode&NV_ADD))
			name = nv_name(mp);
	}
	else
	{
		if(*name=='.' && root==shp->var_tree && !dp)
			root = shp->var_base;
		np = dtmatch(root,(void*)name);
	}
#if SHOPT_COSHELL
	if(shp->inpool)
		mode |= HASH_NOSCOPE;
#endif /* SHOPT_COSHELL */
	if(!np && (mode&NV_ADD))
	{
		if(shp->namespace && !(mode&HASH_NOSCOPE) && root==shp->var_tree)
			root = nv_dict(shp->namespace);
		else if(!dp && !(mode&HASH_NOSCOPE))
		{
			register Dt_t *next;
			while(next=dtvnext(root))
				root = next;
		}
		np = (Namval_t*)dtinsert(root,newnode(name));
		np->nvshell = dtuserdata(root,0,0);
	}
	if(dp)
		dtview(root,dp);
	return(np);
}

/*
 * finds function or builtin for given name and the discipline variable
 * if var!=0 the variable pointer is returned and the built-in name
 *    is put onto the stack at the current offset.
 * otherwise, a pointer to the builtin (variable or type) is returned
 * and var contains the poiner to the variable
 * if last==0 and first component of name is a reference, nv_bfsearch()
	will return 0.
 */ 
Namval_t *nv_bfsearch(const char *name, Dt_t *root, Namval_t **var, char **last)
{
	Shell_t		*shp = dtuserdata(root,0,0);
	int		c,offset = stktell(shp->stk);
	register char	*sp, *cp=0;
	Namval_t	*np, *nq;
	char		*dname=0;
	if(var)
		*var = 0;
	/* check for . in the name before = */
	for(sp=(char*)name+1; *sp; sp++) 
	{
		if(*sp=='=')
			return(0);
		if(*sp=='[')
		{
			while(*sp=='[')
			{
				sp = nv_endsubscript((Namval_t*)0,(char*)sp,0,(void*)shp);
				if(sp[-1]!=']')
					return(0);
			}
			if(*sp==0)
				break;
			if(*sp!='.')
				return(0);
			cp = sp;
		}
		else if(*sp=='.')
			cp = sp; 
	}
	if(!cp)
	{
		if(!var)
			return(0);
#if SHOPT_NAMESPACE
		if(shp->namespace)
		{
			sfprintf(shp->strbuf,"%s.%s%c",nv_name(shp->namespace),name,0);
			if(np = nv_search(sfstruse(shp->strbuf),root,0))
				return(np);
		}
#endif /*SHOPT_NAMESPACE */
		return(nv_search(name,root,0));
	}
	sfputr(shp->stk,name,0);
	dname = cp+1;
	cp = stkptr(shp->stk,offset) + (cp-name); 
	if(last)
		*last = cp;
	c = *cp;
	*cp = 0;
	nq=nv_open(stkptr(shp->stk,offset),shp->var_tree,NV_VARNAME|NV_NOASSIGN|NV_NOADD|NV_NOFAIL);
	*cp = c;
	if(!nq)
	{
		np = 0;
		goto done;
	}
	if(!var)
	{
		np = nq;
		goto done;
	}
	*var = nq;
	if(c=='[')
		nv_endsubscript(nq, cp,NV_NOADD,nq->nvshell);
	stkseek(shp->stk,offset);
#if SHOPT_NAMESPACE
	if(nv_istable(nq))
	{
		Namval_t *nsp = shp->namespace;
		if(last==0)
			return(nv_search(name,root,0));
		shp->namespace = 0;
		sfputr(shp->stk,nv_name(nq),-1);
		shp->namespace = nsp;
		sfputr(shp->stk,dname-1,0);
		np = nv_search(stkptr(shp->stk,offset),root,0);
		stkseek(shp->stk,offset);
		return(np);
	}
#endif /* SHOPT_NAMESPACE */
	while(nv_isarray(nq) && !nv_isattr(nq,NV_MINIMAL|NV_EXPORT) && nq->nvenv && nv_isarray((Namval_t*)nq->nvenv))
		nq = (Namval_t*)nq->nvenv;
	return((Namval_t*)nv_setdisc(nq,dname,nq,(Namfun_t*)nq));
done:
	stkseek(shp->stk,offset);
	return(np);
}

/*
 * add or replace built-in version of command corresponding to <path>
 * The <bltin> argument is a pointer to the built-in
 * if <extra>==1, the built-in will be deleted
 * Special builtins cannot be added or deleted return failure
 * The return value for adding builtins is a pointer to the node or NULL on
 *   failure.  For delete NULL means success and the node that cannot be
 *   deleted is returned on failure.
 */
Namval_t *sh_addbuiltin_20120720(Shell_t* shp,const char *path, Shbltin_f bltin, void *extra)
{
	register const char	*name;
	char			*cp;
	register Namval_t	*np, *nq=0;
	int			offset=stktell(shp->stk);
	if(extra==(void*)1)
		name = path;
	else if((name = path_basename(path))==path && bltin!=(Shbltin_f)SYSTYPESET->nvalue.bfp && (nq=nv_bfsearch(name,shp->bltin_tree,(Namval_t**)0,&cp)))
		path = name = stkptr(shp->stk,offset);
	else if(shp->bltin_dir && extra!=(void*)1)
	{
		sfputr(shp->stk,shp->bltin_dir,'/');
		sfputr(shp->stk,name,0);
		path = stkptr(shp->stk,offset);
	}
	if(np = nv_search(name,shp->bltin_tree,0))
	{
		/* exists without a path */
		stkseek(shp->stk,offset);
		if(extra == (void*)1)
		{
			if(nv_isattr(np,BLT_SPC))
				errormsg(SH_DICT,ERROR_exit(1),"Cannot delete: %s%s",name,is_spcbuiltin);
			if(np->nvfun && !nv_isattr(np,NV_NOFREE))
				free((void*)np->nvfun);
			dtdelete(shp->bltin_tree,np);
			return(0);
		}
		else if(extra == (void*)2)
		{
			nv_onattr(np,BLT_DISABLE);
			return(0);
		}
		if(!bltin)
			return(np);
	}
	else for(np=(Namval_t*)dtfirst(shp->bltin_tree);np;np=(Namval_t*)dtnext(shp->bltin_tree,np))
	{
		if(strcmp(name,path_basename(nv_name(np))))
			continue;
		/* exists probably with different path so delete it */
		if(strcmp(path,nv_name(np)))
		{
			if(nv_isattr(np,BLT_SPC))
				return(np);
			if(!bltin)
				bltin = (Shbltin_f)np->nvalue.bfp;
			if(extra == (void*)1)
			{
				dtdelete(shp->bltin_tree,np);
				return(0);
			}
			np = 0;
		}
		break;
	}
	if(!np && !(np = nv_search(path,shp->bltin_tree,bltin?NV_ADD:0)))
		return(0);
	stkseek(shp->stk,offset);
	if(nv_isattr(np,BLT_SPC))
	{
		if(extra)
			np->nvfun = (Namfun_t*)extra;
		return(np);
	}
	np->nvenv = 0;
	np->nvfun = 0;
	if(bltin)
	{
		np->nvalue.bfp = (Nambfp_f)bltin;
		nv_onattr(np,NV_BLTIN|NV_NOFREE);
		np->nvfun = (Namfun_t*)extra;
	}
	if(nq)
	{
		cp=nv_setdisc(nq,cp+1,np,(Namfun_t*)nq);
		nv_close(nq);
		if(!cp)
			errormsg(SH_DICT,ERROR_exit(1),e_baddisc,name);
	}
	if(extra == (void*)1)
		return(0);
	return(np);
}
#undef sh_addbuiltin
Namval_t *sh_addbuiltin(const char *path, Shbltin_f bltin, void *extra)
{
	return(sh_addbuiltin_20120720(sh_getinterp(),path,bltin,extra));
}

#undef nv_stack
extern Namfun_t *nv_stack(register Namval_t *np, register Namfun_t* fp)
{
	return(nv_disc(np,fp,0));
}

struct table
{
	Namfun_t	fun;
	Namval_t	*parent;
	Shell_t		*shp;
	Dt_t		*dict;
};

static Namval_t *next_table(register Namval_t* np, Dt_t *root,Namfun_t *fp)
{
	struct table *tp = (struct table *)fp;
	if(root)
		return((Namval_t*)dtnext(root,np));
	else
		return((Namval_t*)dtfirst(tp->dict));
}

static Namval_t *create_table(Namval_t *np,const char *name,int flags,Namfun_t *fp)
{
	struct table *tp = (struct table *)fp;
	tp->shp->last_table = np;
	return(nv_create(name, tp->dict, flags, fp));
}

static Namfun_t *clone_table(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	struct table	*tp = (struct table*)fp;
	struct table	*ntp = (struct table*)nv_clone_disc(fp,0);
	Dt_t		*oroot=tp->dict,*nroot=dtopen(&_Nvdisc,Dtoset);
	if(!nroot)
		return(0);
	dtuserdata(nroot,dtuserdata(oroot,0,0),1);
	memcpy((void*)ntp,(void*)fp,sizeof(struct table));
	ntp->dict = nroot;
	ntp->parent = nv_lastdict(mp->nvshell);
	for(np=(Namval_t*)dtfirst(oroot);np;np=(Namval_t*)dtnext(oroot,np))
	{
		mp = (Namval_t*)dtinsert(nroot,newnode(np->nvname));
		mp->nvshell = dtuserdata(nroot,0,0);
		nv_clone(np,mp,flags);
	}
	return(&ntp->fun);
}

struct adata
{
	Shell_t		*sh;
	Namval_t	*tp;
	char		*mapname;
	char		**argnam;
	int		attsize;
	char		*attval;
};

static void delete_fun(Namval_t *np, void *data)
{
	Shell_t *shp = ((struct adata*)data)->sh;
	nv_delete(np,shp->fun_tree,NV_NOFREE);
}

static void put_table(register Namval_t* np, const char* val, int flags, Namfun_t* fp)
{
	register Dt_t		*root = ((struct table*)fp)->dict;
	register Namval_t	*nq, *mp;
	Namarr_t		*ap;
	struct adata		data;
	if(val)
	{
		nv_putv(np,val,flags,fp);
		return;
	}
	if(nv_isarray(np) && (ap=nv_arrayptr(np)) && array_elem(ap))
		return;
	memset(&data,0,sizeof(data));
	data.mapname = nv_name(np);
	data.sh = ((struct table*)fp)->shp;
	nv_scan(data.sh->fun_tree,delete_fun,(void*)&data,NV_FUNCTION,NV_FUNCTION|NV_NOSCOPE);
	dtview(root,0);
	for(mp=(Namval_t*)dtfirst(root);mp;mp=nq)
	{
		_nv_unset(mp,flags);
		nq = (Namval_t*)dtnext(root,mp);
		dtdelete(root,mp);
		free((void*)mp);
	}
	dtclose(root);
	if(!(fp->nofree&1))
		free((void*)fp);
	np->nvfun = 0;
}

/*
 * return space separated list of names of variables in given tree
 */
static char *get_table(Namval_t *np, Namfun_t *fp)
{
	register Dt_t *root = ((struct table*)fp)->dict;
	static Sfio_t *out;
	register int first=1;
	register Dt_t *base = dtview(root,0);
        if(out)
                sfseek(out,(Sfoff_t)0,SEEK_SET);
        else
                out =  sfnew((Sfio_t*)0,(char*)0,-1,-1,SF_WRITE|SF_STRING);
	for(np=(Namval_t*)dtfirst(root);np;np=(Namval_t*)dtnext(root,np))
	{
                if(!nv_isnull(np) || np->nvfun || nv_isattr(np,~NV_NOFREE))
		{
			if(!first)
				sfputc(out,' ');
			else
				first = 0;
			sfputr(out,np->nvname,-1);
		}
	}
	sfputc(out,0);
	if(base)
		dtview(root,base);
	return((char*)out->_data);
}

static const Namdisc_t table_disc =
{
        sizeof(struct table),
        put_table,
        get_table,
        0,
        0,
        create_table,
        clone_table,
        0,
        next_table,
};

Namval_t *nv_parent(Namval_t *np)
{
	struct table *tp = (struct table *)nv_hasdisc(np,&table_disc);
	if(tp)
		return(tp->parent);
	return(0);
}

Dt_t *nv_dict(Namval_t* np)
{
	Shell_t 	*shp=sh_ptr(np);
	struct table *tp = (struct table*)nv_hasdisc(np,&table_disc);
	if(tp)
		return(tp->dict);
	np = shp->last_table;
	while(np)
	{
		if(tp = (struct table*)nv_hasdisc(np,&table_disc))
			return(tp->dict);
#if 0
		np = nv_create(np,(const char*)0, NV_FIRST, (Namfun_t*)0);
#else
		break;
#endif
	}
	return(shp->var_tree);
}

bool nv_istable(Namval_t *np)
{
	return(nv_hasdisc(np,&table_disc)!=0);
}

/*
 * create a mountable name-value pair tree
 */
Namval_t *nv_mount(Namval_t *np, const char *name, Dt_t *dict)
{
	Namval_t *mp, *pp;
	struct table *tp;
	dtuserdata(dict,sh_ptr(np),1);
	if(nv_hasdisc(np,&table_disc))
		pp = np;
	else
		pp = nv_lastdict(np->nvshell);
	if(!(tp = newof((struct table*)0, struct table,1,0)))
		return(0);
	if(name)
	{
		Namfun_t *fp = pp->nvfun;
		mp = (*fp->disc->createf)(pp,name,0,fp);
	}
	else
		mp = np;
	nv_offattr(mp,NV_TABLE);
	if(!nv_isnull(mp))
		_nv_unset(mp,NV_RDONLY);
	tp->shp = sh_ptr(np);
	tp->dict = dict;
	tp->parent = pp;
	tp->fun.disc = &table_disc;
	nv_disc(mp, &tp->fun, NV_FIRST);
	return(mp);
}

const Namdisc_t *nv_discfun(int which)
{
	switch(which)
	{
	    case NV_DCADD:
		return(&Nv_bdisc);
	    case NV_DCRESTRICT:
		return(&RESTRICTED_disc);
	}
	return(0);
}

bool nv_hasget(Namval_t *np)
{
	register Namfun_t	*fp;
	for(fp=np->nvfun; fp; fp=fp->next)
	{
		if(!fp->disc || (!fp->disc->getnum && !fp->disc->getval))
			continue;
		return(true);
	}
	return(false);
}

#if SHOPT_NAMESPACE
Namval_t *sh_fsearch(Shell_t *shp, const char *fname, int add)
{
	Stk_t	*stkp = shp->stk;
	int	offset = stktell(stkp);
	sfputr(stkp,nv_name(shp->namespace),'.');
	sfputr(stkp,fname,0);
	fname = stkptr(stkp,offset);
	return(nv_search(fname,sh_subfuntree(shp,add&NV_ADD),add));
}
#endif /* SHOPT_NAMESPACE */
