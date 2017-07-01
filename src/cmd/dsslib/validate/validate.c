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

static const char validate_usage[] =
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The validate query validates the constraints of the"
"	\afield\a operands. If no operands are specified then all"
"	fields with constraints or maps are validated. A warning is"
"	printed if there are no fields with constraints or maps.]"
"[d:discard?Discard records containing invalid fields.]"
"[l:list?List the field constraints and exit.]"
"[s:summary?Print a summary after all records have been read.]"
"[r:repair?Repair invalid fields if possible.]"
"[v:verbose?Warn about each invalid field and the action taken.]"
"\n"
"\n[ field ... ]\n"
"\n"
;

#include <dsslib.h>
#include <ast_float.h>

struct Field_s; typedef struct Field_s Field_t;
struct Invalid_s; typedef struct Invalid_s Invalid_t;
struct State_s; typedef struct State_s State_t;

struct Field_s
{
	Field_t*	next;
	Cxvariable_t*	variable;
	Cxinteger_t	invalid;
	Cxinteger_t	discarded;
	Cxinteger_t	repaired;
};

struct Invalid_s
{
	Dtlink_t	link;
	Cxvalue_t	value;
	Cxvariable_t*	variable;
	Cxunsigned_t	count;
};

struct State_s
{
	Field_t*	field;
	Cxcallout_f	getf;
	Cxcallout_f	setf;
	Dt_t*		invalid;
	Dtdisc_t	invaliddisc;
	Vmalloc_t*	vm;
	unsigned char	discard;
	unsigned char	summary;
	unsigned char	verbose;
};

extern Dsslib_t		dss_lib_validate;

static void
number(Sfio_t* op, const char* label, Cxnumber_t n, Cxformat_t* format)
{
	sfprintf(op, " %s=", label);
	if (format->details)
	{
		if (((n >= 0) ? n : -n) < 1)
			n = 0;
		else if (n > FLTMAX_INTMAX_MAX)
			n = FLTMAX_INTMAX_MAX;
		else if (n < FLTMAX_INTMAX_MIN)
			n = FLTMAX_INTMAX_MIN;
		sfprintf(op, format->details, (Cxinteger_t)n);
	}
	else if (n == 0 || ((n >= 0) ? n : -n) >= 1 && n >= FLTMAX_INTMAX_MIN && n <= FLTMAX_UINTMAX_MAX && n == (Cxinteger_t)n)
		sfprintf(op, (format->flags & CX_UNSIGNED) ? "%llu" : "%lld", (Cxinteger_t)n);
	else
		sfprintf(op, "%1.15Lg", n);
}

static int
invalidcmp(Dt_t* dict, void* a, void* b, Dtdisc_t* disc)
{
	Invalid_t*	ap = (Invalid_t*)a;
	Invalid_t*	bp = (Invalid_t*)b;
	size_t		az;
	size_t		bz;
	int		r;

	if (!(r = strcmp(ap->variable->name, bp->variable->name)))
	{
		if (cxisstring(ap->variable->type) || cxisbuffer(ap->variable->type))
		{
			az = ap->value.buffer.size;
			bz = bp->value.buffer.size;
			if (!(r = memcmp(ap->value.buffer.data, bp->value.buffer.data, az < bz ? az : bz)))
			{
				if (az < bz)
					r = -1;
				if (az > bz)
					r = 1;
			}
		}
		else if (ap->value.number < bp->value.number)
			r = -1;
		else if (ap->value.number > bp->value.number)
			r = 1;
	}
	return r;
}

static int
validate_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**			argv = (char**)data;
	int			errors = error_info.errors;
	char*			s;
	State_t*		state;
	Cxvariable_t*		variable;
	register Field_t*	field;
	Field_t*		lastfield;
	Cxconstraint_t*		constraint;
	int			all;
	int			list;
	Vmalloc_t*		vm;

	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)) || !(state = vmnewof(vm, 0, State_t, 1, 0)))
	{
		if (vm)
			vmclose(vm);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	state->vm = vm;
	list = 0;
	sfprintf(cx->buf, "%s%s", strchr(dss_lib_validate.description, '['), validate_usage);
	s = sfstruse(cx->buf);
	for (;;)
	{
		switch (optget(argv, s))
		{
		case 'd':
			state->discard = 1;
			continue;
		case 'l':
			list = 1;
			continue;
		case 'r':
			if (!(state->setf = cxcallout(cx, CX_SET, cx->state->type_void, cx->state->type_void, cx->disc)))
			{
				if (cx->disc->errorf)
					(*cx->disc->errorf)(NiL, cx->disc, 3, "reair requires CX_SET callout");
				return -1;
			}
			continue;
		case 's':
			state->summary = 1;
			continue;
		case 'v':
			state->summary = state->verbose = 1;
			continue;
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
		goto bad;
	argv += opt_info.index;
	if (all = !*argv)
		variable = 0;
	do
	{
		if (all)
		{
			if (!(variable = (Cxvariable_t*)(variable ? dtnext(cx->fields, variable) : dtfirst(cx->fields))))
				break;
		}
		else if (!(variable = cxvariable(cx, *argv, NiL, disc)))
			goto bad;
		if (variable->format.constraint || variable->format.map)
		{
			if (!(field = vmnewof(vm, 0, Field_t, 1, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			field->variable = variable;
			if (state->field)
				lastfield = lastfield->next = field;
			else
				lastfield = state->field = field;
		}
	} while (all || *++argv);
	if (!state->field && disc->errorf)
		(*disc->errorf)(NiL, disc, 1, "no field has constraints or maps");
	if (list)
	{
		for (field = state->field; field; field = field->next)
		{
			sfprintf(expr->op, "%16s", field->variable->name);
			if (field->variable->format.map)
				sfprintf(expr->op, " map");
			if (constraint = field->variable->format.constraint)
			{
				if (constraint->name)
					sfprintf(expr->op, " name=%s", constraint->name);
				if (constraint->constraintf)
					sfprintf(expr->op, " external");
				if (cxisnumber(field->variable->type))
				{
					if (constraint->def)
						number(expr->op, "default", constraint->def->number, &field->variable->format);
					if (constraint->min)
						number(expr->op, "min", constraint->min->number, &field->variable->format);
					if (constraint->max)
						number(expr->op, "max", constraint->max->number, &field->variable->format);
				}
				else if (cxisstring(field->variable->type) && constraint->def)
					sfprintf(expr->op, " default=\"%-.*s\"", constraint->def->string.size, constraint->def->string.data);
				if (constraint->expression)
					sfprintf(expr->op, " expression=\"%s\"", constraint->expression);
				if (constraint->pattern)
					sfprintf(expr->op, " pattern=\"%s\"", constraint->pattern);
			}
			sfprintf(expr->op, "\n");
		}
		goto bad;
	}
	if (!(state->getf = cxcallout(cx, CX_GET, cx->state->type_void, cx->state->type_void, cx->disc)))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, 3, "validation requires CX_GET callout");
		goto bad;
	}
	if (!state->verbose)
	{
		state->invaliddisc.comparf = invalidcmp;
		if (!(state->invalid = dtnew(vm, &state->invaliddisc, Dtoset)))
		{
			if (cx->disc->errorf)
				(*cx->disc->errorf)(NiL, cx->disc, 3, "validation requires CX_GET callout");
			goto bad;
		}
	}
	expr->data = state;
	return 0;
 bad:
	vmclose(vm);
	return -1;
}

static int
validate_sel(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;
	register Field_t*	field;
	register Cxconstraint_t*constraint;
	Cxoperand_t		o;
	Cxinstruction_t		x;
	Invalid_t		key;
	Invalid_t*		ip;
	size_t			n;

	for (field = state->field; field; field = field->next)
	{
		x.data.variable = field->variable;
		if ((*state->getf)(cx, &x, &o, NiL, NiL, data, disc))
			return -1;
		if (field->variable->format.map)
		{
			if (cxisstring(field->variable->type))
			{
				if (cxstr2num(cx, &field->variable->format, o.value.string.data, o.value.string.size, NiL))
				{
					if (state->verbose && disc->errorf)
						(*disc->errorf)(NiL, disc, 1, "%s%s: %-.*s: unknown map name", cxlocation(cx, data), field->variable->name, o.value.string.size, o.value.string.data);
					goto invalid;
				}
			}
			else if (cxisnumber(field->variable->type))
			{
				if (cxnum2str(cx, &field->variable->format, (Cxinteger_t)o.value.number, NiL))
				{
					if (state->verbose && disc->errorf)
						(*disc->errorf)(NiL, disc, 1, "%s%s: %I*d: unknown map value", cxlocation(cx, data), field->variable->name, sizeof(Cxinteger_t), (Cxinteger_t)o.value.number);
					goto invalid;
				}
			}
		}
		if (constraint = field->variable->format.constraint)
		{
			if (constraint->constraintf)
				;
			if (cxisnumber(field->variable->type))
			{
				if (constraint->min && o.value.number < constraint->min->number)
				{
					if (state->verbose && disc->errorf)
						(*disc->errorf)(NiL, disc, 1, "%s%s: %1.15Lg violates min constraint %1.15Lg", cxlocation(cx, data), field->variable->name, o.value.number, constraint->min->number);
					goto invalid;
				}
				if (constraint->max && o.value.number > constraint->max->number)
				{
					if (state->verbose && disc->errorf)
						(*disc->errorf)(NiL, disc, 1, "%s%s: %1.15Lg violates max constraint %1.15Lg", cxlocation(cx, data), field->variable->name, o.value.number, constraint->max->number);
					goto invalid;
				}
			}
			if (constraint->expression)
				;
			if (constraint->pattern)
				;
		}
		continue;
	invalid:
		if (state->invalid)
		{
			key.variable = field->variable;
			key.value = o.value;
			if (!(ip = (Invalid_t*)dtsearch(state->invalid, &key)))
			{
				n = cxsize(field->variable->type, &o.value);
				if (!(ip = vmnewof(state->vm, 0, Invalid_t, 1, n)))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
					return -1;
				}
				*ip = key;
				ip->value = o.value;
				if (n)
				{
					ip->value.buffer.data = (void*)(ip + 1);
					memcpy(ip->value.buffer.data, o.value.buffer.data, n);
				}
				dtinsert(state->invalid, ip);
				ip->count = 0;
			}
			ip->count++;
		}
		if (state->setf && constraint && constraint->def)
		{
			o.type = field->variable->type;
			o.value = *constraint->def;
			if ((*state->setf)(cx, &x, &o, &o, NiL, data, disc))
				return -1;
			field->repaired++;
		}
		else if (state->discard)
		{
			field->discarded++;
			return 0;
		}
		else
			field->invalid++;
	}
	return 1;
}

static int
validate_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;
	register Field_t*	field;
	Invalid_t*		ip;
	Cxoperand_t		val;
	int			heading;

	if (state->summary)
	{
		heading = 1;
		if (state->invalid && dtsize(state->invalid))
		{
			heading = 0;
			sfprintf(expr->op, "%16s  %11s  %s\n", "FIELD", "COUNT", "VALUE");
			for (ip = (Invalid_t*)dtfirst(state->invalid); ip; ip = (Invalid_t*)dtnext(state->invalid, ip))
			{
				val.type = ip->variable->type;
				val.value = ip->value;
				if (!cxcast(cx, &val, NiL, cx->state->type_string, NiL, NiL))
					sfprintf(expr->op, "%16s  %11I*u  %*.*s\n", ip->variable->name, sizeof(ip->count), ip->count, val.value.string.size, val.value.string.size, val.value.string.data);
			}
		}
		if (!heading)
		{
			heading = 1;
			sfprintf(expr->op, "\n");
		}
		for (field = state->field; field; field = field->next)
			if (field->invalid || field->discarded || field->repaired)
			{
				if (heading)
				{
					heading = 0;
					sfprintf(expr->op, "%16s  %11s %11s %11s\n", "FIELD", "INVALID", "DISCARDED", "REPAIRED");
				}
				sfprintf(expr->op, "%16s  %11I*u %11I*u %11I*u\n", field->variable->name, sizeof(field->invalid), field->invalid, sizeof(field->discarded), field->discarded, sizeof(field->repaired), field->repaired);
			}
	}
	vmclose(state->vm);
	return 0;
}

static Cxquery_t	queries[] =
{
	{
		"validate",
		"validate field value constraints",
		CXH,
		validate_beg,
		validate_sel,
		0,
		validate_end
	},
	{0}
};

Dsslib_t		dss_lib_validate =
{
	"validate",
	"validate query"
	"[-1lms5P?\n@(#)$Id: dss validate query (AT&T Research) 2003-04-05 $\n]"
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
