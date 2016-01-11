/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

static const char usage[] =
"[-?\n@(#)$Id: codex (AT&T Research) 2009-04-15 $\n]"
USAGE_LICENSE
"[+NAME?codex - encode/decode filter]"
"[+DESCRIPTION?\bcodex\b decodes the standard input and/or encodes the"
"	standard output according to the \amethod\a operand.]"
"[+?Codex method names consist of a leading identifier and 0 or more options"
"	separated by \b-\b or \b+\b. Method names with identical \b-\b options"
"	are equivalent. For example, \buu-base64-text\b specifies the"
"	\buu\b method with the \bbase64\b and \btext\b options, and"
"	\bpzip-crc+partition=test\b specifies the \bpzip\b method with"
"	\bcrc\b enabled using the \btest\b partition.]"
"[+?Methods may be composed using the \b<\b and \b>\b operators"
"	(with no intervening space.) \b<\b\amethod\a applies the \amethod\a"
"	decoder and \b>\b\amethod\a applies the \amethod\a encoder.]"
"[+?Supported methods are listed below. Each method may have one or more"
"	of these attributes:]{"
"	[+decode?Only decoding is supported -- most likely because the"
"		encoder has not been published. If omitted then both"
"		encoding and are supported.]"
"	[+ident?The decoder self-identifies from the input data and does"
"		not require an explicit \amethod\a operand.]"
"	[+vcodex?A self-identifying \bvcodex\b(3) method. The encoder and"
"		decoder may be applied to either the input or output."
"		Otherwise decode is restricted to the input and encode is"
"		restricted to the output.]"
"}"
"[+?By default decoders are applied to the standard input from left to"
"	right, and encoders are applied to the standard output from right"
"	to left. \bvcodex\b methods may be applied to either the input"
"	or output depending on the composition context.]"
"[+?All methods accept these options:]{"
"	[+PASSPHRASE=\apassphrase\a?The method specific passphrase. If"
"		not specified then \b--passphrase\b=\apassphrase\a is used."
"		If \b--passphrase\b is not specified then the passphrase"
"		for each method requiring one is prompted on and read from"
"		\b/dev/tty\b. Default to the interactive prompt to avoid"
"		exposing passphrases to other processes.]"
"	[+RETAIN?Decode/encode state is retained across discipline"
"		\binitf\b/\bdonef\b calls.]"
"	[+SIZE=\asize\a?The decoded size is \asize\a. Some decode methods"
"		that lack end of data markers require this because they"
"		may read/write garbage data at the end of the encoded data.]"
"	[+SOURCE=\afile\a?The delta method source \afile\a.]"
"	[+TRACE?Enable method trace.]"
"	[+VERBOSE?Enable verbose method trace.]"
"}"
"[+?The \bCODEX_OPTIONS\b environment variable may contain space separated"
"	options that control all \bcodex\b(3) methods. The environment options"
"	are:]{"
"	[+trace=\apattern\a?Enable method trace for all methods matching the"
"		\bksh\b(1) \apattern\a.]"
"	[+verbose=\apattern\a?Enable verbose method trace for all methods"
"		matching the \bksh\b(1) \apattern\a. A verbose trace includes"
"		\bcodex\b(3) discipline exception calls.]"
"}"
"[+?The supported methods are:]{\fmethods\f}"

"[d:decode?Apply the \amethod\a operand to the standard input only.]"
"[e:encode?Apply the \amethod\a operand to the standard output only.]"
"[f:passfile?Like \b--passphrase\b, except the passphrase is the first"
"	line (sans newline) from \afile\a.]:[file]"
"[i:identify?Identify and write the standard input encoding name on the"
"	standard output and exit.]"
"[n:null?Write to the \bcodenull\b(3) stream instead of the standard output.]"
"[p:passphrase?The default passphrase for all methods requiring passphrases."
"	The method specific \bPASSPHRASE\b=\apassphrase\a overrides the"
"	default. If \b--passphrase\b and \bPASSPHRASE\b=\apassphrase\a are"
"	not specified then the passphrase for each method requiring one"
"	is prompted on and read from \b/dev/tty\b. Default to the interactive"
"	prompt to avoid exposing passphrases to other processes.]:[passphrase]"
"[r:invert|reverse?Invert the method composition.]"
"[t:trace?Enable method trace. Equivalent to"
"	\bexport CODEX_OPTIONS='trace=*'\b.]"
"[v:verbose?Enable verbose method trace. A verbose trace includes \bcodex\b(3)"
"	discipline exception calls. Equivalent to"
"	\bexport CODEX_OPTIONS='verbose=*'\b.]"

"\n"
"\n[ [ <,> ] method [ <,>,| method ... ] ]\n"
"\n"

"[+SEE ALSO?\bcodex\b(3), \bvcodex\b(3)]"
;

#include <ast.h>
#include <codex.h>
#include <option.h>

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register Codexmeth_t*	meth;
	register const char*	p;
	register int		c;

	switch (*s)
	{
	case 'm':
		for (meth = codexlist(NiL); meth; meth = codexlist(meth))
		{
			sfprintf(sp, "[+%s\b", meth->name);
			p = " (";
			if (meth->identf || meth->vcmeth)
			{
				sfprintf(sp, "%sident", p);
				p = ",";
			}
			if (meth->vcmeth)
			{
				sfprintf(sp, "%svcodex", p);
				p = ",";
			}
			else if (!(meth->flags & CODEX_ENCODE))
			{
				sfprintf(sp, "%sdecode", p);
				p = ",";
			}
			if (*p == ',')
				sfputc(sp, ')');
			sfputc(sp, '?');
			p = meth->description;
			while (c = *p++)
			{
				if (c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, ']');
			if ((p = meth->options) || meth->optionsf)
			{
				sfprintf(sp, "{\n");
				if (meth->optionsf)
					(*meth->optionsf)(meth, sp);
				if (p)
					sfprintf(sp, "%s", p);
				sfprintf(sp, "\n}");
			}
		}
		break;
	}
	return 0;
}

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
	Sfio_t*			pp;
	char*			s;
	Codexnum_t		flags;
	Optdisc_t		optdisc;
	char			ident[CODEX_IDENT];

	static Codexdisc_t	codexdisc;

	error_info.id = "codex";
	codexinit(&codexdisc, errorf);
	optinit(&optdisc, optinfo);
	ip = sfstdin;
	op = sfstdout;
	flags = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			op = 0;
			continue;
		case 'e':
			ip = 0;
			continue;
		case 'f':
			if (!(pp = sfopen(NiL, opt_info.arg, "r")))
				error(ERROR_SYSTEM|3, "%s: cannot read", opt_info.arg);
			else if (!(s = sfgetr(pp, '\n', 1)))
			{
				sfclose(pp);
				error(ERROR_SYSTEM|3, "%s: read error", opt_info.arg);
			}
			codexdisc.passphrase = strcpy(fmtbuf(sfvalue(pp)), s);
			sfclose(pp);
			continue;
		case 'i':
			if (!(buf = sfreserve(ip, CODEX_IDENT, SF_LOCKR)) && !(buf = sfreserve(ip, sfvalue(ip), SF_LOCKR)))
				error(ERROR_SYSTEM|3, "read error");
			if (codexid(buf, sfvalue(ip), ident, sizeof(ident)))
				sfprintf(op, "%s\n", ident);
			else
				error(2, "unknown input encoding");
			sfread(ip, buf, 0);
			return 0;
		case 'n':
			op = codexnull();
			continue;
		case 'p':
			codexdisc.passphrase = opt_info.arg;
			continue;
		case 'r':
			flags |= CODEX_INVERT;
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
	if (error_info.errors || *argv && *(argv + 1))
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (!ip && !op)
		ip = sfstdin;
	if (codex(ip, op, *argv, flags, &codexdisc, NiL) < 0)
		return 1;
	if (!ip)
		ip = sfstdin;
	if (!op)
		op = sfstdout;
	if (sfmove(ip, op, SF_UNBOUND, -1) < 0 || !sfeof(ip) || sferror(ip))
		error(ERROR_SYSTEM|2, "read error");
	checkdata(ip);
	checkdata(op);
	if (sfsync(op) || sferror(op))
		error(ERROR_SYSTEM|2, "write error");
#ifdef main
	codexpop(ip, op, 0);
#endif
	return error_info.errors != 0;
}
