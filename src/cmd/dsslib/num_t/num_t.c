/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
 * dss numeric type library
 * bcd and ibm hacked from Griff Smith's ttu library
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <dsslib.h>
#include <hashpart.h>
#include <swap.h>

#define FIXED_EXTERNAL(f,format,value)	{ \
		int	i; \
		f = (value)->number; \
		if (i = (format)->fixedpoint) \
			for (;;) \
			{ \
				if (i < elementsof(pow_10)) \
				{ \
					f *= pow_10[i]; \
					break; \
				} \
				f *= pow_10[elementsof(pow_10) - 1]; \
				i -= elementsof(pow_10); \
			} \
	}

#define FIXED_INTERNAL(f,w,value,format)	{ \
		int	i; \
		f = (intmax_t)w; /* signed cast is msvc workaround */ \
		if (i = (format)->fixedpoint) \
			for (;;) \
			{ \
				if (i < elementsof(pow_10)) \
				{ \
					f /= pow_10[i]; \
					break; \
				} \
				f /= pow_10[elementsof(pow_10) - 1]; \
				i -= elementsof(pow_10); \
			} \
		(value)->number = f; \
	}

static const Cxnumber_t		pow_10[] =
{
	1E0,
	1E1,
	1E2,
	1E3,
	1E4,
	1E5,
	1E6,
	1E7,
	1E8,
	1E9,
	1E10,
	1E11,
	1E12,
	1E13,
	1E14,
	1E15,
	1E16,
	1E17,
	1E18,
	1E19,
	1E20,
	1E21,
	1E22,
	1E23,
	1E24,
	1E25,
	1E26,
	1E27,
	1E28,
	1E29,
	1E30,
	1E31,
};

/*
 * bcd_pack[x] is the bcd byte for 0<=x<=99
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

static ssize_t
bcd_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned char*	e;
	register Cxunsigned_t	v;
	Cxnumber_t		f;

	if (format->width > size)
		return format->width;
	if ((e = s + format->width - 1) < s)
		return -1;
	FIXED_EXTERNAL(f, format, value);
	if (f < 0)
	{
		v = (Cxinteger_t)(-f);
		*e = 0xD;
	}
	else
	{
		v = (Cxinteger_t)f;
		*e = 0xC;
	}
	*e |= bcd_pack[(v % 10) * 10];
	v /= 10;
	while (e-- > s)
	{
		*e = bcd_pack[v % 100];
		v /= 100;
	}
	return format->width;
}

/*
 * bcd_unpack[x] is the decimal value for bcd byte x
 * invalid codes convert to 0
 */

static const unsigned char	bcd_unpack[] =
{
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    10,11,12,13,14,15,16,17,18,19, 0, 0, 0, 0, 0, 0,
    20,21,22,23,24,25,26,27,28,29, 0, 0, 0, 0, 0, 0,
    30,31,32,33,34,35,36,37,38,39, 0, 0, 0, 0, 0, 0,
    40,41,42,43,44,45,46,47,48,49, 0, 0, 0, 0, 0, 0,
    50,51,52,53,54,55,56,57,58,59, 0, 0, 0, 0, 0, 0,
    60,61,62,63,64,65,66,67,68,69, 0, 0, 0, 0, 0, 0,
    70,71,72,73,74,75,76,77,78,79, 0, 0, 0, 0, 0, 0,
    80,81,82,83,84,85,86,87,88,89, 0, 0, 0, 0, 0, 0,
    90,91,92,93,94,95,96,97,98,99, 0, 0, 0, 0, 0, 0,
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

static ssize_t
bcd_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	register unsigned char*		s = (unsigned char*)buf;
	register unsigned char*		e;
	register Cxinteger_t		w;
	register unsigned const char*	p;
	Cxnumber_t			f;

	if (format->width > size)
		return format->width;
	w = 0;
	p = bcd_unpack;
	e = s + format->width - 1;
	while (s < e && !*s)
		s++;
	switch (e - s)
	{
	case 21: w *= 100; w += p[*s++];
	case 20: w *= 100; w += p[*s++];
	case 19: w *= 100; w += p[*s++];
	case 18: w *= 100; w += p[*s++];
	case 17: w *= 100; w += p[*s++];
	case 16: w *= 100; w += p[*s++];
	case 15: w *= 100; w += p[*s++];
	case 14: w *= 100; w += p[*s++];
	case 13: w *= 100; w += p[*s++];
	case 12: w *= 100; w += p[*s++];
	case 11: w *= 100; w += p[*s++];
	case 10: w *= 100; w += p[*s++];
	case  9: w *= 100; w += p[*s++];
	case  8: w *= 100; w += p[*s++];
	case  7: w *= 100; w += p[*s++];
	case  6: w *= 100; w += p[*s++];
	case  5: w *= 100; w += p[*s++];
	case  4: w *= 100; w += p[*s++];
	case  3: w *= 100; w += p[*s++];
	case  2: w *= 100; w += p[*s++];
	case  1: w *= 100; w += p[*s++];
	case  0: w *=  10; w += p[*s >> 4];
	case -1:if (bcd_negative[*s]) w = -w;
	}
	FIXED_INTERNAL(f, w, &ret->value, format);
	return format->width;
}

static ssize_t
be_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned char*	e;
	register unsigned char*	u;
	register Cxunsigned_t	v;
	_ast_flt4_t		f4;
	_ast_flt8_t		f8;
	Cxnumber_t		f;

	if (format->width > size)
		return format->width;
	e = s + format->width;
	FIXED_EXTERNAL(f, format, value);
	if (format->flags & CX_FLOAT)
	{
		switch (format->width)
		{
		case 4:
			f4 = f;
			u = (unsigned char*)&f4;
			break;
		case 8:
			f8 = f;
			u = (unsigned char*)&f8;
			break;
		}
#if _ast_intswap
		swapmem(_ast_intswap, u, s, format->width);
#else
		while (s < e)
			*s++ = *u++;
#endif
	}
	else
	{
		v = (Cxinteger_t)f;
		while (e > s)
		{
			*--e = v & 0xff;
			v >>= 8;
		}
	}
	return format->width;
}

static ssize_t
be_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned char*	e;
	register unsigned char*	u;
	register Cxunsigned_t	v;
	_ast_flt4_t		f4;
	_ast_flt8_t		f8;
	Cxnumber_t		f;

	if (format->width > size)
		return format->width;
	e = s + format->width;
	if (format->flags & CX_FLOAT)
	{
		switch (format->width)
		{
		case 4:
			u = (unsigned char*)&f4;
#if _ast_intswap
			swapmem(_ast_intswap, u, s, format->width);
#else
			while (s < e)
				*u++ = *s++;
#endif
			ret->value.number = f4;
			break;
		case 8:
			u = (unsigned char*)&f8;
#if _ast_intswap
			swapmem(_ast_intswap, u, s, format->width);
#else
			while (s < e)
				*u++ = *s++;
#endif
			ret->value.number = f8;
			break;
		}
	}
	else
	{
		v = 0;
		while (s < e)
		{
			v <<= 8;
			v |= *s++;
		}
		FIXED_INTERNAL(f, v, &ret->value, format);
	}
	return format->width;
}

/*
 * hash/rand -- hashed/random field -- handy for obfuscating sensitive data
 */

typedef struct
{
	Cxunsigned_t	hash;
	int		rand;
	int		seed;
} Hash_t;

static const char	lower_hash[] = "abcdefghijklmnopqrstuvwxyz";
static const char	upper_hash[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char	digit_hash[] = "0123456789";

extern time_t		time(time_t*);

static void*
hash_init(void* data, Cxdisc_t* disc)
{
	register Cxtype_t*	type = (Cxtype_t*)data;
	register const char*	s;
	register int		n;
	register Cxunsigned_t	h;
	register Hash_t*	hp;

	if (!(hp = newof(0, Hash_t, 1, 0)))
		return 0;
	h = 0;
	for (s = type->name; n = *s++;)
		HASHPART(h, n);
	h ^= (Cxunsigned_t)time(NiL) ^ (((Cxunsigned_t)getpid()) << (h & 077));
	for (n = ((h >> 7) & 077) | 0100; n > 0; n--)
		HASHPART(h, n);
	hp->hash = h;
	hp->rand = type->name[0] == 'r';
	hp->seed = 0;
	return hp;
}

static ssize_t
hash_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	Hash_t*			hp = (Hash_t*)type->data;
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register int		c;
	register Cxunsigned_t	h;

	if (!hp->seed)
	{
		hp->seed = 1;
		if (CXDETAILS(details, format, type, 0))
			hp->hash = strtoul(details, NiL, 0);
	}
	if (value->string.size > size)
		return value->string.size;
	s = (unsigned char*)value->string.data;
	e = s + value->string.size;
	t = (unsigned char*)buf;
	h = hp->hash;
	while (s < e)
	{
		c = *s++;
		HASHPART(h, c);
		if (islower(c))
			c = lower_hash[h % (sizeof(lower_hash) - 1)];
		else if (isupper(c))
			c = upper_hash[h % (sizeof(upper_hash) - 1)];
		else if (c != '+' && c != '-' && c != '_' && c != '.' && c != ' ')
			c = digit_hash[h % (sizeof(digit_hash) - 1)];
		*t++ = c;
	}
	if (hp->rand)
		hp->hash = h;
	return value->string.size;
}

static ssize_t
hash_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	Hash_t*			hp = (Hash_t*)type->data;
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register int		c;
	register Cxunsigned_t	h;

	if (format->width > size)
		return format->width;
	if (!hp->seed)
	{
		hp->seed = 1;
		if (CXDETAILS(details, format, type, 0))
			hp->hash = strtoul(details, NiL, 0);
	}
	if (!(t = vmnewof(vm, 0, unsigned char, format->width, 1)))
		return -1;
	ret->value.string.data = (char*)t;
	ret->value.string.size = format->width;
	s = (unsigned char*)buf;
	e = s + size;
	h = hp->hash;
	while (s < e)
	{
		c = *s++;
		HASHPART(h, c);
		if (islower(c))
			c = lower_hash[h % (sizeof(lower_hash) - 1)];
		else if (isupper(c))
			c = upper_hash[h % (sizeof(upper_hash) - 1)];
		else if (c != '+' && c != '-' && c != '_' && c != '.' && c != ' ')
			c = digit_hash[h % (sizeof(digit_hash) - 1)];
		*t++ = c;
	}
	if (hp->rand)
		hp->hash = h;
	return format->width;
}

/*
 * base 100 integers
 */

static const unsigned char heka_unpack[UCHAR_MAX+1] =
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
  50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
  70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
  90,  0, 91, 92,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0, 
   0,  0,  0, 93, 94, 95, 96, 97, 98, 99
};

static ssize_t
heka_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned char*	e = s + format->width;
	register Cxunsigned_t	u;
	Cxnumber_t		f;
	int			neg;

	if (format->width > size)
		return format->width;
	e = s + format->width;
	if (*s == '-')
	{
		s++;
		neg = 1;
	}
	else
	{
		if (*s == '+')
			s++;
		neg = 0;
	}
	u = 0;
	while (s < e)
		u = u * 100 + heka_unpack[*s++];
	FIXED_INTERNAL(f, u, &ret->value, format);
	if (neg)
		ret->value.number = -ret->value.number;
	return 0;
}

static const unsigned char heka_pack[100] =
{
      33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
      43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
      53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
      63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
      73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
      83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
      93, 94, 95, 96, 97, 98, 99,100,101,102,
     103,104,105,106,107,108,109,110,111,112,
     113,114,115,116,117,118,119,120,121,122,
     123,125,126,241,242,243,244,245,246,247,
};   

static ssize_t
heka_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	register unsigned char*	s;
	register unsigned char*	t;
	register unsigned char*	e;
	register ssize_t	c;
	register Cxunsigned_t	u;
	Cxnumber_t		f;
	int			neg;
	unsigned char		tmp[128];

	FIXED_EXTERNAL(f, format, value);
	if (f >= 0 || (format->flags & CX_UNSIGNED))
	{
		u = (Cxinteger_t)f;
		neg = 0;
	}
	else
	{
		u = (Cxinteger_t)(-f);
		neg = 1;
	}
	s = t = &tmp[elementsof(tmp) - 1];
	if (u == 0)
		*--t = heka_pack[0];
	else
		while (u > 0)
		{
			*--t = heka_pack[u % 100];
			u /= 100;
		}
	if ((c = format->width) > 0 && c < (elementsof(tmp) - 1))
	{
		e = s - c + 1;
		c = heka_pack[0];
		while (t > e)
			*--t = c;
	}
	if (!(format->flags & CX_UNSIGNED))
		*--t = neg ? '-' : '+';
	if ((c = s - t) > size)
		return c;
	s = (unsigned char*)buf;
	switch (c)
	{
	default:memcpy(s, t, c); break;
	case 7:	*s++ = *t++;
	case 6:	*s++ = *t++;
	case 5:	*s++ = *t++;
	case 4:	*s++ = *t++;
	case 3:	*s++ = *t++;
	case 2:	*s++ = *t++;
	case 1:	*s++ = *t++;
	}
	return c;
}

/*
 * positive exponent values for IBM 370 floating point
 */

#if DBL_MAX_10_EXP > 37

#define IBM_FP_HI		7.23700557733226210e+75
#define IBM_FP_LO		5.39760534693402789e-79

static const double	ibm_exp[] =
{
    8.63616855509444463e-78, 1.38178696881511114e-76,
    2.21085915010417782e-75, 3.53737464016668452e-74,
    5.65979942426669523e-73, 9.05567907882671237e-72,
    1.44890865261227398e-70, 2.31825384417963837e-69,
    3.70920615068742139e-68, 5.93472984109987422e-67,
    9.49556774575979875e-66, 1.51929083932156780e-64,
    2.43086534291450848e-63, 3.88938454866321357e-62,
    6.22301527786114171e-61, 9.95682444457782673e-60,
    1.59309191113245228e-58, 2.54894705781192364e-57,
    4.07831529249907783e-56, 6.52530446799852453e-55,
    1.04404871487976392e-53, 1.67047794380762228e-52,
    2.67276471009219565e-51, 4.27642353614751303e-50,
    6.84227765783602085e-49, 1.09476442525376334e-47,
    1.75162308040602134e-46, 2.80259692864963414e-45,
    4.48415508583941463e-44, 7.17464813734306340e-43,
    1.14794370197489014e-41, 1.83670992315982423e-40,
    2.93873587705571877e-39, 4.70197740328915003e-38,
    7.52316384526264005e-37, 1.20370621524202241e-35,
    1.92592994438723585e-34, 3.08148791101957736e-33,
    4.93038065763132378e-32, 7.88860905221011805e-31,
    1.26217744835361890e-29, 2.01948391736579022e-28,
    3.23117426778526435e-27, 5.16987882845642297e-26,
    8.27180612553027675e-25, 1.32348898008484430e-23,
    2.11758236813575085e-22, 3.38813178901720136e-21,
    5.42101086242752217e-20, 8.67361737988403547e-19,
    1.38777878078144568e-17, 2.22044604925031308e-16,
    3.55271367880050093e-15, 5.68434188608080149e-14,
    9.09494701772928238e-13, 1.45519152283668518e-11,
    2.32830643653869629e-10, 3.72529029846191406e-09,
    5.96046447753906250e-08, 9.53674316406250000e-07,
    1.52587890625000000e-05, 2.44140625000000000e-04,
    3.90625000000000000e-03, 6.25000000000000000e-02,
    1.00000000000000000e+00, 1.60000000000000000e+01,
    2.56000000000000000e+02, 4.09600000000000000e+03,
    6.55360000000000000e+04, 1.04857600000000000e+06,
    1.67772160000000000e+07, 2.68435456000000000e+08,
    4.29496729600000000e+09, 6.87194767360000000e+10,
    1.09951162777600000e+12, 1.75921860444160000e+13,
    2.81474976710656000e+14, 4.50359962737049600e+15,
    7.20575940379279360e+16, 1.15292150460684700e+18,
    1.84467440737095516e+19, 2.95147905179352826e+20,
    4.72236648286964521e+21, 7.55578637259143234e+22,
    1.20892581961462920e+24, 1.93428131138340668e+25,
    3.09485009821345069e+26, 4.95176015714152110e+27,
    7.92281625142643376e+28, 1.26765060022822940e+30,
    2.02824096036516704e+31, 3.24518553658426727e+32,
    5.19229685853482763e+33, 8.30767497365572421e+34,
    1.32922799578491587e+36, 2.12676479325586540e+37,
    3.40282366920938463e+38, 5.44451787073501542e+39,
    8.71122859317602466e+40, 1.39379657490816395e+42,
    2.23007451985306231e+43, 3.56811923176489970e+44,
    5.70899077082383952e+45, 9.13438523331814324e+46,
    1.46150163733090292e+48, 2.33840261972944467e+49,
    3.74144419156711147e+50, 5.98631070650737835e+51,
    9.57809713041180536e+52, 1.53249554086588886e+54,
    2.45199286538542217e+55, 3.92318858461667548e+56,
    6.27710173538668076e+57, 1.00433627766186892e+59,
    1.60693804425899028e+60, 2.57110087081438444e+61,
    4.11376139330301511e+62, 6.58201822928482417e+63,
    1.05312291668557190e+65, 1.68499666669691499e+66,
    2.69599466671506398e+67, 4.31359146674410237e+68,
    6.90174634679056379e+69, 1.10427941548649021e+71,
    1.76684706477838433e+72, 2.82695530364541493e+73,
    4.52312848583266388e+74, 7.23700557733226210e+75,
};

#else

/*
 * full ibm range not supported
 */

#define IBM_FP_HI		1.70141183460469230e+38
#define IBM_FP_LO		4.70197740328915000e-38

static const double	ibm_exp[] =
{
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    7.52316384526264005e-37, 1.20370621524202241e-35,
    1.92592994438723585e-34, 3.08148791101957736e-33,
    4.93038065763132378e-32, 7.88860905221011805e-31,
    1.26217744835361890e-29, 2.01948391736579022e-28,
    3.23117426778526435e-27, 5.16987882845642297e-26,
    8.27180612553027675e-25, 1.32348898008484430e-23,
    2.11758236813575085e-22, 3.38813178901720136e-21,
    5.42101086242752217e-20, 8.67361737988403547e-19,
    1.38777878078144568e-17, 2.22044604925031308e-16,
    3.55271367880050093e-15, 5.68434188608080149e-14,
    9.09494701772928238e-13, 1.45519152283668518e-11,
    2.32830643653869629e-10, 3.72529029846191406e-09,
    5.96046447753906250e-08, 9.53674316406250000e-07,
    1.52587890625000000e-05, 2.44140625000000000e-04,
    3.90625000000000000e-03, 6.25000000000000000e-02,
    1.00000000000000000e+00, 1.60000000000000000e+01,
    2.56000000000000000e+02, 4.09600000000000000e+03,
    6.55360000000000000e+04, 1.04857600000000000e+06,
    1.67772160000000000e+07, 2.68435456000000000e+08,
    4.29496729600000000e+09, 6.87194767360000000e+10,
    1.09951162777600000e+12, 1.75921860444160000e+13,
    2.81474976710656000e+14, 4.50359962737049600e+15,
    7.20575940379279360e+16, 1.15292150460684700e+18,
    1.84467440737095516e+19, 2.95147905179352826e+20,
    4.72236648286964521e+21, 7.55578637259143234e+22,
    1.20892581961462920e+24, 1.93428131138340668e+25,
    3.09485009821345069e+26, 4.95176015714152110e+27,
    7.92281625142643376e+28, 1.26765060022822940e+30,
    2.02824096036516704e+31, 3.24518553658426727e+32,
    5.19229685853482763e+33, 8.30767497365572421e+34,
    1.32922799578491587e+36, 2.12676479325586540e+37,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
};

#endif

#define FLOOR(x)	((Cxinteger_t)(x))

static ssize_t
ibm_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned int	lo;
	register unsigned int	hi;
	register unsigned int	ex;
	register double		f;
	int			negative;
	Cxinteger_t		hi3;
	Cxinteger_t		md2;
	Cxinteger_t		lo2;

	if (format->width > size)
		return format->width;
	if (negative = (f = value->number) < 0.0)
		f = -f;
	switch (format->width)
	{
	case 4:
		if (f < IBM_FP_LO)
			s[0] = s[1] = s[2] = s[3] = 0x00;
		else
		{
			if (f > IBM_FP_HI)
				f = IBM_FP_HI;

			/*
			 * find the closest exponent in ibm_exp[]
			 */

			lo = 0;
			hi = elementsof(ibm_exp) - 1;
			while (lo != hi + 1)
			{
				ex = lo + ((hi - lo) >> 1);
				if (ibm_exp[ex] < f)
					lo = ex + 1;
				else
					hi = ex - 1;
			}
			if (ibm_exp[ex] < f)
				ex++;

			/*
			 * scale by the exponent to reduce
			 * to the range 0.0625 -> 0.9999...
			 */

			f /= ibm_exp[ex];

			/*
			 * extract the fraction bits as integers
			 */

			f *= (double)0x1000000;
			hi3 = FLOOR(f + 0.5);

			/*
			 * correct for overflow
			 */

			if (hi3 > 0xFFFFFF)
			{
				hi3 = 0x100000;
				ex++;
			}
			if (ex > 0x7F)
			{
				ex = 0x7F;
				hi3 = 0xFFFFFF;
			}

			/*
			 * set the sign bit
			 */

			if (negative)
				ex |= 0x80;

			/*
			 * done
			 */

			s[0] = ex;
			s[1] = hi3 >> 16;
			s[2] = (hi3 >> 8) & 0xFF;
			s[3] = hi3 & 0xFF;
		}
		return 4;
	case 8:
		if (f < IBM_FP_LO)
			s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = s[6] = s[7] = 0x00;
		else
		{
			if (f > IBM_FP_HI)
				f = IBM_FP_HI;

			/*
			 * find the closest exponent in ibm_exp[]
			 */

			lo = 0;
			hi = elementsof(ibm_exp) - 1;
			while (lo != hi + 1)
			{
				ex = lo + ((hi - lo) >> 1);
				if (ibm_exp[ex] < f)
					lo = ex + 1;
				else
					hi = ex - 1;
			}
			if (ibm_exp[ex] < f)
				ex++;

			/*
			 * scale by the exponent to reduce
			 * to the range 0.0625 -> 0.9999...
			 */

			f /= ibm_exp[ex];

			/*
			 * extract the fraction bits as integers
			 */

			f *= (double)0x1000000;
			hi3 = FLOOR(f);
			f -= hi3;
			f *= (double)0x10000;
			md2 = FLOOR(f);
			f -= md2;
			f *= (double)0x10000;
			lo2 = FLOOR(f + 0.5);

			/*
			 * correct for overflow
			 */

			if (lo2 > 0xFFFF)
			{
				lo2 -= 0x10000;
				md2++;
			}
			if (md2 > 0xFFFF)
			{
				md2 -= 0x10000;
				hi3++;
			}
			if (hi3 > 0xFFFFFF)
			{
				hi3 = 0x100000;
				ex++;
			}
			if (ex > 0x7F)
			{
				ex = 0x7F;
				hi3 = 0xFFFFFF;
				md2 = 0xFFFF;
				lo2 = 0xFFFF;
			}

			/*
			 * set the sign bit
			 */

			if (negative)
				ex |= 0x80;

			/*
			 * done
			 */

			s[0] = ex;
			s[1] = hi3 >> 16;
			s[2] = (hi3 >> 8) & 0xFF;
			s[3] = hi3 & 0xFF;
			s[4] = md2 >> 8;
			s[5] = md2 & 0xFF;
			s[6] = lo2 >> 8;
			s[7] = lo2 & 0xFF;
		}
		return 8;
	}
	return -1;
}

static ssize_t
ibm_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register Cxinteger_t	i;
	register double		f;

	if (format->width > size)
		return format->width;
	switch (format->width)
	{
	case 4:
		i = (s[1] << 16)
                  | (s[2] <<  8)
                  |  s[3];
		f = i * (1.0 / 0x1000000) * ((s[0] < 0x80) ? ibm_exp[s[0]] : -ibm_exp[s[0] & 0x7F]);
		ret->value.number = f;
		return 4;
	case 8:
		i = (s[1] << 16)
                  | (s[2] <<  8)
                  |  s[3];
		f = i * (1.0 / 0x1000000);
		i = (s[4] <<  8)
	          |  s[5];
		f += i * ((1.0 / 0x1000000) / (double)0x10000);
		i = (s[6] <<  8)
		  |  s[7];
		f += i * (((1.0 / 0x1000000) / (double)0x10000) / (double)0x10000);
		f *= (s[0] < 0x80) ? ibm_exp[s[0]] : -ibm_exp[s[0] & 0x7F];
		ret->value.number = f;
		return 8;
	}
	return -1;
}

static ssize_t
le_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned char*	e;
	register unsigned char*	u;
	register Cxunsigned_t	v;
	_ast_flt4_t		f4;
	_ast_flt8_t		f8;
	Cxnumber_t		f;

	if (format->width > size)
		return format->width;
	e = s + format->width;
	FIXED_EXTERNAL(f, format, value);
	if (format->flags & CX_FLOAT)
	{
		switch (format->width)
		{
		case 4:
			f4 = f;
			u = (unsigned char*)&f4;
			break;
		case 8:
			f8 = f;
			u = (unsigned char*)&f8;
			break;
		}
#if _ast_intswap ^ 7
		swapmem(_ast_intswap ^ 7, u, s, format->width);
#else
		while (s < e)
			*s++ = *u++;
#endif
	}
	else
	{
		v = (Cxinteger_t)value->number;
		while (s < e)
		{
			*s++ = v & 0xff;
			v >>= 8;
		}
	}
	return format->width;
}

static ssize_t
le_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned char*	e;
	register unsigned char*	u;
	register Cxunsigned_t	v;
	_ast_flt4_t		f4;
	_ast_flt8_t		f8;
	Cxnumber_t		f;

	if (format->width > size)
		return format->width;
	e = s + format->width;
	if (format->flags & CX_FLOAT)
	{
		switch (format->width)
		{
		case 4:
			u = (unsigned char*)&f4;
#if _ast_intswap ^ 7
			swapmem(_ast_intswap ^ 7, u, s, format->width);
#else
			while (s < e)
				*u++ = *s++;
#endif
			ret->value.number = f4;
			break;
		case 8:
			u = (unsigned char*)&f8;
#if _ast_intswap ^ 7
			swapmem(_ast_intswap ^ 7, u, s, format->width);
#else
			while (s < e)
				*u++ = *s++;
#endif
			ret->value.number = f8;
			break;
		}
	}
	else
	{
		v = 0;
		while (e > s)
		{
			v <<= 8;
			v |= *--e;
		}
		FIXED_INTERNAL(f, v, &ret->value, format);
	}
	return format->width;
}

static ssize_t
sf_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	register int		r;
	register Cxinteger_t	v;
	register Cxunsigned_t	u;
	Cxnumber_t		f;

	FIXED_EXTERNAL(f, format, value);
	sfstrbuf(cx->buf, buf, size, 0);
	if (format->flags & CX_FLOAT)
		r = sfputd(cx->buf, f);
	else if (format->flags & CX_UNSIGNED)
	{
		v = f;
		u = v;
		r = sfputu(cx->buf, u);
	}
	else
	{
		v = f;
		r = sfputl(cx->buf, v);
	}
	if (r < 0)
		return size ? 2 * size : 8;
	return r;
}

static ssize_t
sf_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	if (!size)
		return 4;
	sfstrbuf(cx->buf, (void*)buf, size, 0);
	if (format->flags & CX_FLOAT)
		ret->value.number = sfgetd(cx->buf);
	else if (format->flags & CX_UNSIGNED)
		ret->value.number = (Cxinteger_t)sfgetu(cx->buf);
	else
		ret->value.number = sfgetl(cx->buf);
	if (sferror(cx->buf))
		return size * 2;
	return sfstrtell(cx->buf);
}

static Cxtype_t types[] =
{
	{ "bcd_t",	"Binary coded decimal.", CXH, (Cxtype_t*)"number", 0, bcd_external, bcd_internal, 0, 0, 0, 0, { 0, 0, CX_INTEGER } },
	{ "be_t",	"Big endian binary.", CXH, (Cxtype_t*)"number", 0, be_external, be_internal, 0, 0, 0, 0, { 0, 0, CX_BINARY } },
	{ "hash_t",	"Repeatable string hash.", CXH, (Cxtype_t*)"string", hash_init, hash_external, hash_internal, 0, 0, 0, 0, { 0, 0, CX_STRING } },
	{ "heka_t",	"Base 100 binary integer.", CXH, (Cxtype_t*)"number", 0, heka_external, heka_internal, 0, 0, 0, 0, { 0, 0, CX_INTEGER } },
	{ "ibm_t",	"IBM 4 and 8 byte floating point.", CXH, (Cxtype_t*)"number", 0, ibm_external, ibm_internal, 0, 0, 0, 0, { 0, 0, CX_FLOAT } },
	{ "le_t",	"Little endian binary.", CXH, (Cxtype_t*)"number", 0, le_external, le_internal, 0, 0, 0, 0, { 0, 0, CX_BINARY } },
	{ "rand_t",	"Non-repeatable pseudo-random string hash.", CXH, (Cxtype_t*)"string", hash_init, hash_external, hash_internal, 0, 0, 0, 0, { "The format details string is an optional initial pseudo-random seed number. The default is synthesized using current process/system characteristics.", 0, CX_STRING } },
	{ "sf_t",	"sfio sfputd()/sfputl()/sfputu() encoding.", CXH, (Cxtype_t*)"number", 0, sf_external, sf_internal },
	{0}
};

Dsslib_t dss_lib_num_t =
{
	"num_t",
	"numeric type support"
	"[-?\n@(#)$Id: dss numeric type library (AT&T Research) 2008-06-11 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	0,
	&types[0],
};
