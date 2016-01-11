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
 * More user commands.
 */

#include "mailx.h"

/*
 * If any arguments were given, go to the next applicable argument
 * following dot, otherwise, go to the next applicable message.
 * If given as first command with no arguments, print first message.
 */
int
next(struct msg* msgvec)
{
	register struct msg*	mp;
	register struct msg*	ip;
	register struct msg*	ip2;
	register int		skip;
	int			mdot;

	skip = MDELETE|MNONE;
	if (msgvec->m_index) {
		/*
		 * If some messages were supplied, find the
		 * first applicable one following dot using
		 * wrap around.
		 */

		mdot = state.msg.dot - state.msg.list + 1;

		/*
		 * Find the first message in the supplied
		 * message list which follows dot.
		 */

		for (ip = msgvec;; ip++)
			if (!ip->m_index) {
				ip = msgvec;
				break;
			}
			else if (ip->m_index > mdot)
				break;
		ip2 = ip;
		do {
			mp = state.msg.list + ip2->m_index - 1;
			if (!(mp->m_flag & skip)) {
				state.msg.dot = mp;
				goto hitit;
			}
			if (ip2->m_index)
				ip2++;
			if (!ip2->m_index)
				ip2 = msgvec;
		} while (ip2 != ip);
		note(0, "No messages applicable");
		return 1;
	}

	/*
	 * If this is the first command, select message 1.
	 * Note that this must exist for us to get here at all.
	 */

	if (!state.sawcom)
		goto hitit;

	/*
	 * Just find the next good message after dot, no
	 * wraparound.
	 */

	skip |= MSAVE;
	if (!state.edit && state.var.spamlog)
		skip |= MSPAM;
	for (mp = state.msg.dot + 1; mp < state.msg.list + state.msg.count; mp++)
		if (!(mp->m_flag & skip))
			break;
	if (mp >= state.msg.list + state.msg.count) {
		note(0, "At EOF");
		return 0;
	}
	state.msg.dot = mp;
 hitit:
	/*
	 * Print dot.
	 */

	state.msg.list->m_index = state.msg.dot - state.msg.list + 1;
	(state.msg.list + 1)->m_index = 0;
	return type(state.msg.list);
}

/*
 * Snarf the file from the end of the command line and
 * return a pointer to it.  If there is no file attached,
 * just return 0.  Put a null in front of the file
 * name so that the message list processing won't see it,
 * If the file name is the only thing on the line return 0
 * in the reference flag variable.
 */

char*
snarf(char* line, int* flag)
{
	register char*	s;
	register int	quote;

	if (!*line) {
		*flag = 0;
		return 0;
	}
	s = line + strlen(line) - 1;

	/*
	 * Strip away trailing blanks.
	 */

	while (s > line && isspace(*s))
		s--;
	*++s = 0;

	/*
	 * cmdpipe() allows quoted string as last arg.
	 */

	if (s > line && (*(s - 1) == '"' || *(s - 1) == '\'')) {
		quote = *--s;
		*s = 0;
		do {
			if (s <= line) {
				note(0, "Unbalanced %c quote", quote);
				*flag = -1;
				return 0;
			}
		} while (*--s != quote);
		*flag = s > line;
		*s++ = 0;
		return s;
	}

	/*
	 * Now search for the beginning of the file name.
	 */

	while (s > line && !isspace(*s))
		s--;
	if (!*s) {
		*flag = 0;
		return 0;
	}
	if (isspace(*s)) {
		*s++ = 0;
		*flag = 1;
	}
	else
		*flag = 0;
	return s;
}

/*
 * Save/copy the indicated messages at the end of the passed file name.
 * If mark is true, mark the message "saved."
 */
static int
save1(char* str, Dt_t** ignore, unsigned long flags)
{
	register struct msg*	ip;
	register struct msg*	mp;
	char*			file;
	char*			disp;
	char*			temp;
	int			f;
	FILE*			fp;
	int			folder;
	struct mhcontext	mh;
	struct stat		st;

	if (!(file = snarf(str, &f))) {
		if (f < 0)
			return 1;
	}
	else if (streq(file, "+")) {
		file = 0;
		flags |= FOLLOWUP;
	}
	if (!f) {
		if (!(state.msg.list->m_index = first(0, MMNORM))) {
			note(0, "No messages to %s", state.cmd->c_name);
			return 1;
		}
		(state.msg.list + 1)->m_index = 0;
	}
	else if (getmsglist(str, 0) < 0)
		return 1;
	if (flags & FOLLOWUP)
		file = record(file ? file : grab(state.msg.list + state.msg.list->m_index - 1, GREPLY, NiL), flags);
	else file = expand(file ? file : "&", 1);
	if (!file)
		return 1;
	if (state.var.debug) {
		note(DEBUG, "%s to \"%s\"", state.cmd->c_name, file);
		return 0;
	}
	note(PROMPT, "\"%s\" ", file);
	folder = FFILE;
	if (stat(file, &st) < 0) {
		if (*file == '@') {
			folder = FIMAP;
			disp = "[Imap]";
		}
		else
			disp = "[New file]";
	}
	else if (S_ISDIR(st.st_mode)) {
		folder = FMH;
		disp = "[Added]";
	}
	else if (S_ISREG(st.st_mode))
		disp = state.clobber ? "[Overwritten]" : "[Appended]";
	else {
		note(0, "%s: cannot save to file", file);
		return 1;
	}
	switch (folder) {
	case FFILE:
		if (!(fp = fileopen(file, state.clobber ? "EMw" : "EMa")))
			return 1;
		for (ip = state.msg.list; ip->m_index; ip++) {
			mp = state.msg.list + ip->m_index - 1;
			touchmsg(mp);
			if (copy(mp, fp, ignore, NiL, 0) < 0) {
				note(SYSTEM, "%s", file);
				fileclose(fp);
				return 1;
			}
			if (flags & MARK)
				msgflags(mp, MSAVE, 0);
		}
		if (fileclose(fp))
			note(SYSTEM, "%s", file);
		break;
	case FIMAP:
		for (ip = state.msg.list; ip->m_index; ip++) {
			mp = state.msg.list + ip->m_index - 1;
			touchmsg(mp);
			if (imap_save(mp, file) < 0) {
				note(SYSTEM, "%s", file);
				return 1;
			}
			if (flags & MARK)
				msgflags(mp, MSAVE, 0);
		}
		break;
	case FMH:
		mhgetcontext(&mh, file, 1);
		for (ip = state.msg.list; ip->m_index; ip++) {
			mp = state.msg.list + ip->m_index - 1;
			touchmsg(mp);
			sfprintf(state.path.temp, "%s/%d", file, mh.next);
			temp = struse(state.path.temp);
			if (fp = fileopen(temp, "MEw")) {
				if (copy(mp, fp, ignore, NiL, 0) < 0)
					note(SYSTEM, "%s", temp);
				fileclose(fp);
				if (flags & MARK)
					msgflags(mp, MSAVE, 0);
				mh.next++;
			}
		}
		mhputcontext(&mh, file);
		break;
	}
	note(0, "%s", disp);
	return 0;
}

/*
 * Save a message in a file.  Mark the message as saved
 * so we can discard when the user quits.
 */
int
save(char* str)
{
	return save1(str, &state.saveignore, MARK);
}

/*
 * Save a message in a FOLLOWUP file.  Mark the message as saved
 * so we can discard when the user quits.
 */
int
Save(char* str)
{
	return save1(str, &state.saveignore, FOLLOWUP|MARK);
}

/*
 * Copy a message to a file without affected its saved-ness
 */
int
cmdcopy(char* str)
{
	return save1(str, &state.saveignore, 0);
}

/*
 * Copy a message to a FOLLOWUP file without affected its saved-ness
 */
int
Copy(char* str)
{
	return save1(str, &state.saveignore, FOLLOWUP);
}

/*
 * Write the indicated messages at the end of the passed
 * file name, minus header and trailing blank line.
 */
int
cmdwrite(char* str)
{
	return save1(str, &state.ignoreall, MARK);
}

/*
 * Delete the indicated messages.
 * Set dot to some nice place afterwards.
 * Internal interface.
 */
static int
delm(struct msg* msgvec)
{
	register struct msg*	mp;
	register struct msg*	ip;
	int			last;

	last = 0;
	for (ip = msgvec; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		touchmsg(mp);
		msgflags(mp, MDELETE|MTOUCH, MPRESERVE|MSAVE|MBOX);
		last = ip->m_index;
	}
	if (last) {
		state.msg.dot = state.msg.list + last - 1;
		last = first(0, MDELETE);
		if (last) {
			state.msg.dot = state.msg.list + last - 1;
			return 0;
		}
		else {
			state.msg.dot = state.msg.list;
			return -1;
		}
	}

	/*
	 * Following can't happen -- it keeps lint happy
	 */

	return -1;
}

/*
 * Delete messages.
 */
int
cmddelete(struct msg* msgvec)
{
	delm(msgvec);
	return 0;
}

/*
 * Delete messages, then type the new dot.
 */
int
deltype(struct msg* msgvec)
{
	int	lastdot;

	lastdot = state.msg.dot - state.msg.list + 1;
	if (delm(msgvec) >= 0) {
		if ((state.msg.list->m_index = state.msg.dot - state.msg.list + 1) > lastdot) {
			touchmsg(state.msg.dot);
			(state.msg.list + 1)->m_index = 0;
			return type(state.msg.list);
		}
		note(0, "At EOF");
	}
	else
		note(0, "No more messages");
	return 0;
}

/*
 * Undelete the indicated messages.
 */
int
undelete(struct msg* msgvec)
{
	register struct msg*	mp;
	register struct msg*	ip;

	for (ip = msgvec; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		touchmsg(mp);
		state.msg.dot = mp;
		msgflags(mp, 0, MDELETE);
	}
	return 0;
}

/*
 * Print out all currently retained fields.
 */
static int
ignoreshow(Dt_t* dt, void* object, void* context)
{
	if (((struct name*)object)->flags & *((int*)context)) {
		*((int*)context) |= HIT;
		printf("%s\n", ((struct name*)object)->name);
	}
	return 0;
}

/*
 * Low level for ignore and retain.
 */

static int
ignore1(Dt_t** ignore, char** list, unsigned long flags)
{
	register char**		ap;
	register struct name*	tp;

	if (*list) {
		for (ap = list; *ap; ap++)
			if (tp = dictsearch(ignore, *ap, INSERT|IGNORECASE))
				tp->flags = flags;
		if (*ignore)
			dictflags(ignore) |= flags;
	}
	else {
		dictwalk(ignore, ignoreshow, &flags);
		if (!(flags & HIT))
			note(0, "No fields currently being %s", (flags & RETAIN) ? "retained" : "ignored");
	}
	return 0;
}

/*
 * Add the given header fields to the retained list.
 * If no arguments, print the current list of retained fields.
 */
int
retain(char** list)
{

	return ignore1(&state.ignore, list, RETAIN);
}

int
saveretain(char** list)
{

	return ignore1(&state.saveignore, list, RETAIN);
}

/*
 * Add the given header fields to the ignored list.
 * If no arguments, print the current list of ignored fields.
 */
int
ignore(char** list)
{
	return ignore1(&state.ignore, list, IGNORE);
}

int
saveignore(char** list)
{
	return ignore1(&state.saveignore, list, IGNORE);
}

/*
 * Send mail to a bunch of user names.
 */
int
mail(char* str)
{
	struct header	head;

	memset(&head, 0, sizeof(head));
	extract(&head, GTO|GMETOO, str);
	sendmail(&head, SIGN);
	return 0;
}

/*
 * Low level for map().
 */
static int
maplist(Dt_t* dt, void* object, void* context)
{
	printf("%s ", ((struct name*)object)->name);
	return 0;
}

/*
 * List mapped user names by expanding aliases.
 */
int
map(char* str)
{
	struct header	head;

	memset(&head, 0, sizeof(head));
	extract(&head, GTO|GMETOO, str);
	usermap(&head, state.clobber);
	dictwalk(&head.h_names, maplist, NiL);
	printf("\n");
	return 0;
}

/*
 * Get an attachment from ap in the current message.
 */
static int
getatt(register struct part* ap, register char* name, unsigned long flags, off_t* lines, off_t* chars)
{
	register char*	s;
	register int	n;
	char*		cmd;
	off_t		lc;
	off_t		cc;
	FILE*		ip;
	FILE*		op;
	struct stat	st;

	if (name == ap->name) {
		cmd = 0;
		if (state.var.attachments) {
			sfprintf(state.path.temp, "%s/%s", state.var.attachments, name);
			name = expand(struse(state.path.temp), 1);
		}
		else
			name = savestr(name);
	}
	else if (!(cmd = iscmd(name)) && !(name = expand(name, 1)))
		return 1;
	if (!stat(name, &st) && S_ISDIR(st.st_mode)) {
		if (s = strrchr(ap->name, '/'))
			s++;
		else
			s = ap->name;
		if (!streq(name, ".")) {
			sfprintf(state.path.temp, "%s/%s", name, s);
			s = struse(state.path.temp);
		}
		name = savestr(s);
	}
	if (!ap->code[0] && mimeview(state.part.mime, "encoding", name, ap->type, ap->opts))
		strncopy(ap->code, ap->type, sizeof(ap->code));
	if (ap->code[0] && !isdigit(ap->code[0])) {
		sfprintf(state.path.temp, "uudecode -h -x %s", ap->code);
		if (!mimecmp("text", ap->type, NiL))
			sfprintf(state.path.temp, " -t");
		if (cmd)
			sfprintf(state.path.temp, " -o - | %s", cmd);
		else if (filestd(name, "w")) {
			if ((flags & GMIME) && mime(1) && (s = mimeview(state.part.mime, NiL, NiL, ap->type, ap->opts)))
				sfprintf(state.path.temp, " | %s", s);
			sfprintf(state.path.temp, " | %s", state.var.pager);
		}
		else
		{
			sfprintf(state.path.temp, " -o ");
			shquote(state.path.temp, name);
		}
		s = struse(state.path.temp);
		n = 1;
	}
	else if (cmd) {
		s = cmd;
		n = -1;
	}
	else if (filestd(name, "w")) {
		if ((flags & GMIME) && mime(1) && (s = mimeview(state.part.mime, NiL, NiL, ap->type, ap->opts))) {
			sfprintf(state.path.temp, "%s | %s", s, state.var.pager);
			s = struse(state.path.temp);
			n = 1;
		}
		else {
			s = state.var.pager;
			n = -1;
		}
	}
	else {
		s = name;
		n = 0;
	}
	if (!(op = n ? pipeopen(s, "w") : fileopen(s, "ERw")))
		return 1;
	ip = setinput(state.msg.dot);
	fseek(ip, ap->offset, SEEK_SET);
	if (!lines)
		lines = &lc;
	if (!chars)
		chars = &cc;
	filecopy(NiL, ip, name, op, NiL, ap->size, lines, chars, 0);
	fileclose(op);
	if (flags & GDISPLAY)
		note(0, "\"%s\" %ld/%ld", name, (long)*lines, (long)*chars);
	return 0;
}

/*
 * Low level for get/Get
 */
static int
get1(char** argv, unsigned long flags)
{
	register struct part*	ap;
	register int		i;
	register char*		s;
	char*			name;
	char*			a;
	char*			e;
	int			n;
	int			r;

	if (state.msg.dot < state.msg.list || state.msg.dot >= state.msg.list + state.msg.count) {
		note(0, "No current message");
		return 1;
	}
	if (state.folder == FIMAP)
		return imap_get1(argv, flags);
	if (!(ap = state.part.in.head) || !state.part.in.count) {
		note(0, "No attachments in current message");
		return 1;
	}
	if (!*argv) {
		do {
			if (ap->count)
				printf("(attachment %2d %s %20s \"%s\")\n", ap->count, counts(1, ap->lines, ap->size), ap->type, ap->name);
		} while (ap = ap->next);
		return 0;
	}
	if (!(a = newof(0, char, state.part.in.count, 1)))
		note(PANIC, "Out of space");
	s = *argv++;
	r = 0;
	for (;;) {
		while (isspace(*s))
			s++;
		if (!*s)
			break;
		else if (*s == ',') {
			s++;
			r = 0;
		}
		else if (*s == '*') {
			if (!r)
				r = 1;
			for (i = r; i <= state.part.in.count; i++)
				a[i] = 1;
			r = 0;
		}
		else if (*s == '-') {
			s++;
			r = 1;
		}
		else {
			n = strtol(s, &e, 0);
			if (n > 0 && n <= state.part.in.count) {
				if (r) {
					for (i = r; i <= n; i++)
						a[i] = 1;
					r = 0;
				}
				else
					a[n] = 1;
			}
			else
			{
				note(0, "%s: invalid attachment number", s);
				while (*e && !isspace(*e))
					e++;
			}
			s = e;
			if (*s == '-') {
				s++;
				r = n;
			}
		}
	}
	r = 0;
	for (i = 1; i <= state.part.in.count; i++)
		if (a[i]) {
			while (ap->count != i)
				if (!(ap = ap->next)) {
					note(0, "%d: attachment number out of range", i);
					r = 1;
					goto done;
				}
			if (name = *argv)
				argv++;
			else
				name = ap->name;
			if (getatt(ap, name, flags, NiL, NiL))
				r = 1;
		}
 done:
	free(a);
	return r;
}

/*
 * Get an attachment from the current message.
 * Execute content view command if found.
 */
int
get(char** argv)
{
	return get1(argv, GDISPLAY|GMIME);
}

/*
 * Get an attachment from the current message.
 * Don't execute content view command.
 */
int
Get(char** argv)
{
	return get1(argv, GDISPLAY);
}

/*
 * Low level for split.
 */
static int
split1(char* str, Dt_t** ignore, long num, char* dir, int verbose, int flag)
{
	register struct msg*	mp;
	register struct msg*	ip;
	register struct part*	ap;
	off_t			lc;
	off_t			cc;
	FILE*			fp;
	char*			s;
	char*			file;

	s = dir;
	if (!(dir = expand(dir, 1)) || !*dir || !isdir(dir)) {
		note(0, "\"%s\": directory argument expected", s);
		return 1;
	}
	if (getmsglist(str, 0) < 0)
		return 1;
	for (ip = state.msg.list; ip->m_index; ip++) {
		mp = state.msg.dot = state.msg.list + ip->m_index - 1;
		sfprintf(state.path.buf, "%s/%d", dir, num);
		file = struse(state.path.buf);
		if (fp = fileopen(file, "Ew")) {
			if (copy(mp, fp, ignore, NiL, GMIME) < 0)
				note(SYSTEM, "%s", dir);
			else {
				if (verbose)
					note(PROMPT, "%s %ld %ld %ld", file, num, (long)mp->m_lines, (long)mp->m_size);
				if (ap = state.part.in.head) {
					do {
						if (!(ap->flags & PART_body)) {
							sfprintf(state.path.buf, "%s/%d-%d", dir, num, ap->count);
							file = struse(state.path.buf);
							if (!getatt(ap, file, 0, &lc, &cc) && verbose)
								note(PROMPT, " %d-%d %s %s %ld %ld", num, ap->count, ap->name, ap->type, (long)lc, (long)cc);
						}
					} while (ap = ap->next);
				}
				if (verbose)
					note(0, "");
				msgflags(mp, flag, 0);
			}
			fileclose(fp);
		}
		touchmsg(mp);
		num++;
	}
	state.msg.mh.next = num;
	return 0;
}

/*
 * Split messages into idividual files.
 * low level for split and Split
 */
static int
split0(char* str, Dt_t** ignore, int flag)
{
	long	num;
	int	f;
	char*	file;
	char*	start;
	char*	e;

	if (!(file = snarf(str, &f)) && f < 0) {
		note(0, "file argument expected");
		return 1;
	}
	if (!(start = snarf(str, &f))) {
		note(0, "numeric argument expected");
		return 1;
	}
	if ((num = strtol(start, &e, 0)) < 0 || *e) {
		note(0, "\"%s\": numeric argument expected", start);
		return 1;
	}
	return split1(str, ignore, num, file, 1, flag);
}

/*
 * Split messages into idividual files.
 * Ignore all headers.
 */
int
Split(char* str)
{
	return split0(str, &state.ignoreall, MSAVE);
}

/*
 * Split messages into idividual files.
 * Keep all headers.
 */
int
split(char* str)
{
	return split0(str, NiL, MSAVE);
}

/*
 * Update the mail file with any new messages that have
 * come in since we started reading mail.
 */
int
incorporate(void)
{
	if (state.folder == FMH) {
		int	sawcom = 0;
		long	count;
		long	dot;
		long	next;

		count = state.msg.count;
		next = state.msg.mh.next;
		sawcom = state.sawcom;
		dot = state.msg.dot - state.msg.list;
		state.incorporating = 1;
		if (!setfolder("%")) {
			split1("*", NiL, next, state.path.prev, 0, MSAVE);
			if (!setfolder("#")) {
				folderinfo(count);
				state.msg.dot = state.msg.list + ((dot < 0) ? 0 : dot);
			}
		}
		state.incorporating = 0;
		state.sawcom = sawcom;
	}
	else {
		int	eof;
		int	n;

		eof = (state.msg.dot + 1) >= state.msg.list + state.msg.count;
		if ((n = incfile()) < 0)
			note(0, "The \"%s\" command failed", state.cmd->c_name);
		else if (!n)
			note(0, "No new mail");
		else {
			state.msg.dot = folderinfo(state.msg.count - n);
			if (eof)
				state.sawcom = 0;
		}
	}
	return 0;
}
