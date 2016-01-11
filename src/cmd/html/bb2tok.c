/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[-?\n@(#)$Id: bb2tok (AT&T Research) 2007-12-19 $\n]"
USAGE_LICENSE
"[+NAME?bb2tok - convert bb html to tokens]"
"[+DESCRIPTION?\bbb2tok\b extracts tokens from input \bhtml\b \afile\as. "
    "If \afile\a is not specified then the standard input is read. The "
    "\bhtml\b parse is rudimentary; don't use \bbb2tok\b to detect valid "
    "\bhtml\b files.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bhtml2db\b(1), \bhtml2rtf\b(1)]"
;

#include <ast.h>
#include <ctype.h>
#include <error.h>

#define LINK		0
#define NAME		1
#define HEADER		2
#define BODY		3
#define QUOTE		4
#define CODE		5
#define LABEL		6
#define LINE		7

typedef struct Header_s
{
	char*		in;
	char*		out;
	int		lex;
	int		unary;
} Header_t;

static const	Header_t	header[] =
{
	"a",		"link/",	LINK,		1,
	"name",		"name",		NAME,		0,
	"postdetails",	"header",	HEADER,		0,
	"postbody",	"body",		BODY,		0,
	"quote",	"quote",	QUOTE,		0,
	"code",		"code",		CODE,		0,
	"genmed",	"label",	LABEL,		0,
	"line",		"line/",	LINE,		1,
};

typedef struct State_s
{
	Header_t*	prev;
	int		push;
	int		keep;
	int		last;
	unsigned char*	lex;
} State_t;

static void
token(State_t* state, Sfio_t* op, const char* text, const Header_t* head, int push)
{
	if (!head)
	{
		if (state->keep)
		{
			if (*state->lex == LABEL && (streq(text, ":") || streq(text, "Code") || streq(text, "wrote")))
				return;
			if (state->prev)
			{
				sfprintf(op, "%s<%s%s>\n", (!state->push && (state->prev->lex == HEADER || state->prev->lex == CODE && state->last != '\n')) ? "\n" : "", state->push ? "" : "/", state->prev->out);
				state->prev = 0;
			}
			sfputr(op, text, *state->lex == HEADER ? ' ' : '\n');
		}
	}
	else if (push)
	{
		if (state->prev)
		{
			if (head->lex == LINK && state->prev->lex == NAME && state->push)
				return;
			if (head->lex == LINE && state->prev->lex == HEADER && !state->push)
				return;
			if (head->lex == HEADER && push && state->prev->lex == HEADER && !state->push)
			{
				state->prev = 0;
				return;
			}
			if (state->keep && (state->prev->lex != head->lex || !head->unary && state->push))
				sfprintf(op, "%s<%s%s>\n", (!state->push && (state->prev->lex == HEADER || state->prev->lex == CODE && state->last != '\n')) ? "\n" : "", state->push ? "" : "/", state->prev->out);
			if (head->lex == LINE && state->prev->lex == BODY && !state->push)
				state->keep = 0;
		}
		switch (head->lex)
		{
		case CODE:
			state->prev = 0;
			sfprintf(op, "<%s>", head->out);
			return;
		case NAME:
			state->keep = 1;
			break;
		}
		state->prev = (Header_t*)head;
		state->push = push;
	}
	else
	{
		if (state->keep && state->prev)
		{
			if (state->prev->lex == head->lex && state->push)
			{
				state->prev = 0;
				return;
			}
			sfprintf(op, "%s<%s%s>\n", (!state->push && (state->prev->lex == HEADER || state->prev->lex == CODE && state->last != '\n')) ? "\n" : "", state->push ? "" : "/", state->prev->out);
		}
		state->prev = (Header_t*)head;
		state->push = push;
	}
}

#define TOKEN(sp,op,tok,t)	do { if (t > tok) { *t = 0; token(sp, op, t = tok, 0, 0); } } while (0)
#define PUSH(sp,op,h)		token(sp,op,0,h,1)
#define POP(sp,op,h)		token(sp,op,0,h,0)

static void
parse(const char* path, Sfio_t* ip, Sfio_t* op)
{
	register int		c;
	register int		i;
	register int		k;
	register int		q;
	register int		n;
	register int		x;
	register int		level;
	register char*		e;
	register char*		s;
	register char*		t;
	const Header_t*		h;

	char			tag[256];
	char			tok[4 * 1024];
	unsigned char		lex[4 * 1024];
	const Header_t*		block[4 * 1024];

	State_t			state;

	state.prev = (Header_t*)&header[*(state.lex = lex) = LINE];
	state.push = 1;
	state.keep = 0;
	t = tok;
	k = q = n = level = 0;
	for (;;)
	{
		switch (c = sfgetc(ip))
		{
		case EOF:
			TOKEN(&state, op, tok, t);
			break;
		case '<':
			TOKEN(&state, op, tok, t);
			x = 0;
			s = tag;
			for (;;)
			{
				switch (c = sfgetc(ip))
				{
				case EOF:
					TOKEN(&state, op, tok, t);
					return;
				case '"':
					if (!q)
						q = c;
					else if (q == c)
						q = 0;
					goto keep;
				case '!':
					if (s != tag)
						goto keep;
					x = 1;
					continue;
				case '\n':
					x = 1;
					continue;
				case '>':
					if (!q)
						break;
					/*FALLTHROUGH*/
				default:
				keep:
					if (!x && s < &tag[sizeof(tag)-1])
						*s++ = isupper(c) ? tolower(c) : c;
					continue;
				}
				break;
			}
			*s = 0;
			s = tag;
			if (!k)
			{
				if (s[0] == 'b' && s[1] == 'o' && s[2] == 'd' && s[3] == 'y' && (!s[4] || s[4] == ' '))
					k = 1;
				else
					continue;
			}
			if (s[0] == 's' && s[1] == 'p' && s[2] == 'a' && s[3] == 'n' && (!s[4] || s[4] == ' ') && (s += 4) || s[0] == 't' && s[1] == 'd' && (!s[2] || s[2] == ' ') && (s += 2))
			{
				h = 0;
				if (s[0] == ' ' && strneq(s + 1, "class=\"", 7))
				{
					for (e = s += 8; *e && *e != '"'; e++);
					*e = 0;
					for (i = 0; i < elementsof(header); i++)
						if (streq(s, header[i].in))
						{
							h = &header[i];
							if (level < elementsof(block))
							{
								PUSH(&state, op, h);
								n++;
							}
							break;
						}
				}
				if (level < elementsof(block) && (block[level] = h))
					*++state.lex = h->lex;
				level++;
			}
			else if (s[0] == '/' && (s[1] == 's' && s[2] == 'p' && s[3] == 'a' && s[4] == 'n' && !s[5] || s[1] == 't' && s[2] == 'd' && !s[3]))
			{
				if (level > 0)
				{
					level--;
					if (level < elementsof(block) && (h = block[level]))
					{
						POP(&state, op, h);
						n--;
						state.lex--;
					}
				}
			}
			else if (n)
			{
				if (s[0] == 'b' && s[1] == 'r' && (!s[2] || s[2] == ' ' || s[2] == '/'))
				{
					if ((c = sfgetc(ip)) == '\n')
						continue;
					sfungetc(ip, c);
				}
				if (s[0] == 'a' && s[1] == ' ')
					PUSH(&state, op, &header[LINK]);
				else
				{
					c = ' ';
					goto space;
				}
			}
			continue;
		case '&':
			while ((c = sfgetc(ip)) != EOF && isalnum(c));
			c = ' ';
			goto space;
		case ':':
		case ';':
		case ',':
		case '.':
			if (*state.lex == CODE)
				goto code;
			TOKEN(&state, op, tok, t);
			*t++ = c;
			TOKEN(&state, op, tok, t);
			continue;
		case ' ':
		case '\t':
		case '\r':
		case '\v':
		space:
			if (*state.lex == CODE)
				goto code;
			TOKEN(&state, op, tok, t);
			continue;
		case '\n':
			if (*state.lex == CODE)
				goto code;
			TOKEN(&state, op, tok, t);
			PUSH(&state, op, &header[LINE]);
			continue;
		default:
			if (*state.lex == CODE)
				goto code;
			if (t >= &tok[sizeof(tok) - 1])
				TOKEN(&state, op, tok, t);
			*t++ = c;
			continue;
		code:	
			sfputc(op, c);
			state.last = c;
			continue;
		}
		break;
	}
}

int
main(int argc, char** argv)
{
	register char*		s;
	register Sfio_t*	ip;

	NoP(argc);
	error_info.id = "bb2tok";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	do
	{
		if (!(s = *argv) || streq(s, "-") || streq(s, "/dev/stdin") || streq(s, "/dev/fd/0"))
		{
			s = "/dev/stdin";
			ip = sfstdin;
		}
		else if (!(ip = sfopen(NiL, s, "r")))
		{
			error(ERROR_SYSTEM|2, "%s: cannot read", s);
			continue;
		}
		parse(s, ip, sfstdout);
		if (ip != sfstdin)
			sfclose(ip);
	} while (*argv && *++argv);
	return error_info.errors != 0;
}
