/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * dss ip type library
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <dsslib.h>
#include <bgp.h>
#include <ire.h>
#include <itl.h>
#include <fv.h>
#include <pt.h>
#include <ptv.h>

#define AS16PATH_T	(&types[3])
#define AS32PATH_T	(&types[4])
#define IPV4ADDR_T	(&types[9])
#define IPV6ADDR_T	(&types[10])
#define IPV4PREFIX_T	(&types[12])
#define IPV6PREFIX_T	(&types[13])

/* XXX: some compilers choke on static foo bar[]; */

#define types		_static_types

Cxtype_t		types[];

#define PREFIX(a,b)	((Cxnumber_t)(a)*64+(b))

#if _typ_int64_t

#define PREFIX_ADDR(p)	((Ptaddr_t)(((Cxinteger_t)(p))>>6))
#define PREFIX_BITS(p)	((int)(((Cxinteger_t)(p)) & 0x3f))

#else

#define PREFIX_ADDR(p)	((Ptaddr_t)((p)/64))
#define PREFIX_BITS(p)	prefix_bits(p)

static int
prefix_bits(Cxnumber_t p)
{
	Cxnumber_t	a;
	unsigned long	u;

	u = p / 64;
	a = u;
	a *= 64;
	return (int)(p - a) & 0x3f;
}

#endif

static Iredisc_t	iredisc;

static ssize_t
addrv4_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	ssize_t	n;

	s = fmtip4((Ptaddr_t)value->number, -1);
	n = strlen(s);
	if ((n + 1) > size)
		return n + 1;
	memcpy(buf, s, n + 1);
	return n;
}

static ssize_t
addrv4_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	Ptaddr_t	addr;

	if (strtoip4(buf, &e, &addr, NiL))
	{
		if (disc->errorf && !(cx->flags & CX_QUIET))
			(*disc->errorf)(cx, disc, 1, "%-.*s: invalid ipv4 address", size, buf);
		return -1;
	}
	ret->value.number = addr;
	return e - (char*)buf;
}

static ssize_t
addrv6_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*		s;
	unsigned char*	pp;
	ssize_t		n;
	int		i;

	pp = (unsigned char*)value->buffer.data;
	if (!(s = (char*)CXDETAILS(details, format, type, 0)))
	{
		s = pp ? fmtip6(pp, -1) : "(nil)";
		n = strlen(s);
		if ((n + 1) > size)
			return n + 1;
		memcpy(buf, s, n + 1);
	}
	else if (s[0] == 'C' && s[1] == 0)
	{
		n = 80;
		if (size < n)
			return n;
		s = buf;
		for (i = 0; i < IP6BITS; i++)
		{
			if (i)
				*s++ = ',';
			s += sfsprintf(s, 6, "0x%02x",  pp ? pp[i] : 0);
		}
		*s = 0;
	}
	else
		n = -1;
	return n;
}

static ssize_t
addrv6_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	unsigned char*	ap;
	unsigned char	addr[IP6ADDR];

	if (strtoip6(buf, &e, addr, NiL))
	{
		if (disc->errorf && !(cx->flags & CX_QUIET))
			(*disc->errorf)(cx, disc, 1, "%-.*s: invalid ipv6 address", size, buf);
		return -1;
	}
	if (!vm)
		vm = Vmregion;
	if (!(ap = vmnewof(vm, 0, unsigned char, IP6ADDR, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	memcpy(ap, &addr, sizeof(addr));
	ret->value.buffer.data = ap;
	ret->value.buffer.size = sizeof(*ap);
	return e - (char*)buf;
}

static ssize_t
addr_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	ssize_t	n;

	if (!value->buffer.data || !value->buffer.size)
	{
		s = fmtip4((Ptaddr_t)0, -1);
		n = strlen(s);
		if ((n + 1) > size)
			return n + 1;
		memcpy(buf, s, n + 1);
		return n;
	}
	if (disc->errorf && !(cx->flags & CX_QUIET))
		(*disc->errorf)(cx, disc, 1, "unbound generic ip address");
	return -1;
}

static ssize_t
addr_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	Ptaddr_t	addrv4;
	unsigned char*	ap;
	unsigned char	addrv6[IP6ADDR];

	if (!strtoip4(buf, &e, &addrv4, NiL) && (e >= ((char*)buf + size) || !isalnum(*e) && *e != '.'))
	{
		ret->value.number = addrv4;
		ret->type = IPV4ADDR_T;
		return e - (char*)buf;
	}
	if (!strtoip6(buf, &e, addrv6, NiL) && (e >= ((char*)buf + size) || !isalnum(*e) && *e != '.'))
	{
		if (!vm)
			vm = Vmregion;
		if (!(ap = vmnewof(vm, 0, unsigned char, IP6ADDR, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		memcpy(ap, &addrv6, sizeof(addrv6));
		ret->value.buffer.data = ap;
		ret->value.buffer.size = sizeof(*ap);
		ret->type = IPV6ADDR_T;
		return e - (char*)buf;
	}
	if (disc->errorf && !(cx->flags & CX_QUIET))
		(*disc->errorf)(cx, disc, 1, "%-.*s: invalid ip address", size, buf);
	return -1;
}

static Cxtype_t*	addr_generic[] = { (Cxtype_t*)"ipv4addr_t", (Cxtype_t*)"ipv6addr_t", 0};

static Cxtype_t*	as_generic[] = { (Cxtype_t*)"as16_t", (Cxtype_t*)"as32_t", 0};

static ssize_t
as16path_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	return itl2external(cx, type, 0, 1, 1, details, &format, value, buf, size, disc);
}

static ssize_t
as16path_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl2internal(cx, &ret->value, 0, 1, 1, buf, size, vm, disc);
}

static ssize_t
as32_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	unsigned long	as;
	int		n;

	as = value->number;
	n = sfsprintf(buf, size, "%lu", as & 0xffff);
	if (n >= size)
		return n + 1;
	return n;
}

static ssize_t
as32_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	unsigned long	as;

	as = (unsigned int)strtoul(buf, &e, 10);
	if (*e == '.')
	{
		as <<= 16;
		as += (unsigned int)strtoul(e, &e, 10);
	}
	if (*e)
	{
		if (disc->errorf && !(cx->flags & CX_QUIET))
			(*disc->errorf)(cx, disc, 1, "%-.*s: invalid as32 number", size, buf);
		return -1;
	}
	ret->value.number = as;
	return e - (char*)buf;
}

static ssize_t
as32path_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	return itl4external(cx, type, 0, 1, 1, details, &format, value, buf, size, disc);
}

static ssize_t
as32path_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl4internal(cx, &ret->value, 0, 1, 1, buf, size, vm, disc);
}

static ssize_t
aspath_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	if (disc->errorf && !(cx->flags & CX_QUIET))
		(*disc->errorf)(cx, disc, 1, "unbound generic as path");
	return -1;
}

static ssize_t
aspath_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	register const char*	b = buf;
	register const char*	e = b + size;

	while (b < e && !isdigit(*b))
		b++;
	while (b < e && isdigit(*b))
		b++;
	if (b < e && *b == '.')
	{
		ret->type = AS32PATH_T;
		itl4internal(cx, &ret->value, 0, 1, 1, buf, size, vm, disc);
	}
	ret->type = AS16PATH_T;
	return itl2internal(cx, &ret->value, 0, 1, 1, buf, size, vm, disc);
}

typedef struct Path_match_s
{
	Ire_t*		ire16;
	Ire_t*		ire32;
	char		pat[1];
} Path_match_t;

static void*
aspath_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	Path_match_t*		pm;

	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	if (!(pm = newof(0, Path_match_t, 1, strlen(val->string.data))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	strcpy(pm->pat, val->string.data);
	return pm;
}

static int
aspath_match_exec(Cx_t* cx, void* data, Cxtype_t* type, Cxvalue_t* val, Cxdisc_t* disc)
{
	Path_match_t*		pm = (Path_match_t*)data;
	Ire_t*			ire;

	if (type->externalf == as32path_external)
	{
		if (!(ire = pm->ire32))
		{
			iredisc.version = IRE_VERSION;
			iredisc.errorf = disc->errorf;
			if (!(ire = irecomp(pm->pat, 4, 0, 1, 1, &iredisc)))
				return -2;
			pm->ire32 = ire;
		}
	}
	else if (!(ire = pm->ire16))
	{
		iredisc.version = IRE_VERSION;
		iredisc.errorf = disc->errorf;
		if (!(ire = irecomp(pm->pat, 2, 0, 1, 1, &iredisc)))
			return -2;
		pm->ire16 = ire;
	}
	return ireexec(ire, val->buffer.data, val->buffer.size) != 0;
}

static int
aspath_match_free(Cx_t* cx, void* data, Cxdisc_t* disc)
{
	Path_match_t*		pm = (Path_match_t*)data;

	if (pm->ire16)
		irefree(pm->ire16);
	if (pm->ire32)
		irefree(pm->ire32);
	free(pm);
	return 0;
}

static ssize_t
cluster_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	return itl4external(cx, type, 0, 1, 0, details, &format, value, buf, size, disc);
}

static ssize_t
cluster_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl4internal(cx, &ret->value, 0, 1, 0, buf, size, vm, disc);
}

static void*
cluster_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	iredisc.version = IRE_VERSION;
	iredisc.errorf = disc->errorf;
	return irecomp(val->string.data, 4, 0, 2, 0, &iredisc);
}

static ssize_t
community_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	Cxformat_t*	formats[2];

	formats[0] = 0;
	formats[1] = format;
	return itl2external(cx, type, 0, 2, 0, details, formats, value, buf, size, disc);
}

static ssize_t
community_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl2internal(cx, &ret->value, 0, 2, 0, buf, size, vm, disc);
}

static void*
community_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	iredisc.version = IRE_VERSION;
	iredisc.errorf = disc->errorf;
	return irecomp(val->string.data, 2, 0, 2, 0, &iredisc);
}

static ssize_t
extended_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	Cxformat_t*	formats[2];

	formats[0] = 0;
	formats[1] = format;
	return itl1external(cx, type, 0, 8, 0, details, formats, value, buf, size, disc);
}

static ssize_t
extended_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl1internal(cx, &ret->value, 0, 8, 0, buf, size, vm, disc);
}

static void*
extended_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	iredisc.version = IRE_VERSION;
	iredisc.errorf = disc->errorf;
	return irecomp(val->string.data, 1, 0, 8, 0, &iredisc);
}

static ssize_t
identifier_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	Cxformat_t*	formats[2];

	formats[0] = 0;
	formats[1] = format;
	return itl1external(cx, type, 0, 1, 0, details, formats, value, buf, size, disc);
}

static ssize_t
identifier_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl1internal(cx, &ret->value, 0, 1, 0, buf, size, vm, disc);
}

static void*
identifier_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	iredisc.version = IRE_VERSION;
	iredisc.errorf = disc->errorf;
	return irecomp(val->string.data, 1, 0, 1, 0, &iredisc);
}

static ssize_t
labels_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	Cxformat_t*	formats[2];

	formats[0] = 0;
	formats[1] = format;
	return itl4external(cx, type, 0, 2, 0, details, formats, value, buf, size, disc);
}

static ssize_t
labels_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl4internal(cx, &ret->value, 0, 2, 0, buf, size, vm, disc);
}

static void*
labels_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	iredisc.version = IRE_VERSION;
	iredisc.errorf = disc->errorf;
	return irecomp(val->string.data, 2, 0, 4, 0, &iredisc);
}

static ssize_t
values_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	Cxformat_t*	formats[2];

	formats[0] = 0;
	formats[1] = format;
	return itl4external(cx, type, 0, 1, 0, details, formats, value, buf, size, disc);
}

static ssize_t
values_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	return itl4internal(cx, &ret->value, 0, 1, 0, buf, size, vm, disc);
}

static void*
values_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	iredisc.version = IRE_VERSION;
	iredisc.errorf = disc->errorf;
	return irecomp(val->string.data, 1, 0, 4, 0, &iredisc);
}

static ssize_t
prefixv4_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	ssize_t	n;

	if (s = (char*)CXDETAILS(details, format, type, 0))
		s = sfprints(s, PREFIX_ADDR(value->number), PREFIX_BITS(value->number));
	else
		s = fmtip4(PREFIX_ADDR(value->number), PREFIX_BITS(value->number));
	n = strlen(s);
	if ((n + 1) > size)
		return n + 1;
	memcpy(buf, s, n + 1);
	return n;
}

static ssize_t
prefixv4_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	Ptaddr_t	addr;
	unsigned char	bits;

	if (strtoip4(buf, &e, &addr, &bits))
	{
		if (disc->errorf && !(cx->flags & CX_QUIET))
			(*disc->errorf)(cx, disc, 1, "%-.*s: invalid ipv4 prefix", size, buf);
		return -1;
	}
	ret->value.number = PREFIX(addr, bits);
	return e - (char*)buf;
}

static ssize_t
prefixv6_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*		s;
	unsigned char*	pp;
	ssize_t		n;
	int		i;

	pp = (unsigned char*)value->buffer.data;
	if (!(s = (char*)CXDETAILS(details, format, type, 0)))
	{
		s = pp ? fmtip6(pp, pp[IP6BITS]) : "(nil)";
		n = strlen(s);
		if ((n + 1) > size)
			return n + 1;
		memcpy(buf, s, n + 1);
	}
	else if (s[0] == 'C' && s[1] == 0)
	{
		n = 84;
		if (size < n)
			return n;
		s = buf;
		for (i = 0; i <= IP6BITS; i++)
		{
			if (i)
				*s++ = ',';
			s += sfsprintf(s, 6, "0x%02x",  pp ? pp[i] : 0);
		}
		*s = 0;
	}
	else
		n = -1;
	return n;
}

static ssize_t
prefixv6_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	unsigned char*	pp;
	unsigned char	prefix[IP6PREFIX];

	if (strtoip6(buf, &e, prefix, prefix + IP6BITS))
	{
		if (disc->errorf && !(cx->flags & CX_QUIET))
			(*disc->errorf)(cx, disc, 1, "%-.*s: invalid ipv6 address", size, buf);
		return -1;
	}
	if (!vm)
		vm = Vmregion;
	if (!(pp = vmnewof(vm, 0, unsigned char, IP6PREFIX, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	memcpy(ret->value.buffer.data = pp, prefix, IP6PREFIX);
	ret->value.buffer.size = IP6PREFIX;
	return e - (char*)buf;
}

static ssize_t
prefix_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	if (disc->errorf && !(cx->flags & CX_QUIET))
		(*disc->errorf)(cx, disc, 1, "unbound generic ip prefix");
	return -1;
}

static ssize_t
prefix_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	Ptaddr_t	prefixv4;
	unsigned char*	pp;
	unsigned char	prefixv6[IP6PREFIX];
	unsigned char	bits;

	if (!strtoip4(buf, &e, &prefixv4, &bits) && (e >= ((char*)buf + size) || !*e || isspace(*e)))
	{
		ret->value.number = PREFIX(prefixv4, bits);
		ret->type = IPV4PREFIX_T;
		return e - (char*)buf;
	}
	if (!strtoip6(buf, &e, prefixv6, prefixv6 + IP6BITS) && (e >= ((char*)buf + size) || !*e || isspace(*e)))
	{
		if (!vm)
			vm = Vmregion;
		if (!(pp = vmnewof(vm, 0, unsigned char, IP6PREFIX, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		memcpy(ret->value.buffer.data = pp, prefixv6, IP6PREFIX);
		ret->value.buffer.size = IP6PREFIX;
		ret->type = IPV6PREFIX_T;
		return e - (char*)buf;
	}
	if (disc->errorf && !(cx->flags & CX_QUIET))
		(*disc->errorf)(cx, disc, 1, "%-.*s: invalid ip address", size, buf);
	return -1;
}

static int
match_list_exec(Cx_t* cx, void* data, Cxtype_t* type, Cxvalue_t* val, Cxdisc_t* disc)
{
	return ireexec((Ire_t*)data, val->buffer.data, val->buffer.size) != 0;
}

static int
match_list_free(Cx_t* cx, void* data, Cxdisc_t* disc)
{
	return irefree((Ire_t*)data);
}

static Cxmatch_t	match_as16path =
{
	"as16path-re",
	"Matches on this type treat a string pattern as an ire(3) 16 bit integer list regular expression. Each number in the list is a distinct token. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. For example, '[!1 100]' matches all lists that contain neither 1 nor 100, and '^[!1 100]-701$' matches all lists that don't start with 1 or 100 and end with 701.",
	CXH,
	aspath_match_comp,
	aspath_match_exec,
	aspath_match_free
};

static Cxmatch_t	match_as32path =
{
	"as32path-re",
	"Matches on this type treat a string pattern as an ire(3) 32 bit integer list regular expression. Each number in the list is a distinct token. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. For example, '[!1 100]' matches all lists that contain neither 1 nor 100, and '^[!1 100]-701$' matches all lists that don't start with 1 or 100 and end with 701.",
	CXH,
	aspath_match_comp,
	aspath_match_exec,
	aspath_match_free
};

static Cxmatch_t	match_aspath =
{
	"aspath-re",
	0,
	CXH,
	aspath_match_comp,
	aspath_match_exec,
	aspath_match_free
};

static Cxtype_t*	aspath_generic[] = { (Cxtype_t*)"as16path_t", (Cxtype_t*)"as32path_t", 0};

static Cxmatch_t	match_cluster =
{
	"cluster-re",
	"Matches on this type treat a string pattern as an ire(3) integer list regular expression. Each number in the list is a distinct token. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. For example, '[!1 100]' matches all lists that contain neither 1 nor 100, and '^[!1 100]-701$' matches all lists that don't start with 1 or 100 and end with 701.",
	CXH,
	cluster_match_comp,
	match_list_exec,
	match_list_free
};

static Cxmatch_t	match_community =
{
	"community-re",
	"Matches on this type treat a string pattern as an ire(3) integer list regular expression. Each number in the list is a distinct token. Pairs are separated by :. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. If a : tuple separator is omitted then :.* is assumed. For example, '[!1 100]' matches all lists that contain neither 1 nor 100 as the first pair element, and '^[!1: :100]-701:999$' matches all lists that don't start with 1 as the first pair element or 100 as the second pair element and end with 701:999.",
	CXH,
	community_match_comp,
	match_list_exec,
	match_list_free
};

static Cxmatch_t	match_extended =
{
	"extended-re",
	"Matches on this type treat a string pattern as an ire(3) integer list regular expression for tuples with 8 integer elements in the range 0..255. Each number in the list is a distinct token. Elements are separated by :. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. If a : tuple separator is omitted then :.* is assumed. For example, '[!1 100]' matches all lists that contain neither 1 nor 100 as the first tuple element, and '^[!1: :100]-:201:199$' matches all lists that don't start with 1 as the first tuple element or 100 as the last tuple element and end with :201:199.",
	CXH,
	extended_match_comp,
	match_list_exec,
	match_list_free
};

static Cxmatch_t	match_identifier =
{
	"identifier-re",
	"Matches on this type treat a string pattern as an ire(3) integer list regular expression for tuples with 8 integer elements in the range 0..255. Each number in the list is a distinct token. Elements are separated by :. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. If a : tuple separator is omitted then :.* is assumed. For example, '[!1 100]' matches all lists that contain neither 1 nor 100 as the first tuple element, and '^[!1: :100]-:201:199$' matches all lists that don't start with 1 as the first tuple element or 100 as the last tuple element and end with :201:199.",
	CXH,
	identifier_match_comp,
	match_list_exec,
	match_list_free
};

static Cxmatch_t	match_labels =
{
	"labels-re",
	"Matches on this type treat a string pattern as an ire(3) integer list regular expression. Each number in the list is a distinct token. Pairs are separated by :. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. If a : tuple separator is omitted then :.* is assumed. For example, '[!1 100]' matches all lists that contain neither 1 nor 100 as the first pair element, and '^[!1: :100]-701:999$' matches all lists that don't start with 1 as the first pair element or 100 as the second pair element and end with 701:999.",
	CXH,
	labels_match_comp,
	match_list_exec,
	match_list_free
};

static Cxmatch_t	match_values =
{
	"values-re",
	"Matches on this type treat a string pattern as an ire(3) integer list regular expression. Each number in the list is a distinct token. ^ $ * + . {n,m} [N1 .. Nn] are supported, and - is equivalent to .*. Adjacent numbers may be separated by space, comma, / or _; multiple adjacent separators are ignored in the match. For example, '[!1 100]' matches all lists that contain neither 1 nor 100, and '^[!1 100]-701$' matches all lists that don't start with 1 or 100 as the first element and end with 701.",
	CXH,
	values_match_comp,
	match_list_exec,
	match_list_free
};

static Pt_t*
ptload(int str, Cxvalue_t* val, Ptdisc_t* ptdisc, Cxdisc_t* disc)
{
	Pt_t*		pt;
	Dssmeth_t*	meth;
	Dss_t*		dss;
	Dssfile_t*	ip;
	Bgproute_t*	rp;
	char*		s;
	char*		t;
	Ptaddr_t	addr;
	unsigned char	bits;

	if (!(pt = ptopen(ptdisc)))
		return 0;
	if (!str)
	{
		addr = PREFIX_ADDR(val->number);
		bits = PREFIX_BITS(val->number);
		if (!ptinsert(pt, PTMIN(addr, bits), PTMAX(addr, bits)))
		{
			ptclose(pt);
			return 0;
		}
	}
	else if (*(s = val->string.data) != '<')
	{
		while (!strtoip4(s, &t, &addr, &bits))
		{
			if (!ptinsert(pt, PTMIN(addr, bits), PTMAX(addr, bits)))
			{
				ptclose(pt);
				return 0;
			}
			s = t;
		}
	}
	else if ((meth = dssmeth("bgp", disc)) && (dss = dssopen(0, 0, disc, meth)))
	{
		if (ip = dssfopen(dss, s + 1, NiL, DSS_FILE_READ, NiL))
		{
			while (rp = (Bgproute_t*)dssfread(ip))
				if (!ptinsert(pt, PTMIN(rp->addr.v4, rp->bits), PTMAX(rp->addr.v4, rp->bits)))
				{
					dssfclose(ip);
					ptclose(pt);
					return 0;
				}
			dssfclose(ip);
		}
		dssclose(dss);
	}
	return pt;
}

typedef struct Matchdisc_s
{
	Ptdisc_t	ptdisc;
	int		prefix;
} Matchdisc_t;

static Ptv_t*
ptvload(int str, Cxvalue_t* val, Ptdisc_t* ptdisc, Cxdisc_t* disc)
{
	Ptv_t*		ptv;
	Dssmeth_t*	meth;
	Dss_t*		dss;
	Dssfile_t*	ip;
	Bgproute_t*	rp;
	char*		s;
	char*		t;
	unsigned char*	pp;
	unsigned char	prefix[IP6PREFIX];

	if (!(ptv = ptvopen(ptdisc, 16)))
		return 0;
	if (!str)
	{
		pp = (unsigned char*)val->buffer.data;
		if (!ptvinsert(ptv, ptvmin(ptv->size, ptv->r[0], pp, pp[IP6BITS]), ptvmax(ptv->size, ptv->r[1], pp, pp[IP6BITS])))
		{
			ptvclose(ptv);
			return 0;
		}
	}
	else if (*(s = val->buffer.data) != '<')
	{
		while (!strtoip6(s, &t, prefix, prefix + IP6BITS))
		{
			if (!ptvinsert(ptv, ptvmin(ptv->size, ptv->r[0], prefix, prefix[IP6BITS]), ptvmax(ptv->size, ptv->r[1], prefix, prefix[IP6BITS])))
			{
				ptvclose(ptv);
				return 0;
			}
			s = t;
		}
	}
	else if ((meth = dssmeth("bgp", disc)) && (dss = dssopen(0, 0, disc, meth)))
	{
		if (ip = dssfopen(dss, s + 1, NiL, DSS_FILE_READ, NiL))
		{
			while (rp = (Bgproute_t*)dssfread(ip))
				if (!ptvinsert(ptv, ptvmin(ptv->size, ptv->r[0], rp->prefixv6, rp->prefixv6[IP6BITS]), ptvmax(ptv->size, ptv->r[1], rp->prefixv6, rp->prefixv6[IP6BITS])))
				{
					dssfclose(ip);
					ptvclose(ptv);
					return 0;
				}
			dssfclose(ip);
		}
		dssclose(dss);
	}
	return ptv;
}

typedef struct Prefix_match_s
{
	Ptdisc_t	ptdisc;
	int		prefix;
	int		str;
	Pt_t*		pt;
	Ptv_t*		ptv;
	Cxvalue_t	val;
	char		pat[1];
} Prefix_match_t;

static void*
prefix_match_comp(Cx_t* cx, Cxtype_t* sub, Cxtype_t* pat, Cxvalue_t* val, Cxdisc_t* disc)
{
	Prefix_match_t*		pm;

	if (!cxisstring(pat))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: match requires %s pattern", sub->name, cx->state->type_string->name, sub->name);
		return 0;
	}
	if (!(pm = newof(0, Prefix_match_t, 1, strlen(val->string.data))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	strcpy(pm->pat, val->string.data);
	pm->val.string.data = pm->pat;
	pm->str = cxisstring(pat);
	return pm;
}

static int
prefix_match_exec(Cx_t* cx, void* data, Cxtype_t* type, Cxvalue_t* val, Cxdisc_t* disc)
{
	Prefix_match_t*		pm = (Prefix_match_t*)data;

	if (type->externalf == addrv6_external || type->externalf == prefixv6_external)
	{
		if (!pm->ptv)
		{
			ptvinit(&pm->ptdisc);
			pm->ptdisc.errorf = disc->errorf;
			pm->prefix = type->externalf == prefixv6_external;
			if (!(pm->ptv = ptvload(pm->str, &pm->val, &pm->ptdisc, disc)))
				return -2;
		}
		return ptvmatch(pm->ptv, (Ptvaddr_t)val->buffer.data) != 0;
	}
	if (!pm->pt)
	{
		ptinit(&pm->ptdisc);
		pm->ptdisc.errorf = disc->errorf;
		pm->prefix = type->externalf == prefixv6_external;
		if (!(pm->pt = ptload(pm->str, &pm->val, &pm->ptdisc, disc)))
			return -1;
	}
	return ptmatch(pm->pt, type->externalf == prefixv4_external ? PREFIX_ADDR(val->number) : (Ptaddr_t)val->number) != 0;
}

static int
prefix_match_free(Cx_t* cx, void* data, Cxdisc_t* disc)
{
	Prefix_match_t*		pm = (Prefix_match_t*)data;

	free(pm);
	return 0;
}

static Cxmatch_t	match_prefixv4 =
{
	"prefix-v4-match",
	"Matches on this type treat a string pattern as an ipv4 prefix table and test whether the subject is matched by the table. If the first character of the pattern is \b<\b then the remainder of the string is the path name of a file containing a prefix table. If the pattern is a \bipv4prefix_t\b then matches test if the subject is matched by the prefix.",
	CXH,
	prefix_match_comp,
	prefix_match_exec,
	prefix_match_free
};

static Cxmatch_t	match_prefixv6 =
{
	"prefix-v6-match",
	"Matches on this type treat a string pattern as an ipv6 prefix table and test whether the subject is matched by the table. If the first character of the pattern is \b<\b then the remainder of the string is the path name of a file containing a prefix table. If the pattern is an \bipv6prefix_t\b then matches test if the subject is matched by the prefix.",
	CXH,
	prefix_match_comp,
	prefix_match_exec,
	prefix_match_free
};

static Cxmatch_t	match_prefix =
{
	"prefix-match",
	0,
	CXH,
	prefix_match_comp,
	prefix_match_exec,
	prefix_match_free
};

static Cxtype_t*	prefix_generic[] = { (Cxtype_t*)"ipv4prefix_t", (Cxtype_t*)"ipv6prefix_t", 0};

static int
op_match_ip4_NP(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Ptaddr_t	aa;
	Ptaddr_t	ba;
	int		ab;
	int		bb;

	aa = PREFIX_ADDR(a->value.number);
	ab = PREFIX_BITS(a->value.number);
	ba = PREFIX_ADDR(b->value.number);
	bb = PREFIX_BITS(b->value.number);
	r->value.number = (PTMIN(aa,ab) >= PTMIN(ba,bb) && PTMAX(aa,ab) <= PTMAX(ba,bb)) == (pc->op == CX_MATCH);
	return 0;
}

static int
op_match_ip6_NP(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	unsigned char*	ap;
	unsigned char*	bp;
	unsigned char	r0[IP6ADDR];
	unsigned char	r1[IP6ADDR];

	if (!(ap = (unsigned char*)a->value.buffer.data) || !(bp = (unsigned char*)b->value.buffer.data))
		r->value.number = 0;
	else
		r->value.number = (fvcmp(16, ptvmin(16, r0, ap, ap[IP6BITS]), ptvmin(16, r1, bp, bp[IP6BITS])) >= 0 && fvcmp(16, ptvmax(16, r0, ap, ap[IP6BITS]), ptvmax(16, r1, bp, bp[IP6BITS])) <= 0) == (pc->op == CX_MATCH);
	return 0;
}

static Cxcallout_t callouts[] =
{

CXC(CX_MATCH,	"number",	"ipv4prefix_t",	op_match_ip4_NP,	0)
CXC(CX_MATCH,	"buffer",	"ipv6prefix_t",	op_match_ip6_NP,	0)

{0}

};

/*
 * NOTE: the *_T macros above index into this table
 */

Cxtype_t	types[] =
{
{ "as16_t", "An unsigned 16 bit autonomous system number.", CXH, (Cxtype_t*)"number", 0, 0, 0, 0, 0, 2, 0, { 0, 0, CX_UNSIGNED|CX_INTEGER, 2, 5 }, 0 },
{ "as32_t",	"A 32 bit autonomous system number.", CXH, (Cxtype_t*)"number", 0, as32_external, as32_internal, 0, 0, 4, 0, { 0, 0, CX_UNSIGNED|CX_INTEGER, 4, 11 }, 0 },
{ "as_t", 0, CXH, (Cxtype_t*)"number", 0, 0, 0, 0, 0, 0, 0, { 0, 0, CX_UNSIGNED|CX_INTEGER, 2, 5 }, 0, 0, &as_generic[0] },
{ "as16path_t", "A sequence of as16_t 16 bit autonomous system numbers.", CXH, (Cxtype_t*)"buffer", 0, as16path_external, as16path_internal, 0, 0, 0, 2, { "The format details string is the format character (\b1\b or \b.\b: dotted 1-byte, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: unsigned decimal (default)), followed by the separator string.", "u," }, &match_as16path },
{ "as32path_t", "A sequence of as32_t 32 bit autonomous system numbers.", CXH, (Cxtype_t*)"buffer", 0, as32path_external, as32path_internal, 0, 0, 0, 4, { "The format details string is the format character (\b1\b or \b.\b: dotted 1-byte, \b2\b: dotted 2-byte, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: unsigned decimal (default)), followed by the separator string.", "u," }, &match_as32path },
{ "aspath_t", 0, CXH, (Cxtype_t*)"buffer", 0, aspath_external, aspath_internal, 0, 0, 0, 0, { 0 }, &match_aspath, 0, &aspath_generic[0] },
{ "cluster_t", "A sequence of unsigned 32 bit integers.", CXH, (Cxtype_t*)"buffer", 0, cluster_external, cluster_internal, 0, 0, 0, 4, { "The format details string is the format character (\b.\b: dotted quad, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: default unsigned decimal), followed by the separator string.", ".," }, &match_cluster },
{ "community_t", "A sequence of unsigned 16 bit integer pairs.", CXH, (Cxtype_t*)"buffer", 0, community_external, community_internal, 0, 0, 0, 2, { "The format details string is the format character (\b.\b: dotted quad, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: default unsigned decimal), followed by the separator string.", "u," }, &match_community },
{ "extended_t", "A sequence of unsigned 64 bit integer tuples.", CXH, (Cxtype_t*)"buffer", 0, extended_external, extended_internal, 0, 0, 0, 8, { "The format details string is the format character (\b.\b: dotted elements, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: default unsigned decimal), followed by the separator string.", "u," }, &match_extended },
{ "ipv4addr_t",	"A dotted quad ipv4 address.", CXH, (Cxtype_t*)"number", 0, addrv4_external, addrv4_internal, 0, 0, 4, 0, { 0, 0, CX_UNSIGNED|CX_INTEGER, 4, 16 }, &match_prefixv4 },
{ "ipv6addr_t",	"An RFC 2373 ipv6 address. The details string \"C\" lists the prefix as 16 0x%02x comma-separated values.", CXH, (Cxtype_t*)"buffer", 0, addrv6_external, addrv6_internal, 0, 0, 16, 0, { 0 }, &match_prefixv6 },
{ "ipaddr_t",	0, CXH, (Cxtype_t*)"number", 0, addr_external, addr_internal, 0, 0, 0, 0, { 0, 0, CX_UNSIGNED|CX_INTEGER, 4, 16 }, &match_prefix, 0, &addr_generic[0] },
{ "ipv4prefix_t", "/length appended to an ipv4addr_t prefix.", CXH, (Cxtype_t*)"number", 0, prefixv4_external, prefixv4_internal, 0, 0, 5, 0, { "The format details string is a \bprintf\b(3) format specification for the integer arguments \aaddress,bits\a; e.g., \b%2$u|%1$08x\b prints the decimal bits followed by the hexadecimal prefix address.", 0, CX_UNSIGNED|CX_INTEGER, 8, 19 }, &match_prefixv4 },
{ "ipv6prefix_t", "/length appended to an ipv6addr_t prefix. The details string \"C\" lists the prefix as 17 0x%02x comma-separated values, the first 16 being the address, and the 17th being the number of prefix bits.", CXH, (Cxtype_t*)"buffer", 0, prefixv6_external, prefixv6_internal, 0, 0, 17, 0, { 0 }, &match_prefixv6 },
{ "ipprefix_t", 0, CXH, (Cxtype_t*)"number", 0, prefix_external, prefix_internal, 0, 0, 0, 0, { 0 }, &match_prefix, 0, &prefix_generic[0] },
{ "identifier_t", "A sequence of unsigned 8 bit integers.", CXH, (Cxtype_t*)"buffer", 0, identifier_external, identifier_internal, 0, 0, 0, 1, { "The format details string is the format character (\b.\b: dotted elements, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: default unsigned decimal), followed by the separator string.", "u," }, &match_identifier },
{ "labels_t", "A sequence of unsigned 32 bit integer pairs.", CXH, (Cxtype_t*)"buffer", 0, labels_external, labels_internal, 0, 0, 0, 8, { "The format details string is the format character (\b.\b: dotted quad, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: default unsigned decimal), followed by the separator string.", "u," }, &match_labels },
{ "values_t", "A sequence of unsigned 32 bit integers.", CXH, (Cxtype_t*)"buffer", 0, values_external, values_internal, 0, 0, 0, 8, { "The format details string is the format character (\b.\b: dotted quad, \bd\b: signed decimal, \bo\b: octal, \bx\b: hexadecimal, \bu\b: default unsigned decimal), followed by the separator string.", "u," }, &match_values },
{0}
};

Dsslib_t dss_lib_ip_t =
{
	"ip_t",
	"IP type support"
	"[-?\n@(#)$Id: dss ip type library (AT&T Research) 2013-04-09 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	0,
	&types[0],
	&callouts[0],
};
