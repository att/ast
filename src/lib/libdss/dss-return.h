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
 * return "query"
 */

static const char return_usage[] =
"[-1ls5P?\n@(#)$Id: dss return query (AT&T Research) 2007-09-21 $\n]"
USAGE_LICENSE
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \bdss\b \breturn\b query returns from the current"
"	record query.  If \astatus\a is omitted or \bskip\b the current"
"	record is skipped, if \astatus\a is \bselect\b the current"
"	record is selected, otherwise if \astatus\a is \bterminate\b"
"	the scan is terminated and \bdss\b returns exit status \b1\b.]"
"\n"
"\n [ status ]\n"
"\n"
;

static int
return_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**	argv = (char**)data;
	int	errors = error_info.errors;

	for (;;)
	{
		switch (optget(argv, return_usage))
		{
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
	if (*argv && *(argv + 1))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", optusage(NiL));
		return -1;
	}
	return 0;
}

static int
return_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char*	s;
	int	r;

	if (!(s = expr->argv[1]) || streq(s, "skip"))
		r = -2;
	else if (streq(s, "select"))
		r = 0;
	else
		r = -1;
	return r;
}

#define QUERY_return \
	{ \
		"return", \
		"return from the current record query", \
		CXH, \
		return_beg, \
		0, \
		return_act, \
		0 \
	}
