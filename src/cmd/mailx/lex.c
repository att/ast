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
 * Lexical processing of commands.
 */

#include "mailx.h"

/*
 * Inititialize the folder tmp file pointers.
 */
void
settmp(const char* name, int dir)
{
	int	fd;

	if (name) {
		if (state.msg.op || state.folder == FMH)
			quit();
		state.readonly = 0;
		if (dir) {
			if (eaccess(name, W_OK))
				state.readonly = 1;
		}
		else if ((fd = open(name, O_WRONLY|O_BINARY|O_cloexec)) < 0)
			state.readonly = 1;
		else
			close(fd);
		if (state.msg.ap) {
			fileclose(state.msg.ap);
			state.msg.ap = 0;
		}
		if (state.msg.ip) {
			fileclose(state.msg.ip);
			state.msg.ip = 0;
		}
		if (state.msg.op) {
			fileclose(state.msg.op);
			state.msg.op = 0;
		}
		strncopy(state.path.prev, state.path.mail, sizeof(state.path.prev));
		if (name != state.path.mail)
			strncopy(state.path.mail, name, sizeof(state.path.mail));
	}
	if (!dir) {
		if (!(state.msg.op = fileopen(state.tmp.mesg, "EIw")))
			exit(1);
		if (!(state.msg.ip = fileopen(state.tmp.mesg, "EIr")))
			exit(1);
		rm(state.tmp.mesg);
	}
}

/*
 * Set up editing on the given folder.
 * If the first character of name is not %, we are considered to be
 * editing the folder, otherwise we are reading our mail which has
 * signficance for mbox and so forth.
 */
int
setfolder(char* name)
{
	FILE*	ibuf;
	int	isedit = *name != '%';
	char*	who = name[1] ? name + 1 : state.var.user;

	if (!(name = expand(name, 1)))
		return -1;
	state.msg.inbox = 0;
	if (imap_name(name)) {
		imap_setptr(name, isedit);
		return 0;
	}
	if (state.folder == FIMAP)
		imap_setptr((char*)0, 0);
	if (!(ibuf = fileopen(name, "Rr"))) {
		if (state.var.justcheck)
			exit(1);
		if (!isedit && errno == ENOENT)
			goto nomail;
		if (errno == EISDIR) {
			mh_setptr(name, isedit);
			return 0;
		}
		note(SYSTEM, "%s", name);
		return -1;
	}
	if (!state.openstat.st_size) {
		if (state.var.justcheck)
			exit(1);
		if (isedit)
			note(0, "%s: empty file", name);
		else {
			fileclose(ibuf);
			goto nomail;
		}
	}
	if (S_ISDIR(state.openstat.st_mode)) {
		mh_setptr(name, isedit);
		return 0;
	}
	if (state.var.justcheck)
		exit(0);

	/*
	 * Looks like all will be well.  We must now relinquish our
	 * hold on the current set of stuff.  Must hold signals
	 * while we are reading the new file, else we will ruin
	 * the message[] data structure.
	 */

	holdsigs();
	settmp(name, 0);
	state.edit = isedit;
	state.mailsize = filesize(ibuf);
	setptr(ibuf, 0);
	/*
	 * New mail has arrived while we were reading up the mail file,
	 * so reset mailsize to be where we really are in the file...
	 */
	state.mailsize = ftell(ibuf);
	fileclose(ibuf);
	relsesigs();
	state.sawcom = 0;
	if (!state.edit && !state.msg.count) {
 nomail:
		if (state.var.justcheck)
			exit(1);
		if (!state.var.justheaders) {
			if (strchr(name, '/') || strchr(name, '\\'))
				note(ERROR, "No mail for %s", who);
			else
				note(ERROR, "No mail", who);
		}
		return -1;
	}
	return 0;
}

/*
 * Incorporate any new mail that has arrived since we first
 * started reading mail.
 */
int
incfile(void)
{
	int	newsize;
	int	msgcount = state.msg.count;
	FILE*	fp;

	if (!(fp = fileopen(state.path.mail, "r")))
		return -1;
	holdsigs();
	if (!(newsize = filesize(fp)))
		return -1;		/* mail box is now empty??? */
	if (newsize < state.mailsize)
		return -1;		/* mail box has shrunk??? */
	if (newsize == state.mailsize)
		return 0;		/* no new mail */
	setptr(fp, state.mailsize);
	state.mailsize = ftell(fp);
	fileclose(fp);
	relsesigs();
	return state.msg.count - msgcount;
}

/*
 * The following gets called on receipt of an interrupt.  This is
 * to abort printout of a command, mainly.
 * Dispatching here when command() is inactive crashes rcv.
 * Close all open files except 0, 1, 2, and the temporary.
 * Also, unstack all source files.
 */
static void
intr(int sig)
{
	state.noreset = 0;
	if (state.startup)
		state.startup = 0;
	else
		state.sawcom++;
	while (state.sourcing)
		unstack();
	fileclear();
	note(0, "Interrupt");
	reset(sig);
}

/*
 * When we wake up after ^Z, reprint the prompt.
 */
static void
stop(int sig)
{
	sig_t old_action = signal(sig, SIG_DFL);

	kill(getpid(), sig);
	signal(sig, old_action);
	if (state.stopreset) {
		state.stopreset = 0;
		reset(sig);
	}
}

/*
 * Branch here on hangup signal and simulate "exit".
 */
/*ARGSUSED*/
static void
hangup(int sig)
{
	/* nothing to do? */
	exit(1);
}

/*
 * Interpret user commands one by one.  If standard input is not a tty,
 * print no prompt.
 */
void
commands(void)
{
	register int	n;
	int		eofloop = 0;
	char		linebuf[LINESIZE];

	if (!state.sourcing) {
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			signal(SIGINT, intr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			signal(SIGHUP, hangup);
#if SIGTSTP
		signal(SIGTSTP, stop);
#endif
#if SIGTTOU
		signal(SIGTTOU, stop);
#endif
#if SIGTTIN
		signal(SIGTTIN, stop);
#endif
	}
	setexit();
	for (;;) {
		/*
		 * Print the prompt, if needed.  Clear out
		 * string space, and flush the output.
		 */
		fflush(stdout);
		moretop();
		if (!state.sourcing && state.var.interactive) {
			if (state.var.autoinc && incfile() > 0)
				note(0, "New mail has arrived");
			state.stopreset = 1;
			if (state.var.coprocess)
				note(0, "");
			else if (state.var.prompt)
				note(PROMPT, "%s", state.var.prompt);
		}
		sreset();
		/*
		 * Read a line of commands from the current input
		 * and handle end of file specially.
		 */
		n = 0;
		for (;;) {
			if (readline(state.input, &linebuf[n], LINESIZE - n) < 0) {
				if (!n)
					n = -1;
				break;
			}
			if (!(n = strlen(linebuf)))
				break;
			n--;
			if (linebuf[n] != '\\')
				break;
			linebuf[n++] = ' ';
		}
		state.stopreset = 0;
		if (n < 0) {
				/* eof */
			if (state.loading)
				break;
			if (state.sourcing) {
				unstack();
				continue;
			}
			if (state.var.interactive &&
			    state.var.ignoreeof &&
			    ++eofloop < 25) {
				note(0, "Use \"quit\" to quit");
				continue;
			}
			break;
		}
		eofloop = 0;
		if (execute(linebuf, 0))
			break;
	}
}

/*
 * Execute a single command.
 * Command functions return 0 for success, 1 for error, and -1
 * for abort.  A 1 or -1 aborts a load or source.  A -1 aborts
 * the interactive command loop.
 * Contxt is non-zero if called while composing mail.
 */
int
execute(char* linebuf, int contxt)
{
	struct cmd*	com;
	register char*	s;
	register int	c;
	register char*	a;
	char*		next;
	struct argvec	vec;
	int		e = 1;

	/*
	 * Strip the white space away from the beginning of the command.
	 */

	for (s = linebuf; isspace(*s); s++);

	/*
	 * Look up the command and execute.
	 */

	switch (*s) {
	case '!':
		a = "shell";
		s++;
		break;
	case '#':
		return 0;
	case '=':
		a = "dot";
		s++;
		break;
	case '?':
		a = "help";
		s++;
		break;
	case '|':
		a = "pipe";
		s++;
		break;
	default:
		if (isalpha(*s)) {
			a = s;
			s = 0;
		}
		else {
			if (state.sourcing)
				return 0;
			a = "next";
		}
		break;
	}
	if (!(com = (struct cmd*)strpsearch(state.cmdtab, state.cmdnum, sizeof(struct cmd), a, &next))) {
		note(0, "\"%s\": unknown command", a);
		goto out;
	}
	state.cmd = com;
	if (state.clobber = *next == '!')
		next++;
	if (!s)
		s = next;
	while (isspace(*s))
		s++;

	/*
	 * See if we should execute the command -- if a conditional
	 * we always execute it, otherwise, check the state of cond.
	 */

	if (!(com->c_argtype & C) && state.cond && state.cond != state.mode)
		return 0;

	/*
	 * Process the arguments to the command, depending
	 * on the type expected.  Default to an error.
	 * If we are sourcing an interactive command, it's
	 * an error.
	 */

	if (state.mode == SEND && !(com->c_argtype & M)) {
		note(0, "Cannot execute \"%s\" while sending",
		    com->c_name);
		goto out;
	}
	if (state.sourcing && (com->c_argtype & I)) {
		note(0, "Cannot execute \"%s\" while sourcing",
		    com->c_name);
		goto out;
	}
	if (state.readonly && (com->c_argtype & W)) {
		note(0, "Cannot execute \"%s\" -- message file is read only",
		   com->c_name);
		goto out;
	}
	if (contxt && (com->c_argtype & R)) {
		note(0, "Cannot recursively invoke \"%s\"", com->c_name);
		goto out;
	}
	switch (com->c_argtype & LISTMASK) {
	case MSGLIST:
		/*
		 * A message list defaulting to nearest forward
		 * legal message.
		 */
		if (!state.msg.list) {
			note(0, "Invalid use of \"message list\"");
			break;
		}
		if ((c = getmsglist(s, com->c_msgflag)) < 0)
			break;
		if (!c) {
			state.msg.list->m_index = first(com->c_msgflag, com->c_msgmask);
			(state.msg.list + 1)->m_index = 0;
		}
		if (!state.msg.list->m_index) {
			note(0, "No applicable messages");
			break;
		}
		e = (*com->c_func)(state.msg.list);
		break;

	case NDMLIST:
		/*
		 * A message list with no defaults, but no error
		 * if none exist.
		 */
		if (!state.msg.list) {
			note(0, "Invalid use of \"message list\"");
			break;
		}
		if (getmsglist(s, com->c_msgflag) < 0)
			break;
		e = (*com->c_func)(state.msg.list);
		break;

	case STRLIST:
		/*
		 * Just the straight string.
		 */
		e = (*com->c_func)(s);
		break;

	case RAWLIST:
		/*
		 * A vector of strings, in shell style.
		 */
		initargs(&vec);
		getargs(&vec, s);
		if ((c = endargs(&vec)) < 0)
			break;
		if (c < com->c_minargs) {
			note(0, "%s requires %s %d arg%s",
				com->c_name,
				com->c_minargs == com->c_maxargs ? "exactly" : "at least",
				com->c_minargs,
				com->c_minargs == 1 ? "" : "s");
			break;
		}
		if (c > com->c_maxargs) {
			note(0, "%s takes no more than %d arg%s",
				com->c_name,
				com->c_maxargs,
				com->c_maxargs == 1 ? "" : "s");
			break;
		}
		e = (*com->c_func)(vec.argv);
		break;

	case NOLIST:
		/*
		 * Just the constant zero, for exiting, e.g.
		 */
		e = (*com->c_func)(0);
		break;

	default:
		note(PANIC, "0x%08x: unknown argtype", com->c_argtype & LISTMASK);
	}

 out:
	/*
	 * Exit the current source file on error.
	 */
	if (e) {
		if (e < 0 || state.loading)
			return 1;
		if (state.sourcing)
			unstack();
		return 0;
	}
	if (state.var.autoprint && (com->c_argtype & P))
		if (!(state.msg.dot->m_flag & (MDELETE|MNONE|MSPAM))) {
			state.msg.list->m_index = state.msg.dot - state.msg.list + 1;
			(state.msg.list + 1)->m_index = 0;
			type(state.msg.list);
		}
	if (!state.sourcing && !(com->c_argtype & Z))
		state.sawcom = 1;
	return 0;
}

/*
 * Announce information about the current folder.
 * msgcount is state.msg.count before the file was read.
 * Return a likely place to set dot.
 */
struct msg*
folderinfo(int msgcount)
{
	register struct msg*	mp;
	register int		d;
	register int		m;
	register int		n;
	register int		s;
	register int		u;
	int			i;
	struct msg*		dot;
	char*			name;
	char			buf[LINESIZE];

	if (state.var.justheaders && state.var.header)
		return state.msg.list;
	if (dot = state.msg.context)
		state.msg.context = 0;
	else if (msgcount > 0 && state.msg.dot < state.msg.list + msgcount - 1)
		dot = state.msg.dot;
	else {
		for (mp = state.msg.list + msgcount; mp < state.msg.list + state.msg.count; mp++)
			if (mp->m_flag & MNEW)
			 	break;
		if (mp >= state.msg.list + state.msg.count)
			for (mp = state.msg.list + msgcount; mp < state.msg.list + state.msg.count; mp++)
				if (!(mp->m_flag & MREAD))
					break;
		if (mp < state.msg.list + state.msg.count)
			dot = mp;
		else if (msgcount <= 0 && state.var.recent)
			dot = state.msg.list + state.msg.count - 1;
		else {
			dot = state.msg.list;
			if (!state.msg.count)
				dot += msgcount - 1;
		}
	}
	d = m = n = s = u = 0;
	for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++) {
		if (!(mp->m_flag & MNONE)) {
			m++;
			if (mp->m_flag & MDELETE)
				d++;
			else if (mp->m_flag & MSAVE)
				s++;
			else if (!(mp->m_flag & (MREAD|MNEW)))
				u++;
			else if ((mp->m_flag & (MREAD|MNEW)) == MNEW)
				n++;
		}
	}
	name = state.path.mail;
	if (getfolder(buf, sizeof(buf)) >= 0) {
		i = strlen(buf);
		buf[i++] = '/';
		if (!strncmp(state.path.mail, buf, i))
			sfsprintf(name = buf, sizeof(buf), "+%s", state.path.mail + i);
	}
	printf("\"%s\": %d message%s", name, m, m == 1 ? "" : "s");
	if (msgcount > 0 && state.msg.count > msgcount)
		printf(" %d incorporated", state.msg.count - msgcount);
	if (n > 0)
		printf(" %d new", n);
	if (u > 0)
		printf(" %d unread", u);
	if (d > 0)
		printf(" %d deleted", d);
	if (s > 0)
		printf(" %d saved", s);
	switch (state.folder) {
	case FIMAP:
		printf(" [imap]");
		break;
	case FMH:
		printf(" [mh]");
		break;
	}
	if (state.readonly)
		printf(" [Read only]");
	printf("\n");
	return dot;
}

/*
 * Announce the presence of the current Mail version,
 * give the message count, and print a header listing.
 */
void
announce()
{
	state.msg.dot = folderinfo(0);
	if (state.msg.list) {
		state.msg.list->m_index = state.msg.dot - state.msg.list + 1;
		(state.msg.list + 1)->m_index = 0;
		if (state.msg.count > 0 && state.var.header) {
			state.startup = 1;
			headers(state.msg.list);
			state.startup = 0;
		}
	}
}

/*
 * Print the current version number.
 */

/*ARGSUSED*/
int
version(void* a)
{
	note(0, "Version %s", state.version);
	return 0;
}

/*
 * Print the license and disclaimer.
 */

/*ARGSUSED*/
int
license(void* a)
{
	version(a);
	note(0, "\n%s", state.license);
	return 0;
}

/*
 * Load a file of user definitions.
 */
void
load(char* name)
{
	register FILE*	fp;
	register FILE*	input;

	if (!(fp = fileopen(name, "r")))
		return;
	input = state.input;
	state.input = fp;
	state.loading = 1;
	state.sourcing = 1;
	commands();
	state.loading = 0;
	state.sourcing = 0;
	state.input = input;
	fileclose(fp);
}
