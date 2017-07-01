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

static const char usage[] =
"[-1s1I?\n@(#)$Id: jcm (AT&T Research) 2013-02-13 $\n]"
USAGE_LICENSE
"[+NAME?jcm - job control-M deck converter]"
"[+DESCRIPTION?\bjcm\b converts the control-M job scheduler decks named by"
"	the \afile\a operands to an \bnmake\b(1) makefile on the"
"	standard output. The standard input is read if no \afile\a"
"	operands are specified.]"
"[c:cards?List control card lines instead of the generated makefile on the"
"	standard output.]"
"[d:debug?Set the debug trace level. Higher levels produce more"
"	output.]#[level]"
"[h:comment?Include H card comments in the generated output.]"
"[i:initialize?Initialize variable values from \aidentifier\a=\avalue\a "
    "lines in \afile\a. \afile\a may contain # style comments.]:[file]"
"[I:index?Expand the first input condition to the condition plus "
    "\aindex\a-1 conditions suffixed by \b-%02d\b from 1 through "
    "\aindex\a-1.]#[index]"
"[l:lowercase?Convert job and library names to lower case.]"
"[p:portable?Convert event names for \b/bin/make\b portability.]"
"[v:verbose?Enable verbose tracing.]"
"[T:test?Implementation-specific test and tracing bitmask.]#[test-mask]"
"\n"
"\n[ file ... ]\n"
"\n"
"[+SEE ALSO?\bjcl\b(1), \bnmake\b(1)]"
;

#include <ast.h>
#include <cdt.h>
#include <ccode.h>
#include <ctype.h>
#include <error.h>
#include <jcl.h>
#include <vmalloc.h>

#define CARD		80

#define circular(p)	(strneq((p)->value, "$(", 2) && strneq((p)->value + 2, JCL_AUTO, sizeof(JCL_AUTO) - 1) && strneq((p)->value + sizeof(JCL_AUTO) + 1, (p)->name, strlen((p)->name)) && streq((p)->value + sizeof(JCL_AUTO) + strlen((p)->name) + 1, ")"))

struct Jcmcard_s; typedef struct Jcmcard_s Jcmcard_t;
struct Jcmevent_s; typedef struct Jcmevent_s Jcmevent_t;
struct Jcmjob_s; typedef struct Jcmjob_s Jcmjob_t;
struct Jcmlib_s; typedef struct Jcmlib_s Jcmlib_t;
struct Jcmlist_s; typedef struct Jcmlist_s Jcmlist_t;
struct Jcmset_s; typedef struct Jcmset_s Jcmset_t;
struct Jcmshout_s; typedef struct Jcmshout_s Jcmshout_t;
struct Jcmvar_s; typedef struct Jcmvar_s Jcmvar_t;

struct Jcmcard_s
{
	Jcmcard_t*	next;
	char		data[1];
};

struct Jcmvar_s
{
	Dtlink_t	link;
	unsigned long	dup;
	int		init;
	char*		value;
	char		name[64];
};

struct Jcmlib_s
{
	Dtlink_t	link;
	Jcmvar_t*	var;
	char		name[64];
};

struct Jcmset_s
{
	Dtlink_t	link;
	Jcmset_t*	next;
	char*		value;
	char		name[1];
};

struct Jcmshout_s
{
	Jcmshout_t*	next;
	int		when;
	int		pri;
	char		data[17];
	char		to[17];
	char		text[71];
};

struct Jcmjob_s
{
	Jcmlib_t*	memlib;
	Jcmlib_t*	overlib;
	Jcmlib_t*	doclib;
	Jcmset_t*	set;
	Jcmshout_t*	shout;
	int		namelen;
	int		relationship;
	char*		docmem;
	char*		name;
	char*		tag;
	char		base[1];
};

struct Jcmlist_s
{
	Jcmlist_t*	next;
	Jcmevent_t*	event;
};

struct Jcmevent_s
{
	Dtlink_t	link;
	unsigned long	dup;
	int		mark;
	char		date[5];
	Jcmlist_t*	reqs;
	Jcmlist_t*	raise;
	Jcmlist_t*	clear;
	Jcmjob_t*	job;
	char		name[1];
};

static struct State_s
{
	Vmalloc_t*	vm;
	Dt_t*		events;
	Dt_t*		libs;
	Dt_t*		set;
	Dt_t*		vars;
	Dtdisc_t	eventdisc;
	Dtdisc_t	libdisc;
	Dtdisc_t	setdisc;
	Dtdisc_t	vardisc;
	char*		data;
	char*		last;
	Jcmlib_t*	dummy;
	Jcmevent_t*	all;
	Sfio_t*		tmp;
	unsigned long	pseudo;
	unsigned long	test;
	int		cards;
	int		comment;
	int		index;
	int		lowercase;
	int		portable;
	int		verbose;
} state;

static void
nospace(void)
{
	error(ERROR_SYSTEM|3, "out of space");
}

static char*
stash(const char* s)
{
	char*	r;

	if (!(r = strdup(s)))
		nospace();
	return r;
}

static void
lower(register char* s)
{
	for (; *s; s++)
		if (isupper(*s))
			*s = tolower(*s);
}

static void
upper(register char* s)
{
	for (; *s; s++)
		if (islower(*s))
			*s = toupper(*s);
}

static char*
copy(register char* t, register const char* s, size_t n)
{
	register const char*	e = s + n;

	while (s < e && *s && *s != ' ')
		*t++ = *s++;
	*t = 0;
	return t;
}

static char*
card(Sfio_t* sp, register unsigned char* map)
{
	register char*	s;
	register char*	t;
	register int 	c;
	register int 	o;
	size_t		z;
	size_t		n;
	size_t		m;

	s = state.data;
	z = CARD;
	for (;;)
	{
		if (s >= state.last)
		{
			n = s - state.data;
			m = (state.last - state.data) + 32 * CARD;
			if (!(state.data = newof(state.data, char, m, 0)))
				nospace();
			state.last = state.data + m - CARD - 1;
			s = state.data + n;
		}
		if (sfread(sp, s, z) != z)
			break;
		error_info.line++;
		if (s == state.data)
			o = *(unsigned char*)s;
		ccmapstr(map, s, z);
		for (t = s, s += z; s > t && (*(s - 1) == ' ' || *(s - 1) == 0 || !isprint(*(s - 1))); s--);
		if ((c = sfgetc(sp)) == EOF)
			break;
		if (c != o)
		{
			sfungetc(sp, c);
			break;
		}
		if (ccmapchr(map, c) == 'T')
		{
			if ((c = sfgetc(sp)) == EOF)
			{
				sfungetc(sp, o);
				break;
			}
			if (ccmapchr(map, c) == '%')
			{
				sfungetc(sp, c);
				sfungetc(sp, o);
				break;
			}
		}
		z = CARD - 1;
	}
	if (s == state.data)
		return 0;
	*s = 0;
	while (s > state.data)
		if (!*--s)
			*s = ' ';
	return state.data;
}

static const char*
prefix(const char* s, const char* e, int d)
{
	register Jcmvar_t*	v;
	register int		n;
	Jcmvar_t		var;

	n = (e ? (e - s) : strlen(s)) + 1;
	if (n >= sizeof(var.name))
		n = sizeof(var.name);
	strlcpy(var.name, s, n);
	if (!(v = (Jcmvar_t*)dtprev(state.vars, &var)))
		v = (Jcmvar_t*)dtsearch(state.vars, &var);
	if (*v->name != *s)
		v = (Jcmvar_t*)dtnext(state.vars, v);
	for (; v && *v->name == *s; v = (Jcmvar_t*)dtnext(state.vars, v))
	{
		n = strlen(v->name);
		if (strneq(s, v->name, n))
			return s + n;
	}
	while (s < e && *s != d && isalnum(*s))
		s++;
	return s;
}

static char*
parameterize(register Sfio_t* sp, register const char* s, register const char* e, int append, int index)
{
	register int		c;
	register int		d;
	register const char*	t;

	if (e)
		d = ' ';
	else
	{
		e = s + strlen(s);
		d = 0;
	}
	while (s < e && (c = *s++) != d)
	{
		if (c == '~' && s < e && *s != d)
		{
			sfputr(sp, "$(" JCL_AUTO, -1);
			switch (c = *s++)
			{
			case '#':
				sfputr(sp, "pound", -1);
				break;
			case '@':
				sfputr(sp, "at", -1);
				break;
			case '&':
				sfputr(sp, "and", -1);
				break;
			case '!':
				sfputr(sp, "bang", -1);
				break;
			default:
				if (isalnum(c) && (t = prefix(s - 1, e, d)))
				{
					sfwrite(sp, s - 1, t - s + 1);
					s = t;
				}
				else
					sfprintf(sp, "special_%02x", c);
				break;
			}
			c = ')';
		}
		else if (c == '%' && s < e && *s == c)
		{
			sfputr(sp, "$(" JCL_AUTO, -1);
			for (s++; s < e && (c = *s) != d && (isalnum(c) || c == '$' && (c = '_')); s++)
				sfputc(sp, c);
			c = ')';
		}
		else if (state.portable && (c == '#' || c == '$' || c == ':'))
			c = '_';
		sfputc(sp, c);
	}
	if (index)
		sfprintf(sp, "-%02d", index);
	if (append)
	{
		if (sfputc(sp, 0) < 0)
			nospace();
		sfstrseek(sp, -1, SEEK_CUR);
		return sfstrbase(sp);
	}
	if (!(s = (const char*)sfstruse(sp)))
		nospace();
	return (char*)s;
}

static Jcmevent_t*
getevent(const char* s, const char* d, int uniq, int string, int index)
{
	register Jcmevent_t*	event;

	if (s)
	{
		s = (const char*)parameterize(state.tmp, s, string ? (const char*)0 : s + 20, 1, index);
		if (event = (Jcmevent_t*)dtmatch(state.events, s))
		{
			if (!uniq)
			{
				sfstrseek(state.tmp, 0, SEEK_SET);
				return event;
			}
			sfprintf(state.tmp, "{%lu}", ++event->dup);
		}
	}
	else
		sfprintf(state.tmp, "{%lu}", ++state.pseudo);
	if (!(s = (const char*)sfstruse(state.tmp)) || !(event = newof(0, Jcmevent_t, 1, strlen(s))))
		nospace();
	strcpy(event->name, s);
	dtinsert(state.events, event);
	if (d)
		copy(event->date, d, 4);
	return event;
}

static Jcmjob_t*
getjob(const char* s)
{
	register Jcmjob_t*	job;

	s = (const char*)parameterize(state.tmp, s, s + 8, 0, 0);
	if (!(job = newof(0, Jcmjob_t, 1, strlen(s))))
		nospace();
	strcpy(job->base, s);
	if (state.lowercase)
		lower(job->base);
	job->name = stash(parameterize(state.tmp, s, s + 27, 0, 0));
	if (state.lowercase)
		lower(job->name);
	return job;
}

static Jcmvar_t*
setvar(const char* s, const char* v, int init)
{
	register Jcmvar_t*	var;

	if (var = (Jcmvar_t*)dtmatch(state.vars, s))
	{
		if (!v || !init && var->init)
			return var;
		free(var->value);
	}
	else
	{
		if (!(var = newof(0, Jcmvar_t, 1, 0)))
			nospace();
		var->dup = 1;
		strcpy(var->name, s);
		dtinsert(state.vars, var);
	}
	var->init = init;
	var->value = stash(v ? v : "");
	return var;
}

static Jcmlib_t*
getlib(const char* s)
{
	register Jcmlib_t*	lib;
	register Jcmvar_t*	var;
	register char*		t;
	char			name[64];

	copy(name, s, 44);
	if (!*name)
		return 0;
	if (state.lowercase)
		lower(name);
	if (!(lib = (Jcmlib_t*)dtmatch(state.libs, name)))
	{
		if (!(lib = newof(0, Jcmlib_t, 1, 0)))
			nospace();
		strcpy(lib->name, name);
		dtinsert(state.libs, lib);
		if (!(t = strchr(lib->name, '%')) || *++t != '%')
		{
			if (!(t = strrchr(lib->name, '.')) || !*++t)
				t = lib->name;
			sfsprintf(name, sizeof(name), "lib_%s", t);
			if (state.lowercase)
				upper(name);
			if (var = (Jcmvar_t*)dtmatch(state.vars, name))
				do
				{
					sfsprintf(name, sizeof(name), "lib_%s_%lu", t, ++var->dup);
					if (state.lowercase)
						upper(name);
				} while (dtmatch(state.vars, name));
			lib->var = setvar(name, lib->name, 0);
		}
	}
	return lib;
}

static Jcmlist_t*
append(Jcmlist_t* list, Jcmevent_t* event)
{
	register Jcmlist_t*	p;
	register Jcmlist_t*	q;

	for (p = list; p; p = p->next)
	{
		if (p->event == event)
			return list;
		if (!p->next)
			break;
	}
	if (!(q = newof(0, Jcmlist_t, 1, 0)))
		nospace();
	q->event = event;
	if (p)
	{
		p->next = q;
		return list;
	}
	return q;
}

static void
assert(register Jcmjob_t* job, register Jcmlist_t* reqs, register Jcmlist_t* raise, register Jcmlist_t* clear, Jcmevent_t* group)
{
	register Jcmevent_t*	event;

	event = getevent(job ? sfprints("JOB-%s", job->name) : NiL, NiL, 1, 1, 0);
	event->job = job;
	event->reqs = reqs;
	event->raise = raise;
	event->clear = clear;
	if (group)
		group->reqs = append(group->reqs, event);
}

static int
init(const char* path)
{
	register char*	s;
	register char*	t;
	register char*	e;
	register int	i;
	Sfio_t*		sp;
	char*		file;
	size_t		line;

	if (!(sp = sfopen(NiL, path, "r")))
	{
		error(ERROR_SYSTEM|2, "%s: cannot read initialization file", path);
		return -1;
	}
	file = error_info.file;
	error_info.file = (char*)path;
	line = error_info.line;
	error_info.line = 0;
	while (s = sfgetr(sp, '\n', 1))
	{
		error_info.line++;
		e = s + sfvalue(sp) - 1;
		while (isspace(*s))
			s++;
		if (!*s || *s == '#')
			continue;
		i = 0;
		for (t = s; *s; s++)
			if (*s == '=')
			{
				i = 1;
				*s++ = 0;
				break;
			}
			else if (isspace(*s) && !i)
			{
				i = -1;
				*s = 0;
			}
		if (i > 0 && isalpha(*t))
		{
			while (isspace(*s))
				s++;
			while (e > s && isspace(*(e - 1)))
				e--;
			*e = 0;
			if (strneq(t, JCL_AUTO, sizeof(JCL_AUTO) - 1))
				t += sizeof(JCL_AUTO) - 1;
			if (*t)
				setvar(t, s, 1);
		}
		else
			error(1, "invalid initialization line ignored: %s", t);
	}
	sfclose(sp);
	error_info.file = file;
	error_info.line = line;
	return 0;
}

static void
dump(Sfio_t* sp, register Jcmevent_t* event)
{
	register Jcmlist_t*	p;
	register Jcmset_t*	v;

	event->mark = 1;
	sfprintf(sp, "%s : .VIRTUAL", event->name);
	if (event->job)
	{
		if (event->job->memlib == state.dummy || event->job->overlib == state.dummy)
		{
			sfprintf(sp, " .FOREGROUND .DO.NOTHING");
			for (p = event->reqs; p; p = p->next)
				sfprintf(sp, " %s", p->event->name);
		}
		else
		{
			sfprintf(sp, " .JOB");
			for (v = event->job->set; v; v = v->next)
				if (v->value)
					sfprintf(sp, " %s%s=%s", JCL_AUTO, v->name, fmtquote(v->value, "$'", "'", strlen(v->value), 0));
				else
					sfprintf(sp, " (%s%s)", JCL_AUTO, v->name);
			for (p = event->reqs; p; p = p->next)
				sfprintf(sp, " %s", p->event->name);
			sfprintf(sp, " %s", event->job->base);
		}
	}
	else if (!event->reqs)
		sfprintf(sp, " .EVENT.WAIT");
	else
		for (p = event->reqs; p; p = p->next)
			sfprintf(sp, " %s", p->event->name);
	if (event->raise)
		for (p = event->raise; p; p = p->next)
			sfprintf(sp, " %s.RAISE", p->event->name);
	if (event->clear)
		for (p = event->clear; p; p = p->next)
			sfprintf(sp, " %s.CLEAR", p->event->name);
	sfprintf(sp, "\n");
	if (event->raise)
	{
		for (p = event->raise; p; p = p->next)
			sfprintf(sp, "%s.RAISE ", p->event->name);
		sfprintf(sp, ": .AFTER .EVENT.RAISE\n");
	}
	if (event->clear)
	{
		for (p = event->clear; p; p = p->next)
			sfprintf(sp, "%s.CLEAR ", p->event->name);
		sfprintf(sp, ": .AFTER .EVENT.CLEAR\n");
	}
	for (p = event->reqs; p; p = p->next)
		if (!p->event->mark)
			dump(sp, p->event);
}

int
main(int argc, char** argv)
{
	register char*		file;
	register char*		s;
	register char*		t;
	register Jcmcard_t*	cp;
	register Jcmevent_t*	ep;
	register Jcmevent_t*	group;
	register Jcmjob_t*	job;
	register Jcmlist_t*	clear;
	register Jcmlist_t*	raise;
	register Jcmlist_t*	reqs;
	Jcmcard_t*		firstcard;
	Jcmcard_t*		lastcard;
	Jcmset_t*		set;
	Jcmset_t*		lastset;
	Jcmset_t*		global;
	Jcmshout_t*		shout;
	Jcmshout_t*		lastshout;
	Jcmvar_t*		var;
	unsigned char*		map;
	Sfio_t*			sp;
	int			n;
	int			index;

	error_info.id = "jcm";
	if (!(state.vm = vmopen(Vmdcheap, Vmlast, 0)) || !(state.tmp = sfstropen()))
		nospace();
	state.eventdisc.link = offsetof(Jcmevent_t, link);
	state.eventdisc.key = offsetof(Jcmevent_t, name);
	state.libdisc.link = offsetof(Jcmlib_t, link);
	state.libdisc.key = offsetof(Jcmlib_t, name);
	state.setdisc.link = offsetof(Jcmset_t, link);
	state.setdisc.key = offsetof(Jcmset_t, name);
	state.vardisc.link = offsetof(Jcmvar_t, link);
	state.vardisc.key = offsetof(Jcmvar_t, name);
	if (!(state.events = dtopen(&state.eventdisc, Dtoset)) ||
	    !(state.libs = dtopen(&state.libdisc, Dtoset)) ||
	    !(state.set = dtopen(&state.setdisc, Dtoset)) ||
	    !(state.vars = dtopen(&state.vardisc, Dtoset)))
		nospace();
	index = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			state.cards = opt_info.number;
			continue;
		case 'd':
			error_info.trace = -opt_info.number;
			continue;
		case 'h':
			state.comment = 1;
			continue;
		case 'i':
			if (init(opt_info.arg))
				return 1;
			continue;
		case 'I':
			index = opt_info.number;
			continue;
		case 'l':
			state.lowercase = opt_info.number;
			continue;
		case 'p':
			state.portable = opt_info.number;
			continue;
		case 'v':
			state.verbose = opt_info.number;
			continue;
		case 'T':
			state.test |= opt_info.number;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE, "%s", optusage(NiL));
	state.dummy = getlib("DUMMY");
	state.all = getevent("all", NiL, 0, 1, 0);
	file = *argv;
	if (!state.cards)
	{
		sfprintf(sfstdout, ":JCL:\n\n");
		n = 0;
		for (var = (Jcmvar_t*)dtfirst(state.vars); var; var = (Jcmvar_t*)dtnext(state.vars, var))
			if (var->init)
			{
				sfprintf(sfstdout, "%s%s == %s\n", JCL_AUTO, var->name, var->value);
				n = 1;
			}
		if (n)
			sfprintf(sfstdout, "\n");
	}
	do
	{
		if (!file)
			sp = sfstdin;
		else if (!(sp = sfopen(NiL, file, "r")))
		{
			error(ERROR_SYSTEM|2, "%s: cannot read", file);
			continue;
		}
		else if (state.cards)
			sfprintf(sfstdout, "=== %s ===\n", file);
		else if (state.verbose)
			sfprintf(sfstderr, "=== %s ===\n", file);
		n = sfgetc(sp);
		sfungetc(sp, n);
		map = isupper(n) ? (unsigned char*)0 : ccmap(CC_EBCDIC_O, CC_NATIVE);
		error_info.file = file;
		error_info.line = 0;
		vmclear(state.vm);
		firstcard = lastcard = 0;
		while (s = card(sp, map))
		{
			if (*s == 'T' && (t = strchr(s + 3, '=')) && *++t == '~')
				setvar(t + 1, NiL, 0);
			if (!(cp = vmnewof(state.vm, 0, Jcmcard_t, 1, strlen(s))))
				nospace();
			strcpy(cp->data, s);
			if (lastcard)
				lastcard = lastcard->next = cp;
			else
				lastcard = firstcard = cp;
		}
		if (cp = firstcard)
		{
			group = state.all;
			job = 0;
			reqs = raise = clear = 0;
			do
			{
				s = cp->data;
				if (state.cards)
				{
					sfprintf(sfstdout, "%s\n", s);
					continue;
				}
				if (state.verbose)
					sfprintf(sfstderr, "%s\n", s);
				switch (s[0])
				{
				case 'B':
					/* XXX: unknown per-job sparse */
					break;
				case 'C':
					/* XXX: event control */
					break;
				case 'D':
				case 'J':
				case 'K':
				case 'W':
					/* XXX: cron info */
					break;
				case 'E':
					/* XXX: event relationships */
					break;
				case 'G':
					/* XXX: group */
					break;
				case 'H':
					if (state.comment)
						sfprintf(sfstdout, "# %s\n\n", parameterize(state.tmp, s+1, NiL, 0, 0));
					break;
				case 'I':
					if (job)
					{
						if (index)
						{
							s++;
							for (n = 0; n < index; n++)
								reqs = append(reqs, getevent(s, s+20, 0, 0, n));
							s += 23;
						}
						for (s++; *s; s += 24)
							reqs = append(reqs, getevent(s, s+20, 0, 0, 0));
					}
					break;
				case 'L':
					if (job)
						job->memlib = getlib(s+1);
					break;
				case 'M':
					if (job || reqs || raise || clear)
						assert(job, reqs, raise, clear, group);
					reqs = raise = clear = 0;
					job = getjob(s+1);
					job->relationship = s[64];
					if (s[60] == 'G')
					{
						ep = getevent(job->name, NiL, 1, 1, 0);
						group->reqs = append(group->reqs, ep);
						group = ep;
					}
					else if (group == state.all)
					{
						if (s = strrchr(file, '/'))
							s++;
						else
							s = file;
						if (t = strrchr(s, '.'))
							s = sfprints("%-.*s", t - s, s);
						ep = getevent(s, NiL, 1, 1, 0);
						group->reqs = append(group->reqs, ep);
						group = ep;
					}
					break;
				case 'N':
					if (job)
						job->tag = stash(parameterize(state.tmp, s+1, s+21, 0, 0));
					break;
				case 'O':
					if (job)
					{
						for (s++; *s; s += 25)
							if (s[24] == '-')
								clear = append(clear, getevent(s, s+20, 0, 0, 0));
							else
								raise = append(raise, getevent(s, s+20, 0, 0, 0));
					}
					break;
				case 'Q':
					if (state.verbose)
						for (s++; *s; s += 24)
							sfprintf(sfstdout, "Q='%-20.20s'\n", s);
					break;
				case 'R':
					/* XXX: unknown per-job sparse */
					break;
				case 'S':
					if (job)
					{
						if (lastshout = job->shout)
							while (lastshout->next)
								lastshout = lastshout->next;
						s++;
						while (*s)
						{
							if (!(shout = newof(0, Jcmshout_t, 1, 0)))
								nospace();
							if (lastshout)
								lastshout->next = shout;
							else
								job->shout = shout;
							lastshout = shout;
							shout->when = *s++;
							for (t = shout->data; !isalpha(*s); s++)
								if (t < &shout->data[sizeof(shout->data)-1])
									*t++ = *s;
							*t = 0;
							for (s += (copy(shout->to, s, 16) - shout->to); *s == ' '; s++);
							shout->pri = *s++;
							n = 0;
							while (*s >= '0' && *s <= '9')
								n = n * 10 + (*s++ - '0');
							if (n >= sizeof(shout->text))
								n = sizeof(shout->text) - 1;
							memcpy(shout->text, s, n);
							s += n;
						}
					}
					break;
				case 'T':
					if (job)
					{
						s = parameterize(state.tmp, s + 3, NiL, 0, 0);
						n = strlen(s) + 2;
						if (!(set = newof(0, Jcmset_t, 1, n)))
							nospace();
						strcpy(set->name, s);
						if (s = strchr(set->name, '='))
						{
							*s++ = 0;
							set->value = s;
						}
						else
							set->value = "";
						var = (Jcmvar_t*)dtmatch(state.vars, set->name);
						if ((!(var = (Jcmvar_t*)dtmatch(state.vars, set->name)) || !var->init) && (!(global = (Jcmset_t*)dtmatch(state.set, set->name)) || streq(global->value, set->value) || circular(set)))
						{
							if (!global)
							{
								if (!state.cards)
									sfprintf(sfstdout, "%s%s == %s\n", JCL_AUTO, set->name, circular(set) ? "1" : set->value);
								dtinsert(state.set, global = set);
								if (!(set = newof(0, Jcmset_t, 1, n)))
									nospace();
								memcpy(set, global, sizeof(Jcmset_t) + n);
							}
							set->value = 0;
						}
						if (lastset = job->set)
						{
							while (lastset->next)
								lastset = lastset->next;
							lastset->next = set;
						}
						else
							job->set = set;
					}
					break;
				case 'V':
					if (job)
						job->overlib = getlib(s+1);
					break;
				case 'Z':
					if (job)
					{
						job->docmem = stash(parameterize(state.tmp, s+1, s+9, 0, 0));
						job->doclib = getlib(s+9);
					}
					break;
				case '0':
					/* XXX: job modification owner/date */
					break;
				default:
					error(1, "%c: unknown op", s[0]);
					break;
				}
			} while (cp = cp->next);
			if (job || reqs || raise || clear)
				assert(job, reqs, raise, clear, group);
		}
		if (sp != sfstdin)
			sfclose(sp);
		error_info.file = 0;
		error_info.line = 0;
	} while (file && (file = *++argv));
	if (!state.cards)
	{
		sfprintf(sfstdout, "\n");
		dump(sfstdout, state.all);
	}
	return error_info.errors != 0;
}
