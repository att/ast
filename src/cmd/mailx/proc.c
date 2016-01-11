/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2009 The Regents of the University of California an*
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
 * Process control.
 */

#include "mailx.h"

#define READ 0
#define WRITE 1

static void
register_file(FILE* fp, int pid)
{
	struct file*	fpp;

	if (!(fpp = (struct file*)malloc(sizeof *fpp)))
		note(PANIC, "Out of space");
	fpp->fp = fp;
	fpp->pid = pid;
	fpp->link = state.files;
	state.files = fpp;
}

/*
 * return std stream for file,mode
 * 0 if no match
 */

FILE*
filestd(char* file, char* mode)
{
	if (*mode == 'r') {
		if (streq(file, "-") || streq(file, "/dev/stdin") || streq(file, "/dev/fd/0"))
			return stdin;
	}
	else if (*mode == 'w') {
		if (streq(file, "-") || streq(file, "/dev/stdout") || streq(file, "/dev/fd/1"))
			return stdout;
		else if (streq(file, "/dev/stderr") || streq(file, "/dev/fd/2"))
			return stderr;
	}
	return 0;
}

/*
 * fopen() with some extra mode prefixes:
 *
 *	E	enable error messages
 *	I	don't register_file (fclose to close)
 *	M	umask(~MAILMODE)
 *	R	file must be S_ISREG()
 *	X	expand() file name before open
 *
 * Open fd is set close-on-exec.
 */

FILE*
fileopen(char* file, char* mode)
{
	FILE*		fp;
	int		n;
	int		ignore = 0;
	int		mask = 0;
	int		regular = 0;
	int		verbose = 0;

	for (;; mode++) {
		switch (*mode) {
		case 'E':
			verbose = 1;
			continue;
		case 'I':
			ignore = 1;
			continue;
		case 'M':
			mask = 1;
			continue;
		case 'R':
			regular = 1;
			continue;
		case 'X':
			if (!(file = expand(file, 1)))
				return 0;
			continue;
		}
		break;
	}
	if (fp = filestd(file, mode))
		return fp;
	if (mask)
		n = umask(~MAILMODE);
	fp = fopen(file, mode);
	if (mask) {
		umask(n);
		if (state.readonly && streq(mode, "w"))
			chmod(file, S_IRUSR);
	}
	if (fp) {
		if (fstat(fileno(fp), &state.openstat)) {
			fclose(fp);
			if (verbose)
				note(SYSTEM, "%s", file);
			return 0;
		}
		if (S_ISDIR(state.openstat.st_mode)) {
			fclose(fp);
			errno = EISDIR;
			if (verbose)
				note(SYSTEM, "%s", file);
			return 0;
		}
		if (regular && !S_ISREG(state.openstat.st_mode)) {
			fclose(fp);
			errno = EFTYPE;
			if (verbose)
				note(SYSTEM, "%s", file);
			return 0;
		}
		if (!ignore)
			register_file(fp, 0);
		fcntl(fileno(fp), F_SETFD, 1);
#if 0 && MORE_DISCIPLINE
		sfsetbuf(fp, (void*)fp, SF_UNBOUND);
#endif
	}
	else if (verbose)
		note(SYSTEM, "%s", file);
	return fp;
}

/*
 * fdopen() that calls register_file()
 */

FILE*
filefd(int fd, char* mode)
{
	FILE *fp;

	if (fp = fdopen(fd, mode)) {
		register_file(fp, 0);
		fcntl(fileno(fp), F_SETFD, 1);
	}
	return fp;
}

/*
 * Generate temp file name.
 * If fd>0 then create it and return file pointer.
 * If fd>0 and buf==0 then remove after create.
 */

FILE*
filetemp(char* buf, int size, int type, int fd)
{
	register char*	s;
	register char*	b;
	register char*	e;
	FILE*		fp = 0;

	if (!(b = buf)) {
		if (fd <= 0)
			return 0;
		b = state.path.path;
		size = sizeof(state.path.path);
	}
	e = b + size - 1;
	s = strncopy(b, state.tmp.dir, e - b);
	s = strncopy(s, "Mail", e - s);
	if (s < e)
		*s++ = type;
	strncopy(s, "XXXXXX", e - s);
	if (fd) {
		fd = mkstemp(b);
		if (!buf && *b)
			remove(b);
		if (fd < 0 || !(fp = filefd(fd, "r+"))) {
			if (fd >= 0)
				close(fd);
			note(FATAL|SYSTEM|ERROR|IDENTIFY, "\"%s\": temporary file error", b);
		}
	}
	else
		mktemp(b);
	if (!*b)
		note(FATAL|SYSTEM|ERROR|IDENTIFY, "\"%s\": temporary file error", b);
	return fp;
}

/*
 * Call this after fileopen() or filefd().
 */

int
fileclose(FILE* fp)
{
	int		r;
	struct file*	p;
	struct file**	pp;

	if (fp == 0 || fp == stdin)
		return 0;
	if (fp == stdout || fp == stderr)
		return fflush(fp);
	r = 0;
	for (pp = &state.files;; pp = &p->link) {
		if (!(p = *pp)) {
			fclose(fp);
			return 0;
		}
		if (p->fp == fp) {
			r = p->pid;
			*pp = p->link;
			free(p);
			break;
		}
	}
	if (r) {
		holdsigs();
		fclose(fp);
		signal(SIGPIPE, SIG_IGN);
		r = wait_command(r);
		signal(SIGPIPE, SIG_DFL);
		relsesigs();
	}
	else
		r = fclose(fp);
	return r;
}

/*
 * fileclose() all registered files.
 */

void
fileclear(void)
{
	while (state.files)
		fileclose(state.files->fp);
}

/*
 * Copy n chars from from file ip to file op.
 * If in is specified then input error messages enabled.
 * If on is specified then output error messages enabled.
 * If lines!=0 then it will point to the copied line count.
 * If chars!=0 then it will point to the copied char count.
 * If n==0 then all chars copied.
 * If n>0 then exactly that many chars are copied.
 *
 * 0 returned on success.
 */

int
filecopy(const char* in, FILE* ip, const char* on, FILE* op, FILE* ap, register off_t n, register off_t* lines, off_t* chars, unsigned long flags)
{
	register off_t	c;
	register char*	s;
	int		r = 0;
	off_t		lc = 0;
	off_t		cc = 0;
	char		buf[LINESIZE + 1];

	buf[sizeof(buf) - 1] = 0;
	while (n >= 0) {
		if ((c = fread(buf, 1, sizeof(buf) - 1, ip)) <= 0) {
			if (c < 0) {
				r = -1;
				if (in) {
					note(SYSTEM, "%s", in);
					in = 0;
				}
			}
			break;
		}
		if (n) {
			if (n > c)
				n -= c;
			else {
				c = n;
				n = -1;
			}
		}
		if (fwrite(buf, 1, c, op) != c) {
			r = -1;
			if (on) {
				note(SYSTEM, "%s", on);
				on = 0;
			}
			break;
		}
		cc += c;
		if (ap)
			fwrite(buf, 1, c, ap);
		if (lines)
			for (s = buf; s = strchr(s, '\n'); lc++, s++);
	}
	if (flags & GNL)
		putc('\n', op);
	if (fflush(op)) {
		r = -1;
		if (on)
			note(SYSTEM, "%s", on);
	}
	if (n > 0) {
		r = -1;
		if (in)
			note(SYSTEM, "%s", in);
	}
	if (lines)
		*lines = lc;
	if (chars)
		*chars = cc;
	return r;
}

/*
 * Respond to a broken pipe signal --
 * probably caused by quitting more.
 */
static void
sigpipe(int sig)
{
	longjmp(state.jump.sigpipe, sig);
}

/*
 * popen() via register_file()
 */
FILE*
pipeopen(char* cmd, char* mode)
{
	int	myside;
	int	hisside;
	int	fd0;
	int	fd1;
	int	pid;
	FILE*	fp;
	int	p[2];
	int	background = 0;
	int	jump = 0;

	if (pipe(p) < 0)
		goto bad;
	fcntl(p[READ], F_SETFD, 1);
	fcntl(p[WRITE], F_SETFD, 1);
	for (;; mode++) {
		switch (*mode) {
		case 'J':
			jump = 1;
			continue;
		case 'N':
			background = 1;
			continue;
		}
		break;
	}
	if (*mode == 'r') {
		myside = p[READ];
		fd0 = -1;
		hisside = fd1 = p[WRITE];
	}
	else {
		myside = p[WRITE];
		hisside = fd0 = p[READ];
		fd1 = -1;
	}
	if ((pid = start_command(state.var.shell, 0, fd0, fd1, "-c", cmd, NiL)) < 0) {
		close(p[READ]);
		close(p[WRITE]);
		goto bad;
	}
	close(hisside);
	if (!(fp = fdopen(myside, mode)))
		goto bad;
	register_file(fp, background ? 0 : pid);
	if (jump && !background)
		signal(SIGPIPE, sigpipe);
	return fp;
 bad:
	note(SYSTEM, "\"%s\"", cmd);
	return 0;
}

/*
 * Run a command without a shell, with optional arguments and splicing
 * of stdin and stdout.  The command name can be a sequence of words.
 * Signals must be handled by the caller.
 * critical will mask interesting signals in the new process.
 * SIGINT is enabled if critical==0.
 */
int
run_command(char* cmd, int critical, int infd, int outfd, char* a0, char* a1, char* a2)
{
	int	pid;
	int	code;

	if ((pid = start_command(cmd, critical, infd, outfd, a0, a1, a2)) < 0)
		return -1;
	if (code = wait_command(pid)) {
		note(SYSTEM, "Fatal exit code %d from %s", code, cmd);
		return -1;
	}
	return 0;
}

extern pid_t	spawnvp(const char*, char* const*);	/* ast obsolete, but so is mailx, sort of */

int
start_command(char* cmd, int critical, int infd, int outfd, char* a0, char* a1, char* a2)
{
	int		pid;
	int		savein;
	int		saveout;
	char**		args;
	char**		p;

	struct argvec	vec;

	if (!a0 && a1)
		args = (char**)a1;
	else {
		initargs(&vec);
		getargs(&vec, cmd);
		if (a0) {
			addarg(&vec, a0);
			if (a1) {
				addarg(&vec, a1);
				if (a2)
					addarg(&vec, a2);
			}
		}
		endargs(&vec);
		cmd = vec.argv[0];
		args = vec.argv;
	}
	if (infd > READ) {
		if ((savein = dup(READ)) < 0) {
			note(SYSTEM, "%s: Cannot save standard input", cmd);
			return -1;
		}
		fcntl(savein, F_SETFD, 1);
		fcntl(infd, F_SETFD, 1);
		close(READ);
		if (dup(infd) != READ) {
			note(SYSTEM, "%s: Cannot redirect standard input", cmd);
			dup(savein);
			close(savein);
			return -1;
		}
	}
	else infd = -1;
	if (outfd > WRITE) {
		if ((saveout = dup(WRITE)) < 0) {
			note(SYSTEM, "%s: Cannot save standard output", cmd);
			return -1;
		}
		fcntl(saveout, F_SETFD, 1);
		fcntl(outfd, F_SETFD, 1);
		close(WRITE);
		if (dup(outfd) != WRITE) {
			note(SYSTEM, "%s: Cannot redirect standard input", cmd);
			dup(savein);
			close(savein);
			dup(saveout);
			close(saveout);
			return -1;
		}
	}
	else
		outfd = -1;
	if (state.var.debug) {
		note(DEBUG|PROMPT, "spawn:");
		for (p = args; *p; p++)
			printf(" \"%s\"", *p);
		printf("\n");
	}
	if (critical)
		sigcritical(critical);
	if ((pid = spawnvp(cmd, args)) == -1)
		note(SYSTEM, "%s", cmd);
	if (critical)
		sigcritical(0);
	if (infd > READ) {
		close(READ);
		if (dup(savein) != READ) {
			note(SYSTEM, "%s: Cannot restore standard input", cmd);
			return -1;
		}
		close(savein);
		fcntl(READ, F_SETFD, 0);
	}
	if (outfd > WRITE) {
		close(WRITE);
		if (dup(saveout) != WRITE) {
			note(SYSTEM, "%s: Cannot restore standard output", cmd);
			return -1;
		}
		close(saveout);
		fcntl(WRITE, F_SETFD, 0);
	}
	return pid;
}

static struct child*
findchild(int pid)
{
	register struct child**	cpp;

	for (cpp = &state.children; *cpp && (*cpp)->pid != pid;
	     cpp = &(*cpp)->link)
			;
	if (*cpp || (*cpp = (struct child*)malloc(sizeof(struct child)))) {
		(*cpp)->pid = pid;
		(*cpp)->done = (*cpp)->free = 0;
		(*cpp)->link = 0;
	}
	return *cpp;
}

static void
delchild(register struct child* cp)
{
	register struct child**	cpp;

	for (cpp = &state.children; *cpp != cp; cpp = &(*cpp)->link) ;
	*cpp = cp->link;
	free(cp);
}

/*
 * Wait for a specific child to die.
 */
int
wait_command(int pid)
{
	register struct child*	cp = findchild(pid);
	int			status = -1;

	holdsigs();
	while (waitpid(pid, &status, 0) == -1 && errno == EINTR);
	relsesigs();
	delchild(cp);
	return status;
}

/*
 * Mark a command as don't care.
 */
void
free_command(int pid)
{
	register struct child*	cp = findchild(pid);

	if (cp->done)
		delchild(cp);
	else
		cp->free = 1;
}
