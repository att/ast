/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2012 AT&T Intellectual Property          *
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

#include "sed.h"

static const char usage[] =
"[-?\n@(#)$Id: sed (AT&T Research) 2012-03-28 $\n]"
USAGE_LICENSE
"[+NAME?sed - stream editor]"
"[+DESCRIPTION?\bsed\b is a stream editor that reads one or more text files,"
"	makes editing changes according to a script of editing commands,"
"	and writes the results to standard output. The script is obtained"
"	from either the script operand string or a combination of the"
"	option-arguments from the \b--expression\b and \b--file\b options.]"

"[b:strip-blanks?Strip leading blanks from \ba\b, \bc\b, and \bi\b text.]"
"[e:expression?Append the editing commands in \ascript\a to the end of the"
"	the editing command script. \ascript\a may contain more than one"
"	newline separated command.]:[script]"
"[f:file?Append the editing commands in \ascript-file\a to the end of the"
"	the editing command script.]:[script-file]"
"[n:quiet|silent?Suppress the default output in which each line, after it is"
"	examined for editing, is written to standard output. Only lines"
"	explicitly selected for output will be written.]"
"[A|X:augmented?Enable augmented regular expressions; this includes negation"
"	and conjunction.]"
"[E|r:extended|regexp-extended?Enable extended regular expressions, i.e.,"
"	\begrep\b(1) style.]"
"[O:lenient?Enable lenient regular expression interpretation."
"	This is the default if \bgetconf CONFORMANCE\b is not \bstandard\b.]"
"[S:strict|posix?Enable strict regular expression interpretation. This is the"
"	default if \bgetconf CONFORMANCE\b is \bstandard\b. You'd be"
"	suprised what the lenient mode lets by.]"
"[m?multi-digit-reference?Enable \a\\dd\a multi-digit backreferences.]"
"[d?Ignored by this implementation.]"
"[u:unbuffered?Unbuffered output.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bawk\b(1), \bed\b(1), \bgrep\b(1), \bregex\b(3)]"
;

#if 0
static void	readscript(Text*, char*);
static void	copyscript(Text*, unsigned char*);
static int	initinput(int, char **);
static Sfio_t*	aopen(char*, int);
#endif

#define ustrncmp(a,b,c) strncmp((char*)(a), (char*)(b), c)

int reflags = 0;	/* regcomp() flags */
int recno = 0;		/* current record number */
int nflag = 0;		/* nonprint option */
int qflag = 0;		/* command q executed */
int sflag = 0;		/* substitution has occurred */
int bflag = 0;		/* strip leading blanks from c,a,i <text> */
int uflag = 0;		/* unbuffered output */

unsigned char*	map;	/* CC_NAT*IVE => CC_ASCII map */

void
grow(Text *t, word n)
{
	word w = t->w - t->s;
	word e = t->e - t->s + (n/SF_BUFSIZE+1)*SF_BUFSIZE;
	t->s = oldof(t->s, unsigned char, e, 0);
	if(t->s == 0)
		error(3, "out of space");
	t->w = t->s + w;
	t->e = t->s + e;
}

/* BUG: a segment that ends with a comment whose
   last character is \ causes a diagnostic */

void
safescript(Text *t)
{
	if(t->w > t->s+1 && t->w[-2] == '\\')
		error(1, "script segment ends with \\");
}

static Sfio_t *
aopen(char *s, int level)
{
	Sfio_t *f = sfopen(NiL, s, "r");
	if(f == 0)
		error(ERROR_SYSTEM|level, "%s: cannot open", s);
	if (uflag)
		sfsetbuf(f, 0, 0);
	return f;
}

static void
readscript(Text *t, char *s)
{
	word n;
	Sfio_t *f = aopen(s, 3);
	for(;;) {
		assure(t, 4);
		n = sfread(f, t->w, t->e - t->w - 3);
		if(n <= 0)
			break;
		t->w += n;
	}
	sfclose(f);
	if(t->w > t->s && t->w[-1] != '\n') {
		*t->w++ = '\n';
		error(1, "newline appended to script segment");
	}
	*t->w = 0;
	safescript(t);
}

static void
copyscript(Text *t, const unsigned char *s)
{
	do {
		assure(t, 2);
	} while(*t->w++ = *s++);
	if(--t->w > t->s && t->w[-1] != '\n') {
		*t->w++ = '\n';
		*t->w = 0;
	}
	safescript(t);
}

/* DATA INPUT */

struct {
	int iargc;		/* # of files not fully read */
	char **iargv;		/* current file */
	Sfio_t *ifile;		/* current input file */
} input;

int
readline(Text *t)
{
	char*	s;
	word	c;

	coda();
	if (qflag || input.iargc <= 0)
		return 0;
	for (;;)
	{
		if (s = sfgetr(input.ifile, '\n', 1))
		{
			c = sfvalue(input.ifile);
			break;
		}
		if (s = sfgetr(input.ifile, '\n', -1))
		{
			c = sfvalue(input.ifile) + 1;
			error(1, "newline appended");
			break;
		}
		error_info.file = 0;
		error_info.line = 0;
		sfclose(input.ifile);
		do
		{
			if (--input.iargc <= 0)
				return 0;
		} while (!(input.ifile = aopen(*++input.iargv, 2)));
		error_info.file = *input.iargv;
	}
	assure(t, c);
	memcpy(t->w, s, c);
	t->w += c - 1;
	error_info.line++;
	recno++;
	sflag = 0;
	return 1;
}	

int 
ateof(void)
{
	int	c;

	if (input.iargc == 1)
	{
		if ((c = sfgetc(input.ifile)) != EOF)
			sfungetc(input.ifile, c);
		else
			input.iargc = 0;
	}
	return input.iargc <= 0;
}	

static int
initinput(int argc, char **argv)
{
	input.iargc = argc;
	input.iargv = argv;
	if(input.iargc == 0) {
		input.iargc = 1;	/* for ateof() */
		input.ifile = sfstdin;
	} else {
		while (!(input.ifile = aopen(*input.iargv, 2))) {
			if (--input.iargc <= 0)
				return 0;
			++input.iargv;
		}
		error_info.file = *input.iargv;
	}
	return 1;
}

#if DEBUG & 1

/* debugging code 1; compile and execute stubs.
   simply prints the already collected script and
   prints numbered input lines */

void
compile(Text *script, Text *t)
{
	unsigned char *s = t->s;
	assure(script, 1);
	*script->w++ = 0;
	while(*s) sfputc(sfstdout, *s++);
}

void
execute(Text *x, Text *y)
{
	x = x;		
	sfprintf(sfstdout, "%d: %s", recno, y->s);
}

#endif

int
main(int argc, char **argv)
{
	int c;
	static Text script;
	static Text data;
	error_info.id = "sed";
	if (!conformance(0, 0))
		reflags = REG_LENIENT;
	map = ccmap(CC_NATIVE, CC_ASCII);
	while (c = optget(argv, usage))
		switch (c)
		{
		case 'A':
		case 'X':
			reflags |= REG_AUGMENTED;
			break;
		case 'E':
		case 'r':
			reflags |= REG_EXTENDED;
			break;
		case 'O':
			reflags |= REG_LENIENT;
			break;
		case 'S':
			reflags &= ~REG_LENIENT;
			break;
		case 'b':
			bflag++;
			break;
		case 'e':
			copyscript(&data, (unsigned char*)opt_info.arg);
			break;
		case 'f':
			readscript(&data, opt_info.arg);
			break;
		case 'm':
			reflags |= REG_MULTIREF;
			break;
		case 'n':
			nflag++;
			break;
		case 'd':
			break;
		case 'u':
			uflag++;
			break;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argv += opt_info.index;
	argc -= opt_info.index;
	if(data.s == 0) {
		if(!*argv)
			error(3, "no script");
		copyscript(&data, (unsigned char*)*argv++);
		argc--;
	}
	if(ustrncmp(data.s, "#n", 2) == 0)
		nflag = 1;
	copyscript(&data, (const unsigned char*)"\n\n");  /* e.g. s/a/\ */
	compile(&script, &data);
#if DEBUG
	printscript(&script);
#endif
	if (uflag)
		sfsetbuf(sfstdout, 0, 0);
	if (initinput(argc, argv))
		for(;;) {
			data.w = data.s;
			if(!readline(&data))
				break;
			execute(&script, &data);
		}
	if(sfclose(sfstdout) < 0)
		error(ERROR_SYSTEM|3, stdouterr);
	return error_info.errors != 0;
}
