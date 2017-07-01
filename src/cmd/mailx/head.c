/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2013 The Regents of the University of California an*
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
 * Routines for processing and detecting headlines.
 */

#include "mailx.h"

static const struct lab fields[] = {
	"newsgroups",		GNEWS,
	"article-id",		GNEWS,
	"reply-to",		GNEWS|GREPLY,
	"original-from",	GNEWS|GREPLY,
	"from",			GNEWS|GREPLY,
	"sender",		GNEWS,
	"apparently-to",	GTO,
	"original-to",		GTO,
	"to",			GTO,
	"cc",			GCC,
	"bcc",			GBCC,
	"subject",		GSUB,
	"subj",			GSUB,
	"status",		GSTATUS,
	"references",		GREFERENCES,
	"message-id",		GMESSAGEID,
	"received",		0,
	"return-path",		0,
	"importance",		0,
	"in-reply-to",		0,
	"priority",		0,
	"x-",			0,
};

/*
 * Data setup for mimehead().
 */

#define CONTENT_boundary	1
#define CONTENT_disposition	2
#define CONTENT_encoding	3
#define CONTENT_filename	4
#define CONTENT_name		5
#define CONTENT_type		6

struct content {
	const char*	name;
	int		index;
};

static const struct content	contents[] = {
	"boundary",		CONTENT_boundary,
	"disposition",		CONTENT_disposition,
	"filename",		CONTENT_filename,
	"name",			CONTENT_name,
	"transfer-encoding",	CONTENT_encoding,
	"type",			CONTENT_type,
};

/*
 * Called by mimehead() to set content data values.
 */
static int
content(Mime_t* mp, void* entry, char* data, size_t size, Mimedisc_t* disc)
{
	register struct content*	cp = (struct content*)entry;
	register struct part*		ap;
	register struct bound*		bp;
	char*				s;
	char*				e;

	note(DEBUG, "content multi=%d %s `%-.*s'", state.part.in.multi, cp->name, size, data);
	if (!(ap = ((struct state_part*)disc)->head))
		ap = &state.part.global;
	switch (cp->index) {
	case CONTENT_boundary:
		if (state.part.in.multi) {
			if (!(bp = newof(0, struct bound, 1, size + 1))) {
				note(ERROR|SYSTEM, "Out of space");
				break;
			}
			bp->size = size;
			memcpy(bp->data, data, size);
			bp->next = state.part.in.boundary;
			state.part.in.boundary = bp;
			note(DEBUG, "content boundary=`%s'", bp->data);
		}
		break;
	case CONTENT_disposition:
		ap->flags |= PART_disposition;
		if (!mimecmp("inline", data, NiL))
			ap->flags |= PART_inline;
		/*FALLTHROUGH*/
	case CONTENT_type:
		if (!mimecmp("multipart", data, NiL))
			state.part.in.multi = 1;
		else if (!*ap->type || !strchr(ap->type, '/') || cp->index == CONTENT_type && (ap->flags & PART_disposition)) {
			ap->flags &= ~PART_disposition;
			if (size >= sizeof(ap->type))
				size = sizeof(ap->type) - 1;
			strncopy(ap->type, data, size + 1);
			strlower(ap->type);
			s = data + size;
			if (!(e = strchr(s, '\n')))
				e = s + strlen(s);
			if ((size = (e - s)) >= sizeof(ap->opts))
				size = sizeof(ap->opts) - 1;
			strncopy(ap->opts, s, size + 1);
			if (!mimecmp("text/plain", data, NiL) || !mimecmp("text/enriched", data, NiL))
			{
				ap->flags |= PART_text;
				if (!strncmp(ap->code, "bin", 3))
					*ap->code = 0;
			}
			else if (!mimecmp("message", data, NiL))
				ap->flags |= PART_message;
			else
				ap->flags |= PART_application;
		}
		break;
	case CONTENT_encoding:
		if (size >= sizeof(ap->code))
			size = sizeof(ap->code) - 1;
		strncopy(ap->code, data, size + 1);
		strlower(ap->code);
		if ((ap->flags & PART_text) && !strncmp(ap->code, "bin", 3))
			*ap->code = 0;
		break;
	case CONTENT_name:
		if (*ap->name)
			break;
		/*FALLTHROUGH*/
	case CONTENT_filename:
		if (size >= sizeof(ap->name))
			size = sizeof(ap->name) - 1;
		strncopy(ap->name, data, size + 1);
		break;
	}
	return 0;
}

/*
 * Open a mime handle and return non-0 if ok to mime.
 * op==0 for header parsing, op==1 for viewing.
 */
int
mime(int op)
{
	if (state.part.init < 0)
		return 0;
	if (!state.part.mime) {
		state.part.disc.version = MIME_VERSION;
		state.part.disc.flags = 0;
		state.part.disc.errorf = 0;
		state.part.disc.valuef = content;
		if (!(state.part.mime = mimeopen(&state.part.disc))) {
			state.part.init = -1;
			return 0;
		}
	}
	if (state.part.init < op) {
		mimeload(state.part.mime, state.var.mailcap, MIME_LIST);
		state.part.init = op;
	}
	return 1;
}

/*
 * Set up a message for header parsing.
 */
int
headset(register struct parse* pp, struct msg* mp, FILE* fp, struct header* hp, Dt_t** ignore, unsigned long flags)
{
	int		r;
	struct bound*	bp;

	while (bp = state.part.in.boundary) {
		state.part.in.boundary = bp->next;
		free(bp);
	}
	pp->fp = mp ? setinput(mp) : fp;
	pp->mp = mp;
	if ((flags & GMIME) && !mime(0))
		flags &= ~GMIME;
	if (pp->hp = hp) {
		if (flags & GRULE)
			headclear(hp, flags);
		else
			hp->h_clear = flags;
	}
	pp->count = mp ? mp->m_size : 0;
	pp->flags = flags & (GDISPLAY|GNL);
	pp->ignore = 0;
	if (hp || (flags & GFROM))
		r = 1;
	else if ((r = headget(pp)) && (pp->flags & GDISPLAY) && pp->length > 6) {
		register char*	s;
		register char*	t;
		struct name*	np;
		int		c;

		/*
		 * Pun on GMETOO -- if from me then don't ignore To:
		 */
		for (s = t = pp->buf + 5; *t && !isspace(*t); t++);
		c = *t;
		*t = 0;
		if (streq(s, state.var.user) || (np = dictsearch(&state.aliases, s, LOOKUP)) && streq(np->name, state.var.user))
			pp->flags |= GMETOO;
		*t = c;
	}
	pp->ignore = ignore;
	pp->flags |= flags;
	pp->type = 0;
	if (state.part.in.head) {
		state.part.in.count = 0;
		do {
			state.part.in.tail = state.part.in.head->next;
			free(state.part.in.head);
		} while (state.part.in.head = state.part.in.tail);
	}
	state.part.in.multi = 0;
	if (state.part.global.flags)
		memset(&state.part.global, 0, sizeof(state.part.global));
	return r;
}

/*
 * Generate the message status field.
 */
static void
status(struct parse* pp)
{
	register char*	s;

	pp->flags &= ~GSTATUS;
	s = strcopy(pp->buf, "Status");
	pp->separator = s;
	*s++ = (pp->flags & GDISPLAY) ? ':' : 0;
	*s++ = ' ';
	if (pp->mp->m_flag & MSPAM)
		*s++ = 'X';
	if (pp->mp->m_flag & MREAD)
		*s++ = 'R';
	if (!(pp->mp->m_flag & MNEW))
		*s++ = 'O';
	if (*(s - 1) == ' ')
		s--;
	*s++ = '\n';
	*s = 0;
	pp->length = s - pp->buf;
	if (!(pp->flags & GNL))
		pp->buf[pp->length - 1] = 0;
}

/*
 * Attach part ap to the current message.
 */
static void
attach(register struct part* ap)
{
	register int	c;
	register int	p;
	register char*	s;
	register char*	t;

	if (!(ap->flags & (PART_application|PART_message|PART_text)))
		ap->flags |= PART_text;
	if ((ap->flags & PART_text) && ((ap->flags & PART_inline) || !ap->name[0]))
		ap->flags |= PART_body;
	if (!(ap->flags & PART_body))
		ap->count = ++state.part.in.count;
	if (!ap->name[0])
		sfsprintf(ap->name, sizeof(ap->name), "%d.att", ap->count);
	else {
		s = t = ap->name;
		for (p = 0; c = *s++; p = c) {
			if (c == '/' || c == '\\') {
				c = 0;
				t = ap->name;
			}
			else {
				if (isspace(c) || iscntrl(c) || !isprint(c) || c == '#' || c == '$' || c == '&' || c == '*' || c == '(' || c == ')' || c == '"' || c == '\'' || c == '`' || c == '<' || c == '>' || c == '?' || c == '[' || c == ']' || c == '{' || c == '}')
					c = '_';
				if (c != '_' && c != '-' || p != '_' && p != '-')
					*t++ = c;
			}
		}
		*t = 0;
	}
	if (!*ap->type)
		strcpy(ap->type, "unknown");
	note(DEBUG, "part %d offset=%ld size=%ld lines=%ld flags=0x%08x type=%s name=%s", ap->count, (long)ap->offset, (long)ap->size, (long)ap->lines, ap->flags, ap->type, ap->name);
	if (state.part.in.tail)
		state.part.in.tail->next = ap;
	else
		state.part.in.head = ap;
	state.part.in.tail = ap;
}

/*
 * Read the next header line.
 */
static char*
headline(register struct parse* pp)
{
	register char*	s;
	register char*	e;
	register char*	t;
	register int	c;
	register int	n;

	if (pp->mp && pp->count <= 0 || (pp->flags & GDONE) || !fgets(pp->buf, sizeof(pp->buf), pp->fp)) {
		pp->flags |= GDONE;
		if (pp->hp)
			pp->hp->h_clear = 0;
		return 0;
	}
	pp->length = n = strlen(pp->buf);
	s = pp->buf + n;
	e = pp->buf + sizeof(pp->buf) - 1;
	pp->count -= n;
	if (*pp->buf != '\n' && (*pp->buf != '\r' || *(pp->buf + 1) != '\n')) {
		while ((!pp->mp || pp->count > 0) && (c = fgetc(pp->fp)) != EOF) {
			if (c == '\r' || c == '\n' || !isspace(c)) {
				ungetc(c, pp->fp);
				break;
			}
			t = s;
			if (s < e) {
				*(s - 1) = ' ';
				*s++ = ' ';
			}
			n = 1;
			while ((c = fgetc(pp->fp)) != EOF) {
				n++;
				if (s < e)
					*s++ = c;
				if (c == '\n')
					break;
			}
			pp->count -= n;
			pp->length += s - t;
		}
	}
	*s = 0;
	return pp->buf;
}

/*
 * Get mime multipart content.
 */
static void
multipart(register struct parse* pp)
{
	register char*	s;
	unsigned long	count;
	off_t		offset;
	int		n;
	int		header;
	struct part*	ap;
	struct bound*	bp;

	if (!state.part.in.boundary)
		return;
	pp->flags &= ~GMIME;
	offset = ftell(pp->fp) - pp->length;
	count = pp->count + pp->length;
	ap = 0;
	header = 0;
	while (s = headline(pp)) {
		if (state.part.in.boundary && (pp->length == (state.part.in.boundary->size + 3) || pp->length == (state.part.in.boundary->size + 5)) && s[0] == '-' && s[1] == '-' && !strncmp(s + 2, state.part.in.boundary->data, state.part.in.boundary->size)) {
			if (ap) {
				ap->raw.size = ftell(pp->fp) - ap->raw.offset - pp->length;
				attach(ap);
				note(DEBUG, "raw offset=%ld size=%ld", (long)ap->raw.offset, (long)ap->raw.size);
			}
			if (!(ap = newof(0, struct part, 1, 0))) {
				note(ERROR|SYSTEM, "Out of space");
				break;
			}
			ap->raw.offset = ftell(pp->fp);
			header = 1;
			if (s[state.part.in.boundary->size + 2] == '-' && s[state.part.in.boundary->size + 3] == '-') {
				bp = state.part.in.boundary->next;
				free(state.part.in.boundary);
				if (!(state.part.in.boundary = bp)) {
					header = 0;
					while (pp->count > 0 && (n = getc(pp->fp)) == '\n')
						pp->count--;
					if (n != EOF)
						ungetc(n, pp->fp);
					ap->offset = ftell(pp->fp);
				}
			}
		}
		else if (header) {
			state.part.head = ap;
			if (!mimehead(state.part.mime, (void*)contents, elementsof(contents), sizeof(*contents), s)) {
				if (ap->flags & PART_message) {
					ap->flags &= ~PART_message;
					header = 1;
				}
				else
					header = strchr(s, ':') != 0;
				ap->offset = ftell(pp->fp);
			}
			state.part.head = 0;
		}
		else if (ap) {
			ap->size += pp->length;
			ap->lines++;
		}
	}
	if (ap) {
		if (header)
			free(ap);
		else
			attach(ap);
	}
	fseek(pp->fp, offset, SEEK_SET);
	pp->count = count;
}

/*
 * Get the next message header.
 */
int
headget(register struct parse* pp)
{
	register char*			s;
	register char*			t;
#if _PACKAGE_ast
	register char*			u;
	register char*			v;
	char*				e;
	char*				h;
	char*				d;
	ssize_t				(*decode)(const void*, size_t, void**, void*, size_t, void**);
	ssize_t				z;
#endif
	register int			n;
	register const struct lab*	lp;
	off_t				body;
	off_t				text;
	int				i;
	int				separator;

	while (s = headline(pp)) {
		/*
		 * A blank line separates the headers from the body.
		 */
		if ((n = *s) == '\n' || n == '\r' && (n = *(s + 1)) == '\n') {
			if (state.var.headerbotch && pp->fp != stdin && (body = ftell(pp->fp)) > 0) {
				/*
				 * If the next batch of lines up to
				 * a blank line look like headers then
				 * treat them as such rather than
				 * message body.  This compensates
				 * for those mailers that botch the
				 * message manual of style.
				 */
				text = body;
				separator = 1;
				i = 0;
				for (;;) {
					if (!(s = fgets(pp->buf, sizeof(pp->buf), pp->fp)))
						goto done;
					if ((n = *s) == '\n' || n == '\r' && (n = *(s + 1)) == '\n') {
						if (!separator)
							break;
						text = ftell(pp->fp);
					}
					else if (isspace(n)) {
						if (!i)
							goto done;
						separator = 0;
					}
					else if (isalpha(n) || n == '>') {
						if (n != '>' || strncmp(++s, "From ", 5)) {
							t = s;
							do {
								if (!*s || isspace(*s))
									goto done;
							} while (*s++ != ':');
							if (!i) {
								n = s - t - 1;
								if (n > 2 && (t[0] == 'X' || t[0] == 'x') && t[1] == '-')
									i = 1;
								else {
									i = -1;
									do {
										if (++i >= elementsof(fields))
											goto done;
									} while (strncasecmp(fields[i].name, t, n));
								}
							}
						}
						separator = 0;
					}
					else if (isprint(n)) {
						while (*++s == n);
						if (*s != '\n' && (*s != '\r' || *(s + 1) != '\n'))
							goto done;
						if (separator)
							text = ftell(pp->fp);
					}
					else
						goto done;
				}
				n = fseek(pp->fp, text, SEEK_SET);
				pp->count -= text - body;
				continue;
 done:
				n = fseek(pp->fp, body, SEEK_SET);
				s = pp->buf;
				*s++ = '\n';
				*s = 0;
			}
			pp->flags |= GDONE;
			break;
		}
		if (isspace(n))
			continue;
		if (!isalnum(n) && isprint(n)) {
			t = s;
			while (*++t == n);
			if (*t == '\n' || *t == '\r' && *(t + 1) == '\n') {
				if (n == '-' && (t - s) > 2) {
					/*
					 * Edit rule treated like a blank line.
					 */
					break;
				}
				/*
				 * Treat rules as ignored header lines.
				 */
				continue;
			}
		}
		if ((pp->flags & GMIME) && mimehead(state.part.mime, (void*)contents, elementsof(contents), sizeof(*contents), s))
			multipart(pp);
		else {
			i = 0;
			while (*s == '>') {
				s++;
				i = 1;
			}
			pp->name = s;
			for (n = 0; *s && !isspace(*s) && *s != ':'; n = *s++) {
				if (isupper(*s)) {
					if (isalnum(n))
						*s = tolower(*s);
				}
				else if (islower(*s)) {
					if (!isalnum(n))
						*s = toupper(*s);
				}
			}
			if ((separator = *(pp->separator = s)) != ':' && i)
				continue;
			*s = 0;
			if (!pp->ignore || !ignored(pp->ignore, pp->name) || (pp->flags & GMETOO) && streq(pp->name, "To") && ((pp->flags &= ~GMETOO), 1)) {
				while (isspace(*++s));
#if _PACKAGE_ast
				if (state.var.convertheaders) {
					t = state.tmp.head;
					e = t + sizeof(state.tmp.head) - 1;
					for (u = h = s; *u; u++)
						if (*u == '=' && *(u + 1) == '?')
						{
							i = 0;
							for (v = u + 2; *v; v++)
								if (*v == '?')
								{
									if (*(v + 1) == '=')
										break;
									if (i == 0)
									{
										/* XXX UTF-8 for now */ if (strncasecmp(u + 2, "UTF-8", v - (u + 2))) break;
										d = v;
										i = *(v + 1);
										if (i == 'B' || i == 'b')
											decode = base64decode;
										else if (i == 'Q' || i == 'q')
											decode = qpdecode;
										else
											break;
										if (*(v + 2) != '?')
											break;
										v += 2;
										i = 1;
									}
									else
									{
										i = 2;
										break;
									}
								}
							if (*v && i == 1)
							{
								if (u - s)
								{
									if ((t + (u - s)) > e)
										goto enough;
									memcpy(t, s, u - s);
									t += u - s;
								}
								if (*(v + 2) == '?')
									u = v;
								else
									for (u = v + 1; isspace(*(u + 1)); u++);
								s = u + 1;
								*d = 0;
								/* XXX must do iconv here too */
								if ((z = (*decode)(d + 3, v - (d + 3), 0, t, 75, 0)) > 0)
									t += z;
							}
						}
				enough:
					if (s != h)
					{
						if (u - s)
						{
							if ((t + (u - s)) <= e)
							{
								memcpy(t, s, u - s);
								t += u - s;
							}
							else
								note(WARNING, "header too large -- %zu max", sizeof(state.tmp.head));
						}
						*t = 0;
						s = state.tmp.head;
					}
				}
#endif
				pp->data = s;
				pp->type = 0;
				if (pp->flags & GCOMPOSE) {
					for (lp = state.hdrtab; lp->name; lp++)
						if (pp->flags & (lp->type|GMISC)) {
							s = pp->name;
							t = (char*)lp->name;
							if (upper(*s) == *t) {
								while (lower(*++s) == *++t);
								if (!*s && *t == ':') {
									if (pp->flags & lp->type) {
										pp->flags &= ~lp->type;
										pp->type = lp->type;
										if (lp->type & GSTATUS)
											status(pp);
									}
									break;
								}
							}
						}
					if (!pp->type && (pp->flags & GMISC)) {
						pp->type = GMISC;
						*pp->separator = separator;
						pp->data = pp->name;
					}
				}
				if (!(pp->flags & GNL))
					pp->buf[pp->length - 1] = 0;
				if (pp->type && pp->hp)
					extract(pp->hp, pp->type, pp->data);
				if (pp->flags & GDISPLAY)
					*pp->separator = separator;
				return 1;
			}
		}
	}
	if (pp->flags & GSTATUS) {
		status(pp);
		return 1;
	}
	if (state.part.in.head) {
		fseek(pp->fp, state.part.in.head->offset, SEEK_SET);
		pp->count = state.part.in.head->size;
	}
	return 0;
}

/*
 * Fetch the field info by name from the passed message.
 */
static char*
grabname(struct parse* pp, struct msg* mp, char* name, unsigned long type)
{
	register char*	r = 0;
	register int	n = 0;
	register int	i;
	register int	u;

	if (headset(pp, mp, NiL, NiL, NiL, 0)) {
		u = (type & GUSER) ? strlen(state.var.user) : 0;
		while (headget(pp)) {
			if (!strcasecmp(pp->name, name)) {
				if (!(type & GLAST))
					return savestr(pp->data);
				if (u > 0 && !strncasecmp(pp->data, state.var.user, u) && pp->data[u] == '@')
					u = -u;
				else if (u < 0) {
					if (!*pp->data)
						return savestr(pp->data);
					continue;
				}
				i = strlen(pp->data) + 1;
				if (i > n) {
					n = i;
					r = savestr(pp->data);
				}
				else
					strcpy(r, pp->data);
			}
		}
	}
	return r;
}

/*
 * Fetch the field info by type from the passed message.
 */
static char*
grabtype(struct parse* pp, register struct msg* mp, unsigned long type)
{
	register char*	s;
	register char*	e;
	register char*	t;
	register FILE*	fp;
	struct sender*	sp;
	struct sendor*	op;
	struct sendand*	ap;
	int		i;
	int		first = 1;
	char		namebuf[LINESIZE];

	if (type & (GREPLY|GSENDER)) {
		for (sp = state.sender; sp; sp = sp->next)
			for (op = &sp->sendor; op; op = op->next)
				for (ap = &op->sendand; ap;) {
					if (!(s = grabname(pp, mp, ap->head, ap->flags)) || !strmatch(s, ap->pattern))
						break;
					if (!(ap = ap->next))
						return sp->address;
				}
		if (type & GSENDER)
			return 0;
	}
	e = namebuf + sizeof(namebuf);
	for (i = 0; i < elementsof(fields); i++)
		if ((fields[i].type & type) &&
		    (s = grabname(pp, mp, (char*)fields[i].name, type)) &&
		    ((type & (GNEWS|GSUB)) || (s = skin(s, type))) &&
		    yankword(s, namebuf)) {
			if ((type & GNEWS) && (t = strchr(s, ',')))
				*t = 0;
			if (type & (GNEWS|GSUB))
				for (e = s, i = 0; *e; e++)
					if (isspace(*e))
					{
						if (i || *e != ' ')
						{
							t = e;
							while (i = *e++)
							{
								if (isspace(i))
									for (i = ' '; isspace(*e); e++);
								*t++ = i;
							}
							*t = 0;
							break;
						}
						i = 1;
					}
					else
						i = 0;
			return s;
		}
#if 0
	if (!(type & (GREPLY|GTO)) || (type & GLAST))
#else
	if (!(type & (GREPLY|GTO)))
#endif
		return 0;
	fp = setinput(mp);
	namebuf[0] = 0;
	if (readline(fp, pp->buf, sizeof(pp->buf)) >= 0) {
 again:
		for (;;) {
			for (s = pp->buf; *s && *s != ' '; s++);
			for (; *s == ' ' || *s == '\t'; s++) ;
			for (t = &namebuf[strlen(namebuf)]; *s && *s != ' ' && *s != '\t' && t < namebuf + sizeof(namebuf) - 1;)
				*t++ = *s++;
			*t = 0;
			if (readline(fp, pp->buf, sizeof(pp->buf)) < 0)
				break;
			if (!(s = strchr(pp->buf, 'F')))
				break;
			if (strncmp(s, "From", 4))
				break;
			while (s = strchr(s, 'r')) {
				if (!strncmp(s, "remote", 6)) {
					if (!(s = strchr(s, 'f')))
						break;
					if (strncmp(s, "from", 4))
						break;
					if (!(s = strchr(s, ' ')))
						break;
					s++;
					if (first) {
						first = 0;
						s = strncopy(namebuf, s, e - namebuf);
					}
					else if (t = strrchr(namebuf, '!')) {
						t++;
						s = strncopy(t, s, e - t);
						strncopy(s, "!", e - s);
					}
					goto again;
				}
				s++;
			}
		}
	}
	return skin(savestr(namebuf), type);
}

/*
 * Grab field/address header from this message.
 * If name!=0 then it contains the header name,
 * otherwise grab by type.
 */
char*
grab(register struct msg* mp, unsigned long type, char* name)
{
	register char*	s;
	struct parse	pp;

	if (!name)
		s = grabtype(&pp, mp, type);
	else if ((s = grabname(&pp, mp, name, type)) && (type & (GCOMPARE|GDISPLAY)))
		s = normalize(s, type, NiL, 0);
	return s;
}

/*
 * Collect a liberal (space, tab delimited) word into b.
 * Update p to point to the next word.
 */
char*
wordnext(char** p, char* b)
{
	register char*	s;
	register char*	t;
	register int	c;

	if (!(s = *p) || !*s)
		return 0;
	while (isspace(*s))
		s++;
	t = b;
	while ((c = *s++) && !isspace(c)) {
		*t++ = c;
		if (c == '"') {
			do {
				if (!(c = *s++)) {
					s--;
					break;
				}
			} while ((*t++ = c) != '"');
 		}
	}
	if (!c)
		s--;
	else
		while (isspace(*s))
			s++;
	*p = s;
	*t = 0;
	return *b ? b : (char*)0;
}

/*
 * Split a headline into its useful components.
 * Copy the line into dynamic string space, then set
 * pointers into the copied line in the passed headline
 * structure.  Actually, it scans.
 */
void
parse(struct msg* mp, char* line, register struct headline* hl, char* pbuf, size_t psize)
{
	register char*	s;
	register char*	t;
	register char*	e;
	struct parse	pp;

	hl->l_from = 0;
	hl->l_info = 0;
	hl->l_date = 0;
	t = pbuf;
	e = t + psize;
	if (mp && state.folder == FMH && strncmp(line, "From ", 5)) {
		hl->l_from = grabname(&pp, mp, "From", 0);
		hl->l_date = grabname(&pp, mp, "Date", 0);
	}
	else if (wordnext(&line, pp.buf) && (s = wordnext(&line, pp.buf))) {
		if (state.var.news) {
			t = strncopy(hl->l_info = t, s, e - t);
			if (mp) {
				hl->l_from = grabname(&pp, mp, "From", 0);
				hl->l_date = grabname(&pp, mp, "Date", 0);
			}
		}
		else {
			t = strncopy(hl->l_from = t, s, e - t - 1) + 1;
			if (*(s = line)) {
				if (s[0] == 't' && s[1] == 't' && s[2] == 'y') {
					t = strncopy(hl->l_info = t, s, e - t);
					s = wordnext(&line, pp.buf);
				}
				if (*(s = line))
					t = strncopy(hl->l_date = t, s, e - t);
			}
		}
	}
}

/*
 * See if the passed line buffer is a mail header.
 * Return true if yes.  Note the extreme pains to
 * accomodate all funny formats.
 */
int
ishead(char* linebuf, int inhead)
{
	register char*	cp;
	struct headline	hl;
	char		parbuf[LINESIZE];


	if (state.var.news) {
		cp = linebuf;
		if (*cp++ == 'A' && *cp++ == 'r' && *cp++ == 't' &&
		    *cp++ == 'i' && *cp++ == 'c' && *cp++ == 'l' &&
		    *cp++ == 'e' && (*cp == ' ' || *cp == ':'))
			return 1;
	}
	cp = linebuf;
	if (*cp++ != 'F' || *cp++ != 'r' || *cp++ != 'o' || *cp++ != 'm' ||
	    *cp++ != ' ')
		return 0;
	parse(NiL, linebuf, &hl, parbuf, sizeof(parbuf));
	if (!hl.l_from || !hl.l_date || !isdate(hl.l_date)) {
		note(DEBUG, "\"%s\": not a header: no from or date field", linebuf);
		return 0;
	}
	return 1;
}
