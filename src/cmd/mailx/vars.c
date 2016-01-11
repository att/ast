/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2012 The Regents of the University of California an*
*                                                                      *
* Redistribution and use in source and binary forms, with or           *
* without modification, are permitted provided that the following      *
* conditions are met:                                                  *
*                                                                      *
*    1. Redistributions of source code must retain the above           *
*       copyright notice, this list of conditions and the              *
*       following disclaimer.                                          *
*                                                                      *
*    2. Redistributions in binary form must reproduce the above        *
*       copyright notice, this list of conditions and the              *
*       following disclaimer in the documentation and/or other         *
*       materials provided with the distribution.                      *
*                                                                      *
*    3. Neither the name of The Regents of the University of California*
*       names of its contributors may be used to endorse or            *
*       promote products derived from this software without            *
*       specific prior written permission.                             *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND               *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,          *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF             *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             *
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS    *
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,             *
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED      *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,        *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON    *
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,      *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY       *
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE              *
* POSSIBILITY OF SUCH DAMAGE.                                          *
*                                                                      *
* Redistribution and use in source and binary forms, with or without   *
* modification, are permitted provided that the following conditions   *
* are met:                                                             *
* 1. Redistributions of source code must retain the above copyright    *
*    notice, this list of conditions and the following disclaimer.     *
* 2. Redistributions in binary form must reproduce the above copyright *
*    notice, this list of conditions and the following disclaimer in   *
*    the documentation and/or other materials provided with the        *
*    distribution.                                                     *
* 3. Neither the name of the University nor the names of its           *
*    contributors may be used to endorse or promote products derived   *
*    from this software without specific prior written permission.     *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS"    *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A      *
* PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS    *
* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT     *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF     *
* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  *
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT   *
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF   *
* SUCH DAMAGE.                                                         *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                                 gsf                                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Mail -- a mail program
 *
 * Variable handling stuff.
 */

#include "mailx.h"

/*
 * Initialize the variables before command line options and rc sourcing.
 */

void
varinit(void)
{
	register const struct var*	vp;
	register int			c;
	register char*			s;
	register char*			t;
	register FILE*			fp;
	int				i;
	char				buf[LINESIZE];

	static const char*		domains[] = { _PATH_RESCONF };

	state.version = fmtident(state.version);
	setscreensize();
	for (vp = state.vartab; vp->name; vp++) {
		if ((vp->flags & E) && (s = getenv(vp->name)) && *s)
			varset(vp->name, s);
		else if (vp->initialize)
			varset(vp->name, vp->initialize);
	}

	/*
	 * Get the local host name.
	 */

#if _lib_gethostname
	if (!gethostname(buf, sizeof(buf)))
		state.var.hostname = varkeep(buf);
#endif

	/*
	 * Get the local domain name.
	 */

	if ((s = strchr(state.var.hostname, '.')) && *++s)
		state.var.domain = varkeep(s);
	else
		for (i = 0; i < sizeof(domains) / sizeof(domains[0]); i++) {
			s = (char*)domains[i];
			do {
				if (fp = fileopen(s, "r")) {
					while (s = fgets(buf, sizeof(buf), fp)) {
						while (isspace(*s))
							s++;
						if (((c = *s++) == 'd' || c == 'D') &&
						    ((c = *s++) == 'o' || c == 'O') &&
						    ((c = *s++) == 'm' || c == 'M') &&
						    ((c = *s++) == 'a' || c == 'A') &&
						    ((c = *s++) == 'i' || c == 'M') &&
						    ((c = *s++) == 'n' || c == 'N') &&
						    (isspace(c = *s++) || c == ':' || c == '=')) {
							while (isspace(*s))
								s++;
							t = s;
							while (*t && !isspace(*t))
								t++;
							while (t > s && *(t - 1) == '.')
								t--;
							if (*t)
								*t = 0;
							if (*s)
							{
								state.var.domain = varkeep(s);
								break;
							}
						}
					}
					fileclose(fp);
					i = sizeof(domains) / sizeof(domains[0]);
					break;
				}
			} while (!i && (s = strchr(s + 1, '/')));
		}

	/*
	 * Interactive.
	 */

	if (isatty(0))
		state.var.interactive = state.on;
}

/*
 * Copy a variable value into permanent (ie, not collected after each
 * command) space.  Do not bother to alloc space for "".
 */

char*
varkeep(const char* val)
{
	char*	p;
	int	len;

	if (!val)
		return 0;
	if (!*val)
		return state.on;
	len = strlen(val) + 1;
	if (!(p = (char*)malloc(len)))
		note(PANIC, "Out of space");
	memcpy(p, val, len);
	stresc(p);
	return p;
}

/*
 * List variable settings.
 */

int
varlist(int all)
{
	register const struct var*	vp;

	for (vp = state.vartab; vp->name; vp++)
		if (*vp->variable) {
			if (all > 0 || !all && !(vp->flags & A)) {
				printf("%16s", vp->name);
				if (vp->flags & I)
					printf("=%ld\n", *((long*)vp->variable));
				else if (**vp->variable)
					printf("=\"%s\"\n", fmtesc(*vp->variable));
				else
					putchar('\n');
			}
		}
		else if (all) {
			sfprintf(state.path.temp, "no%s", vp->name);
			printf("%16s\n", struse(state.path.temp));
		}
	return 0;
}

/*
 * Set a variable value.
 */

int
varset(register const char* name, register const char* value)
{
	register struct var*	vp;
	char*			s;
	int			n;
	int			m;

	note(DEBUG, "set name=%s value=%s", name, value);
	if (name[0] == '-') {
		name += 1;
		value = 0;
	}
	else if (name[0] == 'n' && name[1] == 'o') {
		name += 2;
		value = 0;
	}
	for (;;) {
		if (!(vp = (struct var*)strsearch(state.vartab, state.varnum, sizeof(struct var), stracmp, name, NiL))) {
			if (state.sourcing)
				return 0;
			note(0, "\"%s\": unknown variable", name);
			return 1;
		}
		if (!(vp->flags & A))
			break;
		name = (const char*)vp->help;
	}
	if ((vp->flags & C) && !state.cmdline) {
		note(0, "\"%s\": can only set variable on command line", name);
		return 1;
	}
	if ((vp->flags & S) && state.sourcing) {
		note(0, "\"%s\": cannot set variable from script", name);
		return 1;
	}
	if (vp->flags & R) {
		note(0, "\"%s\": readonly variable", name);
		return 1;
	}
	else if (vp->flags & I) {
		if (!value)
			*((long*)vp->variable) = 0;
		else if (isdigit(*value) || *value == '-' || *value == '+')
			*((long*)vp->variable) = strtol(value, NiL, 0);
	}
	else {
		if (value && (*value || !(vp->flags & N))) {
			if ((vp->flags & L) && *value && *vp->variable && **vp->variable) {
				if (state.cmdline)
					return 0;
				m = strlen(*vp->variable);
				n = m + strlen(value) + 2;
				if (!(s = newof(0, char, n, 0)))
					note(PANIC, "Out of space");
				memcpy(s, *vp->variable, m);
				s[m] = '\n';
				strcpy(s + m + 1, value);
				value = (const char*)s;
			}
			else if (value != (char*)vp->initialize)
				value = varkeep(value);
		}
		else if (vp->flags & D)
			value = vp->initialize;
		else
			value = 0;
		if (*vp->variable && *vp->variable != state.on && *vp->variable != (char*)vp->initialize)
			free(*vp->variable);
		*vp->variable = (char*)value;
	}
	if (vp->set)
		(*vp->set)(vp, value);
	return 0;
}

/*
 * Get a variable value.
 */

char*
varget(register const char* name)
{
	register struct var*	vp;
	char*			s;

	for (;;) {
		if (!(vp = (struct var*)strsearch(state.vartab, state.varnum, sizeof(struct var), stracmp, name, NiL))) {
			if (s = getenv(name))
				return s;
			if (!state.sourcing)
				note(0, "\"%s\": unknown variable", name);
			return 0;
		}
		if (!(vp->flags & A))
			break;
		name = (const char*)vp->help;
	}
	if (vp->flags & I) {
		sfsprintf(state.number, sizeof(state.number), "%ld", *((long*)vp->variable));
		return state.number;
	}
	return *vp->variable;
}

/*
 * Trap unimplemented variable assignment.
 */

void
set_notyet(struct var* vp, const char* value)
{
	if (value) {
		note(0, "\"%s\": variable not implemented yet", vp->name);
		*vp->variable = 0;
	}
}

/*
 * Trap askbcc variable assignment.
 */

void
set_askbcc(struct var* vp, const char* value)
{
	if (value)
		state.askheaders |= GBCC;
	else
		state.askheaders &= ~GBCC;
}

/*
 * Trap askcc variable assignment.
 */

void
set_askcc(struct var* vp, const char* value)
{
	if (value)
		state.askheaders |= GCC;
	else
		state.askheaders &= ~GCC;
}

/*
 * Low level for askheaders and editheaders.
 */

static unsigned long
setheaders(struct var* vp, const char* value)
{
	register char*			s;
	register const char*		t;
	register char*			b;
	char*				p;
	register const struct lab*	lp;
	unsigned long			flags;

	flags = GTO;
	p = (char*)value;
	while (b = wordnext(&p, state.path.path))
		for (lp = state.hdrtab;; lp++) {
			if (!(t = lp->name)) {
				note(0, "\"%s %s\": unknown header field", vp->name, b);
				break;
			}
			s = b;
			if (upper(*s) == *t) {
				while (lower(*++s) == *++t);
				if (!*s && *t == ':') {
					flags |= lp->type;
					break;
				}
			}
		}
	return flags;
}

/*
 * Trap askheaders variable assignment.
 */

void
set_askheaders(struct var* vp, const char* value)
{
	state.askheaders = setheaders(vp, value);
}

/*
 * Trap asksub variable assignment.
 */

void
set_asksub(struct var* vp, const char* value)
{
	if (value)
		state.askheaders |= GSUB;
	else
		state.askheaders &= ~GSUB;
}

/*
 * Trap coprocess variable assignment.
 */

void
set_coprocess(struct var* vp, const char* value)
{
	if (value) {
		if (!*value)
			state.var.coprocess = varkeep("From @");
		state.var.crt = 0;
		state.var.interactive = state.on;
		state.var.header = 0;
		state.var.more = 0;
	}
}

/*
 * Trap crt variable assignment.
 */

void
set_crt(struct var* vp, const char* value)
{
	if (value && !*value)
		state.var.crt = state.realscreenheight;
}

/*
 * Trap editheaders variable assignment.
 */

void
set_editheaders(struct var* vp, const char* value)
{
	state.editheaders = setheaders(vp, value);
}

/*
 * Trap justfrom variable assignment.
 */

void
set_justfrom(struct var* vp, const char* value)
{
	if (value) {
		state.var.justheaders = state.var.justfrom ? "" : (char*)0;
		state.var.interactive = 0;
		state.var.screen = 0;
	}
}

/*
 * Trap list variable assignment.
 */

void
set_list(struct var* vp, const char* value)
{
	register char*	s;

	*vp->variable = s = varkeep(value);
	while (s = strchr(s, ' '))
		*s++ = ',';
}

/*
 * Trap mail variable assignment.
 */

void
set_mail(struct var* vp, const char* value)
{
	state.var.mail = varkeep(mailbox(state.var.user, value));
}

/*
 * Trap mailcap variable assignment.
 */

void
set_mailcap(struct var* vp, const char* value)
{
	state.part.init = (value && *value) ? 0 : -1;
}

/*
 * Trap news variable assignment.
 */

void
set_news(struct var* vp, const char* value)
{
	if (value && *value && close(open(state.var.news, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY|O_cloexec, MAILMODE)))
		note(FATAL|SYSTEM, "\"%s\"", state.var.news);
}

/*
 * Trap screen variable assignment.
 */

void
set_pwd(struct var* vp, const char* value)
{
	if (value && !*value) {
		if (!getcwd(state.path.pwd[0], sizeof(state.path.pwd[0])))
			strcpy(state.path.pwd[0], ".");
		state.var.pwd = state.path.pwd[0];
		state.var.oldpwd = state.path.pwd[1];
	}
}

/*
 * Trap sender variable assignment.
 */

void
set_sender(struct var* vp, const char* value)
{
	struct sender*	sp;
	struct sendor*	op;
	struct sendand*	ap;
	char*		v;
	int		c;
	int		d;
	int		n;

	if (value && *value) {
		if (!(sp = newof(0, struct sender, 1, strlen(value))))
			note(PANIC, "Out of space");
		v = sp->address;
		d = *value++;
		while ((c = *value++) && c != d)
			*v++ = c;
		*v++ = 0;
		op = &sp->sendor;
		ap = &op->sendand;
		n = 1;
		value--;
		while (c = *value++) {
			if (c == d) {
				*v++ = 0;
				if (n == 1) {
					n = 2;
					ap->head = v;
				}
				else if (n == 2) {
					n = 3;
					ap->pattern = v;
					if (!strcasecmp(ap->head, "received"))
						ap->flags |= GLAST;
				}
				else if (!(c = *value++) || *value != d)
					break;
				else if (c == '|') {
					if (!(op->next = newof(0, struct sendor, 1, 0)))
						note(PANIC, "Out of space");
					op = op->next;
					ap = &op->sendand;
					n = 1;
				}
				else if (c == '&') {
					if (!(ap->next = newof(0, struct sendand, 1, 0)))
						note(PANIC, "Out of space");
					ap = ap->next;
					n = 1;
				}
				else
					break;
			}
			else
				*v++ = c;
		}
		if (c)
			note(ERROR, "sender match operand syntax error: '%s' not expected", value - 1);
		else {
			sp->next = state.sender;
			state.sender = sp;
		}
	}
	else
		state.sender = 0;
}

/*
 * Trap screen variable assignment.
 */

void
set_screen(struct var* vp, const char* value)
{
	if (value && !*value)
		state.var.screen = state.screenheight > 4 ? (state.screenheight - 4) : state.screenheight;
}

/*
 * Trap sendmail variable assignment.
 */

void
set_sendmail(struct var* vp, const char* value)
{
	if (value && !*value) {
		sfprintf(state.path.temp, "%s -oi", _PATH_SENDMAIL);
		state.var.sendmail = varkeep(struse(state.path.temp));
	}
}

/*
 * Trap shell variable assignment.
 */

void
set_shell(struct var* vp, const char* value)
{
	if (!value || !*value)
		state.var.shell = varkeep(pathshell());
}

/*
 * Trap spambody variable assignment.
 */

void
set_spambody(struct var* vp, const char* value)
{
	register char*			s;
	register char*			t;
	register int			n;
	register struct linematch*	mp;
	register struct match*		xp;

	if (!(mp = state.bodymatch)) {
		if (!(mp = newof(0, struct linematch, 1, 0)))
			note(PANIC, "Out of space");
		state.bodymatch = mp;
	}
	s = (char*)value;
	for (;;) {
		if (t = strchr(s, '|'))
			n = t - s;
		else
			n = strlen(s);
		if (n) {
			if (mp->minline < n)
				mp->minline = n;
			if (!(xp = newof(0, struct match, 1, n)))
				note(PANIC, "Out of space");
			memcpy(xp->string, s, n);
			xp->length = n;
			mp->beg[xp->beg = s[0]] = 1;
			mp->mid[xp->mid = s[n/2]] = 1;
			mp->end[xp->end = s[n-1]] = 1;
			if (mp->last)
				mp->last = mp->last->next = xp;
			else
				mp->match = mp->last = xp;
		}
		if (!t)
			break;
		s = t + 1;
	}
}

/*
 * Trap toplines variable assignment.
 */

void
set_toplines(struct var* vp, const char* value)
{
	if (state.var.toplines < 0 || state.var.toplines >= 1000)
		state.var.toplines = 5;
}

/*
 * Trap trace variable assignment.
 */

void
set_trace(struct var* vp, const char* value)
{
	register int		c;
	register const char*	s;

	state.trace = 0;
	if (s = value)
		while (c = *s++)
			TRACE(c);
}

/*
 * Trap user variable assignment.
 */

void
set_user(struct var* vp, const char* value)
{
	if (!value || !*value)
		state.var.user = varkeep(username());
	else if (userid(state.var.user) < 0)
		note(FATAL|IDENTIFY, "\"%s\": unknown user", state.var.user);
}

/*
 * struct name case insensitive comparf
 */

static int
ignorecase(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	return strcasecmp(a, b);
}

/*
 * struct name freef
 */

static void
drop(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	register struct name*	np = (struct name*)obj;
	register struct list*	ap;
	register struct list*	bp;

	if (np->value) {
		if (!(np->flags & (GALIAS|GALTERNATE)))
			free(np->value);
		else {
			ap = (struct list*)np->value;
			do {
				bp = ap->next;
				free(ap);
			} while (ap = bp);
		}
	}
	free(np);
}

/*
 * Find/insert name in dict.
 * The dict*() cdt wrappers create dictionaries as needed.
 *
 *	op:	LOOKUP		lookup
 *		CREATE		INSERT, return 0 if already there
 *		IGNORECASE	case insensitive
 *		INSERT		insert
 *		DELETE		delete
 *		OBJECT		name is punned object
 *		STACK		object on stack
 */

struct name*
dictsearch(Dt_t** dp, const char* name, register int op)
{
	register Dt_t*		dt;
	register struct dict*	dict;
	register struct name*	np;
	register struct name*	xp;
	struct name*		object;

	if (!(dt = *dp)) {
		if (!(op & (CREATE|INSERT)))
			return 0;
		if (!(dict = (op & STACK) ? (struct dict*)salloc(sizeof(struct dict)) : newof(0, struct dict, 1, 0)))
			note(PANIC, "Out of space");
		memset(dict, 0, sizeof(*dict));
		dict->disc.key = offsetof(struct name, name);
		if (op & IGNORECASE)
			dict->disc.comparf = ignorecase;
		if (!(op & STACK))
			dict->disc.freef = drop;
		if (!(dt = dtopen(&dict->disc, Dtoset)))
			note(PANIC, "Out of space");
		if ((op & STACK) && state.onstack > 0) {
			dict->next = state.stacked;
			state.stacked = dt;
		}
		*dp = dt;
	}
	if (op & OBJECT) {
		object = (struct name*)name;
		name = (const char*)object->name;
	}
	else object = 0;
	if (!(np = (struct name*)dtmatch(dt, name))) {
		if (!(op & (CREATE|INSERT)))
			return 0;
		if (!(np = object) || (op & COPY)) {
			if (!(xp = (op & STACK) ? (struct name*)salloc(sizeof(struct name) + strlen(name) + 1) : newof(0, struct name, 1, strlen(name) + 1)))
				note(PANIC, "Out of space");
			strcpy(xp->name, name);
			xp->value = np ? np->value : 0;
			xp->flags = np ? np->flags : 0;
			np = xp;
		}
		np = (struct name*)dtinsert(dt, np);
	}
	else if (op & DELETE) {
		dtdelete(dt, np);
		np = 0;
	}
	else if (op & CREATE)
		np = 0;
	return np;
}

/*
 * Apply walkf(node,handle) to each node in the dictionary.
 * Non-zero return from walkf terminates walk with that value.
 */

int
dictwalk(Dt_t** dp, int (*walkf)(Dt_t*, void*, void*), void* context)
{
	return *dp ? dtwalk(*dp, walkf, context) : 0;
}

/*
 * Drop STACK dictionaries.
 */

void
dictreset(void)
{
	register Dt_t*	dt;

	while (dt = state.stacked) {
		state.stacked = ((struct dict*)dt->disc)->next;
		dtclose(dt);
	}
}

/*
 * Drop all entries from the dictionary.
 */

void
dictclear(Dt_t** dp)
{
	if (*dp)
		dtclear(*dp);
}
