/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
 * ire re implementation
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <ast.h>
#include <vmalloc.h>

struct Ire_s;

typedef int (*Ireexec_f)(struct Ire_s*, void*, size_t);

#define _IRE_PRIVATE_ \
	Iredisc_t*	disc; \
	Vmalloc_t*	vm; \
	struct Re_s*	re; \
	int		left; \
	int		right; \
	int		must; \
	int		keepvm; \
	int		type; \
	Ireexec_f	execf;

#include "ire.h"

#include <error.h>

typedef  uint8_t Ireint1_t;
typedef uint16_t Ireint2_t;
typedef uint32_t Ireint4_t;

typedef struct Re_s
{
	struct Re_s*		next;
	int			invert;
	int			lo;
	int			hi;
	int			n;
	Ireint_t		id[1];
} Re_t;

#define irenewof(b,p,t,s,x)	(t*)irealloc(b, p, sizeof(t) * s + x)

static void*
irealloc(Ire_t* ire, void* p, size_t n)
{
	void*	r;

	if (ire->vm)
		r = vmnewof(ire->vm, p, char, n, 0);
	else
		r = (*ire->disc->resizef)(ire->disc->resizehandle, p, n);
	if (r)
		((Re_t*)r)->lo = ((Re_t*)r)->hi = 1;
	else if (ire->disc->errorf)
		(*ire->disc->errorf)(ire, ire->disc, ERROR_SYSTEM|2, "out of space");
	return r;
}

#define IREINT	Ireint1_t
#define IRENEXT	irenext1
#define IREEXEC	ireexec1

#include "ireexec.h"

#define IREINT	Ireint2_t
#define IRENEXT	irenext2
#define IREEXEC	ireexec2

#include "ireexec.h"

#define IREINT	Ireint4_t
#define IRENEXT	irenext4
#define IREEXEC	ireexec4

#include "ireexec.h"

/*
 * convert tuple number string
 */

Ireint_t
irenum(const char* s, char** e)
{
	register Ireint_t	n;
	char*			p;

	n = strton(s, &p, NiL, 0);
	for (;;)
	{
		s = (char*)p;
		if (*s != '.')
			break;
		n = (n << 8) | (strtol(s, &p, 0) & 0xff);
	}
	if (e)
		*e = p;
	return n;
}

/*
 * compile and return ire re handle
 */

Ire_t*
irecomp(const char* pattern, int element, int dots, int tuple, int group, Iredisc_t* disc)
{
	register char*	s;
	char*		e;
	int		m;
	int		mem;
	Ire_t*		ire;
	Vmalloc_t*	vm;
	Re_t*		re;
	Re_t*		pe;

	switch (element)
	{
	case 1:
	case 2:
	case 4:
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%d: element size must be one of { 1 2 4 }", element);
		return 0;
	}
	if (tuple <= 0)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%d: tuple size must > 0", tuple);
		return 0;
	}
	if (disc->resizef)
	{
		if (!disc->resizehandle)
			disc->resizehandle = (*disc->resizef)(NiL, NiL, 0);
		if (!(ire = (Ire_t*)(*disc->resizef)(disc->resizehandle, NiL, sizeof(Ire_t))))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			if (disc->resizehandle)
				(*disc->resizef)(disc->resizehandle, NiL, 0);
			return 0;
		}
		vm = 0;
	}
	else if (!(vm = vmopen(Vmdcheap, Vmlast, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	else if (!(ire = vmnewof(vm, 0, Ire_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		vmclose(vm);
		return 0;
	}
	ire->id = "ire";
	ire->disc = disc;
	ire->vm = vm;
	ire->element = element;
	ire->dots = dots;
	ire->tuple = tuple;
	s = (char*)pattern;
	pe = 0;
	mem = 0;
	for (;;)
	{
		if (++mem > tuple)
			mem = 1;
		if (!(re = irenewof(ire, 0, Re_t, 1, 0)))
			goto nospace;
		for (;;)
		{
			switch (*s++)
			{
			case 0:
				if (mem != 1)
				{
					s--;
					goto append;
				}
				if (!ire->left)
				{
					pe = 0;
					mem = 0;
					for (re = ire->re; re; re = re->next)
					{
						if (re->lo)
							break;
						if (++mem == tuple)
						{
							mem = 0;
							pe = re;
						}
					}
					if (pe)
						ire->re = pe;
				}
				switch (element)
				{
				case 1:
					ire->execf = (Ireexec_f)ireexec1;
					ire->group = 0xff;
					break;
				case 2:
					ire->execf = (Ireexec_f)ireexec2;
					ire->group = 0xffff;
					break;
				case 4:
					ire->execf = (Ireexec_f)ireexec4;
					ire->group = 0xffffffff;
					break;
				}
#if 0
sfprintf(sfstderr, "ire: element=%d tuple=%d group=%d left=%d right=%d must=%d type=%d\n", ire->element, ire->tuple, ire->group, ire->left, ire->right, ire->must, ire->type);
for (re = ire->re; re; re = re->next)
	sfprintf(sfstderr, "	id=%05u n=%d invert=%d lo=%d hi=%d\n", re->id[0], re->n, re->invert, re->lo, re->hi);
#endif
				return ire;
			case ':':
				s--;
				goto append;
			case ' ':
			case '\t':
			case '_':
			case ',':
			case '/':
				if (mem != 1)
				{
					s--;
					goto append;
				}
				continue;
			case '^':
				if (pe)
					goto syntax;
				ire->left = 1;
				continue;
			case '$':
				ire->right = 1;
				for (; *s == ' ' || *s == '\t'; s++);
				if (*s)
					goto syntax;
				continue;
			case '.':
				break;
			case '-':
				goto append;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				re->n = 1;
				re->id[0] = irenum(s - 1, &e);
				s = e;
				break;
			case '[':
				m = 1;
				for (;;)
				{
					switch (*s++)
					{
					case 0:
						goto syntax;
					case ' ':
					case '\t':
					case '_':
					case ',':
						continue;
					case '!':
					case '^':
						if (re->n || re->invert)
							goto syntax;
						re->invert = 1;
						continue;
					case ']':
						break;
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						if (re->n >= m)
						{
							if (m == 1)
								m = 8;
							else
								m *= 2;
							if (!(re = irenewof(ire, re, Re_t, 1, (m - 1) * sizeof(re->id[0]))))
								goto nospace;
						}
						re->id[re->n++] = irenum(s - 1, &e);
						s = e;
						continue;
					}
					if (re->n < m && !(re = irenewof(ire, re, Re_t, 1, (re->n - 1) * sizeof(re->id[0]))))
						goto nospace;
					break;
				}
				if (!re->n)
					goto syntax;
				break;
			default:
				goto syntax;
			}
			break;
		}
		for (;;)
		{
			switch (*s++)
			{
			case '*':
				re->lo = 0;
				re->hi = 0;
				break;
			case '+':
				re->hi = 0;
				break;
			case '{':
				if (!group)
					goto syntax;
				re->lo = (int)irenum(s, &e);
				for (s = e; *s == ' ' || *s == '\t'; s++);
				if (*s == ',')
				{
					re->hi = (int)irenum(s + 1, &e);
					for (s = e; *s == ' ' || *s == '\t'; s++);
				}
				else
					re->hi = re->lo;
				if (*s != '}' || re->lo > re->hi && re->hi)
					goto syntax;
				s++;
				break;
			default:
				s--;
				break;
			}
			break;
		}
	append:
		if (pe)
			pe->next = re;
		else
			ire->re = re;
		pe = re;
		ire->must += re->lo;
		if (*s == ':')
		{
			if (mem >= tuple)
				goto syntax;
			s++;
			if (*s != ' ' && *s != '\t')
				continue;
		}
		if (mem < tuple)
		{
			mem++;
			if (!(re = irenewof(ire, 0, Re_t, 1, 0)))
				goto nospace;
			goto append;
		}
	}
 syntax:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%-.*s<<<: invalid regular expression", s - (char*)pattern + 1, pattern);
 nospace:
	if (ire->vm)
		vmclose(ire->vm);
	return 0;
}

/*
 * match compiled ire re against data with size elements
 * return 1:match 0:no-match
 */

int
ireexec(Ire_t* ire, void* data, size_t size)
{
	if (!ire->re)
		return !ire->left || !ire->right || !size;
	return (*ire->execf)(ire, data, size);
}

/*
 * free ire re handle
 */

int
irefree(Ire_t* ire)
{
	if (!ire)
		return -1;
	if (ire->vm)
		vmclose(ire->vm);
	else if (ire->disc->resizehandle)
		(*ire->disc->resizef)(ire->disc->resizehandle, NiL, 0);
	return 0;
}
