/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2013 AT&T Intellectual Property          *
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
 * C expression library open/close and global initialization
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "cxlib.h"

#include <ccode.h>

typedef struct Type_s
{
	Cxtype_t**	state;
	Cxtype_t	type;
} Type_t;

static Cxstate_t	state;
static Cxtable_t	table;

static const char	name_bool[] = "bool";
static const char	name_buffer[] = "buffer";
static const char	name_number[] = "number";
static const char	name_pointer[] = "pointer";
static const char	name_reference[] = "reference";
static const char	name_string[] = "string";
static const char	name_type[] = "type_t";
static const char	name_void[] = "void";

static void*
match_string_comp(Cx_t* cx, Cxtype_t* st, Cxtype_t* pt, Cxvalue_t* pv, Cxdisc_t* disc)
{
	regex_t*	re;

	if (!cxisstring(pt))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", st->name, st->name);
		return 0;
	}
	if (!(re = newof(0, regex_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	re->re_disc = &cx->redisc;
	if (regcomp(re, pv->string.data, REG_AUGMENTED|REG_DISCIPLINE))
	{
		free(re);
		return 0;
	}
	return re;
}

static int
match_string_exec(Cx_t* cx, void* data, Cxtype_t* st, Cxvalue_t* sv, Cxdisc_t* disc)
{
	int	i;

	if ((i = regnexec((regex_t*)data, sv->string.data, sv->string.size, 0, NiL, 0)) && i != REG_NOMATCH)
		i = -1;
	return i == 0;
}

static int
match_string_free(Cx_t* cx, void* data, Cxdisc_t* disc)
{
	if (data)
	{
		regfree((regex_t*)data);
		free(data);
	}
	return 0;
}

static Cxmatch_t	match_string =
{
	"regex",
	"Matches on this type treat the pattern as a regex(3) pattern string.",
	CXH,
	match_string_comp,
	match_string_exec,
	match_string_free
};

/*
 * default void externalf
 */

static ssize_t
void_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	ssize_t	i;

	if (!format)
		return 0;
	if (format->width > size)
		return format->width;
	for (i = 0; i < format->width; i++)
		buf[i] = 0;
	return i;
}

/*
 * default void internalf
 */

static ssize_t
void_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	ret->value.number = 0;
	return format ? format->width : 0;
}

/*
 * default number externalf
 */

static ssize_t
number_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	ssize_t	n;
	char*	f;
	char	fmt[16];

	if (CXDETAILS(details, format, type, 0))
		n = sfsprintf(buf, size, details, (intmax_t)value->number);
	else if (value->number == (intmax_t)value->number || (format->flags & CX_INTEGER))
	{
		f = fmt;
		*f++ = '%';
		if (format)
		{
			if (format->width)
			{
				if (format->fill > 0)
					*f++ = format->fill;
				f += sfsprintf(f, sizeof(fmt) - (f - fmt), "%d", format->width);
			}
			*f++ = 'l';
			*f++ = 'l';
			*f++ = (format->flags & CX_UNSIGNED) ? 'u' : 'd';
		}
		else
		{
			*f++ = 'l';
			*f++ = 'l';
			*f++ = 'd';
		}
		*f = 0;
		n = sfsprintf(buf, size, fmt, (intmax_t)value->number);
	}
	else if (format->width)
	{
		int	w;

		w = format->width - ((value->number < 0) ? 2 : 1);
		n = sfsprintf(buf, size, "%#.*I*g", w, sizeof(value->number), value->number);
		if (n != w)
		{
			w += w - n;
			n = sfsprintf(buf, size, "%#.*I*g", w, sizeof(value->number), value->number);
		}
	}
	else
		n = sfsprintf(buf, size, "%1.15I*g", sizeof(value->number), value->number);
	if (n < 0)
		return -1;
	if (n > size)
		return size ? 2 * size : 32;
	return n;
}

/*
 * default number internalf
 */

static ssize_t
number_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	Cxunsigned_t	m;
	char*		e;

	if (size == 0)
	{
		ret->value.number = 0;
		return 0;
	}
	if (format && (format->flags & CX_UNSIGNED))
		ret->value.number = (Cxinteger_t)strntoull(buf, size, &e, format->base);
	else if (format && format->base)
		ret->value.number = strntoll(buf, size, &e, format->base);
	else
		ret->value.number = strntod(buf, size, &e);
	if (e != ((char*)buf + size) && *buf)
	{
		if (format && format->map && !cxstr2num(cx, format, buf, size, &m))
		{
			ret->value.number = (Cxinteger_t)m;
			return size;
		}
		if (disc->errorf && !(cx->flags & CX_QUIET))
			(*disc->errorf)(cx, disc, 1, "%s: invalid number [(buf+size)=%p e=%p%s base=%d size=%d]", fmtquote(buf, NiL, NiL, size, 0), (char*)buf+size, e, (format && (format->flags & CX_UNSIGNED)) ? " unsigned" : "", format ? format->base : 0, size);
	}
	return e - (char*)buf;
}

/*
 * default bool externalf
 */

static ssize_t
bool_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	return sfsprintf(buf, size, "%c", value->number ? '1' : '0');
}

/*
 * default bool internalf
 */

static ssize_t
bool_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	Cxunsigned_t	m;
	char*		e;

	if (size == 0)
	{
		ret->value.number = 0;
		return 0;
	}
	ret->value.number = (Cxinteger_t)strntoull(buf, size, &e, format->base);
	if (e != ((char*)buf + size) && *buf)
	{
		if (format && format->map && !cxstr2num(cx, format, buf, size, &m))
		{
			ret->value.number = (Cxinteger_t)m;
			return size;
		}
		if (disc->errorf && !(cx->flags & CX_QUIET))
			(*disc->errorf)(cx, disc, 1, "%s: invalid bool [(buf+size)=%p e=%p%s base=%d size=%d]", fmtquote(buf, NiL, NiL, size, 0), (char*)buf+size, e, (format && (format->flags & CX_UNSIGNED)) ? " unsigned" : "", format ? format->base : 0, size);
	}
	return e - (char*)buf;
}

/*
 * default string externalf
 */

static ssize_t
string_external(Cx_t* cx, Cxtype_t* type, const char* details, register Cxformat_t* format, register Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	if (format && format->width)
	{
		if (format->width > size)
			return format->width;
		if (format->width <= value->string.size)
			memcpy(buf, value->string.data, format->width);
		else
		{
			memcpy(buf, value->string.data, value->string.size);
			memset(buf + value->string.size, format->fill >= 0 ? format->fill : 0, format->width - value->string.size);
		}
		return format->width;
	}
	else
	{
		if (value->string.size > size)
			return value->string.size;
		memcpy(buf, value->string.data, value->string.size);
		return value->string.size;
	}
}

/*
 * default string internalf
 */

static ssize_t
string_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	s;

	ret->value.string.data = (char*)buf;
	if ((!format || !(format->flags & CX_NUL) && format->fill <= 0) && (s = memchr(buf, 0, size)))
		size = s - (char*)buf;
	return ret->value.string.size = size;
}

/*
 * default buffer externalf -- mime base64 encode
 */

static ssize_t
buffer_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	register unsigned char*	t;
	register unsigned char*	f;
	register unsigned char*	e;
	register int		v;
	int			z;

	static const char	hex[] = "0123456789abcdefg";

	switch (details ? *details : 0)
	{
	case 0:
	case 'b':
	case 'm':
		return base64encode(value->buffer.data, value->buffer.size, NiL, buf, size, NiL);
	case 'h':
	case 'x':
		z = value->buffer.size;
		f = (unsigned char*)value->buffer.data;
		e = f + z;
		z *= 2;
		if (size < z)
			return z;
		t = (unsigned char*)buf;
		while (f < e)
		{
			v = *f++;
			*t++ = hex[v >> 4];
			*t++ = hex[v & 0xf];
		}
		return z;
	}
	if (cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, ERROR_SYSTEM|2, "%s: unknown buffer representation details", details);
	return -1;
}

/*
 * default buffer internalf -- mime base64 decode
 */

static ssize_t
buffer_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	void*	t;
	size_t	n;
	ssize_t	r;

	n = (size * 3) / 4 + 1;
	if (!vm)
		vm = Vmregion;
	if (!(t = vmnewof(vm, 0, unsigned char, n, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if ((r = base64decode(buf, size, NiL, t, n, NiL)) > n)
		vmfree(vm, t);
	else
	{
		ret->value.buffer.data = t;
		ret->value.buffer.size = r;
	}
	return r;
}

/*
 * type externalf
 */

static ssize_t
type_external(Cx_t* cx, Cxtype_t* type, const char* details, register Cxformat_t* format, register Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	size_t	n;

	type = (Cxtype_t*)value->pointer;
	if ((n = strlen(type->name)) <= size)
		memcpy(buf, type->name, n);
	return n;
}

/*
 * default string internalf
 */

static ssize_t
type_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return -1;
}

#define BT(r,n,s,d,e,i,m)	{s,{n,d,CXH,0,0,e,i,r,0,0,0,{0},m},},

/*
 * NOTE: void must be first
 */

static Type_t types[] =
{
BT(CX_void,	&name_void[0],	   &state.type_void,	"No value. May be used for padding.", void_external, void_internal, 0)
BT(CX_number,	&name_number[0],   &state.type_number,	"An integral or floating point number.", number_external, number_internal, 0)
BT(CX_string,	&name_string[0],   &state.type_string,	"A string. The format details string specifies quoting and C style character escape processing: \bquote\b[=\achar\a] quotes string with \achar\a (\b\"\b) as the begin and end quote; \bendquote\b=\achar\a changes the \bquote\b end to \achar\a; \bshell\b[=\abeg\a] quotes using shell conventions and \abeg\a (\b$'\b) as the quote begin; \bopt\b performs \bquote\b and \bshell\b quoting only when necessary; \bescape\b assumes that escape processing has already been performed; \bwide\b does not escape characters with the bit 8 set; \bexpand\b=\amask\a expands escaped characters according to \amask\a which may be a \b,\b or \b|\b separated list of \ball\b: expand all escaped chars, \bchar\b expands 7 bit character escapes, \bline\b expands \b\\r\b and \b\\n\b escapes, \bwide\b expands wide character escapes, \bnocr\b eliminates \b\\r\b, and \bnonl\b eliminates \b\\n\b.", string_external, string_internal, &match_string)
BT(CX_reference,&name_reference[0],&state.type_reference,"A referenced type.", 0,0,0)
BT(CX_buffer,	&name_buffer[0],   &state.type_buffer,	"A separately allocated sized buffer. The external representation is a newline separated base64 mime encoding.", buffer_external, buffer_internal, 0)
BT(CX_bool,	&name_bool[0],   &state.type_bool,	"An boolean value: 0==false, 1==true.", bool_external, bool_internal, 0)
BT(CX_type,	&name_type[0],   &state.type_type_t,	"A type.", type_external, type_internal, 0)
BT(CX_pointer,	&name_pointer[0],  0,			"A generic pointer.", 0,0,0)
};

#define CX_bool_t	((Cxtype_t*)&name_bool[0])
#define CX_buffer_t	((Cxtype_t*)&name_buffer[0])
#define CX_number_t	((Cxtype_t*)&name_number[0])
#define CX_pointer_t	((Cxtype_t*)&name_pointer[0])
#define CX_reference_t	((Cxtype_t*)&name_reference[0])
#define CX_string_t	((Cxtype_t*)&name_string[0])
#define CX_type_t	((Cxtype_t*)&name_type[0])
#define CX_void_t	((Cxtype_t*)&name_void[0])

static int
op_call_V(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	return (*pc->data.variable->function)(cx, pc->data.variable, r, b + (pc->pp + 1), -pc->pp, data, disc);
}

static int
op_nop_V(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	return 0;
}

static int
op_end_V(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	/*
	 * NOTE: this special case error return breaks out of the
	 *       cxeval() execute() inner loop and is not recorded
	 *	 as an error
	 */

	return -1;
}

static int
op_ref_V(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	return (r->value.variable = cxvariable(cx, b->value.string.data, a->type, disc)) ? 0 : -1;
}

static int
op_sc_V(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	if ((pc->op == CX_SC0) == (b->value.number == 0))
	{
		cx->jump = (int)pc->data.number;
		return 1;
	}
	return 0;
}

static int
op_const_V(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value = pc->data;
	return 0;
}

static int
op_tst_V(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	if (cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, 2, "CX_TST not implemented yet");
	return -1;
}

static int
op_add_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number + b->value.number;
	return 0;
}

static int
op_sub_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number - b->value.number;
	return 0;
}

static int
op_mpy_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number * b->value.number;
	return 0;
}

static int
op_div_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	if (b->value.number == 0.0)
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(cx, cx->disc, 2, "divide by 0");
		return -1;
	}
	r->value.number = a->value.number / b->value.number;
	return 0;
}

static int
op_mod_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	if (b->value.number < 1.0)
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(cx, cx->disc, 2, "modulus by number < 1.0");
		return -1;
	}
	r->value.number = a->value.number / b->value.number;
	r->value.number = (Cxnumber_t)(((Cxinteger_t)a->value.number) % ((Cxinteger_t)b->value.number));
	return 0;
}

static int
op_and_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = (Cxnumber_t)(((Cxinteger_t)a->value.number) & ((Cxinteger_t)b->value.number));
	return 0;
}

static int
op_or_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = (Cxnumber_t)(((Cxinteger_t)a->value.number) | ((Cxinteger_t)b->value.number));
	return 0;
}

static int
op_xor_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = (Cxnumber_t)(((Cxinteger_t)a->value.number) ^ ((Cxinteger_t)b->value.number));
	return 0;
}

static int
op_andand_L(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number > 0 && b->value.number > 0;
	return 0;
}

static int
op_oror_L(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number > 0 || b->value.number > 0;
	return 0;
}

static int
op_lsh_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = (Cxnumber_t)(((Cxinteger_t)a->value.number) << ((int)b->value.number));
	return 0;
}

static int
op_rsh_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = (Cxnumber_t)(((Cxinteger_t)a->value.number) >> ((int)b->value.number));
	return 0;
}

static int
op_inv_L(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = (Cxnumber_t)(~((Cxinteger_t)a->value.number));
	return 0;
}

static int
op_log_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.number > 0.0;
	return 0;
}

static int
op_not_L(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.number == 0.0;
	return 0;
}

static int
op_uplus_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.number;
	return 0;
}

static int
op_uminus_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = -b->value.number;
	return 0;
}

static int
op_lt_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number < b->value.number;
	return 0;
}

static int
op_le_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number <= b->value.number;
	return 0;
}

static int
op_eq_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number == b->value.number;
	return 0;
}

static int
op_ne_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number != b->value.number;
	return 0;
}

static int
op_ge_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number >= b->value.number;
	return 0;
}

static int
op_gt_N(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.number > b->value.number;
	return 0;
}

static int
op_log_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.string.size > 0 && b->value.string.data[0] != 0;
	return 0;
}

static int
op_lt_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = strncmp(a->value.string.data, b->value.string.data, CXMIN(a->value.string.size, b->value.string.size))) < 0 || c == 0 && a->value.string.size < b->value.string.size;
	return 0;
}

static int
op_le_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = strncmp(a->value.string.data, b->value.string.data, CXMIN(a->value.string.size, b->value.string.size))) < 0 || c == 0 && a->value.string.size == b->value.string.size;
	return 0;
}

static int
op_eq_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.string.size == b->value.string.size && strncmp(a->value.string.data, b->value.string.data, a->value.string.size) == 0;
	return 0;
}

static int
op_ne_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.string.size != b->value.string.size || strncmp(a->value.string.data, b->value.string.data, a->value.string.size) != 0;
	return 0;
}

static int
op_ge_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = strncmp(a->value.string.data, b->value.string.data, CXMIN(a->value.string.size, b->value.string.size))) > 0 || c == 0 && a->value.string.size > b->value.string.size;
	return 0;
}

static int
op_gt_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = strncmp(a->value.string.data, b->value.string.data, CXMIN(a->value.string.size, b->value.string.size))) > 0 || c == 0 && a->value.string.size > b->value.string.size;
	return 0;
}

static int
op_not_S(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = *b->value.string.data == 0;
	return 0;
}

static int
op_lt_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = memcmp(a->value.buffer.data, b->value.buffer.data, CXMIN(a->value.buffer.size, b->value.buffer.size))) < 0 || c == 0 && a->value.buffer.size < b->value.buffer.size;
	return 0;
}

static int
op_le_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = memcmp(a->value.buffer.data, b->value.buffer.data, CXMIN(a->value.buffer.size, b->value.buffer.size))) < 0 || c == 0 && a->value.buffer.size == b->value.buffer.size;
	return 0;
}

static int
op_eq_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.buffer.size == b->value.buffer.size && memcmp(a->value.buffer.data, b->value.buffer.data, a->value.buffer.size) == 0;
	return 0;
}

static int
op_ne_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.buffer.size != b->value.buffer.size || memcmp(a->value.buffer.data, b->value.buffer.data, a->value.buffer.size) != 0;
	return 0;
}

static int
op_ge_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = memcmp(a->value.buffer.data, b->value.buffer.data, CXMIN(a->value.buffer.size, b->value.buffer.size))) > 0 || c == 0 && a->value.buffer.size > b->value.buffer.size;
	return 0;
}

static int
op_gt_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	c;

	r->value.number = (c = memcmp(a->value.buffer.data, b->value.buffer.data, CXMIN(a->value.buffer.size, b->value.buffer.size))) > 0 || c == 0 && a->value.buffer.size > b->value.buffer.size;
	return 0;
}

static int
op_log_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.buffer.data && b->value.buffer.size;
	return 0;
}

static int
op_not_B(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.buffer.size == 0;
	return 0;
}

static int
op_eq_T(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->value.type == b->value.type;
	return 0;
}

static int
op_ne_T(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = a->type != b->type;
	return 0;
}

static int
op_cast_SN(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	char*	e;

	r->value.number = strtod(b->value.string.data, &e);
	if (*e && cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, 2, "%s: invalid number", b->value.string.data);
	return 0;
}

static int
op_cast_BL(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.buffer.data && b->value.buffer.size;
	return 0;
}

static int
op_cast_SL(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = b->value.string.data && b->value.string.size && *b->value.string.data;
	return 0;
}

static int
op_cast_BN(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.number = !!b->value.buffer.data;
	return 0;
}

static int
op_cast_BS(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value.string.data = b->value.buffer.data ? "1" : "0";
	r->value.string.size = 1;
	return 0;
}

static int
op_match(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	i;

	if ((i = (*a->type->match->execf)(cx, pc->data.pointer, a->type, &a->value, disc)) < 0)
		return i;
	r->value.number = i == 1;
	return 0;
}

static int
op_nomatch(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	int	i;

	if ((i = (*a->type->match->execf)(cx, pc->data.pointer, a->type, &a->value, disc)) < 0)
		return i;
	r->value.number = i == 0;
	return 0;
}

static int
re_match(Cx_t* cx, Cxexpr_t* expr, Cxinstruction_t* x, Cxinstruction_t* a, Cxinstruction_t* b, void* data, Cxdisc_t* disc)
{
	if (!(x->data.pointer = (*a->type->match->compf)(cx, a->type, b->type, &b->data, disc)))
		return -1;
	if (a->type->match->freef && cxatfree(cx, expr, a->type->match->freef, x->data.pointer))
	{
		(*a->type->match->freef)(cx, x->data.pointer, disc);
		return -1;
	}
	b->type = state.type_void;
	x->callout = x->op == CX_MATCH ? op_match : op_nomatch;
	return 0;
}

static Cxcallout_t callouts[] =
{

CXC(CX_CALL,	CX_void_t,	CX_void_t,	op_call_V,	0)
CXC(CX_NOP,	CX_void_t,	CX_void_t,	op_nop_V,	0)
CXC(CX_POP,	CX_void_t,	CX_void_t,	op_nop_V,	0)
CXC(CX_REF,	CX_reference_t,	CX_void_t,	op_const_V,	0)
CXC(CX_REF,	CX_string_t,	CX_void_t,	op_ref_V,	0)
CXC(CX_SC0,	CX_bool_t,	CX_void_t,	op_sc_V,	0)
CXC(CX_SC1,	CX_bool_t,	CX_void_t,	op_sc_V,	0)
CXC(CX_SC0,	CX_number_t,	CX_void_t,	op_sc_V,	0)
CXC(CX_SC1,	CX_number_t,	CX_void_t,	op_sc_V,	0)
CXC(CX_TST,	CX_void_t,	CX_void_t,	op_tst_V,	0)
CXC(CX_END,	CX_void_t,	CX_void_t,	op_end_V,	0)

CXC(CX_NUM,	CX_number_t,	CX_void_t,	op_const_V,	0)
CXC(CX_STR,	CX_string_t,	CX_void_t,	op_const_V,	0)
CXC(CX_NUM,	CX_type_t,	CX_void_t,	op_const_V,	0)

CXC(CX_ADD,	CX_number_t,	CX_number_t,	op_add_N,	0)
CXC(CX_SUB,	CX_number_t,	CX_number_t,	op_sub_N,	0)
CXC(CX_MPY,	CX_number_t,	CX_number_t,	op_mpy_N,	0)
CXC(CX_DIV,	CX_number_t,	CX_number_t,	op_div_N,	0)
CXC(CX_MOD,	CX_number_t,	CX_number_t,	op_mod_N,	0)
CXC(CX_AND,	CX_number_t,	CX_number_t,	op_and_N,	0)
CXC(CX_OR,	CX_number_t,	CX_number_t,	op_or_N,	0)
CXC(CX_XOR,	CX_number_t,	CX_number_t,	op_xor_N,	0)

CXC(CX_ANDAND,	CX_bool_t,	CX_bool_t,	op_andand_L,	0)
CXC(CX_OROR,	CX_bool_t,	CX_bool_t,	op_oror_L,	0)
CXC(CX_INV,	CX_bool_t,	CX_void_t,	op_inv_L,	0)
CXC(CX_NOT,	CX_bool_t,	CX_void_t,	op_not_L,	0)

CXC(CX_ANDAND,	CX_number_t,	CX_number_t,	op_andand_L,	0)
CXC(CX_OROR,	CX_number_t,	CX_number_t,	op_oror_L,	0)
CXC(CX_INV,	CX_number_t,	CX_void_t,	op_inv_L,	0)
CXC(CX_NOT,	CX_number_t,	CX_void_t,	op_not_L,	0)

CXC(CX_LSH,	CX_number_t,	CX_number_t,	op_lsh_N,	0)
CXC(CX_RSH,	CX_number_t,	CX_number_t,	op_rsh_N,	0)

CXC(CX_UPLUS,	CX_number_t,	CX_void_t,	op_uplus_N,	0)
CXC(CX_UMINUS,	CX_number_t,	CX_void_t,	op_uminus_N,	0)

CXC(CX_LT,	CX_number_t,	CX_number_t,	op_lt_N,	0)
CXC(CX_LE,	CX_number_t,	CX_number_t,	op_le_N,	0)
CXC(CX_EQ,	CX_number_t,	CX_number_t,	op_eq_N,	0)
CXC(CX_NE,	CX_number_t,	CX_number_t,	op_ne_N,	0)
CXC(CX_GE,	CX_number_t,	CX_number_t,	op_ge_N,	0)
CXC(CX_GT,	CX_number_t,	CX_number_t,	op_gt_N,	0)

CXC(CX_LT,	CX_string_t,	CX_string_t,	op_lt_S,	0)
CXC(CX_LE,	CX_string_t,	CX_string_t,	op_le_S,	0)
CXC(CX_EQ,	CX_string_t,	CX_string_t,	op_eq_S,	0)
CXC(CX_NE,	CX_string_t,	CX_string_t,	op_ne_S,	0)
CXC(CX_GE,	CX_string_t,	CX_string_t,	op_ge_S,	0)
CXC(CX_GT,	CX_string_t,	CX_string_t,	op_gt_S,	0)

CXC(CX_LT,	CX_buffer_t,	CX_buffer_t,	op_lt_B,	0)
CXC(CX_LE,	CX_buffer_t,	CX_buffer_t,	op_le_B,	0)
CXC(CX_EQ,	CX_buffer_t,	CX_buffer_t,	op_eq_B,	0)
CXC(CX_NE,	CX_buffer_t,	CX_buffer_t,	op_ne_B,	0)
CXC(CX_GE,	CX_buffer_t,	CX_buffer_t,	op_ge_B,	0)
CXC(CX_GT,	CX_buffer_t,	CX_buffer_t,	op_gt_B,	0)

CXC(CX_EQ,	CX_type_t,	CX_type_t,	op_eq_T,	0)
CXC(CX_NE,	CX_type_t,	CX_type_t,	op_ne_T,	0)

CXC(CX_CAST,	CX_string_t,	CX_bool_t,	op_cast_SL,	0)
CXC(CX_CAST,	CX_buffer_t,	CX_bool_t,	op_cast_BL,	0)
CXC(CX_CAST,	CX_string_t,	CX_number_t,	op_cast_SN,	0)
CXC(CX_CAST,	CX_buffer_t,	CX_number_t,	op_cast_BN,	0)
CXC(CX_CAST,	CX_buffer_t,	CX_string_t,	op_cast_BS,	0)

};

size_t
cxsizeof(Cx_t* cx, Cxvariable_t* var, Cxtype_t* type, Cxvalue_t* value)
{
	size_t		size;

	if (var->array)
		size = var->array->size;
	else
		do
		{
			if (size = type->size)
				break;
			switch (type->representation)
			{
			case CX_buffer:
				if (size = value->buffer.size)
				{
					if (value->buffer.elements)
						size = value->buffer.elements;
					else if (type->element)
						size /= type->element;
				}
				break;
			case CX_number:
			case CX_pointer:
				size = 8;
				break;
			case CX_string:
				size = value->string.size;
				break;
			default:
				continue;
			}
			break;
		} while (type = type->base);
	return size;
}

static int
cx_edit_B(Cx_t* cx, Cxvariable_t* var, Cxoperand_t* ret, Cxoperand_t* arg, int n, void* data, Cxdisc_t* disc)
{
	Cxedit_t*	edit;

	if (!(edit = cxedit(cx, arg[0].value.string.data, disc)))
		return -1;
	ret->value = arg[1].value;
	return cxsub(cx, edit, ret);
}

static int
cx_sizeof_B(Cx_t* cx, Cxvariable_t* var, Cxoperand_t* ret, Cxoperand_t* arg, int n, void* data, Cxdisc_t* disc)
{
	ret->value.number = cxsizeof(cx, var, arg->type, &arg->value);
	return 0;
}

static int
cx_typeof_B(Cx_t* cx, Cxvariable_t* var, Cxoperand_t* ret, Cxoperand_t* arg, int n, void* data, Cxdisc_t* disc)
{
	ret->value.type = arg->type;
	return 0;
}

static Cxvariable_t builtins[] =
{
CXF("edit",		"string",	cx_edit_B,	"string,string",
			"Returns the result of applying the ed(1) style substitute expression"
			" in the first argument to the second argument.")
CXF("sizeof",		"number",	cx_sizeof_B,	"*",
			"Returns the size of the \avariable\a argument;"
			" the size of an array variable is the number of elements,"
			" otherwise the size is in bytes.")
CXF("typeof",		"type_t",	cx_typeof_B,	"*",
			"Returns the type of the \avariable\a argument"
			" for comparison with other types.")
};

/*
 * open a cx session
 */

Cx_t*
cxopen(Cxflags_t flags, Cxflags_t test, Cxdisc_t* disc)
{
	register Cx_t*		cx;
	register Vmalloc_t*	vm;
	register Vmalloc_t*	em;
	register Vmalloc_t*	rm;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)) || !(em = vmopen(Vmdcheap, Vmlast, 0)) || !(rm = vmopen(Vmdcheap, Vmlast, 0)))
	{
		if (vm)
		{
			vmclose(vm);
			if (em)
				vmclose(em);
		}
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (!(cx = vmnewof(vm, 0, Cx_t, 1, 0)) || !(cx->cvtbuf = vmnewof(vm, 0, char, cx->cvtsiz = CX_CVT, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	cx->state = cxstate(disc);
	cx->table = &table;
	cx->id = CX_ID;
	cx->vm = vm;
	cx->em = em;
	cx->rm = rm;
	cx->disc = disc;
	cx->flags = flags;
	cx->test = test;
	cx->redisc.re_version = REG_VERSION;
	cx->redisc.re_flags = REG_NOFREE;
	cx->redisc.re_errorf = (regerror_t)disc->errorf;
	if (!(cx->buf = sfstropen()) || !(cx->tp = sfstropen()))
	{
		cxclose(cx);
		return 0;
	}
	cx->scoped = 1;
	if (!(flags & CX_SCOPE))
	{
		cx->op = sfstdout;
		if (!(cx->fields = dtnew(cx->vm, &state.listdisc, Dtqueue)) || !(cx->buf = sfstropen()) || !(cx->tp = sfstropen()))
		{
			cxclose(cx);
			return 0;
		}
		cx->callouts = state.callouts;
		cx->constraints = state.constraints;
		cx->edits = state.edits;
		cx->maps = state.maps;
		cx->queries = state.queries;
		cx->recodes = state.recodes;
		cx->types = state.types;
		cx->variables = state.variables;
	}
	cx->ctype = state.ctype;
	return cx;
 bad:
	vmclose(vm);
	vmclose(em);
	vmclose(rm);
	return 0;
}

/*
 * scope control
 */

Cx_t*
cxscope(Cx_t* top, Cx_t* bot, Cxflags_t flags, Cxflags_t test, Cxdisc_t* disc)
{
	if (!top)
	{
		if (!(top = cxopen(CX_SCOPE|flags, test, disc)))
			return 0;
		top->op = sfstdout;
	}
	if (top->scoped != 1)
	{
		if (top->disc->errorf)
			(*top->disc->errorf)(NiL, top->disc, 2, "cannot change active scope");
		return 0;
	}
	if (bot)
	{
		/*
		 * scope top on bot
		 */

		if (top->scope)
		{
			if (top->disc->errorf)
				(*top->disc->errorf)(NiL, top->disc, 2, "already scoped");
			return 0;
		}
		if (top->view & CX_VIEW_callouts)
			dtview(top->callouts, bot->callouts);
		else
			top->callouts = bot->callouts;
		if (top->view & CX_VIEW_constraints)
			dtview(top->constraints, bot->constraints);
		else
			top->constraints = bot->constraints;
		if (top->view & CX_VIEW_edits)
			dtview(top->edits, bot->edits);
		else
			top->edits = bot->edits;
		if (top->view & CX_VIEW_fields)
			dtview(top->fields, bot->fields);
		else
			top->fields = bot->fields;
		if (top->view & CX_VIEW_maps)
			dtview(top->maps, bot->maps);
		else
			top->maps = bot->maps;
		if (top->view & CX_VIEW_queries)
			dtview(top->queries, bot->queries);
		else
			top->queries = bot->queries;
		if (top->view & CX_VIEW_recodes)
			dtview(top->recodes, bot->recodes);
		else
			top->recodes = bot->recodes;
		if (top->view & CX_VIEW_types)
			dtview(top->types, bot->types);
		else
			top->types = bot->types;
		if (top->view & CX_VIEW_variables)
			dtview(top->variables, bot->variables);
		else
			top->variables = bot->variables;
		bot->scoped++;
		top->scope = bot;
		bot = top;
	}
	else if (bot = top->scope)
	{
		/*
		 * pop the scope
		 */

		if (top->view & CX_VIEW_callouts)
			dtview(top->callouts, NiL);
		if (top->view & CX_VIEW_constraints)
			dtview(top->constraints, NiL);
		if (top->view & CX_VIEW_edits)
			dtview(top->edits, NiL);
		if (top->view & CX_VIEW_fields)
			dtview(top->fields, NiL);
		if (top->view & CX_VIEW_maps)
			dtview(top->maps, NiL);
		if (top->view & CX_VIEW_queries)
			dtview(top->queries, NiL);
		if (top->view & CX_VIEW_recodes)
			dtview(top->recodes, NiL);
		if (top->view & CX_VIEW_types)
			dtview(top->types, NiL);
		if (top->view & CX_VIEW_variables)
			dtview(top->variables, NiL);
		top->scope = 0;
		bot->scoped--;
	}
	else
		bot = top;
	return bot;
}

/*
 * close a cx session
 */

int
cxclose(register Cx_t* cx)
{
	if (!cx)
		return -1;
	if (cx->scope)
		cxscope(cx, NiL, 0, 0, cx->disc);
	if (--cx->scoped <= 0)
	{
		if (cx->view & CX_VIEW_callouts)
			dtclose(cx->callouts);
		if (cx->view & CX_VIEW_constraints)
			dtclose(cx->constraints);
		if (cx->view & CX_VIEW_edits)
			dtclose(cx->edits);
		if (cx->view & CX_VIEW_maps)
			dtclose(cx->maps);
		if (cx->view & CX_VIEW_queries)
			dtclose(cx->queries);
		if (cx->view & CX_VIEW_recodes)
			dtclose(cx->recodes);
		if (cx->view & CX_VIEW_types)
			dtclose(cx->types);
		if (cx->view & CX_VIEW_fields)
			dtclose(cx->fields);
		if (cx->view & CX_VIEW_variables)
			dtclose(cx->variables);
		if (cx->scope)
			cx->scope->scoped--;
		if (cx->buf)
			sfclose(cx->buf);
		if (cx->tp)
			sfclose(cx->tp);
		if (cx->em)
			vmclose(cx->em);
		if (cx->rm)
			vmclose(cx->rm);
		if (cx->vm)
			vmclose(cx->vm);
	}
	return 0;
}

/*
 * add a type
 */

int
cxaddtype(Cx_t* cx, register Cxtype_t* type, Cxdisc_t* disc)
{
	char*		base;
	Cxvariable_t*	v;
	Cxvariable_t*	member;
	Dt_t*		dict;
	Cxtype_t*	copy;
	Cxrecode_t*	re;
	int		i;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_types)
			dict = cx->types;
		else if (!(dict = dtnew(cx->vm, &state.namedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->types);
			cx->types = dict;
			cx->view |= CX_VIEW_types;
		}
		if (!(copy = vmnewof(cx->vm, 0, Cxtype_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		*copy = *type;
		type = copy;
	}
	else
		dict = state.types;
	if (dtsearch(dict, type))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: type already defined", type->name);
		return -1;
	}
	dtinsert(dict, type);
	if (!(type->header.flags & CX_NORMALIZED))
	{
		type->header.flags |= CX_NORMALIZED;
		if ((base = (char*)type->base) && !(type->base = cxtype(cx, base, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: unknown base type", base);
			return -1;
		}
		if ((base = (char*)type->fundamental) && !(type->fundamental = cxtype(cx, base, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: unknown fundamental type", base);
			return -1;
		}
		if (type->member)
		{
			if (!type->member->getf)
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s: no member get function", type->name);
				return -1;
			}
			if (!(member = (Cxvariable_t*)type->member->members))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s: no member table", type->name);
				return -1;
			}
			if (type->header.flags & CX_SCHEMA)
				type->header.flags |= CX_REFERENCED;
			else if (!(type->member->members = cx ? dtnew(cx->vm, &state.namedisc, Dtoset) : dtopen(&state.namedisc, Dtoset)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			else
				for (i = 0; member->name; member++)
				{
					v = member;
					if (!(v->header.flags & CX_NORMALIZED))
					{
						v->header.flags |= CX_NORMALIZED;
						if ((base = (char*)v->type) && !(v->type = cxtype(cx, base, disc)))
						{
							if (disc->errorf)
								(*disc->errorf)(NiL, disc, 2, "%s: unknown type", base);
							return -1;
						}
						v->member = type;
					}
					v->header.index = i++;
					dtinsert(type->member->members, v);
				}
		}
		if (type->generic)
			for (i = 0; base = (char*)type->generic[i]; i++)
				if (!(type->generic[i] = cxtype(cx, base, disc)))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s: unknown type", base);
					return -1;
				}
	}
	if (!(type->header.flags & CX_INITIALIZED))
	{
		type->header.flags |= CX_INITIALIZED;
		if (type->fundamental)
		{
			if (type->base)
				type->representation = type->base->representation;
		}
		else if (!type->base)
			type->fundamental = type;
		else
		{
			type->fundamental = type->base->fundamental;
			type->representation = type->base->representation;
		}
		if (type->match)
		{
			if (!(re = newof(0, Cxrecode_t, 1, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			re->header.flags = CX_NORMALIZED;
			re->op.code = CX_MATCH;
			re->op.type1 = type;
			re->op.type2 = state.type_void;
			re->recode = re_match;
			if (cxaddrecode(cx, re, disc))
				return -1;
		}
		if (type->initf && !(type->data = (*type->initf)(type, disc)))
			return -1;
	}
	return 0;
}

/*
 * return type given name
 */

Cxtype_t*
cxtype(Cx_t* cx, const char* name, Cxdisc_t* disc)
{
	register char*	s;
	register char*	lib;
	Cxtype_t*	t;
	size_t		n;

	if ((s = strchr(name, ':')) && *++s == ':')
	{
		n = s - (char*)name;
		lib = fmtbuf(n);
		memcpy(lib, name, --n);
		lib[n] = 0;
		name = (const char*)s + 1;
	}
	else
		lib = (char*)name;
	if (!(t = (Cxtype_t*)dtmatch(cx ? cx->types : state.types, name)) && disc->loadf && (*disc->loadf)(lib, disc))
		t = (Cxtype_t*)dtmatch(cx ? cx->types : state.types, name);
	return t;
}

/*
 * add an op callout
 */

int
cxaddcallout(Cx_t* cx, register Cxcallout_t* callout, Cxdisc_t* disc)
{
	char*		name;
	Dt_t*		dict;
	Cxcallout_t*	copy;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_callouts)
			dict = cx->callouts;
		else if (!(dict = dtnew(cx->vm, &state.codedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->callouts);
			cx->callouts = dict;
			cx->view |= CX_VIEW_callouts;
		}
		if (!(copy = vmnewof(cx->vm, 0, Cxcallout_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		*copy = *callout;
		callout = copy;
	}
	else if (callout->op.code == CX_GET || callout->op.code == CX_SET || callout->op.code == CX_DEL || callout->op.code == CX_RET)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: callout must be local", cxcodename(callout->op.code));
		return -1;
	}
	else
		dict = state.callouts;
	if (!(callout->header.flags & CX_NORMALIZED))
	{
		callout->header.flags |= CX_NORMALIZED;
		if (!(name = (char*)callout->op.type1))
			callout->op.type1 = state.type_void;
		else if (!(callout->op.type1 = cxtype(cx, name, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: unknown type", name);
			return -1;
		}
		if (!(name = (char*)callout->op.type2))
			callout->op.type2 = state.type_void;
		else if (!(callout->op.type2 = cxtype(cx, name, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: unknown type", name);
			return -1;
		}
	}
	if (!(copy = (Cxcallout_t*)dtinsert(dict, callout)) || copy->callout != callout->callout)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "callout initialization error");
		return -1;
	}
	return 0;
}

/*
 * return callout given <code,type1,type2>
 */

Cxcallout_f
cxcallout(Cx_t* cx, int code, Cxtype_t* type1, Cxtype_t* type2, Cxdisc_t* disc)
{
	Cxcallout_t*	callout;
	Cxop_t		op;

	memset(&op, 0, sizeof(op));
	op.code = code;
	if (!(op.type1 = type1))
		op.type1 = state.type_void;
	if (!(op.type2 = type2))
		op.type2 = state.type_void;
	while (!(callout = (Cxcallout_t*)dtmatch(cx ? cx->callouts : state.callouts, &op)))
	{
		if (op.code == CX_NOMATCH)
			op.code = CX_MATCH;
		else if (op.type2 == state.type_void)
			return 0;
		else
			op.type2 = state.type_void;
	}
	return callout->callout;
}

/*
 * add a query
 */

int
cxaddquery(Cx_t* cx, Cxquery_t* query, Cxdisc_t* disc)
{
	Dt_t*		dict;
	Cxquery_t*	copy;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_queries)
			dict = cx->queries;
		else if (!(dict = dtnew(cx->vm, &state.namedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->queries);
			cx->queries = dict;
			cx->view |= CX_VIEW_queries;
		}
		if (!(copy = vmnewof(cx->vm, 0, Cxquery_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		*copy = *query;
		query = copy;
	}
	else
		dict = state.queries;
	if (dtsearch(dict, query))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: query already defined", query->name);
		return -1;
	}
	dtinsert(dict, query);
	return 0;
}

/*
 * return query given name
 */

Cxquery_t*
cxquery(Cx_t* cx, const char* name, Cxdisc_t* disc)
{
	register char*	s;
	register char*	lib;
	Cxquery_t*	q;
	size_t		n;

	if ((s = strchr(name, ':')) && *++s == ':')
	{
		n = s - (char*)name;
		lib = fmtbuf(n);
		memcpy(lib, name, --n);
		lib[n] = 0;
		name = (const char*)s + 1;
	}
	else
		lib = (char*)name;
	if (!(q = (Cxquery_t*)dtmatch(cx ? cx->queries : state.queries, name)) && disc->loadf && (*disc->loadf)(lib, disc))
		q = (Cxquery_t*)dtmatch(cx ? cx->queries : state.queries, name);
	return q;
}

/*
 * return function given name
 */

Cxvariable_t*
cxfunction(Cx_t* cx, const char* name, Cxdisc_t* disc)
{
	register char*	s;
	register char*	p;
	Cxvariable_t*	f;
	Cxlib_t*	lib;
	int		i;

	if (!cx)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: function must be local", name);
		return 0;
	}
	if (f = (Cxvariable_t*)dtmatch(cx->variables, name))
	{
		if (f->function)
			return f;
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: not a function", name);
		return 0;
	}
	if (!(s = strchr(name, ':')) || *++s != ':')
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: function must be local", name);
		return 0;
	}
	i = s - (char*)name;
	p = fmtbuf(i);
	memcpy(p, name, --i);
	p[i] = 0;
	if (!disc->loadf || !(lib = (*disc->loadf)(p, disc)))
		return 0;
	if (lib->functions)
		for (i = 0; lib->functions[i].name; i++)
			if (cxaddvariable(cx, &lib->functions[i], disc))
				return 0;
	if (f = (Cxvariable_t*)dtmatch(cx->variables, name))
	{
		if (f->function)
			return f;
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: not a function", name);
	}
	else
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: undefined function", name);
		return 0;
	}
	return 0;
}

/*
 * add a map
 */

int
cxaddmap(Cx_t* cx, Cxmap_t* map, Cxdisc_t* disc)
{
	Dt_t*		dict;
	Cxmap_t*	copy;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_maps)
			dict = cx->maps;
		if (!(dict = dtnew(cx->vm, &state.namedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->maps);
			cx->maps = dict;
			cx->view |= CX_VIEW_maps;
		}
		if (!(copy = vmnewof(cx->vm, 0, Cxmap_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		*copy = *map;
		map = copy;
	}
	else
		dict = state.maps;
	if (dtsearch(dict, map))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: map already defined", map->name);
		return -1;
	}
	if (cxinitmap(map, disc))
		return -1;
	dtinsert(dict, map);
	return 0;
}

/*
 * return map given name
 */

Cxmap_t*
cxmap(Cx_t* cx, const char* name, Cxdisc_t* disc)
{
	return (Cxmap_t*)dtmatch(cx ? cx->maps : state.maps, name);
}

/*
 * add an op recode
 */

int
cxaddrecode(Cx_t* cx, register Cxrecode_t* recode, Cxdisc_t* disc)
{
	Cxrecode_t*	o;
	char*		name;
	Dt_t*		dict;
	Cxrecode_t*	copy;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_recodes)
			dict = cx->recodes;
		else if (!(dict = dtnew(cx->vm, &state.codedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->recodes);
			cx->recodes = dict;
			cx->view |= CX_VIEW_recodes;
		}
		if (!(copy = vmnewof(cx->vm, 0, Cxrecode_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		*copy = *recode;
		recode = copy;
	}
	else if (recode->op.code == CX_GET || recode->op.code == CX_SET || recode->op.code == CX_DEL || recode->op.code == CX_RET)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: recode must be local", cxcodename(recode->op.code));
		return -1;
	}
	else
		dict = state.recodes;
	if (!(recode->header.flags & CX_NORMALIZED))
	{
		recode->header.flags |= CX_NORMALIZED;
		if ((name = (char*)recode->op.type1) && !(recode->op.type1 = cxtype(cx, name, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: unknown type", name);
			return -1;
		}
		if ((name = (char*)recode->op.type2) && !(recode->op.type2 = cxtype(cx, name, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: unknown type", name);
			return -1;
		}
	}
	if (!(o = (Cxrecode_t*)dtsearch(dict, recode)) || o != recode && dtdelete(dict, o))
		dtinsert(dict, recode);
	return 0;
}

/*
 * return recode given <code,type1,type2>
 */

Cxrecode_f
cxrecode(Cx_t* cx, int code, Cxtype_t* type1, Cxtype_t* type2, Cxdisc_t* disc)
{
	Cxrecode_t*	recode;
	Cxop_t		op;

	switch (code)
	{
	case CX_NOMATCH:
		code = CX_MATCH;
		/*FALLTHROUGH*/
	case CX_MATCH:
		type2 = 0;
		break;
	}
	memset(&op, 0, sizeof(op));
	op.code = code;
	if (!(op.type1 = type1))
		op.type1 = state.type_void;
	if (!(op.type2 = type2))
		op.type2 = state.type_void;
	return (recode = (Cxrecode_t*)dtmatch(cx ? cx->recodes : state.recodes, &op)) ? recode->recode : (Cxrecode_f)0;
}

/*
 * add an edit
 */

int
cxaddedit(Cx_t* cx, register Cxedit_t* edit, Cxdisc_t* disc)
{
	Dt_t*		dict;
	Cxedit_t*	copy;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_edits)
			dict = cx->edits;
		else if (!(dict = dtnew(cx->vm, &state.namedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->edits);
			cx->edits = dict;
			cx->view |= CX_VIEW_edits;
		}
		if (!(copy = vmnewof(cx->vm, 0, Cxedit_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		*copy = *edit;
		edit = copy;
	}
	else
		dict = state.edits;
	if (dtsearch(dict, edit))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: edit already defined", edit->name);
		return -1;
	}
	dtinsert(dict, edit);
	if (!(edit->header.flags & CX_INITIALIZED))
	{
		edit->header.flags |= CX_INITIALIZED;
		if (edit->initf && !(edit->data = (*edit->initf)(edit, disc)))
			return -1;
	}
	return 0;
}

/*
 * return edit given name
 * optional substitute expression instantiated
 */

Cxedit_t*
cxedit(Cx_t* cx, const char* data, Cxdisc_t* disc)
{
	Cxedit_t*	e;
	Cxedit_t*	o;
	char*		s;

	e = (Cxedit_t*)dtmatch(cx ? cx->edits : state.edits, data);
	if (isalpha(*data))
	{
		if (!e)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: edit not defined", data);
			return 0;
		}
		o = e;
		if (!(e = cx ? vmnewof(cx->vm, 0, Cxedit_t, 1, 0) : newof(0, Cxedit_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		e->name = o->name;
		e->description = o->description;
		e->initf = o->initf;
		e->num2strf = o->num2strf;
		e->str2numf = o->str2numf;
	}
	else if (e)
		return e;
	else
	{
		if (!(e = cx ? vmnewof(cx->vm, 0, Cxedit_t, 1, strlen(data) + 1) : newof(0, Cxedit_t, 1, strlen(data) + 1)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		e->redisc.re_version = REG_VERSION;
		e->redisc.re_errorf = (regerror_t)disc->errorf;
		if (cx)
		{
			e->redisc.re_flags = REG_NOFREE;
			e->redisc.re_resizef = (regresize_t)vmgetmem;
			e->redisc.re_resizehandle = cx->vm;
		}
		e->re.re_disc = &e->redisc;
		s = (char*)data;
		if (regcomp(&e->re, s, REG_DELIMITED|REG_LENIENT|REG_NULL))
			return 0;
		s += e->re.re_npat;
		if (regsubcomp(&e->re, s, NiL, 0, 0))
			return 0;
		s += e->re.re_npat;
		e->re.re_npat = s - (char*)data;
		if (*s && disc->errorf)
			(*disc->errorf)(NiL, disc, 1, "invalid character after substitution: %s", s);
		strcpy((char*)(e->name = (const char*)(e + 1)), data);
		if (cx && cxaddedit(cx, e, disc))
			return 0;
	}
	return e;
}

/*
 * add a constraint
 */

int
cxaddconstraint(Cx_t* cx, register Cxconstraint_t* constraint, Cxdisc_t* disc)
{
	Dt_t*		dict;
	Cxconstraint_t*	copy;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_constraints)
			dict = cx->constraints;
		else if (!(dict = dtnew(cx->vm, &state.namedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->constraints);
			cx->constraints = dict;
			cx->view |= CX_VIEW_constraints;
		}
		if (!(copy = vmnewof(cx->vm, 0, Cxconstraint_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		*copy = *constraint;
		constraint = copy;
	}
	else
		dict = state.constraints;
	if (dtsearch(dict, constraint))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: constraint already defined", constraint->name);
		return -1;
	}
	if (!(constraint->header.flags & CX_INITIALIZED))
	{
		constraint->header.flags |= CX_INITIALIZED;
		if (constraint->initf && !(constraint->data = (*constraint->initf)(constraint, disc)))
			return -1;
	}
	dtinsert(dict, constraint);
	return 0;
}

/*
 * return constraint given name
 */

Cxconstraint_t*
cxconstraint(Cx_t* cx, const char* name, Cxdisc_t* disc)
{
	return (Cxconstraint_t*)dtmatch(cx ? cx->constraints : state.constraints, name);
}

/*
 * mark type CX_REFERENCED
 */

static void
referenced(register Cxtype_t* type)
{
	register Cxvariable_t*	mp;

	if (!(type->header.flags & CX_REFERENCED))
	{
		type->header.flags |= CX_REFERENCED;
		if (type->base)
			referenced(type->base);
		if (type->member)
			for (mp = (Cxvariable_t*)dtfirst(type->member->members); mp; mp = (Cxvariable_t*)dtnext(type->member->members, mp))
				referenced(mp->type);
	}
}

/*
 * add a variable
 */

int
cxaddvariable(register Cx_t* cx, register Cxvariable_t* variable, Cxdisc_t* disc)
{
	Dt_t*	dict;
	Cx_t*	sx;
	char*	name;

	if (cx)
	{
		disc = cx->disc;
		if (cx->view & CX_VIEW_variables)
			dict = cx->variables;
		else if (!(dict = dtnew(cx->vm, &state.namedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		else
		{
			dtview(dict, cx->variables);
			cx->variables = dict;
			cx->view |= CX_VIEW_variables;
		}
	}
	else
		dict = state.variables;
	if (!variable)
		return 0;
	if (dtsearch(dict, variable))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: variable already defined", variable->name);
		return -1;
	}
	if (!(variable->header.flags & CX_NORMALIZED))
	{
		variable->header.flags |= CX_NORMALIZED;
		if ((name = (char*)variable->type) && !(variable->type = cxtype(cx, name, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: %s: unknown type", variable->name, name);
			return -1;
		}
	}
	dtinsert(dict, variable);
	if (cx)
	{
		if (sx = cx->scope)
		{
			variable->header.index = sx->index++;
			cx->index = sx->index;
		}
		else
			variable->header.index = cx->index++;
	}
	if (!(variable->header.flags & CX_INITIALIZED))
	{
		variable->header.flags |= CX_INITIALIZED;
		referenced(variable->type);
		if (cx)
			dtinsert(cx->fields, variable);
	}
	return 0;
}

/*
 * return variable given name
 */

Cxvariable_t*
cxvariable(Cx_t* cx, const char* name, register Cxtype_t* m, Cxdisc_t* disc)
{
	register char*	s;
	register char*	t;
	Cxvariable_t*	v;
	Dt_t*		dict;
	Cxreference_t*	ref;
	Cxreference_t*	head;
	Cxreference_t*	tail;

	if (!cx)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: variable must be local", name);
		return 0;
	}
	disc = cx->disc;
	if (!m || !m->member)
	{
		if (!(v = (Cxvariable_t*)dtmatch(cx->variables, name)))
		{
			if (!(t = strchr(name, '.')))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s: undefined variable", name);
				return 0;
			}

			/*
			 * cxparse() never gets here
			 */

			strcpy(s = fmtbuf(strlen(name) + 1), name);
			t = s + (t - (char*)name);
			m = 0;
			dict = cx->variables;
			head = 0;
			for (;;)
			{
				if (t)
					*t++ = 0;
				v = (Cxvariable_t*)dtmatch(dict, *s ? s : ".");
				if (!v)
				{
					if (disc->errorf)
					{
						if (m)
							(*disc->errorf)(NiL, disc, 2, "%s: not a member of %s", s, m->name);

						else
							(*disc->errorf)(NiL, disc, 2, "%s: undefined variable", s);
					}
					return 0;
				}
				if (!(ref = newof(0, Cxreference_t, 1, 0)))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
					return 0;
				}
				ref->variable = v;
				if (m)
					ref->member = m->member;
				if (head)
					tail->next = ref;
				else
					head = ref;
				tail = ref;
				if (!(s = t))
					break;
				if ((!(m = v->type) || !m->member || !(dict = m->member->members)) &&
				    (!(m = v->type->base) || !m->member || !(dict = m->member->members)))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s: struct or union variable expected",  v->name);
					return 0;
				}
				t = strchr(t, '.');
			}
			if (!(v = newof(0, Cxvariable_t, 1, strlen(name) + 1)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return 0;
			}
			strcpy((char*)(v->name = (const char*)(v + 1)), name);
			v->reference = head;
			v->type = tail->variable->type;
			dtinsert(cx->variables, v);
		}
	}
	else if (!(v = (Cxvariable_t*)dtmatch(m->member->members, name)))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, 2, "%s: not a member of %s", name, m->name);
		return 0;
	}
	return v;
}

/*
 * cast var value to type
 * if var==0 or data==0 then the value is already in ret
 */

int
cxcast(Cx_t* cx, Cxoperand_t* ret, Cxvariable_t* var, Cxtype_t* type, void* data, const char* format)
{
	Cxinstruction_t	x;
	Cxoperand_t	val;
	Cxreference_t*	ref;
	Cxtype_t*	from;
	unsigned char*	map;
	Cxbuf_t*	cvt;
	char*		s;
	void*		d;
	ssize_t		n;
	Cxunsigned_t	m;
	int		c;
	Cxoperand_t	a;
	Cxoperand_t	b;
	Cxexternal_f	e;

	x.callout = 0;
	if (x.data.variable = var)
	{
		from = var->type;
		if (!type)
			type = from;
		if (data)
		{
			if (!cx->getf && !(cx->getf = cxcallout(cx, CX_GET, 0, 0, cx->disc)))
			{
				if (cx->disc->errorf)
					(*cx->disc->errorf)(NiL, cx->disc, 3, "%s: cx CX_GET callout must be defined", var->name);
				return -1;
			}
			a.type = state.type_string;
			a.refs = 0;
			a.value.string.size = (a.value.string.data = (char*)format) ? strlen(format) : 0;
			ret->type = var->type;
			ret->value.number = 0;
			if (ref = var->reference)
			{
				b.type = var->type;
				b.refs = 0;
				b.value.number = 0;
				x.data.variable = ref->variable;
				x.type = ret->type = ref->variable->type;
				if (var->name != var->type->name && (*cx->getf)(cx, &x, ret, &a, &b, data, cx->disc))
					return -1;
				while (ref = ref->next)
				{
					b.type = x.data.variable->type;
					x.data.variable = ref->variable;
					ret->type = x.data.variable->type;
					if ((*ref->member->getf)(cx, &x, ret, &a, &b, (ref->member->flags & CX_VIRTUAL) ? data : (void*)0, cx->disc))
						return -1;
				}
			}
			else if ((*cx->getf)(cx, &x, ret, &a, NiL, data, cx->disc))
				return -1;
			if (type == state.type_void)
				return 0;
			from = ret->type;
		}
	}
	else
		from = 0;
	if (var && var->format.map && var->format.map->part && var->format.map->part->edit)
		cxsuball(cx, var->format.map->part, ret);
	if ((var && (c = var->format.code) || (c = type->format.code)) &&
	    c != CC_NATIVE &&
	    (CCCONVERT(c) || !(type->format.flags & CX_BINARY) && (c = CCOP(c, CC_NATIVE))) &&
	    (map = ccmap(CCIN(c), CCOUT(c))))
	{
		if (ret->value.string.size > cx->ccsiz)
		{
			n = roundof(ret->value.string.size + 1, CX_CVT);
			if (!(cx->ccbuf = vmoldof(cx->vm, cx->ccbuf, char, n, 0)))
			{
				if (cx->disc->errorf)
					(*cx->disc->errorf)(NiL, cx->disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			cx->ccsiz = n;
		}
		ret->value.string.data = (char*)ccmapcpy(map, cx->ccbuf, ret->value.string.data, ret->value.string.size);
	}
	if (ret->type->representation != type->representation)
	{
		val = *ret;
		if (cxisstring(type))
		{
			if (!var || !cxisnumber(from) || !var->format.map || format || cxnum2str(cx, &var->format, (Cxinteger_t)ret->value.number, &s))
			{
				if (!(e = ret->type->externalf) && var && var->type->member)
					e = cxmembers;
				if (e)
				{
					if (!(cvt = cx->cvt))
					{
						if (!cx->top)
						{
							if (!(cx->top = vmnewof(cx->vm, 0, Cxbuf_t, 1, 0)) || !(cx->top->data = vmoldof(cx->vm, 0, char, CX_CVT, 0)))
							{
								if (cx->disc->errorf)
									(*cx->disc->errorf)(NiL, cx->disc, ERROR_SYSTEM|2, "out of space");
								return -1;
							}
							cx->top->size = CX_CVT;
						}
						cx->cvt = cx->top;
					}
					else
					{
						if (!cx->cvt->next)
						{
							if (!(cx->cvt->next = vmnewof(cx->vm, 0, Cxbuf_t, 1, 0)) || !(cx->cvt->next->data = vmoldof(cx->vm, 0, char, CX_CVT, 0)))
							{
								if (cx->disc->errorf)
									(*cx->disc->errorf)(NiL, cx->disc, ERROR_SYSTEM|2, "out of space");
								return -1;
							}
							cx->cvt->next->size = CX_CVT;
						}
						cx->cvt = cx->cvt->next;
					}
					while ((n = (*e)(cx, ret->type, format, var ? &var->format : (Cxformat_t*)0, &ret->value, cx->cvt->data, cx->cvt->size, cx->disc)) > cx->cvt->size)
					{
						n = roundof(n + 1, CX_CVT);
						if (!(cx->cvt->data = vmoldof(cx->vm, cx->cvt->data, char, n, 0)))
						{
							if (cx->disc->errorf)
								(*cx->disc->errorf)(NiL, cx->disc, ERROR_SYSTEM|2, "out of space");
							cx->cvt = cvt;
							return -1;
						}
						cx->cvt->size = n;
					}
					if (n < 0)
					{
						cx->cvt = cvt;
						return -1;
					}
					ret->value.string.data = cx->cvt->data;
					ret->value.string.size = n;
					cx->cvt = cvt;
				}
				else if (!cxisnumber(ret->type))
					goto bad;
				else
					ret->value.string.size = strlen(ret->value.string.data = sfprints("%Lg", val.value.number));
			}
			else
				ret->value.string.size = strlen(ret->value.string.data = s);
		}
		else if (var && data)
			goto bad;
		else if (cxisnumber(type) && var && cxisstring(from) && var->format.map && !cxstr2num(cx, &var->format, val.value.string.data, val.value.string.size, &m))
			ret->value.number = (Cxinteger_t)m;
#if 0
		else if (ret->type->representation != type->representation)
			goto bad;
#endif
		else if (!type->internalf || (*type->internalf)(cx, type, format, var ? &var->format : (Cxformat_t*)0, ret, val.value.string.data, val.value.string.size, cx->em, cx->disc) < 0)
			goto bad;
		ret->type = type;
	}
	return 0;
 bad:
	if (((x.callout = cxcallout(cx, CX_CAST, ret->type, type, cx->disc)) || ret->type->fundamental && (x.callout = cxcallout(cx, CX_CAST, ret->type->fundamental, type, cx->disc))) && !(*x.callout)(cx, &x, ret, NiL, &val, data, cx->disc))
		return 0;
	if (cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, 2, "cannot cast %s to %s", ret->type->name, type->name);
	return -1;
}

/*
 * generic struct type externalf that lists defined members in ksh93u compound notation
 */

ssize_t
cxmembers(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	Cxvariable_t*		v;
	unsigned char*		p;
	unsigned char*		e;
	char*			b;
	char*			m;
	char*			s;
	size_t			n;
	int			i;
	Cxoperand_t		o;
	Cxinstruction_t		x;

	if (!type->member || !value->buffer.data)
		return 0;
	b = buf;
	m = b + size;
	b += sfsprintf(b, m - b, "( ");
	for (v = (Cxvariable_t*)dtfirst(type->member->members); v; v = (Cxvariable_t*)dtnext(type->member->members, v))
	{
		if (!v->type->generic)
		{
			x.data.variable = v;
			o.type = v->type;
			o.value.buffer.data = value->buffer.data;
			if (!type->member->getf(cx, &x, &o, NiL, NiL, NiL, disc))
			{
				switch (cxrepresentation(v->type))
				{
				case CX_buffer:
					if (!o.value.buffer.size)
						continue;
					p = (unsigned char*)o.value.buffer.data;
					e = p + o.value.buffer.size;
					while (p < e && !*p)
						p++;
					if (p >= e)
						continue;
					break;
				case CX_number:
					if (!o.value.number)
						continue;
					break;
				case CX_string:
					if (!o.value.string.size)
						continue;
					break;
				case CX_void:
					continue;
				}
				if (!cxcast(cx, &o, v, cx->state->type_string, NiL, NiL))
					b += sfsprintf(b, m > b ? m - b : 0, "%s=%s ", v->name, o.value.string.data);
			}
		}
	}
	if ((n = b - buf) <= 2)
		n = 0;
	else
	{
		b += sfsprintf(b, m > b ? m - b : 0, ")");
		n++;
		if (n >= size)
			return n + 1;
		*b = 0;
	}
	return n;
}

/*
 * initialize the global state
 */

static void
initialize(Cxdisc_t* disc)
{
	register int		i;

	static const char	cx_alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_$";
	static const char	cx_digit[] = "0123456789";
	static const char	cx_float[] = "_.#";
	static const char	cx_space[] = " \f\n\t\r\v";

	if (!state.initialized++)
	{
		table.opcode['+'] = CX_ADD;
		table.opcode['&'] = CX_AND;
		table.opcode['/'] = CX_DIV;
		table.opcode['>'] = CX_GT;
		table.opcode['~'] = CX_INV;
		table.opcode['<'] = CX_LT;
		table.opcode['%'] = CX_MOD;
		table.opcode['*'] = CX_MPY;
		table.opcode['!'] = CX_NOT;
		table.opcode['|'] = CX_OR;
		table.opcode['='] = CX_SET;
		table.opcode['-'] = CX_SUB;
		table.opcode['^'] = CX_XOR;

		table.comparison[CX_LOG] =
		table.comparison[CX_LT] =
		table.comparison[CX_LE] =
		table.comparison[CX_EQ] =
		table.comparison[CX_NE] =
		table.comparison[CX_MATCH] =
		table.comparison[CX_NOMATCH] =
		table.comparison[CX_GE] =
		table.comparison[CX_GT] =	1;

		table.logical[CX_ANDAND] =
		table.logical[CX_LOG] =
		table.logical[CX_OROR] =
		table.logical[CX_NOT] =		1;

		table.precedence[CX_INV] =
		table.precedence[CX_LOG] =
		table.precedence[CX_NOT] =
		table.precedence[CX_UMINUS] =
		table.precedence[CX_UPLUS] =	15;
		table.precedence[CX_DIV] =
		table.precedence[CX_MOD] =
		table.precedence[CX_MPY] =	14;
		table.precedence[CX_ADD] =
		table.precedence[CX_SUB] =	13;
		table.precedence[CX_LSH] =
		table.precedence[CX_RSH] =	12;
		table.precedence[CX_GE] =
		table.precedence[CX_GT] =
		table.precedence[CX_LE] =
		table.precedence[CX_LT] =	11;
		table.precedence[CX_EQ] =
		table.precedence[CX_NE] =
		table.precedence[CX_MATCH] =
		table.precedence[CX_NOMATCH] =	10;
		table.precedence[CX_AND] =	9;
		table.precedence[CX_XOR] =	8;
		table.precedence[CX_AND] =
		table.precedence[CX_OR] =	7;
		table.precedence[CX_ANDAND] =	6;
		table.precedence[CX_OROR] =	5;
		table.precedence[CX_TST] =	4;
		table.precedence[CX_SET] =	3;
		table.precedence[CX_PAREN] =	2;
		table.precedence[CX_CALL] =	1;

		state.codedisc.key = offsetof(Cxcodeheader_t, op);
		state.codedisc.size = sizeof(Cxop_t);
		state.codedisc.link = offsetof(Cxcodeheader_t, header.link);
		state.listdisc.key = offsetof(Cxlistheader_t, name);
		state.listdisc.size = -1;
		state.listdisc.link = offsetof(Cxlistheader_t, header.list);
		state.namedisc.key = offsetof(Cxnameheader_t, name);
		state.namedisc.size = -1;
		state.namedisc.link = offsetof(Cxnameheader_t, header.link);

		if (!(state.libraries = dtopen(&state.namedisc, Dtqueue)) ||
		    !(state.methods = dtopen(&state.namedisc, Dtoset)) ||
		    !(state.types = dtopen(&state.namedisc, Dtoset)) ||
		    !(state.callouts = dtopen(&state.codedisc, Dtoset)) ||
		    !(state.recodes = dtopen(&state.codedisc, Dtoset)) ||
		    !(state.maps = dtopen(&state.namedisc, Dtoset)) ||
		    !(state.queries = dtopen(&state.namedisc, Dtoset)) ||
		    !(state.constraints = dtopen(&state.namedisc, Dtoset)) ||
		    !(state.edits = dtopen(&state.namedisc, Dtoset)) ||
		    !(state.variables = dtopen(&state.namedisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			goto panic;
		}

		for (i = 0; i < elementsof(types); i++)
		{
			if (cxaddtype(NiL, &types[i].type, disc))
				goto panic;
			if (types[i].state)
				*types[i].state = cxtype(NiL, types[i].type.name, disc);
		}
		for (i = 0; i < elementsof(callouts); i++)
			if (cxaddcallout(NiL, &callouts[i], disc))
				goto panic;
		for (i = 0; i < elementsof(builtins); i++)
			if (cxaddvariable(NiL, &builtins[i], disc))
				goto panic;

		for (i = 0; i < (sizeof(cx_alpha) - 1); i++)
			state.ctype[cx_alpha[i]] |= CX_CTYPE_ALPHA;
		for (i = 0; i < (sizeof(cx_digit) - 1); i++)
			state.ctype[cx_digit[i]] |= CX_CTYPE_DIGIT;
		for (i = 0; i < (sizeof(cx_float) - 1); i++)
			state.ctype[cx_float[i]] |= CX_CTYPE_FLOAT;
		for (i = 0; i < (sizeof(cx_space) - 1); i++)
			state.ctype[cx_space[i]] |= CX_CTYPE_SPACE;
	}
	return;
 panic:
	error(ERROR_PANIC, "%s library initialization error", CX_ID);
}

/*
 * return initialized global state pointer
 */

Cxstate_t*
cxstate(Cxdisc_t* disc)
{
	if (!state.initialized)
		initialize(disc);
	return &state;
}

/*
 * return the input location (path,record,offset) or the empty string
 */

char*
cxlocation(Cx_t* cx, void* data)
{
	char*	s;

	return cx->disc->locationf && (s = (*cx->disc->locationf)(cx, data, cx->disc)) ? s : "";
}

/*
 * for when only a 0-terminated string will do
 * copy n bytes of s to cvt buffer and 0-terminate it
 * pointer to cvt buf returned
 * data ok until the next cvtbuf() call
 */

char*
cxcvt(register Cx_t* cx, const char* s, size_t n)
{
	if (cx->cvtsiz <= n || !cx->cvtbuf)
	{
		cx->cvtsiz = roundof(n + 1, CX_CVT);
		if (!(cx->cvtbuf = vmoldof(cx->vm, cx->cvtbuf, char, cx->cvtsiz, 0)))
		{
			if (cx->disc->errorf)
				(*cx->disc->errorf)(NiL, cx->disc, ERROR_SYSTEM|2, "out of space");
			return (char*)s;
		}
	}
	memcpy(cx->cvtbuf, s, n);
	cx->cvtbuf[n] = 0;
	return cx->cvtbuf;
}
