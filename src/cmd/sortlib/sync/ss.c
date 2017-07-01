/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * ibm dfsort control file parser and library support
 */

#include <ast.h>
#include <error.h>
#include <ccode.h>
#include <ctype.h>
#include <dlldefs.h>
#include <ls.h>

#ifndef DEBUG_TRACE
#define DEBUG_TRACE	0
#endif

#define _SS_PRIVATE_ \
	Sfio_t*		io; \
	Sfio_t*		buf; \
	Sfio_t*		xio; \
	char*		xsum; \
	unsigned char*	n2a; \
	unsigned char*	n2e; \
	char*		data; \
	char*		peek; \
	char*		peekpeek; \
	int		ch; \
	int		item; \
	int		part; \
	char		acex[16]; \
	char		chex[16]; \
	unsigned char	acin[UCHAR_MAX+1]; \
	unsigned char	chin[UCHAR_MAX+1];

#define _SS_FILE_PRIVATE_ \
	Ssgroup_t*	lastgroup;

#define _SS_GROUP_PRIVATE_ \
	unsigned char*	beg; \
	unsigned char*	cur; \
	unsigned char*	end;

#include <ss.h>

struct Dd_s; typedef struct Dd_s Dd_t;

struct Dd_s
{
	Dd_t*		next;
	char*		id;
	char*		alt;
	char*		name;
};

static struct State_s
{
	Dd_t*		dd;
} state;

#define LEX_COND	(1<<0)
#define LEX_COPY	(1<<1)
#define LEX_GROUP	(1<<2)
#define LEX_JOIN	(1<<3)
#define LEX_NONE	(1<<4)

#define SS_BLOCK	1024
#define SS_RESERVE	(128*SS_BLOCK)

static char		null[] = "";

static char		CO[] = ":";
static char		CP[] = ")";
static char		EQ[] = "=";
static char		OP[] = "(";
static char		SC[] = ";";

/*
 * bcd_pack[x] is bcd byte for 0<=x<=99
 */

static const unsigned char	bcd_pack[] =
{
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
};

/*
 * bcd_unit[x] is bcd units (last byte with no sign) byte for 0<=x<=9
 */

static const unsigned char	bcd_unit[] =
{
    0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,
};

/*
 * bcd_unpack[x] is the binary value for bcd byte x
 * invalid codes convert to 0
 */

static const unsigned char	bcd_unpack[] =
{
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    10,11,12,13,14,15,16,17,18,19, 0, 0, 1, 1, 0, 0,
    20,21,22,23,24,25,26,27,28,29, 0, 0, 2, 2, 0, 0,
    30,31,32,33,34,35,36,37,38,39, 0, 0, 3, 3, 0, 0,
    40,41,42,43,44,45,46,47,48,49, 0, 0, 4, 4, 0, 0,
    50,51,52,53,54,55,56,57,58,59, 0, 0, 5, 5, 0, 0,
    60,61,62,63,64,65,66,67,68,69, 0, 0, 6, 6, 0, 0,
    70,71,72,73,74,75,76,77,78,79, 0, 0, 7, 7, 0, 0,
    80,81,82,83,84,85,86,87,88,89, 0, 0, 8, 8, 0, 0,
    90,91,92,93,94,95,96,97,98,99, 0, 0, 9, 9, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/*
 * bcd_negative[x]!=0 if bcd sign is negative
 */

static const unsigned char	bcd_negative[] =
{
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
};

/*
 * common syntax error message
 */

static void
syntax(Ss_t* ss, char* s)
{
	if (ss->disc->errorf)
	{
		if (s)
			(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: unknown keyword", s);
		else
			(*ss->disc->errorf)(NiL, ss->disc, 2, "unexpected EOF");
	}
}

/*
 * push back one token
 */

#define xel(o,s)	((o)->peekpeek=(o)->peek,(o)->peek=(s))

static void		listexpr(Ss_t*, Sfio_t*, Ssexpr_t*);

/*
 * return the next token
 */

static char*
lex(register Ss_t* ss)
{
	register char*	s;
	register char*	t;
	register int	q;

	if (s = ss->peek)
	{
		ss->peek = ss->peekpeek;
		ss->peekpeek = 0;
		ss->item++;
#if DEBUG_TRACE
sfprintf(sfstderr, "xel: %s\n", s);
#endif
		return s;
	}
	for (;;)
	{
		switch (*ss->data)
		{
		case ',':
			ss->data++;
			ss->part = 1;
			continue;
		case ' ':
		case '\t':
		case '\r':
			if (ss->item < 2)
			{
				ss->data++;
				continue;
			}
			/*FALLTHROUGH*/
		case '*':
		case 0:
			if (!ss->part)
			{
				ss->part = 1;
				ss->data = null;
#if DEBUG_TRACE
sfprintf(sfstderr, "lex: null\n");
#endif
				return SC;
			}
			do
			{
				if (!(ss->data = sfgetr(ss->io, '\n', 1)))
				{
					ss->data = null;
					return 0;
				}
				error_info.line++;
			} while ((q = sfvalue(ss->io)) <= 0);
			if (q > 72)
				ss->data[72] = 0;
			ss->item = 0;
#if DEBUG_TRACE
sfprintf(sfstderr, "lex: line=`%s'\n", ss->data);
#endif
			continue;
		}
		break;
	}
	ss->part = 0;
	s = ss->data;
	q = 0;
	for (;;)
	{
		switch (*ss->data++)
		{
		case 0:
			ss->data = null;
			ss->item++;
			return s;
		case '=':
			if (!q)
			{
				if ((ss->data - s) == 1)
				{
					ss->item++;
					return EQ;
				}
				xel(ss, EQ);
				break;
			}
			continue;
		case ':':
			if (!q)
			{
				if ((ss->data - s) == 1)
				{
					ss->item++;
					return CO;
				}
				xel(ss, CO);
				break;
			}
			continue;
		case '(':
			if (!q)
			{
				if ((ss->data - s) == 1)
				{
					ss->item++;
					return OP;
				}
				xel(ss, OP);
				break;
			}
			continue;
		case ')':
			if (!q)
			{
				if ((ss->data - s) == 1)
				{
					ss->item++;
					return CP;
				}
				xel(ss, CP);
				break;
			}
			continue;
		case ',':
			if (!q)
			{
				ss->part = 1;
				break;
			}
			continue;
		case ' ':
		case '\t':
		case '\r':
			if (!q)
				break;
			continue;
		case '\'':
			if (q && *ss->data == '\'')
			{
				for (t = ++s; t < ss->data; t++)
					*t = *(t - 1);
				ss->data++;
			}
			else
				q = !q;
			continue;
		default:
			continue;
		}
		break;
	}
	*(ss->data - 1) = 0;
	ss->item++;
#if DEBUG_TRACE
sfprintf(sfstderr, "lex: %s part=%d item=%d\n", s, ss->part, ss->item);
#endif
	return s;
}

/*
 * return the first variable value token
 * '=' and '(' if (flags&LEX_GROUP) are eaten
 */

static char*
value(Ss_t* ss, int flags)
{
	register char*	s;

	if (!(s = lex(ss)) || (flags & LEX_COND) && streq(s, "COND") && !(s = lex(ss)))
	{
		syntax(ss, s);
		return 0;
	}
	if (s == OP && (flags & LEX_GROUP))
		/* apparently accepted by dfsort */;
	else if (s != EQ || !(s = lex(ss)) ||
	    (flags & LEX_GROUP) && s != OP &&
	    (!(flags & LEX_NONE) || streq(s, "NONE") && (s = 0)) &&
	    (!(flags & LEX_COPY) || streq(s, "COPY") && (s = 0)))
	{
		if (ss->disc->errorf)
		{
			if (s)
				(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: %svalue expected", s, (flags & LEX_GROUP) ? "group " : null);
			else
				(*ss->disc->errorf)(NiL, ss->disc, 2, "unexpected EOF");
		}
		return 0;
	}
	return s;
}

/*
 * return the type in s
 */

#define CH(a,b,c)		(((a)<<16)|((b)<<8)|(c))

static int
type(Ss_t* ss, char* s, int silent)
{
	int	n;

	switch (CH(toupper(s[0]),toupper(s[0]?s[1]:0),toupper(s[1]?s[2]:0)))
	{
	case CH('A','C',0):
		n = SS_ascii;
		break;
	case CH('A','D',0):
		n = SS_AC_dec;
		break;
	case CH('A','H',0):
		n = SS_AC_hex;
		break;
	case CH('A','O',0):
		n = SS_AC_oct;
		break;
	case CH('A','Q',0):
		n = SS_AC_alt;
		break;
	case CH('B','I',0):
		n = SS_bit;
		break;
	case CH('C','H',0):
		n = ss->ch;
		break;
	case CH('C','S','F'):
	case CH('E','D',0):
	case CH('F','S',0):
		n = ss->ch == CC_ASCII ? SS_AC_dec : SS_CH_dec;
		break;
	case CH('C','T','O'):
	case CH('O','T',0):
	case CH('Z','D',0):
		n = SS_zd;
		break;
	case CH('E','H',0):
		n = ss->ch == CC_ASCII ? SS_AC_hex : SS_CH_hex;
		break;
	case CH('E','O',0):
		n = ss->ch == CC_ASCII ? SS_AC_oct : SS_CH_oct;
		break;
	case CH('F','I',0):
		n = SS_be;
		break;
	case CH('L','I',0):
		n = SS_le;
		break;
	case CH('P','D',0):
	case CH('P','D','O'):
		n = SS_bcd;
		break;
	case CH('S','S',0):
		n = SS_ss;
		break;
	default:
		if (silent)
			xel(ss, s);
		else if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid type", s);
		n = 0;
		break;
	}
	return n;
}

/*
 * set the ch codeset to n
 */

static void
codeset(register Ss_t* ss, register int n)
{
	register char*	s;
	register int	c;

	if (!n)
		n = CC_EBCDIC_O;
	ss->n2a = ccmap(CC_NATIVE, CC_ASCII);
	ss->n2e = ccmap(CC_NATIVE, n);
	ss->ch = n == CC_ASCII ? SS_ascii : SS_ebcdic;
	memset(ss->acin, UCHAR_MAX, UCHAR_MAX + 1);
	memset(ss->chin, UCHAR_MAX, UCHAR_MAX + 1);
	for (n = 0, s = "0123456789"; *s; s++, n++)
	{
		c = ccmapchr(ss->n2a, *s);
		ss->acin[c] = n;
		ss->acex[n] = c;
		c = ccmapchr(ss->n2e, *s);
		ss->chin[c] = n;
		ss->chex[n] = c;
	}
	for (n = 10, s = "abcdef"; *s; s++, n++)
	{
		c = ccmapchr(ss->n2a, *s);
		ss->acin[c] = n;
		ss->acex[n] = c;
		c = ccmapchr(ss->n2e, *s);
		ss->chin[c] = n;
		ss->chex[n] = c;
	}
	for (n = 10, s = "ABCDEF"; *s; s++, n++)
	{
		c = ccmapchr(ss->n2a, *s);
		ss->acin[c] = n;
		c = ccmapchr(ss->n2e, *s);
		ss->chin[c] = n;
	}
}

/*
 * eat the remainder of a =(...) value
 */

static int
eat(Ss_t* ss)
{
	register char*	s;

	while (s = lex(ss))
	{
		if (s == CP)
			return 0;
		if (s == SC)
		{
			xel(ss, s);
			return 0;
		}
	}
	syntax(ss, s);
	return -1;
}

/*
 * parse a field tuple list
 * (flags & LEX_JOIN) collapses adjacent fields
 * zp!=0 is set to the total size
 */

static Ssfield_t*
fields(Ss_t* ss, int tuple, int flags, size_t* zp)
{
	register char*		s;
	register char*		v;
	char*			e;
	unsigned char*		map;
	register Ssfield_t*	dp;
	register Ssfield_t*	bp;
	register Ssfield_t*	ep;
	size_t			n;
	size_t			z;
	int			a;
	int			c;
	int			x;

	if (!(s = value(ss, flags|LEX_GROUP|LEX_NONE)))
		return 0;
	if ((flags & LEX_COPY) && streq(s, "COPY"))
	{
		ss->copy = 1;
		return 0;
	}
	z = 0;
	bp = dp = 0;
	if (s == OP)
	{
		if (!(s = lex(ss)))
			goto msg;
		if (isdigit(*s))
		{
			a = tuple;
			do
			{
				if (s == CO)
					a = 0;
				else
				{
					if (++a > tuple && (a != 3 || !isalpha(*s) || !isalpha(*(s + 1))))
					{
						if (!bp)
						{
							bp = ep = dp;
							dp = 0;
						}
						else if ((flags & LEX_JOIN) && (ep->offset + ep->size) == dp->offset && !dp->value && !ep->value && dp->reverse == ep->reverse)
							ep->size += dp->size;
						else
						{
							ep = ep->next = dp;
							dp = 0;
						}
						if (!dp && !(dp = vmnewof(ss->vm, 0, Ssfield_t, 1, 0)))
						{
							if (ss->disc->errorf)
								(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
							return 0;
						}
						a = 1;
					}
					switch (a)
					{
					case 1:
					case 2:
						if ((x = toupper(*s)) == 'E' && streq(s, "EDIT"))
						{
							if (ss->disc->errorf)
								(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: not supported", s);
							if (!(s = value(ss, LEX_GROUP)) || eat(ss))
								return 0;
						}
						else if (x == 'L' && streq(s, "LENGTH"))
						{
							if (ss->disc->errorf)
								(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: not supported", s);
							if (!(s = value(ss, 0)))
								return 0;
						}
						else if (x == 'S' && streq(s, "SIGNS"))
						{
							if (ss->disc->errorf)
								(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: not supported", s);
							if (!(s = value(ss, LEX_GROUP)) || eat(ss))
								return 0;
						}
						else
						{
							n = strtoul(s, &e, 10);
							if (((x = toupper(*e)) == 'A' || x == 'B' || x == 'C' || x == 'X') && *(e + 1) == '\'' && *(v = e + strlen(e) - 1) == '\'')
							{
								s = e + 2;
								e = v;
								c = e - s;
								if (!n)
									n = 1;
								if (!(dp->value = vmnewof(ss->vm, 0, char, n * c, 1)))
								{
									if (ss->disc->errorf)
										(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
									return 0;
								}
								if (x == 'B')
								{
									for (v = dp->value; s < e; s += 8)
										*v++ = (unsigned char)strntoul(s, 8, NiL, 2);
									c = v - dp->value;
								}
								else if (x == 'X')
								{
									for (v = dp->value; s < e; s += 2)
										*v++ = (unsigned char)strntoul(s, 2, NiL, 16);
									c = v - dp->value;
								}
								else
								{
									strcpy(dp->value, s);
									dp->value[c] = 0;
									c = stresc(dp->value);
									if (x == 'C' && (map = ss->n2e) || x == 'A' && (map = ss->n2a))
										ccmapcpy(map, dp->value, dp->value, c);
								}
								z += (dp->size = n * c);
								s = dp->value;
								while (--n)
									memcpy(s += c, dp->value, c);
								a = 2;
							}
							else if ((x == 'X' || x == 'Z') && !*(e + 1))
							{
								if (x == 'X')
									c = ' ';
								else
									c = 0;
								if (!n)
									n = 1;
								if (!(dp->value = vmnewof(ss->vm, 0, char, n, 0)))
								{
									if (ss->disc->errorf)
										(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
									return 0;
								}
								z += (dp->size = n);
								while (n--)
									dp->value[n] = c;
								a = 2;
							}
							else
							{
								switch (a)
								{
								case 1:
									dp->offset = n - 1;
									break;
								case 2:
									z += (dp->size = n);
									break;
								}
								if (*e)
								{
									if (ss->disc->errorf)
										(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid number", s);
									return 0;
								}
							}
						}
						break;
					case 3:
						if (tuple != 3)
						{
							if (tuple > 3 && !bp && !*(s + 1))
								tuple = 3;
							else if (dp->type = type(ss, s, 0))
								break;
							else
								return 0;
							tuple = 3;
						}
						/*FALLTHROUGH*/
					case 4:
						switch (*s)
						{
						case 'a':
						case 'A':
							break;
						case 'd':
						case 'D':
							dp->reverse = 1;
							break;
						default:
							if (ss->disc->errorf)
								(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid sort attribute", s);
							return 0;
						}
					}
				}
				if (!(s = lex(ss)))
					goto msg;
			} while (s != CP);
		}
		else if ((flags & LEX_COPY) && streq(s, "COPY"))
		{
			if ((s = lex(ss)) != CP)
				goto msg;
			ss->copy = 1;
			return 0;
		}
		else if ((flags & LEX_NONE) && streq(s, "NONE"))
		{
			if ((s = lex(ss)) != CP)
				goto msg;
			return 0;
		}
		else
			xel(ss, s);
	}
	if (!bp)
		bp = dp;
	else if ((flags & LEX_JOIN) && (ep->offset + ep->size) == dp->offset && !dp->value && !ep->value && dp->reverse == ep->reverse)
		ep->size += dp->size;
	else
		ep->next = dp;
	if (zp)
		*zp = z;
	return bp;
 msg:
	syntax(ss, s);
	return 0;
}

/*
 * parse a field option list
 */

static int
options(Ss_t* ss, int flags, Ssfile_t* fp, Ssfield_t** dp, int tuple, size_t* zp)
{
	register char*		s;
	register Ssfield_t*	ip;
	char*			e;
	int			n;

	if (dp)
		*dp = 0;
	while (s = lex(ss))
	{
		if (s == SC)
			break;
		else if (s == OP || s == CP)
			;
		else if (streq(s, "CODE") || streq(s, "CODESET"))
		{
			if (!(s = value(ss, 0)))
				return -1;
			if ((n = ccmapid(s)) < 0)
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid codeset name", s);
				return -1;
			}
			codeset(ss, n);
		}
		else if (streq(s, "COPY"))
			ss->copy = 1;
		else if (streq(s, "CONVERT") || streq(s, "VTOF"))
		{
			if (!fp)
				goto msg;
			fp->format = REC_F_TYPE(0);
		}
		else if (streq(s, "EQUALS"))
			ss->stable = 'Y';
		else if (streq(s, "NOEQUALS"))
			ss->stable = 'N';
		else if (streq(s, "FIELDS"))
		{
			n = ss->copy;
			if (dp && !(*dp = fields(ss, tuple, flags, zp)) && ss->copy == n)
				return -1;
		}
		else if (streq(s, "FORMAT"))
		{
			if (!(s = value(ss, 0)) || !(n = type(ss, s, 0)))
				return -1;
			for (ip = *dp; ip; ip = ip->next)
				ip->type = n;
		}
		else if (streq(s, "FTOV"))
		{
			if (!fp)
				goto msg;
			fp->format = SS_V_IBM;
		}
		else if (streq(s, "LENGTH"))
		{
			if (!(s = value(ss, 0)))
				return -1;
			if ((n = s == OP) && !(s = lex(ss)))
				goto msg;
			ss->size = strtoul(s, &e, 10);
			if (*e)
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid number", s);
				return -1;
			}
			if (n && eat(ss))
				return -1;
		}
		else if (streq(s, "TYPE"))
		{
			if (!(s = value(ss, 0)))
				return -1;
			switch (*(e = s))
			{
			case 'd':
			case 'D':
				ss->type = 'D';
				e++;
				break;
			case 'f':
			case 'F':
				ss->type = 'F';
				e++;
				break;
			case 'v':
			case 'V':
				if (*++e == 'b' || *e == 'B')
				{
					e++;
					ss->type = 'B';
				}
				else
					ss->type = 'V';
				break;
			}
			if (*e)
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid record type", s);
				return -1;
			}
		}
		else if (streq(s, "SIZE"))
		{
			if (!(s = value(ss, 0)))
				return -1;
		}
		else if (streq(s, "SKIPREC"))
		{
			if (!(s = value(ss, 0)))
				return -1;
			ss->skip = strtoull(s, &e, 10);
			if (*e)
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid number", s);
				return -1;
			}
		}
		else if (streq(s, "STOPAFT"))
		{
			if (!(s = value(ss, 0)))
				return -1;
			ss->stop = strtoull(s, &e, 10) + 1;
			if (*e)
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid number", s);
				return -1;
			}
		}
		else if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, 1, "%s: unknown option", s);
	}
	return 0;
 msg:
	syntax(ss, s);
	return -1;
}

static Ssexpr_t*	compile(Ss_t*, int);

/*
 * return the next expression operand
 */

static Ssexpr_t*
operand(Ss_t* ss, Ssexpr_t* lp)
{
	register Ssexpr_t*	xp;
	register Ssfield_t*	dp;
	register char*		s;
	register char*		v;
	char*			e;
	unsigned char*		map;
	int			c;

	if (!(s = lex(ss)))
	{
		syntax(ss, s);
		return 0;
	}
	else if (s == OP)
	{
		if (!(xp = compile(ss, 1)))
			return 0;
		if ((s = lex(ss)) != CP)
		{
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: ) expected", s);
			return 0;
		}
	}
	else if (!(xp = vmnewof(ss->vm, 0, Ssexpr_t, 1, 0)))
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	else if (streq(s, "NOT"))
	{
		xp->op = SS_OP_not;
		if (!(xp->left.expr = compile(ss, 13)))
			return 0;
	}
	else if (!(dp = vmnewof(ss->vm, 0, Ssfield_t, 1, 0)))
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	else
	{
		xp->left.field = dp;
		if (isdigit(*s))
		{
			xp->op = SS_OP_field;
			dp->offset = strtol(s, &e, 10);
			if (*e)
			{
				if (ss->disc->errorf)
				{
					if (lp)
						(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid number", s);
					else
						(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: field offset expected", s);
				}
				return 0;
			}
			if (!(s = lex(ss)))
			{
				syntax(ss, s);
				return 0;
			}
			if (isdigit(*s))
			{
				dp->offset--;
				dp->size = strtoul(s, &e, 10);
				if (*e)
				{
					if (ss->disc->errorf)
						(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: field size expected", s);
					return 0;
				}
				if (!(s = lex(ss)))
				{
					syntax(ss, s);
					return 0;
				}
				dp->type = type(ss, s, 1);
			}
			else
			{
				xel(ss, s);
				if (lp)
				{
					dp->type = lp->left.field->type;
					dp->size = lp->left.field->size;
				}
				xp->op = SS_OP_value;
			}
		}
		else if (((c = toupper(*s)) == 'C' || c == 'A') && *(s + 1) == '\'')
		{
			e = s + strlen(s) - 1;
			if (*e != '\'' || *(e + 1))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: invalid string constant", s);
				return 0;
			}
			s += 2;
			dp->size = e - s;
			if (!(dp->value = vmnewof(ss->vm, 0, char, dp->size, 1)))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
				return 0;
			}
			memcpy(dp->value, s, dp->size);
			dp->size = stresc(dp->value);
			if (map = c == 'C' ? ss->n2e : ss->n2a)
				ccmapstr(map, dp->value, dp->size);
			xp->op = SS_OP_value;
		}
		else if (c == 'X' && *(s + 1) == '\'')
		{
			e = s + strlen(s) - 1;
			if (*e != '\'')
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: invalid hex constant", s);
				return 0;
			}
			s += 2;
			dp->size = (e - s + 1) / 2;
			if (!(dp->value = vmnewof(ss->vm, 0, char, dp->size, 0)))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
				return 0;
			}
			for (v = dp->value; s < e; s += 2)
				*v++ = strntoul(s, 2, NiL, 16);
			xp->op = SS_OP_value;
		}
		else if (c == 'B' && *(s + 1) == '\'')
		{
			e = s + strlen(s) - 1;
			if (*e != '\'')
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: invalid hex constant", s);
				return 0;
			}
			s += 2;
			dp->size = (e - s + 7) / 8;
			if (!(dp->value = vmnewof(ss->vm, 0, char, dp->size, 0)))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
				return 0;
			}
			for (v = dp->value; s < e; s += 8)
				*v++ = strntoul(s, 8, NiL, 2);
			xp->op = SS_OP_value;
		}
		else
		{
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: operand expected", s);
			return 0;
		}
	}
	return xp;
}

/*
 * compile an expression or subexpression
 */

static Ssexpr_t*
compile(Ss_t* ss, int precedence)
{
	register Ssexpr_t*	xp;
	register Ssexpr_t*	lp;
	register Ssexpr_t*	rp;
	register char*		s;
	int			op;
	int			pr;

	if (!(xp = operand(ss, NiL)))
		return 0;
	for (;;)
	{
#if DEBUG_TRACE
sfprintf(sfstderr, "compile(%d) ", precedence); listexpr(ss, sfstderr, xp); sfprintf(sfstderr, "\n");
#endif
		if (!(s = lex(ss)))
		{
			if (precedence)
				syntax(ss, s);
			break;
		}
		else if (s == CP)
		{
			xel(ss, s);
			break;
		}
		else
		{
			if (streq(s, "AND"))
			{
				pr = 4;
				op = SS_OP_and;
			}
			else if (streq(s, "OR"))
			{
				pr = 3;
				op = SS_OP_or;
			}
			else if (streq(s, "EQ"))
			{
				pr = 8;
				op = SS_OP_eq;
			}
			else if (streq(s, "NE"))
			{
				pr = 8;
				op = SS_OP_ne;
			}
			else if (streq(s, "LT"))
			{
				pr = 8;
				op = SS_OP_lt;
			}
			else if (streq(s, "LE"))
			{
				pr = 8;
				op = SS_OP_le;
			}
			else if (streq(s, "GE"))
			{
				pr = 8;
				op = SS_OP_ge;
			}
			else if (streq(s, "GT"))
			{
				pr = 8;
				op = SS_OP_gt;
			}
			else
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: operator expected", s);
				return 0;
			}
			if (precedence >= pr)
			{
				xel(ss, s);
				break;
			}
			lp = xp;
			if (ssopexpr(op))
			{
				if (!(rp = compile(ss, pr)))
					return 0;
				if (ssopdata(lp->op) || ssopdata(rp->op))
				{
					if (ss->disc->errorf)
						(*ss->disc->errorf)(NiL, ss->disc, 2, "expression operand expected");
					return 0;
				}
			}
			else if (!ssopdata(lp->op))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "data operand expected");
				return 0;
			}
			else if (!(rp = operand(ss, lp)))
				return 0;
			else if (!ssopdata(rp->op))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 2, "data operand expected");
				return 0;
			}
			if (!(xp = vmnewof(ss->vm, 0, Ssexpr_t, 1, 0)))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
				return 0;
			}
			xp->op = op;
			xp->left.expr = lp;
			xp->right.expr = rp;
		}
	}
	return xp;
}

/*
 * finalize constant values
 * set default operand types
 * migrate easy operands left
 */

static int
finalize(Ss_t* ss, register Ssexpr_t* xp, int type)
{
	register Ssexpr_t*	tp;
	register Ssfield_t*	dp;
	register char*		s;
	register char*		v;
	register long		n;

	switch (xp->op)
	{
	case SS_OP_false:
	case SS_OP_true:
		return 0;
	case SS_OP_and:
	case SS_OP_or:
		if (ssopexpr(xp->left.expr->op) && !ssopexpr(xp->right.expr->op))
		{
			tp = xp->left.expr;
			xp->left.expr = xp->right.expr;
			xp->right.expr = tp;
		}
		if (finalize(ss, xp->right.expr, type))
			return -1;
		/*FALLTHROUGH*/
	case SS_OP_not:
		return finalize(ss, xp->left.expr, type);
	}
	if (!xp->left.expr->left.field->type)
		xp->left.expr->left.field->type = type;
	dp = xp->right.expr->left.field;
	if (!dp->type)
		dp->type = xp->left.expr->left.field->type;
	if (xp->right.expr->op == SS_OP_value && !dp->value)
	{
		if (!(v = vmnewof(ss->vm, 0, char, dp->size, 1)))
		{
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		dp->value = v;
		s = v + dp->size - 1;
		n = dp->offset;
		dp->offset = 0;
		switch (dp->type)
		{
		case SS_bcd:
			if (n < 0)
			{
				n = -n;
				*s = 0x0D;
			}
			else
				*s = 0x0C;
			*s |= bcd_unit[n % 10];
			n /= 10;
			while (s-- > v)
			{
				*s = bcd_pack[n % 100];
				n /= 100;
			}
			break;
		case SS_be:
			do
			{
				*s = n & 0xFF;
				n >>= 8;
			} while (s-- > v);
			break;
		case SS_le:
			do
			{
				*v = n & 0xFF;
				n >>= 8;
			} while (v++ < s);
			break;
		case SS_zd:
			if (n < 0)
			{
				n = -n;
				*s = 0xD0;
			}
			else
				*s = 0xC0;
			*s |= n % 10;
			n /= 10;
			while (s-- > v)
			{
				*s = n % 10;
				n /= 10;
			}
			break;
		case 0:
			dp->type = SS_void;
			/*FALLTHROUGH*/
		default:
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, 2, "%c: numeric constant type not supported", dp->type);
			return -1;
		}
	}
	return 0;
}

/*
 * parse a conditional expression
 */

static Ssexpr_t*
cond(Ss_t* ss)
{
	register Ssexpr_t*	xp;
	register char*		s;
	int			n;

	if (!(s = value(ss, LEX_COND)))
		return 0;
	else if (streq(s, "ALL"))
	{
		if (!(xp = vmnewof(ss->vm, 0, Ssexpr_t, 1, 0)))
		{
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		xp->op = SS_OP_true;
	}
	else if (streq(s, "NONE"))
	{
		if (!(xp = vmnewof(ss->vm, 0, Ssexpr_t, 1, 0)))
		{
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		xp->op = SS_OP_false;
	}
	else if (s != OP)
	{
		syntax(ss, s);
		return 0;
	}
	else if (!(xp = compile(ss, 0)))
		return 0;
	else if ((s = lex(ss)) != CP)
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: ) expected", s);
		return 0;
	}
	else if (s = lex(ss))
	{
		if (!streq(s, "FORMAT"))
		{
			xel(ss, s);
			n = SS_be;
		}
		else if (!(s = value(ss, 0)) || !(n = type(ss, s, 0)))
			return 0;
		if (finalize(ss, xp, n))
			return 0;
	}
	return xp;
}

/*
 * add a file group member to the circular group list
 */

static int
group(Ss_t* ss, Ssfile_t* fp, const char* id)
{
	Ssgroup_t*	gp;

	if (!(gp = vmnewof(ss->vm, 0, Ssgroup_t, 1, strlen(id) + 1)))
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	strcpy(gp->id = (char*)(gp + 1), id);
	if (fp->lastgroup)
		fp->lastgroup->next = gp;
	else
		fp->group = gp;
	fp->lastgroup = gp;
	gp->next = fp->group;
	return 0;
}

/*
 * load user exit usr function fun from library lib
 */

static int
load(Ss_t* ss, const char* usr, const char* fun, const char* lib, Ssdisc_t* disc)
{
	int			i;
	char*			e;
	char*			s;
	char*			t;
	void*			dll;
	Ssexit_f		exitf;
	Ssintercept_f		interceptf;
	char			path[PATH_MAX];

	static const char*	sys[] = { "syncsort", "sort" };

	for (i = 0; i < elementsof(sys); i++)
		if (dll = dllplugin(sys[i], lib, NiL, RS_PLUGIN_VERSION, NiL, RTLD_LAZY, path, sizeof(path)))
			break;
	if (!dll)
	{
		if ((e = getenv(lib)) && (e = strdup(e)))
		{
			s = e;
			while (*s)
			{
				while (isspace(*s))
					s++;
				t = s;
				while (*s && !isspace(*s))
					s++;
				if (*s)
					*s++ = 0;
				if (*t)
				{
					for (i = 0; i < elementsof(sys); i++)
						if (dll = dllplugin(sys[i], t, NiL, RS_PLUGIN_VERSION, NiL, RTLD_LAZY, path, sizeof(path)))
							break;
					if (dll)
						break;
				}
			}
			free(e);
		}
		if (!dll)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: cannot find library for %s callout %s", lib, usr, fun);
			return -1;
		}
	}
	if (!(exitf = (Ssexit_f)dlllook(dll, fun)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: %s: cannot find %s callout library", path, fun, usr);
		return -1;
	}
	if ((interceptf = (Ssintercept_f)dlllook(dll, "rs_intercept")) || (interceptf = (Ssintercept_f)dlllook(NiL, "rs_intercept")))
		ss->intercept = interceptf;
	switch ((int)strtol(usr + 1, NiL, 10))
	{
	case 11:
	case 21:
	case 31:
		ss->initexit = exitf;
		break;
	case 14:
	case 15:
	case 16:
		ss->readexit = exitf;
		break;
	case 25:
	case 32:
	case 35:
		ss->writeexit = exitf;
		break;
	case 17:
	case 27:
	case 37:
	case 38:
	case 39:
		ss->doneexit = exitf;
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: %s: unknown %s user exit function", path, fun, usr);
		return -1;
	}
	return 0;
}

/*
 * open a handle to the control file and parse it
 */

Ss_t*
ssopen(const char* file, Ssdisc_t* disc)
{
	register Ss_t*	ss;
	register char*	s;
	Vmalloc_t*	vm;
	Ssfile_t*	fp;
	Ssfile_t*	ep;
	Ssfield_t*	dp;
	char*		ofile;
	int		oline;
	int		n;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (!(ss = vmnewof(vm, 0, Ss_t, 1, sizeof(Ssfile_t))))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		vmclose(vm);
		return 0;
	}
	ss->vm = vm;
	ss->disc = disc;
	ss->format = REC_N_TYPE();
	codeset(ss, disc->code);
	oline = error_info.line;
	ofile = error_info.file;
	if (!(ss->buf = sfstropen()))
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	if (!file || !*file || streq(file, "-"))
	{
		file = 0;
		ss->io = sfstdin;
	}
	else if (!(ss->io = sfopen(NiL, file, "r")))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: cannot read", file);
		goto bad;
	}
	ss->file = fp = ep = (Ssfile_t*)(ss + 1);
	fp->format = REC_N_TYPE();
	ss->data = null;
	error_info.line = 0;
	error_info.file = (char*)file;
	while (s = lex(ss))
	{
		if (s == SC)
			;
		else if (streq(s, "ALTSEQ"))
		{
			while (s = lex(ss))
			{
				if (s == SC)
					break;
				else if (streq(s, "CODE"))
				{
					if (!(s = value(ss, LEX_GROUP)))
						goto bad;
					while (s = lex(ss))
					{
						if (s == SC)
							goto msg;
						else if (s == CP)
							break;
						else if (disc->errorf)
							(*disc->errorf)(NiL, disc, 2, "ALTSEQ: not supported (CODE=%s)", s);
					}
				}
				else
					goto msg;
			}
		}
		else if (streq(s, "END"))
			break;
		else if (streq(s, "INCLUDE") || streq(s, "OMIT") && (ss->omit = 1))
		{
			if (!(ss->expr = cond(ss)))
				goto bad;
		}
		else if (streq(s, "INREC"))
		{
			if (options(ss, LEX_JOIN, NiL, &ss->in, 2, &ss->insize))
				goto bad;
		}
		else if (streq(s, "MODS"))
		{
			while (s = lex(ss))
			{
				if (s == SC)
					break;
				else if (s[0] == 'E' && isdigit(s[1]) && isdigit(s[2]) && !s[3]) 
				{
					char*		usr[4];

					usr[0] = s;
					if (!(s = value(ss, LEX_GROUP)))
						goto bad;
					n = 1;
					while (s = lex(ss))
					{
						if (s == SC)
							goto msg;
						else if (s == CP)
							break;
						else if (n < elementsof(usr))
							usr[n++] = s;
					}
					if (n != 4)
						goto msg;
					if (load(ss, usr[0], usr[1], usr[3], disc))
						goto bad;
				}
				else
					goto msg;
			}
		}
		else if (streq(s, "OPTION"))
		{
			while (s = lex(ss))
			{
				if (s == SC)
					break;
				else if (streq(s, "EQUALS"))
					ss->stable = 'Y';
				else if (streq(s, "NOEQUALS"))
					ss->stable = 'N';
				else
					goto msg;
			}
		}
		else if (streq(s, "OUTFIL"))
		{
			if (!(fp = vmnewof(ss->vm, 0, Ssfile_t, 1, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			ep = ep->next = fp;
			fp->format = REC_N_TYPE();
			while (s = lex(ss))
			{
				if (s == SC)
					break;
				else if (streq(s, "FILES") || streq(s, "FNAMES"))
				{
					if (!(s = value(ss, 0)))
						goto bad;
					if (s == OP)
					{
						while ((s = lex(ss)) && s != CP)
							if (group(ss, fp, s))
								goto bad;
						if (s != CP)
							goto msg;
					}
					else if (group(ss, fp, s))
						goto bad;
				}
				else if (streq(s, "INCLUDE") || streq(s, "OMIT") && (fp->omit = 1))
				{
					if (!(fp->expr = cond(ss)))
						goto bad;
				}
				else if (streq(s, "OUTREC"))
				{
					if (!(fp->out = fields(ss, 2, LEX_JOIN, &fp->size)))
						goto bad;
				}
				else if (streq(s, "SAVE"))
					fp->save = 1;
				else
					goto msg;
			}
			if (!fp->group)
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "FILES or FNAMES expected in OUTFIL group");
				goto bad;
			}
		}
		else if (streq(s, "OUTREC"))
		{
			if (options(ss, LEX_JOIN, fp, &fp->out, 2, &fp->size))
				goto bad;
		}
		else if (streq(s, "RECORD"))
		{
			if (options(ss, 0, NiL, NiL, 0, NiL))
				goto bad;
		}
		else if (streq(s, "SORT") || streq(s, "MERGE") && (ss->merge = 1))
		{
			if (options(ss, LEX_COPY|LEX_JOIN, NiL, &ss->sort, 4, NiL))
				goto bad;
		}
		else if (streq(s, "SUM"))
		{
			while (s = lex(ss))
			{
				if (s == SC)
					break;
				else if (streq(s, "FIELDS"))
				{
					ss->sum = fields(ss, 2, LEX_NONE, NiL);
					ss->uniq = 1;
				}
				else if (streq(s, "FORMAT"))
				{
					if (!(s = value(ss, 0)) || !(n = type(ss, s, 0)))
						goto bad;
					for (dp = ss->sum; dp; dp = dp->next)
						if (!dp->type)
							dp->type = n;
				}
				else if (streq(s, "XSUM"))
				{
					if ((s = getenv(SS_DD_XSUM)) && *s)
						ss->xsum = vmstrdup(ss->vm, s);
				}
				else
					goto msg;
			}
		}
		else if (s[0] == '/' && (s[1] == '*' || s[1] == '/' && s[2] == '*'))
		{
			/*
			 * kosher comment?
			 */

			while ((s = lex(ss)) && s != SC);
		}
		else
			goto msg;
	}
	error_info.line = oline;
	error_info.file = ofile;
	if (ss->io != sfstdin)
		sfclose(ss->io);
	ss->io = 0;
	if (!ss->file->group && group(ss, ss->file, "out"))
		goto bad;
	return ss;
 msg:
	syntax(ss, s);
 bad:
	error_info.line = oline;
	error_info.file = ofile;
	ssclose(ss);
	return 0;
}

/*
 * return sort(1) key for dp
 * dp==0 returns the fixed record key
 * 0 return means no key, no error
 */

char*
sskey(Ss_t* ss, Ssfield_t* dp)
{
	char*	s;

	if (dp)
	{
		sfprintf(ss->buf, ".%u.%u", dp->offset + 1, dp->size);
		switch (dp->type)
		{
		case SS_bcd:
			sfprintf(ss->buf, "p");
			break;
		case SS_zd:
			sfprintf(ss->buf, "Z");
			break;
		case SS_AC_dec:
		case SS_AC_oct:
		case SS_AC_hex:
		case SS_CH_dec:
		case SS_CH_oct:
		case SS_CH_hex:
			sfprintf(ss->buf, "n");
			break;
		case SS_AC_flt:
		case SS_CH_flt:
			sfprintf(ss->buf, "g");
			break;
		}
		if (dp->reverse)
			sfprintf(ss->buf, "r");
	}
	else if (ss->size)
		sfprintf(ss->buf, ".%u", ss->size);
	else
		return 0;
	if (!(s = sfstruse(ss->buf)) && ss->disc->errorf)
		(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
	return s;
}

static const char*	opname[] =
{
	"FALSE",
	"TRUE",
	"FIELD",
	"VALUE",
	"<",
	"<=",
	"==",
	"!=",
	">=",
	">",
	"&&",
	"||",
	"!",
};

/*
 * list an expression
 */

static void
listexpr(Ss_t* ss, Sfio_t* io, Ssexpr_t* xp)
{
	register unsigned char*	s;
	register unsigned char*	e;

	if (xp)
		switch (xp->op)
		{
		case SS_OP_field:
			sfprintf(io, "[%u,%u,%c]", xp->left.field->offset, xp->left.field->size, xp->left.field->type);
			break;
		case SS_OP_value:
			sfprintf(io, "'");
			for (e = (s = (unsigned char*)xp->left.field->value) + xp->left.field->size; s < e; s++)
				sfprintf(io, "%02x", *s);
			sfprintf(io, "'");
			break;
		case SS_OP_not:
			sfprintf(io, "%s", opname[xp->op]);
			listexpr(ss, io, xp->left.expr);
			break;
		default:
			if (ssopexpr(xp->op))
				sfprintf(io, "(");
			listexpr(ss, io, xp->left.expr);
			sfprintf(io, "%s", opname[xp->op]);
			listexpr(ss, io, xp->right.expr);
			if (ssopexpr(xp->op))
				sfprintf(io, ")");
			break;
		}
}

/*
 * list a field list
 */

static void
listfields(Ss_t* ss, Sfio_t* io, Ssfield_t* dp, const char* label)
{
	register unsigned char*	v;
	register unsigned char*	e;

	if (dp)
	{
		sfprintf(io, "  %s\n", label);
		do
		{
			sfprintf(io, "     %4lu %4lu %c %u", dp->offset, dp->size, dp->type ? dp->type : SS_void, dp->reverse);
			if (v = (unsigned char*)dp->value)
			{
				sfprintf(io, " ");
				for (e = v + dp->size; v < e; v++)
					sfprintf(io, "%02x", *v);
			}
			sfprintf(io, "\n");
		} while (dp = dp->next);
	}
}

/*
 * list file info
 */

static void
listfile(Ss_t* ss, Sfio_t* io, Ssfile_t* fp)
{
	Ssgroup_t*	gp;

	sfprintf(io, "FILE");
	if (fp->size)
		sfprintf(io, " size=%u", fp->size);
	if (fp->format != REC_N_TYPE())
		sfprintf(io, " format=%s", fmtrec(fp->format, 1));
	if (!fp->out)
		sfprintf(io, " COPY");
	if (gp = fp->group)
		do
		{
			sfprintf(io, " %s", gp->id);
			if (gp->name)
				sfprintf(io, "=%s", gp->name);
		} while ((gp = gp->next) != fp->group);
	sfprintf(io, "\n");
	if (fp->expr)
	{
		sfprintf(io, "  %s=", fp->omit ? "OMIT" : "INCLUDE");
		listexpr(ss, io, fp->expr);
		sfprintf(io, "\n");
	}
	listfields(ss, io, fp->out, "OUT");
}

/*
 * list all control file info
 */

int
sslist(Ss_t* ss, Sfio_t* io)
{
	Ssfile_t*	fp;
	Ssfield_t*	dp;
	char*		s;

	if (ss->in)
	{
		sfprintf(io, "FILE");
		if (ss->insize)
			sfprintf(io, " size=%u", ss->insize);
		sfprintf(io, " in\n");
		listfields(ss, io, ss->in, "INREC");
	}
	fp = ss->file;
	listfile(ss, io, fp);
	if (ss->copy)
		sfprintf(io, "  COPY\n");
	if (ss->skip)
		sfprintf(io, "  SKIPREC %I*u\n", sizeof(ss->skip), ss->skip);
	if (ss->stop)
		sfprintf(io, "  STOPAFT %I*u\n", sizeof(ss->stop), ss->stop - 1);
	if (ss->size)
		sfprintf(io, "  SIZE %u\n", ss->size);
	if (ss->expr)
	{
		sfprintf(io, "  %s=", ss->omit ? "OMIT" : "INCLUDE");
		listexpr(ss, io, ss->expr);
		sfprintf(io, "\n");
	}
	if (ss->sort)
	{
		if (ss->stable == 'Y')
			sfprintf(io, "  EQUALS\n");
		sfprintf(io, "  KEY");
		if (s = sskey(ss, NiL))
			sfprintf(io, " -k%s", s);
		for (dp = ss->sort; dp; dp = dp->next)
			if (s = sskey(ss, dp))
				sfprintf(io, " -k%s", s);
		sfprintf(io, "\n");
		listfields(ss, io, ss->sort, "SORT");
	}
	if (ss->sum)
		listfields(ss, io, ss->sum, "SUM");
	if (ss->xsum)
		sfprintf(io, "  XSUM %s\n", ss->xsum);
	while (fp = fp->next)
		listfile(ss, io, fp);
	return !sferror(io);
}

/*
 * copy <buf,bsize> to <out,osize> according to fp->out
 * if return value > osize then copy not done but
 * subsequent call with osize >= return value will work
 */

ssize_t
sscopy(Ss_t* ss, Ssfile_t* fp, const char* buf, size_t bsize, char* out, size_t osize)
{
	register unsigned char*	s;
	register unsigned char*	t;
	register Ssfield_t*	dp;
	size_t			rsize;
	int			v;

	v = fp->format == SS_V_IBM ? 4 : 0;
	if (!(dp = fp->out))
	{
		if (v && bsize >= 4 && (t = (unsigned char*)buf)[2] == 0 && t[3] == 0 && ((t[0]<<8)|t[1]) == bsize)
			v = 0;
		if ((rsize = bsize + v) > osize)
			return rsize;
		memcpy(out + v, buf, bsize);
	}
	else if ((rsize = fp->size + v) > osize)
		return rsize;
	else
	{
		t = (unsigned char*)out + v;
		do
		{
			s = dp->value ? (unsigned char*)dp->value : ((unsigned char*)buf + dp->offset);
			switch (dp->size)
			{
			case 7: *t++ = *s++;
			case 6: *t++ = *s++;
			case 5: *t++ = *s++;
			case 4: *t++ = *s++;
			case 3: *t++ = *s++;
			case 2: *t++ = *s++;
			case 1: *t++ = *s++;
				break;
			default:
				memcpy(t, s, dp->size);
				t += dp->size;
				break;
			}
		} while (dp = dp->next);
	}
	if (v)
	{
		t = (unsigned char*)out;
		*t++ = (rsize>>8)&0xff;
		*t++ = rsize&0xff;
		*t++ = 0;
		*t = 0;
	}
	return rsize;
}

/*
 * write buf according to <fp->out,fp->size>
 */

ssize_t
sswrite(Ss_t* ss, Ssfile_t* fp, const char* buf, size_t size)
{
	register unsigned char*	s;
	register unsigned char*	t;
	register Ssgroup_t*	gp;
	register Ssfield_t*	dp;
	size_t			v;
	size_t			r;
	ssize_t			z;

	if ((gp = fp->group) != fp->group)
		fp->group = gp->next;
	v = fp->format == SS_V_IBM ? 4 : 0;
	if (dp = fp->out)
		size = fp->size;
	else if (fp->format == SS_V_IBM && (size < 4 || (t = (unsigned char*)buf)[2] == 0 && t[3] == 0 && ((t[0]<<8)|t[1]) == size))
		v = 0;
	r = size + v;
	if ((gp->end - gp->cur) < r)
	{
		if ((z = gp->cur - gp->beg) && sfwrite(gp->io, gp->beg, z) != z)
		{
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: write error", gp->name);
			return -1;
		}
		z = (r < SS_RESERVE) ? SS_RESERVE : roundof(z, SS_BLOCK);
		if (!(gp->beg = (unsigned char*)sfreserve(gp->io, z, SF_LOCKR)))
		{
			if ((z = sfvalue(gp->io)) < r)
				z = r;
			if (!(gp->beg = (unsigned char*)sfreserve(gp->io, z, SF_LOCKR)))
			{
				if (ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|1, "%s: write buffer error", gp->name);
				return -1;
			}
		}
		gp->end = (gp->cur = gp->beg) + z;
	}
	t = gp->cur + v;
	if (dp)
	{
		do
		{
			s = dp->value ? (unsigned char*)dp->value : ((unsigned char*)buf + dp->offset);
			switch (dp->size)
			{
			case 7: *t++ = *s++;
			case 6: *t++ = *s++;
			case 5: *t++ = *s++;
			case 4: *t++ = *s++;
			case 3: *t++ = *s++;
			case 2: *t++ = *s++;
			case 1: *t++ = *s++;
				break;
			default:
				memcpy(t, s, dp->size);
				t += dp->size;
				break;
			}
		} while (dp = dp->next);
	}
	else
	{
		memcpy(t, buf, size);
		t += size;
	}
	if (v)
	{
		s = gp->cur;
		r = t - s;
		*s++ = (r>>8)&0xff;
		*s++ = (r)&0xff;
		*s++ = 0;
		*s = 0;
	}
	gp->cur = t;
	return size;
}

/*
 * sum buf into out according to field list dp
 */

int
sssum(Ss_t* ss, register Ssfield_t* dp, const char* buf, size_t size, char* out)
{
	register unsigned char*	s;
	register unsigned char*	se;
	register unsigned char*	t;
	register unsigned char*	te;
	register intmax_t	n;
	register int		i;
	register int		j;

	do
	{
		s = (unsigned char*)buf + dp->offset;
		t = (unsigned char*)out + dp->offset;
		switch (dp->type)
		{
		case SS_bcd:
			se = s + dp->size - 1;
			te = t + dp->size - 1;
			if (bcd_negative[*se] == bcd_negative[*te])
			{
				if ((j = bcd_unpack[*te] + bcd_unpack[*se]) >= 10)
				{
					j -= 10;
					i = 1;
				}
				else
					i = 0;
				*te = bcd_unit[j] | (*se & 0x0F);
				while (te-- > t)
				{
					se--;
					if ((j = bcd_unpack[*te] + bcd_unpack[*se] + i) >= 100)
					{
						j -= 100;
						i = 1;
					}
					else
						i = 0;
					*te = bcd_pack[j];
				}
				if (i && ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 1, "sum overflow");
			}
			else
			{
				if ((j = (int)bcd_unpack[*te] - (int)bcd_unpack[*se]) < 0)
				{
					j += 10;
					i = 1;
				}
				else
					i = 0;
				*te = bcd_unit[j] | (*te & 0x0F);
				while (se-- > s)
				{
					te--;
					if ((j = (int)bcd_unpack[*te] - (int)bcd_unpack[*se] - i) < 0)
					{
						j += 100;
						i = 1;
					}
					else
						i = 0;
					*te = bcd_pack[j];
				}
				if (i)
				{
					te = t + dp->size - 1;
					if ((j = (int)0 - (int)bcd_unpack[*te]) < 0)
					{
						j += 10;
						i = 1;
					}
					else
						i = 0;
					*te = bcd_unit[j] | ((*te & 0x0F) == 0x0C ? 0x0D : 0x0C);
					while (te-- > t)
					{
						if ((j = (int)0 - (int)bcd_unpack[*te] - i) < 0)
						{
							j += 100;
							i = 1;
						}
						else
							i = 0;
						*te = bcd_pack[j];
					}
				}
			}
			break;
		case SS_be:
			n = 0;
			for (i = 0; i < dp->size; i++)
				n = (n << 8) + s[i] + t[i];
			for (i = dp->size - 1; i >= 0; i--)
			{
				t[i] = (n & 0xFF);
				n >>= 8;
			}
			break;
		case SS_le:
			n = 0;
			for (i = dp->size - 1; i >= 0; i--)
				n = (n << 8) + s[i] + t[i];
			for (i = 0; i < dp->size; i++)
			{
				t[i] = (n & 0xFF);
				n >>= 8;
			}
			break;
		case SS_zd:
			se = s + dp->size - 1;
			te = t + dp->size - 1;
			if ((*se ^ *te) & 0x10)
			{
				if ((j = (int)(*te & 0x0F) - (int)(*se & 0x0F)) < 0)
				{
					j += 10;
					i = 1;
				}
				else
					i = 0;
				*te = (*te & 0xF0) | j;
				while (se-- > s)
				{
					te--;
					if ((j = (int)(*te & 0x0F) - (int)(*se & 0x0F) - i) < 0)
					{
						j += 10;
						i = 1;
					}
					else
						i = 0;
					*te = 0xF0 | j;
				}
				if (i)
				{
					te = t + dp->size - 1;
					if ((j = (int)0 - (int)(*te & 0x0F)) < 0)
					{
						j += 10;
						i = 1;
					}
					else
						i = 0;
					*te = ((*te & 0x10) ? 0xC0 : 0xD0) | j;
					while (te-- > t)
					{
						if ((j = (int)0 - (int)(*te & 0x0F) - i) < 0)
						{
							j += 10;
							i = 1;
						}
						else
							i = 0;
						*te = 0xF0 | j;
					}
				}
			}
			else
			{
				if ((j = (*te & 0x0F) + (*se & 0x0F)) >= 10)
				{
					j -= 10;
					i = 1;
				}
				else
					i = 0;
				*te = (*se & 0xF0) | j;
				while (te-- > t)
				{
					se--;
					if ((j = (*te & 0x0F) + (*se & 0x0F) + i) >= 10)
					{
						j -= 10;
						i = 1;
					}
					else
						i = 0;
					*te = 0xF0 | j;
				}
				if (i && ss->disc->errorf)
					(*ss->disc->errorf)(NiL, ss->disc, 1, "sum overflow");
			}
			break;
		case SS_AC_dec:
			n = 0;
			for (i = 0; i < dp->size; i++)
				n = n * 10 + ss->acin[s[i]] + ss->acin[t[i]];
			while (i-- > 0)
			{
				t[i] = ss->acex[n % 10];
				n /= 10;
			}
			break;
		case SS_AC_oct:
			n = 0;
			for (i = 0; i < dp->size; i++)
				n = (n << 3) + ss->acin[s[i]] + ss->acin[t[i]];
			while (i-- > 0)
			{
				t[i] = ss->acex[n & 07];
				n >>= 3;
			}
			break;
		case SS_AC_hex:
			n = 0;
			for (i = 0; i < dp->size; i++)
				n = (n << 4) + ss->acin[s[i]] + ss->acin[t[i]];
			while (i-- > 0)
			{
				t[i] = ss->acex[n & 0x0F];
				n >>= 4;
			}
			break;
		case SS_CH_dec:
			n = 0;
			for (i = 0; i < dp->size; i++)
				n = n * 10 + ss->chin[s[i]] + ss->chin[t[i]];
			while (i-- > 0)
			{
				t[i] = ss->chex[n % 10];
				n /= 10;
			}
			break;
		case SS_CH_oct:
			n = 0;
			for (i = 0; i < dp->size; i++)
				n = (n << 3) + ss->chin[s[i]] + ss->chin[t[i]];
			while (i-- > 0)
			{
				t[i] = ss->chex[n & 07];
				n >>= 3;
			}
			break;
		case SS_CH_hex:
			n = 0;
			for (i = 0; i < dp->size; i++)
				n = (n << 4) + ss->chin[s[i]] + ss->chin[t[i]];
			while (i-- > 0)
			{
				t[i] = ss->chex[n & 0x0F];
				n >>= 4;
			}
			break;
		default:
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, 2, "%c: sum type not supported", dp->type);
			return -1;
		}
	} while (dp = dp->next);
	if (ss->xio && sfwrite(ss->xio, buf, size) != size)
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: XSUM write error", ss->xsum);
		return -1;
	}
	return 0;
}

/*
 * evaluate expression xp on buf
 */

int
sseval(Ss_t* ss, register Ssexpr_t* xp, const char* buf, size_t size)
{
	register unsigned char*	a;
	register unsigned char*	b;
	register unsigned char*	e;
	register int		r;

	switch (xp->op)
	{
	case SS_OP_false:
		return 0;
	case SS_OP_true:
		return 1;
	case SS_OP_not:
		return !sseval(ss, xp->left.expr, buf, size);
	case SS_OP_and:
		return sseval(ss, xp->left.expr, buf, size) && sseval(ss, xp->right.expr, buf, size);
	case SS_OP_or:
		return sseval(ss, xp->left.expr, buf, size) || sseval(ss, xp->right.expr, buf, size);
	}
	a = xp->left.expr->left.field->value ? (unsigned char*)xp->left.expr->left.field->value : ((unsigned char*)buf + xp->left.expr->left.field->offset);
	e = a + xp->left.expr->left.field->size;
	b = xp->right.expr->left.field->value ? (unsigned char*)xp->right.expr->left.field->value : ((unsigned char*)buf + xp->right.expr->left.field->offset);
	while (a < e && !(r = (int)*a++ - (int)*b++));
	switch (xp->op)
	{
	case SS_OP_lt:
		return r < 0;
	case SS_OP_le:
		return r <= 0;
	case SS_OP_eq:
		return r == 0;
	case SS_OP_ne:
		return r != 0;
	case SS_OP_ge:
		return r >= 0;
	case SS_OP_gt:
		return r > 0;
	}
	return 0;
}

int
ssdd(const char* id, const char* name, Ssdisc_t* disc)
{
	Dd_t*	dd;

	if (!(dd = newof(0, Dd_t, 1, strlen(id) + strlen(name) + 2)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	dd->id = (char*)(dd + 1);
	dd->name = stpcpy(dd->id, id) + 1;
	strcpy(dd->name, name);
	dd->alt = dd->id + 3;
	if (*dd->alt == '-' || *dd->alt == '_')
		dd->alt++;
	dd->next = state.dd;
	state.dd = dd;
	return 0;
}

#define SAME(a,b)	((a)->st_dev==(b)->st_dev&&(a)->st_ino==(b)->st_ino)

/*
 * initialize io to active files
 */

int
ssio(Ss_t* ss, int list)
{
	register Ssfile_t*	fp;
	register Ssgroup_t*	ep;
	register Ssgroup_t*	gp;
	register Ssgroup_t*	pg;
	register Dd_t*		dd;
	register Dd_t*		pd;
	register char*		s;
	register int		n;
	char*			e;
	struct stat		ns;
	struct stat		ws;

	if (stat("/dev/null", &ns))
	{
		ns.st_dev = 0;
		ns.st_ino = 0;
	}
	fp = ss->file;
	if (!fp->group->io)
		fp->group->io = sfstdout;
	if (!fstat(sffileno(fp->group->io), &ws) && SAME(&ws, &ns))
		fp->group->io = 0;
	for (n = 0; fp; fp = fp->next)
		if (gp = ep = fp->group)
		{
			pg = fp->group = 0;
			do
			{
				if (!gp->io)
				{
					for (s = 0, pd = 0, dd = state.dd; dd; pd = dd, dd = dd->next)
						if (streq(gp->id, dd->id) || streq(gp->id, dd->alt))
						{
							if (gp->name)
							{
								if (ss->disc->errorf)
									(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: auxiliary file %s already named %s", dd->name, gp->id, gp->name);
								return -1;
							}
							s = dd->name;
							break;
						}
					if (!gp->name && (s || (s = getenv(sfprints("%s%s", SS_DD_OUT, gp->id)))))
					{
						if (ss->mark && !strmatch(s, SS_MARKED) && !strmatch(s, "/dev/*"))
						{
							if (fp->size || !fp->out && ss->size)
							{
								if ((e = strrchr(s, '.')) && strmatch(e, SS_SUFFIX))
									s = sfprints("%-.*s%%%d%s", e - s, s, fp->size ? fp->size : ss->size, e);
								else
									s = sfprints("%s%%%d%s", s, fp->size ? fp->size : ss->size, ss->suffix);
							}
							else if (ss->format != REC_N_TYPE())
							{
								if ((e = strrchr(s, '.')) && strmatch(e, SS_SUFFIX))
									s = sfprints("%-.*s%%%s%s", e - s, s, fmtrec(ss->format, 1), e);
								else
									s = sfprints("%s%%%s%s", s, fmtrec(ss->format, 1), ss->suffix);
							}
						}
						if (!(gp->name = vmstrdup(ss->vm, s)))
						{
							if (ss->disc->errorf)
								(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
							return -1;
						}
						else if (!list && !(gp->io = sfopen(NiL, gp->name, "w")))
						{
							if (ss->disc->errorf)
								(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: cannot write auxiliary file %s", gp->name, gp->id);
							return -1;
						}
					}
					if (dd)
					{
						if (pd)
							pd->next = dd->next;
						else
							state.dd = dd->next;
						free(dd);
					}
				}
				if (gp->io && !fstat(sffileno(gp->io), &ws) && SAME(&ws, &ns))
				{
					sfclose(gp->io);
					gp->io = 0;
				}
				if (gp->io || list)
				{
					if (gp->io != sfstdout)
						n++;
					if (pg)
						pg->next = gp;
					else
						fp->group = gp;
					pg = gp;
				}
			} while ((gp = gp->next) != ep);
			if (pg)
				pg->next = fp->group;
		}
	if (dd = state.dd)
	{
		n = -1;
		do
		{
			if (ss->disc->errorf)
				(*ss->disc->errorf)(NiL, ss->disc, 2, "%s: unknown auxiliary file", dd->id);
			pd = dd;
			dd = dd->next;
			free(pd);
		} while (dd);
	}
	if (!list && ss->xsum && !(ss->xio = sfopen(NiL, ss->xsum, "w")))
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: cannot write XSUM file", ss->xsum);
		n = -1;
	}
	return n;
}

/*
 * announce ss io to recsort
 */

int
ssannounce(Ss_t* ss, Rs_t* rs)
{
	register Ssfile_t*	fp;
	register Ssgroup_t*	ep;
	register Ssgroup_t*	gp;
	register Ssgroup_t*	pg;

	for (fp = ss->file; fp; fp = fp->next)
		if (gp = ep = fp->group)
		{
			pg = fp->group = 0;
			do
			{
				if (gp->io)
				{
					if (rsfilewrite(rs, gp->io, gp->name))
						return -1;
					if (pg)
						pg->next = gp;
					else
						fp->group = gp;
					pg = gp;
				}
			} while ((gp = gp->next) != ep);
			if (pg)
				pg->next = fp->group;
		}
	if (ss->xsum && ss->xio && rsfilewrite(rs, ss->xio, ss->xsum))
		return -1;
	return 0;
}

/*
 * close handle ss
 */

int
ssclose(Ss_t* ss)
{
	register Ssfile_t*	fp;
	register Ssgroup_t*	gp;
	ssize_t			z;
	int			r;

	if (!ss)
		return -1;
	r = 0;
	for (fp = ss->file; fp; fp = fp->next)
		if (gp = fp->group)
			do
			{
				if (gp->io && ((z = gp->cur - gp->beg) >= 0 && sfwrite(gp->io, gp->beg, z) != z || ((fp == ss->file || gp->io == sfstdout) ? sfsync(gp->io) : sfclose(gp->io))))
				{
					if (ss->disc->errorf)
						(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: write error", gp->name);
					r = -1;
				}
			} while ((gp = gp->next) != fp->group);
	if (ss->xio && sfclose(ss->xio))
	{
		if (ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "%s: XSUM write error", ss->xsum);
		return -1;
	}
	if (ss->io && ss->io != sfstdin)
		sfclose(ss->io);
	if (ss->buf)
		sfstrclose(ss->buf);
	vmclose(ss->vm);
	return r;
}
