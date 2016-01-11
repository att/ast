/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2012 AT&T Intellectual Property          *
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
 * nl.c
 * Written by David Korn
 * AT&T Labs
 * Mon Mar 24 10:10:10 EST 2003
 */


static const char usage[] =
"[-?\n@(#)$Id: nl (AT&T Research) 2003-03-24 $\n]"
USAGE_LICENSE
"[+NAME? nl - line numbering filter ]"
"[+DESCRIPTION?\bnl\b reads lines from the file \afile\a or from standard "
	"input if \afile\a is omitted or is \b-\b, and writes the lines to "
	"standard output with possible line numbering preceding each line.]"
"[+?The \bnl\b utility treats its input in terms of logical pages and resets "
	"numbering on each logical page.  A logical page consists of a header, "
	"a body, and a footer any of which can be empty, and each can use "
	"their own line numbering options.]" 
"[+?The start of logical page sections consist of input lines containing only "
	"the two delimiter characters whose defaults are \b\\\b and \b:\b as "
		"follows:]{"
		"[d1d2d1d2d1d2?Header.]"
		"[d1d2d1d2?Body.]"
		"[d1d2?Footer.]"
	"}"
"[+?\bnl\b assumes that the first section is a logical body until it encounters "
	"one of the section delimiters.]"
""
"[b:body-numbering]:[type:=t?\atype\a can be one of the following:]{"
		"[+a?Number all lines.]"
		"[+t?Number only non-empty lines.]"
		"[+n?No line numbering.]"
		"[+p\astring\a?Number only lines that contain the basic "
			"regular expression \astring\a.]"
	"}"
"[d:section-delimiter]:[delim:=\\::?\adelim\a specifies the two delimiter "
	"characters that indicate start of a section.  If only one character "
	"is specified, the second character remains unchanged.]"
"[f:footer-numbering]:[type:=n?\atype\a is the same as for the \bb\b option.]"
"[h:header-numbering]:[type:=n?\atype\a is the same as for the \bb\b option.]"
"[i:page-increment]#[incr:=1?\aincr\a is the increment used to number logical "
	"page lines.]"
"[l:join-blank-lines]#[num:=1?\anum\a is the number of blank lines to be "
	"treated as one]"
"[n:number-format]:[format?\aformat\a specifies the line numbering format.  "
	"It must be one of the following:]{"
		"[101:ln?left justified, leading zeros supressed.]"
		"[102:rn?right justified, leading zeros supressed.]"
		"[103:rz?right justified, leading zeros kept.]"
	"}"
"[p:no-renumber?Start renumbering at logical page delimiters.]"
"[s:number-separator]:[sep:=\\t?\asep\a is the string that is used to separate the line number "
	"and the corresponding text line.]"
"[v:starting-line-number]#[startnum:=1?\astartnum\a is the initial value to number "
	"logical pages.]"
"[w:number-width]#[width:=6?\awidth\a is the number of characters to be used "
	"for line numbering.]"
	
"\n"
"\n[file]\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Success.]"
        "[+>0?An error occurred.]"
"}"
"[+SEE ALSO?\bpr\b(1), \bregex\b(3)]"
;

#include	<cmd.h>
#include	<regex.h>

typedef struct _nl_
{
	void	*section[3];
	char	*sep;
	int	delim1;
	int	delim2;
	int	format;
	int	startnum;
	int	incr;
	int	width;
	int	blines;
	int	pflag;
} Nl_t;

static const char letter_a, letter_t, letter_n;
#define TYPE_ALL	((void*)(&letter_a))
#define TYPE_NONE	((void*)(&letter_n))
#define TYPE_TEXT	((void*)(&letter_t))


#define SECTION_HEAD	0
#define SECTION_BODY	1
#define SECTION_FOOT	2

/* These numbers need to be the same as with -n option in option string */
#define FORMAT_LN	-101
#define FORMAT_RN	-102
#define FORMAT_RZ	-103

static int donl(Nl_t *pp, Sfio_t *in, Sfio_t *out)
{
	register char	*cp;
	register int	n,line=pp->startnum, outline, sectnum=SECTION_BODY;
	int		blank=0, width=pp->width+strlen(pp->sep);
	char		format[20];
	if(pp->format==FORMAT_LN)
		sfsprintf(format,sizeof(format),"%%-%dd%%s",pp->width);
	else if(pp->format==FORMAT_RN)
		sfsprintf(format,sizeof(format),"%%%dd%%s",pp->width);
	else
		sfsprintf(format,sizeof(format),"%%0%dd%%s",pp->width);
	while(cp=sfgetr(in, '\n', 0))
	{
		outline = 0;
		if(*cp!='\n')
			blank = 0;
		else
			blank++;
		n = sfvalue(in);
		if(*cp==pp->delim1 && cp[1]==pp->delim2)
		{
			if(cp[2]=='\n')
			{
				sectnum = SECTION_FOOT;
				sfputc(out,'\n');
				continue;
			}
			else if(cp[2]==pp->delim1 && cp[3]==pp->delim2)
			{
				if(cp[4]=='\n')
				{
					sectnum = SECTION_BODY;
					sfputc(out,'\n');
					continue;
				}
				if(cp[4]==pp->delim1 && cp[5]==pp->delim2 && cp[6]=='\n')
				{
					sectnum = SECTION_HEAD;
					if(!pp->pflag)
						line = pp->startnum;
					sfputc(out,'\n');
					continue;
				}
			}
		}
		if(pp->section[sectnum]==TYPE_NONE)
			;
#if 0
		{
			sfwrite(out, cp, n);
			continue;
		}
#endif
		else if(pp->section[sectnum]==TYPE_ALL)
		{
			if(!blank || blank==pp->blines)
			{
				outline = 1;
				blank = 0;
			}
		}
		else if(pp->section[sectnum]!=TYPE_TEXT)
			outline = !regnexec((regex_t*)pp->section[sectnum], cp, n, (size_t)0, NULL, 0);
		else if(*cp!='\n')
			outline = 1;
		if(outline)
		{
			blank = 0;
			sfprintf(out, format, line, pp->sep);
			line += pp->incr;
		}
		else
			sfnputc(out,' ',width);
		sfwrite(out, cp, n);
	}
	return(0);
}

int
b_nl(int argc, char** argv, Shbltin_t* context)
{
	register int	n,m;
	Sfio_t		*in=sfstdin;
	Nl_t		nl;
	regex_t		re[3];

	cmdinit(argc, argv, context, (const char*)0, 0);
	nl.width =  6;
	nl.startnum = 1;
	nl.blines = 1;
	nl.incr = 1;
	nl.delim1 = '\\';
	nl.delim2 = ':';
	nl.section[SECTION_BODY] = TYPE_TEXT;
	nl.section[SECTION_HEAD] = TYPE_NONE;
	nl.section[SECTION_FOOT] = TYPE_NONE;
	nl.format = FORMAT_RN;
	nl.sep = "\t";
	nl.pflag = 0;

	while (n = optget(argv, usage)) switch (n)
	{
	    case 'p':
		nl.pflag |= 1;
		break;
	    case 'd':
		nl.delim1 = *opt_info.arg;
		if(opt_info.arg[1])
			nl.delim2 = opt_info.arg[1];
		break;
	    case 'f': case 'b': case 'h':
		if(n=='h')
			m = SECTION_HEAD;
		else if(n=='b')
			m = SECTION_BODY;
		else
			m = SECTION_FOOT;
		switch(*opt_info.arg)
		{
		    case 'a':
			nl.section[m] = TYPE_ALL;
			break;
		    case 't':
			nl.section[m] = TYPE_TEXT;
			break;
		    case 'n':
			nl.section[m] = TYPE_NONE;
			break;
		    case 'p':
			nl.section[m] = &re[m];
			if(regcomp(&re[m], opt_info.arg+1, REG_NOSUB))
				error(2, "-%c: invalid basic regular expression %s", n, opt_info.arg+1);
			break;
		}
		break;
	    case 'i':
		if((nl.incr=opt_info.num) < 0)
			error(2, "-%c: option requires non-negative number", n);
		break;
	    case 'l':
		if((nl.blines=opt_info.num) < 0)
			error(2, "-%c: option requires non-negative number", n);
		break;
	    case 'n':
		nl.format = opt_info.num;
		break;
	    case 's':
		nl.sep = opt_info.arg;
		break;
	    case 'v':
		nl.startnum = opt_info.num;
		break;
	    case 'w':
		if((nl.width=opt_info.num) < 0)
			error(2, "-%c: option requires non-negative number", n);
		break;
	    case ':':
		error(2, "%s", opt_info.arg);
		break;
	    case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if(argc>1 || error_info.errors)
		error(ERROR_usage(2),"%s",optusage((char*)0));
	if(*argv && !(in = sfopen((Sfio_t*)0, *argv, "r"))) 
		error(ERROR_system(1),"%s: cannot open for reading",*argv);
	n = donl(&nl,in,sfstdout);
	for(m=0; m < 3; m++)
	{
		if(nl.section[m] == (void*)&re[m])
			regfree(&re[m]);
	}
	return(n?1:0);
}
