/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
 * C expression library value map support
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "cxlib.h"

typedef struct Frame_s
{
	struct Frame_s*		prev;
	Dt_t*			str2num;
} Frame_t;

/*
 * dt string ignorecase comparison
 */

static int
ignorecase(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	return strcasecmp((char*)a, (char*)b);
}

/*
 * cxinitmap helper
 */

static int
initmap(Frame_t* frame, Cxmap_t* map, Cxdisc_t* disc)
{
	register Cxpart_t*	part;
	register Cxitem_t*	item;
	Frame_t			top;
	Frame_t*		fp;
	int			easy;
	Cxunsigned_t		masks;

	static Dtdisc_t		num2strdisc;
	static Dtdisc_t		str2numdisc;
	static Dtdisc_t		stricase2numdisc;

	if (map->header.flags & CX_INITIALIZED)
		return -1;
	map->header.flags |= CX_INITIALIZED;
	for (;;)
	{
		if (!map->mask)
			map->mask = ~map->mask;
		if (!map->map)
			break;
		map = map->map;
	}
	if (!map->part)
		return 0;
	if (map->str2num)
	{
		if (!frame)
			return 0;
		easy = 0;
	}
	else
	{
		str2numdisc.link = offsetof(Cxitem_t, str2num);
		str2numdisc.key = offsetof(Cxitem_t, name);
		str2numdisc.size = -1;
		if (map->header.flags & CX_IGNORECASE)
		{
			stricase2numdisc = str2numdisc;
			stricase2numdisc.comparf = ignorecase;
			map->str2num = dtopen(&stricase2numdisc, Dtoset);
		}
		else
			map->str2num = dtopen(&str2numdisc, Dtoset);
		if (!map->str2num)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		top.prev = frame;
		top.str2num = map->str2num;
		frame = &top;
		easy = 1;
	}
	for (part = map->part; part; part = part->next)
	{
		if (part->mask)
			easy = 0;
		else
			part->mask = ~part->mask;
		masks = part->item ? part->item->mask : 0;
		for (item = part->item; item; item = item->next)
		{
			for (fp = frame; fp; fp = fp->prev)
				dtinsert(fp->str2num, item);
			if (item->mask != masks)
				part->flags |= CX_ALL;
			if (item->mask)
				easy = 0;
			else
				item->mask = ~item->mask;
			if (item->map)
			{
				if (initmap(frame, item->map, disc))
					return -1;
				easy = 0;
			}
		}
	}
	if (easy)
	{
		num2strdisc.link = offsetof(Cxitem_t, num2str);
		num2strdisc.key = offsetof(Cxitem_t, value);
		num2strdisc.size = sizeof(Cxunsigned_t);
		if (!(map->num2str = dtopen(&num2strdisc, Dtoset)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		part = map->part;
		map->shift += part->shift;
		map->mask |= part->mask;
		for (item = part->item; item; item = item->next)
			dtinsert(map->num2str, item);
	}
	return 0;
}

/*
 * initialize map
 */

int
cxinitmap(Cxmap_t* map, Cxdisc_t* disc)
{
	return initmap(NiL, map, disc);
}

/*
 * cxnum2str helper
 */

static int
num2str(Cx_t* cx, Cxmap_t* map, Sfio_t* sp, Cxunsigned_t num, int del)
{
	register Cxpart_t*	part;
	register Cxitem_t*	item;
	register Cxedit_t*	edit;
	Cxunsigned_t		n;
	char*			s;
	int			v;
	int			p;
	int			r;
	regmatch_t		match[10];
	char			buf[64];

	for (;;)
	{
		num >>= map->shift;
		num &= map->mask;
		if (!map->map)
			break;
		map = map->map;
	}
	if (map->num2str && (item = (Cxitem_t*)dtmatch(map->num2str, &num)))
	{
		sfprintf(sp, "%c%s", del, item->name);
		return 1;
	}
	r = 0;
	for (part = map->part; part; part = part->next)
	{
		n = num;
		n >>= part->shift;
		n &= part->mask;
		v = !(part->flags & CX_ALL);
		p = r;
		for (item = part->item; item; item = item->next)
			if ((n & item->mask) == item->value)
			{
				if (item->name)
				{
					sfprintf(sp, "%c%s", del, item->name);
					r++;
				}
				if (item->map)
					r += num2str(cx, item->map, sp, n, del);
				if (v)
					break;
			}
		if (r == p && part->num2str)
		{
			buf[0] = 0;
			for (edit = part->num2str; edit; edit = edit->next)
				if (!edit->num2strf)
				{
					if (!buf[0])
						sfsprintf(buf, sizeof(buf), "%lld", n);
					if (!regexec(&edit->re, buf, elementsof(match), match, 0) && !regsubexec(&edit->re, buf, elementsof(match), match))
					{
						sfprintf(sp, "%c%s", del, edit->re.re_sub->re_buf);
						r++;
						break;
					}
				}
				else if (s = (*edit->num2strf)(cx, n, cx->disc))
				{
					sfprintf(sp, "%c%s", del, s);
					r++;
					break;
				}
		}
	}
	return r;
}

/*
 * map number to string
 */

int
cxnum2str(Cx_t* cx, Cxformat_t* format, Cxunsigned_t num, char** p)
{
	char*	s;
	int	del;

	if (format->map)
	{
		if ((del = format->delimiter) == -1)
			del = '|';
		if (!num2str(cx, format->map, cx->tp, num, del))
			return -1;
	}
	else
		sfprintf(cx->tp, "|%I*u", sizeof(num), num);
	if (!(s = sfstruse(cx->tp)))
		return -1;
	if (p)
		*p = s + 1;
	return 0;
}

/*
 * cxstr2num helper
 */

static int
str2num(Cx_t* cx, Cxmap_t* map, const char* str, Cxunsigned_t* num)
{
	register Cxpart_t*	part;
	register Cxedit_t*	edit;
	regmatch_t		match[10];

	for (part = map->part; part; part = part->next)
		for (edit = part->str2num; edit; edit = edit->next)
			if (edit->str2numf)
			{
				if (!(*edit->str2numf)(cx, str, strlen(str), num, cx->disc))
					return 1;
			}
			else if (!regexec(&edit->re, str, elementsof(match), match, 0) && !regsubexec(&edit->re, str, elementsof(match), match))
			{
				*num = strtoull(edit->re.re_sub->re_buf, NiL, 0);
				return 1;
			}
	return 0;
}

/*
 * map string to number
 */

int
cxstr2num(Cx_t* cx, Cxformat_t* format, const char* str, size_t siz, Cxunsigned_t* p)
{
	register char*	s;
	register char*	b;
	int		del;
	Dt_t*		dt;
	Cxitem_t*	item;
	Cxunsigned_t	n;
	Cxunsigned_t	m;

	if (!format->map)
		return -1;
	del = format->delimiter;
	dt = format->map->str2num;
	n = 0;
	sfwrite(cx->tp, str, siz);
	if (!(s = sfstruse(cx->tp)))
		return -1;
	while (*s)
	{
		for (b = s; *s && *s != del && *s != '|' && *s != '+'; s++);
		if (*s)
			*s++ = 0;
		if (item = (Cxitem_t*)dtmatch(dt, b))
			n |= item->value;
		else if (str2num(cx, format->map, b, &m))
			n |= m;
		else
			return -1;
	}
	if (p)
		*p = n;
	return 0;
}

/*
 * apply edit substitutions in edit to r
 */

int
cxsub(Cx_t* cx, Cxedit_t* edit, Cxoperand_t* r)
{
	Cxtype_t*	type;
	regmatch_t	match[10];

	if (!cxisstring(r->type))
	{
		type = r->type;
		if (cxcast(cx, r, NiL, cx->state->type_string, NiL, NiL))
			return -1;
	}
	else
		type = 0;
	if (!regnexec(&edit->re, r->value.string.data, r->value.string.size, elementsof(match), match, 0) && !regsubexec(&edit->re, r->value.string.data, elementsof(match), match))
		r->value.string.size = strlen(r->value.string.data = edit->re.re_sub->re_buf);
	if (type && cxcast(cx, r, NiL, type, NiL, NiL))
		return -1;
	return 0;
}

/*
 * apply edit substitutions in part to r
 */

int
cxsuball(Cx_t* cx, Cxpart_t* part, Cxoperand_t* r)
{
	while (part)
	{
		if (part->edit && cxsub(cx, part->edit, r))
			return -1;
		part = part->next;
	}
	return 0;
}
