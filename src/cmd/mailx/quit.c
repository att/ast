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
 * Mail -- a mail program.
 *
 * Termination processing.
 */

#include "mailx.h"

/*
 * Give names to all the temporary files that we will need.
 */

void
tempinit(void)
{
	register char*	cp;

	if (!(state.tmp.dir = tempnam(NiL, NiL)) || !*state.tmp.dir || !(cp = strrchr(state.tmp.dir, '/')) && !(cp = strrchr(state.tmp.dir, '\\')))
		state.tmp.dir = _PATH_TMP;
	else {
		*++cp = 0;
		if (!(cp = (char*)malloc(cp - state.tmp.dir + 1)))
			note(ERROR|FATAL, "Out of space");
		strcpy(cp, state.tmp.dir);
		state.tmp.dir = cp;
	}
	filetemp(state.tmp.edit, sizeof(state.tmp.edit), 'E', 0);
	filetemp(state.tmp.mail, sizeof(state.tmp.mail), 'M', 0);
	filetemp(state.tmp.mesg, sizeof(state.tmp.mesg), 'G', 0);
	filetemp(state.tmp.more, sizeof(state.tmp.more), 'X', 0);
	filetemp(state.tmp.quit, sizeof(state.tmp.quit), 'Q', 0);
}

/*
 * The "quit" command.
 */
int
cmdquit(void)
{
	/*
	 * If we are sourcing, then return 1 so execute() can handle it.
	 * Otherwise, return -1 to abort command loop.
	 */
	return state.sourcing ? 1 : -1;
}

/*
 * Terminate an editing session by attempting to write out the user's
 * file from the temporary.  Save any new stuff appended to the file.
 */
static void
edstop(void)
{
	register int		gotcha;
	register int		c;
	register struct msg*	mp;
	int			update;
	char*			s;
	char*			temp;
	char*			move;
	FILE*			obuf;
	FILE*			ibuf;
	FILE*			news;
	struct mhcontext	mh;
	struct stat		st;

	if (state.readonly)
		return;
	holdsigs();
	news = state.var.news && *state.var.news ? fileopen(state.var.news, "w") : (FILE*)0;
	update = MODIFY|MDELETE|MSTATUS;
	if (!state.var.keepsave)
		update |= MSAVE;
	for (mp = state.msg.list, gotcha = 0; mp < state.msg.list + state.msg.count; mp++) {
		if (!(mp->m_flag & MNONE)) {
			if (mp->m_flag & MNEW)
				msgflags(mp, MSTATUS, MNEW);
			if (mp->m_flag & update)
				gotcha++;
			if (news && (mp->m_flag & (MREAD|MDELETE)) && (s = grab(mp, GNEWS, NiL)))
				fprintf(news, "%s\n", s);
		}
	}
	if (news) {
		fileclose(news);
		goto done;
	}
	if (!gotcha)
		goto done;
	if (state.folder == FMH) {
		if (!state.incorporating)
			note(PROMPT, "\"%s\" ", state.path.mail);
		mhgetcontext(&mh, state.path.mail, 1);
		mh.dot = state.msg.dot - state.msg.list + 1;
		for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++) {
			if ((mp->m_flag & update) && !(mp->m_flag & MNONE)) {
				sfprintf(state.path.temp, "%s/%d", state.path.mail, mp - state.msg.list + 1);
				temp = struse(state.path.temp);
				if (mp->m_flag & (MDELETE|MSAVE))
					rm(temp);
				else {
					sfprintf(state.path.move, "%s/%d~", state.path.mail, mp - state.msg.list + 1);
					move = struse(state.path.move);
					if (obuf = fileopen(move, "Ew")) {
						if (copy(mp, obuf, NiL, NiL, 0) < 0) {
							note(SYSTEM, "%s", temp);
							rm(move);
							relsesigs();
							reset(0);
						}
						rm(temp);
						if (rename(move, temp))
							note(SYSTEM, "%s", temp);
						fileclose(obuf);
					}
				}
			}
		}
		mhputcontext(&mh, state.path.mail);
		if (!state.incorporating)
			note(0, "complete");
	}
	else {
		ibuf = 0;
		if (stat(state.path.mail, &st) >= 0 && st.st_size > state.mailsize) {
			temp = state.path.path;
			filetemp(temp, sizeof(state.path.path), 'B', 0);
			if (!(obuf = fileopen(temp, "Ew"))) {
				relsesigs();
				reset(0);
			}
			if (!(ibuf = fileopen(state.path.mail, "Er"))) {
				fileclose(obuf);
				rm(temp);
				relsesigs();
				reset(0);
			}
			fseek(ibuf, state.mailsize, SEEK_SET);
			filecopy(NiL, ibuf, NiL, obuf, NiL, (off_t)0, NiL, NiL, 0);
			fileclose(ibuf);
			fileclose(obuf);
			if (!(ibuf = fileopen(temp, "Er"))) {
				rm(temp);
				relsesigs();
				reset(0);
			}
			rm(temp);
		}
		if (!state.incorporating)
			note(PROMPT, "\"%s\" ", state.path.mail);
		if (!(obuf = fileopen(state.path.mail, "Er+"))) {
			relsesigs();
			reset(0);
		}
		filetrunc(obuf);
		update &= (MDELETE|MSAVE);
		update |= MNONE;
		c = 0;
		for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++)
			if (!(mp->m_flag & update)) {
				c = 1;
				if (copy(mp, obuf, NiL, NiL, 0) < 0) {
					note(SYSTEM, "%s", state.path.mail);
					relsesigs();
					reset(0);
				}
			}
		gotcha = !c && !ibuf;
		if (ibuf) {
			filecopy(NiL, ibuf, NiL, obuf, NiL, (off_t)0, NiL, NiL, 0);
			fileclose(ibuf);
		}
		if (fileclose(obuf)) {
			note(SYSTEM, "%s", state.path.mail);
			relsesigs();
			reset(0);
		}
		if (!state.incorporating) {
			if (gotcha) {
				rm(state.path.mail);
				note(0, "removed");
			}
			else
				note(0, "complete");
		}
	}
 done:
	relsesigs();
}

/*
 * Preserve all the appropriate messages back in the system
 * mailbox, and print a nice message indicated how many were
 * saved.  On any error, just return -1.  Else return 0.
 * Incorporate the any new mail that we found.
 */
static int
writeback(register FILE* res)
{
	register struct msg*	mp;
	register int		p;
	FILE*			obuf;

	p = 0;
	if (!(obuf = fileopen(state.path.mail, "Er+")))
		return -1;
#if !APPEND_MAILBOX
	if (res)
		filecopy(NiL, res, NiL, obuf, NiL, (off_t)0, NiL, NiL, 0);
#endif
	for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++)
		if ((mp->m_flag&MPRESERVE) || !(mp->m_flag&MTOUCH)) {
			p++;
			if (copy(mp, obuf, NiL, NiL, 0) < 0) {
				note(SYSTEM, "%s", state.path.mail);
				fileclose(obuf);
				return -1;
			}
		}
#if APPEND_MAILBOX
	if (res && filecopy(NiL, res, state.path.mail, obuf, NiL, (off_t)0, NiL, NiL, 0)) {
		fileclose(obuf);
		return -1;
	}
#endif
	if (filetrunc(obuf)) {
		note(SYSTEM, "%s", state.path.mail);
		fileclose(obuf);
		return -1;
	}
	fileclose(obuf);
	if (res)
		fileclose(res);
	alter(state.path.mail);
	note(0, "Held %d message%s in %s", p, p == 1 ? "" : "s", state.path.mail);
	return 0;
}

/*
 * Save all of the undetermined messages at the top of "mbox"
 * Save all untouched messages back in the system mailbox.
 * Remove the system mailbox, if none saved there.
 */
void
quit(void)
{
	register struct msg*	mp;
	register int		c;
	off_t			size;
	char*			mbox;
	char*			s;
	char*			xbox;
	int			anystat;
	int			holdbit;
	int			mcount;
	int			modify;
	int			nohold;
	int			p;
	int			x;
	int			set;
	int			clr;
	FILE*			abuf;
	FILE*			fbuf;
	FILE*			ibuf;
	FILE*			news;
	FILE*			obuf;
	FILE*			rbuf;
	FILE*			xbuf;

	/*
	 * If we are read only, we can't do anything,
	 * so just return quickly.
	 */
	if (state.readonly)
		return;
	/*
	 * If editing (not reading system mail box), then do the work
	 * in edstop()
	 */
	if (state.edit) {
		edstop();
		return;
	}
	if (state.folder != FIMAP) {

		/*
		 * See if there any messages to save in mbox.  If no, we
		 * can save copying mbox to /tmp and back.
		 *
		 * Check also to see if any files need to be preserved.
		 * Delete all untouched messages to keep them out of mbox.
		 * If all the messages are to be preserved, just exit with
		 * a message.
		 *
		 * NOTE: filelock() requires open for r/w
		 */

		if (!(fbuf = fileopen(state.path.mail, "r+")))
			goto newmail;
		size = state.openstat.st_size;
		filelock(state.path.mail, fbuf, 1);
		rbuf = 0;
		if (size > state.mailsize) {
			note(0, "New mail has arrived");
			if (!(rbuf = fileopen(state.tmp.more, "w")) || !fbuf)
				goto newmail;
#if APPEND_MAILBOX
			fseek(fbuf, state.mailsize, SEEK_SET);
			filecopy(NiL, fbuf, NiL, rbuf, NiL, (off_t)0, NiL, NiL, 0);
#else
			if (filecopy(NiL, fbuf, NiL, rbuf, NiL, (off_t)(size - state.mailsize), NiL, NiL, 0))
				goto newmail;
#endif
			fileclose(rbuf);
			if (!(rbuf = fileopen(state.tmp.more, "r")))
				goto newmail;
			rm(state.tmp.more);
		}
	}
	else
		fbuf = 0;

	/*
	 * Adjust the message flags in each message.
	 */

	anystat = 0;
	holdbit = state.var.hold ? MPRESERVE : MBOX;
	nohold = MBOX|MSAVE|MDELETE|MPRESERVE;
	if (state.var.keepsave)
		nohold &= ~MSAVE;
	for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++) {
		set = clr = 0;
		if (mp->m_flag & (MBOX|MDELETE|MPRESERVE|MSAVE))
			clr |= MSPAM;
		if (mp->m_flag & MNEW) {
			set |= MSTATUS;
			clr |= MNEW;
		}
		if ((mp->m_flag | set) & MSTATUS)
			anystat++;
		if (!(mp->m_flag & MTOUCH))
			set |= MPRESERVE;
		if (!(mp->m_flag & nohold))
			set |= holdbit;
		if (set || clr)
			msgflags(mp, set, clr);
	}
	modify = 0;
	news = state.var.news && *state.var.news ? fileopen(state.var.news, "w") : (FILE*)0;
	for (c = p = x = 0, mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++) {
		if (mp->m_flag & MSPAM) {
			msgflags(mp, MDELETE|MTOUCH, MBOX|MPRESERVE|MODIFY);
			x++;
		}
		if (mp->m_flag & MBOX)
			c++;
		if (mp->m_flag & MPRESERVE)
			p++;
		if (mp->m_flag & MODIFY)
			modify++;
		if (news && (mp->m_flag & (MREAD|MDELETE)) &&
		    (s = grab(mp, GNEWS, NiL)))
			fprintf(news, "%s\n", s);
	}
	if (news)
		fileclose(news);
	if (p == state.msg.count && !modify && !anystat) {
		note(0, "Held %d message%s in %s", p, p == 1 ? "" : "s", state.path.mail);
		if (fbuf)
			fileclose(fbuf);
		return;
	}
	xbuf = 0;
	if (x && (!state.var.spamlog || !*state.var.spamlog || !(xbuf = fileopen(xbox = expand(state.var.spamlog, 1), "a"))))
		x = 0;
	if (state.folder == FIMAP) {
		for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++) {
			if (mp->m_flag & MSAVE)
				msgflags(mp, MDELETE, 0);
			if (xbuf && (mp->m_flag & (MSPAM|MDELETE)) == (MSPAM|MDELETE)) {
				if (copy(mp, xbuf, &state.saveignore, NiL, 0) < 0) {
					msgflags(mp, MPRESERVE, MSPAM|MDELETE|MTOUCH);
					x--;
				}
			}
		}
		if (xbuf) {
			fileclose(xbuf);
			if (x)
				note(0, "Saved %d message%s in %s", x, x == 1 ? "" : "s", xbox);
		}
	}
	if (state.msg.imap.state) {
		imap_quit();
		if (state.folder == FIMAP)
			return;
	}
	if (!c && !x) {
		if (p) {
			writeback(rbuf);
			fileclose(fbuf);
			fileclose(xbuf);
			return;
		}
		goto cream;
	}

	/*
	 * Create another temporary file and copy user's mbox file
	 * darin.  If there is no mbox, copy nothing.
	 * If he has specified "append" don't copy his mailbox,
	 * just copy saveable entries at the end.
	 */

	mbox = expand("&", 1);
	mcount = c;
	if (state.var.append) {
		if (!(obuf = fileopen(mbox, "Ea"))) {
			fileclose(fbuf);
			fileclose(xbuf);
			return;
		}
		chmod(mbox, MAILMODE);
	}
	else {
		if (!(obuf = fileopen(state.tmp.quit, "Ew"))) {
			fileclose(fbuf);
			fileclose(xbuf);
			return;
		}
		if (!(ibuf = fileopen(state.tmp.quit, "Er"))) {
			rm(state.tmp.quit);
			fileclose(obuf);
			fileclose(fbuf);
			fileclose(xbuf);
			return;
		}
		rm(state.tmp.quit);
		if (abuf = fileopen(mbox, "r")) {
			if (filecopy(mbox, abuf, state.tmp.quit, obuf, NiL, (off_t)0, NiL, NiL, 0)) {
				fileclose(abuf);
				fileclose(ibuf);
				fileclose(obuf);
				fileclose(fbuf);
				fileclose(xbuf);
				return;
			}
			fileclose(abuf);
		}
		if (fileclose(obuf)) {
			note(SYSTEM, "%s", state.tmp.quit);
			fileclose(ibuf);
			fileclose(obuf);
			fileclose(fbuf);
			fileclose(xbuf);
			return;
		}
		close(open(mbox, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY|O_cloexec, MAILMODE));
		if (!(obuf = fileopen(mbox, "Er+"))) {
			fileclose(ibuf);
			fileclose(fbuf);
			fileclose(xbuf);
			return;
		}
	}
	for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++) {
		if (mp->m_flag & MBOX) {
			if (copy(mp, obuf, &state.saveignore, NiL, 0) < 0) {
				note(SYSTEM, "%s", mbox);
				fileclose(ibuf);
				fileclose(obuf);
				fileclose(fbuf);
				fileclose(xbuf);
				return;
			}
		}
		else if (xbuf && (mp->m_flag & (MSPAM|MDELETE)) == (MSPAM|MDELETE)) {
			if (copy(mp, xbuf, &state.saveignore, NiL, 0) < 0) {
				msgflags(mp, MPRESERVE, MSPAM|MDELETE|MTOUCH);
				x--;
			}
		}
	}
	if (xbuf) {
		fileclose(xbuf);
		if (x)
			note(0, "Saved %d message%s in %s", x, x == 1 ? "" : "s", xbox);
	}

	/*
	 * Copy the user's old mbox contents back
	 * to the end of the stuff we just saved.
	 * If we are appending, this is unnecessary.
	 */

	if (!state.var.append) {
		rewind(ibuf);
		filecopy(NiL, ibuf, NiL, obuf, NiL, (off_t)0, NiL, NiL, 0);
		fileclose(ibuf);
	}
	if (filetrunc(obuf)) {
		note(SYSTEM, "%s", mbox);
		fileclose(obuf);
		fileclose(fbuf);
		return;
	}
	fileclose(obuf);
	if (mcount)
		note(0, "Saved %d message%s in %s", mcount, mcount == 1 ? "" : "s", mbox);

	/*
	 * Now we are ready to copy back preserved files to
	 * the system mailbox, if any were requested.
	 */

	if (p) {
		writeback(rbuf);
		fileclose(fbuf);
		return;
	}

	/*
	 * Finally, remove his /usr/mail file.
	 * If new mail has arrived, copy it back.
	 */

 cream:
	if (rbuf) {
		if (!(abuf = fileopen(state.path.mail, "r+")))
			goto newmail;
		filecopy(NiL, rbuf, NiL, abuf, NiL, (off_t)0, NiL, NiL, 0);
		fileclose(rbuf);
		filetrunc(abuf);
		fileclose(abuf);
		alter(state.path.mail);
		fileclose(fbuf);
		return;
	}
	demail();
	if (fbuf)
		fileclose(fbuf);
	return;

 newmail:
	note(0, "Thou hast new mail");
	if (fbuf)
		fileclose(fbuf);
}
