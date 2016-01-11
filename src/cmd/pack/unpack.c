/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1993-2011 AT&T Intellectual Property          *
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
 * unpack file ...
 * pcat file ...
 *
 * unpack files that have been Huffman encoded
 *
 *   David Korn
 *   AT&T Research
 *
 */

static const char usage_head[] =
"[-?@(#)$Id: unpack (AT&T Research) 2003-04-28 $\n]"
USAGE_LICENSE
;

static const char usage_common[] =
	"For each \afile\a specified, a search is made for a file named "
	"\afile\a\b.z\b (or just \afile\a, if \afile\a ends in \b.z\b).  If "
	"this file appears to be a packed file, ";

static const char usage_tail[] = "\n\nfile ...\n\n"
"[+EXIT STATUS]{"
        "[+0?All files unpacked successfully.]"
        "[+\an\a?\an\a files failed to unpack, where \an\a is less than 125.]"
        "[+125?125 or more files failed to unpack.]"
"}";


static const char usage_pcat1[] =
"[+NAME?pcat - unpack and concatenate files created by pack]"
"[+DESCRIPTION?\bpcat\b does for packed files what \bcat\b(1) does "
	"for ordinary files, except that \bpcat\b cannot be used "
	"as filter.  ";
static const char usage_pcat2[] =
	"\bpcat\b unpacks the file and writes it to standard output.]";
static const char pcat_see_also[] =
"[+SEE ALSO?pack(1), unpack(1), cat(1), zcat(1), gzcat(1)]";

static const char usage_unpack1[] =
"[+NAME?unpack - unpack files created by pack]"
"[+DESCRIPTION?\bunpack\b expands files created by \bpack\b.  ";

static const char usage_unpack2[] =
	"it is replaced by its original expanded version.  The new file "
	"has the \b.z\b suffix stripped from its name, and has the same "
	"access modes, access and modification dates, and owner as those "
	"of the packed file.]";

static const char unpack_see_also[] =
"[+SEE ALSO?pack(1), pcat(1), uncompress(1), gunzip(1)]";



#include "huffman.h"
#include <error.h>
#include <ls.h>
#include <times.h>

#define PERM(m)	((m)&(S_IRWXU|S_IRWXG|S_IRWXO))

static char *inname(char*);
static const char suffix[] = ".z";

int
main(int argc, char *argv[])
{
	register Huff_t	*hp;
	register char	*infile,*outfile;
	char		*command;
	char		*usage;
	int		out;
	Sfio_t		*fpin,*fpout,*iop;
	int		nunpack = 1;
	int		pcat;
	int		deleted;
	int		n;
	struct stat	statb;

	if(command = strrchr(argv[0],'/'))
		command++;
	else
		command = argv[0];
	error_info.id = command;
	pcat = (*command=='p');
        if(!(iop = sfstropen()))
                error(ERROR_system(1), "out of space [tmp string]");
        sfputr(iop, usage_head, -1);
	if(pcat)
	{
		sfputr(iop, usage_pcat1, -1);
		sfputr(iop, usage_common, -1);
		sfputr(iop, usage_pcat2, -1);
	        sfputr(iop, usage_tail, -1);
		sfputr(iop, pcat_see_also, -1);
	}
	else
	{
		sfputr(iop, usage_unpack1, -1);
		sfputr(iop, usage_common, -1);
		sfputr(iop, usage_unpack2, -1);
	        sfputr(iop, usage_tail, -1);
		sfputr(iop, unpack_see_also, -1);
	}
	if (!(usage = sfstruse(iop)))
		error(ERROR_SYSTEM|3, "out of space");
	
	while(n = optget(argv,usage)) switch(n)
	{
	case ':':
		error(2, "%s", opt_info.arg);
		break;
	case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors || !*argv)
		error(ERROR_usage(2), "%s", optusage((char*)0));
	sfclose(iop);
	while (outfile = *argv++)
	{
		fpin = fpout = (Sfio_t*)0;
		hp = (Huff_t*)0;
		deleted = 0;
		if(!(infile = inname(outfile)))
			continue;
		if(!(fpin=sfopen((Sfio_t*)0,infile,"r")))
		{
			error(ERROR_system(0), "%s: cannot open", infile);
			continue;
		}
		if(pcat)
			fpout = sfstdout;
		else
		{
			if(fstat(sffileno(fpin),&statb) < 0)
				error(ERROR_system(0), "%s: cannot stat", infile);
			else if(S_ISDIR(statb.st_mode))
				error(2, "%s: cannot pack a directory", infile);
			else if(statb.st_nlink > 1)
				error(2, "%s: has links", infile);
			else if(access(outfile,F_OK) ==0)
				error(2, "%s: already exists", outfile);
			else if(((out=open(outfile,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,PERM(statb.st_mode))) < 0) ||
				!((fpout = sfnew((Sfio_t*)0,(char*)0,SF_UNBOUND,out,SF_WRITE))))
				error(2, "%s: cannot create", outfile);
			else if((deleted++,chmod(outfile,statb.st_mode)) < 0)
				error(ERROR_system(0), "%s: cannot change mode to %o",outfile,statb.st_mode);
			else
			{
				chown(outfile,statb.st_uid,statb.st_gid);
				goto ok;
			}
			goto skip;
		}
	ok:
		if(!(hp = huffgethdr(fpin)))
			error(2, "%s: read error", infile);
		else if(huffdecode(hp,fpin,fpout,SF_UNBOUND)<0)
			error(2, "%s: read error", outfile);
		else
		{
			if(!pcat)
			{
				touch(outfile,statb.st_atime,statb.st_mtime,1);
				sfprintf(sfstdout,"%s: %s: unpacked\n",command, infile);
				deleted = 0;
				if(remove(infile) < 0)
					error(ERROR_system(0), "%s: cannot remove", infile);
			}
			nunpack++;
		}
	skip:
		if(hp)
			huffend(hp);
		if(fpin)
			sfclose(fpin);
		if(fpout)
			sfclose(fpout);
		if(deleted)
			remove(outfile);
		if(infile)
			free(infile);
	}
	argc -= nunpack;
	if(argc > 125)
		argc = 125;
	exit(argc);
}


static char *inname(register char *outfile)
{
	register char *cp;
	register int n = strlen(outfile);
	register int sufflen = strlen(suffix);
	if(cp = (char*)malloc(n+sufflen+1))
	{
		strcpy(cp,outfile);
		if(streq(suffix,cp+n-sufflen))
			outfile[n-sufflen] = 0;
		else
			strcat(cp+n,suffix);
	}
	return(cp);
}
