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
 * Still more user commands.
 */

#include "mailx.h"

/*
 * Expand the shell escape by expanding unescaped !'s into the
 * last issued command where possible.
 */
static int
bangexp(char* str, size_t size)
{
	register char*	s;
	register char*	t;
	register char*	e;
	int		changed = 0;
	char		buf[LINESIZE];

	s = str;
	t = buf;
	e = t + sizeof(buf) - 1;
	while (*s) {
		if (*s == '!' && state.var.bang) {
			if ((e - t) < strlen(state.last.bang)) {
 overf:
				note(0, "Command buffer overflow");
				return -1;
			}
			changed++;
			t = strcopy(t, state.last.bang);
			s++;
			continue;
		}
		if (*s == '\\' && s[1] == '!') {
			if (t >= e)
				goto overf;
			*t++ = '!';
			s += 2;
			changed++;
		}
		if (t >= e)
			goto overf;
		*t++ = *s++;
	}
	*t = 0;
	if (changed)
		note(0, "!%s", buf);
	strncopy(str, buf, size);
	strncopy(state.last.bang, buf, sizeof(state.last.bang) - 1);
	return 0;
}

/*
 * Process a shell escape by saving signals, ignoring signals,
 * and running sh -c
 */
int
shell(char* str)
{
	sig_t	saveint = signal(SIGINT, SIG_IGN);
	char	cmd[LINESIZE];

	strncopy(cmd, str, sizeof(cmd));
	if (bangexp(cmd, sizeof(cmd)) < 0)
		return 1;
	if (*cmd)
		run_command(state.var.shell, 0, -1, -1, "-c", cmd, NiL);
	else
		run_command(state.var.shell, 0, -1, -1, NiL, NiL, NiL);
	signal(SIGINT, saveint);
	note(0, "!");
	return 0;
}

/*
 * Stay within the lines.
 */
static void
margin(FILE* fp, const char* name, register const char* s, int indent, int sep, int tab, const struct var* vp)
{
	register const char**	ap;
	register int		c;
	register int		n;
	register int		q;
	const char*		av[8];

	if (sep) {
		fprintf(fp, "%*s", n = indent, name);
		for (c = 0; c < sep; c++) {
			putc(' ', fp);
			n++;
		}
	}
	else {
		fprintf(fp, "%s ", name);
		n = strlen(name) + 1;
	}
	ap = av;
	*ap++ = T(s);
	if (vp && (s = vp->initialize)) {
		*ap++ = " ";
		if (*s) {
			*ap++ = T("The default value is");
			*ap++ = " \"";
			*ap++ = s;
			*ap++ = "\".";
		}
		else if (vp->set)
			*ap++ = T("The default value is computed at runtime.");
		else
			*ap++ = T("On by default.");
	}
	*ap = 0;
	ap = av;
	q = 0;
	while (s = *ap++)
		while (c = *s++) {
			if (c == '\n' || (isspace(c) || q && !isalnum(c)) && n >= (MARGIN - 4)) {
				if (!isspace(c))
					putchar(c);
				fprintf(fp, "\n%*s", n = indent, " ");
				for (c = 0; c < sep; c++) {
					putc(' ', fp);
					n++;
				}
			}
			else if (c == '\t') {
				if (tab) {
					while (n < tab) {
						putc(' ', fp);
						n++;
					}
				}
				else do {
					putc(' ', fp);
				} while (++n % 4);
			}
			else {
				if (c == q)
					q = 0;
				else if (c == '"')
					q = c;
				putc(c, fp);
				n++;
			}
		}
	putc('\n', fp);
}

/*
 * List command help.
 */
static void
helpcmd(FILE* fp, register const struct cmd* cp)
{
	margin(fp, cp->c_name, cp->c_help, 4, 0, 0, NiL);
}

/*
 * List escape command help.
 */
static void
helpesc(FILE* fp, register const struct esc* ep)
{
	char	buf[3];

	buf[0] = *state.var.escape;
	buf[1] = *ep->e_name;
	buf[2] = 0;
	margin(fp, buf, ep->e_help, 4, 0, 16, NiL);
}

/*
 * List variable help.
 */
static void
helpvar(FILE* fp, register const struct var* vp)
{
	char*	help;

	if (vp->flags & A)
	{
		sfprintf(state.path.temp, T("Equivalent to %s."), vp->help);
		help = struse(state.path.temp);
	}
	else
		help = (char*)vp->help;
	margin(fp, vp->name, help, 14, 2, 0, vp);
}

/*
 * Print out a nice help message.
 */
int
help(char** argv)
{
	register const struct cmd*	cp;
	register const struct esc*	ep;
	register const struct var*	vp;
	register char*			s;
	register char*			a;
	register char*			t;
	register char*			l;
	char*				r;
	int				all;
	int				cat;
	FILE*				fp;

	if (state.more.discipline ||
	    !state.var.interactive ||
	    !state.var.crt ||
	    !(fp = pipeopen(state.var.pager, "w")))
		fp = stdout;
	s = *argv++;
	a = s ? *argv : (char*)0;
	l = "commands";
	t = T(l);
	r = "--------";
	all = isall(s);
	cat = 0;
	if (all || !s || (streq(s, l) || streq(s, t)) && ++cat) {
		fprintf(fp, "%s\n%s\n%s\n", r, t, r);
		for (cp = state.cmdtab; cp < &state.cmdtab[state.cmdnum]; cp++)
			helpcmd(fp, cp);
	}
	if (all || s) {
		l = "variables";
		t = T(l);
		r = "---------";
		if (all && !(cp = 0)|| (cp = (const struct cmd*)strpsearch(state.cmdtab, state.cmdnum, sizeof(struct cmd), s, NiL)) || (streq(s, l) || streq(s, t)) && ++cat) {
			if (!cp || a && cp->c_func == (Cmd_f)set) {
				if (!cp && !cat || !a || isall(a)) {
					fprintf(fp, "%s\n%s\n%s\n", r, t, r);
					for (vp = state.vartab; vp < &state.vartab[state.varnum]; vp++)
						helpvar(fp, vp);
				}
				else if (vp = (const struct var*)strsearch(state.vartab, state.varnum, sizeof(struct var), stracmp, a, NiL))
					helpvar(fp, vp);
				else
					note(0, "\"%s\": unknown variable", a);
			}
			else
				helpcmd(fp, cp);
		}
		if (!cp) {
			if (s && (*s == '~' || *s == *state.var.escape)) {
				if (*++s)
					a = s;
				s = state.var.escape;
			}
			l = "escape";
			t = T(l);
			r = "---------------";
			if (all || *s == *state.var.escape || (streq(s, t) || streq(s, l) || streq(s, "tilde")) && ++cat) {
				if (!a) {
					fprintf(fp, "%s\n%s %s\n%s\n", r, t, T("commands"), r);
					for (ep = state.esctab; ep < &state.esctab[state.escnum]; ep++)
						helpesc(fp, ep);
				}
				else if (ep = (const struct esc*)strsearch(state.esctab, state.escnum, sizeof(struct esc), stracmp, a, NiL))
					helpesc(fp, ep);
				else
					note(0, "\"%s\": unknown escape command", a);
			}
			else if (!cat)
				note(0, "\"%s\": unknown command", s);
		}
	}
	fileclose(fp);
	return 0;
}

/*
 * Change user's working directory.
 */
int
cd(char** arglist)
{
	char*	cp;
	char*	tp;
	int	show = 0;

	if (!*arglist)
		cp = state.var.home;
	else
		if (!(cp = expand(*arglist, 1)))
			return 1;
	if (cp[0] == '-' && cp[1] == 0) {
		if (!(cp = state.var.oldpwd)) {
			note(0, "No previous working directory");
			return 1;
		}
		show = 1;
	}
	if (chdir(cp) < 0) {
#if _PACKAGE_ast
		if (state.var.cdpath && (cp[0] != '.' || cp[1] != 0 && cp[1] != '/' && (cp[1] != '.' || cp[2] != 0 && cp[2] != '/')) && pathaccess(state.var.cdpath, cp, NiL, 0, state.path.path, sizeof(state.path.path))) {
			cp = state.path.path;
			show = 1;
		}
		else
#endif
			show = -1;
		if (show < 0 || chdir(cp) < 0) {
			note(SYSTEM, "%s", cp);
			return 1;
		}
	}
	tp = state.var.oldpwd;
	state.var.oldpwd = state.var.pwd;
	state.var.pwd = tp;
	if (!getcwd(state.var.pwd, PATHSIZE))
		strncopy(state.var.pwd, cp, PATHSIZE);
	if (show)
		printf("%s\n", state.var.pwd);
	return 0;
}

/*
 * Print the full path of the current working directory.
 */
int
pwd(void)
{
	printf("%s\n", state.var.pwd);
	return 0;
}

/*
 * Create a new folder directory.
 */

int
cmdmkdir(char** arglist)
{
	char*	cp;

	if (!(cp = expand(*arglist, 1)))
		return 1;
	if (*cp == '@')
		imap_mkdir(cp);
	else if (mkdir(cp, MAILMODE|S_IXUSR)) {
		note(SYSTEM, "%s", cp);
		return 1;
	}
	return 0;
}

/*
 * Rename a folder or folder directory.
 */

int
cmdrename(char** arglist)
{
	char*	f;
	char*	t;

	if (!(f = expand(arglist[0], 1)) || !(t = expand(arglist[1], 1)))
		return 1;
	if (*f == '@')
		imap_rename(f, t);
	else if (rename(f, t)) {
		note(SYSTEM, "%s: cannot rename to %s", f, t);
		return 1;
	}
	return 0;
}

/*
 * Remove an empty folder directory.
 */

int
cmdrmdir(char** arglist)
{
	char*	cp;

	if (!(cp = expand(*arglist, 1)))
		return 1;
	if (*cp == '@')
		imap_rmdir(cp);
	else if (rmdir(cp)) {
		note(SYSTEM, "%s", cp);
		return 1;
	}
	return 0;
}

/*
 * Modify the subject we are replying to to begin with Re: if
 * it does not already.
 */
static char*
reedit(register char* subj)
{
	char*	newsubj;

	if (!subj)
		return 0;
#if _PACKAGE_ast
	if (isalpha(subj[0]) && isalpha(subj[1]) && subj[2] == ':' && subj[3] == ' ' && subj[4])
#else
	if ((subj[0] == 'r' || subj[0] == 'R') &&
	    (subj[1] == 'e' || subj[1] == 'E') &&
	    subj[2] == ':')
#endif
		return subj;
	newsubj = salloc(strlen(subj) + 5);
	strcpy(newsubj, "Re: ");
	strcpy(newsubj + 4, subj);
	return newsubj;
}

/*
 * Low level for the reply variants.
 */
static int
reply1(struct msg* msgvec, unsigned long flags, int all)
{
	struct msg*	mp;
	char*		cp;
	char*		rp;
	struct msg*	ip;
	struct header	head;

	memset(&head, 0, sizeof(head));
	if (all) {
		if (msgvec->m_index && (msgvec + 1)->m_index) {
			note(0, "Sorry, can't reply to multiple messages at once");
			return 1;
		}
		mp = state.msg.list + msgvec->m_index - 1;
		touchmsg(mp);
		state.msg.dot = mp;
		rp = grab(mp, GREPLY, NiL);
		if (cp = grab(mp, GTO, NiL))
			extract(&head, GTO, cp);
		if (cp = grab(mp, GCC, NiL))
			extract(&head, GCC, cp);
		extract(&head, GTO|GFIRST|GMETOO, rp);
	}
	else {
		for (ip = msgvec; ip->m_index; ip++) {
			mp = state.msg.list + ip->m_index - 1;
			touchmsg(mp);
			state.msg.dot = mp;
			extract(&head, GTO, grab(mp, GREPLY, NiL));
		}
		if (!head.h_names)
			return 0;
		mp = state.msg.list + msgvec->m_index - 1;
	}
	if (head.h_subject = grab(mp, GSUB, NiL))
		head.h_flags |= GSUB;
	head.h_subject = reedit(head.h_subject);
	if (flags & (FOLLOWUP|INTERPOLATE)) {
		if (head.h_messageid = grab(mp, GMESSAGEID, NiL))
			head.h_flags |= GMESSAGEID;
		if (head.h_references = grab(mp, GREFERENCES, NiL))
			head.h_flags |= GREFERENCES;
	}
	sendmail(&head, flags|HEADERS);
	return 0;
}

int
followup(struct msg* msgvec)
{
	return reply1(msgvec, FOLLOWUP, !state.var.flipr);
}

int
Followup(struct msg* msgvec)
{
	return reply1(msgvec, FOLLOWUP, !!state.var.flipr);
}

int
join(struct msg* msgvec)
{
	return reply1(msgvec, INTERPOLATE, !state.var.flipr);
}

int
Join(struct msg* msgvec)
{
	return reply1(msgvec, INTERPOLATE, !!state.var.flipr);
}

int
reply(struct msg* msgvec)
{
	return reply1(msgvec, 0, !state.var.flipr);
}

int
Reply(struct msg* msgvec)
{
	return reply1(msgvec, 0, !!state.var.flipr);
}

int
replyall(struct msg* msgvec)
{
	return reply1(msgvec, 0, 1);
}

int
replysender(struct msg* msgvec)
{
	return reply1(msgvec, 0, 0);
}

/*
 * Print the size of each message.
 */
int
size(struct msg* msgvec)
{
	register struct msg*	mp;
	register struct msg*	ip;

	for (ip = msgvec; ip->m_index; ip++) {
		mp = state.msg.list + ip->m_index - 1;
		note(0, "%d: %ld/%ld", mp->m_index, (long)mp->m_lines, (long)mp->m_size);
	}
	return 0;
}

/*
 * Quit quickly.  If we are sourcing, just pop the input level
 * by returning an error.
 */
int
cmdexit(int e)
{
	if (!state.sourcing) {
		if (state.msg.imap.state)
			imap_exit(e);
		exit(e);
	}
	return 1;
}

/*
 * Set or display a variable value.  Syntax is similar to that
 * of csh.
 */
int
set(register char** argv)
{
	register char*	cp;
	register char*	np;
	register char*	ep;
	int		all = 0;
	int		errs = 0;

	if (!*argv || (all = isall(*argv) && !*(argv + 1)))
		errs += varlist(all);
	else while (cp = *argv++) {
		np = cp;
		for (;;) {
			if (*cp == '=') {
				ep = cp;
				*cp++ = 0;
				break;
			}
			if (!*cp++) {
				ep = 0;
				cp = state.on;
				break;
			}
		}
		if (!*np) {
			note(0, "Non-null variable name required");
			errs++;
		}
		else
			errs += varset(np, cp);
		if (ep)
			*ep = '=';
	}
	return errs;
}

/*
 * Unset a bunch of variable values.
 */
int
unset(register char** argv)
{
	register char*	s;
	register int	all = -1;
	register int	errs = 0;

	if (!*argv || (all = isall(*argv) && !*(argv + 1)))
		errs += varlist(all);
	else while (s = *argv++)
		errs += varset(s, NiL);
	return errs;
}

/*
 * List an alias.
 */
static int
listalias(Dt_t* dt, void* object, void* context)
{
	register struct list*	mp;

	printf("%-16s", ((struct name*)object)->name);
	for (mp = (struct list*)((struct name*)object)->value; mp; mp = mp->next)
		printf(" %s", mp->name);
	putchar('\n');
	return 0;
}

/*
 * Low level for alias/alternates.
 */
static int
alias1(register char** argv, unsigned long flags)
{
	register struct name*	ap;
	register struct list*	mp;
	register char*		s;
	register char*		t;
	register char*		v;
	char*			name;
	FILE*			fp;
	char			buf[LINESIZE];

	if (!(name = *argv++))
		dictwalk(&state.aliases, listalias, NiL);
	else if (!*argv) {
		if (!(ap = dictsearch(&state.aliases, name, LOOKUP|IGNORECASE)))
			note(0, "\"%s\": unknown alias", name);
		else
			listalias(NiL, ap, NiL);
	}
	else if (streq(name, "<")) {
		/*
		 * sendmail alias file parse
		 */
		while (name = *argv++) {
			if (fp = fileopen(name, "EXr")) {
				while (s = fgets(buf, sizeof(buf), fp)) {
					for (; isspace(*s); s++);
					if (*s != '#') {
						for (t = s; *t && *t != ':' && !isspace(*t); t++);
						if (*t) {
							*t++ = 0;
							while (*t == ':' || isspace(*t))
								t++;
							ap = dictsearch(&state.aliases, s, INSERT|IGNORECASE);
							ap->flags |= flags;
							do {
								for (v = t; *v && *v != ',' && !isspace(*v); v++);
								if (*v) {
									*v++ = 0;
									while (*v == ',' || isspace(*v))
										v++;
								}
								if (!(mp = newof(0, struct list, 1, strlen(t) + 1)))
									note(PANIC, "Out of space");
								strcpy(mp->name, t);
								mp->next = (struct list*)ap->value;
								ap->value = (void*)mp;
							} while (*(t = v));
						}
					}
				}
				fileclose(fp);
			}
		}
	}
	else {
		/*
		 * Insert names from the command list into the alias group.
		 */
		ap = dictsearch(&state.aliases, name, INSERT|IGNORECASE);
		ap->flags |= flags;
		while (name = *argv++) {
			if (!(mp = newof(0, struct list, 1, strlen(name) + 1)))
				note(PANIC, "Out of space");
			strcpy(mp->name, name);
			mp->next = (struct list*)ap->value;
			ap->value = (void*)mp;
		}
	}
	return 0;
}

/*
 * Case sensitive alias.
 */
int
alias(register char** argv)
{
	return alias1(argv, GALIAS);
}

/*
 * Unset a bunch of aliases.
 */
int
unalias(register char** argv)
{
	register char*	s;

	while (s = *argv++)
		if (!(dictsearch(&state.aliases, s, DELETE)))
			note(0, "\"%s\": unknown alias", s);
	return 0;
}

/*
 * List an alternate.
 */
static int
listalternate(Dt_t* dt, void* object, void* context)
{
	if (((struct name*)object)->flags & GALTERNATE)
		printf("%s\n", ((struct name*)object)->name);
	return 0;
}

/*
 * Set the list of alternate names.
 */
int
alternates(register char** argv)
{
	register char*	s;
	char*		av[3];

	if (!*argv)
		dictwalk(&state.aliases, listalternate, NiL);
	else {
		av[1] = state.var.user;
		av[2] = 0;
		while (s = *argv++) {
			av[0] = s;
			alias1(av, GALTERNATE);
		}
	}
	return 0;
}

/*
 * The do nothing command for comments.
 */

/*ARGSUSED*/
int
null(int e)
{
	return 0;
}

/*
 * Change to another folder.  With no argument, print information about
 * the current folder.
 */
int
folder(register char** argv)
{
	if (!argv[0]) {
		folderinfo(0);
		return 0;
	}
	if (setfolder(*argv) < 0)
		return 1;
	announce();
	return 0;
}

/*
 * Expand file names like echo
 */
int
echo(register char** argv)
{
	register char*	s;
	register int	sep;

	sep = 0;
	while (s = *argv++) {
		if (s = expand(s, 0)) {
			if (sep++)
				putchar(' ');
			printf("%s", s);
		}
	}
	putchar('\n');
	return 0;
}

/*
 * Conditional commands.  These allow one to parameterize one's
 * .mailrc and do some things if sending, others if receiving.
 */
int
cmdif(char** argv)
{
	register char*	s;
	register char*	t;
	register char*	x;
	int		n;

	if (state.cond) {
		note(0, "Invalid nested \"%s\"", state.cmd->c_name);
		return 1;
	}
	s = argv[0];
	t = (x = argv[1]) ? argv[2] : (char*)0;
	if (n = streq(s, "!")) {
		s = x;
		x = t;
		t = 0;
	}
	else if (n = *s == '!')
		s++;
	if (*s && (!*(s + 1) || *(s + 1) == '?') && !x && *(x = s + 2)) {
		switch (*s) {
		case 'd':
			if (!x || t)
				goto bad;
			n ^= isdir(expand(x, 1));
			goto ok;
		case 'f':
			if (!x || t)
				goto bad;
			n ^= isreg(expand(x, 1));
			goto ok;
		case 'r':
		case 'R':
			if (x)
				goto bad;
			state.cond = n ? SEND : RECEIVE;
			return 0;
		case 's':
		case 'S':
			if (x)
				goto bad;
			state.cond = n ? RECEIVE : SEND;
			return 0;
		}
	}
	if (!x)
		n ^= varget(s) != 0;
	else if (!t || n)
		goto bad;
	else {
		if (n = *x == '!')
			x++;
		switch (*x) {
		case '=':
			if (*++x == '=')
				x++;
			n ^= streq(s, t);
			break;
		default:
			n = -1;
			break;
		}
		if (n < 0 || *x) {
			note(0, "\"%s\": unknown %s condition operator", argv[1], state.cmd->c_name);
			return 1;
		}
	}
 ok:
	state.cond = n ? state.mode : -state.mode;
	return 0;
 bad:
	sfprintf(state.path.temp, "%s", s);
	if (x) {
		sfprintf(state.path.temp, " %s", x);
		if (t)
			sfprintf(state.path.temp, " %s", t);
	}
	note(0, "\"%s\": unknown %s condition", struse(state.path.temp), state.cmd->c_name);
	return 1;
}

/*
 * Implement 'else'.  This is pretty simple -- we just
 * flip over the conditional flag.
 */
int
cmdelse(void)
{
	if (!state.cond) {
		note(0, "\"%s\" without matching \"if\"", state.cmd->c_name);
		return 1;
	}
	state.cond = -state.cond;
	return 0;
}

/*
 * End of if statement.  Just set cond back to anything.
 */
int
cmdendif(void)
{

	if (!state.cond) {
		note(0, "\"%s\" without matching \"if\"", state.cmd->c_name);
		return 1;
	}
	state.cond = 0;
	return 0;
}
