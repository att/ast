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
 * asa.c
 * Written by David Korn
 * AT&T Labs
 * Tue Mar 25 11:03:23 EST 2003
 */

static const char usage[] =
"[-?\n@(#)$Id: asa (AT&T Research) 2003-03-25 $\n]"
"[-author?David Korn <dgk@research.att.com>]"
"[-license?http://www.research.att.com/sw/tools/reuse]"
"[+NAME?asa - interpret carriage-control characters]"
"[+DESCRIPTION?\basa\b writes its input files to standard output mapping "
	"carriage control characters to line-printer control sequences.]"
"[+?The first character of each line or record is removed from the input and "
	"is processed as follows before the rest of the line is output:]{"
		"[+0?A new-line is output.]"
		"[+1?Advance to the next page.]"
		"[++?Overwrite the previous line.]"
		"[+\aspace\a?Just output the remainder of the line.]"
	"}"
	"[+?Any other character as the first character is treated as if "
		"it were a space.]"
"[+?If no \afile\a operands are given or if the \afile\a is \b-\b, \basa\b "
        "reads from standard input. The start of the file is defined as the "
        "current offset.]"
"[r]#[reclen?If \areclen\a is greater than zero, the file is assumed to "
	"consist of fixed length records of length \areclen\a.]"
"\n"
"\n[file ... ]\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Success.]"
        "[+>0?An error occurred.]"
"}"
"[+SEE ALSO?\blpr\b(1)]"
;

#include	<cmd.h>

static int asa(register Sfio_t *in, Sfio_t *out, int reclen)
{
	register char	*cp;
	register int	n, c = 0;
	while(1)
	{
		if(reclen>0)
			cp = sfreserve(in,n=reclen, -1);
		else
			cp = sfgetr(in,'\n',0);
		if(!cp)
			break;
		if(reclen<=0)
			n = sfvalue(in)-2;
		else
			while(--n>0 && cp[n]==' ');
		switch(*cp)
		{
		    case '\n':
			break;
		    case '0':
			sfputc(out,'\n');
			break;
		    case '1':
			if(c)
			{
				sfputc(out,c);
				c = '\f';
			}
			break;
		    case '+':
			c = '\r';
			break;
		}
		if(c)
			sfputc(out,c);
		if(n>0)
			sfwrite(out,cp+1,n);
		c = '\n';
	}
	if(c)
		sfputc(out,c);
	return(0);
}

int
b_asa(int argc, char** argv, Shbltin_t* context)
{
	register char	*cp;
	register Sfio_t	*fp;
	register int	n, reclen=0;

	cmdinit(argc, argv, (void*)0, (const char*)0, 0);
	while (n = optget(argv, usage)) switch (n)
	{
	    case 'r':
		reclen = opt_info.num;
		break;
	    case ':':
		error(2, "%s", opt_info.arg);
		break;
	    case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors)
		error(ERROR_usage(2),"%s",optusage((char*)0));
	if (cp = *argv)
		argv++;
	do
	{
		if (!cp || streq(cp,"-"))
			fp = sfstdin;
		else if (!(fp = sfopen(NiL, cp, "r")))
		{
			error(ERROR_system(0), "%s: cannot open", cp);
			error_info.errors = 1;
			continue;
		}
		n = asa(fp, sfstdout,reclen);
		if (fp != sfstdin)
			sfclose(fp);
	} while (cp = *argv++);
	return(error_info.errors);
}
