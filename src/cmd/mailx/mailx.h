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
 * Author:
 *
 *	Kurt Shoens (UCB) March 25, 1978
 *
 * Contributors:
 *
 *	Glenn Fowler (AT&T Research) 1996-09-11
 */

#if _PACKAGE_ast

#include <ast.h>
#include <error.h>
#include <getopt.h>
#include <ls.h>
#include <sig.h>
#include <times.h>
#include <wait.h>

#else /*_PACKAGE_ast*/

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef O_BINARY
#define O_BINARY	0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC	0
#endif

extern char*	optarg;
extern int	optind;
extern int	opterr;

#endif /*_PACKAGE_ast*/

#include <cdt.h>
#include <ctype.h>
#include <errno.h>
#include <mime.h>
#include <setjmp.h>
#include <stdio.h>

#include "local.h"

#define APPEND_MAILBOX		1	/* New mail goes to end of mailbox */
#if defined(SFIO_VERSION) && defined(SF_READ) && defined(SF_WRITE)
#define MORE_DISCIPLINE		1	/* Sfio more discipline on stdout */
#endif

#define CALL(f)		(*(state.folder==FIMAP?imap_##f:f))
#define TRACE(c)	(state.trace|=(1<<((c)-'a')))
#define TRACING(c)	(state.trace&(1<<((c)-'a')))

#define sig_t		Sig_handler_t

#define holdsigs()	sigcritical(SIG_REG_EXEC)
#define relsesigs()	sigcritical(SIG_REG_POP)

#define initargs(ap)	((ap)->argp=(ap)->argv)
#define endargs(ap)	(*(ap)->argp=0,(ap)->argp-(ap)->argv)

#if MORE_DISCIPLINE
#define moretop()	(state.more.match=0,state.more.row=state.more.col=1)
#else
#define moretop()
#endif

#define shquote		shellquote	/* netbsd has one in <stdlib.h>! */

#define ESCAPE		'~'		/* Default escape for sending */
#define NMLSIZE		1024		/* max names in a message list */
#define PATHSIZE	MAXPATHLEN	/* Size of pathnames throughout */
#define LINESIZE	(32*STRINGLEN)	/* max readable line width */
#define HEADSIZE	128		/* maximum header line length */
#define LASTSIZE	256		/* max saved cmd line size */
#define STRINGSIZE	((unsigned)128)	/* Dynamic allocation units */
#define MAILMODE	(S_IRUSR|S_IWUSR) /* private mail file mode */
#define METAFILE	"%#&+/"		/* `Metafile' prefix */
#define REGDEP		2		/* Maximum regret depth. */
#define STRINGLEN	1024		/* Maximum length of string token */
#define MARGIN		72		/* Right line margin */
#define REFLEN		(12*MARGIN)	/* Maximum length or References: */

typedef struct msg {
	int	m_index;		/* command address vector index */
	short	m_flag;			/* flags, see below */
	short	m_offset;		/* offset in block of message */
	long	m_block;		/* block number of this message */
	off_t	m_size;			/* Bytes in the message */
	off_t	m_lines;		/* Lines in the message */
	void*	m_info;			/* Folder type specific info */
} Msg_t;

/*
 * Folder types.
 */

#define FFILE		1		/* File folder */
#define FIMAP		2		/* IMAP folder */
#define FMH		3		/* MH folder */

/*
 * Command flag bits.
 */

#define MBOX		(1<<0)		/* send this to mbox, regardless */
#define MDELETE		(1<<1)		/* entry has been deleted */
#define MINIT		(1<<2)		/* folder specific init mark */
#define MMARK		(1<<3)		/* message is marked! */
#define MNEW		(1<<4)		/* message has never been seen */
#define MNONE		(1<<5)		/* never matches */
#define MODIFY		(1<<6)		/* message has been modified */
#define MPRESERVE	(1<<7)		/* keep entry in sys mailbox */
#define MREAD		(1<<8)		/* message has been read sometime */
#define MSAVE		(1<<9)		/* entry has been saved */
#define MSCAN		(1<<10)		/* entry has been scanned */
#define MSPAM		(1<<11)		/* message is probably spam */
#define MSTATUS		(1<<12)		/* message status has changed */
#define MTOUCH		(1<<13)		/* entry has been noticed */
#define MUSED		(1<<14)		/* entry is used, but this bit isn't */
#define MZOMBIE		(1<<15)		/* deleted but still there */

/*
 * Given a file address, determine the block number it represents.
 */

#define blocknumber(off)		((int) ((off) / 4096))
#define blockoffset(off)		((int) ((off) % 4096))
#define blockposition(block,offset)	((off_t)(block) * 4096 + (offset))

/*
 * Format of the command description table.
 */

typedef int (*Cmd_f)(void*);

struct cmd {
	const char*	c_name;		/* Name of command */
	Cmd_f		c_func;		/* Implementor of the command */
	unsigned long	c_argtype;	/* Type of arglist (see below) */
	unsigned long	c_msgflag;	/* Required flags of messages */
	size_t		c_msgmask;	/* Relevant flags of messages */
	const char*	c_help;		/* Command help text */
};

/*
 * Yechh, can't initialize unions.
 */

#define c_minargs c_msgflag		/* Minimum argcount for RAWLIST */
#define c_maxargs c_msgmask		/* Max argcount for RAWLIST */

struct esc {
	const char*	e_name;		/* Name of command */
	const char*	e_help;		/* Command help text */
};

/*
 * Common header labels.
 */

struct lab {
	const char*	name;		/* Header label name */
	long		type;		/* G* type */
};

/*
 * Header parse state.
 */

struct parse {
	struct msg*	mp;		/* Parsing this message */
	struct header*	hp;		/* Matched headers here */
	Dt_t**		ignore;		/* Ignore these headers */
	FILE*		fp;		/* Message io */
	long		count;		/* Remaining message size */
	char*		name;		/* Header name */
	char*		data;		/* Header data */
	char*		separator;	/* Header name separator position */
	int		length;		/* Total header length */
	unsigned long	flags;		/* G* flags */
	unsigned long	type;		/* Matched header type */
	char		buf[LINESIZE];	/* Work buffer */
};

/*
 * Argument types.
 */

#define MSGLIST	 0		/* Message list type */
#define STRLIST	 1		/* A pure string */
#define RAWLIST	 2		/* Shell string list */
#define NOLIST	 3		/* Just plain 0 */
#define NDMLIST	 4		/* Message list, no defaults */

#define LISTMASK 07		/* Mask list type from argument type */

#define A	(1<<4)		/* Var alias */
#define C	(1<<5)		/* Is a conditional command, Cmd line var set */
#define D	(1<<6)		/* Var unset default to initial value */
#define E	(1<<7)		/* Var init from environ */
#define I	(1<<8)		/* Interactive command, Var is integer */
#define L	(1<<9)		/* Append line values */
#define M	(1<<10)		/* Valid from send mode */
#define N	(1<<11)		/* Var null value means off */
#define P	(1<<12)		/* Autoprint dot after command */
#define R	(1<<13)		/* Cannot call from collect, Readonly var */
#define S	(1<<14)		/* Var cannot change while sourcing */
#define W	(1<<15)		/* Invalid for readonly */
#define Z	(1L<<16)	/* Is a transparent command */

/*
 * Oft-used mask values
 */

#define MMNORM		(MDELETE|MSAVE)/* Look at both save and delete bits */
#define MMNDEL		MDELETE	/* Look only at deleted bit */

/*
 * note() type bits.
 */

#define DEBUG		(1<<0)		/* debug trace */
#define ERROR		(1<<1)		/* stderr message */
#define FATAL		(1<<2)		/* message and exit(1) */
#define IDENTIFY	(1<<3)		/* prefix with command name */
#define PANIC		(1<<4)		/* message and abort() */
#define PROMPT		(1<<5)		/* no trailing newline */
#define SYSTEM		(1<<6)		/* append errno message */
#define WARNING		(1<<7)		/* warning prefix */

/*
 * Structure used to return a break down of a head
 * line (hats off to Bill Joy!)
 */

struct headline {
	char*	l_from;		/* The name of the sender */
	char*	l_info;		/* Special info (tty or article number) */
	char*	l_date;		/* The entire date string */
};

/*
 * Name extraction and name dictionary node flags.
 */

#define GALIAS		(1<<0)		/* Alias name */
#define GALTERNATE	(1<<1)		/* Alternate name */
#define GBCC		(1<<2)		/* Grab Bcc: line */
#define GCC		(1<<3)		/* Grab Cc: line */
#define GCOMMA		(1<<4)		/* Comma separated */
#define GCOMPARE	(1<<5)		/* For comparison */
#define GDISPLAY	(1<<6)		/* For display */
#define GDONE		(1<<7)		/* Done with it */
#define GFIRST		(1<<8)		/* First recipient */
#define GFROM		(1<<9)		/* Don't skip initial `From ' */
#define GINTERPOLATE	(1<<10)		/* Check headers for interpolate() */
#define GLAST		(1<<11)		/* Get last instance */
#define GMAP		(1<<12)		/* Already mapped */
#define GMESSAGEID	(1<<13)		/* Grab Message-ID: line */
#define GMETOO		(1<<14)		/* Send to state.var.user too */
#define GMIME		(1L<<15)	/* Check MIME content headers */
#define GMISC		(1L<<16)	/* Grab miscellaneous headers */
#define GNEWS		(1L<<17)	/* For newsgroup article id */
#define GNL		(1L<<18)	/* Print blank line after headers */
#define GREFERENCES	(1L<<19)	/* Grab References: line */
#define GREPLY		(1L<<20)	/* For reply to sender */
#define GRULE		(1L<<21)	/* Ouput rule if GNL */
#define GSEND		(1L<<22)	/* Get it ready to send */
#define GSENDER		(1L<<23)	/* Get state.var.sender address only */
#define GSTACK		(1L<<24)	/* savestr() unmapped names */
#define GSTATUS		(1L<<25)	/* Grab Status: line */
#define GSUB		(1L<<26)	/* Grab Subject: line */
#define GTO		(1L<<27)	/* Grab To: line */
#define GUSER		(1L<<28)	/* Stop if ${user}@ */

#define GCOMPOSE	(GEDIT|GSTATUS)	/* Composable headers */
#define GEDIT		(GSTD|GMISC)	/* Editable headers */
#define GEXTERN		(GEDIT&~GBCC)	/* External headers */
#define GMASK		(GNAME|GDONE)	/* Active mask */
#define GNAME		(GBCC|GCC|GTO)	/* Name fields */
#define GSTD		(GNAME|GSUB)	/* Standard headers */

/*
 * Structure of a variable node.
 */

struct var {
	const char*	name;
	char**		variable;
	unsigned long	flags;
	const char*	initialize;
	void		(*set)(struct var*, const char*);
	const char*	help;
};

struct list {
	struct list*	next;
	char		name[1];
};

struct name {
	Dtlink_t	link;
	void*		value;
	unsigned long	flags;
	char		name[1];
};

struct argvec {
	char*		argv[64 * 1024];
	char**		argp;
};

struct child {
	struct child*	link;
	int		pid;
	short		done;
	short		free;
};

struct file {
	struct file*	link;
	FILE*		fp;
	int		pid;
};

struct mhcontext {
	int		type;
	unsigned long	dot;
	unsigned long	next;
	struct {
	unsigned long	dot;
	unsigned long	next;
	}		old;
};

typedef struct {
	int		type;
	void*		state;
} Imapcontext_t;

#define PART_application	(1<<0)
#define PART_body		(1<<1)
#define PART_disposition	(1<<2)
#define PART_inline		(1<<3)
#define PART_message		(1<<4)
#define PART_text		(1<<5)

typedef struct part {
	struct part*	next;
	off_t		offset;
	off_t		size;
	off_t		lines;
	struct {
	off_t		offset;
	off_t		size;
	}		raw;
	unsigned long	flags;
	int		count;
	char		name[HEADSIZE];
	char		type[HEADSIZE];
	char		opts[HEADSIZE];
	char		code[HEADSIZE];
} Part_t;

struct bound {
	struct bound*	next;
	int		size;
	char		data[1];
};

/*
 * Structure used to pass about the current
 * state of the user-typed message header.
 */

struct header {
	unsigned long	h_flags;	/* Active fields */
	Dt_t*		h_names;	/* Recipients */
	char**		h_options;	/* Mailer options */
	char*		h_subject;	/* Subject string */
	char*		h_first;	/* First recipient */
	char*		h_messageid;	/* Parent message-id */
	char*		h_references;	/* References */
	struct {
	struct list*	head;
	struct list*	tail;
	}		h_misc;		/* Miscellaneous headers */
	unsigned long	h_clear;	/* Clear these on change */
};

struct dict {
	Dtdisc_t	disc;		/* Object discipline */
	unsigned long	flags;		/* Member node flags */
	Dt_t*		next;		/* Next STACK dict */
};

struct match {
	struct match*	next;		/* next in list */
	int		length;		/* string length */
	int		beg;		/* begin character match */
	int		mid;		/* mid character match */
	int		end;		/* end character match */
	char		string[1];	/* match string */
};

struct linematch {
	int		minline;	/* minimum line size */
	unsigned char	beg[256];	/* begin character match */
	unsigned char	mid[256];	/* mid character match */
	unsigned char	end[256];	/* end character match */
	struct match*	match;		/* exact match list */
	struct match*	last;		/* last match list item */
};

struct sendand {
	struct sendand*	next;		/* next in and list */
	char*		head;		/* head	*/
	char*		pattern;	/* match pattern */
	unsigned long	flags;		/* grab*() flags */
};

struct sendor {
	struct sendor*	next;		/* next in or list */
	struct sendand	sendand;	/* and list */
};

struct sender {
	struct sender*	next;		/* next in list */
	struct sendor	sendor;		/* or list */
	char		address[1];	/* sender address override */
};

/*
 * dictsearch() flag values
 */

#define LOOKUP		0
#define COPY		(1<<0)
#define CREATE		(1<<1)
#define DELETE		(1<<2)
#define IGNORECASE	(1<<3)
#define INSERT		(1<<4)
#define OBJECT		(1<<5)
#define STACK		(1<<6)

/*
 * ignore flags
 */

#define dictflags(p)	(((struct dict*)(*(p))->disc)->flags)

#define HIT		(1<<0)		/* Global table flag */
#define IGNORE		(1<<1)		/* Global table flag */
#define RETAIN		(1<<2)		/* Global table flag */

/*
 * collect() and mail() flags
 */

#define FOLLOWUP	(1<<0)
#define HEADERS		(1<<1)
#define INTERPOLATE	(1<<2)
#define MARK		(1<<3)
#define REPLY		(1<<4)
#define SIGN		(1<<5)

/*
 * Token values returned by the scanner used for argument lists.
 */

#define TEOL	0			/* End of the command line */
#define TNUMBER	1			/* A message number */
#define TDASH	2			/* A simple dash */
#define TSTRING	3			/* A string (possibly containing -) */
#define TDOT	4			/* A "." */
#define TUP	5			/* An "^" */
#define TDOLLAR	6			/* A "$" */
#define TSTAR	7			/* A "*" */
#define TOPEN	8			/* An '(' */
#define TCLOSE	9			/* A ')' */
#define TPLUS	10			/* A '+' */
#define TERROR	11			/* A lexical error */

/*
 * Constants for conditional commands.  These describe whether
 * we should be executing stuff or not.
 */

#define RECEIVE		(-1)		/* Execute in receive mode only */
#define SEND		(1)		/* Execute in send mode only */

/*
 * Kludges to handle the change from setexit / reset to setjmp / longjmp
 */

#define setexit()	do {						\
				int x = setjmp(state.jump.sr);		\
				if (x) sigunblock(x);			\
			} while(0)
#define reset(x)	longjmp(state.jump.sr, x)

/*
 * <unistd.h> etc encroachment
 * we asked for extentions -- and now pay for it ...
 */

#define undelete	mail_undelete

/*
 * The pointers for the string allocation routines,
 * there are NSPACE independent areas.
 * The first holds STRINGSIZE bytes, the next
 * twice as much, and so on.
 */

#define NSPACE	25			/* Total number of string spaces */
struct strings {
	char*		s_topfree;	/* Beginning of this area */
	char*		s_nextfree;	/* Next alloctable place here */
	unsigned int	s_nleft;	/* Number of bytes left here */
};

typedef struct {
	const char*		version;	/* Version string */
	const char*		license;	/* License text */
	char*			on;		/* Variable on value */
	const struct cmd*	cmdtab;		/* Command table */
	int			cmdnum;		/* Number of commands */
	const struct esc*	esctab;		/* Escape command table */
	int			escnum;		/* Number of escape commands */
	const struct var*	vartab;		/* Variable table */
	int			varnum;		/* Number of variables */
	const struct lab*	hdrtab;		/* Header label table */
	int			hdrnum;		/* Number of header labels */
	unsigned long		askheaders;	/* Ask for these headers */

	/* the rest are implicitly initialized */

	int	clobber;		/* Last command had ! */
	int	cmdline;		/* Currently reading cmd line options */
	int	colmod;			/* Mark bit */
	int	cond;			/* Current state of conditional exc. */
	int	edit;			/* Indicates editing a file */
	int	incorporating;		/* Executing incorporate command */
	int	folder;			/* Folder type */
	int	hung;			/* Op hung til alarm */
	int	loading;		/* Loading user definitions */
	int	mode;			/* SEND or RECEIVE */
	int	noreset;		/* String resets suspended */
	int	onstack;		/* salloc() != malloc() */
	int	readonly;		/* Will be unable to rewrite file */
	int	realscreenheight;	/* Real screen height */
	int	sawcom;			/* Set after first command */
	int	screenheight;		/* Screen height, or best guess */
	int	screenwidth;		/* Screen width, or best guess */
	int	scroll;			/* Current scroll size */
	int	senderr;		/* An error while checking */
	int	sourcing;		/* Currently reading variant file */
	int	startup;		/* Listing startup headers */
	int	stopreset;		/* Reset on stop */

	FILE*	input;			/* Current command input file */
	off_t	mailsize;		/* Size of system mailbox */
	int	lexnumber;		/* Number of TNUMBER from scan() */
	char	lexstring[STRINGLEN];	/* String from TSTRING, scan() */
	int	regretp;		/* Pointer to TOS of regret tokens */
	int	regretstack[REGDEP];	/* Stack of regretted tokens */
	char*	string_stack[REGDEP];	/* Stack of regretted strings */
	int	numberstack[REGDEP];	/* Stack of regretted numbers */
	char	number[16];		/* Temp variable number string */
	char	counts[32];		/* Temp counts number string */

	unsigned long	trace;		/* Trace bits */
	unsigned long	editheaders;	/* These headers in edit template */
	struct child*	children;	/* Child list */
	struct cmd*	cmd;		/* Current command table entry */
	struct file*	files;		/* fileopen() list */
	struct linematch* bodymatch;	/* compiled state.var.spambody */
	struct sender*	sender;		/* compiled state.var.sender */
	struct stat	openstat;	/* fileopen stat */
	Dt_t* 		ignore;		/* Ignored fields */
	Dt_t*		saveignore;	/* Ignored fields on save to folder */
	Dt_t*		ignoreall;	/* Special: ignore all headers */
	Dt_t*		aliases;	/* aliases */
	Dt_t*		userid;		/* User name -> id map */
	Dt_t*		stacked;	/* STACK dict list */

	struct strings	stringdope[NSPACE];

	struct {
	sig_t	sigint;			/* Previous SIGINT value */
	sig_t	sighup;			/* Previous SIGHUP value */
	sig_t	sigtstp;		/* Previous SIGTSTP value */
	sig_t	sigttou;		/* Previous SIGTTOU value */
	sig_t	sigttin;		/* Previous SIGTTIN value */
	FILE*	fp;			/* File for saving away */
	int	hadintr;		/* Have seen one SIGINT so far */
	jmp_buf	work;			/* To get back to work */
	int	working;		/* Whether to long jump */
	jmp_buf	abort;			/* To end collection with error */
	}	collect;

	struct {
	jmp_buf	header;			/* Printing headers */
	jmp_buf	sr;			/* set/reset longjmp buffer */
	jmp_buf	sigpipe;		/* SIGPIPE longjmp buffer */
	}	jump;

	struct {
	char	bang[LASTSIZE];		/* Last ! command */
	char	scan[LASTSIZE];		/* Last message search string */
	}	last;

	struct {
	int		count;		/* Count of messages read in */
	int		inbox;		/* Current folder mh state.var.inbox */
	Imapcontext_t	imap;		/* imap message format */
	struct mhcontext mh;		/* mh message format */
	int		size;		/* Max messages in vector */
	int*		vec;		/* Current message vector */
	FILE*		ap;		/* Actual file pointer */
	FILE*		ip;		/* Input temp file buffer */
	FILE*		op;		/* Output temp file buffer */
	struct msg*	active;		/* ip points to this message */
	struct msg*	context;	/* Folder read context */
	struct msg*	dot;		/* Pointer to current message */
	struct msg*	list;		/* The actual message structure */
	}		msg;

	struct {
	char	pwd[2][PATHSIZE];	/* pwd and oldpwd paths */
	char	mail[PATHSIZE];		/* Name of current file */
	char	path[PATHSIZE];		/* Very temporary fixed path buffer */
	char	prev[PATHSIZE];		/* Name of previous file */
	Sfio_t*	buf;			/* Very temporary name buffer */
	Sfio_t*	move;			/* Very temporary name buffer */
	Sfio_t*	part;			/* Very temporary name buffer */
	Sfio_t*	temp;			/* Very temporary name buffer */
	}	path;

	struct {
	int	sp;			/* Top of stack */
	struct {
	FILE*	input;			/* Saved state.input */
	int	cond;			/* Saved state.cond */
	int	loading;		/* Saved state.loading */
	}	stack[NOFILE];
	}	source;

	struct more {
#if MORE_DISCIPLINE
	Sfdisc_t	disc;
	int		row;
	int		col;
	int		match;
	char		pattern[HEADSIZE];
	char		tmp[HEADSIZE];
#endif
	int		discipline;
	int		init;
	}		more;

	struct state_part {

	Mimedisc_t	disc;
	Mime_t*		mime;
	Part_t*		head;
	Part_t		global;
	int		init;

	struct {
	int		multi;
	int		count;
	Part_t*		head;
	Part_t*		tail;
	struct bound*	boundary;
	}		in;

	struct {
	int		multi;
	int		boundlen;
	char		boundary[HEADSIZE];
	}		out;

	}		part;

	struct {
	char	conv[256];
	char	edit[256];
	char	head[8*1024];
	char	mail[256];
	char	mesg[256];
	char	more[256];
	char	quit[256];
	char*	dir;
	}	tmp;

	struct
	{
	char*	allnet;
	char*	append;
	char*	askbcc;
	char*	askcc;
	char*	askheaders;
	char*	asksub;
	char*	attachments;
	char*	autoinc;
	char*	autoprint;
	char*	autosign;
	char*	bang;
	char*	cdpath;
	char*	cmd;
	char*	convertheaders;
	char*	coprocess;
	long	crt;
	char*	dead;
	char*	debug;
	char*	domain;
	char*	dot;
	char*	editheaders;
	char*	editor;
	char*	escape;
	char*	fixedheaders;
	char*	flipr;
	char*	folder;
	char*	followup;
	char*	header;
	char*	headerbotch;
	char*	headfake;
	char*	hold;
	char*	home;
	char*	hostname;
	char*	ignore;
	char*	ignoreeof;
	char*	imap;
	char*	inbox;
	char*	indentprefix;
	char*	interactive;
	char*	justcheck;
	long	justfrom;
	char*	justheaders;
	char*	keep;
	char*	keepsave;
	char*	lister;
	char*	local;
	char*	lock;
	char*	log;
	char*	mail;
	char*	mailcap;
	char*	mailrc;
	char*	master;
	char*	mbox;
	char*	metoo;
	char*	more;
	char*	news;
	char*	oldpwd;
	char*	onehop;
	char*	outfolder;
	char*	page;
	char*	pager;
	char*	prompt;
	char*	pwd;
	char*	quiet;
	char*	receive;
	char*	recent;
	char*	rule;
	char*	save;
	long	screen;
	char*	searchheaders;
	char*	sender;
	char*	sendheaders;
	char*	sendmail;
	char*	sendwait;
	char*	shell;
	char*	showto;
	char*	Sign;
	char*	sign;
	char*	signature;
	char*	smtp;
	char*	spam;
	char*	spambody;
	long	spamdelay;
	char*	spamfrom;
	char*	spamfromok;
	char*	spamlog;
	char*	spamsub;
	char*	spamsubhead;
	long	spamtest;
	char*	spamto;
	char*	spamtook;
	char*	spamvia;
	char*	spamviaok;
	long	toplines;
	char*	trace;
	char*	user;
	char*	verbose;
	char*	visual;
	}	var;

} State_t;

extern State_t		state;

extern int		Blast(struct msg*);
extern int		Copy(char*);
extern int		Followup(struct msg*);
extern int		From(struct msg*);
extern int		Get(char**);
extern int		Join(struct msg*);
extern int		More(struct msg*);
extern int		Reply(struct msg*);
extern int		Save(char*);
extern int		Split(char*);
extern int		Type(struct msg*);
extern int		addarg(struct argvec*, const char*);
extern int		alias(char**);
extern void		alter(char*);
extern int		alternates(char**);
extern void		announce();
extern int		anyof(char*, char*);
extern int		blankline(char*);
extern int		blast(struct msg*);
extern void		boundary(void);
extern int		capability(char**);
extern int		cd(char**);
extern int		check(int, int);
extern int		cmdcopy(char*);
extern int		cmddelete(struct msg*);
extern int		cmdelse(void);
extern int		cmdendif(void);
extern int		cmdexit(int);
extern int		cmdif(char**);
extern int		cmdmkdir(char**);
extern int		cmdpipe(char*);
extern int		cmdquit(void);
extern int		cmdrename(char**);
extern int		cmdrmdir(char**);
extern int		cmdtouch(char*);
extern int		cmdwrite(char*);
extern FILE*		collect(struct header*, unsigned long);
extern void		commands(void);
extern int		copy(struct msg*, FILE*, Dt_t**, char*, unsigned long);
extern char*		counts(int, off_t, off_t);
extern void		dictclear(Dt_t**);
extern void		dictreset(void);
extern struct name*	dictsearch(Dt_t**, const char*, int);
extern int		dictwalk(Dt_t**, int(*)(Dt_t*, void*, void*), void*);
extern int		deltype(struct msg*);
extern void		demail(void);
extern char*		detract(struct header*, unsigned long);
extern int		dot(void);
extern int		duplicate(char*);
extern int		echo(char**);
extern int		editor(struct msg*);
extern int		execute(char*, int);
extern char*		expand(char*, int);
extern void		extract(struct header*, unsigned long, char*);
extern void		fileclear(void);
extern int		fileclose(FILE*);
extern FILE*		filefd(int, char*);
extern int		filecopy(const char*, FILE*, const char*, FILE*, FILE*, off_t, off_t*, off_t*, unsigned long);
extern int		filelock(const char*, FILE*, int);
extern FILE*		fileopen(char*, char*);
extern off_t		filesize(FILE*);
extern FILE*		filestd(char*, char*);
extern FILE*		filetemp(char*, int, int, int);
extern int		filetrunc(FILE*);
extern int		first(int, int);
extern int		folder(char**);
extern struct msg*	folderinfo(int);
extern int		folders(void);
extern int		followup(struct msg*);
extern void		free_command(int);
extern int		from(struct msg*);
extern int		get(char**);
extern void		getargs(struct argvec*, char*);
extern int		getfolder(char*, size_t);
extern int		getmsglist(char*, unsigned long);
extern char*		grab(struct msg*, unsigned long, char*);
extern void		grabedit(struct header*, unsigned long);
extern void		headclear(struct header*, unsigned long);
extern int		headers(struct msg*);
extern int		headget(struct parse*);
extern int		headout(FILE*, struct header*, unsigned long);
extern int		headset(struct parse*, struct msg*, FILE*, struct header*, Dt_t**, unsigned long);
extern int		help(char**);
extern int		ignore(char**);
extern int		ignored(Dt_t**, const char*);
extern int		incorporate(void);
extern int		incfile(void);
extern int		isall(const char*);
extern char*		iscmd(char*);
extern int		isdate(char*);
extern int		isdir(char*);
extern int		ishead(char*, int);
extern int		isreg(char*);
extern int		join(struct msg*);
extern int		license(void*);
extern int		list(void);
extern void		load(char*);
extern char*		localize(char*);
extern int		lower(int);
extern int		mail(char*);
extern char*		mailbox(const char*, const char*);
extern int		map(char*);
extern int		mark(char*);
extern int		mboxit(char*);
extern void		mhgetcontext(struct mhcontext*, const char*, int);
extern void		mhputcontext(struct mhcontext*, const char*);
extern int		mime(int);
extern int		more(struct msg*);
extern void		msgflags(struct msg*, int, int);
extern struct msg*	newmsg(off_t offset);
extern int		next(struct msg*);
extern char*		normalize(char*, unsigned long, char*, size_t);
extern void		note(int, const char*, ...);
extern int		notyet(char*);
extern int		null(int);
extern void		parse(struct msg*, char*, struct headline*, char*, size_t);
extern FILE*		pipeopen(char*, char*);
extern int		preserve(char*);
extern int		puthead(FILE*, struct header*, int);
extern int		putline(FILE*, char*);
extern int		pwd(void);
extern void		quit(void);
extern int		readline(FILE*, char*, int);
extern char*		record(char*, unsigned long);
extern int		regular(FILE*);
extern int		reply(struct msg*);
extern int		replyall(struct msg*);
extern int		replysender(struct msg*);
extern void		resume(int);
extern int		retain(char**);
extern int		rm(char*);
extern int		run_command(char*, int, int, int, char*, char*, char*);
extern FILE*		run_editor(FILE*, off_t, struct header*, int, int);
extern char*		salloc(int);
extern int		save(char*);
extern void		savedeadletter(FILE*);
extern int		saveignore(char**);
extern int		saveretain(char**);
extern char*		savestr(char*);
extern int		scan(char**);
extern int		scroll(char*);
extern int		sender(char*, int);
extern void		sendmail(struct header*, unsigned long);
extern int		sendsmtp(FILE*, char*, char**, off_t);
extern int		set(char**);
extern void		set_askbcc(struct var*, const char*);
extern void		set_askcc(struct var*, const char*);
extern void		set_askheaders(struct var*, const char*);
extern void		set_asksub(struct var*, const char*);
extern void		set_coprocess(struct var*, const char*);
extern void		set_crt(struct var*, const char*);
extern void		set_editheaders(struct var*, const char*);
extern void		set_justfrom(struct var*, const char*);
extern void		set_list(struct var*, const char*);
extern void		set_mail(struct var*, const char*);
extern void		set_mailcap(struct var*, const char*);
extern void		set_more(struct var*, const char*);
extern void		set_news(struct var*, const char*);
extern void		set_notyet(struct var*, const char*);
extern void		set_pwd(struct var*, const char*);
extern void		set_screen(struct var*, const char*);
extern void		set_sender(struct var*, const char*);
extern void		set_sendmail(struct var*, const char*);
extern void		set_shell(struct var*, const char*);
extern void		set_spambody(struct var*, const char*);
extern void		set_spamtest(struct var*, const char*);
extern void		set_toplines(struct var*, const char*);
extern void		set_trace(struct var*, const char*);
extern void		set_user(struct var*, const char*);
extern int		setfolder(char*);
extern FILE*		setinput(struct msg*);
extern void		setptr(FILE*, off_t);
extern void		setscreensize(void);
extern void		settmp(const char*, int);
extern int		shell(char*);
extern void		shquote(Sfio_t*, char*);
extern int		size(struct msg*);
extern char*		skin(char*, unsigned long);
extern char*		snarf(char*, int*);
extern int		source(char**);
extern int		spammed(struct msg*);
extern int		split(char*);
extern void		sreset(void);
extern int		start_command(char*, int, int, int, char*, char*, char*);
extern char*		strlower(char*);
extern char*		strncopy(char*, const char*, size_t);
extern char*		struse(Sfio_t*);
extern void		tempinit(void);
extern int		top(struct msg*);
extern void		touchmsg(struct msg*);
extern int		ttyedit(int, int, const char*, char*, size_t);
extern int		ttyquery(int, int, const char*);
extern int		type(struct msg*);
extern int		unalias(char**);
extern int		undelete(struct msg*);
extern int		unread(char*);
extern int		unset(char**);
extern int		unstack(void);
extern int		upper(int);
extern int		userid(char*);
extern int		usermap(struct header*, int);
extern char*		username(void);
extern char*		varget(const char*);
extern void		varinit(void);
extern char*		varkeep(const char*);
extern int		varlist(int);
extern int		varset(const char*, const char*);
extern int		version(void*);
extern int		visual(struct msg*);
extern int		wait_command(int);
extern char*		wordnext(char**, char*);
extern char*		yankword(char*, char*);

/*
 * IMAP support
 */

#define imap_name(p)	((p)[0]=='@'||strneq(p,"imap://",7))

extern int		imap_command(char*);
extern int		imap_copy(struct msg*, FILE*, Dt_t**, char*, unsigned long);
extern void		imap_exit(int);
extern int		imap_folders(void);
extern int		imap_get1(char**, unsigned long);
extern int		imap_mkdir(char*);
extern void		imap_msgflags(struct msg*, int, int);
extern int		imap_msglist(char*);
extern void		imap_printhead(int, int);
extern void		imap_quit(void);
extern int		imap_rename(char*, char*);
extern int		imap_rmdir(char*);
extern int		imap_save(struct msg*, char*);
extern FILE*		imap_setinput(struct msg*);
extern int		imap_setptr(char*, int);

#if _PACKAGE_ast

#define T(s)		ERROR_translate(0,0,0,s)
#define X(s)		ERROR_catalog(s)

#else

#define T(s)		(s)	/* Dynamic translation string */
#define X(s)		(s)	/* Static translation string */

#define imap_command(a)		(-1)
#define imap_copy(a,b,c,d,e)	(-1)
#define	imap_exit(a)
#define imap_folders()		(-1)
#define imap_get1(a,b)		(-1)
#define imap_mkdir(a)		(-1)
#define imap_msgflags(a,b,c)	(-1)
#define imap_msglist(a)		(-1)
#define imap_printhead(a)
#define imap_quit()
#define imap_rename(a,b)	(-1)
#define imap_rmdir(a)		(-1)
#define imap_save(a)		(-1)
#define imap_setinput(a)	((FILE*)0)

#endif

/*
 * MH support
 */

extern int		mh_setptr(char*, int);
