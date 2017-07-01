/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2012 AT&T Intellectual Property          *
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
 * mam support routines
 */

#include "make.h"

/*
 * mam special pathcanon()
 */

char*
mamcanon(char* path)
{
	register char*	s;

	s = path + strlen(path);
	while (s > path + 1 && *(s - 1) == '.' && *(s - 2) == '/')
	{
		for (s -= 2; s > path + 1 && *(s - 1) == '/'; s--);
		*s = 0;
	}
	return s;
}

/*
 * output dynamic mam error message
 */

ssize_t
mamerror(int fd, const void* b, size_t n)
{
	register char*	s = (char*)b;
	register char*	e;
	char*		t;

	if (state.mam.level > 0)
	{
		if (e = strchr(s, ':'))
		{
			for (s = e; *++s && *s == ' ';);
			n -= s - (char*)b;
		}
		switch (state.mam.level)
		{
		case ERROR_WARNING:
			t = "warning";
			break;
		case ERROR_PANIC:
			t = "panic";
			break;
		default:
			t = 0;
			break;
		}
		if (!t)
			t = "error";
		sfprintf(state.mam.out, "%sinfo %s %-.*s", state.mam.label, t, n, s);
		if (state.mam.regress)
			return 0;
	}
	return (fd == sffileno(sfstderr) && error_info.write == write) ? sfwrite(sfstderr, b, n) : write(fd, b, n);
}

/*
 * translate mam target name
 */

char*
mamname(register Rule_t* r)
{
	char*		a;
	char*		s;
	Stat_t		st;

	if (r->property & P_state)
		return r->name;
	if (state.mam.dynamic && (r->dynamic & D_alias))
		r = makerule(r->name);
	if (state.mam.statix)
	{
		a = localview(r);
		if ((s = call(makerule(external.mamname), a)) && !streq(a, s))
			a = s;
	}
	else
		a = ((r->property & P_target) || !state.user) ? unbound(r) : (state.mam.regress || state.expandview) ? r->name : localview(r);
	if (state.mam.root && (*a == '/' || (r->dynamic & (D_entries|D_member|D_membertoo|D_regular)) || stat(r->name, &st)))
	{
		if (*a != '/')
			sfprintf(internal.nam, "%s/", internal.pwd);
		sfprintf(internal.nam, "%s", a);
		a = sfstruse(internal.nam);
		pathcanon(a, 0, 0);
		if (!strncmp(a, state.mam.root, state.mam.rootlen) && *(a + state.mam.rootlen) == '/')
			a += state.mam.rootlen + 1;
	}
	if (state.mam.regress && *a == '/')
		a = strrchr(a, '/') + 1;
	return a;
}

/*
 * push make|prev op
 * non-zero returned if matching done required
 */

int
mampush(Sfio_t* sp, register Rule_t* r, Flags_t flags)
{
	int	pop;

	if (strmatch(r->name, "${mam_*}"))
		return 0;
	pop = !(r->dynamic & D_built) || (flags & P_force);
	if (!state.mam.hold)
	{
		sfprintf(sp, "%s%s %s%s%s%s%s\n"
			, state.mam.label
			, pop ? "make" : "prev"
			, mamname(r)
			, (r->property & P_archive) ? " archive" : null
			, (flags & P_implicit) ? " implicit" : null
			, (flags & P_joint) ? " joint" : null
			, (r->property & P_state) ? " state" : null
			);
		if (pop && (state.mam.dynamic || state.mam.regress))
		{
			if (r->uname && strcmp(r->uname, mamname(r)))
				sfprintf(sp, "%sbind %s %s %s\n", state.mam.label, r->uname, timefmt(NiL, r->time), mamname(r));
			else
				sfprintf(sp, "%sbind %s %s\n", state.mam.label, mamname(r), timefmt(NiL, r->time));
		}
	}
	return pop;
}

/*
 * pop done op
 */

void
mampop(Sfio_t* sp, register Rule_t* r, Flags_t flags)
{
	Rule_t*		s;
	List_t*		p;

	if ((r->property & (P_joint|P_target)) == (P_joint|P_target) && r->prereqs->rule->prereqs->rule == r && mampush(sp, r->prereqs->rule, flags|P_joint|P_virtual) && !(r->prereqs->rule->property & P_target))
	{
		for (p = r->prereqs->rule->prereqs; p; p = p->next)
			if (mampush(sp, p->rule, flags))
				mampop(sp, p->rule, flags|P_joint);
		mampop(sp, r->prereqs->rule, flags|P_joint|P_virtual);
		r->prereqs->rule->property |= P_target;
	}
	if (!state.mam.hold)
	{
		s = staterule(RULE, r, NiL, 0);
		sfprintf(sp, "%sdone %s%s%s%s%s\n"
			, state.mam.label
			, mamname(r)
			, (r->property & P_dontcare) ? " dontcare" : null
			, (r->property & P_ignore) ? " ignore" : null
			, (flags & P_joint) ? " generated" : null
			, (flags & P_virtual) && !(r->property & P_state) && ((r->property & P_virtual) || !(r->dynamic & (D_entries|D_member|D_membertoo|D_regular)) && (!s || !s->time)) ? " virtual" : s && (s->dynamic & D_built) ? " generated" : null
			);
	}
}

/*
 * return mam output stream pointer for r
 */

Sfio_t*
mamout(register Rule_t* r)
{
	register Rule_t*	r0;
	register char*		s;

	if (!state.mam.out || !state.user || r == internal.empty)
		return 0;
	if (state.mam.regress)
		return (r->property & P_dontcare) && !state.mam.dontcare ? (Sfio_t*)0 : state.mam.out;
	if (r->property & (P_make|P_state))
		return 0;
	if (state.mam.dynamic)
		return state.mam.out;
	if ((r->property & (P_after|P_before|P_virtual)) || (r->property & P_dontcare) && !(state.mam.dontcare || r->prereqs || r->action && *r->action || (r0 = staterule(RULE, r, NiL, 0)) && (r0->dynamic & D_built) && r0->action && *r0->action || !(r->dynamic & D_global)))
		return 0;
	if (*(s = mamname(r)) == '/' || !r->time && *s != '$' && strchr(s, '/'))
		return 0;
	return state.mam.out;
}
