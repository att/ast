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
 * Shell macro expander
 * expands ~
 * expands ${...}
 * expands $(...)
 * expands $((...))
 * expands `...`
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#include	"defs.h"
#include	<fcin.h>
#include	<pwd.h>
#include	<ctype.h>
#include	<regex.h>
#include	"name.h"
#include	"variables.h"
#include	"shlex.h"
#include	"io.h"
#include	"jobs.h"
#include	"shnodes.h"
#include	"path.h"
#include	"national.h"
#include	"streval.h"

#if SHOPT_MULTIBYTE
#   undef isascii
#   define isacii(c)	((c)<=UCHAR_MAX)
#   include	<lc.h>
#else
#   define mbchar(p)       (*(unsigned char*)p++)
#endif /* SHOPT_MULTIBYTE */

#if _WINIX
    static int Skip;
#endif /*_WINIX */

static int	_c_;
typedef struct  _mac_
{
	Shell_t		*shp;		/* pointer to shell interpreter */
	Sfio_t		*sp;		/* stream pointer for here-document */
	struct argnod	**arghead;	/* address of head of argument list */
	char		*ifsp;		/* pointer to IFS value */
	int		fields;		/* number of fields */
	short		quoted;		/* set when word has quotes */
	unsigned char	ifs;		/* first char of IFS */
	char		atmode;		/* when processing $@ */
	char		quote;		/* set within double quoted contexts */
	char		lit;		/* set within single quotes */
	char		split;		/* set when word splittin is possible */
	char		pattern;	/* set when file expansion follows */
	char		patfound;	/* set if pattern character found */
	char		assign;		/* set for assignments */
	char		arith;		/* set for ((...)) */
	char		let;		/* set when expanding let arguments */
	char		zeros;		/* strip leading zeros when set */
	char		arrayok;	/* $x[] ok for arrays */
	char		subcopy;	/* set when copying subscript */
	char		macsub;		/* set to 1 when running mac_substitute */
	char		maccase;	/* set to 1 when expanding case pattern */
	int		dotdot;		/* set for .. in subscript */
	void		*nvwalk;	/* for name space walking*/
} Mac_t;

#undef ESCAPE
#define ESCAPE		'\\'
#define isescchar(s)	((s)>S_QUOTE)
#define isqescchar(s)	((s)>=S_QUOTE)
#define isbracechar(c)	((c)==RBRACE || (_c_=sh_lexstates[ST_BRACE][c])==S_MOD1 ||_c_==S_MOD2)
#define ltos(x)		fmtbase((long)(x),0,0)

/* type of macro expansions */
#define M_BRACE		1	/* ${var}	*/
#define M_TREE		2	/* ${var.}	*/
#define M_SIZE		3	/* ${#var}	*/
#define M_VNAME		4	/* ${!var}	*/
#define M_SUBNAME	5	/* ${!var[sub]}	*/
#define M_NAMESCAN	6	/* ${!var*}	*/
#define M_NAMECOUNT	7	/* ${#var*}	*/
#define M_TYPE		8	/* ${@var}	*/
#define M_EVAL		9	/* ${$var}	*/

static int	substring(const char*, size_t, const char*, int[], int);
static void	copyto(Mac_t*, int, int);
static void	comsubst(Mac_t*, Shnode_t*, int);
static bool	varsub(Mac_t*);
static void	mac_copy(Mac_t*,const char*, size_t);
static void	tilde_expand2(Shell_t*,int);
static char 	*sh_tilde(Shell_t*,const char*);
static char	*special(Shell_t *,int);
static void	endfield(Mac_t*,int);
static void	mac_error(Namval_t*);
static char	*mac_getstring(char*);
static int	charlen(const char*,int);
#if SHOPT_MULTIBYTE
    static char	*lastchar(const char*,const char*);
#endif /* SHOPT_MULTIBYTE */

void *sh_macopen(Shell_t *shp)
{
	void *addr = newof(0,Mac_t,1,0);
	Mac_t *mp = (Mac_t*)addr;
	mp->shp = shp;
	return(addr);
}

/*
 * perform only parameter substitution and catch failures
 */
char *sh_mactry(Shell_t *shp,register char *string)
{
	if(string)
	{
		int		jmp_val;
		int		savexit = shp->savexit;
		struct checkpt	buff;
		sh_pushcontext(shp,&buff,SH_JMPSUB);
		jmp_val = sigsetjmp(buff.buff,0);
		if(jmp_val == 0)
			string = sh_mactrim(shp,string,0);
		sh_popcontext(shp,&buff);
		shp->savexit = savexit;
		return(string);
	}
	return("");
}

/*
 * Perform parameter expansion, command substitution, and arithmetic
 * expansion on <str>. 
 * If <mode> greater than 1 file expansion is performed if the result 
 * yields a single pathname.
 * If <mode> negative, than expansion rules for assignment are applied.
 */
char *sh_mactrim(Shell_t *shp, char *str, register int mode)
{
	register Mac_t	*mp = (Mac_t*)shp->mac_context;
	Stk_t		*stkp = shp->stk;
	Mac_t		savemac;
	savemac = *mp;
	stkseek(stkp,0);
	mp->arith = (mode==3);
	mp->let = 0;
	shp->argaddr = 0;
	mp->pattern = (mode==1||mode==2);
	mp->patfound = 0;
	mp->assign = 0;
	if(mode<0)
		mp->assign = -mode;
	mp->quoted = mp->lit = mp->split = mp->quote = 0;
	mp->sp = 0;
	if(mp->ifsp=nv_getval(sh_scoped(shp,IFSNOD)))
		mp->ifs = *mp->ifsp;
	else
		mp->ifs = ' ';
	stkseek(stkp,0);
	fcsopen(str);
	copyto(mp,0,mp->arith);
	str = stkfreeze(stkp,1);
	if(mode==2)
	{
		/* expand only if unique */
		struct argnod *arglist=0;
		if((mode=path_expand(shp,str,&arglist))==1)
			str = arglist->argval;
		else if(mode>1)
			errormsg(SH_DICT,ERROR_exit(1),e_ambiguous,str);
		sh_trim(str);
	}
	*mp = savemac;
	return(str);
}

/*
 * Perform all the expansions on the argument <argp>
 */
int sh_macexpand(Shell_t* shp, register struct argnod *argp, struct argnod **arghead,int flag)
{
	register int	flags = argp->argflag;
	register char	*str = argp->argval;
	register Mac_t  *mp = (Mac_t*)shp->mac_context;
	char		**saveargaddr = shp->argaddr;
	Mac_t		savemac;
	Stk_t		*stkp = shp->stk;
	savemac = *mp;
	mp->sp = 0;
	if(mp->ifsp=nv_getval(sh_scoped(shp,IFSNOD)))
		mp->ifs = *mp->ifsp;
	else
		mp->ifs = ' ';
	if((flag&ARG_OPTIMIZE) && !shp->indebug && !(flags&ARG_MESSAGE))
		shp->argaddr = (char**)&argp->argchn.ap;
	else
		shp->argaddr = 0;
	mp->arghead = arghead;
	mp->quoted = mp->lit = mp->quote = 0;
	mp->arith = ((flag&ARG_ARITH)!=0);
	mp->let = ((flag&ARG_LET)!=0);
	mp->split = !(flag&ARG_ASSIGN);
	mp->assign = !mp->split;
	mp->pattern = mp->split && !(flag&ARG_NOGLOB) && !sh_isoption(mp->shp,SH_NOGLOB);
	mp->arrayok = mp->arith || (flag&ARG_ARRAYOK);
	str = argp->argval;
	fcsopen(str);
	mp->fields = 0;
	mp->atmode = 0;
	if(!arghead)
	{
		mp->split = 0;
		mp->pattern = ((flag&ARG_EXP)!=0);
		mp->maccase = ((flag&ARG_CASE)!=0);
		stkseek(stkp,0);
	}
	else
	{
		stkseek(stkp,ARGVAL);
		*stkptr(stkp,ARGVAL-1) = 0;
	}
	mp->patfound = 0;
	if(mp->pattern)
		mp->arrayok = 0;
	copyto(mp,0,mp->arith);
	if(!arghead)
	{
		argp->argchn.cp = stkfreeze(stkp,1);
		if(shp->argaddr)
			argp->argflag |= ARG_MAKE;
	}
	else
	{
		endfield(mp,mp->quoted|mp->atmode);
		flags = mp->fields;
		if(flags==1 && shp->argaddr)
			argp->argchn.ap = *arghead; 
	}
	shp->argaddr = saveargaddr;
	*mp = savemac;
	return(flags);
}

/*
 * Expand here document which is stored in <infile> or <string>
 * The result is written to <outfile>
 */
void sh_machere(Shell_t *shp,Sfio_t *infile, Sfio_t *outfile, char *string)
{
	register int	c,n;
	register const char	*state = sh_lexstates[ST_QUOTE];
	register char	*cp;
	register Mac_t	*mp = (Mac_t*)shp->mac_context;
	Lex_t		*lp = (Lex_t*)mp->shp->lex_context;
	Fcin_t		save;
	Mac_t		savemac;
	Stk_t		*stkp = shp->stk;
	savemac = *mp;
	stkseek(stkp,0);
	shp->argaddr = 0;
	mp->sp = outfile;
	mp->split = mp->assign = mp->pattern = mp->patfound = mp->lit = mp->arith = mp->let = 0;
	mp->quote = 1;
	mp->ifsp = nv_getval(sh_scoped(shp,IFSNOD));
	mp->ifs = ' ';
	fcsave(&save);
	if(infile)
		fcfopen(infile);
	else
		fcsopen(string);
	fcnotify(0,lp);
	cp = fcseek(0);
	while(1)
	{
#if SHOPT_MULTIBYTE
		if(mbwide())
		{
			do
			{
				ssize_t len;
				switch(len = mbsize(cp))
				{
				    case -1:	/* illegal multi-byte char */
				    case 0:
				    case 1:
					n=state[*(unsigned char*)cp++];
					break;
				    default:
					/* use state of alpha character */
					n=state['a'];
					cp += len;
				}
			}
			while(n == 0);
		}
		else
#endif /* SHOPT_MULTIBYTE */
		while((n=state[*(unsigned char*)cp++])==0);
		if(n==S_NL || n==S_QUOTE || n==S_RBRA)
			continue;
		if(c=(cp-1)-fcseek(0))
			sfwrite(outfile,fcseek(0),c);
		cp = fcseek(c+1);
		switch(n)
		{
		    case S_EOF:
			if((n=fcfill()) <=0)
			{
				/* ignore 0 byte when reading from file */
				if(n==0 && fcfile())
					continue;
				fcrestore(&save);
				*mp = savemac;
				return;
			}
			cp = fcseek(-1);
			continue;
		    case S_ESC:
			fcgetc(c);
			cp=fcseek(-1);
			if(c>0)
				cp++;
			if(!isescchar(state[c]))
				sfputc(outfile,ESCAPE);
			continue;
		    case S_GRAVE:
			comsubst(mp,(Shnode_t*)0,0);
			break;
		    case S_DOL:
			c = fcget();
			if(c=='.')
				goto regular;
		    again:
			switch(n=sh_lexstates[ST_DOL][c])
			{
			    case S_ALP: case S_SPC1: case S_SPC2:
			    case S_DIG: case S_LBRA:
			    {
				Fcin_t	save2;
				int	offset = stktell(stkp);
				int	offset2;
				fcnotify(0,lp);
				sfputc(stkp,c);
				if(n==S_LBRA)
				{
					c = fcget();
					fcseek(-1);
					if(sh_lexstates[ST_NORM][c]==S_BREAK)
					{
						comsubst(mp,(Shnode_t*)0,2);
						break;
					}
					sh_lexskip(lp,RBRACE,1,ST_BRACE);
				}
				else if(n==S_ALP)
				{
					while(fcgetc(c),isaname(c))
						sfputc(stkp,c);
					fcseek(-1);
				}
				sfputc(stkp,0);
				offset2 = stktell(stkp);
				fcsave(&save2);
				fcsopen(stkptr(stkp,offset));
				varsub(mp);
				if(c=stktell(stkp)-offset2)
					sfwrite(outfile,(char*)stkptr(stkp,offset2),c);
				fcrestore(&save2);
				stkseek(stkp,offset);
				break;
			    }
			    case S_PAR:
				comsubst(mp,(Shnode_t*)0,3);
				break;
			    case S_EOF:
				if((c=fcfill()) > 0)
					goto again;
				/* FALL THRU */
			    default:
			    regular:
				sfputc(outfile,'$');
				fcseek(-1);
				break;
			}
		}
		cp = fcseek(0);
	}
}

/*
 * expand argument but do not trim pattern characters
 */
char *sh_macpat(Shell_t *shp,register struct argnod *arg, int flags)
{
	register char *sp = arg->argval;
	if((arg->argflag&ARG_RAW))
		return(sp);
	sh_stats(STAT_ARGEXPAND);
	if(flags&ARG_OPTIMIZE)
		arg->argchn.ap=0;
	if(!(sp=arg->argchn.cp))
	{
		sh_macexpand(shp,arg,NIL(struct argnod**),flags|ARG_ARRAYOK);
		sp = arg->argchn.cp;
		if(!(flags&ARG_OPTIMIZE) || !(arg->argflag&ARG_MAKE))
			arg->argchn.cp = 0;
		arg->argflag &= ~ARG_MAKE;
	}
	else
		sh_stats(STAT_ARGHITS);
	return(sp);
}

/*
 * Process the characters up to <endch> or end of input string 
 */
static void copyto(register Mac_t *mp,int endch, int newquote)
{
	register int	c,n;
	register const char	*state = sh_lexstates[ST_MACRO];
	register char	*cp,*first;
	Shell_t		*shp = mp->shp;
	Lex_t		*lp = (Lex_t*)shp->lex_context;
	int		tilde = -1;
	int		oldquote = mp->quote;
	int		ansi_c = 0;
	int		paren = 0;
	int		ere = 0;
	int		dotdot = 0;
	int		brace = 0;
	Sfio_t		*sp = mp->sp;
	Stk_t		*stkp = shp->stk;
	char		*resume = 0;
	mp->sp = NIL(Sfio_t*);
	mp->quote = newquote;
	first = cp = fcseek(0);
	if(!mp->quote && *cp=='~' && cp[1]!=LPAREN)
		tilde = stktell(stkp);
	/* handle // operator specially */
	if(mp->pattern==2 && *cp=='/')
		cp++;
	while(1)
	{
#if SHOPT_MULTIBYTE
		if(mbwide())
		{
			ssize_t len;
			do
			{
				switch(len = mbsize(cp))
				{
				    case -1:	/* illegal multi-byte char */
				    case 0:
					len = 1;
				    case 1:
					n = state[*(unsigned char*)cp++];
					break;
				    default:
					/* treat as if alpha */
					cp += len;
					n=state['a'];
				}
			}
			while(n == 0);
			c = (cp-len) - first;
		}
		else
#endif /* SHOPT_MULTIBYTE */
		{
			while((n=state[*(unsigned char*)cp++])==0);
			c = (cp-1) - first;
		}
		switch(n)
		{
		    case S_ESC:
			if(ansi_c)
			{
				/* process ANSI-C escape character */
				char *addr= --cp;
				if(c)
					sfwrite(stkp,first,c);
				c = chresc(cp,&addr);
				cp = addr;
				first = fcseek(cp-first);
#if SHOPT_MULTIBYTE
				if(c > UCHAR_MAX && mbwide())
				{
					int		i;
					unsigned char	mb[8];

					n = mbconv((char*)mb, c);
					for(i=0;i<n;i++)
						sfputc(stkp,mb[i]);
				}
				else
#endif /* SHOPT_MULTIBYTE */
				sfputc(stkp,c);
				if(c==ESCAPE && mp->pattern)
					sfputc(stkp,ESCAPE);
				break;
			}
			else if(sh_isoption(shp,SH_BRACEEXPAND) && mp->pattern==4 && (*cp==',' || *cp==LBRACE || *cp==RBRACE || *cp=='.'))
				break;
			else if(mp->split && endch && !mp->quote && !mp->lit)
			{
				if(c)
					mac_copy(mp,first,c);
				cp = fcseek(c+2);
				if(c= cp[-1])
				{
					sfputc(stkp,c);
					if(c==ESCAPE)
						sfputc(stkp,ESCAPE);
				}
				else
					cp--;
				first = cp;
				break;
			}
			n = state[*(unsigned char*)cp];
			if(n==S_ENDCH && *cp!=endch)
				n = S_PAT;
			if(mp->pattern)
			{
				/* preserve \digit for pattern matching */
				/* also \alpha for extended patterns */
				if(!mp->lit && !mp->quote)
				{
					int nc = *(unsigned char*)cp;
					if((n==S_DIG || ((paren+ere) && (sh_lexstates[ST_DOL][nc]==S_ALP) || nc=='<' || nc=='>')))
						break;
					if(ere && mp->pattern==1 && strchr(".[()*+?{|^$&!",*cp))
						break;
				}
				/* followed by file expansion */
				if(!mp->lit && (n==S_ESC || (!mp->quote && 
					(n==S_PAT||n==S_ENDCH||n==S_SLASH||n==S_BRACT||*cp=='-'))))
				{
					cp += (n!=S_EOF);
					if(ere && n==S_ESC && *cp =='\\' && cp[1]=='$')
					{
						/* convert \\\$ into \$' */
						sfwrite(stkp,first,c+1);
						cp = first = fcseek(c+3);
					}
					break;
				}
				if(!(ere && *cp=='$') && (mp->lit || (mp->quote && !isqescchar(n) && n!=S_ENDCH)))
				{
					/* add \ for file expansion */
					sfwrite(stkp,first,c+1);
					first = fcseek(c);
					break;
				}
			}
			if(mp->lit)
				break;
			if(!mp->quote || isqescchar(n) || n==S_ENDCH)
			{
				/* eliminate \ */
				if(c)
					sfwrite(stkp,first,c);
				/* check new-line joining */
				first = fcseek(c+1);
			}
			cp += (n!=S_EOF);
			break;
		    case S_GRAVE: case S_DOL:
			if(mp->lit)
				break;
			if(c)
			{
				if(mp->split && !mp->quote && endch)
					mac_copy(mp,first,c);
				else
					sfwrite(stkp,first,c);
			}
			first = fcseek(c+1);
			c = mp->pattern;
			if(n==S_GRAVE)
				comsubst(mp,(Shnode_t*)0,0);
			else if((n= *cp) == '"' && !mp->quote)
			{
				int off = stktell(stkp);
				char	*dp;
				cp = first = fcseek(1);
				mp->quote = 1;
				if(!ERROR_translating())
					break;
				while(n=c, c= *++cp)
				{
					if(c=='\\' && n==c)
						c = 0;
					else if(c=='"' && n!='\\')
						break;
				}
				n = cp-first;
				sfwrite(stkp,first,n);
				sfputc(stkp,0);
				cp = stkptr(stkp,off);
				dp = (char*)sh_translate(cp);
				stkseek(stkp,off);
				if(dp==cp)
				{
					cp = first;
					break;
				}
				resume = fcseek(n);
				fcclose();
				fcsopen(dp);
				cp = first = fcseek(0);
				break;
			}
			else if(n==0 || !varsub(mp))
			{
				if(n=='\'' && !mp->quote)
					ansi_c = 1;
				else if(mp->quote || n!='"')
					sfputc(stkp,'$');
			}
			cp = first = fcseek(0);
			if(mp->quote && cp)
				mp->pattern = c;
			break;
		    case S_ENDCH:
			if((mp->lit || cp[-1]!=endch || mp->quote!=newquote))
				goto pattern;
			if(endch==RBRACE && mp->pattern && brace)
			{
				brace--;
				if(*cp==LPAREN && mp->pattern!=2)
					goto pattern;
				continue;
			}
		    case S_EOF:
			if(c)
			{
				if(mp->split && !mp->quote && !mp->lit && endch)
					mac_copy(mp,first,c);
				else
					sfwrite(stkp,first,c);
			}
			if(n==S_EOF && resume)
			{
				fcclose();
				fcsopen(resume);
				resume = 0;
				cp = first = fcseek(0);
				continue;
			}
			c += (n!=S_EOF);
			first = fcseek(c);
			if(tilde>=0)
				tilde_expand2(shp,tilde);
			goto done;
		    case S_QUOTE:
			if(mp->lit || mp->arith)
				break;
		    case S_LIT:
			if(mp->arith)
			{
				if((*cp=='`' || *cp=='[') && cp[1]=='\'')
					cp +=2;
				break;
			}
			if(n==S_LIT && mp->quote)
				break;
			if(c)
			{
				if(mp->split && endch && !mp->quote && !mp->lit)
					mac_copy(mp,first,c);
				else
					sfwrite(stkp,first,c);
			}
			first = fcseek(c+1);
			if(n==S_LIT)
			{
				if(mp->quote)
					continue;
				if(mp->lit)
					mp->lit = ansi_c = 0;
				else
					mp->lit = 1;
			}
			else
				mp->quote = !mp->quote;
			mp->quoted++;
			break;
		    case S_BRACT:
			if(mp->arith || (((mp->assign&1) || endch==RBRACT) &&
				!(mp->quote || mp->lit)))
			{
				int offset=0,oldpat = mp->pattern;
				int oldarith = mp->arith, oldsub=mp->subcopy;
				sfwrite(stkp,first,++c);
				if(mp->assign&1)
				{
					if(first[c-2]=='.')
						offset = stktell(stkp);
					if(isastchar(*cp) && cp[1]==']')
						errormsg(SH_DICT,ERROR_exit(1),
e_badsubscript,*cp);
				}
				first = fcseek(c);
				mp->pattern = 4;
				mp->arith = 0;
				mp->subcopy = 0;
				copyto(mp,RBRACT,0);
				mp->subcopy = oldsub;
				mp->arith = oldarith;
				mp->pattern = oldpat;
				sfputc(stkp,RBRACT);
				if(offset)
				{
					cp = stkptr(stkp,stktell(stkp));
					if(sh_checkid(stkptr(stkp,offset),cp)!=cp)
						stkseek(stkp,stktell(stkp)-2);
				}
				cp = first = fcseek(0);
				break;
			}
		    case S_PAT:
			if(mp->pattern && !(mp->quote || mp->lit))
			{
				mp->patfound = mp->pattern;
				if((n=cp[-1])==LPAREN)
				{
					paren++;
					if((cp-first)>1 && cp[-2]=='~')
					{
						char *p = cp;
						while((c=mbchar(p)) && c!=RPAREN)
							if(c=='A'||c=='E'||c=='K'||c=='P'||c=='X')
							{
								ere = 1;
								break;
							}
					}
				}
				else if(n==RPAREN)
					--paren;
			}
			goto pattern;
		    case S_COM:
			if(mp->pattern==4 && (mp->quote || mp->lit))
			{
				if(c)
				{
					sfwrite(stkp,first,c);
					first = fcseek(c);
				}
				sfputc(stkp,ESCAPE);
			}
			break;
		    case S_BRACE:
			if(!(mp->quote || mp->lit))
			{
				mp->patfound = mp->split && sh_isoption(shp,SH_BRACEEXPAND);
				brace++;
			}
		    pattern:
			if(!mp->pattern || !(mp->quote || mp->lit))
			{
				/* mark beginning of {a,b} */
				if(n==S_BRACE && endch==0 && mp->pattern)
					mp->pattern=4;
				if(n==S_SLASH && mp->pattern==2)
					mp->pattern=3;
				break;
			}
			if(mp->pattern==3)
				break;
			if(c)
				sfwrite(stkp,first,c);
			first = fcseek(c);
			sfputc(stkp,ESCAPE);
			break;
		    case S_EQ:
			if(mp->assign==1)
			{
				if(*cp=='~' && !endch && !mp->quote && !mp->lit)
					tilde = stktell(stkp)+(c+1);
				mp->assign = 2;
			}
			break;
		    case S_SLASH:
		    case S_COLON:
			if(tilde >=0)
			{
				if(c)
					sfwrite(stkp,first,c);
				first = fcseek(c);
				tilde_expand2(shp,tilde);
#if _WINIX
				if(Skip)
				{
					first = cp = fcseek(Skip);
					Skip = 0;
				}
#endif /*_WINIX */
				tilde = -1;
				c=0;
			}
			if(n==S_COLON && mp->assign==2 && *cp=='~' && endch==0 && !mp->quote &&!mp->lit)
				tilde = stktell(stkp)+(c+1);
			else if(n==S_SLASH && mp->pattern==2)
#if 0
				goto pattern;
#else
			{
				if(mp->quote || mp->lit)
					goto pattern;
				sfwrite(stkp,first,c+1);
				first = fcseek(c+1);
				c = stktell(stkp);
				sh_lexskip(lp,RBRACE,0,ST_NESTED);
				stkseek(stkp,c);
				cp = fcseek(-1);
				sfwrite(stkp,first,cp-first);
				first=cp;
			}
#endif
			break;
		    case S_DOT:
			if(*cp=='.' && mp->subcopy==1)
			{
				sfwrite(stkp,first,c);
				sfputc(stkp,0);
				dotdot = stktell(stkp);
				cp = first = fcseek(c+2);
			}
			break;
		}
	}
done:
	mp->sp = sp;
	mp->dotdot = dotdot;
	mp->quote = oldquote;
}

/*
 * copy <str> to stack performing sub-expression substitutions
 */
static void mac_substitute(Mac_t *mp, register char *cp,char *str,register int subexp[],int subsize)
{
	register int	c,n;
	register char *first=fcseek(0);
	char		*ptr;
	Mac_t		savemac;
	Stk_t		*stkp = mp->shp->stk;
	n = stktell(stkp);
	savemac = *mp;
	mp->pattern = 3;
	mp->split = 0;
	mp->macsub++;
	fcsopen(cp);
	copyto(mp,0,0);
	sfputc(stkp,0);
	ptr = cp = strdup(stkptr(stkp,n));
	stkseek(stkp,n);
	*mp = savemac;
	fcsopen(first);
	first = cp;
	while(1)
	{
		while((c= *cp++) && c!=ESCAPE);
		if(c==0)
			break;
		if((n= *cp++)=='\\' || n==RBRACE || (n>='0' && n<='9' && (n-='0')<subsize))
		{
			c = cp-first-2;
			if(c)
				mac_copy(mp,first,c);
			first=cp;
			if(n=='\\' || n==RBRACE)
			{
				first--;
				continue;
			}
			if((c=subexp[2*n])>=0)
			{
				if((n=subexp[2*n+1]-c)>0)
					mac_copy(mp,str+c,n);
			}
		}
		else if(n==0)
			break;
	}
	if(n=cp-first-1)
		mac_copy(mp,first,n);
	free(ptr);
}

#if  SHOPT_FILESCAN
#define	MAX_OFFSETS	 (sizeof(shp->offsets)/sizeof(shp->offsets[0]))
#define MAX_ARGN	(32*1024)

/*
 * compute the arguments $1 ... $n and $# from the current line as needed
 * save line offsets in the offsets array.
 */
static char *getdolarg(Shell_t *shp, int n, int *size)
{
	register int c=S_DELIM, d=shp->ifstable['\\'];
	register unsigned char *first,*last,*cp = (unsigned char*)shp->cur_line;
	register int m=shp->offsets[0],delim=0;
	if(m==0)
		return(0);
	if(m<0)
		m = 0;
	else if(n<=m)
		m = n-1;
	else
		m--;
	if(m >= MAX_OFFSETS-1)
		m =  MAX_OFFSETS-2;
	cp += shp->offsets[m+1];
	n -= m;
	shp->ifstable['\\'] = 0;
	shp->ifstable[0] = S_EOF;
	while(1)
	{
		if(c==S_DELIM)
			while(shp->ifstable[*cp++]==S_SPACE);
		first = --cp;
		if(++m < MAX_OFFSETS)
			shp->offsets[m] = (first-(unsigned char*)shp->cur_line);
		while((c=shp->ifstable[*cp++])==0);
		last = cp-1;
		if(c==S_SPACE)
			while((c=shp->ifstable[*cp++])==S_SPACE);
		if(--n==0 || c==S_EOF)
		{
			if(last==first && c==S_EOF && (!delim || (m>1)))
			{
				n++;
				m--;
			}
			break;
		}
		delim = (c==S_DELIM);
	}
	shp->ifstable['\\'] = d;
	if(m > shp->offsets[0])
		shp->offsets[0] = m;
	if(n)
		first = last = 0;
	if(size)
		*size = last-first;
	return((char*)first);
}
#endif /* SHOPT_FILESCAN */

/*
 * get the prefix after name reference resolution
 */
static char *prefix(Shell_t *shp, char *id)
{
	Namval_t *np;
	register char *sub=0, *cp = strchr(id,'.');
	if(cp)
	{
		*cp = 0;
		np = nv_search(id, shp->var_tree,0);
		*cp = '.';
		if(isastchar(cp[1]))
			cp[1] = 0;
		if(np && nv_isref(np))
		{
			size_t n;
			char *sp;
			shp->argaddr = 0;
			while(nv_isref(np) && np->nvalue.cp)
			{
				sub = nv_refsub(np);
				np = nv_refnode(np);
				if(sub)
					nv_putsub(np,sub,0,0L);
			}
			id = (char*)malloc(strlen(cp)+1+(n=strlen(sp=nv_name(np)))+ (sub?strlen(sub)+3:1));
			memcpy(id,sp,n);
			if(sub)
			{
				id[n++] = '[';
				strcpy(&id[n],sub);
				n+= strlen(sub)+1;
				id[n-1] = ']';
			}
			strcpy(&id[n],cp);
			return(id);
		}
	}
	return(strdup(id));
}

/*
 * copy to ']' onto the stack and return offset to it
 */
static int subcopy(Mac_t *mp, int flag)
{
	int split = mp->split;
	int xpattern = mp->pattern;
	int loc = stktell(mp->shp->stk);
	int xarith = mp->arith;
	int arrayok = mp->arrayok;
	mp->split = 0;
	mp->arith = 0;
	mp->pattern = flag?4:0;
	mp->arrayok=1;
	mp->subcopy++;
	copyto(mp,RBRACT,0);
	mp->subcopy = 0;
	mp->pattern = xpattern;
	mp->split = split;
	mp->arith = xarith;
	mp->arrayok = arrayok;
	return(loc);
}

/*
 * if name is a discipline function, run the function and put the results
 * on the stack so that ${x.foo} behaves like ${ x.foo;}
 */
bool sh_macfun(Shell_t *shp, const char *name, int offset)
{
	Namval_t	*np, *nq;
	np = nv_bfsearch(name,shp->fun_tree,&nq,(char**)0);
	if(np)
	{
		/* treat ${x.foo} as ${x.foo;} */
		union
		{
			struct comnod	com;
			Shnode_t	node;
		} t;
		union
		{
			struct argnod	arg;
			struct dolnod	dol;
			char buff[sizeof(struct dolnod)+sizeof(char*)];
		} d;
		memset(&t,0,sizeof(t));
		memset(&d,0,sizeof(d));
		t.node.com.comarg = &d.arg;
		t.node.com.comline = shp->inlineno;
		d.dol.dolnum = 1;
		d.dol.dolval[0] = strdup(name);
		stkseek(shp->stk,offset);
		comsubst((Mac_t*)shp->mac_context,&t.node,2);
		free(d.dol.dolval[0]);
		return(true);
	}
	return(false);
}

static int namecount(Mac_t *mp,const char *prefix)
{
	int count = 0;
	mp->nvwalk = nv_diropen((Namval_t*)0,prefix,mp->shp);
	while(nv_dirnext(mp->nvwalk))
		count++;
	nv_dirclose(mp->nvwalk);
	return(count);
}

static char *nextname(Mac_t *mp,const char *prefix, int len)
{
	char *cp;
	if(len==0)
	{
		mp->nvwalk = nv_diropen((Namval_t*)0,prefix,mp->shp);
		return((char*)mp->nvwalk);
	}
	if(!(cp=nv_dirnext(mp->nvwalk)))
		nv_dirclose(mp->nvwalk);
	return(cp);
}

/*
 * This routine handles $param,  ${parm}, and ${param op word}
 * The input stream is assumed to be a string
 */
static bool varsub(Mac_t *mp)
{
	register int	c;
	register int	type=0; /* M_xxx */
	register char	*v,*argp=0;
	register Namval_t	*np = NIL(Namval_t*);
	register int 	dolg=0, mode=0;
	Lex_t		*lp = (Lex_t*)mp->shp->lex_context;
	Namarr_t	*ap=0;
	int		dolmax=0, vsize= -1, offset= -1, nulflg, replen=0, bysub=0;
	char		idbuff[3], *id = idbuff, *pattern=0, *repstr=0, *arrmax=0;
	char		*idx = 0;
	int		var=1,addsub=0,oldpat=mp->pattern,idnum=0,flag=0,d;
	Stk_t		*stkp = mp->shp->stk;
retry1:
	mp->zeros = 0;
	mp->dotdot = 0;
	idbuff[0] = 0;
	idbuff[1] = 0;
	c = fcmbget(&LEN);
	switch(isascii(c)?sh_lexstates[ST_DOL][c]:S_ALP)
	{
	    case S_RBRA:
		if(type<M_SIZE)
			goto nosub;
		/* This code handles ${#} */
		c = mode;
		mode = type = 0;
		/* FALL THRU */
	    case S_SPC1:
		if(type==M_BRACE)
		{
			if(isaletter(mode=fcpeek(0)) || mode=='.')
			{
				if(c=='#')
					type = M_SIZE;
				else if(c=='@')
				{
					type = M_TYPE;
					goto retry1;
				}
				else
					type = M_VNAME;
				mode = c;
				goto retry1;
			}
			else if(c=='#' && (isadigit(mode)||fcpeek(1)==RBRACE))
			{
				type = M_SIZE;
				mode = c;
				goto retry1;
			}
#if SHOPT_BASH
			else if(sh_isoption(mp->shp,SH_BASH) && c=='!' && isadigit(mode))
				c = '$';
#endif
		}
		/* FALL THRU */
	    case S_SPC2:
		if(type==M_BRACE && c=='$' && isalnum(mode=fcpeek(0)))
		{
			type = M_EVAL;
			mode = c;
			goto retry1;
		}
		var = 0;
		*id = c;
		v = special(mp->shp,c);
		if(isastchar(c))
		{
			mode = c;
#if  SHOPT_FILESCAN
			if(mp->shp->cur_line)
			{
				dolmax = MAX_ARGN;
				v = getdolarg(mp->shp,1,&vsize);
				if(c=='*' || !mp->quoted)
				{
					dolmax = 1;
					vsize = -1;
				}
			}
			else
#endif  /* SHOPT_FILESCAN */
			dolmax = mp->shp->st.dolc+1;
			mp->atmode = (v && mp->quoted && c=='@');
			dolg = (v!=0);
		}
		break;
	    case S_LBRA:
		if(type)
			goto nosub;
		type = M_BRACE;
		goto retry1;
	    case S_PAR:
		if(type)
			goto nosub;
		comsubst(mp,(Shnode_t*)0,3);
		return(true);
	    case S_DIG:
		var = 0;
		c -= '0';
		mp->shp->argaddr = 0;
		if(type)
		{
			while((d=fcget()),isadigit(d))
				c = 10*c + (d-'0');
			fcseek(-1);
		}
		idnum = c;
		if(c==0)
			v = special(mp->shp,c);
#if  SHOPT_FILESCAN
		else if(mp->shp->cur_line)
		{
			mp->shp->used_pos = 1;
			v = getdolarg(mp->shp,c,&vsize);
		}
#endif  /* SHOPT_FILESCAN */
		else if(c <= mp->shp->st.dolc)
		{
			mp->shp->used_pos = 1;
			v = mp->shp->st.dolv[c];
		}
		else
			v = 0;
		if(!v && sh_isoption(mp->shp,SH_NOUNSET))
		{
			d=fcget();
			fcseek(-1);
			if(!strchr(":+-?=",d))
				errormsg(SH_DICT,ERROR_exit(1),e_notset,ltos(c));
		}
		break;
	    case S_ALP:
		if(c=='.' && type==0)
			goto nosub;
		offset = stktell(stkp);
		do
		{
			np = 0;
			do
			{
				if(LEN==1)
					sfputc(stkp,c);
				else
					sfwrite(stkp,fcseek(0)-LEN,LEN);
			}
			while((d=c,(c=fcmbget(&LEN)),isaname(c))||type && c=='.');
			while(c==LBRACT && (type||mp->arrayok))
			{
				mp->shp->argaddr=0;
				if((c=fcmbget(&LEN),isastchar(c)) && fcpeek(0)==RBRACT && d!='.')
				{
					if(type==M_VNAME)
						type = M_SUBNAME;
					idbuff[0] = mode = c;
					fcget();
					c = fcmbget(&LEN);
					if(c=='.' || c==LBRACT)
					{
						sfputc(stkp,LBRACT);
						sfputc(stkp,mode);
						sfputc(stkp,RBRACT);
					}
					else
						flag = NV_ARRAY;
					break;
				}
				else
				{
					fcseek(-LEN);
					c = stktell(stkp);
					if(d!='.')
						sfputc(stkp,LBRACT);
					v = stkptr(stkp,subcopy(mp,1));
					if(type && mp->dotdot)
					{
						mode = '@';
						v[-1] = 0;
						if(type==M_VNAME)
							type = M_SUBNAME;
						else if(type==M_SIZE)
							goto nosub;
					}
					else if(d!='.')
						sfputc(stkp,RBRACT);
					c = fcmbget(&LEN);
					if(c==0 && type==M_VNAME)
						type = M_SUBNAME;
				}
			}
		}
		while(type && c=='.');
		if(type!=M_VNAME && c==RBRACE && type &&  fcpeek(-2)=='.')
		{
			/* ${x.} or ${x..} */
			if(fcpeek(-3) == '.')
			{
				stkseek(stkp,stktell(stkp)-2);
				nv_local = 1;
			}
			else
			{
				stkseek(stkp,stktell(stkp)-1);
				type = M_TREE;
			}
		}
		sfputc(stkp,0);
		id=stkptr(stkp,offset);
		if(isastchar(c) && type)
		{
			if(type==M_VNAME || type==M_SIZE)
			{
				idbuff[0] = mode = c;
				if((d=fcpeek(0))==c)
					idbuff[1] = fcget();
				if(type==M_VNAME)
					type = M_NAMESCAN;
				else
					type = M_NAMECOUNT;
				break;
			}
			goto nosub;
		}
		flag |= NV_NOASSIGN|NV_VARNAME|NV_NOADD;
		if(c=='=' || c=='?' || (c==':' && ((d=fcpeek(0))=='=' || d=='?')))
		{
			if(c=='=' || (c==':' && d=='='))
				flag |= NV_ASSIGN;
			flag &= ~NV_NOADD;
		}
#if  SHOPT_FILESCAN
		if(mp->shp->cur_line && *id=='R' && strcmp(id,"REPLY")==0)
		{
			mp->shp->argaddr=0;
			np = REPLYNOD;
		}
		else
#endif  /* SHOPT_FILESCAN */
		{
			if(mp->shp->argaddr)
				flag &= ~NV_NOADD;
			np = nv_open(id,mp->shp->var_tree,flag|NV_NOFAIL);
			if(!np)
			{
				sfprintf(mp->shp->strbuf,"%s%c",id,0);
				id = sfstruse(mp->shp->strbuf);
			}
#if SHOPT_BASH
			if(sh_isoption(mp->shp,SH_BASH) && (c=='}'||c==':') && type==M_VNAME && !nv_isattr(np,NV_REF))
				type = M_EVAL;
#endif
			if(type==M_EVAL && np && (v=nv_getval(np)))
			{
				char *last;
				int n = strtol(v,&last,10);
				type = M_BRACE;
				if(*last==0)
				{
					np = 0;
					v = 0;
					idnum = n;
					if(n==0)
						v = special(mp->shp,n);
					else if(n <= mp->shp->st.dolc)
					{
						mp->shp->used_pos = 1;
						v = mp->shp->st.dolv[n];
					}
					else
						idnum = 0;
					fcseek(-LEN);
					stkseek(stkp,offset);
					break;
				}
				else
				{
					np = nv_open(v,mp->shp->var_tree,flag|NV_NOFAIL);
#if  0
					type = M_BRACE;
				if(c!='}')
					mac_error(np);
#endif
				}
			}
		}
		if(isastchar(mode))
			var = 0;
		if((!np || nv_isnull(np)) && type==M_BRACE && c==RBRACE && !(flag&NV_ARRAY) && strchr(id,'.'))
		{
			if(sh_macfun(mp->shp,id,offset))
			{
				fcmbget(&LEN);
				return(true);
			}
		}
		if(np && (flag&NV_NOADD) && nv_isnull(np))
		{
			if(nv_isattr(np,NV_NOFREE))
				nv_offattr(np,NV_NOFREE);
#if  SHOPT_FILESCAN
			else if(np!=REPLYNOD  || !mp->shp->cur_line)
#else
			else
#endif  /* SHOPT_FILESCAN */
				np = 0;
		}
		ap = np?nv_arrayptr(np):0;
		if(type)
		{
			if(mp->dotdot)
			{
				Namval_t *nq;
#if SHOPT_FIXEDARRAY
				if(ap && !ap->fixed && (nq=nv_opensub(np)))
#else
				if(ap && (nq=nv_opensub(np)))
#endif /* SHOPT_FIXEDARRAY */
					ap = nv_arrayptr(np=nq);
				if(ap)
				{
					np = nv_putsub(np,v,0,ARRAY_SCAN);
					v = stkptr(stkp,mp->dotdot);
					dolmax =1;
					if(array_assoc(ap))
						arrmax = strdup(v);
					else if((dolmax = (int)sh_arith(mp->shp,v))<0)
						dolmax += array_maxindex(np);
					if(type==M_SUBNAME)
						bysub = 1;
				}
				else
				{
					if((int)sh_arith(mp->shp,v))
						np = 0;
				}
			}
			else if(ap && (isastchar(mode)||type==M_TREE)  && !(ap->flags&ARRAY_SCAN) && type!=M_SIZE)
				nv_putsub(np,NIL(char*),0,ARRAY_SCAN);
			if(!isbracechar(c))
				goto nosub;
			else
				fcseek(-LEN);
		}
		else
			fcseek(-1);
		if(type<=1 && np && nv_isvtree(np) && mp->pattern==1 && !mp->split)
		{
			int cc=fcmbget(&LEN),peek=LEN;
			if(type && cc=='}')
			{
				cc = fcmbget(&LEN);
				peek++;
			}
			if(mp->quote && cc=='"')
			{
				cc = fcmbget(&LEN);
				peek++;
			}
			fcseek(-peek);
			if(cc==0)
				mp->assign = 1;
		}
		if((type==M_VNAME||type==M_SUBNAME)  && mp->shp->argaddr && strcmp(nv_name(np),id))
			mp->shp->argaddr = 0;
		c = (type>M_BRACE && isastchar(mode));
		if(np && (type==M_TREE || !c || !ap))
		{
			char *savptr;
			c = *((unsigned char*)stkptr(stkp,offset-1));
			savptr = stkfreeze(stkp,0);
			if(type==M_VNAME || (type==M_SUBNAME && ap))
			{
				type = M_BRACE;
				v = nv_name(np);
				if(ap && !mp->dotdot && !(ap->flags&ARRAY_UNDEF))
					addsub = 1;
			}
			else if(type==M_TYPE)
			{
				Namval_t *nq = nv_type(np);
				type = M_BRACE;
				if(nq)
					nv_typename(nq,mp->shp->strbuf);
				else
					nv_attribute(np,mp->shp->strbuf,"typeset",1);
				v = sfstruse(mp->shp->strbuf);
			}
#if  SHOPT_FILESCAN
			else if(mp->shp->cur_line && np==REPLYNOD)
				v = mp->shp->cur_line;
#endif  /* SHOPT_FILESCAN */
			else if(type==M_TREE)
				v = nv_getvtree(np,(Namfun_t*)0);
			else
			{
				if(type && fcpeek(0)=='+')
				{
					if(ap)
						v = nv_arrayisset(np,ap)?(char*)"x":0;
					else
						v = nv_isnull(np)?0:(char*)"x";
				}
				else
					v = nv_getval(np);
				mp->atmode = (v && mp->quoted && mode=='@');
				/* special case --- ignore leading zeros */  
				if((mp->let || (mp->arith&&nv_isattr(np,(NV_LJUST|NV_RJUST|NV_ZFILL)))) && !nv_isattr(np,NV_INTEGER) && (offset==0 || isspace(c) || strchr(",.+-*/=%&|^?!<>",c)))
					mp->zeros = 1;
			}
			if(savptr==stkptr(stkp,0))
				stkseek(stkp,offset);
			else
				stkset(stkp,savptr,offset);
		}
		else
		{
			if(sh_isoption(mp->shp,SH_NOUNSET) && !isastchar(mode) && (type==M_VNAME || type==M_SIZE))
				errormsg(SH_DICT,ERROR_exit(1),e_notset,id);
			v = 0;
			if(type==M_VNAME)
			{
				v = id;
				type = M_BRACE;
			}
			else if(type==M_TYPE)
				type = M_BRACE;
		}
		stkseek(stkp,offset);
		if(ap)
		{
#if SHOPT_OPTIMIZE
			if(mp->shp->argaddr)
				nv_optimize(np);
#endif
			if(isastchar(mode) && array_elem(ap)> !c)
				dolg = -1;
			else
			{
				ap->flags &= ~ARRAY_SCAN;
				dolg = 0;
		
			}
		}
		break;
	    case S_EOF:
		fcseek(-1);
	    default:
		goto nosub;
	}
	c = fcmbget(&LEN);
	if(type>M_TREE)
	{
		if(c!=RBRACE && type!=M_EVAL)
			mac_error(np);
		if(type==M_NAMESCAN || type==M_NAMECOUNT)
		{
			mp->shp->last_root = mp->shp->var_tree;
			id = idx = prefix(mp->shp,id);
			stkseek(stkp,offset);
			if(type==M_NAMECOUNT)
			{
				c = namecount(mp,id);
				v = ltos(c);
			}
			else
			{
				dolmax = (int)strlen(id);
				dolg = -1;
				nextname(mp,id,0);
#if 1
				if(nv_open(id,mp->shp->var_tree,NV_NOREF|NV_NOADD|NV_VARNAME|NV_NOFAIL))
					v = id;
				else
#endif
					v = nextname(mp,id,dolmax);
			}
		}
		else if(type==M_SUBNAME)
		{
			if(dolg<0)
			{
				v = nv_getsub(np);
				bysub=1;
			}
			else if(v)
			{
				if(!ap || isastchar(mode))
					v = "0";
				else
					v = nv_getsub(np);
			}
		}
		else if(type==M_EVAL && (np = nv_open(v,mp->shp->var_tree,NV_NOREF|NV_NOADD|NV_VARNAME|NV_NOFAIL)))
		{
			v = nv_getval(np);
			goto skip;
		}
		else
		{
			if(!isastchar(mode))
				c = charlen(v,vsize);
			else if(dolg>0)
			{
#if  SHOPT_FILESCAN
				if(mp->shp->cur_line)
				{
					getdolarg(mp->shp,MAX_ARGN,(int*)0);
					c = mp->shp->offsets[0];
				}
				else
#endif  /* SHOPT_FILESCAN */
				c = mp->shp->st.dolc;
			}
			else if(dolg<0)
				c = array_elem(ap);
			else
				c = (v!=0);
			dolg = dolmax = 0;
			v = ltos(c);
		}
		c = RBRACE;
	}
    skip:
	nulflg = 0;
	if(type && c==':')
	{
		c = fcmbget(&LEN);
		if(isascii(c) &&sh_lexstates[ST_BRACE][c]==S_MOD1 && c!='*' && c!= ':')
			nulflg=1;
		else if(c!='%' && c!='#')
		{
			fcseek(-LEN);
			c = ':';
		}
	}
	if(type)
	{
		if(!isbracechar(c))
		{
			if(!nulflg)
				mac_error(np);
			fcseek(-LEN);
			c = ':';
		}
		if(c!=RBRACE)
		{
			bool newops = sh_lexstates[ST_BRACE][c]==S_MOD2;
			offset = stktell(stkp);
			if(newops && sh_isoption(mp->shp,SH_NOUNSET) && *id && id!=idbuff  && (!np || nv_isnull(np)))
				errormsg(SH_DICT,ERROR_exit(1),e_notset,id);
			if(c==',' || c=='^' || c=='/' ||c==':' || ((!v || (nulflg && *v==0)) ^ (c=='+'||c=='#'||c=='%')))
			{
				int newquote = mp->quote;
				int split = mp->split;
				int quoted = mp->quoted;
				int arith = mp->arith;
				int zeros = mp->zeros;
				int assign = mp->assign;
				if(newops)
				{
					type = fcget();
					if(type=='%' || type=='#')
					{
						d=fcmbget(&LEN);
						fcseek(-LEN);
						if(d=='(')
							type = 0;
					}
					fcseek(-1);
					mp->pattern = 1+(c=='/');
					mp->split = 0;
					mp->quoted = 0;
					mp->assign &= ~1;
					mp->arith = mp->zeros = 0;
					newquote = 0;
				}
				else if(c=='?' || c=='=')
					mp->split = mp->pattern = 0;
				copyto(mp,RBRACE,newquote);
				if(!oldpat)
					mp->patfound = 0;
				mp->pattern = oldpat;
				mp->split = split;
				mp->quoted = quoted;
				mp->arith = arith;
				mp->zeros = zeros;
				mp->assign = assign;
				/* add null byte */
				sfputc(stkp,0);
				if(c!='=')
					stkseek(stkp,stktell(stkp)-1);
			}
			else
			{
				sh_lexskip(lp,RBRACE,0,(!newops&&mp->quote)?ST_QUOTE:ST_NESTED);
				stkseek(stkp,offset);
			}
			argp=stkptr(stkp,offset);
		}
	}
	else
	{
		fcseek(-1);
		c=0;
	}
	if(c==':')  /* ${name:expr1[:expr2]} */
	{
		char *ptr;
		type = (int)sh_strnum(mp->shp,argp,&ptr,1);
		if(isastchar(mode))
		{
			if(id==idbuff)  /* ${@} or ${*} */
			{
				if(type<0 && (type+= dolmax)<0)
					type = 0;
				if(type==0)
					v = special(mp->shp,dolg=0);
#if  SHOPT_FILESCAN
				else if(mp->shp->cur_line)
				{
					v = getdolarg(mp->shp,dolg=type,&vsize);
					if(!v)
						dolmax = type;
				}
#endif  /* SHOPT_FILESCAN */
				else if(type < dolmax)
					v = mp->shp->st.dolv[dolg=type];
				else
					v =  0;
			}
			else if(ap)
			{
				if(type<0)
				{
					if(array_assoc(ap))
						type = -type;
					else
						type += array_maxindex(np);
				}
				if(array_assoc(ap))
				{
					while(type-- >0 && (v=0,nv_nextsub(np)))
						v = nv_getval(np);
				}
				else if(type > 0)
				{
					if(nv_putsub(np,NIL(char*),type,ARRAY_SCAN))
						v = nv_getval(np);
					else
						v = 0;
				}
			}
			else if(type>0)
				v = 0;
			if(!v)
				mp->atmode = 0;
		}
		else if(v)
		{
			vsize = charlen(v,vsize);
			if(type<0 && (type += vsize)<0)
				type = 0;
			if(vsize < type)
				v = 0;
#if SHOPT_MULTIBYTE
			else if(mbwide())
			{
				mbinit();
				for(c=type;c;c--)
					mbchar(v);
				c = ':';
			}
#endif /* SHOPT_MULTIBYTE */
			else
				v += type;
			vsize = v?(int)strlen(v):0;
		}
		if(*ptr==':')
		{
			if((type = (int)sh_strnum(mp->shp,ptr+1,&ptr,1)) <=0)
			{
				v = 0;
				mp->atmode = 0;
			}
			else if(isastchar(mode))
			{
				if(dolg>=0)
				{
					if(dolg+type < dolmax)
						dolmax = dolg+type;
				}
				else
					dolmax = type;
			}
			else if(type < vsize)
			{
#if SHOPT_MULTIBYTE
				if(mbwide())
				{
					char *vp = v;
					mbinit();
					while(type-->0)
					{
						if((c=mbsize(vp))<1)
							c = 1;
						vp += c;
					}
					type = vp-v;
					c = ':';
				}
#endif /* SHOPT_MULTIBYTE */
				vsize = type;
			}
			else
				vsize = v?(int)strlen(v):0;
		}
		if(*ptr)
			mac_error(np);
		stkseek(stkp,offset);
		argp = 0;
	}
	/* check for substring operations */
	else if(sh_lexstates[ST_BRACE][c]==S_MOD2)
	{
		if(c=='/')
		{
			if(type=='/' || type=='#' || type=='%')
			{
				c = type;
				type = '/';
				argp++;
			}
			else
				type = 0;
		}
		else
		{
			if(type==c) /* ## or %% */
				argp++;
			else
				type = 0;
		}
		pattern = strdup(argp);
		if((type=='/' || c=='/') && (repstr = mac_getstring(pattern)))
			replen = (int)strlen(repstr);
		if(v || c=='/' && offset>=0)
			stkseek(stkp,offset);
	}
	/* check for quoted @ */
	if(mode=='@' && mp->quote && !v && c!='-')
		mp->quoted-=2;
retry2:
	if(v && (!nulflg || *v ) && c!='+')
	{
		int match[2*(MATCH_MAX+1)],index;
		int nmatch, nmatch_prev, vsize_last, tsize;
		char *vlast=NULL,*oldv;
		d = (mode=='@'?' ':mp->ifs);
		while(1)
		{
			if(!v)
				v= "";
			if(c=='/' || c=='#' || c== '%')
			{
				flag = (type || c=='/')?(STR_GROUP|STR_MAXIMAL):STR_GROUP;
				if(c!='/')
					flag |= STR_LEFT;
				index = nmatch = 0;
				tsize = (int)strlen(v);
				while(1)
				{
					vsize = tsize;
					oldv = v;
					nmatch_prev = nmatch;
					if(c=='%')
						nmatch=substring(v,tsize,pattern,match,flag&STR_MAXIMAL);
					else
						nmatch=strngrpmatch(v,vsize,pattern,(ssize_t*)match,elementsof(match)/2,flag|STR_INT);
					if(nmatch && repstr && !mp->macsub)
						sh_setmatch(mp->shp,v,vsize,nmatch,match,index++);
					if(nmatch)
					{
						vlast = v;
						vsize_last = vsize;
						vsize = match[0];
					}
					else if(c=='#')
						vsize = 0;
					if(vsize)
						mac_copy(mp,v,vsize);
					if(nmatch && replen>0 && (match[1] || !nmatch_prev))
						mac_substitute(mp,repstr,v,match,nmatch);
					if(nmatch==0)
						v += vsize;
					else
						v += match[1];
					if(*v &&  c=='/' && type)
					{
						/* avoid infinite loop */
						if(nmatch && match[1]==0)
						{
							nmatch = 0;
							mac_copy(mp,v,1);
							v++;
						}
						tsize -= v-oldv;
						continue;
					}
					vsize = -1;
					break;
				}
				if(!mp->macsub && (!repstr || (nmatch==0 && index==0)))
					sh_setmatch(mp->shp,vlast,vsize_last,nmatch,match,index++);
				if(!mp->macsub && index>0 && c=='/' && type)
					sh_setmatch(mp->shp,0,0,nmatch,0,-1);
			}
			if(vsize)
			{
				if(c==',' || c=='^')
					offset = stktell(stkp);
				mac_copy(mp,v,vsize>0?vsize:strlen(v));
				if(c==',' || c=='^')
				{
					v = stkptr(stkp,offset);
					if(type)
					{
						char *sp;
						for(sp=v;*sp;sp++)
							 *sp = (c=='^'?toupper(*sp):tolower(*sp));
					}
					else if(*pattern=='?' || (v && *pattern==*v && pattern[1]==0))
						 *v = (c=='^'?toupper(*v):tolower(*v));
				}
			}
			if(addsub)
			{
				mp->shp->instance++;
				sfprintf(mp->shp->strbuf,"[%s]",nv_getsub(np));
				mp->shp->instance--;
				v = sfstruse(mp->shp->strbuf);
				mac_copy(mp, v, strlen(v));
			}
			if(dolg==0 && dolmax==0)
				 break;
			if(mp->dotdot)
			{
				if(nv_nextsub(np) == 0)
					break;
				if(bysub)
					v = nv_getsub(np);
				else
					v = nv_getval(np);
				if(array_assoc(ap))
				{
					if(strcmp(bysub?v:nv_getsub(np),arrmax)>0)
						break;
				}
				else
				{
					if(nv_aindex(np) > dolmax)
						break;
				}
			}
			else if(dolg>=0)
			{
				if(++dolg >= dolmax)
					break;
#if  SHOPT_FILESCAN
				if(mp->shp->cur_line)
				{
					if(!(v=getdolarg(mp->shp,dolg,&vsize)))
					{
						dolmax = dolg;
						break;
					}
				}
				else
#endif  /* SHOPT_FILESCAN */
				v = mp->shp->st.dolv[dolg];
			}
			else if(!np)
			{
				if(!(v = nextname(mp,id,dolmax)))
					break;
			}
			else
			{
				if(dolmax &&  --dolmax <=0)
				{
					nv_putsub(np,NIL(char*),0,ARRAY_UNDEF);
					break;
				}
				if(ap)
					ap->flags |= ARRAY_SCAN;
				if(nv_nextsub(np) == 0)
					break;
				if(bysub)
					v = nv_getsub(np);
				else
					v = nv_getval(np);
			}
			if(mp->split && (!mp->quote || mode=='@'))
			{
				if(!np)
					mp->pattern = 0;
				endfield(mp,mp->quoted);
				mp->atmode = mode=='@';
				mp->pattern = oldpat;
			}
			else if(d)
			{
				if(mp->sp)
					sfputc(mp->sp,d);
				else
					sfputc(stkp,d);
			}
		}
		if(arrmax)
			free((void*)arrmax);
	}
	else if(argp)
	{
		if(c=='/' && replen>0 && pattern && strmatch("",pattern))
			mac_substitute(mp,repstr,v,0,0);
		if(c=='?')
		{
			if(np)
				id = nv_name(np);
			else if(idnum)
				id = ltos(idnum);
			if(*argp)
			{
				sfputc(stkp,0);
				errormsg(SH_DICT,ERROR_exit(1),"%s: %s",id,argp);
			}
			else if(v)
				errormsg(SH_DICT,ERROR_exit(1),e_nullset,id);
			else
				errormsg(SH_DICT,ERROR_exit(1),e_notset,id);
		}
		else if(c=='=')
		{
			if(np)
			{
				if(mp->shp->subshell)
					np = sh_assignok(np,1);
				nv_putval(np,argp,0);
				v = nv_getval(np);
				nulflg = 0;
				stkseek(stkp,offset);
				goto retry2;
			}
		else
			mac_error(np);
		}
	}
	else if(var && sh_isoption(mp->shp,SH_NOUNSET) && type<=M_TREE && (!np  || nv_isnull(np) || (nv_isarray(np) && !np->nvalue.cp)))
	{
		if(np)
		{
			if(nv_isarray(np))
			{
				sfprintf(mp->shp->strbuf,"%s[%s]\0",nv_name(np),nv_getsub(np));
				id = sfstruse(mp->shp->strbuf);
			}
			else
				id = nv_name(np);
			nv_close(np);
		}
		errormsg(SH_DICT,ERROR_exit(1),e_notset,id);
	}
	if(np)
		nv_close(np);
	if(pattern)
		free(pattern);
	if(idx)
		free(idx);
	return(true);
nosub:
	if(type==M_BRACE && sh_lexstates[ST_NORM][c]==S_BREAK)
	{
		fcseek(-1);
		comsubst(mp,(Shnode_t*)0,2);
		return(true);
	}
	if(type)
		mac_error(np);
	fcseek(-1);
	nv_close(np);
	return(false);
}

/*
 * This routine handles command substitution
 * <type> is 0 for older `...` version
 */
static void comsubst(Mac_t *mp,register Shnode_t* t, volatile int type)
{
	Sfdouble_t		num;
	register int		c;
	register char		*str;
	Sfio_t			*sp;
	Stk_t			*stkp = mp->shp->stk;
	Fcin_t			save;
	struct slnod            *saveslp = mp->shp->st.staklist;
	struct _mac_		savemac;
	int			savtop = stktell(stkp);
	char			*savptr = stkfreeze(stkp,0);
	ssize_t                 len;
	int			was_history = sh_isstate(mp->shp,SH_HISTORY);
	int			was_verbose = sh_isstate(mp->shp,SH_VERBOSE);
	int			was_interactive = sh_isstate(mp->shp,SH_INTERACTIVE);
	int			newlines,bufsize,nextnewlines;
	Sfoff_t			foff;
	Namval_t		*np;
	pid_t			spid;
	mp->shp->argaddr = 0;
	savemac = *mp;
	mp->shp->st.staklist=0;
#ifdef SHOPT_COSHELL
	if(mp->shp->inpool)
		return;
#endif /*SHOPT_COSHELL */
	if(type)
	{
		sp = 0;
		fcseek(-1);
		if(!t)
			t = sh_dolparen((Lex_t*)mp->shp->lex_context);
		if(t && t->tre.tretyp==TARITH)
		{
			mp->shp->inarith = 1;
			fcsave(&save);
			if(t->ar.arcomp)
				num = arith_exec(t->ar.arcomp);
			else if((t->ar.arexpr->argflag&ARG_RAW))
				num = sh_arith(mp->shp,t->ar.arexpr->argval);
			else
				num = sh_arith(mp->shp,sh_mactrim(mp->shp,t->ar.arexpr->argval,3));
			mp->shp->inarith = 0;
		out_offset:
			stkset(stkp,savptr,savtop);
			*mp = savemac;
			if(num && (Sfulong_t)num==num)
				sfprintf(mp->shp->strbuf,"%llu",(Sfulong_t)num);
			else if((Sflong_t)num!=num)
				sfprintf(mp->shp->strbuf,"%.*Lg",LDBL_DIG,num);
			else if(num)
				sfprintf(mp->shp->strbuf,"%lld",(Sflong_t)num);
			else
				sfprintf(mp->shp->strbuf,"%Lg",num);
			str = sfstruse(mp->shp->strbuf);
			mac_copy(mp,str,strlen(str));
			mp->shp->st.staklist = saveslp;
			fcrestore(&save);
			return;
		}
		else if(type==2 && t && (t->tre.tretyp&COMMSK)==0 && t->com.comarg)
		{
			Namval_t *np;
			str = NULL;
			if(!(t->com.comtyp&COMSCAN))
			{
				struct dolnod *ap = (struct dolnod*)t->com.comarg;
				str = ap->dolval[ap->dolbot];
			}
			else if(t->com.comarg->argflag&ARG_RAW)
				str = t->com.comarg->argval;
		}
	}
	else
	{
		while(fcgetc(c)!='`' && c)
		{
			if(c==ESCAPE)
			{
				fcgetc(c);
				if(!(isescchar(sh_lexstates[ST_QUOTE][c]) ||
				  (c=='"' && mp->quote)))
					sfputc(stkp,ESCAPE);
			}
			sfputc(stkp,c);
		}
		sfputc(stkp,'\n');
		c = stktell(stkp);
		str=stkfreeze(stkp,1);
		/* disable verbose and don't save in history file */
		sh_offstate(mp->shp,SH_HISTORY);
		sh_offstate(mp->shp,SH_VERBOSE);
		if(mp->sp)
			sfsync(mp->sp);	/* flush before executing command */
		sp = sfnew(NIL(Sfio_t*),str,c,-1,SF_STRING|SF_READ);
		c = mp->shp->inlineno;
		mp->shp->inlineno = error_info.line+mp->shp->st.firstline;
		t = (Shnode_t*)sh_parse(mp->shp, sp,SH_EOF|SH_NL);
		mp->shp->inlineno = c;
		type = 1;
	}
#if KSHELL
	if(t)
	{
		fcsave(&save);
		if(t->tre.tretyp==0 && !t->com.comarg && !t->com.comset)
		{
			/* special case $(<file) and $(<#file) */
			register int fd;
			int r;
			struct checkpt buff;
			struct ionod *ip=0;
			if(sp)
				sfclose(sp);
			sh_pushcontext(mp->shp,&buff,SH_JMPIO);
			if((ip=t->tre.treio) && 
				((ip->iofile&IOLSEEK) || !(ip->iofile&IOUFD)) &&
				(r=sigsetjmp(buff.buff,0))==0)
				fd = sh_redirect(mp->shp,ip,3);
			else
				fd = sh_chkopen(e_devnull);
			sh_popcontext(mp->shp,&buff);
			if(r==0 && ip && (ip->iofile&IOLSEEK))
			{
				if(sp=mp->shp->sftable[fd])
					num = sftell(sp);
				else
					num = lseek(fd, (off_t)0, SEEK_CUR);
				goto out_offset;
			}
			if(!(sp=mp->shp->sftable[fd]) || (sffileno(sp)!=fd &&!(sfset(sp,0,0)&SF_STRING)))
				sp = sfnew(NIL(Sfio_t*),(char*)malloc(IOBSIZE+1),IOBSIZE,fd,SF_READ|SF_MALLOC);
			type = 3;
		}
		else
			sp = sh_subshell(mp->shp,t,sh_isstate(mp->shp,SH_ERREXIT),type);
		fcrestore(&save);
	}
	else
		sp = sfopen(NIL(Sfio_t*),"","sr");
	sh_freeup(mp->shp);
	mp->shp->st.staklist = saveslp;
	if(was_history)
		sh_onstate(mp->shp,SH_HISTORY);
	if(was_verbose)
		sh_onstate(mp->shp,SH_VERBOSE);
#else
	sp = sfpopen(NIL(Sfio_t*),str,"r");
#endif
	*mp = savemac;
	np = sh_scoped(mp->shp,IFSNOD);
	nv_putval(np,mp->ifsp,NV_RDONLY);
	mp->ifsp = nv_getval(np);
	stkset(stkp,savptr,savtop);
	newlines = 0;
	/* read command substitution output and put on stack or here-doc */
	sfpool(sp, NIL(Sfio_t*), SF_WRITE);
	sfset(sp, SF_WRITE|SF_PUBLIC|SF_SHARE,0);
	sh_offstate(mp->shp,SH_INTERACTIVE);
	if((foff = sfseek(sp,(Sfoff_t)0,SEEK_END)) > 0)
	{
		size_t soff = stktell(stkp); 
		sfseek(sp,(Sfoff_t)0,SEEK_SET);
		stkseek(stkp,soff+foff+64);
		stkseek(stkp,soff);
	}
	if(foff > IOBSIZE)
		sfsetbuf(sp,NULL,SF_UNBOUND);
	spid = mp->shp->spid;
	mp->shp->spid = 0;
	while((str=(char*)sfreserve(sp,SF_UNBOUND,0)) && (c=bufsize=sfvalue(sp))>0)
	{
#if SHOPT_CRNL
		/* eliminate <cr> */
		register char *dp;
		char *buff = str;
		while(c>1 && (*str !='\r'|| str[1]!='\n'))
		{
			c--;
			str++;
		}
		dp = str;
		while(c>1)
		{
			str++;
			c--;
			while(c>1 && (*str!='\r' || str[1]!='\n'))
			{
				c--;
				*dp++ = *str++;
			}
		}
		if(c)
			*dp++ = *str++;
		str = buff;
		c = dp-str;
#endif /* SHOPT_CRNL */
		/* delay appending trailing new-lines */
		for(nextnewlines=0; c>0 && str[c-1]=='\n'; c--, nextnewlines++);
		if(c < 0)
		{
			newlines += nextnewlines;
			continue;
		}
		if(newlines >0)
		{
			if(mp->sp)
				sfnputc(mp->sp,'\n',newlines);
			else if(!mp->quote && mp->split && mp->shp->ifstable['\n'])
				endfield(mp,0);
			else
				sfnputc(stkp,'\n',newlines);
		}
		newlines = nextnewlines;
		mac_copy(mp,str,c);
	}
	if(type==1 && spid)
		job_wait(spid);
	if(was_interactive)
		sh_onstate(mp->shp,SH_INTERACTIVE);
	if(--newlines>0 && mp->shp->ifstable['\n']==S_DELIM)
	{
		if(mp->sp)
			sfnputc(mp->sp,'\n',newlines);
		else if(!mp->quote && mp->split)
			while(newlines--)
				endfield(mp,1);
		else
			sfnputc(stkp,'\n',newlines);
	}
	sfclose(sp);
	return;
}

/*
 * copy <str> onto the stack
 */
static void mac_copy(register Mac_t *mp,register const char *str, register size_t size)
{
	register char		*state;
	register const char	*cp=str;
	register const char	*ep=cp+size;
	register int		c,n,nopat,len;
	Stk_t			*stkp=mp->shp->stk;
	int			oldpat = mp->pattern;
	nopat = (mp->quote||(mp->assign==1)||mp->arith);
	if(size>512)
	{
		/* pre-allocate to improve performance */
		c = stktell(stkp);
		stkseek(stkp,c+size+(size>>4));
		stkseek(stkp,c);
	}
	if(mp->zeros)
	{
		/* prevent leading 0's from becomming octal constants */
		while(size>1 && *str=='0')
		{
			if(str[1]=='x' || str[1]=='X')
				break;
			str++,size--;
		}
		mp->zeros = 0;
		cp = str;
	}
	if(mp->sp)
		sfwrite(mp->sp,str,size);
	else if(mp->pattern>=2 || (mp->pattern && nopat) || mp->assign==3)
	{
		state = sh_lexstates[ST_MACRO];
		/* insert \ before file expansion characters */
		while(size-->0)
		{
#if SHOPT_MULTIBYTE
			if((len=mbnsize(cp,ep-cp))>1)
			{
				cp += len;
				size -= (len-1);
				continue;
			}
#endif
			c = state[n= *(unsigned char*)cp++];
			if(mp->assign==3 && mp->pattern!=4)
			{
				if(c==S_BRACT)
				{
					nopat = 0;
					mp->pattern = 4;
				}
				continue;
			}
			if(nopat&&(c==S_PAT||c==S_ESC||c==S_BRACT||c==S_ENDCH) && mp->pattern!=3)
				c=1;
			else if(mp->pattern==4 && (c==S_ESC||c==S_BRACT||c==S_ENDCH || isastchar(n)))
			{
				if(c==S_ENDCH && oldpat!=4)
				{
					if(*cp==0 || *cp=='.' || *cp=='[')
					{
						mp->pattern = oldpat;
						c=0;
					}
					else
						c=1;
				}
				else
					c=1;
			}
			else if(mp->pattern==2 && c==S_SLASH)
				c=1;
			else if(mp->pattern==3 && c==S_ESC && (state[*(unsigned char*)cp]==S_DIG||(*cp==ESCAPE)))
			{
				if(!(c=mp->quote))
					cp++;
			}
			else
				c=0;
			if(c)
			{
				if(c = (cp-1) - str)
					sfwrite(stkp,str,c);
				sfputc(stkp,ESCAPE);
				str = cp-1;
			}
		}
		if(c = cp-str)
			sfwrite(stkp,str,c);
	}
	else if(!mp->quote && mp->split && (mp->ifs||mp->pattern))
	{
		/* split words at ifs characters */
		state = mp->shp->ifstable;
		if(mp->pattern)
		{
			char *sp = "&|()";
			while(c = *sp++)
			{
				if(state[c]==0)
					state[c] = S_EPAT;
			}
			sp = "*?[{";
			while(c = *sp++)
			{
				if(state[c]==0)
					state[c] = S_PAT;
			}
			if(state[ESCAPE]==0)
				state[ESCAPE] = S_ESC;
		}
		while(size-->0)
		{
			n=state[c= *(unsigned char*)cp++];
#if SHOPT_MULTIBYTE
			if(n!=S_MBYTE && (len=mbnsize(cp-1,ep-cp+1))>1)
			{
				sfwrite(stkp,cp-1, len);
				cp += --len;
				size -= len;
				continue;
			}
#endif
			if(n==S_ESC || n==S_EPAT)
			{
				/* don't allow extended patterns in this case */
				mp->patfound = mp->pattern;
				sfputc(stkp,ESCAPE);
			}
			else if(n==S_PAT)
				mp->patfound = mp->pattern;
			else if(n && mp->ifs)
			{
#if SHOPT_MULTIBYTE
				if(n==S_MBYTE)
				{
					if(sh_strchr(mp->ifsp,cp-1,ep-cp+1)<0)
						continue;
					n = mbnsize(cp-1,ep-cp+1) - 1;
					if(n==-2)
						n = 0;
					cp += n;
					size -= n;
					n= S_DELIM;
				}
#endif /* SHOPT_MULTIBYTE */
				if(n==S_SPACE || n==S_NL)
				{
					while(size>0 && ((n=state[c= *(unsigned char*)cp++])==S_SPACE||n==S_NL))
						size--;
#if SHOPT_MULTIBYTE
					if(n==S_MBYTE && sh_strchr(mp->ifsp,cp-1,ep-cp+1)>=0)
					{
						n = mbnsize(cp-1,ep-cp+1) - 1;
						if(n==-2)
							n = 0;
						cp += n;
						size -= n;
						n=S_DELIM;
					}
					else
#endif /* SHOPT_MULTIBYTE */
					if(n==S_DELIM)
						size--;
				}
				endfield(mp,n==S_DELIM||mp->quoted);
				mp->patfound = 0;
				if(n==S_DELIM)
					while(size>0 && ((n=state[c= *(unsigned char*)cp++])==S_SPACE||n==S_NL))
						size--;
				if(size<=0)
					break;
				cp--;
				continue;

			}
			sfputc(stkp,c);
		}
		if(mp->pattern)
		{
			cp = "&|()";
			while(c = *cp++)
			{
				if(state[c]==S_EPAT)
					state[c] = 0;
			}
			cp = "*?[{";
			while(c = *cp++)
			{
				if(state[c]==S_PAT)
					state[c] = 0;
			}
			if(mp->shp->ifstable[ESCAPE]==S_ESC)
				mp->shp->ifstable[ESCAPE] = 0;
		}
	}
	else if(mp->pattern==1 && mp->maccase)
	{
		while(1)
		{
			c = 0;
			str = cp;
			while(size-->0 && (c= *cp++)!='\\');
			sfwrite(stkp,str,cp-str);
			if(c!='\\')
				break;
			sfputc(stkp,c);
		}
	}
	else
		sfwrite(stkp,str,size);
}

/*
 * Terminate field.
 * If field is null count field if <split> is non-zero
 * Do filename expansion of required
 */
static void endfield(register Mac_t *mp,int split)
{
	register struct argnod	*argp;
	register int		count=0;
	Stk_t			*stkp = mp->shp->stk;
	if(stktell(stkp) > ARGVAL || split)
	{
		argp = (struct argnod*)stkfreeze(stkp,1);
		argp->argnxt.cp = 0;
		argp->argflag = 0;
		mp->atmode = 0;
		if(mp->patfound)
		{
			mp->shp->argaddr = 0;
#if SHOPT_BRACEPAT
			count = path_generate(mp->shp,argp,mp->arghead);
#else
			count = path_expand(mp->shp,argp->argval,mp->arghead);
#endif /* SHOPT_BRACEPAT */
			if(count)
				mp->fields += count;
			else if(split)	/* pattern is null string */
				*argp->argval = 0;
			else	/* pattern expands to nothing */
				count = -1;
		}
		if(count==0)
		{
			argp->argchn.ap = *mp->arghead;
			*mp->arghead = argp;
			mp->fields++;
		}
		if(count>=0)
		{
			(*mp->arghead)->argflag |= ARG_MAKE;
			if(mp->assign || sh_isoption(mp->shp,SH_NOGLOB))
				argp->argflag |= ARG_RAW|ARG_EXP;
		}
		stkseek(stkp,ARGVAL);
	}
	mp->quoted = mp->quote;
}

/*
 * Finds the right substring of STRING using the expression PAT
 * the longest substring is found when FLAG is set.
 */
static int substring(register const char *string,size_t len,const char *pat,int match[], int flag)
{
	register const char *sp=string;
	register int size,nmatch,n;
	int smatch[2*(MATCH_MAX+1)];
	if(flag)
	{
		if(n=strngrpmatch(sp,len,pat,(ssize_t*)smatch,elementsof(smatch)/2,STR_RIGHT|STR_MAXIMAL|STR_INT))
		{
			memcpy(match,smatch,n*2*sizeof(smatch[0]));
			return(n);
		}
		return(0);
	}
	size = (int)len;
	sp += size;
	while(sp>=string)
	{
#if SHOPT_MULTIBYTE
		if(mbwide())
			sp = lastchar(string,sp);
#endif /* SHOPT_MULTIBYTE */
		if(n=strgrpmatch(sp,pat,(ssize_t*)smatch,elementsof(smatch)/2,STR_RIGHT|STR_LEFT|STR_MAXIMAL|STR_INT))
		{
			nmatch = n;
			memcpy(match,smatch,n*2*sizeof(smatch[0]));
			size = sp-string;
			break;
		}
		sp--;
	}
	if(size==len)
		return(0);
	if(nmatch)
	{
		nmatch *=2;
		while(--nmatch>=0)
			match[nmatch] += size;
	}
	return(n);
}

#if SHOPT_MULTIBYTE
	static char	*lastchar(const char *string, const char *endstring)
	{
		register char *str = (char*)string;
		register int c;
		mbinit();
		while(*str)
		{
			if((c=mbsize(str))<0)
				c = 1;
			if(str+c > endstring)
				break;
			str += c;
		}
		return(str);
	}
#endif /* SHOPT_MULTIBYTE */
static int	charlen(const char *string,int len)
{
	if(!string)
		return(0);
#if SHOPT_MULTIBYTE
	if(mbwide())
	{
		register const char *str = string, *strmax=string+len;
		register int n=0;
		mbinit();
		if(len>0)
		{
			while(str<strmax && mbchar(str))
				n++;
		}
		else while(mbchar(str))
			n++;
		return(n);
	}
	else
#endif /* SHOPT_MULTIBYTE */
	{
		if(len<0)
			return(strlen(string));
		return(len);
	}
}

/*
 * This is the default tilde discipline function
 */
static int sh_btilde(int argc, char *argv[], Shbltin_t *context)
{
	Shell_t *shp = context->shp;
	char *cp = sh_tilde(shp,argv[1]);
	NOT_USED(argc);
	if(!cp)
		cp = argv[1];
	sfputr(sfstdout, cp, '\n');
	return(0);
}
 
/*
 * <offset> is byte offset for beginning of tilde string
 */
static void tilde_expand2(Shell_t *shp, register int offset)
{
	char		shtilde[10], *av[3], *ptr=stkfreeze(shp->stk,1);
	Sfio_t		*iop, *save=sfstdout;
	Namval_t	*np;
	static int	beenhere=0;
	strcpy(shtilde,".sh.tilde");
	np = nv_open(shtilde,shp->fun_tree, NV_VARNAME|NV_NOARRAY|NV_NOASSIGN|NV_NOFAIL);
	if(np && !beenhere)
	{
		beenhere = 1;
		sh_addbuiltin(shp,shtilde,sh_btilde,0);
		nv_onattr(np,NV_EXPORT);
	}
	av[0] = ".sh.tilde";
	av[1] = &ptr[offset];
	av[2] = 0;
	iop = sftmp((IOBSIZE>PATH_MAX?IOBSIZE:PATH_MAX)+1);
	sfset(iop,SF_READ,0);
	sfstdout = iop;
	if(np)
		sh_fun(shp,np, (Namval_t*)0, av);
	else
		sh_btilde(2, av, &shp->bltindata);
	sfstdout = save;
	stkset(shp->stk,ptr, offset);
	sfseek(iop,(Sfoff_t)0,SEEK_SET);
	sfset(iop,SF_READ,1);
	if(ptr = sfreserve(iop, SF_UNBOUND, -1))
	{
		Sfoff_t n = sfvalue(iop);
		while(ptr[n-1]=='\n')
			n--;
		if(n==1 && fcpeek(0)=='/' && ptr[n-1])
			n--;
		if(n)
			sfwrite(shp->stk,ptr,n);
	}
	else
		sfputr(shp->stk,av[1],0);
	sfclose(iop);
}

/*
 * This routine is used to resolve ~ expansion.
 * A ~ by itself is replaced with the users login directory.
 * A ~- is replaced by the previous working directory in shell.
 * A ~+ is replaced by the present working directory in shell.
 * If ~name  is replaced with login directory of name.
 * If string doesn't start with ~ or ~... not found then 0 returned.
 */
                                                            
static char *sh_tilde(Shell_t *shp,register const char *string)
{
	register char		*cp;
	register int		c;
	register struct passwd	*pw;
	register Namval_t *np=0;
	static Dt_t *logins_tree;
	if(*string++!='~')
		return(NIL(char*));
	if((c = *string)==0)
	{
		if(!(cp=nv_getval(sh_scoped(shp,HOME))))
			cp = getlogin();
		return(cp);
	}
	if((c=='-' || c=='+') && string[1]==0)
	{
		if(c=='+')
			cp = nv_getval(sh_scoped(shp,PWDNOD));
		else
			cp = nv_getval(sh_scoped(shp,OLDPWDNOD));
		return(cp);
	}
	if(c=='{')
	{
		char		*s2;
		size_t		len;
		int		fd, offset=stktell(shp->stk);
		Spawnvex_t     *vc = (Spawnvex_t*)shp->vex;
		if(!vc && (vc = spawnvex_open(0)))
			shp->vex = (void*)vc;
		if(!(s2=strchr(string++,'}')))
			return(NIL(char*));
		len = s2-string;
		sfwrite(shp->stk,string,len+1);
		s2 = stkptr(shp->stk,offset);
		s2[len] = 0;
		stkseek(shp->stk,offset);
		if(isdigit(*s2))
		{
			fd=strtol(s2, &s2, 10);
			if(*s2)
				return(NIL(char*));
		}
		else
		{
			Namval_t *np=nv_open(s2, shp->var_tree, NV_VARNAME|NV_NOFAIL|NV_NOADD);
			if (!np)
				return(NIL(char*));
			fd = (int)nv_getnum(np);
			nv_close(np);
		}
		if(fd<0)
			return(NIL(char*));
#ifdef _fd_pid_dir_fmt
		sfprintf(shp->stk, _fd_pid_dir_fmt, (long)getpid(), fd,"","");
#else
#   ifdef _fd_self_dir_fmt
		sfprintf(shp->stk,_fd_self_dir_fmt,fd,"","");
#   else
		sfprintf(shp->stk,"/dev/fd/%d", fd);
#   endif
#endif
		if(vc)
			spawnvex_add(vc,fd,fd,0,0);
		return(stkfreeze(shp->stk,1));
	}
#if _WINIX
	if(fcgetc(c)=='/')
	{
		char	*str;
		int	n=0,offset=stktell(shp->stk);
		sfputr(shp->stk,string,-1);
		do
		{
			sfputc(shp->stk,c);
			n++;
		}
		while (fcgetc(c) && c!='/');
		sfputc(shp->stk,0);
		if(c)
			fcseek(-1);
		str = stkseek(shp->stk,offset);
		Skip = n;
		if(logins_tree && (np=nv_search(str,logins_tree,0)))
			return(nv_getval(np));
		if(pw = getpwnam(str))
		{
			string = str;
			goto skip;
		}
		Skip = 0;
	}
#endif /* _WINIX */
	if(logins_tree && (np=nv_search(string,logins_tree,0)))
		return(nv_getval(np));
	if(!(pw = getpwnam(string)))
		return(NIL(char*));
#if _WINIX
skip:
#endif /* _WINIX */
	if(!logins_tree)
	{
		logins_tree = dtopen(&_Nvdisc,Dtbag);
		dtuserdata(logins_tree,shp,1);
	}
	if(np=nv_search(string,logins_tree,NV_ADD))
	{
		c = shp->subshell;
		shp->subshell = 0;
		nv_putval(np, pw->pw_dir,0);
		shp->subshell = c;
	}
	return(pw->pw_dir);
}

/*
 * return values for special macros
 */
static char *special(Shell_t *shp,register int c)
{
	if(c!='$')
		shp->argaddr = 0;
	switch(c)
	{
	    case '@':
	    case '*':
		return(shp->st.dolc>0?shp->st.dolv[1]:NIL(char*));
	    case '#':
#if  SHOPT_FILESCAN
		if(shp->cur_line)
		{
			getdolarg(shp,MAX_ARGN,(int*)0);
			return(ltos(shp->offsets[0]));
		}
#endif  /* SHOPT_FILESCAN */
		return(ltos(shp->st.dolc));
	    case '!':
		if(shp->bckpid)
#if SHOPT_COSHELL
			return(sh_pid2str(shp,shp->bckpid));
#else
			return(ltos(shp->bckpid));
#endif /* SHOPT_COSHELL */
		break;
	    case '$':
		if(nv_isnull(SH_DOLLARNOD))
			return(ltos(shp->gd->pid));
		return(nv_getval(SH_DOLLARNOD));
	    case '-':
		return(sh_argdolminus((void*)shp));
	    case '?':
		return(ltos(shp->savexit));
	    case 0:
		if(sh_isstate(shp,SH_PROFILE) || shp->fn_depth==0 || !shp->st.cmdname)
			return(shp->shname);
		else
			return(shp->st.cmdname);
	}
	return(NIL(char*));
}

/*
 * Handle macro expansion errors
 */
static void mac_error(Namval_t *np)
{
	if(np)
		nv_close(np);
	errormsg(SH_DICT,ERROR_exit(1),e_subst,fcfirst());
}

/*
 * Given pattern/string, replace / with 0 and return pointer to string
 * \ characters are stripped from string.  The \ are stripped in the
 * replacement string unless followed by a digit or \.
 */ 
static char *mac_getstring(char *pattern)
{
	register char	*cp=pattern, *rep=0, *dp;
	register int	c;
	while(c = *cp++)
	{
		if(c==ESCAPE && (!rep || (*cp && strchr("&|()[]*?",*cp))))
		{
			c = *cp++;
		}
		else if(!rep && c=='/')
		{
			cp[-1] = 0;
			rep = dp = cp;
			continue;
		}
		if(rep)
			*dp++ = c;
	}
	if(rep)
		*dp = 0;
	return(rep);
}
