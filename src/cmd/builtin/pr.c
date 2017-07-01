/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * pr
 * Written by David Korn
 * Mon Mar 30 15:07:41 EST 1992
 */

static const char usage[] =
"[-n?\n@(#)$Id: pr (AT&T Research) 2013-09-13 $\n]"
USAGE_LICENSE
"[+NAME?pr - print files]"
"[+DESCRIPTION?\bpr\b formats and prints files to the standard output."
"	If \afile\a is \b-\b or if no files are specified then the"
"	standard input is read.]"

"[a:across?Print columns across rather than down; \b-\b\acolumns\a must be >1.]"
"[c:control?Print control characters in hat (^G) and octal form.]"
"[C:columns|cols?Print \acolumns\a columns of output. Can also be entered"
"	as -\acolumns\a. May not be used with \b-m\b.]#[columns:=1]"
"[d:double-space?Double space the output.]"
"[e:expand?Expand \achar\a to \awidth\a.]:?[char[width]]:=\\t8]"
"[f:formfeed?Use formfeeds instead of newlines to separate pages with"
"	a 3 line page header.]"
"[F:bigformfeed?Use formfeeds instead of newlines to separate pages with"
"	a 5 line page header and trailer.]"
"[h:header?Use \aheader\a instead of the file name in header text.]:[header]"
"[i:replace?Replace spaces with \achar\as to tab \awidth\a.]:?"
"	[char[width]]:=\\t8]"
"[j:join?Merge full lines.]"
"[l:lines?The page length in lines. 10 lines are reserved for the header"
"	and trailer.]#[lines:=66]"
"[m:multiple?Print the input files one per column, truncating lines to fit.]"
"[n:number?Number lines with \adigits\a digits followed by \asep\a.]:?"
"	[sep[digits]]:=\\t5]"
"[o:indent|margin|offset?Indent each line with \aindent\a spaces.]#[indent:=0]"
"[p:pause?Pause before printing each page if the output is to a terminal."
"	\apr\a prinst a BEL on the terminal and reads one line of input"
"	before continuing. (Not implemented.)]"
"[P:page?Start printing with page \apage\a. Can also be entered as +\apage\a.]"
"	#[page:=1]"
"[r!:warn?Warn about unreadable input files.]"
"[s:separate?Separate columns with \astring\a.]:?[string]"
"[t|T!:headers?Generate headers and trailers and pass form feeds in the input"
"	to the output.]"
"[w:width?The page width in characters.]#[width:=72]"
"[v:literal?Print control characters in C-style form.]"
"[X:test?Canonicalize output for testing.]"

"\n"
"\n[ file ... ]\n"
"\n"
"[+SEE ALSO?\bcat\b(1), \bfold\b(1), \bless\b(1), \bmore\b(1)]"
;

#include	<cmd.h>
#include	<ctype.h>
#include	<ccode.h>
#include	<ls.h>
#include	<tm.h>

#define	A_FLAG	0x0001
#define	D_FLAG	0x0002
#define	F_FLAG	0x0004
#define	M_FLAG	0x0008
#define	R_FLAG	0x0010
#define	T_FLAG	0x0020
#define	P_FLAG	0x0040
#define	X_FLAG	0x0080

#define S_NL	1
#define S_TAB	2
#define S_SPACE	4

#define DATESZ	50
#define SWIDTH	512
#define WIDTH	72
#define NWIDTH	5
#define HDRSZ	5
#define TABSZ	8
#define PAGESZ	66

typedef struct _pr_
{
	Sfdisc_t disc; /* first! */
	char	state[1<<CHAR_BIT];
	Sfio_t	*outfile;
	Sfio_t	*infile;
	Sfio_t	**streams;
	long	pageno;
	long	pageskip;
	long	lineno;
	int	pagelen;
	int	offset;
	int	offlen;
	int	numwidth;
	int	columns;
	int	colno;
	int	width;
	int	flags;
	int	nopen;
	int	pageodd;
	int	igap, ogap;
	int	itab, otab;
	int	imod, omod;
	int	nchar, schar;
	char	**fieldlist;
	char	*fieldbuff;
	char	*fieldptr;
	char	**fieldlast;
	char	*header;
	char	*margin;
	char	*filename;
	char	*date;
	struct
	{
	Sfdisc_t*disc;
	char	*buf;
	char	*cur;
	char	*end;
	size_t	siz;
	unsigned char *map;
	}	control;
	Mbstate_t q;
} Pr_t;

/*
 * allocate and initialize pr global data
 */
static Pr_t *prinit(void)
{
	register Pr_t *pp;
	if(!(pp = (Pr_t*)stakalloc(DATESZ+sizeof(Pr_t))))
		return 0;
	pp->columns = 1;
	pp->pageskip = pp->pageno = 0;
	pp->pagelen = PAGESZ;
	pp->numwidth = 0;
	pp->flags = 0;
	pp->igap = pp->ogap = 0;
	pp->nchar = pp->itab = pp->otab= '\t';
	pp->schar = 0;
	pp->numwidth = pp->width = pp->offset = 0;
	pp->header = 0;
	pp->date = (char*)(pp+1);
	memset(pp->state, 0, 1<<CHAR_BIT);
	return pp;
}

/*
 * print the page header
 */
static int prheader(register Pr_t *pp)
{
	char *header = "";
	if(pp->header)
		header = pp->header;
	else if(!(pp->flags&M_FLAG) && pp->filename)
		header = pp->filename;
	sfputc(pp->outfile,'\n');
	sfputc(pp->outfile,'\n');
	if(pp->offset)
		sfwrite(pp->outfile,pp->margin,pp->offlen);
	if(sfprintf(pp->outfile,"%s  %s Page %d\n\n\n",pp->date,header,pp->pageno)<0)
		return -1;
	return 0;
}

/*
 * print the page trailer
 */
static int prtrailer(register Pr_t *pp,int line)
{
	register int n= pp->pagelen-line;
	if(pp->flags&D_FLAG)
	{
		if(n==0)
			n = !pp->pageodd;
		else
			n = 2*n-pp->pageodd;
	}
	if(pp->flags&F_FLAG)
	{
		if(sfputc(pp->outfile,'\f') < 0)
			return -1;
	}
	else
	{
		if(sfnputc(pp->outfile, '\n',n+HDRSZ) < 0)
			return -1;
	}
	return 0;
}

/*
 * fastest version, process a page at a time
 */
static void prpage(register Pr_t *pp)
{
	register int n, tflag = !(pp->flags&T_FLAG);
	register Sfio_t *out = 0;
	while(1)
	{
		if(pp->pageno++ >= pp->pageskip)
			out = pp->outfile;
		if(out && tflag)
			prheader(pp);
		if((n=sfmove(pp->infile,out,pp->pagelen,'\n')) != pp->pagelen)
			break;
		if(out && tflag)
		{
			if(!sfreserve(pp->infile,0,0)||sfvalue(pp->infile)<0)
				break;
			sfnputc(pp->outfile,'\n',HDRSZ);
		}
	}
	if(out && tflag)
		prtrailer(pp,n);
}

/*
 *  write <space> space characters into output buffer using
 *  tab characters where appropriae.
 *  <col> is the number of columns until the next tab position
 *  The number of characters written is returned
 */
static int outspaces(register Pr_t* pp,register int spaces, register int col)
{
	register int n=0;
	if(pp->ogap)
	{
		/* changes spaces  <pp->otab> */
		while(spaces >= col)
		{
			if(sfputc(pp->outfile,pp->otab) < 0)
				return -1;
			n++;
			spaces -= col;
			col = pp->ogap;
		}
	}
	if(spaces>0) 
		if(sfnputc(pp->outfile,' ',spaces) < 0)
			return -1;
	return n+=spaces;
}

/*
 * Output one column which could be the complete line
 * Do tab canonicalization if necessary
 * <spaces> give the number of spaces that precede the field
 * This routine returns the number of spaces at end of field
 */
static int outcol(register Pr_t *pp,char *buff, register int size, int spaces)
{
	register char *state=pp->state;
	register int n=S_NL,col=0;
	register char *cp, *buffend;
	register int omod=0;
	if(pp->igap || pp->ogap)
	{
		cp = buff;
		buffend = cp+size;
		if(size=spaces)
		{
			if(pp->ogap)
				omod = pp->ogap - (pp->colno-size)%pp->ogap;
			if(n = state[*(unsigned char*)cp++])
				goto skip;
			outspaces(pp,size,omod);
		}
		while(1)
		{
			/* skip over regular characters */
			while((n=state[*(unsigned char*)cp++])==0);
			size= cp-buff;
			col += (size-1);
			if(pp->width && col >= pp->width)
			{
				size -= (col+1-pp->width);
				if(buffend[-1]=='\n')
				{
					buff[size-1] = '\n';
					break;
				}
			}
			if(n==S_NL)
				break;
			if(sfwrite(pp->outfile,buff,--size)< 0)
				return -1;
			if(pp->width && col >= pp->width)
				return 1;
			size = 0;
			if(pp->ogap)
				omod = pp->ogap - (pp->colno+col)%pp->ogap;
		skip:
			while(n>S_NL && cp < buffend)
			{
				if(n==S_SPACE)
					n = 1;
				else
					n= (pp->igap-(col%pp->igap));
				size += n;
				col += n;
				n = state[*(unsigned char*)cp++];
			}
			if(n==S_NL)
			{
				/* delete trailing white-space */
				sfputc(pp->outfile,'\n');
				return 0;
				
			}
			if(cp>=buffend || (pp->width && col>=pp->width))
			{
				/* return spaces needed to complete field */
				if(col>=pp->width)
				{
					if(buffend[-1]=='\n')
					{
						sfputc(pp->outfile,'\n');
						return 0;
					}
					size -= (col-pp->width);
				}
				else
					size += pp->width-col;
				return size;
			}
			outspaces(pp,size,omod);
			size = 0;
			buff = cp-1;
		}
	}
	if(size>0 && sfwrite(pp->outfile,buff,size)< 0)
		return -1;
	return 0;
}

/*
 * print a line at a time
 */
static int prline(register Pr_t *pp)
{
	register int n, line = pp->pagelen;
	register char *cp;
	pp->colno = pp->offset;
	if(pp->pageskip > 0)
	{
		n = pp->pageskip*pp->pagelen;
		if(sfmove(pp->infile, NiL, n, '\n')!=n)
			return 0;
	}
	while(cp = sfgetr(pp->infile,'\n',0))
	{
		n = sfvalue(pp->infile);
		if(line >= pp->pagelen)
		{
			line = 0;
			pp->pageno++;
			if(!(pp->flags&T_FLAG) &&  prheader(pp)<0)
				return -1;
		}
		if(pp->offset)
			sfwrite(pp->outfile,pp->margin,pp->offlen);
		if(pp->numwidth)
			sfprintf(pp->outfile,"%*d%c",pp->numwidth,++pp->lineno,pp->nchar);
		if(outcol(pp,cp,n,0) < 0)
			return -1;
		if(++line >= pp->pagelen && !(pp->flags&T_FLAG))
		{
			if(pp->flags&F_FLAG)
				sfputc(pp->outfile,'\f');
			else
				sfnputc(pp->outfile, '\n',HDRSZ);
		}
		else if(pp->flags&D_FLAG)
			sfputc(pp->outfile,'\n');
	}
	if(!(pp->flags&T_FLAG))
		prtrailer(pp,line);
	return 0;
}

/*
 * output page containing <n> fields
 */
static int outpage(register Pr_t *pp,int n)
{
	register char *cp,**next = pp->fieldlist;
	register int line=0, incr, size,j, c, old;
	if(pp->pageno++ < pp->pageskip)
	{
		pp->lineno += pp->pagelen;
		return 0;
	}
	if(!(pp->flags&T_FLAG))
		prheader(pp);
	if(pp->flags&A_FLAG)
		incr = 1;
	else
		incr = (n+pp->columns-1)/pp->columns;
	while(1)
	{
		/* print out a line */
		if(!(pp->flags&A_FLAG))
		{
			if(line>=incr)
				break;
			next = pp->fieldlist + line;
		}
		else if(next >= pp->fieldlast)
			break;
		if(pp->offset)
			sfwrite(pp->outfile,pp->margin,pp->offlen);
		if(pp->numwidth)
			sfprintf(pp->outfile,"%*d%c",pp->numwidth,++pp->lineno,pp->nchar);
		pp->colno = pp->offset;
		for(old=0,j=pp->columns; --j>=0; )
		{
			if(next+incr >= pp->fieldlast)
				j=0;
			cp = *next;
			size = next[1]-cp;
			if(j==0)
				c = '\n';
			else
				c = ' ';
			cp[size-1] = c;
			if((old=outcol(pp,cp,size,old))<0)
				return -1;
			pp->colno += pp->width;
			if(j>0 && pp->schar)
			{
				sfputc(pp->outfile,pp->schar);
				pp->colno = old;
				if(pp->schar==pp->otab && pp->ogap)
					pp->colno += pp->ogap - pp->colno%pp->ogap;
				else
					pp->colno++;
				old = 0;
			}
			next += incr;
		}
		if(++line >= pp->pagelen)
			break;
		if(pp->flags&D_FLAG)
			sfputc(pp->outfile,'\n');
	}
	if(!(pp->flags&T_FLAG))
		prtrailer(pp,line);
	return 0;
}

/*
 * This routine processes multi-column and merged files
 * The data is put into in array and printed when the page fills.
 */
static int prcol(register Pr_t *pp)
{
	register char *cp, **next, **nextmax;
	register int n=0, size;
	register Sfio_t *fp=pp->infile;
	int nstream,r=0, skip=pp->pageskip;
	nextmax = pp->fieldlist + pp->columns*pp->pagelen;
	do
	{
		next = pp->fieldlist;
		*next = pp->fieldptr = pp->fieldbuff;
		if(pp->streams)
		{
			nstream = 0;
			fp = *pp->streams;
		}
		while(1)
		{
			if(fp && (cp = sfgetr(fp,'\n',0)))
			{
				if((size=sfvalue(fp)) > pp->width)
					size = pp->width;
				if(!skip)
					memcpy(pp->fieldptr,cp,size);
			}
			else if(pp->streams)
			{
				if(fp)
				{
					if(fp!=sfstdin)
						sfclose(fp);
					pp->streams[nstream] = 0;
					if(--pp->nopen<=0)
					{
						next -= (pp->columns-1);
						break;
					}
				}
				size=1;
			}
			else
				break;
			pp->fieldptr +=size;
			*++next = pp->fieldptr;
			if(next >= nextmax)
				break;
			if(pp->streams)
			{
				if(++nstream>=pp->columns)
					nstream = 0;
				fp = pp->streams[nstream];
			}
		}
		if(skip>0)
		{
			skip--;
			break;
		}
		pp->fieldlast = next;
		if((n = next - pp->fieldlist) > 0)
			r=outpage(pp,n);
	}
	while(cp);
	return r;
}

static ssize_t
c_read(Sfio_t* fp, void* buf, size_t n, Sfdisc_t* dp)
{
	register Pr_t*		pp = (Pr_t*)dp;
	register char*		s = (char*)buf;
	register char*		e = s + n;
	register int		c;
	ssize_t			z;

	static const char	hat[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

	if (pp->control.cur >= pp->control.end)
	{
		if (n > pp->control.siz)
		{
			pp->control.siz = roundof(n, 1024);
			if (!(pp->control.buf = newof(pp->control.buf, char, pp->control.siz, 0)))
				error(ERROR_exit(1), "out of space");
		}
		if ((z = sfrd(fp, pp->control.buf, n, dp)) <= 0)
			return z;
		pp->control.cur = pp->control.buf;
		pp->control.end = pp->control.buf + z;
	}
	while (pp->control.cur < pp->control.end && s < e)
	{
		if ((c = mbsize(pp->control.cur, e - s, &pp->q)) > 1)
		{
			if (c > (e - s))
				break;
			while (c--)
				*s++ = *pp->control.cur++;
		}
		else
		{
			c = ccmapchr(pp->control.map, *pp->control.cur);
			if (c > 040 || c == 011 || c == 012)
				*s++ = *pp->control.cur++;
			else if ((e - s) < 2)
				break;
			else
			{
				pp->control.cur++;
				*s++ = '^';
				*s++ = hat[c];
			}
		}
	}
	return s - (char*)buf;
}

static ssize_t
v_read(Sfio_t* fp, void* buf, size_t n, Sfdisc_t* dp)
{
	register Pr_t*	pp = (Pr_t*)dp;
	register char*	s = (char*)buf;
	register char*	e = s + n;
	register char*	t;
	register int	c;
	ssize_t		z;

	if (pp->control.cur >= pp->control.end)
	{
		if (n > pp->control.siz)
		{
			pp->control.siz = roundof(n, 1024);
			if (!(pp->control.buf = newof(pp->control.buf, char, pp->control.siz, 0)))
				error(ERROR_exit(1), "out of space");
		}
		if ((z = sfrd(fp, pp->control.buf, n, dp)) <= 0)
			return z;
		pp->control.cur = pp->control.buf;
		pp->control.end = pp->control.buf + z;
	}
	while (pp->control.cur < pp->control.end && s < e)
	{
		if ((c = mbsize(pp->control.cur, e - s, &pp->q)) > 1)
		{
			if (c > (e - s))
				break;
			while (c--)
				*s++ = *pp->control.cur++;
		}
		else
		{
			c = *pp->control.cur++;
			if ((iscntrl(c) || !isprint(c)) && c != '\t' && c != '\n' || c == '\\')
			{
				t = fmtquote(pp->control.cur - 1, NiL, NiL, 1, 0);
				if (strlen(t) > (e - s))
				{
					pp->control.cur--;
					break;
				}
				while (*s = *t++)
					s++;
			}
			else
				*s++ = c;
		}
	}
	return s - (char*)buf;
}

int
b_pr(int argc, char** argv, Shbltin_t* context)
{
	register int n;
	register Pr_t *pp;
	register char *cp;
	register Sfio_t *fp;
	struct stat statb;

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	pp = prinit();
	for (;;)
	{
		switch (n = optget(argv, usage))
		{
		case 0:
			break;
		case 'a':
			pp->flags |= A_FLAG;
			continue;
		case 'c':
			pp->control.disc = &pp->disc;
			pp->control.disc->readf = c_read;
			pp->control.map = ccmap(CC_NATIVE, CC_ASCII);
			continue;
		case 'd':
			pp->flags |= D_FLAG;
			continue;
		case 'f':
			pp->flags |= (P_FLAG|F_FLAG);
			continue;
		case 'F':
			pp->flags |= F_FLAG;
			continue;
		case 'm':
			pp->flags |= M_FLAG;
			continue;
		case 'p':
			pp->flags |= P_FLAG;
			continue;
		case 'r':
			pp->flags |= R_FLAG;
			continue;
		case 't':
		case 'T':
			pp->flags |= T_FLAG;
			continue;
		case 'C':
			if(opt_info.option[0]=='-')
				pp->columns = opt_info.num;
			else
				pp->pageskip = opt_info.num-1;
			continue;
		case 'h':
			pp->header = opt_info.arg;
			continue;
		case 'w':
			pp->width = opt_info.num;
			continue;
		case 'l':
			pp->pagelen = opt_info.num;
			continue;
		case 'o':
			pp->offset = opt_info.num;
			continue;
		case 'P':
			pp->pageskip = opt_info.num-1;
			continue;
		case 'n':
		case 'i':
		case 'e':
			if(opt_info.arg && opt_info.arg==argv[opt_info.index-1] && (!(cp=argv[opt_info.index]) || *cp!='-'))
			{
				/* space allowed only if next argument
				 * begins with - */
				opt_info.index--;
				opt_info.arg = 0;
			}
			if(n=='e')
				pp->igap = TABSZ;
			else if(n=='i')
				pp->ogap = TABSZ;
			else
				pp->numwidth = NWIDTH;
			if(opt_info.arg)
			{
				if(!isdigit(*opt_info.arg))
				{
					if(n=='e')
						pp->itab = *opt_info.arg++;
					else if(n=='i')
						pp->otab = *opt_info.arg++;
					else
						pp->nchar = *opt_info.arg++;
				}
				if(*opt_info.arg)
				{
					long l = strtol(opt_info.arg,&opt_info.arg,10);
					if(l<0 || *opt_info.arg)
						error(2,"-%c: requires positive integer",n);
					if(n=='e')
						pp->igap = l;
					else if(n=='i')
						pp->ogap = l;
					else
						pp->numwidth = l;
				}
			}
			continue;
		case 's':
			if(opt_info.arg && opt_info.arg==argv[opt_info.index-1] && (!(cp=argv[opt_info.index]) || *cp!='-'))
			{
				/* space allowed only if next argument
				 * begins with - */
				opt_info.index--;
				opt_info.arg = 0;
			}
			if(opt_info.arg)
				pp->schar = *opt_info.arg;
			else
				pp->schar = '\t';
			continue;
		case 'v':
			pp->control.disc = &pp->disc;
			pp->control.disc->readf = v_read;
			continue;
		case 'X':
			pp->flags |= X_FLAG;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		default:
			error(2, "-%c: not implemented", n);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if(error_info.errors)
		error(ERROR_usage(2), "%s", optusage(NiL));
	pp->outfile = sfstdout;
	if(pp->pagelen<=(2*HDRSZ))
		pp->flags |= T_FLAG;
	if(!(pp->flags&T_FLAG))
		pp->pagelen -= (2*HDRSZ);
	pp->pageodd = (pp->pagelen&1);
	if(pp->flags&D_FLAG)
		pp->pagelen = (pp->pagelen+1)/2;
	if(pp->flags&M_FLAG)
	{
		pp->columns = argc-opt_info.index;
		pp->flags |= A_FLAG;
	}
#if 0
	if(pp->width)
		pp->width -= pp->offset;
#endif
	if(pp->columns > 0)
	{
		if(pp->columns==1)
		{
			pp->columns = 0;
			pp->flags &= ~(M_FLAG|A_FLAG);
		}
		else
		{
			if(pp->igap==0)
				pp->igap = TABSZ;
			if(pp->ogap==0)
				pp->ogap = TABSZ;
			if((n=pp->width)==0)
#if 0
				n = (pp->schar?SWIDTH:(WIDTH-pp->offset));
#else
				n = (pp->schar?SWIDTH:WIDTH);
#endif
			n -= pp->numwidth;
			if((pp->width = ((n+1)/pp->columns)) <=0)
				error(ERROR_exit(1),"width too small");
			n = (pp->pagelen*pp->columns+1);
			if(!(pp->fieldlist = (char**)stakalloc(n*(sizeof(char*)+pp->width))))
				error(ERROR_exit(1),"not enough memory");
			pp->fieldbuff = pp->fieldptr = (char*)(pp->fieldlist+n);
		}
	}
	if(pp->igap)
		pp->state[pp->itab] = S_TAB;
	pp->state[' '] |= S_SPACE;
	pp->state['\n'] = S_NL;
	if(pp->ogap)
		pp->omod = (pp->numwidth+pp->numwidth>0)%pp->ogap;
	if(pp->offset)
	{
		/* pre-compute leading margin */
		if(pp->ogap)
		{
			n = pp->offset/pp->ogap; 
			pp->offlen = pp->offset - n*(pp->ogap-1);
		}
		else
			n =  0;
		if(!(pp->margin = (char*)stakalloc(pp->offset)))
			error(ERROR_exit(1),"not enough memory");
		for(cp=pp->margin; cp < pp->margin+pp->offlen; cp++)
		{
			if(n-- > 0)
				*cp = pp->otab;
			else
				*cp = ' ';
		}
	}
	if(cp = *argv)
		argv++;
	do
	{
		pp->filename = cp;
		if(!cp || strcmp(cp,"-")==0)
			fp = sfstdin;
		else if(!(fp = sfopen(NiL,cp,"r")))
		{
			if(!(pp->flags&R_FLAG))
				error(ERROR_system(0),"%s: cannot open",cp);
			error_info.errors = 1;
			continue;
		}
		if (pp->control.disc && !sfdisc(fp, pp->control.disc))
			error(2, "cannot push control character discipline");
		if(pp->streams)
			pp->streams[n++] = fp;
		else
		{
			if(!(pp->flags&T_FLAG))
			{
				if(pp->flags&X_FLAG)
					strcpy(pp->date, "- Date/Time --");
				else
					tmfmt(pp->date, DATESZ, "%b %e %H:%M %Y", fstat(sffileno(fp), &statb) ? (time_t*)0 : &statb.st_mtime);
			}
			pp->lineno = pp->pageno = 0;
			pp->infile = fp;
			if(pp->flags&M_FLAG)
			{
				pp->streams = (Sfio_t**)stakalloc(pp->columns*sizeof(Sfio_t*));
				*pp->streams = fp;
				n=1;
				continue;
			}
			if(pp->columns)
				prcol(pp);
			else if(pp->width || pp->igap || pp->ogap || pp->offset || pp->numwidth || (pp->flags&D_FLAG))
				prline(pp);
			else
				prpage(pp);
			if(fp!=sfstdin)
				sfclose(fp);
		}
	}
	while(cp= *argv++);
	if(pp->streams)
	{
		pp->nopen = n;
		prcol(pp);
	}
	if (pp->control.buf)
		free(pp->control.buf);
	return error_info.errors != 0;
}
