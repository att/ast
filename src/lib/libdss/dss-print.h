/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
 * print query
 */

static const char print_usage[] =
"[-1ls5P?\n@(#)$Id: dss print query (AT&T Research) 2011-08-19 $\n]"
USAGE_LICENSE
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \bdss\b \bprint\b query formats and prints the "
    "current record according to \aformat\a. If \aformat\a is omitted then "
    "the default method \bprint\b format is used; an error occurs if there "
    "is no default \bprint\b format.]"
"[+?\aformat\a follows \bprintf\b(3) conventions, except that "
    "\bsfio\b(3) inline ids are used instead of arguments: "
    "%[-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\adetails\a]])\achar\a.]"
"[+?If \achar\a is \bs\b then the string form of the value of \aid\a is "
    "listed, otherwise the corresponding numeric form is listed. If "
    "\awidth\a is omitted then the default width is assumed. \adetails\a "
    "optionally specify field type specific details. Documentation for "
    "format details appear with \bdss\b types that support them. For "
    "example, a \bstrftime\b(3) format for \btime_t\b fields, or a field "
    "separator for array fields.]"
"[+?A format specification %(:\adetails\a:)\achar\a (\aid\a omitted) "
    "causes no output but instead specifies default \adetails\a for all "
    "subsequent %...\achar\a. Multiple default specifications may appear.]"
"[+?The default print format is \fprint\f. The \bdf\b(1), \bls\b(1), "
    "\bpax\b(1) and \bps\b(1) commands have \b--format\b options in this "
    "same style.]"
"[a:all?Print the name and value of field, one per line, using the field "
    "default output format.]"
"\n"
"\n[ format ]\n"
"\n";

static int
print_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**	argv = (char**)data;
	int	all = 0;
	int	errors = error_info.errors;

	for (;;)
	{
		switch (optget(argv, print_usage))
		{
		case 'a':
			all = 1;
			continue;
		case '?':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		return -1;
	argv += opt_info.index;
	if (all)
		expr->data = 0;
	else if (!(expr->data = *argv++))
	{
		argv--;
		if (!(expr->data = (char*)DSS(cx)->meth->print))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: no default method print format", DSS(cx)->meth->name);
			return -1;
		}
	}
	if (*argv)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", optusage(NiL));
		return -1;
	}
	return cx->referencef ? dssprintf(DSS(cx), cx, 0, (char*)expr->data, NiL) : 0;
}

static int
print_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	return dssprintf(DSS(cx), cx, expr->op, (char*)expr->data, data);
}

static int
print_ref(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char*	s;
	char**	a;

	if (cx->referencef && (a = (char**)data) && *a++)
		while (s = *a++)
			if (*s != '-')
				return dssprintf(DSS(cx), cx, 0, s, NiL);
			else if (*(s + 1) == '-' && !*(s + 2))
				return *a ? dssprintf(DSS(cx), cx, 0, *a, NiL) : 0;
	return 0;
}

#define QUERY_print \
	{ \
		"print", \
		"format and print the current record", \
		CXH, \
		print_beg, \
		0, \
		print_act, \
		0, \
		0, \
		print_ref, \
		0 \
	}
