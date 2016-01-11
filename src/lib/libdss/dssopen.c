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
 * dss open/close/library/method implementation
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[-1ls5P?\n@(#)$Id: dss library (AT&T Research) 2012-09-04 $\n]"
USAGE_LICENSE
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \bdss\b default method provides types, global "
    "variables, and a schema available for all other methods.]"
"{\fvariables\f}"
"[+?The schema is a pure XML (tags "
    "only) file that specifies the \bdss\b method and optional field value "
    "maps and constraints. Public schemas are usually placed in a "
    "\b../lib/dss\b sibling directory on \b$PATH\b. The supported tags are:]"
"{"
;

#include "dsshdr.h"

#include <dlldefs.h>
#include <pzip.h>
#include <stak.h>

typedef Dsslib_t* (*Dsslib_f)(const char*, Dssdisc_t*);

static const char	id[] = DSS_ID;

static Dssstate_t	state;

#define DSS_MEM_file		1
#define DSS_MEM_format		2
#define DSS_MEM_length		3
#define DSS_MEM_offset		4
#define DSS_MEM_queried		5
#define DSS_MEM_record		6
#define DSS_MEM_selected	7

static Cxvariable_t dss_mem_struct[] =
{
CXV("file",     "string", DSS_MEM_file,     "Current record file name.")
CXV("format",   "string", DSS_MEM_format,   "Current record format.")
CXV("length",   "number", DSS_MEM_length,   "Current record length (always 0 for some formats.)")
CXV("offset",   "number", DSS_MEM_offset,   "Current record offset (always 0 for some formats.).")
CXV("queried",  "number", DSS_MEM_queried,  "Current queried record count.")
CXV("record",   "number", DSS_MEM_record,   "Current record number.")
CXV("selected", "number", DSS_MEM_selected, "Current selected record count.")
{0}
};

/*
 * find and open file for read
 */

Sfio_t*
dssfind(const char* name, const char* suffix, Dssflags_t flags, char* path, size_t size, Dssdisc_t* disc)
{
	Sfio_t*		sp;

	if (!suffix)
		suffix = id;
	if (*name == ':')
	{
		sfsprintf(path, size, "%s", "schema-string");
		sp = sfnew(NiL, (char*)name + 1, strlen(name) - 1, -1, SF_READ|SF_STRING);
	}
	else if (!pathfind(name, id, suffix, path, size))
	{
		if ((flags & DSS_VERBOSE) && disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: %s file not found", name, suffix);
		return 0;
	}
	else
		sp = sfopen(NiL, path, "r");
	if (!sp)
	{
		if ((flags & DSS_VERBOSE) && disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: cannot read %s file", path, suffix);
		return 0;
	}
	return sp;
}

/*
 * load tags file
 */

static Dssmeth_t*
loadtags(const char* name, const char* suffix, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Sfio_t*		sp;
	char		path[PATH_MAX];

	if (streq(name, DSS_ID))
		return meth;
	if (!(sp = dssfind(name, suffix, DSS_VERBOSE, path, sizeof(path), disc)))
		return 0;
	return dsstags(sp, path, 1, 0, disc, meth);
}

/*
 * dss identf
 */

static int
dssidentf(Dssfile_t* file, void* buf, size_t size, Dssdisc_t* disc)
{
	register char*	s;
	register char*	e;

	s = (char*)buf;
	e = s + size;
	while (s < e && isspace(*s))
		s++;
	if (*s++ != '<')
		return 0;
	if (*s == '!')
		while (s < e && *s++ != '!');
	else
		while (s < e && isalpha(*s))
			s++;
	return s < e && *s == '>';
}

/*
 * dss openf
 */

static int
dssopenf(Dssfile_t* file, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss readf
 */

static int
dssreadf(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss writef
 */

static int
dsswritef(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss seekf
 */

static Sfoff_t
dssseekf(Dssfile_t* file, Sfoff_t offset, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss closef
 */

static int
dssclosef(Dssfile_t* file, Dssdisc_t* disc)
{
	return 0;
}

static Dssformat_t	dss_format =
{
	&id[0],
	"pseudo-format that treats all files as /dev/null",
	CXH,
	dssidentf,
	dssopenf,
	dssreadf,
	dsswritef,
	dssseekf,
	dssclosef
};

static Dssmeth_t*	dssmethf(const char*, const char*, const char*, Dssdisc_t*, Dssmeth_t*);

static Dssmeth_t	dss_method =
{
	&id[0],
	"meta-method that specifies a method, value maps and constraints",
	CXH,
	dssmethf
};

/*
 * dss methf
 */

static Dssmeth_t*
dssmethf(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Sfio_t*		up;
	char*		us;
	Tagdisc_t	tagdisc;

	if (options)
	{
		if (!(up = sfstropen()))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		sfprintf(up, "%s", usage);
		taginit(&tagdisc, disc->errorf);
		if (tagusage(dss_tags, up, &tagdisc))
		{
			sfclose(up);
			return 0;
		}
		sfputc(up, '}');
		sfputc(up, '\n');
		if (!(us = sfstruse(up)))
		{
			sfclose(up);
			return 0;
		}
		dss_method.data = dss_mem_struct;
		state.global = &dss_method;
		for (;;)
		{
			switch (optstr(options, us))
			{
			case '?':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
				return 0;
			case ':':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
				return 0;
			}
			break;
		}
		state.global = 0;
		sfclose(up);
	}
	if (schema)
		return loadtags(schema, NiL, disc, meth);
	dtinsert(meth->formats, &dss_format);
	return meth;
}

#include "dss-compress.h"
#include "dss-count.h"
#include "dss-null.h"
#include "dss-print.h"
#include "dss-return.h"
#include "dss-scan.h"
#include "dss-write.h"

static Cxquery_t	queries[] =
{
	QUERY_compress,
	QUERY_count,
	QUERY_null,
	QUERY_print,
	QUERY_return,
	QUERY_scan,
	QUERY_write,
	{0},
};

static Dsslib_t		dss_library =
{
	&id[0],
	"dss method",
	CXH,
	0,
	&dss_method,
	0,
	0,
	0,
	0,
	&queries[0],
	0,
	0,
	0
};

/*
 * initialize library given name and dlopen() handle
 */

static Dsslib_t*
init(void* dll, Dsslib_t* lib, const char* path, Dssflags_t flags, Dssdisc_t* disc)
{
	Dsslib_f	libf;
	char		buf[64];

	/*
	 * check for the Dsslib_t* function
	 */

	if (dll)
	{
		sfsprintf(buf, sizeof(buf), "%s_lib", id);
		lib = (libf = (Dsslib_f)dlllook(dll, buf)) ? (*libf)(path, disc) : (Dsslib_t*)0;
	}
	if (lib)
	{
		if (!(lib->path = (const char*)strdup(path)))
		{
			if (disc && disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		if (!dtsearch(state.cx->libraries, lib))
			dtinsert(state.cx->libraries, lib);
		return lib;
	}
	if ((flags & DSS_VERBOSE) && disc && disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: %s: initialization function not found in library", path, buf);
	return 0;
}

/*
 * open and return library info for name
 * name==0 scans for all related libraries on $PATH
 */

Dsslib_t*
dsslib(const char* name, Dssflags_t flags, Dssdisc_t* disc)
{
	register Dsslib_t*	lib;
	Dllscan_t*		dls;
	Dllent_t*		dle;
	void*			dll;
	Dllnames_t		names;

	dssstate(disc);
	if (!name)
	{
		if (!state.scanned)
		{
			state.scanned++;
			if (dtsize(state.cx->libraries) == 1 && (dls = dllsopen(id, NiL, NiL)))
			{
				while (dle = dllsread(dls))
					if (dll = dlopen(dle->path, RTLD_LAZY))
						init(dll, NiL, dle->path, 0, disc);
					else if (disc && disc->errorf)
						(*disc->errorf)(NiL, disc, 1, "%s: %s", dle->path, dlerror());
				dllsclose(dls);
			}
		}
		return (Dsslib_t*)dtfirst(state.cx->libraries);
	}
	if (!dllnames(id, name, &names))
		return 0;
	if ((lib = (Dsslib_t*)dtmatch(state.cx->libraries, names.base)) ||
	    (lib = (Dsslib_t*)dll_lib(&names, DSS_PLUGIN_VERSION, (flags & DSS_VERBOSE) ? (Error_f)disc->errorf : (Error_f)0, disc)))
		init(NiL, lib, names.path, flags|DSS_VERBOSE, disc);
	return lib;
}

/*
 * add lib tables
 */

int
dssadd(register Dsslib_t* lib, Dssdisc_t* disc)
{
	register int	i;
	int		schema;

	if (lib->header.flags & CX_INITIALIZED)
		return 0;
	lib->header.flags |= CX_INITIALIZED;
	if (lib->libraries)
		for (i = 0; lib->libraries[i]; i++)
			if (!dssload(lib->libraries[i], disc))
				return -1;
	if (lib->types)
	{
		schema = -1;
		for (i = 0; lib->types[i].name; i++)
		{
			if (cxaddtype(NiL, &lib->types[i], disc))
				return -1;
			if (lib->types[i].header.flags & CX_SCHEMA)
				schema = i;
		}
		for (i = 0; i <= schema; i++)
			if ((lib->types[i].header.flags & (CX_INITIALIZED|CX_SCHEMA)) == CX_SCHEMA)
			{
				lib->types[i].header.flags |= CX_INITIALIZED;
				lib->types[i].member->members = state.cx->variables;
			}
	}
	if (lib->callouts)
		for (i = 0; lib->callouts[i].callout; i++)
			if (cxaddcallout(NiL, &lib->callouts[i], disc))
				return -1;
	if (lib->recodes)
		for (i = 0; lib->recodes[i].recode; i++)
			if (cxaddrecode(NiL, &lib->recodes[i], disc))
				return -1;
	if (lib->maps)
		for (i = 0; lib->maps[i]; i++)
			if (cxaddmap(NiL, lib->maps[i], disc))
				return -1;
	if (lib->queries)
		for (i = 0; lib->queries[i].name; i++)
			if (cxaddquery(NiL, &lib->queries[i], disc))
				return -1;
	if (lib->constraints)
		for (i = 0; lib->constraints[i].name; i++)
			if (cxaddconstraint(NiL, &lib->constraints[i], disc))
				return -1;
	if (lib->edits)
		for (i = 0; lib->edits[i].name; i++)
			if (cxaddedit(NiL, &lib->edits[i], disc))
				return -1;
	if (!dtsearch(state.cx->libraries, lib))
		dtinsert(state.cx->libraries, lib);
	return 0;
}

/*
 * find and add library name
 */

Dsslib_t*
dssload(const char* name, Dssdisc_t* disc)
{
	Dsslib_t*	lib;

	if (!(lib = dsslib(name, DSS_VERBOSE, disc)))
		return 0;
	return dssadd(lib, disc) ? 0 : lib;
}

/*
 * return the input location string for data
 * suitable for errorf
 */

static char*
location(Cx_t* cx, void* data, Cxdisc_t* disc)
{
	register Dssfile_t*	file = DSSRECORD(data)->file;
	char*			path;
	char*			sep;
	char*			loc;
	char*			nxt;
	char*			end;
	size_t			n;

	if (!file)
		return "";
	if (path = strrchr(file->path, '/'))
		path++;
	else
		path = file->path;
	n = strlen(path) + 3;
	sep = ": ";
	if (file->count || file->offset)
	{
		sep = ", ";
		n += 64;
	}
	loc = nxt = fmtbuf(n);
	end = loc + n;
	nxt += sfsprintf(nxt, end - nxt, "%s%s", path, sep);
	if (file->count || file->offset)
		sfsprintf(nxt, end - nxt, "%s %I*u, %s %I*d: ", ERROR_translate(NiL, NiL, id, "record"), sizeof(file->count), file->count, ERROR_translate(NiL, NiL, id, "offset"), sizeof(file->offset), file->offset);
	return loc;
}

static int
dss_mem_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Dssfile_t*	file = DSSRECORD(data)->file;

	switch (pc->data.variable->index)
	{
	case DSS_MEM_file:
		r->value.string.data = (char*)file->path;
		r->value.string.size = strlen(file->path);
		break;
	case DSS_MEM_format:
		r->value.string.data = (char*)file->format->name;
		r->value.string.size = strlen(file->format->name);
		break;
	case DSS_MEM_length:
		r->value.number = file->length;
		break;
	case DSS_MEM_offset:
		r->value.number = file->offset;
		break;
	case DSS_MEM_queried:
		r->value.number = cx->expr ? cx->expr->parent->queried : 0;
		break;
	case DSS_MEM_record:
		r->value.number = file->count;
		break;
	case DSS_MEM_selected:
		r->value.number = cx->expr ? cx->expr->parent->selected : 0;
		break;
	default:
		return -1;
	}
	return 0;
}

static Cxmember_t	dss_mem =
{
	dss_mem_get,
	0,
	(Dt_t*)&dss_mem_struct[0],
	CX_VIRTUAL
};

static Cxtype_t		dss_type[] =
{
	{ DSS_ID "_s", "Global state struct.", CXH, (Cxtype_t*)"void", 0, 0, 0, 0, 0, 0, 0, {0}, 0, &dss_mem },
	{ DSS_ID "_t", "Global state.", CXH, (Cxtype_t*)DSS_ID "_s" },
};

static Cxvariable_t	dss_var[] =
{
	CXV(".",	DSS_ID "_t",	0,	"Global State.")
	CXV(DSS_ID,	DSS_ID "_t",	0,	"Global State.")
};

/*
 * open a dss session
 */

Dss_t*
dssopen(Dssflags_t flags, Dssflags_t test, Dssdisc_t* disc, Dssmeth_t* meth)
{
	register Dss_t*		dss;
	register Vmalloc_t*	vm;
	Cxvariable_t*		var;
	Dsslib_t*		lib;
	int			i;

	if (!disc)
		return 0;
	dssstate(disc);
	if (!meth)
	{
		/*
		 * find the first user library that defines a method
		 */

		lib = (Dsslib_t*)dtfirst(state.cx->libraries);
		while ((lib = (Dsslib_t*)dtnext(state.cx->libraries, lib)) && !lib->meth);
		if (lib)
			meth = lib->meth;
		else
		{
			if (!(flags & DSS_QUIET) && disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "a method must be specified");
			return 0;
		}
	}
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (!(dss = vmnewof(vm, 0, Dss_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		vmclose(vm);
		return 0;
	}
	if (!disc->loadf)
		disc->loadf = dssload;
	if (!disc->locationf)
		disc->locationf = location;
	dss->id = id;
	dss->vm = vm;
	dss->disc = disc;
	if (!meth->cx || (meth->flags & DSS_BASE))
		meth = dssmethinit(NiL, NiL, NiL, disc, meth);
	dss->meth = meth;
	dss->flags = flags;
	dss->test = test;
	dss->state = &state;
	if (!(dss->cx = cxscope(NiL, meth->cx, flags & DSS_CX_FLAGS, test, disc)) || disc->map && !loadtags(disc->map, "map", disc, meth))
		goto bad;
	dss->cx->caller = dss;
	if (meth->openf && (*meth->openf)(dss, dss->disc))
		goto bad;
	for (var = (Cxvariable_t*)dtfirst(dss->cx->variables); var; var = (Cxvariable_t*)dtnext(dss->cx->variables, var))
		if (var->format.map)
			var->format.map->header.flags |= CX_REFERENCED;
	for (i = 0; i < elementsof(dss_type); i++)
		if (!cxtype(NiL, dss_type[i].name, disc) && cxaddtype(NiL, &dss_type[i], disc))
			goto bad;
	for (i = 0; i < elementsof(dss_var); i++)
	{
		if (!(var = vmnewof(vm, 0, Cxvariable_t, 1, 0)))
			goto bad;
		*var = dss_var[i];
		var->header.flags |= CX_INITIALIZED;
		if (cxaddvariable(dss->cx, var, disc))
			goto bad;
	}
	return state.dss = dss;
 bad:
	dssclose(dss);
	return 0;
}

/*
 * close a dss session
 */

int
dssclose(register Dss_t* dss)
{
	int	r;

	if (!dss)
		return -1;
	if (dss->meth->closef)
		r = (*dss->meth->closef)(dss, dss->disc);
	else
		r = 0;
	if (dss == state.dss)
		state.dss = 0;
	if (!dss->vm)
		r = -1;
	else
		vmclose(dss->vm);
	return r;
}

/*
 * initialize method given pointer
 * this is a library private global for dssmeth() and dssstatic()
 */

Dssmeth_t*
dssmethinit(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Dssmeth_t*	ometh;
	Opt_t		opt;

	if (meth->flags & DSS_BASE)
	{
		if (!(ometh = newof(0, Dssmeth_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		*ometh = *meth;
		meth = ometh;
		meth->flags &= ~DSS_BASE;
		meth->cx = 0;
	}
	if (!meth->cx && !(meth->cx = cxopen(0, 0, disc)))
		return 0;
	if (!meth->formats && !(meth->formats = dtopen(&state.cx->namedisc, Dtoset)))
	{
		cxclose(meth->cx);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (name)
	{
		if (meth->methf)
		{
			ometh = meth;
			opt = opt_info;
			state.cx->header = (Cxheader_t*)meth;
			state.meth = meth;
			meth = (*meth->methf)(name, options, schema, disc, meth);
			opt_info = opt;
			if (!meth)
				return 0;
			if (meth != ometh)
				meth->reference++;
		}
		else if (options)
			return 0;
	}
	return state.meth = meth;
}

/*
 * return method given name
 */

Dssmeth_t*
dssmeth(const char* name, Dssdisc_t* disc)
{
	register char*	s;
	const char*	options;
	const char*	schema;
	Dsslib_t*	lib;
	Dssmeth_t*	meth;
	char		buf[1024];
	char		path[1024];

	buf[sfsprintf(buf, sizeof(buf) - 1, "%s", name)] = 0;
	options = schema = 0;
	for (s = buf; *s; s++)
		if (*s == ',' || *s == '\t' || *s == '\r' || *s == '\n')
		{
			if (!options)
			{
				*s++ = 0;
				options = (char*)s;
			}
		}
		else if (*s == ':')
		{
			*s++ = 0;
			schema = name + (s - buf);
			break;
		}
	name = (const char*)buf;
	if (!*name)
		name = id;
	if (!(meth = (Dssmeth_t*)dtmatch(state.cx->methods, name)))
	{
		if (pathfind(name, id, id, path, sizeof(path)))
		{
			meth = &dss_method;
			name = id;
			schema = path;
		}
		else if (!(lib = dsslib(name, 0, disc)) || !(meth = lib->meth) || dssadd(lib, disc))
			return 0;
	}
	return dssmethinit(name, options, schema, disc, meth);
}

/*
 * return initialized global state pointer
 */

Dssstate_t*
dssstate(Dssdisc_t* disc)
{
	if (!state.initialized && !state.initialized++)
	{
		error(-1, "%s", fmtident(usage));
		state.cx = cxstate(disc);
		dtinsert(state.cx->libraries, &dss_library);
		if (dssadd(&dss_library, disc))
			error(ERROR_PANIC, "%s library initialization error", id);
	}
	return &state;
}

/*
 * return 1 if expr contains a query
 */

static int
hasquery(register Dssexpr_t* expr)
{
	do
	{
		if (!expr->query->prog)
			return 1;
		if (expr->pass && hasquery(expr->pass))
			return 1;
		if (expr->fail && hasquery(expr->fail))
			return 1;
		if (expr->group && hasquery(expr->group))
			return 1;
	} while (expr = expr->next);
	return 0;
}

/*
 * apply expression with optional head and tail queries to files in argv
 */

int
dssrun(Dss_t* dss, const char* expression, const char* head, const char* tail, char** argv)
{
	register Dssexpr_t*	x;
	Dssexpr_t*		expr;
	Dssexpr_t*		xh;
	Dssexpr_t*		xt;
	int			errors;
	int			r;

	errors = error_info.errors;
	if (!expression || !*expression || *expression == '-' && !*(expression + 1))
		expression = tail ? tail : "{write}";
	if (!(expr = dsscomp(dss, expression, NiL)))
		return -1;
	xh = xt = 0;
	r = -1;
	if (expression == tail)
		tail = 0;
	else if (!tail && !hasquery(expr))
		tail = "{write}";
	if (tail)
	{
		if (!(xt = dsscomp(dss, tail, NiL)))
			goto bad;
		if (xt->query->beg == null_beg)
		{
			dssfree(dss, xt);
			xt = 0;
		}
	}
	for (x = expr; x->group; x = x->group);
	if (!x->query->head)
	{
		if (!head)
			head = "{scan}";
		if (!(xh = dsscomp(dss, head, NiL)))
			goto bad;
		if (!xh->query->head)
		{
			if (dss->disc->errorf)
				(*dss->disc->errorf)(dss, dss->disc, 2, "%s: not a head query", head);
			goto bad;
		}
		xh->files = argv;
	}
	else if (head)
	{
		if (dss->disc->errorf)
			(*dss->disc->errorf)(dss, dss->disc, 2, "%s: expression already has %s head", head, x->query->name);
		goto bad;
	}
	else
		x->files = argv;
	if (xh || xt)
	{
		if (expr->pass || expr->fail || expr->next)
		{
			if (!(x = vmnewof(expr->vm, 0, Cxexpr_t, 1, sizeof(Cxquery_t))))
			{
				if (dss->disc->errorf)
					(*dss->disc->errorf)(dss, dss->disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			x->vm = expr->vm;
			x->done = expr->done;
			x->stack = expr->stack;
			x->op = expr->op;
			x->query = (Cxquery_t*)(x + 1);
			x->group = expr;
			expr = x;
		}
		if (xt)
		{
			expr->pass = xt;
			xt->parent = expr;
		}
		if (xh)
		{
			x = xh->pass = expr;
			expr = xh;
			xh = x;
		}
	}
	if (expr->pass)
		expr->pass->parent = expr->pass;
	if (dss->test & 0x00000100)
		dsslist(dss, expr, sfstdout);
	if (dssbeg(dss, expr) || dssend(dss, expr))
		goto bad;
	dssfree(dss, expr);
	r = error_info.errors != errors ? -1 : 0;
 bad:
	if (xh)
		dssfree(dss, xh);
	if (xt)
		dssfree(dss, xt);
	dssfree(dss, expr);
	return r;
}
