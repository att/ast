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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
/*	Command to encode and decode with Vcodex methods.
**
**	Written by Kiem-Phong Vo (09/06/2003 - ILNGUYEN)
*/

#include <ast.h>
#include <vcodex.h>
#include <codex.h>
#include <ctype.h>
#include <error.h>
#include <tmx.h>

#define BUFFERSIZE	8*1024*1024

#define S(s)		# s
#define X(s)		S(s)

static const char usage[] =
"[-?\n@(#)$Id: vczip (AT&T Research) 2013-08-11 $\n]"
USAGE_LICENSE
"[+NAME?vczip - vcodex method encode/decode filter]"
"[+DESCRIPTION?\bvczip\b is a filter that decodes the standard input "
    "and/or encodes the standard output. The decoding method is "
    "automatically determined. The encoding method is determined by: the "
    "\b--method\b option if specified, or if one or more \afile\a operands "
    "are specified, by the file name pattern methods listed under "
    "\b--method\b below, or if there is no file name match, by the alias "
    "for the command base name.]"
"[+?Method names consist of a leading identifier and 0 or more "
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
"[+?Method aliases and file name pattern methods may be defined in \b../"
    VC_ALIASES "\b in one of the directories on \b$PATH\b, or in \b$HOME/"
    VC_ZIPRC "\b, searched in order. Each alias is a \aname=method\a pair "
    "and each file name pattern method is a \apattern:method\a pair, "
    "where \apattern\a is a \bksh\b(1) file match pattern and method\a is "
    "described under \b--method\b below. Method names are searched before "
    "alias names.]"
"[b:buffer-size?The io buffer size; \b0\b to use \bsfmove\b(3) instead of "
    "\bsfread\b(3)/\bsfwrite\b(3).]#[size:=" X(BUFFERSIZE) "]"
"[c:cat?Decode input files to the standard output.]"
"[f:force?Force compression or decompression even if the input file has "
    "multiple links, or the output file already exists, or if the compressed "
    "data is written to a terminal.]"
"[m:method|encode?Set the transformation method from the \b,\b (or \b^\b) "
    "separated list of \amethod\a[.\aarg\a]] elements, where \amethod\a is "
    "a method alias or primitive method, \aarg\a is an optional method "
    "specific argument, and \b-\b denotes the default argument. Parenthesized "
    "values are implementation details. \alibvcodex\a and \alibcodex\a "
    "(\bcatalog\b) values denote methods supplied by the default libraries; "
    "otherwise the method is a separate plugin; that plugin will be required "
    "to decode any encoded data. Differentiate \bcodex\b and \bvcodex\b "
    "methods with the same name by using \b-\b or \b+\b for \bcodex\b options "
    "and \b.\b or \b=\b for \bvcodex\b options. The methods, aliases, and file "
    "name pattern defaults are:]:[method[.arg]][,method[.arg]]...]]:=vczip]"
    "{\fvcodex\f\fcodex\f}"
"[o:output?Output data is written to \afile\a instead of the standard "
    "output. Invalid if more than one \afile\a operand is specified.]:[file]"
"[p:plain?Do not encode transformation information in the data. This means "
    "the transform must be explicitly supplied to decode.]"
"[q:identify?Identify the standard input encoding and write the "
    "\b--method\b transformation string on the standard output. "
    "If the standard input is not vczip encoded then nothing is "
    "printed.]"
"[s:source?Delta method source file.]:[file]"
"[u:undo|uncompress|decode?Decode data.]"
"[w:window?Set the data partition window size to \awindow\a. "
    "\amethod\a specifies an optional window matching "
    "method. The window methods are:]:[window[,method]]]"
    "{\fwindows\f}"
"[D:debug?Set the debug trace level to \alevel\a. Higher levels produce "
    "more output.]:[level]"
"[M:move?Use sfmove() for io.]"

"\n"
"\nfile ... | < input > output\n"
"\n"

"[+FILES]"
    "{"
        "[+../" VC_ALIASES "?Method alias and file name patterns, found "
            "on \b$PATH\b.]"
    "}"
"[+SEE ALSO?\bcodex\b(1), \bcodex\b(3), \bvcodex\b(3)]"
;

#define VC_cat		0x01
#define VC_force	0x02
#define VC_remove	0x04

static int
optmethod(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;
	Vcmethod_t*	mt = (Vcmethod_t*)obj;
	int		i;

	sfprintf(sp, "[+%s?", name);
	optesc(sp, desc, 0);
	if(mt->args)
	{	sfprintf(sp, " The arguments are:]{");
		for(i = 0; mt->args[i].desc; i++)
		{	sfprintf(sp, "[+%s?", mt->args[i].name ? mt->args[i].name : "-");
			if(mt->args[i].desc)
				optesc(sp, mt->args[i].desc, 0);
			sfputc(sp, ']');
			if(!mt->args[i].name)
				break;
		}
	}
	else
		sfputc(sp, ']');
	if(mt->about)
	{	if(!mt->args)
			sfputc(sp, '{');
		sfprintf(sp, "%s}", mt->about);
	}
	else if(mt->args)
		sfputc(sp, '}');
	return 0;
}

static int
optalias(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;

	sfprintf(sp, "[+%s?Equivalent to \b%s\b.]", name, desc);
	return 0;
}

static int
optfname(Void_t* obj, char* pattern, char* transform, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;

	sfprintf(sp, "[+%s?File name pattern match encoding method \b%s\b.]", pattern, transform);
	return 0;
}

static int
optwindow(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;

	sfprintf(sp, "[+%s?", name);
	optesc(sp, desc, 0);
	sfprintf(sp, "]");
	return 0;
}

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
	case 'c':
		/* codex methods */
		for (meth = codexlist(NiL); meth; meth = codexlist(meth))
		{
			sfprintf(sp, "[+%s\b (codex", meth->name);
			if (meth->identf)
				sfprintf(sp, ",ident");
			if (!(meth->flags & CODEX_ENCODE))
				sfprintf(sp, ",decode");
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
	case 'v':
		/* vcodex methods */
		vcwalkmeth(optmethod, sp);
		/* aliases */
		vcwalkalias(optalias, sp);
		/* vcodex file name pattern methods */
		vcwalkfname(optfname, sp);
		break;
	case 'w':
		/* vcodex window methods */
		vcwwalkmeth(optwindow, sp);
		break;
	}
	return 0;
}

/*
 * apply transform from input to output
 */

static int
apply(int action, int flags, char* command, char* transform, char* input, char* output, char* buf, size_t bufsize, Codexdisc_t* codexdisc)
{
	Sfio_t*			ip;
	Sfio_t*			op;
	char*			s;
	ssize_t			n;
	int			r;
	Codexdata_t		data;
	struct stat		si;
	struct stat		so;
	char			sx[3];

	Sfio_t*			pp = 0;
	char*			suffix = 0;

	static const char	devstdin[] = "/dev/stdin";
	static const char	devstdout[] = "/dev/stdout";

	/*
	 * set up the sfio input stream
	 */

	if (!input || streq(input, "-") || streq(input, devstdin))
	{
		input = (char*)devstdin;
		ip = sfstdin,
		sfopen(ip, NiL, "rb");
		flags |= VC_cat;
	}
	else if (!(ip = sfopen(NiL, input, "rb")))
	{
		error(ERROR_SYSTEM|2, "%s: cannot read", input);
		return 0;
	}
	sfset(ip, SF_SHARE, 0);
	if (fstat(sffileno(ip), &si))
	{
		error(ERROR_SYSTEM|2, "%s: cannot stat", input);
		goto nope;
	}

	/*
	 * set up the sfio output stream
	 */

	if (!output && !(flags & VC_cat))
	{
		if (!transform && vcgetfname(input, &transform, &suffix))
			transform = command;
		if (transform && vcgetsuff(transform, &suffix))
		{
			suffix = sx;
			sx[0] = transform[0];
			sx[1] = 'z';
			sx[2] = 0;
		}
		if (suffix)
		{
			if (!(pp = sfstropen()))
			{
				error(ERROR_SYSTEM|2, "%s: out of space", input);
				goto nope;
			}
			if (action == VC_ENCODE)
				sfprintf(pp, "%s.%s", input, suffix);
			else if ((s = strrchr(input, '.')) && !strcmp(s + 1, suffix))
				sfwrite(pp, input, s - input);
			else
			{
				error(2, "%s: unknown suffix -- ignored", input);
				goto nope;
			}
			output = sfstruse(pp);
			flags |= VC_remove;
		}
		else
		{
			error(2, "%s: unknown suffix for method %s -- ignored", input, transform);
			goto nope;
		}
		if (!stat(output, &so) && si.st_dev == so.st_dev && si.st_ino == so.st_ino)
		{
			error(ERROR_SYSTEM|2, "%s: identical to %s", output, input);
			goto nope;
		}
		if (!(op = sfopen(NiL, output, "wb")))
		{
			error(ERROR_SYSTEM|2, "%s: cannot write", output);
			goto nope;
		}
	}
	if (!output || streq(output, "-") || streq(output, devstdout))
	{
		output = (char*)devstdout;
		op = sfstdout,
		sfopen(op, NiL, "wb");
	}
	sfset(op, SF_SHARE, 0);

	/*
	 * check codex sfio discipline
	 */

	if (action == VC_ENCODE)
	{
		if (codex(op, transform, CODEX_ENCODE, codexdisc, NiL) < 0)
		{
			error(2, "%s: cannot push output stream discipline", output);
			goto bad;
		}
	}
	else if (codex(ip, transform, CODEX_DECODE, codexdisc, NiL) < 0)
	{
		error(2, "%s: cannot push input stream discipline", input);
		goto bad;
	}

	/*
	 * copy from sfstdin to sfstdout
	 */

	if (action)
	{
		if (buf)
			for (;;)
			{	
				if ((n = sfread(ip, buf, bufsize)) <= 0)
				{
					if (n < 0)
						error(ERROR_SYSTEM|2, "%s: read error", input);
					break;
				}
				if (sfwrite(op, buf, n) < 0)
					break;
			}
		else
		{
			sfmove(ip, op, SF_UNBOUND, -1);
			if (!sfeof(ip))
				error(ERROR_SYSTEM|2, "%s: read error", input);
		}
		if (sferror(op) || sfsync(op))
			error(ERROR_SYSTEM|2, "%s: write error", output);
		else if (codexdata(action == VC_ENCODE ? op : ip, &data) > 0)
		{
			unsigned char*	u;
			unsigned char*	e;

			if (data.buf)
			{
				for (e = (u = (unsigned char*)data.buf) + data.size; u < e; u++)
					sfprintf(sfstderr, "%02x", *u);
				sfprintf(sfstderr, "\n");
			}
			else
				sfprintf(sfstderr, "%0*I*x\n", data.size * 2, sizeof(data.num), data.num);
		}
		codexpop(action == VC_ENCODE ? op : ip, 0);
	}
	else
		sfprintf(sfstdout, "\n");
	r = 1;
	goto done;
 bad:
	r = -1;
	goto done;
 nope:
	r = 0;
 done:
	if (ip && ip != sfstdin)
		sfclose(ip);
	if (op && op != sfstdout)
		sfclose(op);
	if (r > 0 && (flags & VC_remove))
	{
		if (chmod(output, si.st_mode))
			error(ERROR_SYSTEM|1, "%s: cannot set file mode", output);
		if (tmxtouch(output, tmxgetatime(&si), tmxgetmtime(&si), tmxgetctime(&si), 0))
			error(ERROR_SYSTEM|1, "%s: cannot set file times", output);
		if (remove(input))
			error(ERROR_SYSTEM|1, "%s: cannot remove input file", input);
	}
	if (pp)
		sfstrclose(pp);
	return r;
}

int
main(int argc, char** argv)
{
	int		action;
	int		c;
	char*		s;
	char*		input;
	char*		transform;
	Optdisc_t	optdisc;
	char		command[256];

	char*		buf = 0;
	char*		output = 0;
	size_t		bufsize = BUFFERSIZE;
	int		flags = 0;

	/* NOTE: disciplines may be accessed after main() returns */

	static Codexdisc_t	codexdisc;	/* codex discipline	*/

	error_info.id = (s = strrchr(argv[0], '/')) ? (s + 1) : argv[0];
	s = error_info.id;
	transform = command;
	action = VC_ENCODE;
	while (transform < &command[sizeof(command)-1] && (c = *s++) && isalnum(c) && (c = tolower(c)))
		if (c == 'a' && tolower(*s) == 't' && !*(s+1))
		{
			action = VC_DECODE;
			flags |= VC_cat;
			s = "zip";
		}
		else if (c == 'u' && tolower(*s) == 'n')
		{	
			action = VC_DECODE;
			s++;
		}
		else
			*transform++ = c;
	*transform = 0;
	transform = 0;
	codexinit(&codexdisc, errorf);
	optinit(&optdisc, optinfo);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'b':
			bufsize = (size_t)opt_info.number;
			continue;
		case 'c':
			action = VC_DECODE;
			flags |= VC_cat;
			continue;
		case 'f':
			flags |= VC_force;
			continue;
		case 'm':
			transform = opt_info.arg;
			continue;
		case 'o':
			output = opt_info.arg;
			continue;
		case 'p':
			codexdisc.flags |= CODEX_PLAIN;
			continue;
		case 'q':
			action = 0;
			codexdisc.identify = sfstdout;
			continue;
		case 's':
			codexdisc.source = opt_info.arg;
			continue;
		case 'u':
			action = VC_DECODE;
			continue;
		case 'w':
			codexdisc.window = opt_info.arg;
			continue;
		case 'D':
			error_info.trace = -(int)opt_info.num;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if ((input = *argv) && *++argv && output)
		error(3, "--output=%s: invalid when more than one file operand is specified", output);
	if (output && (flags & VC_cat))
		error(3, "--output=%s: invalid --cat is specified", output);
	while (bufsize > 0)
	{
		if (buf = malloc(bufsize))
			break;
		bufsize /= 2;
	}
	while (apply(action, flags, command, transform, input, output, buf, bufsize, &codexdisc) >= 0 && (input = *argv++));
	return error_info.errors != 0;
}
