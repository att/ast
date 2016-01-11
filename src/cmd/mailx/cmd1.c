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
 * User commands.
 */

#include "mailx.h"

/*
 * Print out the header of a specific message.
 * This is a slight improvement to the standard one.
 */
static void
printhead(int mesg, int who)
{
	register struct msg*	mp;
	char*			name;
	char*			sizes;
	char*			subjline;
	int			curind;
	int			dispc;
	int			subjlen;
	struct headline		hl;
	char			pbuf[LINESIZE];
	char			headline[LINESIZE];

	mp = state.msg.list + mesg - 1;
	readline(setinput(mp), headline, sizeof(headline));
	subjline = grab(mp, GSUB, NiL);
	curind = state.msg.dot == mp && !state.var.justheaders ? '>' : ' ';
	parse(mp, headline, &hl, pbuf, sizeof(pbuf));
	if (mp->m_flag & MBOX)
		dispc = 'M';
	else if (mp->m_flag & MPRESERVE)
		dispc = 'P';
	else if (mp->m_flag & MSAVE)
		dispc = '*';
	else if (mp->m_flag & MSPAM)
	{
		dispc = 'X';
		if (subjline && state.var.spamsubhead)
		{
			register int	c;
			register char*	s;
			register char*	t;
			register char*	u;

			s = t = subjline;
			while (c = *s++)
			{
				if (!isalnum(c) && c == *s)
				{
					while (*s == c)
						s++;
					for (u = s; *u && *u != c; u++);
					if (*u == c && *(u + 1) == c)
					{
						for (s = u; *s == c; s++);
						if (t == subjline)
							while (isspace(*s))
								s++;
						continue;
					}
				}
				*t++ = c;
			}
			*t = 0;
		}
	}
	else if (!(mp->m_flag & (MREAD|MNEW)))
		dispc = 'U';
	else if ((mp->m_flag & (MREAD|MNEW)) == MNEW)
		dispc = 'N';
	else
		dispc = who ? 'R' : ' ';
	if (who)
	{
		if (!(name = grab(mp, GSENDER|GDISPLAY, NiL)))
			name = hl.l_from;
		if (!state.var.domain || strchr(name, '@'))
			printf("%c %s\n", dispc, name);
		else
			printf("%c %s@%s\n", dispc, name, state.var.domain);
	}
	else
	{
		name = grab(mp, (state.var.news ? GNEWS : state.var.showto && sender(state.var.user, mesg) ? GTO : GREPLY)|GDISPLAY, NiL);
		sizes = counts(!!state.var.news, mp->m_lines, mp->m_size);
		subjlen = state.screenwidth - 50 - strlen(sizes);
		if (subjline && subjlen >= 0)
			printf("%c%c%3d %-20.20s  %16.16s %s %.*s\n",
				curind, dispc, mesg, name, hl.l_date, sizes,
				subjlen, subjline);
		else
			printf("%c%c%3d %-20.20s  %16.16s %s\n",
				curind, dispc, mesg, name, hl.l_date, sizes);
	}
}

/*
 * Print the current active headings.
 * Don't change dot if invoker didn't give an argument.
 */

int
headers(struct msg* msgvec)
{
	register int		n;
	register int		mesg;
	register int		flag;
	register struct msg*	mp;
	int			m;

	if (state.var.justfrom) {
		flag = 1;
		mp = state.msg.list;
		if (state.var.justfrom > 0 && state.var.justfrom < state.msg.count)
			mp += state.msg.count - state.var.justfrom;
		m = state.msg.count + 1;
	}
	else {
		flag = 0;
		n = msgvec->m_index;
		if ((m = state.var.screen) <= 0 && (m = state.msg.count) <= 0)
			m = 1;
		if (n != 0)
			state.scroll = (n - 1) / m;
		if (state.scroll < 0)
			state.scroll = 0;
		mp = state.msg.list + state.scroll * m;
		if (mp >= state.msg.list + state.msg.count)
			mp = state.msg.list + state.msg.count - m;
		if (mp < state.msg.list)
			mp = state.msg.list;
		if (state.msg.dot != state.msg.list + n - 1)
			state.msg.dot = mp;
	}
	for (mesg = mp - state.msg.list; mp < state.msg.list + state.msg.count; mp++) {
		mesg++;
		if (mp->m_flag & (MDELETE|MNONE))
			continue;
		CALL(printhead)(mesg, !!state.var.justfrom);
		if (++flag >= m)
			break;
	}
	if (!flag) {
		note(0, "No more mail");
		return 1;
	}
	return 0;
}

/*
 * Scroll to the next/previous screen
 */
int
scroll(char* arg)
{
	register int	s;

	s = state.scroll;
	switch (*arg) {
	case 0:
	case '+':
		s++;
		if (s * state.var.screen > state.msg.count) {
			note(0, "On last screenful of messages");
			return 0;
		}
		state.scroll = s;
		break;

	case '-':
		if (--s < 0) {
			note(0, "On first screenful of messages");
			return 0;
		}
		state.scroll = s;
		break;

	default:
		note(0, "Unrecognized scrolling command \"%s\"", arg);
		return 1;
	}
	state.msg.list->m_index = 0;
	return headers(state.msg.list);
}

/*
 * from or From
 */
static int
from2(struct msg* msgvec, int who)
{
	register struct msg*	ip;

	for (ip = msgvec; ip->m_index; ip++)
		CALL(printhead)(ip->m_index, who);
	if (--ip >= msgvec)
		state.msg.dot = state.msg.list + ip->m_index - 1;
	return 0;
}

/*
 * Print out the headlines for each message
 * in the passed message list.
 */
int
from(struct msg* msgvec)
{
	return from2(msgvec, 0);
}

/*
 * Print out the status, message number, and sender
 * in the passed message list.
 */
int
From(struct msg* msgvec)
{
	return from2(msgvec, 1);
}

/*
 * Print out the value of dot.
 */
int
dot(void)
{
	note(0, "%d", state.msg.dot - state.msg.list + 1);
	return 0;
}

/*
 * Print out all the possible commands.
 */
int
list(void)
{
	register int			i;
	register int			j;
	register int			cmds;
	register int			cols;
	register int			rows;

	cmds = state.cmdnum;
	cols = 5;
	rows = (cmds + cols - 1) / cols;
	for (i = 0; i < rows; i++) {
		for (j = i; j < cmds; j += rows)
			printf("%-15s", state.cmdtab[j].c_name);
		putchar('\n');
	}
	return 0;
}

/*
 * Type out the messages requested.
 */
static int
type1(struct msg* msgvec, Dt_t** ignore, int page, unsigned long flags)
{
	register struct msg*	ip;
	register struct msg*	mp;
	int			nlines;
	int			sig;
	FILE*			obuf;

	obuf = stdout;
	if (sig = setjmp(state.jump.sigpipe))
		resume(sig);
	else {
		if (!state.more.discipline && state.var.interactive && (page || state.var.crt)) {
			nlines = 0;
			if (!page) {
				for (ip = msgvec; ip->m_index; ip++)
					nlines += state.msg.list[ip->m_index - 1].m_lines;
			}
			if (page || nlines > state.var.crt) {
				if (!(obuf = pipeopen(state.var.pager, "Jw")))
					obuf = stdout;
			}
		}
		for (ip = msgvec; ip->m_index; ip++) {
			mp = state.msg.dot = state.msg.list + ip->m_index - 1;
			touchmsg(mp);
			if (!state.var.quiet)
				fprintf(obuf, "Message %d:\n", ip->m_index);
			copy(mp, obuf, ignore, NiL, flags);
		}
	}
	if (obuf != stdout)
		fileclose(obuf);
	return 0;
}

/*
 * Paginate messages, honor ignored fields.
 */
int
more(struct msg* msgvec)
{
	return type1(msgvec, &state.ignore, 1, GMIME);
}

/*
 * Paginate messages, even printing ignored fields.
 */
int
More(struct msg* msgvec)
{

	return type1(msgvec, NiL, 1, 0);
}

/*
 * Type out messages, honor ignored fields.
 */
int
type(struct msg* msgvec)
{

	return type1(msgvec, &state.ignore, 0, GMIME);
}

/*
 * Type out messages, even printing ignored fields.
 */
int
Type(struct msg* msgvec)
{

	return type1(msgvec, NiL, 0, 0);
}

/*
 * Print the top so many lines of each desired message.
 * The number of lines is taken from the variable "toplines"
 * and defaults to 5.
 */
int
top(struct msg* msgvec)
{
	register struct msg*	ip;
	register struct msg*	mp;
	int			c;
	int			lines;
	int			lineb;
	char			linebuf[LINESIZE];
	FILE*			ibuf;

	lineb = 1;
	for (ip = msgvec; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		touchmsg(mp);
		state.msg.dot = mp;
		if (!state.var.quiet)
			printf("Message %d:\n", ip->m_index);
		ibuf = setinput(mp);
		c = mp->m_lines;
		if (!lineb)
			printf("\n");
		for (lines = 0; lines < c && lines <= state.var.toplines; lines++) {
			if (readline(ibuf, linebuf, sizeof(linebuf)) < 0)
				break;
			puts(linebuf);
			lineb = blankline(linebuf);
		}
	}
	return 0;
}

/*
 * List the folders the user currently has.
 */
int
folders(void)
{
	char*	cmd;
	char	dirname[LINESIZE];

	if (state.folder == FIMAP)
		return imap_folders();
	if (getfolder(dirname, sizeof(dirname)) < 0) {
		note(0, "No value set for \"%s\"", state.cmd->c_name);
		return 1;
	}
	if (!(cmd = state.var.lister))
		cmd = "ls";
	run_command(cmd, 0, -1, -1, dirname, NiL, NiL);
	return 0;
}

/*
 * Pipe messages through command.
 */
int
cmdpipe(char* str)
{
	register struct msg*	mp;
	register struct msg*	ip;
	int			f;
	char*			cmd;
	char*			s;
	char*			mode;
	off_t			lc;
	off_t			cc;
	FILE*			fp;

	fp = 0;
	if (f = setjmp(state.jump.sigpipe)) {
		resume(f);
		if (fp)
			fileclose(fp);
		return 1;
	}
	if (!(cmd = snarf(str, &f))) {
		if (f < 0)
			return 1;
		if (!(cmd = state.var.cmd)) {
			note(0, "\"cmd\" variable not set");
			return 1;
		}
	}
	if (!f) {
		if (!(state.msg.list->m_index = first(0, MMNORM))) {
			note(0, "No messages to %s", cmd);
			return 1;
		}
		(state.msg.list + 1)->m_index = 0;
	}
	else if (getmsglist(str, 0) < 0)
		return 1;
	if (*(s = cmd + strlen(cmd) - 1) == '&') {
		*s = 0;
		mode = "JNw";
	}
	else
		mode = "Jw";
	if (!(cmd = expand(cmd, 0)))
		return 1;
	if (!(fp = pipeopen(cmd, mode)))
		return 1;
	note(0, "Pipe to: \"%s\"", cmd);
	lc = cc = 0;
	for (ip = state.msg.list; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		touchmsg(mp);
		if (copy(mp, fp, NiL, NiL, 0) < 0) {
			note(SYSTEM, "\"%s\"", cmd);
			fileclose(fp);
			return 1;
		}
		if (state.var.page)
			putc('\f', fp);
		lc += mp->m_lines;
		cc += mp->m_size;
	}
	if (fp)
		fileclose(fp);
	note(0, "\"%s\" %ld/%ld", cmd, (long)lc, (long)cc);
	return 0;
}

/*
 * Single quote s to fp.
 */
static void
quote(FILE* fp, register char* s)
{
	register int	c;

	for (;;) {
		switch (c = *s++) {
		case 0:
			break;
		case '\'':
			fprintf(fp, "'\\''");
			continue;
		default:
			fputc(c, fp);
			continue;
		}
		break;
	}
}

/*
 * Dump the blasted header fields.
 */
static int
blastdump(Dt_t* dt, void* object, void* context)
{
	register struct name*	hp = (struct name*)object;

	printf("header[%s]='", hp->name);
	quote(stdout, (char*)hp->value);
	printf("'\n");
	hp->flags = 0;
	hp->value = 0;
	return 0;
}

/*
 * Low level for blast, Blast.
 */
static int
blast1(struct msg* msgvec, Dt_t** ignore)
{
	register int		n;
	register struct msg*	ip;
	register struct msg*	mp;
	int			first;
	struct name*		hp;
	char*			next;
	char			tmp[LINESIZE];
	Dt_t*			headers = 0;
	struct parse		pp;

	for (ip = msgvec; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;

		/*
		 * Collect the header names and sizes.
		 */

		if (headset(&pp, mp, NiL, NiL, ignore, 0))
			while (headget(&pp))
				if (hp = dictsearch(&headers, pp.name, INSERT|IGNORECASE|STACK))
					hp->flags = pp.length - (pp.data - pp.buf);

		/*
		 * Collect the header values.
		 */

		if (headset(&pp, mp, NiL, NiL, ignore, 0)) {
			printf("from='");
			quote(stdout, pp.data);
			printf("'\n");
			next = tmp;
			while (headget(&pp))
				if (hp = dictsearch(&headers, pp.name, LOOKUP)) {
					if (!hp->value) {
						hp->value = (void*)next;
						next += hp->flags + 1;
						if (next > &tmp[sizeof(tmp)]) {
							note(0, "Too many headers");
							goto skip;
						}
						*((char*)hp->value) = 0;
					}
					strcat((char*)hp->value, pp.data);
				}
		}

		/*
		 * Dump the header values.
		 */

		dictwalk(&headers, blastdump, NiL);

		/*
		 * Dump the message text.
		 */

		first = 1;
		printf("text='");
		while (pp.count > 0 && fgets(pp.buf, sizeof(pp.buf), pp.fp)) {
			if (n = strlen(pp.buf)) {
				pp.count -= n;
				pp.buf[n - 1] = 0;
			}
			if (first)
				first = 0;
			else
				putchar('\n');
			quote(stdout, pp.buf);
		}
		printf("'\n");
	skip:	;
	}
	return 0;
}

/*
 * Blast message into name=value pairs.
 */
int
blast(struct msg* msgvec)
{
	return blast1(msgvec, &state.ignore);
}

/*
 * Blast message into name=value pairs, including ignored fields.
 */
int
Blast(struct msg* msgvec)
{
	return blast1(msgvec, NiL);
}

/*
 * Add/delete/list content capabilities.
 */
int
capability(register char** argv)
{
	register char*		s;
	register char*		t;

	if (!mime(1))
		return 0;
	if (!(s = *argv++))
		mimelist(state.part.mime, stdout, NiL);
	else {
		if ((t = s + strlen(s)) > s && *--t == ';')
			*t = 0;
		else
			t = 0;
		if (!*argv) {
			if (t)
				mimeset(state.part.mime, s, MIME_REPLACE);
			else if (!mimelist(state.part.mime, stdout, s))
				note(0, "\"%s\": unknown capability", s);
		}
		else if (streq(s, "<")) {
			while (s = *argv++)
				if (mimeload(state.part.mime, s, MIME_REPLACE))
					note(SYSTEM, "%s: mime load error", s);
		}
		else {
			sfprintf(state.path.buf, "%s;", s);
			while (s = *argv++)
				sfprintf(state.path.buf, " %s", s);
			mimeset(state.part.mime, struse(state.path.buf), MIME_REPLACE);
		}
	}
	return 0;
}

/*
 * Mark names.
 */
struct mark {
	const char*	name;
	int		flag;
	int		set;
	int		clear;
};

static const struct mark marks[] = {

"delete",	MDELETE,	MDELETE,		0,
"dot",		MMARK,		MMARK,			0,
"mbox",		MBOX,		MBOX|MTOUCH,		MPRESERVE|MSPAM,
"new",		MNEW,		MNEW,			MDELETE|MREAD|MSPAM,
"preserve",	MPRESERVE,	MPRESERVE,		MBOX|MSPAM,
"read",		MREAD,		MREAD|MTOUCH|MSTATUS,	MNEW,
"save",		MSAVE,		MSAVE,			0,
"scan",		MSCAN,		MSCAN,			0,
"spam",		MSPAM,		MSPAM,			0,
"touch",	MTOUCH,		MTOUCH,			MPRESERVE,

};

/*
 * Mark the indicated messages with the named mark or flags by default.
 */
static int
mark1(char* str, int set, int clr)
{
	register struct msg*		ip;
	register struct msg*		mp;
	register const struct mark*	kp;
	register char*			mark;
	register char*			next;
	int				f;
	int				no;

	if ((mark = snarf(str, &f)) && isalpha(*mark)) {
		set = clr = 0;
		do {
			if ((next = strchr(mark, ',')) || (next = strchr(mark, '|')))
				*next++ = 0;
			no = 0;
			switch (lower(mark[0])) {
			case 'n':
				if (lower(mark[1]) == 'o')
					no = 2;
				break;
			case 'u':
				if (lower(mark[1]) == 'n')
					no = 2;
				break;
			}
			mark += no;
			if (!(kp = (struct mark*)strsearch(marks, elementsof(marks), sizeof(*marks), strcasecmp, mark, NiL))) {
				note(0, "%s: unknown mark", mark);
				return 1;
			}
			if (no) {
				set |= kp->clear;
				clr |= kp->set;
			}
			else {
				set |= kp->set;
				clr |= kp->clear;
			}
		} while (mark = next);
	}
	else if (f < 0)
		return 1;
	else {
		if (f)
			*(mark - 1) = ' ';
		else if (mark)
			f = 1;
		if (!clr) {
			if (!set)
				set = MMARK;
			else
				for (kp = marks; kp < &marks[elementsof(marks)]; kp++)
					if (kp->flag & set) {
						set |= kp->set;
						clr |= kp->clear;
					}
		}
	}
	if (!f) {
		if (!state.msg.list || !(state.msg.list->m_index = first(0, MMNORM))) {
			note(0, "No messages to %s", state.cmd->c_name);
			return 1;
		}
		(state.msg.list + 1)->m_index = 0;
	}
	else if (getmsglist(str, 0) < 0)
		return 1;
	clr |= MMARK;
	for (mp = 0, ip = state.msg.list; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		msgflags(mp, set, clr);
	}
	if (mp) {
		if (set & MMARK)
			state.msg.dot = mp;
		else if ((set|clr) & MSPAM) {
			state.msg.dot = mp;
			if (f = first(0, MDELETE|MSPAM))
				state.msg.dot = state.msg.list + f - 1;
		}
	}
	return 0;
}

/*
 * Mark all given messages to the given state.
 */
int
mark(char* str)
{
	return isupper(*state.cmd->c_name) ? mark1(str, 0, MSPAM) : mark1(str, MSPAM, 0);
}

/*
 * Touch all the given messages so that they will
 * get mboxed.
 */
int
cmdtouch(char* str)
{
	return mark1(str, MTOUCH, 0);
}

/*
 * Make sure all passed messages get mboxed.
 */
int
mboxit(char* str)
{
	return mark1(str, MBOX, 0);
}

/*
 * Preserve the named messages, so that they will be sent
 * back to the system mailbox.
 */
int
preserve(char* str)
{
	return mark1(str, MPRESERVE, 0);
}

/*
 * Mark all given messages as unread.
 */
int
unread(char* str)
{
	return mark1(str, MREAD, 0);
}

/*
 * For unimplemented commands.
 */
int
notyet(char* str)
{
	note(0, "\"%s\": command not implemented yet", state.cmd->c_name);
	return 0;
}

/*
 * Duplicate messages to address preserving the original senders.
 */
int
duplicate(char* str)
{
	register struct msg*	ip;
	register struct msg*	mp;
	int			f;
	char*			addr[2];

	if (!(addr[0] = snarf(str, &f))) {
		note(0, "Recipent address required");
		return 1;
	}
	if (getmsglist(str, 0) < 0)
		return 1;
	if (state.var.debug) {
		note(DEBUG, "%s to \"%s\"", state.cmd->c_name, addr[0]);
		return 0;
	}
	addr[1] = 0;
	for (ip = state.msg.list; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		if (sendsmtp(setinput(mp), state.var.smtp, addr, mp->m_size))
			return 1;
	}
	return 0;
}
