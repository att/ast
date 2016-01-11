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
 * File I/O.
 */

#include "mailx.h"

#include <fts.h>

/*
 * Return next msg struct for current folder.
 */
struct msg*
newmsg(off_t offset)
{
	register struct msg*	mp;
	unsigned long		dot;

	if (state.msg.count >= state.msg.size) {
		dot = state.msg.dot - state.msg.list;
		state.msg.size += 256;
		if (!(state.msg.list = newof(state.msg.list, struct msg, state.msg.size, 0)))
			note(PANIC, "Insufficient memory for %d messages", state.msg.count);
		state.msg.dot = state.msg.list + dot;
	}
	mp = state.msg.list + state.msg.count++;
	mp->m_index = 0;
	mp->m_flag = MUSED|MNEW;
	mp->m_size = 0;
	mp->m_lines = 0;
	mp->m_block = blocknumber(offset);
	mp->m_offset = blockoffset(offset);
	mp->m_info = 0;
	return mp;
}

/*
 * Set up the input pointers while copying the mail file into /tmp.
 */
void
setptr(register FILE* ibuf, off_t offset)
{
	register struct msg*	mp;
	register int		c;
	register int		count;
	register int		prevcount;
	register char*		cp;
	register char*		cp2;
	struct match*		xp;
	int			maybe;
	int			inhead;
	off_t			roff;
	off_t			zoff;
	char			buf[LINESIZE];

	if (roff = offset) {
		/* Seek into the file to get to the new messages */
		fseek(ibuf, offset, SEEK_SET);
		/*
		 * We need to make "offset" a pointer to the end of
		 * the temp file that has the copy of the mail file.
		 * If any messages have been edited, this will be
		 * different from the offset into the mail file.
		 */
		fseek(state.msg.op, (off_t)0, SEEK_END);
		offset = ftell(state.msg.op);
	}
	else
		state.msg.count = 0;
	state.folder = FFILE;
	maybe = 1;
	inhead = 0;
	mp = 0;
	count = 0;
	for (;;) {
		cp = buf;
		if (state.var.headfake) {
			state.var.headfake = 0;
			strcopy(cp, "From bozo@bigtop Wed Feb 29 00:00:00 2012\r\n");
		}
		else if (!fgets(cp, LINESIZE, ibuf))
			break;
		prevcount = count;
		count = strlen(cp);
		if (count == 0 && (zoff = ftell(ibuf)) > roff) {
			if ((count = zoff - roff) > LINESIZE)
				count = LINESIZE;
			for (cp2 = cp + count; cp < cp2 && *cp == 0; cp++);
			if (count = cp - buf)
				note(WARNING, "%d nul%s at offset %lld", count, count == 1 ? "" : "s", (Sflong_t)roff);
			count = cp2 - cp;
		}
		else if (count >= 2 && cp[count - 1] == '\n' && cp[count - 2] == '\r') {
			cp[count - 2] = '\n';
			count--;
		}
		if (fwrite(cp, 1, count, state.msg.op) != count)
			note(FATAL|SYSTEM, "Temporary file");
		cp[count - 1] = 0;
		if (state.bodymatch && !inhead && mp && !(mp->m_flag & MSCAN) && count >= state.bodymatch->minline && state.bodymatch->beg[cp[0]] && state.bodymatch->mid[cp[state.bodymatch->minline/2]] && state.bodymatch->end[cp[state.bodymatch->minline-1]])
			for (xp = state.bodymatch->match; xp; xp = xp->next)
				if (count >= xp->length && cp[0] == xp->beg && cp[xp->length/2] == xp->mid && cp[xp->length-1] == xp->end && !memcmp(cp, xp->string, xp->length))
				{
					if (TRACING('x'))
						note(0, "spam: body match `%s'", xp->string);
					msgflags(mp, MSCAN|MSPAM, 0);
					break;
				}
		if (maybe && ishead(cp, inhead || prevcount != 1)) {
			mp = newmsg(offset);
			inhead = 1;
		}
		else if (maybe = !cp[0])
			inhead = 0;
		else if (inhead) {
			for (cp2 = "status";; cp++) {
				if ((c = *cp2++) == 0) {
					while (isspace(*cp++))
						;
					if (cp[-1] != ':')
						break;
					for (;;) {
						switch (*cp++) {
						case 0:
							break;
						case 'O':
							msgflags(mp, 0, MNEW);
							goto scanned;
						case 'R':
							msgflags(mp, MREAD, 0);
							goto scanned;
						case 'X':
							msgflags(mp, MSPAM, 0);
							goto scanned;
						default:
							continue;
						scanned:
							if (!state.edit)
								msgflags(mp, MSCAN, 0);
							continue;
						}
						break;
					}
					inhead = 0;
					break;
				}
				if (c != lower(*cp))
					break;
			}
		}
		roff += count;
		offset += count;
		if (mp) {
			mp->m_size += count;
			mp->m_lines++;
		}
	}
}

/*
 * fts sort order to find max entry
 */
static int
dirmax(FTSENT* const* a, FTSENT* const* b)
{
	return strtol((*b)->fts_name, NiL, 10) - strtol((*a)->fts_name, NiL, 10);
}

/*
 * mh context file names
 */
static const char* mh_context[] = {
	".current",
	".mh_sequences",
	".exmhcontext",
	"../.exmhcontext",
	"../../.exmhcontext",
};

/*
 * Return the mh message context for folder name.
 */
void
mhgetcontext(register struct mhcontext* xp, const char* name, int next)
{
	register char*		s;
	register int		n;
	register int		i;
	register FTS*		fts;
	register FTSENT*	ent;
	char*			e;
	FILE*			fp;
	struct stat		ds;
	struct stat		fs;
	char			buf[LINESIZE];

	xp->type = 0;
	xp->dot = xp->next = 0;
	i = strlen(name);
	for (n = 0; n < elementsof(mh_context); n++) {
		sfprintf(state.path.temp, "%s/%s", name, mh_context[n]);
		if (fp = fileopen(struse(state.path.temp), "r")) {
			while (s = fgets(buf, sizeof(buf), fp)) {
				if (!strncasecmp(s, "mhcurmsg=", 9)) {
					xp->dot = strtol(s + 9, &e, 10);
					for (s = e; isspace(*s); s++);
					if (!strncasecmp(s, "mhlastmsg=", 10))
						xp->next = strtol(s + 10, NiL, 10) + 1;
					if (next && !fstat(fileno(fp), &fs) && !stat(name, &ds) && fs.st_mtime >= ds.st_mtime)
						next = 0;
					xp->type = n + 1;
					break;
				}
				else if (!strncasecmp(s, "cur:", 4)) {
					xp->dot = strtol(s + 4, NiL, 10);
					xp->type = n + 1;
					break;
				}
				else if (!strncasecmp(s, "atr-cur-", 8) && !strncmp(s += 8, name, i) && *(s += i) == ':') {
					xp->dot = strtol(s + 1, NiL, 10);
					xp->type = n + 1;
					break;
				}
			}
			fileclose(fp);
		}
	}
	if (next && (fts = fts_open((char**)name, FTS_ONEPATH|FTS_NOCHDIR|FTS_NOPOSTORDER, dirmax))) {
		while (ent = fts_read(fts)) {
			xp->next = strtol(ent->fts_name, &e, 10) + 1;
			if (!*e)
				break;
			if (ent->fts_level > 0)
				fts_set(NiL, ent, FTS_SKIP);
		}
		fts_close(fts);
	}
	if (xp->next <= 0)
		xp->next = 1;
	if (next && !xp->type)
		xp->type = 1;
	xp->old.dot = xp->dot;
	xp->old.next = xp->next;
}

/*
 * Update the mh message context for folder name.
 */
void
mhputcontext(register struct mhcontext* xp, const char* name)
{
	FILE*		fp;

	if (xp->type == 1) {
		if (xp->dot != xp->old.dot || xp->next != xp->old.next) {
			sfprintf(state.path.temp, "%s/%s", name, mh_context[xp->type - 1]);
			if (fp = fileopen(struse(state.path.temp), "Ew")) {
				fprintf(fp, "MhCurmsg=%ld MhLastmsg=%ld\n", xp->dot, xp->next - 1);
				fileclose(fp);
			}
		}
	}
	else if (xp->type) {
		if (xp->dot != xp->old.dot) {
			note(0, "%s: don't know how to update context yet", name);
		}
	}
}

/*
 * fts sort order
 */
static int
dircmp(FTSENT* const* a, FTSENT* const* b)
{
	return strtol((*a)->fts_name, NiL, 10) - strtol((*b)->fts_name, NiL, 10);
}

/*
 * Set up pointers to folder directory.
 */
int
mh_setptr(char* name, int isedit)
{
	register struct msg*	mp;
	register FTS*		fts;
	register FTSENT*	ent;
	register char*		s;
	register int		n;
	unsigned long		index;
	unsigned long		next;
	int			status;
	char*			e;
	FILE*			fp;
	char			buf[LINESIZE];

	if (!(fts = fts_open((char**)name, FTS_ONEPATH|FTS_NOCHDIR|FTS_NOPOSTORDER, dircmp))) {
		note(SYSTEM, "%s", name);
		return -1;
	}
	holdsigs();
	settmp(name, 1);
	state.edit = isedit;
	state.mailsize = 0;
	state.msg.count = 0;
	state.msg.active = 0;
	next = 0;
	mp = 0;
	while (ent = fts_read(fts)) {
		index = strtol(ent->fts_name, &e, 10);
		if (!*e) {
			next = index;
			if ((ent->fts_info & FTS_F) && (fp = fileopen(ent->fts_accpath, "Er"))) {
				for (;;) {
					mp = newmsg(0);
					if (state.msg.count >= index)
						break;
					mp->m_flag = MUSED|MNONE;
				}
				mp->m_size = ent->fts_statp->st_size;
				status = 0;
				while (s = fgets(buf, sizeof(buf), fp)) {
					mp->m_lines++;
					if (!status) {
						n = strlen(s);
						if (n <= 1)
							status = 1;
						else if (!strncasecmp(s, "status:", 7)) {
							status = 1;
							for (s += 7;;) {
								switch (*s++) {
								case 0:
									break;
								case 'O':
									msgflags(mp, 0, MNEW);
									continue;
								case 'R':
									msgflags(mp, MREAD, 0);
									continue;
								case 'X':
									msgflags(mp, MSPAM, 0);
									continue;
								default:
									continue;
								}
								break;
							}
						}
					}
				}
				fileclose(fp);
			}
			else
				fts_set(NiL, ent, FTS_SKIP);
		}
		else if (ent->fts_level > 0)
			fts_set(NiL, ent, FTS_SKIP);
	}
	fts_close(fts);
	mhgetcontext(&state.msg.mh, name, 0);
	state.folder = FMH;
	while (state.msg.mh.dot < state.msg.count && ((state.msg.list + state.msg.mh.dot - 1)->m_flag & MNONE))
		state.msg.mh.dot++;
	if (state.msg.mh.dot > 0 && state.msg.mh.dot <= state.msg.count)
		state.msg.context = state.msg.list + state.msg.mh.dot - 1;
	state.msg.mh.next = next + 1;
	if (!state.msg.list) {
		newmsg(0);
		state.msg.count = 0;
	}
	state.msg.inbox = state.var.inbox && (s = expand(state.var.inbox, 1)) && streq(s, state.path.mail);
	relsesigs();
	state.sawcom = 0;
	return 0;
}

/*
 * Drop the passed line onto the passed output buffer.
 * If a write error occurs, return -1, else the count of
 * characters written, including the newline.
 */
int
putline(FILE* obuf, char* buf)
{
	register int	c;
	register int	x;

	x = 1;
	if (strneq(buf, "From ", 5)) {
		if (putc('>', obuf) == EOF)
			return -1;
		x++;
	}
	c = strlen(buf);
	if (fwrite(buf, 1, c, obuf) != c || putc('\n', obuf) == EOF)
		return -1;
	return c + x;
}

/*
 * Read up a line from the specified input into the line
 * buffer.  Return the number of characters read.  Do not
 * include the newline at the end.
 */
int
readline(FILE* ibuf, char* buf, int size)
{
	register int	n;

	clearerr(ibuf);
	if (!fgets(buf, size, ibuf))
		return -1;
	n = strlen(buf);
	if (n > 0 && buf[n - 1] == '\n') {
		buf[--n] = 0;
		if (n > 0 && buf[n - 1] == '\r')
			buf[--n] = 0;
	}
	return n;
}

/*
 * Return a file buffer all ready to read up the
 * passed message pointer.
 */
FILE*
setinput(register struct msg* mp)
{
	FILE*	fp;
	off_t	offset;

	if (state.folder == FIMAP) {
		fp = imap_setinput(mp);
		offset = 0;
	}
	else if (state.folder == FFILE || (mp->m_flag & MODIFY)) {
		if (state.msg.op)
			fflush(state.msg.op);
		fp = state.msg.ip;
		offset = blockposition(mp->m_block, mp->m_offset);
	}
	else {
		if (state.msg.active != mp) {
			state.msg.active = mp;
			if (state.msg.ap)
				fileclose(state.msg.ap);
			sfprintf(state.path.temp, "%s/%d", state.path.mail, mp - state.msg.list + 1);
			if (!(state.msg.ap = fileopen(struse(state.path.temp), "EIr")) && !(state.msg.ap = fileopen("/dev/null", "EIr")))
				note(PANIC|SYSTEM, "Empty file open");
		}
		fp = state.msg.ap;
		offset = 0;
	}
	if (fseek(fp, offset, SEEK_SET) < 0)
		note(PANIC|SYSTEM, "Temporary file seek");
	if (!(mp->m_flag & MSCAN)) {
		msgflags(mp, MSCAN, 0);
		if (state.var.spam && !(mp->m_flag & MSPAM)) {
			if (spammed(mp))
				msgflags(mp, MSPAM, 0);
			if (fseek(fp, offset, SEEK_SET) < 0)
				note(PANIC|SYSTEM, "Temporary file seek");
		}
	}
	return fp;
}

/*
 * Delete a file, but only if the file is a plain file.
 */
int
rm(char* name)
{
	if (!isreg(name)) {
		errno = EFTYPE;
		return -1;
	}
	return remove(name);
}

/*
 * Determine the size of the file possessed by
 * the passed buffer.
 */
off_t
filesize(FILE* fp)
{
	struct stat	st;

	if (fstat(fileno(fp), &st) < 0)
		return 0;
	return st.st_size;
}

#if !_lib_ftruncate
int
ftruncate(int fd, off_t off)
{
	return 0;
}
#endif

/*
 * Truncate a file to the last character written. This is
 * useful just before closing an old file that was opened
 * for read/write.
 */
int
filetrunc(FILE* fp)
{
	return (fflush(fp) || ftruncate(fileno(fp), ftell(fp))) ? -1 : 0;
}

/*
 * Evaluate the string given as a new mailbox name.
 * Supported meta characters:
 *	%	for my system mail box
 *	%user	for user's system mail box
 *	#	for previous file
 *	&	invoker's mbox file
 *	+file	file in folder directory
 *	@name	IMAP folder
 *	any shell meta character
 * Return the file name as a dynamic string.
 */
char*
expand(register char* name, int verify)
{
	register int	n;
	FILE*		fp;
	char		buf[LINESIZE];
	char		cmd[LINESIZE];

	/*
	 * The order of evaluation is "%" and "#" expand into constants.
	 * "&" can expand into "+".  "+" can expand into shell meta characters.
	 * Shell meta characters expand into constants.
	 * This way, we make no recursive expansion.
	 */
 again:
	switch (name[0]) {
	case '%':
		if (!*++name)
			name = state.var.user;
		else if (!isalnum(*name))
			goto again;
		return mailbox(name, state.var.mail);
	case '#':
		if (name[1])
			break;
		if (!state.path.prev[0]) {
			note(0, "No previous file");
			return 0;
		}
		return savestr(state.path.prev);
	case '&':
		if (!name[1])
			name = state.var.mbox;
		break;
	}
	if (name[0] == '+' && getfolder(cmd, sizeof(cmd)) >= 0) {
		sfsprintf(buf, sizeof(buf), "%s/%s", cmd, name + 1);
		name = buf;
	}
	if (name[0] == '~' && (name[1] == '/' || name[1] == 0)) {
		if (name == buf) {
			strncopy(cmd, name, sizeof(cmd));
			name = cmd;
		}
		sfsprintf(buf, sizeof(buf), "%s%s", state.var.home, name + 1);
		name = buf;
	}
	if (!anyof(name, "~{[*?$`'\";<>"))
		return savestr(name);
	sfsprintf(cmd, sizeof(cmd), "echo %s", name);
	name = cmd + 5;
	if (!(fp = pipeopen(cmd, "r")))
		return 0;
	n = fread(buf, 1, sizeof(buf), fp);
	note(DEBUG, "expand: n=%d \"%-*.s\"", n, n, buf);
	if (fileclose(fp)) {
		note(0, "\"%s\": expansion failed", name);
		return 0;
	}
	if (n < 0) {
		note(SYSTEM, "%s: read error", name);
		return 0;
	}
	if (n >= sizeof(buf)) {
		note(0, "\"%s\": expansion buffer overflow", name);
		n = sizeof(buf) - 1;
	}
	while (n > 0 && isspace(buf[n - 1]))
		n--;
	buf[n] = 0;
	if (verify) {
		if (!n) {
			note(0, "\"%s\": no match", name);
			return 0;
		}
		if (strchr(buf, ' ') && access(buf, F_OK)) {
			note(0, "\"%s\": ambiguous (%s)", name, buf);
			return 0;
		}
	}
	return savestr(buf);
}

/*
 * Determine the current folder directory name.
 */
int
getfolder(char* name, size_t size)
{
	char*	folder;

	if (!(folder = state.var.folder))
		return -1;
	if (*folder == '/')
		strncopy(name, folder, size);
	else
		sfsprintf(name, size, "%s/%s", state.var.home, folder);
	return 0;
}

#if MORE_DISCIPLINE

/*
 * For mail purposes it's safe to assume line-at-a-time input.
 */
static ssize_t
morein(Sfio_t* fp, void* buf, size_t n, Sfdisc_t* dp)
{
	state.more.match = 0;
	state.more.row = 2;
	state.more.col = 1;
	return sfrd(fp, buf, n, dp);
}

/*
 * Simple (but fast) more style pager.
 */
static ssize_t
moreout(Sfio_t* fp, const void* buf, size_t n, Sfdisc_t* dp)
{
	register char*		b;
	register char*		s;
	register char*		e;
	register ssize_t	w;
	register size_t		m;
	register int		r;

	if (!state.more.row)
		return n;
	if (!state.more.col)
		return sfwr(fp, buf, n, dp);
	w = 0;
	b = (char*)buf;
	s = b;
	e = s + n;
	if (state.more.match) {
 match:
		for (r = state.more.pattern[0];; s++) {
			if (s >= e)
				return n;
			if (*s == '\n')
				b = s + 1;
			else if (*s == r && (e - s) >= state.more.match && !strncmp(s, state.more.pattern, state.more.match))
				break;
		}
		s = b;
		w += b - (char*)buf;
		state.more.match = 0;
	}
	while (s < e) {
		switch (*s++) {
		case '\t':
			state.more.col = ((state.more.col + 8) & ~7) - 1;
			/*FALLTHROUGH*/
		default:
			if (++state.more.col <= state.screenwidth)
				continue;
			/*FALLTHROUGH*/
		case '\n':
			state.more.col = 1;
			if (++state.more.row < state.screenheight)
				continue;
			break;
		case '\b':
			if (state.more.col > 1)
				state.more.col--;
			continue;
		case '\r':
			state.more.col = 1;
			continue;
		}
		w += sfwr(fp, b, s - b, dp);
		b = s;
		r = ttyquery(0, 1, *state.var.more ? state.var.more : "[More]");
		if (r == '/' || r == 'n') {
			if (r == '/') {
				sfwr(fp, "/", 1, dp);
				if (fgets(state.more.tmp, sizeof(state.more.tmp), stdin)) {
					if ((m = strlen(state.more.tmp)) > 1 && state.more.tmp[m - 1] == '\n')
						state.more.tmp[m - 1] = 0;
					if (state.more.tmp[0])
						strncopy(state.more.pattern, state.more.tmp, sizeof(state.more.pattern));
				}
			}
			if (state.more.match = strlen(state.more.pattern)) {
				state.more.row = 1;
				state.more.col = 1;
				goto match;
			}
		}
		switch (r) {
		case '\n':
		case '\r':
			state.more.row--;
			state.more.col = 1;
			break;
		case ' ':
			state.more.row = 2;
			state.more.col = 1;
			break;
		default:
			state.more.row = 0;
			return n;
		}
	}
	if (s > b)
		w += sfwr(fp, b, s - b, dp);
	return w;
}

#endif

/*
 * Trap more variable assignment.
 */
void
set_more(struct var* vp, const char* value)
{
#if MORE_DISCIPLINE
	if (state.var.interactive && !state.var.coprocess) {
		if (value) {
			if (!state.more.discipline) {
				state.more.discipline = 1;
				if (!state.more.init) {
					state.more.init = 1;
					state.more.disc.readf = morein;
					state.more.disc.writef = moreout;
				}
				sfdisc(sfstdin, &state.more.disc);
				sfdisc(sfstdout, &state.more.disc);
			}
		}
		else if (state.more.discipline) {
			state.more.discipline = 0;
			sfdisc(sfstdin, NiL);
			sfdisc(sfstdout, NiL);
		}
		moretop();
	}
#else
	if (value && state.var.interactive && !state.var.coprocess && !state.more.init) {
		state.more.init = 1;
		note(0, "Recompile mailx with sfio to enable \"%s\"", vp->name);
	}
#endif
}

/*
 * clear signal and io state before resuming from interrupt
 */

void
resume(int sig)
{
	sigunblock(sig);
#if MORE_DISCIPLINE
	sfclrlock(sfstdin);
	sfclrlock(sfstdout);
	sfclrlock(sfstderr);
#endif
}
