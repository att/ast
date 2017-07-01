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
 * text method
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[+DESCRIPTION?The \bdss\b text method describes newline-terminated"
"	field-delimited text file data. The method schema is a \bscanf\b(3)"
"	style format string with embedded field names of the form"
"	\b%(\b\afield\a\b)\b\aformat-char\a \adelimiter\a ...]"
"[+?Use the \bdss\b \bflat\b method for generic record-oriented flat "
    "file data.]"
"[+EXAMPLES]{"
"	[+dss -x text::::\b\"%(name)s::%(passwd::Encrypted\\ password.)s::%(uid)d::%(gid)d::%(comment)s::%(home)s::%(shell)s\"\\ 'passwd==\"\"&&uid==0'\\ "
"	/etc/passwd?Prints \b/etc/passwd\b entries with uid==0 and no"
"	password.]"
"}"
"\n"
"\n--method=text[,option...]\n"
"\n"
;

#include <dsslib.h>
#include <tm.h>

typedef uint32_t Ipaddr_t;

struct Text_s; typedef struct Text_s Text_t;

struct Text_s				/* Dssmeth_t.data		*/
{
	char*		format;
	Dt_t*		dict;
	int		vars;
	char		name[1];
};

static char		null[1];

extern Dsslib_t		dss_lib_text;

/*
 * text identf
 */

static int
textident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	return 1;
}

/*
 * text fopenf
 */

static int
textfopen(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!(file->data = vmnewof(file->dss->vm, 0, Cxvalue_t, ((Text_t*)file->dss->meth->data)->vars, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

/*
 * text fclosef
 */

static int
textfclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file || !file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

/*
 * get one string token into p
 */

static char*
lextok(register char* s, register int c, Cxstring_t* p)
{
	register char*	t;
	register int	q;
	char*		b;
	char*		u;

	b = s;
	q = 0;
	t = 0;
	for (;;)
	{
		if (!*s)
		{
			if (!q)
			{
				if (c && c != ' ')
				{
					s = b;
					b = null;
					break;
				}
			}
			if (t)
				*t = 0;
			break;
		}
		else if (*s == '\\')
		{
			u = s;
			if (!*++s)
				continue;
			if (b == u)
				b = s;
			else if (!t)
				t = u;
		}
		else if (q)
		{
			if (*s == q)
			{
				q = 0;
				if (!t)
					t = s;
				s++;
				continue;
			}
			else if (*s == '\r')
				*s = 0;
		}
		else if (*s == '"' || *s == '\'')
		{
			q = *s++;
			if (b == (s - 1))
				b = s;
			else if (!t)
				t = s - 1;
			continue;
		}
		else if (*s == c || c == ' ' && *s == '\t')
		{
			*s++ = 0;
			if (t)
				*t = 0;
			if (c == ' ')
				while (*s == ' ' || *s == '\t')
					s++;
			break;
		}
		if (t)
			*t++ = *s;
		s++;
	}
	p->data = b;
	p->size = *b ? (s - b) : 0;
	return s;
}

/*
 * text readf
 */

static int
textread(register Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Cxvalue_t*	data = (Cxvalue_t*)file->data;
	register Text_t*	text = (Text_t*)file->dss->meth->data;
	register char*		s;
	register char*		f;
	register int		c;
	char*			t;
	int			num;
	int			q;
	Ipaddr_t		a;

	if (!(s = sfgetr(file->io, '\n', 1)))
		return 0;
	num = 0;
	f = text->format;
	for (;;)
	{
		switch (c = *f++)
		{
		case 0:
			break;
		case ' ':
			while (*s == ' ' || *s == '\t')
				s++;
			break;
		case '%':
			switch (c = *f++)
			{
			case 'h':
			case 'l':
				q = c;
				c = *f++;
				break;
			default:
				q = 0;
				break;
			}
			switch (c)
			{
			case 0:
				f--;
				continue;
			case '%':
				if (*s++ != c)
					s = null;
				continue;
			case 'c':
				if (data[num].number = *s)
					s++;
				num++;
				break;
			case 'd':
				c = 10;
				goto number;
			case 'i':
				if (!*s)
					data[num].number = 0;
				else
				{
					strtoip4(s, &t, &a, NiL);
					data[num].number = a;
					s = t;
				}
				num++;
				break;
			case 'n':
			case 'u':
				c = 0;
				goto number;
			case 'o':
				c = 8;
				goto number;
			case 'x':
				c = 16;
			number:
				if (!*s)
					data[num].number = 0;
				else
				{
					data[num].number = strtol(s, &t, c);
					s = t;
				}
				num++;
				break;
			case 'f':
			case 'g':
				if (!*s)
					data[num].number = 0;
				else
				{
					data[num].number = strtod(s, &t);
					s = t;
				}
				num++;
				break;
			case 's':
				if (q = *f)
					f++;
				if (!*s)
				{
					data[num].string.data = null;
					data[num].string.size = 0;
				}
				else
					s = lextok(s, q, &data[num].string);
				num++;
				break;
			case 't':
				if (!*s)
					data[num].number = 0;
				else
				{
					data[num].number = tmdate(s, &t, NiL);
					if (*t && *t != *f && *t != '\n')
						data[num].number = strtol(s, &t, 0);
					s = t;
				}
				num++;
				break;
			}
			continue;
		case '\n':
			break;
		default:
			if (*s++ != c)
				s = null;
			continue;
		}
		break;
	}
	record->data = data;
	return 1;
}

/*
 * text writef
 */

static int
textwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Text_t*	text = (Text_t*)file->dss->meth->data;
	Cxvalue_t*		data = (Cxvalue_t*)record->data;
	register char*		f;
	register int		c;
	int			num;

	num = 0;
	f = text->format;
	for (;;)
	{
		switch (c = *f++)
		{
		case 0:
			break;
		case ' ':
			sfputc(file->io, ' ');
			break;
		case '%':
			switch (c = *f++)
			{
			case 'h':
			case 'l':
				c = *f++;
				break;
			}
			switch (c)
			{
			case 0:
				f--;
				continue;
			case '%':
				sfputc(file->io, '%');
				continue;
			case 'c':
				sfputc(file->io, (int)data[num].number);
				num++;
				break;
			case 'd':
			case 'n':
			case 'u':
				sfprintf(file->io, "%d", (long)data[num].number);
				num++;
				break;
			case 'i':
				sfprintf(file->io, "%s", fmtip4((Ipaddr_t)data[num].number, -1));
				num++;
				break;
			case 'f':
			case 'g':
				sfprintf(file->io, "%Lg", data[num].number);
				num++;
				break;
			case 'o':
				sfprintf(file->io, "%o", (long)data[num].number);
				num++;
				break;
			case 'x':
				sfprintf(file->io, "%x", (long)data[num].number);
				num++;
				break;
			case 's':
				sfprintf(file->io, "%-.*s", data[num].string.size, data[num].string.data);
				num++;
				break;
			case 't':
				sfprintf(file->io, "%s", fmttime("%K", (time_t)data[num].number));
				num++;
				break;
			}
			continue;
		case '\n':
			break;
		default:
			sfputc(file->io, c);
			continue;
		}
		break;
	}
	sfputc(file->io, '\n');
	return 0;
}

static Dssformat_t text_format =
{
	"text",
	"text format (2010-05-28)",
	CXH,
	textident,
	textfopen,
	textread,
	textwrite,
	0,
	textfclose,
	0,
	0,
	0
};

static int
op_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	r->value = ((Cxvalue_t*)DSSDATA(data))[((Cxvariable_t*)pc->data.variable)->index];
	return 0;
}

static Cxcallout_t local_callouts[] =
{
CXC(CX_GET, "void", "void", op_get, 0)
};

/*
 * methf
 */

static Dssmeth_t*
textmeth(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* ometh)
{
	register Text_t*	text;
	register Dssmeth_t*	meth;
	register Cxvariable_t*	var;
	register char*		s;
	register char*		t;
	register char*		f;
	register int		c;
	char*			d;
	int			p;
	int			index;

	if (options)
	{
		if (dssoptlib(ometh->cx->buf, &dss_lib_text, usage, disc))
			goto drop;
		s = sfstruse(ometh->cx->buf);
		for (;;)
		{
			switch (optstr(options, s))
			{
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
	if (!schema || !*schema)
		return ometh;
	if (!(meth = newof(0, Dssmeth_t, 1, sizeof(Text_t) + strlen(name) + 2 * strlen(schema) + 2)))
	{
		free(meth);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "out of space");
		return 0;
	}
	*meth = *ometh;
	meth->data = text = (Text_t*)(meth + 1);
	text->format = stpcpy(text->name, name) + 1;
	index = 0;
	s = (char*)schema;
	f = text->format;
	for (;;)
	{
		switch (c = *s++)
		{
		case 0:
			break;
		case '%':
			*f++ = '%';
			var = 0;
			switch (c = *s++)
			{
			case 0:
				goto invalid;
			case 'h': case 'l': case 'L':
			case '+': case '-': case '.': case '_':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				continue;
			case '%':
				*f++ = '%';
				continue;
			case '(':
				t = f;
				d = 0;
				p = 1;
				for (;;)
				{
					switch (c = *s++)
					{
					case 0:
						goto invalid;
					case '(':
						p++;
						*t++ = c;
						continue;
					case ')':
						if (!--p)
							break;
						*t++ = c;
						continue;
					case ':':
						if (d)
							*t++ = c;
						else
						{
							*t++ = 0;
							d = t;
						}
						continue;
					default:
						*t++ = c;
						continue;
					}
					break;
				}
				*t = 0;
				if (dtmatch(meth->cx->variables, f))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s: duplicate field", f);
					goto drop;
				}
				if (!(var = newof(0, Cxvariable_t, 1, t - f + 1)))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "out of space");
					goto drop;
				}
				var->index = index;
				t = stpcpy((char*)(var->name = (char*)(var + 1)), f);
				if (d)
					var->description = strcpy(t + 1, d);
				break;
			}
			for (;;)
			{
				switch (c = *s++)
				{
				case 0:
					goto invalid;
				case 'h': case 'l': case 'L':
				case '+': case '-': case '.': case '_':
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					continue;
				}
				break;
			}
			if (var)
			{
				switch (c)
				{
				case 'd':
				case 'f':
				case 'g':
				case 'n':
				case 'o':
				case 'u':
				case 'x':
					var->type = (Cxtype_t*)"number";
					break;
				case 'i':
					var->type = (Cxtype_t*)"ipaddr_t";
					break;
				case 's':
					var->type = (Cxtype_t*)"string";
					break;
				case 't':
					var->type = (Cxtype_t*)"time_t";
					break;
				default:
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%c: invalid field format >>>%s", c, s - 1);
					goto drop;
				}
				if (cxaddvariable(meth->cx, var, disc))
					goto drop;
			}
			index++;
			*f++ = c;
			continue;
		case ' ':
		case '\t':
		case '\n':
			if (f == text->format || *(f - 1) != ' ')
				*f++ = ' ';
			continue;
		default:
			*f++ = c;
			continue;
		}
		break;
	}
	if (!(text->vars = index))
		goto invalid;
	*f = 0;
	dtinsert(meth->formats, &text_format);
	for (c = 0; c < elementsof(local_callouts); c++)
		if (cxaddcallout(meth->cx, &local_callouts[c], disc))
			return 0;
	return meth;
 invalid:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: invalid schema", schema);
 drop:
	free(meth);
	return 0;
}

/*
 * openf
 */

static int
textopen(Dss_t* dss, Dssdisc_t* disc)
{
	return dss->meth->data ? 0 : -1;
}

static Dssmeth_t	method =
{
	"text",
	"Newline-terminated field-delimited text file; the method schema is"
	" a scanf(3) like format string with embedded field names of the form:"
	" %(field1)format-char delimiter ...",
	CXH,
	textmeth,
	textopen,
	0,
	0
};

Dsslib_t		dss_lib_text =
{
	"text",
	"text method"
	"[-1ls5Pp0?\n@(#)$Id: dss text method (AT&T Research) 2002-12-17 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	&method,
};
