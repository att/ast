/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2012 AT&T Intellectual Property          *
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
 * dss type library
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#include	<shell.h>
#include	<dsslib.h>

#ifndef SH_DICT
#   define SH_DICT	"libshell"
#endif

#define vnode(np)	((Cxvariable_t*)((np)->nvname))

static int match(int, char *[], Shbltin_t *);
static Namval_t *create_dss(Namval_t*,const char*,int,Namfun_t*);
static Namval_t *create_type(Namval_t*,const char*,int,Namfun_t*);

static const Namdisc_t parent_disc;
static Dt_t		*typedict;
static Dssdisc_t	Dssdisc;
static size_t		buflen;

struct Optdisc
{
	Dssoptdisc_t	dss;
	Namval_t	*np;
};

typedef struct Namtype Namtype_t;
typedef struct Namchld
{
	Namfun_t	fun;
	Namtype_t	*ptype;
	Namtype_t	*ttype;
} Namchld_t;

struct type
{
	Namfun_t	fun;
	Cxtype_t	*type;
	Cx_t		*cx;
	Namval_t	*bltins[1];
	Namfun_t	*pfun;
	Namval_t	details;
};

struct Namtype
{
	Namfun_t	fun;
	Shell_t		*sh;
	Namval_t	*np;
	Namval_t	*parent;
	char		*nodes;
	char		*data;
	Namchld_t	childfun;
	int		numnodes;
	char		**names;
};

struct dsstype
{
	Namfun_t	fun;
	Shell_t		*sh;
	char		**names;
	int		nnames;
};

struct dssfile
{
	Sfdisc_t	disc;
	Dssfile_t	*fp;
	Dssrecord_t	*rp;
	Namval_t	*np;
	Namval_t	*mp;
	char		*dp;
};

struct query
{
	struct query	*next;
	Cxexpr_t	expr;
	Cxquery_t	*query;
	int		index;
};


struct parent
{
	Namtype_t	hdr;
	Dt_t		*dict;
	Namfun_t	parentfun;
	size_t		namelen;
	char		*name;
	Dssdisc_t	*disc;
	Cxstate_t	*state;
	Cx_t		*cx;
	Dss_t		*dss;
	Cxtype_t	*type;
	struct dssfile	*sp;
	struct query	*qp;
};


/*
 * the following function is added as a builtin for each query
 * function that a variable can have
 */
static int query(int argc, char *argv[], Shbltin_t *bp)
{
	struct parent	*dp = (struct parent*)nv_hasdisc(bp->vnode,&parent_disc);
	Dss_t		*dss = dp->dss;
	struct query	*pp = dp->qp;
	Cxquery_t	*qp = pp?pp->query:0;
	Cx_t		*cx = dss->cx;
	Dssrecord_t	*data;
	int		 n = (char**)bp->ptr-dp->hdr.names;
	char		*savedata;
	size_t		savesize;
	while(pp && pp->index!=n)
		pp = pp->next;
	if(!pp || argv[1])
	{
		if(!pp)
		{
			if(!(qp = cxquery(cx, dp->hdr.names[n], cx->disc)))
				errormsg(SH_DICT,ERROR_exit(1),"%s: load method %s",argv[0]);
			pp = newof(NiL,struct query,1,0);
			pp->index = n;
			pp->next = dp->qp;
			dp->qp = pp;
			pp->query = qp;
			memset((void*)&pp->expr,0,sizeof(pp->expr));
		}
		if(pp->expr.query && qp->end)
			(*qp->end)(cx,&pp->expr,(void*)0,cx->disc);
		if(argc==2 && strcmp(argv[1],"end")==0)
		{
			struct query *xp, **ppp = &dp->qp;
			while((xp= *ppp) && xp != pp)
				ppp = &xp->next;
			if(xp)
				*ppp = pp->next;
			free((void*)pp);
			return(0);
		}
		pp->expr.query = pp->query;
		pp->expr.op = sfstdout;
		if(qp->beg)
		{
			if((*qp->beg)(cx,&pp->expr,(void*)argv,cx->disc))
			{
				pp->expr.query = 0;
				return(2);
			}
		}
		return(0);
	}
	if(!bp->vnode->nvalue || !dp->sp || !(data = dp->sp->rp))
		return(1);
	savedata = data->data;
	savesize = data->size;
	data->data = bp->vnode->nvalue;
	data->size = nv_size(bp->vnode);
	if(qp->sel)
	{
		if((n=(*qp->sel)(cx,&pp->expr,data,cx->disc))<=0)
			return(n<0?2:1);
	}
	if(qp->act)
		return(-(*qp->act)(cx, &pp->expr,data,cx->disc));
	data->data = savedata;
	data->size = savesize;
	return(0);
}

static char *getbuf(size_t len)
{
	static char *buf;
	if(buflen < len)
	{
		if(buflen==0)
			buf = (char*)malloc(len);
		else
			buf = (char*)realloc(buf,len);
		buflen = len;
	}
	return(buf);
}

static void check_numeric(Namval_t *np, Cxtype_t *tp, Cxstate_t *sp)
{
	if(cxisnumber(tp))
	{
		nv_onattr(np,NV_LDOUBLE|NV_EXPNOTE);
		nv_setsize(np,10);
	}
	else if(cxisbuffer(tp))
		nv_onattr(np,NV_BINARY);
}

static Namval_t *typenode(const char *name, int flag)
{
	int		offset = stktell(stkstd);
	Namval_t	*mp;
	sfputc(stkstd,0);
	sfprintf(stkstd,"dss.%s",name);
	sfputc(stkstd,0);
	mp = nv_search(stkptr(stkstd,offset+1),typedict,flag);
	stkseek(stkstd,offset);
#if 0
	if(flag&NV_ADD)
	{
		/* create reference variable name to NV_CLASS.dss.name */
		Shell_t *shp = sh_getinterp();
		Namval_t *rp;
		sfputc(stkstd,0);
		sfprintf(stkstd,NV_CLASS".dss.%s",name);
		sfputc(stkstd,0);
		rp = nv_open(name, shp->var_tree, NV_IDENT);
		nv_unset(rp);
		nv_putval(rp,stkptr(stkstd,offset+1),0);
		nv_setref(rp,shp->var_tree,NV_VARNAME|NV_NOREF);
		stkseek(stkstd,offset);
	}
#endif
	return(mp);
}

Cxvalue_t *get_child_common(Namval_t *np, Namfun_t *fp, Cxoperand_t *op)
{
	Namchld_t	*cp = (Namchld_t*)fp;
	struct parent	*pp = (struct parent*)cp->ptype;
	Cxtype_t	*tp = pp->type;
	Cxvariable_t	*vp = vnode(np);
	Cxinstruction_t	in;
	if(nv_isattr(pp->hdr.np,NV_INTEGER))
	{
		in.data.variable = vp;
		op->type = vp->type;
		op->value.number = nv_getnum(pp->hdr.np);
		if((*tp->member->getf)(pp->cx,&in,op,NiL,NiL,NiL,pp->cx->disc)==0)
			return(&op->value);
	}
	return(0);
}

static char* get_child(register Namval_t* np, Namfun_t *fp)
{
	Cxvariable_t	*vp = vnode(np);
	Cxoperand_t	op;
	Cxvalue_t	*val;
	if(val = get_child_common(np, fp, &op))
	{
		if(cxisnumber(vp->type))
		{
			long l = nv_size(np)+8;
			char *sp = getbuf(l);
			sfsprintf(sp,l,"%.*Lg\0",nv_size(np),val->number);
			return(sp);
		}
		return(val->string.data);
	}
	return(0);
}

static char* get_mchild(register Namval_t* np, Namfun_t *fp)
{
	Cxoperand_t	ret;
	Namchld_t	*pp = (Namchld_t*)fp;
	struct parent	*dp = (struct parent*)pp->ptype;
	Cxvariable_t	*vp = vnode(np);
	char		*savedata=0, *value;
	size_t		savesize;
	Dssrecord_t	*rp, rec;
	if(!dp->hdr.np->nvalue)
		return(0);
	if(dp->sp && dp->sp->rp) 
	{
		rp = dp->sp->rp;
		savedata = rp->data;
		savesize = rp->size;
	}
	else
		memset(rp= &rec, 0, sizeof(rec));
	rp->data = dp->hdr.np->nvalue;
	rp->size = nv_size(dp->hdr.np);
	if(nv_isattr(np,NV_BINARY) && !nv_isattr(np,NV_RAW))
	{
		if(cxcast(dp->cx,&ret,vp,dp->state->type_buffer,rp,(char*)0)==0)
		{
			nv_setsize(np,ret.value.buffer.size);
			value = ret.value.buffer.data;
		}
	}
	else if(cxcast(dp->cx,&ret,vp,dp->state->type_string,rp,(char*)0)==0)
		value = ret.value.string.data;
	else
		value = nv_name(np);
	if(savedata)
	{
		rp->data = savedata;
		rp->size = savesize;
	}
	return(value);
}

static Sfdouble_t nget_child(register Namval_t* np, Namfun_t *fp)
{
	Cxvariable_t	*vp = vnode(np);
	Cxoperand_t	op;
	Cxvalue_t	*val;
	op.type = 0;
	if(val = get_child_common(np, fp, &op))
	{
		Sfdouble_t	ld;
		if(cxisnumber(vp->type))
			return(val->number);
		else
		{
			ld = sh_strnum(val->string.data, (char**)0, 1);
			return(ld);
		}
	}
	return(0);
}

static Sfdouble_t nget_mchild(register Namval_t* np, Namfun_t *fp)
{
	Cxoperand_t ret;
	Namchld_t *pp = (Namchld_t*)fp;
	struct parent *dp = (struct parent*)pp->ptype;
	Cxvariable_t *vp = vnode(np);
#if 1
	if(cxcast(dp->cx,&ret,vp,dp->state->type_number,dp->hdr.np->nvalue,(char*)0)==0)
#else
	if(cxcast(dp->cx,&ret,vp,dp->state->type_number,dp->hdr.data,(char*)0)==0)
#endif
		return(ret.value.number);
	return(0.0);
}

static void put_child(Namval_t* np, const char* val, int flags, Namfun_t* fp)
{
	Namchld_t	*cp = (Namchld_t*)fp;
	struct parent	*pp = (struct parent*)cp->ptype;
	Cxtype_t	*tp = pp->type;
	Cxvariable_t	*vp = vnode(np);
	Cxinstruction_t	in;
	Cxoperand_t	ret,op;
	in.data.variable = vp;
	nv_onattr(np,NV_NODISC);
	nv_putval(np,val,flags);
	nv_offattr(np,NV_NODISC);
	ret.type = vp->type;
	ret.value.number = nv_getnum(pp->hdr.np);
	op.value.number = nv_getn(np,fp);
	if((*tp->member->setf)(pp->cx,&in, &ret,&op,NiL,NiL,pp->cx->disc)==0)
		nv_putval(pp->hdr.np,(char*)&ret.value.number,NV_INTEGER|NV_DOUBLE|NV_LONG|NV_NODISC);
}

static void put_mchild(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
}

static char *name_child(Namval_t *np, Namfun_t *fp)
{
	Namchld_t	*pp = (Namchld_t*)fp;
	struct parent	*dp = (struct parent*)pp->ptype;
	const char	*name = vnode(np)->name;
	char		*cp;
	size_t		l,len;
	cp = nv_name(dp->hdr.np);
	len = (l=strlen(cp)) + strlen(name)+2;
	if(dp->namelen < len)
	{
		if(dp->namelen==0)
			dp->name = (char*)malloc(len);
		else
			dp->name = (char*)realloc(dp->name,len);
		dp->namelen = len;
	}
	memcpy(dp->name,cp,l);
	dp->name[l++] = '.';
	strcpy(&dp->name[l],name);
	return(dp->name);
}
 
static Namval_t *type_child(register Namval_t* np, Namfun_t *fp)
{
	struct parent	*pp =  (struct parent*)((Namchld_t*)fp)->ptype;
	Cxstate_t	*sp = pp->state;
	Cxvariable_t	*vp = vnode(np);
	if(!pp->dss || vp->type==sp->type_number || vp->type==sp->type_string)
		return(0);
	return(typenode(vp->type->name,0));
}

/* for child variables of compound variables */
static const Namdisc_t child_disc =
{
	sizeof(Namchld_t),
	put_child,
	get_child,
	nget_child,
	0,
	0,
	0,
	name_child,
	0,
	type_child
};

/* for child variables of methods */
static const Namdisc_t mchild_disc =
{
	sizeof(Namchld_t),
	put_mchild,
	get_mchild,
	nget_mchild,
	0,
	0,
	0,
	name_child,
	0,
	type_child
};

static const Namdisc_t type_disc;

static Namval_t *node(Cxvariable_t *vp,struct parent *dp)
{
	Namval_t	*mp, *np=0;
	Namfun_t	*fp, *nfp=0;
	if(!dp->hdr.nodes)
	{
		struct type	*tp;
		struct parent	*pp;
		if(!(dp->hdr.nodes = (char*)calloc(dp->hdr.numnodes,NV_MINSZ)))
			return(0);
		np = nv_namptr(dp->hdr.nodes,0);
		np->nvname = NV_DATA;
		nv_onattr(np,NV_MINIMAL);
		tp = (struct type*)nv_hasdisc(mp=dp->hdr.fun.type,&type_disc);
		if(tp)
			pp = (struct parent*)tp->fun.next;
		else
			pp = (struct parent*)nv_hasdisc(mp,&parent_disc);
		if(mp!=dp->hdr.np && pp && pp->hdr.fun.disc==&parent_disc && pp->hdr.nodes)
			mp =  nv_namptr(pp->hdr.nodes,0);
		else if(tp)
			mp = &tp->details;
		else
			mp = 0;
		if(mp && !nv_isnull(mp))
			nv_clone(mp,np,0);
	}
	if(!vp)
		return(np?np:nv_namptr(dp->hdr.nodes,0));
	np = nv_namptr(dp->hdr.nodes,vp->header.index+1);
	if(!np->nvname)
	{
		nv_disc(np,&dp->hdr.childfun.fun,NV_FIRST);
		nv_onattr(np,NV_MINIMAL);
		check_numeric(np, vp->type, dp->state);
		if(!dp->dss && vp->type->base && (mp=typenode(vp->type->name,0)) && (fp = nv_hasdisc(mp, &type_disc)))
		{
			int size = fp->dsize;
			if(size==0 || (!fp->disc || (size=fp->disc->dsize)==0)) 
				size = sizeof(Namfun_t);
			if(fp->disc && fp->disc->clonef)
				nfp = (*fp->disc->clonef)(np,mp,0,fp);
			else if(nfp = malloc(size))
			{
				memcpy((void*)nfp,(void*)fp,size);
				nfp->nofree &= ~1;
			}
			if(nfp)
				nv_disc(np,nfp,NV_FIRST);
			if(((struct type*)fp)->details.nvalue)
				nv_setvtree(np);
		}
		np->nvname = (char*)vp;
	}
	return(np);
}

static Namfun_t *clone_parent(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	Namtype_t 	*dp;
	struct type	*tp;
	size_t		 size = fp->dsize;
	if(size==0 && (!fp->disc || (size=fp->disc->dsize)==0)) 
		size = sizeof(Namfun_t);
	dp = (Namtype_t*)calloc(1,size);
	memcpy((void*)dp,(void*)fp,size);
	dp->childfun.ptype = dp;
	dp->childfun.ttype = (Namtype_t*)fp;
	if(tp=(struct type*)nv_hasdisc(mp,&type_disc))
		tp->pfun = &dp->fun;
	dp->data = 0;
	dp->np = mp;
	dp->nodes = 0;
	dp->parent = nv_lastdict();
	((struct parent*)dp)->namelen = 0;
	return(&dp->fun);
}

static void pushtype(Namval_t *np, Cxtype_t *tt, Namfun_t *fp)
{
	Namval_t *mp;
	for(; tt->base; tt = tt->base)
	{
		if(tt->member)
		{
			Namfun_t *nfp;
			mp = typenode(tt->name,0);
			if(!fp->next && np!=fp->type)
			{
				fp = nv_disc(np, fp, NV_CLONE);
			}
			nfp = nv_hasdisc(mp, &parent_disc);
			nfp = clone_parent(mp,np,0,nfp);
			nfp->type = fp->type;
			((struct type*)fp)->pfun = nfp;
			if(mp=node((Cxvariable_t*)0,(struct parent*)nfp))
				nv_clone( &((struct type*)fp)->details,mp,NV_MOVE);
			nfp->next = fp->next;
			fp->next = nfp;
		}
	}
}

static Namval_t *create_parent(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	struct parent	*dp = (struct parent*)fp;
	Dt_t		*dict = dp->dict;
	Cxvariable_t	*vp;
	char		*last=(char*)name;
	int		c;
	if(!name)
		return(dp->hdr.np);
	while((c=*last) && c!='.' && c!='=' && c!='[' && c!='+')
		last++;
	if(c)
		*last = 0;
	if(strcmp(name,NV_DATA)==0)
	{
		np = node((Cxvariable_t*)0,dp);
		if(c)
			*last = c;
		fp->last = last;
		return(np);
	}
	vp = (Cxvariable_t*)dtmatch(dict,name);
	if(c)
		*last = c;
	if(vp)
	{
		np = node(vp,dp);
		fp->last = last;
		if(c=='.')
		{
			Namfun_t *rp = np->nvfun;
			if(rp == &dp->hdr.childfun.fun)
			{
				fp = nv_disc(np,np->nvfun, NV_CLONE);
				pushtype(np, vp->type, fp);
			}
			for(;rp && !(rp->disc && rp->disc->createf);rp = rp->next);
			if(rp)
				fp = rp;
			np = (*fp->disc->createf)(np,last+1, flag, fp);
			dp->hdr.fun.last = fp->last;
		}
		return(np);
	}
	return(0);
}

static void dss_unset(Namval_t *np, struct parent *dp)
{
	if(dp->namelen)
		free((void*)dp->name);
	if(!dp->hdr.np)
	{
		if(dp->dss)
			dssclose(dp->dss);
		else
			cxclose(dp->cx);
	}
	nv_disc(np,&dp->hdr.fun,NV_POP);
	if(dp->hdr.nodes)
		free((void*)dp->hdr.nodes);
	if(!dp->hdr.fun.nofree&1)
		free((void*)dp);
}

static void put_parent(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	struct parent	*pp = (struct parent*)fp;
	Namval_t	*nq;
	if(!val)
	{
		nv_putv(np,val,flag,fp);
		dss_unset(np, pp);
	}
	else if(nq=nv_open(val,sh.var_tree,NV_VARNAME|NV_ARRAY|NV_NOADD|NV_NOFAIL))
	{
		Namfun_t  *pp;
		if((pp=nv_hasdisc(nq,fp->disc)) && pp->type==fp->type)
		{
			nv_unset(np);
			nv_clone(nq,np,0);
			return;
		}
	}
	if(!pp->dss || (flag&NV_NOFREE))
		nv_putv(np,val, flag, fp);
}


#define DSS_EVENT (SF_EVENT+100)
static int	dss_except(Sfio_t* iop, int event, void *data, Sfdisc_t *fp)
{
	if(event==SF_CLOSING)
	{
		struct dssfile	*sp = (struct dssfile*)fp;
		if(sp && sp->np)
		{
			Namfun_t *nfp;
			for(nfp=sp->np->nvfun; nfp && !nfp->disc && !nfp->disc->readf; nfp=nfp->next);
			if(nfp)
				(*nfp->disc->readf)(sp->np, (Sfio_t*)0, 0, nfp);
		}
		dssfclose(sp->fp);
		free((void*)fp);
	}
	if(event!= DSS_EVENT)
		return(0);
	*(Sfdisc_t**)data = fp;
	return(1);
}

static int read_parent(register Namval_t* np, Sfio_t *iop, int delim, Namfun_t *fp)
{
	Namval_t	*mp;
	struct parent	*dp = (struct parent*)fp;
	struct dssfile	*sp;
	if(!iop)
		sp = dp->sp;
	else if(sfraise(iop, DSS_EVENT, (void*)&sp)==0)
	{
		sp = (struct dssfile*)calloc(1,sizeof(struct dssfile));
		sp->np = np;
		if(!((sp->fp = dssfopen(dp->dss,(char*)0,iop,DSS_FILE_READ,0))))
		{
			errormsg(SH_DICT,ERROR_exit(1),"%d: invalid dss format",sffileno(iop));
			return(1);
		}
		sp->disc.exceptf = dss_except;
		sfdisc(iop, &sp->disc);
	}
	if(sp && sp->rp)
	{
		if(!iop)
			mp = np;
		else if(sp->np != np)
			mp = sp->mp?sp->mp:sp->np;
		else
			mp = sp->mp;
		if(mp && mp->nvalue==sp->rp->data)
		{
			char *bp = (char*)malloc(sp->rp->size);
			mp->nvalue = memcpy(bp,sp->rp->data,sp->rp->size);
			nv_offattr(mp,NV_NOFREE);
		}
	}
	dp->sp = 0;
	if(iop)
	{
		int flags = nv_isattr(np,~NV_RDONLY)|NV_NOFREE|NV_NODISC;
		Namarr_t *ap = nv_arrayptr(np);
		dp->sp = sp;
		sp->np = np;
		sp->mp = 0;
		if(!(sp->rp = dssfread(sp->fp)))
			return(1);
		if(ap)
		{
			(*ap->hdr.disc->putval)(np,sp->rp->data,flags,&ap->hdr);
			np = sp->mp = nv_opensub(np);
		}
		else
			nv_putval(np, sp->rp->data, flags);
		nv_setsize(np,sp->rp->size);
		nv_onattr(np,NV_BINARY);
	}
	return(0);
}

static int write_parent(register Namval_t* np, Sfio_t *iop, int delim, Namfun_t *fp)
{
	struct parent	*dp = (struct parent*)fp;
	struct dssfile	*sp;
	if(sfraise(iop, DSS_EVENT, (void*)&sp)==0)
	{
		sp = (struct dssfile*)calloc(1,sizeof(struct dssfile));
		sp->np = np;
		if(!((sp->fp = dssfopen(dp->dss,(char*)0,iop,DSS_FILE_WRITE,0))))
		{
			errormsg(SH_DICT,ERROR_exit(1),"%d: cannot open for dss write",sffileno(iop));
			return(1);
		}
		sp->disc.exceptf = dss_except;
		sfdisc(iop, &sp->disc);
	}
	if(!sp->rp)
		sp->rp = (Dssrecord_t*)calloc(1,sizeof(Dssrecord_t));
	sp->rp->data = np->nvalue;
	sp->rp->size = nv_size(np);
	return dssfwrite(sp->fp,sp->rp) < 0 ? -1 : 0;
}

static Namval_t *next_parent(register Namval_t* np, Dt_t *root,Namfun_t *fp)
{
	struct parent	*dp = (struct parent*)fp;
	Cxvariable_t	*vp=0;
	if(root && (np!=nv_namptr(dp->hdr.nodes,0)))
	{
		if((fp=nv_hasdisc(np,&mchild_disc)) || (fp=nv_hasdisc(np,&child_disc)))
			dp = (struct parent*)((Namchld_t*)fp)->ptype;
			
	}
	if(!fp)
		return(0);
	do
	{
		if(root)
		{
			if(np==nv_namptr(dp->hdr.nodes,0))
			{	root=dp->dict;
				vp=(Cxvariable_t*)dtfirst(root);
			}
			else
			{
				if(!vp)
					vp = vnode(np);
				vp=(Cxvariable_t*)dtnext(dp->dict,vp);
			}
#if 0
if(vp)
sfprintf(sfstderr,"%x: next=%s index=%d np=%x\n",vp,vp->name,vp->header.index,np);
else sfprintf(sfstderr,"vp==NULL\n");
#endif
		}
		else
		{
			np=node(vp,dp);
			return (np);
#if 0
if(vp)
sfprintf(sfstderr,"%x: first=%s index=%d\n",vp,vp->name,vp->header.index);
#endif
		}
	}
	while(vp && vp->header.index>=dp->hdr.numnodes-1);
	if(!vp)
		return((Namval_t*)0);
	np = node(vp,dp);
	return(np);
}

static const Namdisc_t parent_disc =
{
	sizeof(struct parent),
	put_parent,
	0,
	0,
	0,
	create_parent,
	clone_parent,
	0,
	next_parent,
	0,
	read_parent,
	write_parent
};

/*
 * should only get here when requesting raw binary data
 * This must come after the nv_tree discipline
 */
static char* get_parent(register Namval_t* np, Namfun_t *fp)
{
#if 1
	char	*data = np->nvalue;
	size_t	size = nv_size(np);
#else
#if 1
	Dssrecord_t	*rp = (Dssrecord_t*)np->nvalue;
#else
	struct parent	*dp = (struct parent*)nv_hasdisc(np,&parent_disc);
	Dssrecord_t	*rp = (Dssrecord_t*)dp->hdr.data;
#endif
#endif
	char		*cp;
#if 1
	if(size>0)
	{
		size_t m, n = (4*size)/3 + size/45 + 8;
		m = base64encode(data,size,(void**)0,cp=getbuf(n),n,(void**)0);
#else
	if(rp && rp->size>0)
	{
		size_t m, n = (4*rp->size)/3 + rp->size/45 + 8;
		m = base64encode(rp->data,rp->size, (void**)0, cp=getbuf(n), n, (void**)0);
#endif
		nv_setsize(np,m);
		return(cp);
	}
	return(0);
}

static const Namdisc_t parent2_disc =
{
	0,0,get_parent
};

/*
 * add discipline builtin given by name to type
 */
static Namval_t *add_discipline(const char *typename, const char *name, int (*fun)(int, char*[],Shbltin_t*), void* context)
{
	Namval_t *mp;
	int offset = stktell(stkstd);
	sfputc(stkstd,0);
	sfprintf(stkstd,NV_CLASS".dss.%s.%s",typename,name);
	sfputc(stkstd,0);
	mp =  sh_addbuiltin(stkptr(stkstd,offset+1),fun,context);
	stkseek(stkstd,offset);
	nv_onattr(mp,NV_RDONLY);
	nv_offattr(mp,NV_NOFREE);
	return(mp);
}

static void put_type(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	struct type	*tp = (struct type*)fp;
	Cxoperand_t	cop;
	Namval_t	*nq;
	if(val && (nq=nv_open(val,sh.var_tree,NV_VARNAME|NV_ARRAY|NV_NOADD|NV_NOFAIL))) 
	{
		Namfun_t  *pp;
		if((pp=nv_hasdisc(nq,fp->disc)) && pp->type==fp->type)

		{
			_nv_unset(np, flag);
			nv_clone(nq,np,0);
			return;
		}
	}
	if(val && !(flag&NV_INTEGER) && tp->type->internalf && !cxisstring(tp->type))
	{
		size_t	 size = strlen(val);
		cop.type = tp->type;
		if((*tp->type->internalf)(tp->cx, tp->type, NiL, NiL, &cop, val, size, Vmregion, &Dssdisc) <0)
			errormsg(SH_DICT,ERROR_exit(1),"%s: cannot covert to type dss.%s",val,tp->type->name);
		if(cxisnumber(cop.type))
			nv_putv(np,(char*)&cop.value.number,flag|NV_LDOUBLE,fp);
		else
		{
			nv_setsize(np,cop.value.buffer.size);
			nv_putv(np, (char*)cop.value.buffer.data, NV_RAW|NV_BINARY,fp);
		}
	}
	else
		nv_putv(np,val,flag,fp);
	if(!val)
	{
		nv_disc(np,fp,NV_POP);
		if(fp->nofree&~1)
			free((void*)fp);
		else
		{
			Namval_t *mp;
			int i;
			for(i=0; i < sizeof(tp->bltins)/sizeof(Namval_t*); i++)
			{
				if(mp = tp->bltins[i])
				{
					Shell_t *shp = sh_getinterp();
					dtdelete(shp->bltin_tree,mp);
					free((void*)mp);
				}
			}
			free((void*)fp);
		}
	}
}

static char* get_type(register Namval_t* np, Namfun_t *fp)
{
	struct type	*tp = (struct type*)fp;
	struct parent	*dp;
	Cxvalue_t	cval;
	int		i,n;
	char		*buf,*format;
	Cxvariable_t	*vp=0;
	Namval_t	*mp = &tp->details;
	if(!tp->type->externalf)
		return(nv_getv(np,fp));
	if(nv_isattr(np,NV_INTEGER))
		cval.number = nv_getn(np,fp);
	else
	{
		nv_onattr(np,NV_RAW);
		cval.string.data = nv_getv(np,fp);
		nv_offattr(np,NV_RAW);
		if((n=nv_size(np))==0)
			n = strlen(cval.string.data);
		cval.string.size = n;
	}
	buf = getbuf(32);
	if(nv_hasdisc(np,&child_disc))
		vp = vnode(np);
	if(dp=(struct parent*)nv_hasdisc(np,&parent_disc))
		mp = node((Cxvariable_t*)0,dp);
	if(!(format = nv_getval(mp)) && vp)
		format = vp->format.details;
	for(i=0; i < 2; i++)
	{
		n = (*tp->type->externalf)(tp->cx, tp->type, format, NiL, &cval, buf, buflen, &Dssdisc);
		if(n<buflen)
			break;
		buf = getbuf(n);
	}
	return(buf);
}

/*
 *  They can all share one instance of the discipline
 */
static Namfun_t *clone_type(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	struct type *tp = (struct type*)fp;
	if(!fp->next)
		 pushtype(np, tp->type, fp);
	if(fp->next)
	{
		struct type *dp = newof((struct type*)0, struct type,1,0);
		*dp = *tp;
		dp->fun.nofree &= ~0;
		return(&dp->fun);
	}
	fp->nofree |=1;
	return(fp);
}

static Namval_t *next_type(register Namval_t* np, Dt_t *root,Namfun_t *fp)
{
	struct type	*tp = (struct type*)fp;
	Namfun_t	*fpn = fp->next;
	if(!root && (!fpn || fpn->disc != &parent_disc))
	{
		pushtype(np, tp->type, fp);
		if(fp->next==fpn)
			return(&((struct type*)fp)->details);
	}
	if(tp->pfun)
		return((*tp->pfun->disc->nextf)(np,root,tp->pfun));
	return(0);
}

static char *setdisc_type(Namval_t *np, const char *event, Namval_t* action, Namfun_t
 *fp)
{
	struct type *tp = (struct type*)fp;
	int n = -1;
	const char *name = event?event:(const char*)action;
	if(name)
	{
		if(strcmp(name,"match")==0 && tp->type->match)
			n = 0;
	}
	if(!event)
	{
		if(!action && tp->type->match)
			return("match");
		n = -1;
	}
	if(n<0)
		return(nv_setdisc(np,event,action,fp));
	if(action==np)
	{
		/* clone the discipline for variable specific function */
		if((fp->nofree&1) && fp->type!=np)
		{
			tp = (struct type*)nv_disc(np, fp, NV_CLONE);
			fp->nofree &= ~1;
		}
		action = tp->bltins[n];
	}
	else if(action)
		tp->bltins[n] = action;
	else
	{
		action = tp->bltins[n];
		tp->bltins[n] = 0;
	}
	return(action?(char*)action:"");
}

static Namval_t *create_type(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	struct type	*tp = (struct type*)fp;
	if(!fp->next)
	{
		pushtype(np, tp->type, fp);
		fp = nv_hasdisc(np,fp->disc);
	}
	while (fp = fp->next)
		if (fp->disc && fp->disc->createf)
		{
			if(np = (*fp->disc->createf)(np, name, flag, fp))
				tp->fun.last = fp->last;
			return(np);
		}
	if(memcmp(name,NV_DATA,sizeof(NV_DATA)-1)==0 && (name[sizeof(NV_DATA)-1]==0 || strchr("+=[",name[sizeof(NV_DATA)-1])))
	{
		tp->fun.last = (char*)name + sizeof(NV_DATA)-1;
		return(&tp->details);
	}
	return(0);
}

static const Namdisc_t type_disc =
{
	0,
	put_type,
	get_type,
	0,
	setdisc_type,
	create_type,
	clone_type,
	0,
	next_type
};

static const char sh_opttype[] =
"[-1c?\n@(#)$Id: type (AT&T Research) 2008-06-09 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - set the type of variables to \b\f?\f\b]"
"[+DESCRIPTION?\b\f?\f\b sets the type of each of the variables specified "
	"by \aname\a to \b\f?\f\b. If \b=\b\avalue\a is specified, "
	"the variable \aname\a is set to \avalue\a before the variable "
	"is converted to \b\f?\f\b.]"
"[+?If no \aname\as are specified then the names and values of all "
	"variables of this type are written to standard output.]" 
"[+?\b\f?\f\b is built-in to the shell as a declaration command so that "
	"field splitting and pathname expansion are not performed on "
	"the arguments.  Tilde expansion occurs on \avalue\a.]"
	"[+?The types are:]{\ftypes\f}"
"[r?Enables readonly.  Once enabled, the value cannot be changed or unset.]"
"[a?index array.  Each \aname\a will converted to an index "
	"array of type \b\f?\f\b.  If a variable already exists, the current "
	"value will become index \b0\b.]"
"[A?Associative array.  Each \aname\a will converted to an associate "
	"array of type \b\f?\f\b.  If a variable already exists, the current "
	"value will become subscript \b0\b.]"
"[p?Causes the output to be in a form of \b\f?\f\b commands that can be "
	"used as input to the shell to recreate the current type of "
	"these variables.]"
"\n"
"\n[name[=value]...]\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"

"[+SEE ALSO?\breadonly\b(1), \btypeset\b(1)]"
;

static void mktype(Namval_t *np, Cxtype_t *tp, Cx_t *cx)
{
	struct type	*pp;
	struct Optdisc	optdisc;
	pp = newof((struct type*)0, struct type,1,0);
	pp->fun.dsize = sizeof(struct type);
	pp->fun.type = np;
	pp->fun.nofree = 1;
	pp->fun.disc = &type_disc;
	pp->type = tp;
	pp->cx = cx;
	if(tp->format.details)
	{
		Namval_t *mp = &pp->details;
		nv_setvtree(np);
		mp->nvname = NV_DATA;
		mp->nvalue = tp->format.details;
		nv_onattr(mp,NV_NOFREE);
	}
	nv_disc(np,&pp->fun,NV_LAST);
	if(tp->match)
		pp->bltins[0] = add_discipline(pp->type->name, "match",match,pp );
	memset(&optdisc,0,sizeof(optdisc));
	optdisc.dss.optdisc.infof = dssoptinfo;
	optdisc.dss.header = (Cxheader_t*)tp;
	optdisc.np = np;
	nv_addtype(np, sh_opttype, &optdisc.dss.optdisc, sizeof(optdisc));
	while(tp = tp->base)
	{
		if(tp->base && !typenode(tp->name, 0))
			create_dss((Namval_t*)0, tp->name, 0, (Namfun_t*)0);
	}
}

static Namval_t *create_dss(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	Cxvariable_t	*vp;
	Cxstate_t	*sp;
	Cxtype_t	*tp=0;
	Cx_t		*cx;
	struct parent	*dp;
	Namval_t	*mp;
	Dss_t		*dss=0;
	Dssmeth_t	*meth;
	long		n;
	Dt_t		*dict;
	Namval_t	*parent = nv_lastdict();
	struct Optdisc	optdisc;
	char		*cp=0;
	Dssdisc.errorf = (Error_f)errorf;
	if(fp && !(cp = strchr(name,'.')))
		fp->last =  (char*)name + strlen(name);
	mp = typenode(name,0);
	if(cp)
	{
		Namfun_t *fq;
		*cp = '.';
		if(!mp)
			return(0);
		for(fq=mp->nvfun ;fq && !(fq->disc && fq->disc->createf); fq->next);
		if(!fq)
			return(0);
		if(np = (*fq->disc->createf)(mp, cp+1, flag, fq))
			fp->last = fq->last;
		return(np);
	}
	if(mp)
		return(mp);
	if(meth = dssmeth(name,&Dssdisc))
	{
		if(!(dss = dssopen(0, 0,&Dssdisc,meth)))
			errormsg(SH_DICT,ERROR_exit(1),"dssopen failed");
		cx = dss->cx;
		sp = cx->state;
	}
	else
	{
		Dssstate_t *state = dssstate(&Dssdisc);
		if(!state || !(tp = dtmatch(state->cx->types,name)))
			errormsg(SH_DICT,ERROR_exit(1),"%s: unknown dss type",name);
		sp = state->cx;
		cx = cxopen(0,0,&Dssdisc);
	}
	mp = typenode(name,NV_ADD);
	if(dss)
		dict = dss->cx->variables;
	else
	{
		check_numeric(mp, tp, sp);
		if(!tp->member || !(dict = tp->member->members))
		{
			mktype(mp, tp, cx);
			return(mp);
		}
	}
	for (n=0,vp = (Cxvariable_t*)dtfirst(dict); vp; vp = (Cxvariable_t*)dtnext(dict, vp))
	{
		Namval_t *qp;
		if(strcmp(vp->name,"dss")==0)
			continue;
#if 0
vp->header.index = n;
sfprintf(sfstderr,"tname=%s name=%s n=%d index=%d\n",name,vp->name,n,vp->header.index);
#endif
		vp->data = (void*)n++;
		if(vp->type==sp->type_number || vp->type==sp->type_string || vp->type==sp->type_buffer)
			continue;
		if(!(qp=typenode(vp->type->name,0)))
		{
			if(!vp->type->member && (qp=typenode(vp->type->name,NV_ADD)))
			{
				mktype(qp,vp->type,cx);
				check_numeric(qp, vp->type, sp);
			}
			else if(vp->type->member)
				qp = create_dss(np, vp->name,flag,fp);
		}
	}
	if(!(dp = newof((struct parent*)0, struct parent,1,0)))
		return(0);
	dp->hdr.numnodes = n+1;
	dp->hdr.fun.disc = &parent_disc;
	dp->hdr.fun.type = mp;
	dp->hdr.np = mp;
	dp->hdr.sh = fp?((struct dsstype*)fp)->sh:0;
	dp->hdr.parent = parent;
	dp->dss = dss;
	dp->cx = cx;
	dp->dict = dict;
	dp->state = sp;
	dp->type = tp;
	dp->disc = &Dssdisc;
	dp->hdr.childfun.fun.disc = meth?&mchild_disc:&child_disc;
	dp->hdr.childfun.fun.nofree = 1;
	dp->hdr.childfun.ptype = &dp->hdr;
	dp->parentfun.disc = &parent2_disc;
	dp->parentfun.nofree = 1;
#if 0
	nv_disc(mp, &dp->parentfun,NV_FIRST);
#endif
	nv_setvtree(mp);
	if(dss)
	{
		int		i,n=((struct dsstype*)fp)->nnames;
		dp->hdr.names = ((struct dsstype*)fp)->names;
		nv_adddisc(mp,(const char**)dp->hdr.names,0);
		for(i=0; i < n; i++)
			add_discipline(name,dp->hdr.names[i], query,&dp->hdr.names[i]);
	}
	nv_disc(mp,&dp->hdr.fun,NV_FIRST);
	mp->nvalue = 0;
	if(dss)
		nv_onattr(mp,NV_BINARY|NV_RAW);
	memset(&optdisc,0,sizeof(optdisc));
	optdisc.dss.optdisc.infof = dssoptinfo;
	optdisc.dss.header = meth?(Cxheader_t*)meth:(Cxheader_t*)tp;
	optdisc.np = mp;
	nv_addtype(mp, sh_opttype, &optdisc.dss.optdisc, sizeof(optdisc));
	return(mp);
}

static char *name_dss(Namval_t *np, Namfun_t *fp)
{
	int	len = sizeof(NV_CLASS)+strlen(np->nvname)+1 ;
	char	*name = getbuf(len);
	memcpy(name,NV_CLASS,sizeof(NV_CLASS)-1);
	name[sizeof(NV_CLASS)-1] = '.';
	strcpy(&name[sizeof(NV_CLASS)],np->nvname);
	return(name);
}

static const Namdisc_t dss_disc =
{
	0,
	0,
	0,
	0,
	0,
	create_dss,
	0,
	name_dss,
};

static const char *discnames[] = { "list", "load", 0 };

static const char optlist[] =
"[-1c?\n@(#)$Id: dss.list (AT&T Research) 2007-05-09 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - list the known dss entities]"
"[+DESCRIPTION?\b\f?\f\b causes each of the specified dss entities "
	"specified as options to be written to standard output.  If no "
	"options are specified, all entities are written.]"
"[+?If multiple entities types are written, then the entity type is written "
	"before each of the names.]"
"[l?List the names of library entities.]"
"[m?List the names of method entities.]"
"[q?List the names of query entities.]"
"[t?List the names of type entities.]"
"[v?A description of each entity will be listed along with the name.]"
"\n"
"\n\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"
"[+SEE ALSO?\b"NV_CLASS".dss.load\b(1)]"
;

#define fval(x)		(1L<<(x)-'a')
static const char *listnames[] = { "library", "method", "query", "type", 0 };

static int listdss(int argc, char *argv[], Shbltin_t *bp)
{
	Cxstate_t	*sp = cxstate((Cxdisc_t*)bp->ptr);
	int		flags=0, n, delim='\n';
	Dt_t		*dict;
	char		*name;
	Cxtype_t	*tp;
	NOT_USED(argc);
	while((n = optget(argv,optlist))) switch(n)
	{
	    case 'm': case 'l': case 't': case 'q':
		flags |= fval(n);
		break;
	    case 'v':
		delim = '\t';
		break;
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		return(2);
	}
	argv += opt_info.index;
	if(error_info.errors || *argv)
		 errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if(flags==0)
		flags = fval('m')|fval('l')|fval('q')|fval('t');
	n = flags&(flags-1);
	argv = (char**)listnames;
	while(name = *argv++)
	{
		if(fval('t') && *name=='t')
			dict =  sp->types;
		else if(fval('l') && *name=='l')
			dict =  sp->libraries;
		else if(fval('q') && *name=='q')
			dict =  sp->queries;
		else if(fval('m') && *name=='m')
			dict =  sp->methods;
		else
			continue;
		for (tp = (Cxtype_t*)dtfirst(dict); tp; tp = (Cxtype_t*)dtnext(dict, tp))
		{
			if(n)
				sfprintf(sfstdout,"%s\t",name);
			sfputr(sfstdout,tp->name,delim);
			if(delim!='\n')
				sfputr(sfstdout,tp->description,'\n');
		}
	}
	return(error_info.errors);
}

static const char optload[] =
"[-1c?\n@(#)$Id: dss.load (AT&T Research) 2003-01-10 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - load a dss format library]"
"[+DESCRIPTION?\b\f?\f\b causes each of the specified dss libraries \alib\a "
	"to be loaded into the shell and its contents merged with other "
	"dss libraries.]"
"\n"
"\nlib ...\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"
"[+SEE ALSO?\b"NV_CLASS".dss.list\b(1)]"
;

static int loadlib(int argc, char *argv[], Shbltin_t *bp)
{
	Cxdisc_t	*dp = (Cxdisc_t*)bp->ptr;
	char		*name;
	int		n;
	NOT_USED(argc);
	while((n = optget(argv,optload))) switch(n)
	{
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		return(2);
	}
	argv += opt_info.index;
	if(error_info.errors || !*argv)
		 errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	while(name = *argv++)
		if(!dssload(name, dp))
			error_info.errors = 1;
	return(error_info.errors);
}

static Cxvalue_t *getvalue(Namval_t *np, Namfun_t *fp, Cxoperand_t *valp) 
{
	if(fp && fp->disc==&child_disc)
	{
		Namchld_t *pp = (Namchld_t*)fp;
		struct parent *dp = (struct parent*)pp->ptype;
		Cxvariable_t *vp = vnode(np);
#if 1
		cxcast(dp->cx,valp,vp,vp->type,dp->hdr.parent->nvalue,(char*)0);
#else
		cxcast(dp->cx,valp,vp,vp->type,dp->hdr.data,(char*)0);
#endif
	}
	else
	{
		if(nv_isattr(np, NV_INTEGER))
			valp->value.number = nv_getnum(np);
		else
		{
			valp->value.string.data = nv_getval(np);
			valp->value.string.size = nv_size(np);
			if(!valp->value.string.size)
				valp->value.string.size = strlen(valp->value.string.data);
		}
	}
	return(&valp->value);
}

static const char optmatch[] =
"[-1c?\n@(#)$Id: dss.match (AT&T Research) 2003-01-15 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - match a dss type variable to a pattern]"
"[+DESCRIPTION?\b\f?\f\b causes the value of the variable whose name "
	"precedes \b.match\b in the callers name to be matched against "
	"the specified pattern \apattern\a.  The interpretation of "
	"\apattern\a depends on the type of this variable.]"
"[+?\fmatch\f]"
"[v?\apattern\a is a variable name that contains the pattern.]"
"\n"
"\npattern\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?The value of the variable matched \apattern\a.]"
        "[+1?The value of the variable did not match \apattern\a.]"
        "[+2?A usage error was found or a help request was made.]"
        "[+>2?An error occurred.]"
"}"
"[+SEE ALSO?\bdss\b(3)]"
;

static int match(int argc, char *argv[], Shbltin_t *bp)
{
	struct type	*tp = (struct type*)bp->ptr;
	Cxmatch_t	*mp = tp->type->match;
	Namval_t	*np;
	Namfun_t	*fp;
	Cxoperand_t	cval;
	Cxvalue_t	*valp = &cval.value;
	Cxtype_t	*tptr=0;
	Dssoptdisc_t	disc;
	char		*cp;
	void		*comp;
	int		n, flag=0;
	if(comp=bp->data)
	{
		if(!(bp->flags&SH_END_OPTIM))
			goto exec;
		(*mp->freef)(tp->cx, comp, &Dssdisc);
		bp->data = 0;
		if(argc<=0)
			return(0);
	}
	memset((void*)&disc,0,sizeof(disc));
	disc.header = (Cxheader_t*)tp->type;
	disc.optdisc.infof = dssoptinfo;
	opt_info.disc = &disc.optdisc;
	while((n = optget(argv,optmatch))) switch(n)
	{
	    case 'v':
		flag = 'v';
		break;
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		opt_info.disc = 0;
		return(2);
	}
	opt_info.disc = 0;
	argv += opt_info.index;
	if(error_info.errors || !(cp= *argv))
		 errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if(flag)
	{
		if(!(np = nv_open(*argv, ((Shell_t*)bp->shp)->var_tree, NV_NOADD|NV_VARNAME|NV_NOASSIGN)))
			errormsg(SH_DICT,ERROR_exit(3),"%s: variable not set",*argv);
		if((fp=nv_hasdisc(np,&mchild_disc)) || (fp=nv_hasdisc(np,&child_disc)))
		{
			Namval_t *mp = (*fp->disc->typef)(np,fp);
			if(mp)
				tptr = ((struct type*)(mp->nvfun))->type;
			if(!tptr)
				fp = 0;
		}
		else if(fp=nv_hasdisc(np,&type_disc))
			tptr = ((struct type*)fp)->type;
		valp = getvalue(np, fp, &cval);
	}
	else
	{
		valp->string.data = cp;
		valp->string.size = strlen(cp);
	}
	if(!tptr)
		tptr = tp->cx->state->type_string;
	comp = (*mp->compf)(tp->cx, tp->type, tptr, valp, &Dssdisc);
	if(!comp)
		return(3);
	if(bp->flags&SH_BEGIN_OPTIM)
		bp->data = comp;
exec:
	if(bp->vnode)
		fp = nv_hasdisc(bp->vnode, &child_disc);
	valp = getvalue(bp->vnode, fp, &cval);
	n = (*mp->execf)(tp->cx, comp, tptr, valp, &Dssdisc);
	if(!bp->data)
		(*mp->freef)(tp->cx, comp, &Dssdisc);
	return(n<0?4:!n);
}

void lib_init(int flag, void* context)
{
	Shell_t		*shp = ((Shbltin_t*)context)->shp;
	Dssstate_t	*state;
	Dsslib_t	*lib;
	Namval_t	*np,*rp;
	struct dsstype	*nfp = newof(NiL,struct dsstype,1,0);
	const char 	*name;
	char 		**av,tmp[sizeof(NV_CLASS)+17];
	int		level,i,len=0,n=0;
	state = dssinit(&Dssdisc,errorf);
	sfsprintf(tmp, sizeof(tmp), "%s.dss", NV_CLASS);
	np = nv_open(tmp, shp->var_tree, NV_VARNAME);
	typedict = nv_dict(np);
	nfp->fun.disc = &dss_disc;
	nv_disc(np,&nfp->fun,NV_FIRST);
	nv_adddisc(np,discnames,0);
	sfsprintf(tmp, sizeof(tmp), "%s.dss.load", NV_CLASS);
	sh_addbuiltin(tmp, loadlib, &Dssdisc); 
	sfsprintf(tmp, sizeof(tmp), "%s.dss.list", NV_CLASS);
	sh_addbuiltin(tmp, listdss, &Dssdisc); 
	/* create reference variable dss to NV_CLASS.dss */
	rp = nv_open("dss", shp->var_tree, NV_IDENT);
	nv_unset(rp);
	nv_putval(rp,nv_name(np),NV_NOFREE);
	nv_setref(rp,shp->var_tree,NV_VARNAME|NV_NOREF);
	for(level=0; level <2; level++)
	{
		char *cp;
		n = 0;
		for (lib = dsslib(NiL, DSS_VERBOSE, &Dssdisc); lib; lib = (Dsslib_t*)dtnext(state->cx->libraries, lib))
		{
			int m;
			if (!lib->queries)
				continue;
			for (i = 0; name = lib->queries[i].name; i++)
			{
				if(strcmp(name,"null")==0)
					continue;
				if(strcmp(name,"return")==0)
					continue;
				if(strcmp(name,"print")==0)
					continue;
				m = strlen(name)+1;
				if(level==0)
				{
					n++;
					len += m;
				}
				else
				{
					av[n++] = memcpy(cp,name,m);
					cp += m;
				}
			}
			if(level==0)
			{
				av = (char**)malloc((n+1)*sizeof(char*) + len);
				cp = (char*)&av[n+1];
			}
		}
	}
	nfp->names = av;
	nfp->nnames = n;
	nfp->sh = shp;
}

SHLIB(dss)
