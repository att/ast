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
 * Perform message editing functions.
 */

#include "mailx.h"

/*
 * Edit a message list.
 */
static int
edit1(struct msg* msgvec, int type)
{
	register struct msg*	ip;
	register struct msg*	mp;
	FILE*			fp;
	off_t			size;
	sig_t			sigint;

	/*
	 * Deal with each message to be edited . . .
	 */
	for (ip = msgvec; ip->m_index; ip++) {
		if (ip > msgvec) {
			char	buf[100];
			char*	p;

			note(PROMPT, "Edit message %d [ynq]? ", ip->m_index);
			if (!fgets(buf, sizeof buf, stdin))
				break;
			for (p = buf; isspace(*p); p++);
			if (*p == 'q' || *p == 'Q')
				break;
			if (*p == 'n' || *p == 'N')
				continue;
		}
		state.msg.dot = mp = state.msg.list + ip->m_index - 1;
		touchmsg(mp);
		sigint = signal(SIGINT, SIG_IGN);
		if (fp = run_editor(setinput(mp), mp->m_size, NiL, type, state.readonly)) {
			if (!state.msg.op)
				settmp(NiL, 0);
			fseek(state.msg.op, (off_t)0, SEEK_END);
			size = ftell(state.msg.op);
			mp->m_block = blocknumber(size);
			mp->m_offset = blockoffset(size);
			msgflags(mp, MODIFY, 0);
			rewind(fp);
			filecopy(NiL, fp, state.tmp.dir, state.msg.op, NiL, (off_t)0, &mp->m_lines, &mp->m_size, 0);
			fileclose(fp);
		}
		signal(SIGINT, sigint);
	}
	return 0;
}

/*
 * Edit a message list.
 */
int
editor(struct msg* msgvec)
{
	return edit1(msgvec, 'e');
}

/*
 * Invoke the visual editor on a message list.
 */
int
visual(struct msg* msgvec)
{
	return edit1(msgvec, 'v');
}

/*
 * Run an editor on the file at "fp" of "size" bytes,
 * and return a new file pointer.
 * Signals must be handled by the caller.
 * "Type" is 'e' for state.var.editor, 'v' for state.var.visual.
 */
FILE*
run_editor(register FILE* fp, off_t size, struct header* hp, int type, int readonly)
{
	FILE*		ep;
	time_t		modtime;
	int		lc;
	int		err;
	char*		edit;
	unsigned long	editheaders = 0;
	struct parse	pp;

	/*
	 * Create and copy to the temporary file.
	 */
	if (!(ep = fileopen(state.tmp.edit, "EMw")))
		goto ret1;
	if (size) {
		if (size < 0)
			size = 0;
		if (hp && state.var.editheaders) {
			editheaders = GEDIT|GRULE;
			headout(ep, hp, editheaders|GNL);
		}
		if (filecopy(NiL, fp, state.tmp.edit, ep, NiL, size, NiL, NiL, 0))
			goto ret2;
	}
	modtime = state.openstat.st_mtime;
	fileclose(ep);
	ep = 0;
	/*
	 * Edit the file.
	 */
	edit = type == 'e' ? state.var.editor : state.var.visual;
	err = run_command(edit, 0, -1, -1, state.tmp.edit, NiL, NiL) < 0;
	/*
	 * If in readonly mode or file unchanged, clean up and return.
	 */
	if (readonly)
		goto ret2;
	if (stat(state.tmp.edit, &state.openstat) < 0)
		goto ret1;
	if (modtime == state.openstat.st_mtime)
		goto ret2;
	if (err)
		note(0, "%s did not exit normally but did do some changes -- you may want to re-edit", edit);
	/*
	 * Now, switch to the temporary file.
	 */
	if (!(ep = fileopen(state.tmp.edit, "Ea+")))
		goto ret2;
	if (editheaders && headset(&pp, NiL, ep, hp, NiL, editheaders|GTO|GMETOO)) {
		while (headget(&pp));
		remove(state.tmp.edit);
		if (!(ep = fileopen(state.tmp.edit, "EMa+")))
			goto ret1;
		filecopy(NiL, pp.fp, state.tmp.edit, ep, NiL, (off_t)0, NiL, NiL, 0);
		fileclose(pp.fp);
	}
	/*
	 * Ensure that the tempEdit file ends with two newlines.
	 *
	 * XXX
	 * Probably ought to have a `From' line, as well.
	 *
	 * XXX
	 * If the file is only a single byte long, the seek is going to
	 * fail, but I'm not sure we care.  In the case of any error, we
	 * pass back the file descriptor -- if we fail because the disk
	 * is too full, reads should continue to work and I see no reason
	 * to discard the user's work.
	 */
	if (fseek(ep, (off_t)-2, SEEK_END) < 0)
		return ep;
	lc = getc(ep) == '\n' ? 1 : 0;
	if (getc(ep) == '\n')
		++lc;
	else
		lc = 0;

	switch (lc) {
	case 0:
		if (putc('\n', ep) == EOF)
			break;
		/* FALLTHROUGH */
	case 1:
		putc('\n', ep);
		/* FALLTHROUGH */
	case 2:
		break;
	default:
		abort();
		break;
	}
	/*
	 * XXX: fflush() is necessary, so future stat(2) succeeds.
	 */
	fflush(ep);
	remove(state.tmp.edit);
	return ep;

 ret1:	note(SYSTEM, "%s", state.tmp.edit);
 ret2:	remove(state.tmp.edit);
	if (ep)
		fileclose(ep);
	return 0;
}
