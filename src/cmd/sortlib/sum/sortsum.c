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

/*
 * sort uniq summary discipline
 */

static const char usage[] =
"[-1lp0s5P?\n@(#)$Id: sum (AT&T Research) 2011-08-23 $\n]"
USAGE_LICENSE
"[+PLUGIN?sum - sort uniq summary discipline]"
"[+DESCRIPTION?The \bsum\b \bsort\b(1) discipline applies "
    "summarization operations to selected fields in records that compare "
    "equal. The discipline sets the \bsort\b \b--unique\b option. Summary "
    "fields in non-unique records are modified according to the operations "
    "specified in the \bop\b discipline option.]"
"[l:library?Load the \bdss\b(1) type library \alibrary\a. Types are used "
    "by the \bop\b option. The \bnum_t\b library is loaded by default. \vdss "
    "--plugin=man\v lists the information on all \bdss\b libraries and \vdss "
    "--plugin=man\v \aname\a lists the information for the \aname\a "
    "library.]:[library]"
"[o:op?A field summary operation. \aarg\a is a \bdss\b(1) type name for "
    "all but the \bset\b \aop\a, either from the \bnum_t\b library or from a "
    "library loaded by the \blibrary\b option. \atype\a may also contain one "
    "or more \b:\b separated attributes. \akey\a is a \bsort\b(1) \b-k\b "
    "style field specification. \aop\a\b:\b\aarg\a may be specified multiple "
    "times; \aop\a and \aarg\a are inherited across \akey\a values from left "
    "to right. The default type is native character set \binteger\b; some "
    "operations may ignore the type. Spaces may be used in place of the "
    "\b:\b. \aop\a may be one of:]:[op::[arg...]]::key[...]]]"
    "{"
        "[+max (M)?maximum value]"
        "[+min (m)?minimum value]"
        "[+average (a)?average value]"
        "[+sum (s)?sum]"
        "[+count (c)?multiply subsequent field values and increment the "
            "total count by this value]"
        "[+set (s)?set all field field bytes to the first character of "
            "\aarg\a, which may be a C-style escape sequence]"
    "}"
"[r:regress?Massage output for regression testing.]"
"[d:debug?List the field operations on the standard error.]"
"[+EXAMPLES]"
    "{"
        "[+sort -k.2.1 -lsum,op=sum::integer::.6.2?Sorts on the 1 byte "
            "fixed width field starting at byte position 2 (counting from 1) "
            "and computes the sum of the integers in the 2 byte fixed width "
            "field starting at byte position 6.]"
        "[+dlls --base dss | grep '_t$'?Lists the \bdss\b(1) type "
            "library names.]"
        "[+dss --plugin=man num_t?Lists the \bdss\b(1) \bnum_t\b type "
            "library description in the \b--man\b style.]"
    "}"
"[+SEE ALSO?\bdss\b(1), \bsort\b(1)]"
"\n\n--library=sum[,option[=value]...]\n\n"
;

#include <ast.h>
#include <ctype.h>
#include <ccode.h>
#include <dss.h>
#include <error.h>
#include <recsort.h>
#include <recfmt.h>
#include <vmalloc.h>

struct Library_s; typedef struct Library_s Library_t;
struct Summary_s; typedef struct Summary_s Summary_t;

struct Library_s
{
	Library_t*	next;
	Cxtype_t*	types;
};

typedef struct Position_s
{
	short		field;
	short		index;
} Position_t;

struct Summary_s
{
	Summary_t*	next;
	Cxtype_t*	type;
	unsigned char*	map;
	unsigned char*	pam;
	Cxformat_t	format;
	Position_t	beg;
	Position_t	end;
	int		op;
	int		set;
	int		fixed;
	int		width;
	Sflong_t	count;
	Sfdouble_t	value;
};

typedef struct Buffer_s
{
	unsigned char*	buf;
	size_t		siz;
} Buffer_t;

typedef struct State_s
{
	Rsdisc_t	disc;
	Dss_t*		dss;
	Summary_t*	sum;
	Sflong_t	records;
	Recfmt_t	fmt;
	unsigned char*	tab;
	unsigned char	delim[256];
	int		alt;
	int		regress;
	Buffer_t	tmp;
	Buffer_t	buf[2];
} State_t;

#define ASSURE(s,b,z)	do{if(((b)->siz<(z))&&assure(s,b,z))return -1;}while(0)

static int
assure(State_t* state, Buffer_t* b, size_t z)
{
	if (b->siz < z)
	{
		b->siz = roundof(z, 32);
		if (!(b->buf = vmnewof(state->dss->vm, b->buf, unsigned char, b->siz, 0)))
		{
			error(ERROR_SYSTEM|3, "out of space extending to %I*u", sizeof(b->siz), b->siz);
			return -1;
		}
	}
	return 0;
}

static int
record(register State_t* state, register Rsobj_t* r, int op)
{
	Cx_t*				cx = state->dss->cx;
	register Summary_t*		sum;
	register unsigned char*		s;
	register unsigned char*		e;
	register unsigned char*		a;
	register unsigned char*		z;
	register unsigned char*		del;
	register const unsigned char*	map;
	unsigned char*			x;
	unsigned char*			tab;
	Buffer_t*			ext;
	int				beg;
	int				end;
	int				t;
	int				c;
	size_t				count;
	size_t				w;
	size_t				y;
	ssize_t				n;
	Cxoperand_t			v;

	state->records++;
	s = r->data;
	e = s + r->datalen - (RECTYPE(state->fmt) == REC_delimited);
	beg = end = 0;
	count = 1;
	tab = state->tab;
	t = *tab++;
	if (!*tab)
		tab = 0;
	del = state->delim;
	for (sum = state->sum; sum; sum = sum->next)
	{
		while (beg < sum->beg.field)
		{
		tab1:
			while (s < e)
				if (del[*s++])
				{
					if (tab)
					{
						for (c = 0; (s + c) < e; c++)
							if (!tab[c])
							{
								s += c;
								break;
							}
							else if (tab[c] != s[c])
								goto tab1;
					}
					else if (t == ' ')
						while (s < e && del[*s])
							s++;
					break;
				}
			end = ++beg;
		}
		if (sum->beg.index < (e - s))
		{
			a = s + sum->beg.index;
			while (end < sum->end.field)
			{
			tab2:
				while (s < e)
					if (del[*s++])
					{
						if (tab)
						{
							for (c = 0; (s + c) < e; c++)
								if (!tab[c])
								{
									s += c;
									break;
								}
								else if (tab[c] != s[c])
									goto tab2;
						}
						else if (t == ' ')
							while (s < e && del[*s])
								s++;
						break;
					}
				end++;
			}
			if (!sum->end.index)
			{
			tab3:
				while (s < e)
					if (del[*s++])
					{
						if (tab)
						{
							for (c = 0; (s + c) < e; c++)
								if (!tab[c])
									break;
								else if (tab[c] != s[c])
									goto tab3;
						}
						else if (t == ' ')
							while (s < e && del[*s])
								s++;
						s--;
						break;
					}
				z = s;
			}
			else if (sum->end.index <= (e - s))
				z = s + sum->end.index;
			else
				z = a;
		}
		else
			a = z = s;
		w = z - a;
		if (!sum->width)
			sum->format.width = RECTYPE(state->fmt) == REC_fixed ? w : (!(sum->format.flags & CX_FLOAT) || sum->end.index || w >= 8) ? 0 : 8;
		if (map = sum->map)
		{
			ASSURE(state, &state->tmp, w + 2);
			for (x = state->tmp.buf; a < z; *a++ = map[*x++]);
			map = sum->pam;
			x = state->tmp.buf;
			a -= w;
		}
		else
			x = a;
		if (sum->op == 'v' || (*sum->type->internalf)(cx, sum->type, NiL, &sum->format, &v, (char*)x, w, cx->rm, cx->disc) < 0)
			v.value.number = 0;
		else if (state->regress && (sum->format.flags & CX_FLOAT))
		{
			n = v.value.number * 1000.0;
			n /= 10;
			v.value.number = n;
		}
		if (op < 0)
		{
			sum->value = v.value.number;
			sum->count = 1;
		}
		else
		{
			if (count != 1)
				v.value.number *= count;
			switch (sum->op)
			{
			case 'a':
				sum->value += v.value.number;
				sum->count += count;
				break;
			case 'c':
				count = v.value.number;
				continue;
			case 'M':
				if (sum->value < v.value.number)
					sum->value = v.value.number;
				break;
			case 'm':
				if (sum->value > v.value.number)
					sum->value = v.value.number;
				break;
			case 's':
				sum->value += v.value.number;
				break;
			}
			if (op > 0)
			{
				v.value.number = sum->value;
				switch (sum->op)
				{
				case 'a':
					v.value.number /= sum->count;
					break;
				case 'v':
					while (a < z)
						*a++ = sum->set;
					continue;
				}
				n = (RECTYPE(state->fmt) == REC_fixed || w < 7) ? 7 : w;
				for (;;)
				{
					y = n + 1;
					ASSURE(state, &state->tmp, y);
					if ((n = (*sum->type->externalf)(cx, sum->type, NiL, &sum->format, &v.value, (char*)state->tmp.buf, y, cx->disc)) < 0)
					{
						error(2, "%s value %I*g conversion error", sum->type->name, sizeof(v.value.number), v.value.number);
						return -1;
					}
					if (n < y)
						break;
				}
				if (n > w)
				{
					if (sum->end.index || RECTYPE(state->fmt) == REC_fixed)
					{
						error(2, "%s value %I*g width exceeds %d", sum->type->name, sizeof(v.value.number), v.value.number, w);
						return -1;
					}
					ext = &state->buf[state->alt = !state->alt];
					ASSURE(state, ext, r->datalen + (n - w));
					memcpy(ext->buf, r->data, a - r->data);
					memcpy(ext->buf + (a - r->data) + n, a + w, r->datalen - (w + (a - r->data)));
					s = ext->buf + (s - r->data);
					a = ext->buf + (a - r->data);
					z = ext->buf + (z - r->data) + (n - w);
					r->data = ext->buf;
					r->datalen += n - w;
					e = s + r->datalen - (RECTYPE(state->fmt) == REC_delimited);
				}
				if (map)
				{
					if (n < w)
					{
						c = (sum->type->format.flags & CX_BINARY) ? 0 : map[' '];
						while (n++ < w)
							*a++ = c;
					}
					for (x = state->tmp.buf; a < z; *a++ = map[*x++]);
				}
				else
				{
					if (n < w)
					{
						c = (sum->type->format.flags & CX_BINARY) ? 0 : ' ';
						while (n++ < w)
							*a++ = c;
					}
					for (x = state->tmp.buf; a < z; *a++ = *x++);
				}
			}
		}
	}
	return 0;
}

static int
summary(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	State_t*		state = (State_t*)disc;
	register Rsobj_t*	r;
	register Rsobj_t*	q;

	switch (op)
	{
	case RS_POP:
		dssclose(state->dss);
		break;
	case RS_SUMMARY:
		r = (Rsobj_t*)data;
		for (op = -1, q = r->equal; q; op = 0, q = q->right)
			if (record(state, q, op))
				return -1;
		if (record(state, r, 1))
			return -1;
		break;
	default:
		return -1;
	}
	return 0;
}

Rsdisc_t*
rs_disc(Rskey_t* key, const char* options)
{
	register Summary_t*	sum;
	char*			s;
	char*			t;
	char*			b;
	char*			loc;
	State_t*		state;
	Cxtype_t*		type;
	Dss_t*			dss;
	Position_t*		pos;
	Summary_t*		cur;
	Summary_t*		def;
	Summary_t*		prv;
	int			tok;
	int			n;
	int			debug;
	char			chr;

	static Dssdisc_t	disc;

	dssinit(&disc, errorf);
	if (!(dss = dssopen(0, 0, &disc, dssmeth("dss", &disc))))
		return 0;
	if (!(state = vmnewof(dss->vm, 0, State_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	state->dss = dss;
	if (!dssload("num_t", dss->disc))
		goto drop;
	debug = 0;
	if (options)
	{
		for (;;)
		{
			switch (optstr(options, usage))
			{
			case 0:
				break;
			case 'd':
				debug = 1;
				continue;
			case 'l':
				if (!dssload(opt_info.arg, dss->disc))
					goto drop;
				continue;
			case 'o':
				def = 0;
				s = opt_info.arg;
				for (;;)
				{
					while (*s == ':' || isspace(*s))
						s++;
					if (!*s)
						break;
					if (!(sum = vmnewof(dss->vm, 0, Summary_t, 1, 0)))
						error(ERROR_SYSTEM|3, "out of space");
					sum->beg.field = -1;
					if (def)
					{
						sum->type = def->type;
						sum->format = def->format;
						sum->op = def->op;
						sum->set = def->set;
					}
					else
						sum->format.code = key->code;
					def = sum;
					b = s;
					tok = 0;
					/*UNDENT...*/
	for (;;)
	{
		if (*s == '.' || isdigit(*s))
		{
			pos = 0;
			while (*s == '.' || isdigit(*s))
			{
				if (!pos)
				{
					pos = &sum->beg;
					loc = "begin";
				}
				else if (pos == &sum->beg)
				{
					pos = &sum->end;
					loc = "end";
				}
				else
				{
					error(2, "%s: invalid summary field position", s);
					goto drop;
				}
				if (*s == '.')
					n = 1;
				else
					for (n = 0; *s >= '0' && *s <= '9'; n = n * 10 + (*s++ - '0'));
				if ((pos->field = n - 1) < 0)
				{
					error(2, "%d: invalid summary field %s position", n, loc);
					goto drop;
				}
				switch (*s)
				{
				case '.':
					for (n = 0; *++s >= '0' && *s <= '9'; n = n * 10 + (*s - '0'));
					if ((pos->index = n - 1) < 0)
					{
						error(2, "%d: invalid summary field %s offset", n, loc);
						goto drop;
					}
					if (*s == '.')
					{
						n = 0;
						if (pos == &sum->beg)
							for (n = 0; *++s >= '0' && *s <= '9'; n = n * 10 + (*s - '0'));
						if (n <= 0)
						{
							error(2, "%d: invalid summary field %s size", n, loc);
							goto drop;
						}
						sum->end.field = sum->beg.field;
						sum->end.index = sum->beg.index + n;
					}
					break;
				case 'C':
					s++;
					switch (*s++)
					{
					case 'a':
						n = CC_ASCII;
						break;
					case 'e':
						n = CC_EBCDIC_E;
						break;
					case 'i':
						n = CC_EBCDIC_I;
						break;
					case 'o':
						n = CC_EBCDIC_O;
						break;
					case 'n':
						n = CC_NATIVE;
						break;
					default:
						error(2, "%s: invalid code set", s - 1);
						goto drop;
					}
					switch (*s++)
					{
					case 'a':
						n = CCOP(n, CC_ASCII);
						break;
					case 'e':
						n = CCOP(n, CC_EBCDIC_E);
						break;
					case 'i':
						n = CCOP(n, CC_EBCDIC_I);
						break;
					case 'o':
						n = CCOP(n, CC_EBCDIC_O);
						break;
					case 'n':
						n = CCOP(n, CC_NATIVE);
						break;
					default:
						s--;
						break;
					}
					if (n && n != CC_NATIVE && CCIN(n) != CCOUT(n))
						sum->format.code = n;
					break;
				default:
					if (isalpha(*s))
					{
						error(2, "%s: invalid summary field attribute", s);
						goto drop;
					}
					break;
				}
			}
			break;
		}
		switch (tok)
		{
		case 0:
			switch (sum->op = *s++)
			{
			case 'a':
			case 'c':
				break;
			case 'M':
				if (*s == 'I')
					sum->op = 'm';
				break;
			case 'm':
				if (*s == 'a')
					sum->op = 'M';
				break;
			case 's':
				if (*s != 'e')
					break;
				sum->op = 'v';
				/*FALLTHROUGH*/
			case 'v':
				t = s - 1;
				while (isalnum(*s))
					s++;
				if (*s != ':' || !*++s)
				{
					error(2, "%s: summary field character value expected", t);
					goto drop;
				}
				sum->set = chresc(s, &s);
				break;
			default:
				error(2, "%s: invalid summary field operation", s - 1);
				goto drop;
			}
			while (isalnum(*s))
				s++;
			tok++;
			break;
		case 1:
			if (type = cxattr(dss->cx, s, &t, &sum->format, dss->cx->disc))
			{
				s = t;
				sum->type = type;
				sum->width = sum->format.width;
				tok++;
				break;
			}
			/*FALLTHROUGH*/
		default:
			error(2, "%s: invalid summary field specification", s);
			goto drop;
		}
		while (*s == ':' || isspace(*s))
			s++;
		if (!*s)
			break;
	}
					/*...INDENT*/
					if (sum->beg.field < 0)
					{
						error(2, "%s: field position expected", b);
						goto drop;
					}
					if (!sum->type)
						sum->type = cxattr(dss->cx, "integer", NiL, &sum->format, dss->cx->disc);
					for (prv = 0, cur = state->sum; cur; cur = (prv = cur)->next)
						if (sum->beg.field < cur->beg.field || sum->beg.field == cur->beg.field && sum->end.field < cur->end.field)
							break;
					if (prv)
						prv->next = sum;
					else
						state->sum = sum;
					sum->next = cur;
				}
				continue;
			case 'r':
				state->regress = 1;
				continue;
			case '?':
				error(ERROR_USAGE|4, "%s", opt_info.arg);
				goto drop;
			case ':':
				error(2, "%s", opt_info.arg);
				goto drop;
			}
			break;
		}
	}
	key->type &= ~RS_DATA;
	key->type |= RS_UNIQ;
	state->fmt = key->disc->data;
	if (!*key->tab || *key->tab == ' ')
	{
		state->tab = (unsigned char*)" ";
		for (n = 0; n < elementsof(state->delim); n++)
			if (isspace(n))
				state->delim[n] = 1;
	}
	else
		state->delim[*(state->tab = key->tab)] = 1;
	state->disc.eventf = summary;
	state->disc.events = RS_SUMMARY|RS_POP;
	for (sum = state->sum; sum; sum = sum->next)
		if (sum->format.code)
		{
			if (!CCCONVERT(sum->format.code))
			{
				if (sum->format.code == CC_NATIVE || (sum->type->format.flags & CX_BINARY))
					sum->format.code = 0;
				else
					sum->format.code = CCOP(sum->format.code, CC_NATIVE);
			}
			if (sum->format.code)
			{
				sum->map = ccmap(CCIN(sum->format.code), CCOUT(sum->format.code));
				sum->pam = ccmap(CCOUT(sum->format.code), CCIN(sum->format.code));
			}
		}
	if (debug || key->verbose)
		for (n = 1, sum = state->sum; sum; n++, sum = sum->next)
		{
			sfprintf(sfstderr, "op %d ", n);
			if (sum->beg.field == sum->end.field)
				sfprintf(sfstderr, ".%d.%d", sum->beg.index + 1, sum->end.index - sum->beg.index);
			else
				sfprintf(sfstderr, "%d.%d,%d.%d", sum->beg.field + 1, sum->beg.index + 1, sum->end.field + 1, sum->end.index);
			sfprintf(sfstderr, " %c", sum->op);
			if (sum->format.code)
				sfprintf(sfstderr, " %d=>%d ", CCIN(sum->format.code), CCOUT(sum->format.code));
			else
				sfprintf(sfstderr, "      ");
			if (sum->op == 'v')
			{
				chr = sum->set;
				sfprintf(sfstderr, "'%s'", fmtquote(&chr, NiL, "'", 1, 0));
			}
			else
				sfprintf(sfstderr, "%s", sum->type->name);
			sfprintf(sfstderr, "\n");
		}
	return &state->disc;
 drop:
	dssclose(dss);
	return 0;
}

SORTLIB(sum)
