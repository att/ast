/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2010 The Regents of the University of California an*
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
 * Handle name lists.
 */

#include "mailx.h"

/*
 * Grab a single word (liberal word)
 * Throw away things between ()'s, and take anything between <>.
 */
char*
yankword(char* ap, char* wbuf)
{
	register char*	cp;
	register char*	cp2;

	cp = ap;
	for (;;) {
		if (*cp == 0)
			return 0;
		if (*cp == '(') {
			register int	nesting = 0;

			while (*cp != 0) {
				switch (*cp++) {
				case '(':
					nesting++;
					break;
				case ')':
					--nesting;
					break;
				}
				if (nesting <= 0)
					break;
			}
		}
		else if (*cp == ' ' || *cp == '\t' || *cp == ',')
			cp++;
		else
			break;
	}
	if (*cp ==  '<')
		for (cp2 = wbuf; *cp && (*cp2++ = *cp++) != '>';)
			;
	else
		for (cp2 = wbuf; *cp && !strchr(" \t,(", *cp); *cp2++ = *cp++)
			;
	*cp2 = 0;
	return *wbuf ? cp : (char*)0;
}

/*
 * Clear flags.
 */
static int
clear(Dt_t* dt, void* object, void* context)
{
	if (!(((struct name*)object)->flags &= ~(*(unsigned long*)context)))
		((struct name*)object)->flags |= GDONE;
	return 0;
}

/*
 * Clear items from header.
 */
void
headclear(struct header* hp, unsigned long flags)
{
	hp->h_clear &= ~flags;
	if (flags & GSUB) {
		hp->h_flags &= ~GSUB;
		hp->h_subject = 0;
	}
	if (flags & GMISC) {
		hp->h_clear &= ~GMISC;
		hp->h_misc.head = hp->h_misc.tail = 0;
	}
	if (flags &= (GNAME|GMETOO)) {
		hp->h_clear &= ~(flags|GFIRST);
		hp->h_first = 0;
		dictwalk(&hp->h_names, clear, &flags);
	}
}

/*
 * Extract a list of names from a line,
 * and make a list of names from it.
 */
void
extract(struct header* hp, unsigned long flags, register char* s)
{
	register struct name*	np;
	register int		n;
	register struct list*	x;
	char			buf[LINESIZE];

	if (s) {
		note(DEBUG, "extract type=0x%08x data=\"%s\"%s", flags, s, (hp->h_clear & flags) ? " [clear]" : "");
		if (flags & GNAME) {
			if (hp->h_clear & flags) {
				hp->h_clear &= ~flags;
				dictwalk(&hp->h_names, clear, &flags);
			}
			while (s = yankword(s, buf))
				if (np = dictsearch(&hp->h_names, buf, INSERT|IGNORECASE|STACK)) {
					np->flags = flags;
					hp->h_flags |= flags;
					if (!hp->h_first && (flags & GTO) || (flags & GFIRST)) {
						flags &= ~GFIRST;
						hp->h_first = np->name;
					}
				}
		}
		else if (flags & GSUB) {
			hp->h_clear &= ~flags;
			if (!*s) {
				hp->h_flags &= ~flags;
				hp->h_subject = 0;
			}
			else if (!hp->h_subject || !streq(hp->h_subject, s)) {
				hp->h_flags |= flags;
				hp->h_subject = savestr(s);
			}
		}
		else if (flags & GMISC) {
			if (hp->h_clear & flags) {
				hp->h_clear &= ~flags;
				hp->h_misc.head = hp->h_misc.tail = 0;
			}
			if (*s) {
				if ((n = strlen(s)) > 0 && s[n - 1] == '\n')
					s[--n] = 0;
				for (x = hp->h_misc.head; x; x = x->next)
					if (streq(x->name, s))
						return;
				hp->h_flags |= flags;
				x = (struct list*)salloc(sizeof(struct list) + n + 1);
				strcpy(x->name, s);
				x->next = 0;
				if (hp->h_misc.tail)
					hp->h_misc.tail->next = x;
				else
					hp->h_misc.head = x;
				hp->h_misc.tail = x;
			}
		}
	}
}

/*
 * Low level for detract().
 */

typedef struct
{
	size_t	count;
	int	flags;
	int	sep;
	char*	base;
	char*	next;
} Walk_str_t;

static int
stringize(Dt_t* dt, void* object, void* context)
{
	register struct name*	np = (struct name*)object;
	register Walk_str_t*	ws = (Walk_str_t*)context;
	register char*		s;
	register char*		t;

	if (!ws->flags || ws->flags == (np->flags & GMASK)) {
		if (s = ws->next) {
			if (s > ws->base) {
				if (ws->sep)
					*s++ = ',';
				*s++ = ' ';
			}
			for (t = np->name; *s = *t++; s++);
			ws->next = s;
			np->flags |= GDONE;
		}
		else
			ws->count += strlen(np->name) + ws->sep;
	}
	return 0;
}

/*
 * Turn a list of names into a string of the same names.
 */
char*
detract(struct header* hp, unsigned long flags)
{
	Walk_str_t	ws;

	if (flags & GNAME) {
		if (hp->h_names) {
			if (flags & GCOMMA)
				note(DEBUG, "detract asked to insert commas");
			ws.sep = (flags & GCOMMA) ? 2 : 1;
			ws.flags = flags &= ~GCOMMA;
			ws.count = 0;
			ws.base = ws.next = 0;
			dictwalk(&hp->h_names, stringize, &ws);
			if (ws.count) {
				ws.base = ws.next = salloc(ws.count + 2);
				ws.sep--;
				dictwalk(&hp->h_names, stringize, &ws);
				return ws.base;
			}
		}
	}
	else if (flags & GSUB)
		return hp->h_subject;
	return 0;
}

typedef struct {
	int		more;
	int		show;
	Dt_t*		seen;
	Dt_t*		prev;
	Dt_t*		next;
} Walk_map_t;

/*
 * Add name,flags to the next map level.
 */
static void
mapadd(register Walk_map_t* wm, register struct name* np, char* name, unsigned long flags)
{
	if (wm->show)
		note(0, "\"%s\" -> \"%s\"", np->name, name);
	if (!dictsearch(&wm->prev, name, LOOKUP)) {
		np = dictsearch(&wm->next, name, INSERT|IGNORECASE|STACK);
		np->flags = flags;
		if (dictsearch(&wm->seen, name, CREATE|IGNORECASE|STACK)) {
			wm->more = 1;
		}
		else {
			np->flags |= GMAP;
			note(0, "\"%s\": alias loop", name);
		}
	}
}

/*
 * Map all of the aliased users in the invoker's mailrc
 * file and insert them into the list.
 * Changed after all these months of service to recursively
 * expand names (2/14/80).
 * And changed after all these years to weed out state.var.user
 * in one place (8/11/96).
 */
static int
mapuser(Dt_t* dt, void* object, void* context)
{
	register struct name*	np = (struct name*)object;
	register Walk_map_t*	wm = (Walk_map_t*)context;
	struct name*		ap;
	struct list*		mp;
	char*			s;

	if (np->flags & GDONE)
		return 0;
	if (!(np->flags & GMAP)) {
		np->flags |= GMAP;
		dictsearch(&wm->seen, np->name, INSERT|IGNORECASE|STACK);
		if (*np->name != '\\' && (ap = dictsearch(&state.aliases, np->name, LOOKUP))) {
			for (mp = (struct list*)ap->value; mp; mp = mp->next)
				mapadd(wm, np, mp->name, np->flags & ~(GMAP|GMETOO));
			return 0;
		}
		if (s = normalize(np->name, GCOMPARE, state.path.path, sizeof(state.path.path))) {
			mapadd(wm, np, s, np->flags & ~GMAP);
			return 0;
		}
		if (!(np->flags & GMETOO) && !state.var.metoo && streq(np->name, state.var.user) && (np->flags & (GCC|GBCC))) {
			if (wm->show)
				note(0, "\"%s\" -> DELETE", np->name);
			return 0;
		}
	}
	if (!dictsearch(&state.aliases, np->name, LOOKUP))
		dictsearch(&wm->next, (char*)np, INSERT|IGNORECASE|STACK|OBJECT|COPY);
	return 0;
}

int
usermap(struct header* hp, int show)
{
	struct name*	np;
	Walk_map_t	wm;

	wm.seen = 0;
	wm.prev = 0;
	do {
		wm.more = 0;
		wm.next = wm.prev;
		wm.prev = hp->h_names;
		dictclear(&wm.next);
		wm.show = show;
		dictwalk(&hp->h_names, mapuser, &wm);
		hp->h_names = wm.next;
	} while (wm.more);
	if (!hp->h_names &&
	    hp->h_first &&
	    (np = dictsearch(&wm.prev, hp->h_first, LOOKUP)) &&
	    ((np->flags & GMETOO) || streq(np->name, state.var.user))) {
		if (wm.show)
			note(0, "\"%s\" -> ADD", np->name);
		dictsearch(&hp->h_names, (char*)np, INSERT|IGNORECASE|OBJECT);
	}
	return hp->h_names != 0;
}

/*
 * Return path of file to record outgoing messages.
 */
char*
record(char* author, unsigned long flags)
{
	register int	c;
	register char*	ap;
	register char*	rp;
	register char*	ep;
	register char*	tp;

	rp = state.path.path;
	ep = rp + sizeof(state.path.path) - 1;
	if ((flags & FOLLOWUP) && (ap = author)) {
		if (tp = state.var.followup) {
			if (state.var.outfolder && !strchr(METAFILE, *tp) && rp < ep)
				*rp++ = '+';
			while ((*rp = *tp++) && rp < ep)
				rp++;
			if (*(rp - 1) != '/' && rp < ep)
				*rp++ = '/';
		}
		else if (state.var.outfolder && rp < ep)
			*rp++ = '+';
		tp = rp;
		ap = author;
		for (;;) {
			switch (c = *ap++) {
			case 0:
				break;
			case '@':
				break;
			case '!':
				rp = tp;
				continue;
			default:
				if (rp < ep)
					*rp++ = c;
				continue;
			}
			break;
		}
		tp = state.path.path;
		*rp = 0;
	}
	else if (!(tp = state.var.log))
		*rp = 0;
	else if (state.var.outfolder && !strchr(METAFILE, *tp)) {
		if (rp < ep)
			*rp++ = '+';
		strncopy(rp, tp, ep - rp);
	}
	return tp && *tp ? expand(tp, 1) : (char*)0;
}
