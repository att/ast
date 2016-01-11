/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * html to rtf filter
 */

static const char usage[] =
"[-?\n@(#)$Id: html2rtf (AT&T Research) 1999-01-01 $\n]"
USAGE_LICENSE
"[+NAME?html2rtf - html to rtf filter]"
"[+DESCRIPTION?\bhtml2rtf\b converts input \bhtml\b documents to an \bRTF\b"
"	document on the standard output. \bhtml2rtf\b expects properly nested"
"	begin/end tags in the input \bhtml\b and warns about imbalance.]"

"[d:debug?Set the debug trace level to \alevel\a. Higher levels produce"
"	more output.]#[level]"
"[f:font-size?Set the initial font size to \asize\a points.]#[size:=12]"
"[p:project-file?Appends MS HELP project information to the help project file"
"	\afile\a. This file combines individual RTF files into a"
"	hyper-linked collection. Note that MS expects \afile\a to have a"
"	\b.hlp\b extension.]:[file]"
"[v:verbose?Enable verbose error and warning messages. Some \bhtml\b source"
"	can't stand the heat.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bman\b(1), \bmm\b(1), \bmm2html\b(1), \btroff\b(1),"
"	\btroff2html\b(1)]"
;

#include "html2rtf.h"

#include <error.h>

#define LIST_INDENT		140
#define STK_LIST_COMPACT	(STK_TAG<<0)

#define a_close			data[0].number
#define a_label			data[1].string

#define font_size		data[0].number

#define list_counter		data[0].number
#define list_hanging		data[1].number
#define list_indent		data[2].number
#define list_label		data[3].string
#define list_type		data[4].number

#define title_cc		data[0].number
#define title_lastlastc		data[1].number
#define title_op		data[2].io
#define title_tc		data[3].number

State_t			state;

/*
 * return the attribute pointer for name in ap
 */

static Attribute_t*
attribute(register Attribute_t* ap, const char* name)
{
	if (ap)
		for (; ap->name; ap++)
			if (!strcasecmp(ap->name, name))
				return ap;
	return 0;
}

/*
 * new paragraph with optional hanging indent
 */

static void
par(int hanging, const char* tail)
{
	if (hanging && !(state.sp->flags & STK_LIST_COMPACT))
		sfputr(state.out, "\\line", -1);
	sfputr(state.out, "\\par\\pard", -1);
	if (state.center)
		sfputr(state.out, "\\qc", -1);
	if (hanging)
		sfprintf(state.out, "\\fi%d", twips(state.hanging - state.indent));
	sfprintf(state.out, "\\li%d\\tx%d\\tx20000%s", twips(state.indent), twips(state.indent), tail ? tail : "");
	state.sep = 1;
}

static void
anchor(int ref, register char* s)
{
	register int	c;

	if (s)
	{
		if (ref)
		{
			if (*s != '#')
			{
				error(1, "%s: unknown link", s);
				return;
			}
			s++;
			sfprintf(state.out, "{\\uldb %s}{\\v", s);
		}
		else
			sfputr(state.out, "#{\\footnote", -1);
		sfprintf(state.out, " %s.", state.prefix);
		while (c = *s++)
			sfputc(state.out, isalnum(c) ? c : '.');
		sfputc(state.out, '}');
		if (ref)
		{
			sfputr(state.out, "{\\*\\comment", -1);
			state.sep = 1;
			state.sp->a_close = 1;
		}
	}
}

static int
start_a(Tag_t* tp, Attribute_t* ap)
{
	Attribute_t*	op;

	NoP(tp);
	state.sp->a_close = 0;
	if (op = attribute(ap, "HREF"))
		anchor(1, op->value);
	if (op = attribute(ap, "NAME"))
		anchor(0, op->value);
	return 1;
}

static int
end_a(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.sp->a_close)
	{
		sfputc(state.out, '}');
		state.sep = 0;
	}
	return 0;
}

static int
start_b(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "{\\b", -1);
	state.sep = 1;
	return 1;
}

static int
start_bq(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.sp->list_hanging = state.hanging;
	state.sp->list_indent = state.indent;
	state.hanging += LIST_INDENT;
	state.indent = state.hanging;
	par(0, NiL);
	sfprintf(state.out, "\\ri%d", state.indent);
	state.sep = 1;
	return 1;
}

static int
end_bq(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.hanging = state.sp->list_hanging;
	state.indent = state.sp->list_indent;
	sfprintf(state.out, "\\ri0");
	state.sep = 1;
	return 1;
}

static int
start_body(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
start_br(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "\\line", -1);
	state.sep = 1;
	return 0;
}

static int
start_caption(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
start_center(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.center++;
	state.pre++;
	par(0, NiL);
	return 1;
}

static int
end_center(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.center > 0)
		state.center--;
	if (state.pre > 0)
		state.pre--;
	return 1;
}

static int
start_dd(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.sp->flags & STK_HEADING)
		par(0, NiL);
	else
		sfputr(state.out, "\\tab", -1);
	state.sep = 1;
	return 1;
}

static int
start_dl(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	if (attribute(ap, "COMPACT"))
		state.sp->flags |= STK_LIST_COMPACT;
	state.sp->list_hanging = state.hanging;
	state.sp->list_indent = state.indent;
	state.hanging += LIST_INDENT;
	state.indent = state.hanging + LIST_INDENT * 2;
	return 1;
}

static int
end_LIST(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.hanging = state.sp->list_hanging;
	state.indent = state.sp->list_indent;
	return 1;
}

static int
start_dt(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	par(1, NiL);
	return 1;
}

static int
start_fn(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "\\~[\\~", -1);
	return 1;
}

static int
end_fn(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "\\~]\\~", -1);
	return 0;
}

static int
start_font(Tag_t* tp, Attribute_t* ap)
{
	char*		s;
	char*		e;
	int		n;
	Attribute_t*	op;

	NoP(tp);
	if ((op = attribute(ap, "SIZE")) && (s = op->value) && (n = strtol(s, &e, 10)) && !*e)
	{
		if (*s == '+' || *s == '-')
			n += state.fontsize;
		state.sp->font_size = state.fontsize;
		state.fontsize = n;
		sfprintf(state.out, "{\\fs%d", twips(n));
		state.sep = 1;
		return 1;
	}
	return 0;
}

static int
end_font(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.fontsize = state.sp->font_size;
	sfputc(state.out, '}');
	state.sep = 0;
	return 1;
}

static int
start_H(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	(state.sp - 1)->flags |= STK_HEADING;
	state.sp->font_size = state.fontsize;
	state.fontsize += (7 - (tp->name[1] - '0')) * 1;
	sfprintf(state.out, "{\\b\\fs%d", twips(state.fontsize));
	state.sep = 1;
	return 1;
}

static int
end_H(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.fontsize = state.sp->font_size;
	sfputc(state.out, '}');
	state.sep = 0;
	return 1;
}

static int
start_head(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
end_head(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
start_hr(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "{\\brdrt\\brdrsh\\par}", -1);
	state.sep = 0;
	return 0;
}

static int
start_html(Tag_t* tp, Attribute_t* ap)
{
	char*	s;

	NoP(tp);
	NoP(ap);
	sfputr(state.out, "{\\rtf1 \\ansi \\deff0", '\n');
	s = strchr(usage, '\n') + 5;
	sfprintf(state.out, "{\\*\\comment generator: %-.*s}\n", strchr(usage, '\n') - s, s);
	sfputr(state.out, "{\\fonttbl", '\n');
	sfputr(state.out, "{\\f0 \\froman Times New Roman;}", '\n');
	sfputr(state.out, "{\\f1 \\fmodern Line Printer;}", '\n');
	sfputr(state.out, "{\\f2 \\froman Symbol;}", '\n');
	sfputr(state.out, "{\\f3 \\fswiss Ariel;}", '\n');
	sfputr(state.out, "}", '\n');
	sfprintf(state.out, "\\fs%d\n", twips(state.fontsize));
	return 1;
}

static int
end_html(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "\n}", '\n');
	return 1;
}

static int
start_i(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "{\\i", -1);
	state.sep = 1;
	return 1;
}

static int
start_img(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

/*
 * NOTE: roman() transcribed from GNU groff
 */

static void
roman(register int n, int format)
{
	register char*	dig;
	register int	i;
	register int	m;

	dig = islower(format) ? "zwmdclxvi" : "ZWMDCLXVI";
	if (n <= -40000 || n >= 40000)
	{
		sfprintf(state.out, "<%d>", n);
		return;
	}
	if (n == 0)
	{
		sfputc(state.out, '0');
		return;
	}
	if (n < 0)
	{
		n = -n;
		sfputc(state.out, '-');
	}
	while (n >= 10000)
	{
		n -= 10000;
		sfputc(state.out, dig[0]);
	}
	for (i = 1000; i > 0; i /= 10, dig += 2)
	{
		m = n / i;
		n -= m * i;
		switch (m)
		{
		case 9:
			sfputc(state.out, dig[2]);
			sfputc(state.out, dig[0]);
			break;
		case 8:
			sfputc(state.out, dig[1]);
			sfputc(state.out, dig[2]);
			sfputc(state.out, dig[2]);
			sfputc(state.out, dig[2]);
			break;
		case 7:
			sfputc(state.out, dig[1]);
			sfputc(state.out, dig[2]);
			sfputc(state.out, dig[2]);
			break;
		case 6:
			sfputc(state.out, dig[1]);
			sfputc(state.out, dig[2]);
			break;
		case 5:
			sfputc(state.out, dig[1]);
			break;
		case 4:
			sfputc(state.out, dig[2]);
			sfputc(state.out, dig[1]);
			break;
		case 3:
			sfputc(state.out, dig[2]);
			/*FALLTHROUGH*/
		case 2:
			sfputc(state.out, dig[2]);
			/*FALLTHROUGH*/
		case 1:
			sfputc(state.out, dig[2]);
			break;
		}
	}
}

static int
start_li(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	par(1, "{\\b ");
	switch (state.sp->list_type)
	{
	case '1':
		sfprintf(state.out, "%d.", state.sp->list_counter);
		break;
	case 'A':
		sfprintf(state.out, "%c)", 'A' + state.sp->list_counter);
		break;
	case 'a':
		sfprintf(state.out, "%c)", 'a' + state.sp->list_counter);
		break;
	case 'I':
	case 'i':
		roman(state.sp->list_counter, state.sp->list_type);
		sfputc(state.out, ')');
		break;
	default:
		sfputr(state.out, state.sp && state.sp->list_label ? state.sp->list_label : "\\bullet", -1);
		break;
	}
	state.sp->list_counter++;
	sfputr(state.out, "}\\tab", -1);
	state.sep = 1;
	return 1;
}

static int
start_meta(Tag_t* tp, Attribute_t* ap)
{
	Attribute_t*	op;

	NoP(tp);
	if ((op = attribute(ap, "NAME")) && op->value)
	{
		sfprintf(state.out, "{\\*\\comment %s", op->value);
		if ((op = attribute(ap, "CONTENT")) && op->value)
			sfprintf(state.out, ": %s", op->value);
		sfputr(state.out, "}", '\n');
	}
	return 0;
}

static int
start_ol(Tag_t* tp, Attribute_t* ap)
{
	char*		e;
	Attribute_t*	op;

	NoP(tp);
	if (attribute(ap, "COMPACT"))
		state.sp->flags |= STK_LIST_COMPACT;
	if (!(op = attribute(ap, "START")) || !op->value || (state.sp->list_counter = strtol(op->value, &e, 10)) < 0 || *e)
		state.sp->list_counter = 1;
	state.sp->list_type = (op = attribute(ap, "TYPE")) && op->value ? *op->value : '1';
	state.sp->list_hanging = state.hanging;
	state.sp->list_indent = state.indent;
	state.hanging += LIST_INDENT;
	state.indent = state.hanging + LIST_INDENT;
	return 1;
}

static int
start_p(Tag_t* tp, Attribute_t* ap)
{
	register char*	s;
	Attribute_t*	op;

	NoP(tp);
	par(0, NiL);
	if ((op = attribute(ap, "ALIGN")) && (s = op->value))
	{
		if (!strcasecmp(s, "CENTER"))
			sfputr(state.out, "\\qc", -1);
		else if (!strcasecmp(s, "LEFT"))
			sfputr(state.out, "\\ql", -1);
		else if (!strcasecmp(s, "RIGHT"))
			sfputr(state.out, "\\qr", -1);
	}
	return 1;
}

static int
start_pre(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.pre++;
	return 1;
}

static int
end_pre(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.pre > 0)
		state.pre--;
	return 1;
}

static int
start_rendering(register Tag_t* tp, Attribute_t* ap)
{
	register Render_t*	rp;
	register int		i;

	if (rp = (Render_t*)tp->data)
		for (i = 0; i < rp->tags; i++)
			if ((tp = rp->tag[i]) && tp->start)
				(*tp->start)(tp, ap);
	return 1;
}

static int
end_rendering(register Tag_t* tp, Attribute_t* ap)
{
	register Render_t*	rp;
	register int		i;

	if (rp = (Render_t*)tp->data)
		for (i = rp->tags - 1; i > 0; i--)
			if ((tp = rp->tag[i]) && tp->end)
				(*tp->end)(tp, ap);
	return 1;
}

static int
start_render(register Tag_t* tp, Attribute_t* ap)
{
	register Render_t*	rp;
	register char*		s;
	register char*		e;
	register int		n;
	Attribute_t*		op;

	if ((op = attribute(ap, "TAG")) && (s = op->value))
	{
		if (tp = (Tag_t*)hashget(state.tags, s))
		{
			if (tp->data)
				free(tp->data);
			tp->start = 0;
			tp->end = 0;
			tp->data = 0;
		}
		else if (!(tp = newof(NiL, Tag_t, 1, 0)) || !(tp->name = hashput(state.tags, 0, tp)))
			error(ERROR_SYSTEM|3, "out of space [tag]");
		if ((op = attribute(ap, "STYLE")) && (s = op->value))
		{
			for (n = 0, e = s; e && (e = strchr(e, ',')); n++, e++);
			if (!(rp = newof(NiL, Render_t, 1, n * sizeof(Tag_t*))))
				error(ERROR_SYSTEM|3, "out of space [render]");
			n = 0;
			do
			{
				if (e = strchr(s, ','))
					*e++ = 0;
				if (rp->tag[n] = (Tag_t*)hashget(state.tags, s))
					n++;
			} while (s = e);
			if (!(rp->tags = n))
				free(rp);
			else
			{
				tp->start = start_rendering;
				tp->end = end_rendering;
				tp->data = (void*)rp;
			}
		}
	}
	return 0;
}

static int
start_sub(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
end_sub(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
start_sup(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
end_sup(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
start_table(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.center++;
	par(0, NiL);
	return 1;
}

static int
end_table(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.center > 0)
		state.center++;
	sfputr(state.out, "}", '\n');
	return 0;
}

static int
start_td(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
end_td(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 1;
}

static int
start_th(Tag_t* tp, Attribute_t* ap)
{
	register Attribute_t*	op;

	NoP(tp);
	if (!(op = attribute(ap, "ALIGN")) || !op->value || *op->value != 'l' && *op->value != 'L')
		sfputr(state.out, "\\~\\~\\~\\~\\~\\~\\~\\~\\~\\~\\~\\~", -1);
	return 0;
}

static int
start_title(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	state.pre++;
	return 1;
}

static int
end_title(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.pre > 0)
		state.pre--;
	return 1;
}

static int
start_tr(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 0;
}

static int
end_tr(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	return 1;
}

static int
start_tt(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "{\\f1", -1);
	state.sep = 1;
	return 1;
}

static int
start_ul(Tag_t* tp, Attribute_t* ap)
{
	Attribute_t*	op;

	NoP(tp);
	if (attribute(ap, "COMPACT"))
		state.sp->flags |= STK_LIST_COMPACT;
	state.sp->list_type = 0;
	switch ((op = attribute(ap, "TYPE")) && op->value ? *op->value : 0)
	{
	case 'c':
		state.sp->list_label = "\\'b0";
		break;
	case 's':
		state.sp->list_label = "\\'a4";
		break;
	default:
		state.sp->list_label = "\\bullet";
		break;
	}
	state.sp->list_hanging = state.hanging;
	state.sp->list_indent = state.indent;
	state.hanging += LIST_INDENT;
	state.indent = state.hanging + LIST_INDENT;
	return 1;
}

static int
start_var(Tag_t* tp, Attribute_t* ap)
{
	NoP(tp);
	NoP(ap);
	sfputr(state.out, "{\\f3\\i", -1);
	state.sep = 1;
	return 1;
}

/*
 * generic tag end
 */

static int
end(Tag_t* tp, Attribute_t* ap)
{
	sfputc(state.out, '}');
	state.sep = 0;
	return 1;
}

/*
 * convert html file in to rtf file out
 */

#define COMMENT	1
#define PUN	4
#define STRING	2

static void
process(char* file, register Sfio_t* ip, register Sfio_t* op)
{
	register int	c;
	register int	lastc;
	register int	item;
	register int	cc;
	register int	tc;
	register char*	s;
	int		lastlastc;
	int		quote;
	int		n;
	Entity_t*	ep;
	Tag_t*		tp;
	Attribute_t	attributes[16];
	Attribute_t*	ap;
	Stack_t*	sp;

	error_info.file = file;
	error_info.line = 1;
	state.center = 0;
	state.in = ip;
	state.out = op;
	state.pre = 0;
	state.sp = state.sp_min;
	ap = 0;
	item = 0;
	lastc = 0;
	cc = tc = 0;
	for (;;)
	{
		switch (c = sfgetc(ip))
		{
		case EOF:
			goto done;
		case '<':
			if (!item)
			{
				item = c;
				lastlastc = lastc;
				quote = 0;
				ap = attributes;
				ap->name = 0;
				ap->value = 0;
				op = state.tmp;
				if ((c = sfgetc(ip)) != EOF)
				{
					sfungetc(ip, c);
					if (c == '!')
						quote |= COMMENT;
				}
				continue;
			}
			break;
		case '>':
			if (item == '<' && !(quote & STRING))
			{
				item = 0;
				if (!(s = sfstruse(op)))
					error(ERROR_SYSTEM|3, "out of space");
				op = state.out;
				if (*s == '!')
				{
					if ((cc -= strlen(s)) <= 0)
					{
						cc = 0;
						if ((c = sfgetc(ip)) != EOF)
						{
							if (c == '\n')
								error_info.line++;
							else
								sfungetc(ip, c);
						}
					}
					continue;
				}
				(ap + 1)->name = 0;
				for (;;)
				{
					ap->name = s + (((unsigned int)ap->name) >> PUN);
					if (!*ap->name)
						ap->name = 0;
					else if (ap->value)
					{
						ap->value = s + (((unsigned int)ap->value) >> PUN);
						if (!*ap->value)
							ap->value = 0;
					}
					if (ap == attributes)
						break;
					ap--;
				}
				if (c = *s == '/')
					s++;
				if (!(tp = (Tag_t*)hashget(state.tags, s)))
					error(1, "<%s>: unknown tag", s);
				else if (!c)
				{
					if (tp->end)
					{
						if (state.sp >= state.sp_max)
						{
							c = state.sp - state.sp_min;
							n = (state.sp_max - state.sp_min + 1) * 2;
							if (!(state.sp_min = oldof(state.sp_min, Stack_t, n, 0)))
								error(ERROR_SYSTEM|3, "out of space [tag stack]");
							state.sp_max = state.sp_min + n - 1;
							state.sp = state.sp_min + c;
						}
						state.sp++;
						state.sp->tag = tp;
						state.sp->line = error_info.line;
						state.sp->flags = 0;
						if (tp->flags & TAG_IGNORE)
						{
							state.sp->title_cc = cc;
							state.sp->title_lastlastc = lastlastc;
							state.sp->title_op = op;
							state.sp->title_tc = tc;
							op = state.nul;
							sfstrseek(op, 0, SEEK_SET);
						}
					}
					if (tp->start && !(*tp->start)(tp, ap) && tp->end)
						state.sp->flags |= STK_NOEND;
				}
				else
				{
					sp = state.sp;
					if (state.sp->tag != tp)
					{
						for (;;)
						{
							if (sp == state.sp_min)
							{
								if (!(tp->flags & TAG_UNBALANCED))
									error(1, "</%s> has no matching <%s>", tp->name, tp->name);
								sp = 0;
								break;
							}
							if (sp->tag == tp)
								break;
							sp--;
						}
						if (sp)
						{
							while (state.sp > sp)
							{
								if (state.sp->tag->end && !(state.sp->flags & STK_NOEND))
								{
									if (!(state.sp->tag->flags & TAG_UNBALANCED))
										error(1, "<%s> on line %d has no matching </%s>", state.sp->tag->name, state.sp->line, state.sp->tag->name);
									(*state.sp->tag->end)(state.sp->tag, NiL);
								}
								state.sp--;
							}
						}
					}
					if (sp)
					{
						if (tp->end && !(state.sp->flags & STK_NOEND))
							(*tp->end)(tp, ap);
						if (tp->flags & TAG_IGNORE)
						{
							cc = state.sp->title_cc;
							lastlastc = state.sp->title_lastlastc;
							op = state.sp->title_op;
							tc = state.sp->title_tc;
						}
						state.sp--;
					}
				}
				ap = 0;
				lastc = lastlastc;
				continue;
			}
			break;
		case '=':
			if (ap && !ap->value)
			{
				sfputc(op, 0);
				ap->value = (char*)(sfstrtell(op) << PUN);
				continue;
			}
			break;
		case '"':
			if (ap)
			{
				quote ^= STRING;
				if (!(quote & COMMENT))
					continue;
			}
			break;
		case '&':
			if (!item)
			{
				item = c;
				op = state.tmp;
				continue;
			}
			break;
		case ';':
			if (item == '&')
			{
				item = 0;
				if (!(s = sfstruse(op)))
					error(ERROR_SYSTEM|3, "out of space");
				op = state.out;
				if (*s == '#')
				{
					n = (int)strtol(s + 1, NiL, 10) & 0377;
					cc += sfprintf(op, "\\'%02x", n);
					tc++;
					if (isspace(n))
						lastc = ' ';
				}
				else if (ep = (Entity_t*)hashget(state.entities, s))
				{
					cc += sfputr(op, ep->value, -1);
					tc++;
					if (ep->flags & ENT_SPACE)
						lastc = ' ';
				}
				else
				{
					error(1, "&%s;: unknown entity reference", s);
					cc += sfprintf(op, "&%s;", s);
					tc++;
				}
				continue;
			}
			break;
		case '{':
		case '}':
		case '\\':
			sfputc(op, '\\');
			cc++;
			state.sep = 0;
			break;
		case '\n':
			error_info.line++;
			if (state.pre && !item)
			{
				state.sep = 0;
				sfputr(op, "\\line", -1);
				cc += 5;
				tc = 0;
				break;
			}
			/*FALLTHROUGH*/
		case ' ':
		case '\t':
		case '\v':
			if (ap)
			{
				if (!quote)
				{
					if (lastc != ' ' && ap < &attributes[elementsof(attributes) - 1])
					{
						sfputc(op, 0);
						ap++;
						ap->name = (char*)(sfstrtell(op) << PUN);
						ap->value = 0;
						lastc = ' ';
					}
					continue;
				}
			}
			else if (!state.pre)
			{
				if (lastc == ' ')
					continue;
				c = ' ';
				if (cc >= 72)
				{
					cc = 0;
					sfputc(op, '\n');
				}
			}
			else if (c == ' ')
			{
				sfputr(op, "\\~", -1);
				cc += 2;
				tc++;
				state.sep = 0;
				continue;
			}
			else if (c == '\t')
			{
				do
				{
					sfputr(op, "\\~", -1);
					cc += 2;
					tc++;
				} while (tc % 8);
				state.sep = 0;
				continue;
			}
			break;
		default:
			if (iscntrl(c))
				continue;
			if (c > 0177)
			{
				cc += sfprintf(op, "\\'%02x", c & 0377);
				tc++;
				continue;
			}
			break;
		}
		if (state.sep && op == state.out)
		{
			state.sep = 0;
			if (c != ' ')
			{
				sfputc(op, ' ');
				cc++;
				tc++;
			}
		}
		lastc = c;
		sfputc(op, c);
		cc++;
		tc++;
	}
 done:
	while (state.sp > state.sp_min)
	{
		error(1, "<%s> on line %d has no matching </%s>", state.sp->tag->name, state.sp->line, state.sp->tag->name);
		state.sp--;
	}
	error_info.file = 0;
	error_info.line = 0;
}

/*
 * return 1 if project file must be updated
 */

static int
project_update(const char* s, char* v, void* h)
{
	NoP(s);
	return v == (char*)h;
}

/*
 * list project file names
 */

static int
project_list(const char* s, char* v, void* h)
{
	NoP(v);
	sfputr((Sfio_t*)h, s, '\n');
	return 0;
}

/*
 * create/update help project file
 */

static void
project(char* file)
{
	register char*	s;
	Sfio_t*		fp;

	if (state.files)
	{
		if (fp = sfopen(NiL, file, "r"))
		{
			while (s = sfgetr(fp, '\n', 1))
			{
				if (*s == '[' && !strncasecmp(s, "[FILES]", 7))
				{
					while ((s = sfgetr(fp, '\n', 1)) && *s != '[')
						hashput(state.files, s, &state);
					if (!s)
						break;
				}
				sfputr(state.tmp, s, '\n');
			}
			sfclose(fp);
			if (!(s = sfstruse(state.tmp)))
				error(ERROR_SYSTEM|3, "out of space");
		}
		else
			s = "\
[OPTIONS]\n\
COMPRESS=TRUE\n\
REPORT=ON\n\
TITLE=Manual\n\
";
		if (hashwalk(state.files, 0, project_update, state.files))
		{
			if (!(fp = sfopen(NiL, file, "w")))
				error(ERROR_SYSTEM|2, "%s: cannot write", file);
			else
			{
				sfputr(fp, s, -1);
				sfputr(fp, "[FILES]", '\n');
				hashwalk(state.files, 0, project_list, fp);
				sfclose(fp);
			}
		}
	}
}

/*
 * html to rtf entity reference map
 */

static const Entity_t entities[] =
{
	"AElig",	"\\'c6",	0,
	"Aacute",	"\\'c1",	0,
	"Acirc",	"\\'c2",	0,
	"Agrave",	"\\'c0",	0,
	"Aring",	"\\'c5",	0,
	"Atilde",	"\\'c3",	0,
	"Auml",		"\\'c4",	0,
	"Ccedil",	"\\'c7",	0,
	"ETH",		"\\'d0",	0,
	"Eacute",	"\\'c9",	0,
	"Ecirc",	"\\'ca",	0,
	"Egrave",	"\\'c8",	0,
	"Euml",		"\\'cb",	0,
	"Iacute",	"\\'cd",	0,
	"Icirc",	"\\'ce",	0,
	"Igrave",	"\\'cc",	0,
	"Iuml",		"\\'cf",	0,
	"Ntilde",	"\\'d1",	0,
	"Oacute",	"\\'d3",	0,
	"Ocirc",	"\\'d4",	0,
	"Ograve",	"\\'d2",	0,
	"Oslash",	"\\'d8",	0,
	"Otilde",	"\\'d5",	0,
	"Ouml",		"\\'d6",	0,
	"THORN",	"\\'de",	0,
	"Uacute",	"\\'da",	0,
	"Ucirc",	"\\'db",	0,
	"Ugrave",	"\\'d9",	0,
	"Uuml",		"\\'dc",	0,
	"Yacute",	"\\'dd",	0,
	"aacute",	"\\'e1",	0,
	"acirc",	"\\'e2",	0,
	"acute",	"\\'b4",	0,
	"aelig",	"\\'e6",	0,
	"agrave",	"\\'e0",	0,
	"amp",		"&",		0,
	"aring",	"\\'e5",	0,
	"atilde",	"\\'e3",	0,
	"auml",		"\\'e4",	0,
	"brvbar",	"\\'a6",	0,
	"ccedil",	"\\'e7",	0,
	"cedil",	"\\'b8",	0,
	"cent",		"\\'a2",	0,
	"copy",		"\\'a9",	0,
	"curren",	"\\'a4",	0,
	"deg",		"\\'b0",	0,
	"divide",	"\\'f7",	0,
	"eacute",	"\\'e9",	0,
	"ecirc",	"\\'ea",	0,
	"egrave",	"\\'e8",	0,
	"emdash",	"\\emdash",	0,
	"emspace",	"\\emspace",	ENT_SPACE,
	"endash",	"\\endash",	0,
	"enspace",	"\\enspace",	ENT_SPACE,
	"eth",		"\\'f0",	0,
	"euml",		"\\'eb",	0,
	"frac12",	"\\'bd",	0,
	"frac14",	"\\'bc",	0,
	"frac34",	"\\'be",	0,
	"gt",		">",		0,
	"iacute",	"\\'ed",	0,
	"icirc",	"\\'ee",	0,
	"iexcl",	"\\'a1",	0,
	"igrave",	"\\'ec",	0,
	"iquest",	"\\'bf",	0,
	"iuml",		"\\'ef",	0,
	"laquo",	"\\'ab",	0,
	"lt",		"<",		0,
	"macr",		"\\'af",	0,
	"micro",	"\\'b5",	0,
	"middot",	"\\bullet",	0,
	"nbsp",		"\\~",		ENT_SPACE,
	"not",		"\\'ac",	0,
	"ntilde",	"\\'f1",	0,
	"oacute",	"\\'f3",	0,
	"ocirc",	"\\'f4",	0,
	"ograve",	"\\'f2",	0,
	"ordf",		"\\'aa",	0,
	"ordm",		"\\'ba",	0,
	"oslash",	"\\'f8",	0,
	"otilde",	"\\'f5",	0,
	"ouml",		"\\'f6",	0,
	"para",		"\\'b6",	0,
	"plusmn",	"\\'b1",	0,
	"pound",	"\\'a3",	0,
	"quot",		"\"",		0,
	"raquo",	"\\'bb",	0,
	"reg",		"\\'ae",	0,
	"sect",		"\\'a7",	0,
	"shy",		"\\'ad",	0,
	"sup1",		"\\'b9",	0,
	"sup2",		"\\'b2",	0,
	"sup3",		"\\'b3",	0,
	"szlig",	"\\'df",	0,
	"thorn",	"\\'fe",	0,
	"times",	"\\'d7",	0,
	"uacute",	"\\'fa",	0,
	"ucirc",	"\\'fb",	0,
	"ugrave",	"\\'f9",	0,
	"uml",		"\\'a8",	0,
	"uuml",		"\\'fc",	0,
	"yacute",	"\\'fd",	0,
	"yen",		"\\'a5",	0,
	"yuml",		"\\'ff",	0,
#if 0
	"trademark",	"",		0,
#endif
};

/*
 * html tag table
 */

static const Tag_t tags[] =
{
	"A",		start_a,	end_a,		0,0,
	"ADDRESS",	start_i,	end,		0,0,
	"B",		start_b,	end,		0,0,
	"BLOCKQUOTE",	start_bq,	end_bq,		0,0,
	"BQ",		start_bq,	end_bq,		0,0,
	"BODY",		start_body,	end,		0,0,
	"BR",		start_br,	0,		0,0,
	"CAPTION",	start_caption,	end,		0,0,
	"CENTER",	start_center,	end_center,	0,0,
	"CITE",		start_i,	end,		0,0,
	"CODE",		start_tt,	end,		0,0,
	"DD",		start_dd,	0,		0,0,
	"DIR",		start_ul,	end_LIST,	0,0,
	"DL",		start_dl,	end_LIST,	0,0,
	"DT",		start_dt,	0,		0,0,
	"EM",		start_i,	end,		0,0,
	"FN",		start_fn,	end_fn,		0,0,
	"FONT",		start_font,	end_font,	0,0,
	"H1",		start_H,	end_H,		0,0,
	"H2",		start_H,	end_H,		0,0,
	"H3",		start_H,	end_H,		0,0,
	"H4",		start_H,	end_H,		0,0,
	"H5",		start_H,	end_H,		0,0,
	"H6",		start_H,	end_H,		0,0,
	"HEAD",		start_head,	end_head,	0,TAG_UNBALANCED,
	"HR",		start_hr,	0,		0,0,
	"HTML",		start_html,	end_html,	0,0,
	"I",		start_i,	end,		0,0,
	"IMG",		start_img,	0,		0,0,
	"KBD",		start_tt,	end,		0,0,
	"LI",		start_li,	0,		0,TAG_UNBALANCED,
	"META",		start_meta,	0,		0,0,
	"MENU",		start_ul,	end_LIST,	0,0,
	"NULL",		0,		0,		0,0,
	"OL",		start_ol,	end_LIST,	0,0,
	"P",		start_p,	0,		0,TAG_UNBALANCED,
	"PRE",		start_pre,	end_pre,	0,0,
	"RENDER",	start_render,	0,		0,0,
	"SAMP",		start_tt,	end,		0,0,
	"STRONG",	start_b,	end,		0,0,
	"SUB",		start_sub,	end_sub,	0,0,
	"SUP",		start_sup,	end_sup,	0,0,
	"TABLE",	start_table,	end_table,	0,0,
	"TD",		start_td,	end_td,		0,0,
	"TH",		start_th,	0,		0,0,
	"TITLE",	start_title,	end_title,	0,TAG_IGNORE,
	"TR",		start_tr,	end_tr,		0,0,
	"TT",		start_tt,	end,		0,0,
	"UL",		start_ul,	end_LIST,	0,0,
	"UNKNOWN",	0,		0,		0,0,
	"VAR",		start_var,	end,		0,0,
};

/*
 * case insensitive hash
 */

static unsigned int
strcasehash(const char* s)
{
	register const unsigned char*	p = (const unsigned char*)s;
	register unsigned int		h = 0;
	register unsigned int		c;

	while (c = *p++)
	{
		if (isupper(c))
			c = tolower(c);
		HASHPART(h, c);
	}
	return h;
}

/*
 * initialize the global data
 */

static void
init(void)
{
	register int	i;

	if (!state.nul && !(state.nul = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [nul buffer]");
	if (!state.tmp && !(state.tmp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [tmp buffer]");
	i = 1024;
	if (!(state.sp_min = oldof(NiL, Stack_t, i, 0)))
		error(ERROR_SYSTEM|3, "out of space [tag stack]");
	state.sp_max = state.sp_min + i - 1;
	if (!(state.entities = hashalloc(NiL, HASH_name, "entities", 0)))
		error(ERROR_SYSTEM|3, "out of space [entity hash]");
	if (!(state.tags = hashalloc(NiL, HASH_compare, strcasecmp, HASH_hash, strcasehash, HASH_name, "tags", 0)))
		error(ERROR_SYSTEM|3, "out of space [tag hash]");
	if (state.project && !(state.files = hashalloc(state.tags, HASH_set, HASH_ALLOCATE, HASH_name, "files", 0)))
		error(ERROR_SYSTEM|3, "out of space [file hash]");
	for (i = 0; i < elementsof(entities); i++)
		if (!(hashput(state.entities, entities[i].name, &entities[i])))
			error(ERROR_SYSTEM|3, "out of space [entity hash put]");
	for (i = 0; i < elementsof(tags); i++)
		if (!(hashput(state.tags, tags[i].name, &tags[i])))
			error(ERROR_SYSTEM|3, "out of space [tag hash put]");
	hashset(state.tags, HASH_ALLOCATE);
}

int
main(int argc, char** argv)
{
	register int		c;
	register char*		s;
	register char*		t;
	register char*		u;
	register Sfio_t*	ip;
	register Sfio_t*	op;

	NoP(argc);
	error_info.id = "html2rtf";
	state.fontsize = FONTSIZE;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			error_info.trace = -opt_info.num;
			continue;
		case 'f':
			state.fontsize = opt_info.num;
			continue;
		case 'p':
			state.project = opt_info.arg;
			continue;
		case 'v':
			state.verbose = 1;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	init();
	if (!*argv)
	{
		if (state.project)
			error(ERROR_SYSTEM|3, "%s: input files required when project file specified", state.project);
		process(NiL, sfstdin, sfstdout);
	}
	else while (s = *argv++)
	{
		if (ip = sfopen(NiL, s, "r"))
		{
			if (state.project)
			{
				if (!(t = strrchr(s, '/')))
					t = s;
				if (u = strrchr(t, '.'))
					c = u - t;
				else
					c = strlen(t);
				sfprintf(state.tmp, "%-.*s.rtf", c, t);
				if (!(u = sfstruse(state.tmp)))
					error(ERROR_SYSTEM|3, "out of space");
				if (!(op = sfopen(NiL, u, "w")))
				{
					error(ERROR_SYSTEM|2, "%s: cannot write", u);
					sfclose(ip);
					continue;
				}
				hashput(state.files, u, state.files);
				while (c = *t++)
					sfputc(state.tmp, isalnum(c) ? c : '.');
				if (!(state.prefix = strdup(sfstruse(state.tmp))))
					error(ERROR_SYSTEM|3, "out of space");
			}
			else
			{
				state.prefix = "HTML2RTF";
				op = sfstdout;
			}
			process(s, ip, op);
			sfclose(ip);
			if (state.project)
			{
				sfclose(op);
				free(state.prefix);
			}
		}
		else error(ERROR_SYSTEM|2, "%s: cannot read", s);
	}
	if (state.project)
		project(state.project);
	exit(error_info.errors != 0);
}
