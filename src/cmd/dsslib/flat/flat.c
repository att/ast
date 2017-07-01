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
 * flat method
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[+DESCRIPTION?The \bdss\b flat method schema is the name of an XML file "
    "that describes fixed-width and/or field-delimited flat files. Public "
    "schemas are usually placed in a \b../lib/dss\b sibling directory on "
    "\b$PATH\b.]"
"[b!:binary?Enable the fixed binary record field optimization.]"
"[e:emptyspace?Write empty field values as one \bspace\b character.]"
"[h:header?Print a C header for schema member access on the standard"
"	output and exit. The \bFLAT\b(\adata\a) macro returns a pointer"
"	to the flat record \bstruct\b given the \bDssrecord_t\b pointer"
"	\adata\a. This macro cannot be evaluated in a declaration"
"	assignment. The member names are prefixed by \ashema\a_ to"
"	avoid C identifier conflicts.]"
"[o:offsets?Print the name, offset, size, number of elements and type"
"	of each member, one per line, on the standard output, and exit."
"	Scalar fields have 0 elements.]"
"[p:prototype?Print a prototype flat record with the field data replaced by"
"	the field name on the standard output and exit.]"
"[s:struct?Print the fixed width record schema C \bstruct\b on the standard"
"	output and exit.]"
"[T:test?Enable implementation-specific tests and tracing.]#[mask]"
"[+TAGS?The supported tags are:]{"
;

#include <dsslib.h>
#include <ccode.h>
#include <magicid.h>
#include <regex.h>

struct Flatten_s; typedef struct Flatten_s Flatten_t;
struct Field_s; typedef struct Field_s Field_t;
struct Flat_s; typedef struct Flat_s Flat_t;
struct Format_s; typedef struct Format_s Format_t;
struct Key_s; typedef struct Key_s Key_t;
struct Library_s; typedef struct Library_s Library_t;
struct Magic_s; typedef struct Magic_s Magic_t;
struct Member_s; typedef struct Member_s Member_t;
struct Physical_s; typedef struct Physical_s Physical_t;
struct Record_s; typedef struct Record_s Record_t;
struct Section_s; typedef struct Section_s Section_t;
struct Size_s; typedef struct Size_s Size_t;
struct Table_s; typedef struct Table_s Table_t;
struct Value_s; typedef struct Value_s Value_t;

typedef Cxvalue_t* (*Flattenget_f)(Flatten_t*, Cxvariable_t*, void*);
typedef Cxoperand_t* (*Flatget_f)(Record_t*, int);

struct Value_s
{
	const char*	name;
	long		value;
};

struct Physical_s			/* physical field info		*/
{
	Cxtype_t*	type;		/* type				*/
	Cxformat_t	format;		/* format details		*/
	Cxarray_t*	array;		/* physical array info		*/
};

struct Flatten_s
{
	Dss_t*		dss;		/* output method stream		*/
	Dssfile_t*	file;		/* output file			*/
	Cx_t*		cx;		/* dss->cx			*/
	Flat_t*		flat;		/* dss->meth->data		*/
	Vmalloc_t*	vm;		/* vm region			*/
	Cxcallout_f	getf;		/* dss->cx->getf		*/
	Cxoperand_t	value;		/* flattenget() value		*/
	int		emptyspace;	/* empty field value => space	*/
};

struct Table_s				/* key table info		*/
{
	Dtdisc_t	disc;		/* dict discipline		*/
	Sfio_t*		oob;		/* out of band stream		*/
	Dt_t*		dict;		/* dictionary			*/
	int		span;		/* key value span char		*/
	unsigned char	identified;	/* id table initialized		*/
	unsigned char	qualified;	/* qualification check done	*/
	unsigned char	unknown;	/* span unknown keys		*/
	char		id[UCHAR_MAX+1];/* identifier test map		*/
};

struct Field_s				/* field info			*/
{
	Cxvariable_t	variable;	/* logical field variable	*/
	Physical_t	physical;	/* physical field info		*/
	Field_t* 	next;		/* next in list			*/
	Cxexpr_t*	width;		/* variable size		*/
	Cxvariable_t*	flatten;	/* flatten source variable	*/
	Flattenget_f	flattengetf;	/* flatten flatget()		*/
	Flatget_f	flatgetf;	/* flat flatget()		*/
	Cxstructure_t	structure;	/* structure info		*/
	Record_t*	record;		/* sub-record context		*/
	Table_t*	table;		/* parse time table holder	*/
	size_t		truncate;	/* fixed width before truncation*/
	unsigned char*	map;		/* physical.format.code map	*/
	unsigned char*	pam;		/* conversion map		*/
	unsigned char	keyed;		/* name=value key		*/
	unsigned char	structref;	/* struct w/ member values only	*/
};

struct Key_s				/* field key			*/
{
	Dtlink_t	link;		/* dictionary link		*/
	Key_t*		next;		/* ambiguous key link		*/
	Field_t*	field;		/* field for this key		*/
	char*		qualification;	/* key qualification		*/
	Cxexpr_t*	expr;		/* key qualification expression	*/
	char		name[1];	/* key name			*/
};

struct Member_s				/* record member		*/
{
	Cxoperand_t	ret;		/* value (first for dynamic Q)	*/
	Field_t*	field;		/* static field info		*/
	Cxtype_t*	type;		/* dynamic record data type	*/
	size_t		off;		/* record data offset		*/
	size_t		siz;		/* record data size		*/
	unsigned int	serial;		/* read serial number		*/
	unsigned int	keyed;		/* keyed serial number		*/
};

struct Record_s				/* current record info		*/
{
	Member_t*	fields;		/* fields (first for dynamic Q)	*/
	Flatget_f	getf;		/* getf (second for dynamic Q)	*/
	Dss_t*		dss;		/* dss handle			*/
	Flat_t*		flat;		/* flat handle			*/
	Dssrecord_t*	record;		/* dss record handle		*/
	Cx_t*		cx;		/* cx handle			*/
	Sfio_t*		io;		/* io stream			*/
	Table_t*	table;		/* key table			*/
	char*		image;		/* original record data buffer	*/
	char*		buf;		/* record data buffer		*/
	char*		cur;		/* current position in buf	*/
	size_t		siz;		/* record data buffer size	*/
	size_t		offset;		/* subrecord field offset	*/
	size_t		nfields;	/* #fields per record		*/
	size_t		kfields;	/* first keyed field index	*/
	size_t		serial;		/* record serial		*/
	size_t		copy;		/* record copy serial		*/
	int		index;		/* next field index		*/
};

struct Section_s			/* section info			*/
{
	Section_t* 	next;		/* next in list			*/
	regex_t*	re;		/* section line pattern		*/
	int		delimiter;	/* section record delimiter	*/
	size_t		count;		/* pattern/delimiter count	*/
	size_t		size;		/* section size			*/
};

struct Magic_s				/* magic info			*/
{
	const char*	string;		/* magic string			*/
	unsigned long	number;		/* magic number			*/
	size_t		size;		/* magic header size		*/
	size_t		length;		/* magic number/string length	*/
	unsigned long	version;	/* version stamp		*/
	int		swap;		/* swap op			*/
};

struct Library_s			/* library list			*/
{
	Library_t*	next;		/* next in list			*/
	char		name[1];	/* library name			*/
};

struct Size_s				/* block/record size field info	*/
{
	size_t		fixed;		/* fixed size			*/
	int		type;		/* field type			*/
	int		width;		/* field width			*/
	int		offset;		/* field offset			*/
	int		reserve;	/* min reserve size		*/
	int		size;		/* total field size		*/
	int		base;		/* numeric text base		*/
	unsigned char	add;		/* add size to computed size	*/
	char*		buf;		/* conversion buffer		*/
};

struct Flat_s				/* Dssmeth_t.data		*/
{
	Dsstagdisc_t	dsstagdisc;
	Dssmeth_t	meth;
	Dssmeth_t*	basemeth;
	int		nfields;
	Magic_t*	magic;
	Section_t*	section;
	Section_t*	header;
	Section_t*	lastheader;
	Section_t*	trailer;
	Section_t*	lasttrailer;
	Cxflags_t	flags;
	Cxflags_t	test;
	int		binary;
	int		code;
	int		continuator;
	int		delimiter;
	int		emptyspace;
	int		escape;
	int		force;
	int		list;
	int		prototype;
	int		quotebegin;
	int		quoteend;
	int		sufficient;
	int		swap;
	int		terminator;
	int		variable;
	size_t		fixed;
	Sfio_t*		buf;
	Field_t*	fields;
	Field_t*	lastfield;
	Field_t		root;
	Cxarray_t*	array;
	Cxformat_t*	format;
	Library_t*	libraries;
	Library_t*	lastlibrary;
	char*		valbuf;
	size_t		valsiz;
	Record_t*	current;
	Flatget_f	getf;
	unsigned char*	e2a;
	Size_t*		record;
	Size_t*		block;
	struct
	{
	size_t*		field;
	size_t		index;
	int		fixed;
	}		truncate;
};

#define FW(f,w)	(((w)<<4)|((f)&(CX_STRING|CX_INTEGER|CX_UNSIGNED|CX_FLOAT)))

#define SWAP(n)		(((n)^int_swap)&07)

#define SWAP_none	(-1)
#define SWAP_native	(-2)
#define SWAP_be		0
#define SWAP_le		7

static Cxvalue_t	nullval;
static Cxoperand_t	nullret;

/*
 * lazy flat field retrieval
 */

static Cxoperand_t*
flatget(register Record_t* r, int index)
{
	Flat_t*			flat = r->flat;
	register Member_t*	w;
	register Member_t*	p;
	register Member_t*	y;
	register Field_t*	f;
	register unsigned char*	s;
	register unsigned char*	t;
	register unsigned char*	u;
	register unsigned char*	e;
	register unsigned char*	h;
	unsigned char*		g;
	unsigned char*		m;
	Record_t*		b;
	Cxoperand_t*		v;
	Key_t*			k;
	Cxoperand_t		a;
	int			c;
	int			d;
	int			q;
	int			x;
	size_t			n;

	if (index >= r->nfields)
		return &nullret;
	w = &r->fields[index];
	if (w->serial != r->serial)
	{
		w->serial = r->serial;
		if (w->field->structure.level > 1 && !(((Field_t*)w->field->structure.parent))->structref)
		{
			b = ((Field_t*)w->field->structure.parent)->record;
			if (b->serial != r->serial)
			{
				b->serial = r->serial;
				b->index = b->offset;
				if (!(v = flatget(r, w->field->structure.parent->index)))
					goto empty;
				if (b->buf = b->cur = v->value.string.data)
				{
					b->siz = v->value.string.size;
					r = b;
				}
			}
			else if (b->buf)
				r = b;
		}
		if (r->index <= index && r->index < r->kfields)
		{
			s = (unsigned char*)r->cur;
			e = (unsigned char*)r->buf + r->siz;
			if (index < r->kfields)
			{
				y = w;
				if (w->field->structref)
					y--;
			}
			else
			{
				y = &r->fields[r->kfields - 1];
				sfstrseek(r->table->oob, 0, SEEK_SET);
			}
			for (p = &r->fields[r->index]; p <= y; p++)
			{
				f = p->field;
				d = f->physical.format.delimiter;
				if (d >= 0)
				{
					if (f->width)
					{
						if (cxeval(r->cx, f->width, r->record, &a) < 0)
							goto empty;
						n = a.value.number;
					}
					else if (f->physical.format.escape < 0 && f->physical.format.quotebegin < 0)
					{
						for (t = s; t < e && *t != d; t++);
						n = t - s;
						t++;
					}
					else
					{
						q = f->physical.format.quotebegin;
						x = f->physical.format.escape;
						if ((s + 1) >= e || *s != q)
						{
							n = 1;
							if (f->physical.format.flags & CX_QUOTEALL)
								q = -1;
						}
						else
						{
							s++;
							n = 0;
							q = f->physical.format.quoteend;
						}
						for (u = t = s; t < e; t++)
						{
							if ((c = *t) == x)
							{
								if (++t >= e)
								{
									if (r->dss->disc->errorf)
										(*r->dss->disc->errorf)(r->dss, r->dss->disc, 2, "%sinvalid escape", cxlocation(r->cx, r->record), p->field->variable.name);
									r->index = r->nfields + 1;
									goto empty;
								}
								c = *t;
							}
							else if (n)
							{
								if (c == d || c == flat->terminator)
									break;
								if (c == q)
								{
									n = 0;
									q = f->physical.format.quoteend;
									continue;
								}
							}
							else if (c == q)
							{
								if (x >= 0 || (t + 1) >= e || *(t + 1) != c)
								{
									n = 1;
									q = f->physical.format.quotebegin;
									continue;
								}
								t++;
							}
							if (u < t)
							{
								/*
								 * modify a copy of the input record image
								 */

								if (r->copy != r->serial && (h = vmnewof(r->cx->rm, 0, unsigned char, r->siz, 0)))
								{
									r->copy = r->serial;
									memcpy(h, r->buf, r->siz);
									r->cur = (char*)h + (r->cur - r->buf);
									s = h + (s - (unsigned char*)r->buf);
									t = h + (t - (unsigned char*)r->buf);
									u = h + (u - (unsigned char*)r->buf);
									e = h + r->siz;
									r->buf = (char*)h;
								}
								*u = c;
							}
							u++;
						}
						if (!n)
						{
							if (r->dss->disc->errorf)
								(*r->dss->disc->errorf)(r->dss, r->dss->disc, 2, "%s%s: unterminated quote", cxlocation(r->cx, r->record), p->field->variable.name);
							r->index = r->nfields + 1;
							goto empty;
						}
						n = u - s;
						t++;
					}
					if (d == '\n' && n > 0 && *(s + n - 1) == '\r')
						n--;
					if (f->physical.format.flags & CX_MULTIPLE)
						while (t < e && *t == d)
							t++;
				}
				else
				{
					if (f->width)
					{
						if (cxeval(r->cx, f->width, r->record, &a) < 0)
							goto empty;
						n = a.value.number;
					}
					else if (f->physical.format.flags & CX_VARIABLE)
						n = e - s;
					else
						n = f->physical.format.width;
					t = s + n;
				}
				p->off = (char*)s - r->buf;
				p->siz = n;
				s = t;
			}
			r->index = index + 1;
			if (w->field->structref)
				goto empty;
			r->cur = (char*)s;
		}
		else if (w->field->structref)
			goto empty;
		if (w->field->keyed)
		{
			if (w->keyed != r->serial)
			{
				d = w->field->physical.format.delimiter; /*HERE verify*/
				s = (unsigned char*)r->cur;
				e = (unsigned char*)r->buf + r->siz - 1;
				while (s < e)
				{
					if ((u = s) < e && (r->table->id[*(unsigned char*)s] & 1))
						for (s++; s < e && r->table->id[*(unsigned char*)s]; s++);
					t = s;
					for (;;)
					{
						for (; s < e && *s != d; s++);
						if (r->table->span < 0)
							break;
						for (h = s; h < e && *h == d; h++);
						if (h >= e)
							break;
						if (r->table->id[*(unsigned char*)h] & 1)
						{
							for (g = h++; h < e && r->table->id[*(unsigned char*)h]; h++);
							if (h < e && *h == r->table->span && (!r->table->unknown || dtmatch(r->table->dict, g)))
								break;
						}
						s = h;
					}
					if (t > u && s > t)
					{
						if (k = (Key_t*)dtmatch(r->table->dict, u))
						{
							while (t < s && isspace(*++t));
							for (u = s++; u > t && isspace(*(u - 1)); u--);
							do
							{
								if (!k->expr || cxeval(r->cx, k->expr, r->record, &a) > 0)
								{
									p = &r->fields[k->field->variable.index];
									p->off = t - (unsigned char*)r->buf;
									p->siz = u - t;
									p->keyed = r->serial;
									break;
								}
							} while (k = k->next);
							if (p == w)
								break;
							continue;
						}
						else if (s > (t + 1) && (r->dss->flags & DSS_VERBOSE) && r->dss->disc->errorf)
							(*r->dss->disc->errorf)(r->dss, r->dss->disc, 2, "%s%-.*s: unknown key", cxlocation(r->cx, r->record), t - u, u);
					}
					if (sfstrtell(r->table->oob))
						sfputc(r->table->oob, d);
					for (t = s++; t > u && isspace(*(t - 1)); t--);
					sfwrite(r->table->oob, u, t - u);
				}
				r->cur = (char*)s;
				if (w->keyed != r->serial)
				{
					if (w->field->keyed > 1)
					{
						w->off = 0;
						w->siz = sfstrtell(r->table->oob);
						s = (unsigned char*)sfstrbase(r->table->oob);
						goto found;
					}
					goto empty;
				}
			}
		}
		s = (unsigned char*)r->buf + w->off;
	found:
		if (w->field->width)
		{
			if (cxeval(r->cx, w->field->width, r->record, &a) < 0)
				goto empty;
			n = a.value.number;
			if (w->field->physical.format.width && n > w->field->physical.format.width)
			{
				if (r->dss->disc->errorf)
					(*r->dss->disc->errorf)(r->dss, r->dss->disc, 1, "%s%s: variable field width %d exceeds fixed maximum %d -- maximum assumed", cxlocation(r->cx, r->record), w->field->variable.name, n, w->field->physical.format.width);
				w->siz = w->field->physical.format.width;
			}
			else
			{
				w->siz = n;
				if (!flat->fixed)
					r->cur = (char*)s + w->siz;
				else if (flat->sufficient && w->field->physical.format.width)
					r->cur = (char*)s + w->field->physical.format.width;
				else
				{
					r->cur = (char*)s + w->siz;
					for (y = w; y < &r->fields[r->nfields - 1]; y++)
					{
						n = y->off + y->siz;
						(++y)->off = n;
						if (y->field->width)
							break;
					}
				}
			}
		}
		else if (w->field->physical.format.flags & CX_VARIABLE)
			w->siz = r->siz - w->off;
		if (m = w->field->map)
			s = (unsigned char*)ccmapcpy(m, fmtbuf(w->siz), (char*)s, w->siz);
		w->ret.type = w->field->variable.type;
		if ((*w->field->physical.type->internalf)(r->cx, w->field->physical.type, NiL, &w->field->physical.format, &w->ret, (char*)s, w->siz, r->cx->rm, r->cx->disc) < 0)
		{
	empty:
			w->ret.type = w->field->variable.type;
			if (cxisstring(w->ret.type))
			{
				w->ret.value.string.data = "";
				w->ret.value.string.size = 0;
			}
			else
				w->ret.value.number = 0;
		}
		if (w->ret.type->generic)
			for (x = 0; w->ret.type->generic[x]; x++)
				if (w->ret.type->generic[x]->base == w->field->physical.type || w->ret.type->generic[x]->base == w->field->physical.type->base)
				{
					w->ret.type = w->ret.type->generic[x];
					break;
				}
	}
	return &w->ret;
}

/*
 * lazy flat field retrieval for aligned binary data
 */

static Cxoperand_t*
flatgetbinary(register Record_t* r, int index)
{
	register Member_t*	w = &r->fields[index];
	register char*		s;

	w->ret.type = w->field->variable.type;
	if (w->serial != r->serial)
	{
		w->serial = r->serial;
		switch (FW(w->field->physical.format.flags,w->field->physical.format.width))
		{
		case FW(CX_UNSIGNED|CX_INTEGER,1):
			w->ret.value.number = *(uint8_t*)(r->buf + w->off);
			break;
		case FW(CX_UNSIGNED|CX_INTEGER,2):
			w->ret.value.number = *(uint16_t*)(r->buf + w->off);
			break;
		case FW(CX_UNSIGNED|CX_INTEGER,4):
			w->ret.value.number = *(uint32_t*)(r->buf + w->off);
			break;
#if _typ_int64_t
		case FW(CX_UNSIGNED|CX_INTEGER,8):
			w->ret.value.number = (int64_t)(*(uint64_t*)(r->buf + w->off));
			break;
#endif
		case FW(CX_INTEGER,1):
			w->ret.value.number = *(unsigned _ast_int1_t*)(r->buf + w->off);
			break;
		case FW(CX_INTEGER,2):
			w->ret.value.number = *(int16_t*)(r->buf + w->off);
			break;
		case FW(CX_INTEGER,4):
			w->ret.value.number = *(int32_t*)(r->buf + w->off);
			break;
#if _typ_int64_t
		case FW(CX_INTEGER,8):
			w->ret.value.number = *(int64_t*)(r->buf + w->off);
			break;
#endif
		case FW(CX_FLOAT,4):
			w->ret.value.number = *(_ast_flt4_t*)(r->buf + w->off);
			break;
		case FW(CX_FLOAT,8):
			w->ret.value.number = *(_ast_flt8_t*)(r->buf + w->off);
			break;
#ifdef _ast_flt12_t
		case FW(CX_FLOAT,12):
			w->ret.value.number = *(_ast_flt12_t*)(r->buf + w->off);
			break;
#endif
#ifdef _ast_flt16_t
		case FW(CX_FLOAT,16):
			w->ret.value.number = *(_ast_flt16_t*)(r->buf + w->off);
			break;
#endif
		default:
			if (!(w->siz = w->field->physical.format.width))
				w->ret.value.string.size = 0;
			else if ((w->field->physical.format.flags & (CX_STRING|CX_NUL)) == CX_STRING)
				w->ret.value.string.size = (s = memchr(w->ret.value.string.data = r->buf + w->off, 0, w->siz)) ? (s - (r->buf + w->off)) : w->siz;
			else 
				memcpy(&w->ret.value.number, r->buf + w->off, w->siz);
			break;
		}
	}
	return &w->ret;
}

/*
 * flat identf
 */

static int
flatident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)file->dss->meth->data;
	register Magicid_t*	magicid;
	register unsigned char*	s;
	register unsigned char*	e;
	int			i;
	int			swap;
	unsigned long		num;
	Magicid_data_t		magic;
	unsigned char		tmp[4];

	if (!flat->magic)
		return 1;
	if (flat->magic->version)
	{
		if (n < sizeof(Magicid_t))
			return 0;
		magicid = (Magicid_t*)buf;
		magic = MAGICID;
		if ((swap = swapop(&magic, &magicid->magic, sizeof(magic))) < 0)
			return 0;
		if (swap)
		{
			swapmem(swap, &magicid->size, &magicid->size, sizeof(magicid->size));
			swapmem(swap, &magicid->version, &magicid->version, sizeof(magicid->version));
		}
		if (n < magicid->size)
			return 0;
		if (!streq(file->dss->id, magicid->name))
			return 0;
		if (flat->magic->string && !streq(flat->magic->string, magicid->type))
			return 0;
		s = (unsigned char*)buf + sizeof(Magicid_t);
		e = (unsigned char*)buf + magicid->size;
		while (s < e)
			if (*s++)
				return 0;
		if (flat->fixed && magicid->size != flat->fixed)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: data size %d does not match schema size %d", file->path, magicid->size, flat->fixed);
			return -1;
		}
		if (magicid->version > flat->magic->version)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: data version %s is newer than schema version %s", file->path, fmtversion(magicid->size), fmtversion(flat->fixed));
			return -1;
		}
		if (flat->magic->swap == SWAP_native)
			file->ident = swap;
		else if (flat->magic->swap >= 0)
			file->ident = swap ^ flat->magic->swap;
		file->skip = magicid->size;
		return 1;
	}
	if (n < flat->magic->size)
		return 0;
	if (flat->magic->string)
	{
		if (!memcmp(buf, flat->magic->string, flat->magic->length))
		{
			file->skip = flat->magic->size;
			return 1;
		}
	}
	else
	{
		num = flat->magic->number;
		i = flat->magic->length;
		while (i-- > 0)
		{
			tmp[i] = num & 0xff;
			num >>= 8;
		}
		if ((swap = swapop(tmp, buf, flat->magic->length)) >= 0)
		{
			if (flat->magic->swap == SWAP_native)
				file->ident = swap;
			else if (flat->magic->swap >= 0)
				file->ident = swap ^ flat->magic->swap;
			file->skip = flat->magic->size;
			return 1;
		}
	}
	return 0;
}

/*
 * get record/block size field
 */

static ssize_t
size_get(register Dssfile_t* file, register Size_t* z, Dssdisc_t* disc)
{
	register char*		b;
	register char*		s;
	register char*		e;
	register ssize_t	n;

	error(-1, "AHA:flat#%d size_get size=%d width=%d reserve=%d", __LINE__, z->size, z->width, z->reserve);
	if (!z->width)
		return z->reserve;
	if (!(b = (char*)sfreserve(file->io, z->reserve, z->add != 0)))
		return sfvalue(file->io);
	s = b + z->offset;
	switch (z->type)
	{
	case 'a':
		n = strntoul(s, z->width, NiL, z->base);
		break;
	case 'b':
		n = swapget(0, s, z->width);
		break;
	case 'd':
		n = 0;
		for (e = s + z->width; s < e; s++)
			n = n * 100 + 10 * (*(unsigned char*)s >> 4) + (*s & 0xf);
		if (z->base & 1)
			n /= 10;
		break;
	case 'e':
		s = (char*)ccmapcpy(((Flat_t*)disc)->e2a, z->buf, s, z->width);
		n = strntoul(s, z->width, NiL, z->base);
		break;
	case 'l':
		n = swapget(7, s, z->width);
		break;
	case 'm':
		n = swapget(((Flat_t*)disc)->swap, s, z->width);
		break;
	}
	if (z->add)
		sfread(file->io, b, 0);
	return n;
}

/*
 * put record/block size field
 */

static ssize_t
size_put(register Dssfile_t* file, ssize_t n, register Size_t* z, Dssdisc_t* disc)
{
	register char*		s;

	if (z->add)
		n += z->size;
	s = z->buf;
	memset(s, 0, z->reserve);
	s += z->offset;
	switch (z->type)
	{
	case 'a':
		sfsprintf(s, z->width, "%u", n);
		break;
	case 'b':
		swapput(0, s, z->width, n);
		break;
	case 'd':
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "d type not supported");
		return -1;
	case 'e':
		sfsprintf(s, z->width, "%u", n);
		ccmapcpy(((Flat_t*)disc)->e2a, s, s, z->width);
		break;
	case 'l':
		swapput(7, s, z->width, n);
		break;
	case 'm':
		swapput(((Flat_t*)disc)->swap, s, z->width, n);
		break;
	}
	return sfwrite(file->io, z->buf, z->size) == z->size ? 0 : -1;
}

/*
 * flat readf
 */

static int
flatread(register Dssfile_t* file, register Dssrecord_t* record, Dssdisc_t* disc)
{
	register Record_t*	r = (Record_t*)file->data;
	register Flat_t*	flat = r->flat;
	register char*		s;
	register ssize_t	i;
	size_t			j;
	size_t			k;
	size_t			m;
	Field_t*		f;
	Field_t*		p;

	for (;;)
	{
		if (!++r->serial)
		{
			r->serial = 1;
			r->copy = 0;
			for (i = 0; i < r->nfields; i++)
				r->fields[i].serial = r->fields[i].keyed = 0;
			if (flat->fixed && flat->terminator >= 0 && !flat->truncate.field)
			{
				/*
				 * if the first record doesn't have a terminator
				 * then we ignore the terminator for all records;
				 * only string fields are checked
				 */

				if (s = (char*)sfreserve(file->io, flat->fixed, SF_LOCKR))
				{
					if ((j = sfvalue(file->io)) > flat->fixed)
						j = flat->fixed + 1;
					f = flat->fields;
					m = f->physical.format.width;
					for (k = 0;; k++)
					{
						if (k >= j)
						{
							flat->terminator = -1;
							break;
						}
						if (k >= m)
						{
							if (f = f->next)
								m += f->physical.format.width;
							else
							{
								if (s[k] != flat->terminator)
									flat->terminator = -1;
								break;
							}
						}
						if (((unsigned char*)s)[k] == flat->terminator && (f->physical.format.flags & CX_STRING))
							break;
					}
					sfread(file->io, s, 0);
				}
				else
					flat->terminator = -1;
				if (flat->terminator >= 0)
				{
					flat->variable = 0;
					flat->truncate.index = flat->nfields;
					if (!(flat->truncate.field = newof(0, size_t, flat->fixed, 0)))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
						return -1;
					}
					k = 0;
					for (f = flat->fields; f; f = f->next)
					{
						f->truncate = f->physical.format.width;
						m = k + f->physical.format.width;
						while (k < m)
							flat->truncate.field[k++] = f->variable.index;
					}
				}
				else
				{
					flat->sufficient = 1;
					if (disc->errorf && !(file->dss->flags & DSS_QUIET))
						(*disc->errorf)(NiL, disc, 1, "%sflat record terminator ignored", cxlocation(r->cx, record));
				}
			}
		}
		error(-1, "AHA:flat#%d terminator=%d continuator=%d delimiter=%d fixed=%d record=%d", __LINE__, flat->terminator, flat->continuator, flat->delimiter, flat->fixed, flat->record ? flat->record->fixed : 0);
		if (flat->terminator >= 0)
		{
			if (flat->continuator >= 0)
			{
				if (!(s = sfgetr(file->io, flat->terminator, 0)))
					break;
				i = sfvalue(file->io);
				if (i > 1 && *(s + i - 2) == flat->continuator)
				{
					for (;;)
					{
						s[i - 2] = flat->delimiter;
						sfwrite(flat->buf, s, i - 1);
						if (!(s = sfgetr(file->io, flat->terminator, 0)))
							goto eof;
						i = sfvalue(file->io);
						if (i < 2 || *(s + i - 2) != flat->continuator)
						{
							sfwrite(flat->buf, s, i);
							break;
						}
					}
					i = sfstrtell(flat->buf);
					s = sfstrseek(flat->buf, 0, SEEK_SET);
				}
			}
			else
			{
				if (!(s = sfgetr(file->io, flat->terminator, 0)))
					break;
				i = sfvalue(file->io);
			}
			if (flat->fixed)
			{
				if (flat->sufficient = flat->fixed == (i - 1))
					k = r->nfields;
				else if (!flat->truncate.field)
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%sinvalid record length %d -- record ignored", cxlocation(r->cx, record), i);
						continue;
				}
				else if (i <= flat->fixed)
				{
					f = r->fields[flat->truncate.field[i-1]].field;
					f->physical.format.width = i - r->fields[f->variable.index].off - 1;
					p = f;
					while (p = p->next)
						p->physical.format.width = 0;
					k = f->variable.index;
				}
				else
					k = r->nfields;
				for (j = flat->truncate.index; j < k; j++)
					r->fields[j].field->physical.format.width = r->fields[j].field->truncate;
				flat->truncate.index = k;
			}
			r->index = 0;
		}
		else if (i = flat->fixed)
		{
			if (flat->record && (i = size_get(file, flat->record, disc)) <= 0)
				break;
			error(-1, "AHA:flat#%d size=%d", __LINE__, i);
			if (!(s = (char*)sfreserve(file->io, i, flat->variable)) && (!flat->variable || !(i = sfvalue(file->io)) || !(s = (char*)sfreserve(file->io, i, flat->variable))))
				break;
		}
		else
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%snon-unique terminator record read not implemented", cxlocation(r->cx, record));
			return -1;
		}
		r->image = r->buf = r->cur = s;
		r->siz = i;
		r->record = record;
		record->data = flat->current = r;
		if (flat->force)
		{
			for (j = 0; j < r->nfields; j++)
				if (!r->fields[j].field->structref)
					(*flat->getf)(r, j);
			if (flat->variable)
			{
				i = r->fields[r->nfields - 1].off + r->fields[r->nfields - 1].siz;
				sfread(file->io, s, i);
			}
		}
		record->size = r->siz = i;
		return 1;
	}
 eof:
	if (sfvalue(file->io) && disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%slast record truncated (%ld/%ld)", cxlocation(r->cx, record), (long)sfvalue(file->io), (long)flat->fixed);
	return 0;
}

/*
 * flat writef
 */

static int
flatwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Record_t*	r = (Record_t*)record->data;
	register Flat_t*	flat = r->flat;
	register Sfio_t*	io;
	register Field_t*	f;
	register int		i;
	register unsigned char*	s;
	Cxoperand_t*		v;
	ssize_t			n;
	unsigned char*		b;
	unsigned char*		e;
	int			q;

	if (r == flat->current)
		return sfwrite(file->io, r->image, r->siz) == r->siz ? 0 : -1;
	io = flat->record ? flat->buf : file->io;
	for (i = 0; i < r->nfields; i++)
	{
		f = r->fields[i].field;
		v = (*f->flatgetf)(r, i);
		while ((n = (*f->physical.type->externalf)(r->cx, f->physical.type, NiL, &f->physical.format, &v->value, flat->valbuf, flat->valsiz, r->cx->disc)) > flat->valsiz)
		{
			n = roundof(n, 32);
			if (!(flat->valbuf = newof(flat->valbuf, char, n, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			flat->valsiz = n;
		}
		if (n < 0)
			return -1;
		else if (n > 0)
		{
			if ((f->physical.format.flags & (CX_STRING|CX_BUFFER)) && f->physical.format.delimiter >= 0 && (f->physical.format.escape >= 0 || f->physical.format.quotebegin >= 0))
			{
				if (f->physical.format.flags & CX_QUOTEALL)
				{
					q = 1;
					sfputc(io, f->physical.format.quotebegin);
				}
				else
					q = 0;
				for (e = (s = b = (unsigned char*)flat->valbuf) + n; s < e; s++)
					if (*s == f->physical.format.delimiter || *s == f->physical.format.escape || *s == f->physical.format.quotebegin || *s == f->physical.format.quoteend)
					{
						if (f->physical.format.escape >= 0)
						{
							sfwrite(io, b, s - b);
							sfputc(io, f->physical.format.escape);
							sfputc(io, *s);
						}
						else if (*s == f->physical.format.delimiter)
						{
							if (q)
								continue;
							q = 1;
							sfwrite(io, b, s - b);
							sfputc(io, f->physical.format.quotebegin);
							sfputc(io, *s);
						}
						else
						{
							sfwrite(io, b, s - b + 1);
							sfputc(io, *s);
							if (!q)
							{
								q = 1;
								sfputc(io, *s);
							}
						}
						b = s + 1;
					}
				if (q && !(f->physical.format.flags & CX_QUOTEALL))
				{
					q = 0;
					sfputc(io, f->physical.format.quoteend);
				}
				sfwrite(io, b, s - b);
				if (q)
					sfputc(io, f->physical.format.quoteend);
			}
			else
				sfwrite(io, flat->valbuf, n);
		}
		else if (flat->emptyspace && f->physical.format.delimiter >= 0)
			sfputc(io, ' ');
		if (f->physical.format.delimiter >= 0)
			sfputc(io, f->physical.format.delimiter);
	}
	if (flat->record)
	{
		n = sfstrtell(io);
		sfstrseek(io, 0, SEEK_SET);
		if (size_put(file, n, flat->record, disc) || sfwrite(file->io, sfstrbase(io), n) != n)
			return -1;
	}
	return 0;
}

/*
 * skip file sections in s
 */

static int
skip(Dssfile_t* file, const char* section, register Section_t* s, Dssdisc_t* disc)
{
	register size_t		i;
	register char*		t;
	register char*		u;
	int			code;

	for (; s; s = s->next)
	{
		i = s->count;
		do
		{
			if (s->delimiter >= 0)
			{
				if (!(t = sfgetr(file->io, s->delimiter, 0)))
					goto eof;
			}
			else if (s->size > 0)
			{
				if (!(t = sfreserve(file->io, s->size, 0)))
					goto eof;
			}
			else
				break;
			if (s->re && (code = regnexec(s->re, t, sfvalue(file->io), 0, NiL, 0)))
			{
				if (code != REG_NOMATCH)
				{
					if (disc->errorf)
					{
						char	buf[256];

						regerror(code, s->re, buf, sizeof(buf));
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: %s: regular expression: %s", file->path, section, buf);
					}
					return -1;
				}
				for (u = t + sfvalue(file->io); u > t;)
					sfungetc(file->io, *--u);
				break;
			}
		} while (!i || --i);
	}
	return 0;
 eof:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: unexpected EOF in %s", file->path, section);
	return -1;
}

static int
keycmp(Dt_t* dt, void* va, void* vb, Dtdisc_t* disc)
{
	register unsigned char*	a = (unsigned char*)va;
	register unsigned char*	b = (unsigned char*)vb;
	register char*		t = ((Table_t*)disc)->identified ? ((Table_t*)disc)->id : (char*)0;
	register int		c;
	register int		d;

	while (!(d = (c = *a++) - (int)*b++))
		if (!(t ? t[c] : c))
			return 0;
	return (!*(b - 1) && !(t ? t[c] : c)) ? 0 : d;
}

/*
 * allocate and initialize a Table_t
 */

static Table_t*
tabinit(Flat_t* flat, Dssdisc_t* disc)
{
	register Table_t*	t;

	if (!(t = newof(0, Table_t, 1, 0)) || !(t->oob = sfstropen()))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	t->disc.link = offsetof(Key_t, link);
	t->disc.key = offsetof(Key_t, name);
	t->disc.comparf = keycmp;
	if (!(t->dict = dtopen(&t->disc, Dtoset)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	t->span = -1;
	return t;
}

static Cxexpr_t*
keycomp(Flat_t* flat, const char* s, Dssdisc_t* disc)
{
	Cxexpr_t*	expr;
	void*		pop;

	if (!(pop = cxpush(flat->meth.cx, NiL, NiL, s, -1, 0)))
		return 0;
	expr = cxcomp(flat->meth.cx);
	cxpop(flat->meth.cx, pop);
	return expr;
}

/*
 * final dictionary table initialization
 */

static int
tabcomp(Flat_t* flat, register Table_t* tab, Dssdisc_t* disc)
{
	register char*		s;
	register Key_t*		k;
	register Key_t*		q;
	register int		i;

	if (!tab->identified)
	{
		tab->identified = 1;
		for (i = 0; i <= UCHAR_MAX; i++)
		{
			if (isalpha(i) || i == '_')
				tab->id[i] |= 3;
			else if (isdigit(i) || i == '-' || i == '.' || i == ',')
				tab->id[i] |= 2;
		}
	}
	for (k = (Key_t*)dtfirst(tab->dict); k; k = (Key_t*)dtnext(tab->dict, k))
		for (q = k; q; q = q->next)
		{
			if (q->qualification && !(q->expr = keycomp(flat, q->qualification, disc)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: %s: %s: cannot compile key qualification expression", q->field->variable.name, q->name, q->qualification);
				return -1;
			}
			s = q->name;
			tab->id[*s] |= 3;
			while (*++s)
				tab->id[*s] |= 2;
		}
	return 0;
}

/*
 * allocate and initialize a Record_t
 */

static Record_t*
recinit(register Flat_t* flat, Dssfile_t* file, Record_t* b, Table_t* t, Field_t* fields, size_t n, size_t i, Dssdisc_t* disc)
{
	register Field_t*	f;
	register Record_t*	r;
	register Key_t*		k;
	size_t			level;
	size_t			off;

	if (!fields)
		return 0;
	level = fields->structure.level;
	if (!n)
		for (f = fields; f; f = f->next, n++)
			if (f->structure.level < level)
				break;
	if (!(r = vmnewof(file->dss->vm, 0, Record_t, 1, (level == 1 ? n * sizeof(Member_t) : 0))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	r->fields = level == 1 ? (Member_t*)(r + 1) : b->fields;
	r->getf = flat->getf;
	r->offset = i;
	r->serial--;
	r->nfields = n + i;
	r->kfields = n + i + 1;
	if (t && !t->qualified)
	{
		t->qualified = 1;
		for (k = (Key_t*)dtfirst(t->dict); k; k = (Key_t*)dtnext(t->dict, k))
			if (k->next && !k->qualification)
				error(1, "%s: %s: field key is ambiguous -- qualification required", k->field->variable.name, k->name);
	}
	off = 0;
	for (f = fields; f; f = f->next, i++)
	{
		if (f->structure.members && !f->record && !(f->record = recinit(flat, file, r, f->table, f->next, 0, i + 1, disc)))
			return 0;
		if (f->structure.level == level)
		{
			if (f->keyed && r->kfields == (r->nfields + 1))
				r->kfields = i;
		}
		if (level == 1)
		{
			r->fields[i].field = f;
			if (flat->fixed)
			{
				if (flat->truncate.fixed)
				{
					if (off > flat->fixed)
					{
						off = flat->fixed;
						f->physical.format.width = 0;
						f->physical.format.delimiter = -1;
					}
					else if ((off + f->physical.format.width) >= flat->fixed)
					{
						f->physical.format.width = flat->fixed - off;
						f->physical.format.delimiter = -1;
					}
				}
				r->fields[i].off = off;
				off += r->fields[i].siz = f->physical.format.width;
				if (f->physical.format.delimiter >= 0)
					off++;
			}
		}
	}
	if (r->kfields <= r->nfields && (!(r->table = t) && !(r->table = tabinit(flat, disc)) || tabcomp(flat, r->table, disc)))
		return 0;
	if (flat->fixed)
		r->index = r->nfields;
	r->dss = file->dss;
	r->flat = flat;
	r->cx = file->dss->cx;
	r->io = file->io;
	return r;
}

/*
 * flat fopenf
 */

static int
flatfopen(Dssfile_t* file, Dssdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)file->dss->meth->data;
	size_t			i;

	if (file->flags & DSS_FILE_READ)
	{
#if 0
		if (file->ident)
			goto noswap;
#endif
#if 0
		if (flat->variable)
		{
			char*		s;

			i = roundof(flat->fixed, 8 * 1024);
			if (s = vmnewof(file->dss->vm, 0, char, i, 0))
				sfsetbuf(file->io, s, i);
		}
#endif
		if (file->skip && !sfreserve(file->io, file->skip, -1))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: unexpected EOF in magic header", file->path);
			return -1;
		}
		if (flat->header && skip(file, "header", flat->header, disc))
			return -1;
	}
	if (!(file->data = flat->root.record = recinit(flat, file, NiL, flat->root.table, flat->fields, flat->nfields, 0, disc)))
		return -1;
	if ((file->flags & DSS_FILE_WRITE) && flat->magic)
	{
		if (flat->swap > 0)
			goto noswap;
		if (flat->magic->version)
		{
			Magicid_t	magicid;

			memset(&magicid, 0, sizeof(Magicid_t));
			magicid.magic = MAGICID;
			strncopy(magicid.name, file->dss->id, sizeof(magicid.name));
			if (flat->magic->string)
				strncopy(magicid.type, flat->magic->string, sizeof(magicid.type));
			magicid.version = flat->magic->version;
			magicid.size = i = flat->fixed ? flat->fixed : sizeof(magicid);
			if (flat->magic->swap > 0)
			{
				swapmem(flat->magic->swap, &magicid.magic, &magicid.magic, sizeof(magicid.magic));
				swapmem(flat->magic->swap, &magicid.version, &magicid.version, sizeof(magicid.version));
				swapmem(flat->magic->swap, &magicid.size, &magicid.size, sizeof(magicid.size));
			}
			sfwrite(file->io, &magicid, sizeof(magicid));
			i -= sizeof(magicid);
		}
		else
		{
			if (flat->magic->string)
				sfwrite(file->io, flat->magic->string, flat->magic->length);
			else
			{
				union
				{
					uint8_t		u1;
					uint16_t	u2;
					uint32_t	u4;
#if _typ_int64_t
					uint64_t	u8;
#endif
					char		buf[sizeof(intmax_t)];
				}	num;

				switch (flat->magic->length)
				{
				case 1:
					num.u1 = flat->magic->number;
					break;
				case 2:
					num.u2 = flat->magic->number;
					break;
				case 4:
					num.u4 = flat->magic->number;
					break;
#if _typ_int64_t
				case 8:
					num.u8 = flat->magic->number;
					break;
#endif
				}
				if (flat->magic->swap > 0)
					swapmem(flat->magic->swap, num.buf, num.buf, flat->magic->length);
				sfwrite(file->io, num.buf, flat->magic->length);
			}
			i = flat->magic->size - flat->magic->length;
		}
		while (i-- > 0)
			sfputc(file->io, 0);
		if (sferror(file->io))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|22, "%s: magic header write error", file->path);
			return -1;
		}
	}
	return 0;
 noswap:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: binary record data swap not implemented yet", file->path);
	return -1;
}

/*
 * flat fclosef
 */

static int
flatfclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file || !file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

static Dssformat_t flat_format =
{
	"flat",
	"flat format (2010-11-10)",
	CXH,
	flatident,
	flatfopen,
	flatread,
	flatwrite,
	0,
	flatfclose,
	0,
	0,
	0
};

static int
op_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
#if __APPLE__
	Cxoperand_t*	x;

	x = (*(((Record_t*)DSSDATA(data))->getf))((Record_t*)DSSDATA(data), pc->data.variable->index);
	r->type = x->type;
	r->refs = x->refs;
	r->value.number = x->value.number;
#else
	*r = *(*(((Record_t*)DSSDATA(data))->getf))((Record_t*)DSSDATA(data), pc->data.variable->index);
#endif
	return 0;
}

static Cxcallout_t local_callouts[] =
{
CXC(CX_GET, "void", "void", op_get, 0)
};

static Tags_t*	flat_field_beg(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
static int	flat_field_end(Tag_t*, Tagframe_t*, Tagdisc_t*);

static Tags_t*	flat_field_physical_beg(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
static int	flat_field_physical_end(Tag_t*, Tagframe_t*, Tagdisc_t*);

static Tags_t*	flat_array_physical_beg(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
static int	flat_array_physical_end(Tag_t*, Tagframe_t*, Tagdisc_t*);

static int
flat_field_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->lastfield->variable.name = (const char*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_field_description_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->lastfield->variable.description = (const char*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_field_details_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->format->details = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_field_map_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (dss_map_end(tag, fp, disc))
		return -1;
	flat->format->map = (Cxmap_t*)fp->data;
	return 0;
}

static int
flat_field_con_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->format->constraint = (Cxconstraint_t*)fp->data;
	return 0;
}

static int
flat_field_type_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			s;
	Cxtype_t*		t;

	(void)cxattr(NiL, data, &s, flat->format, NiL);
	if (!*s)
		t = (Cxtype_t*)"number";
	else if (!(t = (Cxtype_t*)strdup(s)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (flat->format->flags & CX_FLOAT)
		flat->format->flags &= ~(CX_STRING|CX_BUFFER|CX_UNSIGNED|CX_INTEGER);
	else if (flat->format->flags & CX_UNSIGNED)
	{
		flat->format->flags &= ~(CX_STRING|CX_BUFFER);
		flat->format->flags |= CX_UNSIGNED|CX_INTEGER;
	}
	else if (!(flat->format->flags & (CX_STRING|CX_BUFFER|CX_INTEGER)))
	{
		if (streq(s, "string"))
			flat->format->flags |= CX_STRING;
		else if (streq(s, "buffer"))
			flat->format->flags |= CX_BUFFER;
	}
	if (flat->format == &flat->lastfield->variable.format)
		flat->lastfield->variable.type = t;
	else
		flat->lastfield->physical.type = t;
	return 0;
}

static int
flat_field_delimiter_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->format->delimiter = *data;
	if (flat->format->code != CC_NATIVE && (fp->attr & TAG_ATTR_conv))
		flat->format->delimiter = ccmapc(flat->format->delimiter, CC_NATIVE, flat->format->code);
	return 0;
}

static int
flat_field_multiple_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (strtol(data, NiL, 0) > 0)
		flat->format->flags |= CX_MULTIPLE;
	else
		flat->format->flags &= ~CX_MULTIPLE;
	return 0;
}

static int
flat_field_escape_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->format->escape = *data;
	return 0;
}

static int
flat_field_quotebegin_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->format->quotebegin = *data;
	return 0;
}

static int
flat_field_quoteend_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->format->quoteend = *data;
	return 0;
}

static int
flat_field_quoteall_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (strtol(data, NiL, 0) > 0)
		flat->format->flags |= CX_QUOTEALL;
	else
		flat->format->flags &= ~CX_QUOTEALL;
	return 0;
}

static int
flat_field_fixedpoint_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->format->fixedpoint = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
flat_field_width_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	if (isdigit(*data))
	{
		flat->format->width = strtoul(data, &e, 0);
		if (*e)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
			return -1;
		}
	}
	else if (!(flat->lastfield->width = (Cxexpr_t*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_field_remainder_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->format->width = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	flat->format->flags |= CX_VARIABLE;
	return 0;
}

static int
flat_field_fill_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->format->fill = *data;
	return 0;
}

static int
flat_field_base_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->format->base = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
flat_field_codeset_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if ((flat->format->code = ccmapid(data)) < 0)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: unknown character code set", data);
		return -1;
	}
	return 0;
}

static int
flat_array_delimiter_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->array->delimiter = *data;
	return 0;
}

static int
flat_array_size_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	if (isdigit(*data))
	{
		flat->array->size = strtoul(data, &e, 0);
		if (*e)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
			return -1;
		}
	}
	else if (!(flat->array->variable = (Cxvariable_t*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static Tags_t	tags_array[] =
{
	"SIZE",		"Fixed array size or variable size field name.",
			0,0,flat_array_size_dat,0,
	"PHYSICAL",	"Physical (file representation) details; the"
			" remaining tags may appear inside <PHYSICAL>."
			" Outside of <PHYSICAL> the tags provide logical"
			" (print representation) details. Logical details"
			" are used as physical defaults.",
			0,flat_array_physical_beg,0,flat_array_physical_end,
	"DELIMITER",	"Array value delimiter character.",
			0,0,flat_array_delimiter_dat,0,
	0
};

static Tags_t*
flat_array_physical_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (name)
	{
		if (!flat->lastfield->physical.array && !(flat->lastfield->physical.array = newof(0, Cxarray_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		flat->array = flat->lastfield->physical.array;
	}
	return &tags_array[2];
}

static int
flat_array_physical_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->array = flat->lastfield->variable.array;
	return 0;
}

static Tags_t*
flat_array_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (name)
	{
		if (!flat->lastfield->variable.array && !(flat->lastfield->variable.array = newof(0, Cxarray_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		flat->array = flat->lastfield->variable.array;
	}
	return &tags_array[0];
}

static Key_t*
addkey(const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	register Key_t*		key;
	register Key_t*		old;
	register Dt_t*		dict;

	if (!(key = newof(0, Key_t, 1, strlen(data))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	strcpy(key->name, data);
	key->field = flat->lastfield;
	if (streq(key->name, "*"))
		flat->lastfield->keyed = 2;
	else
	{
		flat->lastfield->keyed = 1;
		dict = ((Field_t*)flat->lastfield->structure.parent)->table->dict;
		if (old = (Key_t*)dtmatch(dict, key->name))
		{
			if (!old->qualification)
				error(1, "%s: %s: field key is ambiguous -- qualification required", old->field->variable.name, key->name);
			key->next = old;
			dtdelete(dict, old);
		}
		dtinsert(dict, key);
	}
	return key;
}

static int
flat_field_key_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Key_t*		key;

	if (fp->prev->data)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: key name already specified", data);
		return -1;
	}
	if (!(key = addkey(data, disc)))
		return -1;
	fp->prev->data = key;
	return 0;
}

static int
flat_field_key_qualification_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Key_t*		key;

	if (!(key = (Key_t*)fp->prev->data))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "key must be named");
		return -1;
	}
	if (!(key->qualification = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
identifiers(register Table_t* tab, register const char* s, int flags, Tagdisc_t* disc)
{
	register int	c;
	register int	d;
	register char*	t;
	char*		e;
	regclass_t	f;
	char		m;

	while (c = *s++)
	{
		if (*s == '[')
		{
			switch (*(s + 1))
			{
			case ':':
				if (f = regclass(s + 1, &e))
				{
					s = (const char*)e;
					for (c = 0; c <= UCHAR_MAX; c++)
						if ((*f)(c))
							tab->id[c] |= flags;
					continue;
				}
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "[%s: unknown character class", s);
				return -1;
			case '.':
			case '=':
				if (regcollate(s, &e, &m, 1, NiL) == 1)
				{
					s = (const char*)e;
					tab->id[m] |= flags;
					continue;
				}
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "[%s: invalid collation element", s);
				return -1;
			}
		}
		tab->id[c] |= flags;
		if (*s == '-' && (d = *(s + 1)))
		{
			s += 2;
			if (c == 'A' && d == 'Z')
				t = "BCDEFGHIJKLMNOPQRSTUVWXYZ";
			else if (c == 'a' && d == 'z')
				t = "bcdefghijklmnopqrstuvwxyz";
			else if (c == '0' && d == '9')
				t = "123456789";
			else
			{
				while (++c <= d)
					tab->id[c] |= flags;
				continue;
			}
			while (c = *t++)
				tab->id[c] |= flags;
		}
	}
	return 0;
}

static Table_t*
table(Tagframe_t* fp, Tagdisc_t* disc)
{
	Field_t*	p;

	if (!(p = fp->data) && !(p = (Field_t*)fp->prev->data))
		p = &((Flat_t*)disc)->root;
	if (!p->table)
		p->table = tabinit((Flat_t*)disc, (Dssdisc_t*)disc);
	return p->table;
}

static int
flat_field_key_id1_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	return identifiers(table(fp, disc), data, 3, disc);
}

static int
flat_field_key_id2_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	return identifiers(table(fp, disc), data, 2, disc);
}

static int
flat_field_key_span_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	table(fp, disc)->span = *data ? *data : -1;
	return 0;
}

static int
flat_field_key_unknown_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	table(fp, disc)->unknown = strtol(data, NiL, 0) == 1;
	return 0;
}

static Tags_t	tags_key[] =
{
	"NAME",		"Key name.",
			0,0,flat_field_key_name_dat,0,
	"QUALIFICATION","Key qualification expression; the key is active"
			" for the current record when the expression evaluates"
			" non-zero. Used to disambiguate conflicting key"
			" names. Why design data with conflicts?",
			0,0,flat_field_key_qualification_dat,0,
	"ID1",		"Characters that may appear anywhere in key names,"
			" interpreted as the contents of an RE character"
			" class. The default is \b[:alpha:]_\b.",
			0,0,flat_field_key_id1_dat,0,
	"ID2",		"Characters that may appear after the first character"
			" in key names, interpreted as the contents of an RE"
			" character class. The default is"
			" \b[:alnum:]_.,-\b.",
			0,0,flat_field_key_id2_dat,0,
	"SPAN",		"The value is the key assignment character. If set"
			" then key values may span the field separator"
			" character up to the end of record or the next"
			" \akey\a\aspan\a\avalue\a. Such data is fraught with"
			" ambiguities. One must suppose that it never occurred"
			" to the writers that the data would someday be read.",
			0,0,flat_field_key_span_dat,0,
	"UNKNOWN",	"Span unknown but otherwise syntactically correct"
			" keys.",
			0,0,flat_field_key_unknown_dat,0,
	0
};

static Tags_t*
flat_field_key_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (name && !((Field_t*)flat->lastfield->structure.parent)->table && !(((Field_t*)flat->lastfield->structure.parent)->table = tabinit(flat, (Dssdisc_t*)disc)))
		return 0;
	return &tags_key[0];
}

static int
flat_field_key_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	return addkey(data, disc) ? 0 : -1;
}

static Tags_t	tags_flat_field[] =
{
	"NAME",		"Field name.",
			0,0,flat_field_name_dat,0,
	"DESCRIPTION",	"Field description.",
			0,0,flat_field_description_dat,0,
	"PHYSICAL",	"Physical (file representation) details; the"
			" remaining tags may appear inside <PHYSICAL>."
			" Outside of <PHYSICAL> the tags provide logical"
			" (print representation) details. Logical details"
			" are used as physical defaults.",
			0,flat_field_physical_beg,0,flat_field_physical_end,
	"MAP",		"Field value map;"
			" either a map reference name or a map definition.",
			0,dss_map_beg,dss_map_dat,flat_field_map_end,
	"TYPE",		"Field type. The intrinsic types are number and"
			" string. Other types are defined in optional"
			" method and schema libraries.",
			0,0,flat_field_type_dat,0,
	"DETAILS",	"An optional type-specific comma-separated string"
			" of name=value pairs.",
			0,0,flat_field_details_dat,0,
	"DELIMITER",	"Field delimiter character.",
			0,0,flat_field_delimiter_dat,0,
	"MULTIPLE",	"Multiple adjacent delimiters are equivalent to one.",
			0,0,flat_field_multiple_dat,0,
	"ESCAPE",	"Field delimiter and/or quote escape character.",
			0,0,flat_field_escape_dat,0,
	"QUOTE",	"Field quote begin and end character.",
			0,0,flat_field_quotebegin_dat,0,
	"QUOTEBEGIN",	"Field quote begin character.",
			0,0,flat_field_quotebegin_dat,0,
	"QUOTEEND",	"Field quote end character.",
			0,0,flat_field_quoteend_dat,0,
	"QUOTEALL",	"Field quotes are the first and last characetrs.",
			0,0,flat_field_quoteall_dat,0,
	"CODESET",	"Field codeset name; one of { native ascii ebcdic }.",
			0,0,flat_field_codeset_dat,0,
	"FIXEDPOINT",	"Fixed point width.",
			0,0,flat_field_fixedpoint_dat,0,
	"WIDTH",	"Field fixed width in bytes.",
			0,0,flat_field_width_dat,0,
	"REMAINDER",	"Field is variable length up to the end of record.",
			0,0,flat_field_remainder_dat,0,
	"BASE",		"Numeric field representation base.",
			0,0,flat_field_base_dat,0,
	"FILL",		"Fixed width field fill character.",
			0,0,flat_field_fill_dat,0,
	"KEY",		"name=value keyed field details. Keyed fields are"
			" optional and may appear in any order after the"
			" positional fields. All unknown keys and invalid"
			" key data are passed to the key named \b*\b; if there"
			" is no \b*\b key or if \b--verbose\b is on then an"
			" error message is emitted for each occurrence. If"
			" \b<SPAN>\b and \b<UNKNOWN>\b are enabled then"
			" unknown keys are spanned and unknown/invalid"
			" key messages are disabled.",
			0,flat_field_key_beg,flat_field_key_dat,0,
	"CONSTRAINT",	"Field value constraints. Constraints, when"
			" enabled, are applied to each record as it is read.",
			0,dss_con_beg,dss_con_dat,flat_field_con_end,
	"ARRAY",	"Array info.",
			0,flat_array_beg,0,0,
	"FIELD",	"Field structure.",
			0,flat_field_beg,0,flat_field_end,
	0
};

static Tags_t*
flat_field_physical_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (name)
		flat->format = &flat->lastfield->physical.format;
	return &tags_flat_field[3];
}

static int
flat_field_physical_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->format = &flat->lastfield->variable.format;
	return 0;
}

static Tags_t*
flat_field_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	Field_t*		f;
	Field_t*		p;
	Field_t*		t;

	if (name)
	{
		if (!(f = newof(0, Field_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		f->variable.structure = &f->structure;
		if (p = fp->data)
		{
			f->structure.parent = p->structure.parent;
			f->structure.level = p->structure.level;
			((Field_t*)fp->tail)->structure.next = &f->variable;
		}
		else
		{
			fp->data = f;
			if (p = (Field_t*)fp->prev->data)
			{
				f->structure.parent = &p->variable;
				f->structure.level = p->structure.level + 1;
				if (t = (Field_t*)fp->prev->head)
					t->structure.next = &f->variable;
				else
					p->structure.members = &f->variable;
				fp->prev->head = f;
			}
			else
			{
				f->structure.level = 1;
				f->structure.parent = &flat->root.variable;
			}
		}
		fp->tail = f;
		if (!flat->lastfield)
			flat->fields = f;
		else
			flat->lastfield->next = f;
		flat->lastfield = f;
		flat->format = &f->variable.format;
		f->variable.format.delimiter = f->physical.format.delimiter = flat->delimiter;
		f->variable.format.flags = f->physical.format.flags = flat->flags;
		f->variable.format.escape = f->physical.format.escape = flat->escape;
		f->variable.format.quotebegin = f->physical.format.quotebegin = flat->quotebegin;
		f->variable.format.quoteend = f->physical.format.quoteend = flat->quoteend;
		f->variable.format.code = f->physical.format.code = CC_NATIVE;
		f->variable.index = flat->nfields++;
		f->variable.type = (Cxtype_t*)"void";
	}
	return &tags_flat_field[0];
}

static int
flat_field_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (flat->lastfield && !flat->lastfield->variable.name)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "field name must be specified");
		return -1;
	}
	return 0;
}

static int
flat_size_type_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	switch (((Size_t*)fp->prev->data)->type = *data)
	{
	case 'a':
	case 'b':
	case 'd':
	case 'e':
	case 'l':
	case 'm':
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: invalid size field type", data);
		return -1;
	}
	return 0;
}

static int
flat_size_width_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Size_t*)fp->prev->data)->width = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
flat_size_size_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Size_t*)fp->prev->data)->size = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	if (*data == '+')
		((Size_t*)fp->prev->data)->add = 1;
	return 0;
}

static int
flat_size_fixed_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Size_t*)fp->prev->data)->fixed = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
flat_size_offset_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Size_t*)fp->prev->data)->offset = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
flat_size_base_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Size_t*)fp->prev->data)->base = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static Tags_t	tags_flat_size[] =
{
	"TYPE",		"Size field physical type. The types are:"
			" ascii - ascii text digits,"
			" be_t - big endian binary,"
			" ebcdic - ebcdic text digits,"
			" le_t - little endian binary,"
			" magic - binary with same byte order as the"
			" header magic number.",
			0,0,flat_size_type_dat,0,
	"WIDTH",	"Size field width in bytes.",
			0,0,flat_size_width_dat,0,
	"OFFSET",	"Size field offset in bytes.",
			0,0,flat_size_offset_dat,0,
	"BASE",		"Size field field representation base.",
			0,0,flat_size_base_dat,0,
	"SIZE",		"Size field total width in bytes, <WIDTH>+<OFFSET>"
			" by default. A + prefix specifies that the size"
			" field total width must be added to the computed"
			" size to determine the record length.",
			0,0,flat_size_size_dat,0,
	"FIXED",	"Fixed record size.",
			0,0,flat_size_fixed_dat,0,
	0
};

static Tags_t*
flat_size_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	Size_t*			z;

	if (name)
	{
		if (!(z = newof(0, Size_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		if (*name == 'B')
			flat->block = z;
		else
			flat->record = z;
		fp->data = z;
	}
	return &tags_flat_size[0];
}

static int
flat_size_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	register Size_t*	z = (Size_t*)fp->data;
	int			n;

	if (z->size < (z->offset + z->width))
		z->size = z->offset + z->width;
	z->reserve = z->fixed;
	if (z->size)
	{
		z->fixed = 0;
		if (z->reserve < z->size)
			z->reserve = z->size;
	}
	if (!(z->buf = newof(0, char, z->reserve, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (z->type == 'e')
	{
		if (!flat->e2a)
			flat->e2a = ccmap(CC_EBCDIC_O, CC_ASCII);
	}
	return 0;
}

static int
flat_section_count_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->section->count = strtoul(data, &e, 0);
	if (*e && (*e != '*' || *(e + 1)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
flat_section_delimiter_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->section->delimiter = *data;
	return 0;
}

static int
flat_section_pattern_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	int			code;

	if (!(flat->section->re = newof(0, regex_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (code = regcomp(flat->section->re, data, REG_AUGMENTED|REG_LENIENT))
	{
		if (disc->errorf)
		{
			char	buf[256];

			regerror(code, flat->section->re, buf, sizeof(buf));
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "regular expression: %s: %s", data, buf);
		}
		return -1;
	}
	return 0;
}

static int
flat_section_size_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->section->size = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static Tags_t	tags_flat_section[] =
{
	"COUNT",	"Number of delimiters, patterns or sized blocks.",
			0,0,flat_section_count_dat,0,
	"DELIMITER",	"Delimiter character.",
			0,0,flat_section_delimiter_dat,0,
	"PATTERN",	"Regular expression; ignored if"
			" delimiter or size specified.",
			0,0,flat_section_pattern_dat,0,
	"SIZE",		"Fixed size in bytes.",
			0,0,flat_section_size_dat,0,
	0
};

static Tags_t*
flat_header_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	Section_t*		s;

	if (name)
	{
		if (!(s = newof(0, Section_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		s->count = 1;
		s->delimiter = -1;
		if (!flat->lastheader)
			flat->header = flat->lastheader = s;
		else
			flat->lastheader = flat->lastheader->next = s;
		flat->section = s;
	}
	return &tags_flat_section[0];
}

static Tags_t*
flat_trailer_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	Section_t*		s;

	if (name)
	{
		if (!(s = newof(0, Section_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		s->count = 1;
		s->delimiter = -1;
		if (!flat->lasttrailer)
			flat->trailer = flat->lasttrailer = s;
		else
			flat->lasttrailer = flat->lasttrailer->next = s;
		flat->section = s;
	}
	return &tags_flat_section[0];
}

static int
flat_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->meth.name = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_print_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->meth.print = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_description_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->meth.description = (const char*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_library_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	register Library_t*	p;

	if (!(p = newof(0, Library_t, 1, strlen(data))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	strcpy(p->name, data);
	if (!flat->lastlibrary)
		flat->libraries = flat->lastlibrary = p;
	else
		flat->lastlibrary = flat->lastlibrary->next = p;
	return 0;
}

static int
flat_magic_string_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->magic->string = (const char*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (!flat->magic->length)
		flat->magic->length = strlen(flat->magic->string);
	return 0;
}

static int
flat_magic_number_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->magic->number = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
flat_magic_size_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->magic->size = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static Value_t	flat_swap_val[] =
{
	"none",		SWAP_none,
	"native",	SWAP_native,
	"be",		SWAP_be,
	"le",		SWAP_le,
};

static int
flat_magic_swap_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	register Value_t*	v;
	char*			e;

	for (v = flat_swap_val; v < &flat_swap_val[elementsof(flat_swap_val)]; v++)
		if (!strcasecmp(data, v->name))
		{
			flat->magic->swap = v->value;
			break;
		}
	if (v >= &flat_swap_val[elementsof(flat_swap_val)])
	{
		flat->magic->swap = strtoul(data, &e, 0);
		if (*e)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
			return -1;
		}
	}
	if (flat->magic->swap >= 0)
		flat->magic->swap = SWAP(flat->magic->swap);
	return 0;
}

static int
flat_magic_version_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	char*			e;

	flat->magic->version = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static Tags_t	tags_flat_magic[] =
{
	"STRING",	"Magic string value.",
			0,0,flat_magic_string_dat,0,
	"NUMBER",	"Magic number value.",
			0,0,flat_magic_number_dat,0,
	"LENGTH",	"Magic number/string size in bytes.",
			0,0,flat_magic_size_dat,0,
	"SIZE",		"Magic header size in bytes. If omitted then an AST"
			"magic header is assumed.",
			0,0,flat_magic_size_dat,0,
	"SWAP",		"Magic header binary field swap operation:"
			" native - native byte order (default),"
			" be - big endian swap,"
			" le - little endian swap,"
			" 1|2|4 - swap OR of 1:bytes 2:shorts 4:ints.",
			0,0,flat_magic_swap_dat,0,
	"VERSION",	"Magic version stamp, either YYYYMMDD or 0xWWXXYYZZ.",
			0,0,flat_magic_version_dat,0,
	0
};

static Tags_t*
flat_magic_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->magic = newof(0, Magic_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	flat->magic->swap = SWAP_native;
	return &tags_flat_magic[0];
}

static int
flat_magic_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (flat->magic)
	{
		if (!flat->magic->length)
			flat->magic->length = 4;
		if (!flat->magic->size)
			flat->magic->size = flat->magic->length;
		if (flat->magic->size < flat->magic->length)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "magic header size %u is smaller than string/number length %u", flat->magic->size, flat->magic->length);
			return -1;
		}
		if (!flat->magic->string && (flat->magic->length > 4 || (flat->magic->length & (flat->magic->length - 1))))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "magic number length %u must be a power of 2 less than or equal to 4", flat->magic->length);
			return -1;
		}
	}
	return 0;
}

static int
flat_physical_swap_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;
	register Value_t*	v;
	char*			e;

	for (v = flat_swap_val; v < &flat_swap_val[elementsof(flat_swap_val)]; v++)
		if (!strcasecmp(data, v->name))
		{
			flat->swap = v->value;
			break;
		}
	if (v >= &flat_swap_val[elementsof(flat_swap_val)])
	{
		flat->swap = strtoul(data, &e, 0);
		if (*e)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
			return -1;
		}
	}
	if (flat->swap >= 0)
		flat->swap = SWAP(flat->swap);
	return 0;
}

static int
flat_compress_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (!(flat->meth.compress = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
flat_physical_continue_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->continuator = *data;
	return 0;
}

static int
flat_physical_delimiter_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->delimiter = *data;
	return 0;
}

static int
flat_physical_escape_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->escape = *data;
	return 0;
}

static int
flat_physical_quotebegin_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->quotebegin = *data;
	return 0;
}

static int
flat_physical_quoteend_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->quoteend = *data;
	return 0;
}

static int
flat_physical_quoteall_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (strtol(data, NiL, 0) > 0)
		flat->flags |= CX_QUOTEALL;
	else
		flat->flags &= ~CX_QUOTEALL;
	return 0;
}

static int
flat_physical_multiple_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if (strtol(data, NiL, 0) > 0)
		flat->flags |= CX_MULTIPLE;
	else
		flat->flags &= ~CX_MULTIPLE;
	return 0;
}

static int
flat_physical_codeset_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	if ((flat->code = ccmapid(data)) < 0)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: unknown character code set", data);
		return -1;
	}
	return 0;
}

static int
flat_physical_terminator_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Flat_t*	flat = (Flat_t*)disc;

	flat->terminator = *data;
	if (flat->code != CC_NATIVE && (fp->attr & TAG_ATTR_conv))
		flat->terminator = ccmapc(flat->terminator, CC_NATIVE, flat->code);
	return 0;
}

static Tags_t	tags_flat_physical_key[] =
{
	"ID1",		"Characters that may appear anywhere in key names,"
			" interpreted as the contents of an RE character"
			" class. The default is \b[:alpha:]_\b.",
			0,0,flat_field_key_id1_dat,0,
	"ID2",		"Characters that may appear after the first character"
			" in key names, interpreted as the contents of an RE"
			" character class. The default is"
			" \b[:alnum:]_.,-\b.",
			0,0,flat_field_key_id2_dat,0,
	"SPAN",		"The value is the key assignment character. If set"
			" then key values may span the field separator"
			" character up to the end of record or the next"
			" \akey\a\aspan\a\avalue\a. Such data is frought with"
			" ambiguities. One must suppose that it never occurred"
			" to the writers that the data would someday be read.",
			0,0,flat_field_key_span_dat,0,
	"UNKNOWN",	"Span unknown but otherwise syntactically correct"
			" keys.",
			0,0,flat_field_key_unknown_dat,0,
	0
};

static Tags_t*
flat_physical_key_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	return &tags_flat_physical_key[0];
}

static Tags_t	tags_flat_physical[] =
{
	"SWAP",		"Binary record field swap operation:"
			" none - don't swap (default),"
			" native - swap to match the magic header NUMBER,"
			" be - big endian swap,"
			" le - little endian swap,"
			" 1|2|4 - swap OR of 1:bytes 2:shorts 4:ints.",
			0,0,flat_physical_swap_dat,0,
	"DELIMITER",	"Default field delimiter and continuation replacement"
			" character.",
			0,0,flat_physical_delimiter_dat,0,
	"ESCAPE",	"Default field delimiter and/or quote escape character.",
			0,0,flat_physical_escape_dat,0,
	"QUOTE",	"Default field quote begin and end character.",
			0,0,flat_physical_quotebegin_dat,0,
	"QUOTEBEGIN",	"Default field quote begin character.",
			0,0,flat_physical_quotebegin_dat,0,
	"QUOTEEND",	"Default field quote end character.",
			0,0,flat_physical_quoteend_dat,0,
	"QUOTEALL",	"Field quotes are the first and last characetrs.",
			0,0,flat_physical_quoteall_dat,0,
	"MULTIPLE",	"Multiple adjacent delimiters are equivalent to one.",
			0,0,flat_physical_multiple_dat,0,
	"CODESET",	"Default records codeset name; one of { native ascii ebcdic }.",
			0,0,flat_physical_codeset_dat,0,
	"CONTINUE",	"Terminator continuation (escape) character.",
			0,0,flat_physical_continue_dat,0,
	"TERMINATOR",	"Default record termination character.",
			0,0,flat_physical_terminator_dat,0,
	"KEY",		"Default field key attributes.",
			0,flat_physical_key_beg,0,0,
};

static Tags_t*
flat_physical_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	return &tags_flat_physical[0];
}

static Tags_t	tags_flat[] =
{
	"NAME",		"Schema name.",
			0,0,flat_name_dat,0,
	"DESCRIPTION",	"Schema description.",
			0,0,flat_description_dat,0,
	"IDENT",	"Schema ident string.",
			0,0,0,0,
	"LIBRARY",	"Required type/map library name;"
			" more than one library may be specified.",
			0,0,flat_library_dat,0,
	"MAGIC",	"File magic number (identification header)"
			" section definitions.",
			0,flat_magic_beg,0,flat_magic_end,
	"PRINT",	"Default {print} query format.",
			0,0,flat_print_dat,0,
	"COMPRESS",	"Preferred compression method; compression is applied"
			" by the {compress} query.",
			0,0,flat_compress_dat,0,
	"HEADER",	"File header section definitions.",
			0,flat_header_beg,0,0,
	"TRAILER",	"File trailer section definitions.",
			0,flat_trailer_beg,0,0,
	"BLOCK",	"Block size definitions.",
			0,flat_size_beg,0,flat_size_end,
	"RECORD",	"Record size definitions; same as <BLOCK>.",
			0,flat_size_beg,0,flat_size_end,
	"PHYSICAL",	"Default physical record and field attributes.",
			0,flat_physical_beg,0,0,
	"FIELD",	"Schema field list.",
			0,flat_field_beg,0,flat_field_end,
	0
};

static Tags_t*
flat_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	return &tags_flat[0];
}

static Tags_t	tags[] =
{
	"FLAT",		"Flat method schema.",
			0,flat_beg,0,0,
	"MAP",		"Field value map;"
			" either a map reference name or a map definition.",
			0,dss_map_beg,dss_map_dat,dss_map_end,
	"METHOD",	"Method name; must be flat.",
			0,0,0,0,
	0
};

/*
 * outal rententive support
 */

static int
tabs(Sfio_t* op, int cur, int nxt)
{
	cur = (nxt - cur - 1) / 8 + 1;
	while (cur-- > 0)
		sfputc(op, '\t');
	return nxt;
}

/*
 * fill in type and format defaults
 */

static void
defaults(register Cxtype_t* type, register Cxformat_t* format, int binary, Dssdisc_t* disc)
{
	register char*	s;
	Cxstate_t*	state = cxstate(disc);
	Cxtype_t*	base;
	char		details[16];

	if (!(format->flags & (CX_INTEGER|CX_FLOAT|CX_STRING|CX_BUFFER)))
		format->flags |= type->format.flags & (CX_BINARY|CX_UNSIGNED|CX_INTEGER|CX_FLOAT|CX_STRING|CX_BUFFER);
	else if ((type->format.flags & CX_BINARY) && (format->flags & (CX_INTEGER|CX_FLOAT)))
		format->flags |= CX_BINARY;
	if (!type->internalf || !type->externalf)
	{
		if (cxisvoid(type))
			base = state->type_void;
		else if (cxisstring(type))
			base = state->type_string;
		else if (cxisbuffer(type))
			base = state->type_buffer;
		else
		{
			base = state->type_number;
			if (!(format->flags & CX_FLOAT))
			{
				s = details;
				*s++ = '%';
				if (format->fill > 0)
					*s++ = format->fill;
				if (format->width)
					s += sfsprintf(s, &details[sizeof(details)] - s, "%d", format->width);
				*s++ = 'l';
				*s++ = 'l';
				switch (format->base)
				{
				default:
					if (format->base < 64)
					{
						*s++ = '.';
						*s++ = '.';
						s += sfsprintf(s, &details[sizeof(details)] - s, "%d", format->base);
						break;
					}
					/*FALLTHROUGH*/
				case 0:
				case 10:
					*s++ = (format->flags & CX_UNSIGNED) ? 'u' : 'd';
					break;
				case 8:
					*s++ = 'o';
					break;
				case 16:
					*s++ = 'x';
					break;
				}
				*s = 0;
				format->details = strdup(details);
			}
		}
		if (!type->internalf)
			type->internalf = base->internalf;
		if (!type->externalf)
			type->externalf = base->externalf;
	}
}

/*
 * methf
 */

extern Dsslib_t	dss_lib_flat;

static Dssmeth_t*
flatmeth(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	register Flat_t*	flat;
	register Library_t*	p;
	register Field_t*	f;
	Field_t*		g;
	Tag_t*			tag;
	Sfio_t*			sp;
	char*			s;
	ssize_t			n;
	int			i;
	int			errors;
	int			fixed;
	char			path[PATH_MAX];

	if (!(flat = newof(0, Flat_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	flat->basemeth = meth;
	flat->meth = *meth;
	flat->meth.data = flat;
	taginit(&flat->dsstagdisc.tagdisc, disc->errorf);
	flat->dsstagdisc.tagdisc.id = DSS_ID;
	flat->dsstagdisc.disc = disc;
	flat->dsstagdisc.meth = meth;
	flat->binary = 1;
	flat->code = CC_NATIVE;
	flat->swap = SWAP_none;
	flat->delimiter = flat->escape = flat->quotebegin = flat->quoteend = flat->terminator = flat->continuator = -1;
	sp = 0;
	if (options)
	{
		if (!(sp = sfstropen()))
			goto drop;
		sfprintf(sp, "%s", usage);
		if (tagusage(tags, sp, &flat->dsstagdisc.tagdisc))
			goto drop;
		sfprintf(sp, "}\n");
		if (dssoptlib(meth->cx->buf, &dss_lib_flat, sfstruse(sp), disc))
			goto drop;
		sfclose(sp);
		sp = 0;
		s = sfstruse(meth->cx->buf);
		for (;;)
		{
			switch (optstr(options, s))
			{
			case 'b':
				flat->binary = opt_info.num;
				continue;
			case 'e':
				flat->emptyspace = 1;
				continue;
			case 'h':
			case 'o':
			case 's':
				flat->list = opt_info.option[1];
				continue;
			case 'p':
				flat->prototype = 1;
				continue;
			case 'T':
				flat->test = opt_info.num;
				continue;
			case '?':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
				goto drop;
			case ':':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
				goto drop;
			}
			break;
		}
	}
	if (schema && *schema)
	{
		if (!(sp = dssfind(schema, NiL, DSS_VERBOSE, path, sizeof(path), disc)))
			return 0;
		if (!(tag = tagopen(sp, path, 1, &tags[0], &flat->dsstagdisc.tagdisc)) || tagclose(tag))
			goto drop;
		sfclose(sp);
		sp = 0;
		if (!flat->fields)
			goto invalid;
	}
	dtinsert(flat->meth.formats, &flat_format);
	for (p = flat->libraries; p; p = p->next)
		if (!dssload(p->name, disc))
			return 0;
	for (i = 0; i < elementsof(local_callouts); i++)
		if (cxaddcallout(flat->meth.cx, &local_callouts[i], disc))
			return 0;
	for (f = flat->fields; f; f = f->next)
	{
		if (cxaddvariable(flat->meth.cx, &f->variable, disc))
			return 0;
		if ((s = (char*)f->width) && !(f->width = keycomp(flat, s, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: %s: invalid field width index", f->variable.name, s);
			return 0;
		}
		if (f->variable.array && (s = (char*)f->variable.array->variable) && !(f->variable.array->variable = (Cxvariable_t*)dtmatch(flat->meth.cx->variables, s)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: %s: unknown array index", f->variable.name, s);
			return 0;
		}
		defaults(f->variable.type, &f->variable.format, 0, disc);
		if (!(s = (char*)f->physical.type))
			f->physical.type = f->variable.type;
		else if (!(f->physical.type = cxtype(flat->meth.cx, s, disc)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: %s: unknown type", f->variable.name, s);
			return 0;
		}
		defaults(f->physical.type, &f->physical.format, flat->binary, disc);
		if (!f->physical.format.delimiter)
			f->physical.format.delimiter = f->variable.format.delimiter;
		if (!(f->physical.format.flags & (CX_INTEGER|CX_FLOAT|CX_STRING|CX_BUFFER)))
			f->physical.format.flags |= f->variable.format.flags & (CX_BINARY|CX_UNSIGNED|CX_INTEGER|CX_FLOAT|CX_STRING|CX_BUFFER|CX_NUL);
		if (f->physical.format.flags & CX_BINARY)
		{
			if (!f->variable.format.width && f->physical.type)
				f->variable.format.width = f->physical.type->format.width;
		}
		else if (!f->physical.format.map)
			f->physical.format.map = f->variable.format.map;
		if (f->physical.format.flags & CX_BUFFER)
			f->variable.format.flags |= CX_BUFFER;
	}
	if (flat->lastfield && flat->terminator < 0)
		flat->terminator = flat->lastfield->physical.format.delimiter;
	if (flat->terminator < 0)
		flat->continuator = -1;
	if ((flat->continuator >= 0 || flat->record) && !(flat->buf = sfstropen()))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	errors = 0;
	fixed = 1;
	for (f = flat->fields; f; f = f->next)
	{
		if (!(f->physical.format.flags & (CX_BINARY|CX_STRING|CX_BUFFER)))
			flat->binary = 0;
		if (f->physical.format.delimiter < 0 && !f->physical.format.width && !f->width && !f->keyed)
		{
			if (f->structure.members)
			{
				f->structref = 1;
				continue;
			}
			errors++;
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: a field delimiter or size must be specified", f->variable.name);
		}
		else if (f->structure.members)
		{
#if 0
			if (f->physical.format.width || f->width)
				/*OK*/;
			else if (disc->errorf)
				(*disc->errorf)(NiL, disc, 1, "%s: structure attributes ignored", f->variable.name);
#endif
			continue;
		}
		if (!f->variable.type)
		{
			f->variable.type = (Cxtype_t*)"string";
			f->variable.format.flags |= CX_STRING;
		}
		if (f->physical.format.delimiter == flat->terminator && f != flat->lastfield)
			flat->terminator = -1;
		if (f->physical.format.width)
		{
			flat->fixed += f->physical.format.width;
			if (f->width)
				flat->variable = SF_LOCKR;
		}
		else
			fixed = 0;
		if (f->physical.format.quotebegin >= 0 && f->physical.format.quoteend < 0)
			f->physical.format.quoteend = f->physical.format.quotebegin;
		if (f->physical.format.quotebegin < 0)
			f->physical.format.flags &= ~CX_QUOTEALL;
		f->map = ccmap(f->physical.format.code, CC_NATIVE);
	}
	if (errors)
		return 0;
	if (flat->block)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "blocked data not supported yet");
		return 0;
	}
	if (flat->record)
	{
		if (flat->record->fixed && flat->fixed > flat->record->reserve)
		{
			flat->truncate.fixed = 1;
			n = flat->record->reserve;
			for (f = flat->fields; f; f = f->next)
			{
				if (f->width && f->physical.format.width)
				{
					g = f;
					while (g = g->next)
						n -= g->physical.format.width;
					if (n > 0)
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 1, "%s: maximum variable field size shortened from %d to %d to comply with fixed record size %d", f->variable.name, f->physical.format.width, n, flat->record->reserve);
						f->physical.format.width = n;
						flat->truncate.fixed = 0;
					}
					break;
				}
				n -= f->physical.format.width;
			}
		}
		flat->fixed = flat->record->reserve;
		if (!flat->record->width)
			flat->record->width = flat->record->size;
	}
	else if (!fixed)
	{
		flat->fixed = 0;
		flat->binary = 0;
	}
	return &flat->meth;
 invalid:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: invalid schema", options);
 drop:
	free(flat);
	if (sp)
		sfclose(sp);
	return 0;
}

/*
 * openf
 */

static int
flatopen(Dss_t* dss, Dssdisc_t* disc)
{
	Flat_t*			flat = (Flat_t*)dss->meth->data;
	register Field_t*	f;
	register Cxvariable_t*	v;
	char*			a;
	char*			s;
	char*			p;
	char*			t;
	char*			u;
	int			offset;
	int			fixed;
	int			pad;
	int			m;
	int			n;
	char			tmp[2];

	if (flat)
	{
		if (flat->prototype)
		{
			for (f = flat->fields; f; f = f->next)
			{
				if ((n = f->variable.format.delimiter) < 0 && (n = f->physical.format.delimiter) < 0)
					n = f->next ? '|' : '\n';
				sfprintf(sfstdout, "%s%c", cxisvoid(f->variable.type) ? "" : f->variable.name, n);
			}
		}
		switch (flat->list)
		{
		case 'h':
			if (!(a = newof(0, char, 3 * (strlen(flat->meth.name) + 1), 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(dss, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			p = a;
			for (s = (char*)flat->meth.name; *s; s++)
				*a++ = isalnum(*s) ? *s : '_';
			*a++ = 0;
			if (!isalpha(*p))
				*p = '_';
			t = a;
			a = stpcpy(a, flat->meth.name);
			*a++ = 0;
			if (islower(*t))
				*t = toupper(*t);
			u = a;
			for (s = p; *s; s++)
				*a++ = islower(*s) ? toupper(*s) : *s;
			*a = 0;
			sfprintf(sfstdout, "/*\n");
			sfprintf(sfstdout, " * %s dynamic interface\n", flat->meth.name);
			if (flat->meth.description)
				sfprintf(sfstdout, " * %s\n", flat->meth.description);
			sfprintf(sfstdout, " */\n\n");
			sfprintf(sfstdout, "#define %s_RECORD(data)	(_%s_record_=(%s_record_t*)DSSDATA(data))\n", u, p, t);
			sfprintf(sfstdout, "\n");
			sfprintf(sfstdout, "typedef Cxvalue_t* (*%s_get_f)(void*,int);\n", t);
			sfprintf(sfstdout, "\n");
			sfprintf(sfstdout, "typedef struct %s_field_s		/* record field			*/\n", t);
			sfprintf(sfstdout, "{\n");
			sfprintf(sfstdout, "	Cxvalue_t	value;		/* value (first for dynamic Q)	*/\n");
			sfprintf(sfstdout, "	void*		field;		/* static field info		*/\n");
			sfprintf(sfstdout, "	size_t		off;		/* record data offset		*/\n");
			sfprintf(sfstdout, "	size_t		siz;		/* record data size		*/\n");
			sfprintf(sfstdout, "	unsigned int	serial;		/* read serial number		*/\n");
			sfprintf(sfstdout, "	unsigned int	keyed;		/* keyed serial number		*/\n");
			sfprintf(sfstdout, "} %s_field_t;\n", t);
			sfprintf(sfstdout, "\n");
			sfprintf(sfstdout, "typedef struct %s_record_s		/* current record info		*/\n", t);
			sfprintf(sfstdout, "{\n");
			sfprintf(sfstdout, "	%s_field_t*	fields;		/* fields (first for dynamic Q)	*/\n", t);
			sfprintf(sfstdout, "	%s_get_f	getf;		/* getf (second for dynamic Q)	*/\n", t);
			sfprintf(sfstdout, "} %s_record_t;\n", t);
			sfprintf(sfstdout, "\n");
			sfprintf(sfstdout, "static %s_record_t*	_%s_record_;\n", t, p);
			sfprintf(sfstdout, "\n");
			for (f = flat->fields; f; f = f->next)
			{
				if (f->structure.members)
					continue;
				n = sfprintf(sfstdout, "#define %s_%s", p, f->variable.name);
				n = tabs(sfstdout, n, 32);
				n += sfprintf(sfstdout, "((*_%s_record_->getf)(_%s_record_,%d)->", p, p, f->variable.index);
				if (f->variable.format.flags & (CX_BUFFER|CX_STRING))
				{
					a = (f->variable.format.flags & CX_STRING) ? "string" : "buffer";
					n += sfprintf(sfstdout, "%s.data)", a);
				}
				else
					n += sfprintf(sfstdout, "number)");
				offset += m * f->physical.format.width;
				if (f->variable.description)
				{
					tabs(sfstdout, n, 64);
					sfprintf(sfstdout, "/* %s */\n", f->variable.description);
				}
				else
					sfputc(sfstdout, '\n');
				if (f->variable.format.flags & (CX_BUFFER|CX_STRING))
				{
					n = sfprintf(sfstdout, "#define %s_%s_size", p, f->variable.name);
					tabs(sfstdout, n, 32);
					sfprintf(sfstdout, "((*_%s_record_->getf)(_%s_record_,%d)->%s.size)\n", p, p, f->variable.index, a);
				}
			}
			break;
		case 'o':
			if (!flat->fixed)
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "variable width record offsets not supported");
				return -1;
			}
			/* HERE: under construction */
			offset = 0;
			pad = 0;
			for (f = flat->fields; f; f = f->next)
			{
				if (f->structure.members)
				{
					f->structure.size = offset;
					continue;
				}
				a = p = s = "";
				m = 1;
				n = 0;
				if (f->variable.array)
				{
					if (f->variable.array->size)
					{
						m = f->variable.array->size;
						a = sfprints("[%d]", f->variable.array->size);
					}
					else
						p = "*";
				}
				if (!f->structure.level)
					f->variable.structure = 0;
				if (f->variable.format.flags & CX_BUFFER)
				{
					n += sfprintf(sfstdout, "struct _%s_%s_buf_s", flat->basemeth->name, flat->meth.name);
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "%s%s%s;", p, f->variable.name, a);
				}
				else if ((f->variable.format.flags & CX_STRING) || !(f->physical.format.flags & CX_BINARY))
				{
					n += sfprintf(sfstdout, "char");
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "%s%s%s", p, f->variable.name, a);
					if (f->physical.format.width > 1)
						n += sfprintf(sfstdout, "[%u];", f->physical.format.width);
					else
						n += sfprintf(sfstdout, ";");
				}
				else
				{
					if (f->variable.format.flags & CX_FLOAT)
						n += sfprintf(sfstdout, "_ast_flt");
					else
					{
						if ((f->variable.format.flags & CX_UNSIGNED) || !streq((char*)f->variable.type, "number"))
							n += sfprintf(sfstdout, "unsigned ");
						n += sfprintf(sfstdout, "_ast_int");
					}
					n += sfprintf(sfstdout, "%u_t", f->physical.format.width);
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "%s%s%s;", p, f->variable.name, a);
					if ((offset % f->physical.format.width) && !cxisvoid(f->variable.type))
					{
						s = "(MISALIGNED) ";
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 1, "%s: field size %u offset %u is not aligned", f->variable.name, f->physical.format.width, offset);
					}
				}
				offset += m * f->physical.format.width;
				sfputc(sfstdout, '\n');
				if (f->physical.format.delimiter >= 0)
				{
					n = sfprintf(sfstdout, "char");
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "_delimiter_%d;", ++pad);
					tabs(sfstdout, n, 40);
					offset += 1;
					tmp[0] = f->physical.format.delimiter;
					tmp[1] = 0;
					sfprintf(sfstdout, "/* delimiter '%s' */\n", fmtesc(tmp));
				}
				if (!f->structure.next)
				{
					v = &f->variable;
					if (v->structure)
						while (!v->structure->next && (v = v->structure->parent) && v->structure)
						{
							v->structure->size = offset - v->structure->size;
							if (v->array)
							{
								if (v->array->size)
								{
									offset += (v->array->size - 1) * v->structure->size;
									sfprintf(sfstdout, "} %s[%d]", v->name, v->array->size);
								}
								else
									sfprintf(sfstdout, "} *%s", v->name);
								if (v->array->variable)
									sfprintf(sfstdout, " /*%s*/", v->array->variable->name);
								sfprintf(sfstdout, ";\n");
							}
							else
								sfprintf(sfstdout, "} %s;\n", v->name);
						}
				}
			}
			if (flat->binary && (fixed = offset % 8))
			{
				fixed = 8 - fixed;
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 1, "%s: record has %u unused pad byte%s", flat->meth.name, fixed, fixed == 1 ? "" : "s");
			}
			sfprintf(sfstdout, "%s\t%lu\t%u\t%u\tstruct\n", ".", offset, 0, 0);
			break;
		case 's':
			if (!flat->fixed)
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "variable width record structs not supported");
				return -1;
			}
			for (f = flat->fields; f; f = f->next)
				if (f->variable.format.flags & CX_BUFFER)
				{
					sfprintf(sfstdout, "/* buffer field info */\n");
					sfprintf(sfstdout, "struct _%s_%s_buf_s\n", flat->basemeth->name, flat->meth.name);
					sfprintf(sfstdout, "{\n");
					n = sfprintf(sfstdout, "unsigned _ast_int2_t");
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "offset;");
					tabs(sfstdout, n, 40);
					sfprintf(sfstdout, "/* buffer data _HEAP_ offset */\n");
					n = sfprintf(sfstdout, "unsigned _ast_int2_t");
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "size;");
					tabs(sfstdout, n, 40);
					sfprintf(sfstdout, "/* buffer data size */\n");
					sfprintf(sfstdout, "};\n");
					break;
				}
			if (flat->meth.description)
				sfprintf(sfstdout, "/* %s */\n", flat->meth.description);
			sfprintf(sfstdout, "struct _%s_%s_s\n", flat->basemeth->name, flat->meth.name);
			sfprintf(sfstdout, "{\n");
			offset = 0;
			pad = 0;
			for (f = flat->fields; f; f = f->next)
			{
				if (f->structure.members)
				{
					f->structure.size = offset;
					sfprintf(sfstdout, "struct\n{\n");
					continue;
				}
				a = p = s = "";
				m = 1;
				n = 0;
				if (f->variable.array)
				{
					if (f->variable.array->size)
					{
						m = f->variable.array->size;
						a = sfprints("[%d]", f->variable.array->size);
					}
					else
						p = "*";
				}
				if (f->structure.level)
					n += sfprintf(sfstdout, "/*%02d*/", f->structure.level);
				else
					f->variable.structure = 0;
				if (f->variable.format.flags & CX_BUFFER)
				{
					n += sfprintf(sfstdout, "struct _%s_%s_buf_s", flat->basemeth->name, flat->meth.name);
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "%s%s%s;", p, f->variable.name, a);
				}
				else if ((f->variable.format.flags & CX_STRING) || !(f->physical.format.flags & CX_BINARY))
				{
					n += sfprintf(sfstdout, "char");
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "%s%s%s", p, f->variable.name, a);
					if (f->physical.format.width > 1)
						n += sfprintf(sfstdout, "[%u];", f->physical.format.width);
					else
						n += sfprintf(sfstdout, ";");
				}
				else
				{
					if (f->variable.format.flags & CX_FLOAT)
						n += sfprintf(sfstdout, "_ast_flt");
					else
					{
						if ((f->variable.format.flags & CX_UNSIGNED) || !streq((char*)f->variable.type, "number"))
							n += sfprintf(sfstdout, "unsigned ");
						n += sfprintf(sfstdout, "_ast_int");
					}
					n += sfprintf(sfstdout, "%u_t", f->physical.format.width);
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "%s%s%s;", p, f->variable.name, a);
					if ((offset % f->physical.format.width) && !cxisvoid(f->variable.type))
					{
						s = "(MISALIGNED) ";
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 1, "%s: field size %u offset %u is not aligned", f->variable.name, f->physical.format.width, offset);
					}
				}
				offset += m * f->physical.format.width;
				if (*s || f->variable.description)
				{
					tabs(sfstdout, n, 40);
					sfprintf(sfstdout, "/* %s%s */\n", s, f->variable.description ? f->variable.description : "");
				}
				else
					sfputc(sfstdout, '\n');
				if (f->physical.format.delimiter >= 0)
				{
					n = sfprintf(sfstdout, "char");
					n = tabs(sfstdout, n, 24);
					n += sfprintf(sfstdout, "_delimiter_%d;", ++pad);
					tabs(sfstdout, n, 40);
					offset += 1;
					tmp[0] = f->physical.format.delimiter;
					tmp[1] = 0;
					sfprintf(sfstdout, "/* delimiter '%s' */\n", fmtesc(tmp));
				}
				if (!f->structure.next)
				{
					v = &f->variable;
					if (v->structure)
						while (!v->structure->next && (v = v->structure->parent) && v->structure)
						{
							v->structure->size = offset - v->structure->size;
							if (v->array)
							{
								if (v->array->size)
								{
									offset += (v->array->size - 1) * v->structure->size;
									sfprintf(sfstdout, "} %s[%d]", v->name, v->array->size);
								}
								else
									sfprintf(sfstdout, "} *%s", v->name);
								if (v->array->variable)
									sfprintf(sfstdout, " /*%s*/", v->array->variable->name);
								sfprintf(sfstdout, ";\n");
							}
							else
								sfprintf(sfstdout, "} %s;\n", v->name);
						}
				}
			}
			sfprintf(sfstdout, "};\n");
			if (flat->binary && (fixed = offset % 8))
			{
				fixed = 8 - fixed;
				sfprintf(sfstdout, "/* struct has %u unused pad byte%s */\n", fixed, fixed == 1 ? "" : "s");
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 1, "%s: record has %u unused pad byte%s", flat->meth.name, fixed, fixed == 1 ? "" : "s");
			}
			sfprintf(sfstdout, "/* sizeof(struct _%s_%s_s)==%u */\n", flat->basemeth->name, flat->meth.name, offset);
			break;
		}
		if (flat->list || flat->prototype)
			exit(0);
		flat->getf = flatget;
		if (flat->binary && flat->fixed && !(flat->test & 0x0010))
		{
			offset = 0;
			for (f = flat->fields; f; f = f->next)
			{
				if ((f->variable.format.flags & CX_BINARY) && ((offset % f->physical.format.width) || (f->physical.format.width & (f->physical.format.width - 1)) || f->physical.format.width > 8) || f->width)
					break;
				offset += f->physical.format.width;
			}
			if (!f && !(offset % 8))
				flat->getf = flatgetbinary;
		}
		if (flat->record || (flat->fixed % 8))
			flat->getf = flatget;
		flat->valsiz = 64;
		if (!(flat->valbuf = newof(0, char, flat->valsiz, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(dss, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		if (!flat->fixed || flat->record)
			flat->variable = 0;
		if (flat->variable || (dss->flags & DSS_FORCE))
			flat->force = 1;
	}
	return 0;
}

/*
 * closef
 */

static int
flatclose(Dss_t* dss, Dssdisc_t* disc)
{
	Flat_t*			flat = (Flat_t*)dss->meth->data;

	if (!dss->meth || !(flat = (Flat_t*)dss->meth->data))
		return -1;
	if (flat->buf)
		sfstrclose(flat->buf);
	return 0;
}

static Dssmeth_t method =
{
	"flat",
	"fixed-width and/or field-delimited flat file data",
	CXH,
	flatmeth,
	flatopen,
	flatclose,
	0,
	0,
	0,
	0,
	0,
	0,
	DSS_BASE
};

static const char flatten_usage[] =
"[-1ls5P?\n@(#)$Id: dss flatten query (AT&T Research) 2013-03-01 $\n]"
USAGE_LICENSE
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?Flatten input data to match flat method \bschema\b.]"
"[e:emptyspace?Write empty field values as one \bspace\b character.]"
"\n"
"\nschema\n"
"\n"
;

/*
 * get source field value
 */

static Cxvalue_t*
flattenget(register Flatten_t* flatten, Cxvariable_t* var, void* data)
{
	Cxinstruction_t	x;
	Cxreference_t*	ref;

	if (ref = var->reference)
	{
		x.data.variable = ref->variable;
		if ((*flatten->cx->getf)(flatten->cx, &x, &flatten->value, NiL, NiL, data, flatten->cx->disc))
			return &nullval;
		while (ref = ref->next)
		{
			x.data.variable = ref->variable;
			if ((*ref->member->getf)(flatten->cx, &x, &flatten->value, NiL, NiL, NiL, flatten->cx->disc))
				return &nullval;
		}
	}
	else
	{
		x.data.variable = var;
		if ((*flatten->cx->getf)(flatten->cx, &x, &flatten->value, NiL, NiL, data, flatten->cx->disc))
			return &nullval;
	}
	return &flatten->value.value;
}

/*
 * get source field size value
 */

static Cxvalue_t*
flattensize(register Flatten_t* flatten, Cxvariable_t* var, void* data)
{
	Cxvalue_t*	val;

	if ((val = flattenget(flatten, var, data)) != &nullval)
		val->number = val->string.size;
	return val;
}

/*
 * flattenget() with cxnum2str()
 */

static Cxvalue_t*
flattennum2str(register Flatten_t* flatten, Cxvariable_t* var, void* data)
{
	Cxvalue_t*	val;

	if ((val = flattenget(flatten, var, data)) != &nullval)
	{
		if (cxnum2str(flatten->cx, &var->format, (Cxinteger_t)val->number, &val->string.data))
			return &nullval;
		val->string.size = strlen(val->string.data);
	}
	return val;
}

/*
 * flattenget() with cxstr2num()
 */

static Cxvalue_t*
flattenstr2num(register Flatten_t* flatten, Cxvariable_t* var, void* data)
{
	Cxvalue_t*	val;
	Cxunsigned_t	u;

	if ((val = flattenget(flatten, var, data)) != &nullval)
	{
		if (cxstr2num(flatten->cx, &var->format, val->string.data, val->string.size, &u))
			return &nullval;
		val->number = (Cxinteger_t)u;
	}
	return val;
}

/*
 * get nullval field value
 */

static Cxvalue_t*
flattennull(Flatten_t* flatten, Cxvariable_t* var, void* data)
{
	return &nullval;
}

/*
 * flatten query begin
 */

static int
flatten_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**		argv = (char**)data;
	int		errors = error_info.errors;
	int		emptyspace = 0;
	char*		schema;
	Dssmeth_t*	meth;
	Flatten_t*	flatten;
	Field_t*	f;
	Field_t*	g;
	Vmalloc_t*	vm;
	int		offset;
	int		fixed;

	for (;;)
	{
		switch (optget(argv, flatten_usage))
		{
		case 'e':
			emptyspace = 1;
			continue;
		case '?':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		return -1;
	argv += opt_info.index;
	if (!(schema = *argv++) || *argv)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", optusage(NiL));
		return -1;
	}
	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)) || !(flatten = vmnewof(vm, 0, Flatten_t, 1, 0)))
	{
		if (vm)
			vmclose(vm);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
#if _HUH_2010_05_28
	if (!strchr(schema, ':'))
		schema = sfprints("%s:%s", *(char**)data, schema);
#endif
	flatten->vm = vm;
	flatten->emptyspace = emptyspace;
	if (!(meth = dssmeth(schema, disc)) || !(flatten->dss = dssopen(0, 0, disc, meth)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: cannot open conversion method", schema);
		goto bad;
	}
	flatten->cx = flatten->dss->cx;
	if (!flatten->cx->getf && !(flatten->cx->getf = cxcallout(flatten->cx, CX_GET, flatten->cx->state->type_void, flatten->cx->state->type_void, flatten->cx->disc)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 3, "cx CX_GET callout must be defined");
		goto bad;
	}
	flatten->getf = flatten->cx->getf;
	flatten->flat = (Flat_t*)flatten->dss->meth->data;
	offset = 0;
	fixed = flatten->flat->fixed;
	for (f = flatten->flat->fields; f; f = f->next)
	{
		f->flattengetf = flattenget;
		if (!(g = (Field_t*)dtmatch(cx->variables, f->variable.name)) && f->structure.parent && (g = (Field_t*)dtmatch(cx->variables, f->structure.parent->name)))
		{
			if (cxisnumber(f->variable.type) && cxisstring(g->variable.type))
				f->flattengetf = flattensize;
			else if (!cxisstring(f->variable.type))
				g = 0;
		}
		if (!(f->flatten = (Cxvariable_t*)g))
		{
			f->flattengetf = flattennull;
			if (!cxisvoid(f->variable.type) && disc->errorf)
				(*disc->errorf)(NiL, disc, 1, "%s: field not in source record -- default value will be output", f->variable.name);
		}
		else if (f->flatten->format.map)
		{
			if (cxisnumber(f->variable.type) && cxisstring(f->flatten->type))
				f->flattengetf = flattenstr2num;
			else if (cxisstring(f->variable.type) && cxisnumber(f->flatten->type))
				f->flattengetf = flattennum2str;
		}
		if (g && g->physical.format.code != f->physical.format.code && g->physical.format.code == CC_NATIVE)
			f->pam = ccmap(g->physical.format.code, f->physical.format.code);
		if (fixed && (f->variable.format.flags & CX_BINARY) && (offset % f->physical.format.width))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: field size %u offset %u is not aligned", f->variable.name, f->physical.format.width, offset);
			goto bad;
		}
		offset += f->physical.format.width;
	}
	if (flatten->flat->binary && (offset %= 8))
	{
		offset = 8 - offset;
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: record has %u unused pad byte%s", flatten->dss->meth->name, offset, offset == 1 ? "" : "s");
		goto bad;
	}
	if (!(flatten->file = dssfopen(flatten->dss, NiL, expr->op, DSS_FILE_WRITE, &flat_format)))
		goto bad;
	expr->op = flatten->file->io;
	expr->data = flatten;
	return 0;
 bad:
	if (flatten->file)
		dssfclose(flatten->file);
	if (flatten->dss)
		dssclose(flatten->dss);
	vmclose(vm);
	return -1;
}

/*
 * flatten query action
 */

static int
flatten_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register Flatten_t*	flatten = (Flatten_t*)expr->data;
	register Field_t*	f;
	register Sfio_t*	io;
	register unsigned char*	s;
	Cxvalue_t*		v;
	unsigned char*		b;
	unsigned char*		e;
	ssize_t			n;
	int			q;

	io = flatten->flat->record ? flatten->flat->buf : flatten->file->io;
	for (f = flatten->flat->fields; f; f = f->next)
	{
		v = (*f->flattengetf)(flatten, f->flatten, data);
		while ((n = (*f->physical.type->externalf)(flatten->cx, f->physical.type, NiL, &f->physical.format, v, flatten->flat->valbuf, flatten->flat->valsiz, disc)) > flatten->flat->valsiz)
		{
			n = roundof(n, 32);
			if (!(flatten->flat->valbuf = newof(flatten->flat->valbuf, char, n, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			flatten->flat->valsiz = n;
		}
		if (n < 0)
			return -1;
		else if (n > 0)
		{
			if (f->pam)
				ccmapcpy(f->pam, flatten->flat->valbuf, flatten->flat->valbuf, n);
			if ((f->physical.format.flags & (CX_STRING|CX_BUFFER)) && f->physical.format.delimiter >= 0 && (f->physical.format.escape >= 0 || f->physical.format.quotebegin >= 0))
			{
				if (f->physical.format.flags & CX_QUOTEALL)
				{
					q = 1;
					sfputc(io, f->physical.format.quotebegin);
				}
				else
					q = 0;
				for (e = (s = b = (unsigned char*)flatten->flat->valbuf) + n; s < e; s++)
					if (*s == f->physical.format.delimiter || *s == f->physical.format.escape || *s == f->physical.format.quotebegin || *s == f->physical.format.quoteend)
					{
						if (f->physical.format.escape >= 0)
						{
							sfwrite(io, b, s - b);
							sfputc(io, f->physical.format.escape);
							sfputc(io, *s);
						}
						else if (*s == f->physical.format.delimiter)
						{
							if (q)
								continue;
							q = 1;
							sfwrite(io, b, s - b);
							sfputc(io, f->physical.format.quotebegin);
							sfputc(io, *s);
						}
						else
						{
							sfwrite(io, b, s - b + 1);
							sfputc(io, *s);
							if (!q)
							{
								q = 1;
								sfputc(io, *s);
							}
						}
						b = s + 1;
					}
				if (q && !(f->physical.format.flags & CX_QUOTEALL))
				{
					q = 0;
					sfputc(io, f->physical.format.quoteend);
				}
				sfwrite(io, b, s - b);
				if (q)
					sfputc(io, f->physical.format.quoteend);
			}
			else
				sfwrite(io, flatten->flat->valbuf, n);
		}
		else if (flatten->emptyspace && f->physical.format.delimiter >= 0)
			sfputc(io, ' ');
		if (f->physical.format.delimiter >= 0)
			sfputc(io, f->physical.format.delimiter);
	}
	if (flatten->flat->record)
	{
		n = sfstrtell(io);
		sfstrseek(io, 0, SEEK_SET);
		if (size_put(flatten->file, n, flatten->flat->record, disc) || sfwrite(flatten->file->io, sfstrbase(io), n) != n)
			return -1;
	}
	return 0;
}

/*
 * flatten query end
 */

static int
flatten_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	Flatten_t*	flatten = (Flatten_t*)expr->data;
	int		r;

	if (!flatten)
		return -1;
	expr->data = 0;
	r = 0;
	if (dssfclose(flatten->file))
		r = -1;
	if (dssclose(flatten->dss))
		r = -1;
	vmclose(flatten->vm);
	return r;
}

static Cxquery_t queries[] =
{
{ "flatten",	"query to flatten input data to flat schema format",
		CXH, flatten_beg, 0, flatten_act, flatten_end },
{0}
};

Dsslib_t dss_lib_flat =
{
	"flat",
	"flat method"
	"[-1ls5Pp0?\n@(#)$Id: dss flat method (AT&T Research) 2013-03-11 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	&method,
	0,
	0,
	0,
	0,
	&queries[0],
};
