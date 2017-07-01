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
#ifndef _VMHDR_H
#define _VMHDR_H	1
#ifndef _BLD_vmalloc
#define _BLD_vmalloc	1
#endif

/*	Common types, and macros for vmalloc functions.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94.
*/

#if _PACKAGE_ast

#if !_UWIN
#define getpagesize		______getpagesize
#define _npt_getpagesize	1
#endif

#include	<ast.h>

#if _npt_getpagesize
#undef				getpagesize
#endif

#else

#include	<ast_common.h>
#include	<sys/types.h>
#include	<unistd.h>

#if !_UWIN
#define _npt_getpagesize	1
#endif

#ifndef O_cloexec
#ifdef O_CLOEXEC
#define O_cloexec		O_CLOEXEC
#else
#define O_cloexec		0
#endif
#endif

#ifndef F_dupfd_cloexec
#ifdef F_DUPFD_CLOEXEC
#define F_dupfd_cloexec		F_DUPFD_CLOEXEC
#else
#define F_dupfd_cloexec		F_DUPFD
#endif
#endif

#undef free
#undef malloc
#undef realloc

#endif /*_PACKAGE_ast*/

#include	"FEATURE/vmalloc"
#include	"vmalloc.h"

#include	<aso.h>		/* atomic scalar operations		*/
#include	<setjmp.h>	/* use the type jmp_buf for alignment	*/
#include	<debug.h>	/* DEBUG_ASSERT() and friends		*/

/* extra information needed about methods to get memory from the system */
#if defined(_WIN32)
#define _mem_win32	1	/* use the VirtualAlloc interface	*/
#endif
#if !_mem_win32 && !_mem_sbrk && !_mem_mmap_anon && !_mem_mmap_zero
#undef _std_malloc
#define _std_malloc	1	/* use native malloc/free/realloc	*/
#endif

typedef unsigned char	Vmuchar_t;
typedef unsigned long	Vmulong_t;

typedef union _head_u	Head_t;	/* the header of a memory block		*/
typedef union _body_u	Body_t;	/* the body of a memory block when free	*/
typedef struct _block_s	Block_t; /* the type of a memory block		*/
typedef struct _seg_s	Seg_t;	/* the type of a raw memory segment	*/

#define NIL(t)		((t)0)
#if __STD_C
#define NOTUSED(x)	(void)(x)
#else
#define NOTUSED(x)	(&x,1)
#endif

/* safe typecasting of scalar values */
#define VMCAST(ty,x)	((ty)((Vmulong_t)(x)) )

/* convert an address to an integral value */
#define VMLONG(addr)	((Vmulong_t)(VMCAST(Vmuchar_t*, addr) - (Vmuchar_t*)0) )

/* Round x up to a multiple of y. ROUND2 does powers-of-2 and ROUNDX does others */
#define ROUND2(x,y)	(((x) + ((y)-1)) & ~((y)-1))
#define ROUNDX(x,y)	((((x) + ((y)-1)) / (y)) * (y))
#define ROUND(x,y)	(((y)&((y)-1)) ? ROUNDX((x),(y)) : ROUND2((x),(y)) )

/* compute a value that is a common multiple of x and y */
#define MULTIPLE(x,y)	((x)%(y) == 0 ? (x) : (y)%(x) == 0 ? (y) : (y)*(x))

/* _Vmassert flags -- 0x0001..0x8000 reserved for test du jour via TEST=0x....	*/

#define VM_test		0x0000ffff	/* any TEST set				*/

#define VM_abort	0x00010000	/* abort() on assertion failure		*/
#define VM_check_reg	0x00020000	/* enable region integrity checks	*/
#define VM_check_seg	0x00040000	/* enable segment availability prechecks*/
#define VM_debug	0x00080000	/* test=debug				*/
#define VM_keep		0x00100000	/* disable free()			*/
#define VM_pause	0x00200000	/* pause() on assertion failure		*/
#define VM_usage	0x00400000	/* usage stats at each getmemory	*/
#define VM_verbose	0x00800000	/* verbose messages to standard error	*/

#define VM_anon		0x01000000	/* MAP_ANON block allocator		*/
#define VM_break	0x02000000	/* sbrk() block allocator		*/
#define VM_native	0x04000000	/* native malloc() block allocator	*/
#define VM_safe		0x08000000	/* safe MAP_ANON emulation of sbrk()	*/
#define VM_zero		0x10000000	/* /dev/zero block allocator		*/

#define VM_GETMEMORY	(VM_anon|VM_break|VM_native|VM_safe|VM_zero)

#if _UWIN
#include <ast_windows.h>
#endif

#ifndef DEBUG
#ifdef _BLD_DEBUG
#define DEBUG		1
#endif /*_BLD_DEBUG*/
#endif /*DEBUG*/
extern void		_vmmessage _ARG_((const char*, long, const char*, long));
#if DEBUG
#define MESSAGE(s)	_vmmessage(__FILE__,__LINE__, (s), 0)
#define PRINT(s,n)	_vmmessage(__FILE__,__LINE__, (s), (n))
#define ABORT()		((_Vmassert & VM_abort) )
#define PAUSE()		((_Vmassert & VM_pause) )
#define ASSERT(p)	((p) ? 0 : (MESSAGE("Assertion failed"), \
				    (ABORT() ? (abort(),0) : PAUSE() ? (pause(),0) : 0)) )
#define COUNT(n)	((n) += 1)
#define ACCOUNT(a,b)	((a) += (b))
#define INITMEMORY(m,z)	((m) ? (memset((m), 'i', (z) > 2*ALIGN ? 2*ALIGN : (z)), 0) : 0 )
#define SETBUSYMEM(m,z)	(memset(((char*)(m))+2*ALIGN, 'b', (z) <= 2*ALIGN ? 0 : ALIGN ) )
#define CHKBUSYMEM(m,z)	(memcmp(((char*)(m))+2*ALIGN, "bbbbbbbb", (z) <= 2*ALIGN ? 0 : 8) == 0 ? 1 : 0 )
#define SETFREEMEM(m,z)	(memset(((char*)(m))+2*ALIGN, 'f', (z) <= 2*ALIGN ? 0 : ALIGN ) )
#define CHKFREEMEM(m,z)	(memcmp(((char*)(m))+2*ALIGN, "ffffffff", (z) <= 2*ALIGN ? 0 : 8) == 0 ? 1 : 0 )
#define DEBUGDECL(_ty_,_ob_)	_ty_ _ob_;
#else
#define ABORT()		(0)
#define ASSERT(p)
#define COUNT(n)
#define MESSAGE(s)	(0)
#define ACCOUNT(a,b)
#define INITMEMORY(m,z)
#define SETBUSYMEM(m,z)
#define CHKBUSYMEM(m,z)
#define SETFREEMEM(m,z)
#define CHKFREEMEM(m,z)
#define DEBUGDECL(_ty_,_ob_)
#endif /*DEBUG*/

#define VM_PAGESIZE	8192 /* default assumed page size */
#define VMPAGESIZE()	(_Vmpagesize ? _Vmpagesize : _vmpagesize())
#define VMBOUNDARIES()	(_Vmmemaddr ? 0 : _vmboundaries())

/* get file name and line number recorded in region */
#define VMFLF(vm,fi,ln,fn)	((fi) = (vm)->file, (vm)->file = NIL(char*), \
		 		 (ln) = (vm)->line, (vm)->line = 0 , \
		 		 (fn) = (vm)->func, (vm)->func = NIL(Void_t*) )

/* local recursive calls */
#define KPVALLOC(vm,sz,func)		(func((vm),(sz),1) )
#define KPVRESIZE(vm,dt,sz,mv,func)	(func((vm),(dt),(sz),(mv),1) )
#define KPVFREE(vm,dt,func)		(func((vm),(dt),1) )
#define KPVALIGN(vm,sz,al,func)		(func((vm),(sz),(al),1) )

/* Block sizes will always be 0%(BITS+1) so the below bits will be free */
#define BUSY		(0x1)	/* a normal (Vmbest) block is busy	*/
#define PFREE		(0x2)	/* preceding normal block is free	*/
#define SMALL		(0x4)	/* a segment block is busy		*/
#define MARK		(0x8)	/* for marking usage (eg, beststat())	*/
#define BITS		(BUSY|PFREE|SMALL|MARK)
#define ALIGNB		(BITS+1) /* to guarantee blksize == 0%(BITS+1)	*/

/* ALIGN is chosen for three conditions:
** 1. Able to address all primitive types.
** 2. A multiple of ALIGNB==(BITS+1) as discussed above.
** 3. Large enough to cover two pointers. Note that on some machines
**    a double value will be that large anyway.
**
** Of paramount importance is the ALIGNA macro below. If the compilation
** environment is too strange to calculate ALIGNA right, then the below
** code should be commented out and ALIGNA redefined as needed.
*/
union _align_u
{	char		c, *cp;
	int		i, *ip;
	long		l, *lp;
	double		d, *dp;
	size_t		s, *sp;
	void(*		fn)();
	union _align_u*	align;
	Head_t*		head;
	Body_t*		body;
	Block_t*	block;
	_ast_fltmax_t	ld, *ldp;
	_ast_intmax_t	li, *lip;
	Vmuchar_t	a[ALIGNB];
	jmp_buf		jmp;
};
struct _a_s
{	char		c;
	union _align_u	a;
};
struct _two_s
{	void*		one;
	void*		two;
};
#define ALIGNA	(sizeof(struct _a_s) - sizeof(union _align_u))
#undef	ALIGN	/* Blocks will be aligned on both ALIGNA & ALIGNB */
#define ALIGNAB	MULTIPLE(ALIGNA,ALIGNB)
#define ALIGN	MULTIPLE(ALIGNAB, sizeof(struct _two_s))

typedef union _word_u
{	size_t		size;	/* to store a size_t	*/
	unsigned int	intdt;	/* to store an integer	*/
	Void_t*		ptrdt;	/* to store a pointer	*/
} Word_t;

struct _head_s /* a block header has two words */
{	Word_t		one;
	Word_t		two;
};
#define HEADSIZE	ROUND(sizeof(struct _head_s), ALIGN)
union _head_u
{	Vmuchar_t	data[HEADSIZE];	/* to standardize size	*/
	struct _head_s	head;
};
	
struct _body_s /* Note that self is actually at end of block */
{	Block_t*	link;	/* next in link list		*/
	Block_t*	rght;	/* right child in free tree	*/
	Block_t*	left;	/* left child in free tree	*/
	Block_t**	self;	/* self pointer when free	*/
};
#define BODYSIZE	ROUND(sizeof(struct _body_s), ALIGN)
union _body_u
{	Vmuchar_t	data[BODYSIZE];	/* to standardize size	*/
	struct _body_s	body;
};

/* After all the songs and dances, we should now have:
**	sizeof(Head_t)%ALIGN == 0
**	sizeof(Body_t)%ALIGN == 0
** and	sizeof(Block_t) = sizeof(Head_t)+sizeof(Body_t)
*/
struct _block_s
{	Head_t		head;
	Body_t		body;
};

#define SEG(b)		((b)->head.head.one.ptrdt)	/* the containing segment	*/
#define	SIZE(b)		((b)->head.head.two.size)	/* field containing block size	*/
#define BDSZ(b)		(SIZE(b) & ~BITS)		/* naked size of block		*/

#define PACK(b)		((b)->head.head.one.ptrdt)	/* the containing pack		*/

#define LINK(b)		((b)->body.body.link)		/* linked list 			*/
#define LEFT(b)		((b)->body.body.left)		/* left child in splay tree	*/
#define RGHT(b)		((b)->body.body.rght)		/* right child in splay tree	*/

/* translating between a block and its data area */
#define DATA(b)		((Void_t*)((b)->body.data) )
#define BLOCK(d)	((Block_t*)((Vmuchar_t*)(d) - sizeof(Head_t)) )

/* when a block is free, its last word stores a pointer to itself.
** in this way, a block can find its predecessor if the predecessor is free.
*/
#define SELF(b)		((Block_t**)((b)->body.data + BDSZ(b) - sizeof(Block_t*)) )
#define PREV(b)		(*((Block_t**)(((Vmuchar_t*)(b)) - sizeof(Block_t*)) ) )
#define NEXT(b)		((Block_t*)((b)->body.data + BDSZ(b)) )

#if _ast_sizeof_pointer == 4
#define SMENCODE(i)	((uint32_t)(i) << 24)		/* code index of a small block	*/
#define SMDECODE(i)	((uint32_t)(i) >> 24)		/* code index of a small block	*/
#define SMBITS		(BITS | SMENCODE(0xff))		/* bits not related to size	*/
#else
#define SMENCODE(i)	((uint64_t)(i) << 24)		/* code index of a small block	*/
#define SMDECODE(i)	((uint64_t)(i) >> 24)		/* code index of a small block	*/
#define SMBITS		(BITS | SMENCODE(0xffff))	/* bits not related to size	*/
#endif

#define SMINDEXB(b)	(SMDECODE(SIZE(b)))		/* get index of a small block	*/
#define SMBDSZ(b)	(SIZE(b) & ~SMBITS) /* size of small block	*/
#define TRUESIZE(z)	((z) & (((z)&SMALL) ? ~SMBITS : ~BITS) )
#define TRUEBDSZ(b)	((SIZE(b)&SMALL) ? SMBDSZ(b) : BDSZ(b))
#define TRUENEXT(b)	((Block_t*)((b)->body.data + TRUEBDSZ(b)) )

/* the sentinel block at the end of a "segment block" */
#define ENDB(sgb)	((Block_t*)((Vmuchar_t*)NEXT(sgb) - sizeof(Head_t)) )

/* the start of allocatable memory in a segment */
#define SEGDATA(sg)	((Vmuchar_t*)(sg) + ROUND(sizeof(Seg_t),ALIGN) )

/* testing to see if "sg" is the root segment of a region */
#define SEGROOT(sg)	((Vmuchar_t*)(sg)->vmdt >= (sg)->base && \
			 (Vmuchar_t*)(sg)->vmdt < (Vmuchar_t*)(sg) )

#if !_PACKAGE_ast
/* we don't use these here and they interfere with some local names */
#undef malloc
#undef free
#undef realloc
#endif

typedef struct _vmuser_s	Vmuser_t; /* structure for user's data	*/
struct _vmuser_s
{	Vmuser_t*		next;
	unsigned int		dtid;	/* key to identify data item	*/
	ssize_t			size;	/* size of data area		*/
	Void_t*			data;	/* user data area		*/
};

struct _seg_s /* a segment of raw memory obtained via Vmdisc_t.memoryf */
{	Vmdata_t*		vmdt;	/* region holding this segment	*/
	Vmuchar_t*		base;	/* true base address of segment	*/
	size_t			size;	/* true size of segment		*/
	int			iffy;	/* should not extend segment	*/
	Block_t*		begb;	/* starting allocatable memory	*/
	Block_t*		endb;	/* block at end of memory	*/
	Seg_t*			next;	/* next segment in linked list	*/
};

struct _free_s /* list of objects locked out by concurrent free() */
{
	struct _free_s*	next;
};
typedef struct _free_s Free_t;

struct Vmdata_s /* Vmdata_t: common region data */
{	int			mode;	/* operation modes 		*/
	unsigned int 		lock;	/* lock for segment management	*/
	size_t			incr;	/* to round memory requests	*/
	Seg_t*			seg;	/* list of raw memory segments	*/
	Vmuchar_t*		segmin;	/* min address in all segments	*/
	Vmuchar_t*		segmax;	/* max address in all segments	*/
	Block_t*		free;	/* not allocated to method yet	*/
	Vmuser_t*		user;	/* user data identified by key	*/
	unsigned int 		ulck;	/* lock of user list for update	*/
	unsigned int		dlck;	/* lock used by Vmdebug		*/
	Free_t*			delay;	/* delayed free list		*/
};

typedef struct _vmhold_s	Vmhold_t; /* to hold open regions 	*/
struct _vmhold_s
{	Vmhold_t*		next;
	Vmalloc_t*		vm;
};

#define VM_SEGEXTEND	(01)	/* physically extend memory as needed	*/
#define VM_SEGALL	(02)	/* always return entire segment		*/

/* external symbols for use inside vmalloc only */
typedef struct _vmextern_s
{	Block_t*		(*vm_seginit)_ARG_((Vmdata_t*, Seg_t*, Vmuchar_t*, ssize_t, int));
	Block_t*		(*vm_segalloc)_ARG_((Vmalloc_t*, Block_t*, ssize_t, int ));
	void			(*vm_segfree)_ARG_((Vmalloc_t*, Block_t*));
	char*			(*vm_strcpy)_ARG_((char*, const char*, int));
	char*			(*vm_itoa)_ARG_((Vmulong_t, int));
	ssize_t			(*vm_lcm)_ARG_((ssize_t, ssize_t));
	void			(*vm_trace)_ARG_((Vmalloc_t*, Vmuchar_t*, Vmuchar_t*, size_t, size_t));
	int			(*vm_chkmem)_ARG_((Vmuchar_t*, size_t));
	Vmuchar_t*		vm_memmin;   /* address lower abound	*/ 
	Vmuchar_t*		vm_memmax;   /* address upper abound	*/ 
	Vmuchar_t*		vm_memaddr;  /* vmmaddress() memory	*/
	Vmuchar_t*		vm_memsbrk;  /* Vmdcsystem's memory	*/
	Vmhold_t*		vm_hold;     /* list to hold regions	*/
	size_t			vm_pagesize; /* OS memory page size	*/
	size_t			vm_segsize;  /* min segment size	*/
	unsigned int 		vm_sbrklock; /* lock for sbrkmem	*/
	unsigned int		vm_assert;   /* options for ASSERT() 	*/
} Vmextern_t;

#define _Vmseginit	(_Vmextern.vm_seginit)
#define _Vmsegalloc	(_Vmextern.vm_segalloc)
#define _Vmsegfree	(_Vmextern.vm_segfree)
#define _Vmstrcpy	(_Vmextern.vm_strcpy)
#define _Vmitoa		(_Vmextern.vm_itoa)
#define _Vmlcm		(_Vmextern.vm_lcm)
#define _Vmtrace	(_Vmextern.vm_trace)
#define _Vmchkmem	(_Vmextern.vm_chkmem)
#define _Vmmemmin	(_Vmextern.vm_memmin)
#define _Vmmemmax	(_Vmextern.vm_memmax)
#define _Vmmemaddr	(_Vmextern.vm_memaddr)
#define _Vmmemsbrk	(_Vmextern.vm_memsbrk)
#define _Vmpagesize	(_Vmextern.vm_pagesize)
#define _Vmsegsize	(_Vmextern.vm_segsize)
#define _Vmsbrklock	(_Vmextern.vm_sbrklock)
#define _Vmhold		(_Vmextern.vm_hold)
#define _Vmassert	(_Vmextern.vm_assert)

extern Vmalloc_t*	_vmheapinit _ARG_((Vmalloc_t*)); /* initialize Vmheap	*/
extern int		_vmheapbusy _ARG_((void)); /* initializing Vmheap	*/
extern ssize_t		_vmpagesize _ARG_((void)); /* get system page size	*/
extern int		_vmboundaries _ARG_((void)); /* get mem boundaries	*/
extern Vmalloc_t*	_vmopen _ARG_((Vmalloc_t*, Vmdisc_t*, Vmethod_t*, int));
extern void		_vmoptions _ARG_((int)); /* VMALLOC_OPTIONS preferences	*/
extern int		_vmstat _ARG_((Vmalloc_t*, Vmstat_t*, size_t)); /* internal vmstat() */

_BEGIN_EXTERNS_

extern Vmextern_t	_Vmextern;

#if _PACKAGE_ast

#if _npt_getpagesize
extern int		getpagesize _ARG_((void));
#endif

#else

#if _hdr_unistd
#include	<unistd.h>
#else
extern void		abort _ARG_(( void ));
extern ssize_t		write _ARG_(( int, const void*, size_t ));
extern int		getpagesize _ARG_((void));
extern Void_t*		sbrk _ARG_((ssize_t));
#endif

#if !__STDC__ && !_hdr_stdlib
extern size_t		strlen _ARG_(( const char* ));
extern char*		strcpy _ARG_(( char*, const char* ));
extern int		strcmp _ARG_(( const char*, const char* ));
extern int		atexit _ARG_(( void(*)(void) ));
extern char*		getenv _ARG_(( const char* ));
extern Void_t*		memcpy _ARG_(( Void_t*, const Void_t*, size_t ));
extern Void_t*		memset _ARG_(( Void_t*, int, size_t ));
#else
#include	<stdlib.h>
#include	<string.h>
#endif

/* for vmexit.c */
extern int		onexit _ARG_(( void(*)(void) ));
extern void		_exit _ARG_(( int ));
extern void		_cleanup _ARG_(( void ));

#endif /*_PACKAGE_ast*/

/* for vmdcsbrk.c */
#if !_typ_ssize_t
typedef int		ssize_t;
#endif

_END_EXTERNS_

#endif /* _VMHDR_H */
