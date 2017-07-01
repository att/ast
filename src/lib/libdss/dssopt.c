/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
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
 * dss optget() support
 */

#include "dsshdr.h"

/*
 * format optget(3) description item on sp given plain text and item name
 * you're in unless you optout
 */

static int
optout(register Sfio_t* sp, const char* name, const char* type, const char* map, const char* ret, const char* s, const char* x)
{
	register const char*	t;
	register int		c;

	sfputc(sp, '[');
	sfputc(sp, '+');
	if (name)
	{
		if (ret)
			sfprintf(sp, "%s ", ret);
		for (t = name; c = *t++;)
		{
			if (c == ':')
				sfputc(sp, c);
			sfputc(sp, c);
		}
		if (type && !ret)
			sfputc(sp, ' ');
	}
	if (type)
	{
		sfprintf(sp, "(%s", type);
		if (map)
			sfprintf(sp, "::::%s", map);
		sfputc(sp, ')');
	}
	sfputc(sp, '?');
	if (s)
	{
		if (*s == '[')
			sfputr(sp, s + 1, -1);
		else if (optesc(sp, s, 0))
			return -1;
		if (x)
		{
			sfputc(sp, ' ');
			if (optesc(sp, x, 0))
				return -1;
		}
	}
	else
		sfputc(sp, ' ');
	if (!s || *s != '[')
		sfputc(sp, ']');
	return 0;
}

/*
 * structure type info output
 */

static int
optmem(register Sfio_t* sp, Cxtype_t* type)
{
	register Cxvariable_t*	mp;

	sfprintf(sp, "{\n");
	for (mp = (Cxvariable_t*)dtfirst(type->member->members); mp; mp = (Cxvariable_t*)dtnext(type->member->members, mp))
		if (optout(sp, mp->name, mp->type->name, NiL, NiL, mp->description, 0))
			return -1;
	sfprintf(sp, "}\n");
	return 0;
}

/*
 * type info output
 */

static int
opttype(Sfio_t* sp, register Cxtype_t* tp, int members)
{
	register const char*	x;
	register const char*	b;
	register const char*	d;
	register int		i;
	Sfio_t*			tmp = 0;

	if (tp->header.flags & CX_SCHEMA)
		x = "This is the main schema type.";
	else if (tp->member)
		x = "This structure type has the following members:";
	else if (tp->match)
		x = tp->match->description;
	else
		x = 0;
	if (tp->format.description)
	{
		b = !x ? "" : x[0] == '[' && x[1] == '+' && x[2] == '?' ? (x + 3) : x;
		if (tp->format.details)
			x = sfprints("%s The default value is \b%s\b. %s", tp->format.description, tp->format.details, b);
		else if (x)
			x = sfprints("%s %s", tp->format.description, b);
		else
			x = tp->format.description;
	}
	if (!(d = tp->description) && tp->generic && (tmp = sfstropen()))
	{
		sfprintf(tmp, "A generic type that maps to %s", tp->generic[0]->name);
		for (i = 1; tp->generic[i]; i++)
			sfprintf(tmp, "%s%s", tp->generic[i + 1] ? ", " : " or ", tp->generic[i]->name);
		sfprintf(tmp, " at runtime.");
		d = sfstruse(tmp);
	}
	i = optout(sp, tp->name, tp->base ? tp->base->name : (const char*)0, NiL, NiL, d, x);
	if (tmp)
		sfstrclose(tmp);
	if (i)
		return -1;
	if (!(tp->header.flags & CX_SCHEMA) && tp->member && optmem(sp, tp))
		return -1;
	else if (tp->base && tp->base->member && !(tp->base->header.flags & CX_REFERENCED) && opttype(sp, tp->base, 0))
		return -1;
	if (tp->generic)
		for (i = 0; tp->generic[i]; i++)
			if (!(tp->generic[i]->header.flags & CX_REFERENCED) && opttype(sp, tp->generic[i], 0))
				return -1;
	return 0;
}

/*
 * data map output
 */

static int
optmap(register Sfio_t* sp, Cxmap_t* map)
{
	size_t		n;
	Cxpart_t*	part;
	Cxitem_t*	item;

	sfprintf(sp, "{\n");
	if (map->shift)
		sfprintf(sp, "[+SHIFT=%u]", map->shift);
	if (~map->mask)
		sfprintf(sp, "[+MASK=0x%016llx]", map->mask);
	if (map->num2str)
	{
		if ((n = dtsize(map->num2str)) > 16)
			sfprintf(sp, "[+----- %u entries omitted -----]", n);
		else
			for (item = (Cxitem_t*)dtfirst(map->num2str); item; item = (Cxitem_t*)dtnext(map->num2str, item))
				if (item->mask == item->value)
					sfprintf(sp, "[+%s?0x%016llx]", item->name, item->value);
				else
					sfprintf(sp, "[+%s?%llu]", item->name, item->value);
	}
	else
		for (part = map->part; part; part = part->next)
		{
			if (part->shift)
				sfprintf(sp, "[+SHIFT=%u]", part->shift);
			if (~part->mask)
				sfprintf(sp, "[+MASK=0x%016llx]", part->mask);
			for (item = part->item; item; item = item->next)
			{
				if (item->mask == item->value)
					sfprintf(sp, "[+%s?0x%016llx]", item->name, item->value);
				else
					sfprintf(sp, "[+%s?%llu]", item->name, item->value);
				if (item->map && optmap(sp, item->map))
					return -1;
			}
		}
	sfprintf(sp, "}\n");
	return 0;
}

/*
 * optget() info discipline function
 */

int
dssoptinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	Dssdisc_t*	disc = ((Dssoptdisc_t*)dp)->disc;
	Dssstate_t*	state = dssstate(disc);
	Dsslib_t*	lib;
	Dssmeth_t*	meth;
	Dssformat_t*	format;
	Cx_t*		cx;
	Cxmap_t*	mp;
	Cxtype_t*	tp;
	Cxvariable_t*	vp;
	long		pos;
	int		all;
	int		head;
	char		name[64];

	switch (*s)
	{
	case 'd':
		if (*(s + 1) == 'e' && *(s + 2) == 't')
		{
			/* details */
			tp = (Cxtype_t*)((Dssoptdisc_t*)dp)->header;
			if (tp->format.description && optesc(sp, tp->format.description, 0))
				return -1;
		}
		else
		{
			/* description */
			sfprintf(sp, "%s", state->cx->header ? state->cx->header->description : "unknown-description");
		}
		return 0;
	case 'i':
		/* ident|index */
		if (state->cx->header)
			sfprintf(sp, "%s - %s", state->cx->header->name, state->cx->header->description);
		else
			sfprintf(sp, "unknown - description");
		return 0;
	case 'm':
		if (*(s + 1) == 'a')
		{
			/* match */
			tp = (Cxtype_t*)((Dssoptdisc_t*)dp)->header;
			if (tp->match && optesc(sp, tp->match->description, 0))
				return -1;
		}
		else
		{
			/* methods */
			for (lib = dsslib(NiL, DSS_VERBOSE, disc); lib; lib = (Dsslib_t*)dtnext(state->cx->libraries, lib))
				if (lib->meth && optout(sp, lib->meth->name, NiL, NiL, NiL, lib->meth->description, NiL))
					return -1;
		}
		return 0;
	case 'n':
		/* name */
		sfprintf(sp, "%s", state->cx->header ? state->cx->header->name : "unknown-name");
		return 0;
	case 'p':
		/* default print format */
		if (state->meth && state->meth->print)
			sfprintf(sp, "\"%s\"", state->meth->print);
		else
			sfprintf(sp, "undefined");
		return 0;
	case 't':
		/* type */
		sfputc(sp, '{');
		if (opttype(sp, (Cxtype_t*)((Dssoptdisc_t*)dp)->header, 1))
			return -1;
		sfputc(sp, '}');
		return 0;
	}
	if (!(meth = state->meth) && !(meth = state->global))
	{
		sfprintf(sp, "[+NOTE::?Specify \b--method\b=\amethod\a for a list of supported \b%s\b.]", s);
		return 0;
	}
	pos = sfstrtell(sp);
	head = 0;
	switch (*s)
	{
	case 'f':
		if (*(s + 1) != 'i')
		{
			/* formats */
			if ((head = !!strchr(s, ' ')) && meth->description && optout(sp, meth->name, NiL, NiL, NiL, meth->description, NiL))
				return -1;
			if (meth->formats && dtsize(meth->formats))
			{
				if (optout(sp, "----- formats -----", NiL, NiL, NiL, NiL, NiL))
					return -1;
				for (format = (Dssformat_t*)dtfirst(meth->formats); format; format = (Dssformat_t*)dtnext(meth->formats, format))
					if (optout(sp, format->name, NiL, NiL, NiL, format->description, NiL))
						return -1;
			}
			if (!head)
				break;
		}
		/*FALLTHROUGH*/
	case 'v':
		/* fields|variables */
		all = !state->meth || meth->cx && (!meth->cx->fields || !dtsize(meth->cx->fields));
		if (head)
			head = 0;
		else if (meth->description && optout(sp, meth->name, NiL, NiL, NiL, meth->description, NiL))
			return -1;
		if (cx = meth->cx)
		{
			for (tp = (Cxtype_t*)dtfirst(cx->types); tp; tp = (Cxtype_t*)dtnext(cx->types, tp))
				if (all || (tp->base || tp->match) && (tp->header.flags & CX_REFERENCED))
				{
					if (!head)
					{
						if (optout(sp, "----- data types -----", NiL, NiL, NiL, NiL, NiL))
							return -1;
						head = 1;
					}
					if (opttype(sp, tp, 0))
						return -1;
				}
			head = 0;
			for (mp = (Cxmap_t*)dtfirst(cx->maps); mp; mp = (Cxmap_t*)dtnext(cx->maps, mp))
				if (all || (mp->header.flags & CX_REFERENCED))
				{
					if (!head)
					{
						if (optout(sp, "----- data maps -----", NiL, NiL, NiL, NiL, NiL))
							return -1;
						head = 1;
					}
					if (optout(sp, mp->name, NiL, NiL, NiL, mp->description, NiL))
						return -1;
					if (optmap(sp, mp))
						return -1;
				}
			if (all && cx->variables)
			{
				head = 0;
				for (vp = (Cxvariable_t*)dtfirst(cx->variables); vp; vp = (Cxvariable_t*)dtnext(cx->variables, vp))
					if (vp->prototype)
					{
						if (!head)
						{
							if (optout(sp, "----- functions -----", NiL, NiL, NiL, NiL, NiL))
								return -1;
							head = 1;
						}
						if (optout(sp, vp->name, vp->prototype, NiL, vp->type->name, vp->description, NiL))
							return -1;
					}
			}
		}
		head = 0;
		if (!all)
			for (vp = (Cxvariable_t*)dtfirst(cx->fields); vp; vp = (Cxvariable_t*)dtnext(cx->fields, vp))
			{
				if (!head)
				{
					if (optout(sp, "----- data fields -----", NiL, NiL, NiL, NiL, NiL))
						return -1;
					head = 1;
				}
				if (optout(sp, vp->name, vp->type->name, vp->format.map ? vp->format.map->name : (char*)0, NiL, vp->description, NiL))
					return -1;
			}
		else if (meth->data)
			for (vp = (Cxvariable_t*)meth->data; vp->name; vp++)
			{
				if (!head)
				{
					if (optout(sp, "----- data fields -----", NiL, NiL, NiL, NiL, NiL))
						return -1;
					head = 1;
				}
				sfsprintf(name, sizeof(name), ".%s", vp->name);
				if (optout(sp, name, (char*)vp->type, vp->format.map ? vp->format.map->name : (char*)0, NiL, vp->description, NiL))
					return -1;
			}
		break;
	}
	if (sfstrtell(sp) == pos)
		sfprintf(sp, "[+NOTE::?Specify a method schema to list the %s.]", s);
	return 0;
}

/*
 * generate lib info usage
 */

int
dssoptlib(Sfio_t* sp, Dsslib_t* lib, const char* usage, Dssdisc_t* disc)
{
	register int	i;
	register char*	s;
	Dssstate_t*	state = dssstate(disc);

	if (lib->libraries)
		for (i = 0; lib->libraries[i]; i++)
			if (!dssload(lib->libraries[i], disc))
				return -1;
	if (dssadd(lib, disc))
		return -1;
	if (lib->description && (s = strchr(lib->description, '[')))
	{
		sfprintf(sp, "%s[+PLUGIN?%s - %-.*s]\n", s, lib->name, s - lib->description, lib->description);
	}
	else
		sfprintf(sp, "[-1ls5Pp0?][+PLUGIN?%s - %s]\n", lib->name, lib->description ? lib->description : "support library");
	if (usage)
		sfprintf(sp, "%s", usage);
	if (lib->meth)
	{
		if (!state->meth)
			state->meth = lib->meth;
		if (!usage || strncmp(usage, "[+DESCRIPTION?", 13))
			sfprintf(sp, "[+DESCRIPTION?The %s method handles %s.]\n", lib->meth->name, lib->meth->description);
		sfprintf(sp, "{\fformats and variables\f}\n\n--method=%s[,option...]\n\n", lib->meth->name);
	}
	if (lib->types)
	{
		sfprintf(sp, "[+TYPES]{\n");
		for (i = 0; lib->types[i].name; i++)
			if (opttype(sp, &lib->types[i], 1))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->maps)
	{
		sfprintf(sp, "[+MAPS]{\n");
		for (i = 0; lib->maps[i]; i++)
			if (optout(sp, lib->maps[i]->name, NiL, NiL, NiL, lib->maps[i]->description, NiL))
				return -1;
			else if (optmap(sp, lib->maps[i]))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->callouts)
	{
		sfprintf(sp, "[+CALLOUTS]{\n");
		for (i = 0; lib->callouts[i].callout; i++)
			if (optout(sp, cxopname(lib->callouts[i].op.code, lib->callouts[i].op.type1, lib->callouts[i].op.type2), NiL, NiL, NiL, lib->callouts[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->recodes)
	{
		sfprintf(sp, "[+RECODES]{\n");
		for (i = 0; lib->recodes[i].recode; i++)
			if (optout(sp, cxopname(lib->recodes[i].op.code, lib->recodes[i].op.type1, lib->recodes[i].op.type2), NiL, NiL, NiL, lib->recodes[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->queries)
	{
		sfprintf(sp, "[+QUERIES]{\n");
		for (i = 0; lib->queries[i].name; i++)
			if (optout(sp, lib->queries[i].name, NiL, NiL, NiL, lib->queries[i].description, lib->queries[i].method ? sfprints("Limited to methods matching \"%s\".", lib->queries[i].method) : (char*)0))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->constraints)
	{
		sfprintf(sp, "[+CONSTRAINTS]{\n");
		for (i = 0; lib->constraints[i].name; i++)
			if (optout(sp, lib->constraints[i].name, NiL, NiL, NiL, lib->constraints[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->edits)
	{
		sfprintf(sp, "[+EDITS]{\n");
		for (i = 0; lib->edits[i].name; i++)
			if (optout(sp, lib->edits[i].name, NiL, NiL, NiL, lib->edits[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->functions)
	{
		sfprintf(sp, "[+FUNCTIONS]{\n");
		for (i = 0; lib->functions[i].name; i++)
			if (optout(sp, lib->functions[i].name, lib->functions[i].prototype, NiL, (lib->functions[i].header.flags & CX_INITIALIZED) ? lib->functions[i].type->name : (char*)lib->functions[i].type, lib->functions[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	return 0;
}
