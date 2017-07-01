/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
/*
 * Shell arithmetic - uses streval library
 *   David Korn
 *   AT&T Labs
 */

#include	"defs.h"
#include	"lexstates.h"
#include	"name.h"
#include	"streval.h"
#include	"variables.h"
#include	"builtins.h"

#undef SHOPT_FIXEDARRAY
#ifndef LLONG_MAX
#define LLONG_MAX	LONG_MAX
#endif

typedef Sfdouble_t (*Math_f)(Sfdouble_t, ...);

extern const Namdisc_t	ENUM_disc;
static bool		Varsubscript;
static Sfdouble_t	NaN, Inf, Fun;
static Namval_t Infnod =
{
	{ 0 },
	"Inf",
};

static Namval_t NaNnod =
{
	{ 0 },
	"NaN",
};

static Namval_t FunNode =
{
	{ 0 },
	"?",
};

struct Mathconst
{
	char		name[9];
	Sfdouble_t  	value;
};

#ifndef M_1_PIl
#   define M_1_PIl	0.3183098861837906715377675267450287L
#endif
#ifndef M_2_PIl
#   define M_2_PIl	0.6366197723675813430755350534900574L
#endif
#ifndef M_2_SQRTPIl
#   define M_2_SQRTPIl	1.1283791670955125738961589031215452L
#endif
#ifndef M_El
#   define M_El		2.7182818284590452353602874713526625L
#endif
#ifndef M_LOG2El
#   define M_LOG2El	1.4426950408889634073599246810018921L
#endif
#ifndef M_LOG10El
#   define M_LOG10El	0.4342944819032518276511289189166051L
#endif
#ifndef M_LN2l
#   define M_LN2l	0.6931471805599453094172321214581766L
#endif
#ifndef M_LN10l
#   define M_LN10l	2.3025850929940456840179914546843642L
#endif
#ifndef M_PIl
#   define M_PIl	3.1415926535897932384626433832795029L
#endif
#ifndef M_PI_2l
#   define M_PI_2l	1.5707963267948966192313216916397514L
#endif
#ifndef M_PI_4l
#   define M_PI_4l	0.7853981633974483096156608458198757L
#endif
#ifndef M_SQRT2l
#   define M_SQRT2l	1.4142135623730950488016887242096981L	
#endif
#ifndef M_SQRT1_2l
#   define M_SQRT1_2l	0.7071067811865475244008443621048490L
#endif

/* The firs three entries cann't be moved or it will break the code */
static const struct Mathconst Mtable[]=
{
	"1_PI",		M_1_PIl,
	"2_PI",		M_2_PIl,
	"2_SQRTPI",	M_2_SQRTPIl,
	"E",		M_El,
	"LOG2E",	M_LOG2El,
	"LOG10E",	M_LOG10El,
	"LN2",		M_LN2l,
	"PI",		M_PIl,
	"PI_2",		M_PI_2l,
	"PI_4",		M_PI_4l,
	"SQRT2",	M_SQRT2l,
	"SQRT1_2",	M_SQRT1_2l,
	0,		0
};

typedef struct Intconst_s
{
	const char	name[4];
	short		ss;
	unsigned short	us;
	int		si;
	unsigned int	ui;
	Sflong_t	sl;
	Sfulong_t	ul;
} Intconst_t;

typedef struct Fltconst_s
{
	const char	name[12];
	float		f;
	double		d;
	Sfdouble_t	l;
} Fltconst_t;

static const Intconst_t	intconst[] =
{
	{
		"DIG",
		USHRT_DIG,	USHRT_DIG,
		UINT_DIG,	UINT_DIG,
#ifdef LLONG_DIG
		ULLONG_DIG,	ULLONG_DIG,
#else
		ULONG_DIG,	ULONG_DIG,
#endif
	},
	{
		"MAX",
		SHRT_MAX,	USHRT_MAX,
		INT_MAX,	UINT_MAX,
#ifdef LLONG_MAX
		LLONG_MAX,	ULLONG_MAX,
#else
		LONG_MAX,	ULONG_MAX,
#endif
	},
	{
		"MIN",
		SHRT_MIN,	0,
		INT_MIN,	0,
#ifdef LLONG_MIN
		LLONG_MIN,	0,
#else
		LONG_MIN,	0,
#endif
	},
};

static const Fltconst_t	fltconst[] =
{
	{
		"DIG",
		FLT_DIG,
		DBL_DIG, 
#ifdef LDBL_DIG
		LDBL_DIG,
#else
		DBL_DIG,
#endif
	},
	{
		"EPSILON",
		FLT_EPSILON,
		DBL_EPSILON, 
#ifdef LDBL_EPSILON
		LDBL_EPSILON,
#else
		DBL_EPSILON,
#endif
	},
	{
		"INT_MAX",
		FLT_INTMAX_MAX,
		DBL_INTMAX_MAX, 
#ifdef LDBL_INTMAX_MAX
		LDBL_INTMAX_MAX,
#else
		DBL_INTMAX_MAX,
#endif
	},
	{
		"INT_MIN",
		FLT_INTMAX_MIN,
		DBL_INTMAX_MIN, 
#ifdef LDBL_INTMAX_MIN
		LDBL_INTMAX_MIN,
#else
		DBL_INTMAX_MIN,
#endif
	},
	{
		"MAX",
		FLT_MAX,
		DBL_MAX, 
#ifdef LDBL_MAX
		LDBL_MAX,
#else
		DBL_MAX,
#endif
	},
	{
		"MAX_10_EXP",
		FLT_MAX_10_EXP,
		DBL_MAX_10_EXP, 
#ifdef LDBL_MAX_10_EXP
		LDBL_MAX_10_EXP,
#else
		DBL_MAX_10_EXP,
#endif
	},
	{
		"MAX_EXP",
		FLT_MAX_EXP,
		DBL_MAX_EXP, 
#ifdef LDBL_MAX_EXP
		LDBL_MAX_EXP,
#else
		DBL_MAX_EXP,
#endif
	},
	{
		"MIN",
		FLT_MIN,
		DBL_MIN, 
#ifdef LDBL_MIN
		LDBL_MIN,
#else
		DBL_MIN,
#endif
	},
	{
		"MIN_10_EXP",
		FLT_MIN_10_EXP,
		DBL_MIN_10_EXP, 
#ifdef LDBL_MIN_10_EXP
		LDBL_MIN_10_EXP,
#else
		DBL_MIN_10_EXP,
#endif
	},
	{
		"MIN_EXP",
		FLT_MIN_EXP,
		DBL_MIN_EXP, 
#ifdef LDBL_MIN_EXP
		LDBL_MIN_EXP,
#else
		DBL_MIN_EXP,
#endif
	},
	{
		"UINT_MAX",
		FLT_UINTMAX_MAX,
		DBL_UINTMAX_MAX, 
#ifdef LDBL_UINTMAX_MAX
		LDBL_UINTMAX_MAX,
#else
		DBL_UINTMAX_MAX,
#endif
	},
	{
		"UINT_MIN",
		0,
		0, 
		0,
	},
};

static Namval_t	*check_limits(Shell_t *shp, char *cp)
{
	static Namval_t	node;
	static Sfdouble_t dd;
	Namval_t	*np;
	char		*ep;
	int 		n;
	if(!(ep=strrchr(cp,'.')))
		return(0);
	*ep = 0;
	np = nv_open(cp, shp->var_tree, NV_VARNAME|NV_NOADD|NV_NOFAIL);
	*ep++ = '.';
	if(!np || !nv_isattr(np,NV_INTEGER))
		return(0);
	if(nv_isattr (np,NV_DOUBLE)==NV_DOUBLE)
	{
		const Fltconst_t *fp = fltconst;
		n = sizeof(fltconst)/sizeof(Fltconst_t);
		for(; n-->0; fp++)
		{
			if(strcmp(fp->name,ep)==0)
				break;
		}
		if(n>=0)
		{
			node = *np;
			node.nvalue.ldp = &dd;
			if(nv_isattr(np,NV_SHORT))
				*node.nvalue.fp = fp->f; 
			else if(nv_isattr(np,NV_LONG))
				*node.nvalue.ldp = fp->l; 
			else
				*node.nvalue.dp = fp->d; 
			return(&node);
		}
	}
	else
	{
		const Intconst_t *ip = intconst;
		n = sizeof(intconst)/sizeof(Intconst_t);
		for(; n-->=0; ip++)
		{
			if(strcmp(ip->name,ep)==0)
				break;
		}
		if(n>=0)
		{
			int unsign = nv_isattr(np,NV_LTOU);
			node = *np;
			node.nvalue.ldp = &dd;
			if(nv_isattr(np,NV_SHORT))
			{
				if(unsign)
					node.nvalue.u = ip->us; 
				else
					node.nvalue.s = ip->ss; 
			}
			else if(nv_isattr(np,NV_LONG))
			{
				if(unsign)
					*node.nvalue.llp = ip->ul; 
				else
					*node.nvalue.llp = ip->sl; 
			}
			else
			{
				if(unsign)
					*node.nvalue.ip = ip->ui; 
				else
					*node.nvalue.ip = ip->ul; 
			}
			return(&node);
		}
	}
	return(0);
}

static Namval_t *scope(register Namval_t *np,register struct lval *lvalue,int assign)
{
	register int flag = lvalue->flag;
	register char *sub=0, *cp=(char*)np;
	register Namval_t *mp;
	Shell_t		*shp = lvalue->shp;
	int	flags = HASH_NOSCOPE|HASH_SCOPE|HASH_BUCKET;
	int	c=0,nosub = lvalue->nosub;
	Dt_t	*sdict = (shp->st.real_fun? shp->st.real_fun->sdict:0);
	Dt_t	*nsdict = (shp->namespace?nv_dict(shp->namespace):0);
	Dt_t	*root = shp->var_tree;
	assign = assign?NV_ASSIGN:NV_NOASSIGN;
	lvalue->nosub = 0;
	if(nosub<0 && lvalue->ovalue)
		return((Namval_t*)lvalue->ovalue);
	lvalue->ovalue = 0;
	if(cp>=lvalue->expr &&  cp < lvalue->expr+lvalue->elen)
	{
		int offset;
		/* do binding to node now */
		int d = cp[flag];
		cp[flag] = 0;
		if((!(np = nv_open(cp,root,assign|NV_VARNAME|NV_NOADD|NV_NOFAIL)) || nv_isnull(np)) && sh_macfun(shp,cp, offset = stktell(shp->stk)))
		{
			Fun = sh_arith(shp,sub=stkptr(shp->stk,offset));
			FunNode.nvalue.ldp = &Fun;
			FunNode.nvshell = shp;
			nv_onattr(&FunNode,NV_NOFREE|NV_LDOUBLE|NV_RDONLY);
			cp[flag] = d;
			return(&FunNode);
		}
		if(!np)
		{
			if(assign)
				np = nv_open(cp,root,assign|NV_VARNAME);
			else
				np = check_limits(shp,cp);
		}
		cp[flag] = d;
		if(!np)
			return(0);
		root = shp->last_root;
		if(cp[flag+1]=='[')
			flag++;
		else
			flag = 0;
		cp = (char*)np;
	}
	else if(assign==NV_ASSIGN  && nv_isnull(np) && !nv_isattr(np, ~(NV_MINIMAL|NV_NOFREE)))
		flags |= NV_ADD;
	if((lvalue->emode&ARITH_COMP) && dtvnext(root) && ((sdict && (mp=nv_search(cp,sdict,flags&~NV_ADD))) || (mp=nv_search(cp,root,flags&~(NV_ADD))) || (nsdict && (mp=nv_search(cp,nsdict,flags&~(NV_ADD|HASH_NOSCOPE)))) ))
		np = mp;
	while(nv_isref(np))
	{
#if SHOPT_FIXEDARRAY
		int n,dim;
		dim = nv_refdimen(np);
		n = nv_refindex(np);
#endif /* SHOPT_FIXEDARRAY */
		sub = nv_refsub(np);
		np = nv_refnode(np);
#if SHOPT_FIXEDARRAY
		if(n)
		{
			Namarr_t *ap = nv_arrayptr(np);
			ap->nelem = dim;
			nv_putsub(np,(char*),n,0);
		}
		else
#endif /* SHOPT_FIXEDARRAY */
		if(sub)
			nv_putsub(np,sub,0,assign==NV_ASSIGN?ARRAY_ADD:0);
	}
	if(!nosub && flag)
	{
		int		hasdot = 0;
		cp = (char*)&lvalue->expr[flag];
		if(sub)
		{
			goto skip;
		}
		sub = cp;
		while(1)
		{
			Namarr_t	*ap;
			Namval_t	*nq;
			cp = nv_endsubscript(np,cp,0,(void*)shp);
			if(c || *cp=='.')
			{
				c = '.';
				while(*cp=='.')
				{
					hasdot=1;
					cp++;
					while(c=mbchar(cp),isaname(c));
				}
				if(c=='[')
					continue;
			}
			flag = *cp;
			*cp = 0;
			if(c || hasdot)
			{
				sfprintf(shp->strbuf,"%s%s%c",nv_name(np),sub,0);
				sub = sfstruse(shp->strbuf);
			}
			if(strchr(sub,'$'))
				sub = sh_mactrim(shp,sub,0);
			*cp = flag;
			if(c || hasdot)
			{
				np = nv_open(sub,shp->var_tree,NV_VARNAME|assign);
				return(np);
			}
#if SHOPT_FIXEDARRAY
			ap = nv_arrayptr(np);
			cp = nv_endsubscript(np,sub,(assign==NV_ASSIGN?NV_ADD:0)|NV_SUBQUOTE|(ap&&ap->fixed?NV_FARRAY:0),np->nvshell);
#else
			cp = nv_endsubscript(np,sub,(assign==NV_ASSIGN?NV_ADD:0)|NV_SUBQUOTE,np->nvshell);
#endif /* SHOPT_FIXEDARRAY */
			if(*cp!='[')
				break;
		skip:
			if(nq = nv_opensub(np))
				np = nq;
			else
			{
				ap = nv_arrayptr(np);
				if(ap && !ap->table)
				{
					ap->table = dtopen(&_Nvdisc,Dtoset);
					dtuserdata(ap->table,shp,1);
				}
				if(ap && ap->table && (nq=nv_search(nv_getsub(np),ap->table,NV_ADD)))
					nq->nvenv = (char*)np;
				if(nq && nv_isnull(nq))
					np = nv_arraychild(np,nq,0);
			}
			sub = cp;
		}
	}
	else if(nosub>0)
		nv_putsub(np,(char*)0,nosub-1,0);
	return(np);
}

Math_f sh_mathstdfun(const char *fname, size_t fsize, short * nargs)
{
	register const struct mathtab *tp;
	register char c = fname[0];
	for(tp=shtab_math; *tp->fname; tp++)
	{
		if(*tp->fname > c)
			break;
		if(tp->fname[1]==c && tp->fname[fsize+1]==0 && strncmp(&tp->fname[1],fname,fsize)==0)
		{
			if(nargs)
				*nargs = *tp->fname;
			return(tp->fnptr);
		}
	}
	return(0);
}

int	sh_mathstd(const char *name)
{
	return(sh_mathstdfun(name,strlen(name),NULL)!=0);
}

static Sfdouble_t number(const char* s, char** p, int b, struct lval* lvalue)
{
	Sfdouble_t	r;
	char*		t;
	int		oerrno;
	int		c;
	char		base;
	struct lval	v;

	oerrno = errno;
	errno = 0;
	base = b;
	if (!lvalue)
		lvalue = &v;
	else if (lvalue->shp->bltindata.bnode==SYSLET && !sh_isoption(lvalue->shp, SH_LETOCTAL))
		while (*s=='0' && isdigit(s[1]))
			s++;
	lvalue->eflag = 0;
	lvalue->isfloat = 0;
	r = strtonll(s, &t, &base, -1);
	if (*t=='8' || *t=='9')
	{
		base = 10;
		errno = 0;
		r = strtonll(s, &t, &base, -1);
	}
	if (base <= 1)
		base = 10;
	if (*t=='_')
	{
		if ((r==1||r==2) && strcmp(t,"_PI")==0)
		{
			t += 3;
			r = Mtable[(int)r-1].value;
		}
		else if (r==2 && strcmp(t,"_SQRTPI")==0)
		{
			t += 7;
			r = Mtable[2].value;
		}
	}
	c = r==LLONG_MAX && errno ? 'e' : *t;
	if (c==GETDECIMAL(0) || c=='e' || c == 'E' || base == 16 && (c == 'p' || c == 'P'))
	{
		r = strtold(s, &t);
		lvalue->isfloat = TYPE_LD;
	}
	if (t > s)
	{
		if (*t=='f' || *t=='F')
		{
			t++;
			lvalue->isfloat = TYPE_F;
			r = (float)r;
		}
		else if (*t=='l' || *t=='L')
		{
			t++;
			lvalue->isfloat= TYPE_LD;
		}
		else if (*t=='d' || *t=='D')
		{
			t++;
			lvalue->isfloat= TYPE_LD;
			r = (double)r;
		}
	}
	errno = oerrno;
	*p = t;
	return r;
}

static Sfdouble_t arith(const char **ptr, struct lval *lvalue, int type, Sfdouble_t n)
{
	Shell_t		*shp = lvalue->shp;
	register Sfdouble_t r= 0;
	char *str = (char*)*ptr;
	register char *cp;
	switch(type)
	{
	    case ASSIGN:
	    {
		register Namval_t *np = (Namval_t*)(lvalue->value);
		np = scope(np,lvalue,1);
		nv_putval(np, (char*)&n, NV_LDOUBLE);
		if(lvalue->eflag)
			lvalue->ptr = (void*)nv_hasdisc(np,&ENUM_disc);
		lvalue->eflag = 0;
		r=nv_getnum(np);
		lvalue->value = (char*)np;
		break;
	    }
	    case LOOKUP:
	    {
		register int c = *str;
		register char *xp=str;
		lvalue->value = (char*)0;
		if(c=='.')
			str++;
		c = mbchar(str);
		if(isaletter(c))
		{
			register Namval_t *np=0;
			int dot=0;
			while(1)
			{
				while(xp=str, c=mbchar(str), isaname(c));
				str = xp;
				while(c=='[' && dot==NV_NOADD)
				{
					str = nv_endsubscript((Namval_t*)0,str,0,shp);
					c = *str;
				}
				if(c!='.')
					break;
				dot=NV_NOADD;
				if((c = *++str) !='[')
					continue;
				str = nv_endsubscript((Namval_t*)0,cp=str,NV_SUBQUOTE,shp)-1;
				if(sh_checkid(cp+1,(char*)0))
					str -=2;
			}
			if(c=='(')
			{
				int off=stktell(shp->stk);
				int fsize = str- (char*)(*ptr);
				const struct mathtab *tp;
				Namval_t	*nq;
				c = **ptr;
				lvalue->fun = 0;
				sfprintf(shp->stk,".sh.math.%.*s%c",fsize,*ptr,0);
				stkseek(shp->stk,off);
				if(nq=nv_search(stkptr(shp->stk,off),shp->fun_tree,0))
				{
						lvalue->nargs = -nq->nvalue.rp->argc;
						lvalue->fun = (Math_f)nq;
						break;
				}
				if(fsize<=(sizeof(tp->fname)-2))
					lvalue->fun = (Math_f)sh_mathstdfun(*ptr,fsize,&lvalue->nargs);
				if(lvalue->fun)
					break;
				if(lvalue->emode&ARITH_COMP)
					lvalue->value = (char*)e_function;
				else
					lvalue->value = (char*)ERROR_dictionary(e_function);
				return(r);
			}
			if((lvalue->emode&ARITH_COMP) && dot)
			{
				lvalue->value = (char*)*ptr;
				lvalue->flag =  str-lvalue->value;
				break;
			}
			*str = 0;
			if(sh_isoption(shp,SH_NOEXEC))
				np = L_ARGNOD;
			else
			{
				int offset = stktell(shp->stk);
				char *saveptr = stkfreeze(shp->stk,0);
				Dt_t  *root = (lvalue->emode&ARITH_COMP)?shp->var_base:shp->var_tree;
				*str = c;
				cp = str;
				while(c=='[' || c=='.')
				{
					if(c=='[')
					{
						str = nv_endsubscript(np,str,0,(void*)shp);
						if((c= *str)!='[' &&  c!='.')
						{
							str = cp;
							c = '[';
							break;
						}
					}
					else
					{
						dot = NV_NOADD|NV_NOFAIL;
						str++;
						while(xp=str, c=mbchar(str), isaname(c));
						str = xp;
					}
				}
				*str = 0;
				cp = (char*)*ptr;
				Varsubscript = false;
				if ((cp[0] == 'i' || cp[0] == 'I') && (cp[1] == 'n' || cp[1] == 'N') && (cp[2] == 'f' || cp[2] == 'F') && cp[3] == 0)
				{
					Inf = strtold("Inf", NiL);
					Infnod.nvalue.ldp = &Inf;
					np = &Infnod;
					np->nvshell = shp;
					nv_onattr(np,NV_NOFREE|NV_LDOUBLE|NV_RDONLY);
				}
				else if ((cp[0] == 'n' || cp[0] == 'N') && (cp[1] == 'a' || cp[1] == 'A') && (cp[2] == 'n' || cp[2] == 'N') && cp[3] == 0)
				{
					NaN = strtold("NaN", NiL);
					NaNnod.nvalue.ldp = &NaN;
					np = &NaNnod;
					np->nvshell = shp;
					nv_onattr(np,NV_NOFREE|NV_LDOUBLE|NV_RDONLY);
				}
				else
				{
					const struct Mathconst *mp=0;
					np = 0;
#if 1
					if(strchr("ELPS12",**ptr))
					{
						for(mp=Mtable; *mp->name; mp++)
						{
							if(strcmp(mp->name,*ptr)==0)
								break;
						}
					}
					if(mp && *mp->name)
					{
						r = mp->value;
						lvalue->isfloat = TYPE_LD;
						goto skip2;
					}
#endif
					if(shp->namref_root && !(lvalue->emode&ARITH_COMP))
						np = nv_open(*ptr,shp->namref_root,NV_NOREF|NV_NOASSIGN|NV_VARNAME|NV_NOSCOPE|NV_NOADD|dot);
					if(!np)
						np = nv_open(*ptr,root,NV_NOREF|NV_NOASSIGN|NV_VARNAME|dot);
					if(!np || Varsubscript)
					{
						np = 0;
						lvalue->value = (char*)*ptr;
						lvalue->flag =  str-lvalue->value;
					}
				}
			skip2:
				if(saveptr != stkptr(shp->stk,0))
					stkset(shp->stk,saveptr,offset);
				else
					stkseek(shp->stk,offset);
			}
			*str = c;
#if 1
			if(lvalue->isfloat==TYPE_LD)
				break;
#endif
			if(!np && lvalue->value)
				break;
			lvalue->value = (char*)np;
			/* bind subscript later */
			if(nv_isattr(np,NV_DOUBLE)==NV_DOUBLE)
				lvalue->isfloat=1;
			lvalue->flag = 0;
			if(c=='[')
			{
				lvalue->flag = (str-lvalue->expr);
				do
				{
					while(c=='.')
					{
						str++;
						while(xp=str, c=mbchar(str), isaname(c));
						c = *(str = xp);
					}
					if(c=='[')
						str = nv_endsubscript(np,str,0,np->nvshell);
				}
				while((c= *str)=='[' || c=='.');
				break;
			}
		}
		else
			r = number(xp, &str, 0, lvalue);
		break;
	    }
	    case VALUE:
	    {
		register Namval_t *np = (Namval_t*)(lvalue->value);
		Namarr_t *ap;
		if(sh_isoption(shp,SH_NOEXEC))
			return(0);
		np = scope(np,lvalue,0);
		if(!np)
		{
			if(sh_isoption(shp,SH_NOUNSET))
			{
				*ptr = lvalue->value;
				goto skip;
			}
			return(0);
		}
		lvalue->ovalue = (char*)np;
		if(lvalue->eflag)
			lvalue->ptr = (void*)nv_hasdisc(np,&ENUM_disc);
		else if((Namfun_t*)lvalue->ptr && !nv_hasdisc(np,&ENUM_disc) && !nv_isattr(np,NV_INTEGER))
		{
			Namval_t *mp,node;
			mp = ((Namfun_t*)lvalue->ptr)->type;
			memset(&node,0,sizeof(node));
			nv_clone(mp,&node,0);
			nv_offattr(&node,NV_RDONLY|NV_NOFREE);
			nv_putval(&node,np->nvname,0);
			if(nv_isattr(&node,NV_NOFREE))
				return(r=nv_getnum(&node));
		}
		lvalue->eflag = 0;
		if(((lvalue->emode&2) || lvalue->level>1 || (lvalue->nextop!=A_STORE && sh_isoption(shp,SH_NOUNSET))) && nv_isnull(np) && !nv_isattr(np,NV_INTEGER))
		{
			*ptr = nv_name(np);
		skip:
			lvalue->value = (char*)ERROR_dictionary(e_notset);
			lvalue->emode |= 010;
			return(0);
		}
		if(lvalue->userfn && (ap=nv_arrayptr(np)) && (ap->flags&ARRAY_UNDEF))
		{
			r = (Sfdouble_t)integralof(np);
			lvalue->isfloat=5;
			return(r);
		}
		r = nv_getnum(np);
		if(nv_isattr(np,NV_INTEGER|NV_BINARY)==(NV_INTEGER|NV_BINARY))
			lvalue->isfloat= (r!=(Sflong_t)r)?TYPE_LD:0;
		else if(nv_isattr(np,(NV_DOUBLE|NV_SHORT))==(NV_DOUBLE|NV_SHORT))
		{
			lvalue->isfloat = TYPE_F;
			r = (float)r;
		}
		else if(nv_isattr(np,(NV_DOUBLE|NV_LONG))==(NV_DOUBLE|NV_LONG))
			lvalue->isfloat = TYPE_LD;
		else if(nv_isattr(np,NV_DOUBLE)==NV_DOUBLE)
		{
			lvalue->isfloat = TYPE_D;
			r = (double)r;
		}
		if((lvalue->emode&ARITH_ASSIGNOP) && nv_isarray(np))
			lvalue->nosub = nv_aindex(np)+1;
		return(r);
	    }

	    case MESSAGE:
		sfsync(NIL(Sfio_t*));
#if 0
		if(warn)
			errormsg(SH_DICT,ERROR_warn(0),lvalue->value,*ptr);
		else
#endif
		if(lvalue->emode&ARITH_COMP)
			return(-1);
			
		errormsg(SH_DICT,ERROR_exit((lvalue->emode&3)!=0),lvalue->value,*ptr);
	}
	*ptr = str;
	return(r);
}

Sfdouble_t sh_arith(Shell_t *shp,register const char *str)
{
	return(sh_strnum(shp, str, (char**)0, 1));
}

void	*sh_arithcomp(Shell_t *shp,register char *str)
{
	const char *ptr = str;
	Arith_t *ep;
	ep = arith_compile(shp,str,(char**)&ptr,arith,ARITH_COMP|1);
	if(*ptr)
		errormsg(SH_DICT,ERROR_exit(1),e_lexbadchar,*ptr,str);
	return((void*)ep);
}

/*
 * convert number defined by string to a Sfdouble_t
 * ptr is set to the last character processed
 * if mode>0, an error will be fatal with value <mode>
 */
Sfdouble_t sh_strnum_20120720(Shell_t *shp,register const char *str, char** ptr, int mode)
{
	register Sfdouble_t d;
	char *last;
	if(*str==0)
	{
		if(ptr)
			*ptr = (char*)str;
		return(0);
	}
	errno = 0;
	d = number(str,&last,shp->inarith?0:10,NiL);
	if(*last)
	{
		if(*last!='.' || last[1]!='.')
		{
			d = strval(shp,str,&last,arith,mode);
			Varsubscript = true;
		}
		if(!ptr && *last && mode>0)
			errormsg(SH_DICT,ERROR_exit(1),e_lexbadchar,*last,str);
	}
	else if (!d && *str=='-')
		d = -0.0;
	if(ptr)
		*ptr = last;
	return(d);
}

#undef sh_strnum
Sfdouble_t sh_strnum(register const char *str, char** ptr, int mode)
{
	return(sh_strnum_20120720(sh_getinterp(),str,ptr,mode));
}

