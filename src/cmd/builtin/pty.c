/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

static const char usage[] =
"[-?\n@(#)pty (AT&T Research) 2013-05-22\n]"
USAGE_LICENSE
"[+NAME?pty - create pseudo terminal and run command]"
"[+DESCRIPTION?\bpty\b creates a pseudo pty and then runs \bcommand\b "
    "with arguments given by \aarg\a and the standard input, standard "
    "output, and standard error connected to the pseudo terminal. By "
    "default, the \bpty\b creates a new session.]"
"[+?If \bcommand\b does not contain a \b/\b, the \bPATH\b variable will "
    "be used to locate the \bcommand\b.]"
"[+?Input to \bpty\b will be written to the standard input of this "
    "command. The standard output and standard error from the command will "
    "be written to the standard output of \bpty\b.]"
"[+?The \bpty\b commmand terminates when the command completes.]"
"[d:dialogue?Execute the dialogue on the standard input. A dialogue is a "
    "sequence of commands, one command per line. All \are\a patterns are "
    "extended regular expressions. The \are\a \b?1\b will print the subject "
    "string on the standard error and match the string; the \are\a \b?0\b "
    "will print the subject string on the standard error and not match the "
    "string. The \are\a \b?.\b matches EOF. The commands are:]"
    "{"
        "[\b#\b \acomment\a?comment line]"
        "[c \atext\a?write \atext\a to the master; C style escapes "
            "in text are converted, including \\E for ESC and \\cX for "
            "control-X]"
        "[d \amilliseconds\a?set the delay before each master write to "
            "\amilliseconds\a; the default is no delay]"
        "[i \are\a?read a line from the master; if it matches \are\a "
            "then execute lines until matching \be\b or \bf\b]"
        "[e [re]]?else [if match re]] then execute lines until matching "
            "\be\b or \bf\b]"
        "[f?end of \bi\b/\be\b block]"
        "[m \atext\a?write \atext\a to the standard error]"
	"[p \atext\a?peek input until \atext\a is found at the beginning "
	    "of a line; input is not consumed]"
        "[r [\are\a]]?read a line from the master [and it should match "
            "re]]]"
        "[s \amilliseconds\a?sleep for \amilliseconds\a]"
        "[t \amilliseconds\a?set the master read timout to "
            "\amilliseconds\a; the default is \b1000\b]"
        "[u \are\a?read lines from the master until one matches \are\a]"
        "[v \alevel\a?set the verbose trace \alevel\a, more output for "
            "higher levels, disabled for level 0]"
        "[w \atext\a?write \atext\a\\r\\n to the master; C style escapes "
            "in text are converted, including \\E for ESC and \\cX for "
            "control-X]"
        "[x [\acode\a]]"
            "?exit \bpty\b with exit code \b0\b [\acode\a]]]"
        "[I \are\a?ignore master lines matching \are\a]"
        "[L \alabel\a?prefix all diagnostics with \alabel\a:]"
        "[P \atext\a?delay each master write until the beginning of "
            "an unread input line exactly matches \atext\a]"
    "}"
"[D:debug?Set the debug trace \alevel\a, higher levels produce more "
    "output, disabled for level 0.]#[level]"
"[l:log?Log the master stdout and stderr to \afile\a.]:[file]"
"[m:messages?Redirect diagnostic message output to \afile\a.]:[file]"
"[s!:session?Create a separate session for the process started by "
    "\bpty\b.]"
"[t:timeout?Set the master read timeout to "
    "\amilliseconds\a.]#[milliseconds:=1000]"
"[T:tty?Pass \astty\a to the \bstty\b(1) command to initialize the "
    "pty.]:[stty]"
"[w:delay?Set the delay before each master write to "
    "\amilliseconds\a.]#[milliseconds:=0]"

"\n"
"\ncommand [arg ...]\n"
"\n"

"[+EXIT STATUS?If the command determined by \bcommand\b is run the exit "
    "status of \bpty\b is that of this command. Otherwise, the exit status "
    "is one of the following:]"
    "{"
        "[+127?The command is found but cannot be executed.]"
        "[+128?The command could not be found.]"
    "}"
"[+SEE ALSO?\bcommand\b(1), \bexec\b(1)]"
;

#include <cmd.h>
#include <error.h>
#include <fcntl.h>
#include <termios.h>
#include <proc.h>
#include <ctype.h>
#include <regex.h>
#include <vmalloc.h>
#include <ast_time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "FEATURE/pty"

#define MODE_666	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define MAXNAME		64

#if !_lib_openpty && !_lib__getpty && !defined(_pty_clone)
# if !_lib_grantpt || !_lib_unlock
#   if !_lib_ptsname
	static char *slavename(const char *name)
	{
		static char sname[MAXNAME];
		char *last;
		strncpy(sname,name,sizeof(sname));
		last = strrchr(sname,'/');
		last[1] = 't';
		return(sname);
	}
#   endif

    static char *master_name(char *name)
    {
	static char sname[MAXNAME];
	int n;
	if(!name)
	{
		strcpy(sname,_pty_first);
		return(sname);
	}
	n = strlen(_pty_first);
	if(name[n-1]=='9')
		name[n-1]='a';
	else if(name[n-1]=='f')
	{
		if(_pty_first[n-2]=='0' && name[n-2]=='9')
		{
			name[n-2]='0';
			if(name[n-3]=='9' || name[n-3]=='z')
				return(NULL);
			name[n-3]++;
		}
		if(_pty_first[n-2]=='p' && (name[n-2]=='z' || name[n-2]=='Z'))
		{
			if(name[n-2]=='z')
				name[n-2]=='P';
			else
				return(0);
		}
		else
			name[n-2]++;
		name[n-1]='0';
	}
	else
		name[n-1]++;
	return(name);
    }
#endif

#if !_lib_openpty
	static char *ptymopen(int *master)
	{
		char *slave=0;
#   if _lib__getpty
		return(_getpty(master,O_RDWR,MODE_666,0));
#   else
#	if defined(_pty_clone)
		*master = open(_pty_clone,O_RDWR|O_CREAT,MODE_666);
		if(*master>=0)
			slave = ptsname(*master);
#	else
		int fdm;
		char *name=0;
		while(name=master_name(name))
		{
			fdm = open(name,O_RDWR|O_CREAT,MODE_666);
			if(fdm >= 0)
			{
				*master = fdm;
#	   if _lib_ptsname
				slave = ptsname(fdm);
#	   else
				slave = slavename(name);
#	   endif
				break;
			}
		}
# 	endif
#   endif
		return(slave);
	}
# endif
#endif

static int
mkpty(int* master, int* slave)
{
	struct termios	tty;
	struct termios	tst;
	struct termios*	ttyp;
#ifdef TIOCGWINSZ
	struct winsize	win;
	struct winsize*	winp;
#endif
#if !_lib_openpty
	char*		sname;
#endif
	/*
	 * some systems hang hard during the handshake
	 * if you know why then please let us know
	 */

	alarm(4);
	if (tcgetattr(sffileno(sfstderr), &tty) >= 0)
		ttyp = &tty;
	else
	{
		ttyp = 0;
		error(-1, "unable to get standard error terminal attributes");
	}
#ifdef TIOCGWINSZ
	if (ioctl(sffileno(sfstderr), TIOCGWINSZ, &win) >= 0)
		winp = &win;
	else
	{
		winp = 0;
		error(-1, "unable to get standard error window size");
	}
#endif
#if _lib_openpty
	if (openpty(master, slave, NULL, ttyp, winp) < 0)
		return -1;
#else
#if _lib_grantpt && _lib_unlockpt
#if !_lib_posix_openpt
#ifndef _pty_clone
#define _pty_clone	"/dev/ptmx"
#endif
#define posix_openpt(m)	open(_pty_clone,m)
#endif
	if ((*master = posix_openpt(O_RDWR)) < 0)
		return -1;
	if (grantpt(*master) || unlockpt(*master) || !(sname = ptsname(*master)) || (*slave = open(sname, O_RDWR|O_cloexec)) < 0)
	{
		close(*master);
		return -1;
	}
#else
	if (!(sname = ptymopen(master)) || (*slave = open(sname, O_RDWR|O_cloexec)) < 0)
		return -1;
#endif
#ifdef I_PUSH
	if (tcgetattr(*slave, &tst) < 0 && (ioctl(*slave, I_PUSH, "ptem") < 0 || ioctl(*slave, I_PUSH, "ldterm") < 0))
	{
		close(*slave);
		close(*master);
		return -1;
	}
#endif
#endif
	if (ttyp && tcsetattr(*slave, TCSANOW, ttyp) < 0)
		error(ERROR_warn(0), "unable to set pty terminal attributes");
#ifdef TIOCSWINSZ
	if (winp && ioctl(*slave, TIOCSWINSZ, winp) < 0)
		error(ERROR_warn(0), "unable to set pty window size");
#endif
	fcntl(*master, F_SETFD, FD_CLOEXEC);
#if !O_cloexec
	fcntl(*slave, F_SETFD, FD_CLOEXEC);
#endif
	alarm(0);
	return 0;
}

static Proc_t*
runcmd(char** argv, int slave, int session)
{
	long	ops[4];

	if (session)
	{
		ops[0] = PROC_FD_CTTY(slave);
		ops[1] = 0;
	}
	else
	{
		ops[0] = PROC_FD_DUP(slave, 0, PROC_FD_CHILD);
		ops[1] = PROC_FD_DUP(slave, 1, PROC_FD_CHILD);
		ops[2] = PROC_FD_DUP(slave, 2, PROC_FD_CHILD);
		ops[3] = 0;
	}
	return procopen(argv[0], argv, NiL, ops, 0);
}

/*
 * default master dance
 */

static int
process(Sfio_t* mp, Sfio_t* lp, int delay, int timeout)
{
	int		i;
	int		n;
	int		t;
	ssize_t		r;
	char*		s;
	Sfio_t*		ip;
	Sfio_t*		sps[2];
	struct stat	dst;
	struct stat	fst;

	ip = sfstdin;
	if (!fstat(sffileno(ip), &dst) && !stat("/dev/null", &fst) && dst.st_dev == fst.st_dev && dst.st_ino == fst.st_ino)
		ip = 0;
	do
	{
		i = 0;
		t = timeout;
		if (mp)
			sps[i++] = mp;
		if (ip)
		{
			sps[i++] = ip;
			t = -1;
		}
		if (!i)
			break;
		if ((n = sfpoll(sps, i, t)) <= 0)
		{
			if (n < 0)
				error(ERROR_SYSTEM|2, "poll failed");
			break;
		}
		for (i = t = 0; i < n; i++)
		{
			if (!(sfvalue(sps[i]) & SF_READ))
				/*skip*/;
			else if (sps[i] == mp)
			{
				t++;
				if (!(s = (char*)sfreserve(mp, SF_UNBOUND, -1)))
				{
					sfclose(mp);
					mp = 0;
				}
				else if ((r = sfvalue(mp)) > 0 && (sfwrite(sfstdout, s, r) != r || sfsync(sfstdout)))
				{
					error(ERROR_SYSTEM|2, "output write failed");
					goto done;
				}
			}
			else
			{
				t++;
				if (!(s = sfgetr(ip, '\n', 1)))
					ip = 0;
				else if (sfputr(mp, s, '\r') < 0 || sfsync(mp))
				{
					error(ERROR_SYSTEM|2, "write failed");
					goto done;
				}
			}
		}
	} while (t);
 done:
	if (mp)
		sfclose(mp);
	return error_info.errors != 0;
}

/*
 * return 1 is extended re pattern matches text
 */

static int
match(char* pattern, char* text, int must)
{
	regex_t*	re;
	int		code;
	char		buf[64];

	if (!pattern[0])
		return 1;
	if (pattern[0] == '?' && pattern[1] && !pattern[2])
	{
		switch (pattern[1])
		{
		case '0':
		case '1':
			if (text)
				error(2, "got \"%s\"", fmtesq(text, "\""));
			else
				error(2, "got EOF");
			return pattern[1] == '1';
		case '.':
			if (!text)
				return 1;
			if (must)
				error(2, "expected EOF, got \"%s\"", fmtesq(text, "\""));
			return 0;
		}
	}
	if (!text)
	{
		if (must)
			error(2, "expected \"%s\", got EOF", pattern);
		return 0;
	}
	if (!(re = regcache(pattern, REG_EXTENDED, &code)))
	{
		regerror(code, re, buf, sizeof(buf));
		error(2, "%s: %s", pattern, buf);
		return 0;
	}
	if (regexec(re, text, 0, NiL, 0))
	{
		if (must)
			error(2, "expected \"%s\", got \"%s\"", pattern, fmtesq(text, "\""));
		return 0;
	}
	return 1;
}

typedef struct Master_s
{
	Vmalloc_t*	vm;		/* allocation region			*/
	char*		ignore;		/* ignore master lines matching this re	*/
	char*		peek;		/* peek buffer pointer			*/
	char*		cur;		/* current line				*/
	char*		nxt;		/* next line				*/
	char*		end;		/* end of lines				*/
	char*		max;		/* end of buf				*/
	char*		buf;		/* current buffer			*/
	char*		prompt;		/* peek prompt				*/
	int		cursor;		/* cursor in buf, 0 if fresh line	*/
	int		line;		/* prompt line number			*/
	int		restore;	/* previous line save char		*/
} Master_t;

/*
 * read one line from the master
 */

#define MASTER_EOF	(-1)
#define MASTER_TIMEOUT	(-2)

static char*
masterline(Sfio_t* mp, Sfio_t* lp, char* prompt, int must, int timeout, Master_t* bp)
{
	char*		r;
	char*		s;
	char*		t;
	ssize_t		n;
	ssize_t		a;
	size_t		promptlen;
	ptrdiff_t	d;
	char		promptbuf[64];

	if (prompt)
		promptlen = sfsprintf(promptbuf, sizeof(promptbuf), prompt, ++bp->line);
 again:
	if (prompt)
	{
		if (bp->cur < bp->end && bp->restore >= 0)
			*bp->cur = bp->restore;
		if (strneq(bp->cur, promptbuf, promptlen))
			r = bp->cur;
		else
			r = 0;
		if (bp->cur < bp->end && bp->restore >= 0)
			*bp->cur = 0;
		if (r)
		{
			error(-1, "p \"%s\"", fmtnesq(promptbuf, "\"", promptlen));
			return r;
		}
		if (r = bp->nxt)
		{
			if (strneq(r, promptbuf, promptlen))
			{
				error(-1, "p \"%s\"", fmtnesq(promptbuf, "\"", promptlen));
				return r;
			}
			while (r = memchr(r, '\n', bp->end - r))
			{
				if (strneq(r, promptbuf, promptlen))
				{
					error(-1, "p \"%s\"", fmtnesq(promptbuf, "\"", promptlen));
					return r;
				}
				r++;
			}
		}
		*bp->cur = 0;
	}
	else if (bp->nxt)
	{
		if (bp->restore >= 0)
			*bp->cur = bp->restore;
		r = bp->cur;
		bp->restore = *bp->nxt;
		*bp->nxt = 0;
		if (bp->nxt >= bp->end)
		{
			bp->cur = bp->end = bp->buf;
			bp->nxt = 0;
		}
		else
		{
			bp->cur = bp->nxt;
			if (bp->nxt = memchr(bp->nxt + 1, '\n', bp->end - bp->nxt - 1))
				bp->nxt++;
		}
		goto done;
	}
	if ((n = sfpoll(&mp, 1, timeout)) <= 0 || !((int)sfvalue(mp) & SF_READ))
	{
		if (n < 0)
		{
			if (must)
				error(ERROR_SYSTEM|2, "poll failed");
			else
				error(-1, "r poll failed");
		}
		else if (bp->cur < bp->end)
		{
			if (bp->restore >= 0)
			{
				*bp->cur = bp->restore;
				bp->restore = -1;
			}
			r = bp->cur;
			*bp->end = 0;
			bp->nxt = 0;
			if (prompt && strneq(r, promptbuf, promptlen))
			{
				error(-1, "p \"%s\"", fmtnesq(promptbuf, "\"", promptlen));
				return r;
			}
			bp->cur = bp->end = bp->buf;
			goto done;
		}
		else if (must >= 0)
			error(2, "read timeout");
		else
		{
			errno = 0;
			error(-1, "r EOF");
		}
		return 0;
	}
	if (!(s = sfreserve(mp, SF_UNBOUND, -1)))
	{
		if (!prompt)
		{
			if (bp->cur < bp->end)
			{
				if (bp->restore >= 0)
				{
					*bp->cur = bp->restore;
					bp->restore = -1;
				}
				r = bp->cur;
				*bp->end = 0;
				bp->cur = bp->end = bp->buf;
				bp->nxt = 0;
				goto done;
			}
			else
			{
				errno = 0;
				error(-1, "r EOF");
			}
		}
		return 0;
	}
	n = sfvalue(mp);
	error(-2, "b \"%s\"", fmtnesq(s, "\"", n));
	if ((bp->max - bp->end) < n)
	{
		a = roundof(bp->max - bp->buf + n, SF_BUFSIZE);
		r = bp->buf;
		if (!(bp->buf = vmnewof(bp->vm, bp->buf, char, a, 0)))
		{
			error(ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		bp->max = bp->buf + a;
		if (bp->buf != r)
		{
			d = bp->buf - r;
			bp->cur += d;
			bp->end += d;
		}
	}
	memcpy(bp->end, s, n);
	bp->end += n;
	if ((r = bp->cur) > bp->buf && bp->restore >= 0)
		*r = bp->restore;
	if (bp->cur = memchr(bp->cur, '\n', bp->end - bp->cur))
	{
		bp->restore = *++bp->cur;
		*bp->cur = 0;
		if (bp->cur >= bp->end)
		{
			bp->cur = bp->end = bp->buf;
			bp->nxt = 0;
		}
		else if (bp->nxt = memchr(bp->cur + 1, '\n', bp->end - bp->cur - 1))
			bp->nxt++;
		if (prompt)
			goto again;
	}
	else
	{
		bp->restore = -1;
		bp->cur = r;
		bp->nxt = 0;
		must = 0;
		goto again;
	}
 done:
	error(-3, "Q \"%s\"", fmtesq(r, "\""));
	s = r;
	if (bp->cursor)
	{
		r -= bp->cursor;
		bp->cursor = 0;
	}
	for (t = 0, n = 0; *s; s++)
		if (*s == '\n')
		{
			if (t)
			{
				*t++ = '\n';
				*t = 0;
				t = 0;
				n = 0;
			}
		}
		else if (*s == '\r' && *(s + 1) != '\n')
		{
			if (t = strchr(s + 1, '\r'))
				n += t - s;
			else
				n += strlen(s);
			t = r;
		}
		else if (*s == '\a')
		{
			if (!t)
				t = s;
			*t = ' ';
			n++;
		}
		else if (*s == '\b')
		{
			if (!t)
				t = s;
			if (t > r)
				t--;
			else
				n++;
		}
		else if (t)
			*t++ = *s;
	if (t)
		error(-3, "R \"%s\"", fmtesq(r, "\""));
	if (n)
		*(r + strlen(r) - n) = 0;
	error(-1, "r \"%s\"", fmtesq(r, "\""));
	if (lp)
		sfputr(lp, fmtesq(r, "\""), '\n');
	if (t)
		bp->cursor = t - r;
	if (bp->ignore && match(bp->ignore, r, 0))
		goto again;
	return r;
}

/*
 * execute dialogue script on stdin
 */

#define NESTING	64

#define ELSE	0x01
#define IF	0x02
#define KEPT	0x04
#define SKIP	0x08

struct Cond_s;
typedef struct Cond_s Cond_t;

struct Cond_s
{
	Cond_t*	next;
	Cond_t*	prev;
	char*	text;
	int	flags;
};

static int
dialogue(Sfio_t* mp, Sfio_t* lp, int delay, int timeout)
{
	int		op;
	int		line;
	int		n;
	char*		s;
	char*		m;
	char*		e;
	char*		id;
	Vmalloc_t*	vm;
	Cond_t*		cond;
	Master_t*	master;

	int		status = 0;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)) ||
	    !(cond = vmnewof(vm, 0, Cond_t, 1, 0)) ||
	    !(master = vmnewof(vm, 0, Master_t, 1, 0)) ||
	    !(master->buf = vmnewof(vm, 0, char, 2 * SF_BUFSIZE, 0)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	master->vm = vm;
	master->cur = master->end = master->buf;
	master->max = master->buf + 2 * SF_BUFSIZE - 1;
	master->restore = -1;
	errno = 0;
	id = error_info.id;
	error_info.id = 0;
	line = error_info.line;
	error_info.line = 0;
	while (s = sfgetr(sfstdin, '\n', 1))
	{
		error_info.line++;
		while (isspace(*s))
			s++;
		if ((op = *s++) && isspace(*s))
			s++;
		switch (op)
		{
		case 0:
		case '#':
			break;
		case 'c':
		case 'w':
			if (cond->flags & SKIP)
				continue;
			if (master->prompt && !masterline(mp, lp, master->prompt, 0, timeout, master))
				goto done;
			if (delay)
				usleep((unsigned long)delay * 1000);
			if (op == 'w')
				error(-1, "w \"%s\\r\"", s);
			else
				error(-1, "w \"%s\"", s);
			if ((n = stresc(s)) >= 0)
				s[n] = 0;
			if (sfputr(mp, s, op == 'w' ? '\n' : -1) < 0 || sfsync(mp))
			{
				error(ERROR_SYSTEM|2, "write failed");
				goto done;
			}
			if (delay)
				usleep((unsigned long)delay * 1000);
			break;
		case 'd':
			delay = (int)strtol(s, &e, 0);
			if (*e)
				error(2, "%s: invalid delay -- milliseconds expected", s);
			break;
		case 'i':
			if (!cond->next && !(cond->next = vmnewof(vm, 0, Cond_t, 1, 0)))
			{
				error(ERROR_SYSTEM|2, "out of space");
				goto done;
			}
			cond = cond->next;
			cond->flags = IF;
			if ((cond->prev->flags & SKIP) && !(cond->text = 0) || !(cond->text = masterline(mp, lp, 0, 0, timeout, master)))
				cond->flags |= KEPT|SKIP;
			else if (match(s, cond->text, 0))
				cond->flags |= KEPT;
			else
				cond->flags |= SKIP;
			break;
		case 'e':
			if (!(cond->flags & IF))
			{
				error(2, "no matching i for e");
				goto done;
			}
			if (!*s)
			{
				if (cond->flags & ELSE)
				{
					error(2, "i block already has a default e");
					goto done;
				}
				cond->flags |= ELSE;
				if (cond->flags & KEPT)
					cond->flags |= SKIP;
				else
				{
					cond->flags |= KEPT;
					cond->flags &= ~SKIP;
				}
			}
			else if ((cond->flags & KEPT) || !match(s, cond->text, 0))
				cond->flags |= SKIP;
			else
				cond->flags |= KEPT;
			break;
		case 'f':
			if (!(cond->flags & IF))
			{
				error(2, "no matching i for f");
				goto done;
			}
			cond = cond->prev;
			break;
		case 'm':
			if (cond->flags & SKIP)
				continue;
			if (sfputr(sfstderr, s, '\n') < 0 || sfsync(sfstderr))
			{
				error(ERROR_SYSTEM|2, "standard error write failed");
				goto done;
			}
			break;
		case 'p':
			if (cond->flags & SKIP)
				continue;
			if (!(m = masterline(mp, lp, s, 1, timeout, master)))
				goto done;
			break;
		case 'r':
			if (cond->flags & SKIP)
				continue;
			if (!(m = masterline(mp, lp, 0, s[0] == '?' && s[1] == '.' ? -1 : 1, timeout, master)))
				goto done;
			match(s, m, 1);
			break;
		case 's':
			n = (int)strtol(s, &e, 0);
			if (*e)
				error(2, "%s: invalid delay -- milliseconds expected", s);
			if (n)
				usleep((unsigned long)n * 1000);
			break;
		case 't':
			timeout = (int)strtol(s, &e, 0);
			if (*e)
				error(2, "%s: invalid timeout -- milliseconds expected", s);
			break;
		case 'u':
			if (cond->flags & SKIP)
				continue;
			do
			{
				if (!(m = masterline(mp, lp, 0, -1, timeout, master)))
				{
					match(s, m, 1);
					goto done;
				}
			} while (!match(s, m, 0));
			break;
		case 'v':
			error_info.trace = -(int)strtol(s, &e, 0);
			if (*e)
				error(2, "%s: invalid verbose level -- number expected", s);
			break;
		case 'x':
			status = (int)strtol(s, &e, 0);
			if (*e)
				error(2, "%s: invalid exit code", s);
			break;
		case 'I':
			if (master->ignore)
			{
				vmfree(vm, master->ignore);
				master->ignore = 0;
			}
			if (*s && !(master->ignore = vmstrdup(vm, s)))
			{
				error(ERROR_SYSTEM|2, "out of space");
				goto done;
			}
			break;
		case 'L':
			if (error_info.id)
			{
				vmfree(vm, error_info.id);
				error_info.id = 0;
			}
			if (*s && !(error_info.id = vmstrdup(vm, s)))
			{
				error(ERROR_SYSTEM|2, "out of space");
				goto done;
			}
			break;
		case 'P':
			if (master->prompt)
			{
				vmfree(vm, master->prompt);
				master->prompt = 0;
			}
			if (*s && !(master->prompt = vmstrdup(vm, s)))
			{
				error(ERROR_SYSTEM|2, "out of space");
				goto done;
			}
			break;
		default:
			if (cond->flags & SKIP)
				continue;
			error(2, "'%c': unknown op", op);
			goto done;
		}
	}
	if (cond->prev)
		error(2, "missing 1 or more f statements");
 done:
	if (mp)
		sfclose(mp);
	error_info.id = id;
	error_info.line = line;
	if (vm)
		vmclose(vm);
	return status ? status : error_info.errors != 0;
}

typedef struct Argv_s
{
	char**	argv;
	char*	args;
	int	argc;
} Argv_t;

int
b_pty(int argc, char** argv, Shbltin_t* context)
{
	int		master;
	int		slave;
	int		fd;
	int		drop;
	int		n;
	char*		s;
	Proc_t*		proc;
	Sfio_t*		mp;
	Sfio_t*		lp;
	Argv_t*		ap;
	char		buf[64];

	int		delay = 0;
	char*		log = 0;
	char*		messages = 0;
	char*		stty = 0;
	int		session = 1;
	int		timeout = 1000;
	int		(*fun)(Sfio_t*,Sfio_t*,int,int) = process;

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			if (opt_info.num)
				fun = dialogue;
			continue;
		case 'D':
			error_info.trace = -(int)opt_info.num;
			continue;
		case 'l':
			log = opt_info.arg;
		case 'm':
			messages = opt_info.arg;
			continue;
		case 's':
			session = !!opt_info.num;
			continue;
		case 't':
			timeout = (int)opt_info.num;
			continue;
		case 'T':
			stty = opt_info.arg;
			continue;
		case 'w':
			delay = (int)opt_info.num;
			continue;
		case ':':
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (!argv[0])
		error(ERROR_exit(1), "command must be specified");
	if (mkpty(&master, &slave) < 0)
		error(ERROR_system(1), "unable to create pty");
	if (!(mp = sfnew(NiL, 0, SF_UNBOUND, master, SF_READ|SF_WRITE)))
		error(ERROR_system(1), "cannot open master stream");
	if (stty)
	{
		n = 2;
		for (s = stty; *s; s++)
			if (isspace(*s))
				n++;
		if (!(ap = newof(0, Argv_t, 1, (n + 2) * sizeof(char*) + (s - stty + 1))))
			error(ERROR_system(1), "out of space");
		ap->argc = n + 1;
		ap->argv = (char**)(ap + 1);
		ap->args = (char*)(ap->argv + n + 2);
		strcpy(ap->args, stty);
		ap->argv[0] = "stty";
		sfsprintf(ap->argv[1] = buf, sizeof(buf), "--fd=%d", slave);
		ap->argv[2] = s = ap->args;
		for (n = 2; *s; s++)
			if (isspace(*s))
			{
				*s = 0;
				ap->argv[++n] = s + 1;
			}
		ap->argv[n + 1] = 0;
		b_stty(ap->argc, ap->argv, 0);
	}
	if (!log)
		lp = 0;
	else if (!(lp = sfopen(NiL, log, "w")))
		error(ERROR_system(1), "%s: cannot write", log);
	if (!(proc = runcmd(argv, slave, session)))
		error(ERROR_system(1), "unable run %s", argv[0]);
	close(slave);
	if (messages)
	{
		drop = 1;
		if (strneq(messages, "/dev/fd/", 8))
			fd = atoi(messages + 8);
		else if (streq(messages, "/dev/stdout"))
			fd = 1;
		else if ((fd = open(messages, O_CREAT|O_WRONLY, MODE_666)) >= 0)
			drop = 0;
		else
			error(ERROR_system(1), "%s: cannot redirect messages", messages);
		close(2);
		dup(fd);
		if (drop)
			close(fd);
	}
	slave = (*fun)(mp, lp, delay, timeout);
	master = procclose(proc);
	if (lp && sfclose(lp))
		error(ERROR_system(1), "%s: write error", log);
	return slave ? slave : master;
}
