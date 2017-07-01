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
 * David Korn
 * AT&T Labs
 *
 */
#include        "defs.h"
#include        "io.h"
#include        "variables.h"

static const char sh_opttype[] =
"[-1c?\n@(#)$Id: type (AT&T Labs Research) 2008-07-01 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - set the type of variables to \b\f?\f\b]"
"[+DESCRIPTION?\b\f?\f\b sets the type on each of the variables specified "
	"by \aname\a to \b\f?\f\b. If \b=\b\avalue\a is specified, "
	"the variable \aname\a is set to \avalue\a before the variable "
	"is converted to \b\f?\f\b.]"
"[+?If no \aname\as are specified then the names and values of all "
	"variables of this type are written to standard output.]" 
"[+?\b\f?\f\b is built-in to the shell as a declaration command so that "
	"field splitting and pathname expansion are not performed on "
	"the arguments.  Tilde expansion occurs on \avalue\a.]"
"[r?Enables readonly.  Once enabled, the value cannot be changed or unset.]"
"[a]:?[type?Indexed array. Each \aname\a will converted to an index "
	"array of type \b\f?\f\b.  If a variable already exists, the current "
	"value will become index \b0\b.  If \b[\b\atype\a\b]]\b is "
	"specified, each subscript is interpreted as a value of enumeration "
	"type \atype\a.]"
"[A?Associative array.  Each \aname\a will converted to an associate "
        "array of type \b\f?\f\b.  If a variable already exists, the current "
	"value will become subscript \b0\b.]"
"[h]:[string?Used within a type definition to provide a help string  "
	"for variable \aname\a.  Otherwise, it is ignored.]"
"[S?Used with a type definition to indicate that the variable is shared by "
	"each instance of the type.  When used inside a function defined "
	"with the \bfunction\b reserved word, the specified variables "
	"will have function static scope.  Otherwise, the variable is "
	"unset prior to processing the assignment list.]"
"[+DETAILS]\ftypes\f"
"\n"
"\n[name[=value]...]\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"

"[+SEE ALSO?\fother\f \breadonly\b(1), \btypeset\b(1)]"
;

typedef struct Namtype Namtype_t;
typedef struct Namchld
{
	Namfun_t	fun;
	Namtype_t 	*ptype;
	Namtype_t 	*ttype;
} Namchld_t;

struct Namtype
{
	Namfun_t	fun;
	Shell_t		*sh;
	Namval_t	*np;
	Namval_t	*parent;
	Namval_t	*bp;
	Namval_t	*cp;
#if SHOPT_NAMESPACE
	Namval_t	*nsp;
#endif /* SHOPT_NAMESPACE */
	char		*nodes;
	char		*data;
	Namchld_t	childfun;
	int		numnodes;
	char		**names;
	size_t		dsize;
	short		strsize;
	unsigned short	ndisc;
	unsigned short	current;
	unsigned short	nref;
};

#if 0
struct type
{
	Namtype_t	hdr;
	unsigned short	ndisc;
	unsigned short	current;
	unsigned short	nref;
};
#endif

typedef struct
{
	char		_cSfdouble_t;
	Sfdouble_t	_dSfdouble_t;
	char		_cdouble;
	double		_ddouble;
	char		_cfloat;
	float		_dfloat;
	char		_cSflong_t;
	Sflong_t	_dSflong_t;
	char		_clong;
	long		_dlong;
	char		_cshort;
	short		_dshort;
	char		_cpointer;
	char		*_dpointer;
	int32_t		_cint32_t;
	int32_t		*_dint32_t;
} _Align_;

#define alignof(t)	((char*)&((_Align_*)0)->_d##t-(char*)&((_Align_*)0)->_c##t)

static void put_type(Namval_t*, const char*, int, Namfun_t*);
static Namval_t* create_type(Namval_t*, const char*, int, Namfun_t*);
static Namfun_t* clone_type(Namval_t*, Namval_t*, int, Namfun_t*);
static Namval_t* next_type(Namval_t*, Dt_t*, Namfun_t*);

static const Namdisc_t type_disc =
{
	sizeof(Namtype_t),
	put_type,
	0,
	0,
	0,
	create_type,
	clone_type,
	0,
	next_type,
	0,
#if 0
	read_type
#endif
};

static char *Null = "";

size_t nv_datasize(Namval_t *np, size_t *offset)
{
	size_t s=0, a=0;
	if(nv_isattr(np,NV_INTEGER))
	{
		if(nv_isattr(np,NV_DOUBLE)==NV_DOUBLE)
		{
			if(nv_isattr(np, NV_LONG))
			{
				a = alignof(Sfdouble_t);
				s = sizeof(Sfdouble_t);
			}
			else if(nv_isattr(np, NV_SHORT))
			{
				a = alignof(float);
				s = sizeof(float);
			}
			else
			{
				a = alignof(double);
				s = sizeof(double);
			}
		}
		else
		{
			if(nv_isattr(np, NV_LONG))
			{
				a = alignof(Sflong_t);
				s = sizeof(Sflong_t);
			}
			else if(nv_isattr(np, NV_SHORT))
			{
				a = alignof(short);
				s = sizeof(short);
			}
			else
			{
				a = alignof(int32_t);
				s = sizeof(int32_t);
			}
		}
	}
	else if(nv_isattr(np, NV_BINARY) || nv_isattr(np,NV_LJUST|NV_RJUST|NV_ZFILL))
		s = nv_size(np);
	else
	{
		a = alignof(pointer);
		s = nv_size(np);
	}
	if(a>1 && offset)
		*offset = a*((*offset +a-1)/a);
	return(s);
}

static char *name_chtype(Namval_t *np, Namfun_t *fp)
{
	Namchld_t	*pp = (Namchld_t*)fp;
	Shell_t		*shp = pp->ptype->sh;
	char		*cp, *sub;
	Namval_t	*tp = shp->last_table;
	Namval_t	*nq = pp->ptype->np;
	Namarr_t	*ap;
	if(nv_isattr(np,NV_REF|NV_TAGGED)==(NV_REF|NV_TAGGED))
		shp->last_table = 0;
	cp = nv_name(nq);
	if((ap = nv_arrayptr(nq)) && !(ap->flags&ARRAY_UNDEF) && (sub= nv_getsub(nq)))
		sfprintf(shp->strbuf,"%s[%s].%s",cp,sub,np->nvname);
	else
		sfprintf(shp->strbuf,"%s.%s",cp,np->nvname);
#if SHOPT_FIXEDARRAY
	if((ap=nv_arrayptr(np)) && ap->fixed)
		nv_arrfixed(np,shp->strbuf,1,(char*)0);
#endif /* SHOPT_FIXEDARRAY */
	shp->last_table = tp;
	return(sfstruse(shp->strbuf));
}

static void put_chtype(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	if(!val && nv_isattr(np,NV_REF))
		return;
	nv_putv(np,val,flag,fp);
	if(!val)
	{
		Namchld_t	*pp = (Namchld_t*)fp;
		size_t		dsize=0,offset = (char*)np-(char*)pp->ptype;
		Namval_t	*mp = (Namval_t*)((char*)pp->ttype+offset);
		dsize = nv_datasize(mp,&dsize);
		if(mp->nvalue.cp >=  pp->ttype->data && mp->nvalue.cp < (char*)pp->ttype +pp->ttype->fun.dsize)
		{
			np->nvalue.cp = pp->ptype->data + (mp->nvalue.cp-pp->ttype->data);
			if(np->nvalue.cp!=mp->nvalue.cp)
				memcpy((char*)np->nvalue.cp,mp->nvalue.cp,dsize);
		}
		else if(!nv_isarray(mp) && mp->nvalue.cp)
		{
			np->nvalue.cp = mp->nvalue.cp;
			nv_onattr(np,NV_NOFREE);
		}
		np->nvsize = mp->nvsize;
		np->nvflag = mp->nvflag&~NV_RDONLY;
	}
}

static Namfun_t *clone_chtype(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	if(flags&NV_NODISC)
		return(0);
	return(nv_clone_disc(fp,flags));
}

static Namval_t *create_chtype(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	Namchld_t	*xp = (Namchld_t*)fp;
	Namval_t	*pp = xp->ptype->parent, *nq;
	Shell_t		*shp = np->nvshell;
	for(fp=pp->nvfun; fp; fp=fp->next)
	{
		if(fp->disc && (fp->disc->createf==create_type || fp->disc->createf==create_chtype))
		{
			sfprintf(shp->strbuf,"%s.%s",np->nvname,name);
			if(nq = (*fp->disc->createf)(pp,sfstruse(shp->strbuf),flag,fp))
				xp->fun.last = fp->last;
			return(nq);
		}
	}
	return(0);
}

static const Namdisc_t chtype_disc =
{
	sizeof(Namchld_t),
	put_chtype,
	0,
	0,
	0,
	create_chtype,
	clone_chtype,
	name_chtype
};

static Namval_t *findref(void *nodes, int n)
{
	Namval_t	*tp,*np = nv_namptr(nodes,n);
	char		*name = np->nvname;
	int		i=n, len= strrchr(name,'.')-name;
	Namtype_t	*pp;
	while(--i>0)
	{
		np = nv_namptr(nodes,i);
		if(np->nvname[len]==0)
		{
			tp = nv_type(np);
			pp = (Namtype_t*)nv_hasdisc(tp,&type_disc);
			return(nv_namptr(pp->nodes,n-i-1));
		}
	}
	return(0);
}

static int fixnode(Namtype_t *dp, Namtype_t *pp, int i, struct Namref *nrp,int flag)
{
	Namval_t	*nq = nv_namptr(dp->nodes,i);
	Namfun_t	*fp;
	if(fp=nv_hasdisc(nq,&chtype_disc))
		nv_disc(nq, fp, NV_POP);
	if(nv_isattr(nq,NV_REF))
	{
		nq->nvalue.nrp = nrp++;
		nv_setsize(nq,0);
		if(strchr(nq->nvname,'.'))
			nq->nvalue.nrp->np = findref(dp->nodes,i);
		else
			nq->nvalue.nrp->np = nv_namptr(pp->childfun.ttype->nodes,i);
		nq->nvalue.nrp->root = pp->sh->last_root;
		nq->nvalue.nrp->table = pp->np;
		nq ->nvflag = NV_REF|NV_NOFREE|NV_MINIMAL;
		return(1);
	}
	if(nq->nvalue.cp || nq->nvfun)
	{
		const char *data = nq->nvalue.cp;
		if(nq->nvfun)
		{
			Namval_t *np = nv_namptr(pp->nodes,i);
			if(nv_isarray(nq))
				nq->nvalue.cp = 0;
			nq->nvfun = 0;
			if(nv_isarray(nq) && ((flag&NV_IARRAY) || nv_type(np)))
			{
				Shell_t *shp = sh_ptr(np);
				Namval_t *last_table = shp->last_table;
				if(nv_type(np))
					shp->last_table = np;
				clone_all_disc(np,nq,flag&~NV_TYPE);
				shp->last_table = last_table;
			}
			else
				clone_all_disc(np,nq,flag);
			if(fp)
				nv_disc(np, fp, NV_LAST);
		}
#if 0
		if(nq->nvalue.cp >=  pp->data && nq->nvalue.cp < (char*)pp +pp->fun.dsize)
			nq->nvalue.cp = dp->data + (nq->nvalue.cp-pp->data);
#else
		if(data >=  pp->data && data < (char*)pp +pp->fun.dsize)
			nq->nvalue.cp = dp->data + (data-pp->data);
#endif
		else if(!nq->nvfun && pp->childfun.ttype!=pp->childfun.ptype)
		{
			Namval_t *nr = nv_namptr( pp->childfun.ttype->nodes,i);
			if(nr->nvalue.cp!=nq->nvalue.cp)
			{
				if(i=nv_size(nq))
				{
					const char *cp = nq->nvalue.cp;
					nq->nvalue.cp = (char*)malloc(i);
					memcpy((char*)nq->nvalue.cp,cp,i);
				}
				else
					nq->nvalue.cp = strdup(nq->nvalue.cp);
				nv_offattr(nq,NV_NOFREE);
			}
		}
		else if(nq->nvalue.cp==Null)
			nq->nvalue.cp = Empty;
		else if(nq->nvalue.cp==Empty)
			nv_offattr(nq,NV_NOFREE);
	
	}
	if(fp)
		nv_disc(nq, &dp->childfun.fun, NV_LAST);
	return(0);
}

static Namfun_t *clone_type(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	Namtype_t		*dp, *pp=(Namtype_t*)fp;
	Shell_t			*shp = (Shell_t*)np->nvshell;
	register int		i;
	register Namval_t	*nq, *nr;
	size_t			size = fp->dsize;
	int			save, offset=stktell(shp->stk);
	char			*cp;
	Dt_t			*root = shp->last_root;
	Namval_t		*last_table = shp->last_table;
	struct Namref		*nrp = 0;
	Namarr_t		*ap;
	if(flags&NV_MOVE)
	{
		pp->np = mp;
		pp->parent = mp;
		pp->childfun.ptype = pp;
		return(fp);
	}
	if(flags&NV_TYPE)
		return(nv_clone_disc(fp,flags));
	if(size==0 && (!fp->disc || (size=fp->disc->dsize)==0)) 
		size = sizeof(Namfun_t);
	dp = (Namtype_t*)malloc(size+pp->nref*sizeof(struct Namref));
	if(pp->nref)
	{
		nrp = (struct Namref*)((char*)dp + size);
		memset((void*)nrp,0,pp->nref*sizeof(struct Namref));
	}
	memcpy((void*)dp,(void*)pp,size);
#if 0
	dp->parent = nv_lastdict(np->nvshell);
#else
	dp->parent = mp;
#endif
	dp->fun.nofree = (flags&NV_RDONLY?1:0);
	dp->np = mp;
	dp->childfun.ptype = dp;
#if 0
	dp->childfun.ttype = (Namtype_t*)nv_hasdisc(dp->fun.type,&type_disc);
#endif
	dp->nodes = (char*)(dp+1);
	dp->data = (char*)dp + (pp->data - (char*)pp);
	for(i=dp->numnodes; --i >= 0; )
	{
		Namtype_t *xp;
		nq = nv_namptr(dp->nodes,i);
		nq->nvshell = np->nvshell;
		if(fixnode(dp,pp,i,nrp,NV_TYPE|(flags&NV_IARRAY)))
		{
			nrp++;
			nq = nq->nvalue.nrp->np;
		}
		else if(nv_isattr(nq,NV_RDONLY) && nv_isattr(nq,NV_INTEGER))
		{
			if(nv_type(np)!=np || nv_getnum(nq)==0)
				nq->nvalue.cp = Empty;
		}
		if(xp = (Namtype_t*)nv_hasdisc(nq,&type_disc))
			xp->parent = mp;
		if(flags==(NV_NOFREE|NV_ARRAY))
			continue;
		if(nq->nvalue.cp || !nv_isvtree(nq) || nv_isattr(nq,NV_RDONLY))
		{
			/* see if default value has been overwritten */
			if(!mp->nvname)
				continue;
			shp->last_table = last_table;
			if(pp->strsize<0)
				cp = nv_name(np);
			else
				cp = nv_name(mp);
			sfputr(shp->stk,cp,'.');
			sfputr(shp->stk,nq->nvname,0);
			root = nv_dict(mp);
			save = fp->nofree;
			fp->nofree = 1;
			nr = nv_create(stkptr(shp->stk,offset),root,NV_VARNAME|NV_NOADD,fp);
			fp->nofree = save;
			stkseek(shp->stk,offset);
			if(nr)
			{
				if(nv_isattr(nq,NV_RDONLY) && nv_type(np)!=np && (nq->nvalue.cp || nv_isattr(nq,NV_INTEGER)))
					errormsg(SH_DICT,ERROR_exit(1),e_readonly, nq->nvname);
				if(nv_isref(nq))
					nq = nv_refnode(nq);
				if((size = nv_datasize(nr,(size_t*)0)) && size==nv_datasize(nq,(size_t*)0))
				{
					if(nv_isattr(nr,NV_INT16|NV_DOUBLE) == NV_INT16)
					{
						if(nv_isattr(nq,NV_INT16P)==NV_INT16P)
							*nq->nvalue.sp = nr->nvalue.s;
						else
							nq->nvalue.cp = nr->nvalue.cp;
					}
					else
						memcpy((char*)nq->nvalue.cp,nr->nvalue.cp,size);
				}
				else if(ap=nv_arrayptr(nr))
				{
					nv_putsub(nr,NIL(char*),0,ARRAY_SCAN|ARRAY_NOSCOPE);
					do
					{
						if(array_assoc(ap))
							cp = (char*)((*ap->fun)(nr,NIL(char*),NV_ANAME));
						else
							cp = nv_getsub(nr);
						nv_putsub(nq,cp,0,ARRAY_ADD|ARRAY_NOSCOPE);
						if(array_assoc(ap))
						{
							Namval_t *mr = (Namval_t*)((*ap->fun)(nr,NIL(char*),NV_ACURRENT));
							Namval_t *mq = (Namval_t*)((*ap->fun)(nq,NIL(char*),NV_ACURRENT));
							nv_clone(mr,mq,NV_MOVE);
							ap->nelem--;
							nv_delete(mr,ap->table,0);
						}
						else
						{
							cp = nv_getval(nr);
							nv_putval(nq,cp,0);
						}
					}
					while(nv_nextsub(nr));
				}
				else
					nv_putval(nq,nv_getval(nr),NV_RDONLY);
				if(shp->mktype)
					nv_addnode(nr,1);
				if(pp->strsize<0)
					continue;
				if(nr != (Namval_t*)np->nvenv)
				{
					_nv_unset(nr,0);
					if(!nv_isattr(nr,NV_MINIMAL))
						nv_delete(nr,shp->last_root,0);
				}
			}
		}
	}
	if(nv_isattr(mp,NV_BINARY))
	{
		mp->nvalue.cp = dp->data;
		nv_onattr(mp,NV_NOFREE);
	}
	if(pp->strsize<0)
		dp->strsize = -pp->strsize;
	return(&dp->fun);
}


/*
 * return Namval_t* corresponding to child <name> in <np>
 * try complete match first, otherwise find match to first .
 */
static Namval_t *create_type(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	Namtype_t		*dp = (Namtype_t*)fp;
	register const char	*cp=name;
	register int		i=0,n,r=0,s=0;
	Namval_t		*nq=0;
	if(!name)
		return(dp->parent);
	if(*name=='_' && name[1]=='_' && (name[2]=='.' || name[2]==0))
	{
		fp->last = (char*)&name[2];
		return(dp->parent);
	}
	while((n=*cp++) && n != '=' && n != '+' && n!='[')
	{
		if(n=='.') 
			r = (cp-1)-name;
	}
	n = (cp-1) -name;
	if(dp->numnodes && dp->strsize<0)
	{
		char *base =  (char*)np-(NV_MINSZ-sizeof(Dtlink_t));
		size_t m=strlen(np->nvname);
		while(i<=dp->numnodes && (nq=nv_namptr(base,++i)) && strncmp(nq->nvname,np->nvname,m)==0)
		{
			if(nq->nvname[m]=='.' && strncmp(name,&nq->nvname[m+1],n)==0 && nq->nvname[m+n+1]==0)
				goto found;
		}
		nq = 0;
	}
	else for(i=0; i < dp->numnodes; i++)
	{
		nq = nv_namptr(dp->nodes,i);
		if(!nq->nvname)
			continue;
		if((n==0||strncmp(name,nq->nvname,n)==0) && nq->nvname[n]==0)
		{
			while(nv_isref(nq))
				nq = nq->nvalue.nrp->np;
			goto found;
		}
		if(r && nq->nvname[r]==0 && strncmp(name,nq->nvname,r)==0)
			s = i;
	}
	nq = 0;
	if(s)
	{
		nq = nv_namptr(dp->nodes,s);
		n = r;
	}
found:
	if(nq)
	{
		fp->last = (char*)&name[n];
		dp->sh->last_table = dp->parent;
	}
	else
	{
		if(name[n]!='=') for(i=0; i < dp->ndisc; i++)
		{
			if((strncmp(name,dp->names[i],n)==0) && dp->names[i][n]==0)
				return(nq);
		}
		if(!(flag&NV_NOFAIL))
			errormsg(SH_DICT,ERROR_exit(1),e_notelem,n,name,nv_name(np));
	}
	return(nq);
}

static void put_type(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	Shell_t	*shp = sh_ptr(np);
	Namval_t	*nq;
	if(val && (nq=nv_open(val,shp->var_tree,NV_VARNAME|NV_ARRAY|NV_NOADD|NV_NOFAIL))) 
	{
		Namfun_t  *pp;
		if((pp=nv_hasdisc(nq,fp->disc)) && pp->type==fp->type)

		{
			if(!nq->nvenv)
				flag |= NV_EXPORT;
			_nv_unset(np, flag);
			nv_clone(nq,np,NV_IARRAY);
			return;
		}
	}
	nv_putv(np,val,flag,fp);
	if(!val)
	{
		Namtype_t	*dp = (Namtype_t*)fp;
		Namarr_t	*ap;
		int		i;
		if(nv_isarray(np) && (ap=nv_arrayptr(np)) && ap->nelem>0)
			return;
		for(i=0; i < dp->numnodes; i++)
		{
			nq = nv_namptr(dp->nodes,i);
			if(ap=nv_arrayptr(nq))
				ap->flags |= ARRAY_UNDEF;
			if(!nv_hasdisc(nq,&type_disc))
				_nv_unset(nq,flag|NV_TYPE|nv_isattr(nq,NV_RDONLY));
		}
		nv_disc(np,fp,NV_POP);
		if(!(fp->nofree&1))
			free((void*)fp);
	}
}

static Namval_t *next_type(register Namval_t* np, Dt_t *root,Namfun_t *fp)
{
	Namtype_t	*dp = (Namtype_t*)fp;
	Namarr_t	*ap;
	if(!root)
	{
		if((ap=nv_arrayptr(np)) && ap->nelem && (ap->flags&ARRAY_UNDEF))
			nv_putsub(np,(char*)0,0,ARRAY_SCAN);
		dp->current = 0;
	}
	else if(++dp->current>=dp->numnodes)
	{
		if((ap = nv_arrayptr(dp->parent)) && !ap->fun && (ap->flags&ARRAY_SCAN) && nv_nextsub(dp->parent))
			dp->current = 0;
		else
			return(0);
	}
	return(nv_namptr(dp->nodes,dp->current));
}

static Namfun_t *clone_inttype(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	Namfun_t	*pp=  (Namfun_t*)malloc(fp->dsize);
	memcpy((void*)pp, (void*)fp, fp->dsize);
	fp->nofree &= ~1;
	if(nv_isattr(mp,NV_NOFREE) && mp->nvalue.cp)
		memcpy((void*)mp->nvalue.cp,np->nvalue.cp, fp->dsize-sizeof(*fp));
	else
		mp->nvalue.cp = (char*)(fp+1);
	if(!nv_isattr(mp,NV_MINIMAL))
		mp->nvenv = 0;
	nv_offattr(mp,NV_RDONLY);
	return(pp);
}

static int typeinfo(Opt_t* op, Sfio_t *out, const char *str, Optdisc_t *od)
{
	Shell_t	*shp = sh_getinterp();
	char		*cp,**help,buffer[256];
	Namtype_t	*dp;
	Namval_t	*np,*nq,*tp;
	int		i, offset=stktell(shp->stk);
	size_t		n;
	Sfio_t		*sp;
	
	np = *(Namval_t**)(od+1);
	sfputr(shp->stk,NV_CLASS,'.');
	sfputr(shp->stk,np->nvname,0);
	np = nv_open(cp=stkptr(shp->stk,offset), shp->var_tree, NV_NOADD|NV_VARNAME);
	stkseek(shp->stk,offset);
	if(!np)
	{
		sfprintf(sfstderr,"%s: no such variable\n",cp);
		return(-1);
	}
	if(!(dp=(Namtype_t*)nv_hasdisc(np,&type_disc)))
	{
		Namfun_t *fp;
		for(fp=np->nvfun;fp;fp=fp->next)
		{
			if(fp->disc && fp->disc->clonef==clone_inttype)
				break;
		}
		if(!fp)
		{
			sfprintf(sfstderr,"%s: not a type\n",np->nvname);
			return(-1);
		}
		if(strcmp(str,"other")==0)
			return(0);
		tp = fp->type;
		nv_offattr(np,NV_RDONLY);
		fp->type = 0;
		if(np->nvenv)
			sfprintf(out,"[+?\b%s\b is a %s.]\n", tp->nvname, np->nvenv);
		cp = (char*)out->_next;
		sfprintf(out,"[+?\b%s\b is a %n ", tp->nvname, &i);
		nv_attribute(np,out,(char*)0, 1);
		if(cp[i+1]=='i')
			cp[i-1]='n';
		fp->type = tp;
		nv_onattr(np,NV_RDONLY);
		sfprintf(out," with default value \b%s\b.]",nv_getval(np));
		return(0);
	}
	if(strcmp(str,"other")==0)
	{
		Nambfun_t	*bp;
		if(bp=(Nambfun_t*)nv_hasdisc(np,nv_discfun(NV_DCADD)))
		{
			for(i=0; i < bp->num; i++)
			{
				if(nv_isattr(bp->bltins[i],NV_OPTGET))
					sfprintf(out,"\b%s.%s\b(3), ",np->nvname,bp->bnames[i]);
                        }
		}
		return(0);
	}
	help = &dp->names[dp->ndisc];
	sp = sfnew((Sfio_t*)0,buffer,sizeof(buffer),-1,SF_STRING|SF_WRITE);
	sfprintf(out,"[+?\b%s\b defines the following fields:]{\n",np->nvname);
	for(i=0; i < dp->numnodes; i++)
	{
		nq = nv_namptr(dp->nodes,i);
		if(strchr(nq->nvname,'.'))
			continue;
		if(tp=nv_type(nq))
		{
			Namfun_t *pp = nv_hasdisc(nq,&type_disc);
			sfprintf(out,"\t[+%s?%s.\n",nq->nvname,tp->nvname);
			n = strlen(nq->nvname);
			while((cp=nv_namptr(dp->nodes,i+1)->nvname) && strncmp(cp,nq->nvname,n)==0 && cp[n]=='.')
				i++;
		}
		else
		{
			sfseek(sp,(Sfoff_t)0, SEEK_SET);
			nv_attribute(nq,sp,(char*)0,1);
			cp = 0;
			if(!nv_isattr(nq,NV_REF))
				cp = sh_fmtq(nv_getval(nq));
			sfputc(sp,0);
			for(n=strlen(buffer); n>0 && buffer[n-1]==' '; n--);
			buffer[n] = 0;
			if(nq->nvalue.cp==Empty && nv_isattr(nq,NV_RDONLY))
				sfprintf(out,"\t[+%s?%s, default value is required at creation\n",nq->nvname,*buffer?buffer:"string");
			else if(cp)
				sfprintf(out,"\t[+%s?%s, default value is %s.\n",nq->nvname,*buffer?buffer:"string",cp);
			else
				sfprintf(out,"\t[+%s?%s.\n",nq->nvname,*buffer?buffer:"string");
		}
		if(help[i])
			sfprintf(out,"  %s.",help[i]);
		sfputc(out,']');
	}
	sfprintf(out,"}\n");
	if(dp->ndisc>0)
	{
		stkseek(shp->stk,offset);
		sfputr(shp->stk,NV_CLASS,'.');
		sfputr(shp->stk,np->nvname,'.');
		n = stktell(shp->stk);
		sfprintf(out,"[+?\b%s\b defines the following discipline functions:]{\n",np->nvname);
		for(i=0; i < dp->ndisc; i++)
		{
			char *dot;
			sfputr(shp->stk,dp->names[i],0);
			dot = cp = Empty;
			if((nq = nv_search(stkptr(shp->stk,offset),shp->fun_tree,0)) && nq->nvalue.cp)
			{
				cp = nq->nvalue.rp->help;
				dot = ".";
			}
			if(nq && nv_isattr(nq,NV_STATICF))
				sfprintf(out,"\t[+%s?:static:%s%s]\n",dp->names[i],cp,dot);
			else
				sfprintf(out,"\t[+%s?%s%s]\n",dp->names[i],cp,dot);
			stkseek(shp->stk,n);
		}
		sfprintf(out,"}\n");
	}
	stkseek(shp->stk,offset);
	sfclose(sp);
	return(0);
}

static int std_disc(Namval_t *mp, Namtype_t *pp)
{
	register const char	*sp, *cp = strrchr(mp->nvname,'.');
	register const char	**argv;
	register int			i;
	Namval_t		*np=0,*nq;
	if(cp)
		cp++;
	else
		cp = mp->nvname;
	if(strcmp(cp,"create")==0)
	{
		if(pp)
			pp->cp = mp;
		return(0);
	}
	for(argv=nv_discnames; sp=*argv; argv++) 
	{
		if(strcmp(cp,sp)==0)
		{
			if(!pp)
				return(1);
			goto found;
		}
	}
	return(0);
found:
	if(strncmp(sp=mp->nvname,NV_CLASS,sizeof(NV_CLASS)-1)==0)
		sp += sizeof(NV_CLASS);
	sp += strlen(pp->fun.type->nvname)+1;
	if(sp == cp)
		np = pp->fun.type;
	else for(i=1; i < pp->numnodes; i++)
	{
		nq = nv_namptr(pp->nodes,i);
		if(strncmp(nq->nvname, sp, cp-sp-1)==0)
		{
			np = nq;
			break;
		}
	}
	if(np)
	{
		nv_onattr(mp,NV_NOFREE);
		if(!nv_setdisc(np,cp, mp, (Namfun_t*)np))
			sfprintf(sfstderr," nvsetdisc failed name=%s sp=%s cp=%s\n",np->nvname,sp,cp);
	}
	else
		sfprintf(sfstderr,"can't set discipline %s cp=%s \n",sp,cp);
	return(1);
}


void nv_addtype(Namval_t *np, const char *optstr, Optdisc_t *op, size_t optsz)
{
	Namdecl_t	*cp = newof((Namdecl_t*)0,Namdecl_t,1,optsz);
	Optdisc_t	*dp = (Optdisc_t*)(cp+1);
	Shell_t		*shp = sh_ptr(np);
	Namval_t	*mp,*bp;
	char		*name;
	if(optstr)
		cp->optstring = optstr;
	else
		cp->optstring = sh_opttype;
	memcpy((void*)dp,(void*)op, optsz);
	cp->optinfof = (void*)dp;
	cp->tp = np;
	mp = nv_search("typeset",shp->bltin_tree,0);
	if(name=strrchr(np->nvname,'.'))
		name++;
	else
		name = np->nvname; 
#if SHOPT_NAMESPACE
	if(bp=(Namval_t*)shp->namespace)
	{
		Namtype_t *tp = (Namtype_t*)nv_hasdisc(np, &type_disc);
		if(tp)
			tp->nsp = bp;
		if(!shp->strbuf2)
			shp->strbuf2 = sfstropen();
		sfprintf(shp->strbuf2,".%s.%s%c\n",nv_name(bp)+1,name,0);
		name = sfstruse(shp->strbuf2);
	}
#endif /* SHOPT_NAMESPACE */
	if((bp=nv_search(name,shp->fun_tree,NV_NOSCOPE)) && !bp->nvalue.ip)
		nv_delete(bp,shp->fun_tree,0);
	bp = sh_addbuiltin(shp,name, (Shbltin_f)mp->nvalue.bfp, (void*)cp); 
	nv_onattr(bp,nv_isattr(mp,NV_PUBLIC));
	nv_onattr(np, NV_RDONLY);
}

void nv_newtype(Namval_t *mp)
{
	struct	{
		    Optdisc_t	opt;
		    Namval_t	*np;
		}	optdisc;
	memset(&optdisc,0,sizeof(optdisc));
	optdisc.opt.infof = typeinfo;
	optdisc.np = mp;
	nv_addtype(mp,sh_opttype, &optdisc.opt, sizeof(optdisc));
}

/*
 * This function creates a type out of the <numnodes> nodes in the
 * array <nodes>.  The first node is the name for the type
 */
Namval_t *nv_mktype(Namval_t **nodes, int numnodes)
{
	Namval_t	*mp=nodes[0], *bp=0, *np, *nq, **mnodes=nodes;
	int		i,j,k,nd=0,nref=0,iref=0,inherit=0;
	int		size=sizeof(NV_DATA), dsize=0, nnodes;
	int		binary=NV_BINARY|NV_RAW;
	size_t		offset=0,m;
	ssize_t		n;
	char		*name=0, *cp, *sp, **help;
	Namtype_t	*pp,*qp=0,*dp,*tp;
	Dt_t		*root = nv_dict(mp);
	struct Namref	*nrp = 0;
	Namfun_t	*fp;
	m = strlen(mp->nvname)+1;
	if(numnodes < 2)
	{
		cp = nodes[0]->nvname;
		_nv_unset(nodes[0],NV_RDONLY);
		errormsg(SH_DICT,ERROR_exit(1),e_badtypedef,cp);
	}
	for(nnodes=1,i=1; i <numnodes; i++)
	{
		np=nodes[i];
		if(is_afunction(np))
		{
			if(!std_disc(np, (Namtype_t*)0))
			{
				size += strlen(np->nvname+m)+1;
				if(strncmp(np->nvname,NV_CLASS,sizeof(NV_CLASS)-1)==0)
					size -= sizeof(NV_CLASS);
				nd++;
			}
			continue;
		}
		if(nv_isattr(np,NV_REF))
			iref++;
		if(np->nvenv)
			size += strlen((char*)np->nvenv)+1;
		if(strcmp(&np->nvname[m],NV_DATA)==0 && !nv_type(np))
			continue;
		if(qp)
		{	/* delete duplicates */
			for(j=0; j < qp->numnodes;j++)
			{
				nq = nv_namptr(qp->nodes,j);
				if(strcmp(nq->nvname,&np->nvname[m])==0)
					break;
			}
			if(j < qp->numnodes)
				continue;
		}
		nnodes++;
		if(name && strncmp(&name[m],&np->nvname[m],n)==0 && np->nvname[m+n]=='.')
			offset -= sizeof(char*);
		dsize = nv_datasize(np,&offset);
		if(!nv_isarray(np) && (dp=(Namtype_t*)nv_hasdisc(np, &type_disc)))
		{
			nnodes += dp->numnodes;
			if((n=dp->strsize)<0)
				n = -n;
			iref = nref += dp->nref;
			if(np->nvname[m]=='_' && np->nvname[m+1]==0 && (bp=nv_type(np)))
			{
				qp = dp;
				nd = dp->ndisc;
				nnodes = dp->numnodes;
				offset = 0;
				dsize = nv_size(np);
				size += n;
			}
			else
				size += n + dp->numnodes*(strlen(&np->nvname[m])+1);
			n = strlen(np->nvname);
			while((i+1) < numnodes && (cp=nodes[i+1]->nvname) && strncmp(cp,np->nvname,n)==0 && cp[n]=='.')
				i++;
		}
		else if(nv_isarray(np) && nv_isattr(np,NV_NOFREE))
			binary = 0;
		else if(nv_isattr(np,NV_REF))
			nref++;
		offset += (dsize?dsize:4);
		size += (n=strlen(name=np->nvname)-m+1);
	}
	offset = roundof(offset,sizeof(char*));
	nv_setsize(mp,offset);
	if(nd)
		nd++;
	k = roundof(sizeof(Namtype_t)+nnodes*NV_MINSZ,sizeof(Sfdouble_t)) - (sizeof(Namtype_t)+nnodes*NV_MINSZ);
	pp = newof(NiL, Namtype_t, 1, nnodes*NV_MINSZ + offset + size + (nnodes+nd)*sizeof(char*) + iref*sizeof(struct Namref)+k);
	pp->fun.dsize = sizeof(Namtype_t)+nnodes*NV_MINSZ +offset+k;
	pp->fun.type = mp;
	pp->sh = sh_ptr(mp);
	pp->parent = nv_lastdict(mp->nvshell);
	pp->np = mp;
	pp->bp = bp;
	pp->childfun.fun.disc = &chtype_disc;
	pp->childfun.fun.nofree = 1;
	pp->childfun.ttype = pp;
	pp->childfun.ptype = pp;
	pp->fun.disc = &type_disc;
	pp->nodes = (char*)(pp+1);
	pp->numnodes = nnodes;
	pp->data = pp->nodes + nnodes*NV_MINSZ +k;
	pp->dsize = offset;
	nrp = (struct Namref*)(pp->data+offset);
	pp->names = (char**)(nrp+iref);
	help = &pp->names[nd];
	pp->strsize = size;
	cp = (char*)&pp->names[nd+nnodes];
	if(qp)
		mnodes = newof(NiL, Namval_t*, nd+1, 0);
	nd = 0;
	nq = nv_namptr(pp->nodes,0);
	nq->nvname = cp;
	nv_onattr(nq,NV_MINIMAL);
	cp = strcopy(cp,NV_DATA);
	*cp++ = 0;
	nq->nvshell = mp->nvshell;
	for(name=0, offset=0, k=i=1; i < numnodes; i++)
	{
		np=nodes[i];
		np->nvshell = mp->nvshell;
		if(is_afunction(np))
		{
			sp = np->nvname+m;
			if(strncmp(np->nvname,NV_CLASS,sizeof(NV_CLASS)-1)==0)
				sp += sizeof(NV_CLASS);
			if(!std_disc(np, pp))
			{
				/* see if discipline already defined */
				for(j=0; j< nd; j++)
				{
					if(strcmp(sp,pp->names[j])==0)
					{
						mnodes[j] = nodes[i];
						break;
					}
				}
				if(j>=nd)
				{
					pp->names[nd] = cp;
					mnodes[nd++] = nodes[i];
					cp = strcopy(cp,sp);
					*cp++ = 0;
				}
				nv_onattr(mnodes[j],NV_NOFREE);
			}
			continue;
		}
		if(inherit)
		{
			for(j=0; j < k ; j++)
			{
				nq = nv_namptr(pp->nodes,j);
				nq->nvshell = mp->nvshell;
				if(strcmp(nq->nvname,&np->nvname[m])==0)
					break;
			}
			if(j < k)
			{
				sp = nv_getval(np);
				if(nv_isvtree(np))
					sfprintf(sfstderr,"initialization not implemented\n");
				else if(sp) 
					nv_putval(nq,sp,0);
				goto skip;
			}
		}
		if(strcmp(&np->nvname[m],NV_DATA)==0 && !nv_type(np))
		{
			char *val=nv_getval(np);
			nq = nv_namptr(pp->nodes,0);
			nq->nvfun = 0;
			nv_putval(nq,(val?val:0),nv_isattr(np,~(NV_IMPORT|NV_EXPORT|NV_ARRAY)));
			nq->nvflag = np->nvflag|NV_NOFREE|NV_MINIMAL;
			goto skip;
		}
		if(qp)
		{
			Nambfun_t	*bfp;
			dp = (Namtype_t*)nv_hasdisc(nv_type(np), &type_disc);
			memcpy(pp->nodes,dp->nodes,dp->numnodes*NV_MINSZ);
			offset = nv_size(np);
			memcpy(pp->data,dp->data,offset);
			for(k=0;k < dp->numnodes; k++)
			{
				Namval_t *nr = nv_namptr(qp->nodes,k);
				nq = nv_namptr(pp->nodes,k);
				if(fixnode(pp,dp,k,nrp,0))
				{
					nrp++;
					nq = nq->nvalue.nrp->np;
				}
				if(!nv_isattr(nr,NV_REF) && !nv_hasdisc(nr,&type_disc))
				{
					if(nr->nvsize)
						memcpy((char*)nq->nvalue.cp,nr->nvalue.cp,size=nv_datasize(nr,(size_t*)0));
					else
					{
						nq->nvalue.cp = nr->nvalue.cp;
						nv_onattr(nq,NV_NOFREE);
					}
				}
			}
			if(bfp=(Nambfun_t*)nv_hasdisc(np,nv_discfun(NV_DCADD)))
			{
				for(j=0; j < bfp->num; j++)
				{
					pp->names[nd++] = (char*)bfp->bnames[j];
					mnodes[j] = bfp->bltins[j];
				}
			}
			qp = 0;
			inherit=1;
			goto skip;
		}
		nq = nv_namptr(pp->nodes,k);
		nq->nvshell = mp->nvshell;
		if(np->nvenv)
		{
			/* need to save the string pointer */
			nv_offattr(np,NV_EXPORT);
			help[k-1] = cp;
			cp = strcopy(cp,np->nvenv);
			j = *help[k-1];
			if(islower(j))
				*help[k-1] = toupper(j);
			*cp++ = 0;
			np->nvenv = 0;
		}
		nq->nvname = cp;
		dsize = nv_datasize(np,&offset);
		cp = strcopy(name=cp, &np->nvname[m]);
		n = cp-name;
		*cp++ = 0;
		nq->nvsize = np->nvsize;
		nq->nvflag = (np->nvflag&~(NV_IMPORT|NV_EXPORT))|NV_NOFREE|NV_MINIMAL;
		if(dp = (Namtype_t*)nv_hasdisc(np, &type_disc))
		{
			int r,kfirst=k;
			char *cname = &np->nvname[m];
			/*
			 * If field is a type, mark the type by setting
			 * strsize<0.  This changes create_type()
			 */
			clone_all_disc(np,nq,NV_RDONLY);
			if(nv_isarray(np))
			{
				nv_disc(nq, &pp->childfun.fun, NV_LAST);
				k++;
				goto skip;
			}
			if(fp=nv_hasdisc(nq,&chtype_disc))
				nv_disc(nq, &pp->childfun.fun, NV_LAST);
			if(tp = (Namtype_t*)nv_hasdisc(nq, &type_disc))
				tp->strsize = -tp->strsize;
			for(r=0; r < dp->numnodes; r++)
			{
				Namval_t *nr = nv_namptr(dp->nodes,r);
				nq = nv_namptr(pp->nodes,++k);
				nq->nvname = cp;
				nq->nvshell = mp->nvshell;
				dsize = nv_datasize(nr,&offset);
				nq->nvflag = nr->nvflag;
				if(nv_hasdisc(nr, &type_disc))
					clone_all_disc(nr,nq,NV_RDONLY);
				if(nr->nvalue.cp)
				{
					Namchld_t *xp = (Namchld_t*)nv_hasdisc(nr,&chtype_disc);
					if(xp && nr->nvalue.cp >= xp->ptype->data && nr->nvalue.cp < xp->ptype->data+xp->ptype->fun.dsize)
					{
						nq->nvalue.cp = pp->data+offset;
						memcpy((char*)nq->nvalue.cp,nr->nvalue.cp,dsize);
						nv_onattr(nq,NV_NOFREE);
					}
					else if(nr->nvalue.cp == Empty)
						nq->nvalue.cp = Empty;
					else
						nq->nvalue.cp = strdup(nr->nvalue.cp);
					nv_disc(nq, &pp->childfun.fun, NV_LAST);
				}
				nq->nvsize = nr->nvsize;
				offset += dsize;
				if(*cname!='_' || cname[1])
				{
					cp = strcopy(cp,cname);
					*cp++ = '.';
				}
				cp = strcopy(cp,nr->nvname);
				*cp++ = 0;
			}
			while((i+1) < numnodes && (cname=&nodes[i+1]->nvname[m]) && strncmp(cname,&np->nvname[m],n)==0 && cname[n]=='.')
			{
				j=kfirst;
				nv_unset(np);
				nv_delete(np,root,0);
				np = nodes[++i];
				while(j < k)
				{
					nq = nv_namptr(pp->nodes,++j);
					if(strcmp(nq->nvname,cname)==0)
					{
						sfprintf(sfstderr,"%s found at k=%d\n",nq->nvname,k);
						if(nq->nvalue.cp>=pp->data && nq->nvalue.cp< (char*)pp->names)
							memcpy((char*)nq->nvalue.cp,np->nvalue.cp,nv_datasize(np,0));
						break;
					}
				}
			}
		}
		else
		{
			Namarr_t *ap;
			j = nv_isattr(np,NV_NOFREE);
			if(j==0 && (ap=nv_arrayptr(np)) && !ap->fun)
				j = 1;
			nq->nvfun = np->nvfun;
			nq->nvshell = np->nvshell;
			np->nvfun = 0;
			nv_disc(nq, &pp->childfun.fun, NV_LAST);
			if(nq->nvfun)
			{
				for(fp=nq->nvfun; fp; fp = fp->next)
					fp->nofree |= 1;
			}
			nq->nvalue.cp = np->nvalue.cp;
			if(dsize  && (np->nvalue.cp || !nv_isarray(np)))
			{
				nq->nvalue.cp = pp->data+offset;
				sp = (char*)np->nvalue.cp;
				if(!nv_isarray(np) && nv_isattr(np,NV_INT16P) ==NV_INT16)
				{
					sp= (char*)&np->nvalue;
					nv_onattr(nq,NV_INT16P);
					j = 1;
				}
				if(sp)
					memcpy((char*)nq->nvalue.cp,sp,dsize);
				else if(nv_isattr(np,NV_LJUST|NV_RJUST))
					memset((char*)nq->nvalue.cp,' ',dsize);
				if(!j)
					free((void*)np->nvalue.cp);
			}
			if(!nq->nvalue.cp && nq->nvfun== &pp->childfun.fun)
			{
				if(nv_isattr(np,NV_ARRAY|NV_NOFREE)==(NV_ARRAY|NV_NOFREE))
					nq->nvalue.cp = Null;
				else
					nq->nvalue.cp = Empty;
			}
			np->nvalue.cp = 0;
			offset += (dsize?dsize:4);
		}
		k++;
	skip:
		if(!nv_isnull(np))
			_nv_unset(np,0);
		nv_delete(np,root,0);
	}
	pp->ndisc = nd;
	pp->nref = nref;
	if(k>1)
	{
		nv_setsize(mp,offset);
		mp->nvalue.cp = pp->data;
		nv_onattr(mp,NV_NOFREE|binary);
	}
	else if(!mp->nvalue.cp)
		mp->nvalue.cp = Empty;
	nv_onattr(mp,NV_TAGGED);
	nv_disc(mp, &pp->fun, NV_LAST);
	if(nd>0)
	{
		pp->names[nd] = 0;
		nv_adddisc(mp, (const char**)pp->names, mnodes);
	}
	if(mnodes!=nodes)
		free((void*)mnodes);
	nv_newtype(mp);
	return(mp);
}

Namval_t *nv_mkinttype(char *name, size_t size, int sign, const char *help, Namdisc_t *ep)
{
	Shell_t *shp = sh_getinterp();
	Namval_t	*mp;
	Namfun_t	*fp;
	Namdisc_t	*dp;
	int		offset=stktell(shp->stk);
	sfputr(shp->stk,NV_CLASS,'.');
	sfputr(shp->stk,name,0);
        mp = nv_open(stkptr(shp->stk,offset), shp->var_tree, NV_VARNAME);
	stkseek(shp->stk,offset);
	offset = size + sizeof(Namdisc_t);
	fp = newof(NiL, Namfun_t, 1, offset);
	fp->type = mp;
	fp->nofree |= 1;
	fp->dsize = sizeof(Namfun_t)+size;
	dp = (Namdisc_t*)(fp+1);
	if(ep)
		*dp = *ep;
	dp->clonef =  clone_inttype;
	fp->disc = dp;
	mp->nvalue.cp = (char*)(fp+1) + sizeof(Namdisc_t);
	nv_setsize(mp,10);
	mp->nvenv = (char*)help;
	nv_onattr(mp,NV_NOFREE|NV_RDONLY|NV_INTEGER|NV_EXPORT);
	if(size==16)
		nv_onattr(mp,NV_INT16P);
	else if(size==64)
		nv_onattr(mp,NV_INT64);
	if(!sign)
		nv_onattr(mp,NV_UNSIGN);
	nv_disc(mp, fp, NV_LAST);
	nv_newtype(mp);
	return(mp);
}

void nv_typename(Namval_t *tp, Sfio_t *out)
{
	char *v,*cp;
	Namtype_t	*dp;
	cp = nv_name(tp);
	if(v=strrchr(cp,'.'))
		cp = v+1;
	if((dp = (Namtype_t*)nv_hasdisc(tp,&type_disc)) && dp->bp)
	{
		nv_typename(dp->bp,out);
		sfprintf(out,"%s.%s",sfstruse(out),cp);
	}
	else
		sfputr(out,cp,-1);
}

Namval_t *nv_type(Namval_t *np)
{
	Namfun_t  *fp;
	if(nv_isattr(np,NV_BLTIN|BLT_DCL)==(NV_BLTIN|BLT_DCL))
	{
		Namdecl_t *ntp = (Namdecl_t*)nv_context(np);
		return(ntp?ntp->tp:0);
	}
	for(fp=np->nvfun; fp; fp=fp->next)
	{
		if(fp->type)
			return(fp->type);
		if(fp->disc && fp->disc->typef && (np= (*fp->disc->typef)(np,fp)))
			return(np);
	}
	return(0);
}

/*
 * call any and all create functions
 */
static void type_init(Namval_t *np)
{
	Shell_t		*shp = sh_ptr(np);
	int 		i;
	Namtype_t	*dp, *pp=(Namtype_t*)nv_hasdisc(np,&type_disc);
	Namval_t	*nq;
	if(!pp)
		return;
	for(i=0; i < pp->numnodes; i++)
	{
		nq = nv_namptr(pp->nodes,i);
		if((dp=(Namtype_t*)nv_hasdisc(nq,&type_disc)) && dp->cp)
			sh_fun(shp,dp->cp,nq, (char**)0);
	}
	if(pp->cp)
		sh_fun(shp,pp->cp, np, (char**)0);
}

/*
 * This function turns variable <np>  to the type <tp>
 */
int nv_settype(Namval_t* np, Namval_t *tp, int flags)
{
	int		isnull = nv_isnull(np);
	int		rdonly = nv_isattr(np,NV_RDONLY);
	char		*val=0;
	Namarr_t	*ap=0;
	Shell_t		*shp = sh_ptr(np);
	int		nelem=0,subshell=shp->subshell;
	Namval_t	*tq;
	if(nv_type(np)==tp)
		return(0);
	if(nv_isarray(np) && (tq=nv_type(np)))
	{
		if(tp==tq)
			return(0);
		errormsg(SH_DICT,ERROR_exit(1),e_redef,nv_name(np));
	}
	if((ap=nv_arrayptr(np)) && ap->nelem>0)
	{
		nv_putsub(np,NIL(char*),0,ARRAY_SCAN);
		ap->hdr.type = tp;
		do
		{
			Namval_t *mp = nv_opensub(np);
			if(mp && nv_isarray(mp))
				nv_settype(mp,tp,flags);
			else
			{
				nv_arraysettype(np, tp, nv_getsub(np),flags);
				if(mp)
					nv_checkrequired(mp);
			}
		}
		while(nv_nextsub(np));
	}
	else if(ap || nv_isarray(np))
	{
		flags &= ~NV_APPEND;
		if(!ap)
		{
			if(subshell)
			{
				sh_assignok(np,1);
				shp->subshell = 0;
			}
			nv_putsub(np,"0",0,ARRAY_FILL);
			ap = nv_arrayptr(np);
			nelem = 1;
			nv_arraysettype(np, tp, "0" ,flags);
		}
	}
	else
	{
		if(isnull)
			flags &= ~NV_APPEND;
		else if(!nv_isvtree(np))
		{
			val = strdup(nv_getval(np));
			if(!(flags&NV_APPEND))
				_nv_unset(np, NV_RDONLY);
		}
		if(!nv_clone(tp,np,flags|NV_NOFREE))
			return(0);
	}
	if(ap)
	{
		int nofree;
		nv_disc(np,&ap->hdr,NV_POP);
		np->nvalue.up = 0;
		nv_clone(tp,np,flags|NV_NOFREE);
		if(np->nvalue.cp && np->nvalue.cp!=Empty && !nv_isattr(np,NV_NOFREE))
			free((void*)np->nvalue.cp);
		np->nvalue.up = 0;
		nofree = ap->hdr.nofree;
		ap->hdr.nofree = 0;
		ap->hdr.type = tp;
		nv_disc(np, &ap->hdr, NV_FIRST);
		ap->hdr.nofree = nofree;
		nv_onattr(np,NV_ARRAY);
		if(nelem)
		{
			ap->nelem++;
			nv_putsub(np,"0",0,0);
			_nv_unset(np,NV_RDONLY|NV_TYPE);
			ap->nelem--;
			shp->subshell = subshell;
		}
	}
	type_init(np);
	if(!rdonly)
		nv_offattr(np,NV_RDONLY);
	if(val)
	{
		nv_putval(np,val,NV_RDONLY);
		free((void*)val);
	}
	return(0);
}

#define S(x)	#x
#define FIELD(x,y)	{ S(y##x),	S(x##_t), offsetof(struct stat,st_##y##x) }
typedef struct _field_
{
	char	*name;
	char	*type;
	int	offset;
} Fields_t;

static Fields_t foo[]=
{
	FIELD(dev,),
	FIELD(ino,),
	FIELD(nlink,),
	FIELD(mode,),
	FIELD(uid,),
	FIELD(gid,),
	FIELD(size,),
	FIELD(time,a),
	FIELD(time,m),
	FIELD(time,c),
#if 0
	FIELD(blksize,),
	FIELD(blocks,),
#endif
	0
}; 


Namval_t *nv_mkstruct(const char *name, int rsize, Fields_t *fields, void *context) 
{
	Shell_t	*shp = (Shell_t*)context;
	Namval_t	*mp, *nq, *nr, *tp;
	Fields_t	*fp;
	Namtype_t	*dp, *pp;
	char		*cp, *sp;
	int		nnodes=0, offset=stktell(shp->stk), n, r, i, j;
	size_t		m, size=0;
	sfputr(shp->stk,NV_CLASS,'.');
	r = stktell(shp->stk);
	sfputr(shp->stk,name,0);
	mp = nv_open(stkptr(shp->stk,offset), shp->var_tree, NV_VARNAME);
	stkseek(shp->stk,r);

	for(fp=fields; fp->name; fp++)
	{
		m = strlen(fp->name)+1;
		size += m;
		nnodes++;
		if(strncmp(fp->type,"typeset",7))
		{
			sfputr(shp->stk,fp->type,0);
			tp = nv_open(stkptr(shp->stk,offset), shp->var_tree, NV_VARNAME|NV_NOADD|NV_NOFAIL);
			stkseek(shp->stk,r);
			if(!tp)
				errormsg(SH_DICT,ERROR_exit(1),e_unknowntype,strlen(fp->type),fp->type);
			if(dp = (Namtype_t*)nv_hasdisc(tp,&type_disc))
			{
				nnodes += dp->numnodes;
				if((i=dp->strsize) < 0)
					i = -i;
				size += i + dp->numnodes*m;
			}
		}
	}
	pp = newof(NiL,Namtype_t, 1,  nnodes*NV_MINSZ + rsize + size);
	pp->fun.dsize = sizeof(Namtype_t)+nnodes*NV_MINSZ +rsize;
	pp->fun.type = mp;
	pp->np = mp;
	pp->childfun.fun.disc = &chtype_disc;
	pp->childfun.fun.nofree = 1;
	pp->childfun.ttype = pp;
	pp->childfun.ptype = pp;
	pp->fun.disc = &type_disc;
	pp->nodes = (char*)(pp+1);
	pp->numnodes = nnodes;
	pp->strsize = size;
	pp->data = pp->nodes + nnodes*NV_MINSZ;
	cp = pp->data + rsize;
	for(i=0,fp=fields; fp->name; fp++)
	{
		nq = nv_namptr(pp->nodes,i++);
		nq->nvname = cp;
		nq->nvalue.cp = pp->data + fp->offset;
		nv_onattr(nq,NV_MINIMAL|NV_NOFREE);
		m = strlen(fp->name)+1;
		memcpy(cp, fp->name, m);
		cp += m;
		if(strncmp(fp->type,"typeset",7))
		{
			sfputr(shp->stk,fp->type,0);
			tp = nv_open(stkptr(shp->stk,offset), shp->var_tree, NV_VARNAME);
			stkseek(shp->stk,r);
			clone_all_disc(tp,nq,NV_RDONLY);
			nq->nvflag = tp->nvflag|NV_MINIMAL|NV_NOFREE;
			nq->nvsize = tp->nvsize;
			if(dp = (Namtype_t*)nv_hasdisc(nq,&type_disc))
				dp->strsize = -dp->strsize;
			if(dp = (Namtype_t*)nv_hasdisc(tp,&type_disc))
			{
				if(nv_hasdisc(nq,&chtype_disc))
					nv_disc(nq, &pp->childfun.fun, NV_LAST);
				sp = (char*)nq->nvalue.cp;
				memcpy(sp, dp->data, nv_size(tp));
				for(j=0; j < dp->numnodes; j++)
				{
					nr = nv_namptr(dp->nodes,j);
					nq = nv_namptr(pp->nodes,i++);
					nq->nvname = cp;
					memcpy(cp,fp->name,m);
					cp[m-1] = '.';
					cp += m;
					n = strlen(nr->nvname)+1;
					memcpy(cp,nr->nvname,n);
					cp += n;
					if(nr->nvalue.cp>=dp->data && nr->nvalue.cp < (char*)pp + pp->fun.dsize)
					{
						nq->nvalue.cp = sp + (nr->nvalue.cp-dp->data);
					}
					nq->nvflag = nr->nvflag;
					nq->nvsize = nr->nvsize;
				}
			}
		}
		else if(strmatch(fp->type+7,"*-*i*")==0)
		{
			nv_onattr(nq,NV_NOFREE|NV_RDONLY|NV_INTEGER);
			if(strmatch(fp->type+7,"*-*s*")==0)
				nv_onattr(nq,NV_INT16P);
			else if(strmatch(fp->type+7,"*-*l*")==0)
				nv_onattr(nq,NV_INT64);
			if(strmatch(fp->type+7,"*-*u*")==0)
				nv_onattr(nq,NV_UNSIGN);
		}
		
	}
	stkseek(shp->stk,offset);
	nv_onattr(mp,NV_RDONLY|NV_NOFREE|NV_BINARY);
	nv_setsize(mp,rsize);
	nv_disc(mp, &pp->fun, NV_LAST);
	mp->nvalue.cp = pp->data;
	nv_newtype(mp);
	return(mp);
}

static void put_stat(Namval_t* np, const char* val, int flag, Namfun_t* nfp)
{
	if(val)
	{
		if(stat(val,(struct stat*)np->nvalue.cp)<0)
			sfprintf(sfstderr,"stat of %s failed\n",val);
		return;
	}
	nv_putv(np,val,flag,nfp);
	nv_disc(np,nfp,NV_POP);
	if(!(nfp->nofree&1))
		free((void*)nfp);
}

static const Namdisc_t stat_disc =
{
        0,
        put_stat
};


void nv_mkstat(Shell_t *shp)
{
	Namval_t *tp;
	Namfun_t *fp;
	tp = nv_mkstruct("stat_t", sizeof(struct stat), foo, (void*)shp);
	nv_offattr(tp,NV_RDONLY);
	nv_setvtree(tp);
	fp = newof(NiL,Namfun_t,1,0);
	fp->type = tp;
	fp->disc = &stat_disc;
	nv_disc(tp,fp,NV_FIRST);
	nv_putval(tp,e_devnull,0);
	nv_onattr(tp,NV_RDONLY);
}

static void write_indent(Sfio_t *out,char *str,int n,int indent)
{
	register int	c, first=1;
	register char	*cp = str;
	while(n-- && (c = *str++))
	{
		if(c=='\n')
		{
			if(!first)
				sfnputc(out,'\t',indent);
			first = 0;
			sfwrite(out,cp,str-cp);
			cp = str;
		}
	}
	if(cp > str)
	{
		sfnputc(out,'\t',indent);
		sfwrite(out,cp,str-cp);
	}
}

int	sh_outtype(Shell_t *shp,Sfio_t *out)
{
	Namval_t	node,*mp,*tp;
	Dt_t		*dp;
	char		*cp,*sp,*xp,nvtype[sizeof(NV_CLASS)];
	Sfio_t		*iop=0;
	int		n=0,indent = 0;
	if(cp=shp->prefix)
	{
		indent=1;
		while(*cp)
		{
			if(*cp++ =='.')
				indent++;
		}
		n = cp-shp->prefix+1;
	}
	strcpy(nvtype,NV_CLASS);
	if(!(mp = nv_open(nvtype, shp->var_base,NV_NOADD|NV_VARNAME)))
		return(0);
	memcpy(&node,L_ARGNOD,sizeof(node));
	L_ARGNOD->nvfun = 0;
	L_ARGNOD->nvalue.cp = 0;
	dp  = 	nv_dict(mp);
	if(indent==0)
	for(tp = (Namval_t*)dtfirst(dp); tp; tp = (Namval_t*)dtnext(dp,tp))
	{
		/* skip over enums */
		if(tp->nvfun && !nv_isvtree(tp))
			continue;
		if(!nv_search(tp->nvname,shp->bltin_tree,0))
			continue;
		sfprintf(out,"typeset -T %s\n",tp->nvname);
	}
	for(tp = (Namval_t*)dtfirst(dp); tp; tp = (Namval_t*)dtnext(dp,tp))
	{
		if(nv_isnull(tp) || !nv_isvtree(tp))
			continue;
		if(indent && (strncmp(tp->nvname,shp->prefix,n-1) || tp->nvname[n-1]!='.' || strchr(tp->nvname+n,'.')))
			continue;
		nv_settype(L_ARGNOD,tp,0);
		if(indent)
			sfnputc(out,'\t',indent);
		sfprintf(out,"typeset -T %s=",tp->nvname+n);
		shp->last_table = 0;
		cp = nv_getval(L_ARGNOD);
		if(indent)
			write_indent(out,cp,strlen(cp)-1,indent);
		else
			sfprintf(out,"%.*s",strlen(cp)-1,cp);
		_nv_unset(L_ARGNOD,NV_RDONLY);
		for(sp=0; sp=nv_setdisc(tp,(char*)0,(Namval_t*)sp,(Namfun_t*)tp);)
		{
			mp = (Namval_t*)nv_setdisc(tp,sp,tp,(Namfun_t*)tp);
			if(!mp || mp==tp)
				continue;
			if(cp=strrchr(mp->nvname,'.'))
				cp++;
			else
				cp = mp->nvname;
			if(indent)
				sfnputc(out,'\t',indent);
			if(nv_isattr(mp,NV_FPOSIX))
				sfprintf(out,"\t%s()",cp);
			else
				sfprintf(out,"\tfunction %s",cp);
			xp = 0;
			if(mp->nvalue.ip && mp->nvalue.rp->hoffset>=0)
			{
				if(nv_isattr(mp,NV_FTMP))
					iop = shp->heredocs;
				else if(xp=mp->nvalue.rp->fname)
					iop = sfopen(iop,xp,"r");
				else if(shp->gd->hist_ptr)
					iop = (shp->gd->hist_ptr)->histfp;
				if(iop && sfseek(iop,(Sfoff_t)mp->nvalue.rp->hoffset,SEEK_SET)>=0)
					sfmove(iop,out, nv_size(mp), -1);
				else
					sfputc(iop,'\n');
				if(xp)
					sfclose(iop);
				if(nv_isattr(mp,NV_STATICF|NV_TAGGED))
				{
					if(indent)
						sfnputc(out,'\t',indent);
					sfwrite(out,"\ttypeset -f",11);
					if(nv_isattr(mp,NV_STATICF))
						sfputc(out,'S');
					if(nv_isattr(mp,NV_TAGGED))
						sfputc(out,'t');
					if(mp->nvalue.rp->help)
						sfprintf(out,"h '%s'",mp->nvalue.rp->help);
					sfprintf(out," %s\n",cp);
				}
				iop = 0;
			}
		}
		if(indent)
			sfnputc(out,'\t',indent);
		sfwrite(out,")\n",2);
	}
	dtdelete(shp->var_base,L_ARGNOD);
	memcpy(L_ARGNOD,&node,sizeof(node));
	dtinsert(shp->var_base,L_ARGNOD);
	return(0);
}

Namval_t *nv_typeparent(Namval_t *np)
{
	Namchld_t *fp;
	Namtype_t *tp;
	if(fp = (Namchld_t*)nv_hasdisc(np,&chtype_disc))
		return(fp->ptype->parent);
	if(tp = (Namtype_t*)nv_hasdisc(np,&type_disc))
		return(tp->parent);
	return(0);
}

void nv_checkrequired(Namval_t *mp)
{
	Namtype_t	*dp, *dq=0;
	Namval_t	*np, *mq, *nq;
	int		i;
	if(nv_arrayptr(mp) || !(dp=(Namtype_t*)nv_hasdisc(mp,&type_disc)))
		return;
	if(mq = nv_type(mp))
		dq=(Namtype_t*)nv_hasdisc(mq,&type_disc);
	for(i=0; i < dp->numnodes; i++) 
	{
		if(dq)
			nq = nv_namptr(dq->nodes,i);
		np = nv_namptr(dp->nodes,i);
		if(nv_isattr(np,NV_RDONLY) && np->nvalue.cp==Empty)
			errormsg(SH_DICT,ERROR_exit(1),e_required,np->nvname,nv_type(mp)->nvname);
		if(nv_isattr(nq,NV_RDONLY))
			nv_onattr(np,NV_RDONLY);
	}
}
