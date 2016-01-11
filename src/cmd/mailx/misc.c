/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2006 The Regents of the University of California an*
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
 * Auxiliary functions.
 *
 * So why isn't this stuff still in the file aux.c?
 * What file base name is special on what system.
 * I mean really special.
 * Any wagers on whether aux is in the POSIX conformance test suite?
 */

#include "mailx.h"

#include <stdarg.h>

/*
 * Note message.
 */
void
note(register int flags, const char* fmt, ...)
{
	register FILE*	fp;
	va_list		ap;

	va_start(ap, fmt);
	if ((flags & DEBUG) && !state.var.debug)
		return;
	if (flags & (ERROR|PANIC)) {
		fp = stderr;
		fflush(stdout);
	}
	else
		fp = stdout;
	if (state.var.coprocess)
		fprintf(fp, "%s ", state.var.coprocess);
	if (flags & IDENTIFY)
		fprintf(fp, "mail: ");
	if (flags & PANIC)
		fprintf(fp, T("panic: "));
	else if (flags & WARNING)
		fprintf(fp, T("warning: "));
	else if (flags & DEBUG)
		fprintf(fp, T("debug: "));
	vfprintf(fp, T(fmt), ap);
	va_end(ap);
	if (flags & SYSTEM)
		fprintf(fp, ": %s", strerror(errno));
	if (!(flags & PROMPT))
		fprintf(fp, "\n");
	fflush(fp);
	if (flags & PANIC)
		abort();
	if (flags & (FATAL|PANIC))
		exit(1);
}

/*
 * Return a pointer to a dynamic copy of the argument.
 */
char*
savestr(char* str)
{
	char*	p;
	int	size = strlen(str) + 1;

	if ((p = salloc(size)))
		memcpy(p, str, size);
	return p;
}

/*
 * Touch the named message by setting its MTOUCH flag.
 * Touched messages have the effect of not being sent
 * back to the system mailbox on exit.
 */
void
touchmsg(register struct msg* mp)
{
	if (mp->m_flag & MREAD)
		msgflags(mp, MTOUCH, 0);
	else
		msgflags(mp, MREAD|MSTATUS, 0);
}

/*
 * Test to see if the passed file name is a directory.
 */
int
isdir(char* name)
{
	struct stat	st;

	if (!name || stat(name, &st) < 0)
		return 0;
	return S_ISDIR(st.st_mode);
}

/*
 * Test to see if the passed file name is a regular file.
 */
int
isreg(char* name)
{
	struct stat	st;

	if (!name || stat(name, &st) < 0)
		return 0;
	return S_ISREG(st.st_mode);
}

/*
 * The following code deals with input stacking to do source
 * commands.  All but the current file pointer are saved on
 * the stack.
 */

/*
 * Pushdown current input file and switch to a new one.
 * Set the global flag "sourcing" so that others will realize
 * that they are no longer reading from a tty (in all probability).
 */
int
source(char** arglist)
{
	FILE*	fp;

	if (!(fp = fileopen(*arglist, "EXr")))
		return 1;
	if (state.source.sp >= NOFILE - 1) {
		note(0, "Too much \"sourcing\" going on");
		fileclose(fp);
		return 1;
	}
	state.source.stack[state.source.sp].input = state.input;
	state.source.stack[state.source.sp].cond = state.cond;
	state.source.stack[state.source.sp].loading = state.loading;
	state.source.sp++;
	state.loading = 0;
	state.cond = 0;
	state.input = fp;
	state.sourcing++;
	return 0;
}

/*
 * Pop the current input back to the previous level.
 * Update the "sourcing" flag as appropriate.
 */
int
unstack(void)
{
	if (state.source.sp <= 0) {
		note(0, "\"Source\" stack over-pop");
		state.sourcing = 0;
		return 1;
	}
	fileclose(state.input);
	if (state.cond)
		note(0, "Unmatched \"if\"");
	state.source.sp--;
	state.cond = state.source.stack[state.source.sp].cond;
	state.loading = state.source.stack[state.source.sp].loading;
	state.input = state.source.stack[state.source.sp].input;
	if (state.source.sp == 0)
		state.sourcing = state.loading;
	return 0;
}

/*
 * Touch the indicated file.
 * This is nifty for the shell.
 */
void
alter(char* name)
{
	touch(name, (time_t)0, (time_t)(-1), 0);
}

/*
 * Examine the passed line buffer and
 * return true if it is all blanks and tabs.
 */
int
blankline(char* linebuf)
{
	register char*	cp;

	for (cp = linebuf; *cp; cp++)
		if (*cp != ' ' && *cp != '\t')
			return 0;
	return 1;
}

/*
 * Start of a "comment".
 * Ignore it.
 */
static char*
skip_comment(register char* cp)
{
	register int	nesting = 1;

	for (; nesting > 0 && *cp; cp++) {
		switch (*cp) {
		case '\\':
			if (cp[1])
				cp++;
			break;
		case '(':
			nesting++;
			break;
		case ')':
			nesting--;
			break;
		}
	}
	return cp;
}

/*
 * shorten host if it is part of the local domain
 */

char*
localize(char* host)
{
	register char*	lp;
	register char*	le;
	register char*	hx;

	hx = strchr(host, '.');
	lp = state.var.local;
	for (;;) {
		if (le = strchr(lp, ','))
			*le = 0;
		if (!strcasecmp(lp, host)) {
			if (le)
				*le = ',';
			return 0;
		}
		if (hx && !strcasecmp(lp, hx + 1)) {
			*hx = 0;
			if (le)
				*le = ',';
			return host;
		}
		if (!(lp = le))
			break;
		*lp++ = ',';
	}
	return host;
}

/*
 * apply GCOMPARE, GDISPLAY, state.var.allnet, state.var.local
 */

char*
normalize(char* addr, unsigned long type, char* buf, size_t size)
{
	register char*	p;
	register char*	e;
	register int	n;
	char*		uucp;
	char*		arpa;
	char*		user;
	char*		inet;
	int		hadarpa;
	int		hadinet;
	char		temp[LINESIZE];
	char		norm[LINESIZE];

	while (isspace(*addr))
		addr++;
	if (!(type & GFROM) && (p = strrchr(addr, ':')))
		addr = p + 1;
	if ((type & GCOMPARE) && state.var.allnet) {
		if (p = strrchr(addr, '!'))
			addr = p + 1;
		if ((p = strchr(addr, '%')) || (p = strchr(addr, '@'))) {
			if (buf) {
				if ((n = p - addr) > size)
					n = size;
				addr = (char*)memcpy(buf, addr, n);
				p = addr + n;
			}
			*p = 0;
			return addr;
		}
	}
	else {
		strncopy(user = temp, addr, sizeof(temp));
		uucp = arpa = inet = 0;
		hadarpa = hadinet = 0;
		if (p = strrchr(user, '!')) {
			uucp = user;
			*p++ = 0;
			user = p;
			if (p = strrchr(uucp, '!'))
				p++;
			if (p && (type & GDISPLAY)) {
				uucp = p;
				p = 0;
			}
			if (p && state.var.local)
				uucp = localize(p);
		}
		if (p = strchr(user, '@')) {
			hadinet = 1;
			*p++ = 0;
			inet = state.var.local ? localize(p) : p;
		}
		if (p = strchr(user, '%')) {
			hadarpa = 1;
			*p++ = 0;
			if (!(type & (GCOMPARE|GDISPLAY)))
				arpa = state.var.local ? localize(p) : p;
		}
		if (uucp &&
		    (hadinet || (inet && streq(uucp, inet)) ||
		    (hadarpa || arpa && streq(uucp, arpa))))
			uucp = 0;
		if (arpa && (hadinet || inet && streq(arpa, inet)))
			arpa = 0;
		if (type & GDISPLAY) {
			if (inet)
				uucp = 0;
			else if (uucp) {
				inet = uucp;
				uucp = 0;
			}
		}
		p = norm;
		e = p + sizeof(norm);
		if (uucp) {
			p = strncopy(p, uucp, e - p);
			p = strncopy(p, "!", e - p);
		}
		p = strncopy(p, user, e - p);
		if (arpa) {
			p = strncopy(p, "%", e - p);
			p = strncopy(p, arpa, e - p);
		}
		if (inet) {
			p = strncopy(p, "@", e - p);
			p = strncopy(p, inet, e - p);
		}
		if (!streq(addr, norm))
			return savestr(norm);
	}
	return buf ? (char*)0 : (type & GSTACK) ? savestr(addr) : addr;
}

/*
 * Skin an arpa net address according to the RFC 822 interpretation
 * of "host-phrase."
 */
char*
skin(char* name, unsigned long type)
{
	register int	c;
	register char*	cp;
	register char*	cp2;
	char*		bufend;
	int		gotlt;
	int		lastsp;
	char		buf[LINESIZE];

	if (!name)
		return 0;
	if (type & (GMESSAGEID|GREFERENCES))
		return savestr(name);
	if (!strchr(name, '(') && !strchr(name, '<') && !strchr(name, ' '))
		return normalize(name, type, NiL, 0);
	gotlt = 0;
	lastsp = 0;
	bufend = buf;
	for (cp = name, cp2 = bufend; c = *cp++; ) {
		switch (c) {
		case '(':
			cp = skip_comment(cp);
			lastsp = 0;
			break;

		case '"':
			/*
			 * Start of a "quoted-string".
			 * Copy it in its entirety.
			 */
			while (c = *cp) {
				cp++;
				if (c == '"')
					break;
				if (c != '\\')
					*cp2++ = c;
				else if (c = *cp) {
					*cp2++ = c;
					cp++;
				}
			}
			lastsp = 0;
			break;

		case ' ':
			if (cp[0] == 'a' && cp[1] == 't' && cp[2] == ' ')
				cp += 3, *cp2++ = '@';
			else if (cp[0] == '@' && cp[1] == ' ')
				cp += 2, *cp2++ = '@';
			else
				lastsp = 1;
			break;

		case '<':
			cp2 = bufend;
			gotlt++;
			lastsp = 0;
			break;

		case '>':
			if (gotlt) {
				gotlt = 0;
				while ((c = *cp) && c != ',') {
					cp++;
					if (c == '(')
						cp = skip_comment(cp);
					else if (c == '"')
						while (c = *cp) {
							cp++;
							if (c == '"')
								break;
							if (c == '\\' && *cp)
								cp++;
						}
				}
				lastsp = 0;
				break;
			}
			/* Fall into . . . */

		default:
			if (lastsp) {
				lastsp = 0;
				*cp2++ = ' ';
			}
			*cp2++ = c;
			if (c == ',' && !gotlt) {
				*cp2++ = ' ';
				for (; *cp == ' '; cp++)
					;
				lastsp = 0;
				bufend = cp2;
			}
		}
	}
	*cp2 = 0;
	return normalize(buf, type|GSTACK, NiL, 0);
}

/*
 * Are any of the characters in the two strings the same?
 */
int
anyof(register char* s1, register char* s2)
{

	while (*s1)
		if (strchr(s2, *s1++))
			return 1;
	return 0;
}

/*
 * Convert c to lower case
 */
int
lower(register int c)
{
	return isupper(c) ? tolower(c) : c;
}

/*
 * Convert c to upper case
 */
int
upper(register int c)
{
	return islower(c) ? toupper(c) : c;
}

/*
 * Convert s to lower case
 */
char*
strlower(register char* s)
{
	register char*	b = s;
	register int	c;

	while (c = *s)
		*s++ = isupper(c) ? tolower(c) : c;
	return b;
}

/*
 * 0 terminate tmp string stream, rewind, and return beginning of string
 */
char*
struse(Sfio_t* sp)
{
	char*	s;

	if (!(s = sfstruse(sp)))
		note(FATAL, "out of space");
	return s;
}

/*
 * See if the given header field is supposed to be ignored.
 */
int
ignored(Dt_t** ignore, const char* field)
{
	struct name*	tp;

	if (ignore == &state.ignoreall)
		return 1;
	tp = dictsearch(ignore, field, LOOKUP);
	if (*ignore && (dictflags(ignore) & RETAIN))
		return !tp || !(tp->flags & RETAIN);
	return tp && (tp->flags & IGNORE);
}

/*
 * Allocate size more bytes of space and return the address of the
 * first byte to the caller.  An even number of bytes are always
 * allocated so that the space will always be on a word boundary.
 * The string spaces are of exponentially increasing size, to satisfy
 * the occasional user with enormous string size requests.
 *
 * Strings handed out here are reclaimed at the top of the command
 * loop each time, so they need not be freed.
 */

char*
salloc(register int size)
{
	register char*			t;
	register struct strings*	sp;
	int				index;

	if (state.onstack <= 0) {
		if (!(t = newof(0, char, size, 0)))
			note(PANIC, "Out of space");
		return t;
	}
	size += 7;
	size &= ~7;
	index = 0;
	for (sp = &state.stringdope[0]; sp < &state.stringdope[elementsof(state.stringdope)]; sp++) {
		if (!sp->s_topfree && (STRINGSIZE << index) >= size)
			break;
		if (sp->s_nleft >= size)
			break;
		index++;
	}
	if (sp >= &state.stringdope[elementsof(state.stringdope)])
		note(PANIC, "String too large");
	if (!sp->s_topfree) {
		index = sp - &state.stringdope[0];
		sp->s_topfree = (char*)malloc(STRINGSIZE << index);
		if (!sp->s_topfree)
			note(PANIC, "No room for dynamic string space %d", index);
		sp->s_nextfree = sp->s_topfree;
		sp->s_nleft = STRINGSIZE << index;
	}
	sp->s_nleft -= size;
	t = sp->s_nextfree;
	sp->s_nextfree += size;
	return t;
}

/*
 * Reset the string area to be empty.
 * Called to free all strings allocated
 * since last reset.
 */
void
sreset(void)
{
	register struct strings*	sp;
	register int			index;

	if (state.noreset)
		return;
	index = 0;
	for (sp = &state.stringdope[0]; sp < &state.stringdope[elementsof(state.stringdope)]; sp++) {
		if (!sp->s_topfree)
			continue;
		sp->s_nextfree = sp->s_topfree;
		sp->s_nleft = STRINGSIZE << index;
		index++;
	}
	dictreset();
}

/*
 * Return lines/chars for display.
 */
char*
counts(int wide, off_t lines, off_t chars)
{
	sfsprintf(state.counts, sizeof(state.counts), wide ? "%5ld/%-7ld" : "%3ld/%-5ld", (long)lines, (long)chars);
	return state.counts;
}

/*
 * Check if s matches `all'.
 */
int
isall(register const char* s)
{
	return s && (streq(s, "all") || streq(s, "*"));
}

/*
 * Check if name is a pipe command.
 */
char*
iscmd(register char* s)
{
	if (!s)
		return 0;
	while (isspace(*s))
		s++;
	if (*s != '!' && *s != '|')
		return 0;
	do {
		if (!*++s)
			return 0;
	} while (isspace(*s));
	return s;
}

/*
 * Set/Clear message flags
 */

void
msgflags(register struct msg* mp, int set, int clr)
{
	if (state.folder == FIMAP)
		imap_msgflags(mp, set, clr);
	else {
		if (clr)
			mp->m_flag &= ~clr;
		if (set)
			mp->m_flag |= set;
	}
}

/*
 * strncpy() with trailing nul, even if n==0
 * pointer to the copied nul returned
 */

char*
strncopy(register char* t, register const char* f, size_t n)
{
	register char*	e = t + n - 1;

	do
	{
		if (t >= e)
		{
			*t = 0;
			return t;
		}
	} while (*t++ = *f++);
	return t - 1;
}

/*
 * quote s into sp according to sh syntax
 */

void
shquote(register Sfio_t* sp, char* s)
{
	register char*	t;
	register char*	b;
	register int	c;
	register int	q;

	if (*s == '"' && *(s + strlen(s) - 1) == '"')
	{
		sfprintf(sp, "\\\"%s\\\"", s);
		return;
	}
	q = 0;
	b = 0;
	for (t = s;;)
	{
		switch (c = *t++)
		{
		case 0:
			break;
		case '\n':
		case ';':
		case '&':
		case '|':
		case '<':
		case '>':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
		case '*':
		case '?':
		case ' ':
		case '\t':
		case '\\':
			q |= 4;
			continue;
		case '\'':
			q |= 1;
			if (q & 2)
				break;
			continue;
		case '"':
		case '$':
			q |= 2;
			if (q & 1)
				break;
			continue;
		case '=':
			if (!q && !b && *(b = t) == '=')
				b++;
			continue;
		default:
			continue;
		}
		break;
	}
	if (!q)
		sfputr(sp, s, -1);
	else if (!(q & 1))
	{
		if (b)
			sfprintf(sp, "%-.*s'%s'", b - s, s, b);
		else
			sfprintf(sp, "'%s'", s);
	}
	else if (!(q & 2))
	{
		if (b)
			sfprintf(sp, "%-.*s\"%s\"", b - s, s, b);
		else
			sfprintf(sp, "\"%s\"", s);
	}
	else
		for (t = s;;)
			switch (c = *t++)
			{
			case 0:
				return;
			case '\n':
				sfputc(sp, '"');
				sfputc(sp, c);
				sfputc(sp, '"');
				break;
			case ';':
			case '&':
			case '|':
			case '<':
			case '>':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case '$':
			case '*':
			case '?':
			case ' ':
			case '\t':
			case '\\':
			case '\'':
			case '"':
				sfputc(sp, '\\');
				/*FALLTHROUGH*/
			default:
				sfputc(sp, c);
				break;
			}
}
