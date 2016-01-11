/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2010-2011 AT&T Intellectual Property          *
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

static const char lookup_usage[] =
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The lookup checks if the \avariable\a operand value "
    "appears in the \afile\a operand, where \afile\a contains one value per "
    "line.]"
"\n"
"\nfile variable\n"
"\n"
;

#include <dsslib.h>
#include <dt.h>

typedef struct Value_s
{
	Dtlink_t	link;
	char		value[1];
} Value_t;

typedef struct State_s
{
	Dtdisc_t	dictdisc;
	Dt_t*		dict;
	Cxvariable_t*	variable;
	Vmalloc_t*	vm;
} State_t;

extern Dsslib_t		dss_lib_lookup;

static int
lookup_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**		argv = (char**)data;
	int		errors = error_info.errors;
	char*		s;
	State_t*	state;
	Sfio_t*		sp;
	Value_t*	v;
	Vmalloc_t*	vm;
	char		buf[PATH_MAX];

	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)) || !(state = vmnewof(vm, 0, State_t, 1, 0)))
	{
		if (vm)
			vmclose(vm);
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	state->vm = vm;
	state->dictdisc.key = offsetof(Value_t, value);
	if (!(state->dict = dtnew(vm, &state->dictdisc, Dtset)))
		goto bad;
	sfprintf(cx->buf, "%s%s", strchr(dss_lib_lookup.description, '['), lookup_usage);
	s = sfstruse(cx->buf);
	for (;;)
	{
		switch (optget(argv, s))
		{
		case '?':
			if (disc->errorf)
			{
				(*disc->errorf)(cx, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			}
			else
				goto bad;
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(cx, disc, 2, "%s", opt_info.arg);
			else
				goto bad;
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		goto bad;
	argv += opt_info.index;
	if (!argv[0] || !argv[1] || argv[2])
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, 2, "file and variable arguments expected");
		goto bad;
	}
	if (!(state->variable = cxvariable(cx, argv[1], NiL, disc)))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, 2, "%s: variable not defined", argv[1]);
		goto bad;
	}
	if (!(sp = dssfind(argv[0], NiL, DSS_VERBOSE, buf, sizeof(buf), disc)))
		goto bad;
	while (s = sfgetr(sp, '\n', SF_STRING))
	{
		if (!(v = vmnewof(vm, NiL, Value_t, 1, sfvalue(sp))))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
			goto bad;
		}
		strcpy(v->value, s);
		dtinsert(state->dict, v);
	}
	sfclose(sp);
	expr->data = state;
	return 0;
 bad:
	vmclose(vm);
	return -1;
}

static int
lookup_sel(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;
	Cxoperand_t		val;

	if (cxcast(cx, &val, state->variable, cx->state->type_string, data, NiL))
		return -1;
	return !!dtmatch(state->dict, val.value.string.data);
}

static int
lookup_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;

	vmclose(state->vm);
	return 0;
}

static Cxquery_t	queries[] =
{
	{
		"lookup",
		"look up variable value in file",
		CXH,
		lookup_beg,
		lookup_sel,
		0,
		lookup_end
	},
	{0}
};

Dsslib_t		dss_lib_lookup =
{
	"lookup",
	"lookup query"
	"[-1lms5P?\n@(#)$Id: dss lookup query (AT&T Research) 2010-04-20 $\n]"
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
