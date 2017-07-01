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
 * IMAP4rev1 client
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "mailx.h"

#if _PACKAGE_ast

#include <css.h>
#include <tm.h>
#include <vmalloc.h>

#define imapfree(sp,op)	((op)->state=(op)->retain=0)

#define IMAP		((Imap_t*)state.msg.imap.state)

#define IMAP_VERSION	4		/* at least this		*/
#define IMAP_REVISION	1		/* at least this		*/

#define IMAP_data	1		/* data item			*/
#define IMAP_list	2		/* list item			*/
#define IMAP_name	3		/* name (atom) item		*/
#define IMAP_number	4		/* number item			*/
#define IMAP_string	5		/* string item			*/

struct Imaparg_s; typedef struct Imaparg_s Imaparg_t;
struct Imapblock_s; typedef struct Imapblock_s Imapblock_t;
struct Imappart_s; typedef struct Imappart_s Imappart_t;

typedef struct {			/* IMAP_data item		*/
	char*		buffer;
	size_t		length;
} Imapdata_t;

typedef struct {			/* IMAP_list item		*/
	Imaparg_t*	head;
	Imaparg_t*	tail;
} Imaplist_t;

typedef union {				/* list item value		*/
	Imapdata_t	data;
	Imaplist_t	list;
	long		number;
	char*		name;
	char*		string;
} Imapvalue_t;

struct Imaparg_s {			/* response arg list element	*/
	Imaparg_t*	next;
	int		type;
	Imapvalue_t	value;
};

typedef struct {			/* flag table element		*/
	const char*	name;		/* canonical flag name		*/
	int		code;		/* IMAP_* code			*/
	int		flag;		/* mailx M* flag		*/
} Imapflag_t;

typedef struct {			/* lex table element		*/
	const char*	name;		/* canonical name		*/
	int		code;		/* IMAP_* code			*/
} Imaplex_t;

typedef struct {			/* IMAP Msg_t.m_info info	*/
	char*		from;		/* from address			*/
	char*		date;		/* localized date		*/
	char*		subject;	/* subject			*/
	Imappart_t*	parts;		/* multipart parts		*/
	int		attachments;	/* attachment count		*/
} Imapmsg_t;

struct Imappart_s {			/* message part			*/
	Imappart_t*	next;		/* next part			*/
	short		content;	/* IMAP_CONTENT_* type		*/
	short		attachment;	/* attachment ordinal		*/
	int		lines;		/* # lines			*/
	size_t		size;		/* # octets			*/
	char*		id;		/* IMAP BODY[id]		*/
	char*		type;		/* MIME type/subtype		*/
	char*		encoding;	/* MIME encoding		*/
	char*		name;		/* attachment name		*/
	char*		options;	/* encoding options		*/
	Imapmsg_t*	msg;		/* IMAP_CONTENT_message info	*/
};

typedef struct {			/* imap_BODYSTRUCTURE discipline*/
	Imappart_t*	lastpart;	/* part list tail		*/
	int		prestack;	/* one before stack is ok	*/
	int		idstack[64];	/* for nested message ids	*/
	int*		id;		/* top of idstack		*/
} Imapbody_t;

struct Imapblock_s {			/* multiline response cache	*/
	Imapblock_t*	next;		/* next in list			*/
	size_t		length;		/* length of this block		*/
	char*		data;		/* data for this block		*/
};

typedef struct {			/* response info		*/
	int		state;		/* IMAP_OP_* state		*/
	int		code;		/* IMAP_* code			*/
	int		count;		/* optional count prefix	*/
	int		retain;		/* retain until imapfree()	*/
	char		msg[32];	/* first part of last message	*/
	Vmalloc_t*	vm;		/* local store			*/
	Imaplist_t	args;		/* arg list			*/
	Imapblock_t*	blocks;		/* multiline cache		*/
} Imapop_t;

typedef struct
{
	Imapop_t	op[8];		/* pending ops			*/
	Sfio_t*		mp;		/* tmp message stream		*/
	Sfio_t*		np;		/* tmp normalization stream	*/
	Sfio_t*		rp;		/* IMAP service recv stream	*/
	Sfio_t*		sp;		/* IMAP service send stream	*/
	Sfio_t*		tp;		/* tmp string stream		*/
	Vmdisc_t	vmdisc;		/* vmopen() discipline		*/
	Vmalloc_t*	gm;		/* IMAP global store		*/
	Vmalloc_t*	vm;		/* IMAP mailbox store		*/
	int		auth;		/* IMAP_AUTH_* methods		*/
	int		authenticated;	/* connection authenticated	*/
	int		connected;	/* connected to server		*/
	int		exiting;	/* attempting to logout		*/
	char*		host;		/* IMAP server host		*/
	Msg_t*		index;		/* last SEARCH message index	*/
	char*		meth;		/* IMAP authentication method	*/
	int		revision;	/* IMAP service revision	*/
	int		selected;	/* mailbox selected		*/
	int		tag;		/* last op tag (index)		*/
	char*		user;		/* IMAP user name		*/
	int		version;	/* IMAP service version		*/

	struct
	{
	Sfio_t*		fp;		/* output stream		*/
	char*		prefix;		/* line prefix			*/
	short		prefixlen;	/* line prefix length		*/
	short		emptylen;	/* empty line prefix length	*/
	}		copy;		/* imap_copy() state for FETCH	*/

	struct
	{
	int		delimiter;	/* mailbox hierarchy delimiter	*/
	int		exists;		/* # messages			*/
	int		recent;		/* first recent message		*/
	int		read_only;	/* no updates			*/
	int		trycreate;	/* op may work after CREATE	*/
	int		uidnext;	/* next expected message UID	*/
	int		uidvalidity;	/* UID validation		*/
	int		unseen;		/* first unseen message		*/
	}		mailbox;	/* mailbox state		*/
} Imap_t;

/*
 * imap dump <item> table
 */

#define IMAP_DUMP_FLAGS			1
#define IMAP_DUMP_FOLDER		2
#define IMAP_DUMP_MESSAGE		3
#define IMAP_DUMP_QUEUE			4
#define IMAP_DUMP_STATE			5

static const Imaplex_t	imapdump[] =
{
	"fl[ags]",			IMAP_DUMP_FLAGS,
	"f[older]",			IMAP_DUMP_FOLDER,
	"m[essage]",			IMAP_DUMP_MESSAGE,
	"q[ueue]",			IMAP_DUMP_QUEUE,
	"s[tate]",			IMAP_DUMP_STATE,
};

/*
 * operation states
 */

#define IMAP_STATE_free			0	/* must be 0 */
#define IMAP_STATE_sent			1
#define IMAP_STATE_more			2
#define IMAP_STATE_done			3

static const Imaplex_t	imapstate[] =
{
	"free",				IMAP_STATE_free,
	"sent",				IMAP_STATE_sent,
	"more",				IMAP_STATE_more,
	"done",				IMAP_STATE_done,
};

/*
 * part content types
 */

#define IMAP_CONTENT_data	0	/* must be 0			*/
#define IMAP_CONTENT_attachment	1	/* part is an attachment	*/
#define IMAP_CONTENT_header	2	/* part is a header		*/
#define IMAP_CONTENT_message	3	/* part is an embeded message	*/
#define IMAP_CONTENT_multipart	4	/* part is an embeded message	*/
#define IMAP_CONTENT_text	5	/* part is plain text		*/

static const Imaplex_t	imapcontent[] =
{
	"attachment",		IMAP_CONTENT_attachment,
	"data",			IMAP_CONTENT_data,
	"header",		IMAP_CONTENT_header,
	"message",		IMAP_CONTENT_message,
	"multipart",		IMAP_CONTENT_multipart,
	"text",			IMAP_CONTENT_text,
};

/*
 * message flags
 */

static const Imaplex_t	imapmflags[] =
{
	"DELETE",		MDELETE,
	"INIT",			MINIT,
	"MARK",			MMARK,
	"MBOX",			MBOX,
	"MODIFY",		MODIFY,
	"NEW",			MNEW,
	"NONE",			MNONE,
	"PRESERVE",		MPRESERVE,
	"READ",			MREAD,
	"SAVE",			MSAVE,
	"SCAN",			MSCAN,
	"SPAM",			MSPAM,
	"STATUS",		MSTATUS,
	"TOUCH",		MTOUCH,
	"USED",			MUSED,
	"ZOMBIE",		MZOMBIE,
};

/*
 * the rest define the parts of the protocol used by this implementation
 */

#define IMAP_UNKNOWN			0	/* must be 0 */
#define IMAP_OK				1
#define IMAP_NO				2
#define IMAP_BAD			3
#define IMAP_BYE			4
#define IMAP_CAPABILITY			5
#define IMAP_COPY			6
#define IMAP_EXISTS			7
#define IMAP_EXPUNGE			8
#define IMAP_FETCH			9
#define IMAP_FLAGS			10
#define IMAP_LIST			11
#define IMAP_LSUB			12
#define IMAP_MORE			13
#define IMAP_PREAUTH			14
#define IMAP_RECENT			15
#define IMAP_SEARCH			16

static const Imaplex_t	imapresponse[] =
{
	"+",				IMAP_MORE,
	"BAD",				IMAP_BAD,
	"BYE",				IMAP_BYE,
	"CAPABILITY",			IMAP_CAPABILITY,
	"COPY",				IMAP_COPY,
	"EXISTS",			IMAP_EXISTS,
	"EXPUNGE",			IMAP_EXPUNGE,
	"FETCH",			IMAP_FETCH,
	"FLAGS",			IMAP_FLAGS,
	"LIST",				IMAP_LIST,
	"LSUB",				IMAP_LSUB,
	"NO",				IMAP_NO,
	"OK",				IMAP_OK,
	"PREAUTH",			IMAP_PREAUTH,
	"RECENT",			IMAP_RECENT,
	"SEARCH",			IMAP_SEARCH,
	"STORE",			IMAP_FETCH,
	"UNKNOWN",			IMAP_UNKNOWN,
};

#define IMAP_AUTH_ANONYMOUS		(1<<0)
#define IMAP_AUTH_KERBEROS_V4		(1<<1)
#define IMAP_AUTH_LOGIN			(1<<2)
#define IMAP_AUTH_XFILE			(1<<3)

static const Imaplex_t	imapauth[] =
{
	"ANONYMOUS",			IMAP_AUTH_ANONYMOUS,
	"KERBEROS_V4",			IMAP_AUTH_KERBEROS_V4,
	"LOGIN",			IMAP_AUTH_LOGIN,
	"XFILE",			IMAP_AUTH_XFILE,
};

#define IMAP_STATUS_ALERT		1
#define IMAP_STATUS_NEWNAME		2
#define IMAP_STATUS_PARSE		3
#define IMAP_STATUS_PERMANENTFLAGS	4
#define IMAP_STATUS_READ_ONLY		5
#define IMAP_STATUS_READ_WRITE		6
#define IMAP_STATUS_TRYCREATE		7
#define IMAP_STATUS_UIDNEXT		8
#define IMAP_STATUS_UIDVALIDITY		9
#define IMAP_STATUS_UNSEEN		10

static const Imaplex_t	imapstatus[] =
{
	"ALERT",			IMAP_STATUS_ALERT,
	"NEWNAME",			IMAP_STATUS_NEWNAME,
	"PARSE",			IMAP_STATUS_PARSE,
	"PERMANENTFLAGS",		IMAP_STATUS_PERMANENTFLAGS,
	"READ-ONLY",			IMAP_STATUS_READ_ONLY,
	"READ-WRITE",			IMAP_STATUS_READ_WRITE,
	"TRYCREATE",			IMAP_STATUS_TRYCREATE,
	"UIDNEXT",			IMAP_STATUS_UIDNEXT,
	"UIDVALIDITY",			IMAP_STATUS_UIDVALIDITY,
	"UNSEEN",			IMAP_STATUS_UNSEEN,
};

#define IMAP_FETCH_BODY			1
#define IMAP_FETCH_BODYSTRUCTURE	2
#define IMAP_FETCH_ENVELOPE		3
#define IMAP_FETCH_FLAGS		4
#define IMAP_FETCH_INTERNALDATE		5
#define IMAP_FETCH_RFC822_SIZE		6
#define IMAP_FETCH_UID			7

static const Imaplex_t	imapfetch[] =
{
	"BODY",				IMAP_FETCH_BODY,
	"BODYSTRUCTURE",		IMAP_FETCH_BODYSTRUCTURE,
	"ENVELOPE",			IMAP_FETCH_ENVELOPE,
	"FLAGS",			IMAP_FETCH_FLAGS,
	"INTERNALDATE",			IMAP_FETCH_INTERNALDATE,
	"RFC822.SIZE",			IMAP_FETCH_RFC822_SIZE,
	"UID",				IMAP_FETCH_UID,
};

#define IMAP_FLAG_APPLICABLE		(1<<0)
#define IMAP_FLAG_PERMANENT		(1<<1)

#define IMAP_FLAG_ANSWERED		(1<<2)
#define IMAP_FLAG_DELETED		(1<<3)
#define IMAP_FLAG_DRAFT			(1<<4)
#define IMAP_FLAG_FLAGGED		(1<<5)
#define IMAP_FLAG_RECENT		(1<<6)
#define IMAP_FLAG_SCAN			(1<<7)
#define IMAP_FLAG_SEEN			(1<<8)
#define IMAP_FLAG_SPAM			(1<<9)

#define IMAP_FLAG_LOCAL			(IMAP_FLAG_SCAN|IMAP_FLAG_SPAM)

static Imapflag_t	imapflags[] =
{
	"/Scan",		IMAP_FLAG_SCAN,		MSCAN,
	"/Spam",		IMAP_FLAG_SPAM,		MSPAM,
	"\\*",			IMAP_FLAG_LOCAL,	0,
	"\\Answered",		IMAP_FLAG_ANSWERED,	0,
	"\\Deleted",		IMAP_FLAG_DELETED,	MDELETE|MZOMBIE,
	"\\Draft",		IMAP_FLAG_DRAFT,	0,
	"\\Flagged",		IMAP_FLAG_FLAGGED,	0,
	"\\Recent",		IMAP_FLAG_RECENT,	MNEW,
	"\\Seen",		IMAP_FLAG_SEEN,		MREAD,
};

static int	imapconnect(Imap_t*);

/*
 * return the name for the given code in table tab with n elements
 */

static char*
imapname(register const Imaplex_t* tab, int n, int code)
{
	register const Imaplex_t*	end;

	for (end = tab + n; tab < end; tab++)
		if (code == tab->code)
			return (char*)tab->name;
	return "[ERROR]";
}

/*
 * return the list of flag names set in flags in the string stream sp
 */

static char*
imapflagnames(register Sfio_t* sp, register const Imaplex_t* tab, int n, register int flags)
{
	register const Imaplex_t*	end;
	register int			sep;

	sep = 0;
	for (end = tab + n; tab < end; tab++)
		if (flags & tab->code)
		{
			if (sep)
				sfputc(sp, ' ');
			else
				sep = 1;
			sfputr(sp, tab->name, -1);
		}
	if (!sep)
		sfputc(sp, '0');
	return struse(sp);
}

/*
 * list arg value on sfstdout
 */

static void
imapdumparg(Imap_t* imap, Imaparg_t* ap, int level)
{
	switch (ap->type)
	{
	case IMAP_data:
		sfprintf(sfstdout, " {%d}%.*s", ap->value.data.length, ap->value.data.length, ap->value.data.buffer);
		break;
	case IMAP_list:
		if (ap = ap->value.list.head)
		{
			sfprintf(sfstdout, " (");
			do {
				imapdumparg(imap, ap, level + 1);
			} while (ap = ap->next);
			sfprintf(sfstdout, " )");
		}
		else
			sfprintf(sfstdout, " NIL");
		break;
	case IMAP_name:
		sfprintf(sfstdout, " %s", ap->value.name);
		break;
	case IMAP_number:
		sfprintf(sfstdout, " %ld", ap->value.number);
		break;
	case IMAP_string:
		sfprintf(sfstdout, " \"%s\"", ap->value.string);
		break;
	}
	if (!level)
		sfprintf(sfstdout, "\n");
}

/*
 * set mailbox flag capabilities
 */

static void
imapset(Imap_t* imap, register Imaparg_t* ap, int code)
{
	register int		i;
	register Imapflag_t*	fp;

	for (i = 0; i < elementsof(imapflags); i++)
		imapflags[i].code &= ~code;
	for (; ap; ap = ap->next)
		if (ap->type == IMAP_name && (fp = (Imapflag_t*)strsearch(imapflags, elementsof(imapflags), sizeof(imapflags[0]), stracmp, ap->value.name, NiL)))
		{
			if (fp->code == IMAP_FLAG_LOCAL)
				for (i = 0; i < elementsof(imapflags); i++)
					if (imapflags[i].code & IMAP_FLAG_LOCAL)
						imapflags[i].code |= code;
			fp->code |= code;
		}
}

/*
 * parse a response arg
 */

static char*
imapgetarg(Imap_t* imap, register Imapop_t* op, register Imaplist_t* lp, int* ep, register char* s)
{
	register int		c;
	register char*		b;
	register Imaparg_t*	ap;
	register int		eol;
	int			m;
	char*			e;

	eol = ep ? *ep : 0;
	for (;;)
	{
		for (; isspace(*s); s++);
		if (*s)
		{
			if (*s == eol)
			{
				*ep = 0;
				return s + 1;
			}
			if (!(ap = vmnewof(op->vm, 0, Imaparg_t, 1, 0)))
				note(FATAL|SYSTEM, "Out of space [imap token]");
			if (lp->tail)
				lp->tail->next = ap;
			else
				lp->head = ap;
			lp->tail = ap;
			if (!eol && op->code <= IMAP_BYE && *s != '[')
			{
				ap->type = IMAP_string;
				ap->value.string = s;
				return 0;
			}
			switch (*s)
			{
			case '{':
				ap->type = IMAP_data;
				ap->value.data.length = strtol(s + 1, &e, 10);
				if (*e != '}' || !op->blocks || !(op->blocks = op->blocks->next))
					goto name;
				ap->value.data.buffer = op->blocks->data;
				return e + 1;
			case '"':
				for (b = ++s; *s && *s != '"'; s++);
				if (*s)
					*s++ = 0;
				ap->type = IMAP_string;
				ap->value.string = b;
				return s;
			case '[':
				m = ']';
				goto list;
			case '(':
				m = ')';
			list:
				ap->type = IMAP_list;
				ap->value.list.head = ap->value.list.tail = 0;
				s++;
				while ((s = imapgetarg(imap, op, &ap->value.list, &m, s)) && m);
				return s;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ap->type = IMAP_number;
				ap->value.number = strtol(s, &e, 10);
				return e;
			case 'N':
				if (*(s + 1) == 'I' && *(s + 2) == 'L' && !isalnum(*(s + 3)))
				{
					ap->type = IMAP_list;
					ap->value.list.head = 0;
					return s + 3;
				}
				/*FALLTHROUGH*/
			default:
			name:
				m = 0;
				for (b = s; (c = *s); s++)
					if (c == '[')
						m = c;
					else if (c == ']')
						m = 0;
					else if (m == 0)
					{
						if (c == eol)
						{
							*ep = 0;
							break;
						}
						else if (isspace(c))
							break;
					}
				if (c)
					*s++ = 0;
				ap->type = IMAP_name;
				ap->value.name = b;
				return s;
			}
		}
		if (!op->blocks || !(op->blocks = op->blocks->next))
			break;
		s = op->blocks->data;
	}
	return 0;
}

/*
 * output an arg value converting \r\n => \n
 */

static void
imapputarg(register Imap_t* imap, register Imaparg_t* ap)
{
	register char*	e;
	register char*	s;
	register char*	t;
	register int	n;

	switch (ap->type)
	{
	case IMAP_data:
		t = s = ap->value.data.buffer;
		e = s + ap->value.data.length;
		while (t = (char*)memchr(t, '\r', e - t))
		{
			if (++t >= e)
				break;
			if (*t != '\n')
				continue;
			*(t - 1) = '\n';
			n = t - s;
			if (imap->copy.prefix)
				sfwrite(imap->copy.fp, imap->copy.prefix, n > 1 ? imap->copy.prefixlen : imap->copy.emptylen);
			sfwrite(imap->copy.fp, s, n);
			if ((s = ++t) >= e)
				break;
		}
		if ((n = e - s) > 0)
		{
			if (imap->copy.prefix)
				sfwrite(imap->copy.fp, imap->copy.prefix, n > 1 ? imap->copy.prefixlen : imap->copy.emptylen);
			sfwrite(imap->copy.fp, s, e - s);
		}
		break;
	case IMAP_string:
		sfputr(imap->copy.fp, ap->value.string, '\n');
		break;
	}
}

/*
 * read and parse an IMAP server response
 */

static Imapop_t*
imapop(register Imap_t* imap)
{
	register char*		s;
	register char*		z;
	register size_t		n;
	register Imapop_t*	op;
	register Imapblock_t*	bp;
	register Imapblock_t*	bt;
	char*			b;
	char*			e;
	Imapblock_t*		bh;
	Imaplex_t*		xp;

	/*
	 * read the first (possibly only) line of the response
	 */

	if (!(s = sfgetr(imap->rp, '\n', 1)))
		return 0;
	if ((z = s + sfvalue(imap->rp)) > s && --z > s && *(z - 1) == '\r')
		*--z = 0;
	if (TRACING('m'))
		note(ERROR, "imap: mesg %s", s);

	/*
	 * tag or *
	 */

	for (; isspace(*s); s++);
	if (*s == '*')
	{
		for (; *++s && !isspace(*s););
		op = imap->op;
		op->state = IMAP_STATE_sent;
	}
	else
	{
		n = (int)strtol(s, &e, 10);
		s = e;
		op = imap->op + n;
	}
	if (op->state != IMAP_STATE_sent)
		note(ERROR, "imap: %d: internal error -- unknown operation", n);

	/*
	 * clear/allocate the local response vm
	 */

	if (op->vm)
		vmclear(op->vm);
	else if (!(op->vm = vmopen(&imap->vmdisc, Vmlast, 0)))
		note(FATAL|SYSTEM, "Out of space [imap vm]");

	/*
	 * optional count
	 */

	for (; isspace(*s); s++);
	if (isdigit(*s))
	{
		n = (int)strtol(s, &e, 10);
		for (s = e; isspace(*s); s++);
	}
	else
		n = 0;

	/*
	 * response code
	 */

	for (b = s; *s && !isspace(*s); s++);
	if (*s)
		for (*s++ = 0; isspace(*s); s++);
	op->code = (xp = (Imaplex_t*)strsearch(imapresponse, elementsof(imapresponse), sizeof(Imaplex_t), stracmp, b, NiL)) ? xp->code : IMAP_UNKNOWN;
	op->count = n;
	if (TRACING('r'))
		note(ERROR, "imap: recv %d %s %d %s", op - imap->op, imapname(imapresponse, elementsof(imapresponse), op->code), op->count, s);

	/*
	 * if the message has imbedded literals there's
	 * more input to consume and we have to cache what's
	 * already been read
	 */

	bt = 0;
	while (z > s && *(z - 1) == '}')
	{
		n = z - s + 1;
		if (!(bp = vmnewof(op->vm, 0, Imapblock_t, 1, n)))
			note(FATAL|SYSTEM, "Out of space [imap block]");
		memcpy(bp->data = (char*)(bp + 1), s, n);
		if (bt)
			bt->next = bp;
		else
			bh = bp;
		bt = bp;
		for (z -= 2; z > s && *z != '{'; z--);
		n = (size_t)strtol(z + 1, NiL, 10);
		if (!(bp = vmnewof(op->vm, 0, Imapblock_t, 1, n)))
			note(FATAL|SYSTEM, "Out of space [imap block]");
		if ((bp->length = n) && sfread(imap->rp, bp->data = (char*)(bp + 1), n) != n)
			return 0;
		bt = bt->next = bp;
		if (!(s = sfgetr(imap->rp, '\n', 1)))
			return 0;
		if ((z = s + sfvalue(imap->rp)) > s && --z > s && *(z - 1) == '\r')
			*--z = 0;
		if (TRACING('m'))
			note(ERROR, "imap: mesg %s", s);
	}
	if (bt)
	{
		if (!(bp = vmnewof(op->vm, 0, Imapblock_t, 1, n)))
			note(FATAL|SYSTEM, "Out of space [imap block]");
		bp->length = z - s;
		bp->data = s;
		bt->next = bp;
		s = bh->data;
	}
	else
		bh = 0;
	op->blocks = bh;
	op->args.head = op->args.tail = 0;
	while (s = imapgetarg(imap, op, &op->args, NiL, s));
	return op;
}

/*
 * handle EXPUNGE response
 */

static void
imap_EXPUNGE(Imap_t* imap, register Imapop_t* op)
{
}

/*
 * consume ENVELOPE response
 */

static void
imap_ENVELOPE(register Imap_t* imap, register Imaparg_t* vp, Msg_t* mp)
{
	register Imapmsg_t*	ip;
	register char*		s;

	ip = (Imapmsg_t*)mp->m_info;
	if (!vp)
		return;
	if (s = vp->value.string)
	{
		s = fmttime("%a %b %e %H:%M %Z %Y", tmdate(s, NiL, NiL));
		if (ip->date)
			strcpy(ip->date, s);
		else
			ip->date = vmstrdup(imap->vm, s);
	}
	if (!(vp = vp->next))
		return;
	if (s = vp->value.string)
		ip->subject = vmstrdup(imap->vm, s);
	if (!(vp = vp->next) ||
	    !(vp = vp->value.list.head) ||
	    !(vp = vp->value.list.head) ||
	    !(vp = vp->next) ||
	    !(vp = vp->next))
		return;
	ip->from = vp->value.string;
	if (!(vp = vp->next))
		return;
	if ((s = vp->value.string) && state.var.local)
		s = localize(s);
	if (s && *s)
	{
		sfprintf(imap->tp, "%s@%s", ip->from, s);
		ip->from = struse(imap->tp);
	}
	ip->from = vmstrdup(imap->vm, ip->from);
}

/*
 * consume BODYSTRUCTURE response
 */

static void
imap_BODYSTRUCTURE(register Imap_t* imap, register Imaparg_t* ap, register Msg_t* mp, register Imapbody_t* bp)
{
	register char*		s;
	register Imappart_t*	pp;
	register Imaparg_t*	vp;
	int*			id;

	if (!ap)
		return;
	if (!(pp = vmnewof(imap->vm, 0, Imappart_t, 1, 0)))
		note(FATAL, "out of space [imap part]");
	if (bp->lastpart)
		bp->lastpart->next = pp;
	else
		((Imapmsg_t*)mp->m_info)->parts = pp;
	bp->lastpart = pp;
	(*bp->id)++;
	if (ap->type == IMAP_list)
	{
		pp->content = IMAP_CONTENT_multipart;
		if (bp->id >= &bp->idstack[elementsof(bp->idstack) - 1])
			note(FATAL, "imap: multipart nesting too deep -- %d max", elementsof(bp->idstack));
		*++bp->id = 0;
		do {
			imap_BODYSTRUCTURE(imap, ap->value.list.head, mp, bp);
		} while ((ap = ap->next) && ap->type == IMAP_list);
		bp->id--;
		sfputr(imap->tp, "multipart", -1);
		if (ap && ap->type == IMAP_string)
			sfprintf(imap->tp, "/%s", strlower(ap->value.string));
		pp->type = vmstrdup(imap->vm, struse(imap->tp));
		pp->id = "";
	}
	else
	{
		id = bp->idstack;
		sfprintf(imap->tp, "%d", *id);
		while (++id <= bp->id)
			sfprintf(imap->tp, ".%d", *id);
		pp->id = vmstrdup(imap->vm, struse(imap->tp));
		if (s = ap->value.string)
		{
			s = strlower(s);
			if (streq(s, "text"))
				pp->content = IMAP_CONTENT_text;
			else if (streq(s, "message"))
				pp->content = IMAP_CONTENT_message;
			else
				pp->content = IMAP_CONTENT_data;
			sfputr(imap->tp, s, -1);
		}
		if ((ap = ap->next) && (s = ap->value.string))
			sfprintf(imap->tp, "/%s", strlower(s));
		pp->type = vmstrdup(imap->vm, struse(imap->tp));
		if (!ap || !(ap = ap->next))
			return;
		if (vp = ap->value.list.head)
			do {
				s = vp->value.string;
				if (!(vp = vp->next))
					break;
				if (s && streq(s, "NAME") && vp->value.string)
				{
					pp->name = vmstrdup(imap->vm, vp->value.string);
					break;
				}
			} while (vp = vp->next);
		if (!(ap = ap->next) ||
		    !(ap = ap->next) ||
		    !(ap = ap->next))
			return;
		if (ap->type == IMAP_string && ap->value.string)
			pp->encoding = vmstrdup(imap->vm, strlower(ap->value.string));
		if (!(ap = ap->next))
			return;
		pp->size = ap->value.number;
		if (!(ap = ap->next))
			return;
		switch (pp->content)
		{
		case IMAP_CONTENT_message:
			if (bp->id >= &bp->idstack[elementsof(bp->idstack) - 1])
				note(FATAL, "imap: message nesting too deep -- %d max", elementsof(bp->idstack));
			*++bp->id = 0;
			imap_ENVELOPE(imap, ap->value.list.head, mp);
			ap = ap->next;
			imap_BODYSTRUCTURE(imap, ap->value.list.head, mp, bp);
			bp->id--;
			break;
		case IMAP_CONTENT_text:
			pp->lines = ap->value.number;
			if (!pp->name)
			{
				if (!(ap = ap->next) || !(ap = ap->next))
					return;
				if (ap->type == IMAP_list &&
				    (ap = ap->value.list.head) &&
				    (s = ap->value.string) &&
				    streq(s, "ATTACHMENT") &&
				    (ap = ap->next) &&
				    (ap = ap->value.list.head))
					while ((s = ap->value.name) &&
					       (ap = ap->next))
					{
						if (streq(s, "FILENAME"))
						{
							pp->name = vmstrdup(imap->vm, ap->value.string);
							break;
						}
						if (!(ap = ap->next))
							break;
					}
			}
			break;
		case IMAP_CONTENT_data:
			if (!pp->name)
			{
				sfprintf(imap->tp, "%d.att", ((Imapmsg_t*)mp->m_info)->attachments + 1);
				pp->name = vmstrdup(imap->vm, struse(imap->tp));
			}
			break;
		}
		if (pp->name)
		{
			mp->m_lines += 2;
			pp->content = IMAP_CONTENT_attachment;
			pp->attachment = ++((Imapmsg_t*)mp->m_info)->attachments;
		}
		else
			mp->m_lines += pp->lines;
	}
}

/*
 * handle FETCH response
 */

static void
imap_FETCH(Imap_t* imap, register Imapop_t* op)
{
	register Msg_t*		mp;
	register Imaparg_t*	ap;
	register Imaparg_t*	vp;
	register Imapmsg_t*	ip;
	Imaplex_t*		xp;
	Imapflag_t*		fp;
	char*			s;
	Imapbody_t		body;

	if (op->count > state.msg.count)
		note(FATAL, "imap: %d: unknown message -- expected %d max", op->count, state.msg.count);
	mp = state.msg.list + op->count - 1;
	ip = (Imapmsg_t*)mp->m_info;
	if (ap = op->args.head)
	{
		if (ap->type == IMAP_list)
			ap = ap->value.list.head;
		while (ap)
		{
			if (ap->type != IMAP_name)
				xp = 0;
			else if (!(xp = (Imaplex_t*)strsearch(imapfetch, elementsof(imapfetch), sizeof(Imaplex_t), stracmp, ap->value.name, NiL)))
			{
				if (s = strchr(ap->value.name, '['))
				{
					*s = 0;
					xp = (Imaplex_t*)strsearch(imapfetch, elementsof(imapfetch), sizeof(Imaplex_t), stracmp, ap->value.name, NiL);
					*s = '[';
				}
				if ((TRACING('u')) && !xp)
					note(ERROR, "imap: %s: unknown response", ap->value.name);
			}
			if (ap = ap->next)
			{
				if (xp) switch (xp->code)
				{
				case IMAP_FETCH_BODY:
					imapputarg(imap, ap);
					break;
				case IMAP_FETCH_BODYSTRUCTURE:
					memset(&body, 0, sizeof(body));
					body.id = body.idstack - 1;
					*body.idstack = 1;
					imap_BODYSTRUCTURE(imap, ap->value.list.head, mp, &body);
					break;
				case IMAP_FETCH_ENVELOPE:
					imap_ENVELOPE(imap, ap->value.list.head, mp);
					break;
				case IMAP_FETCH_FLAGS:
					for (vp = ap->value.list.head; vp; vp = vp->next)
						if (vp->type == IMAP_name && (fp = (Imapflag_t*)strsearch(imapflags, elementsof(imapflags), sizeof(imapflags[0]), stracmp, vp->value.name, NiL)))
							mp->m_flag |= fp->flag;
					break;
				case IMAP_FETCH_INTERNALDATE:
					if (!ip->date && (s = ap->value.string))
						ip->date = vmstrdup(imap->vm, fmttime("%a %b %e %H:%M %Z %Y", tmdate(s, NiL, NiL)));
					break;
				case IMAP_FETCH_RFC822_SIZE:
					mp->m_size = ap->value.number;
					break;
				}
				else if (TRACING('u'))
					note(ERROR, "imap: %s: unknown FETCH response", ap->value.name);
				ap = ap->next;
			}
		}
	}
}

/*
 * sync the state.msg.list from the server
 */

static void
imapsync(register Imap_t* imap)
{
	register Msg_t*	mp;
	int		dot;

	dot = state.msg.dot ? (state.msg.dot - state.msg.list) : 0;
	while (state.msg.count < imap->mailbox.exists)
	{
		mp = newmsg(0);
		if (state.msg.count < imap->mailbox.unseen || !imap->mailbox.unseen)
			mp->m_flag = MUSED|MREAD;
	}
	if (imap->mailbox.unseen < 0)
		imap->mailbox.unseen = 1;
	else if (imap->mailbox.unseen > state.msg.count)
		imap->mailbox.unseen = state.msg.count;
	state.msg.dot = state.msg.list + dot;
}

/*
 * consume IMAP server info until tag is complete
 * tag<0 for any tag completed
 */

static Imapop_t*
imaprecv(register Imap_t* imap, register Imapop_t* wp)
{
	register int		n;
	register Imapop_t*	op;
	register Imaparg_t*	ap;
	register Imaparg_t*	sp;
	Imaplex_t*		xp;
	int			to;
	int			i;
	char*			s;

	if (!wp || wp == imap->op)
	{
		for (op = imap->op + 1;; op++)
			if (op >= &imap->op[elementsof(imap->op)])
				return 0;
			else if (op->state == IMAP_STATE_sent)
				break;
	}
	if (!wp)
		to = 0;
	else if (wp->state != IMAP_STATE_sent && wp != imap->op)
	{
		if (wp->code)
			return wp;
		note(ERROR, "imap: %d: operation never sent", wp - imap->op);
		return 0;
	}
	else
		to = -1;
	for (;;)
	{
		switch (sfpoll(&imap->rp, 1, to))
		{
		case 0:
			if (wp)
				note(ERROR, "imap: %d: operation not completed", wp - imap->op);
			break;
		case 1:
			if (op = imapop(imap))
			{
				if (TRACING('a'))
				{
					note(ERROR, "imap: exec op %d count %d", op->code, op->count);
					for (ap = op->args.head; ap; ap = ap->next)
						imapdumparg(imap, ap, 0);
				}
				switch (op->code)
				{
				case IMAP_BAD:
				case IMAP_NO:
					n = 1;
					goto status;
				case IMAP_BYE:
					n = !imap->exiting;
					imap->connected = 0;
					wp = op;
					goto status;
				case IMAP_CAPABILITY:
					for (ap = op->args.head; ap; ap = ap->next)
						if (ap->type == IMAP_name)
						{
							s = ap->value.name;
							/*UNDENT...*/

	if (strneq(s, "IMAP", 4))
	{
		s += 4;
		if ((i = (int)strtol(s, &s, 10)) >= imap->version)
		{
			if (i > imap->version)
				imap->revision = 0;
			imap->version = i;
			for (; *s && !isdigit(*s); s++);
			i = (int)strtol(s, NiL, 10);
			if (i > imap->revision)
				imap->revision = i;
		}
	}
	else if (strneq(s, "AUTH=", 5))
		if (xp = (Imaplex_t*)strsearch(imapauth, elementsof(imapauth), sizeof(Imaplex_t), stracmp, s + 5, NiL))
			imap->auth |= xp->code;

							/*...INDENT*/
						}
					break;
				case IMAP_EXISTS:
					if (imap->mailbox.exists < 0)
						imap->mailbox.exists = op->count;
					else
					{
						n = op->count - imap->mailbox.exists;
						if (n > 0)
							note(ERROR, "(NOTE %d new message%s incorporated)", n, n == 1 ? "" : "s");
						else if (n < 0)
						{
							n = -n;
							note(ERROR, "(NOTE %d message%s delected by another session)", n, n == 1 ? "" : "s");
							vmclear(imap->vm);
							state.msg.count = 0;
						}
						imap->mailbox.exists = op->count;
						imapsync(imap);
					}
					break;
				case IMAP_EXPUNGE:
					imap_EXPUNGE(imap, op);
					break;
				case IMAP_FETCH:
					imap_FETCH(imap, op);
					break;
				case IMAP_FLAGS:
					if ((ap = op->args.head) && ap->type == IMAP_list)
						imapset(imap, ap->value.list.head, IMAP_FLAG_APPLICABLE);
					break;
				case IMAP_LIST:
					ap = op->args.head;
					ap = ap->next;
					ap = ap->next;
					sfprintf(sfstdout, "@%s\n", ap->value.string);
					break;
				case IMAP_OK:
					n = 0;
				status:
					/*
					 * check for status
					 */
					if (ap = op->args.head)
					{
						if (ap->type == IMAP_list && (sp = ap->value.list.head) && sp->type == IMAP_name && (xp = (Imaplex_t*)strsearch(imapstatus, elementsof(imapstatus), sizeof(Imaplex_t), stracmp, sp->value.name, NiL)))
						{
							/*UNDENT...*/

	ap = ap->next;
	sp = sp->next;
	switch (xp->code)
	{
	case IMAP_STATUS_ALERT:
	case IMAP_STATUS_NEWNAME:
	case IMAP_STATUS_PARSE:
		n = 1;
		break;
	case IMAP_STATUS_PERMANENTFLAGS:
		if (sp && sp->type == IMAP_list)
			imapset(imap, sp->value.list.head, IMAP_FLAG_PERMANENT);
		break;
	case IMAP_STATUS_READ_ONLY:
		imap->mailbox.read_only = 1;
		break;
	case IMAP_STATUS_READ_WRITE:
		imap->mailbox.read_only = 0;
		break;
	case IMAP_STATUS_TRYCREATE:
		imap->mailbox.trycreate = 1;
		break;
	case IMAP_STATUS_UIDNEXT:
		imap->mailbox.uidnext = sp && sp->type == IMAP_number ? sp->value.number : 0;
		break;
	case IMAP_STATUS_UIDVALIDITY:
		imap->mailbox.uidvalidity = sp && sp->type == IMAP_number ? sp->value.number : 0;
		break;
	case IMAP_STATUS_UNSEEN:
		imap->mailbox.unseen = sp && sp->type == IMAP_number ? sp->value.number : 0;
		break;
	}

							/*...INDENT*/
							}
						if (n && ap && ap->type == IMAP_string)
							note(ERROR, "imap: %s", ap->value.string);
					}
					break;
				case IMAP_PREAUTH:
					imap->authenticated = 1;
					break;
				case IMAP_RECENT:
					imap->mailbox.recent = op->count;
					break;
				case IMAP_SEARCH:
					for (ap = op->args.head; ap; ap = ap->next)
						if (ap->type == IMAP_number)
						{
							imap->index->m_index = ap->value.number;
							imap->index++;
						}
					break;
				}
				if (!op->retain)
					imapfree(imap, op);
				if (wp == op || wp == imap->op)
					return op;
			}
			continue;
		default:
			note(ERROR, "imap: lost server connection");
			break;
		}
		break;
	}
	return 0;
}

/*
 * va_list version of imapsend
 */

static Imapop_t*
imapvsend(register Imap_t* imap, int retain, const char* fmt, va_list ap)
{
	register int	i;
	register int	c;
	register int	q;
	register char*	s;
	va_list		oap;

	va_copy(oap, ap);

	/*
	 * autologout may have dropped the connection
	 * the autoreconnect is here
	 */

	if (!imap->connected && imapconnect(imap))
		return 0;

	/*
	 * grab a free tag
	 * this might mean waiting for a previous one to finish
	 */

	if (!(i = imap->tag))
		i = 1;
	c = i;
	while (imap->op[i].state)
	{
		if (++i >= elementsof(imap->op))
			i = 1;
		if (i == c)
			imaprecv(imap, imap->op);
	}
	imap->tag = i;

	/*
	 * write the op
	 */

	sfprintf(imap->np, "%d ", i);
	sfvprintf(imap->np, fmt, ap);
	s = struse(imap->np);
	q = 0;
	for (;;)
	{
		switch (c = *s++)
		{
		case 0:
			break;
		case '"':
		case '\'':
			sfputc(imap->tp, c);
			if (!q)
				q = c;
			continue;
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			if (!q)
			{
				for (; (c = *s) && (c == ' ' || c == '\n' || c == '\r' || c == '\t'); s++);
				if (c && c != ')')
					sfputc(imap->tp, ' ');
			}
			else
				sfputc(imap->tp, c);
			continue;
		case '(':
			sfputc(imap->tp, c);
			if (!q)
				for (; (c = *s) && (c == ' ' || c == '\n' || c == '\r' || c == '\t'); s++);
			continue;
		case ')':
			sfputc(imap->tp, c);
			if (!q)
			{
				for (; (c = *s) && (c == ' ' || c == '\n' || c == '\r' || c == '\t'); s++);
				if (c && c != ']')
					sfputc(imap->tp, ' ');
			}
			continue;
		default:
			sfputc(imap->tp, c);
			continue;
		}
		break;
	}
	s = struse(imap->tp);
	if (TRACING('s'))
		note(ERROR, "imap: send %s", s);
	if (sfprintf(imap->sp, "%s\r\n", s) < 0 || sfsync(imap->sp) < 0)
	{
		note(ERROR|SYSTEM, "imap: server send error");
		i = 0;
	}
	if (!i)
		return 0;
	imap->op[i].state = IMAP_STATE_sent;
	imap->op[i].retain = retain;
	sfvsprintf(imap->op[i].msg, sizeof(imap->op[i].msg) - 1, fmt, oap);
	return imap->op + i;
}

/*
 * send a tagged imap op
 */

static Imapop_t*
imapsend(Imap_t* imap, int retain, const char* fmt, ...)
{
	Imapop_t*	op;
	va_list		ap;

	va_start(ap, fmt);
	op = imapvsend(imap, retain, fmt, ap);
	va_end(ap);
	return op;
}

/*
 * send and recv a tagged imap op
 */

static int
imapexec(Imap_t* imap, const char* fmt, ...)
{
	register Imapop_t*	op;
	va_list			ap;

	va_start(ap, fmt);
	op = imapvsend(imap, 1, fmt, ap);
	va_end(ap);
	if (!op || !imaprecv(imap, op))
		return 0;
	imapfree(imap, op);
	return op->code == IMAP_OK ? 0 : -1;
}

/*
 * connect and authenticate to the IMAP service
 */

static int
imapconnect(register Imap_t* imap)
{
	register char*	svc;
	int		fd;

	/*
	 * imap->connected<0 means we got here from one of
	 * the imapexec()'s in this function -- not good
	 */

	if (imap->connected < 0)
	{
		note(FATAL, "imap: connection/authentication error");
		return -1;
	}

	/*
	 * the old connection if any is useless now
	 */

	if (imap->sp)
	{
		sfclose(imap->sp);
		imap->sp = 0;
	}
	if (imap->rp)
	{
		sfclose(imap->rp);
		imap->rp = 0;
	}
	if (!imap->host || !imap->user || !imap->meth)
	{
		register char*	host;
		register char*	user;
		register char*	meth;
		Sfio_t*		fp;

		/*
		 * get the host and authentication info from the imap file
		 *
		 *	<host> [ <user> [ <meth> [ <arg> ... ] ] ]
		 */

		if (fp = fileopen(state.var.imap, "IRXr"))
		{
			while (host = sfgetr(fp, '\n', 1))
			{
				for (; isspace(*host); host++);
				if (!*host || *host == '#')
					continue;
				for (user = host; *user && !isspace(*user); user++);
				if (*user)
					*user++ = 0;
				if (!imap->host || streq(imap->host, host))
				{
					for (; isspace(*user); user++);
					for (meth = user; *meth && !isspace(*meth); meth++);
					if (*meth)
						*meth++ = 0;
					if (!imap->user || streq(imap->user, user))
					{
						for (; isspace(*meth); meth++);
						goto match;
					}
				}
			}
			host = 0;
			user = 0;
			meth = 0;
		}
		else
		{
			host = imap->host;
			user = imap->user;
			meth = imap->meth;
		}
	match:
		if (host && (!*host || streq(host, "*")))
			host = 0;
		if (!imap->host || host && !streq(host, imap->host))
			imap->host = host ? vmstrdup(imap->gm, host) : "local";
		if (user && (!*user || streq(user, "*")))
			user = 0;
		if (!imap->user || user && !streq(user, imap->user))
			imap->user = user ? vmstrdup(imap->gm, user) : state.var.user;
		if (meth && (!*meth || streq(meth, "*")))
			meth = 0;
		if (!imap->meth || meth && !streq(meth, imap->meth))
		{
			if (!meth)
			{
				if (streq(imap->user, "anonymous"))
				{
					sfprintf(imap->tp, "LOGIN ANONYMOUS %s", state.var.user);
					if (state.var.domain)
						sfprintf(imap->tp, "@%s", state.var.domain);
					meth = struse(imap->tp);
				}
				else
					meth = "AUTHENTICATE XFILE";
			}
			imap->meth = vmstrdup(imap->gm, meth);
		}
		if (fp)
			sfclose(fp);
	}

	/*
	 * connect to the service
	 */

	sfprintf(imap->tp, "/dev/tcp/%s/inet.imap", imap->host);
	svc = struse(imap->tp);
	if ((fd = csopen(&cs, svc, 0)) < 0)
	{
		note(ERROR|SYSTEM, "imap: %s: cannot connect to service", svc);
		return -1;
	}
	if (!(imap->sp = sfnew(NiL, NiL, SF_UNBOUND, fd, SF_WRITE)) ||
	    !(imap->rp = sfnew(NiL, NiL, SF_UNBOUND, fd, SF_READ)))
	{
		if (imap->sp)
		{
			sfclose(imap->sp);
			imap->sp = 0;
		}
		else
			close(fd);
		note(ERROR|SYSTEM, "imap: %s: cannot buffer service", svc);
		return -1;
	}
	imap->connected = -1;
	if (imapexec(imap, "CAPABILITY") || imap->version < IMAP_VERSION || imap->version == IMAP_VERSION && imap->revision < IMAP_REVISION)
	{
		if (imap->version)
			note(ERROR|SYSTEM, "imap: %s: service version %d.%d must be at least %d.%d", svc, imap->version, imap->revision, IMAP_VERSION, IMAP_REVISION);
		else
			note(ERROR|SYSTEM, "imap: %s: service connect error", svc);
		goto drop;
	}
	if (!imap->authenticated)
	{
		/*
		 * do the authentication
		 * imap->authenticated<0 is a hint to the
		 * IMAP_MORE response to consult imap->meth
		 */

		imap->authenticated = -1;
		if (imapexec(imap, "%s", imap->meth))
		{
			note(ERROR|SYSTEM, "imap: %s: service authentication error", svc);
			goto drop;
		}
		imap->authenticated = 1;
	}
	imap->connected = 1;
	return 0;
 drop:
	if (imap->sp)
	{
		sfclose(imap->sp);
		imap->sp = 0;
	}
	if (imap->rp)
	{
		sfclose(imap->rp);
		imap->rp = 0;
	}
	imap->connected = 0;
	return -1;
}

/*
 * initialize the IMAP service state
 */

static Imap_t*
imapinit(void)
{
	register Imap_t*	imap;

	if (!(imap = newof(0, Imap_t, 1, 0)))
	{
		note(ERROR|SYSTEM, "Out of space [imap]");
		return 0;
	}
	memfatal(&imap->vmdisc);
	if (!(imap->gm = vmopen(&imap->vmdisc, Vmlast, 0)) || !(imap->vm = vmopen(&imap->vmdisc, Vmlast, 0)))
	{
		note(ERROR|SYSTEM, "Out of space [imap state vm]");
		if (imap->gm)
			vmclose(imap->gm);
		free(imap);
		return 0;
	}
	if (!(imap->mp = sfstropen()) || !(imap->tp = sfstropen()) || !(imap->np = sfstropen()))
	{
		note(ERROR|SYSTEM, "Out of space [imap tmp string stream]");
		vmclose(imap->gm);
		vmclose(imap->vm);
		free(imap);
		return 0;
	}
	imap->copy.fp = sfstdout;
	return state.msg.imap.state = (void*)imap;
}

/*
 * flush IMAP_STATE_sent requests and CLOSE
 */

static int
imapclose(register Imap_t* imap)
{
	register Msg_t*	mp;
	register int	n;

	while (imaprecv(imap, imap->op));
	imap->selected = 0;
	if (state.folder == FIMAP)
	{
		n = 0;
		for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++)
			if (!(mp->m_flag & MDELETE))
				n++;
		if (n)
			note(ERROR, "Held %d message%s in %s", n, n == 1 ? "" : "s", state.path.mail);
		else
			note(ERROR, "No more messages in %s", state.path.mail);
	}
	return imapexec(imap, "CLOSE");
}

/*
 * IMAP state.msg.list initialization
 */

int
imap_setptr(char* name, int isedit)
{
	register Imap_t*	imap;
	register char*		s;
	register char*		t;

	if (!(imap = IMAP) && (!name || !(imap = imapinit())))
		return -1;
	if (imap->selected && imapclose(imap))
		return -1;
	if (!name)
		return 0;
	vmclear(imap->vm);
	state.folder = FIMAP;
	memset(&imap->mailbox, sizeof(imap->mailbox), 0);
	imap->mailbox.exists = -1;

	/*
	 * determine the service info
	 * info from previous folders is cached
	 * to preserve authentication
	 * ${IMAP} provides defaults
	 */

	if (streq(name, "@"))
		name = "@inbox";
	strncopy(state.path.prev, state.path.mail, sizeof(state.path.prev));
	if (name != state.path.mail)
		strncopy(state.path.mail, name, sizeof(state.path.mail));
	if (*name == '@')
		name++;
	else if (strneq(name, "imap://", 7))
	{
		sfprintf(imap->tp, "%s", name + 7);
		s = struse(imap->tp);
		if (!(t = strchr(s, '/')))
		{
			note(ERROR, "imap: %s: user name expected", name);
			return -1;
		}
		*t++ = 0;
		if (!imap->host || !streq(imap->host, s))
		{
			imap->host = vmstrdup(imap->gm, s);
			imap->connected = 0;
			imap->meth = 0;
		}
		if (s = strchr(t, '/'))
			*s++ = 0;
		if (!imap->user || !streq(imap->user, t))
		{
			imap->user = vmstrdup(imap->gm, t);
			imap->connected = 0;
			imap->meth = 0;
		}
		if (!s || !*s)
			name = "inbox";
		else
			name += (s - sfstrbase(imap->tp)) + 7;
	}
	if (!imap->connected && imapconnect(imap))
		return -1;
	if (imapexec(imap, "%s %s", state.readonly ? "EXAMINE" : "SELECT", name))
		return -1;
	imap->selected = 1;
	state.msg.count = 0;
	imapsync(imap);
	state.msg.dot = state.msg.list + imap->mailbox.unseen - 1;
	return 0;
}

/*
 * return message pointer given number
 */

static Msg_t*
imap_msg(int m)
{
	register Imap_t*	imap;
	register Msg_t*		mp;

	mp = state.msg.list + m - 1;
	if (!mp->m_info)
	{
		imap = IMAP;
		if (!(mp->m_info = (void*)vmnewof(imap->vm, 0, Imapmsg_t, 1, 0)))
			note(FATAL, "out of space [imap msg info]");
		if (imapexec(imap, "FETCH %d (RFC822.SIZE FLAGS ENVELOPE BODYSTRUCTURE INTERNALDATE)", m))
			note(FATAL, "imap: %d: cannot fetch message info", m);
		if (state.var.spam && !(mp->m_flag & (MSCAN|MSPAM)))
			setinput(mp);
		if (TRACING('b'))
		{
			register Imapmsg_t*	ip;
			register Imappart_t*	pp;

			ip = (Imapmsg_t*)mp->m_info;
			for (pp = ip->parts; pp; pp = pp->next)
				note(ERROR, "imap: message %d id=%s content=%s type=%s encoding=%s name=%s size=%d/%d", m, pp->id, imapname(imapcontent, elementsof(imapcontent), pp->content), pp->type, pp->encoding, pp->name, pp->lines, pp->size);
		}
	}
	return mp;
}

/*
 * IMAP setinput()
 */

Sfio_t*
imap_setinput(register Msg_t* mp)
{
	register Imap_t*	imap = IMAP;
	int			m = mp - state.msg.list + 1;

	imap->copy.fp = imap->mp;
	sfstrseek(imap->mp, 0, SEEK_SET);
	if (imapexec(imap, "FETCH %d (BODY.PEEK[HEADER])", m))
		note(FATAL, "imap: %d: cannot fetch message header", m);
	imap->copy.fp = sfstdout;
	imap->mp->_endb = imap->mp->_next;
	sfseek(imap->mp, (Sfoff_t)0, SEEK_SET);
	return imap->mp;
}

/*
/*
 * IMAP command()
 */

int
imap_command(char* s)
{
	register Imap_t*	imap = IMAP;
	register int		n;
	int			items;
	char*			e;
	Imaplex_t*		xp;
	unsigned long		trace;

	if (strneq(s, "dump", 4) && (!(n = *(s + 4)) || isspace(n)))
	{
		for (s += 4; isspace(*s); s++);
		if (!*s)
		{
		list:
			sfprintf(sfstdout, "dump items are:");
			for (n = 0; n < elementsof(imapdump); n++)
				sfprintf(sfstdout, " %s", imapdump[n].name);
			sfprintf(sfstdout, "\n");
		}
		else for (items = 0;;)
		{
			for (; isspace(*s); s++);
			if (!*s)
				break;
			if (items++)
				sfprintf(sfstdout, "\n");
			if (!(xp = (Imaplex_t*)strpsearch(imapdump, elementsof(imapdump), sizeof(imapdump[0]), s, &e)))
			{
				note(ERROR, "%s: unknown dump item", s);
				goto list;
			}
			for (s = e; isspace(*s); s++);
			if (isdigit(*s))
			{
				n = strtol(s, &e, 0);
				s = e;
			}
			else if (*s == '*')
			{
				s++;
				n = -1;
			}
			else
				n = 0;
			switch (xp->code)
			{
			case IMAP_DUMP_FLAGS:
	for (n = 0; n < elementsof(imapflags); n++)
	sfprintf(sfstdout, "%15s%s%s\n", imapflags[n].name, (imapflags[n].code & IMAP_FLAG_APPLICABLE) ? " APPLICABLE" : "", (imapflags[n].code & IMAP_FLAG_PERMANENT) ? " PERMANENT" : "");
				break;
			case IMAP_DUMP_FOLDER:
	sfprintf(sfstdout, "           name %s\n", state.path.mail);
	sfprintf(sfstdout, "      delimiter %s\n", imap->mailbox.delimiter);
	sfprintf(sfstdout, "         exists %d\n", imap->mailbox.exists);
	sfprintf(sfstdout, "         recent %d\n", imap->mailbox.recent);
	sfprintf(sfstdout, "      read_only %d\n", imap->mailbox.recent);
	sfprintf(sfstdout, "      trycreate %d\n", imap->mailbox.trycreate);
	sfprintf(sfstdout, "        uidnext %d\n", imap->mailbox.uidnext);
	sfprintf(sfstdout, "    uidvalidity %d\n", imap->mailbox.uidvalidity);
	sfprintf(sfstdout, "         unseen %d\n", imap->mailbox.unseen);
				break;
			case IMAP_DUMP_MESSAGE:
			{
				register Msg_t*		mp;
				register Msg_t*		ep;
				register Imapmsg_t*	ip;
				register Imappart_t*	pp;

				if (n > state.msg.count)
				{
					sfprintf(sfstdout, "%d: invalid message", n);
					break;
				}
				if (n < 0)
				{
					mp = state.msg.list;
					ep = state.msg.list + state.msg.count;
				}
				else if (n > 0)
				{
					ep = state.msg.list + n;
					mp = ep - 1;
				}
				else
				{
					mp = state.msg.dot;
					ep = mp + 1;
				}
				for (; mp < ep; mp++)
				{
					sfprintf(sfstdout, "message %4d %s\n", mp - state.msg.list + 1, imapflagnames(imap->tp, imapmflags, elementsof(imapmflags), mp->m_flag));
					if (ip = (Imapmsg_t*)mp->m_info)
						for (pp = ip->parts; pp; pp = pp->next)
							sfprintf(sfstdout, "%12s content=%s type=%s encoding=%s name=%s size=%d/%d\n", pp->id, imapname(imapcontent, elementsof(imapcontent), pp->content), pp->type, pp->encoding, pp->name, pp->lines, pp->size);
				}
				break;
			}
			case IMAP_DUMP_QUEUE:
			{
				register Imapop_t*	op;

				for (op = imap->op + 1; op < &imap->op[elementsof(imap->op)]; op++)
					sfprintf(sfstdout, "        [%d] %s %s %d %s%s\n", op - imap->op, imapname(imapstate, elementsof(imapstate), op->state), imapname(imapresponse, elementsof(imapresponse), op->code), op->count, op->msg, op->retain ? " retain" : "");
				break;
			}
			case IMAP_DUMP_STATE:
	sfprintf(sfstdout, "           host %s\n", imap->host);
	sfprintf(sfstdout, "           user %s\n", imap->user);
	sfprintf(sfstdout, "           auth %s\n", imapflagnames(imap->tp, imapauth, elementsof(imapauth), imap->auth));
	sfprintf(sfstdout, "  authenticated %d\n", imap->authenticated);
	sfprintf(sfstdout, "      connected %d\n", imap->connected);
	sfprintf(sfstdout, "        exiting %d\n", imap->exiting);
	sfprintf(sfstdout, "       selected %d\n", imap->selected);
	sfprintf(sfstdout, "            tag %d\n", imap->tag);
	sfprintf(sfstdout, "        version %d.%d\n", imap->version, imap->revision);
				break;
			}
		}
		n = 0;
	}
	else
	{
		trace = state.trace;
		TRACE('r');
		n = imapexec(imap, "%s", s);
		state.trace = trace;
	}
	return n;
}

/*
 * IMAP copy()
 */

int
imap_copy(register struct msg* mp, Sfio_t* op, Dt_t** ignore, char* prefix, unsigned long flags)
{
	register Imap_t*	imap = IMAP;
	register Imapmsg_t*	ip;
	register Imappart_t*	pp;
	register char*		s;
	register int		i;
	struct name*		np;

	/*
	 * Compute the prefix string, without trailing whitespace
	 */

	i = mp - state.msg.list + 1;
	mp = imap_msg(i);
	ip = (Imapmsg_t*)mp->m_info;
	imap->copy.fp = op;
	if (imap->copy.prefix = prefix)
	{
		imap->copy.prefixlen = strlen(prefix);
		s = prefix + imap->copy.prefixlen;
		while (--s >= prefix && isspace(*s));
		imap->copy.emptylen = (s + 1) - prefix;
		sfputr(op, imap->copy.prefix, -1);
	}
	sfprintf(op, "From %s %s\n", ip->from, ip->date);
	sfprintf(imap->tp, "FETCH %d (BODY[HEADER", i);
	if (ignore && *ignore)
	{
		if (ignore == &state.ignoreall)
			sfprintf(imap->tp, ".FIELDS NIL");
		else
		{
			if (dictflags(ignore) & RETAIN)
			{
				i = RETAIN;
				sfprintf(imap->tp, ".FIELDS (");
				s = "";
			}
			else
			{
				i = IGNORE;
				sfprintf(imap->tp, ".FIELDS.NOT (From");
				s = " ";
			}
			for (np = (struct name*)dtfirst(*ignore); np; np = (struct name*)dtnext(*ignore, np))
				if (np->flags & i)
				{
					sfprintf(imap->tp, "%s%s", s, np->name);
					s = " ";
				}
			sfprintf(imap->tp, ")");
		}
		sfprintf(imap->tp, "]");
		for (pp = ip->parts; pp; pp = pp->next)
			if (pp->content == IMAP_CONTENT_text)
				sfprintf(imap->tp, " BODY[%s]", pp->id);
	}
	else
		sfprintf(imap->tp, "] BODY[TEXT]");
	sfprintf(imap->tp, ")");
	if (imapexec(IMAP, struse(imap->tp)))
		note(FATAL, "imap: %d: cannot fetch message info", i);
	imap->copy.fp = sfstdout;
	imap->copy.prefix = 0;
	if (ignore && *ignore)
		for (pp = ip->parts; pp; pp = pp->next)
			if (pp->content == IMAP_CONTENT_attachment)
				sfprintf(op, "\n(attachment %3d %s %18s \"%s\")\n", pp->attachment, counts(1, pp->lines, pp->size), pp->type, pp->name);
	sfprintf(op, "\n");
	return 0;
}

/*
 * IMAP mkdir()
 */

int
imap_mkdir(char* s)
{
	if (*s == '@')
		s++;
	return imapexec(IMAP, "CREATE %s", s);
}

/*
 * IMAP rename()
 */

int
imap_rename(char* f, char* t)
{
	if (*f == '@')
		f++;
	if (*t == '@')
		t++;
	return imapexec(IMAP, "RENAME %s %s", f, t);
}

/*
 * IMAP rmdir()
 */

int
imap_rmdir(char* s)
{
	if (*s == '@')
		s++;
	return imapexec(IMAP, "DELETE %s", s);
}

/*
 * IMAP exit()
 */

void
imap_exit(int code)
{
	register Imap_t*	imap = IMAP;
	register Msg_t*		mp;

	if (state.folder == FIMAP)
		for (mp = state.msg.list; mp < state.msg.list + state.msg.count; mp++)
			if ((mp->m_flag & (MDELETE|MZOMBIE)) == MDELETE)
				imap_msgflags(mp, 0, MDELETE);
	imap->selected = 0;
	if (imap->connected)
		imap_quit();
	exit(code);
}

/*
 * IMAP folders()
 */

int
imap_folders(void)
{
	return imapexec(IMAP, "LIST \"\" *");
}

/*
 * IMAP getatt()
 */

static int
imap_getatt(Msg_t* mp, register Imappart_t* pp, register char* name, unsigned long flags, off_t* lines, off_t* chars)
{
	register Imap_t*	imap = IMAP;
	register char*		s;
	register int		n;
	char*			cmd;

	if (!(cmd = iscmd(name)))
	{
		if (state.var.attachments && name == pp->name)
		{
			sfprintf(state.path.temp, "%s/%s", state.var.attachments, name);
			name = struse(state.path.temp);
		}
		if (!(name = expand(name, 1)))
			return 1;
	}
	if (pp->encoding && mime(1) && mimeview(state.part.mime, "encoding", name, pp->type, pp->options))
		pp->encoding = pp->type;
	if (pp->encoding && !isdigit(pp->encoding[0]))
	{
		sfprintf(state.path.temp, "uudecode -h -x %s", pp->encoding);
		if (!mimecmp("text", pp->type, NiL))
			sfprintf(state.path.temp, " -t");
		if (cmd)
			sfprintf(state.path.temp, " -o - | %s", cmd);
		else if (filestd(name, "w"))
			sfprintf(state.path.temp, " | %s", state.var.pager);
		else
		{
			sfprintf(state.path.temp, " -o ", name);
			shquote(state.path.temp, name);
		}
		s = struse(state.path.temp);
		n = 1;
	}
	else if (cmd)
	{
		s = cmd;
		n = -1;
	}
	else if (filestd(name, "w"))
	{
		s = state.var.pager;
		n = -1;
	}
	else
	{
		s = name;
		n = 0;
	}
	if (state.var.debug)
	{
		note(DEBUG, "%s `%s'", n ? "pipe" : "file", s);
		if ((flags & GMIME) &&
		    mime(1) &&
		    (s = mimeview(state.part.mime, NiL, name, pp->type, pp->options)))
			note(DEBUG, "mimeview `%s'", s);
		return 0;
	}
	if (!(imap->copy.fp = n ? pipeopen(s, "w") : fileopen(s, "ERw"))) {
		imap->copy.fp = sfstdout;
		return 1;
	}
	if (lines)
		*lines = pp->lines;
	if (chars)
		*chars = pp->size;
	n = mp - state.msg.list + 1;
	if (imapexec(imap, "FETCH %d (BODY[%s])", n, pp->id))
		note(FATAL, "imap: %d: cannot fetch message info", n);
	imap->copy.fp = sfstdout;
	fileclose(imap->copy.fp);
	if (flags & GDISPLAY)
		note(ERROR, "\"%s\" %s", name, counts(1, pp->lines, pp->size));
	if ((flags & GMIME) &&
	    mime(1) &&
	    (s = mimeview(state.part.mime, NiL, name, pp->type, pp->options)) &&
	    (n = start_command(state.var.shell, SIG_REG_EXEC, -1, -1, "-c", s, NiL)) >= 0)
		wait_command(n);
	return 0;
}

/*
 * IMAP get1()
 */

int
imap_get1(char** argv, unsigned long flags)
{
	register Imappart_t*	pp;
	register int		i;
	register char*		s;
	Msg_t*			mp;
	Imapmsg_t*		ip;
	char*			name;
	char*			a;
	char*			e;
	int			n;
	int			r;

	if (state.msg.dot < state.msg.list || state.msg.dot >= state.msg.list + state.msg.count)
	{
		note(ERROR, "No current message");
		return 1;
	}
	mp = imap_msg(state.msg.dot - state.msg.list + 1);
	ip = (Imapmsg_t*)mp->m_info;
	if (!ip->attachments || !(pp = ip->parts))
	{
		note(ERROR, "No attachments in current message");
		return 1;
	}
	if (!*argv)
	{
		do {
			if (pp->content == IMAP_CONTENT_attachment)
				sfprintf(sfstdout, "(attachment %3d %s %18s \"%s\")\n", pp->attachment, counts(1, pp->lines, pp->size), pp->type, pp->name);
		} while (pp = pp->next);
		return 0;
	}
	if (!(a = newof(0, char, ip->attachments, 1)))
		note(PANIC, "Out of space [imap attachments]");
	s = *argv++;
	r = 0;
	for (;;)
	{
		while (isspace(*s))
			s++;
		if (!*s)
			break;
		else if (*s == ',')
		{
			s++;
			r = 0;
		}
		else if (*s == '*')
		{
			if (!r)
				r = 1;
			for (i = r; i <= ip->attachments; i++)
				a[i] = 1;
			r = 0;
		}
		else if (*s == '-')
		{
			s++;
			r = 1;
		}
		else
		{
			n = strtol(s, &e, 0);
			if (n > 0 && n <= ip->attachments)
			{
				if (r)
				{
					for (i = r; i <= n; i++)
						a[i] = 1;
					r = 0;
				}
				else
					a[n] = 1;
			}
			else
			{
				note(ERROR, "%s: invalid attachment number", s);
				while (*e && !isspace(*e))
					e++;
			}
			s = e;
			if (*s == '-')
			{
				s++;
				r = n;
			}
		}
	}
	r = 0;
	for (i = 1; i <= ip->attachments; i++)
		if (a[i])
		{
			while (pp->attachment != i)
				if (!(pp = pp->next))
				{
					note(ERROR, "%d: attachment number out of range", i);
					r = 1;
					goto done;
				}
			if (pp->content != IMAP_CONTENT_attachment)
			{
				note(ERROR, "%d: not an attachment", i);
				continue;
			}
			if (name = *argv)
				argv++;
			else
				name = pp->name;
			if (imap_getatt(mp, pp, name, flags, NiL, NiL))
				r = 1;
		}
 done:
	free(a);
	return r;
}

/*
 * IMAP printhead()
 */

void
imap_printhead(int m, int who)
{
	register Msg_t*		mp;
	register Imapmsg_t*	ip;
	char*			sizes;
	int			subjlen;
	int			current;
	int			disposition;

	mp = imap_msg(m);
	if (mp->m_flag & (MDELETE|MNONE))
		return;
	ip = (Imapmsg_t*)mp->m_info;
	current = state.msg.dot == mp && !state.var.justheaders ? '>' : ' ';
	if (mp->m_flag & MBOX)
		disposition = 'M';
	else if (mp->m_flag & MPRESERVE)
		disposition = 'P';
	else if (mp->m_flag & MSAVE)
		disposition = '*';
	else if (mp->m_flag & MSPAM)
		disposition = 'X';
	else if (!(mp->m_flag & (MREAD|MNEW)))
		disposition = 'U';
	else if ((mp->m_flag & (MREAD|MNEW)) == MNEW)
		disposition = 'N';
	else
		disposition = who ? 'R' : ' ';
	if (who)
	{
		if (!state.var.domain || strchr(ip->from, '@'))
			printf("%c %s\n", disposition, ip->from);
		else
			printf("%c %s@%s\n", disposition, ip->from, state.var.domain);
	}
	else
	{
		sizes = counts(!!state.var.news, mp->m_lines, mp->m_size);
		subjlen = state.screenwidth - 50 - strlen(sizes);
		if (ip->subject && subjlen >= 0)
			printf("%c%c%3d %-20.20s  %16.16s %s %.*s\n",
				current, disposition, m, ip->from, ip->date, sizes,
				subjlen, ip->subject);
		else
			printf("%c%c%3d %-20.20s  %16.16s %s\n",
				current, disposition, m, ip->from, ip->date, sizes);
	}
}

/*
 * imap_msgflags() support
 */

static void
imap_flags(register Imap_t* imap, Msg_t* mp, register int flags, char* op)
{
	register int	i;
	register int	c;

	if (TRACING('z'))
		note(ERROR, "IMAP: smap_flags msg=%d flags=0x%04x op=%s", mp - state.msg.list + 1, flags, op);
	sfprintf(imap->tp, "STORE %d %sFLAGS.SILENT ", mp - state.msg.list + 1, op);
	c = '(';
	for (i = 0; i < elementsof(imapflags); i++)
		if ((imapflags[i].flag & flags) && (imapflags[i].code & IMAP_FLAG_PERMANENT))
		{
			sfprintf(imap->tp, "%c%s", c, imapflags[i].name);
			c = ' ';
		}
	if (c == ' ')
	{
		sfprintf(imap->tp, ")");
		if (!imapsend(imap, 0, "%s", struse(imap->tp)))
			note(ERROR, "%d: message flags not updated", mp - state.msg.list + 1);
	}
	else
		sfstrseek(imap->tp, 0, SEEK_SET);
}

/*
 * IMAP message flag update
 */

void
imap_msgflags(register Msg_t* mp, int set, int clr)
{
	register Imap_t*	imap = IMAP;
	register int		flags;

	if (mp->m_flag & ~clr & MDELETE)
		return;
	if (flags = mp->m_flag & clr)
	{
		mp->m_flag &= ~clr;
		if (flags & (MDELETE|MREAD|MSCAN|MSPAM))
			imap_flags(imap, mp, flags, "-");
	}
	if (flags = ~mp->m_flag & set)
	{
		mp->m_flag |= set;
		if (flags & (MDELETE|MREAD|MSCAN|MSPAM))
			imap_flags(imap, mp, flags, "+");
	}
}

/*
 * IMAP message search/list
 * '(' IMAP SEARCH expression ')'
 */

int
imap_msglist(register char* s)
{
	register Imap_t*	imap = IMAP;
	register char*		t;

	for (; isspace(*s); s++);
	if (*s != '(')
	{
		note(ERROR, "imap: %s: invalid SEARCH expression", s);
		return 0;
	}
	s++;
	t = s + strlen(s);
	for (; t > s && isspace(*--t) && *t != ')';);
	imap->index = state.msg.list;
	if (imapexec(imap, "SEARCH %*.*s", t - s, t - s, s))
		return 0;
	imap->index->m_index = 0;
	return imap->index - state.msg.list;
}

/*
 * IMAP quit()
 */

void
imap_quit(void)
{
	register Imap_t*	imap = IMAP;
	register int		i;

	state.msg.imap.state = 0;
	if (imap->selected)
		imapclose(imap);
	imap->exiting = 1;
	imapexec(imap, "LOGOUT");
	for (i = 0; i < elementsof(imap->op); i++)
		if (imap->op[i].vm)
			vmclose(imap->op[i].vm);
	if (imap->gm)
		vmclose(imap->gm);
	if (imap->vm)
		vmclose(imap->vm);
	if (imap->np)
		sfclose(imap->np);
	if (imap->sp)
		sfclose(imap->sp);
	if (imap->rp)
		sfclose(imap->rp);
	if (imap->mp)
		sfclose(imap->mp);
	if (imap->tp)
		sfclose(imap->tp);
}

/*
 * IMAP save()
 */

int
imap_save(register Msg_t* mp, char* folder)
{
	return imapexec(IMAP, "COPY %d %s", mp - state.msg.list + 1, folder + 1);
}

#else

/*
 * stubs when IMAP can't fly
 */

int
imap_setptr(char* name, int isedit)
{
	note(ERROR, "imap: support not enabled");
	return -1;
}

#endif
