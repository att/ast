/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2011 The Regents of the University of California an*
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
 * Mail to others.
 */

#include "mailx.h"

struct letter {
	FILE*		fp;
	struct header*	hp;
	off_t		head;
	off_t		body;
};

/*
 * Ouput message part to op.
 */
static int
part(register struct parse* pp, FILE* op, struct part* ap, off_t size, char* prefix, int prefixlen, int emptylen, unsigned long flags)
{
	register int	n = 0;
	register int	i;
	register FILE*	ip;
	int		r = -1;
	char*		s;
	FILE*		tp;
	int		skip;
	int		line;

	ip = pp->fp;
	if (*ap->code && !isdigit(*ap->code)) {
		if (!(tp = fileopen(state.tmp.more, "Ew")))
			return -1;
		filecopy(NiL, ip, NiL, tp, NiL, size, NiL, NiL, 0);
		fileclose(tp);
		sfprintf(state.path.temp, "uudecode -h -t -o - -x %s %s", ap->code, state.tmp.more);
		if (!(ap->flags & PART_text) &&
		    !mimecmp("text", ap->type, NiL) &&
		    mime(1) &&
		    (s = mimeview(state.part.mime, NiL, "", ap->type, NiL)))
			sfprintf(state.path.temp, " | %s", s);
		if (!(ip = pipeopen(struse(state.path.temp), "r")))
			goto bad;
	}
	else if (!(ap->flags & PART_text) &&
	         !mimecmp("text", ap->type, NiL) &&
		 mime(1) &&
		 (s = mimeview(state.part.mime, NiL, state.tmp.more, ap->type, NiL))) {
		if (!(tp = fileopen(state.tmp.more, "Ew")))
			return -1;
		filecopy(NiL, ip, NiL, tp, NiL, size, NiL, NiL, 0);
		fileclose(tp);
		if (!(ip = pipeopen(s, "r")))
			goto bad;
	}
	if (prefix) {
		skip = 1;
		line = 0;
		while (size > 0) {
			if (!fgets(pp->buf, sizeof(pp->buf), ip)) {
				n = 0;
				break;
			}
			size -= n = strlen(pp->buf);
			if (pp->buf[0] == '\n' || pp->buf[0] == '\r' && pp->buf[1] == '\n') {
				if (!skip)
					line++;
				continue;
			}
			else
				skip = 0;
#if 0
			if ((flags & GREFERENCES) && pp->buf[0] == '-' && pp->buf[1] == '-' && isspace(pp->buf[2]))
				break;
#endif
			if (line) {
				line = 0;
				fputc('\n', op);
			}
			i = n > 1 ? prefixlen : emptylen;
			if (fwrite(prefix, 1, i, op) != i || fwrite(pp->buf, 1, n, op) != n)
				goto bad;
		}
	}
	else {
		while (size > 0) {
			n = size < sizeof(pp->buf) ? size : sizeof(pp->buf);
			if ((n = fread(pp->buf, 1, n, ip)) <= 0)
				break;
			size -= n;
			if (fwrite(pp->buf, 1, n, op) != n)
				goto bad;
		}
	}
	if (n > 0 && pp->buf[n - 1] != '\n')
		if ((n = getc(ip)) != EOF && putc(n, op) == EOF)
			goto bad;
	r = 0;
 bad:
	if (ip != pp->fp) {
		if (ip)
			fileclose(ip);
		remove(state.tmp.more);
	}
	return r;
}

/*
 * Show the message described by the passed pointer to the
 * passed output buffer.  Return -1 on error.
 * Adjust the status: field if need be.
 * If ignore is given, suppress ignored header fields.
 * prefix is a string to prepend to each output line.
 */
int
copy(register struct msg* mp, FILE* op, Dt_t** ignore, char* prefix, unsigned long flags)
{
	register char*	s;
	register int	i;
	char*		date;
	char*		head;
	char*		from;
	int		emptylen;
	int		prefixlen;
	long		n;
	struct part*	ap;
	struct parse	pp;

	if (state.folder == FIMAP)
		return imap_copy(mp, op, ignore, prefix, flags);

	/*
	 * Compute the prefix string, without trailing whitespace
	 */

	if (prefix) {
		prefixlen = strlen(prefix);
		s = prefix + prefixlen;
		while (--s >= prefix && isspace(*s));
		emptylen = (s + 1) - prefix;
	}
	flags |= GDISPLAY|GNL;
	if (flags & GREFERENCES) {
		if (headset(&pp, mp, NiL, NiL, NiL, 0)) {
			date = 0;
			from = 0;
			head = savestr(pp.data);
			while (headget(&pp)) {
				if (!strcasecmp(pp.name, "date"))
					date = savestr(pp.data);
				else if (!strcasecmp(pp.name, "from"))
					from = savestr(pp.data);
			}
			if (from) {
				if (s = strchr(from, '<')) {
					while (s > from && isspace(*(s - 1)))
						s--;
					*s = 0;
				}
				if (*from == '"' && *++from && *(s = from + strlen(from) - 1) == '"')
					*s = 0;
			}
			else {
				from = head;
				if (s = strchr(head, ' ')) {
					*s++ = 0;
					if (!date)
						date = s;
				}
			}
			if (date || (date = strchr(head, ' ')) && ++date)
				fprintf(op, "On %s ", date);
			fprintf(op, "%s wrote:\n", from);
		}
	}
	else {
		if (!ignore || !ignored(ignore, "status"))
			flags |= GSTATUS;
		if (headset(&pp, mp, NiL, NiL, ignore, flags)) {
			i = pp.length > 1 ? prefixlen : emptylen;
			do {
				if (prefix && fwrite(prefix, 1, i, op) != i ||
			    	fwrite(pp.buf, 1, pp.length, op) != pp.length)
					return -1;
			} while (headget(&pp));
		}
		fputc('\n', op);
	}

	/*
	 * Copy out the message body
	 */

	if (ap = state.part.in.head) {
		n = 0;
		do {
			if (flags & GINTERPOLATE) {
				boundary();
				note(DEBUG, "interpolate boundary=%s offset=%ld size=%ld", state.part.out.boundary, (long)ap->raw.offset, (long)ap->raw.size);
				fprintf(op, "\n--%s\n", state.part.out.boundary);
				fseek(pp.fp, ap->raw.offset, SEEK_SET);
				if (part(&pp, op, &state.part.global, ap->raw.size, prefix, prefixlen, emptylen, flags))
					return -1;
				n += ap->raw.size;
			}
			else if ((ap->flags & PART_body) || (flags & GMIME) && ap->count == 1 && !n && ap->next && !ap->next->lines && mime(1) && (s = mimeview(state.part.mime, NiL, NiL, ap->type, ap->opts))) {
				note(DEBUG, "copy part text offset=%ld size=%ld lines=%ld", (long)ap->offset, (long)ap->size, (long)ap->lines);
				fseek(pp.fp, ap->offset, SEEK_SET);
				if (part(&pp, op, ap, ap->size, prefix, prefixlen, emptylen, flags))
					return -1;
				n += ap->lines;
			}
			else
				fprintf(op, "(attachment %2d %s %20s \"%s\")\n\n", ap->count, counts(1, ap->lines, ap->size), ap->type, ap->name);
		} while (ap = ap->next);
	}
	else if (part(&pp, op, &state.part.global, pp.count, prefix, prefixlen, emptylen, flags))
		return -1;
	return 0;
}

/*
 * Prepend headers in front of the collected stuff.
 */
static int
prepend(struct letter* lp)
{
	register FILE*	nfi;
	register FILE*	nfo;
	time_t		now;

	lp->head = lp->body = 0;
	if (!(nfo = fileopen(state.tmp.mail, "Ew")))
		return -1;
	if (!(nfi = fileopen(state.tmp.mail, "Er"))) {
		fileclose(nfo);
		return -1;
	}
	rm(state.tmp.mail);
	time(&now);
	fprintf(nfo, "From %s %s", state.var.user, ctime(&now));
	lp->head = ftell(nfo);
	headout(nfo, lp->hp, GSEND|GEXTERN|GNL|GCOMMA);
	lp->body = ftell(nfo);
	if (filecopy("message body", lp->fp, state.tmp.mail, nfo, NiL, (off_t)0, NiL, NiL, 0)) {
		fileclose(nfo);
		fileclose(nfi);
		lp->head = lp->body = 0;
		rewind(lp->fp);
		return -1;
	}
	fileclose(nfo);
	fileclose(lp->fp);
	lp->fp = nfi;
	rewind(nfi);
	return 0;
}

/*
 * Save the outgoing mail on the passed file.
 */
static void
savemail(struct letter* lp, char* name)
{
	register FILE*	fp;

	if (fp = fileopen(name, "Ea")) {
		filecopy(NiL, lp->fp, name, fp, NiL, (off_t)0, NiL, NiL, GNL);
		fileclose(fp);
		rewind(lp->fp);
	}
}

/*
 * Pack ACTIVE args for execution.
 */
static int
packargs(Dt_t* dt, void* object, void* context)
{
	if (!(((struct name*)object)->flags & GDONE))
		addarg((struct argvec*)context, ((struct name*)object)->name);
	return 0;
}

/*
 * Determine if the passed address is a local "send to file" address.
 * If any of the network metacharacters precedes any slashes, it can't
 * be a filename.  We cheat with .'s to allow path names like ./...
 * As a final check the dir prefix must exist.
 */
static int
isfileaddr(char* name)
{
	register char*	cp;
	register char*	sp;
	int		r;

	if (*name == '+' || *name == '~')
		return 1;
	sp = 0;
	for (cp = name; *cp; cp++) {
		if (*cp == '!' || *cp == '%' || *cp == '@')
			break;
		if (*cp == '/')
			sp = cp;
	}
	if (!sp)
		return 0;
	*sp = 0;
	r = access(name, F_OK) >= 0;
	*sp = '/';
	return r;
}

/*
 * For each recipient in the header name list with a /
 * in the name, append the message to the end of the named file
 * and remove him from the recipient list.
 *
 * Recipients whose name begins with | are piped through the given
 * program and removed.
 */
static int
special(Dt_t* dt, void* object, void* context)
{
	register struct name*	np = (struct name*)object;
	register struct letter*	lp = (struct letter*)context;
	register char*		name;
	FILE*			fp;
	char*			cmd;
	int			n;

	if ((cmd = iscmd(np->name)) || isfileaddr(np->name)) {
		name = cmd ? cmd : expand(np->name, 1);
		if (state.var.debug) {
			note(DEBUG, "mail to %s: \"%s\"", cmd ? "pipe" : "file", name);
			goto cant;
		}

		/*
		 * Now either copy the letter to the desired file
		 * or give it as the standard input to the desired
		 * program as appropriate.
		 */

		if (cmd) {
			if (!(fp = fileopen(state.tmp.edit, "Ew+"))) {
				state.senderr++;
				goto cant;
			}
			remove(state.tmp.edit);
			filecopy(NiL, lp->fp, state.tmp.edit, fp, NiL, (off_t)0, NiL, NiL, GNL);
			rewind(lp->fp);
			n = start_command(state.var.shell, SIG_REG_EXEC, fileno(fp), -1, "-c", name, NiL);
			fileclose(fp);
			if (n < 0) {
				state.senderr++;
				goto cant;
			}
			free_command(n);
		}
		else {
			if (!(fp = fileopen(name, "Ea"))) {
				state.senderr++;
				goto cant;
			}
			filecopy(NiL, lp->fp, name, fp, NiL, (off_t)0, NiL, NiL, GNL);
			rewind(lp->fp);
			fileclose(fp);
		}
 cant:
		np->flags |= GDONE;
	}
	return 0;
}

/*
 * Mail a message on standard input to the people indicated
 * in the passed header.
 */
void
sendmail(struct header* hp, unsigned long flags)
{
	register char*	s;
	char**		p;
	int		type;
	int		pid;
	struct argvec	args;
	struct letter	letter;

	/*
	 * Collect user's mail from standard input.
	 * Get the result as letter.fp.
	 */
	if (!(letter.fp = collect(hp, flags)))
		return;
	letter.hp = hp;
	if (state.var.interactive) {
		if (type = (state.askheaders & (GBCC|GCC)))
			grabedit(letter.hp, type);
		else
			note(0, "EOT");
	}
	if (!filesize(letter.fp))
		if (letter.hp->h_subject)
			note(0, "Null message body; hope that's ok");
		else
			note(0, "No message, no subject; hope that's ok");
	/*
	 * Now, take the user names from the combined
	 * to and cc lists and do all the alias processing.
	 */
	if (!usermap(letter.hp, 0)) {
		note(0, "No recipients specified");
		state.senderr++;
	}
	if (prepend(&letter))
		state.senderr++;
	dictwalk(&letter.hp->h_names, special, &letter);
	if (state.senderr) {
		fseek(letter.fp, letter.body, SEEK_SET);
		savedeadletter(letter.fp);
	}
	initargs(&args);
	getargs(&args, state.var.sendmail);
	if (p = letter.hp->h_options)
		while (s = *p++)
			addarg(&args, s);
	p = args.argp;
	dictwalk(&letter.hp->h_names, packargs, &args);
	endargs(&args);
	if (args.argp != p) {
		s = record(letter.hp->h_first, flags);
		if (state.var.debug) {
			note(DEBUG|PROMPT, "sendmail command:");
			for (p = args.argv; *p; p++)
				printf(" \"%s\"", *p);
			printf("\n");
			if (s)
				note(DEBUG, "record mail in \"%s\"", s);
			note(DEBUG, "message contents:");
			filecopy(NiL, letter.fp, NiL, stdout, NiL, (off_t)0, NiL, NiL, 0);
		}
		else {
			if (s)
				savemail(&letter, s);
			/*
			 * Set up the temporary mail file as standard input for "mail"
			 * and exec with the user list we generated far above.
			 */
			endargs(&args);
			fseek(letter.fp, letter.head, SEEK_SET);
			fflush(letter.fp);
			s = args.argv[0];
			if (strneq(s, "smtp://", 7)) {
				if (!*(s += 7))
					s = state.var.smtp;
				if (sendsmtp(letter.fp, s, args.argv + 1, (off_t)0)) {
					fseek(letter.fp, letter.body, SEEK_SET);
					savedeadletter(letter.fp);
				}
			}
			else if ((pid = start_command(s, SIG_REG_EXEC|SIG_REG_TERM, fileno(letter.fp), -1, NiL, (char*)args.argv, NiL)) < 0) {
				fseek(letter.fp, letter.body, SEEK_SET);
				savedeadletter(letter.fp);
			}
			else if (state.var.sendwait)
				wait_command(pid);
			else
				free_command(pid);
		}
	}
	fileclose(letter.fp);
}

/*
 * Low level for fmt().
 */

typedef struct
{
	int		col;
	int		comma;
	int		flags;
	const char*	label;
	FILE*		fp;
} Format_t;

static int
format(Dt_t* dt, void* object, void* context)
{
	register struct name*	np = (struct name*)object;
	register Format_t*	fs = (Format_t*)context;
	register int		n;

	if (fs->flags & np->flags) {
		n = strlen(np->name);
		if (fs->label) {
			if (fs->col = strlen(fs->label))
				fputs(fs->label, fs->fp);
			fs->label = 0;
		}
		else if ((fs->col + fs->comma + n + 1) > MARGIN) {
			if (fs->comma)
				putc(',', fs->fp);
			fputs("\n    ", fs->fp);
			fs->col = 4;
		}
		else {
			if (fs->comma) {
				putc(',', fs->fp);
				fs->col++;
			}
			putc(' ', fs->fp);
			fs->col++;
		}
		fputs(np->name, fs->fp);
		fs->col += n;
	}
	return 0;
}

/*
 * Format the given header line to not exceed MARGIN characters.
 */
static void
fmt(FILE* fp, struct header* hp, const char* label, unsigned long flags, int comma)
{
	Format_t	fs;

	fs.col = 0;
	fs.comma = comma;
	fs.flags = flags;
	fs.label = label;
	fs.fp = fp;
	dictwalk(&hp->h_names, format, &fs);
	if (!fs.label)
		putc('\n', fp);
}

/*
 * Dump the to, subject, cc header to fp.
 */
int
headout(FILE* fp, struct header* hp, register unsigned long flags)
{
	register const struct lab*	lp;
	register struct list*		x;
	int				comma;
	int				force;
	int				gotcha;

	comma = (flags & GCOMMA);
	gotcha = 0;
	flags &= hp->h_flags|GSEND|GCOMMA|GRULE|GNL;
	force = (flags & GRULE) ? state.editheaders : 0;
	if (flags & GMISC) {
		gotcha = 1;
		for (x = hp->h_misc.head; x; x = x->next)
			fprintf(fp, "%s\n", x->name);
	}
	if (flags & GSEND) {
		gotcha = 1;
		if (state.var.fixedheaders)
			fprintf(fp, "%s\n", state.var.fixedheaders);
		fprintf(fp, "X-Mailer: %s\n", state.version);
		fprintf(fp, "Mime-Version: 1.0\n");
		if (state.part.out.multi)
			fprintf(fp, "Content-Type: multipart/mixed; boundary=\"%s\"\n", state.part.out.boundary);
		else {
			fprintf(fp, "Content-Type: text/plain; charset=us-ascii\n");
			fprintf(fp, "Content-Transfer-Encoding: 7bit\n");
		}
		if (hp->h_flags & GMESSAGEID) {
			fprintf(fp, "References: ");
			if (hp->h_flags & GREFERENCES) {
				char*	s;
				int	m;
				int	n;

				m = REFLEN - strlen(hp->h_messageid) - 12;
				s = hp->h_references;
				if ((n = strlen(s)) > m) {
					for (s += n - m; *s && !isspace(*s); s++);
					for (; isspace(*s); s++);
				}
				fprintf(fp, "%s ", s);
			}
			fprintf(fp, "%s\n", hp->h_messageid);
			hp->h_flags &= ~(GMESSAGEID|GREFERENCES);
		}
	}
	for (lp = state.hdrtab; lp->name; lp++)
		if (flags & lp->type) {
			gotcha = 1;
			if (lp->type & GSUB)
				fprintf(fp, "%s%s\n", lp->name, hp->h_subject);
			else
				fmt(fp, hp, lp->name, lp->type, comma);
		}
		else if (force & lp->type)
			fprintf(fp, "%s\n", lp->name);
	if (gotcha && (flags & GNL)) {
		if ((flags & GRULE) && state.var.rule && *state.var.rule)
			fprintf(fp, "%s\n\n", state.var.rule);
		else
			fputc('\n', fp);
	}
	return 0;
}
