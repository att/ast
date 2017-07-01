/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2010-2012 AT&T Intellectual Property          *
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
 * xml method
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[+DESCRIPTION?The \bdss\b xml method reads XML data in two formats: "
    "pure XML (\aname=value\a attributes within tags ignored) and JSON. In "
    "general XML data provides field names but not type information, so by "
    "default all fields are treated as strings. Only fields specified in the "
    "\bdss\b(1) \aexpression\a are parsed from the data. Fields are named "
    "using \b.\b'd notation, where each prefix name represents XML tag "
    "nesting. For XML data, if all records have the same XML tag prefix then "
    "that prefix may be omitted, except that all field names must have at "
    "least one prefix component. For example, \busers.user.name\b and "
    "\buser.name\b are valid, but \bname\b is not.]"
"[+?The xml method schema is an XML document that specifies the type of "
    "one or more fields, and any libraries required to support those types.]"
"[T:test?Enable implementation-specific tests and tracing.]#[mask]"
"[+TAGS?The supported tags are:]{"
;

#include <dsslib.h>
#include <ctype.h>

struct Field_s; typedef struct Field_s Field_t;
struct File_s; typedef struct File_s File_t;
struct Value_s; typedef struct Value_s Value_t;
struct Library_s; typedef struct Library_s Library_t;
struct Xml_s; typedef struct Xml_s Xml_t;

struct Library_s			/* library list			*/
{
	Library_t*	next;		/* next in list			*/
	char		name[1];	/* library name			*/
};

struct Value_s				/* value in current record	*/
{
	uintmax_t	record;		/* record number for value	*/
	size_t		offset;		/* File_t.value offset		*/
	size_t		size;		/* string value size		*/
	int		number;		/* value is a number		*/
	Cxinternal_f	internalf;	/* convert to internal value	*/
};

struct File_s				/* file read state		*/
{
	uintmax_t	record;		/* current record number	*/
	unsigned char*	buf;		/* input buffer 		*/
	unsigned char*	rec;		/* input record position	*/
	unsigned char*	cur;		/* input buffer position	*/
	unsigned char*	end;		/* input buffer end		*/
	char*		name;		/* current .'d name		*/
	char*		root;		/* root path			*/
	char*		value;		/* current record tag values	*/
	int		image;		/* keep current record image	*/
	int		level;		/* part[] index			*/
	int		maxlevel;	/* max part[] index		*/
	int		maxname;	/* max .'d name length		*/
	int		prefix;		/* implied .'d prefix		*/
	int		save;		/* real char at *f->end		*/
	size_t		maxvalue;	/* size of value		*/
	unsigned char*	prv;		/* previous buffer chunk	*/
	size_t		prvsize;	/* max previous buffer size	*/
	size_t		prvlen;		/* current previous buffer size	*/
	char*		part[1];	/* .'d part stack		*/
};

struct Field_s				/* current proto schema field	*/
{
	Field_t*	next;		/* next in list			*/
	char*		name;		/* qualified field name		*/
	char*		type;		/* field type name		*/
	Cxformat_t	format;		/* field output format		*/
};

struct Xml_s				/* Dssmeth_t.data		*/
{
	Dsstagdisc_t	dsstagdisc;
	Dssmeth_t	meth;
	Dssmeth_t*	basemeth;
	Library_t*	libraries;
	Library_t*	lastlibrary;
	Field_t*	fields;
	Field_t*	lastfield;
	Cxflags_t	test;
	char*		root;
	int		image;
	int		maxname;
	int		maxlevel;
	int		prefix;
};

static const char	null[1];

static char		xml_beg_tag[UCHAR_MAX+1];
static char		xml_end_tag[UCHAR_MAX+1];
static char		xml_end_att[UCHAR_MAX+1];

static char		json_beg_tag[UCHAR_MAX+1];
static char		json_end_val[UCHAR_MAX+1];

extern Dsslib_t		dss_lib_xml;

/*
 * xml var create/lookup
 * type==0 for prefix components
 */

static Cxvariable_t*
xmlvar(Cx_t* cx, char* name, const char* type, Cxdisc_t* disc)
{
	Xml_t*		xml = (Xml_t*)DSS(cx)->meth->data;
	Cxvariable_t*	var;
	Value_t*	val;
	char*		s;
	int		n;
	int		i;

	if (*name == '.')
		var = cxvariable(cx, name, NiL, disc);
	else if (!(var = dtmatch(cx->variables, name)))
	{
		n = strlen(name) + 1;
		if (!(var = vmnewof(cx->vm, 0, Cxvariable_t, 1, sizeof(Value_t) + n)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		var->data = val = (Value_t*)(var + 1);
		strcpy((char*)(var->name = (const char*)(var + 1) + sizeof(Value_t)), name);
		var->type = (Cxtype_t*)(type ? type : "number");
		if (cxaddvariable(cx, var, disc))
			return 0;
		if ((val->number = cxisnumber(var->type)) && !(val->internalf = var->type->internalf) && var->type->base)
			val->internalf = var->type->base->internalf;
		if (type)
		{
			if (xml->maxname < ++n)
				xml->maxname = n;
			n = 0;
			for (s = name; *s; s++)
				if (*s == '.')
				{
					*s = 0;
					i = !xmlvar(cx, name, NiL, disc);
					*s = '.';
					if (i)
						return 0;
					n++;
				}
			if (xml->maxlevel < n)
				xml->maxlevel = n;
			if (n && !xml->root && (s = strchr(var->name, '.')))
			{
				if (xml->root = vmnewof(cx->vm, 0, char, s - var->name, 1))
					memcpy(xml->root, var->name, s - var->name);
				else
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
					return 0;
				}
			}
		}
	}
	return var;
}

/*
 * xml identf
 */

static int
xmlident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	static const char	magic[] = "<?xml";

	return (n > (sizeof(magic) - 1) && !memcmp(buf, magic, sizeof(magic) - 1));
}

/*
 * refill the input buffer and return the next char, -1 on error
 */

static int
refill(Dssfile_t* file, register File_t* f, int c, Dssdisc_t* disc)
{
	size_t	n;

	if (f->cur >= f->end)
	{
		if (f->rec)
		{
			if ((n = f->cur - f->rec + f->prvlen) > f->prvsize)
			{
				f->prvsize = roundof(f->prvsize + n, 1024);
				if (!(f->prv = vmnewof(file->vm, f->prv, unsigned char, f->prvsize, 0)))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
					return -1;
				}
			}
			if (n > 1)
				memcpy(f->prv + f->prvlen, f->rec, n - 1);
			f->prv[n-1] = f->save;
			f->prvlen += n;
		}
		if (!(f->buf = (unsigned char*)sfreserve(file->io, SF_UNBOUND, 0)))
			return -1;
		if (f->rec)
			f->rec = f->buf;
		f->cur = f->buf;
		c = f->save;
		f->end = f->buf + sfvalue(file->io) - 1;
		f->save = *f->end;
		*f->end = 0;
	}
	return c;
}

#define REFILL(f,c,r)	do { if ((c = refill(file, f, c, disc)) < 0) r; } while (0)

#define RESIZE() \
	do \
	{ \
		o = vp - f->value; \
		f->maxvalue += 1024; \
		if (!(f->value = vmnewof(file->vm, f->value, char, f->maxvalue, 0))) \
		{ \
			if (disc->errorf) \
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space"); \
			return -1; \
		} \
		vb = f->value; \
		vp = vb + o; \
		ve = vb + f->maxvalue - 1; \
	} while (0)

/*
 * xml readf -- consume 1 xml record and retain field values of interest
 */

static int
xmlread(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register File_t*	f = file->data;
	register char*		np;
	register char*		ne;
	register char*		vp;
	register char*		ve;
	register int		c;
	char*			vb;
	Cxvariable_t*		v;
	ssize_t			o;
	int			q;

	f->record++;
	f->rec = 0;
	f->prvlen = 0;
	vp = vb = f->value;
	ve = f->value + f->maxvalue - 1;
	ne = f->name + f->maxname;
	np = f->level >= f->maxlevel ? ne : f->level ? f->part[f->level] : f->name;
	for (;;)
	{
		/* find the next tag */

		for (;;)
		{
			while (!xml_beg_tag[*f->cur++]);
			if (*(f->cur - 1))
				break;
			REFILL(f, c, goto done);
			if (c == '<')
				break;
		}
	tag:
		if (!(c = *f->cur++))
			REFILL(f, c, -1);
		switch (c)
		{
		case '/':
			if (f->level)
				f->level--;
			if (f->level <= f->maxlevel)
				np = f->part[f->level];
			/*FALLTHROUGH*/
		case '?':
			for (;;)
			{
				while (!xml_end_tag[*f->cur++]);
				if (*(f->cur - 1))
					break;
				REFILL(f, o, goto incomplete);
				if (o == '>')
					break;
			}
			if (c == '/' && f->level == f->prefix)
			{
				record->data = f;
				file->count = f->record;
				return 1;
			}
			break;
		default:
			if (np <= ne && f->level > f->prefix)
			{
				*np = 0;
				if (v = dtmatch(file->dss->cx->variables, f->part[f->prefix] + 1))
				{
					if (f->image && !f->rec)
						f->rec = f->cur - 1;
					((Value_t*)v->data)->record = f->record;
					((Value_t*)v->data)->offset = vp - vb;
					((Value_t*)v->data)->size = 1;
					if (vp >= ve)
						RESIZE();
					*vp++ = '1';
					if (vp >= ve)
						RESIZE();
					*vp++ = 0;
				}
			}
			if (f->level <= f->maxlevel)
				f->part[f->level] = np;
			else
				np = ne + 1;
			f->level++;
			if (np < ne)
				*np++ = '.';
			if (np < ne)
				*np++ = c;
			q = 0;
			for (;;)
			{
				while (!xml_end_tag[c = *f->cur++])
					if (np < ne)
						*np++ = c;
					else
						q = c;
				if (c)
					break;
				REFILL(f, c, goto incomplete);
				if (c == '>')
					break;
				if (np < ne)
					*np++ = c;
				else
					q = c;
			}
			if (!q && *(np - 1) == '/' || q == '/')
			{
				/* null tag */

				if (f->level)
					f->level--;
				if (f->level <= f->maxlevel)
					np = f->part[f->level];
			}
			else
			{
				/* ignore tag name=value attributes -- why did they allow them */

				if (c == ' ')
				{
					q = 0;
					for (;;)
					{
						while (!xml_end_att[c = *f->cur++]);
						if (!c)
							REFILL(f, c, goto incomplete);
						if (c == '"')
							q = !q;
						else if (!q && c == '>')
							break;
					}
				}
				if (np < ne && f->level > f->prefix)
				{
					*np = 0;
					if (v = dtmatch(file->dss->cx->variables, f->part[f->prefix] + 1))
					{
						if (f->image && !f->rec)
							f->rec = f->cur - 1;
						((Value_t*)v->data)->record = f->record;
						((Value_t*)v->data)->offset = vp - vb;
						for (;;)
						{
							while (!xml_beg_tag[c = *f->cur++])
							{
								if (vp >= ve)
									RESIZE();
								*vp++ = c;
							}
							if (*(f->cur - 1))
								break;
							REFILL(f, c, goto incomplete);
							if (c == '<')
								break;
							if (vp >= ve)
								RESIZE();
							*vp++ = c;
						}
						((Value_t*)v->data)->size = vp - (vb + ((Value_t*)v->data)->offset);
						if (vp >= ve)
							RESIZE();
						*vp++ = 0;
						goto tag;
					}
				}
			}
			break;
		}
	}
 done:
	if (f->level <= f->prefix)
		return 0;
 incomplete:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: record %I*u incomplete", file->path, sizeof(f->record), f->record);
	return -1;
}

/*
 * xml writef -- output current record
 */

static int
xmlwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register File_t*	r = (File_t*)record->data;
	size_t			n;

	sfprintf(file->io, "<%s", r->root);
	if (r->prvlen && sfwrite(file->io, r->prv, r->prvlen) != r->prvlen)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: write error", file->path);
		return -1;
	}
	if ((n = r->cur - r->rec) && sfwrite(file->io, r->rec, n) != n)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: write error", file->path);
		return -1;
	}
	sfputc(file->io, '\n');
	return 0;
}

/*
 * xml fopenf
 */

static int
xmlfopen(Dssfile_t* file, Dssdisc_t* disc)
{
	register Xml_t*		xml = (Xml_t*)file->dss->meth->data;
	register unsigned char*	s;
	register unsigned char*	t;
	register int		n;
	File_t*			f;
	int			c;
	int			m;
	int			x;
	unsigned char*		buf;
	unsigned char*		end;

	if (file->flags & DSS_FILE_WRITE)
		buf = 0;
	else if (buf = (unsigned char*)sfreserve(file->io, SF_UNBOUND, 0))
	{
		end = buf + sfvalue(file->io) - 1;
		if (xml->prefix < 0)
		{
			xml->image = !!(file->dss->flags & DSS_WRITE);
			if (file->format->readf == xmlread)
			{
				if (!xml->root)
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s variable names must be qualified by at least the immediate containing tag", file->format->name);
					return -1;
				}
				x = *end;
				*end = 0;
				s = buf;
				n = 0;
				m = -1;
				for (;;)
				{
					while (!xml_beg_tag[*s++]);
					if (*(s - 1))
					{
						t = s;
						while (!xml_end_tag[*s++]);
						if (*t == '/')
						{
							if (m > 0)
							{
								m--;
								n -= s - t - 2;
							}
						}
						else if (*t != '?')
						{
							m++;
							n += (c = s - t - 1);
							if (!memcmp(xml->root, t, c) && !*(xml->root + c))
								break;
						}
					}
					else if (s >= end)
						break;
				}
				*end = x;
				xml->prefix = m;
				xml->maxlevel += m;
				xml->maxname += n;
			}
			else
			{
				xml->prefix = 0;
				xml->maxname += 1024; /*XXX*/
			}
		}
	}
	if (!(f = vmnewof(file->vm, 0, File_t, 1, (xml->maxlevel + 1) * sizeof(char*) + xml->maxname + 1)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	file->data = f;
	f->name = (char*)(f + 1) + (xml->maxlevel + 1) * sizeof(char*);
	if (!(file->flags & DSS_FILE_WRITE))
	{
		if (buf)
		{
			f->cur = f->buf = buf;
			f->end = end;
			f->save = *end;
			*end = 0;
		}
		else
			f->buf = f->cur = f->end = (unsigned char*)null;
		f->image = xml->image;
		f->prefix = xml->prefix;
		f->maxlevel = xml->maxlevel;
		f->maxname = xml->maxname;
		f->maxvalue = 1024;
		f->root = xml->root;
		if (!(f->value = vmnewof(file->vm, 0, char, f->maxvalue, 0)))
		{
			vmfree(file->vm, f);
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
	}
	return 0;
}

/*
 * xml fclosef
 */

static int
xmlfclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file || !file->data)
		return -1;
	return 0;
}

static Dssformat_t xml_format =
{
	"xml",
	"xml format (2010-05-19)",
	CXH,
	xmlident,
	xmlfopen,
	xmlread,
	xmlwrite,
	0,
	xmlfclose,
	0,
	0,
	0
};

/*
 * json identf
 */

static int
jsonident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register char*		s;
	register char*		e;
	register const char*	m;

	static const char	magic[] = "{\"";

	s = buf;
	e = s + n;
	for (m = magic; s < e; s++)
		if (isspace(*s))
			;
		else if (*s != *m)
			return 0;
		else if (!*++m)
			return 1;
	return 0;
}

/*
 * json readf
 */

static int
jsonread(register Dssfile_t* file, register Dssrecord_t* record, Dssdisc_t* disc)
{
	register File_t*	f = file->data;
	register char*		np;
	register char*		ne;
	register char*		vp;
	register char*		ve;
	register int		c;
	char*			vb;
	Cxvariable_t*		v;
	size_t			o;
	int			a;
	int			e;
	int			q;
	int			n;

	f->record++;
	error(-1, "AHA jsonread file=%p record.file=%p", file, record->file);
	f->rec = 0;
	f->prvlen = 0;
	vp = vb = f->value;
	ve = f->value + f->maxvalue - 1;
	ne = f->name + f->maxname;
	np = f->level > f->maxlevel ? ne : f->level ? f->part[f->level] : f->name;
	for (;;)
	{
	beg:
		do
		{
			while (!json_beg_tag[c = *f->cur++]);
			if (!c)
				REFILL(f, c, goto done);
			if (c == '}')
			{
				if (!f->level)
				{
					record->data = f;
					file->count = f->record;
					return 1;
				}
				if (--f->level <= f->maxlevel)
					np = f->part[f->level];
			}
		} while (c != '{' && c != ',');
		if (f->image && !f->rec)
			f->rec = f->cur - 1;
	tag:
		do
		{
			while (json_end_val[c = *f->cur++] == 1);
			if (!c)
				REFILL(f, c, goto incomplete);
		} while (json_end_val[c] == 1);
		if (f->level <= f->maxlevel)
			f->part[f->level] = np;
		else
			np = ne + 1;
		f->level++;
		if (np < ne)
			*np++ = '.';
		if (!(q = c == '"') && np < ne)
			*np++ = c;
		for (;;)
		{
			while (!json_end_val[c = *f->cur++])
				if (np < ne)
					*np++ = c;
			if (!c)
				REFILL(f, c, goto incomplete);
			if (c == '"')
			{
				q = !q;
				continue;
			}
			else if (c == '\\')
			{
				if (!(c = *f->cur++))
					REFILL(f, c, goto incomplete);
			}
			else if (!q)
			{
				if (c == '}')
				{
					if (!f->level)
					{
						record->data = f;
						file->count = f->record;
						return 1;
					}
					if (--f->level <= f->maxlevel)
						np = f->part[f->level];
					break;
				}
				else if (c == ':')
				{
					do
					{
						while (json_end_val[c = *f->cur++] == 1);
						if (!c)
							REFILL(f, c, goto incomplete);
					} while (json_end_val[c] == 1);
					if (c == '{')
					{
						if (np <= ne)
						{
							*np = 0;
							if (v = dtmatch(file->dss->cx->variables, f->part[f->prefix] + 1))
							{
								((Value_t*)v->data)->record = f->record;
								((Value_t*)v->data)->offset = vp - vb;
								if (vp >= ve)
									RESIZE();
								*vp++ = '1';
								((Value_t*)v->data)->size = 1;
								if (vp >= ve)
									RESIZE();
								*vp++ = 0;
							}
						}
						goto tag;
					}
					a = 0;
					q = 0;
					if (np < ne && f->level > f->prefix)
					{
						*np = 0;
						if (v = dtmatch(file->dss->cx->variables, f->part[f->prefix] + 1))
						{
							((Value_t*)v->data)->record = f->record;
							((Value_t*)v->data)->offset = vp - vb;
							e = c == 'n';
							for (;;)
							{
								if (c == '"')
								{
									q = !q;
									goto ignore;
								}
								else if (c == '\\')
								{
									if (!(c = *f->cur++))
										REFILL(f, c, goto incomplete);
									if (c != '\\' && c != '"' && c != ',' && c != '}')
									{
										if (vp >= ve)
											RESIZE();
										*vp++ = '\\';
									}
								}
								else if (!q)
								{
									if (c == '[')
									{
										a++;
										goto ignore;
									}
									else if (c == ']')
									{
										if (a)
											a--;
										goto ignore;
									}
									else if (json_end_val[c] == 1)
										goto ignore;
									else if (a)
										/*array*/;
									else if (c == '}')
									{
										if (!f->level)
										{
											record->data = f;
											file->count = f->record;
											return 1;
										}
										if (--f->level <= f->maxlevel)
											np = f->part[f->level];
										break;
									}
									else if (c == ',')
										break;
								}
								if (vp >= ve)
									RESIZE();
								*vp++ = c;
							ignore:
								while (!json_end_val[c = *f->cur++])
								{
									if (vp >= ve)
										RESIZE();
									*vp++ = c;
								}
								if (!c)
									REFILL(f, c, goto incomplete);
							}
							if (e)
								vp = vb + ((Value_t*)v->data)->offset;
							((Value_t*)v->data)->size = vp - (vb + ((Value_t*)v->data)->offset);
							*vp++ = 0;
							if (!f->level)
							{
								record->data = f;
								file->count = f->record;
								return 1;
							}
							if (--f->level <= f->maxlevel)
								np = f->part[f->level];
							if (c == ',')
								goto tag;
							goto beg;
						}
					}
					n = 1;
					for (;;)
					{
						if (c == '"')
							q = !q;
						else if (c == '\\')
						{
							if (!(c = *f->cur++))
								REFILL(f, c, goto incomplete);
						}
						else if (!q)
						{
							if (c == '[')
								a++;
							else if (c == ']')
							{
								if (a)
									a--;
							}
							else if (a)
								/*array*/;
							else if (c == '{')
								n++;
							else if (c == '}' && !--n)
							{
								if (!f->level)
								{
									record->data = f;
									file->count = f->record;
									return 1;
								}
								if (--f->level <= f->maxlevel)
									np = f->part[f->level];
								break;
							}
							else if (c == ',' && n == 1)
								break;
						}
						while (!json_end_val[c = *f->cur++]);
						if (!c)
							REFILL(f, c, goto done);
					}
					if (!f->level)
					{
						record->data = f;
						file->count = f->record;
						return 1;
					}
					if (--f->level <= f->maxlevel)
						np = f->part[f->level];
					if (c == ',')
						goto tag;
					goto beg;
				}
				else if (json_end_val[c] == 1)
					continue;
			}
			if (np < ne)
				*np++ = c;
		}
	}
 done:
	if (!f->level)
		return 0;
 incomplete:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: record %I*u incomplete", file->path, sizeof(f->record), f->record);
	return -1;
}

/*
 * xml writef -- output current record
 */

static int
jsonwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register File_t*	r = (File_t*)record->data;
	size_t			n;

	if (r->prvlen && sfwrite(file->io, r->prv, r->prvlen) != r->prvlen)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: write error", file->path);
		return -1;
	}
	if ((n = r->cur - r->rec) && sfwrite(file->io, r->rec, n) != n)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: write error", file->path);
		return -1;
	}
	sfputc(file->io, '\n');
	return 0;
}

static Dssformat_t json_format =
{
	"json",
	"json format (2010-05-19)",
	CXH,
	jsonident,
	xmlfopen,
	jsonread,
	jsonwrite,
	0,
	xmlfclose,
	0,
	0,
	0
};

static int
op_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	File_t*		f = (File_t*)DSSDATA(data);
	Value_t*	v = (Value_t*)pc->data.variable->data;
	char*		s;

	if (v)
	{
		if (v->record == f->record)
			s = f->value + v->offset;
		else
		{
			s = (char*)null;
			v->size = 0;
		}
		if (!v->internalf)
		{
			r->value.string.data = s;
			r->value.string.size = v->size;
		}
		else if ((*v->internalf)(cx, pc->data.variable->type, NiL, &pc->data.variable->format, r, s, v->size, cx->rm, disc) < 0)
			return -1;
	}
	return 0;
}

static int
op_ref(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	return (r->value.variable = xmlvar(cx, b->value.string.data, "string", disc)) ? 0 : -1;
}

static Cxcallout_t local_callouts[] =
{
CXC(CX_GET, "void", "void", op_get, 0)
CXC(CX_REF, "string", "void", op_ref, 0)
};

static int
xml_field_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Xml_t*		xml = (Xml_t*)disc;

	if (!(xml->lastfield->name = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
xml_field_type_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Xml_t*		xml = (Xml_t*)disc;
	char*			s;

	memset(&xml->lastfield->format, 0, sizeof(xml->lastfield->format));
	(void)cxattr(NiL, data, &s, &xml->lastfield->format, NiL);
	if (!*s)
		s = "number";
	if (!(xml->lastfield->type = strdup(s)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (xml->lastfield->format.flags & CX_FLOAT)
		xml->lastfield->format.flags &= ~(CX_STRING|CX_BUFFER|CX_UNSIGNED|CX_INTEGER);
	else if (xml->lastfield->format.flags & CX_UNSIGNED)
	{
		xml->lastfield->format.flags &= ~(CX_STRING|CX_BUFFER);
		xml->lastfield->format.flags |= CX_UNSIGNED|CX_INTEGER;
	}
	else if (!(xml->lastfield->format.flags & (CX_STRING|CX_BUFFER|CX_INTEGER)))
	{
		if (streq(s, "string"))
			xml->lastfield->format.flags |= CX_STRING;
		else if (streq(s, "buffer"))
			xml->lastfield->format.flags |= CX_BUFFER;
	}
	return 0;
}

static Tags_t	tags_xml_field[] =
{
	"NAME",		"Field name.",
			0,0,xml_field_name_dat,0,
	"TYPE",		"Field type. The intrinsic types are number and"
			" string. Other types are defined in optional"
			" method and schema libraries.",
			0,0,xml_field_type_dat,0,
	0
};

static int
xml_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Xml_t*	xml = (Xml_t*)disc;

	if (!(xml->meth.name = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
xml_description_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Xml_t*	xml = (Xml_t*)disc;

	if (!(xml->meth.description = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
xml_library_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Xml_t*		xml = (Xml_t*)disc;
	register Library_t*	p;

	if (!(p = newof(0, Library_t, 1, strlen(data))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	strcpy(p->name, data);
	if (!xml->lastlibrary)
		xml->libraries = xml->lastlibrary = p;
	else
		xml->lastlibrary = xml->lastlibrary->next = p;
	return 0;
}

static Tags_t*
xml_field_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	register Xml_t*		xml = (Xml_t*)disc;
	Field_t*		f;

	if (name)
	{
		if (!(f = newof(0, Field_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		if (!xml->lastfield)
			xml->fields = f;
		else
			xml->lastfield->next = f;
		xml->lastfield = f;
	}
	return &tags_xml_field[0];
}

static int
xml_field_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	register Xml_t*		xml = (Xml_t*)disc;

	if (xml->lastfield && (!xml->lastfield->name || !xml->lastfield->type))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "field name and type must be specified");
		return -1;
	}
	return 0;
}

static Tags_t	tags_xml[] =
{
	"NAME",		"Schema name.",
			0,0,xml_name_dat,0,
	"DESCRIPTION",	"Schema description.",
			0,0,xml_description_dat,0,
	"LIBRARY",	"Required type/map library name;"
			" more than one library may be specified.",
			0,0,xml_library_dat,0,
	"FIELD",	"Field info.",
			0,xml_field_beg,0,xml_field_end,
	0
};

static Tags_t*
xml_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	return &tags_xml[0];
}

static Tags_t	tags[] =
{
	"METHOD",	"Method name; must be xml.",
			0,0,0,0,
	"XML",		"xml method schema.",
			0,xml_beg,0,0,
	0
};

/*
 * methf
 */

static Dssmeth_t*
xmlmeth(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	register Xml_t*		xml;
	Tag_t*			tag;
	Sfio_t*			sp;
	Library_t*		p;
	char*			s;
	char			path[PATH_MAX];

	if (!(xml = newof(0, Xml_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	xml->basemeth = meth;
	xml->meth = *meth;
	meth = &xml->meth;
	meth->data = xml;
	taginit(&xml->dsstagdisc.tagdisc, disc->errorf);
	xml->dsstagdisc.tagdisc.id = DSS_ID;
	xml->dsstagdisc.disc = disc;
	xml->dsstagdisc.meth = meth;
	sp = 0;
	if (options)
	{
		if (!(sp = sfstropen()))
			goto drop;
		sfprintf(sp, "%s", usage);
		if (tagusage(tags, sp, &xml->dsstagdisc.tagdisc))
			goto drop;
		sfprintf(sp, "}\n");
		if (dssoptlib(meth->cx->buf, &dss_lib_xml, sfstruse(sp), disc))
			goto drop;
		sfclose(sp);
		sp = 0;
		s = sfstruse(meth->cx->buf);
		for (;;)
		{
			switch (optstr(options, s))
			{
			case 'T':
				xml->test = opt_info.num;
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
		if (!(tag = tagopen(sp, path, 1, &tags[0], &xml->dsstagdisc.tagdisc)) || tagclose(tag))
			goto drop;
		sfclose(sp);
		sp = 0;
	}
	dtinsert(meth->formats, &xml_format);
	dtinsert(meth->formats, &json_format);
	for (p = xml->libraries; p; p = p->next)
		if (!dssload(p->name, disc))
			return 0;
	return meth;
 drop:
	free(xml);
	if (sp)
		sfclose(sp);
	return 0;
}

/*
 * openf
 */

static int
xmlopen(Dss_t* dss, Dssdisc_t* disc)
{
	Xml_t*		xml = (Xml_t*)dss->meth->data;
	Field_t*	f;
	Field_t*	g;
	Cxvariable_t*	v;
	int		i;

	if (xml)
	{
		dss->cx->ctype['.'] |= CX_CTYPE_ALPHA;
		for (i = 0; i < elementsof(local_callouts); i++)
			if (cxaddcallout(dss->cx, &local_callouts[i], disc))
				return -1;

		xml_beg_tag[0] = 1;
		xml_beg_tag['<'] = 1;

		xml_end_tag[0] = 1;
		xml_end_tag['>'] = 1;
		xml_end_tag[' '] = 1;

		xml_end_att[0] = 1;
		xml_end_att['"'] = 1;
		xml_end_att['>'] = 1;

		json_beg_tag[0] = 2;
		json_beg_tag[','] = 2;
		json_beg_tag['{'] = 2;
		json_beg_tag['}'] = 2;

		json_end_val[0] = 2;
		json_end_val['\\'] = 2;
		json_end_val['"'] = 2;
		json_end_val[':'] = 2;
		json_end_val[','] = 2;
		json_end_val['{'] = 2;
		json_end_val['}'] = 2;
		json_end_val['['] = 2;
		json_end_val[']'] = 2;
		json_end_val[' '] = 1;
		json_end_val['\n'] = 1;
		json_end_val['\r'] = 1;
		json_end_val['\t'] = 1;
		json_end_val['\v'] = 1;

		xml->prefix = -1;
		for (f = xml->fields; f; f = g)
		{
			g = f->next;
			if (!(v = xmlvar(dss->cx, f->name, f->type, disc)))
				return -1;
			v->format = f->format;
			free(f->name);
			free(f->type);
			free(f);
		}
	}
	return 0;
}

static Dssmeth_t method =
{
	"xml",
	"xml and json method",
	CXH,
	xmlmeth,
	xmlopen,
	0,
	0
};

Dsslib_t dss_lib_xml =
{
	"xml",
	"xml method"
	"[-1ls5Pp0?\n@(#)$Id: dss xml method (AT&T Research) 2010-04-22 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	&method,
};
