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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <dsslib.h>

typedef struct State_s
{
	Cxunsigned_t	selected;
} State_t;

extern Dsslib_t		dss_lib_test;

static const char even_usage[] =
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \beven\b test query selects even ordinal records.]"
"\n"
"\n[ file ... ]\n"
"\n"
;

static int
even_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state;
	char*		s;
	int		errors;

	errors = error_info.errors;
	if (!(state = vmnewof(cx->vm, 0, State_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	expr->data = state;
	sfprintf(cx->buf, "%s%s", strchr(dss_lib_test.description, '['), even_usage);
	s = sfstruse(cx->buf);
	for (;;)
	{
		switch (optget((char**)data, s))
		{
		case '?':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			else
				return -1;
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
			else
				return -1;
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		return -1;
	sfprintf(sfstdout, "even_beg\n");
	return 0;
}

static int
even_sel(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state = (State_t*)expr->data;

	if (!(expr->queried & 1))
	{
		state->selected++;
		sfprintf(sfstdout, "even_sel %I*u\n", sizeof(expr->queried), expr->queried);
		return 1;
	}
	return 0;
}

static int
even_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	sfprintf(sfstdout, "even_act %I*u\n", sizeof(expr->queried), expr->queried);
	return 0;
}

static int
even_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state = (State_t*)expr->data;

	sfprintf(sfstdout, "even_end %I*u %I*u %I*u%s\n", sizeof(expr->queried), expr->queried, sizeof(expr->selected), expr->selected, sizeof(state->selected), state->selected, expr->selected == state->selected ? "" : " FAILED");
	return 0;
}

static const char odd_usage[] =
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \bodd\b test query selects odd ordinal records.]"
"\n"
"\n[ file ... ]\n"
"\n"
;

static int
odd_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state;
	char*		s;
	int		errors;

	errors = error_info.errors;
	if (!(state = vmnewof(cx->vm, 0, State_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	expr->data = state;
	sfprintf(cx->buf, "%s%s", strchr(dss_lib_test.description, '['), odd_usage);
	s = sfstruse(cx->buf);
	for (;;)
	{
		switch (optget((char**)data, s))
		{
		case '?':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			else
				return -1;
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
			else
				return -1;
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		return -1;
	sfprintf(sfstdout, "odd_beg\n");
	return 0;
}

static int
odd_sel(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state = (State_t*)expr->data;

	if (expr->queried & 1)
	{
		state->selected++;
		sfprintf(sfstdout, "odd_sel %I*u\n", sizeof(expr->queried), expr->queried);
		return 1;
	}
	return 0;
}

static int
odd_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	sfprintf(sfstdout, "odd_act %I*u\n", sizeof(expr->queried), expr->queried);
	return 0;
}

static int
odd_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state = (State_t*)expr->data;

	sfprintf(sfstdout, "odd_end %I*u %I*u %I*u%s\n", sizeof(expr->queried), expr->queried, sizeof(expr->selected), expr->selected, sizeof(state->selected), state->selected, expr->selected == state->selected ? "" : " FAILED");
	return 0;
}

static Cxquery_t	queries[] =
{
	{
		"even",
		"select even ordinal records",
		CXH,
		even_beg,
		even_sel,
		even_act,
		even_end,
	},
	{
		"odd",
		"select odd ordinal records",
		CXH,
		odd_beg,
		odd_sel,
		odd_act,
		odd_end,
	},
	{0}
};

Dsslib_t		dss_lib_test =
{
	"test",
	"test queries"
	"[-1lms5P?\n@(#)$Id: dss test queries (AT&T Research) 2003-09-22 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	0,
	0,
	0,
	0,
	0,
	&queries[0]
};
