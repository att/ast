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
 * count query
 */

static const char count_usage[] = 
"[-1ls5P?\n@(#)$Id: dss count query (AT&T Research) 2002-12-09 $\n]"
USAGE_LICENSE
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \bdss\b \bcount\b query prints the parent expression"
"	selected/queried record counts. The output line is preceded by"
"	\alabel:\a if the \alabel\a operand is specified.]"
"\n"
"\n [ label ]\n"
"\n";

static int
count_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**	argv = (char**)data;
	int	errors = error_info.errors;

	for (;;)
	{
		switch (optget(argv, count_usage))
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
	if ((expr->data = *argv++) && *argv)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", optusage(NiL));
		return -1;
	}
	return 0;
}

static int
count_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	if (expr->data)
		sfprintf(expr->op, "%s: ", (char*)expr->data);
	sfprintf(expr->op, "%I*u/%I*u\n", sizeof(expr->parent->selected), expr == expr->parent->fail ? (expr->parent->queried - expr->parent->selected) : expr->parent->selected, sizeof(expr->parent->queried), expr->parent->queried);
	return 0;
}

#define QUERY_count \
	{ \
		"count", \
		"print parent expression record counts", \
		CXH, \
		count_beg, \
		0, \
		0, \
		count_end \
	}
