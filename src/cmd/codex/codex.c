/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

static const char usage[] =
"[-?\n@(#)$Id: codex (AT&T Research) 2013-08-11 $\n]"
USAGE_LICENSE
"[+NAME?codex - encode/decode filter]"
"[+DESCRIPTION?\bcodex\b decodes the standard input and/or encodes the "
    "standard output according to the \b--decode\b and/or \b--encode\b "
    "methods. Either \b--identify\b or one or both of \b--decode\b and "
    "\b--encode\b must be specified.]"
"[+?Codex method names consist of a leading identifier and 0 or more "
    "options separated by \b-\b or \b.\b or \b+\b. Method names with "
    "identical \b-\b options are equivalent. For example, \buu-base64-text\b "
    "specifies the \buu\b method with the \bbase64\b and \btext\b options, "
    "and \bpzip-crc+partition=test\b specifies the \bpzip\b method with "
    "\bcrc\b enabled using the \btest\b partition.  \alibvcodex\a and \alibcodex\a "
    "(\bcatalog\b) values denote methods supplied by the default libraries; "
    "otherwise the method is a separate plugin; that plugin will be required "
    "to decode any encoded data. Differentiate \bcodex\b and \bvcodex\b "
    "methods with the same name by using \b-\b or \b+\b for \bcodex\b options "
    "and \b.\b for \bvcodex\b options.]"
"[+?Methods may be composed using the \b^\b or \b,\b operators (with no "
    "intervening space.)]"
"[+?Supported methods are listed below. Each method may have one or more "
    "of these attributes:]"
    "{"
        "[+codex?A \bcodex\b(3) method.]"
        "[+decode?Only decoding is supported -- most likely because the "
            "encoder has not been published. If omitted then both encoding "
            "and decoding are supported.]"
        "[+ident?The decoder self-identifies from the input data and "
            "does not require an explicit \amethod\a operand.]"
        "[+vcodex?A \bvcodex\b(3) method.]"
    "}"
"[+?All methods accept these options:]"
    "{"
        "[+PASSPHRASE=\apassphrase\a?The method specific passphrase. If "
            "not specified then \b--passphrase\b=\apassphrase\a is used. If "
            "\b--passphrase\b is not specified then the passphrase for each "
            "method requiring one is prompted on and read from \b/dev/tty\b. "
            "Default to the interactive prompt to avoid exposing passphrases "
            "to other processes.]"
        "[+RETAIN?Decode/encode state is retained across discipline "
            "\binitf\b/\bdonef\b calls.]"
        "[+SIZE=\asize\a?The decoded size is \asize\a. Some decode "
            "methods that lack end of data markers require this because they "
            "may read/write garbage data at the end of the encoded data.]"
        "[+SOURCE=\afile\a?The delta method source \afile\a.]"
        "[+TRACE?Enable method trace.]"
        "[+VERBOSE?Enable verbose method trace.]"
        "[+WINDOW=\asize\a?The method window size.]"
    "}"
"[+?The \bCODEX_OPTIONS\b environment variable may contain space "
    "separated options that control all \bcodex\b(3) methods. The "
    "environment options are:]"
    "{"
        "[+trace[=\apattern\a]]"
            "?Enable method trace for all methods matching the \bksh\b(1) "
            "\apattern\a or all methods if \apattern\a is omitted.]"
        "[+verbose[=\apattern\a]]"
            "?Enable verbose method trace for all methods matching the "
            "\bksh\b(1) \apattern\a or all methods if \apattern\a is "
            "omitted. A verbose trace includes \bcodex\b(3) discipline "
            "exception calls.]"
    "}"
"[+?The methods and method aliases are:]"
    "{\fvcodex\f\fcodex\f}"

"[d:decode?Decode the standard input using \amethod\a. \amethod\a may be "
    "omitted for self-identifying input.]:?[method]"
"[e:encode?Encode the standard output using \amethod\a.]:[method]"
"[f:passfile?Like \b--passphrase\b, except the passphrase is the first "
    "line (sans newline) from \afile\a.]:[file]"
"[i:identify?Identify and write the standard input encoding method the "
    "standard output and exit.]"
"[n:null?Write to the \bcodenull\b(3) stream instead of the standard "
    "output.]"
"[p:passphrase?The default passphrase for all methods requiring "
    "passphrases. The method specific \bPASSPHRASE\b=\apassphrase\a "
    "overrides the default. If \b--passphrase\b and "
    "\bPASSPHRASE\b=\apassphrase\a are not specified then the passphrase for "
    "each method requiring one is prompted on and read from \b/dev/tty\b. "
    "Default to the interactive prompt to avoid exposing passphrases to "
    "other processes.]:[passphrase]"
"[P:plain?Do not encode identification headers.]"
"[t:trace?Enable method trace. Equivalent to \bexport "
    "CODEX_OPTIONS='trace'\b.]"
"[v:verbose?Enable verbose method trace. A verbose trace includes "
    "\bcodex\b(3) discipline exception calls. Equivalent to \bexport "
    "CODEX_OPTIONS='verbose'\b.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bvczip\b(1), \bcodex\b(3), \bvcodex\b(3)]"
;

#include <ast.h>
#include <codex.h>
#include <option.h>

static void
checkdata(Sfio_t* sp)
{
	register unsigned char*	u;
	register unsigned char*	e;
	Codexdata_t		data;

	if (codexdata(sp, &data) > 0)
	{
		if (data.buf)
		{
			for (e = (u = (unsigned char*)data.buf) + data.size; u < e; u++)
				sfprintf(sfstderr, "%02x", *u);
			sfprintf(sfstderr, "\n");
		}
		else
			sfprintf(sfstderr, "%0*I*x\n", data.size * 2, sizeof(data.num), data.num);
	}
}

#if defined(main)
#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif
extern
#endif

int
main(int argc, register char** argv)
{
	void*			buf;
	Sfio_t*			ip;
	Sfio_t*			op;
	char*			s;
	char*			decode;
	char*			encode;
	int			identify;
	Codexnum_t		flags;
	Optdisc_t		optdisc;
	char			ident[CODEX_IDENT];

	static Codexdisc_t	codexdisc;

	error_info.id = "codex";
	codexinit(&codexdisc, errorf);
	optinit(&optdisc, codexoptinfo);
	decode = encode = 0;
	identify = 0;
	op = sfstdout;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			flags |= CODEX_DECODE;
			decode = opt_info.arg;
			continue;
		case 'e':
			flags |= CODEX_ENCODE;
			encode = opt_info.arg;
			continue;
		case 'f':
			if (!(ip = sfopen(NiL, opt_info.arg, "rb")))
				error(ERROR_SYSTEM|3, "%s: cannot read", opt_info.arg);
			else if (!(s = sfgetr(ip, '\n', 1)))
			{
				sfclose(ip);
				error(ERROR_SYSTEM|3, "%s: read error", opt_info.arg);
			}
			codexdisc.passphrase = strcpy(fmtbuf(sfvalue(ip)), s);
			sfclose(ip);
			continue;
		case 'i':
			identify = 1;
			continue;
		case 'n':
			op = codexnull();
			continue;
		case 'p':
			codexdisc.passphrase = opt_info.arg;
			continue;
		case 'P':
			codexdisc.flags |= CODEX_PLAIN;
			continue;
		case 't':
			flags |= CODEX_TRACE;
			continue;
		case 'v':
			flags |= CODEX_VERBOSE;
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
	if (error_info.errors || !identify && *argv && *(argv + 1))
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (identify)
	{
		if (!(codexdisc.identify = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space");
		if (s = *argv)
			argv++;
		identify = !!*argv;
		do
		{
			if (!s || streq(s, "-"))
			{
				s = "/dev/stdin";
				ip = sfstdin;
			}
			else if (!(ip = sfopen(NiL, s, "rb")))
			{
				error(ERROR_SYSTEM|2, "%s: cannot open", s);
				continue;
			}
			if (codex(ip, 0, CODEX_DECODE, &codexdisc, NiL) >= 0)
			{
				if (identify)
					sfprintf(sfstdout, "%s: ", s);
				sfputr(sfstdout, sfstruse(codexdisc.identify), '\n');
				codexpop(ip, 0);
			}
		} while (s = *argv++);
	}
	else
	{
		ip = sfstdin;
		if ((s = *argv) && !(ip = sfopen(NiL, s, "rb")))
			error(ERROR_SYSTEM|2, "%s: cannot open", s);
		else
		{
			if ((!(flags & CODEX_DECODE) || codex(ip, decode, CODEX_DECODE, &codexdisc, NiL) >= 0) &&
			    (!(flags & CODEX_ENCODE) || codex(op, encode, CODEX_ENCODE, &codexdisc, NiL) >= 0))
			{
				if (sfmove(ip, op, SF_UNBOUND, -1) < 0 || !sfeof(ip) || sferror(ip))
				{
					if (s)
						error(ERROR_SYSTEM|2, "%s: read error", s);
					else
						error(ERROR_SYSTEM|2, "read error");
				}
				checkdata(ip);
				checkdata(op);
				if (sfsync(op) || sferror(op))
					error(ERROR_SYSTEM|2, "write error");
			}
#ifdef main
			codexpop(ip, 0);
			codexpop(op, 0);
#endif
			if (ip != sfstdin)
				sfclose(ip);
		}
	}
	return error_info.errors != 0;
}
