/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2012 AT&T Intellectual Property          *
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
 *
 * makefile lexical scanner and parser
 *
 * interleaved recursive parse() and expand() nesting supported
 *
 * NOTE: readline() from file does double copy to pp->ip and sp
 *	 it should be possible to go directly to sp
 */

#include "make.h"
#include "options.h"

#include <sig.h>

#define PARSEDEPTH	64		/* maximum parse stack depth	*/

#define PS1		"make> "	/* primary query prompt		*/
#define PS2		">>>>> "	/* secondary query prompt	*/

#define OP_ASSERT	(1<<0)		/* assertion statement		*/
#define OP_ASSIGN	(1<<1)		/* assignment statement		*/
#define OP_EMPTY	(1<<2)		/* no operator found		*/
#define OP_STATEMENT	((1<<3)-1)	/* statement type mask		*/

#define OP_ACTION	(1<<3)		/* operator takes action	*/
#define OP_APPEND	(1<<4)		/* append assignment		*/
#define OP_AUXILIARY	(1<<5)		/* auxiliary assignment		*/
#define OP_EXPAND	(1<<6)		/* expand lhs and rhs		*/
#define OP_STATE	(1<<7)		/* state assignment		*/

#define CON_kept	(1<<0)		/* already kept part of block	*/
#define CON_skip	(1<<1)		/* skip this part of block	*/
#define CON_stash	(1<<2)		/* file loop body lines stashed	*/

#define CON_if		(1<<3)		/* if block			*/
#define CON_elif	(1<<4)		/* not used in flags		*/
#define CON_else	(1<<5)		/* had else			*/
#define CON_end		(1<<6)		/* not used in flags		*/
#define CON_for		(1<<7)		/* for loop block		*/
#define CON_while	(1<<8)		/* while loop block		*/
#define CON_eval	(1<<9)		/* eval block			*/

#define CON_break	(1<<10)		/* loop break			*/
#define CON_continue	(2<<10)		/* loop continue		*/
#define CON_error	(3<<10)		/* error message output		*/
#define CON_exit	(4<<10)		/* exit				*/
#define CON_let		(5<<10)		/* let expression		*/
#define CON_local	(6<<10)		/* local var declaration	*/
#define CON_print	(7<<10)		/* standard output message	*/
#define CON_read	(8<<10)		/* read				*/
#define CON_return	(9<<10)		/* return			*/
#define CON_rules	(10<<10)	/* rules			*/
#define CON_set		(11<<10)	/* set options			*/

#define PUSHLOCAL(p)	do{for(p=pp->local;p;p=p->next)if(!(p->newv.property&V_scope))p->bucket->value=(char*)p->oldv;}while(0)
#define POPLOCAL(p)	do{for(p=pp->local;p;p=p->next)if(!(p->newv.property&V_scope))p->bucket->value=(char*)&p->newv;}while(0)

#define freelocal(x)	do{x->next=freelocals;freelocals=x;}while(0)
#define newlocal(x)	do{if(x=freelocals)freelocals=x->next;else x=newof(0,Local_t,1,0);}while(0)

struct Local_s; typedef struct Local_s Local_t;

struct Local_s				/* local variable state		*/
{
	Hash_bucket_t*	bucket;		/* table.var hash bucket	*/
	Var_t*		oldv;		/* old variable value		*/
	Var_t		newv;		/* new variable info		*/
	Local_t*	next;		/* next in list			*/
	int		line;		/* declaration line number	*/
};

typedef struct Control_s		/* flow control block stack	*/
{
	int		flags;		/* block flags			*/
	union
	{
	char*		buffer;		/* loop body buffer		*/
	size_t		offset;		/* loop body ip offset		*/
	}		body;
	int		line;		/* loop line number		*/
	union
	{
	struct
	{
	char*		test;		/* while loop test		*/
	int		free;		/* free test			*/
	}		w;
	struct
	{
	Var_t*		var;		/* for loop variable		*/
	char**		args;		/* for loop arg vector		*/
	Sfio_t*		tmp;		/* for loop arg tmp		*/
	Sfio_t*		vec;		/* for loop args tmp		*/
	}		f;
	}		loop;
} Control_t;

typedef struct Parseinfo_s		/* recursive parse state stack	*/
{
	Control_t*	cp;		/* control block pointer	*/
	Control_t*	block;		/* control block lo water	*/

	/* the rest are implicitly initialized */

	int		argc;		/* local argc			*/
	char*		name;		/* current level input name	*/
	char*		here;		/* <<? termination string	*/
	Sfio_t*		fp;		/* input file pointer		*/
	Sfio_t*		ip;		/* readline write string	*/
	Sfio_t*		scoped;		/* scoped options/assignments	*/
	char*		bp;		/* input buffer pointer		*/
	char*		stashget;	/* loop body stash get		*/
	char*		pushback;	/* line pushback pointer	*/
	Local_t*	local;		/* local variables		*/
	int		checkhere;	/* <<? offset			*/
	int		line;		/* prev level input line number	*/
	int		splice;		/* splice line			*/
	short		indent;		/* active indentation level	*/
	unsigned char	eval;		/* eval block level		*/
	unsigned char	status;		/* action return status		*/
	unsigned int	newline:1;	/* \n at *bp replaced by 0	*/
	unsigned int	prompt:1;	/* interactive input with prompt*/
	unsigned int	stashput:1;	/* put lines in stash		*/
} Parseinfo_t;

/*
 * WARNING: getline() uses the first keyword char
 */

static Namval_t		controls[] =	/* control keywords		*/
{
	"break",	CON_break,
	"continue",	CON_continue,
	"elif",		CON_elif,
	"else",		CON_else,
	"end",		CON_end,
	"error",	CON_error,
	"eval",		CON_eval,
	"exit",		CON_exit,
	"for",		CON_for,
	"if",		CON_if,
	"let",		CON_let,
	"local",	CON_local,
	"print",	CON_print,
	"read",		CON_read,
	"return",	CON_return,
	"rules",	CON_rules,
	"set",		CON_set,
	"while",	CON_while,
};

static Control_t	constack[PARSEDEPTH * 3];

static Parseinfo_t	parsestack[PARSEDEPTH] = { &constack[0], &constack[0] };
static Parseinfo_t*	pp = &parsestack[0];

static Local_t*		freelocals;

/*
 * unwind the parse stack to level on errors
 */

void
unparse(int level)
{
	register Local_t*	lcl;
	register Local_t*	olcl;
	register Control_t*	cp;
	register Rule_t*	r;

	if (pp >= &parsestack[elementsof(parsestack)])
		pp = &parsestack[elementsof(parsestack) - 1];
	while (pp > &parsestack[level])
	{
		if ((r = getrule(pp->name)) && r->active)
		{
			r->active = r->active->previous;
			if (r->status == UPDATE)
				r->status = EXISTS;
		}
		if (pp->fp)
		{
			if (pp->fp != sfstdin)
				sfclose(pp->fp);
			sfstrclose(pp->ip);
		}
		if (lcl = pp->local)
		{
			pp->local = 0;
			while (lcl)
			{
				lcl->bucket->value = (char*)lcl->oldv;
				olcl = lcl;
				lcl = lcl->next;
				freelocal(olcl);
			}
		}
		for (cp = pp->cp; cp > pp->block; cp--)
			if (cp->flags & CON_for)
			{
				if (cp->loop.f.vec)
				{
					sfstrclose(cp->loop.f.vec);
					sfstrclose(cp->loop.f.tmp);
				}
			}
			else if (cp->flags & CON_while)
			{
				if (pp->cp->loop.w.free)
					free(pp->cp->loop.w.test);
			}
		pp--;
	}
}

/*
 * declare a single local variable t
 * redefinitions on same line>0 get value 1
 * if (flags & V_append) then old value is retained
 * if (flags & V_scope) then value survives PUSHLOCAL/POPLOCAL
 */

static void
declare(char* t, int line, long flags)
{
	register Var_t*		v;
	register Local_t*	p;
	register char*		s;
	register char*		d;

	if (d = strchr(t, '='))
		*d = 0;
	if (!(v = getvar(t)))
	{
		for (s = t; *s; s++)
			if (istype(*s, C_TERMINAL))
				error(3, "%s: invalid local variable name", t);
		v = setvar(t, null, 0);
	}
	else if (!(flags & V_scope))
	{
		/*
		 * check for duplicate declarations
		 */

		for (p = pp->local; p; p = p->next)
			if (v->name == p->newv.name)
			{
				if (d)
					*d++ = '=';
				else if (line && p->line == line)
					d = "1";
				else
					d = null;
				p->line = line;
				setvar(v->name, d, 0);
				return;
			}
	}
	newlocal(p);
	p->next = pp->local;
	pp->local = p;
	p->oldv = v;
	p->newv.name = v->name;
	p->newv.property &= ~(V_readonly|V_scope);
	p->newv.property |= (v->property & (V_auxiliary|V_scan)) | (flags & V_scope);
	if (!p->newv.value || (p->newv.property & V_import))
	{
		p->newv.value = newof(0, char, MINVALUE + 1, 0);
		p->newv.length = MINVALUE;
		p->newv.property &= ~V_import;
		p->newv.property |= V_free;
	}
	p->bucket = hashlast(table.var);
	p->bucket->value = (char*)&p->newv;
	p->line = line;
	if (d)
	{
		*d++ = '=';
		setvar(v->name, d, 0);
	}
	else if (flags & V_append)
		setvar(v->name, v->value, 0);
	else
		*p->newv.value = 0;
}

/*
 * declare local variables using tmp string xp
 * v is first expanded
 * 2 funtion argument styles supported
 *	(formal ...) actual ...
 *	-[n] actual
 */

static void
local(Sfio_t* xp, char* v)
{
	register char*	t;
	register char*	a;
	long		top;
	int		argn;
	Control_t*	cp;
	Local_t*	p;
	int		argc = 0;
	int		optional = 0;
	char*		argv = 0;
	char*		formal = 0;
	Sfio_t*		ap = 0;

	for (cp = pp->cp; cp > pp->block; cp--)
		if (cp->flags & (CON_for|CON_while))
		{
			for (p = pp->local; p; p = p->next)
				p->line = 0;
			break;
		}
	if (t = strchr(v, '\n'))
		*t = 0;
	top = sfstrtell(xp);
	expand(xp, v);
	sfputc(xp, 0);
	v = sfstrseek(xp, top, SEEK_SET);
	if (t)
		*t = '\n';
	while (t = getarg(&v, NiL))
	{
		if (!ap && *t == '-')
		{
			ap = sfstropen();
			if (!argc)
				argc = 1;
			argn = strtol(t + 1, NiL, 0);
		}
		else if (!ap && *t == '(')
		{
			ap = sfstropen();
			argv = t + strlen(t) - 1;
			if (*argv != ')')
				error(3, "%s: missing ) in formal argument list", t);
			*argv = 0;
			argv = t + 1;
			argc = 1;
			argn = 0;
			formal = getarg(&argv, NiL);
		}
		else
		{
			if (argv)
			{
				if (!formal)
					error(3, "%s: only %d actual argument%s expected", t, argc - 1, argc == 2 ? null : "s");
				if ((a = getarg(&argv, NiL)) && !optional && streq(a, "..."))
				{
					optional = 1;
					if (*v)
						*(v - 1) = ' ';
					if (!(a = getarg(&argv, NiL)))
						v = null;
				}
				argc++;
				sfprintf(ap, "%s=%s", formal, t);
				t = sfstruse(ap);
				formal = a;
			}
			else if (argc)
			{
				sfprintf(ap, "%d=%s", argc++, t);
				t = sfstruse(ap);
			}
			declare(t, error_info.line, 0);
		}
	}
	if (argc)
	{
		if (formal)
		{
			if (!optional)
			{
				a = getarg(&argv, NiL);
				if (!a || !streq(a, "..."))
					error(3, "%s: actual argument expected", formal, argc - 1);
			}
			while (formal)
			{
				declare(formal, error_info.line, 0);
				formal = getarg(&argv, NiL);
			}
		}
		pp->argc = argc - 1;
		while (argc <= argn)
		{
			sfprintf(ap, "%d", argc++);
			declare(sfstruse(ap), error_info.line, 0);
		}
		sfstrclose(ap);
	}
}

/*
 * copy the local argc into internal.val
 */

void
argcount(void)
{
	sfprintf(internal.val, "%d", pp->argc);
}

/*
 * compute next for loop iteration variable
 */

static int
iterate(void)
{
	register char*		p;
	register Var_t*		v = pp->cp->loop.f.var;

	if (!(p = *pp->cp->loop.f.args++))
		return 0;

	/*
	 * NOTE: this may bypass checks in setvar()
	 */

	if (!(v->property & V_import) && v->value && strlen(p) <= v->length)
		strcpy(v->value, p);
	else
	{
		if (v->property & V_free)
		{
			v->property &= ~V_free;
			free(v->value);
		}
		v->value = p;
		v->property |= V_import;
	}
	debug((-6, "assignment: lhs=`%s' rhs=`%s'", v->name, v->value));
	return 1;
}

/*
 * check for pp directives
 */

static void
directive(register char* s)
{
	register char*	t;
	register int	n;
	Rule_t*		r;
	Stat_t		st;

	static Sfio_t*	file;

	while (isspace(*++s));
	if (isdigit(*s))
	{
		n = 0;
		while (isdigit(*s))
			n = n * 10 + *s++ - '0';
		error_info.line = n - 1;
		while (isspace(*s))
			s++;
		if (*s++ == '"')
		{
			if (!file)
				file = sfstropen();
			while (*s && *s != '"')
				sfputc(file, *s++);
			t = sfstruse(file);
			if (*s == '"')
			{
				if (!*t)
					error_info.file = pp->name;
				else
				{
					pathcanon(error_info.file = t, 0, 0);
					state.init++;
					r = makerule(t);
					state.init--;
					if (!r->time && !(r->property & P_dontcare))
					{
						if (stat(t, &st))
							r->property |= P_dontcare;
						else
						{
							r->time = tmxgetmtime(&st);
							compref(r, COMP_INCLUDE);
						}
					}
				}
			}
		}
	}
	else
	{
		for (t = s; *s && !isspace(*s); s++);
		if (*s)
			*s++ = 0;
		if (streq(t, "rules"))
			rules(s);
		else if (!state.preprocess && strmatch(t, "assert|define|elif|else|endif|endmac|error|if|ifdef|ifndef|include|line|macdef|pragma|unassert|undef|warning"))
		{
			state.preprocess = 1;
			punt(0);
		}
	}
}

/*
 * read line from file or buffer
 *
 * `\<newline>' splices the current and next lead line
 * `#...<newline>' comments (preceded by space) are stripped from files
 *
 * return value for file input placed in pp->ip
 */

static char*
readline(int lead)
{
	register char*	s;
	register char*	t;
	register char*	f;
	register int	c;
	register int	n;
	register int	q;
	Rule_t*		r;
	int		start;
	int		here;
	int		m;
	long		line;
	Sfio_t*		sps[2];

	trap();
	if (s = pp->pushback)
	{
		pp->pushback = 0;
		if (pp->fp)
		{
			if (lead > 0 && (t = sfstrseek(pp->ip, 0, SEEK_CUR) - 1) > s && *(t - 1) == '\\')
			{
				sfstrseek(pp->ip, -2, SEEK_CUR);
				line = s - sfstrbase(pp->ip);
				goto pushback;
			}
			return s;
		}
		else if (t = pp->bp = strchr(s, '\n'))
		{
			if (lead > 0 && t > s && *(t - 1) == '\\')
				*(t - 1) = *t = ' ';
			else
			{
				pp->newline = 1;
				*pp->bp = 0;
				return s;
			}
		}
		else
			return s;
	}
	else
		s = pp->bp;
	if (pp->fp)
	{
		if (s = pp->stashget)
		{
			*s++ = '\n';
			if (t = strchr(s, '\n'))
				*(pp->stashget = t) = 0;
			else
				pp->stashget = 0;
			return s;
		}
		if (pp->stashput)
		{
			sfstrseek(pp->ip, -1, SEEK_CUR);
			sfputc(pp->ip, '\n');
		}
		else
			sfstrseek(pp->ip, 0, SEEK_SET);
		line = sfstrtell(pp->ip);
	pushback:
		do
		{
			trap();
			if (r = getrule(external.makeprompt))
				maketop(r, P_dontcare|P_foreground, NiL);
			if (pp->prompt)
				error(ERROR_PROMPT, lead > 0 && pp->cp == pp->block ? PS1 : PS2);
			if (pp->fp != sfstdin || !state.coshell || !state.coshell->outstanding)
				break;
			sps[0] = pp->fp;
			sps[1] = state.coshell->msgfp;
			if ((n = sfpoll(sps, 2, -1)) <= 0)
				continue;
			if (sps[0] == state.coshell->msgfp || n > 1 && sps[1] == state.coshell->msgfp)
				while (block(1));
		} while (sps[0] != pp->fp && (n <= 1 || sps[1] != pp->fp));
		here = pp->checkhere = 0;
		n = q = 0;
		for (;;)
		{
			switch (c = sfgetc(pp->fp))
			{
			case EOF:
			eof:
				if (q == COMMENT)
					error(lead > 0 ? 2 : 1, "EOF in %c comment", q);
				else if (q)
					error(lead > 0 ? 2 : 1, "EOF in %c...%c quote starting at line %d", q, q, start);
				else if (sfstrtell(pp->ip) > line)
					error(lead > 0 ? 2 : 1, "file does not end with newline");
				if (sfstrtell(pp->ip) > line)
				{
					sfputc(pp->ip, 0);
					return sfstrbase(pp->ip) + line;
				}
				return 0;
			case '\r':
				if ((c = sfgetc(pp->fp)) != '\n')
				{
					if (c != EOF)
						sfungetc(pp->fp, c);
					c = '\r';
					break;
				}
				/*FALLTHROUGH*/
			case '\n':
			newline:
				error_info.line++;
				if (!q || q == COMMENT)
				{
					t = sfstrseek(pp->ip, 0, SEEK_CUR);
					s = sfstrbase(pp->ip) + line;
					while (t > s && (*(t - 1) == ' ' || *(t - 1) == '\t'))
						t--;
					sfstrseek(pp->ip, t - sfstrbase(pp->ip), SEEK_SET);
					sfputc(pp->ip, 0);
					s = sfstrbase(pp->ip) + line;
					if (*s == COMMENT)
					{
						directive(s);
						*s = 0;
						sfstrseek(pp->ip, line + 1, SEEK_SET);
					}
					return s;
				}
				if (pp->prompt)
					error(ERROR_PROMPT, PS2);
				break;
			case '\\':
				switch (c = sfgetc(pp->fp))
				{
				case EOF:
					sfputc(pp->ip, '\\');
					goto eof;
				case '\r':
					if ((c = sfgetc(pp->fp)) != '\n')
					{
						if (c != EOF)
							sfungetc(pp->fp, c);
						sfputc(pp->ip, '\\');
						c = '\r';
						break;
					}
					/*FALLTHROUGH*/
				case '\n':
					if (lead > 0)
					{
						error_info.line++;
						if (pp->prompt)
							error(ERROR_PROMPT, PS2);
						continue;
					}
					pp->splice = error_info.line + 2;
					sfungetc(pp->fp, '\n');
					c = '\\';
					break;
				default:
					sfputc(pp->ip, '\\');
					break;
				}
				break;
			case '"':
			case '\'':
				if (c == q)
					q = 0;
				else if (!q)
				{
					q = c;
					start = error_info.line;
				}
				break;
			case '/':
				if (!q && (sfstrtell(pp->ip) == line || isspace(*(sfstrseek(pp->ip, 0, SEEK_CUR) - 1))))
					switch (c = sfgetc(pp->fp))
					{
					case EOF:
						sfputc(pp->ip, '/');
						goto eof;
					case '*':
						start = ++error_info.line;
					mid:
						for (;;) switch (sfgetc(pp->fp))
						{
						case EOF:
						comeof:
							error(2, "EOF in /*...*/ comment starting at line %d", start);
							goto end;
						case '\n':
							error_info.line++;
							sfputc(pp->ip, '\n');
							if (pp->prompt)
								error(ERROR_PROMPT, PS2);
							break;
						case '/':
							for (;;) switch (c = sfgetc(pp->fp))
							{
							case EOF:
								goto comeof;
							case '*':
								if (error_info.line > start)
									error(2, "/* in /*...*/ comment starting at line %d", start);
								break;
							case '/':
								break;
							default:
								sfungetc(pp->fp, c);
								goto mid;
							}
						case '*':
							for (;;) switch (c = sfgetc(pp->fp))
							{
							case EOF:
								goto comeof;
							case '*':
								break;
							case '/':
								goto end;
							default:
								sfungetc(pp->fp, c);
								goto mid;
							}
						}
					end:
						error_info.line--;
						c = ' ';
						break;
					default:
						sfungetc(pp->fp, c);
						c = '/';
						break;
					}
				break;
			case COMMENT:
				if (!q && (q = c) && sfstrtell(pp->ip) > line && isspace(*(sfstrseek(pp->ip, 0, SEEK_CUR) - 1)))
					for (;;)
						switch (sfgetc(pp->fp))
						{
						case EOF:
							goto eof;
						case '\\':
							switch (sfgetc(pp->fp))
							{
							case EOF:
								goto eof;
							case '\\':
								sfungetc(pp->fp, '\\');
								break;
							case '\n':
								if (!lead)
									goto newline;
								error_info.line++;
								sfputc(pp->ip, '\n');
								if (pp->prompt)
									error(ERROR_PROMPT, PS2);
								break;
							}
							continue;
						case '\n':
							goto newline;
						}
				break;
			case '(':
			case '[':
			case '{':
				if (!q)
					n++;
				break;
			case ')':
			case ']':
			case '}':
				if (!q && n)
					n--;
				break;
			case '<':
				if (!q && !n && !pp->checkhere && (m = sfstrtell(pp->ip) - line))
				{
					if (m == here)
						pp->checkhere = 1;
					else
						here = m + 1;
				}
				break;
			}
			sfputc(pp->ip, c);
		}
	}
	else if (s)
	{
		error_info.line++;
		if (pp->newline)
		{
			pp->newline = 0;
			*s++ = '\n';
		}
		if (f = strchr(s, '\n'))
		{
			if (lead > 0 && f > s && *(f - 1) == '\\')
			{
				/*
				 * NOTE: `\\n' are permanently eliminated *each pass*
				 *	 line counts are preserved by adding newlines
				 *	 after sliding the spliced segment(s)
				 */

				n = 1;
				t = f - 1;
				for (;;)
				{
					if (!(c = *++f))
					{
						f = 0;
						*t = 0;
						break;
					}
					else if (c != '\n')
						*t++ = c;
					else if (*(f - 1) != '\\')
					{
						f = t + 1;
						while (n--)
						{
							*t++ = ' ';
							*t++ = '\n';
						}
						break;
					}
					else
					{
						n++;
						t--;
					}
				}
			}
		}
		if (pp->bp = f)
		{
			*f = 0;
			pp->newline = 1;
		}
		return s;
	}
	else
		return 0;
}

/*
 * structured flow control line input into sp
 *
 * lead identifies a lead line (as opposed to an action line)
 * term is the line terminator char placed in sp
 * 0 return on EOF
 */

static int
getline(Sfio_t* sp, int lead, int term)
{
	register int	c;
	register char*	s;
	register char*	t;
	register int	indent;
	Time_t		tm;
	long		n;
	int		i;
	char*		e;
	char*		lin;
	char*		tok;
	Control_t*	cp;
	Namval_t*	nv;

	if (pp->here && !lead)
	{
		t = pp->here;
		pp->here = 0;
		while ((s = sfgetr(pp->fp, '\n', 1)) && !streq(s, t))
			sfputr(sp, s, term);
		if (!s)
			error(3, "here document terminator \"%s\" not found", t);
		free(t);
		return sfstrtell(sp) != 0;
	}
 again:
	while (s = lin = readline(lead))
	{
		indent = 0;
		if (lead > 0 && !(pp->cp->flags & CON_skip))
			pp->indent = SHRT_MAX;
		while (indent <= pp->indent)
		{
			if (*s == '\t')
			{
				s++;
				indent += state.tabstops - indent % state.tabstops;
			}
			else if (*s == ' ')
			{
				static int	warned;

				for (t = s; isspace(*t); t++);
				if (!*t)
					goto again;
				if (!warned)
				{
					error(1, "<space> indentation may be non-portable");
					warned = 1;
				}
				while (*s == ' ')
				{
					s++;
					indent++;
				}
			}
			else
				break;
		}
		if (lead > 0 && indent <= pp->indent && (c = *s))
		{
			pp->indent = indent;
			if (c == 'b' || c == 'c' || c == 'e' || c == 'f' || c == 'i' || c == 'l' || c == 'p' || c == 'r' || c == 's' || c == 'w')
			{
				for (t = s; istype(*t, C_VARIABLE2); t++);
				if (istype(c = *t, C_SEP))
				{
					*t = 0;
					i = (nv = (Namval_t*)strsearch(controls, elementsof(controls), sizeof(*controls), stracmp, s, NiL)) ? nv->value : 0;
					for (*t = c; isspace(*t); t++);
					c = pp->cp->flags;
					/*UNDENT*/
	if (i)
	{
		debug((-7, "%s:%d:test: `%s'", error_info.file, error_info.line, s));
		switch (i)
		{

		case CON_if:
			if (++pp->cp >= &constack[elementsof(constack)])
				error(3, "if nesting too deep");
			pp->cp->flags = CON_if;
			if (c & CON_skip)
				pp->cp->flags |= CON_kept | CON_skip;
			else if (expr(sp, t))
				pp->cp->flags |= CON_kept;
			else
				pp->cp->flags |= CON_skip;
			continue;

		case CON_else:
			if (!(c & CON_if))
				error(3, "no matching if for else");
			if (*t != 'i' || *(t + 1) != 'f' || !isspace(*(t + 2)))
			{
				if (c & CON_else)
					error(1, "only one else per if");
				if (*t)
					error(1, "%s: tokens after else ignored", t);
				pp->cp->flags |= CON_else;
				if (c & CON_kept)
					pp->cp->flags |= CON_skip;
				else
				{
					pp->cp->flags |= CON_kept;
					pp->cp->flags &= ~CON_skip;
				}
				continue;
			}
			t++;
			while (isspace(*++t));
			/*FALLTHROUGH*/

		case CON_elif:
			if (!(c & CON_if))
				error(3, "no matching if for elif");
			if (c & CON_else)
				error(1, "elif after else");
			if ((c & CON_kept) || !expr(sp, t))
				pp->cp->flags |= CON_skip;
			else
			{
				pp->cp->flags |= CON_kept;
				pp->cp->flags &= ~CON_skip;
			}
			continue;

		case CON_end:
			if (pp->cp <= pp->block)
				error(3, "unmatched end");
			if (*t)
				error(1, "%s: tokens after end ignored", t);
			if (c & CON_eval)
				pp->eval--;
			else if (!(c & CON_skip) || (c & (CON_for | CON_while)) && !(c & CON_kept))
			{
				pp->cp->flags &= ~(CON_kept | CON_skip);
				if ((c & CON_while) && expr(sp, pp->cp->loop.w.test) || (c & CON_for) && iterate())
				{
					if (pp->newline)
					{
						pp->newline = 0;
						*pp->bp = '\n';
					}
					if (pp->fp)
					{
						if (pp->stashget)
							*pp->stashget = '\n';
						pp->stashget = sfstrbase(pp->ip) + pp->cp->body.offset;
					}
					else
						pp->bp = pp->cp->body.buffer;
					error_info.line = pp->cp->line;
					continue;
				}
			}
			if (c & CON_for)
			{
				if (pp->cp->loop.f.vec)
				{
					sfstrclose(pp->cp->loop.f.vec);
					sfstrclose(pp->cp->loop.f.tmp);
				}
			}
			if (c & CON_while)
			{
				if (pp->cp->loop.w.free)
					free(pp->cp->loop.w.test);
			}
			if (c & CON_stash)
				pp->stashput = 0;
			pp->cp--;
			continue;

		case CON_for:
			if (++pp->cp >= &constack[elementsof(constack)])
				error(3, "for nesting too deep");
			pp->cp->flags = CON_for;
			pp->cp->loop.f.vec = 0;
			if (c & CON_skip)
				pp->cp->flags |= CON_kept | CON_skip;
			else
			{
				Sfio_t*	tp;

				pp->cp->loop.f.tmp = tp = sfstropen();
				expand(tp, t);
				tok = sfstruse(tp);
				if (!(s = getarg(&tok, NiL)))
					error(3, "for loop variable omitted");
				pp->cp->loop.f.vec = tp = sfstropen();
				do putptr(tp, t = getarg(&tok, NiL)); while (t);
				if (*(pp->cp->loop.f.args = (char**)sfstrseek(tp, 0, SEEK_SET)))
				{
					pp->cp->loop.f.var = setvar(s, null, 0);
					if (!iterate())
						pp->cp->flags |= CON_kept | CON_skip;
					else
					{
						if (pp->fp)
						{
							if (!pp->stashput)
							{
								pp->stashput = 1;
								pp->cp->flags |= CON_stash;
								sfstrseek(pp->ip, 0, SEEK_SET);
								sfputc(pp->ip, 0);
							}
							pp->cp->body.offset = pp->stashget ? (pp->stashget - sfstrbase(pp->ip)) : (sfstrtell(pp->ip) - 1);
						}
						else if (!pp->bp)
							error(3, "for loop body expected");
						else
							pp->cp->body.buffer = pp->bp + 1;
						pp->cp->line = error_info.line;
					}
				}
				else
					pp->cp->flags |= CON_kept | CON_skip;
			}
			continue;

		case CON_while:
			if (++pp->cp >= &constack[elementsof(constack)])
				error(3, "while nesting too deep");
			pp->cp->flags = CON_while;
			pp->cp->loop.w.free = 0;
			if ((c & CON_skip) || !expr(sp, t))
				pp->cp->flags |= CON_kept | CON_skip;
			else
			{
				pp->cp->loop.w.test = t;
				if (pp->fp)
				{
					if (!pp->stashput)
					{
						pp->stashput = 1;
						pp->cp->flags |= CON_stash;
						sfstrseek(pp->ip, 0, SEEK_SET);
						sfputc(pp->ip, 0);
						pp->cp->loop.w.test = strdup(t);
						pp->cp->loop.w.free = 1;
					}
					pp->cp->body.offset = pp->stashget ? (pp->stashget - sfstrbase(pp->ip)) : (sfstrtell(pp->ip) - 1);
				}
				else if (!pp->bp)
					error(3, "while loop body expected");
				else
					pp->cp->body.buffer = pp->bp + 1;
				pp->cp->line = error_info.line;
			}
			continue;

		case CON_break:
		case CON_continue:
			if (!(pp->cp->flags & CON_skip))
			{
				c = *t ? expr(sp, t) : 1;
				for (cp = pp->cp; cp > pp->block; cp--)
				{
					if (cp <= pp->block)
						error(3, "%s outside of loop", s);
					cp->flags |= CON_skip | CON_kept;
					if ((cp->flags & (CON_for | CON_while)) && --c <= 0)
					{
						if (i == CON_continue)
							cp->flags &= ~CON_kept;
						break;
					}
				}
			}
			continue;

		case CON_return:
			if (!(pp->cp->flags & CON_skip))
			{
				if (state.frame->target->property & P_functional)
				{
					n = sfstrtell(sp);
					expand(sp, t);
					s = sfstrbase(sp) + n;
					t = sfstrseek(sp, 0, SEEK_CUR);
					while (t > s && isspace(*(t - 1)))
						t--;
					sfstrseek(sp, t - sfstrbase(sp), SEEK_SET);
					sfputc(sp, 0);
					setvar(state.frame->target->name, sfstrseek(sp, n, SEEK_SET), 0);
					debug((-5, "%s returns `%s'", state.frame->target->name, sfstrseek(sp, 0, SEEK_CUR)));
				}
				else if (*t)
				{
					if ((tm = timenum(t, &e)) != TMX_NOTIME && *e)
					{
						if ((n = expr(sp, t)) == -1)
							tm = TMX_NOTIME;
						else
							tm = tmxsns(n, 0);
					}
					if (tm == TMX_NOTIME)
					{
						pp->status = FAILED;
						debug((-5, "return fail"));
					}
					else if (tm)
					{
						pp->status = TOUCH;
						internal.internal->time = tm;
						debug((-5, "return [%s]", timestr(internal.internal->time)));
					}
					else
					{
						pp->status = EXISTS;
						debug((-5, "return no update"));
					}
				}
				else
					debug((-5, "return normal"));
				for (cp = pp->cp; cp >= pp->block; cp--)
					cp->flags |= CON_skip | CON_kept;
			}
			continue;

		case CON_eval:
			if (++pp->cp >= &constack[elementsof(constack)])
				error(3, "eval nesting too deep");
			pp->cp->flags = CON_eval | (c & (CON_kept | CON_skip));
			pp->eval++;
			continue;

		case CON_rules:
			if (!(pp->cp->flags & CON_skip))
				rules(t);
			continue;

		case CON_let:
			if (!(pp->cp->flags & CON_skip))
				expr(sp, t);
			continue;

		case CON_local:
			if (!(pp->cp->flags & CON_skip))
				local(sp, t);
			continue;

		case CON_error:
			if (!(pp->cp->flags & CON_skip))
			{
				n = sfstrtell(sp);
				expand(sp, t);
				sfputc(sp, 0);
				i = strtol(sfstrseek(sp, n, SEEK_SET), &tok, 0);
				for (t = tok; isspace(*t); t++);
				c = error_info.line;
				if (i > 0 && !(i & 040))
					error_info.line = 0;
				error(i, "%s", t);
				error_info.line = c;
			}
			continue;

		case CON_exit:
			if (!(pp->cp->flags & CON_skip))
			{
				n = sfstrtell(sp);
				expand(sp, t);
				sfputc(sp, 0);
				i = strtol(sfstrseek(sp, n, SEEK_SET), &tok, 0);
				finish(i);
			}
			continue;

		case CON_print:
		case CON_read:
			if (!(pp->cp->flags & CON_skip))
			{
				int		d;
				int		m;
				int		x;
				char*		a;
				char*		f;

				n = sfstrtell(sp);
				expand(sp, t);
				sfputc(sp, 0);
				a = sfstrseek(sp, n, SEEK_SET);
				d = i == CON_print;
				f = 0;
				n = '\n';
				/*UNDENT...*/

	for (;;)
	{
		for (t = a; *t == ' '; t++);
		if ((x = *t) != '-' && x != '+')
			break;
		a = t;
		t = getarg(&a, NiL) + 1;
		if (*(t - 1) == *t && !*(t + 1))
		{
			for (t = a; *t == ' '; t++);
			break;
		}
		for (;;)
		{
			switch (m = *t++)
			{
			case 0:
				break;
			case 'f':
				if (*t)
					f = t;
				else if (!(f = getarg(&a, NiL)))
					error(3, "-f: format argument expected");
				break;
			case 'i':
			case 'o':
			case 'p':
				if (!*t && !(t = getarg(&a, NiL)))
				{
					error(3, "-%c: file argument expected", m);
					t = "-";
				}
				if (state.io[d])
				{
					if (state.io[d] != sfstdin && state.io[d] != sfstdout && state.io[d] != sfstderr)
						sfclose(state.io[d]);
					state.io[d] = 0;
				}
				if (!streq(t, "-"))
					switch (m)
					{
					case 'i':
						if (!(state.io[d] = sfopen(NiL, t, x == '+' ? "rwe" : "re")))
							error(ERROR_SYSTEM|2, "%s: cannot read", t);
						break;
					case 'o':
						if (!(state.io[d] = sfopen(NiL, t, x == '+' ? "ae" : "we")))
							error(ERROR_SYSTEM|2, "%s: cannot write", t);
						break;
					case 'p':
						if (!(state.io[d] = sfpopen(NiL, t, "rw")))
							error(ERROR_SYSTEM|2, "%s: cannot connect to coprocess", t);
						else
							sfdcslow(state.io[d]);
						break;
					}
				if (!state.io[d])
					switch (d)
					{
					case 0:
						state.io[d] = sfstdin;
						break;
					case 1:
						state.io[d] = sfstdout;
						break;
					case 2:
						state.io[d] = sfstderr;
						break;
					}
				else if (m != 'p')
					sfsetbuf(state.io[d], NiL, 0);
				break;
			case 'n':
				n = -1;
				continue;
			case 'u':
				if (!*t && !(t = getarg(&a, NiL)))
					error(3, "-u: unit argument expected");
				switch (c = *t++)
				{
				case 'm':
					if ((state.io[d = elementsof(state.io) - 1] = state.mam.out) && *state.mam.label)
						sfputr(state.io[d], state.mam.label, -1);
					break;
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					d = c - '0';
					break;
				default:
					error(2, "-u: unit [0-9m] expected");
					break;
				}
				continue;
			default:
				error(3, "-%c: unknown option", *(t - 1));
				continue;
			}
			break;
		}
	}

				/*...INDENT*/
				if (i == CON_print)
				{
					if (state.io[d] && (sfset(state.io[d], 0, 0) & SF_WRITE))
					{
						if (f)
							strprintf(state.io[d], f, t, 0, n);
						else
							sfputr(state.io[d], t, n);
					}
					else if (*t || n != -1)
						error(2, "unit %d not open for writing", d);
				}
				else if (n != -1 || *t)
				{
					if (state.io[d] && (sfset(state.io[d], 0, 0) & SF_READ))
					{
						if (!*(a = t) || !(t = getarg(&a, NiL)) || getarg(&a, NiL))
							error(2, "one variable argument expected");
						else
						{
							if (f)
								error(1, "read format ignored");
							sfset(state.io[d], SF_IOINTR, 1);
							if (!(f = sfgetr(state.io[d], '\n', 1)))
								f = null;
							setvar(t, f, 0);
						}
					}
					else
						error(2, "unit %d not open for reading", d);
				}
			}
			continue;

		case CON_set:
			if (!(pp->cp->flags & CON_skip))
			{
				n = sfstrtell(sp);
				expand(sp, t);
				sfputc(sp, 0);
				set(sfstrseek(sp, n, SEEK_SET), 1, pp->scoped);
			}
			continue;
		}
	}
					/*INDENT*/
				}
			}
		}
#if DEBUG
		if (pp->cp->flags & CON_skip)
			debug((-8, "%s:%d:skip: `%s'", error_info.file, error_info.line, s));
		else
			debug((-7, "%s:%d:data: `%s'", error_info.file, error_info.line, s));
#endif
		if (!(pp->cp->flags & CON_skip))
		{
			for (t = s; isspace(*t); t++);
			if (*t)
			{
				if (!lead && indent <= pp->indent)
				{
					if (pp->newline)
					{
						pp->newline = 0;
						*pp->bp = '\n';
					}
					pp->pushback = lin;
					return 0;
				}
				break;
			}
			else if (!lead && pp->fp == sfstdin)
				return 0;
			else if (pp->splice == error_info.line)
				break;
			else if (!lead)
			{
				sfputr(sp, s, term);
				return 1;
			}
		}
	}
	if (!s)
		return 0;
	switch (i = pp->eval)
	{
	case 0:
		sfputr(sp, s, term);
		break;
	case 1:
		expand(sp, s);
		sfputc(sp, term);
		break;
	default:
		{
			Sfio_t*	tp;
			Sfio_t*	xp;

			tp = sfstropen();
			for (;;)
			{
				xp = (i & 1) ? sp : tp;
				expand(xp, s);
				if (--i <= 0)
					break;
				s = sfstruse(xp);
			}
			sfstrclose(tp);
			sfputc(sp, term);
		}
		break;
	}
	return 1;
}

/*
 * makefile statement parser
 *
 * the statement grammar is
 *
 *	<lhs> <op> <rhs> <act>
 *
 * <lhs>, <rhs> and <act> are placed in sp
 * <op> determines which components are expanded when read
 *
 *	<op>	<lhs>	<rhs>	<act>
 *	-----------------------------
 *	 =	  0	  0	  -
 *	 :=	  1	  1	  -
 *	 +=	  1	  1	  -
 *	 :	  1	  1	  0
 *	:op:	  0	  0	  0
 *
 * an action <act> is not expanded when read
 * actions are indented by a number of <tab> chars
 * the indent tabs are stripped from each line in the action
 * the first line not indented by this amount terminates the action
 * ideally only '\t' should be used although ' ' are ok using state.tabstops
 * the statement operator flags OP_* are returned
 */

static int
statement(Sfio_t* sp, char** lhs, Rule_t** opr, char** rhs, char** act)
{
	register int	c;
	register char*	s;
	register char*	t;
	char*		b;
	char*		p;
	char*		brace = 0;
	char*		ecarb = 0;
	int		item = 0;
	int		op = 0;
	int		nest = 0;
	int		paren = 0;
	int		quote = 0;
	long		rhs_pos = -1;
	long		act_pos = -1;
	long		lin_pos;

	if (!getline(sp, 1, 0))
		return op;
	*opr = 0;
	b = s = sfstrbase(sp);
	while (c = *s++)
	{
		if (c == '\\')
		{
			if (*s)
				s++;
		}
		else if (c == quote)
			quote = 0;
		else if (c == '"' || c == '\'')
			quote = c;
		else if (!quote)
		{
			if (brace)
			{
				if (c == '{')
					nest++;
				else if (c == '}' && !--nest)
				{
					for (p = s; isspace(*p); p++);
					if (*p || brace > b && *(brace - 1) == '$')
					{
						brace = 0;
						continue;
					}
					ecarb = s - 1;
				}
			}
			else if (c == '(')
				paren++;
			else if (c == ')' && !paren--)
				error(3, "to many )'s");
			else if (!paren)
			{
				if (c == '{')
				{
					brace = s - 1;
					nest++;
				}
				else if (c == '}')
					error(3, "unbalanced {...}");
				else if (c == ':')
				{
					if (item < 2 && *s == '=')
					{
						t = s + 1;
						op = OP_ASSIGN|OP_EXPAND;
						break;
					}
					t = s;
					if (istype(*t, C_ID1))
						while (istype(*++t, C_ID2));
					if (*t++ == ':')
					{
						c = *t;
						*t = 0;
						if (!(*opr = getrule(s - 1)) || !((*opr)->property & P_operator))
						{
							Sfio_t*	tmp;

							tmp = sfstropen();
							*(t - 1) = 0;
							sfprintf(tmp, "%s%s", s, external.source);
							*(t - 1) = ':';
							op = readfile(sfstruse(tmp), COMP_INCLUDE|COMP_DONTCARE, NiL);
							sfstrclose(tmp);
							if (!op || !(*opr = getrule(s - 1)) || !((*opr)->property & P_operator))
							{
								*opr = internal.op;
								error(1, "operator %s not defined", s - 1);
							}
						}
						*t = c;
						op = OP_ASSERT|OP_ACTION|OP_EXPAND;
						if (((*opr)->property & P_ignore))
							op &= ~OP_EXPAND;
						break;
					}
					t = s;
					op = OP_ASSERT|OP_ACTION|OP_EXPAND;
					break;
				}
				else if (item < 2)
				{
					if (isspace(c))
					{
						item++;
						while (isspace(*s))
							s++;
						if ((c = *s) != ':' && c != '+' && c != '&' && c != '=')
							item++;
					}
					else if (c == '+')
					{
						if (*s == '=')
						{
							t = s + 1;
							op = OP_ASSIGN|OP_APPEND|OP_EXPAND;
							break;
						}
					}
					else if (c == '&')
					{
						if (*s == '=')
						{
							t = s + 1;
							op = OP_ASSIGN|OP_EXPAND|OP_AUXILIARY;
							break;
						}
					}
					else if (c == '=')
					{
						if (*s == '=')
						{
							t = s + 1;
							op = OP_ASSIGN|OP_STATE;
							break;
						}
						t = s;
						op = OP_ASSIGN;
						break;
					}
				}
			}
		}
	}
	if (quote)
		error(1, "missing closing %c quote", quote);
	else if (paren)
		error(1, "%d closing )%s missing from line", paren, paren == 1 ? null : "'s");
	if (op)
	{
		if (op & OP_EXPAND)
		{
			Sfio_t*	tmp;

			tmp = sfstropen();
			*(s - 1) = 0;
			sfputr(tmp, sfstrbase(sp), 0);
			rhs_pos = sfstrtell(tmp);
			sfputr(tmp, t, 0);
			sfstrseek(sp, 0, SEEK_SET);
			expand(sp, sfstrseek(tmp, 0, SEEK_SET));
			sfputc(sp, 0);
			sfstrseek(tmp, rhs_pos, SEEK_SET);
			rhs_pos = sfstrtell(sp);
			expand(sp, sfstrseek(tmp, 0, SEEK_CUR));
			sfputc(sp, 0);
			sfstrclose(tmp);
			s = t = sfstrbase(sp) + rhs_pos;
			p = sfstrseek(sp, 0, SEEK_CUR) - 1;
		}
		else
			p = t + strlen(t);
		while (p > t && isspace(*(p - 1)))
			p--;
		*p = 0;
		while (isspace(*t))
			t++;
		rhs_pos = t - sfstrbase(sp);
		t = sfstrbase(sp);
		for (s--; s > t && isspace(*(s - 1)); s--);
		*s = 0;
	}
	else
	{
		op = OP_EMPTY;
		p = s - 1;
		s = sfstrbase(sp);
		while (p > s && isspace(*(p - 1)))
			p--;
		*p = 0;
	}
	if (brace)
	{
		for (t = brace; t > s && isspace(*(t - 1)); t--);
		*t = 0;
		if (ecarb)
		{
			*ecarb = 0;
			nest = 0;
		}
		if (p > ++brace)
			*p++ = '\n';
		for (;;)
		{
			if (!*brace)
			{
				*brace = ' ';
				*(brace + 1) = 0;
				break;
			}
			else if (!isspace(*brace) || *brace == '\n')
				break;
			brace++;
		}
		act_pos = brace - sfstrbase(sp);
		sfstrseek(sp, p - sfstrbase(sp), SEEK_SET);
		quote = 0;
		while (nest)
		{
			lin_pos = sfstrtell(sp);
			if (!getline(sp, -1, '\n'))
			{
				error(2, "unbalanced {...} action");
				break;
			}
			s = sfstrbase(sp) + lin_pos;
			t = sfstrseek(sp, 0, SEEK_CUR);
			while (s < t)
			{
				if ((c = *s++) == '\\')
				{
					if (s < t)
						s++;
				}
				else if (c == quote)
					quote = 0;
				else if (c == '"' || c == '\'')
					quote = c;
				else if (!quote)
				{
					if (c == '{')
						nest++;
					else if (c == '}' && !--nest)
					{
						p = s - 1;
						while (s < t && isspace(*s))
							s++;
						if (s < t)
							error(2, "tokens after closing } ignored");
						t = sfstrbase(sp) + act_pos;
						while (p > t && isspace(*(p - 1)))
							p--;
						*p = 0;
						if (!*t)
							act_pos = -1;
						break;
					}
				}
			}
		}
	}
	else if (op & OP_ACTION)
	{
		if (pp->checkhere)
		{
			t = b = sfstrbase(sp) + rhs_pos;
			nest = quote = 0;
			for (;;)
			{
				switch (*t++)
				{
				case 0:
					break;
				case '"':
				case '\'':
					if (*(t - 1) == quote)
						quote = 0;
					else if (!quote)
						quote = *(t - 1);
					continue;
				case '(':
				case '[':
				case '{':
					if (!quote)
						nest++;
					continue;
				case ')':
				case ']':
				case '}':
					if (!quote)
						nest--;
					continue;
				case '<':
					if (!nest && !quote && *t == '<')
					{
						s = t - 1;
						while (isspace(*++t));
						if (*t)
						{
							p = t;
							while (*++t)
								if (isspace(*t))
								{
									*t = 0;
									break;
								}
							pp->here = strdup(p);
							while (--s >= b && isspace(*s));
							*(s + 1) = 0;
						}
						break;
					}
					continue;
				default:
					continue;
				}
				break;
			}
			if (!*b)
				rhs_pos = -1;
		}
		act_pos = ++p - sfstrbase(sp);
		sfstrseek(sp, act_pos, SEEK_SET);
		while (getline(sp, 0, '\n'));
		t = sfstrbase(sp) + act_pos;
		s = sfstrseek(sp, 0, SEEK_CUR);
		for (;;)
		{
			if (s <= t)
			{
				act_pos = -1;
				break;
			}
			if (*--s != '\n')
			{
				*(s + 1) = 0;
				break;
			}
		}
	}
	*lhs = sfstrseek(sp, 0, SEEK_SET);
	*rhs = (rhs_pos >= 0) ? sfstrbase(sp) + rhs_pos : null;
	*act = (act_pos >= 0) ? sfstrbase(sp) + act_pos : null;
	return op;
}

static const Namval_t	nametypes[] =
{
	"altstate",	NAME_altstate,
	"assignment",	NAME_assignment,
	"context",	NAME_context,
	"dynamic",	NAME_dynamic,
	"glob",		NAME_glob,
	"identifier",	NAME_identifier,
	"intvar",	NAME_intvar,
	"option",	NAME_option,
	"path",		NAME_path,
	"staterule",	NAME_staterule,
	"statevar",	NAME_statevar,
	"variable",	NAME_variable,
};

#if DEBUG

static void
fds(int details)
{
	char*		m;
	char*		s;
	char*		x;
	int		i;
	int		flags;
	int		open_max;
	struct stat	st;

	if ((open_max = (int)strtol(astconf("OPEN_MAX", NiL, NiL), NiL, 0)) <= 0)
		open_max = OPEN_MAX;
	for (i = 0; i <= open_max; i++)
	{
		if (fstat(i, &st))
		{
			/* not open */
			continue;
		}
		if (!details)
		{
			sfprintf(sfstdout, "%d\n", i);
			continue;
		}
		if ((flags = fcntl(i, F_GETFL, (char*)0)) == -1)
			m = "--";
		else
			switch (flags & (O_RDONLY|O_WRONLY|O_RDWR))
			{
			case O_RDONLY:
				m = "r-";
				break;
			case O_WRONLY:
				m = "-w";
				break;
			case O_RDWR:
				m = "rw";
				break;
			default:
				m = "??";
				break;
			}
		x = (fcntl(i, F_GETFD, (char*)0) > 0) ? "x" : "-";
		if (isatty(i) && (s = ttyname(i)))
		{
			sfprintf(sfstdout, "%02d %s%s %s %s\n", i, m, x, fmtmode(st.st_mode, 0), s);
			continue;
		}
#if defined(S_IFSOCK) && 0
		addrlen = sizeof(addr);
		memset(&addr, 0, addrlen);
		if (!getsockname(i, (struct sockaddr*)&addr, (void*)&addrlen))
		{
			type = 0;
			prot = 0;
#ifdef SO_TYPE
			len = sizeof(type);
			if (getsockopt(i, SOL_SOCKET, SO_TYPE, (void*)&type, (void*)&len))
				type = -1;
#endif
#ifdef SO_PROTOTYPE
			len = sizeof(prot);
			if (getsockopt(i, SOL_SOCKET, SO_PROTOTYPE, (void*)&prot, (void*)&len))
				prot = -1;
#endif
			if (!st.st_mode)
				st.st_mode = S_IFSOCK|S_IRUSR|S_IWUSR;
			s = 0;
			switch (type)
			{
			case SOCK_DGRAM:
				switch (addr.sin_family)
				{
				case AF_INET:
#ifdef AF_INET6
				case AF_INET6:
#endif
					s = "udp";
					break;
				}
				break;
			case SOCK_STREAM:
				switch (addr.sin_family)
				{
				case AF_INET:
#ifdef AF_INET6
				case AF_INET6:
#endif
#ifdef IPPROTO_SCTP
					if (prot == IPPROTO_SCTP)
						s = "sctp";
					else
#endif
						s = "tcp";
					break;
				}
				break;
#ifdef SOCK_RAW
			case SOCK_RAW:
				s = "raw";
				break;
#endif
#ifdef SOCK_RDM
			case SOCK_RDM:
				s = "rdm";
				break;
#endif
#ifdef SOCK_SEQPACKET
			case SOCK_SEQPACKET:
				s = "seqpacket";
				break;
#endif
			}
			if (!s)
			{
				for (type = 0; family[type].name && family[type].value != addr.sin_family; type++);
				if (!(s = (char*)family[type].name))
					sfsprintf(s = num, sizeof(num), "family.%d", addr.sin_family);
			}
			port = 0;
#ifdef INET6_ADDRSTRLEN
			if (a = (char*)inet_ntop(addr.sin_family, &addr.sin_addr, nam, sizeof(nam)))
				port = ntohs(addr.sin_port);
			else
#endif
			if (addr.sin_family == AF_INET)
			{
				a = inet_ntoa(addr.sin_addr);
				port = ntohs(addr.sin_port);
			}
			else
			{
				a = fam;
				e = (b = (unsigned char*)&addr) + addrlen;
				while (b < e && a < &fam[sizeof(fam)-1])
					a += sfsprintf(a, &fam[sizeof(fam)] - a - 1, ".%d", *b++);
				a = a == fam ? "0" : fam + 1;
			}
			if (port)
				sfprintf(sfstdout, "%02d %s%s %s /dev/%s/%s/%d\n", i, m, x, fmtmode(st.st_mode, 0), s, a, port);
			else
				sfprintf(sfstdout, "%02d %s%s %s /dev/%s/%s\n", i, m, x, fmtmode(st.st_mode, 0), s, a);
			continue;
		}
#endif
		sfprintf(sfstdout, "%02d %s%s %s /dev/inode/%u/%u\n", i, m, x, fmtmode(st.st_mode, 0), st.st_dev, st.st_ino);
	}
}

#endif

/*
 * parse a basic assertion statement
 */

static void
assertion(char* lhs, Rule_t* opr, char* rhs, char* act, int op)
{
	register char*		s;
	register Rule_t*	r;
	register List_t*	p;
	register List_t*	q;
	int			c;
	int			i;
	int			n;
	int			isactive;
	Flags_t			jointproperty;
	Rule_t*			x;
	Rule_t*			joint;
	Var_t*			v;
	List_t*			jointail;
	List_t*			prereqs;
	char*			name;
	struct					/* prereq attributes	*/
	{
		Rule_t		rule;		/* rule attributes	*/
		int		op;		/* assertion op		*/
	}			*att, clr, set;

	if (opr)
	{
		debug((-6, "operator: lhs=`%s' %s rhs=`%s' act=`%-.1024s'", lhs, opr->name, rhs, act));
		apply(opr, lhs, rhs, act, CO_ALWAYS|CO_LOCAL|CO_URGENT);
		return;
	}
	if (internal.assert_p->prereqs && (opr = associate(internal.assert_p, NiL, lhs, NiL)) && opr->prereqs && (opr = opr->prereqs->rule) && (opr->property & P_operator) && !opr->uname)
	{
		s = opr->uname = opr->name;
		opr->name = lhs;
		apply(opr, lhs, rhs, act, CO_ALWAYS|CO_LOCAL|CO_URGENT);
		opr->name = s;
		opr->uname = 0;
		return;
	}
	debug((-6, "assertion: lhs=`%s' rhs=`%-.1024s' act=`%-.1024s'", lhs, rhs, act));

	/*
	 * special check for internal.query
	 */

	if (getrule(lhs) == internal.query)
	{
		c = 0;
		if (!(s = getarg(&rhs, NiL)))
			interpreter(NiL);
		else if (*s == '-' && !*(s + 1))
			while (s = getarg(&rhs, NiL))
		{
			if (streq(s, "blocked"))
			{
#if DEBUG
				c = 1;
				error(0, null);
				dumpjobs(2, JOB_blocked);
#else
				error(2, "%s: implemented in DEBUG==1 version", s);
#endif
			}
			else if (streq(s, "buckets"))
			{
				c = 0;
				hashdump(NiL, HASH_BUCKET);
			}
			else if (streq(s, "fds"))
			{
#if DEBUG
				fds(1);
				c = 0;
#else
				error(2, "%s: implemented in DEBUG==1 version", s);
#endif
			}
			else if (streq(s, "hash"))
			{
				c = 0;
				hashdump(NiL, 0);
			}
			else if (streq(s, "jobs"))
			{
#if DEBUG
				c = 1;
				error(0, null);
				dumpjobs(2, JOB_status);
#else
				error(2, "%s: implemented in DEBUG==1 version", s);
#endif
			}
			else if (streq(s, "nametype"))
			{
				while (s = getarg(&rhs, NiL))
				{
					n = nametype(s, NiL);
					sfprintf(sfstdout, "%16s", s);
					for (i = 0; i < elementsof(nametypes); i++)
						if (n & nametypes[i].value)
							sfprintf(sfstdout, " %s", nametypes[i].name);
					sfputc(sfstdout, '\n');
				}
				break;
			}
			else if (streq(s, "rules"))
			{
				c = 1;
				state.ruledump = 1;
				dump(sfstderr, 0);
				state.ruledump = 0;
			}
			else if (streq(s, "stack"))
			{
				Parseinfo_t*	sp = pp;
				Local_t*	lp;

				c = 1;
				while (--sp > &parsestack[0])
				{
					sfprintf(sfstderr, "\n%s:\n", sp->name);
					for (lp = sp->local; lp; lp = lp->next)
						sfprintf(sfstderr, " %15s=%s [%s]\n", lp->oldv->name, lp->newv.value, lp->oldv->value);
				}
			}
			else if (streq(s, "variables"))
			{
				c = 1;
				state.vardump = 1;
				dump(sfstderr, 0);
				state.vardump = 0;
			}
			else if (streq(s, "view"))
			{
				if (state.maxview)
				{
					sfprintf(sfstderr, "\n");
					for (c = 0; c <= state.maxview; c++)
						sfprintf(sfstderr, "[%d] %2d %s\n", c, state.view[c].pathlen, state.view[c].path);
					c = 1;
				}
			}
			else
				error(1, "%s: options are {blocked,buckets,hash,jobs,nametype,rules,stack,variables,view}", s);
		}
		else
			while (s)
			{
				if (r = getrule(s))
				{
					c = 1;
					dumprule(sfstderr, r);
				}
				if (v = getvar(s))
				{
					c = 1;
					sfprintf(sfstderr, "\n");
					dumpvar(sfstderr, v);
					if (r = staterule(VAR, NiL, s, 0))
						dumprule(sfstderr, r);
				}
				s = getarg(&rhs, NiL);
			}
		if (c)
			sfprintf(sfstderr, "\n");
		return;
	}

	/*
	 * construct the prerequsite list and attributes
	 */

	x = 0;
	zero(clr);
	zero(set);
	set.op = op;
	if (!*rhs)
		set.op |= A_target;
	p = q = 0;
	while (s = getarg(&rhs, &set.op))
	{
		/*
		 * <ATTRNAME><attribute> names (and sets) the attribute
		 * <ATTRSET><attribute> sets the attribute
		 * <ATTRCLEAR><attribute> clears the attribute
		 */

		if (((c = *s) == ATTRSET || c == ATTRCLEAR) && *(s + 1))
		{
			*s = ATTRNAME;
			r = getrule(s);
			*s = c;
			if (!r)
				r = getrule(s + 1);
			if (!r || !(r->property & P_attribute))
			{
				if (r)
				{
					Flags_t		m = 0;

					/*
		 			 * user controlled dynamic
					 * staterule attributes
					 */

					if (r == internal.entries)
						m = D_entries;
					else if (r == internal.member)
						m = D_member;
					else if (r == internal.regular)
						m = D_regular;
					else if (r == internal.scanned)
						m = D_scanned;
					if (m)
					{
						if (c == ATTRCLEAR)
						{
							set.op |= A_negate;
							att = &clr;
						}
						else
							att = &set;
						att->rule.dynamic |= m;
						continue;
					}
				}
				r = makerule(s);
			}
		}
		else
			r = makerule(s);
		if (r->property & P_attribute)
		{
			if (c == ATTRCLEAR)
			{
				set.op |= A_negate;
				att = &clr;
			}
			else
				att = &set;

			/*
			 * assertion attributes
			 */

			if (r == internal.clear)
				att->op |= A_clear;
			else if (r == internal.copy)
				att->op |= A_copy;
			else if (r == internal.delete)
				att->op |= A_delete;
			else if (r == internal.insert)
				att->op |= A_insert;
			else if (r == internal.null)
				att->op |= A_null;
			else if (r == internal.special)
				att->op |= A_special;

			/*
			 * attributes not propagated by merge()
			 *
			 * NOTE: internal.make->make is cleared in immediate()
			 */

			else if (r == internal.attribute)
				att->rule.property |= P_attribute;
			else if (r == internal.immediate)
				att->rule.property |= P_immediate;
			else if (r == internal.make)
				att->rule.property |= P_make;
			else if (r == internal.op)
				att->rule.property |= P_operator;
			else if (r == internal.readonly)
				att->rule.property |= P_readonly;
			else if (r == internal.scan)
				att->op |= A_scan;
			else if (r == internal.semaphore)
			{
				if (att->rule.semaphore < UCHAR_MAX)
					att->rule.semaphore++;
				else
					error(1, "%s: maximum semaphore count is %d", r->name, UCHAR_MAX - 1);
			}
			else if (r == internal.state)
				att->rule.property |= P_state;
			else if (r == internal.target)
				att->rule.property |= P_target;
			else if (r == internal.use)
				att->rule.property |= P_use;

			/*
			 * merge() handles the rest
			 */

			else
				merge(r, &att->rule, MERGE_ATTR|MERGE_FORCE);
		}
		else
		{
			if ((set.op & (A_metarule|A_special)) == A_metarule || !*s)
			{
				if (!x)
					x = r;
				else if (!*s)
					error(1, "multiple prerequisite patterns in metarule assertion");
				else
				{
					while (c = *s++)
					{
						if (c == '%')
							sfputr(internal.tmp, "$(%)", -1);
						else
							sfputc(internal.tmp, c);
					}
					r = makerule(sfstruse(internal.tmp));
				}
			}
			else if (set.op & A_scope)
			{
				r->dynamic |= D_scope;
				set.rule.dynamic |= D_hasscope;
				if (state.user <= 1 && state.reading && state.makefile && (s = strchr(r->name, '=')) && *(s + 1) == '=')
				{
					*s = 0;
					if (nametype(r->name, NiL) & NAME_identifier)
						setvar(r->name, NiL, V_scan);
					*s = '=';
				}
			}
			if (!(set.rule.dynamic & D_dynamic) && !(r->dynamic & D_scope) && isdynamic(r->name))
				set.rule.dynamic |= D_dynamic;
			if (r->property & P_use)
				merge(r, &set.rule, MERGE_ATTR);
			if (!p)
				p = q = cons(r, NiL);
			else
				q = q->next = cons(r, NiL);
			if (!(r->dynamic & D_scope))
				set.op |= A_target;
		}
	}
	prereqs = p;
	if (*act || (set.op & (A_null|A_target)) || (set.rule.property & (P_make|P_local)) == (P_make|P_local))
		set.rule.property |= P_target;
	joint = (set.rule.property & P_joint) ? internal.joint : 0;
	jointproperty = 0;

	/*
	 * assert each target
	 */

	name = getarg(&lhs, &set.op);
	while (name)
	{
		r = makerule(name);
		if (joint)
		{
			if (streq(r->name, "-"))
			{
				jointproperty |= P_dontcare;
				name = getarg(&lhs, &set.op);
				continue;
			}
			if (joint == internal.joint)
			{
				joint = catrule(internal.joint->name, ".", name, 1);
				joint->property |= P_joint|P_readonly|P_virtual;
				jointail = joint->prereqs = cons(r, NiL);
			}
			else
				jointail = jointail->next = cons(r, NiL);
			r->property |= jointproperty;
		}
		if ((set.op & (A_metarule|A_special)) == A_metarule)
		{
			Rule_t*		in;
			Rule_t*		out;

			in = 0;
			if (*name == ATTRNAME)
			{
				for (s = name + 1; istype(*s, C_ID1|C_ID2); s++);
				if (*s == ATTRNAME && s > (name + 1) && (c = *++s))
				{
					*s = 0;
					in = getrule(name);
					*s = c;
				}
			}
			if (in)
			{
				/*
				 * pattern association rule
				 */

				if (*s != '%' || *(s + 1) != ATTRNAME || !(x = getrule(s + 1)) || !(x->property & P_attribute))
					x = makerule(s);
				addprereq(in, x, ((set.op & A_insert) || *(s + strlen(s) - 1) != '%') ? PREREQ_INSERT : PREREQ_APPEND);
				if (set.op & A_negate)
				{
					*name = ATTRCLEAR;
					merge(&clr.rule, makerule(name), MERGE_ATTR|MERGE_FORCE);
					*name = ATTRNAME;
				}
				name = getarg(&lhs, &set.op);
			}
			else
			{
				/*
				 * metarule assertion
				 */

				for (p = prereqs, prereqs = q = 0; p; q = p, p = p->next)
				{
					if (p->rule == x)
					{
						/*
						 * rhs pattern
						 */

						in = p->rule;
						if (q)
							q->next = p->next;
					}
					else if (!prereqs)
						prereqs = p;
				}
				c = ((set.op & A_clear) && !prereqs && !*act) ? PREREQ_DELETE : PREREQ_APPEND;
				out = *(r->name + 1) ? r : 0;

				/*
				 * update the metarule intermediate prerequisite graph
				 */

				if (in)
				{
					r = metarule(in->name, r->name, 1);
					if (out)
					{
						metaclose(in, out, c);
						if (name = getarg(&lhs, &set.op))
						{
							addprereq(metainfo('S', in->name, out->name, 1), out, c);
							do
							{
								x = makerule(name);
								addprereq(metainfo('P', in->name, x->name, 1), out, c);
								addprereq(metainfo('S', in->name, out->name, 1), x, c);
								metaclose(in, x, c);
							} while (name = getarg(&lhs, &set.op));
						}
					}
					else
						addprereq(metainfo((set.rule.property & P_terminal) ? 'T' : 'N', NiL, NiL, 1), in, c);
				}
				else
				{
					if (out && c != PREREQ_DELETE)
						addprereq(internal.metarule, out, PREREQ_LENGTH);
					name = getarg(&lhs, &set.op);
				}
			}
		}
		else
		{
			name = getarg(&lhs, &set.op);
			if (!internal.main->prereqs && !state.global && !(set.rule.property & P_operator) && !(set.op & A_special) && !special(r) && !special(&set.rule))
			{
				internal.main->prereqs = cons(r, NiL);
				internal.main->dynamic &= ~D_compiled;
			}
		}
		if ((r->property & P_readonly) || (r->property & P_staterule) && !istype(*(r->name + 1), C_ID1))
		{
			if (r == internal.readonly)
				continue; /* drop this in 2000 */
			if (pp->fp != sfstdin)
			{
				error(2, "%s: %s atom cannot appear as target", r->name, (r->property & P_readonly) ? "readonly" : "staterule");
				continue;
			}
			else if (r->property & P_readonly)
				error(1, "%s: modifying readonly atom", r->name);
		}
		if (!((r->property|set.rule.property) & P_immediate) && (r->status == UPDATE || r->status == MAKING))
		{
			if (*act || (set.op & A_null))
			{
				error(2, "%s: cannot reset active target action", r->name);
				continue;
			}
			if (set.op & (A_clear|A_copy|A_delete|A_insert))
			{
				error(2, "%s: cannot reorder active target prerequisites", r->name);
				continue;
			}
			isactive = 1;
		}
		else
			isactive = 0;
		if (set.op & (A_clear|A_copy))
		{
			int	dynamic;
			int	property;

			if (r->property & P_attribute)
			{
				error(2, "%s: atom cannot be cleared", r->name);
				continue;
			}
			dynamic = r->dynamic & (D_compiled);
			property = r->property & (P_state|P_staterule|P_statevar);
			if ((r->property & P_metarule) && (prereqs || *act || (set.op & A_null)))
				property |= r->property & (P_metarule|P_use);
			s = r->uname && !(r->property & P_state) ? r->uname : r->name;
			if (!(x = (r->property & P_state) ? rulestate(r, 0) : staterule(RULE, r, NiL, 0)) || r->prereqs != x->prereqs)
				freelist(r->prereqs);
			if (set.op & A_copy)
			{
				if (!prereqs || prereqs->next)
				{
					error(2, "%s: 1-1 copy only", r->name);
					continue;
				}
				x = prereqs->rule;
				*r = *x;
				r->name = s;
				r->uname = 0;
				r->prereqs = listcopy(x->prereqs);
				r->dynamic &= ~D_cached;
				r->dynamic |= dynamic;
				r->property |= property;
				continue;
			}
			zero(*r);
			r->name = s;
			r->dynamic = dynamic;
			r->property = property;
			if (r->property & P_state)
				state.savestate = 1;
		}
		if (set.op & A_delete)
		{
			for (p = prereqs; p; p = p->next)
				addprereq(r, p->rule, PREREQ_DELETE);
			negate(&set.rule, r);
			r->dynamic &= ~D_cached;
			continue;
		}
		if (set.op & A_null)
			r->action = null;
		if (prereqs)
		{
			p = name ? listcopy(prereqs) : prereqs;
			if ((set.op & A_insert) && (r->property & (P_joint|P_target)) == (P_joint|P_target))
				r->prereqs->next = append(p, r->prereqs->next);
			else
			{
				if (isactive && (set.rule.dynamic & D_dynamic))
				{
					r->dynamic |= D_dynamic;
					q = r->prereqs;
					r->prereqs = p;
					dynamic(r);
					p = r->prereqs;
					r->prereqs = q;
				}
				r->prereqs = (set.op & A_insert) ? append(p, r->prereqs) : append(r->prereqs, p);
			}
			remdup(r->prereqs);
			if (r->property & P_state)
				state.savestate = 1;
		}

		/*
		 * check action
		 */

		if (*act)
		{
			if (!(s = r->action) || !streq(act, s))
				r->action = strdup(act); /* XXX: possible leak */
			if (s && r->action != s && !state.user && !(set.rule.property & P_operator) && !(set.op & A_special) && !special(r) && !special(&set.rule))
				error(1, "multiple actions for %s", r->name);
		}

		/*
		 * assign attributes
		 */

		merge(&set.rule, r, MERGE_ATTR|MERGE_FORCE);
		if (set.op & A_negate)
			negate(&clr.rule, r);

		/*
		 * attributes not handled by merge()
		 */

		if (!isactive && ((set.rule.dynamic & D_dynamic) || isdynamic(r->name)))
			r->dynamic |= D_dynamic;
		if (((set.rule.property|clr.rule.property) & P_functional) && !(r->property & P_state) && ((v = getvar(r->name)) || (v = setvar(r->name, null, 0))))
		{
			if (set.rule.property & P_functional)
				v->property |= V_functional;
			else
				v->property &= ~V_functional;
		}
		if (set.rule.property & P_immediate)
			r->property |= (P_ignore|P_immediate);
		if (set.rule.property & P_operator)
		{
			s = r->name;
			if (*s == ':' && istype(*++s, C_ID1))
				while (istype(*++s, C_ID2));
			if (*s == ':' && !*++s)
				r->property |= P_operator;
			else
				error(2, "%s: invalid operator name", r->name);
		}
		if (set.rule.property & P_readonly)
			r->property |= P_readonly;
		if (set.rule.semaphore)
			r->semaphore = set.rule.semaphore + 1;
		if ((set.rule.property & P_target) && !((clr.rule.property | r->property) & P_target))
		{
			r->property |= P_target;
			if (state.targetcontext && (s = strrchr(r->name, '/')))
				makerule(s + 1)->dynamic |= D_context;
		}
		if ((set.rule.property & P_use) && (!(r->property & P_attribute) || !r->attribute))
			r->property |= P_use;

		/*
		 * user controlled dynamic staterule attributes
		 */

		if ((set.rule.dynamic | clr.rule.dynamic) & ~D_dynamic)
		{
			r->dynamic |= (set.rule.dynamic & ~D_dynamic);
			r->dynamic &= ~(clr.rule.dynamic & ~D_dynamic);
			if (x = staterule(RULE, r, NiL, 0))
			{
				x->dynamic |= (set.rule.dynamic & ~D_dynamic);
				x->dynamic &= ~(clr.rule.dynamic & ~D_dynamic);
				if (clr.rule.dynamic & D_scanned)
					x->property |= P_force;
			}
			if (x = staterule(PREREQS, r, NiL, 0))
			{
				x->dynamic |= (set.rule.dynamic & ~D_dynamic);
				x->dynamic &= ~(clr.rule.dynamic & ~D_dynamic);
				if (clr.rule.dynamic & D_scanned)
					x->property |= P_force;
			}
		}

		/*
		 * these are done after attributes have been assigned
		 */

		if ((set.rule.property & P_attribute) && !(r->property & P_attribute))
		{
			r->property |= P_attribute;
			if (!(r->property & P_use))
			{
				if (internal.attribute->attribute << 1)
				{
					r->dynamic |= D_index;
					r->attribute = internal.attribute->attribute;
					internal.attribute->attribute <<= 1;
					addprereq(internal.attribute, r, PREREQ_APPEND);
				}
				else
					error(1, "%s: too many named attributes", r->name);
			}
		}
		if (joint)
			r->prereqs = cons(joint, r->prereqs);
		if ((set.op & A_scan) && !r->scan)
		{
			if (internal.scan->scan == SCAN_MAX)
				error(1, "%s: too many scan strategies", r->name);
			else
			{
				r->dynamic |= D_index;
				r->property |= P_attribute;
				r->scan = internal.scan->scan++;
				addprereq(internal.scan, r, PREREQ_APPEND);
			}
		}
		if (!state.init && !state.readonly && (!state.op && state.reading || !(r->property & P_immediate)))
			r->dynamic &= ~D_compiled;
		r->dynamic &= ~D_cached;

		/*
		 * do immediate actions right away
		 */

		if (r->property & P_immediate)
			immediate(r);
	}
}

/*
 * parse an assignment statement
 */

static void
assignment(char* lhs, int op, char* rhs)
{
	register Rule_t*	r;
	register char*		s;
	register int		n;
	Var_t*			v;

	if (internal.assign_p->prereqs && (r = associate(internal.assign_p, NiL, lhs, NiL)) && r->prereqs && (r = r->prereqs->rule) && (r->property & P_operator) && !r->uname)
	{
		s = r->uname = r->name;
		r->name = (op & OP_APPEND) ? "+=" : (op & OP_AUXILIARY) ? "&=" : (op & OP_STATE) ? "==" : "=";
		apply(r, lhs, rhs, NiL, CO_ALWAYS|CO_LOCAL|CO_URGENT);
		r->name = s;
		r->uname = 0;
		return;
	}
	debug((-6, "assignment: lhs=`%s' %s%srhs=`%-.1024s'", lhs, (op & OP_APPEND) ? "[append] " : null, (op & OP_STATE) ? "[state] " : null, rhs));
	if (!(s = getarg(&lhs, NiL)))
		error(1, "variable name missing in assignment");
	else
	{
		if (getarg(&lhs, NiL))
			error(1, "only one variable per assignment");
		n = 0;
		if (op & OP_APPEND)
			n |= V_append;
		if (op & OP_AUXILIARY)
			n |= V_auxiliary;
		if (op & OP_STATE)
			n |= V_scan;
		if (pp->scoped)
		{
			declare(s, NiL, n|V_scope);
			v = setvar(s, rhs, n);
			if (state.localview && (!(r = staterule(VAR, NiL, s, 0)) || !(r->property & P_parameter)))
			{
				Sfio_t*	tmp;

				tmp = sfstropen();
				expand(tmp, rhs);
				localvar(NiL, v, sfstruse(tmp), V_local_D);
				sfstrclose(tmp);
			}
		}
		else
			setvar(s, rhs, n);
	}
}

/*
 * invoke or verify rules s
 */

void
rules(char* s)
{
	register char*	t;
	register char*	e;

	if (e = strchr(s, '\n'))
		*e = 0;
	sfputr(internal.tmp, s, 0);
	if (e)
		*e = '\n';
	s = sfstruse(internal.tmp);
	if (!(t = getarg(&s, NiL)))
		t = null;
	if (state.rules)
	{
		edit(internal.nam, t, DELETE, KEEP, DELETE);
		edit(internal.wrk, state.rules, DELETE, KEEP, DELETE);
		if (strcmp(sfstruse(internal.nam), sfstruse(internal.wrk)))
			error(3, "%s: incompatible with current base rules %s", t, state.rules);
	}
	else if (t == null)
		state.rules = null;
	else
		state.rules = makerule(t)->name;
	if (t != null && (t = getarg(&s, NiL)))
		error(3, "%s: invalid base rule argument", t);
	state.explicitrules = 1;
}

/*
 * external PUSHLOCAL()
 */

void*
pushlocal(void)
{
	register Local_t*	p;

	PUSHLOCAL(p);
	return (void*)pp->local;
}

/*
 * external POPLOCAL()
 * pos is return value of previous pushlocal()
 */

void
poplocal(void* pos)
{
	register Local_t*	p;
	register Local_t*	t;

	p = (Local_t*)pos;
	while (pp->local != p)
	{
		pp->local->bucket->value = (char*)pp->local->oldv;
		t = pp->local;
		pp->local = pp->local->next;
		freelocal(t);
	}
	POPLOCAL(p);
}

static long	makeexpr(const char*, char**, void*);

/*
 * <var>
 * <var> = <expression>
 * [ <var> = ] <quote> ... <quote>
 */

static char*
nextarg(char* s, char** p, char** end, long* val)
{
	register char*	arg;
	register int	c;
	char*		var;
	char*		varend;
	char		buf[10];
	long		n;

	if ((c = *s) && c != MARK_QUOTE && c != '"')
	{
		if (!istype(*s, C_VARIABLE1))
			error(3, "argument expected in expression [%s]", s);
		for (var = s++; istype(*s, C_VARIABLE2); s++);
		varend = s;
		while (isspace(*s))
			s++;
		if (*s != '=' || *(s + 1) == '=')
		{
			c = *varend;
			*varend = 0;
			arg = getval(var, VAL_PRIMARY|VAL_AUXILIARY);
			*varend = c;
			*p = s;

			/*
			 * determine if in string or numeric context
			 */

			if ((*s == '!' || *s == '=') && *(s + 1) == '=')
			{
				s++;
				while (isspace(*++s));
				if (*s == MARK_QUOTE)
				{
					*end = s;
					return arg;
				}
			}
			if (!*arg)
				*val = 0;
			else
			{
				*val = strtol(arg, &s, 0);
				if (*s)
					*val = 1;
			}
			return 0;
		}
		while (isspace(*++s));
	}
	else
		var = 0;
	if ((c = *s) == MARK_QUOTE)
	{
		for (arg = ++s; *s && *s != c; s++);
		*end = s;
		if (*s)
			while (isspace(*++s));
	}
	else if (c == '"')
	{
		for (arg = ++s; *s && *s != c; s++)
			if (*s == '\\' && *(s + 1))
				s++;
		*end = s;
		if (*s)
			while (isspace(*++s));
	}
	else if (var)
	{
		sfsprintf(arg = buf, sizeof(buf), "%ld", *val = strexpr(s, &s, makeexpr, NiL));
		end = 0;
	}
	else
		error(3, "string argument expected in expression");
	if (var)
	{
		c = *varend;
		*varend = 0;
		if (end)
		{
			n = **end;
			**end = 0;

			/*
			 * XXX: this handles the symptom but not the bug
			 */

			if (*arg == '"' && !*(arg + 1))
				arg++;
		}
		debug((-6, "assignment: lhs=`%s' rhs=`%s'", var, arg));
		setvar(var, arg, 0);
		*varend = c;
		if (end)
			**end = n;
	}
	*p = s;
	return end ? arg : 0;
}

/*
 * supplementary make expression evaluator for strexpr()
 *
 *	"..." == "..."		strmatch()
 *	"..." != "..."		strmatch()
 *	"..." <  "..."		strcoll()
 *	"..." <= "..."		strcoll()
 *	"..." >  "..."		strcoll()
 *	"..." >= "..."		strcoll()
 *	<var>			"<value>"
 *	<var> = "..."		"..."
 *	( <var> = "..." )	"..."
 *	<var> = <expression>	<expression> ? "1" : ""
 *
 * NOTE: '"' translated to MARK_QUOTE by expr()
 */

static long
makeexpr(const char* cs, char** p, void* handle)
{
	char*	s = (char*)cs;
	int	c;
	int	q;
	int	c1;
	int	c2;
	int	m1;
	int	m2;
	char*	paren;
	char*	s1;
	char*	s2;
	char*	e1;
	char*	e2;
	long	n;

	NoP(handle);
	if (!s)
		error(3, "%s in expression", *p);
	else if (!(s1 = nextarg(s, &s, &e1, &n)))
		/* n == expression value */;
	else
	{
		if ((c = *s) == ')')
		{
			paren = s;
			e2 = s;
			while (isspace(*++s));
		}
		else
			paren = 0;
		if (!(c = *s) || c != '<' && c != '>' && (*(s + 1) != '=' || c != '!' && c != '='))
		{
			n = e1 > s1;
			if (paren)
				s = paren;
		}
		else
		{
			q = *++s != '=';
			while (isspace(*++s));
			if (!(s2 = nextarg(s, &s, &e2, &n)))
				n = 0;
			else
			{
				c1 = *e1;
				*e1 = 0;
				c2 = *e2;
				*e2 = 0;
				if (state.context)
				{
					if (*s1 == MARK_CONTEXT && *(e1 - 1) == MARK_CONTEXT)
					{
						*(e1 - 1) = 0;
						m1 = 1;
					}
					else
						m1 = 0;
					if (*s2 == MARK_CONTEXT && *(e2 - 1) == MARK_CONTEXT)
					{
						*(e2 - 1) = 0;
						m2 = 1;
					}
					else
						m2 = 0;
				}
				switch (c)
				{
				case '>':
					n = strcoll(s1, s2) >= q;
					break;
				case '<':
					n = strcoll(s2, s1) >= q;
					break;
				case '!':
					n = !strmatch(s1, s2);
					break;
				default:
					n = strmatch(s1, s2);
					break;
				}
				if (state.context)
				{
					if (m1)
						*(e1 - 1) = MARK_CONTEXT;
					if (m2)
						*(e2 - 1) = MARK_CONTEXT;
				}
				*e1 = c1;
				*e2 = c2;
			}
		}
		if (paren)
		{
			if (!*s)
				s--;
			*s = ')';
		}
	}
	*p = s;
	return n;
}

/*
 * expression evaluation on s using temporary string xp
 * '"' temporarily converted to MARK_QUOTE
 * s is first expanded
 */

long
expr(Sfio_t* xp, register char* s)
{
	register char*	t;
	register int	p;
	register char**	v;
	int		c;
	long		top;
	char*		restore[PARSEDEPTH];

	v = restore;
	t = s;
	p = 0;
	for (;;)
		switch (*t++)
		{
		case '(':
			if (p)
				p++;
			break;
		case ')':
			if (p)
				p--;
			break;
		case '"':
			if (p <= 1)
			{
				p = !p;
				if (v < &restore[elementsof(restore)])
					*(*v++ = t - 1) = MARK_QUOTE;
			}
			break;
		case '\\':
			if (*t++)
				break;
			/*FALLTHROUGH*/
		case 0:
		case '\n':
			c = *--t;
			*t = 0;
			top = sfstrtell(xp);
			expand(xp, s);
			sfputc(xp, 0);
			while (v > restore)
				**--v = '"';
			*t = c;
			return strexpr(sfstrseek(xp, top, SEEK_SET), NiL, makeexpr, NiL);
		}
}

/*
 * error exit during interpreter()
 */

static void
exit_interpreter(int code)
{
	NoP(code);
	sfsync(sfstdout);
	sfsync(sfstderr);
	longjmp(state.resume.label, 1);
	finish(code);
}

/*
 * interactive query loop
 */

void
interpreter(char* msg)
{
	int		level;
	void		(*errexit)(int);
	Frame_t		frame;
	Label_t		resume;

	if (msg)
		error(0, "\n%s\n", msg);
	level = pp - &parsestack[0];
	errexit = error_info.exit;
	error_info.exit = exit_interpreter;
	zero(frame);
	frame.target = internal.query;
	frame.parent = state.frame;
	frame.previous = frame.target->active;
	state.frame = frame.target->active = &frame;
	state.keepgoing |= 2;
	resume = state.resume;
	if (setjmp(state.resume.label))
	{
		unparse(level);
		state.hold = 0;
		state.frame = frame.target->active = &frame;
		sfclrlock(sfstdin);
	}
	else
		state.interpreter++;
	parse(sfstdin, NiL, "query", NiL);
	state.interpreter--;
	state.resume = resume;
	state.keepgoing &= 1;
	frame.target->active = frame.previous;
	state.frame = frame.parent;
	sfclrlock(sfstdin);
	error_info.exit = errexit;
	if (msg)
		error(0, "\n");
}

/*
 * read and parse file fp or line buffer bp
 * non-zero returned if target to be updated
 */

int
parse(Sfio_t* fp, char* bp, char* name, Sfio_t* scoped)
{
	register int		op;
	register Local_t*	lcl;
	char*			lhs;
	char*			rhs;
	char*			act;
	char*			alt;
	Rule_t*			opr;
	Local_t*		olcl;
	Sfio_t*			buf;
	Sfio_t*			tmp;

	if (pp->newline)
		*pp->bp = '\n';
	else if (!pp->fp && pp->bp && !*pp->bp)
	{
		error(1, "parse: early pop");
		return 0;
	}
	PUSHLOCAL(lcl);
	if (++pp >= &parsestack[elementsof(parsestack)])
		error(3, "input nesting too deep");
	if ((pp->block = (pp - 1)->cp + 1) >= &constack[elementsof(constack)])
		error(3, "control block nesting too deep");
#if DEBUG
	message((fp ? -2 : -7, "reading %s", name));
#else
	if (fp)
		message((-2, "reading %s", name));
#endif

	/*
	 * push the parse stack
	 */

	if (pp->fp = fp)
	{
		pp->ip = sfstropen();
		pp->prompt = fp == sfstdin && isatty(sffileno(fp)) && isatty(sffileno(sfstderr));
		if (fp == sfstdin)
			sfdcslow(fp);
	}
	else
	{
		pp->bp = bp;
		pp->prompt = 0;
	}
	pp->name = error_info.file = name;
	pp->line = error_info.line;
	error_info.line = 0;
	pp->argc = 0;
	pp->stashget = 0;
	pp->stashput = 0;
	pp->pushback = 0;
	pp->eval = 0;
	pp->indent = 0;
	pp->newline = 0;
	pp->splice = 0;
	pp->status = UPDATE;
	pp->cp = pp->block;
	pp->cp->flags = 0;
	if (pp->scoped = scoped)
		pp->local = (pp - 1)->local;

	/*
	 * statement parse loop
	 */

	buf = sfstropen();
	for (;;)
	{
		op = statement(buf, &lhs, &opr, &rhs, &act);
		if (trap() && !op)
			continue;
		if (!op)
			break;
		switch (op & OP_STATEMENT)
		{

		case OP_ASSERT:
			assertion(lhs, opr, rhs, act, 0);
			break;

		case OP_ASSIGN:
			assignment(lhs, op, rhs);
			break;

		case OP_EMPTY:
			if (*lhs || *act)
			{
				tmp = sfstropen();
				if (*lhs)
				{
					expand(tmp, lhs);
					rhs = lhs = sfstruse(tmp);
					sfputc(internal.nam, '.');
					while ((op = *rhs++) && !isspace(op))
						sfputc(internal.nam, islower(op) ? toupper(op) : isupper(op) ? tolower(op) : op);
					rhs--;
					alt = sfstruse(internal.nam);
				}
				else
				{
					sfputr(tmp, internal.always->name, -1);
					rhs = lhs = alt = sfstruse(tmp);
				}
				if (!(opr = getrule(alt)) || !(opr->property & (P_attribute|P_functional|P_immediate)))
				{
					if (pp->fp == sfstdin)
					{
						if ((op = *++alt) == 'P' && (!*(alt + 1) || !strcmp(alt, "PRINT")))
						{
							while (isspace(*rhs))
								rhs++;
							sfputr(sfstdout, rhs, '\n');
							sfstrclose(tmp);
							break;
						}
						if (op == 'Q' && (!*(alt + 1) || !strcmp(alt, "QUIT")))
						{
							if (*rhs)
								finish(strtol(rhs, NiL, 0));
							sfstrclose(tmp);
							goto quit;
						}
					}
					else if (!*act)
						error(3, "no operator on line");
					if (*act)
					{
						if (*rhs)
						{
							*rhs++ = 0;
							while (isspace(*rhs))
								rhs++;
						}
						opr = 0;
					}
					else
					{
						opr = internal.query;
						rhs = lhs;
					}
				}
				if (opr)
				{
					if (opr->property & P_functional)
					{
						while (isspace(*rhs))
							rhs++;
						maketop(opr, 0, rhs);
						sfstrclose(tmp);
						break;
					}
					sfputr(internal.nam, opr->name, -1);
					alt = sfstruse(internal.nam);
					if (opr->property & P_immediate)
						lhs = alt;
					else
					{
						lhs = rhs;
						rhs = alt;
					}
				}
				assertion(lhs, NiL, rhs, act, A_special);
				sfstrclose(tmp);
			}
			break;

#if DEBUG
		default:
			error(PANIC, "invalid statement type %d", op);
#endif
		}
	}
 quit:
	sfstrclose(buf);

	/*
	 * pop the parse stack
	 */

	if (pp->cp > pp->block)
		error(3, "missing %d closing end statement%s", pp->cp - pp->block, pp->cp - pp->block == 1 ? null : "s");
	if (pp->fp)
		sfstrclose(pp->ip);
	else if (pp->newline)
		*pp->bp = '\n';
	if (scoped)
	{
		(pp - 1)->local = pp->local;
		pp->local = 0;
	}
	else if (lcl = pp->local)
	{
		pp->local = 0;
		while (lcl)
		{
			lcl->bucket->value = (char*)lcl->oldv;
			olcl = lcl;
			lcl = lcl->next;
			freelocal(olcl);
		}
	}
	error_info.line = pp->line;
	pp--;
	if (pp->newline)
		*pp->bp = 0;
	POPLOCAL(lcl);
	error_info.file = pp->name;
#if DEBUG
	message((fp ? -2 : -7, "popping %s", name));
#else
	if (fp)
		message((-2, "popping %s", name));
#endif
	return (pp + 1)->status;
}

char*
parsefile(void)
{
	register Parseinfo_t*	pi;

	if (state.loading)
		return state.loading;
	for (pi = pp; pi >= &parsestack[0]; pi--)
		if (pi->fp)
			return pi->name;
	return error_info.file;
}
