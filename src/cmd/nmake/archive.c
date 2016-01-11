/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2011 AT&T Intellectual Property          *
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
 * make archive access routines
 */

#include "make.h"

#include <ardir.h>

static int		ntouched;	/* count of touched members	*/

/*
 * return the update command for the named archive
 */

char*
arupdate(char* name)
{
	Ardir_t*	ar;
	char*		update;

	update = "$(RANLIB) $(<)";
	if (!state.regress)
	{
		if (!(ar = ardiropen(name, NiL, ARDIR_LOCAL)))
			return 0;
		if (streq(ar->meth->name, "local"))
			update = "($(RANLIB|\":\") $(<)) >/dev/null 2>&1 || true";
		else if (!(ar->flags & ARDIR_RANLIB))
			update = 0;
		ardirclose(ar);
	}
	return update;
}

/*
 * walk through an archive
 * d==0 updates the modify time of preselected members (see artouch())
 * else each member is recorded using addfile()
 */

static int
walkar(register Ardir_t* ar, Dir_t* d, char* name)
{
	register Ardirent_t*	ent;
	register Rule_t*	r;

	if (d)
	{
		putar(d->name, d);
		d->truncate = ar->truncate;
	}
	while (ent = ardirnext(ar))
	{
		if (d)
		{
			if ((Seconds_t)ent->mtime > (Seconds_t)ar->st.st_mtime)
				message((-1, "member %s is newer than archive %s", ent->name, name));
			addfile(d, ent->name, ((r = staterule(RULE, NiL, ent->name, -1)) && ent->mtime == tmxsec(r->time)) ? r->time : tmxsns(ent->mtime, 0));
		}
		else if ((r = getrule(ent->name)) && r->status == TOUCH)
		{
			ent->mtime = CURSECS;
			ardirchange(ar, ent);
			r->status = EXISTS;
			staterule(RULE, r, NiL, 1)->time = r->time = tmxsns(ent->mtime, 0);
			state.savestate = 1;
			if (!state.silent)
				error(0, "touch %s/%s", name, ent->name);
			ntouched--;
		}
	}
	return 0;
}

/*
 * check for any untouched r->status==TOUCH members
 */

static int
chktouch(const char* s, char* v, void* h)
{
	Rule_t*		r = (Rule_t*)v;

	NoP(s);
	NoP(h);
	if (r->status == TOUCH)
	{
		r->status = FAILED;
		error(1, "archive member %s not touched", r->name);
	}
	return 0;
}

/*
 * touch the modify time of an archive member (and the archive itself!)
 */

void
artouch(char* name, register char* member)
{
	register Rule_t*	r;
	Ardir_t*		ar;

	if (member)
	{
		if (!(r = getrule(member)))
			error(PANIC, "%s[%s] not scanned", name, member);
		else
		{
			r->status = TOUCH;
			ntouched++;
		}
	}
	else if (ar = ardiropen(name, NiL, ARDIR_LOCAL|ARDIR_UPDATE))
	{
		walkar(ar, NiL, name);
		if (ardirclose(ar))
			error(1, "error touching archive %s", name);
		if (ntouched > 0)
		{
			message((-2, "checking %d untouched members in %s", ntouched, name));
			hashwalk(table.rule, 0, chktouch, NiL);
		}
		ntouched = 0;
	}
}

/*
 * scan archive r and record all its entries
 */

void
arscan(Rule_t* r)
{
	Ardir_t*	ar;
	Dir_t*		d;

	if (r->dynamic & D_scanned)
		return;
	r->dynamic |= D_scanned;
	if (r->property & P_state)
		r->dynamic &= ~D_entries;
	else if (!(d = unique(r)))
		r->dynamic |= D_entries;
	else if (r->scan >= SCAN_USER)
	{
		debug((-5, "scan aggregate %s", r->name));
		d->archive = 1;
		state.archive = d;
		scan(r, NiL);
		state.archive = 0;
		r->dynamic |= D_entries;
	}
	else if (ar = ardiropen(r->name, NiL, ARDIR_LOCAL))
	{
		debug((-5, "scan archive %s", r->name));
		d->archive = 1;
		if (walkar(ar, d, r->name))
			r->dynamic |= D_entries;
		else
			r->dynamic &= ~D_entries;
		if (ardirclose(ar))
			error(1, "%s: archive scan error", r->name);
	}
	else
		debug((-5, "arscan(%s) failed", r->name));
}
