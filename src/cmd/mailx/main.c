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
 * Startup -- interface with user.
 */

#include "mailx.h"

#if _PACKAGE_ast

#include "stamp.h"

static const char usage[] =
"[-?" STAMP "]"
USAGE_LICENSE
"[+NAME?mailx - send and receive mail]"
"[+DESCRIPTION?\bmailx\b is a mail processing command. The options place"
"	\bmailx\b in either \asend\a or \areceive\a mode. Send mode composes"
"	and sends messages from text on the standard input. Receive mode"
"	provides both an interactive command language and non-interactive"
"	command line actions. Receive mode, on startup, reads commands and"
"	variable settings from the file \b$HOME/.mailrc\b, if it exists.]"
"[+?\bmailx\b provides commands for saving, deleting and responding to"
"	messages. Message composition supports editing, reviewing and"
"	other modifications as the message is entered.]"
"[+?Incoming mail is stored in one or more unspecified locations for each"
"	user, collectively called the system mailbox for that user. When"
"	\bmailx\b is invoked in \areceive\a mode, the system mailbox is"
"	searched by default.]"
"[+?For additional help run \bmailx\b with no options or operands,"
"	and then enter the \bhelp\b command at the interactive prompt.]"
"[A:articles?Treat mail folders as netnews article.]"
"[F:followup?Automatically include the previous message (followup) text"
"	when composing a reply.]"
"[H:headers?List all headers and exit.]"
"[I:interactive?Force interactive receive mode.]"
"[N!:list-headers?List a screen of headers on receive mode startup.]"
"[P:pipe?Coprocess receive mode from a pipe.]"
"[Q:query?List the status character and sender address for the \acount\a"
"	most recent messages, one line per message, and exit. See \b--status\b"
"	for message status characters details.]:[count]"
"[S:status?List the status character and sender address for all messages,"
"	one line per message, and exit. The message status characters are:]{"
"		[M?To be saved in \bmbox\b and marked for delete on exit.]"
"		[N?New message.]"
"		[P?Preserved and will not be deleted.]"
"		[R?Already read.]"
"		[U?Unread message from previous session.]"
"		[X?Possible spam.]"
"		[\b*\b?Saved to a folder and marked for delete on exit.]"
"}"
"[T:oldnews?Read/deleted netnews article names are appended to \afile\a]:[file]"
"[b:bcc?Prompt for the blind carbon copy recipient list when composing"
"	messages.]"
"[c:cc?Prompt for the carbon copy recipient list when composing messages.]"
"[d:debug?Enable implementation specific debug output.]"
"[e:check?Silently exit 0 if there is mail, 1 otherwise.]"
"[f:folder?Mail is read from \afolder\a instead of the default"
"	user mailbox.]:?[folder:=$HOME/mbox]"
"[i:ignore-interrupts?Ignore interrupts.]"
"[n!:master?Read commands from \b/etc/mailx.rc\b on receive mode startup.]"
"[o:set?Set \aname\a=\avalue\a options. The interactive mail command"
"	\bhelp set all\b lists details for each option.]:[name=value]"
"[r:address?Set the reply to header address to \aaddress\a.]:[address]"
"[s:subject?The non-interactive send mode subject text.]:[text]"
"[t:sendheaders?Check for headers in send mode message text.]"
"[u:user?Pretend to be this \buser\b in send mode. For debugging.]:[user]"
"[v:verbose?Enable implementation specific send mode verbose trace.]"
"\n"
"\n[ address ... ]\n"
"\n"
"[+SEE ALSO?\b/bin/mail\b(1), \bMail\b(1)]"
;

#undef	optarg
#define optarg		opt_info.arg
#undef	optnum
#define optnum		opt_info.num
#undef	optind
#define optind		opt_info.index

#undef	getopt
#define getopt(c,v,u)	optget(v,u)

#else

static const char usage[] = "AFHINPQ:ST:b:c:defino:r:s:tu:v";

#endif

/*
 * Interrupt printing of the headers.
 */
static void
hdrstop(int sig)
{
	note(0, "\nInterrupt");
	longjmp(state.jump.header, sig);
}

/*
 * Set command line options and append to
 * op list for resetopt() after the rc's.
 */
static struct list*
setopt(register struct list* op, char* s, char* v)
{
	int		n;
	struct argvec	vec;

	n = strlen(s) + 1;
	if (v)
		n += strlen(v) + 1;
	if (!(op->next = newof(0, struct list, 1, n)))
		note(PANIC, "Out of space");
	op = op->next;
	s = strcopy(op->name, s);
	if (v) {
		*s++ = '=';
		strcpy(s, v);
	}
	state.onstack++;
	initargs(&vec);
	getargs(&vec, op->name);
	if (endargs(&vec) > 0) {
		state.cmdline = 1;
		set(vec.argv);
		state.cmdline = 0;
	}
	sreset();
	state.onstack--;
	return op;
}

/*
 * Reset the setopt() options after the rc's.
 */
static void
resetopt(register struct list* op)
{
	register struct list*	np;
	struct argvec		vec;

	np = op->next;
	while (op = np) {
		initargs(&vec);
		getargs(&vec, op->name);
		if (endargs(&vec) > 0) {
			state.cmdline = 1;
			set(vec.argv);
			state.cmdline = 0;
		}
		sreset();
		np = op->next;
		free(op);
	}
}

int
main(int argc, char** argv)
{
	register int	i;
	int		sig;
	char*		ef;
	int		flags = SIGN;
	sig_t		prevint;
	struct header	head;
	struct list	options;
	struct list*	op;
#if _PACKAGE_ast
	int		fatal = 0;
#endif

#if _PACKAGE_ast
	error_info.id = "mailx";
#endif

	/*
	 * Set up a reasonable environment.
	 * Figure out whether we are being run interactively,
	 * and so forth.
	 */
	memset(&head, 0, sizeof(head));
	(op = &options)->next = 0;
	if (!(state.path.buf = sfstropen()) || !(state.path.move = sfstropen()) || !(state.path.part = sfstropen()) || !(state.path.temp = sfstropen()))
		note(FATAL, "out of space");
	varinit();
	/*
	 * Now, determine how we are being used.
	 * We successively pick off - flags.
	 * If there is anything left, it is the base of the list
	 * of users to mail to.  Argp will be set to point to the
	 * first of these users.
	 */
	ef = 0;
	opterr = 0;
	for (;;) {
		switch (getopt(argc, argv, usage)) {
		case 0:
		case EOF:
			break;
		case 'A':
			op = setopt(op, "news", NiL);
			continue;
		case 'F':
			flags |= FOLLOWUP;
			flags &= ~SIGN;
			continue;
		case 'H':
			/*
			 * List all headers and exit.
			 */
			op = setopt(op, "justheaders", NiL);
			state.var.quiet = state.on;
			continue;
		case 'I':
			/*
			 * We're interactive
			 */
			op = setopt(op, "interactive", NiL);
			continue;
		case 'N':
			/*
			 * Avoid initial header printing.
			 */
			op = setopt(op, "noheader", NiL);
			state.var.quiet = state.on;
			continue;
		case 'P':
			/*
			 * Coprocess on pipe.
			 */
			op = setopt(op, "coprocess", NiL);
			continue;
		case 'Q':
			/*
			 * List all n most recent status and senders and exit.
			 */
			op = setopt(op, "justfrom", optarg);
			state.var.quiet = state.on;
			continue;
		case 'S':
			/*
			 * List all status and senders and exit.
			 */
			op = setopt(op, "justfrom", "-1");
			state.var.quiet = state.on;
			continue;
		case 'T':
			/*
			 * Next argument is temp file to write which
			 * articles have been read/deleted for netnews.
			 */
			op = setopt(op, "news", optarg);
			continue;
		case 'b':
			/*
			 * Get Blind Carbon Copy Recipient list
			 */
			extract(&head, GBCC|GMETOO, optarg);
			continue;
		case 'c':
			/*
			 * Get Carbon Copy Recipient list
			 */
			extract(&head, GCC|GMETOO, optarg);
			continue;
		case 'd':
			/*
			 * Debug output.
			 */
			op = setopt(op, "debug", NiL);
			continue;
		case 'e':
			/*
			 * Silently exit 0 if mail, 1, otherwise.
			 */
			op = setopt(op, "justcheck", NiL);
			state.var.quiet = state.on;
			continue;
		case 'f':
#if _PACKAGE_ast
			if (!(ef = opt_info.arg))
				ef = "&";
#else
			/*
			 * User is specifying file to "edit" with Mail,
			 * as opposed to reading system mailbox.
			 * If no argument is given after -f, we read his
			 * mbox file.
			 *
			 * getopt() can't handle optional arguments, so here
			 * is an ugly hack to get around it.
			 */
			if (argv[optind] && argv[optind][0] != '-')
				ef = argv[optind++];
			else
				ef = "&";
#endif
			continue;
		case 'i':
			/*
			 * User wants to ignore interrupts.
			 * Set the variable "ignore"
			 */
			op = setopt(op, "ignore", NiL);
			continue;
		case 'n':
			/*
			 * User doesn't want to source state.var.master
			 */
			op = setopt(op, "nomaster", NiL);
			continue;
		case 'o':
			/*
			 * Set option(s) by name.
			 */
			op = setopt(op, optarg, NiL);
			continue;
		case 'r':
			/*
			 * Set replyto.
			 */
			{
				char*			s;
				int			n;

				static const char	h[] = "fixedheaders=Reply-To:\" \"";

				n = strlen(optarg);
				if (!(s = newof(0, char, n + sizeof(h) + 1, 0)))
					note(PANIC, "Out of space");
				memcpy(s, h, sizeof(h) - 1);
				memcpy(s + sizeof(h) - 1, optarg, n);
				op = setopt(op, s, NiL);
			}
			continue;
		case 's':
			/*
			 * Give a subject field for sending from
			 * non terminal
			 */
			if (head.h_subject = optarg)
				head.h_flags |= GSUB;
			continue;
		case 't':
			/*
			 * Check for headers in message text.
			 */
			op = setopt(op, "sendheaders", optarg);
			state.mode = SEND;
			continue;
		case 'u':
			/*
			 * Next argument is person to pretend to be.
			 */
			op = setopt(op, "user", optarg);
			continue;
		case 'v':
			/*
			 * Send mailer verbose flag
			 */
			op = setopt(op, "verbose", NiL);
			continue;
#if _PACKAGE_ast
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			fatal = 1;
			break;
#else
		case '?':
			note(FATAL, "\
Usage: mail [-o [no]name[=value]] [-s subject] [-c cc] [-b bcc] to ...\n\
       mail [-o [no]name[=value]] [-f [folder]]");
			break;
#endif
		}
		break;
	}
#if _PACKAGE_ast
	if (fatal)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
#endif
	for (i = optind; (argv[i]) && (*argv[i] != '-'); i++)
		extract(&head, GTO|GMETOO, argv[i]);
	if (argv[i])
		head.h_options = argv;
	if (!state.mode)
		state.mode = (head.h_flags & GTO) ? SEND : RECEIVE;
	/*
	 * Check for inconsistent arguments.
	 */
	if (state.mode == RECEIVE && (head.h_flags & GSTD)) {
		if (!state.var.sendheaders)
			note(FATAL|IDENTIFY, "You must specify direct recipients with -s, -c, or -b");
		state.mode = SEND;
	}
	if (state.mode == RECEIVE)
		state.var.receive = state.on;
	if (state.mode == SEND && ef)
		note(FATAL|IDENTIFY, "Cannot give -f and people to send to");
	if (state.var.justcheck && state.mode == SEND)
		exit(1);
	tempinit();
	state.input = stdin;
	/*
	 * Up to this point salloc()==malloc() by default.
	 * From now on salloc() space cleared by sreset().
	 */
	state.onstack = 1;
	if (state.var.master)
		load(expand(state.var.master, 1));
	/*
	 * Expand returns a savestr, but load only uses the file name
	 * for fopen, so it's safe to do this.
	 */
	load(expand(state.var.mailrc, 1));
	/*
	 * Reset command line options so they take precedence over the rc's.
	 */
	resetopt(&options);
	if (state.mode == SEND) {
		sendmail(&head, flags);
		/*
		 * why wait?
		 */
		exit(state.senderr);
	}
	/*
	 * Ok, we are reading mail.
	 * Decide whether we are editing a mailbox or reading
	 * the system mailbox, and open up the right stuff.
	 */
	if (!ef)
		ef = "%";
	if (setfolder(ef) < 0)
		exit(1);
	if (sig = setjmp(state.jump.header))
		resume(sig);
	else {
		if ((prevint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
			signal(SIGINT, hdrstop);
		if (!state.var.quiet)
			note(0, "Mail version %s.  Type ? for help", state.version);
		announce();
		fflush(stdout);
		signal(SIGINT, prevint);
	}
	if (!state.var.justheaders) {
		commands();
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		quit();
	}
	exit(0);
}
