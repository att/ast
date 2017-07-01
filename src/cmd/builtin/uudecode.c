/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2011 AT&T Intellectual Property          *
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
 * uuencode/uudecode common main
 */

static const char usage[] =
"[-?\n@(#)$Id: uudecode (AT&T Research) 2002-03-24 $\n]"
USAGE_LICENSE
"[+NAME?uudecode - decode a uuencoded binary file]"
"[+DESCRIPTION?\buudecode\b decodes the named input \afile\a"
"	or the standard input if no file is specified, to the file"
"	name encoded in \afile\a by \buuencode\b. If \adecode-file\a is"
"	specified then the output is written there. \b-\b is equivalent"
"	to the standard output.]"

"[c:local?Convert pathnames to file names in the local directory.]"
"[h!:header?The input file contains header and trailer sequences."
"	The header for some encoding formats may contain file name and"
"	access infomation.]"
"[l:list?List the encoding method names on the standard output.]"
"[o:output?Write the output data into \afile\a instead of the standard"
"	output. \adecode-file\a if specified overrides \b--output\b.]:[file]"
"[t:text?The input file is a text file that requires \\n => \\r\\n translation"
"	on encoding.]"
"[x:method?Specifies the encoding \amethod\a. Some encoding methods self"
"	identify and for those \b--method\b is optional.]:[method]{\fmethods\f}"
"[m?Equivalent to \b--method=mime\b.]"
"[q?Equivalent to \b--method=quoted-printable\b.]"
"[u?Equivalent to \b--method=uuencode\b.]"

"\n"
"\n[ file [ decode-file ] ]\n"
"\n"

"[+SEE ALSO?\bmailx\b(1), \buuencode\b(1)]"
;

#include <ast.h>
#include <uu.h>
#include <error.h>
#include <option.h>

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register Uumeth_t*	mp;

	switch (*s)
	{
	case 'm':
		for (mp = uumeth(NiL); mp->name; mp++)
			sfprintf(sp, "[+%s|%s]", mp->name, mp->alias);
		break;
	}
	return 0;
}

int
main(int argc, register char** argv)
{
	Uu_t*		uu;
	Uumeth_t*	meth;
	char*		encoding;
	char*		ipath;
	char*		opath;
	Sfio_t*		ip;
	Sfio_t*		op;
	char		buf[2];
	Optdisc_t	optdisc;
	Uudisc_t	disc;

	error_info.id = "uudecode";
	memset(&optdisc, 0, sizeof(optdisc));
	optdisc.version = OPT_VERSION;
	optdisc.infof = optinfo;
	opt_info.disc = &optdisc;
	memset(&disc, 0, sizeof(disc));
	disc.version = UU_VERSION;
	disc.flags = UU_HEADER;
	disc.errorf = errorf;
	encoding = 0;
	ipath = opath = 0;
	ip = sfstdin;
	op = sfstdout;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'm':
		case 'q':
		case 'u':
			buf[0] = opt_info.option[1];
			buf[1] = 0;
			encoding = buf;
			continue;
		case 'c':
			disc.flags |= UU_LOCAL;
			continue;
		case 'h':
			disc.flags &= ~UU_HEADER;
			continue;
		case 'l':
			uulist(sfstdout);
			exit(0);
		case 'o':
			opath = opt_info.arg;
			continue;
		case 't':
			disc.flags |= UU_TEXT;
			continue;
		case 'x':
			encoding = opt_info.arg;
			if (streq(encoding, "?"))
			{
				uulist(sfstdout);
				exit(0);
			}
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if ((ipath = *argv) && *++argv)
		opath = *argv++;
	if (error_info.errors || *argv)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (!(meth = uumeth(encoding)))
		error(ERROR_exit(1), "%s: unknown method", encoding);
	if (uu = uuopen(&disc, meth))
	{
		if (!ipath)
			ip = sfstdin;
		else if (!(ip = sfopen(NiL, ipath, "rt")))
			error(ERROR_system(1), "%s: cannot read", ipath);
		op = 0;
		uudecode(uu, ip, op, SF_UNBOUND, opath);
		uuclose(uu);
	}
	return error_info.errors != 0;
}
