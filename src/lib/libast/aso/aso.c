/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "asohdr.h"
#include "FEATURE/aso"

#if defined(_UWIN) && defined(_BLD_ast)

NoN(aso)

#else

/*
 * ast atomic scalar operations
 * AT&T Research
 *
 * cas { 8 16 32 [64] } subset snarfed from the work by
 * Adam Edgar and Kiem-Phong Vo 2010-10-10
 *
 * additional intrinsics and emulations by
 * Glenn Fowler 2011-11-11
 */

static const char	lib[] = "libast:aso";

typedef union
{
	uint8_t			c[2];
	uint16_t		i;
} U16_8_t;

typedef union
{
	uint8_t			c[4];
	uint32_t		i;
} U32_8_t;

typedef union
{
	uint16_t		c[2];
	uint32_t		i;
} U32_16_t;

#ifdef _ast_int8_t

typedef union
{
	uint8_t			c[8];
	uint64_t		i;
} U64_8_t;

typedef union
{
	uint16_t		c[4];
	uint64_t		i;
} U64_16_t;

typedef union
{
	uint32_t		c[2];
	uint64_t		i;
} U64_32_t;

#endif

#if !_ASO_INTRINSIC || defined(_ast_int8_t) && !defined(asocas64)

/*
 * used only when no cas intrinsic is available
 * should work most of the time in a single threaded process 
 * with no multi-process shared mem access and low freq signals
 */

static ssize_t
lock(ssize_t k)
{
	static int	locked;

	if (k >= 0)
	{
		locked--;
		return 0;
	}
	while (locked++)
		locked--;
	return 1;
}

#endif

#if _ASO_Interlocked

#if ( _BLD_posix || __CYGWIN__ ) && !_X64

#if __CYGWIN__

#include <dlfcn.h>

#define MODULE_kernel	0
#define getsymbol(m,s)	dlsym(m,s)

#else

#include "dl.h"

#endif

typedef struct LL_s
{
	LONG		a;
	LONG		b;
} LL_t;

typedef union
{
	LONGLONG	i;
	LL_t		ll;
} LL_u;

static LONGLONG _aso_InterlockedCompareExchange64_init(LONGLONG volatile*, LONGLONG, LONGLONG);

_aso_InterlockedCompareExchange64_f _aso_InterlockedCompareExchange64 = _aso_InterlockedCompareExchange64_init;

static LONGLONG _aso_InterlockedCompareExchange64_32(LONGLONG volatile* p, LONGLONG o, LONGLONG n)
{
	LL_t*		lp = (LL_t*)p;
	LL_t*		op = (LL_t*)&o;
	LL_t*		np = (LL_t*)&n;
	LONGLONG	r;

	r = *p;
	if (asocas32(&lp->a, op->a, np->a) == op->a)
	{
		if (asocas32(&lp->b, op->b, np->b) == op->b)
			return o;
		asocas32(&lp->a, np->a, op->a);
	}
	return r;
}

static LONGLONG _aso_InterlockedCompareExchange64_init(LONGLONG volatile* p, LONGLONG o, LONGLONG n)
{
	if (!(_aso_InterlockedCompareExchange64 = (_aso_InterlockedCompareExchange64_f)getsymbol(MODULE_kernel, "InterlockedCompareExchange64")))
		_aso_InterlockedCompareExchange64 = _aso_InterlockedCompareExchange64_32;
	return _aso_InterlockedCompareExchange64(p, o, n);
}

static LONGLONG _aso_InterlockedExchangeAdd64_init(LONGLONG volatile*, LONGLONG);

_aso_InterlockedExchangeAdd64_f _aso_InterlockedExchangeAdd64 = _aso_InterlockedExchangeAdd64_init;

static LONGLONG _aso_InterlockedExchangeAdd64_32(LONGLONG volatile* p, LONGLONG n)
{
	LONGLONG	o;

	do
	{
		o = *p;
	} while (_aso_InterlockedCompareExchange64_32(p, o, o + n) != o);
	return o;
}

static LONGLONG _aso_InterlockedExchangeAdd64_init(LONGLONG volatile* p, LONGLONG n)
{
	if (!(_aso_InterlockedExchangeAdd64 = (_aso_InterlockedExchangeAdd64_f)getsymbol(MODULE_kernel, "InterlockedExchangeAdd64")))
		_aso_InterlockedExchangeAdd64 = _aso_InterlockedExchangeAdd64_32;
	return _aso_InterlockedExchangeAdd64(p, n);
}

#endif

#elif _ASO_i386

uint32_t
_aso_cas32(uint32_t volatile* p, uint32_t o, uint32_t n)
{
	uint32_t	r;

	__asm__ __volatile__ (
		"lock ; cmpxchg %3,%4"
		: "=a"(r), "=m"(*p)
		: "0"(o), "q"(n), "m"(*p)
		: "memory", "cc"
		);
	return r;
}

#if _ast_sizeof_pointer == 8

uint64_t
_aso_cas64(uint64_t volatile* p, uint64_t o, uint64_t n)
{
	uint64_t	r;

	__asm__ __volatile__ (
		"lock ; cmpxchg %3,%4"
		: "=a"(r), "=m"(*p)
		: "0"(o), "q"(n), "m"(*p)
		: "memory", "cc"
		);
	return r;
}

#endif

#elif _ASO_ia64

uint32_t
_aso_cas32(uint32_t volatile* p, uint32_t o, uint32_t n)
{
	int	r;

	__asm__ __volatile__ (
		"0:	lwarx %0,0,%1 ;"
		"	xor. %0,%3,%0;"
		"	bne 1f;"
		"	stwcx. %2,0,%1;"
		"	bne- 0b;"
		"1:"
		: "=&r"(r)
		: "r"(p), "r"(n), "r"(o)
		: "cr0", "memory"
		);
	__asm__ __volatile__ ("isync" : : : "memory");
	return r ? *p : o;
}

uint64_t
_aso_cas64(uint64_t volatile* p, uint64_t o, uint64_t n)
{
	long	r;

	__asm__ __volatile__ (
		"0:	ldarx %0,0,%1 ;"
		"	xor. %0,%3,%0;"
		"	bne 1f;"
		"	stdcx. %2,0,%1;"
		"	bne- 0b;"
		"1:"
		: "=&r"(r)
		: "r"(p), "r"(n), "r"(o)
		: "cr0", "memory"
		);
	__asm__ __volatile__ ("isync" : : : "memory");
	return r ? *p : o;
}

#elif _ASO_ppc

uint32_t
_aso_cas32(uint32_t volatile* p, uint32_t o, uint32_t n)
{
	int	r;

	__asm__ __volatile__ (
		"0:	lwarx %0,0,%1 ;"
		"	xor. %0,%3,%0;"
		"	bne 1f;"
		"	stwcx. %2,0,%1;"
		"	bne- 0b;"
		"1:"
		: "=&r"(r)
		: "r"(p), "r"(n), "r"(o)
		: "cr0", "memory"
		);
	__asm__ __volatile__ ("isync" : : : "memory");
	return r ? *p : o;
}

uint64_t
_aso_cas64(uint64_t volatile* p, uint64_t o, uint64_t n)
{
	long	r;

	__asm__ __volatile__ (
		"0:	ldarx %0,0,%1 ;"
		"	xor. %0,%3,%0;"
		"	bne 1f;"
		"	stdcx. %2,0,%1;"
		"	bne- 0b;"
		"1:"
		: "=&r"(r)
		: "r"(p), "r"(n), "r"(o)
		: "cr0", "memory"
		);
	__asm__ __volatile__ ("isync" : : : "memory");
	return r ? *p : o;
}

#endif

/*
 * sync and return "current" value
 */

#ifndef asoget8
uint8_t
asoget8(uint8_t volatile* p)
{
	int	o;

	do
	{
		o = *p;
	} while (asocas8(p, o, o) != o);
	return o;
}
#endif

#ifndef asoget16
uint16_t
asoget16(uint16_t volatile* p)
{
	int	o;

	do
	{
		o = *p;
	} while (asocas16(p, o, o) != o);
	return o;
}
#endif

#ifndef asoget32
uint32_t
asoget32(uint32_t volatile* p)
{
	uint32_t	o;

	do
	{
		o = *p;
	} while (asocas32(p, o, o) != o);
	return o;
}
#endif

#ifdef _ast_int8_t

#ifndef asoget64
uint64_t
asoget64(uint64_t volatile* p)
{
	uint64_t	o;

	do
	{
		o = *p;
	} while (asocas64(p, o, o) != o);
	return o;
}
#endif

#endif

#ifndef asogetptr
void*
asogetptr(void volatile* p)
{
	void*	o;

	do
	{
		o = *(void* volatile*)p;
	} while (asocasptr((void**)p, o, o) != o);
	return o;
}
#endif

/*
 * add and return old value
 */

#ifndef asoadd8
uint8_t
asoadd8(uint8_t volatile* p, int n)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas8(p, o, o + n) != o);
	return o;
}
#endif

#ifndef asoadd16
uint16_t
asoadd16(uint16_t volatile* p, int n)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas16(p, o, o + n) != o);
	return o;
}
#endif

#ifndef asoadd32
uint32_t
asoadd32(uint32_t volatile* p, uint32_t n)
{
	uint32_t	o;

	do
	{
		o = *p;
	} while (asocas32(p, o, o + n) != o);
	return o;
}
#endif

#ifdef _ast_int8_t

#ifndef asoadd64
uint64_t
asoadd64(uint64_t volatile* p, uint64_t n)
{
	uint64_t	o;

	do
	{
		o = *p;
	} while (asocas64(p, o, o + n) != o);
	return o;
}
#endif

#endif

/*
 * subtract and return old value
 */

#ifndef asosub8
uint8_t
asosub8(uint8_t volatile* p, int n)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas8(p, o, o - n) != o);
	return o;
}
#endif

#ifndef asosub16
uint16_t
asosub16(uint16_t volatile* p, int n)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas16(p, o, o - n) != o);
	return o;
}
#endif

#ifndef asosub32
uint32_t
asosub32(uint32_t volatile* p, uint32_t n)
{
	uint32_t	o;

	do
	{
		o = *p;
	} while (asocas32(p, o, o - n) != o);
	return o;
}
#endif

#ifdef _ast_int8_t

#ifndef asosub64
uint64_t
asosub64(uint64_t volatile* p, uint64_t n)
{
	uint64_t	o;

	do
	{
		o = *p;
	} while (asocas64(p, o, o - n) != o);
	return o;
}
#endif

#endif

/*
 * increment and return old value
 */

#ifndef asoinc8
uint8_t
asoinc8(uint8_t volatile* p)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas8(p, o, o + 1) != o);
	return o;
}
#endif

#ifndef asoinc16
uint16_t
asoinc16(uint16_t volatile* p)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas16(p, o, o + 1) != o);
	return o;
}
#endif

#ifndef asoinc32
uint32_t
asoinc32(uint32_t volatile* p)
{
	uint32_t	o;

	do
	{
		o = *p;
	} while (asocas32(p, o, o + 1) != o);
	return o;
}
#endif

#ifdef _ast_int8_t

#ifndef asoinc64
uint64_t
asoinc64(uint64_t volatile* p)
{
	uint64_t	o;

	do
	{
		o = *p;
	} while (asocas64(p, o, o + 1) != o);
	return o;
}
#endif

#endif

/*
 * decrement and return old value
 */

#ifndef asodec8
uint8_t
asodec8(uint8_t volatile* p)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas8(p, o, o - 1) != o);
	return o;
}
#endif

#ifndef asodec16
uint16_t
asodec16(uint16_t volatile* p)
{
	int		o;

	do
	{
		o = *p;
	} while (asocas16(p, o, o - 1) != o);
	return o;
}
#endif

#ifndef asodec32
uint32_t
asodec32(uint32_t volatile* p)
{
	uint32_t	o;

	do
	{
		o = *p;
	} while (asocas32(p, o, o - 1) != o);
	return o;
}
#endif

#ifdef _ast_int8_t

#ifndef asodec64
uint64_t
asodec64(uint64_t volatile* p)
{
	uint64_t	o;

	do
	{
		o = *p;
	} while (asocas64(p, o, o - 1) != o);
	return o;
}
#endif

#endif

/*
 * if *p <= n then return *p
 * else *p = n and return n
 */

#ifndef asomin8
uint8_t
asomin8(uint8_t volatile* p, int n)
{
	int		o;

	for (;; asospinrest())
	{
		if ((o = *p) <= n)
			return o;
		if (asocas8(p, o, n) == o)
			break;
	}
	return n;
}
#endif

#ifndef asomin16
uint16_t
asomin16(uint16_t volatile* p, int n)
{
	int		o;

	for (;; asospinrest())
	{
		if ((o = *p) <= n)
			return o;
		if (asocas16(p, o, n) == o)
			break;
	}
	return o;
}
#endif

#ifndef asomin32
uint32_t
asomin32(uint32_t volatile* p, uint32_t n)
{
	uint32_t	o;

	for (;; asospinrest())
	{
		if ((o = *p) <= n)
			return o;
		if (asocas32(p, o, n) == o)
			break;
	}
	return o;
}
#endif

#ifdef _ast_int8_t

#ifndef asomin64
uint64_t
asomin64(uint64_t volatile* p, uint64_t n)
{
	uint64_t	o;

	for (;; asospinrest())
	{
		if ((o = *p) <= n)
			return o;
		if (asocas64(p, o, n) == o)
			break;
	}
	return o;
}
#endif

#endif

/*
 * if *p >= n then return *p
 * else *p = n and return n
 */

#ifndef asomax8
uint8_t
asomax8(uint8_t volatile* p, int n)
{
	int		o;

	for (;; asospinrest())
	{
		if ((o = *p) >= n)
			return o;
		if (asocas8(p, o, n) == o)
			break;
	}
	return n;
}
#endif

#ifndef asomax16
uint16_t
asomax16(uint16_t volatile* p, int n)
{
	int		o;

	for (;; asospinrest())
	{
		if ((o = *p) >= n)
			return o;
		if (asocas16(p, o, n) == o)
			break;
	}
	return o;
}
#endif

#ifndef asomax32
uint32_t
asomax32(uint32_t volatile* p, uint32_t n)
{
	uint32_t	o;

	for (;; asospinrest())
	{
		if ((o = *p) >= n)
			return o;
		if (asocas32(p, o, n) == o)
			break;
	}
	return o;
}
#endif

#ifdef _ast_int8_t

#ifndef asomax64
uint64_t
asomax64(uint64_t volatile* p, uint64_t n)
{
	uint64_t	o;

	for (;; asospinrest())
	{
		if ((o = *p) >= n)
			return o;
		if (asocas64(p, o, n) == o)
			break;
	}
	return o;
}
#endif

#endif

/*
 * { 8 16 32 [64] } compare with old, swap with new if same, and return old value
 */

#ifndef asocas8
uint8_t
asocas8(uint8_t volatile* p, int o, int n)
{
#if defined(asocas16)
	U16_8_t		u;
	U16_8_t		v;
	U16_8_t*	a;
	int		s;
	int		i;

	s = (int)(integralof(p) & (sizeof(u.i) - 1));
	a = (U16_8_t*)((char*)0 + (integralof(p) & ~(sizeof(u.i) - 1)));
	for (;;)
	{
		u.i = a->i;
		u.c[s] = o;
		v.i = u.i;
		v.c[s] = n;
		if (asocas16(&a->i, u.i, v.i) == u.i)
			break;
		for (i = 0;; i++)
			if (i >= elementsof(u.c))
				return a->c[s];
			else if (i != s && u.c[i] != a->c[i])
				break;
	}
	return o;
#elif defined(asocas32)
	U32_8_t		u;
	U32_8_t		v;
	U32_8_t*	a;
	int		s;
	int		i;

	s = (int)(integralof(p) & (sizeof(u.i) - 1));
	a = (U32_8_t*)((char*)0 + (integralof(p) & ~(sizeof(u.i) - 1)));
	for (;;)
	{
		u.i = a->i;
		u.c[s] = o;
		v.i = u.i;
		v.c[s] = n;
		if (asocas32(&a->i, u.i, v.i) == u.i)
			break;
		for (i = 0;; i++)
			if (i >= elementsof(u.c))
				return a->c[s];
			else if (i != s && u.c[i] != a->c[i])
				break;
	}
	return o;
#elif defined(asocas64)
	U64_8_t		u;
	U64_8_t		v;
	U64_8_t*	a;
	int		s;
	int		i;

	s = (int)(integralof(p) & (sizeof(u.i) - 1));
	a = (U64_8_t*)((char*)0 + (integralof(p) & ~(sizeof(u.i) - 1)));
	for (;;)
	{
		u.i = a->i;
		u.c[s] = o;
		v.i = u.i;
		v.c[s] = n;
		if (asocas64(&a->i, u.i, v.i) == u.i)
			break;
		for (i = 0;; i++)
			if (i >= elementsof(u.c))
				return a->c[s];
			else if (i != s && u.c[i] != a->c[i])
				break;
	}
	return o;
#else
	ssize_t		k;

	k = lock(0);
	if (*p == o)
		*p = n;
	else
		o = *p;
	lock(k);
	return o;
#endif
}
#endif

#ifndef asocas16
uint16_t
asocas16(uint16_t volatile* p, int o, int n)
{
#if defined(asocas32)
	U32_8_t		u;
	U32_8_t		v;
	U32_8_t*	a;
	int		s;
	int		i;

	s = (int)(integralof(p) & (sizeof(u.i) - 1));
	a = (U32_8_t*)((char*)0 + (integralof(p) & ~(sizeof(u.i) - 1)));
	for (;;)
	{
		u.i = a->i;
		u.c[s] = o;
		v.i = u.i;
		v.c[s] = n;
		if (asocas32(&a->i, u.i, v.i) == u.i)
			break;
		for (i = 0;; i++)
			if (i >= elementsof(u.c))
				return a->c[s];
			else if (i != s && u.c[i] != a->c[i])
				break;
	}
	return o;
#elif defined(asocas64)
	U64_8_t		u;
	U64_8_t		v;
	U64_8_t*	a;
	int		s;
	int		i;

	s = (int)(integralof(p) & (sizeof(u.i) - 1));
	a = (U64_8_t*)((char*)0 + (integralof(p) & ~(sizeof(u.i) - 1)));
	for (;;)
	{
		u.i = a->i;
		u.c[s] = o;
		v.i = u.i;
		v.c[s] = n;
		if (asocas64(&a->i, u.i, v.i) == u.i)
			break;
		for (i = 0;; i++)
			if (i >= elementsof(u.c))
				return a->c[s];
			else if (i != s && u.c[i] != a->c[i])
				break;
	}
	return o;
#else
	ssize_t		k;

	k = lock(0);
	if (*p == o)
		*p = n;
	else
		o = *p;
	lock(k);
	return o;
#endif
}
#endif

#ifndef asocas32
uint32_t
asocas32(uint32_t volatile* p, uint32_t o, uint32_t n)
{
#if defined(asocas64)
	U64_8_t		u;
	U64_8_t		v;
	U64_8_t*	a;
	int		s;
	int		i;

	s = (int)(integralof(p) & (sizeof(u.i) - 1));
	a = (U64_8_t*)((char*)0 + (integralof(p) & ~(sizeof(u.i) - 1)));
	for (;;)
	{
		u.i = a->i;
		u.c[s] = o;
		v.i = u.i;
		v.c[s] = n;
		if (asocas64(&a->i, u.i, v.i) == u.i)
			break;
		for (i = 0;; i++)
			if (i >= elementsof(u.c))
				return a->c[s];
			else if (i != s && u.c[i] != a->c[i])
				break;
	}
	return o;
#else
	ssize_t		k;

	k = lock(0);
	if (*p == o)
		*p = n;
	else
		o = *p;
	lock(k);
	return o;
#endif
}
#endif

#ifdef _ast_int8_t

#ifndef asocas64
uint64_t
asocas64(uint64_t volatile* p, uint64_t o, uint64_t n)
{
	ssize_t		k;

	k = lock(0);
	if (*p == o)
		*p = n;
	else
		o = *p;
	lock(k);
	return o;
}
#endif

#endif

/*
 * compare with old, swap with new if same, and return old value
 */

#ifndef asocasptr
void*
asocasptr(void volatile* p, void* o, void* n)
{
	ssize_t		k;

	k = lock(0);
	if (*(void* volatile*)p == o)
		*(void* volatile*)p = n;
	else
		o = *(void* volatile*)p;
	lock(k);
	return o;
}
#endif

#if __OBSOLETE__ < 20160101

#if defined(__EXPORT__)
#define extern	extern __EXPORT__
#endif

extern int
asoloop(unsigned int k)
{
	k = (k % 21) + 1;
	if (k > 21 )
		return asoyield();
	return asorelax(1 << ((k % 21) + 1));
}

#undef	extern

#endif

#endif
