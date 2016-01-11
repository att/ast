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
 * Collect input from standard input, handling ~ escapes.
 */

#include "mailx.h"

#define CODE_64		(1<<0)
#define CODE_QP		(1<<1)
#define CODE_TEXT	(1<<2)

#define PART_INIT	0
#define PART_MAIN	1
#define PART_DATA	2

/*
 * On interrupt, come here to save the partial message in ~/dead.letter.
 * Then jump out of the collection loop.
 */
/*ARGSUSED*/
static void
collint(int sig)
{
	/*
	 * the control flow is subtle, because we can be called from ~q.
	 */
	if (!state.collect.hadintr) {
		if (state.var.ignore) {
			puts("@");
			fflush(stdout);
			clearerr(stdin);
			return;
		}
		state.collect.hadintr = 1;
		longjmp(state.collect.work, sig);
	}
	rewind(state.collect.fp);
	if (state.var.save)
		savedeadletter(state.collect.fp);
	longjmp(state.collect.abort, sig);
}

/*ARGSUSED*/
static void
collhup(int sig)
{
	rewind(state.collect.fp);
	savedeadletter(state.collect.fp);
	/*
	 * Let's pretend nobody else wants to clean up,
	 * a true statement at this time.
	 */
	exit(1);
}

/*
 * Print (continue) when continued after ^Z.
 */
/*ARGSUSED*/
static void
collstop(int sig)
{
	sig_t	old_action = signal(sig, SIG_DFL);

	kill(getpid(), sig);
	signal(sig, old_action);
	if (state.collect.working) {
		state.collect.working = 0;
		state.collect.hadintr = 0;
		longjmp(state.collect.work, sig);
	}
}

/*
 * Write a file, ex-like if f set.
 */
static int
exwrite(char* name, FILE* fp, int f)
{
	register FILE*	of;
	off_t		cc;
	off_t		lc;

	if (f)
		note(PROMPT, "\"%s\" ", name);
	if (isreg(name)) {
		if (!f)
			note(PROMPT, "\"%s\" ", name);
		note(0, "[File exists]");
		return -1;
	}
	if (!(of = fileopen(name, "Ew")))
		return -1;
	if (filecopy(NiL, fp, name, of, NiL, (off_t)0, &lc, &cc, 0)) {
		fileclose(of);
		return -1;
	}
	fileclose(of);
	note(0, "%ld/%ld", (long)lc, (long)cc);
	return 0;
}

/*
 * Edit the message on state.collect.fp.
 * On return, make the edit file the new temp file.
 */
static void
editmessage(struct header* hp, int c)
{
	sig_t	sigint;
	FILE*	fp;

	sigint = signal(SIGINT, SIG_IGN);
	if (fp = run_editor(state.collect.fp, (off_t)-1, hp, c, 0)) {
		fseek(fp, (off_t)0, SEEK_END);
		fileclose(state.collect.fp);
		state.collect.fp = fp;
	}
	signal(SIGINT, sigint);
}

/*
 * Pipe the message through the command.
 * Old message is on stdin of command;
 * New message collected from stdout.
 * Sh -c must return 0 to accept the new message.
 */
static void
pipemessage(FILE* fp, char* cmd)
{
	FILE *nf;
	sig_t sigint = signal(SIGINT, SIG_IGN);

	if (!(nf = fileopen(state.tmp.edit, "Ew+")))
		goto out;
	remove(state.tmp.edit);
	/*
	 * stdin = current message.
	 * stdout = new message.
	 */
	if (run_command(state.var.shell, 0, fileno(fp), fileno(nf), "-c", cmd, NiL) < 0) {
		fileclose(nf);
		goto out;
	}
	if (!filesize(nf)) {
		note(0, "No bytes from \"%s\" !?", cmd);
		fileclose(nf);
		goto out;
	}
	/*
	 * Take new files.
	 */
	fseek(nf, (off_t)0, SEEK_END);
	state.collect.fp = nf;
	fileclose(fp);
 out:
	signal(SIGINT, sigint);
}

/*
 * Return the name of the dead.letter file.
 */
static char*
deadletter(void)
{
	register char*	s;

	if ((s = expand(state.var.dead, 1)) && *s != '/') {
		sfprintf(state.path.temp, "~/%s", s);
		s = expand(struse(state.path.temp), 1);
	}
	return s;
}

/*
 * Interpolate the named messages into the current
 * message, preceding each line with a tab.
 * Return a count of the number of characters now in
 * the message, or -1 if an error is encountered writing
 * the message temporary.  The flag argument is 'm' if we
 * should shift over and 'f' if not.
 */
static int
interpolate(char* ms, FILE* fp, int f, int followup)
{
	register struct msg*	mp;
	register struct msg*	ip;
	Dt_t**			ignore;
	char*			prefix;
	unsigned long		flags;

	if (getmsglist(ms, 0) < 0)
		return 0;
	if (!state.msg.list->m_index) {
		if (!(state.msg.list->m_index = first(0, MMNORM))) {
			note(0, "No appropriate messages");
			return 0;
		}
		(state.msg.list + 1)->m_index = 0;
	}
	flags = 0;
	if (f == 'f' || f == 'F') {
		if (f == 'f')
			flags |= GINTERPOLATE|GMIME;
		prefix = 0;
	}
	else if (!(prefix = state.var.indentprefix))
		prefix = "\t";
	ignore = isupper(f) ? (Dt_t**)0 : &state.ignore;
	if (!followup)
		printf("Interpolating:");
	for (ip = state.msg.list; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		touchmsg(mp);
		if (followup)
			flags |= GREFERENCES;
		else
			printf(" %d", ip->m_index);
		if (copy(mp, fp, ignore, prefix, flags) < 0) {
			note(SYSTEM, "%s", state.tmp.mail);
			return -1;
		}
	}
	if (!followup)
		printf("\n");
	return 0;
}

/*
 * Generate multipart boundary.
 */
void
boundary(void)
{
	if (!state.part.out.multi) {
		state.part.out.multi = 1;
		state.part.out.boundlen = sfsprintf(state.part.out.boundary, sizeof(state.part.out.boundary), "=_=_=_=_=%s==%04X==%08X==", state.var.user, getpid(), time(NiL));
	}
}

/*
 * Ouput multipart header.
 */
static void
part(FILE* fp, char* name, char* type, int code)
{
	char*	s;

	if (name && (s = strrchr(name, '/')))
		name = s + 1;
	boundary();
	fprintf(fp, "\n--%s\n", state.part.out.boundary);
	if (!type)
		type = (code & CODE_64) ? "application/octet-stream" : "text/plain";
	fprintf(fp, "Content-Type: %s", type);
	if (!mimecmp("text", type, NiL))
		fprintf(fp, "; charset=us-ascii");
	if (name)
		fprintf(fp, "; name=\"%s\"", name);
	fprintf(fp, "\nContent-Transfer-Encoding: %s\n", (code & CODE_64) ? "base64" : (code & CODE_QP) ? "quoted-printable" : "7bit");
	if (name)
		fprintf(fp, "Content-Disposition: attachment; filename=\"%s\"\n", name);
	fprintf(fp, "\n");
}

/*
 * Collect input from standard input, handling ~ escapes.
 */
FILE*
collect(struct header* hp, unsigned long flags)
{
	register char*	s;
	register int	n;
	int		ask;
	int		c;
	int		eofcount;
	int		escape;
	int		headers;
	int		code;
	int		sig;
	int		g;
	long		cc;
	long		lc;
	long		tc;
	char*		t;
	char*		e;
	FILE*		fp;
	char*		av[2];
	struct parse	pp;

	ask = 0;
	state.collect.fp = 0;
	state.part.out.multi = 0;
	headers = 0;
	/*
	 * Start catching signals from here, but we'll still die on interrupts
	 * until we're in the main loop.
	 */
#ifdef SIGTSTP
	state.collect.sigtstp = signal(SIGTSTP, collstop);
#endif
#ifdef SIGTTOU
	state.collect.sigttou = signal(SIGTTOU, collstop);
#endif
#ifdef SIGTTIN
	state.collect.sigttin = signal(SIGTTIN, collstop);
#endif
	if ((sig = setjmp(state.collect.abort)) || (sig = setjmp(state.collect.work))) {
		resume(sig);
		rm(state.tmp.mail);
		goto err;
	}
	if ((state.collect.sigint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
		signal(SIGINT, collint);
	if ((state.collect.sighup = signal(SIGHUP, SIG_IGN)) != SIG_IGN)
		signal(SIGHUP, collhup);
	state.noreset++;
	if (!(state.collect.fp = fileopen(state.tmp.mail, "Ew+")))
		goto err;
	remove(state.tmp.mail);

	pp.fp = stdin;
	if ((flags & INTERPOLATE) && interpolate("", state.collect.fp, 'm', 1) < 0)
		goto err;
	if ((flags & INTERPOLATE) && state.var.interactive) {
		rewind(state.collect.fp);
		editmessage(hp, 'v');
		note(0, "(continue)");
	}
	else {
		g = GEDIT|GNL;
		if (state.var.sendheaders) {
			headers = 1;
			flags |= HEADERS;
			g &= ~GNL;
		}
		else if (state.var.interactive && !hp->h_subject && (state.askheaders & GSUB)) {
			ask |= GSUB;
			g &= ~GNL;
		}
		if (flags & HEADERS) {
			if (flags & (FOLLOWUP|INTERPOLATE))
				g |= GREFERENCES;
			headout(stdout, hp, g);
			fflush(stdout);
		}
		if (headers)
			headset(&pp, NiL, pp.fp, hp, NiL, g|GDISPLAY);
	}
	escape = state.var.escape ? *state.var.escape : 0;
	eofcount = 0;
	state.collect.hadintr = 0;
	if (sig = setjmp(state.collect.work)) {
		/*
		 * Come here for printing the after-signal message.
		 * Duplicate messages won't be printed because
		 * the write is aborted if we get a SIGTTOU.
		 */
		resume(sig);
 cont:
		if (state.collect.hadintr)
			note(0, "\n(Interrupt -- one more to kill letter)");
		else
			note(0, "(continue)");
	}
	else if (ask)
		grabedit(hp, ask);
	for (;;) {
		state.collect.working = 1;
		if (headers) {
			if (headget(&pp))
				continue;
			headers = 0;
		}
		c = readline(pp.fp, pp.buf, sizeof(pp.buf));
		state.collect.working = 0;
		if (c < 0) {
			if (flags & SIGN) {
				flags &= ~SIGN;
				c = '.';
				goto sign;
			}
			if (state.var.interactive &&
			    state.var.ignoreeof && ++eofcount < 32) {
				note(0, "Use \".\" to terminate letter");
				continue;
			}
			break;
		}
		eofcount = 0;
		state.collect.hadintr = 0;
		if (pp.buf[0] == '.' && pp.buf[1] == 0 &&
		    state.var.interactive &&
		    (state.var.dot || state.var.ignoreeof)) {
			if (flags & SIGN) {
				flags &= ~SIGN;
				c = '.';
				goto sign;
			}
			break;
		}
		if (pp.buf[0] != escape || !state.var.interactive) {
			if (putline(state.collect.fp, pp.buf) < 0)
				goto err;
			continue;
		}
		c = pp.buf[1];
		switch (c) {
		case '!':
			/*
			 * Shell escape, send the balance of the
			 * line to sh -c.
			 */
			shell(&pp.buf[2]);
			break;
		case '.':
			/*
			 * Simulate end of file on input.
			 */
			if (flags & SIGN) {
				flags &= ~SIGN;
				c = '.';
				goto sign;
			}
			goto out;
		case ':':
		case '_':
			/*
			 * Escape to command mode, but be nice!
			 */
			execute(&pp.buf[2], 1);
			goto cont;
		case '?':
			pp.buf[1] = *state.var.escape;
			av[0] = &pp.buf[1];
			av[1] = 0;
			help(av);
			break;
		case 'A':
		case 'a':
		sign:
			/*
			 * Sign letter.
			 */
			s = c == 'A' ? state.var.Sign : state.var.sign;
			if (s)
				goto outstr;
			if (state.var.signature && (fp = fileopen(state.var.signature, "EXr"))) {
				filecopy(state.var.signature, fp, state.var.signature, state.collect.fp, stdout, (off_t)0, NiL, NiL, 0);
				fileclose(fp);
			}
			if (c == '.')
				goto out;
			goto cont;
		case 'b':
			/*
			 * Add stuff to blind carbon copies list.
			 */
			extract(hp, GBCC|GMETOO, &pp.buf[2]);
			break;
		case 'c':
			/*
			 * Add to the CC list.
			 */
			extract(hp, GCC|GMETOO, &pp.buf[2]);
			break;
		case 'd':
			strncopy(pp.buf + 2, deadletter(), sizeof(pp.buf) - 2);
			/* fall into . . . */
		case 'g':
		case 'r':
		case '<':
			/*
			 * Invoke a file:
			 * Search for the file name,
			 * then open it and copy the contents
			 * to state.collect.fp.
			 */
			for (s = &pp.buf[2]; isspace(*s); s++);
			if (!*s || !(s = expand(s, 1))) {
				note(0, "Read what file !?");
				break;
			}
			if (c == 'g') {
				if (!(fp = fileopen(s, "ERr")))
					break;
				code = 0;
				if (mime(1)) {
					t = mimetype(state.part.mime, fp, s, &state.openstat);
					if (!mimecmp("text", t, NiL))
						code |= CODE_TEXT;
				}
				else
					t = 0;
				cc = tc = 0;
				while ((lc = getc(fp)) != EOF) {
					tc++;
					if ((iscntrl(lc) || !isprint(lc)) && !isspace(lc))
						cc++;
				}
				code |= cc ? ((cc < tc / 80) ? CODE_QP : CODE_64) : CODE_TEXT;
				part(state.collect.fp, s, t, code);
				if (!(code & (CODE_64|CODE_QP)))
					rewind(fp);
				else {
					fileclose(fp);
					if (t) {
						if (e = mimeview(state.part.mime, "compose", state.tmp.mail, t, NiL)) {
							sfprintf(state.path.temp, "%s<%s", e, s);
							if (state.part.disc.flags & MIME_PIPE) {
								sfprintf(state.path.temp, "|");
								t = 0;
							}
							else {
								sfprintf(state.path.temp, ";");
								t = state.tmp.mail;
							}
						}
						else
							t = s;
					}
					else
						t = s;
					sfprintf(state.path.temp, "uuencode -h -x %s", (code & CODE_QP) ? "quoted-printable " : "base64 ");
					if (t)
						shquote(state.path.temp, t);
					sfputc(state.path.temp, ' ');
					shquote(state.path.temp, s);
					if (t == state.tmp.mail)
					{
						sfprintf(state.path.temp, "; rm -f ");
						shquote(state.path.temp, t);
					}
					if (!(fp = pipeopen(struse(state.path.temp), "r")))
						break;
				}
			}
			else if (!(fp = iscmd(s) ? pipeopen(s + 1, "r") : fileopen(s, "ERr")))
				break;
			note(PROMPT, "\"%s\" ", s);
			lc = 0;
			cc = 0;
			while (readline(fp, pp.buf, LINESIZE) >= 0) {
				lc++;
				if ((n = putline(state.collect.fp, pp.buf)) < 0) {
					fileclose(fp);
					goto err;
				}
				cc += n;
			}
			fileclose(fp);
			if (c == 'g')
				fprintf(state.collect.fp, "--%s--\n\n", state.part.out.boundary);
			note(0, "%d/%d", lc, cc);
			break;
		case 'e':
		case 'v':
			/*
			 * Edit the current message.
			 * 'e' means to use EDITOR
			 * 'v' means to use VISUAL
			 */
			rewind(state.collect.fp);
			editmessage(hp, c);
			goto cont;
		case 'f':
		case 'F':
		case 'm':
		case 'M':
			/*
			 * Interpolate the named messages, if we
			 * are in receiving mail mode.  Does the
			 * standard list processing garbage.
			 * If ~f is given, we don't shift over.
			 */
			if (interpolate(pp.buf + 2, state.collect.fp, c, 0) < 0)
				goto err;
			goto cont;
		case 'h':
			/*
			 * Grab the standard headers.
			 */
			grabedit(hp, GSTD);
			goto cont;
		case 'i':
			/*
			 * Insert variable value.
			 */
			for (s = pp.buf + 2; isspace(*s); s++);
			s = varget(s);
			if (s) {
		outstr:
				putline(state.collect.fp, s);
				if (state.var.interactive)
					putline(stdout, s);
				if (c == '.')
					goto out;
			}
			goto cont;
		case 'p':
			/*
			 * Print out the current state of the
			 * message without altering anything.
			 */
			rewind(state.collect.fp);
			fp = stdout;
			if (sig = setjmp(state.jump.sigpipe))
				resume(sig);
			else {
				if (!state.more.discipline && state.var.interactive && (lc = state.var.crt)) {
					lc -= 5;
					while ((c = getc(state.collect.fp)) != EOF)
						if (c == '\n' && --lc <= 0) {
							if (!(fp = pipeopen(state.var.pager, "Jw")))
								fp = stdout;
							break;
						}
					rewind(state.collect.fp);
				}
				fprintf(fp, "-------\nMessage contains:\n");
				headout(fp, hp, GEDIT|GNL);
				filecopy(NiL, state.collect.fp, NiL, fp, NiL, (off_t)0, NiL, NiL, 0);
			}
			fileclose(fp);
			goto cont;
		case 's':
			/*
			 * Set the Subject list.
			 */
			s = &pp.buf[2];
			while (isspace(*s))
				s++;
			if (hp->h_subject = savestr(s))
				hp->h_flags |= GSUB;
			break;
		case 't':
			/*
			 * Add to the To list.
			 */
			extract(hp, GTO|GMETOO, &pp.buf[2]);
			break;
		case 'w':
			/*
			 * Write the message on a file.
			 */
			s = &pp.buf[2];
			while (*s == ' ' || *s == '\t')
				s++;
			if (*s == 0) {
				note(0, "Write what file !?");
				break;
			}
			if (!(s = expand(s, 1)))
				break;
			rewind(state.collect.fp);
			exwrite(s, state.collect.fp, 1);
			break;
		case 'x':
			state.var.save = 0;
			/*FALLTHROUGH*/
		case 'q':
			/*
			 * Force a quit of sending mail.
			 * Act like an interrupt happened.
			 */
			state.collect.hadintr++;
			collint(SIGINT);
			exit(1);
		case '|':
			/*
			 * Pipe message through command.
			 * Collect output as new message.
			 */
			rewind(state.collect.fp);
			pipemessage(state.collect.fp, &pp.buf[2]);
			goto cont;
		default:
			/*
			 * On double escape, just send the single one.
			 * Otherwise, it's an error.
			 */
			if (c == escape) {
				if (putline(state.collect.fp, &pp.buf[1]) < 0)
					goto err;
				else
					break;
			}
			if (isprint(c))
				note(0, "\"%c%c\": unknown escape command", escape, c);
			else
				note(0, "\"%c\\%03o\": unknown escape command", escape, c);
			break;
		}
	}
	goto out;
 err:
	if (state.collect.fp) {
		fileclose(state.collect.fp);
		state.collect.fp = 0;
	}
 out:
	if (state.collect.fp) {
		rewind(state.collect.fp);
		if (state.part.out.multi) {
			/*
			 * Copy to a temp file adding the mime boundaries.
			 */
			if (fp = fileopen(state.tmp.edit, "EMw+")) {
				fprintf(fp, "This is a multipart message in MIME format.\n");
				n = PART_INIT;
				while (s = fgets(state.path.path, sizeof(state.path.path), state.collect.fp)) {
					if (s[0] == '-' && s[1] == '-' && !strncmp(s + 2, state.part.out.boundary, state.part.out.boundlen)) {
						if (n == PART_INIT)
							putc('\n', fp);
						t = s + state.part.out.boundlen + 2;
						if (*t == '\n' || *t == '\r' && *(t + 1) == '\n')
							n = PART_DATA;
						else if (*t++ == '-' && *t++ == '-' && (*t == '\n' || *t == '\r' && *(t + 1) == '\n')) {
							n = PART_INIT;
							continue;
						}
					}
					else if (n == PART_INIT) {
						if (*s == '\n' || *s == '\r' && *(s + 1) == '\n')
							continue;
						n = PART_MAIN;
						part(fp, NiL, NiL, 0);
					}
					fputs(s, fp);
				}
				fprintf(fp, "\n--%s--\n\n", state.part.out.boundary);
				fileclose(state.collect.fp);
				remove(state.tmp.edit);
				state.collect.fp = fp;
				rewind(state.collect.fp);
			}
		}
	}
	state.noreset--;
#ifdef SIGTSTP
	signal(SIGTSTP, state.collect.sigtstp);
#endif
#ifdef SIGTTOU
	signal(SIGTTOU, state.collect.sigttou);
#endif
#ifdef SIGTTIN
	signal(SIGTTIN, state.collect.sigttin);
#endif
	signal(SIGINT, state.collect.sigint);
	signal(SIGHUP, state.collect.sighup);
	return state.collect.fp;
}

/*
 * Save fp in ${DEAD}.
 */
void
savedeadletter(FILE* fp)
{
	FILE*	dp;
	char*	s;

	if (filesize(fp) && (s = deadletter()) && (dp = fileopen(s, "Ma"))) {
		filecopy(NiL, fp, s, dp, NiL, (off_t)0, NiL, NiL, 0);
		fileclose(dp);
		rewind(fp);
	}
}
