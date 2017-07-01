/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * data stream scan
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <ast.h>
#include <dsslib.h>
#include <ctype.h>
#include <error.h>

static const char usage[] =
"[-?\n@(#)$Id: dss (AT&T Research) 2012-08-15 $\n]"
USAGE_LICENSE
"[+NAME?dss - scan a data stream and apply a select expression to each "
    "record]"
"[+DESCRIPTION?\bdss\b scans a record-oriented data stream, applies a "
    "select \aexpression\a to each record, and writes the matching records "
    "to the standard output. If \aexpression\a is \b-\b or empty then all "
    "records match. If \aexpression\a begins with \b<\b then the remainder "
    "of the expression is a file containing the expression. If \afile\a is "
    "\b-\b or omitted then the standard input is read. If \afile\a begins "
    "with \b<\b then the string with \b<\b removed is a file containing a "
    "newline-separated list of files to process. If |{\bwrite\b \aformat\a}"
    "is not specified then the output format is the same as the format of "
    "the first input \afile\a.]"
"[+?Input files are \avczip\a(1), \agzip\a(1), \abzip2\a(1) or "
    "\apzip\a(1) decompressed if necessary.]"
"[+EXPRESSIONS?Query expressions support C-style syntax. Strings may be "
    "specified with '...', \"...\" or /.../ delimiters. \b=~\b and \b!~\b "
    "operations on strings treat the right hand operand as a possibly "
    "field-specific matching pattern. \bstring\b field matching patterns are "
    "extended \bregex\b(3) regular expressions (\begrep\b(1) style.)]"
"[+?Types, queries and functions are brought into scope by \b--library\b "
    "references on the command line, library references within schema "
    "definitions, or by explicit \alib\a::\aidentifier\a binding within "
    "expressions. Variables and function return values within expressions "
    "and \b{print}\b formats may be cast to another type using C style casts "
    "(\alib\a::\atype\a) and (\atype\a).]"
"[+?The default output format for struct type values is \bksh93\b(1) "
    "compound array notation with no embedded newlines: ( ... "
    "\aname\a=\avalue\a ... \aname\a=( ... \aname\a=\avalue\a ... ) ... ).]"
"[+?An expression of the form { \aquery\a [--\aoption\a...]] [\aarg\a...]] "
    "[> \aoutput\a]] } accesses the compiled \aquery\a defined in a "
    "\b--library\b dynamic library. { \aquery\a \b--man\b } lists the "
    "documentation for \aquery\a.]"
"[+?Dynamic queries enclosed in {...} and interpreted queries enclosed "
    "in (...) may be composed using the \b|\b, \b?:\b, and \b;\b operators. "
    "\aA\a|\aB\a specifies that query \aB\a processes records selected by "
    "query \aA\a. \aA\a?\aB\a:\aC\a specifies that query \aB\a processes the "
    "records selected by query \aA\a and query \aC\a processes the records "
    "not selected by query \aA\a; query \aB\a may be omitted. \aA\a;\aB\a "
    "specifies that queries \aA\a and \aB\a process the same records.]"
"{\fformats and variables\f}"
"[a:append?Open output files in append mode. This disables file header "
    "output.]"
"[d:debug?Enable debug tracing.]"
"[I:include?Add \adir\a to the list of directories searched for include "
    "files. The default \b../lib/dss\b directories on \b$PATH\b are added to "
    "this list.]:[dir]"
"[l:library?Load the dynamic library \aname\a. \bdss\b libraries may "
    "define methods, types, maps, queries or functions.]:[name]"
"[m:map?Numeric field value map XML file. \b--method=dss,man\b describes "
    "the \b<MAP>\b tag.]:[file]"
"[P:plugins?List plugin information for each \afile\a operand in "
    "--\astyle\a on the standard error. The \aexpression\a operand is not "
    "specified in this case. If no \afile\a operands are specified then the "
    "first instance of each \bdss\b plugin installed on \b$PATH\b or a "
    "sibling dir on \b$PATH\b is listed. The special \astyle\a \blist\b "
    "lists a line on the standard output for each plugin with the name, a "
    "tab character, and plugin specific command line options parameterized "
    "by \b${style}\b (suitable for \beval\b'ing in "
    "\bsh\b(1).)]:[style:=list|man|html|nroff|usage]"
"[q:quiet?Disable non-fatal warning messages.]"
"[v:verbose?Enable verbose status messages.]"
"[x:method?Set the record method. This option must be specified. The "
    "method name may be followed by a , separated list of method specific "
    "\a[no]]name[=value]]\a options and a schema following the first :. "
    "Method specific usage may be listed by the \bman\b or \bhtml\b method "
    "specific options. Each method is typically implemented in a separate "
    "shared library. If the method shared library is not installed on "
    "\b$PATH\b or a sibling dir on \b$PATH\b then \amethod\a must be the "
    "full path name of the shared library. The methods, determined by "
    "\b$PATH\b, are:]:[method[,option[=value]]...]][::schema]]]"
    "{\fmethods\f} [T:test?Enable test code defined by \amask\a. Test code "
    "is implementation specific.]#[mask]"
    "{"
        "[0x00f0?method specific tests]"
        "[0x0f00?cx library specific tests]"
        "[0x0100?enable cxeval() code trace]"
        "[0xf000?dss library specific tests]"
    "}"
"[c:count?Write the \amatched/total\a records on the standard output. "
    "Deprecated: compose the expression with |{\bcount\b} instead.]"
"[f:format?Set the output method format. Deprecated: compose the "
    "expression with |{\bwrite\b \aformat\a} instead.]:[format]"
"[n:nooutput?Disable all output. Deprecated: not required when the "
    "expression contains a dynamic query.]"
"[p:print?Print selected records according to \aformat\a. Deprecated: "
    "compose the expression with |{\bprint\b \aformat\a} instead.]:[format]"
"[+EXAMPLES]"
    "{"
        "[+dss -x dss '{stats --man}'?List the \bstats\b query manpage.]"
        "[+dss -x bgp '{write --man}'?List the formats supported by the "
            "\bbgp\b method.]"
        "[+dss -x netflow \"{stats --group=prot bytes packets}\" *.gz?List "
            "the stats for the \bnetflow\b method fields \bbytes\b and "
            "\bpackets\b, grouped by values in the field \bprot\b.]"
        "[+dss -x bgp {count} cisco.dat?Count the number of \bbgp\b "
            "records in \bcisco.dat\b.]"
        "[+dss -x bgp '(type==\"A\")??{write table > a}::{write cisco > "
            "b}' mrt.dat?Write the announce records from \bmrt.dat\b to the "
            "file \ba\b in the \btable\b format and all other records to the "
            "file \bb\b in the \bcisco\b format.]"
        "[+dss -x foo-txt '{flatten foo-bin}|{compress}' foo.txt > foo.bin?Convert "
            "the \bfoo-txt\b file \bfoo.txt\b to the \bfoo-bin\b \bflat\b "
            "method format file \bfoo.bin\b using the preferred compression "
            "method, where \bfoo-txt.dss\b and \bfoo-bin.dss\b are user "
            "supplied \bdss\b XML schema files describing the input and "
            "output formats.]"
        "[+dss -x foo-bin '(time>=\"jan 1\")|{flat foo-txt}' foo.bin?Select "
            "all \bfoo.bin\b records with \btime\b > jan 1 and list them in "
            "the \bfoo-txt\b format.]"
    "}"

"\n"
"\n [ expression ] [ file ... ]\n"
"\n"

"[+SEE ALSO?\bvczip\b(1), \bgzip\b(1), \bbzip2\b(1), \bpzip\b(1), \bcql\b(1), \bdss::dss\b(5P)]"
;

static void
showplugin(register Dsslib_t* lib, const char* style, Sfio_t* sp, int* sep, Dssdisc_t* disc)
{
	register int		i;
	register const char*	s;

	if (*sep < 0)
	{
		if (lib->meth)
			sfprintf(sfstdout, "%s\t--library=%s --method=%s,${style}\n", lib->meth->name, lib->name, lib->meth->name);
		if (lib->queries)
			for (i = 0; s = lib->queries[i].name; i++)
			{
				if (lib->meth && !streq(lib->meth->name, error_info.id))
					sfprintf(sfstdout, "%s-", lib->meth->name);
				else if (!streq(lib->name, error_info.id) && !streq(lib->name, s))
					sfprintf(sfstdout, "%s-", lib->name);
				sfprintf(sfstdout, "%s\t--library=%s --method=%s \"{%s --'${style}'}\" </dev/null\n", s, lib->name, error_info.id, s);
			}
	}
	else
	{
		if (*sep)
			sfputc(sfstderr, '\n');
		else
			*sep = 1;
		dssoptlib(sp, lib, NiL, disc);
		error(ERROR_USAGE, "%s", opthelp(sfstruse(sp), style));
	}
}

int
main(int argc, char** argv)
{
	int			status;
	int			sep;
	char*			action;
	char*			expression;
	char*			file;
	char*			plugins;
	Sfio_t*			tmp;
	Dss_t*			dss;
	Dssdisc_t		disc;
	Dssmeth_t*		meth;
	Dsslib_t*		lib;
	Dssstate_t*		state;
	Dssflags_t		flags;
	Dssflags_t		test;
	Dssoptdisc_t		optdisc;
	char			style[32];

	if (file = strrchr(*argv, '/'))
		file++;
	else
		file = *argv;
	error_info.id = file;
	state = dssinit(&disc, errorf);
	memset(&optdisc, 0, sizeof(optdisc));
	optdisc.optdisc.version = OPT_VERSION;
	optdisc.optdisc.infof = dssoptinfo;
	optdisc.disc = &disc;
	opt_info.disc = &optdisc.optdisc;
	action = 0;
	flags = 0;
	meth = 0;
	plugins = 0;
	status = 0;
	test = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			flags |= DSS_APPEND;
			continue;
		case 'c':
			action = "{count}";
			continue;
		case 'd':
			flags |= DSS_DEBUG;
			continue;
		case 'f':
			if (!(action = strdup(sfprints("{write %s}", fmtquote(opt_info.arg, "'", "'", strlen(opt_info.arg), FMT_ALWAYS|FMT_ESCAPED)))))
				error(ERROR_SYSTEM|3, "out of space");
			continue;
		case 'I':
			if (pathinclude(opt_info.arg))
				error(ERROR_SYSTEM|3, "out of space");
			continue;
		case 'l':
			if (!dssload(opt_info.arg, &disc))
				break;
			continue;
		case 'm':
			disc.map = opt_info.arg;
			continue;
		case 'n':
			action = "{null}";
			continue;
		case 'p':
			if (!(action = strdup(sfprints("{print %s}", fmtquote(opt_info.arg, "'", "'", strlen(opt_info.arg), FMT_ALWAYS|FMT_ESCAPED)))))
				error(ERROR_SYSTEM|3, "out of space");
			continue;
		case 'q':
			flags |= DSS_QUIET;
			continue;
		case 'v':
			flags |= DSS_VERBOSE;
			continue;
		case 'x':
			if (!(meth = dssmeth(opt_info.arg, &disc)))
				error(3, "%s: unknown method", opt_info.arg);
			continue;
		case 'P':
			sfsprintf(plugins = style, sizeof(style), "?%s", opt_info.arg);
			continue;
		case 'T':
			test = opt_info.num;
			continue;
		case '?':
			if (error_info.errors)
				return 1;
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	if (error_info.errors)
		return 1;
	argv += opt_info.index;
	if (plugins)
	{
		if (!(tmp = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space");
		sep = -streq(plugins, "?list");
		if (!*argv)
			for (lib = dsslib(NiL, DSS_VERBOSE, &disc); lib; lib = (Dsslib_t*)dtnext(state->cx->libraries, lib))
				showplugin(lib, plugins, tmp, &sep, &disc);
		else
			while (file = *argv++)
				if (lib = dsslib(file, DSS_VERBOSE, &disc))
					showplugin(lib, plugins, tmp, &sep, &disc);
		sfclose(tmp);
		return error_info.errors != 0;
	}
	if (!(expression = *argv++))
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(dss = dssopen(flags, test, &disc, meth)))
		return 1;
	if (dssrun(dss, expression, NiL, action, argv))
		status = 1;
	if (dssclose(dss))
		status = 1;
	return status;
}
