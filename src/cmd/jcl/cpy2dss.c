/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * convert copybook to dss flat schema
 */

static const char usage[] =
"[-1s1I?\n@(#)$Id: cpy2dss (AT&T Research) 2013-03-01 $\n]"
USAGE_LICENSE
"[+NAME?cpy2dss - convert copybook to dss flat schema]"
"[+DESCRIPTION?\bcpy2dss\b converts each copybook \afile\a operand to a "
    "\bdss\b flat schema file on the standard output. If no \afile\a "
    "operands are specified then the standard input is read.]"
"[+?Duplicate structure and field names are disambiguated by appending "
    "\b_\b\acount\a to the names, where \acount\a > 1. \bRENAMES\b and level "
    "88 fields are ignored.]"
"[b:bytemask?Output a single line where each character represents the "
    "type of the corresponding byte in the physical representation of a "
    "copybook record, where \b0\b means \bPIC\b and \an\a means "
    "\bCOMP-\b\an\a.]"
"[c:codeset?Set the \bstring\b codeset name.]:[codeset:=ebcdic-m]"
"[C:comp?Convert COMP-\afrom\a to COMP-\ato\a.]:[from::to]"
"[d:delimiter?Set the \b--text\b field delimiter character. XML "
    "&\aname\a; and #\adecimal\a; forms are accepted.]:[delimiter:=|]"
"[D:terminator?Set the \b--text\b record terminator character. XML "
    "&\aname\a; and #\adecimal\a; forms are "
    "accepted.]:[terminator:=&newline;]"
"[e:escape?Set the \b--text\b field escape character. XML &\aname\a; and "
    "#\adecimal\a; forms are accepted.]:[delimiter:=\boff\b]"
"[k:keep?Keep original names. Otherwise non-identifier characters are "
    "converted to \b_\b.]"
"[l:little-endian|le?Little-endian binary integer encoding.]"
"[o:offsets?Output the name, offset, size, number of elements and type "
    "of each member, one per line, on the standard output. Scalar fields "
    "have 0 elements.]"
"[q:quote?Set the \b--text\b field quote begin character. If "
    "\b--end-quote\b is not specified then it is the same as \b--quote\b. "
    "XML &\aname\a; and #\adecimal\a; forms are accepted.]:[quote:=\"]"
"[Q:end-quote?Set the \b--text\b field quote end character. If "
    "\b--quote\b is not specified then it is the same as \b--end-quote\b. "
    "XML &\aname\a; and #\adecimal\a; forms are accepted.]:[quote:=\"]"
"[r:record|recfmt|reclen?Sets the record format to \aformat\a; newlines "
    "will be treated as normal characters. The formats are:]:[format]"
    "{"
        "[+d[\aterminator\a]]?Variable length with record \aterminator\a "
            "character, \b\\n\b by default.]"
        "[+[f]]\areclen\a?Fixed record length \areclen\a.]"
        "[+v[op...]]?Variable length. \bh4o0z2bi\b (4 byte IBM V format "
            "descriptor) if \aop\a are omitted. \aop\a may be a combination "
            "of:]"
            "{"
                "[+h\an\a?Header size is \an\a bytes (default 4).]"
                "[+o\an\a?Size offset in header is \an\a bytes (default "
                    "0).]"
                "[+z\an\a?Size length is \an\a bytes (default "
                    "min(\bh\b-\bo\b,2)).]"
                "[+b?Size is big-endian (default).]"
                "[+l?Size is little-endian (default \bb\b).]"
                "[+i?Record length includes header (default).]"
                "[+n?Record length does not include header (default "
                    "\bi\b).]"
            "}"
    "}"
"[s!:sized-struct?Check for embedded size/data structures.]"
"[t:text?Generate a field-delimited newline-terminated text schema using "
    "the local codeset.]"
"[T:regress?Massage output for regression testing.]"
"[v:variable?If \b--notext\b is on then records are variable length, "
    "delimited by the \b--terminator\b character. If \b--text\b is on then "
    "the size field \b--sized\b structures is omitted from the output.]"
"\n"
"\n[ file ... ]\n"
"\n"
;

#include <ast.h>
#include <ctype.h>
#include <ccode.h>
#include <dt.h>
#include <ls.h>
#include <recfmt.h>
#include <tm.h>
#include <error.h>

#define CPY_VERSION	20030220L

#define cpyinit(d,e)	(memset(d,0,sizeof(Cpydisc_t)),(d)->version=CPY_VERSION,(d)->errorf=(Error_f)(e))

#define CPY_binary	(1<<0)
#define CPY_number	(1<<1)
#define CPY_pointer	(1<<2)
#define CPY_signed	(1<<3)
#define CPY_string	(1<<4)
#define CPY_sync	(1<<5)

#define CPY_XML		0
#define CPY_BYTEMASK	1
#define CPY_OFFSETS	2

#define CPY_ALIGN(w)	(((w)<=2)?2:4)

struct Cpyfield_s; typedef struct Cpyfield_s Cpyfield_t;

struct Cpyfield_s
{
	Dtlink_t	link;
	Cpyfield_t*	parent;
	Cpyfield_t*	members;
	Cpyfield_t*	next;
	Cpyfield_t*	sized;
	Cpyfield_t*	redefines;
	char*		dimension;
	unsigned long	offset;
	int		comp;
	int		dup;
	int		level;
	int		fixedpoint;
	int		mindimension;
	int		maxdimension;
	int		structure;
	int		width;
	int		total;
	unsigned int	flags;
	char		name[8];
};

typedef struct Cpydisc_s
{
	uint32_t	version;
	Error_f		errorf;
} Cpydisc_t;

typedef struct Cpy_s
{
	const char*	id;
	Cpydisc_t*	disc;
	Cpyfield_t*	first;
	Cpyfield_t*	last;
	Sfio_t*		ip;
	char*		cp;
	char*		ofile;
	int		item;
	int		oline;
	int		size;
	Vmalloc_t*	vm;
	Dt_t*		dt;
} Cpy_t;

static struct State_s
{
	char*		codeset;
	char*		delimiter;
	char*		endian;
	char*		escape;
	char*		quotebegin;
	char*		quoteend;
	char*		terminator;

	Recfmt_t	record;

	size_t		fixed;

	int		keep;
	int		output;
	int		regress;
	int		sized;
	int		text;
	int		variable;

	int		comp[10];
} state;

static char		empty[] = "\n";

Cpy_t*
cpyopen(const char* path, Sfio_t* ip, Cpydisc_t* disc)
{
	register Cpy_t*	cpy;
	Vmalloc_t*	vm;

	static Dtdisc_t	dictdisc;

	dictdisc.key = offsetof(Cpyfield_t, name);
	dictdisc.size = 0;
	dictdisc.link = offsetof(Cpyfield_t, link);
	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)) || !(cpy = vmnewof(vm, 0, Cpy_t, 1, 0)) || !(cpy->dt = dtnew(vm, &dictdisc, Dtset)))
	{
		if (vm)
			vmclose(vm);
		if (disc->errorf)
			(*disc->errorf)(cpy, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	cpy->id = "cpy";
	cpy->vm = vm;
	cpy->disc = disc;
	cpy->ip = ip;
	cpy->cp = empty;
	cpy->ofile = error_info.file;
	error_info.file = (char*)path;
	cpy->oline = error_info.line;
	error_info.line = 0;
	return cpy;
}

int
cpyclose(Cpy_t* cpy)
{
	error_info.file = cpy->ofile;
	error_info.line = cpy->oline;
	vmclose(cpy->vm);
	return 0;
}

char*
cpylex(register Cpy_t* cpy)
{
	register char*	s;
	register int	q;

	if (cpy->item)
	{
		cpy->item = 0;
		cpy->size = 0;
		return "";
	}
	for (;;)
	{
		switch (*cpy->cp)
		{
		case '*':
		case '\n':
			do
			{
				if (!(cpy->cp = sfgetr(cpy->ip, '\n', 0)))
				{
					cpy->cp = empty;
					return 0;
				}
				error_info.line++;
			} while ((q = sfvalue(cpy->ip)) < 7);
			if (q > 72)
				cpy->cp[72] = '\n';
			cpy->cp += 6;
			continue;
		case ' ':
		case '\t':
		case '\r':
			cpy->cp++;
			continue;
		}
		break;
	}
	s = cpy->cp;
	q = 0;
	for (;;)
	{
		switch (*cpy->cp++)
		{
		case '\n':
			*(cpy->cp - 1) = 0;
			cpy->size = cpy->cp - s - 1;
			cpy->cp = empty;
			return s;
		case ' ':
		case '\t':
		case '\r':
			if (!q)
				break;
			continue;
		case '\'':
			q = !q;
			continue;
		case '.':
			if (!q)
			{
				if (cpy->cp > s + 1)
				{
					if (isdigit(*cpy->cp) && (isdigit(*(cpy->cp - 2)) || *(cpy->cp - 2) == ')'))
						continue;
					cpy->item = 1;
				}
				*cpy->cp = '\n';
				break;
			}
			continue;
		case '-':
			if ((cpy->cp - 1) == s && isdigit(*cpy->cp))
				continue;
			/*FALLTHROUGH*/
		case ':':
		case '&':
			if (!q && !state.keep)
				*(cpy->cp - 1) = '_';
			continue;
		default:
			continue;
		}
		break;
	}
	*(cpy->cp - 1) = 0;
	cpy->size = cpy->cp - s - 1;
	return s;
}

Cpyfield_t*
cpyfield(register Cpy_t* cpy)
{
	register char*		s;
	register Cpyfield_t*	f;
	Cpyfield_t*		p;
	size_t			n;
	char*			e;
	int			fixedpoint;
	int			level;
	int			width;

	do
	{
		if (!(s = cpylex(cpy)))
			return 0;
	} while (!strncasecmp(s, "SKIP", 4));
	level = (int)strtol(s, &e, 10);
	if (*e)
	{
		if (cpy->disc->errorf)
			(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: syntax error: level number expected", s);
		return 0;
	}
	if (!(s = cpylex(cpy)) || !*s)
		return 0;
	if (!(f = vmnewof(cpy->vm, 0, Cpyfield_t, 1, cpy->size)))
	{
		if (cpy->disc->errorf)
			(*cpy->disc->errorf)(cpy, cpy->disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (cpy->first)
		cpy->last->next = f;
	else
		cpy->first = f;
	cpy->last = f;
	f->level = level;
	e = s;
	while ((p = (Cpyfield_t*)dtmatch(cpy->dt, e)) && p->dup)
	{
		p->dup++;
		e = sfprints("%s_%d", s, p->dup);
	}
	strcpy(f->name, e);
	if (!p)
		dtinsert(cpy->dt, f);
	f->dup = 1;
	f->structure = 1;
	while ((s = cpylex(cpy)) && *s)
	{
	again:
		if (!strcasecmp(s, "BINARY"))
		{
			f->structure = 0;
			f->flags |= CPY_binary;
		}
		else if (!strncasecmp(s, "COMP", 4))
		{
			f->structure = 0;
			s += !strncasecmp(s, "COMPUTATIONAL", 13) ? 13 : 4;
			if (*s++ != '_')
				f->comp = 1;
			else
			{
				f->comp = (int)strtol(s, &e, 10);
				if (*e || f->comp <= 0 || f->comp > 9)
				{
					if (cpy->disc->errorf)
						(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: syntax error: COMP number in range 1..9 expected", s);
					continue;
				}
			}
			f->comp = state.comp[f->comp];
		}
		else if (!strcasecmp(s, "DEPENDING"))
		{
			if (!(s = cpylex(cpy)) || !*s)
				break;
			if (strcasecmp(s, "ON"))
			{
				if (cpy->disc->errorf)
					(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: syntax error: ON expected", s);
				continue;
			}
			if (!(s = cpylex(cpy)) || !*s)
				break;
			if (!(p = (Cpyfield_t*)dtmatch(cpy->dt, s)))
			{
				if (!(p = vmnewof(cpy->vm, 0, Cpyfield_t, 1, cpy->size)))
				{
					if (cpy->disc->errorf)
						(*cpy->disc->errorf)(cpy, cpy->disc, ERROR_SYSTEM|2, "out of space");
					return 0;
				}
				dtinsert(cpy->dt, p);
			}
			else if (p->dup > 1)
			{
				s = sfprints("%s_%d", p->name, p->dup);
				if (!(p = (Cpyfield_t*)dtmatch(cpy->dt, s)))
				{
					if (cpy->disc->errorf)
						(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: internal error: dup not found", s);
					return 0;
				}
			}
			f->dimension = p->name;
		}
		else if (!strcasecmp(s, "INDEXED"))
		{
			if (!(s = cpylex(cpy)) || !*s)
				break;
			if (strcasecmp(s, "BY"))
			{
				if (cpy->disc->errorf)
					(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: syntax error: BY expected", s);
				continue;
			}
			if (!(s = cpylex(cpy)) || !*s)
				break;
		}
		else if (!strcasecmp(s, "IS"))
			/*ignore*/;
		else if (!strcasecmp(s, "OCCURS"))
		{
			if (!(s = cpylex(cpy)) || !*s)
				break;
			f->mindimension = (int)strtol(s, &e, 10);
			if (*e)
			{
				if (cpy->disc->errorf)
					(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: syntax error: minimum OCCURS number expected", s);
				continue;
			}
			if (!(s = cpylex(cpy)) || !*s)
				break;
			if (!strcasecmp(s, "TO"))
			{
				if (!(s = cpylex(cpy)) || !*s)
					break;
				f->maxdimension = (int)strtol(s, &e, 10);
				if (*e)
				{
					if (cpy->disc->errorf)
						(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: syntax error: maximum OCCURS number expected", s);
					continue;
				}
				if (!(s = cpylex(cpy)) || !*s)
					break;
			}
			else
				f->maxdimension = f->mindimension;
			if (strcasecmp(s, "TIMES"))
				goto again;
		}
		else if (!strncasecmp(s, "PACKED", 6))
		{
			f->structure = 0;
			f->comp = 3;
		}
		else if (!strncasecmp(s, "PIC", 3))
		{
			f->structure = 0;
			if (!(s = cpylex(cpy)) || !*s)
				break;
			fixedpoint = width = 0;
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					break;
				case 'S':
				case 's':
				case '-':
					f->flags |= CPY_signed;
					continue;
				case 'V':
				case 'v':
				case '.':
					fixedpoint = 1;
					continue;
				case '9':
					f->flags |= CPY_number;
					width++;
					continue;
				case 'X':
				case 'x':
					f->flags |= CPY_string;
					width++;
					continue;
				case '(':
					width = 0;
					n = (int)strtol(s, &e, 10);
					if (*e == ')')
						e++;
					s = e;
					f->width += n;
					if (fixedpoint)
					{
						fixedpoint = 0;
						f->fixedpoint = n;
					}
					continue;
				default:
					if (cpy->disc->errorf)
						(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: unknown PIC", s - 1);
					break;
				}
				break;
			}
			if (width)
			{
				f->width += width;
				if (fixedpoint)
					f->fixedpoint = width;
			}
		}
		else if (!strcasecmp(s, "POINTER"))
		{
			f->structure = 0;
			f->flags |= CPY_pointer;
			f->width = 4;
			s += 7;
			f->comp = state.comp[5];
		}
		else if (!strcasecmp(s, "REDEFINES"))
		{
			if (!(s = cpylex(cpy)) || !*s)
			{
				if (cpy->disc->errorf)
					(*cpy->disc->errorf)(cpy, cpy->disc, 2, "redefined member name expected");
				return 0;
			}
			e = s;
			while ((p = (Cpyfield_t*)dtmatch(cpy->dt, e)) && p->dup > 1)
				e = sfprints("%s_%d", s, p->dup);
			if (!(f->redefines = p))
			{
				if (cpy->disc->errorf)
					(*cpy->disc->errorf)(cpy, cpy->disc, 2, "%s: unknown member", s);
				return 0;
			}
		}
		else if (!strncasecmp(s, "SYNC", 4))
			f->flags |= CPY_sync;
		else if (!strcasecmp(s, "USAGE"))
			f->structure = 0;
		else if (!strncasecmp(s, "VALUE", 5))
		{
			f->structure = 0;
			while ((s = cpylex(cpy)) && *s);
			break;
		}
		else if (cpy->disc->errorf)
			(*cpy->disc->errorf)(cpy, cpy->disc, 1, "%s: unknown attribute", s);
	}
	return f;
}

static int
compwidth(Cpyfield_t* field, int width)
{
	switch (field->comp)
	{
	case 0:
		if (field->fixedpoint)
			width++;
		if (field->flags & CPY_signed)
			width++;
		break;
	case 1:
	case 5:
		if (width <= 2)
			width = 1;
		else if (width <= 4)
			width = 2;
		else if (width <= 9)
			width = 4;
		else
			width = 8;
		break;
	case 3:
		width = (width + 2) / 2;
		break;
	}
	return width;
}

static char*
typename(Cpyfield_t* field)
{
	char*	type;

	if (field->flags & CPY_pointer)
		type = "unsigned char*";
	else if (field->flags & CPY_binary)
		type = state.endian;
	else
		switch (field->comp)
		{
		case 5:
#if _ast_intswap
			type = "le_t";
			break;
#else
			/*FALLTHROUGH*/
#endif
		case 1:
			type = "be_t";
			break;
		case 2:
			type = "ibm_t";
			break;
		case 3:
			type = "bcd_t";
			break;
		default:
			type = 0;
			break;
		}
	return type;
}

#define INDENT(n)		(&indent[sizeof(indent)-(n)-1])

int
cpy2dss(const char* path, Sfio_t* ip, Sfio_t* op, Cpydisc_t* disc, time_t stamp)
{
	Cpy_t*		cpy;
	Cpyfield_t*	field;
	Cpyfield_t*	next;
	Cpyfield_t*	parent;
	Cpyfield_t*	member;
	char*		delimiter;
	char*		type;
	unsigned long	offset;
	long		n;
	int		c;
	int		i;
	int		j;
	int		lev;
	int		sized;
	int		skip;
	int		structure;
	int		width;
	int		levels[256];
	char		indent[elementsof(levels) + 2];

	if (!(cpy = cpyopen(path, ip, disc)))
		return -1;
	memset(indent, ' ', sizeof(indent) - 1);
	indent[sizeof(indent) - 1] = 0;
	if (field = cpyfield(cpy))
	{
		parent = field;
		member = 0;
		offset = field->width;
		i = state.sized && !state.text && state.fixed;
		while (next = cpyfield(cpy))
			if (i)
			{
				width = next->width;
				if (next->flags & CPY_number)
					width = compwidth(next, width);
				offset += width;
				if (!member && (i = strlen(parent->name)) && !memcmp(parent->name, next->name, i) && strmatch(next->name + i, "?(_)(siz|SIZ|LEN|len)*"))
					member = next;
				parent = next;
			}
		if (i && offset > state.fixed)
		{
			if (!member || !(member = member->next))
				error(1, "accumulated field sizes %lu exceeds fixed record size %lu", offset, state.fixed);
			else
			{
				if ((n = member->width - (offset - state.fixed)) > 0)
				{
					error(1, "%s: maximum variable field size shortened from %d to %ld to comply with fixed record size %u", member->name, member->width, n, state.fixed);
					member->width = n;
				}
			}
		}
		levels[lev = 0] = 0;
		switch (state.output)
		{
		case CPY_OFFSETS:
			offset = 0;
			break;
		case CPY_XML:
			sfprintf(sfstdout, "<METHOD>flat</>\n");
			sfprintf(sfstdout, "<FLAT>\n");
			sfprintf(op, "%s<NAME>%s</>\n", INDENT(lev + 1), field->name);
			if (state.regress)
			{
				stamp = 0x42d9e64b;
				if (path && (delimiter = strrchr(path, '/')))
					path = (const char*)delimiter + 1;
			}
			sfprintf(op, "%s<DESCRIPTION>converted by %s from copybook %s</>\n", INDENT(lev + 1), error_info.id, path ? path : "standard input");
			sfprintf(op, "%s<IDENT>@(#)$Id: %s (AT&T Research) %s $</>\n", INDENT(lev + 1), field->name, fmttime("%Y-%m-%d", stamp));
			sfprintf(sfstdout, "%s<LIBRARY>num_t</>\n", INDENT(lev + 1));
			if (state.record && RECTYPE(state.record) != REC_delimited)
			{
				sfprintf(sfstdout, "%s<RECORD>\n", INDENT(lev + 1));
				switch (RECTYPE(state.record))
				{
				case REC_fixed:
					sfprintf(sfstdout, "%s<FIXED>%I*u</>\n", INDENT(lev + 2), sizeof(state.fixed), state.fixed);
					break;
				case REC_variable:
					if (i = REC_V_HEADER(state.record))
						sfprintf(sfstdout, "%s<SIZE>%s%u</>\n", INDENT(lev + 3), REC_V_INCLUSIVE(state.record) ? "+" : "", i);
					if (i = REC_V_LENGTH(state.record))
						sfprintf(sfstdout, "%s<WIDTH>%u</>\n", INDENT(lev + 3), i);
					if (i = REC_V_OFFSET(state.record))
						sfprintf(sfstdout, "%s<OFFSET>%u</>\n", INDENT(lev + 3), i);
					sfprintf(sfstdout, "%s<TYPE>%s</>\n", INDENT(lev + 3), REC_V_LITTLE(state.record) ? "le_t" : "be_t");
					break;
				}
				sfprintf(sfstdout, "%s</>\n", INDENT(lev + 1));
			}
			if (state.text && (state.escape || state.quotebegin))
			{
				sfprintf(sfstdout, "%s<PHYSICAL>\n", INDENT(lev + 1));
				if (state.escape)
					sfprintf(sfstdout, "%s<ESCAPE>%s</>\n", INDENT(lev + 2), state.escape);
				if (state.quotebegin)
				{
					if (state.quoteend)
					{
						sfprintf(sfstdout, "%s<QUOTEBEGIN>%s</>\n", INDENT(lev + 2), state.quotebegin);
						sfprintf(sfstdout, "%s<QUOTEEND>%s</>\n", INDENT(lev + 2), state.quoteend);
					}
					else
						sfprintf(sfstdout, "%s<QUOTE>%s</>\n", INDENT(lev + 2), state.quotebegin);
					sfprintf(sfstdout, "%s<QUOTEALL>1</>\n", INDENT(lev + 2));
				}
				sfprintf(sfstdout, "%s</>\n", INDENT(lev + 1));
			}
			break;
		}
		parent = 0;
		sized = 0;
		skip = 0;
		structure = 1;
		if (next = field->next)
			levels[++lev] = next->level;
		while (field = next)
		{
			delimiter = (next = field->next) ? state.delimiter : state.terminator;
			if (field->level == 88)
				continue;
			if (!structure)
			{
				if (state.output == CPY_XML && !skip)
					sfprintf(op, "%s</>\n", INDENT(lev));
				for (i = lev; i > 0 && field->level != levels[i]; i--);
				if (i > 0)
					while (lev > i)
					{
						lev--;
						switch (state.output)
						{
						case CPY_OFFSETS:
							parent->total = offset - parent->offset;
							if (parent->maxdimension)
							{
								offset += parent->total * (parent->maxdimension - 1);
								parent->total *= parent->maxdimension;
							}
							if (parent->redefines && parent->redefines->total > parent->total)
							{
								offset += parent->redefines->total - parent->total;
								parent->total = parent->redefines->total;
							}
							break;
						case CPY_XML:
							sfprintf(op, "%s</>\n", INDENT(lev));
							break;
						}
						parent = parent->parent;
					}
			}
			skip = 0;
			field->parent = parent;
			if (structure = field->structure)
			{
				if (!next)
				{
					if (disc->errorf)
						(*disc->errorf)(cpy, disc, 2, "%s: empty struct", field->name);
					cpyclose(cpy);
					return -1;
				}
				parent = field;
				if ((lev + 1) >= elementsof(levels))
				{
					if (disc->errorf)
						(*disc->errorf)(cpy, disc, 2, "%s: nesting too deep", field->name);
					cpyclose(cpy);
					return -1;
				}
				levels[lev + 1] = next->level;
			}
			if (state.output == CPY_XML)
			{
				if (state.sized && state.text && state.variable && field->parent && (field->flags & CPY_number))
				{
					i = strlen(field->parent->name);
					if (!memcmp(field->parent->name, field->name, i) && strmatch(field->name + i, "?(_)(siz|SIZ|LEN|len)*"))
					{
						skip = 1;
						sized = field->level == 49;
						continue;
					}
				}
				if (sized)
					lev--;
				else
				{
					sfprintf(op, "%s<FIELD>\n", INDENT(lev));
					sfprintf(op, "%s<NAME>%s</>\n", INDENT(lev + 1), field->name);
				}
				if (field->redefines)
				{
					sfprintf(op, "%s<UNION>%s</>\n", INDENT(lev + 1), field->redefines->name);
					structure = 1;
				}
				if (field->dimension || field->maxdimension)
				{
					sfprintf(op, "%s<ARRAY>\n", INDENT(lev + 1));
					if (field->dimension)
						sfprintf(op, "%s<SIZE>%s</>\n", INDENT(lev + 2), field->dimension);
					if (field->maxdimension)
						sfprintf(op, "%s<SIZE>%d</>\n", INDENT(lev + 2), field->maxdimension);
					sfprintf(op, "%s</>\n", INDENT(lev + 1));
				}
				if (field->flags & CPY_string)
				{
					sfprintf(op, "%s<TYPE>string</>\n", INDENT(lev + 1));
					sfprintf(op, "%s<PHYSICAL>\n", INDENT(lev + 1));
					sfprintf(op, "%s<CODESET>%s</>\n", INDENT(lev + 2), state.codeset);
				}
				else if (field->flags & (CPY_number|CPY_pointer))
				{
					sfprintf(op, "%s<TYPE>", INDENT(lev + 1));
					if (!(field->flags & CPY_signed))
						sfprintf(op, "unsigned ");
					sfprintf(op, "number</>\n");
					sfprintf(op, "%s<PHYSICAL>\n", INDENT(lev + 1));
					if (state.text)
						sfprintf(op, "%s<CODESET>%s</>\n", INDENT(lev + 2), state.codeset);
					else if (field->flags & CPY_binary)
						sfprintf(op, "%s<TYPE>%s</>\n", INDENT(lev + 2), state.endian);
					else
					{
						field->width = compwidth(field, field->width);
						if (type = typename(field))
							sfprintf(op, "%s<TYPE>%s</>\n", INDENT(lev + 2), type);
						else
						{
							sfprintf(op, "%s<CODESET>%s</>\n", INDENT(lev + 2), state.codeset);
							sfprintf(op, "%s<FILL>0</>\n", INDENT(lev + 2));
						}
					}
					if (field->fixedpoint)
						sfprintf(op, "%s<FIXEDPOINT>%d</>\n", INDENT(lev + 2), field->fixedpoint);
					if (field->flags & CPY_sync)
						sfprintf(op, "%s<ALIGN>%d</>\n", INDENT(lev + 2), CPY_ALIGN(field->width));
				}
				if (field->flags & (CPY_string|CPY_number|CPY_pointer))
				{
					if (!state.text)
						sfprintf(op, "%s<WIDTH>%d</>\n", INDENT(lev + 2), field->width);
					if (state.sized && field->parent)
					{
						if (field->flags & CPY_number)
						{
							i = strlen(field->parent->name);
							if (!memcmp(field->parent->name, field->name, i) && strmatch(field->name + i, "?(_)(siz|SIZ|LEN|len)*"))
								field->parent->sized = field;
						}
						else if (field->parent->sized)
						{
							sfprintf(op, "%s<WIDTH>%s</>\n", INDENT(lev + 2), field->parent->sized->name);
							field->parent->sized = 0;
						}
					}
					if (state.text || state.variable && delimiter == state.terminator)
						sfprintf(op, "%s<DELIMITER>%s</>\n", INDENT(lev + 2), delimiter);
					if (sized)
						lev++;
					else
						sfprintf(op, "%s</>\n", INDENT(lev + 1));
				}
				sized = 0;
			}
			else
			{
				if (field->redefines)
					offset = field->redefines->offset;
				c = '0';
				if (field->flags & CPY_number)
				{
					c += field->comp;
					field->width = compwidth(field, field->width);
				}
				width = field->width;
				if (field->flags & CPY_sync)
				{
					i = CPY_ALIGN(width);
					if (j = offset & (i - 1))
						offset += i - j;
				}
				if (field->maxdimension)
					width *= field->maxdimension;
				if (state.output == CPY_BYTEMASK)
					for (i = 0; i < width; i++)
						sfputc(op, c);
				else
				{
					field->offset = offset;
					offset += width;
					if (field->structure)
					{
						type = "struct";
						for (member = field->next; member && member->level > field->level; member = member->next)
							width += member->width * (member->maxdimension ? member->maxdimension : 1);
						if (field->maxdimension)
							width *= field->maxdimension;
					}
					else if (field->flags & CPY_pointer)
						type = "pointer";
					else if (field->flags & CPY_string)
						type = "string";
					else if (field->flags & CPY_number)
					{
						if (state.text || !(type = typename(field)))
							type = (field->flags & CPY_signed) ? "number" : "unsigned";
					}
					sfprintf(sfstdout, "%s\t%lu\t%u", field->name, field->offset, width);
					if (field->dimension)
						sfprintf(sfstdout, "\t%s", field->dimension);
					else
						sfprintf(sfstdout, "\t%d", field->maxdimension);
					sfprintf(sfstdout, "\t%s\n", type);
				}
			}
			lev += structure;
		}
		switch (state.output)
		{
		case CPY_OFFSETS:
			while (levels[lev] > 0 && parent)
			{
				lev--;
				parent->total = offset - parent->offset;
				if (parent->maxdimension)
				{
					offset += parent->total * (parent->maxdimension - 1);
					parent->total *= parent->maxdimension;
				}
				if (parent->redefines && parent->redefines->total > parent->total)
				{
					offset += parent->redefines->total - parent->total;
					parent->total = parent->redefines->total;
				}
				parent = parent->parent;
			}
			sfprintf(sfstdout, "%s\t%lu\t%u\t%u\tstruct\n", ".", offset, 0, 0);
			break;
		case CPY_XML:
			while (levels[lev] > 0)
			{
				sfprintf(op, "%s</>\n", INDENT(lev));
				lev--;
			}	
			sfprintf(op, "</>\n");
			break;
		}
	}
	if (state.output == CPY_BYTEMASK)
		sfputc(op, '\n');
	lev = sfsync(op);
	if (cpyclose(cpy))
		lev = -1;
	return lev;
}

int
main(int argc, char** argv)
{
	char*		file;
	Sfio_t*		ip;
	char*		e;
	int		i;
	int		j;
	Cpydisc_t	disc;
	struct stat	st;
	char		delimiter[2];

	error_info.id = "cpy2dss";
	state.codeset = "ebcdic-m";
	state.delimiter = "|";
	state.endian = "be_t";
	state.output = CPY_XML;
	state.quotebegin = "\"";
	state.sized = 1;
	state.terminator = "&newline;";
	for (i = 0; i < elementsof(state.comp); i++)
		state.comp[i] = i;
	cpyinit(&disc, errorf);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'b':
			state.output = CPY_BYTEMASK;
			continue;
		case 'c':
			state.codeset = opt_info.arg;
			continue;
		case 'C':
			i = strtol(opt_info.arg, &e, 0);
			j = *e ? strtol(e + 1, &e, 0) : 0;
			if (i <= 0 || i > 9 || j <= 0 || j > 9 || *e)
				error(2, "%s: COMP from:to expected in range 1..9", opt_info.arg);
			state.comp[i] = j;
			continue;
		case 'd':
			state.delimiter = opt_info.arg;
			continue;
		case 'D':
			state.terminator = opt_info.arg;
			continue;
		case 'e':
			state.escape = opt_info.arg;
			continue;
		case 'k':
			state.keep = opt_info.num;
			continue;
		case 'l':
			state.endian = "le_t";
			continue;
		case 'o':
			state.output = CPY_OFFSETS;
			continue;
		case 'r':
			state.record = recstr(opt_info.arg, &e);
			if (*e)
				error(2, "%s: invalid record format '%s'", opt_info.arg, e);
			else
				switch (RECTYPE(state.record))
				{
				case REC_delimited:
					delimiter[0] = REC_D_DELIMITER(state.record);
					delimiter[1] = 0;
					state.delimiter = delimiter;
					break;
				case REC_fixed:
					state.fixed = REC_F_SIZE(state.record);
					break;
				case REC_variable:
					state.fixed = REC_V_SIZE(state.record);
					break;
				default:
					error(2, "%s: record format not supported", opt_info.arg);
					break;
				}
			continue;
		case 's':
			state.sized = opt_info.num;
			continue;
		case 't':
			state.text = 1;
			state.codeset = ccmapname(CC_NATIVE);
			continue;
		case 'T':
			state.regress = opt_info.num;
			continue;
		case 'v':
			state.variable = opt_info.num;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!state.quotebegin)
		state.quotebegin = state.quoteend;
	else if (!state.quoteend)
		state.quoteend = state.quotebegin;
	if (state.quotebegin && state.quoteend && streq(state.quotebegin, state.quoteend))
		state.quoteend = 0;
	if (!*argv)
		cpy2dss(NiL, sfstdin, sfstdout, &disc, time(NiL));
	else
		while (file = *argv++)
		{
			if (!stat(file, &st) && (ip = sfopen(NiL, file, "r")))
			{
				cpy2dss(file, ip, sfstdout, &disc, st.st_mtime);
				sfclose(ip);
			}
			else
				error(ERROR_SYSTEM|2, "%s: cannot read", file);
		}
	return error_info.errors != 0;
}
