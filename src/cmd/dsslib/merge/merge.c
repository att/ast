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

static const char merge_usage[] =
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \bmerge\b query merges the input files based on"
"	the ordering specified by \b--key\b and \b--reverse\b options."
"	The files must already be ordered; there is no verification."
"	Multiple \b--key\b and \b--reverse\b options may be specified.]"
"[k:key?\afield\a is a sort key in normal order.]:[field]"
"[r:reverse?\afield\a is a sort key in reverse order.]:[field]"
"\n"
"\n[ file ... ]\n"
"\n"
;

#include <dsslib.h>

struct File_s; typedef struct File_s File_t;
struct Key_s; typedef struct Key_s Key_t;
struct State_s; typedef struct State_s State_t;

struct File_s
{
	Dtlink_t	link;
	Dssfile_t*	file;
	Dssrecord_t*	record;
	Cxoperand_t*	data;
};

struct Key_s
{
	Key_t*		next;
	Cxvariable_t*	variable;
	int		sense;
};

struct State_s
{
	Dtdisc_t	orderdisc;
	Dt_t*		order;
	Dss_t*		dss;
	Cx_t*		cx;
	size_t		nkeys;
	size_t		nfiles;
	Key_t*		keys;
	Cxcallout_f	getf;
	File_t		files[1];
};

extern Dsslib_t	dss_lib_merge;

static int
ordercmp(Dt_t* dict, void* a, void* b, Dtdisc_t* disc)
{
	State_t*	state = (State_t*)disc;
	File_t*		ap = (File_t*)a;
	File_t*		bp = (File_t*)b;
	Key_t*		kp;
	size_t		az;
	size_t		bz;
	int		k;
	int		r;

	for (k = 0, kp = state->keys; k < state->nkeys; k++, kp = kp->next)
		if (cxisstring(kp->variable->type) || cxisbuffer(kp->variable->type))
		{
			az = ap->data[k].value.buffer.size;
			bz = bp->data[k].value.buffer.size;
			if (r = memcmp(ap->data[k].value.buffer.data, bp->data[k].value.buffer.data, az < bz ? az : bz))
				return (r < 0) ? -kp->sense : kp->sense;
			if (az < bz)
				return -kp->sense;
			if (az > bz)
				return kp->sense;
		}
		else if (ap->data[k].value.number < bp->data[k].value.number)
			return -kp->sense;
		else if (ap->data[k].value.number > bp->data[k].value.number)
			return kp->sense;
	if (ap < bp)
		return -1;
	if (ap > bp)
		return 1;
	return 0;
}

static void
enter(Dss_t* dss, register State_t* state, register File_t* file)
{
	register int	k;
	register Key_t*	kp;
	Cxinstruction_t	x;

	if (file->record)
		dtdelete(state->order, file);
	if (file->record = dssfread(file->file))
	{
		for (k = 0, kp = state->keys; k < state->nkeys; k++, kp = kp->next)
		{
			x.data.variable = kp->variable;
			if ((*state->getf)(state->cx, &x, file->data + k, NiL, NiL, file->record, state->cx->disc))
				return;
		}
		dtinsert(state->order, file);
	}
}

static int
merge_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state;
	register File_t*	file;
	char**			argv = (char**)data;
	char**			files = expr->files;
	Dss_t*			dss = DSS(cx);
	int			errors = error_info.errors;
	int			i;
	int			k;
	int			n;
	int			r;
	char*			path;
	char*			u;
	char**			v;
	Cxvariable_t*		variable;
	Cxoperand_t*		operands;
	Key_t*			key;
	Key_t*			lastkey;
	Key_t*			keys;
	Vmalloc_t*		vm;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	r = -1;
	k = 0;
	keys = 0;
	sfprintf(cx->buf, "%s%s", strchr(dss_lib_merge.description, '['), merge_usage);
	u = sfstruse(cx->buf);
	for (;;)
	{
		switch (i = optget(argv, u))
		{
		case 'k':
		case 'r':
			if (!(variable = cxvariable(cx, opt_info.arg, NiL, disc)))
				goto bad;
			if (!(key = vmnewof(vm, 0, Key_t, 1, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			key->variable = variable;
			key->sense = (i == 'r') ? -1 : 1;
			if (keys)
				lastkey = lastkey->next = key;
			else
				lastkey = keys = key;
			k++;
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
	for (v = argv; *v; v++);
	n = v - argv;
	if (files)
	{
		for (v = files; *v; v++);
		n += v - files;
	}
	if (!n)
		n = 1;
	if (!(state = vmnewof(vm, 0, State_t, 1, (n - 1) * sizeof(File_t))) || !(operands = vmnewof(vm, 0, Cxoperand_t, n * k, 0)))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	state->cx = cx;
	if (!(state->getf = cxcallout(cx, CX_GET, cx->state->type_void, cx->state->type_void, cx->disc)))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, 2, "CX_GET callout required");
		goto bad;
	}
	state->nfiles = n;
	state->nkeys = k;
	state->keys = keys;
	for (n = 0; n < state->nfiles; n++)
	{
		state->files[n].data = operands;
		operands += k;
	}
	state->orderdisc.comparf = ordercmp;
	if (!(state->order = dtnew(vm, &state->orderdisc, Dtoset)))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	n = 0;
	if (path = *argv)
		argv++;
	else if (files)
	{
		argv = files;
		files = 0;
		if (path = *argv)
			argv++;
	}
	for (;;)
	{
		if (!(state->files[n].file = dssfopen(dss, path, NiL, DSS_FILE_READ, NiL)))
			goto drop;
		enter(dss, state, &state->files[n]);
		n++;
		if (!(path = *argv++))
		{
			if (!files)
				break;
			argv = files;
			files = 0;
			if (!(path = *argv++))
				break;
		}
	}
	expr = expr->pass;
	if (dssbeg(dss, expr))
		goto drop;
	while (file = (File_t*)dtfirst(state->order))
	{
		if (dsseval(dss, expr, file->record) < 0)
			goto drop;
		enter(dss, state, file);
	}
	if (error_info.errors == errors)
		r = 0;
 drop:
	for (n = 0; n < state->nfiles; n++)
		if (state->files[n].file)
			dssfclose(state->files[n].file);
 bad:
	vmclose(vm);
	return r;
}

static Cxquery_t	queries[] =
{
	{
		"merge",
		"merge input files",
		CXH,
		merge_beg,
		0,
		0,
		0,
		0,
		0,
		1
	},
	{0}
};

Dsslib_t		dss_lib_merge =
{
	"merge",
	"merge query"
	"[-1lms5P?\n@(#)$Id: dss merge query (AT&T Research) 2003-02-14 $\n]"
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
