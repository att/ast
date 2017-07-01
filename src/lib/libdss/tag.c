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
 * properly nested xml tag parse implementation
 * name=value args are treated as tag members
 *
 * for these extra-xml extensions data is literal (no &element; expansions)
 *
 *	<#INCLUDE#>file</>
 *		include file; the file name is the remainder of line
 *		with leading and trailing space ignored
 *
 *	<#TABLE#>
 *		read table data from following lines in the current file
 *		data is split into header and data sections, each section
 *		terminated by a blank line or EOF
 *		the first line of the header section defines the table tag
 *		names and the data delimiter; only one delimiter is
 *		supported; <#INCLUDE#> may appear in the first section
 *		NOTE: the blank lines are non-negotiable
 *		for example:
 *
 *			<!- external header and table -!>
 *			<#TABLE#>
 *			<#INCLUDE#>file</>
 *
 *			<!- inline header external table -!>
 *			<#TABLE#>
 *			NAME:TYPE:VALUE
 *			zero or more comment lines ignored
 *			<#INCLUDE#>file</>
 *
 *			<!- inline header and table -!>
 *			<#TABLE#>
 *			NAME:TYPE:VALUE
 *			zero or more comment lines ignored
 *
 *			foo:integer:1
 *			bar:double:2.3
 *
 *			<!- end of inline data -!>
 */

#define _TAG_PRIVATE_ \
	Sfio_t*		ip; \
	Sfio_t*		op; \
	Sfio_t*		tmp; \
	Tagdisc_t*	disc; \
	Hit_t*		hits;

#define _TAG_FRAME_PRIVATE_ \
	Tagframe_t*	next;

struct Hit_s; typedef struct Hit_s Hit_t;

#include <ast.h>
#include <tag.h>
#include <ccode.h>
#include <ctype.h>
#include <error.h>

#define COMMENT	1
#define STRING	2

typedef int (*Builtin_f)(Tag_t*, Tagframe_t*, Tags_t*, Tags_t*, Tagdisc_t*);

struct Column_s; typedef struct Column_s Column_t;
struct Include_s; typedef struct Include_s Include_t;

struct Column_s
{
	Column_t*	next;
	Tags_t*		tag;
	char		name[1];
};

struct Hit_s
{
	Hit_t*		next;
	Tagbeg_f	begf;
};

struct Include_s
{
	Sfdisc_t	disc;
	long		line;
	char*		file;
	char		path[1];
};

/*
 * return the current tag stack context
 */

char*
tagcontext(Tag_t* tag, Tagframe_t* fp)
{
	char*	s;

	if (!fp)
		return "";
	if (!tag->tmp && !(tag->tmp = sfstropen()))
		return "<?>";
	fp->next = 0;
	while (fp->prev)
	{
		fp->prev->next = fp;
		fp = fp->prev;
	}
	do
	{
		sfprintf(tag->tmp, "<%s>", fp->tag->name);
	} while (fp = fp->next);
	if (!(s = sfstruse(tag->tmp)))
		s = "out of space";
	return s;
}

/*
 * include file pop exception handler
 */

static int
except(Sfio_t* sp, int op, void* val, Sfdisc_t* dp)
{
	Include_t*	ip = (Include_t*)dp;

	switch (op)
	{
	case SF_CLOSING:
	case SF_DPOP:
	case SF_FINAL:
		if (op != SF_CLOSING)
		{
			error_info.line = ip->line;
			error_info.file = ip->file;
			free(dp);
		}
		break;
	}
	return 0;
}

/*
 * parse include file name and push on the input stack
 */

static int
push(Tag_t* tag, register char* s, Tagdisc_t* disc)
{
	char*		file;
	char*		path;
	Sfio_t*		sp;
	Include_t*	ip;
	char		tmp[PATH_MAX];

	while (isspace(*s))
		s++;
	file = s;
	for (s += strlen(s) - 1; s > file; s--)
		if (*s == '>' && s > (file + 1) && *(s - 1) == '/' && *(s - 2) == '<')
			s -= 2;
		else if (!isspace(*s))
			break;
	*(s + 1) = 0;
	if (!*file)
	{
		if (tag->disc->errorf)
			(*tag->disc->errorf)(NiL, tag->disc, ERROR_SYSTEM|2, "include file name omitted");
		return -1;
	}
	if (!(path = pathfind(file, disc->id, NiL, tmp, sizeof(tmp))))
	{
		if (tag->disc->errorf)
			(*tag->disc->errorf)(NiL, tag->disc, ERROR_SYSTEM|2, "%s: include file not found", file);
		return -1;
	}
	if (!(sp = sfopen(NiL, path, "r")))
	{
		if (tag->disc->errorf)
			(*tag->disc->errorf)(NiL, tag->disc, ERROR_SYSTEM|2, "%s: cannot read include file", path);
		return -1;
	}
	if (!(ip = newof(0, Include_t, 1, strlen(path))))
	{
		if (tag->disc->errorf)
			(*tag->disc->errorf)(NiL, tag->disc, ERROR_SYSTEM|2, "out of space");
		sfclose(sp);
		return -1;
	}
	strcpy(ip->path, path);
	ip->line = error_info.line;
	ip->file = error_info.file;
	error_info.line = 0;
	error_info.file = ip->path;
	ip->disc.exceptf = except;
	if (!(sp = sfstack(tag->ip, sp)) || !sfdisc(sp, &ip->disc))
	{
		if (tag->disc->errorf)
			(*tag->disc->errorf)(NiL, tag->disc, ERROR_SYSTEM|2, "%s: cannot stack include file", ip->path);
		if (sp)
			sfstack(tag->ip, SF_POPSTACK);
		return -1;
	}
	return 0;
}

/*
 * file include builtin
 */

static int
include(Tag_t* tag, Tagframe_t* fp, Tags_t* cp, Tags_t* tags, Tagdisc_t* disc)
{
	register char*		s;

	if (!(s = sfgetr(tag->ip, '\n', 1)))
	{
		if (disc->errorf)
			(*disc->errorf)(tag, disc, 2, "<%s>: unexpected EOF", cp->name);
		return -1;
	}
	error_info.line++;
	return push(tag, s, disc);
}

/*
 * get next line with optional embedded <#INCLUDE#>
 */

static char*
incline(Tag_t* tag, Tagframe_t* fp, int* inc, Tagdisc_t* disc)
{
	register char*	s;

	*inc = 0;
	for (;;)
	{
		if (!(s = sfgetr(tag->ip, '\n', 1)))
		{
			if (disc->errorf)
				(*disc->errorf)(tag, disc, 2, "%s: unexpected EOF", tagcontext(tag, fp));
			break;
		}
		error_info.line++;
		while (isspace(*s))
			s++;
		if (*s != '<')
			return s;
		if (strncmp(s, "<#INCLUDE#>", 11))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: invalid embedded tag", tagcontext(tag, fp));
		}
		if (push(tag, s + 11, disc))
			break;
		*inc = 1;
	}
	return 0;
}

/*
 * table data builtin
 */

static int
table(Tag_t* tag, Tagframe_t* fp, Tags_t* cp, Tags_t* tags, Tagdisc_t* disc)
{
	register char*	s;
	register char*	b;
	Column_t*	col;
	Column_t*	end;
	Column_t*	p;
	int		del;
	int		inc;
	int		ret;
	int		sep;
	Tags_t*		tp;
	Tagframe_t	frame;

	ret = -1;
	col = 0;
	if (sfgetr(tag->ip, '\n', 1))
	{
		error_info.line++;
		memset(&frame, 0, sizeof(frame));
		frame.prev = fp;
		frame.level = fp->level + 1;
		frame.tag = cp;
		if (!(s = incline(tag, fp, &inc, disc)))
			goto done;
		end = 0;
		for (b = s; isalnum(*b); b++);
		if (!(del = *b))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: invalid delimiter", tagcontext(tag, &frame));
			goto done;
		}
		if (b == s)
			s++;
		while (*s)
		{
			for (b = s; *s && *s != del; s++);
			if (!(p = newof(0, Column_t, 1, s - b)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			if (*s)
				*s++ = 0;
			strcpy(p->name, b);
			for (tp = tags; tp->name && !streq(b, tp->name); tp++);
			if (!tp->name)
			{
				if (disc->errorf)
					(*disc->errorf)(tag, disc, 2, "%s<%s>: unknown tag", tagcontext(tag, &frame), b);
				goto done;
			}
			p->tag = tp;
			if (end)
				end = end->next = p;
			else
				col = end = p;
		}
		do
		{
			if (!(s = incline(tag, fp, &inc, disc)))
				goto done;
			if (!*s)
			{
				if (!(s = sfgetr(tag->ip, '\n', 1)))
					goto done;
				break;
			}
		} while (!inc);
		frame.tag = tp;
		sep = 0;
		do
		{
			if (!*s)
			{
				ret = 0;
				goto done;
			}
			if (sep)
			{
				if (fp->tag->endf && !(*fp->tag->endf)(tag, fp, disc))
					return -1;
				if (fp->tag->begf && !(*fp->tag->begf)(tag, fp, fp->tag->name, disc))
					return -1;
			}
			else
				sep = 1;
			for (p = col; p && *s; p = p->next)
			{
				for (b = s; *s && *s != del; s++);
				if (*s)
					*s++ = 0;
				if (p->tag->begf && !(*p->tag->begf)(tag, &frame, tp->name, disc))
					goto done;
				if (*b && p->tag->datf && (*p->tag->datf)(tag, &frame, b, disc))
					goto done;
				if (p->tag->endf && (*p->tag->endf)(tag, &frame, disc))
					goto done;
			}
			if (p)
			{
				frame.tag = cp;
				if (disc->errorf)
					(*disc->errorf)(tag, disc, 2, "%s: not enough columns", tagcontext(tag, &frame));
				goto done;
			}
			if (*s)
			{
				frame.tag = cp;
				if (disc->errorf)
					(*disc->errorf)(tag, disc, 2, "%s: too many columns", tagcontext(tag, &frame));
				goto done;
			}
			error_info.line++;
		} while (s = sfgetr(tag->ip, '\n', 1));
	}
 done:
	while (end = col)
	{
		col = end->next;
		free(end);
	}
	return ret;
}

static Tags_t builtin[] =
{
	"#INCLUDE#",	"Push a tag include file.",
			0,(Tagbeg_f)include,0,0,
	"#TABLE#",	"Read delimited table data.",
			0,(Tagbeg_f)table,0,0,
	0,
};

/*
 * parse a nested tag
 */

static int
tagparse(Tag_t* tag, Tagframe_t* fp, Tags_t* tags, Tagdisc_t* disc)
{
	register int		c;
	register int		item;
	register int		quote;
	register long		back;
	register Tags_t*	tp;
	register Tags_t*	vp;
	register char*		s;
	register char*		t;
	register char*		u;
	long			keep_first;
	long			keep_last;
	char*			v;
	size_t			attrs;
	Tags_t*			np;
	Tagframe_t		frame;
	Tagframe_t		attr;

	item = 0;
	keep_first = keep_last = 0;
	if (fp)
		fp->attr = 0;
	for (;;)
	{
		switch (c = sfgetc(tag->ip))
		{
		case EOF:
			break;
		case '<':
			if (item == '&')
				item = 0;
			if (!item)
			{
				if (fp && fp->tag->datf && (back = sfstrtell(tag->op)))
				{
					if (!(s = sfstruse(tag->op)))
						goto nospace;
					u = s + keep_last;
					for (t = s + back; t > u && isspace(*(t - 1)); t--);
					*t = 0;
					if (keep_first)
						for (u = s + keep_first - 1; s < u && *s && isspace(*s); s++);
					else
						while (*s && isspace(*s))
							s++;
					if ((*s || keep_first) && (*fp->tag->datf)(tag, fp, s, disc))
						return -1;
					back = 0;
				}
				else
					sfstrseek(tag->op, 0, SEEK_SET);
				if (fp)
					fp->attr = 0;
				keep_first = keep_last = 0;
				item = c;
				attrs = 0;
				quote = 0;
				if ((c = sfgetc(tag->ip)) != EOF)
				{
					sfungetc(tag->ip, c);
					if (c == '!')
						quote |= COMMENT;
				}
			}
			else
				sfputc(tag->op, c);
			continue;
		case '>':
			if (item == '<' && !(quote & STRING))
			{
				item = 0;
				if (!(s = sfstruse(tag->op)))
					goto nospace;
				if (quote & COMMENT)
					quote ^= COMMENT;
				else if (*s == '/')
				{
					s++;
					if (!fp)
						return -1;
					if (*s && strcasecmp(s, fp->tag->name))
					{
						if (disc->errorf)
							(*disc->errorf)(tag, disc, 2, "%s: no closing tag", tagcontext(tag, fp));
						return -1;
					}
					fp = 0;
					break;
				}
				else if (!tags)
				{
					if (disc->errorf)
						(*disc->errorf)(tag, disc, 2, "%s: invalid input", tagcontext(tag, fp));
					return -1;
				}
				else
				{
					for (tp = tags; tp->name; tp++)
						if (!strcasecmp(s, tp->name))
							break;
					if (!tp->name)
					{
						for (tp = builtin; tp->name; tp++)
							if (!strcasecmp(s, tp->name))
								break;
						if (!tp->name)
						{
							if (disc->errorf)
								(*disc->errorf)(tag, disc, 2, "%s<%s>: unknown tag", tagcontext(tag, fp), s);
							return -1;
						}
						if ((*(Builtin_f)tp->begf)(tag, fp, tp, tags, disc))
							return -1;
						continue;
					}
					memset(&frame, 0, sizeof(frame));
					frame.level = (frame.prev = fp) ? (fp->level + 1) : 0;
					frame.tag = tp;
					if (!tp->begf)
						np = 0;
					else if (!(np = (*tp->begf)(tag, &frame, s, disc)))
						return -1;
					else if (attrs)
					{
						/*UNDENT...*/
	s = sfstrbase(tag->op) + attrs;
	while (*(t = s))
	{
		while (*s && !isspace(*s) && *s != '=')
			s++;
		if (*s == '=')
		{
			*s++ = 0;
			u = v = s;
			c = 0;
			while (*s)
			{
				if (*s == c)
				{
					c = 0;
					s++;
				}
				else if (c)
					*u++ = *s++;
				else if (*s == '"')
					c = *s++;
				else if (isspace(*s))
				{
					*s++ = 0;
					break;
				}
				else
					*u++ = *s++;
			}
			*u = 0;
		}
		else
		{
			v = "1";
			if (*s)
				*s++ = 0;
		}
		for (vp = np; vp->name; vp++)
			if (!strcasecmp(t, vp->name))
				break;
		if (!vp->name)
		{
			if (disc->errorf)
				(*disc->errorf)(tag, disc, 2, "%s%s: unknown option", tagcontext(tag, &frame), t);
			return -1;
		}
		memset(&attr, 0, sizeof(attr));
		attr.prev = &frame;
		attr.level = frame.level + 1;
		attr.tag = vp;
		if (vp->begf && !(*vp->begf)(tag, &attr, s, disc))
			return -1;
		if (*v && vp->datf && (*vp->datf)(tag, &attr, v, disc))
			return -1;
		if (vp->endf && (*vp->endf)(tag, &attr, disc))
			return -1;
	}
						/*...INDENT*/
					}
					if (disc->errorf && error_info.trace <= -3)
					{
						error_info.indent++;
						(*disc->errorf)(tag, disc, -3, "%s", frame.tag->name);
					}
					c = tagparse(tag, &frame, np, disc);
					if (disc->errorf && error_info.trace <= -3)
					{
						(*disc->errorf)(tag, disc, -3, "/%s", frame.tag->name);
						error_info.indent--;
					}
					if (c)
						return -1;
					if (tp->endf && (*tp->endf)(tag, &frame, disc))
						return -1;
				}
			}
			else
				sfputc(tag->op, c);
			continue;
		case '"':
			if (item == '<')
				quote ^= STRING;
			sfputc(tag->op, c);
			continue;
		case '&':
			if (!item)
			{
				item = c;
				back = sfstrtell(tag->op);
			}
			sfputc(tag->op, c);
			continue;
		case ';':
			if (item == '&')
			{
				item = 0;
				sfputc(tag->op, 0);
				s = sfstrseek(tag->op, back, SEEK_SET) + 1;
				if (*s == '#')
					c = (int)strtol(s + 1, NiL, 10) & 0377;
				else
				{
					if (fp)
						fp->attr |= TAG_ATTR_conv;
					if (streq(s, "alert"))
						c = CC_bel;
					else if (streq(s, "amp"))
						c = '&';
					else if (streq(s, "backspace"))
						c = '\b';
					else if (streq(s, "escape"))
						c = CC_esc;
					else if (streq(s, "formfeed"))
						c = '\f';
					else if (streq(s, "lt"))
						c = '<';
					else if (streq(s, "gt"))
						c = '>';
					else if (streq(s, "newline"))
						c = '\n';
					else if (streq(s, "nul"))
						c = 0;
					else if (streq(s, "quot"))
						c = '"';
					else if (streq(s, "return"))
						c = '\r';
					else if (streq(s, "space"))
						c = ' ';
					else if (streq(s, "tab"))
						c = '\t';
					else if (streq(s, "vtab"))
						c = CC_vt;
					else
					{
						if (disc->errorf)
							(*disc->errorf)(tag, disc, 2, "&%s;: unknown entity", s);
						c = '?';
					}
				}
				sfputc(tag->op, c);
				keep_last = sfstrtell(tag->op);
				if (!keep_first)
					keep_first = keep_last;
				continue;
			}
			else
				sfputc(tag->op, c);
			continue;
		case '\n':
			error_info.line++;
			/*FALLTHROUGH*/
		case ' ':
		case '\t':
		case '\v':
		case '=':
			if (item == '&')
				item = 0;
			if (item == '<' && !attrs)
			{
				attrs = sfstrtell(tag->op) + 1;
				c = 0;
			}
			/*FALLTHROUGH*/
		default:
			sfputc(tag->op, c);
			continue;
		}
		break;
	}
	if (item)
	{
		if (disc->errorf)
			(*disc->errorf)(tag, disc, 2, "%s: unexpected EOF", tagcontext(tag, fp));
		return -1;
	}
	if (fp)
	{
		if (disc->errorf)
			(*disc->errorf)(tag, disc, 2, "%s: no closing tag", tagcontext(tag, fp));
		return -1;
	}
	return 0;
 nospace:
	if (disc->errorf)
		(*disc->errorf)(tag, disc, 2, "out of space");
	return -1;
}

/*
 * open and parse nested tags in ip
 */

Tag_t*
tagopen(Sfio_t* ip, const char* file, int line, Tags_t* tags, Tagdisc_t* disc)
{
	register Tag_t*	tag;
	char*		ofile;
	int		oline;
	int		r;

	if (!(tag = newof(0, Tag_t, 1, 0)) || !(tag->op = sfstropen()))
	{
		if (tag)
			free(tag);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	tag->ip = ip;
	tag->disc = disc;
	ofile = error_info.file;
	oline = error_info.line;
	error_info.file = (char*)file;
	error_info.line = line;
	r = tagparse(tag, NiL, tags, disc);
	error_info.file = ofile;
	error_info.line = oline;
	if (r)
	{
		tagclose(tag);
		return 0;
	}
	return tag;
}

/*
 * sync tagopen handle and return input stream
 */

Sfio_t*
tagsync(Tag_t* tag)
{
	sfstrseek(tag->op, 0, SEEK_SET);
	return tag->ip;
}

/*
 * close a tagopen handle
 */

int
tagclose(Tag_t* tag)
{
	if (!tag)
		return -1;
	if (tag->op)
		sfstrclose(tag->op);
	if (tag->tmp)
		sfstrclose(tag->tmp);
	free(tag);
	return 0;
}

/*
 * tagscan helper
 */

static int
scan(Tag_t* tag, Tagframe_t* fp, Tagscan_f visit, void* handle, register Tags_t* tags, Tagdisc_t* disc)
{
	register Hit_t*	hp;
	register int	r;
	unsigned int	f;
	Tags_t*		nest;
	Tagframe_t*	tp;
	Tagframe_t	frame;

	while (tags->name)
	{
		memset(&frame, 0, sizeof(frame));
		frame.level = (frame.prev = fp) ? (fp->level + 1) : 0;
		frame.tag = tags;
		f = 0;
		if (tags->begf)
		{
			for (tp = fp; tp; tp = tp->prev)
				if (tp->tag->begf == tags->begf)
				{
					f |= TAG_SCAN_rec|TAG_SCAN_rep;
					break;
				}
			if (!f)
			{
				for (hp = tag->hits; hp; hp = hp->next)
					if (hp->begf == tags->begf)
					{
						f |= TAG_SCAN_rep;
						break;
					}
				if (!f)
				{
					if (!(hp = newof(0, Hit_t, 1, 0)))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
						return -1;
					}
					hp->begf = tags->begf;
					hp->next = tag->hits;
					tag->hits = hp;
				}
			}
		}
		if (!f && tags->begf && (nest = (*tags->begf)(tag, &frame, NiL, disc)))
		{
			if (r = (*visit)(tag, &frame, handle, TAG_SCAN_beg, disc))
				return r;
			if (r = scan(tag, &frame, visit, handle, nest, disc))
				return r;
			if (r = (*visit)(tag, &frame, handle, TAG_SCAN_end, disc))
				return r;
		}
		else if (r = (*visit)(tag, &frame, handle, f, disc))
			return r;
		tags++;
	}
	return 0;
}

/*
 * scan the tag tree
 */

int
tagscan(Tags_t* tags, Tagscan_f visit, void* handle, Tagdisc_t* disc)
{
	Tag_t*	tag;
	Hit_t*	hp;
	Hit_t*	np;
	int	r;

	if (!(tag = newof(0, Tag_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	r = scan(tag, NiL, visit, handle, tags, disc);
	for (hp = tag->hits; hp; hp = np)
	{
		np = hp->next;
		free(hp);
	}
	free(tag);
	return r;
}

/*
 * tagusage() helper
 */

static int
usage(Tag_t* tag, register Tagframe_t* fp, void* handle, unsigned int flags, Tagdisc_t* disc)
{
	register Sfio_t*	op = (Sfio_t*)handle;

	if (flags & TAG_SCAN_end)
	{
		sfprintf(op, "}\n");
		return 0;
	}
	sfprintf(op, "[+%s", fp->tag->name);
	if (fp->tag->description)
	{
		sfputc(op, '?');
		if (optesc(op, fp->tag->description, 0))
			return -1;
	}
	if (flags & (TAG_SCAN_rec|TAG_SCAN_rep))
		sfprintf(op, " Defined above.");
	sfputc(op, ']');
	if (flags & TAG_SCAN_beg)
		sfputc(op, '{');
	sfputc(op, '\n');
	return 0;
}

/*
 * generate optget usage[] section in op
 */

int
tagusage(Tags_t* tags, Sfio_t* op, Tagdisc_t* disc)
{
	return tagscan(tags, usage, op, disc);
}
