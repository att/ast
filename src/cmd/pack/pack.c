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
 * pack [-] [-f] file ...
 *
 * pack files using Huffman coding
 *
 *   David Korn
 *   AT&T Research
 *
 */

static const char usage[] =
"[-?\n@(#)$Id: pack (AT&T Research) 2003-04-28 $\n]"
USAGE_LICENSE
"[+NAME?pack - pack files using Huffman coding]"
"[+DESCRIPTION?\bpack\b attempts to store the specified files in a compressed "
	"form using static Huffman coding.  Wherever possible each \afile\a "
	"is replaced by a packed file named \afile\a\b.z\b with the same "
	"access modes, access and modified dates, and owner as those of "
	"\afile\a. The \b-f\b option forces packing of \afile\a even when "
	"there is no space benefit for packing the file.]"
	"[+?If \bpack\b is successful, \afile\a will be removed.  Packed files "
	"can be restored to their original form using \bunpack\b or \bpcat\b.]"
"[+?\bpack\b uses Huffman (minimum redundancy) codes on a byte-by-byte basis. "
	"Ordinarily, for each file that is packed, a line is written to "
	"standard output containing \afile\a\b.z\b and the percent "
	"compression.  If the \b-v\b options is specified, or if the \b-\b "
	"argument is specified, an internal flag is set that causes the "
	"number of times each byte is used, its relative frequency, and the "
	"code for the byte to be written to the standard output. Additional "
	"occurrences of \b-\b in place of name cause the internal flag to be "
	"set and reset.]"
"[+?No packing occurs if:]{"
	"[+-?\afile\a appears to be already packed.]"
	"[+-?\afile\a has links.]"
	"[+-?\afile\a is a directory.]"
	"[+-?\afile\a cannot be opened.]"
	"[+-?No disk storage blocks will be saved by packing unless \b-f\b "
		"is specified.]"
	"[+-?A file called \afile\a\b.z\b already exists.]"
	"[+-?The \b.z\b file cannot be created.]"
	"[+-?An I/O error occurred during processing.]"
	"}"
"[f:force?Pack the file even if the packed size is larger than the original.]"
"[v:verbose?Causes additional information to be written to standard ouput.]"
"\n"
"\nfile ...\n"
"\n"
"[+EXIT STATUS]{"
        "[+0?All files packed successfully.]"
        "[+\an\a?\an\a files failed to pack, where \an\a is less than 125.]"
        "[+125?125 or more files failed to pack.]"
"}"
"[+SEE ALSO?\bunpack\b(1), \bpcat\b(1), \bcompress\b(1), \bgzip\b(1)]"
;


#include "huffman.h"
#include <error.h>
#include <ls.h>

#define BLKSIZE		512
#define block(size)	(((size) + BLKSIZE-1) & ~(BLKSIZE-1))
#define PERM(m)		((m)&(S_IRWXU|S_IRWXG|S_IRWXO))

static void vprint(Huff_t*, int);
static char *outname(char*);
static const char suffix[] = ".z";

int
main(int argc, register char *argv[])
{
	static char command[] = "pack";
	register Huff_t	*hp;
	register char	*infile,*outfile;
	Sfio_t		*fpin,*fpout;
	int		nfile=0, npack=0, force=0, verbose=0;
	int		out, deleted, dsize, n;
	struct stat	statb;

	NOT_USED(argc);
	error_info.id = command;
	while(n = optget(argv,usage)) switch(n)
	{
	case 'f':
		force++;
		break;
	case 'v':
		verbose = !verbose;
		break;
	case ':':
		error(2, opt_info.arg);
		break;
	case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors || !*argv)
		error(ERROR_usage(2), "%s", optusage((char*)0));

	while (infile = *argv++)
	{
		if(*infile == '-')
		{
			/* awful way to handle options, but preserves SVID */
			switch(infile[1])
			{
				case 'f':
					force++;
					continue;
				case 0:
					verbose = !verbose;
					continue;
			}
		}
		nfile++;
		fpin = fpout = (Sfio_t*)0;
		hp = (Huff_t*)0;
		deleted = 0;
		if(!(outfile = outname(infile)))
			continue;
		if (!(fpin=sfopen((Sfio_t*)0,infile,"r")))
			error(ERROR_system(0), "%s: cannot open", infile);
		else if(fstat(sffileno(fpin),&statb) < 0)
			error(ERROR_system(0), "%s: cannot stat", infile);
		else if(S_ISDIR(statb.st_mode))
			error(2, "%s: cannot pack a directory", infile);
		else if(statb.st_nlink > 1)
			error(2, "%s: has links", infile);
		else if(statb.st_size ==0)
			error(2, "%s: cannot pack a zero length file", infile);
		else if(access(outfile,F_OK) ==0)
			error(ERROR_system(0), "%s: already exists", outfile);
		else if(((out=open(outfile,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,PERM(statb.st_mode))) < 0) ||
			!(fpout = sfnew((Sfio_t*)0,(char*)0,SF_UNBOUND,out,SF_WRITE)))
			error(ERROR_system(0), "%s: cannot create", outfile);
		else if((deleted++,chmod(outfile,statb.st_mode)) < 0)
			error(ERROR_system(0), "%s: cannot change mode to %o",outfile,statb.st_mode);
		else
		{
			chown(outfile,statb.st_uid,statb.st_gid);
			if(!(hp = huffinit(fpin,(Sfoff_t)-1)))
				error(2, "%s: read error", infile);
			else if(sfseek(fpin,(Sfoff_t)0,0) < 0)
				error(ERROR_system(0),"%s: seek error", infile);
			else if((dsize = huffputhdr(hp,fpout)) < 0)
				error(2, "%s: write error", infile);
			else if(!force && block(huffisize(hp)) <= block(huffosize(hp)+dsize))
				error(2, "%s:no savings - file unchanged", infile);
			else if(huffencode(hp,fpin,fpout,SF_UNBOUND)<0)
				error(2, "%s: read error", infile);
			else
			{
				double diff;
				if(remove(infile) < 0)
					error(ERROR_system(0), "%s: cannot remove", infile);
				diff = huffisize(hp) - (dsize+huffosize(hp));
				sfprintf(sfstdout,"%s: %s : %.1f%% Compression\n",command,
					infile,(100*diff)/((double)huffisize(hp)));
				if(verbose)
					vprint(hp,dsize);
				npack++;
				deleted = 0;
			}
		}
		if(hp)
			huffend(hp);
		if(fpin)
			sfclose(fpin);
		if(fpout)
			sfclose(fpout);
		if(deleted)
			remove(outfile);
		if(outfile)
			free(outfile);
	}
	nfile -= npack;
	if(nfile > 125)
		nfile = 125;
	exit(nfile);
}


static char *outname(char *infile)
{
	register int n = strlen(infile);
	register int sufflen = strlen(suffix);
	register char *cp;
	if(streq(suffix,infile+n-sufflen))
	{
		error(ERROR_exit(1), "%s: already packed", infile);
		return((char*)0);
	}
	if(cp = (char*)malloc(n+sufflen+1))
	{
		strcpy(cp,infile);
		strcat(cp+n,suffix);
	}
	return(cp);
}

static void vprint(Huff_t *hp,int dsize)
{
	sfprintf(sfstdout,"	from %lld to %lld bytes\n", huffisize(hp), huffosize(hp));
	sfprintf(sfstdout,"	Huffman tree has %d levels below root\n", hp->maxlev);
	sfprintf(sfstdout,"	%d distinct bytes in input\n", hp->nchars);
	sfprintf(sfstdout,"	dictionary overhead = %ld bytes\n", dsize);
	sfprintf(sfstdout,"	effective  entropy  = %.2f bits/byte\n", 
		((double)(huffosize(hp))/(double)huffisize(hp))*CHAR_BIT);
	sfprintf(sfstdout,"	asymptotic entropy  = %.2f bits/byte\n", 
		((double)(huffosize(hp)-dsize)/(double)huffisize(hp))*CHAR_BIT);
}
