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
 * Local installation dependent routines.
 */

#include "mailx.h"

#include <pwd.h>

#if _PACKAGE_ast
#include <tm.h>
#endif

/*
 * Locate the user's mailbox file (ie, the place where new, unread
 * mail is queued).
 */
char*
mailbox(const char* user, const char* mail)
{
	register char*		s;
	register int		i;
	int			n = 0;
	struct stat		st;

	static const char*	dir[] = {
		_PATH_MAILDIR,
		"/var/spool/mail",
		"/usr/spool/mail",
		"/usr/mail"
	};

	if (!user || !*user || !stat(mail, &st) && S_ISREG(st.st_mode))
		return (char*)mail;
	if (mail) {
		if (imap_name(mail))
			return (char*)mail;
		if (s = strrchr(mail, '/')) {
			i = s - (char*)mail;
			sfprintf(state.path.temp, "%-.*s/.", i, mail);
			if (!access(struse(state.path.temp), F_OK))
				n = i;
		}
	}
	if (n == 0) {
		for (i = 0;; i++) {
			if (i >= elementsof(dir)) {
				i = 0;
				break;
			}
			sfprintf(state.path.temp, "%s/.", dir[i]);
			if (!access(struse(state.path.temp), F_OK))
				break;
		}
		mail = dir[i];
		if ((n = strlen(mail)) > 0 && mail[n - 1] == '/')
			n--;
	}
	sfprintf(state.path.temp, "%-.*s/%s", n, mail, user);
	return savestr(struse(state.path.temp));
}

/*
 * Get rid of the queued mail.
 */
void
demail(void)
{
	if (state.var.keep || rm(state.path.mail) < 0)
		close(open(state.path.mail, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY|O_cloexec, MAILMODE));
}

/*
 * Discover user login name.
 */
char*
username(void)
{
	register char*	s;
	register char*	t;
	struct passwd*	pw;

	if ((!(s = getenv("USER")) || !*s) &&
	    (!(s = getenv("LOGIN")) || !*s) &&
	    (pw = getpwuid(getuid())))
		s = pw->pw_name;
	if (s)
	{
		if (t = strrchr(s, '/'))
			s = t + 1;
		if (!*s)
			s = 0;
	}
	return s;
}

/*
 * Convert name to a user id and return it.
 * Mapping cached in state.userid.
 * Return -1 on error.
 */
int
userid(char* name)
{
	register struct name*	up;
	register struct passwd*	pw;

	up = dictsearch(&state.userid, name, INSERT);
	if (!up->value)
		up->flags = (pw = getpwnam(name)) ? pw->pw_uid : -1;
	return up->flags;
}

/*
 * Make sure the lock doesn't hang -- NFS anyone?
 */
static void
alarmed(int sig)
{
	signal(sig, SIG_IGN);
	state.hung = 1;
}

/*
 * Lock (set!=0) or unlock (set==0) open file fp.
 */
int
filelock(const char* name, FILE* fp, int set)
{
#if defined(F_SETLKW) && defined(F_WRLCK) && defined(F_UNLCK) || defined(LOCK_EX) && defined(LOCK_UN)
	int	r;
	sig_t	savealrm;

	if (state.var.lock)
	{
		state.hung = 0;
		savealrm = signal(SIGALRM, alarmed);
		if (!(r = strtol(state.var.lock, NiL, 10)))
			r = 5;
		alarm(r);
#if defined(F_SETLKW) && defined(F_WRLCK) && defined(F_UNLCK)
		{
			struct flock	lck;

			lck.l_type = set ? F_WRLCK : F_UNLCK;
			lck.l_whence = SEEK_SET;
			lck.l_start = 0;
			lck.l_len = 0;
			r = fcntl(fileno(fp), F_SETLKW, &lck);
		}
#else
		r = flock(fileno(fp), set ? LOCK_EX : LOCK_UN);
#endif
		alarm(0);
		signal(SIGALRM, savealrm);
		if (state.hung)
			note(0, "%s: file lock hung -- continuing without lock", name);
		else if (r)
			note(SYSTEM, "%s: continuing without lock", name);
	}
#endif
	return 0;
}

#if _PACKAGE_ast

/*
 * Compute what the screen size for printing headers should be.
 */

void
setscreensize(void)
{
	astwinsize(1, &state.realscreenheight, &state.screenwidth);
	if (!state.realscreenheight)
		state.realscreenheight = 24;
	state.screenheight = state.realscreenheight;
	if (!state.screenwidth)
		state.screenwidth = 80;
}

/*
 * check if date is valid with no trailing junk
 */

int
isdate(char* s)
{
	char*	t;

	(void)tmdate(s, &t, NiL);
	return *t == 0 && strmatch(s, "*[a-zA-Z]*[0-9]:[0-9]*");
}

#else /*_PACKAGE_ast*/

#include <termios.h>

#define CHUNK	32

char*
fmtesc(const char* as)
{
	register unsigned char*	s = (unsigned char*)as;
	register char*		b;
	register int		c;

	static char*		buf;
	static int		bufsiz;

	c = 4 * (strlen((char*)s) + 1);
	if (bufsiz < c)
	{
		bufsiz = ((c + CHUNK - 1) / CHUNK) * CHUNK;
		if (!(buf = newof(buf, char, bufsiz, 0)))
			return 0;
	}
	b = buf;
	while (c = *s++)
	{
		if (iscntrl(c) || !isprint(c))
		{
			*b++ = '\\';
			switch (c)
			{
			case '\007':
				c = 'a';
				break;
			case '\b':
				c = 'b';
				break;
			case '\f':
				c = 'f';
				break;
			case '\n':
				c = 'n';
				break;
			case '\r':
				c = 'r';
				break;
			case '\t':
				c = 't';
				break;
			case '\v':
				c = 'v';
				break;
			case '\033':
				c = 'E';
				break;
			default:
				*b++ = '0' + ((c >> 6) & 07);
				*b++ = '0' + ((c >> 3) & 07);
				c = '0' + (c & 07);
				break;
			}
		}
		*b++ = c;
	}
	*b = 0;
	return buf;
}

#define IDENT	01
#define USAGE	02

/*
 * format what(1) and/or ident(1) string a
 */

char*
fmtident(const char* a)
{
	register char*	s = (char*)a;
	register char*	t;
	char*		buf;
	int		i;

	i = 0;
	for (;;)
	{
		while (isspace(*s))
			s++;
		if (s[0] == '[')
		{
			while (*++s && *s != '\n');
			i |= USAGE;
		}
		else if (s[0] == '@' && s[1] == '(' && s[2] == '#' && s[3] == ')')
			s += 4;
		else if (s[0] == '$' && s[1] == 'I' && s[2] == 'd' && s[3] == ':' && isspace(s[4]))
		{
			s += 5;
			i |= IDENT;
		}
		else
			break;
	}
	if (i)
	{
		i &= IDENT;
		for (t = s; isprint(*t) && *t != '\n'; t++)
			if (i && t[0] == ' ' && t[1] == '$')
				break;
		while (t > s && isspace(t[-1]))
			t--;
		i = t - s;
		if (!(buf = newof(buf, char, i, i)))
			return s;
		memcpy(buf, s, i);
		s = buf;
		s[i] = 0;
	}
	return s;
}

/*
 * return the current shell path
 */

char*
pathshell(void)
{
	char*	shell;

	if (!(shell = state.var.shell) || !*shell)
		shell = _PATH_SHELL;
	return shell;
}

/*
 * Compute what the screen size for printing headers should be.
 * We use the following algorithm for the height:
 *	If baud rate < 1200, use  9
 *	If baud rate = 1200, use 14
 *	If baud rate > 1200, use 24 or ws_row
 * Width is either 80 or ws_col;
 */

void
setscreensize(void)
{
	struct termios	tbuf;
	struct winsize	ws;
	int		speed;

	if (ioctl(1, TIOCGWINSZ, (char*)&ws) < 0)
		ws.ws_col = ws.ws_row = 0;
	if (tcgetattr(1, &tbuf) < 0)
		speed = B9600;
	else
		speed = cfgetospeed(&tbuf);
	if (speed < B1200)
		state.screenheight = 9;
	else if (speed == B1200)
		state.screenheight = 14;
	else if (ws.ws_row)
		state.screenheight = ws.ws_row;
	else
		state.screenheight = 24;
	if (!(state.realscreenheight = ws.ws_row))
		state.realscreenheight = 24;
	if (!(state.screenwidth = ws.ws_col))
		state.screenwidth = 80;
}

/*
 * signal critical region support
 */

static const struct
{
	int	sig;
	int	op;
}
signals[] =		/* held inside critical region	*/
{
	SIGINT,		SIG_REG_EXEC,
#ifdef SIGQUIT
	SIGQUIT,	SIG_REG_EXEC,
#endif
#ifdef SIGHUP
	SIGHUP,		SIG_REG_EXEC,
#endif
#ifdef SIGPIPE
	SIGPIPE,	SIG_REG_EXEC,
#endif
#ifdef SIGCLD
	SIGCLD,		SIG_REG_PROC,
#endif
#ifdef SIGCHLD
	SIGCHLD,	SIG_REG_PROC,
#endif
#ifdef SIGTSTP
	SIGTSTP,	SIG_REG_TERM,
#endif
#ifdef SIGTTIN
	SIGTTIN,	SIG_REG_TERM,
#endif
#ifdef SIGTTOU
	SIGTTOU,	SIG_REG_TERM,
#endif
};

/*
 * critical signal region handler
 *
 * op>0		new region according to SIG_REG_*, return region level
 * op==0	pop region, return region level
 * op<0		return non-zero if any signals held in current region
 *
 * signals[] held until region popped
 */

int
sigcritical(int op)
{
	register int		i;
	sigset_t		nmask;

	static int		region;
	static int		level;
	static sigset_t		mask;

	if (op > 0)
	{
		if (!level++)
		{
			region = op;
			if (op & SIG_REG_SET)
				level--;
			sigemptyset(&nmask);
			for (i = 0; i < elementsof(signals); i++)
				if (op & signals[i].op)
					sigaddset(&nmask, signals[i].sig);
			sigprocmask(SIG_BLOCK, &nmask, &mask);
		}
		return level;
	}
	else if (op < 0)
	{
		sigpending(&nmask);
		for (i = 0; i < elementsof(signals); i++)
			if (region & signals[i].op)
			{
				if (sigismember(&nmask, signals[i].sig))
					return 1;
			}
		return 0;
	}
	else
	{
		/*
		 * a vfork() may have intervened so we
		 * allow apparent nesting mismatches
		 */

		if (--level < 0)
		{
			level = 0;
			sigprocmask(SIG_SETMASK, &mask, NiL);
		}
		return level;
	}
}

/*
 * remove sig from the set of blocked signals
 * sig==0 unblocks them all
 */

int
sigunblock(int sig)
{
	int		op;
	sigset_t	mask;

	sigemptyset(&mask);
	if (sig)
	{
		sigaddset(&mask, sig);
		op = SIG_UNBLOCK;
	}
	else op = SIG_SETMASK;
	return sigprocmask(op, &mask, NiL);
}

/*
 * fork+execvp
 */

int
spawnvp(const char* cmd, char* const* argv)
{
	int	pid;

	sigcritical(1);
	if ((pid = fork()) < 0)
	{
		note(SYSTEM, "fork");
		return -1;
	}
	sigcritical(0);
	if (pid == 0)
	{
		execvp(cmd, argv);
		note(SYSTEM, "%s", cmd);
		_exit(1);
	}
	return pid;
}

/*
 * return the next character in the string s
 * \ character constants are converted
 * p is updated to point to the next character in s
 */

int
chresc(register const char* s, char** p)
{
	register const char*	q;
	register int		c;

	switch (c = *s++)
	{
	case 0:
		s--;
		break;
	case '\\':
		switch (c = *s++)
		{
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			c -= '0';
			q = s + 2;
			while (s < q) switch (*s)
			{
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				c = (c << 3) + *s++ - '0';
				break;
			default:
				q = s;
				break;
			}
			break;
		case 'a':
			c = '\007';
			break;
		case 'b':
			c = '\b';
			break;
		case 'f':
			c = '\f';
			break;
		case 'n':
			c = '\n';
			break;
		case 'r':
			c = '\r';
			break;
		case 's':
			c = ' ';
			break;
		case 't':
			c = '\t';
			break;
		case 'v':
			c = '\013';
			break;
		case 'x':
			c = 0;
			q = s;
			while (q) switch (*s)
			{
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				c = (c << 4) + *s++ - 'a' + 10;
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				c = (c << 4) + *s++ - 'A' + 10;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				c = (c << 4) + *s++ - '0';
				break;
			default:
				q = 0;
				break;
			}
			break;
		case 'E':
			c = '\033';
			break;
		case 0:
			s--;
			break;
		}
		break;
	}
	if (p) *p = (char*)s;
	return c;
}

/*
 * copy t into s, return a pointer to the end of s ('\0')
 */

char*
strcopy(register char* s, register const char* t)
{
	if (!t)
		return s;
	while (*s++ = *t++);
	return s - 1;
}

/*
 * convert \x character constants in s in place
 * the length of the converted s is returned (may have imbedded \0's)
 */

int
stresc(register char* s)
{
	register char*	t;
	register int	c;
	char*		b;
	char*		p;

	b = t = s;
	for (;;)
	{
		switch (c = *s++)
		{
		case '\\':
			c = chresc(s - 1, &p);
			s = p;
			break;
		case 0:
			*t = 0;
			return t - b;
		}
		*t++ = c;
	}
}

/*
 * return a pointer to the isalpha() identifier matching
 * name in the sorted tab of num elements of
 * size siz where the first member of each
 * element is a char*
 *
 * [xxx] brackets optional identifier characters
 *
 * 0 returned if name not found
 * otherwise if next!=0 then it points to the next
 * unmatched char in name
 */

void*
strpsearch(const void* tab, size_t num, size_t siz, const char* name, char** next)
{
	register char*		lo = (char*)tab;
	register char*		hi = lo + (num - 1) * siz;
	register char*		mid;
	register unsigned char*	s;
	register unsigned char*	t;
	register int		c;
	register int		v;
	int			sequential = 0;

	c = *((unsigned char*)name);
	while (lo <= hi) {
		mid = lo + (sequential ? 0 : (((hi - lo) / siz) / 2) * siz);
		if (!(v = c - *(s = *((unsigned char**)mid))) || *s == '[' && !(v = c - *++s) && (v = 1)) {
			t = (unsigned char*)name;
			for (;;) {
				if (!v && *s == '[') {
					v = 1;
					s++;
				}
				else if (v && *s == ']') {
					v = 0;
					s++;
				}
				else if (!isalpha(*t)) {
					if (v || !*s) {
						if (next)
							*next = (char*)t;
						return (void*)mid;
					}
					if (!sequential) {
						while ((mid -= siz) >= lo && (c == *(s = *((unsigned char**)mid)) || *s == '[' && c == *(s + 1)));
						sequential = 1;
					}
					v = 1;
					break;
				}
				else if (*t != *s) {
					v = *t - *s;
					break;
				}
				else {
					t++;
					s++;
				}
			}
		}
		else if (sequential)
			break;
		if (v > 0)
			lo = mid + siz;
		else
			hi = mid - siz;
	}
	return 0;
}

typedef int (*Compare_f)(const char*, const char*);
typedef int (*Compare_context_f)(const char*, const char*, void*);

/*
 * return a pointer to the element matching
 * name in the (*comparf*)() sorted tab of num elements of
 * size siz where the first member of each
 * element is a char*
 *
 * 0 returned if name not found
 */

void*
strsearch(const void* tab, size_t num, size_t siz, Compare_f comparf, const char* name, void* context)
{
	register char*		lo = (char*)tab;
	register char*		hi = lo + (num - 1) * siz;
	register char*		mid;
	register int		v;

	while (lo <= hi)
	{
		mid = lo + (((hi - lo) / siz) / 2) * siz;
		if (!(v = context ? (*(Compare_context_f)comparf)(name, *((char**)mid), context) : (*(Compare_f)comparf)(name, *((char**)mid))))
			return (void*)mid;
		else if (v > 0)
			lo = mid + siz;
		else hi = mid - siz;
	}
	return 0;
}

/*
 * touch file access and modify times of file
 * if force>0 then file will be created if it doesn't exist
 * if force<0 then times are taken verbatim
 * times have one second granularity
 *
 *	(time_t)(-1)	retain old time
 *	0		use current time
 */

int
touch(const char* file, time_t atime, time_t mtime, int force)
{
	struct stat	st;
	struct timeval	tv[2];
	time_t		now;

	now = time(NiL);
	if (stat(file, &st))
	{
		if (!force || close(open(file, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY|O_cloexec, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)))
			return -1;
		st.st_mtime = st.st_atime = now;
	}
	if (force >= 0)
	{
		if (atime == (time_t)0)
			atime = now;
		else if (atime == (time_t)(-1))
			atime = st.st_atime;
		if (mtime == (time_t)0)
			mtime = now;
		else if (mtime == (time_t)(-1))
			mtime = st.st_mtime;
	}
	tv[0].tv_sec = atime;
	tv[1].tv_sec = mtime;
	tv[0].tv_usec = tv[1].tv_usec = 0;
	return utimes(file, tv);
}

/*
 * check if date is valid with no trailing junk
 */

int
isdate(const char* s)
{
	return 1;
}

#endif /*_PACKAGE_ast*/
