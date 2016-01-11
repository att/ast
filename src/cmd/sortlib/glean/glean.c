/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2006-2011 AT&T Intellectual Property          *
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
"[-1lp0s5P?\n@(#)$Id: glean (AT&T Research) 2007-04-20 $\n]"
USAGE_LICENSE
"[+PLUGIN?glean - glean minima and/or maxima from record stream]"
"[+DESCRIPTION?The \bglean\b \bsort\b(1) discipline gleans minima "
    "and maxima from a record stream. Each record is categorized by the main "
    "sort key. If there is no main sort key then all records are in one "
    "category. If the \bmin\b key sorts less than the current category "
    "minimum or if the \bmax\b key sorts greater than the category maximum "
    "then the category minimum or maximum is updated and the record is "
    "written to the standard output. \bmax=\b\ak1\a,\bmax=\b\ak2\a sets one "
    "maximum using the two sort keys \ak1\a and \ak2\a. "
    "\bmax:=\b\ak1\a,\bmax:=\b\ak2\a sets two maxima, the first using sort "
    "key \ak1\a and the second using sort key \ak2\a.]"
"[+?Note that \b,\b is the plugin option separator. Literal \b,\b must "
    "either be quoted or escaped \aafter\a normal shell expansion.]"
"[a:absolute?List only the absolute minimum/maximum records for each "
    "category.]"
"[c:count?Precede each output record with its category index, "
    "category count, and total record count.]"
"[f:flush?Flush each line written to the standard output.]"
"[m:min?Mimima \bsort\b(1) key specification.]:[sort-key]"
"[M:max?Maxima \bsort\b(1) key specification.]:[sort-key]"
"[+EXAMPLES]"
    "{"
        "[+sort -t, -k1,1 -k2,2n -lglean,flush,min='\"3,3n\"',min='\"4,4\"' old.dat -?Sorts "
            "on the \b,\b separated fields 1 (string) and 2 (numeric) and "
            "lists the current minimal records per field 3 (numeric) and "
            "field 4 (string), for each value of the catenation of fields 1 and 2.]"
    "}"
"[+SEE ALSO?\bsort\b(1)]"
"\n\n--library=glean[,option[=value]...]\n\n"
;

#include <ast.h>
#include <debug.h>
#include <dt.h>
#include <error.h>
#include <recsort.h>
#include <vmalloc.h>

typedef struct Data_s
{
	void*		data;
	size_t		size;
	size_t		len;
} Data_t;

typedef struct Category_s
{
	Dtlink_t	link;
	Sfulong_t	count;
	unsigned int	index;
	Data_t		key;
	Data_t		lim[1];
} Category_t;

struct Field_s; typedef struct Field_s Field_t;

struct Field_s
{
	Field_t*	next;
	Rskey_t*	lim;
	int		mm;
	int		index;
	Category_t*	category;
	Sfulong_t	count;
	Sfulong_t	total;
	Data_t		absolute;
};

typedef struct State_s
{
	Rsdisc_t	rsdisc;
	Dtdisc_t	dtdisc;
	Rskeydisc_t	kydisc;
	Dt_t*		categories;
	unsigned int	index;
	Field_t*	field;
	unsigned int	fields;
	Data_t		key;
	Vmalloc_t*	vm;
	int		absolute;
	int		count;
	Category_t*	all;
	Sfulong_t	total;
} State_t;

static int
save(Vmalloc_t* vm, register Data_t* p, void* data, size_t len)
{
	if (p->size < len)
	{
		p->size = roundof(len, 256);
		if (!(p->data = vmnewof(vm, p->data, char, p->size, 0)))
		{
			error(ERROR_SYSTEM|2, "out of space [save]");
			return -1;
		}
	}
	p->len = len;
	if (data)
		memcpy(p->data, data, len);
	return 0;
}

static int
gleancmp(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	Category_t*	x = (Category_t*)a;
	Category_t*	y = (Category_t*)b;

	message((-4, "gleancmp a:%d:%-.*s: b:%d:%-.*s:", x->key.len, x->key.len, x->key.data, y->key.len, y->key.len, y->key.data));
	if (x->key.len < y->key.len)
		return -1;
	if (x->key.len > y->key.len)
		return 1;
	return memcmp(x->key.data, y->key.data, x->key.len);
}

static char*
fmtdata(void* data, size_t size)
{
	size_t	i;
	char*	b;
	char*	s;

	s = b = fmtbuf(2 * size + 1);
	for (i = 0; i < size; i++)
		s += sfsprintf(s, 3, "%02x", ((unsigned char*)data)[i]);
	return b;
}

static int
glean(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	State_t*		state = (State_t*)disc;
	register Rsobj_t*	r;
	register Category_t*	p;
	register Field_t*	f;
	Category_t		x;
	ssize_t			k;
	int			n;
	int			m;
	Data_t			t;

	switch (op)
	{
	case RS_POP:
		if (state->absolute)
			for (f = state->field; f; f = f->next)
			{
				if (state->count)
					sfprintf(sfstdout, "%u/%I*u/%I*u ", f->category->index, sizeof(f->count), f->count, sizeof(f->total), f->total);
				sfwrite(sfstdout, f->absolute.data, f->absolute.len);
			}
		vmclose(state->vm);
		return 0;
	case RS_READ:
		r = (Rsobj_t*)data;
		x.key.data = r->key;
		x.key.len = r->keylen;
		if (state->categories && !(p = (Category_t*)dtsearch(state->categories, &x)) || !state->categories && !(p = state->all))
		{
			if (!(p = vmnewof(state->vm, 0, Category_t, 1, (state->fields - 1) * sizeof(Data_t) + r->keylen)))
			{
				error(ERROR_SYSTEM|2, "out of space [category]");
				return -1;
			}
			p->index = ++state->index;
			p->key.len = r->keylen;
			p->key.data = (char*)(p + 1) + (state->fields - 1) * sizeof(Data_t);
			memcpy(p->key.data, r->key, r->keylen);
			if (state->categories)
				dtinsert(state->categories, p);
			else
				state->all = p;
		}
		state->total++;
		p->count++;
		message((-2, "glean record p=%p %I*u/%I*u key='%-.*s' r:%d:%-.*s: '%-.*s'", p, sizeof(p->count), p->count, sizeof(state->total), state->total, r->keylen, r->key, x.key.len, x.key.len, x.key.data, r->datalen && r->data[r->datalen - 1] == '\n' ? r->datalen - 1 : r->datalen, r->data));
		m = 0;
		for (f = state->field; f; f = f->next)
		{
			if (f->lim->disc->defkeyf)
			{
				if ((k = f->lim->disc->keylen) <= 0)
					k = 4;
				k *= r->datalen;
				if (save(state->vm, &state->key, 0, k))
					return -1;
				if ((k = (*f->lim->disc->defkeyf)(NiL, r->data, r->datalen, state->key.data, state->key.size, f->lim->disc)) < 0)
					return -1;
				t.len = state->key.len = k;
				t.data = state->key.data;
			}
			else
			{
				t.data = r->data + f->lim->disc->key;
				if ((k = f->lim->disc->keylen) <= 0)
					k += r->datalen - f->lim->disc->key;
				t.len = k;
			}
			message((-1, "glean [%d] %c a:%d:%s:", f->index, f->mm, t.len, fmtdata(t.data, t.len)));
			if (!p->lim[f->index].data)
				n = f->mm == 'm' ? -1 : 1;
			else
			{
				message((-1, "glean [%d] %c b:%d:%s:", f->index, f->mm, p->lim[f->index].len, fmtdata(p->lim[f->index].data, p->lim[f->index].len)));
				if ((k = t.len) < p->lim[f->index].len)
					k = p->lim[f->index].len;
				if (!(n = memcmp(t.data, p->lim[f->index].data, k)))
					n = t.len - p->lim[f->index].len;
			}
			if (f->mm == 'm' ? (n < 0) : (n > 0))
			{
				if (f->lim->disc->defkeyf)
				{
					t = state->key;
					state->key = p->lim[f->index];
					p->lim[f->index] = t;
				}
				else if (save(state->vm, &p->lim[f->index], t.data, t.len))
					return -1;
				if (!state->absolute)
					m = 1;
				else if (save(state->vm, &f->absolute, r->data, r->datalen))
					return -1;
				else
				{
					f->category = p;
					f->count = p->count;
					f->total = state->total;
				}
			}
		}
		if (m)
		{
			if (state->count)
				sfprintf(sfstdout, "%u/%I*u/%I*u ", p->index, sizeof(p->count), p->count, sizeof(state->total), state->total);
			sfwrite(sfstdout, r->data, r->datalen);
		}
		return RS_DELETE;
	}
	return -1;
}

Rsdisc_t*
rs_disc(Rskey_t* key, const char* options)
{
	register State_t*	state;
	register Field_t*	f;
	Vmalloc_t*		vm;
	int			i;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)) || !(state = vmnewof(vm, 0, State_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space [state]");
	state->vm = vm;
	key->type &= ~RS_DATA;
	state->dtdisc.link = offsetof(Category_t, link);
	state->dtdisc.comparf = gleancmp;
	state->kydisc.version = RSKEY_VERSION;
	state->kydisc.errorf = errorf;
	state->rsdisc.eventf = glean;
	state->rsdisc.events = RS_READ|RS_POP;
	if ((key->keydisc->flags & RSKEY_KEYS) && !(state->categories = dtnew(vm, &state->dtdisc, Dtoset)))
		error(ERROR_SYSTEM|3, "out of space [dictionary]");
	if (options)
	{
		for (;;)
		{
			switch (i = optstr(options, usage))
			{
			case 0:
				break;
			case 'a':
				state->absolute = opt_info.num;
				continue;
			case 'c':
				state->count = opt_info.num;
				continue;
			case 'f':
				sfset(sfstdout, SF_LINE, 1);
				continue;
			case 'm':
			case 'M':
				if (opt_info.assignment == ':' || !(f = state->field) || f->mm != i)
				{
					if (!(f = vmnewof(vm, 0, Field_t, 1, 0)) || !(f->lim = rskeyopen(&state->kydisc, NiL)))
						error(ERROR_SYSTEM|3, "out of space");
					strcpy((char*)f->lim->tab, (char*)key->tab);
					f->lim->type = key->type;
					f->mm = i;
					f->index = state->fields++;
					f->next = state->field;
					state->field = f;
				}
				if (rskey(f->lim, opt_info.arg, 0))
					goto drop;
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
		for (f = state->field; f; f = f->next)
			if (rskeyinit(f->lim))
				goto drop;
	}
	return &state->rsdisc;
 drop:
	for (f = state->field; f; f = f->next)
		rskeyclose(f->lim);
	vmclose(vm);
	return 0;
}

SORTLIB(glean)
