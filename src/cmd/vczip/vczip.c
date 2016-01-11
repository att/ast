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
*                   Phong Vo <kpv@research.att.com>                    *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
/*	Command to encode and decode with Vcodex methods.
**
**	Written by Kiem-Phong Vo (09/06/2003 - ILNGUYEN)
*/

#if _PACKAGE_ast

#define UNZIP		((char*)0)
#define DISABLED	((char*)disabled)
#define DFLTZIP		"delta,huffgroup"
#define ALIASES		"lib/vcodex/aliases"
#define VCZIPRC		".vcziprc"
#define VCZIP		"vczip"

static const char usage[] =
"[-?\n@(#)$Id: vczip (AT&T Research) 2009-10-01 $\n]"
USAGE_LICENSE
"[+NAME?vczip - vcodex method encode/decode filter]"
"[+DESCRIPTION?\bvczip\b is a filter that decodes the standard input "
    "and/or encodes the standard output. The \b--method\b option specifies "
    "the encoding. The default encoding is \b--method=" DFLTZIP "\b. The "
    "method is automatically determined when decoding.]"
"[+?For delta methods the \asource\a operand optionally specifies the "
    "file to delta against. If \asource\a is omitted then the input file is "
    "simply compressed. Delta-encoded data must be decoded with the same "
    "\asource\a.]"
"[+?Method aliases may be defined in \b../" ALIASES "\b in one of the "
    "directories on \b$PATH\b, or in \b$HOME/" VCZIPRC "\b, searched in "
    "order. Each alias is a \aname=value\a pair where \avalue\a is a "
    "\b--method\b option value, described below. Method names are searched "
    "before alias names.]"
"[i:input?Input data is read from \afile\a instead of the standard "
    "input.]:[file]"
"[m:method|encode?Set the transformation method from the \b,\b (or \b^\b) "
    "separated list of \amethod\a[.\aarg\a]] elements, where \amethod\a is "
    "a method alias or primitive method, \aarg\a is an optional method "
    "specific argument, and \b-\b denotes the default argument. Parenthesized "
    "values are implementation details. The \alibvcodex\a (\b-catalog\b) "
    "denotes a method supplied by the default library. Otherwise the method "
    "is a separate plugin; that plugin will be required to decode any "
    "encoded data. The primitive methods and method aliases "
    "are:]:[method[.arg]][,method[.arg]]...]]:=" DFLTZIP "]"
    "{\fvcodex\f[vcdiff|ietf?Encode as defined in IETF RFC3284.]}"
"[o:output?Output data is written to \afile\a instead of the standard "
    "output.]:[file]"
"[p:plain?Do not encode transformation information in the data. This means "
    "the transform must be explicitly supplied to decode.]"
"[q:identify?Identify the standard input encoding and write the "
    "\b--method\b transformation string on the standard output. "
    "If the standard input is not vczip encoded then nothing is "
    "printed.]"
"[t:transform?Apply the \bcodex\b(3) data transform around the "
    "\bvcodex\b(3) transform. A codex transform is a catenation of the "
    "following methods, each method prefixed by \b<\b to decode the input or "
    "\b>\b to encode the output. Method arguments, if any, must be prefixed "
    "by \b-\b. The method are:]:[[<>]]method[-arg...]]...]"
    "{\fcodex\f}"
"[u:undo|decode?Decode data.]"
"[v:verbose?List the compresses size on the standard error.]"
"[w:window?Set the data partition window size to \awindow\a. "
    "\amethod\a specifies an optional window matching "
    "method. The window methods are:]:[window[,method]]]"
    "{\fwindows\f}"
"[d:vcdiff|ietf?Encode as defined in IETF RFC3284. Obsolete -- use "
    "--method=ietf.]"
"[D:debug?Set the debug trace level to \alevel\a. Higher levels produce "
    "more output.]:[level]"
"[M:move?Use sfmove() for io.]"
"[P:pause?Debug: print the pid and sleep(10); then continue processing.]"

"\n"
"\n[ source ] < input > output\n"
"\n"

"[+FILES]"
    "{"
        "[+../" ALIASES "?\b--method\b \aname=value\a alias file, found "
            "on \b$PATH\b.]"
    "}"
"[+SEE ALSO?\bcodex\b(1), \bcodex\b(3), \bvcodex\b(3)]"
;

#include	<ast.h>
#include	<error.h>
#include	<ccode.h>
#include	<ctype.h>
#include	<vcodex.h>
#include	<codex.h>

static const char	disabled[] = "disabled";

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
			sfprintf(sp, "[+%s\b", meth->name);
			p = " (";
			if (meth->identf)
			{
				sfprintf(sp, "%sident", p);
				p = ",";
			}
			if (!(meth->flags & CODEX_ENCODE))
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
	case 'v':
		/* vcodex methods */
		vcwalkmeth(optmethod, sp);
		/* aliases */
		vcwalkalias(optalias, sp);
		break;
	case 'w':
		/* vcodex window methods */
		vcwwalkmeth(optwindow, sp);
		break;
	}
	return 0;
}

static void
vcsferror(const char* mesg)
{	
	error(2, "%s", mesg);
}

/*
 * apply the codex and/or vcodex transforms to a single input/output pair
 */

static void
apply(int action, const char* vt, Vcsfdata_t* vcodexdisc, const char* ct, Codexdisc_t* codexdisc, const char* input, const char* source, const char* output, void* buf, size_t bufsize, Sfoff_t donez, Sfoff_t lastz)
{
	Sfio_t*		ip;
	Sfio_t*		op;
	ssize_t		n;

	/*
	 * set up the sfio input stream
	 */

	if (!input || !*input || streq(input, "-") || streq(input, "/dev/stdin") || streq(input, "/dev/fd/0"))
	{
		input = "/dev/stdin";
		ip = sfstdin;
		sfset(ip, SF_SHARE, 0);
		sfopen(ip, NiL, "rb");
	}
	else if (!(ip = sfopen(NiL, input, "rb")))
	{
		error(ERROR_SYSTEM|2, "%s: cannot read", input);
		return;
	}

	/*
	 * set up the sfio output stream
	 */

	if (!action)
	{
		sfprintf(sfstdout, "%s: ", input);
		output = "/dev/null";
		codexdisc->identify = sfstdout;
	}
	if (!output || !*output || streq(output, "-") || streq(output, "/dev/stdout") || streq(output, "/dev/fd/1"))
	{
		output = "/dev/stdout";
		op = sfstdout;
		sfset(op, SF_SHARE, 0);
		sfopen(op, NiL, "wb");
	}
	else if (!(op = sfopen(NiL, output, "wb")))
	{
		error(ERROR_SYSTEM|2, "%s: cannot write", output);
		if (ip != sfstdin)
			sfclose(ip);
		return;
	}

	/*
	 * check codex sfio discipline
	 */

	error(-1, "AHA action=%s input=%s source=%s output=%s vt=%s ct=%s", action == VC_ENCODE ? "encode" : "decode", input, source, output, vt, ct);
	if (ct != DISABLED)
	{
		if ((n = codex(ip, op, ct, action == VC_ENCODE ? CODEX_ENCODE : CODEX_DECODE, codexdisc, NiL)) < 0)
			error(3, "%s: cannot push codex io stream discipline", input);
		if (!action && n > 0)
			sfprintf(sfstdout, ",");
	}

	/*
	 * check vcodex sfio discipline
	 */

	if (vt != DISABLED)
	{	
		vcodexdisc->trans  = (char*)vt;
		vcodexdisc->source = (char*)source;
		if (!vcsfio(action == VC_ENCODE ? op : ip, vcodexdisc, action))
			error(3, "%s: cannot push vcodex io stream discipline", input);
		else if (!action)
			sfprintf(sfstdout, "%s", vcodexdisc->trans);
	}

	/*
	 * copy from ip to op
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
				if (donez >= 0) /* verbose mode */
				{	
					if (donez >= lastz + 64 * (bufsize > 1024*1024 ? bufsize : 1024*1024)) 
					{	
						sfprintf(sfstderr, "done %10I*d %s\n", sizeof(donez), donez, output);
						lastz = donez;
					}
					donez += n;
				}
				if ((n = sfwrite(op, buf, n)) < 0)
				{
					error(ERROR_SYSTEM|2, "%s: write error", output);
					break;
				}
			}
		else
		{
			sfmove(ip, op, SF_UNBOUND, -1);
			if (!sfeof(ip))
				error(ERROR_SYSTEM|2, "%s: read error", input);
			else if (sfsync(op) || sferror(op))
				error(ERROR_SYSTEM|2, "%s: write error", output);
		}
	}
	else
		sfprintf(sfstdout, "\n");
	if (ip != sfstdin)
		sfclose(ip);
	if (op != sfstdout)
		sfclose(op);
}

int
main(int argc, char** argv)
{
	Vcchar_t	*buf;
	size_t		bufsize;
	int		action;			/* default is encoding	*/
	int		move = 0;		/* sfmove()		*/
	int		vczip;			/* are we vczip?	*/
	int		c;
	char*		vt;			/* vcodex transform	*/
	char*		ct;			/* codex transform	*/
	Sfio_t*		cs;			/* codex transform buf	*/
	Sfoff_t		donez = -1, lastz = -1;	/* amount processed	*/
	Optdisc_t	optdisc;		/* optget() dscipline	*/
	char		name[256];		/* method name buffer	*/

	/* NOTE: disciplines may be accessed after main() returns */

	static Vcsfdata_t	vcodexdisc;	/* vcodex discipline	*/
	static Codexdisc_t	codexdisc;	/* codex discipline	*/

	error_info.id = (ct = strrchr(argv[0], '/')) ? (ct + 1) : argv[0];
	if (!(cs = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space");
	action = VC_ENCODE;
	ct = error_info.id;
	vt = name;
	while (vt < &name[sizeof(name)-1] && (c = tolower(*ct++)))
		if (c == 'u' && tolower(*ct) == 'n')
		{	action = VC_DECODE;
			ct++;
		}
		else
			*vt++ = c;
	*vt = 0;
	vczip = streq(name, VCZIP);
	if (!vczip && codexmeth(name))
	{	
		vt = DISABLED;
		sfprintf(cs, "%c%s", action == VC_DECODE ? '<' : '>', name);
	}
	else if (action == VC_DECODE)
		vt = UNZIP;
	else
		vt = DFLTZIP;
	memset(&vcodexdisc, 0, sizeof(vcodexdisc));
	vcodexdisc.errorf = vcsferror;
	codexinit(&codexdisc, errorf);
	optinit(&optdisc, optinfo);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			vcodexdisc.type = VCSF_VCDIFF;
			continue;
		case 'i':
			if (sfopen(sfstdin, opt_info.arg, "r") != sfstdin)
				error(ERROR_SYSTEM|3, "%s: cannot read", opt_info.arg);
			continue;
		case 'm':
			if (streq(opt_info.arg, "ietf") || streq(opt_info.arg, "vcdiff"))
				vcodexdisc.type = VCSF_VCDIFF;
			else if (streq(opt_info.arg, "-"))
				vt = DISABLED;
			else
				vt = opt_info.arg;
			continue;
		case 'o':
			if (sfopen(sfstdout, opt_info.arg, "w") != sfstdout)
				error(ERROR_SYSTEM|3, "%s: cannot write", opt_info.arg);
			continue;
		case 'p':
			vcodexdisc.type = VCSF_PLAIN;
			continue;
		case 'q':
			action = 0;
			continue;
		case 't':
			if (*opt_info.arg != '<' && *opt_info.arg != '>')
				sfprintf(cs, "%c", action == VC_DECODE ? '<' : '>');
			sfprintf(cs, "%s", opt_info.arg);
			continue;
		case 'u':
			action = VC_DECODE;
			if (vt)
				vt = UNZIP;
			continue;
		case 'v':
			donez = lastz = 0;
			continue;
		case 'w':
			vcodexdisc.window = opt_info.arg;
			continue;
		case 'D':
			error_info.trace = -(int)opt_info.num;
			continue;
		case 'M':
			move = 1;
			continue;
		case 'P':
			error(1, "pid %d", getpid());
			sleep(10);
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
	if (error_info.errors || argv[0] && argv[1])
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	buf = 0;
	if (!move)
		for (bufsize = 1024*1024; bufsize > 0; bufsize /= 2)
			if (buf = malloc(bufsize))
				break;
	if (!(ct = sfstruse(cs)))
		error(ERROR_SYSTEM|3, "out of space");
	if (streq(ct+1, "-"))
		ct = DISABLED;
	else if (!*(ct+1))
		ct = UNZIP;
	apply(action, vt, &vcodexdisc, ct, &codexdisc, NiL, *argv, NiL, buf, bufsize, donez, lastz);
	return error_info.errors != 0;
}

#else

#include	"vchdr.h"

#define	PR_METHOD	((Void_t*)1)
#define	PR_ALIAS	((Void_t*)2)

#define DFLTZIP	"sieve.delta,bwt,mtf,rle.0,huffgroup"

static char	*Mesg[] =
{
	"vczip [-Arguments] [SourceFile] < Input > Output\n",

	"\nBelow are the standard 'Arguments':\n",
	"-?: This prints this message.\n",
	"-i[InputFile]: This redefines the standard 'Input' to be 'InputFile'.\n",
	"-o[OutputFile]: This redefines the standard 'Output' to be 'OutputFile'.\n",
	"-w[size[.alg]]: This argument is ignored during decoding (i.e., -u).\n",
	"    On encoding, it defines the window processing for a large input file.\n",
	"    'size' is the window size, i.e., the size to break the input file into\n",
	"	chunks for processing. Units 'k' and 'm' mean kilo and megabytes.\n",
	"    'alg' selects a windowing matching algorithm for delta compression:\n",
	"	p: matching windows by a prefix matching method (default) or\n",
	"	m: matching windows by simply mirroring file positions.\n",
	"-E[type]: This translates between EBCDIC and ASCII during encoding or\n",
	"    decoding via the given 'type'. See also the 'map' transform below.\n",
	"-vcdiff: This encodes data as defined in IETF RFC3284.\n",
	"-plain: This means that the transformed data will not have information\n",
	"    about the transformation used for encoding. So, that information\n",
	"    will have to be supplied explicitly on decoding.\n", 
	"-u[transformation]: This decodes some previously encoded data.\n",
	"    If 'transformation' is not empty, it is the transformation used\n",
	"    to encode data. The data to be decoded will be treated as if it\n",
	"    was encoded with '-plain'. That is, '-utransformation' is the same\n",
	"    as '-u -plain -mtransformation'.\n",
	"-m[transformation]: A 'transformation' is defined as a comma-separated list:\n",
	"	transform1[.arg11.arg12...],transform2[.arg21.arg22],...'\n",
	"    For example, '-mbwt,mtf,rle.0,huffgroup' defines a transformation that\n"
	"    compresses data based on the Burrows-Wheeler transform. When the first\n",
	"    transform in a transformation is a delta compressor such as 'delta' or\n",
	"    'sieve', a 'SourceFile' can help enhancing compression. In that case,\n"
	"    the same source file must be given on both encoding and decoding.\n",
	0
};

static char	*Program;	/* name of executable	*/

static void error(const char* mesg, ...)
{
	va_list	args;
	va_start(args, mesg);
	sfprintf(sfstderr,"%s: ",Program);
	sfvprintf(sfstderr,mesg,args);
	sfprintf(sfstderr,"\n");
	va_end(args);
	exit(1);
}

static void vcsferror(const char* mesg)
{	sfprintf(sfstderr,"%s: %s\n", Program, mesg);
}

static int printdesc(Void_t* obj, char* name, char* value, Void_t* type)
{
	Vcmtarg_t	*arg;

	if(type == PR_METHOD)
	{	if(!obj)
			return -1;
		sfprintf(sfstderr,"%s: %s.\n", name, value);
		for(arg = ((Vcmethod_t*)obj)->args; arg && arg->name; ++arg)
			sfprintf(sfstderr, " %12s: %s.\n", arg->name, arg->desc);
		if(arg && !arg->name && arg->desc)
			sfprintf(sfstderr, " %12s: %s.\n", "None", arg->desc);
	}
	else if(type == PR_ALIAS)
		sfprintf(sfstderr, "-%s: %s.\n", name, value);
	else	return -1;

	return 0;
}

static void printmesg()
{
	int		i;

	for(i = 0; Mesg[i]; ++i)
		sfprintf(sfstderr,"%s", Mesg[i]);

	sfprintf(sfstderr, "\nThe default transformation is %s.\n", DFLTZIP);
	sfprintf(sfstderr, "Below are short-hands for common transformations:\n");
	vcwalkalias(printdesc, PR_ALIAS);

	/* print the set of primitive methods */
	sfprintf(sfstderr, "\nBelow are the available transforms and their arguments:\n");
	vcwalkmeth(printdesc, PR_METHOD);
}

int
main(int argc, char** argv)
{
	Vcchar_t	*data, *dt;
	ssize_t		dtsz, n;
	char		buf[1024];
	Vcsfdata_t	sfdt;			/* data passed to vcsf	*/
	Vcsfio_t	*sfio = NIL(Vcsfio_t*);	/* IO handle		*/
	Vcodex_t	*eavc = NIL(Vcodex_t*);	/* ebcdic <-> ascii	*/
	int		action = VC_ENCODE;	/* default is encoding	*/
	int		type = 0;		/* type of processing	*/
	char		*trans = DFLTZIP;	/* transformation spec	*/
	char		*window = NIL(char*);	/* window specification	*/
	ssize_t		donez = -1, lastz = -1;	/* amount processed	*/

	/* get program name */
	for(Program = argv[0]+strlen(argv[0]); Program > argv[0]; --Program)
		if(Program[-1] == '/')
			break;

	/* make list of default aliases */
	vcaddalias(Dfltalias);

	for(; argc > 1 && argv[1][0] == '-'; argc--, argv++)
	{	switch(argv[1][1])
		{
		case '?':
			printmesg();
			return 0;
		case 'o':
		case 'i':
		case 'S':
			if(argv[1][1] == 'S')
				; /* state file has been made obsolete */
			else if(argv[1][2] == 0)
				error("No file was given for %s.", argv[1]);
			else if(argv[1][1] == 'i')
			{	if(sfopen(sfstdin, argv[1]+2, "r") != sfstdin)
					error("Can't open input file '%s'.", argv[1]+2);
			}
			else
			{	if(sfopen(sfstdout, argv[1]+2, "w") != sfstdout)
					error("Can't open output file '%s'.", argv[1]+2);
			}
			break;
		case 'w':
			window = argv[1]+2;
			break;
		case 'E': /* ebcdic <-> ascii translation */
			if(eavc)
				vcclose(eavc);
			if(!(eavc = vcopen(0, Vcmap, argv[1]+2, 0, VC_ENCODE)) )
				error("'%s' specifies bad translation mode.", argv[1]);
			break;
		case 'v':
			if(strcmp(argv[1]+1,"vcdiff") == 0)
				type = VCSF_VCDIFF;
			else	goto dflt_arg;
			break;
		case 'p':
			if(strcmp(argv[1]+1,"plain") == 0)
				type = VCSF_PLAIN;
			else	goto dflt_arg;
			break;
		case 'u':
			action = VC_DECODE;
			if(argv[1][2])
			{	type = VCSF_PLAIN;
				trans = argv[1]+2;
			}
			break;
		case 'm':
			trans = argv[1]+2;
			break;
		case 'V':
			donez = lastz = 0;
			break;
		default:
		dflt_arg:
			trans = vcgetalias(argv[1]+1, buf, sizeof(buf));
			if(!trans || trans == argv[1]+1)
				error("'%s' is invalid. Use '-?' for help.", argv[1]);
		}
	}

	if(strcmp(Program, "vcunzip") == 0)
		action = VC_DECODE;
	else if(strncmp(Program, "vczip", 5) != 0 )
		error("Program name should be vczip or vcunzip");

	if(sfsize(sfstdin) == 0) /* a potentially empty data stream */
	{	Void_t *data;

		/* see if this is just a pipe showing up initially empty */
		if(!(data = sfreserve(sfstdin, -1, SF_LOCKR)) || sfvalue(sfstdin) == 0 )
			return 0; /* empty data transforms to empty output */
		else	sfread(sfstdin, data, 0); /* reset stream for normal transformation */
	}

	/* turn off share mode to avoid peeking on unseekable devices */
	sfset(sfstdin, SF_SHARE, 0);
	sfset(sfstdout, SF_SHARE, 0);

#if _WIN32 /* on Windows systems, use binary mode for file I/O */
	setmode(0, O_BINARY);
	setmode(1, O_BINARY);
#endif

	/* open stream for data processing */
	sfdt.type   = type;
	sfdt.trans  = trans;
	sfdt.source = argc == 2 ? argv[1] : NIL(char*);
	sfdt.window = window;
	sfdt.errorf = vcsferror;
	if(!(sfio = vcsfio(action == VC_ENCODE ? sfstdout : sfstdin, &sfdt, action)) )
		error("Can't set up stream to encode or decode data.");

	/* get buffer for IO */
	data = NIL(Void_t*);
	for(dtsz = 1024*1024; dtsz > 0; dtsz /= 2)
		if((data = (Void_t*)malloc(dtsz)) )
			break;
	if(!data)
		error("Can't allocate I/O buffer.");

	for(;;)
	{	if(action == VC_DECODE) /* get a chunk of data */
			n = vcsfread(sfio, data, dtsz);
		else	n = sfread(sfstdin, data, dtsz);
		if(n <= 0)
			break;

		if(donez >= 0) /* verbose mode */
		{	if(donez >= lastz + 64*(dtsz > 1024*1024 ? dtsz : 1024*1024)) 
			{	sfprintf(sfstderr, "Done %d\n", donez);
				lastz = donez;
			}
			donez += n;
		}

		if(!eavc) /* do any ascii <-> ebcdic mapping required */
			dt = data;
		else if((n = vcapply(eavc, data, n, &dt)) <= 0)
			error("Byte mapping failed.");

		if(action == VC_DECODE) /* write out the data */
			n = sfwrite(sfstdout, dt, n);
		else	n = vcsfwrite(sfio, dt, n);
		if(n <= 0)
			error("Error writing out data.");
	}
	vcsfclose(sfio);

	return 0;
}

#endif
