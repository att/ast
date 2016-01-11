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
 * C expression library
 *
 * common Cxformat_t type attribute parse
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "cxlib.h"

#include <ccode.h>

typedef struct Attribute_s
{
	const char*	name;
	size_t		size;
	short		flags;
	short		width;
	short		base;
	short		code;
} Attribute_t;

#define S(s)		s,(sizeof(s)-1)

static Attribute_t	attributes[] =
{
	S("unsigned"),	CX_UNSIGNED|CX_INTEGER,	0,	0,	-1,
	S("decimal"),	CX_INTEGER,		0,	10,	-1,
	S("integer"),	CX_INTEGER,		0,	10,	-1,
	S("octal"),	CX_UNSIGNED|CX_INTEGER,	0,	8,	-1,
	S("hex"),	CX_UNSIGNED|CX_INTEGER,	0,	16,	-1,
	S("nul"),	CX_NUL,			0,	0,	-1,
	S("float"),	CX_FLOAT,		0,	0,	-1,
	S("double"),	CX_FLOAT,		0,	0,	-1,
	S("long"),	CX_LONG|CX_INTEGER,	0,	0,	-1,
	S("char"),	CX_INTEGER,		1,	0,	-1,
	S("ascii"),	0,			0,	0,	CC_ASCII,
	S("ebcdic"),	0,			0,	0,	CC_EBCDIC,
	S("ebcdic_e"),	0,			0,	0,	CC_EBCDIC_E,
	S("ebcdic_i"),	0,			0,	0,	CC_EBCDIC_I,
	S("ebcdic_o"),	0,			0,	0,	CC_EBCDIC_O,
	S("native"),	0,			0,	0,	CC_NATIVE,
};

/*
 * parse attributes from s into f and return the type
 * p!=0 set to the first char after the type or to the
 * first unrecognized attribute if type==0
 */

Cxtype_t*
cxattr(Cx_t* cx, register const char* s, char** p, Cxformat_t* f, Cxdisc_t* disc)
{
	register Attribute_t*	a;
	register const char*	t;
	char*			e;
	const char*		b;
	Cxtype_t*		type;
	int			n;
	char			buf[32];

	b = s;
	for (;;)
	{
		while (*s == ':' || isspace(*s))
			s++;
		if (!*s)
			break;
		if (!strncasecmp(s, "base=", 5))
		{
			f->base = (int)strtol(s + 5, &e, 10);
			s = (const char*)e;
		}
		else if (!strncasecmp(s, "code=", 5))
		{
			t = s + 5;
			switch (*t++)
			{
			case 'a':
			case 'A':
				n = CC_ASCII;
				break;
			case 'e':
			case 'E':
				n = CC_EBCDIC_E;
				break;
			case 'i':
			case 'I':
				n = CC_EBCDIC_I;
				break;
			case 'o':
			case 'O':
				n = CC_EBCDIC_O;
				break;
			case 'n':
			case 'N':
				n = CC_NATIVE;
				break;
			default:
				if (p)
					*p = (char*)s;
				return 0;
			}
			if (*t == '2')
				t++;
			switch (*t++)
			{
			case 'a':
			case 'A':
				n = CCOP(n, CC_ASCII);
				break;
			case 'e':
			case 'E':
				n = CCOP(n, CC_EBCDIC_E);
				break;
			case 'i':
			case 'I':
				n = CCOP(n, CC_EBCDIC_I);
				break;
			case 'o':
			case 'O':
				n = CCOP(n, CC_EBCDIC_O);
				break;
			case 'n':
			case 'N':
				n = CCOP(n, CC_NATIVE);
				break;
			case ':':
				break;
			default:
				if (p)
					*p = (char*)s;
				return 0;
			}
			s = t;
		}
		else if (!strncasecmp(s, "width=", 6))
		{
			f->width = (int)strtol(s + 6, &e, 10);
			s = (const char*)e;
		}
		else
		{
			for (t = s; *t && *t != ':' && !isspace(*t); t++);
			for (a = attributes;; a++)
			{
				if (a >= &attributes[elementsof(attributes)])
					goto last;
				if ((t - s) == a->size && !strncasecmp(s, a->name, t - s))
				{
					if (a->flags)
						f->flags |= a->flags;
					if (a->width)
						f->width = a->width;
					if (a->base)
						f->base = a->base;
					if (a->code >= 0)
						f->code = a->code;
					break;
				}
			}
			s = t;
		}
	}
 last:
	if (!cx || !isalpha(*s))
	{
		type = (!cx || (s == b)) ? (Cxtype_t*)0 : cxtype(cx, "number", disc);
		if (p)
			*p = (char*)s;
	}
	else
	{
		if (*t)
		{
			if ((t - s) >= sizeof(buf))
			{
				if (p)
					*p = (char*)s;
				return 0;
			}
			memcpy(buf, s, t - s);
			buf[t - s] = 0;
			e = buf;
		}
		else
			e = (char*)s;
		if (type = cxtype(cx, e, disc))
			s = t;
		else if (s != b)
			type = cxtype(cx, "number", disc);
		if (p)
			*p = (char*)s;
	}
	if (type && (f->flags & CX_FLOAT))
		f->base = 0;
	return type;
}
