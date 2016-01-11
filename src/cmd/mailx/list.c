/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2004 The Regents of the University of California an*
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
 * Message list handling.
 */

#include "mailx.h"

/*
 * Bit values for colon modifiers.
 */

#define	CMNEW		0x01		/* New messages */
#define	CMOLD		0x02		/* Old messages */
#define	CMUNREAD	0x04		/* Unread messages */
#define	CMDELETE	0x08		/* Deleted messages */
#define	CMREAD		0x10		/* Read messages */
#define	CMSPAM		0x20		/* (probable) Spam messages */

#define CMNOT		8		/* left shift for :!x */

/*
 * The following table describes the letters which can follow
 * the colon and gives the corresponding modifier bit.
 */

struct coltab {
	char	co_char;		/* What to find past : */
	int	co_bit;			/* Associated modifier bit */
	int	co_mask;		/* m_status bits to mask */
	int	co_equal;		/* ... must equal this */
} coltab[] = {
	'n',		CMNEW,		MNEW,		MNEW,
	'o',		CMOLD,		MNEW,		0,
	'u',		CMUNREAD,	MREAD,		0,
	'd',		CMDELETE,	MDELETE,	MDELETE,
	'r',		CMREAD,		MREAD,		MREAD,
	'x',		CMSPAM,		MSPAM,		MSPAM,
	0,		0,		0,		0
};

/*
 * See if the passed name sent the passed message number.
 */
int
sender(char* name, int mesg)
{
	char*	s;

	if (!*name || !(s = grab(state.msg.list + mesg - 1, GREPLY|GCOMPARE, NiL)))
		return 0;
	return !strcasecmp(name, s);
}

/*
 * See if the passed name received the passed message number.
 */
static int
matchto(char* name, int mesg)
{
	register struct msg*	mp;
	register char*			s;
	register char*			t;
	register char*			b;
	int				i;

	static const int		fields[] = { GTO, GCC, GBCC };

	if (!*name++ || !*name)
		return 0;
	mp = state.msg.list + mesg - 1;
	for (i = 0; i < elementsof(fields); i++)
		if (t = grab(mp, fields[i], NiL)) {
			s = name;
			b = t;
			while (*t) {
				if (!*s)
					return 1;
				if (lower(*s++) != lower(*t++)) {
					t = ++b;
					s = name;
				}
			}
			if (!*s)
				return 1;
		}
	return 0;
}

/*
 * See if the given string matches inside the header field of the
 * given message.  For the purpose of the scan, we ignore case differences.
 * If it does, return true.  The string search argument is assumed to
 * have the form "/search-string."  If it is of the form "/," we use the
 * previous search string.
 */
static int
matchfield(char* str, int mesg)
{
	register struct msg*	mp;
	register char*			s;
	register char*			t;
	register char*			b;

	if (!*str)
		str = state.last.scan;
	else
		strncopy(state.last.scan, str, sizeof(state.last.scan));
	mp = state.msg.list + mesg - 1;
	if (state.var.searchheaders && (s = strchr(str, ':'))) {
		if (lower(str[0]) == 't' && lower(str[1]) == 'o' && str[2] == ':') {
			for (s += 3; isspace(*s); s++);
			return matchto(s, mesg);
		}
		*s++ = 0;
		t = grab(mp, GSUB, str);
		s[-1] = ':';
		while (isspace(*s))
			s++;
		str = s;
	}
	else {
		s = str;
		t = grab(mp, GSUB, NiL);
	}
	if (!t)
		return 0;
	b = t;
	while (*t) {
		if (!*s)
			return 1;
		if (lower(*s++) != lower(*t++)) {
			t = ++b;
			s = str;
		}
	}
	return !*s;
}

/*
 * See if the passed name sent the passed message number.
 */
static int
matchsender(register char* s, int mesg)
{
	register char*	t;

	if (*s && (t = grab(state.msg.list + mesg - 1, GREPLY|GCOMPARE, NiL)))
		do {
			if (!*s)
				return !isalnum(*t);
		} while (lower(*s++) == lower(*t++));
	return 0;
}

/*
 * Return the message number corresponding to the passed meta character.
 */
static int
metamess(int meta, int f)
{
	register int		c;
	register int		m;
	register struct msg*	mp;

	c = meta;
	switch (c) {
	case '^':
		/*
		 * First 'good' message left.
		 */
		for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++)
			if ((mp->m_flag & (MDELETE|MNONE)) == f)
				return mp - state.msg.list + 1;
		note(0, "No applicable messages");
		return -1;

	case '$':
		/*
		 * Last 'good message left.
		 */
		for (mp = state.msg.list + state.msg.count - 1; mp >= state.msg.list; mp--)
			if ((mp->m_flag & (MDELETE|MNONE)) == f)
				return mp - state.msg.list + 1;
		note(0, "No applicable messages");
		return -1;

	case '.':
		/*
		 * Current message.
		 */
		m = state.msg.dot - state.msg.list + 1;
		if ((state.msg.dot->m_flag & (MDELETE|MNONE)) != f) {
			note(0, "%d: inappropriate message", m);
			return -1;
		}
		return m;

	default:
		note(0, "\"%c\": unknown metachar", c);
		return -1;
	}
}

/*
 * Unscan the named token by pushing it onto the regret stack.
 */
static void
regret(int token)
{
	if (++state.regretp >= REGDEP)
		note(PANIC, "Too many regrets");
	state.regretstack[state.regretp] = token;
	state.lexstring[STRINGLEN - 1] = 0;
	state.string_stack[state.regretp] = savestr(state.lexstring);
	state.numberstack[state.regretp] = state.lexnumber;
}

/*
 * Reset all the scanner global variables.
 */
static void
scaninit(void)
{
	state.regretp = -1;
}

/*
 * Unmark the named message.
 */
static void
unmark(int mesg)
{
	register int	i;

	i = mesg;
	if (i < 1 || i > state.msg.count)
		note(PANIC, "Bad message number to unmark");
	msgflags(&state.msg.list[i - 1], 0, MMARK);
}

/*
 * Turn the character(s) after a colon modifier into a bit value.
 */
static int
evalcol(char* s)
{
	register int		c;
	register int		neg;
	register struct coltab*	colp;

	if (neg = *s == '!')
		s++;
	if (!(c = *s))
		return state.colmod;
	for (colp = &coltab[0]; colp->co_char; colp++)
		if (colp->co_char == c) {
			return neg ? (colp->co_bit << CMNOT) : colp->co_bit;
		}
	return 0;
}

/*
 * Mark the named message by setting its mark bit.
 */
static void
markone(register int i)
{
	if (i < 1 || i > state.msg.count)
		note(PANIC, "Bad message number to mark");
	msgflags(&state.msg.list[i - 1], MMARK, 0);
}

/*
 * Mark all messages that the user wanted from the command
 * line in the message structure.  Return 0 on success, -1
 * on error.
 */

static int
markall(char* buf, int f)
{
	register char**		np;
	register int		i;
	register struct msg*	mp;
	char*			bufp;
	int			beg;
	int			colmod;
	int			colresult;
	int			lone;
	int			mc;
	int			other;
	int			star;
	int			tok;
	int			valdot;
	char*			namelist[NMLSIZE];

	valdot = state.msg.dot - state.msg.list + 1;
	colmod = 0;
	for (i = 1; i <= state.msg.count; i++)
		unmark(i);
	bufp = buf;
	mc = 0;
	np = &namelist[0];
	scaninit();
	tok = scan(&bufp);
	star = 0;
	other = 0;
	lone = 0;
	beg = 0;
	while (tok != TEOL) {
		switch (tok) {
		case TNUMBER:
 number:
			if (star) {
				note(0, "No numbers mixed with *");
				return -1;
			}
			mc++;
			other++;
			if (beg) {
				if (check(state.lexnumber, f))
					return -1;
				for (i = beg; i <= state.lexnumber; i++)
					if (!(state.msg.list[i - 1].m_flag & MNONE) &&
					    (f == MDELETE || !(state.msg.list[i - 1].m_flag & MDELETE)))
						markone(i);
				beg = 0;
				break;
			}
			beg = state.lexnumber;
			if (check(beg, f))
				return -1;
			tok = scan(&bufp);
			regret(tok);
			if (tok != TDASH) {
				markone(beg);
				beg = 0;
			}
			break;

		case TPLUS:
			if (beg) {
				note(0, "Non-numeric second argument");
				return -1;
			}
			i = valdot;
			do {
				i++;
				if (i > state.msg.count) {
					note(0, "Referencing beyond EOF");
					return -1;
				}
			} while ((state.msg.list[i - 1].m_flag & (MDELETE|MNONE)) != f);
			markone(i);
			break;

		case TDASH:
			if (!beg) {
				lone = 1;
				i = valdot;
				do {
					i--;
					if (i <= 0) {
						note(0, "Referencing before 1");
						return -1;
					}
				} while ((state.msg.list[i - 1].m_flag & (MDELETE|MNONE)) != f);
				markone(i);
			}
			break;

		case TSTRING:
			if (beg) {
				note(0, "Non-numeric second argument");
				return -1;
			}
			other++;
			if (state.lexstring[0] == ':') {
				colresult = evalcol(state.lexstring + 1);
				if (!colresult) {
					note(0, "\"%s\", unknown : modifier", state.lexstring);
					return -1;
				}
				colmod |= colresult;
			}
			else
				*np++ = savestr(state.lexstring);
			break;

		case TDOLLAR:
		case TUP:
		case TDOT:
			state.lexnumber = metamess(state.lexstring[0], f);
			if (state.lexnumber == -1)
				return -1;
			goto number;

		case TSTAR:
			if (other) {
				note(0, "Can't mix \"*\" with anything");
				return -1;
			}
			star++;
			break;

		case TERROR:
			return -1;
		}
		i = tok;
		tok = scan(&bufp);
		if (!lone && tok == TEOL && i == TDASH) {
			bufp = "$";
			tok = scan(&bufp);
		}
	}
	state.colmod = colmod;
	*np = 0;
	mc = 0;
	if (star) {
		for (i = 0; i < state.msg.count; i++)
			if ((state.msg.list[i].m_flag & (MDELETE|MNONE)) == f) {
				markone(i + 1);
				mc++;
			}
		if (!mc) {
			note(0, "No applicable messages");
			return -1;
		}
		return 0;
	}

	/*
	 * If no numbers were given, mark all of the messages,
	 * so that we can unmark any whose sender was not selected
	 * if any user names were given.
	 */

	if ((np > namelist || colmod) && !mc)
		for (i = 1; i <= state.msg.count; i++)
			if ((state.msg.list[i - 1].m_flag & (MDELETE|MNONE)) == f)
				markone(i);

	/*
	 * If any names were given, go through and eliminate any
	 * messages whose senders were not requested.
	 */

	if (np > namelist) {
		for (i = 1; i <= state.msg.count; i++) {
			for (mc = 0, np = &namelist[0]; *np; np++)
				if (**np == '/') {
					if (matchfield(*np + 1, i)) {
						mc++;
						break;
					}
				}
				else {
					if (matchsender(*np, i)) {
						mc++;
						break;
					}
				}
			if (!mc)
				unmark(i);
		}

		/*
		 * Make sure we got some decent messages.
		 */

		mc = 0;
		for (i = 1; i <= state.msg.count; i++)
			if (state.msg.list[i - 1].m_flag & MMARK) {
				mc++;
				break;
			}
		if (!mc) {
			printf("No applicable messages from {%s",
				namelist[0]);
			for (np = &namelist[1]; *np; np++)
				printf(", %s", *np);
			printf("}.\n");
			return -1;
		}
	}

	/*
	 * If any colon modifiers were given, go through and
	 * unmark any messages which do not satisfy the modifiers.
	 */

	if (colmod) {
		for (i = 1; i <= state.msg.count; i++) {
			register struct coltab*	colp;

			mp = state.msg.list + i - 1;
			for (colp = &coltab[0]; colp->co_char; colp++)
				if ((colp->co_bit & colmod) && (mp->m_flag & colp->co_mask) != colp->co_equal ||
				    (colp->co_bit & (colmod>>CMNOT)) && (mp->m_flag & colp->co_mask) == colp->co_equal)
					unmark(i);
		}
		for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++)
			if (mp->m_flag & MMARK)
				break;
		if (mp >= state.msg.list + state.msg.count) {
			register struct coltab*	colp;

			printf("No messages satisfy");
			for (colp = &coltab[0]; colp->co_char; colp++)
				if (colp->co_bit & colmod)
					printf(" :%c", colp->co_char);
				else if (colp->co_bit & (colmod>>CMNOT))
					printf(" !%c", colp->co_char);
			printf("\n");
			return -1;
		}
	}
	return 0;
}

/*
 * Convert the user string of message numbers and
 * stuff the indices in state.msg.list.m_index.
 *
 * Returns the count of messages picked up or -1 on error.
 */
int
getmsglist(register char* buf, unsigned long flags)
{
	register struct msg*	ip;
	register struct msg*	mp;

	if (!(ip = state.msg.list) || !state.msg.count) {
		if (ip) {
			ip->m_index = 0;
			return 0;
		}
		note(0, "No appropriate messages");
		return -1;
	}
	if (state.folder == FIMAP) {
		for (; isspace(*buf); buf++);
		if (*buf == '(')
			return imap_msglist(buf);
	}
	if (markall(buf, flags) < 0)
		return -1;
	for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++)
		if (mp->m_flag & MMARK) {
			ip->m_index = mp - state.msg.list + 1;
			ip++;
		}
	ip->m_index = 0;
	return ip - state.msg.list;
}

/*
 * Check the passed message number for legality and proper flags.
 * If f is MDELETE, then either kind will do.  Otherwise, the message
 * has to be undeleted.
 */
int
check(int mesg, int f)
{
	register struct msg*	mp;

	if (mesg < 1 || mesg > state.msg.count) {
		note(0, "%d: invalid message number", mesg);
		return -1;
	}
	mp = state.msg.list + mesg - 1;
	if ((mp->m_flag & MNONE) || f != MDELETE && (mp->m_flag & MDELETE)) {
		note(0, "%d: inappropriate message", mesg);
		return -1;
	}
	return 0;
}

/*
 * Add a single arg to the arg vector.
 * 0 returned on success
 */
int
addarg(register struct argvec* vp, const char* s)
{
	if (vp->argp >= &vp->argv[elementsof(vp->argv) - 1]) {
		note(0, "Too many arguments; excess discarded");
		return -1;
	}
	*vp->argp++ = (char*)s;
	return 0;
}

/*
 * Scan out the list of string arguments, shell style
 * and append to the vector.
 */
void
getargs(register struct argvec* vp, register char* s)
{
	register char*	t;
	char*		e;
	register int	c;
	register int	quote;
	int		n;
	char*		pop;
	FILE*		fp;
	char		buf[LINESIZE];
	char		mac[LINESIZE];

	pop = 0;
	for (;;) {
		while (isspace(*s))
			s++;
		if (!*s) {
			if (s = pop) {
				pop = 0;
				continue;
			}
			break;
		}
		quote = 0;
		t = buf;
		for (;;) {
			if (!(c = *s)) {
				if (pop) {
					s = pop;
					pop = 0;
					continue;
				}
				break;
			}
			s++;
			if (!pop) {
				if (c == '$' && quote != '\'' && *s == '{' && (e = strchr(s, '}'))) {
					c = *e;
					*e = 0;
					if (*++s == '<') {
						if (fp = fileopen(++s, "Xr")) {
							if ((n = fread(mac, 1, sizeof(mac) - 1, fp)) > 0) {
								mac[n] = 0;
								s = mac;
							}
							else {
								if (n < 0 && !state.sourcing)
									note(SYSTEM, "%s", s);
								s = 0;
							}
							fileclose(fp);
						}
						else {
							if (!state.sourcing)
								note(SYSTEM, "%s", s);
							s = 0;
						}
					}
					else
						s = varget(s);
					*e++ = c;
					if (s)
						pop = e;
					else
						s = e;
					continue;
				}
				else if (quote) {
					if (c == quote) {
						quote = 0;
						continue;
					}
					if (c == '\\') {
						c = chresc(s - 1, &e);
						s = e;
					}
				}
				else if (c == '"' || c == '\'') {
					quote = c;
					continue;
				}
				else if (isspace(c))
					break;
			}
			else if (isspace(c)) {
				if (!quote) {
					if (!*s || t > buf && *(t - 1) == ' ')
						continue;
					c = ' ';
				}
				else if (c == '\n' && !*s || c == '\r' && *s == '\n' && !*(s + 1))
					continue;
			}
			if (t < &buf[sizeof(buf) - 2])
				*t++ = c;
		}
		*t = 0;
		if (addarg(vp, savestr(buf)))
			break;
	}
}

/*
 * scan out a single lexical item and return its token number,
 * updating the string pointer passed **p.  Also, store the value
 * of the number or string scanned in lexnumber or lexstring as
 * appropriate.  In any event, store the scanned `thing' in lexstring.
 */

struct lex {
	char	l_char;
	char	l_token;
} singles[] = {
	'$',	TDOLLAR,
	'.',	TDOT,
	'^',	TUP,
	'*',	TSTAR,
	'-',	TDASH,
	'+',	TPLUS,
	'(',	TOPEN,
	')',	TCLOSE,
	0,	0
};

int
scan(char** sp)
{
	register char*		cp;
	register char*		cp2;
	register int		c;
	register struct lex*	lp;
	int			quotec;

	if (state.regretp >= 0) {
		strncopy(state.lexstring, state.string_stack[state.regretp], sizeof(state.lexstring));
		state.lexnumber = state.numberstack[state.regretp];
		return state.regretstack[state.regretp--];
	}
	cp = *sp;
	cp2 = state.lexstring;
	c = *cp++;

	/*
	 * strip away leading white space.
	 */

	while (c == ' ' || c == '\t')
		c = *cp++;

	/*
	 * If no characters remain, we are at end of line,
	 * so report that.
	 */

	if (!c) {
		*sp = --cp;
		return TEOL;
	}

	/*
	 * If the leading character is a digit, scan
	 * the number and convert it on the fly.
	 * Return TNUMBER when done.
	 */

	if (isdigit(c)) {
		state.lexnumber = 0;
		while (isdigit(c)) {
			state.lexnumber = state.lexnumber*10 + c - '0';
			*cp2++ = c;
			c = *cp++;
		}
		*cp2 = 0;
		*sp = --cp;
		return TNUMBER;
	}

	/*
	 * Check for single character tokens; return such
	 * if found.
	 */

	for (lp = &singles[0]; lp->l_char; lp++)
		if (c == lp->l_char) {
			state.lexstring[0] = c;
			state.lexstring[1] = 0;
			*sp = cp;
			return lp->l_token;
		}

	/*
	 * We've got a string!  Copy all the characters
	 * of the string into lexstring, until we see
	 * a null, space, or tab.
	 * If the lead character is a " or ', save it
	 * and scan until you get another.
	 */

	quotec = 0;
	if (c == '\'' || c == '"') {
		quotec = c;
		c = *cp++;
	}
	while (c) {
		if (c == quotec) {
			cp++;
			break;
		}
		if (!quotec && (c == ' ' || c == '\t'))
			break;
		if (cp2 - state.lexstring < STRINGLEN - 1)
			*cp2++ = c;
		c = *cp++;
	}
	if (quotec && !c) {
		note(0, "Missing %c", quotec);
		return TERROR;
	}
	*sp = --cp;
	*cp2 = 0;
	return TSTRING;
}

/*
 * Find the first message whose flags & m == f  and return
 * its message number.
 */
int
first(int f, int m)
{
	register struct msg*	mp;

	if (!state.msg.count)
		return 0;
	f &= MDELETE|MNONE;
	m &= MDELETE|MNONE|MSPAM;
	for (mp = state.msg.dot; mp < state.msg.list + state.msg.count; mp++)
		if ((mp->m_flag & m) == f)
			return mp - state.msg.list + 1;
	for (mp = state.msg.dot - 1; mp >= state.msg.list; mp--)
		if ((mp->m_flag & m) == f)
			return mp - state.msg.list + 1;
	return 0;
}
