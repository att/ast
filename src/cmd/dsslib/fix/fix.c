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

static const char usage[] =
"[+DESCRIPTION?The fix query generates a fixed binary flat schema from the"
"	input file schema on the standard output. If the input schema is"
"	variable width then the input file is used to estimate average and"
"	maximum field widths. The generated schema may be used as input for"
"	the \bflat\b method conversion query.]"
"[s:stamp?The identification date stamp.]:[stamp:=YYYY-MM-DD]"
;

#include <dsslib.h>
#include <tm.h>

#define FIELD_string	0
#define FIELD_number	1
#define FIELD_buffer	2

struct Field_s; typedef struct Field_s Field_t;
struct State_s; typedef struct State_s State_t;

struct Field_s
{
	Dtlink_t	link;
	Field_t*	train;
	Cxvariable_t*	variable;
	Cxnumber_t	max;
	size_t		width;
	int		representation;
	int		flags;
};

struct State_s
{
	Dtdisc_t	fielddisc;
	Vmalloc_t*	vm;
	Field_t*	train;
	Dt_t*		fields;
	char*		stamp;
};

static int
fieldcmp(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	register Field_t*	fa = (Field_t*)a;
	register Field_t*	fb = (Field_t*)b;

	if (fa->representation < fb->representation)
		return -1;
	if (fa->representation > fb->representation)
		return 1;
	if (fa->width < fb->width)
		return 1;
	if (fa->width > fb->width)
		return -1;
	return strcmp(fa->variable->name, fb->variable->name);
}

extern Dsslib_t	dss_lib_fix;

static int
fix_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**		argv = (char**)data;
	int		errors = error_info.errors;
	char*		s;
	State_t*	state;
	Cxvariable_t*	variable;
	Field_t*	field;
	Vmalloc_t*	vm;

	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)) || !(state = vmnewof(vm, 0, State_t, 1, 0)))
	{
		if (vm)
			vmclose(vm);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	state->vm = vm;
	if (dssoptlib(cx->buf, &dss_lib_fix, usage, disc))
		goto bad;
	s = sfstruse(cx->buf);
	for (;;)
	{
		switch (optget(argv, s))
		{
		case 's':
			if (!(state->stamp = strdup(opt_info.arg)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			continue;
		case '?':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			else
				goto bad;
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
			else
				goto bad;
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		goto bad;
	argv += opt_info.index;
	state->fielddisc.comparf = fieldcmp;
	if (!(state->fields = dtnew(vm, &state->fielddisc, Dtoset)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	for (variable = (Cxvariable_t*)dtfirst(cx->fields); variable; variable = (Cxvariable_t*)dtnext(cx->fields, variable))
	{
		if (!(field = vmnewof(vm, 0, Field_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			goto bad;
		}
		field->variable = variable;
		switch (variable->type->representation)
		{
		case CX_buffer:
			field->representation = FIELD_buffer;
			break;
		case CX_string:
			field->representation = FIELD_string;
			break;
		default:
			field->representation = FIELD_number;
			break;
		}
		if (field->width = variable->format.width)
		{
			field->flags = variable->format.flags;
			dtinsert(state->fields, field);
		}
		else if (field->width = variable->type->format.width)
		{
			field->flags = variable->type->format.flags;
			dtinsert(state->fields, field);
		}
		else
		{
			field->train = state->train;
			state->train = field;
		}
	}
	expr->data = state;
	return 0;
 bad:
	vmclose(vm);
	return -1;
}

static int
fix_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;
	register Field_t*	field;
	Cxoperand_t		arg;

	for (field = state->train; field; field = field->train)
	{
		if (cxcast(cx, &arg, field->variable, field->variable->type, data, NiL))
			return -1;
		if (field->representation == FIELD_number)
		{
			if (arg.value.number < 0)
			{
				arg.value.number = -arg.value.number;
				field->flags = CX_UNSIGNED;
			}
			if (field->max < arg.value.number)
				field->max = arg.value.number;
		}
		else if (field->width < arg.value.buffer.size)
		{
			error(-1, "AHA fix_act %s.width %d  arg.size %d", field->variable->name, field->width, arg.value.buffer.size);
			field->width = arg.value.buffer.size;
		}
	}
	return 0;
}

static int
fix_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;
	register Field_t*	field;
	register size_t		w;
	register size_t		x;
	register size_t		r;
	register size_t		b;
	Dsslib_t*		lib;
	char*			s;

	b = 0;
	for (field = state->train; field; field = field->train)
	{
		if (field->representation == FIELD_number)
		{
			if (field->max != (Cxinteger_t)field->max)
			{
				field->width = 8;
				field->flags = CX_FLOAT;
			}
			else
			{
				field->flags ^= CX_UNSIGNED|CX_INTEGER;
				if (field->max > (unsigned long)0xffffffff)
					field->width = 8;
				else if (field->max > (unsigned long)0xffff)
					field->width = 4;
				else
					field->width = 2;
			}
		}
		else
		{
			if ((w = (field->width * 3) / 2) < 8)
				w = 8;
			else if (w < (1<<15))
				for (x = w, w = 1; w < x; w <<= 1);
			error(-1, "AHA fix_end %s.width %d w %d", field->variable->name, field->width, w);
			if (field->representation == FIELD_string)
				field->width = w;
			else
			{
				b += w;
				field->width = 4;
			}
		}
		dtinsert(state->fields, field);
	}
	r = 0;
	for (field = (Field_t*)dtfirst(state->fields); field; field = (Field_t*)dtnext(state->fields, field))
		r += field->width;
	x = r;
	if ((r += 2 * b) < 16)
		r = 16;
	else if (r < (1<<15))
		for (w = r, r = 1; r < w; r <<= 1);
	sfprintf(expr->op, "<METHOD>flat</>\n");
	sfprintf(expr->op, "<FLAT>\n");
	sfprintf(expr->op, "	<NAME>%s</>\n", DSS(cx)->meth->name);
	s = (char*)DSS(cx)->meth->description;
	if ((w = strlen(s)) && s[w-1] == '.')
		w--;
	sfprintf(expr->op, "	<DESCRIPTION>%-*.*s fixed width binary format.</>\n", w, w, s);
	if (!state->stamp)
		state->stamp = fmttime("%Y-%m-%d", time(NiL));
	sfprintf(expr->op, "	<IDENT>@(#)$Id: %s bin %s $</>\n", DSS(cx)->meth->name, state->stamp);
	sfprintf(expr->op, "	<MAGIC>\n");
	sfprintf(expr->op, "		<STRING>%s</>\n", DSS(cx)->meth->name);
	sfprintf(expr->op, "		<VERSION>");
	for (s = state->stamp; *s; s++)
		if (isdigit(*s))
			sfputc(expr->op, *s);
	sfprintf(expr->op, "</>\n");
	sfprintf(expr->op, "		<SWAP>be</>\n");
	sfprintf(expr->op, "	</>\n");
	sfprintf(expr->op, "	<COMPRESS>pzip %s-bin</>\n", DSS(cx)->meth->name);
	w = 0;
	for (lib = (Dsslib_t*)dtfirst(cx->state->libraries); lib; lib = (Dsslib_t*)dtnext(cx->state->libraries, lib))
		if (lib->types && !lib->meth)
		{
			sfprintf(expr->op, "	<LIBRARY>%s</>\n", lib->name);
			if (!w && streq(lib->name, "num_t"))
				w = 1;
		}
	if (!w)
		sfprintf(expr->op, "	<LIBRARY>num_t</>\n");
	for (field = (Field_t*)dtfirst(state->fields); field; field = (Field_t*)dtnext(state->fields, field))
	{
		sfprintf(expr->op, "	<FIELD>\n");
		sfprintf(expr->op, "		<NAME>%s</>\n", field->variable->name);
		sfprintf(expr->op, "		<DESCRIPTION>%s</>\n", field->variable->description);
		sfprintf(expr->op, "		<TYPE>%s</>\n", field->variable->type->name);
		sfprintf(expr->op, "		<PHYSICAL>\n");
		switch (field->representation)
		{
		case FIELD_buffer:
			sfprintf(expr->op, "			<TYPE>buffer</>\n");
			break;
		case FIELD_number:
			if (field->flags & CX_UNSIGNED)
				sfprintf(expr->op, "			<TYPE>unsigned be_t</>\n");
			else if (field->flags & CX_INTEGER)
				sfprintf(expr->op, "			<TYPE>be_t</>\n");
			else
				sfprintf(expr->op, "			<TYPE>ibm_t</>\n");
			break;
		}
		sfprintf(expr->op, "			<WIDTH>%d</>\n", field->width);
		sfprintf(expr->op, "		</>\n");
		sfprintf(expr->op, "	</>\n");
	}
	if (r > x)
	{
		sfprintf(expr->op, "	<FIELD>\n");
		sfprintf(expr->op, "		<NAME>%s</>\n", b ? "_HEAP_" : "_PAD_");
		sfprintf(expr->op, "		<DESCRIPTION>%s</>\n", b ? "Variable width data heap." : "Fixed size roundup pad.");
		sfprintf(expr->op, "		<TYPE>void</>\n");
		sfprintf(expr->op, "		<PHYSICAL>\n");
		sfprintf(expr->op, "			<WIDTH>%I*u</>\n", sizeof(r), r - x);
		sfprintf(expr->op, "		</>\n");
		sfprintf(expr->op, "	</>\n");
	}
	sfprintf(expr->op, "</>\n");
	vmclose(state->vm);
	return 0;
}

static Cxquery_t	queries[] =
{
	{
		"fix",
		"generate fixed binary schema from input schema",
		CXH,
		fix_beg,
		0,
		fix_act,
		fix_end
	},
	{0}
};

Dsslib_t	dss_lib_fix =
{
	"fix",
	"fix query"
	"[-1lms5P?\n@(#)$Id: dss fix query (AT&T Research) 2003-01-30 $\n]"
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
