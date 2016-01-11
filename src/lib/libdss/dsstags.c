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
 * dss  { <DSS> <MAP> <CONSTRAINT> } implementation
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "dsslib.h"

static int
dss_method_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Dssmeth_t*	meth;

	if (!(meth = dssmeth(data, state->disc)))
		return -1;
	state->meth = meth;
	return 0;
}

static int
dss_field_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;

	if (fp->prev->data)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: only one <NAME> expected", tagcontext(tag, fp));
		return -1;
	}
	if (!(fp->prev->data = cxvariable(state->meth->cx, data, NiL, state->disc)))
		return -1;
	return 0;
}

static int
dss_field_map_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxmap_t*	map;

	if (!fp->prev->data)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: <NAME> expected", tagcontext(tag, fp));
		return -1;
	}
	if (!(map = cxmap(NiL, data, state->disc)))
		return -1;
	((Cxvariable_t*)fp->prev->data)->format.map = map;
	return 0;
}

static int
dss_field_con_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxconstraint_t*	con;

	if (!fp->prev->data)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: <NAME> expected", tagcontext(tag, fp));
		return -1;
	}
	if (!(con = cxconstraint(NiL, data, state->disc)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: constraint not defined", data);
		return -1;
	}
	((Cxvariable_t*)fp->prev->data)->format.constraint = con;
	return 0;
}

static Tags_t	tags_field[] =
{
	"NAME",		"Field name.",
			0,0,dss_field_name_dat,0,
	"MAP",		"Field map name.",
			0,0,dss_field_map_dat,0,
	"CONSTRAINT",	"Field constraint name.",
			0,0,dss_field_con_dat,0,
	0
};

static Tags_t*
dss_field_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	return &tags_field[0];
}

static int
dss_field_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	if (!fp->data)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: <NAME> expected", tagcontext(tag, fp));
		return -1;
	}
	return 0;
}

static int
dss_compress_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;

	if (!(state->meth->compress = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
dss_print_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;

	if (!(state->meth->print = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static Tags_t	tags_dss[] =
{
	"NAME",		"Field and map association name.",
			0,0,0,0,
	"DESCRIPTION",	"Field and map association description.",
			0,0,0,0,
	"IDENT",	"Field and map association ident string.",
			0,0,0,0,
	"METHOD",	"Method name; the same value as for the command"
			" line --method.",
			0,0,dss_method_dat,0,
	"FIELD",	"Field name with optional maps and constraints.",
			0,dss_field_beg,0,dss_field_end,
	"MAP",		"Field value map definition.",
			0,dss_map_beg,0,dss_map_end,
	"CONSTRAINT",	"Field value constraint definition.",
			0,dss_con_beg,0,0,
	"COMPRESS",	"Preferred compression method; compression is applied"
			" by the {compress} query.",
			0,0,dss_compress_dat,0,
	"PRINT",	"Default {print} query format.",
			0,0,dss_print_dat,0,
	0
};

static Tags_t*
dss_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	return &tags_dss[0];
}

static int
map_part_item_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	if (!(((Cxitem_t*)fp->prev->data)->name = (const char*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
map_part_item_mask_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Cxitem_t*)fp->prev->data)->mask = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
map_part_item_value_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	char*			e;
	Cxoperand_t		r;

	if (r.type = ((Cxpart_t*)fp->prev->prev->data)->type)
	{
		if ((*r.type->internalf)(state->meth->cx, r.type, NiL, NiL, &r, data, strlen(data), NiL, state->disc) < 0)
			return -1;
		((Cxitem_t*)fp->prev->data)->value = (Cxinteger_t)r.value.number;
	}
	else if (isdigit(*data) || *data == '-' || *data == '+')
	{
		((Cxitem_t*)fp->prev->data)->value = strtoll(data, &e, 0);
		if (*e)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
			return -1;
		}
	}
	else
		((Cxitem_t*)fp->prev->data)->value = *data;
	return 0;
}

static int
map_part_item_bit_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Cxitem_t*)fp->prev->data)->mask = ((Cxitem_t*)fp->prev->data)->value = ((Cxunsigned_t)1) << strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
map_part_item_map_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	if (dss_map_end(tag, fp, disc))
		return -1;
	((Cxitem_t*)fp->prev->data)->map = (Cxmap_t*)fp->data;
	return 0;
}

static Tags_t	tags_map_part_item[] =
{
	"NAME",		"Map item name.",
			0,0,map_part_item_name_dat,0,
	"MASK",		"A mask applied to this item value.",
			0,0,map_part_item_mask_dat,0,
	"VALUE",	"Map matching item value after masking.",
			0,0,map_part_item_value_dat,0,
	"BIT",		"The map item is a bit flag at this bit position;"
			" an alternative to an explicit mask and value.",
			0,0,map_part_item_bit_dat,0,
	"MAP",		"A map applied to the unshifted, unmasked value;"
			" either a map reference name or a map definition.",
			0,dss_map_beg,dss_map_dat,map_part_item_map_end,
	0
};

static Tags_t*
map_part_item_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	if (name)
	{
		if (!(fp->data = newof(0, Cxitem_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		if (fp->prev->tail)
			fp->prev->tail = ((Cxitem_t*)fp->prev->tail)->next = (Cxitem_t*)fp->data;
		else
			fp->prev->tail = ((Cxpart_t*)fp->prev->data)->item = (Cxitem_t*)fp->data;
	}
	return &tags_map_part_item[0];
}

static int
map_part_type_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxtype_t*	type;

	if (!(type = cxtype(state->meth->cx, data, state->disc)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: unknown type", data);
		return -1;
	}
	if (!cxisnumber(type))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: map values must be numeric", data);
		return -1;
	}
	if (!type->internalf)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: cannot convert string to map value type", data);
		return -1;
	}
	((Cxpart_t*)fp->prev->data)->type = type;
	return 0;
}

static int
map_part_shift_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Cxpart_t*)fp->prev->data)->shift = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
map_part_mask_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Cxpart_t*)fp->prev->data)->mask = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
map_part_edit_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxpart_t*	part = fp->prev->data;
	Cxedit_t*	edit;

	if (!(edit = cxedit(NiL, data, state->disc)))
		return -1;
	if (fp->tag->name[0] == 'n' || fp->tag->name[0] == 'N')
	{
		edit->next = part->num2str;
		part->num2str = edit;
	}
	else if (fp->tag->name[3] == 'n' || fp->tag->name[3] == 'N')
	{
		edit->next = part->str2num;
		part->str2num = edit;
	}
	else
	{
		edit->next = part->edit;
		part->edit = edit;
	}
	return 0;
}

static Tags_t	tags_map_part[] =
{
	"TYPE",		"Value type for this part. The base type must be"
			" number. The default is number.",
			0,0,map_part_type_dat,0,
	"SHIFT",	"Value right shift for this part.",
			0,0,map_part_shift_dat,0,
	"MASK",		"Value mask for this part.",
			0,0,map_part_mask_dat,0,
	"ITEM",		"Map item list.",
			0,map_part_item_beg,0,0,
	"NUM2STR",	"A number to string ed(1) style substitute expression"
			" that converts a numeric value to a string.",
			0,0,map_part_edit_dat,0,
	"STR2NUM",	"A number to string ed(1) style substitute expression"
			" that converts a string value to a number.",
			0,0,map_part_edit_dat,0,
	"EDIT",		"A string to string ed(1) style substitute expression"
			" that edits string values.",
			0,0,map_part_edit_dat,0,
	0
};

static Tags_t*
map_part_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	if (name)
	{
		if (!(fp->data = newof(0, Cxpart_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		if (fp->prev->tail)
			fp->prev->tail = ((Cxpart_t*)fp->prev->tail)->next = (Cxpart_t*)fp->data;
		else
			fp->prev->tail = ((Cxmap_t*)fp->prev->data)->part = (Cxpart_t*)fp->data;
	}
	return &tags_map_part[0];
}

static int
map_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxmap_t*		map;
	Cxmap_t*		ref;

	map = (Cxmap_t*)fp->prev->data;
	if (ref = cxmap(NiL, data, state->disc))
	{
		if (map->description || map->shift || map->mask || map->part || map->map)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "<%s> must be the first tag inside a <MAP> definition", fp->tag->name);
			return -1;
		}
		fp->prev->data = map = ref;
	}
	else if (!(map->name = (const char*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
map_description_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	if (!(((Cxmap_t*)fp->prev->data)->description = (const char*)strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

static int
map_shift_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Cxmap_t*)fp->prev->data)->shift = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
map_mask_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	((Cxmap_t*)fp->prev->data)->mask = strtoul(data, &e, 0);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static int
map_ignorecase_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	char*	e;

	if (strtol(data, &e, 0))
		((Cxmap_t*)fp->prev->data)->header.flags |= CX_IGNORECASE;
	else
		((Cxmap_t*)fp->prev->data)->header.flags &= ~CX_IGNORECASE;
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	return 0;
}

static Tags_t	tags_map[] =
{
	"NAME",		"Map name.",
			0,0,map_name_dat,0,
	"DESCRIPTION",	"Map description.",
			0,0,map_description_dat,0,
	"IDENT",	"Map ident string.",
			0,0,0,0,
	"SHIFT",	"Value right shift for all contained maps.",
			0,0,map_shift_dat,0,
	"MASK",		"Value mask for all contained maps.",
			0,0,map_mask_dat,0,
	"IGNORECASE",	"Ignore case in string match.",
			0,0,map_ignorecase_dat,0,
	"MAP",		"Map reference name to be applied here.",
			0,0,dss_map_dat,0,
	"PART",		"Map part list.",
			0,map_part_beg,0,0,
	0
};

/*
 * this is a library private global
 */

Tags_t*
dss_map_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	if (name && !(fp->data = newof(0, Cxmap_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	return &tags_map[0];
}

/*
 * this is a library private global
 */

int
dss_map_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxmap_t*		map;

	if (!(map = cxmap(NiL, data, state->disc)))
	{
		if (disc->errorf)
		{
			(*disc->errorf)(NiL, disc, 2, "%s: map not defined", data);
			return -1;
		}
	}
	((Cxmap_t*)fp->data)->map = map;
	return 0;
}

/*
 * this is a library private global
 */

int
dss_map_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxmap_t*	map = (Cxmap_t*)fp->data;
	Cxmap_t*	tmp;

	while (map->map && !map->shift && !map->mask && !map->part && !map->name)
	{
		tmp = map;
		map = map->map;
		free(tmp);
	}
	fp->data = map;
	return map->name && !(map->header.flags & CX_INITIALIZED) ? cxaddmap(NiL, map, state->disc) : 0;
}

static Cxconstraint_t*
constraint_def(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	Cxconstraint_t*	con;

	if (!(con = (Cxconstraint_t*)fp->prev->data) && !(fp->prev->data = con = newof(0, Cxconstraint_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	return con;
}

static int
constraint_name_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	Cxconstraint_t*		con;

	if (!(con = constraint_def(tag, fp, disc)))
		return -1;
	if (con->name)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: constraint already has a name", tagcontext(tag, fp));
		return -1;
	}
	if (!(con->name = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return cxaddconstraint(NiL, con, state->disc);
}

static int
constraint_expression_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Cxconstraint_t*		con;

	if (!(con = constraint_def(tag, fp, disc)))
		return -1;
	if (!(con->expression = strdup(data)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	error(1, "FIXME: %s: cxcomp() expression on variable name `.'", data);
	return -1;
}

static int
constraint_match_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Cxconstraint_t*		con;
	regdisc_t*		redisc;

	if (!(con = constraint_def(tag, fp, disc)))
		return -1;
	if (!(con->re = newof(0, regex_t, 1, sizeof(regdisc_t) + strlen(data))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	redisc = (regdisc_t*)(con->re + 1);
	strcpy((char*)(con->pattern = (const char*)(redisc + 1)), data);
	redisc->re_version = REG_VERSION;
	redisc->re_errorf = (regerror_t)disc->errorf;
	con->re->re_disc = redisc;
	return regcomp(con->re, data, REG_AUGMENTED|REG_DISCIPLINE) ? -1 : 0;
}

static int
constraint_max_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Cxconstraint_t*	con;
	Cxnumber_t	n;
	char*		e;

	if (!(con = constraint_def(tag, fp, disc)))
		return -1;
	n = strtod(data, &e);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	if (!(con->max = newof(0, Cxvalue_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	con->max->number = n;
	return 0;
}

static int
constraint_min_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Cxconstraint_t*	con;
	Cxnumber_t	n;
	char*		e;

	if (!(con = constraint_def(tag, fp, disc)))
		return -1;
	n = strtod(data, &e);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	if (!(con->min = newof(0, Cxvalue_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	con->min->number = n;
	return 0;
}

static int
constraint_def_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	Cxconstraint_t*	con;
	Cxnumber_t	n;
	char*		e;

	if (!(con = constraint_def(tag, fp, disc)))
		return -1;
	n = strtod(data, &e);
	if (*e)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: invalid number", data);
		return -1;
	}
	if (!(con->def = newof(0, Cxvalue_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	con->def->number = n;
	return 0;
}

static Tags_t	tags_constraint[] =
{
	"NAME",		"Constraint name.",
			0,0,constraint_name_dat,0,
	"EXPRESSION",	"Field value ``.'' constraint expression.",
			0,0,constraint_expression_dat,0,
	"MATCH",	"String field value constraint regular expression.",
			0,0,constraint_match_dat,0,
	"MAX",		"Numeric field constraint maximum value.",
			0,0,constraint_max_dat,0,
	"MIN",		"Numeric field constraint minimum value.",
			0,0,constraint_min_dat,0,
	"DEFAULT",	"Numeric field constraint default value.",
			0,0,constraint_def_dat,0,
	0
};

static int
method_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	fp->data = strdup(data);
	return 0;
}

static int
method_end(Tag_t* tag, Tagframe_t* fp, Tagdisc_t* disc)
{
	Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;
	char*		name = (char*)fp->data;
	Dssmeth_t*	meth;

	if (!name)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: method name expected", tagcontext(tag, fp));
		return -1;
	}
	if (!streq(name, state->meth->name))
	{
		sfseek(tagsync(tag), (Sfoff_t)0, SEEK_END);
		if (!(meth = dssmeth(sfprints("%s:%s", name, error_info.file), state->disc)))
		{
			free(name);
			return -1;
		}
		if (!meth->compress)
			meth->compress = state->meth->compress;
		state->meth = meth;
	}
	free(name);
	return 0;
}

/*
 * this is a library private global
 */

Tags_t*
dss_con_beg(Tag_t* tag, Tagframe_t* fp, const char* name, Tagdisc_t* disc)
{
	return &tags_constraint[0];
}

/*
 * this is a library private global
 */

int
dss_con_dat(Tag_t* tag, Tagframe_t* fp, const char* data, Tagdisc_t* disc)
{
	register Dsstagdisc_t*	state = (Dsstagdisc_t*)disc;

	if (fp->data)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: reference not expected", tagcontext(tag, fp));
		return -1;
	}
	if (!(fp->data = cxconstraint(NiL, data, state->disc)))
		return -1;
	return 0;
}

Tags_t	dss_tags[] =
{
	"DSS",		"Method and field map associations.",
			0,dss_beg,0,0,
	"MAP",		"Field value map;"
			" either a map reference name or a map definition.",
			0,dss_map_beg,dss_map_dat,dss_map_end,
	"METHOD",	"Method name; if different from the current method then"
			" the current file is reprocessed by the new method.",
			0,0,method_dat,method_end,
	"CONSTRAINT",	"Field value constraints. Constraints, when"
			" enabled, are applied to each record as it is read.",
			0,dss_con_beg,dss_con_dat,0,
	0
};

/*
 * read tags in ip
 */

Dssmeth_t*
dsstags(Sfio_t* ip, const char* path, int line, Dssflags_t flags, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Tag_t*		tag;
	Dsstagdisc_t	state;

	taginit(&state.tagdisc, disc->errorf);
	state.tagdisc.id = DSS_ID;
	state.disc = disc;
	state.meth = meth;
	if (!(tag = tagopen(ip, path, line, dss_tags, &state.tagdisc)) || tagclose(tag))
		state.meth = 0;
	if (!(flags & DSS_FILE_KEEP))
		sfclose(ip);
	return state.meth;
}
