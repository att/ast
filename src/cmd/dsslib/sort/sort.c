/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2011-2012 AT&T Intellectual Property          *
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

static const char sort_usage[] =
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The sort query writes the sorted input records to the"
"	standard output. The unsorted record stream is passed to the"
"	next query, if any. The sort keys are the \afield\a operands,"
"	or the raw record data if there are no operands.]"
"[c:count?Prepend an integer count field of the number of records that "
    "compare equal. Records with count less that \athreshhold\a are "
    "omitted.]#?[threshold:=1]"
"[r:reverse|invert?Reverse the sense of comparisons.]"
"[u:unique?Keep only the first of multiple records that compare equal on "
    "all keys.]"
"\n"
"\n[ field ... ]\n"
"\n"
"[+CAVEATS?Currently all data is sorted in memory -- spillover to "
    "temporary files not implemented yet.]"
;

#include <dsslib.h>
#include <recsort.h>
#include <stk.h>

struct State_s; typedef struct State_s State_t;

struct State_s
{
	Rsdisc_t	sortdisc;
	Rsdisc_t	uniqdisc;
	Rskeydisc_t	keydisc;
	Rskey_t*	sortkey;
	Rskey_t*	uniqkey;
	Rs_t*		sort;
	Rs_t*		uniq;
	Cx_t*		cx;
	Sfio_t*		op;
	Sfio_t*		sortstack;
	Sfio_t*		uniqstack;
	Sfio_t*		tmp;
	char*		sortbase;
	char*		uniqbase;
	void*		data;
	Dssfile_t*	file;
	Vmalloc_t*	rm;
	Vmalloc_t*	vm;
	size_t		count;
};

extern Dsslib_t	dss_lib_sort;

static ssize_t
key(Rs_t* rs, unsigned char* data, size_t datasize, unsigned char* key, size_t keysize, Rsdisc_t* disc)
{
	State_t*	state = (State_t*)disc;
	Rskeyfield_t*	field;
	Cxoperand_t	r;
	unsigned char*	k;
	unsigned char*	e;

	k = key;
	e = k + keysize;
	for (field = state->sortkey->head; field; field = field->next)
	{
		if (cxcast(state->cx, &r, (Cxvariable_t*)field->user, state->cx->state->type_string, state->data, NiL))
			return -1;
		k += field->coder(state->sortkey, field, (unsigned char*)r.value.string.data, r.value.string.size, k, e);
	}
	return k - key;
}

static ssize_t
rev(Rs_t* rs, unsigned char* data, size_t datasize, unsigned char* key, size_t keysize, Rsdisc_t* disc)
{
	State_t*	state = (State_t*)disc;

	return state->sortkey->head->coder(state->sortkey, state->sortkey->head, data, datasize, key, key + keysize);
}

static int
count(Rs_t* rs, int op, void* data, void* arg, Rsdisc_t* disc)
{
	State_t*	state = (State_t*)disc;
	Rsobj_t*	r;
	Rsobj_t*	q;
	char*		s;
	ssize_t		n;

	switch (op)
	{
	case RS_POP:
		break;
	case RS_WRITE:
		r = (Rsobj_t*)data;
		n = 1;
		for (q = r->equal; q; q = q->right)
			n++;
		if (n >= state->count)
		{
			n = sfprintf(state->uniqstack, "%I*u %-.*s", sizeof(n), n, r->datalen, r->data);
			s = stkfreeze(state->uniqstack, 0);
			if (rsprocess(state->uniq, s, -n) <= 0)
			{
				if (state->cx->disc->errorf)
					(*state->cx->disc->errorf)(state->cx, disc, ERROR_SYSTEM|2, "uniq record process error");
				return -1;
			}
		}
		return RS_DELETE;
	default:
		return -1;
	}
	return 0;
}

static int
sort_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**		argv = (char**)data;
	int		errors = error_info.errors;
	int		n;
	int		uniq;
	char*		s;
	char*		t;
	char*		num;
	State_t*	state;
	Cxvariable_t*	variable;
	Vmalloc_t*	vm;
	char		opt[2];

	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)) || !(state = vmnewof(vm, 0, State_t, 1, 0)))
	{
		if (vm)
			vmclose(vm);
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	state->vm = vm;
	state->keydisc.version = RSKEY_VERSION;
	state->keydisc.errorf = disc->errorf;
	if (!(state->sortkey = rskeyopen(&state->keydisc, &state->sortdisc)))
		goto bad;
	if (!(state->sort = rsnew(state->sortkey->disc)))
		goto bad;
	if (!(state->sortstack = stkopen(0)))
		goto bad;
	if (!(state->tmp = sfstropen()))
		goto bad;
	state->sortbase = stkptr(state->sortstack, 0);
	if (!(state->file = dssfopen(DSS(cx), "-", state->sortstack, DSS_FILE_WRITE, 0)))
		goto bad;
	sfprintf(cx->buf, "%s%s", strchr(dss_lib_sort.description, '['), sort_usage);
	s = sfstruse(cx->buf);
	uniq = 0;
	for (;;)
	{
		switch (optget(argv, s))
		{
		case 0:
			break;
		case 'c':
			state->count = (size_t)opt_info.number;
			continue;
		case 'u':
			uniq = 1;
			continue;
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
		default:
			opt[0] = opt_info.option[1];
			opt[1] = 0;
			if (rskeyopt(state->sortkey, opt, 1))
				goto bad;
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		goto bad;
	argv += opt_info.index;
	n = 0;
	num = state->sortkey->head && state->sortkey->head->rflag ? "nr" : "n";
	while (s = *argv++)
	{
		if (t = strchr(s, '-'))
		{
			sfwrite(cx->buf, s, t - s);
			s = sfstruse(cx->buf);
			t++;
		}
		else
			t = 0;
		if (!(variable = cxvariable(cx, s, NiL, disc)))
			goto bad;
		if (rskey(state->sortkey, t ? t : cxisnumber(variable->type) ? num : "", 0))
			goto bad;
		state->sortkey->tail->user = variable;
		n = 1;
	}
	if (uniq)
	{
		state->sortkey->type &= ~RS_DATA;
		state->sortkey->type |= RS_UNIQ;
	}
	if (state->count)
	{
		state->sortdisc.events |= RS_WRITE;
		state->sortdisc.eventf = count;
		if (!(state->uniqstack = stkopen(0)))
			goto bad;
		state->uniqbase = stkptr(state->uniqstack, 0);
		if (!(state->uniqkey = rskeyopen(&state->keydisc, &state->uniqdisc)))
			goto bad;
		if (!(state->uniq = rsnew(state->uniqkey->disc)))
			goto bad;
		if (rskey(state->uniqkey, "1n", 0))
			goto bad;
		if (rskeyinit(state->uniqkey))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "uniq key error");
			goto bad;
		}
		if (rsinit(state->uniq, state->uniqkey->meth, state->uniqkey->procsize, state->uniqkey->type, state->uniqkey))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "uniq initialization error");
			goto bad;
		}
	}
	if (rskeyinit(state->sortkey))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "sort key error");
		goto bad;
	}
	if (n)
	{
		state->sortdisc.defkeyf = key;
		state->sortdisc.key = 1;
	}
	else if (state->sortkey->head->rflag)
	{
		state->sortdisc.defkeyf = rev;
		state->sortdisc.key = 1;
	}
	if (rsinit(state->sort, state->sortkey->meth, state->sortkey->procsize, state->sortkey->type, state->sortkey))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "sort initialization error");
		goto bad;
	}
	state->cx = cx;
	state->op = expr->op;
	expr->data = state;
	return 0;
 bad:
	if (state->sort)
		rsclose(state->sort);
	if (state->uniq)
		rsclose(state->uniq);
	if (state->sortkey)
		rskeyclose(state->sortkey);
	if (state->uniqkey)
		rskeyclose(state->uniqkey);
	if (state->file)
		dssfclose(state->file);
	else if (state->sortstack)
		stkclose(state->sortstack);
	if (state->uniqstack)
		stkclose(state->uniqstack);
	if (state->tmp)
		sfstrclose(state->tmp);
	vmclose(state->vm);
	return -1;
}

static int
sort_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state = (State_t*)expr->data;
	Dssrecord_t*	record = (Dssrecord_t*)data;
	char*		s;
	ssize_t		n;

	if (dssfwrite(state->file, record))
		return -1;
	n = stktell(state->file->io);
	s = stkfreeze(state->file->io, 0);
	state->data = data;
	if (rsprocess(state->sort, s, -n) <= 0)
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "sort record process error");
		return -1;
	}
	return 0;
}

static int
sort_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	State_t*	state = (State_t*)expr->data;
	int		r;

	r = 0;
	if (rswrite(state->sort, expr->op, RS_OTEXT))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "sort output error");
		r = -1;
	}
	while (rsdisc(state->sort, NiL, RS_POP));
	if (rsclose(state->sort))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "sort close error");
		r = -1;
	}
	if (rskeyclose(state->sortkey))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "sort key error");
		r = -1;
	}
	if (state->uniq)
	{
		if (rswrite(state->uniq, expr->op, RS_OTEXT))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "uniq output error");
			r = -1;
		}
		while (rsdisc(state->uniq, NiL, RS_POP));
		if (rsclose(state->uniq))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "uniq close error");
			r = -1;
		}
		if (rskeyclose(state->uniqkey))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "uniq key error");
			r = -1;
		}
		stkclose(state->uniqstack);
	}
	dssfclose(state->file);
	sfstrclose(state->tmp);
	vmclose(state->vm);
	return r;
}

static Cxquery_t	queries[] =
{
	{
		"sort",
		"sort records to the standard output",
		CXH,
		sort_beg,
		0,
		sort_act,
		sort_end
	},
	{0}
};

Dsslib_t		dss_lib_sort =
{
	"sort",
	"sort query"
	"[-1lms5P?\n@(#)$Id: dss sort query (AT&T Research) 2011-10-18 $\n]"
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
