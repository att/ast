/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * return the local attr attributes for host name
 */

#include "cslib.h"

#include <ctype.h>
#include <hash.h>

typedef struct
{
	HASH_HEADER;
	char		data[1];
} Info_t;

/*
 * load the info from the local info file
 */

static Hash_table_t*
load(register Cs_t* state)
{
	register char*		s;
	char*			h;
	register Info_t*	ip;
	register Sfio_t*	sp;
	register Hash_table_t*	tp;
	int			c;

	if (!(sp = csinfo(state, NiL, NiL))) return 0;
	if (tp = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "state->info", 0))
		while (s = sfgetr(sp, '\n', 1))
		{
			while (isspace(*s)) s++;
			h = s;
			while (!isspace(*s) && *s) s++;
			if (*s)
			{
				c = *s;
				*s++ = 0;
				if (!(ip = (Info_t*)hashlook(tp, h, HASH_CREATE|HASH_SIZE(sizeof(Info_t)+strlen(s)+1), NiL)))
				{
					hashfree(tp);
					tp = 0;
					break;
				}
				*--s = c;
				strcpy(ip->data, s);
			}
		}
	sfclose(sp);
	return tp;
}

/*
 * info cached on first call and return value placed in static buffer ...
 * if name==0 || name=="-" then all valid records returned by successive
 * 	csattr() calls until 0 return
 * if attr==0 || attr=="-" then all attributes returned
 */

char*
csattr(register Cs_t* state, const char* name, const char* attr)
{
	register char*		s;
	register char*		b;
	register char*		x;
	register char*		v;
	register Info_t*	ip;
	int			n;
	unsigned long		addr;

	static Hash_table_t*	tp;
	static Hash_position_t*	pt;
	static char		buf[256];

	messagef((state->id, NiL, -8, "attr(%s,%s) call", name, attr));
	if (!tp && !(tp = load(state)))
		return 0;
	if (attr && (!*attr || *attr == '-' && !*(attr + 1)))
		attr = 0;
	b = buf;
	x = &buf[sizeof(buf) - 1];
	if (!name || *name == '-' && !*(name + 1))
	{
		name = 0;
	scan:
		if (!pt && !(pt = hashscan(tp, 0)))
			return 0;
		n = attr && streq(attr, "name");
		do if (!(ip = (Info_t*)hashnext(pt)))
		{
			hashdone(pt);
			pt = 0;
			return 0;
		} while (n && streq(ip->name, CS_HOST_LOCAL));
		if (n)
			return ip->name;
		if (attr && streq(attr, "*"))
			return buf;
		b += sfsprintf(b, x - b, "%s", ip->name);
	}
	else
	{
		if (streq(name, CS_HOST_LOCAL))
			name = (const char*)csname(state, 0);
		if (!(ip = (Info_t*)hashlook(tp, name, HASH_LOOKUP, NiL)))
			return 0;
		if (attr)
		{
			if (streq(attr, "*"))
				return buf;
			if (streq(attr, "name"))
				return csaddr(state, ip->name) ? state->host : ip->name;
			if (streq(attr, "addr") || streq(attr, "host"))
			{
				if (addr = csaddr(state, ip->name))
					return *attr == 'a' ? csntoa(state, addr) : state->host;
				return CS_HOST_UNKNOWN;
			}
		}
	}
	if (!attr)
	{
		v = ip->data;
		if (b == buf) while (isspace(*v)) v++;
		b += sfsprintf(b, x - b, "%s", v);
		if (addr = csaddr(state, ip->name))
		{
			if (!streq(ip->name, state->host))
				b += sfsprintf(b, x - b, " host=%s", state->host);
			b += sfsprintf(b, x - b, " addr=%s", csntoa(state, addr));
		}
	}
	else if (streq(attr, "addr") || streq(attr, "host"))
		b += sfsprintf(b, x - b, " %s", (addr = csaddr(state, ip->name)) ? (*attr == 'a' ? csntoa(state, addr) : state->host) : CS_HOST_UNKNOWN);
	else for (v = ip->data;;)
	{
		while (isspace(*v)) v++;
		if (!*v)
		{
			if (!name)
			{
				b = buf;
				goto scan;
			}
			return 0;
		}
		for (s = (char*)attr; *s && *v == *s++; v++);
		if (!*s && (*v == '=' || !*v || isspace(*v)))
		{
			if (*v == '=') v++;
			else v = "1";
			if (b > buf && b < x) *b++ = ' ';
			while (b < x && !isspace(*v) && (*b++ = *v++));
			break;
		}
		while (*v && !isspace(*v)) v++;
	}
	*b = 0;
	return buf;
}

char*
_cs_attr(const char* name, const char* attr)
{
	return csattr(&cs, name, attr);
}
