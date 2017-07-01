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
/*
 * Glenn Fowler
 * David Korn
 * AT&T Research
 *
 * od
 */

static const char usage[] =
"[-?\n@(#)$Id: od (AT&T Research) 2013-09-13 $\n]"
USAGE_LICENSE
"[+NAME?od - dump files in octal or other formats]"
"[+DESCRIPTION?\bod\b dumps the contents of the input files in various "
    "formats on the standard output. The standard input is read if \b-\b or "
    "no files are specified. Each output line contains the file offset of "
    "the data in the leftmost column, followed by one or more columns in the "
    "specified format. If more than one format is specified then the "
    "subsequent lines are listed with the offset column blank. Second and "
    "subsequent occurrences of a repeated output line are replaced by a "
    "single line with `*' in the first data column.]"
"[+?\b--format=c\b, \b--format=C\b and \b--format=O\b interpret bytes as "
    "characters in the LC_CTYPE locale category. The three types differ only "
    "in the C style escape sequences recognized; \b--format=C\b explicitly "
    "allows future extensions. The backslash character is written as `\\' "
    "and NUL is written as `\\0'. Other non-printable characters are written "
    "as one three-digit octal number for each byte in the character. "
    "Printable multi-byte characters are written in the area corresponding "
    "to the first byte of the character; the two-character sequence `**' is "
    "written in the area corresponding to each remaining byte in the "
    "character; the remaining bytes for \b-p\b are written as ` '. If "
    "\b--skip\b or \b--count\b position the file into the middle of a "
    "multibyte character then the entire multibyte character is silently "
    "treated as non-character bytes.]"
"[+?If the output format is specified by one of the obsolete forms (not "
    "\b--format\b) then the last file argument is interpreted as an offset "
    "expression if it matches the extended regular expression "
    "\b+?[0-9]]+\\.?[bkm]]?(ll|LL)?\b . In this case the first \aoffset\a "
    "bytes of the file are skipped. The optional \bb\b means bytes, \bk\b "
    "means Kb, and \bm\b means Mb. \bll\b and \bLL\b are ignored for "
    "compatibility with some systems.]"
"[A:address-radix?The file offset radix.]:[radix:=o]"
    "{"
        "[+d?decimal]"
        "[+o?octal]"
        "[+x?hexadecimal]"
        "[+n?none - do not print offset]"
    "}"
"[B:swap?Swap input bytes according to the bit mask \aop\a, which is the "
    "inclusive or of:]#[op]"
    "{"
        "[+01?swap 8-bit bytes]"
        "[+02?swap 16-bit words]"
        "[+04?swap 32-bit longs]"
        "[+0?swap for big endian testing]"
    "}"
"[j:skip-bytes?Skip \bbytes\b bytes into the data before "
    "formatting.]#[bytes]"
"[N:count|read-bytes?Output only \bbytes\b bytes of data.]#[bytes]"
"[m:map?\b--printable\b and \b--format=m\b bytes are converted from "
    "\acodeset\a to the native codeset. The codesets are:]:[codeset]"
    "{\fcodesets\f} [p:printable?Output the printable bytes (after \b--map\b "
    "if specified), in the last data column. Non-printable byte values are "
    "printed as `.'.]"
"[z:strings?Output NUL terminated strings of at least \alength\a "
    "bytes.]#? [length:=3]"
"[t:format|type?The data item output \aformat\a and \asize\a. A decimal byte "
    "count or size code may follow all but the \ba\b, \bc\b, \bC\b, \bm\b "
    "and \bO\b formats.]:[format[size]]:=o2]"
    "{\ftypes\f}"
"[T:test?Enable internal implementation specific tests.]:[test]"
    "{"
        "[+b\an\a?Allocate a fixed input buffer of size \an\a.]"
        "[+m\an\a?Set the mapped input buffer size to \an\a.]"
        "[+n?Turn off the \bSF_SHARE\b input buffer flag.]"
    "}"
"[v:all|output-duplicates?Output all data.]"
"[w:per-line|width?The number of items to format per output line. "
    "\aper-line\a must be a multiple of the least common multiple of the "
    "sizes of the format types.]#[per-line]"
"[a?Equivalent to \b-ta\b.]"
"[b?Equivalent to \b-toC\b.]"
"[c?Equivalent to \b-tO\b.]"
"[C?Equivalent to \b-tO\b.]"
"[d?Equivalent to \b-tuS\b.]"
"[D?Equivalent to \b-tuL\b.]"
"[f?Equivalent to \b-tfF\b.]"
"[F?Equivalent to \b-tfD\b.]"
"[h?Equivalent to \b-txS\b.]"
"[i?Equivalent to \b-tdS\b.]"
"[l?Equivalent to \b-tdL\b.]"
"[o?Equivalent to \b-toS\b.]"
"[O?Equivalent to \b-toL\b.]"
"[s?Equivalent to \b-tdS\b.]"
"[S?Equivalent to \b-tdL\b.]"
"[u?Equivalent to \b-tuS\b.]"
"[U?Equivalent to \b-tuL\b.]"
"[x?Equivalent to \b-txS\b.]"
"[X?Equivalent to \b-txL\b.]"
"\n"
"\n[ file ... ] [ [+]offset[.|b|k|m|ll|LL] ]\n"
"\n"
"[+SEE ALSO?\bsed\b(1), \bstrings\b(1), \bswap\b(3), \bascii\b(5)]"
;

#include <cmd.h>
#include <sig.h>
#include <swap.h>
#include <ccode.h>
#include <ctype.h>
#include <iconv.h>
#include <vmalloc.h>
#include <ast_float.h>

#if _hdr_wctype
#include <wchar.h>
#include <wctype.h>
#else
#define iswprint(w)	isprint(w)
#endif

#define NEW		(1<<0)
#define OLD		(1<<1)

#define BASE_WIDTH	7
#define LINE_LENGTH	78

#define WIDTHINDEX(n)	(((n)>1)+((n)>2)+((n)>4)+((n)>8))

#ifdef _ast_int8_t
#define QUAL		"ll"
#else
#define QUAL		"l"
#endif

#if _typ_long_double
#define double_max	long double
#else
#define double_max	double
#endif

struct State_s; typedef struct State_s State_t;
struct Format_s; typedef struct Format_s Format_t;

typedef void (*Format_f)(State_t*, Format_t*, Sfio_t*, unsigned char*);

typedef union Mem_u
{
	char			m_char[sizeof(intmax_t) + sizeof(double_max)];
	float			m_float;
	double			m_double;
#if _typ_long_double
	long double		m_long_double;
#endif
} Mem_t;

typedef struct Size_s
{
	char*		desc;
	int		name;
	int		map;
	int		dflt;
	const char*	qual;
	int		size;
	int		prefix;
	int		digits;
	int		exponent;
} Size_t;

typedef struct Type_s
{
	char*		desc;
	char		name;
	char		mb;
	const char*	fill;
	Format_f	fun;
	const Size_t*	size;
	char		width[5];
} Type_t;

struct Format_s
{
	struct Format_s*next;
	const Type_t*	type;
	Format_f	fun;
	char		form[16];
	struct
	{
	int		external;
	int		internal;
	}		size;
	int		fp;
	int		width;
	int		us;
	int		per;
};

struct State_s
{
	int		addr;
	char		base[8];
	int		block;
	struct
	{
	char*		base;
	size_t		size;
	int		noshare;
	}		buffer;
	size_t		count;
	Shbltin_t*	context;
	struct
	{
	char		buf[1024];
	char*		data;
	int		mark;
	size_t		size;
	}		dup;
	unsigned char*	eob;
	char*		file;
	Format_t*	form;
	Format_t*	last;
	unsigned char*	map;
	int		mb;
	int		mbmax;
	int		mbp;
	intmax_t	offset;
	struct
	{
	char*		data;
	size_t		size;
	}		peek;
	int		printable;
	int		size;
	size_t		skip;
	char*		span;
	int		spansize;
	int		strings;
	int		style;
	int		swap;
	size_t		total;
	int		verbose;
	int		width;
	Vmalloc_t*	vm;
	Mbstate_t	q;
};

static const Size_t	csize[] =
{
"char",		'C',	0,	1,	0,	1,0,0,0,
0
};

static const Size_t	isize[] =
{
"char",		'C',	0,	0,	0,	1,0,0,0,
"short",	'S',	0,	0,	0,	sizeof(short),0,0,0,
"int",		'I',	0,	1,	0,	sizeof(int),0,0,0,
"long",		'L',	0,	0,	"l",	sizeof(long),0,0,0,
"long long",	'D',	0,	0,
#ifdef _ast_int8_t
					"ll",	sizeof(int64_t),0,0,0,
#else
					0,	sizeof(long),0,0,0,
#endif
	0
};

static const Size_t	fsize[] =
{
"float",	'F',	'e',	0,	0,	sizeof(float),3,FLT_DIG,FLT_MAX_10_EXP,
"double",	'D',	'e',	1,	0,	sizeof(double),3,DBL_DIG,DBL_MAX_10_EXP,
"long double",	'L',	'e',	0,
#if _typ_long_double
					"L",	sizeof(long double),3,LDBL_DIG,LDBL_MAX_10_EXP,
#else
					0,	sizeof(double),3,DBL_DIG,DBL_MAX_10_EXP,
#endif
0
};

static const Size_t	asize[] =
{
"float",	'F',	'a',	0,	0,	sizeof(float),5,FLT_DIG,FLT_MAX_EXP,
"double",	'D',	'a',	1,	0,	sizeof(double),5,DBL_DIG,DBL_MAX_EXP,
"long double",	'L',	'a',	0,
#if _typ_long_double
					"L",	sizeof(long double),5,LDBL_DIG,LDBL_MAX_EXP,
#else
					0,	sizeof(double),5,DBL_DIG,DBL_MAX_EXP,
#endif
0
};

static void
aform(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	register int		c = *u;

	static const char	anames[] =
		"nulsohstxetxeotenqackbel bs ht nl vt ff cr so si"
		"dledc1dc2dc3dc4naksynetbcan emsubesc fs gs rs us"
		" sp  !  \"  #  $  %  &  '  (  )  *  +  ,  -  .  /"
		"  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?"
		"  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O"
		"  P  Q  R  S  T  U  V  W  X  Y  Z  [  \\  ]  ^  _"
		"  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o"
		"  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~del";

	c = 3 * (c & 0177);
	sfputc(op, anames[c]);
	sfputc(op, anames[c+1]);
	sfputc(op, anames[c+2]);
}

static void
bform(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, "%08..2u", *u);
}

static void
cform(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	unsigned char*	v;
	char*		s;
	wchar_t		w;
	int		i;
	int		j;
	char		buf[2];

	if (state->mb)
	{
		state->mb--;
		sfputc(op, ' ');
		sfputc(op, '*');
		sfputc(op, '*');
		return;
	}
	switch (*u)
	{
	case 0:
		sfputc(op, ' ');
		sfputc(op, '\\');
		sfputc(op, '0');
		return;
	case '\\':
		sfputc(op, ' ');
		sfputc(op, ' ');
		sfputc(op, '\\');
		return;
	}
	buf[0] = *(v = u);
	if ((w = mbtchar(&w, u, state->eob - u, &state->q)) > 0 && (i = u - v) > 1 && !iswprint(w))
	{
		state->mb = i - 1;
		for (j = 3 - mbwidth(w); j > 0; j--)
			sfputc(op, ' ');
		while (i--)
			sfputc(op, *v++);
		return;
	}
	buf[1] = 0;
	s = fmtesc(buf);

	/*
	 * posix shalls are sometimes an impediment to progress
	 */

	if (*s != '\\')
	{
		sfputc(op, ' ');
		sfputc(op, ' ');
		sfputc(op, *s);
		return;
	}
	switch (*(s + 1))
	{
	case 'a':
	case 'b':
	case 'f':
	case 'n':
	case 'r':
	case 't':
	case 'v':
		sfputc(op, ' ');
		sfputc(op, '\\');
		sfputc(op, *(s + 1));
		return;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		sfputc(op, *++s);
		sfputc(op, *++s);
		sfputc(op, *++s);
		return;
	}
	sfprintf(op, "%03o", *v);
}

static void
Cform(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	unsigned char*	v;
	char*		s;
	wchar_t		w;
	int		i;
	int		j;
	char		buf[2];

	if (state->mb)
	{
		state->mb--;
		sfputc(op, ' ');
		sfputc(op, '*');
		sfputc(op, '*');
		return;
	}
	switch (*u)
	{
	case 0:
		sfputc(op, ' ');
		sfputc(op, '\\');
		sfputc(op, '0');
		return;
	case '\\':
		sfputc(op, ' ');
		sfputc(op, ' ');
		sfputc(op, '\\');
		return;
	}
	buf[0] = *(v = u);
	if ((w = mbtchar(&w, u, state->eob - u, &state->q)) > 0 && (i = u - v) > 1 && !iswprint(w))
	{
		state->mb = i - 1;
		for (j = 3 - mbwidth(w); j > 0; j--)
			sfputc(op, ' ');
		while (i--)
			sfputc(op, *v++);
		return;
	}
	buf[1] = 0;
	s = fmtesc(buf);
	if (*s != '\\')
	{
		sfputc(op, ' ');
		sfputc(op, ' ');
		sfputc(op, *s);
		return;
	}
	if (isdigit(*(s + 1)))
	{
		sfputc(op, *++s);
		sfputc(op, *++s);
		sfputc(op, *++s);
		return;
	}
	sfputc(op, ' ');
	sfputc(op, '\\');
	sfputc(op, *(s + 1));
}

static void
mform(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	register char*		s;
	char			buf[2];

	switch (buf[0] = ccmapchr(state->map, *u))
	{
	case 0:
		sfputc(op, '0');
		sfputc(op, '0');
		return;
	case '\\':
		sfputc(op, ' ');
		sfputc(op, '\\');
		return;
	}
	buf[1] = 0;
	s = fmtesc(buf);
	if (*s != '\\')
	{
		sfputc(op, ' ');
		sfputc(op, *s);
		return;
	}
	if (isdigit(*(s + 1)))
		sfprintf(op, "%02lx", strtol(s + 1, NiL, 8));
	else
	{
		sfputc(op, '\\');
		sfputc(op, *(s + 1));
	}
}

static void
Oform(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	unsigned char*	v;
	char*		s;
	wchar_t		w;
	int		i;
	int		j;
	char		buf[2];

	if (state->mb)
	{
		state->mb--;
		sfputc(op, ' ');
		sfputc(op, '*');
		sfputc(op, '*');
		return;
	}
	switch (*u)
	{
	case 0:
		sfputc(op, ' ');
		sfputc(op, '\\');
		sfputc(op, '0');
		return;
	case '\\':
		sfputc(op, ' ');
		sfputc(op, ' ');
		sfputc(op, '\\');
		return;
	}
	buf[0] = *(v = u);
	if ((w = mbtchar(&w, u, state->eob - u, &state->q)) > 0 && (i = u - v) > 1 && !iswprint(w))
	{
		state->mb = i - 1;
		for (j = 3 - mbwidth(w); j > 0; j--)
			sfputc(op, ' ');
		while (i--)
			sfputc(op, *v++);
		return;
	}
	buf[1] = 0;
	s = fmtesc(buf);

	/*
	 * posix shalls are sometimes an impediment to progress
	 */

	if (*s != '\\')
	{
		sfputc(op, ' ');
		sfputc(op, ' ');
		sfputc(op, *s);
		return;
	}
	switch (*(s + 1))
	{
	case 'b':
	case 'f':
	case 'n':
	case 'r':
	case 't':
		sfputc(op, ' ');
		sfputc(op, '\\');
		sfputc(op, *(s + 1));
		return;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		sfputc(op, *++s);
		sfputc(op, *++s);
		sfputc(op, *++s);
		return;
	}
	sfprintf(op, "%03o", *v);
}

static const Type_t	type[] =
{
{
	"ISO/IEC 646:1991 named characters",
	'a',	0,	0,	aform,	csize,	 3,  0,  0,  0,  0
},
{
	"hexadecimal floating point",
	'A',	0,	0,	0,	asize,	 0,  0,  0,  0,  0
},
{
	"binary character",
	'b',	0,	0,	bform,	csize,	 8,  0,  0,  0,  0
},
{
	"locale character or backslash escape (\\a \\b \\f \\n \\r \\t \\v)",
	'c',	0,	0,	cform,	csize,	 3,  0,  0,  0,  0
},
{
	"locale character or backslash escape (current and future escapes)",
	'C',	0,	0,	Cform,	csize,	 3,  0,  0,  0,  0
},
{
	"signed decimal",
	'd',	0,	0,	0,	isize,	 4,  6, 11, 21, 31
},
{
	"floating point",
	'f',	0,	0,	0,	fsize,	 0,  0,  0,  0,  0
},
{
	"\b--map\b mapped character or hexadecimal value if not printable",
	'm',	0,	0,	mform,	csize,	 2,  0,  0,  0,  0
},
{
	"octal",
	'o',	0,	"0",	0,	isize,	 3,  6, 11, 22, 33
},
{
	"locale character or backslash escape (\\b \\f \\n \\r \\t)",
	'O',	0,	0,	Oform,	csize,	 3,  0,  0,  0,  0
},
{
	"unsigned decimal",
	'u',	0,	0,	0,	isize,	 3,  5, 10, 20, 30
},
{
	"hexadecimal",
	'x',	0,	"0",	0,	isize,	 2,  4,  8, 16, 32
},
{
	"printable bytes",
	'z',	0,	0,	0,	0,	 1,  0,  0, 0, 0
},
};

static void
form_int8(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, *u);
}

static void
form_int16(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, (int16_t)swapget(state->swap, u, fp->size.external));
}

static void
form_int32(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, (int32_t)swapget(state->swap, u, fp->size.external));
}

static void
form_intmax(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, (intmax_t)swapget(state->swap, u, fp->size.external));
}

static void
form_uint8(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, *u);
}

static void
form_uint16(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, (uint16_t)swapget(state->swap, u, fp->size.external));
}

static void
form_uint32(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, (uint32_t)swapget(state->swap, u, fp->size.external));
}

static void
form_uintmax(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	sfprintf(op, fp->form, (uintmax_t)swapget(state->swap, u, fp->size.external));
}

static void
form_float(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	Mem_t	mem;

	swapmem(state->swap ^ int_swap, u, mem.m_char, fp->size.internal);
	sfprintf(op, fp->form, mem.m_float);
}

static void
form_double(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	Mem_t	mem;

	swapmem(state->swap ^ int_swap, u, mem.m_char, fp->size.internal);
	sfprintf(op, fp->form, mem.m_double);
}

#if _typ_long_double

static void
form_long_double(State_t* state, Format_t* fp, Sfio_t* op, unsigned char* u)
{
	Mem_t	mem;

	swapmem(state->swap ^ int_swap, u, mem.m_char, fp->size.internal);
	sfprintf(op, fp->form, mem.m_long_double);
}

#endif

/*
 * add format type t to the format list
 */

static void
format(State_t* state, register char* t)
{
	register int		c;
	register const Type_t*	tp;
	register const Size_t*	zp;
	register Format_t*	fp;
	const Size_t*		xp;
	uintmax_t		m;
	int			prec;
	int			base;
	int			n;
	int			i;
	char*			e;
	char*			s;
	char			dig[256];

	while (c = *t++)
	{
		if (isspace(c) || c == ',')
			continue;
		tp = type;
		while (c != tp->name)
			if (++tp >= &type[elementsof(type)])
			{
				error(2, "%c: invalid type name", c);
				return;
			}
		if (!(zp = tp->size) && !tp->fun)
		{
			switch (tp->width[0])
			{
			case 1:
				state->printable = 1;
				break;
			}
			continue;
		}
		xp = 0;
		if (isdigit(*t))
		{
			c = 0;
			n = (int)strton(t, &e, NiL, 1);
			t = e;
		}
		else
		{
			c = isupper(*t) ? *t++ : 0;
			n = 0;
		}
		for (;;)
		{
			if (!zp->name)
			{
				if (c)
				{
					error(2, "%c: invalid size for type %c", c, tp->name);
					return;
				}
				if (!(zp = xp) || zp->exponent)
				{
					error(2, "%d: invalid size for type %c", n, tp->name);
					return;
				}
				break;
			}
			if (n)
			{
				if (n == zp->size)
					break;
				zp++;
				if (n > (zp-1)->size && n < zp->size)
					xp = zp;
			}
			else if (c == zp->name)
			{
				if (c != *t)
					break;
				t++;
				zp++;
				if (zp->name)
					break;
			}
			else if (!c && !n && zp->dflt)
				break;
			else
				zp++;
		}
		if (tp->mb && (i = mbmax()) > 1)
			state->mbmax = i;
		i = zp - tp->size;
		if (!(fp = vmnewof(state->vm, 0, Format_t, 1, 0)))
		{
			error(ERROR_system(1), "out of space");
			return;
		}
		if (state->last)
			state->last = state->last->next = fp;
		else
			state->form = state->last = fp;
		fp->type = tp;
		fp->us = tp->name != 'd';
		fp->size.internal = zp->size;
		fp->size.external = n ? n : zp->size;
		if (state->size < fp->size.external)
			state->size = fp->size.external;
		if (zp->exponent)
		{
			if (zp->map == 'a')
			{
				for (m = 0, i = zp->digits; i-- > 0; m = m * 10 + 9);
				prec = sfsprintf(dig, sizeof(dig), "%I*x", sizeof(m), m);
			}
			else
				prec = zp->digits;
			base = sfsprintf(dig, sizeof(dig), "%u", zp->exponent);
			fp->width = zp->prefix + prec + base + 2;
		}
		else
		{
			fp->width = tp->width[i = WIDTHINDEX(fp->size.internal)];
			if (n > 1 && (n & (n - 1)))
			{
				c = (1 << i) - n;
				n = (1 << i) - (1 << (i - 1));
				fp->width -= ((fp->width - tp->width[i - 1] + n - 1) / n) * c;
			}
			prec = 0;
			base = 0;
		}
		if (!(fp->fun = tp->fun))
		{
			e = fp->form;
			*e++ = '%';
			if (s = (char*)tp->fill)
				while (*e = *s++)
					e++;
			e += sfsprintf(e, sizeof(fp->form) - (e - fp->form), "%d", fp->width);
			if (prec)
				e += sfsprintf(e, sizeof(fp->form) - (e - fp->form), ".%d", prec);
			if (base)
				e += sfsprintf(e, sizeof(fp->form) - (e - fp->form), "%s.%d", prec ? "" : ".", base);
			if (s = (char*)zp->qual)
				while (*e = *s++)
					e++;
			*e = zp->map ? zp->map : tp->name;
			if (zp->exponent)
			{
#if _typ_long_double
				if (fp->size.internal == sizeof(long double))
					fp->fun = form_long_double;
				else
#endif
				if (fp->size.internal == sizeof(double))
					fp->fun = form_double;
				else
					fp->fun = form_float;
			}
			else if (fp->us)
				switch (fp->size.internal)
				{
				case 1:
					fp->fun = form_uint8;
					break;
				case 2:
					fp->fun = form_uint16;
					break;
				case 4:
					fp->fun = form_uint32;
					break;
				default:
					fp->fun = form_uintmax;
					break;
				}
			else
				switch (fp->size.internal)
				{
				case 1:
					fp->fun = form_int8;
					break;
				case 2:
					fp->fun = form_int16;
					break;
				case 4:
					fp->fun = form_int32;
					break;
				default:
					fp->fun = form_intmax;
					break;
				}
		}
	}
}

static Sfio_t*
init(State_t* state, char*** p)
{
	Sfio_t*		ip;
	intmax_t	offset;

	for (;;)
	{
		if (**p)
			state->file = *((*p)++);
		else if (state->file)
			return 0;
		if (!state->file || streq(state->file, "-"))
		{
			state->file = "/dev/stdin";
			ip = sfstdin;
		}
		else if (!(ip = sfopen(NiL, state->file, "r")))
		{
			error(ERROR_system(0), "%s: cannot open", state->file);
			error_info.errors = 1;
			continue;
		}
		if (state->buffer.noshare)
			sfset(ip, SF_SHARE, 0);
		if (state->buffer.size)
			sfsetbuf(ip, state->buffer.base, state->buffer.size);
		if (state->skip)
		{
			if ((offset = sfseek(ip, (off_t)0, SEEK_END)) > 0)
			{
				if (offset <= state->skip)
				{
					state->skip -= offset;
					state->offset += offset;
					goto next;
				}
				if (sfseek(ip, state->skip, SEEK_SET) != state->skip)
				{
					error(ERROR_system(2), "%s: seek error", state->file);
					goto next;
				}
				state->offset += state->skip;
				state->skip = 0;
			}
			else
			{
				for (;;)
				{
					if (!(state->peek.data = sfreserve(ip, SF_UNBOUND, 0)))
					{
						if (sfvalue(ip))
							error(ERROR_system(2), "%s: read error", state->file);
						goto next;
					}
					state->peek.size = sfvalue(ip);
					if (state->peek.size < state->skip)
						state->skip -= state->peek.size;
					else
					{
						state->peek.data += state->skip;
						state->peek.size -= state->skip;
						state->skip = 0;
						break;
					}
				}
			}
		}
		break;
	next:
		if (ip != sfstdin)
			sfclose(ip);
	}
	return ip;
}

static int
block(State_t* state, Sfio_t* op, char* bp, char* ep, intmax_t base)
{
	register Format_t*	fp;
	register unsigned char*	u;
	register ssize_t	z;
	unsigned long		n;

	if (!state->verbose)
	{
		if (state->dup.size == (n = ep - bp) && !memcmp(state->dup.data, bp, n))
		{
			if (!state->dup.mark)
			{
				state->dup.mark = 1;
				sfprintf(op, "*\n");
			}
			return 0;
		}
		state->dup.mark = 0;
	}
	for (fp = state->form; fp; fp = fp->next)
	{
		if (*state->base)
		{
			if (fp == state->form)
				sfprintf(op, state->base, base);
			else
				sfprintf(op, "%-*.*s ", BASE_WIDTH, BASE_WIDTH, "");
		}
		u = (unsigned char*)bp;
		for (;;)
		{
			z = fp->per - fp->width;
			while (z-- > 0)
				sfputc(op, ' ');
			(*fp->fun)(state, fp, op, u);
			if ((u += fp->size.external) < (unsigned char*)ep)
				sfputc(op, ' ');
			else
			{
				if (state->printable && fp == state->form)
				{
					register int	c;
					unsigned char*	v;
					wchar_t		w;

					if (c = (state->block - (ep - bp)) / state->size * (state->width + 1))
						sfprintf(op, "%*s", c, "");
					sfputc(op, ' ');
					if (state->mbmax)
						for (u = (unsigned char*)bp; u < (unsigned char*)ep; u++)
						{
							if (state->mbp)
							{
								state->mbp--;
								sfputc(op, '.');
							}
							else if ((v = u) && (w = mbtchar(&w, v, state->eob - u, &state->q)) > 0 && (c = v - u) > 1 && !iswprint(w))
							{
								sfwrite(op, u, c);
								state->mbp = c - 1;
							}
							else
							{
								if ((c = ccmapchr(state->map, *u)) < 040 || c >= 0177)
									c = '.';
								sfputc(op, c);
							}
						}
					else
						for (u = (unsigned char*)bp; u < (unsigned char*)ep;)
						{
							if ((c = ccmapchr(state->map, *u++)) < 040 || c >= 0177)
								c = '.';
							sfputc(op, c);
						}
				}
				sfputc(op, '\n');
				break;
			}
		}
		if (sferror(op))
			return -1;
	}
	return 0;
}

static int
od(State_t* state, char** files)
{
	register char*	s;
	register char*	e;
	register char*	x;
	register char*	span = 0;
	register int	c;
	Sfio_t*		ip;
	size_t		n;
	size_t		m;
	ssize_t		r;

	if (!(ip = init(state, &files)))
		return 0;
	for (;;)
	{
		if (sh_checksig(state->context))
			goto bad;
		if (s = state->peek.data)
		{
			state->peek.data = 0;
			n = state->peek.size;
		}
		else
			for (;;)
			{
				s = sfreserve(ip, SF_UNBOUND, 0);
				n = sfvalue(ip);
				if (s)
					break;
				if (n)
					error(ERROR_system(2), "%s: read error", state->file);
				if (ip != sfstdin)
					sfclose(ip);
				if (!(ip = init(state, &files)))
				{
					s = 0;
					n = 0;
					break;
				}
			}
		state->eob = (unsigned char*)s + n;
		if (state->count)
		{
			if (state->total >= state->count)
			{
				s = 0;
				n = 0;
			}
			else if ((state->total += n) > state->count)
				n -= state->total - state->count;
		}
		if (span)
		{
		respan:
			if (s)
			{
				m = state->spansize - (span - state->span);
				if (m > n)
				{
					memcpy(span, s, n);
					span += n;
					continue;
				}
				memcpy(span, s, m);
				span += m;
				s += m;
				n -= m;
			}
			if (m = (span - state->span) % state->size)
			{
				m = state->size - m;
				while (m--)
					*span++ = 0;
			}
			state->eob = (unsigned char*)span;
			if ((m = span - state->span) > state->block)
				m = state->block;
			if (block(state, sfstdout, state->span, state->span + m, state->offset))
				goto bad;
			state->offset += m;
			if (!state->verbose)
				memcpy(state->dup.data = state->dup.buf, state->span, state->dup.size = m);
			if ((r = (span - state->span) - m) > 0)
			{
				span = state->span;
				x = state->span + m;
				while (r--)
					*span++ = *x++;
				goto respan;
			}
			span = 0;
			if (s && !n)
				continue;
		}
		if (!s)
			break;
		x = s + n;
		if (state->strings)
		{
			state->offset += n;
			n = 0;
			for (;;)
			{
				if (s >= x || (c = *s++) == 0 || c == '\n' || !isprint(c))
				{
					if (n >= state->strings)
					{
						if (*state->base && sfprintf(sfstdout, state->base, state->offset - (x - s) - n - 1) < 0)
							break;
						if (sfprintf(sfstdout, "%.*s\n", n, s - n - 1) < 0)
							break;
					}
					if (s >= x)
						break;
					n = 0;
				}
				else
					n++;
			}
		}
		else
		{
			e = s + (n / state->block) * state->block;
			if (state->mbmax)
				e -= state->block + state->mbmax;
			if (s < e)
			{
				do
				{
					if (block(state, sfstdout, s, s + state->block, state->offset))
						goto bad;
					state->offset += state->block;
					state->dup.data = s;
					state->dup.size = state->block;
				} while ((s += state->block) < e);
				if (!state->verbose)
				{
					memcpy(state->dup.buf, state->dup.data, state->dup.size = state->block);
					state->dup.data = state->dup.buf;
				}
			}
			if (n = x - s)
			{
				memcpy(state->span, s, n);
				span = state->span + n;
			}
		}
	}
	return 0;
 bad:
	if (ip != sfstdin)
		sfclose(ip);
	return -1;
}

/*
 * optinfo() size description
 */

static int
optsize(Sfio_t* sp, const Size_t* zp)
{
	register int	n;

	for (n = 0; zp->name; zp++)
		if (zp->qual && (*zp->qual != zp->name || *(zp->qual + 1)))
			n += sfprintf(sp, "[%c|%s?sizeof(%s)]", zp->name, zp->qual, zp->desc);
		else
			n += sfprintf(sp, "[%c?sizeof(%s)]", zp->name, zp->desc);
	return n;
}

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register iconv_list_t*	ic;
	register int		i;
	register int		n;

	n = 0;
	switch (*s)
	{
	case 'c':
		for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
			if (ic->ccode >= 0)
				n += sfprintf(sp, "[%c:%s?%s]", ic->match[ic->match[0] == '('], ic->name, ic->desc);
		break;
	case 't':
		for (i = 0; i < elementsof(type); i++)
			n += sfprintf(sp, "[%c?%s]", type[i].name, type[i].desc);
		n += sfprintf(sp, "[+----- sizes -----?]");
		n += optsize(sp, isize);
		n += optsize(sp, fsize);
		break;
	}
	return n;
}

int
b_od(int argc, char** argv, Shbltin_t* context)
{
	register int		n;
	register char*		s;
	register Format_t*	fp;
	char*			e;
	int			per;
	char			buf[4];
	Optdisc_t		optdisc;
	State_t			state;

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	memset(&state, 0, sizeof(state));
	if (!(state.vm = vmopen(Vmdcheap, Vmlast, 0)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		return 1;
	}
	state.context = context;
	optinit(&optdisc, optinfo);
	per = 0;
	state.map = ccmap(CC_ASCII, CC_ASCII);
	state.swap = int_swap;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'A':
			*state.base = *opt_info.arg;
			state.style |= NEW;
			continue;
		case 'B':
			if (opt_info.num <= 0)
				state.swap = -opt_info.num;
			else
				state.swap ^= opt_info.num;
			continue;
		case 'j':
			if (*opt_info.option == '+')
			{
				opt_info.index--;
				break;
			}
			state.skip = opt_info.num;
			state.style |= NEW;
			continue;
		case 'm':
			if ((n = ccmapid(opt_info.arg)) < 0)
				error(2, "%s: unknown character code set", opt_info.arg);
			else
				state.map = ccmap(n, CC_NATIVE);
			continue;
		case 'N':
			state.count = opt_info.num;
			state.style |= NEW;
			continue;
		case 'p':
			state.printable = 1;
			continue;
		case 't':
			format(&state, opt_info.arg);
			state.style |= NEW;
			continue;
		case 'T':
			s = opt_info.arg;
			switch (*s)
			{
			case 'b':
			case 'm':
				n = *s++;
				state.buffer.size = strton(s, &e, NiL, 1);
				if (n == 'b' && !(state.buffer.base = vmnewof(state.vm, 0, char, state.buffer.size, 0)))
				{
					error(ERROR_SYSTEM|2, "out of space");
					goto done;
				}
				if (*e)
					error(2, "%s: invalid characters after test", e);
				break;
			case 'n':
				state.buffer.noshare = 1;
				break;
			default:
				error(2, "%s: unknown test", s);
				break;
			}
			continue;
		case 'v':
			state.verbose = 1;
			state.style |= NEW;
			continue;
		case 'w':
			per = opt_info.num;
			continue;
		case 'z':
			state.strings = opt_info.num;
			continue;
		case 'a':
		case 'b':
		case 'c':
		case 'C':
		case 'd':
		case 'D':
		case 'f':
		case 'F':
		case 'h':
		case 'i':
		case 'l':
		case 'o':
		case 'O':
		case 's':
		case 'S':
		case 'u':
		case 'U':
		case 'x':
		case 'X':
			s = buf;
			switch (n = opt_info.option[1])
			{
			case 'b':
				*s++ = 'o';
				*s++ = 'C';
				break;
			case 'c':
			case 'C':
				*s++ = 'O';
				break;
			case 'D':
				n = 'U';
				break;
			case 'd':
				n = 'u';
				break;
			case 'h':
				n = 'x';
				break;
			case 'i':
			case 's':
				n = 'd';
				break;
			case 'l':
			case 'S':
				n = 'D';
				break;
			}
			if (s == buf)
			{
				if (isupper(n))
					switch (*s++ = tolower(n))
					{
					case 'f':
						*s++ = 'D';
						break;
					default:
						*s++ = 'L';
						break;
					}
				else
					switch (*s++ = n)
					{
					case 'd':
					case 'o':
					case 's':
					case 'u':
					case 'x':
						/* pronounce that! */
						*s++ = 'S';
						break;
					case 'f':
						*s++ = 'F';
						break;
					}
			}
			*s = 0;
			format(&state, buf);
			state.style |= OLD;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
	{
		vmclose(state.vm);
		error(ERROR_usage(2), "%s", optusage(NiL));
	}
	switch (n = *state.base)
	{
	case 'n':
		*state.base = 0;
		break;
	case 0:
		n = 'o';
		/*FALLTHROUGH*/
	case 'o':
		state.addr = 8;
		goto base;
	case 'd':
		state.addr = 10;
		goto base;
	case 'x':
		state.addr = 16;
		/*FALLTHROUGH*/
	base:
		sfsprintf(state.base, sizeof(state.base), "%%0%d%s%c ", BASE_WIDTH, QUAL, n);
		break;
	default:
		error(2, "%c: invalid addr-base type", n);
		break;
	}
	if (!state.form)
		format(&state, "oS");
	else if (state.strings)
		error(2, "--strings must be the only format type");
	if (error_info.errors)
		goto done;
	for (fp = state.form; fp; fp = fp->next)
		if ((n = (state.size / fp->size.external) * (fp->width + 1) - 1) > state.width)
			state.width = n;
	for (fp = state.form; fp; fp = fp->next)
		fp->per = state.width * fp->size.external / state.size;
	n = LINE_LENGTH - state.printable;
	if (*state.base)
		n -= (BASE_WIDTH + 1);
	if (!(n /= (state.width + state.printable * state.size + 1)))
		n = 1;
	if (state.addr)
	{
		while (n > state.addr)
			state.addr *= 2;
		while (n < state.addr)
			state.addr /= 2;
		if (state.addr)
			n = state.addr;
	}
	if (per)
		n = per;
	state.block = n * state.size;
	state.spansize = state.block + state.mbmax;
	if (!(state.span = vmnewof(state.vm, 0, char, state.spansize, 0)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	if (!(state.style & NEW) && (s = *argv))
	{
		e = "?(+)+([0-9])?(.)?([bkm])?(ll|LL)";
		if (!*(argv + 1))
		{
			if (strmatch(s, e) && *s == '+')
				argv++;
			else s = 0;
		}
		else if (!*(argv + 2))
		{
			s = *(argv + 1);
			if (strmatch(s, e) && (state.style == OLD || *s == '+'))
				*(argv + 1) = 0;
			else s = 0;
		}
		else s = 0;
		if (s)
		{
			state.skip = strtol(s, &e, strchr(s, '.') ? 10 : 8);
			if (*e == '.')
				e++;
			switch (*e)
			{
			case 'b':
				state.skip *= 512;
				break;
			case 'k':
				state.skip *= 1024;
				break;
			case 'm':
				state.skip *= 1024 * 1024;
				break;
			}
		}
	}
#ifdef SIGFPE
	signal(SIGFPE, SIG_IGN);
#endif
	od(&state, argv);
#ifdef SIGFPE
	signal(SIGFPE, SIG_DFL);
#endif
	if (state.skip)
	{
		error(2, "cannot skip past available data");
		goto done;
	}
	if (*state.base && !state.strings)
	{
		*(state.base + strlen(state.base) - 1) = '\n';
		sfprintf(sfstdout, state.base, (intmax_t)state.offset);
	}
	if (sfsync(sfstdout) && !ERROR_PIPE(errno))
		error(ERROR_SYSTEM|2, "write error");
 done:
	vmclose(state.vm);
	return error_info.errors != 0;
}
