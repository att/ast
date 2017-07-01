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
#if defined(_UWIN) && defined(_BLD_ast)

void _STUB_malloc(){}

#else

#if _UWIN

#define calloc		______calloc
#define _ast_free	______free
#define malloc		______malloc
#define mallinfo	______mallinfo
#define mallopt		______mallopt
#define mstats		______mstats
#define realloc		______realloc

#define _STDLIB_H_	1

extern int		atexit(void(*)(void));
extern char*		getenv(const char*);

#endif /*_UWIN*/

#include	"vmhdr.h"
#include	<errno.h>

#if _UWIN

#include	<malloc.h>

#define _map_malloc	1
#define _mal_alloca	1

#undef	calloc
#define calloc		_ast_calloc
#undef	_ast_free
#define free		_ast_free
#undef	malloc
#define malloc		_ast_malloc
#undef	mallinfo
typedef struct ______mallinfo Mallinfo_t;
#undef	mallopt
#undef	mstats
typedef struct ______mstats Mstats_t;
#undef	realloc
#define realloc		_ast_realloc

#endif /*_UWIN*/

/* If this code is to be used as native malloc then we won't have to worry about
** freeing/resizing data allocated by some other malloc. As such, vmregion() can
** be redefined to be Vmregion to bypass a superfluous computation.
*/
#if VM_NATIVE
#define vmregion(d)	Vmregion
#endif

#if __STD_C
#define F0(f,t0)		f(t0)
#define F1(f,t1,a1)		f(t1 a1)
#define F2(f,t1,a1,t2,a2)	f(t1 a1, t2 a2)
#else
#define F0(f,t0)		f()
#define F1(f,t1,a1)		f(a1) t1 a1;
#define F2(f,t1,a1,t2,a2)	f(a1, a2) t1 a1; t2 a2;
#endif /*__STD_C*/

/*
 * define _AST_std_malloc=1 to force the standard malloc
 * if _map_malloc is also defined then _ast_malloc etc.
 * will simply call malloc etc.
 */

#if !defined(_AST_std_malloc) && __CYGWIN__
#define _AST_std_malloc	1
#endif

/*	Malloc compatibility functions
**
**	These can be used for debugging and are driven by the environment variable
**	VMALLOC_OPTIONS, a space-separated list of [no]name[=value] options:
**
**	    abort	if Vmregion==Vmdebug then VM_DBABORT is set,
**			otherwise _BLD_debug enabled assertions abort()
**			on failure
**	    check=c	enable check c[:d...], prefix c with "no" to disable
**			    region: vmbest-integrity (on by default _BLD_DEBUG)
**			    segment: _vmchkmem() anon memory availability checks
**	    debug	verbose debug trace to stderr
**	    getmemory=f	enable f[:g..] getmemory() functions if supported, all by default
**			    anon: mmap(MAP_ANON)
**			    break|sbrk: sbrk()
**			    native: native malloc()
**			    safe: safe sbrk() emulation via mmap(MAP_ANON)
**			    zero: mmap(/dev/zero)
**	    keep	disable free -- if code works with this enabled then it
**	    		probably accesses free'd data
**	    method=m	sets Vmregion=m if not defined, m (Vm prefix optional)
**			    best:  best fit
**			    debug: detailed verification checks 
**			    last:  only last malloc() value can be freed
**	    pagesize=n	sets memory allocation page size to n
**	    period=n	sets Vmregion=Vmdebug if not defined, if
**			Vmregion==Vmdebug the region is checked every n ops
**	    segsize=n	sets memory allocation segment size to n
**	    start=n	sets Vmregion=Vmdebug if not defined, if
**			Vmregion==Vmdebug region checking starts after n ops
**	    test=x	enable tests du jour in the range 0x0001..0x8000
**	    trace=f	enable tracing to file f
**	    usage	print region usage stats diagnotics on each system allocation
**	    verbose	enable method and discipline initialization messages to stderr
**	    warn=f	sets Vmregion=Vmdebug if not defined, if
**			Vmregion==Vmdebug then warnings printed to file f
**	    watch=a	sets Vmregion=Vmdebug if not defined, if
**			Vmregion==Vmdebug then address a is watched
**
**	Output files are created if they don't exist. &n and /dev/fd/n name
**	the file descriptor n which must be open for writing. The pattern %p
**	in a file name is replaced by the process ID.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94.
*/

#if _sys_stat
#include	<sys/stat.h>
#endif
#include	<fcntl.h>

#ifdef S_IRUSR
#define CREAT_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#else
#define CREAT_MODE	0644
#endif

static Vmulong_t	_Vmdbtime = 0;	/* clock counting malloc/free/realloc	*/
static Vmulong_t	_Vmdbstart = 0;	/* start checking when time passes this	*/
static Vmulong_t	_Vmdbcheck = 0;	/* check region periodically with this	*/

#if __STD_C
static Vmulong_t atou(char** sp)
#else
static Vmulong_t atou(sp)
char**	sp;
#endif
{
	char*		s = *sp;
	Vmulong_t	v = 0;
	int		b;

	if(s[0] == '0' && (s[1] == 'x' || s[1] == 'X') )
	{	for(s += 2; *s; ++s)
		{	if(*s >= '0' && *s <= '9')
				v = (v << 4) + (*s - '0');
			else if(*s >= 'a' && *s <= 'f')
				v = (v << 4) + (*s - 'a') + 10;
			else if(*s >= 'A' && *s <= 'F')
				v = (v << 4) + (*s - 'A') + 10;
			else break;
		}
	}
	else
	{	for(; *s; ++s)
		{	if(*s >= '0' && *s <= '9')
				v = v*10 + (*s - '0');
			else break;
		}
	}
	switch (*s)
	{
	case 'k':
	case 'K':
		b = 10;
		break;
	case 'm':
	case 'M':
		b = 20;
		break;
	case 'g':
	case 'G':
		b = 30;
		break;
	case 't':
	case 'T':
		b = 40;
		break;
	case 'p':
	case 'P':
		b = 50;
		break;
	case 'e':
	case 'E':
		b = 60;
		break;
	default:
		b = 0;
		break;
	}
	if (b)
	{	if (*++s == 'i' || *s == 'I')
		{	if (*++s == 'b' || *s == 'B')
				s++;
			v <<= b;
		}
		else
			for (b /= 10; b; b--)
				v *= 1000;
	}
	*sp = s;
	return v;
}

#if __STD_C
static char* insertpid(char* begs, char* ends)
#else
static char* insertpid(begs,ends)
char*	begs;
char*	ends;
#endif
{	int	pid;
	char*	s;

	if((pid = getpid()) < 0)
		return NIL(char*);

	s = ends;
	do
	{	if(s == begs)
			return NIL(char*);
		*--s = '0' + pid%10;
	} while((pid /= 10) > 0);
	while(s < ends)
		*begs++ = *s++;

	return begs;
}

#if __STD_C
static int createfile(char* file)
#else
static int createfile(file)
char*	file;
#endif
{
	char	buf[1024];
	char	*next, *endb;
	int	fd;

	next = buf;
	endb = buf + sizeof(buf);
	while(*file)
	{	if(*file == '%')
		{	switch(file[1])
			{
			case 'p' :
				if(!(next = insertpid(next,endb)) )
					return -1;
				file += 2;
				break;
			default :
				goto copy;
			}
		}
		else
		{ copy:
			*next++ = *file++;
		}

		if(next >= endb)
			return -1;
	}

	*next = '\0';
	file = buf;
	if (*file == '&' && *(file += 1) || strncmp(file, "/dev/fd/", 8) == 0 && *(file += 8))
	{
		fd = fcntl((int)atou(&file), F_DUPFD_CLOEXEC, 0);
#if F_DUPFD_CLOEXEC == F_DUPFD
		if (fd >= 0)
			SETCLOEXEC(fd);
#endif
	}
	else if (*file)
	{
		fd = open(file, O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC, CREAT_MODE);
#if O_CLOEXEC == 0
		if (fd >= 0)
			SETCLOEXEC(fd);
#endif
	}
	else
		return -1;
	return fd;
}

/* Initialize runtime options from the VMALLOC_OPTIONS env var.
** This function is idempotent. Call at least once with boot==1
** to initialize getmemory preferences (called by _vmstart() and
** _vmheapinit()) and at least once with boot==2 to initialize heap
** options (called by _vmstart()), or call with boot==3 to do both.
*/
void _vmoptions(int boot)
{
	char		*s, *t, *v;
	Vmulong_t	n;
	int		fd;
	int		b;
	int		c;
	char		buf[1024];
	char		*trace = NIL(char*);
	Vmalloc_t	*vm = NIL(Vmalloc_t*);

	static char*	options;

	if (boot & 1)
	{	if (_Vmassert & VM_GETMEMORY)
			return;
		options = getenv("VMALLOC_OPTIONS");
	}
	else if (!(_Vmassert & VM_GETMEMORY))
		return;
	if (options && options[0])
	{	/* copy option string to a writable buffer */
		for(s = &buf[0], t = options, v = &buf[sizeof(buf)-1]; s < v; ++s)
			if((*s = *t++) == 0 )
				break;
		*s = 0;

		for(s = buf;; )
		{	/* skip blanks to option name */
			while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n' || *s == ',')
				s++;
			if (*(t = s) == 0)
				break;

			v = NIL(char*);
			while (*s)
			{	if (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n' || *s == ',')
				{	*s++ = 0; /* end of name */
					break;
				}
				else if (!v && *s == '=')
				{	*s++ = 0; /* end of name */
					if (*(v = s) == 0)
						v = NIL(char*);
				}
				else	s++;
			}
			if (t[0] == 'n' && t[1] == 'o')
				continue;
			switch (t[0])
			{
			case 'a':		/* abort */
				if (boot & 2)
				{	if (!vm)
						vm = vmopen(Vmdcsystem, Vmdebug, 0);
					if (vm && vm->meth.meth == VM_MTDEBUG)
						vmset(vm, VM_DBABORT, 1);
					else	_Vmassert |= VM_abort;
				}
				break;
			case 'c':		/* address/integrity checks */
				if ((boot & 2) && v)
					do
					{
						if (v[0] == 'n' && v[1] == 'o')
						{
							v += 2;
							if (v[0] == '-')
								v++;
							c = 0;
						}
						else
							c = 1;
						switch (v[0])
						{
						case 'r':
							b = VM_check_reg;
							break;
						case 's':
							b = VM_check_seg;
							break;
						default:
							b = 0;
							break;
						}
						if (c)
							_Vmassert |= b;
						else
							_Vmassert &= ~b;
					} while ((v = strchr(v, ':')) && ++v);
				break;
			case 'd':		/* debug */
				if (boot & 2)
					_Vmassert |= VM_debug;
				break;
			case 'g':		/* getmemory() preference */
				if ((boot & 1) && v)
					do
					{
						if (v[0] == 'n' && v[1] == 'o')
						{
							v += 2;
							if (v[0] == '-')
								v++;
							c = 0;
						}
						else
							c = 1;
						if (v[0] == 'm' && v[1] == 'm')
							v++;
						if (v[0] == 'm' && v[1] == 'a' && v[2] == 'p')
							v += 3;
						switch (v[0])
						{
						case 'a':
							b = VM_anon;
							break;
						case 'b':
							b = VM_break;
							break;
						case 'm':
						case 'n':
							b = VM_native;
							break;
						case 's':
							switch (v[1])
							{
							case 'b':
								b = VM_break;
								break;
							default:
								b = VM_safe;
								break;
							}
							break;
						case 'z':
							b = VM_zero;
							break;
						default:
							b = 0;
							break;
						}
						if (c)
							_Vmassert |= b;
						else
							_Vmassert &= ~b;
					} while ((v = strchr(v, ':')) && ++v);
				break;
			case 'k':		/* keep */
				if (boot & 2)
					_Vmassert |= VM_keep;
				break;
			case 'm':		/* method=<method> */
				if ((boot & 2) && v && !vm)
				{
					if ((v[0] == 'V' || v[0] == 'v') && (v[1] == 'M' || v[1] == 'm'))
						v += 2;
					switch (v[0])
					{
					case 'b':
						vm = Vmheap;
						break;
					case 'd':	/* debug */
						vm = vmopen(Vmdcsystem, Vmdebug, 0);
						break;
					case 'l':	/* last */
						vm = vmopen(Vmdcsystem, Vmlast, 0);
						break;
					case 'p':	/* pool */
						vm = vmopen(Vmdcsystem, Vmpool, 0);
						break;
					}
				}
				break;
			case 'p':		/* pagesize=<size> period=<count> */
				switch (t[1])
				{
				case 'a':
					if (boot & 1)
						_Vmpagesize = atou(&v);
					break;
				case 'e':
					if (boot & 2)
					{
						if (!vm)
							vm = vmopen(Vmdcsystem, Vmdebug, 0);
						if (v && vm && vm->meth.meth == VM_MTDEBUG)
							_Vmdbcheck = atou(&v);
					}
					break;
				}
				break;
			case 's':		/* segsize=<size> start=<count> */
				switch (t[1])
				{
				case 'e':
					if (boot & 1)
						_Vmsegsize = atou(&v);
					break;
				case 't':
					if (boot & 2)
					{
						if (!vm)
							vm = vmopen(Vmdcsystem, Vmdebug, 0);
						if (v && vm && vm->meth.meth == VM_MTDEBUG)
							_Vmdbstart = atou(&v);
					}
					break;
				}
				break;
			case 't':		/* test || trace=<path> */
				if (v)
				{
					if ((boot & 1) && t[1] == 'e')	/* test */
						_Vmassert |= atou(&v) & VM_test;
					if ((boot & 2) && t[1] == 'r')	/* trace=<path> */
						trace = v;
				}
				break;
			case 'u':		/* usage */
				if (boot & 1)
					_Vmassert |= VM_usage;
				break;
			case 'v':		/* verbose */
				if (boot & 1)
					_Vmassert |= VM_verbose;
				break;
			case 'w':
				if ((boot & 2) && t[1] == 'a')
				{	switch (t[2])
					{
					case 'r':	/* warn=<path> */
						if (!vm)
							vm = vmopen(Vmdcsystem, Vmdebug, 0);
						if (vm && vm->meth.meth == VM_MTDEBUG &&
						    v && (fd = createfile(v)) >= 0 )
							vmdebug(fd);
						break;
					case 't':	/* watch=<addr> */
						if (!vm)
							vm = vmopen(Vmdcsystem, Vmdebug, 0);
						if (vm && vm->meth.meth == VM_MTDEBUG &&
						    v && (n = atou(&v)) > 0 )
							vmdbwatch((Void_t*)n);
						break;
					}
				}
				break;
			}
		}

		if (vm) /* slip the new region in to drive malloc/free/realloc */
		{	if (vm->meth.meth == VM_MTDEBUG && _Vmdbcheck == 0 )
				_Vmdbcheck = 1;
			Vmregion = vm;
		}

		/* enable tracing */
		if (trace && (fd = createfile(trace)) >= 0)
			vmtrace(fd);
	}
	if ((boot & 1) && !(_Vmassert & VM_GETMEMORY))
		_Vmassert |= VM_GETMEMORY;
}

#if ( !_std_malloc || !_BLD_ast ) && !_AST_std_malloc

#if !_map_malloc

#undef	calloc
#undef	cfree
#undef	free
#undef	mallinfo
#undef	malloc
#undef	mallopt
#undef	memalign
#undef	posix_memalign
#undef	mstats
#undef	realloc
#undef	valloc

#if _malloc_hook

#include <malloc.h>

#undef	calloc
#undef	cfree
#undef	free
#undef	malloc
#undef	memalign
#undef	posix_memalign
#undef	realloc

#define calloc		_ast_calloc
#define cfree		_ast_cfree
#define free		_ast_free
#define malloc		_ast_malloc
#define memalign	_ast_memalign
#define posix_memalign	_ast_posix_memalign
#define realloc		_ast_realloc

#endif

#endif

#if _WINIX

#include <ast_windows.h>

#if _UWIN

#define VMRECORD(p)	_vmrecord(p)
#define VMBLOCK		{ int _vmblock = _sigblock();
#define VMUNBLOCK	_sigunblock(_vmblock); }

extern int		_sigblock(void);
extern void		_sigunblock(int);
extern unsigned long	_record[2048];

__inline Void_t* _vmrecord(Void_t* p)
{
	register unsigned long	v = ((unsigned long)p)>>16; 

	_record[v>>5] |= 1<<((v&0x1f));
	return p;
}

#else

#define getenv(s)	lcl_getenv(s)

static char*
lcl_getenv(const char* s)
{
	int		n;
	static char	buf[512];

	if (!(n = GetEnvironmentVariable(s, buf, sizeof(buf))) || n > sizeof(buf))
		return 0;
	return buf;
}

#endif /* _UWIN */

#endif /* _WINIX */

#ifndef VMRECORD
#define VMRECORD(p)	(p)
#define VMBLOCK
#define VMUNBLOCK
#endif

#if defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

/* not sure of all the implications -- 0 is conservative for now */
#define USE_NATIVE	0	/* native free/realloc on non-vmalloc ptrs */

#if USE_NATIVE
static void*		native_realloc _ARG_((void*, size_t));
static void		native_free _ARG_((void*));
#endif

#define VM_STARTING	1
#define VM_STARTED	2
static unsigned int	_Vmstart = 0;	/* calling _vmstart() just once		*/
#define VMPROLOGUE(f) \
	{ if(_Vmstart != VM_STARTED)	_vmstart(f); \
	  if(_Vmdbcheck && Vmregion->meth.meth == VM_MTDEBUG) \
	  { _Vmdbtime += 1; \
	    if(_Vmdbtime >= _Vmdbstart && (_Vmdbtime % _Vmdbcheck) == 0 ) \
		vmset(Vmregion, VM_DBCHECK, 1); \
	  } \
	}
#define VMEPILOGUE(f) \
	{ if(_Vmdbcheck && Vmregion->meth.meth == VM_MTDEBUG) \
		vmset(Vmregion, VM_DBCHECK, 0); \
	}

static int _vmstart(int freeing)
{
	unsigned int	start;
	char		*file;
	int		line;
	Void_t		*func;

	_vmoptions(1);

	if (_Vmassert & VM_debug) debug_printf(2, "%s:%d: _Vmstart=%s\n", _Vmstart == 0 ? "UNINITIALIZED" : _Vmstart == VM_STARTING ? "STARTING" : _Vmstart == VM_STARTED ? "STARTED" : "ERROR" );

	/* compete for the right to do initialization */
	if((start = asocasint(&_Vmstart, 0, VM_STARTING)) == VM_STARTED )
		return 0;
	else if(start == VM_STARTING) /* wait until initialization is done */
	{	asospindecl();
		int	i = 0;

		/*
		 * we allow free() to be called by signal handlers and not deadlock
		 * not so for *alloc()
		 */

		for(asospininit();; asospinnext())
		{	if((start = asogetint(&_Vmstart)) == VM_STARTED)
				return 0;
			if(freeing && ++i >= 10)
				return 0;
		}
	}

	/* initialize the heap if not done yet */
	if(_vmheapinit(NIL(Vmalloc_t*)) != Vmheap )
	{	write(9, "vmalloc: panic: heap initialization error\n", 42);
		return -1;
	}
	/**/DEBUG_ASSERT(Vmheap->data != NIL(Vmdata_t*));

	/* setting options. note that Vmregion may change */
	VMFLF(Vmregion, file, line, func);
	_vmoptions(2);
	Vmregion->file = file; /* reset values for the real call */
	Vmregion->line = line;
	Vmregion->func = func;

	asocasint(&_Vmstart, VM_STARTING, VM_STARTED);

	if (_Vmassert & VM_verbose) debug_printf(2, "vmalloc: method=%s\n", Vmregion->meth.meth == VM_MTBEST ? "best" : Vmregion->meth.meth == VM_MTDEBUG ? "debug" : Vmregion->meth.meth == VM_MTLAST ? "last" : Vmregion->meth.meth == VM_MTPOOL ? "pool" : "unknown");

	return 0;
}

extern Void_t* calloc(size_t n_obj, size_t s_obj)
{
	Void_t		*addr;

	VMPROLOGUE(0); 
	addr = (*Vmregion->meth.resizef)(Vmregion, NIL(Void_t*), n_obj*s_obj, VM_RSZERO, 0);
	VMEPILOGUE(0); 

	return VMRECORD(addr);
}

extern Void_t* malloc(size_t size)
{
	Void_t		*addr;

	VMPROLOGUE(0);
	addr = (*Vmregion->meth.allocf)(Vmregion, size, 0);
	VMEPILOGUE(0); 

	return VMRECORD(addr);
}

extern Void_t* realloc(Void_t* data, size_t size)
{
	Void_t		*addr;
	Vmalloc_t	*vm;

	VMPROLOGUE(0);

	if(!data)
		return malloc(size);
	else if((vm = vmregion(data)) )
		addr = (*vm->meth.resizef)(vm, data, size, VM_RSCOPY|VM_RSMOVE, 0);
	else /* not our data */
#if USE_NATIVE
		addr = native_realloc(data, size);
#else 
		addr = NIL(Void_t*);
#endif

	VMEPILOGUE(0);
	return VMRECORD(addr);
}

extern void free(Void_t* data)
{
	Vmalloc_t	*vm;

	VMPROLOGUE(1);

	if(data && !(_Vmassert & VM_keep))
	{	if((vm = vmregion(data)) )
			(void)(*vm->meth.freef)(vm, data, 0);
#if USE_NATIVE
		else /* not our data */
			native_free(data);
#endif
	}

	VMEPILOGUE(1);
}

extern void cfree(Void_t* data)
{
	free(data);
}

extern Void_t* memalign(size_t align, size_t size)
{
	Void_t		*addr;

	VMPROLOGUE(0);

	VMBLOCK
	addr = (*Vmregion->meth.alignf)(Vmregion, size, align, 0);
	VMUNBLOCK

	VMEPILOGUE(0);

	return VMRECORD(addr);
}

extern Void_t* aligned_alloc(size_t align, size_t size)
{
	return memalign(align, ROUND(size,align));
}

extern int posix_memalign(Void_t **memptr, size_t align, size_t size)
{
	Void_t	*mem;

	if(align == 0 || (align%sizeof(Void_t*)) != 0 || ((align-1)&align) != 0 )
		return EINVAL;

	if(!(mem = memalign(align, size)) )
		return ENOMEM;

	*memptr = mem;
	return 0;
}

extern Void_t* valloc(size_t size)
{
	Void_t	*addr;

	VMPROLOGUE(0);

	VMPAGESIZE();
	addr = memalign(_Vmpagesize, size);

	VMEPILOGUE(0);

	return VMRECORD(addr);
}

extern Void_t* pvalloc(size_t size)
{
	Void_t	*addr;

	VMPROLOGUE(0);

	VMPAGESIZE();
	addr = memalign(_Vmpagesize, ROUND(size,_Vmpagesize));

	VMEPILOGUE(0);
	return VMRECORD(addr);
}

#if !_PACKAGE_ast
char* strdup(const char* s)
{
	char	*ns;
	size_t	n;

	if(!s)
		return NIL(char*);
	else
	{	n = strlen(s);
		if((ns = malloc(n+1)) )
			memcpy(ns,s,n+1);
		return ns;
	}
}
#endif /* _PACKAGE_ast */

#if !_lib_alloca || _mal_alloca
#ifndef _stk_down
#define _stk_down	0
#endif
typedef struct Alloca_s	Alloca_t;
union Alloca_u
{	struct
	{	char*		addr;
		Alloca_t*	next;
	} head;
	char	array[ALIGN];
};
struct Alloca_s
{	union Alloca_u	head;
	Vmuchar_t	data[1];
};

extern Void_t* alloca(size_t size)
{	char		array[ALIGN];
	char*		file;
	int		line;
	Void_t*		func;
	Alloca_t*	f;
	Vmalloc_t	*vm;
	static Alloca_t* Frame;

	VMPROLOGUE(0);

	VMFLF(Vmregion,file,line,func); /* save info before freeing frames */

	while(Frame) /* free unused frames */
	{	if(( _stk_down && &array[0] > Frame->head.head.addr) ||
		   (!_stk_down && &array[0] < Frame->head.head.addr) )
		{	f = Frame; Frame = f->head.head.next;
			if((vm = vmregion(f)) )
				(void)(*vm->meth.freef)(vm, f, 0);
			/* else: something bad happened. just keep going */
		}
		else	break;
	}

	Vmregion->file = file; /* restore file/line info before allocation */
	Vmregion->line = line;
	Vmregion->func = func;

	f = (Alloca_t*)(*Vmregion->meth.allocf)(Vmregion, size+sizeof(Alloca_t)-1, 0);

	/* if f is NULL, this mimics a stack overflow with a memory error! */
	f->head.head.addr = &array[0];
	f->head.head.next = Frame;
	Frame = f;

	VMEPILOGUE(0);

	return (Void_t*)f->data;
}
#endif /*!_lib_alloca || _mal_alloca*/

#if _map_malloc

/* _ast_* versions of malloc & friends */
#else

#if _malloc_hook

static void vm_free_hook(void* ptr, const void* caller)
{
	free(ptr);
}

static void* vm_malloc_hook(size_t size, const void* caller)
{
	void*	r;

	r = malloc(size);
	return r;
}

static void* vm_memalign_hook(size_t align, size_t size, const void* caller)
{
	void*	r;

	r = memalign(align, size);
	return r;
}

static void* vm_realloc_hook(void* ptr, size_t size, const void* caller)
{
	void*	r;

	r = realloc(ptr, size);
	return r;
}

static void vm_initialize_hook(void)
{
	__free_hook = vm_free_hook;
	__malloc_hook = vm_malloc_hook;
	__memalign_hook = vm_memalign_hook;
	__realloc_hook = vm_realloc_hook;
}

typeof (__malloc_initialize_hook) __malloc_initialize_hook = vm_initialize_hook;

#if 0 /* 2012-02-29 this may be needed to cover shared libs */

void __attribute__ ((constructor)) vm_initialize_initialize_hook(void)
{
	vm_initialize_hook();
	__malloc_initialize_hook = vm_initialize_hook;
}

#endif

#else

/* intercept _* __* __libc_* variants */

#if __lib__malloc
extern Void_t*	F2(_calloc, size_t,n, size_t,m) { return calloc(n, m); }
extern Void_t	F1(_cfree, Void_t*,p) { free(p); }
extern Void_t	F1(_free, Void_t*,p) { free(p); }
extern Void_t*	F1(_malloc, size_t,n) { return malloc(n); }
#if _lib_memalign
extern Void_t*	F2(_memalign, size_t,a, size_t,n) { return memalign(a, n); }
#endif
#if _lib_pvalloc
extern Void_t*	F1(_pvalloc, size_t,n) { return pvalloc(n); }
#endif
extern Void_t*	F2(_realloc, Void_t*,p, size_t,n) { return realloc(p, n); }
#if _lib_valloc
extern Void_t*	F1(_valloc, size_t,n) { return valloc(n); }
#endif
#endif

#if _lib___malloc
extern Void_t*	F2(__calloc, size_t,n, size_t,m) { return calloc(n, m); }
extern Void_t	F1(__cfree, Void_t*,p) { free(p); }
extern Void_t	F1(__free, Void_t*,p) { free(p); }
extern Void_t*	F1(__malloc, size_t,n) { return malloc(n); }
#if _lib_memalign
extern Void_t*	F2(__memalign, size_t,a, size_t,n) { return memalign(a, n); }
#endif
#if _lib_pvalloc
extern Void_t*	F1(__pvalloc, size_t,n) { return pvalloc(n); }
#endif
extern Void_t*	F2(__realloc, Void_t*,p, size_t,n) { return realloc(p, n); }
#if _lib_valloc
extern Void_t*	F1(__valloc, size_t,n) { return valloc(n); }
#endif
#endif

#if _lib___libc_malloc
extern Void_t*	F2(__libc_calloc, size_t,n, size_t,m) { return calloc(n, m); }
extern Void_t	F1(__libc_cfree, Void_t*,p) { free(p); }
extern Void_t	F1(__libc_free, Void_t*,p) { free(p); }
extern Void_t*	F1(__libc_malloc, size_t,n) { return malloc(n); }
#if _lib_memalign
extern Void_t*	F2(__libc_memalign, size_t,a, size_t,n) { return memalign(a, n); }
#endif
#if _lib_pvalloc
extern Void_t*	F1(__libc_pvalloc, size_t,n) { return pvalloc(n); }
#endif
extern Void_t*	F2(__libc_realloc, Void_t*,p, size_t,n) { return realloc(p, n); }
#if _lib_valloc
extern Void_t*	F1(__libc_valloc, size_t,n) { return valloc(n); }
#endif
#endif

#endif /* _malloc_hook */

#endif /* _map_malloc */

#undef	extern

#if _hdr_malloc /* need the mallint interface for statistics, etc. */

#undef	calloc
#define calloc		______calloc
#undef	cfree
#define cfree		______cfree
#undef	free
#define free		______free
#undef	malloc
#define malloc		______malloc
#undef	pvalloc
#define pvalloc		______pvalloc
#undef	realloc
#define realloc		______realloc
#undef	valloc
#define valloc		______valloc

#if !_UWIN

#include	<malloc.h>

typedef struct mallinfo Mallinfo_t;
typedef struct mstats Mstats_t;

#endif

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#if _lib_mallopt
#if __STD_C
extern int mallopt(int cmd, int value)
#else
extern int mallopt(cmd, value)
int	cmd;
int	value;
#endif
{
	VMPROLOGUE(0);
	VMEPILOGUE(0);
	return 0;
}
#endif /*_lib_mallopt*/

#if _lib_mallinfo && _mem_arena_mallinfo
#if __STD_C
extern Mallinfo_t mallinfo(void)
#else
extern Mallinfo_t mallinfo()
#endif
{
	Vmstat_t	sb;
	Mallinfo_t	mi;

	VMPROLOGUE(0);
	VMEPILOGUE(0);

	memset(&mi,0,sizeof(mi));
	if(vmstat(Vmregion,&sb) >= 0)
	{	mi.arena = sb.extent;
		mi.ordblks = sb.n_busy+sb.n_free;
		mi.uordblks = sb.s_busy;
		mi.fordblks = sb.s_free;
	}
	return mi;
}
#endif /* _lib_mallinfo */

#if _lib_mstats && _mem_bytes_total_mstats
#if __STD_C
extern Mstats_t mstats(void)
#else
extern Mstats_t mstats()
#endif
{
	Vmstat_t	sb;
	Mstats_t	ms;

	VMPROLOGUE(0);
	VMEPILOGUE(0);

	memset(&ms,0,sizeof(ms));
	if(vmstat(Vmregion,&sb) >= 0)
	{	ms.bytes_total = sb.extent;
		ms.chunks_used = sb.n_busy;
		ms.bytes_used = sb.s_busy;
		ms.chunks_free = sb.n_free;
		ms.bytes_free = sb.s_free;
	}
	return ms;
}
#endif /*_lib_mstats*/

#undef	extern

#endif/*_hdr_malloc*/

#else

/*
 * even though there is no malloc override, still provide
 * _ast_* counterparts for object compatibility
 */

#undef	calloc
extern Void_t*	calloc _ARG_((size_t, size_t));

#undef	cfree
extern void	cfree _ARG_((Void_t*));

#undef	free
extern void	free _ARG_((Void_t*));

#undef	malloc
extern Void_t*	malloc _ARG_((size_t));

#if _lib_memalign
#undef	memalign
extern Void_t*	memalign _ARG_((size_t, size_t));
#endif

#if _lib_pvalloc
#undef	pvalloc
extern Void_t*	pvalloc _ARG_((size_t));
#endif

#undef	realloc
extern Void_t*	realloc _ARG_((Void_t*, size_t));

#if _lib_valloc
#undef	valloc
extern Void_t*	valloc _ARG_((size_t));
#endif

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Void_t*	F2(_ast_calloc, size_t,n, size_t,m) { return calloc(n, m); }
extern Void_t	F1(_ast_cfree, Void_t*,p) { free(p); }
extern Void_t	F1(_ast_free, Void_t*,p) { free(p); }
extern Void_t*	F1(_ast_malloc, size_t,n) { return malloc(n); }
#if _lib_memalign
extern Void_t*	F2(_ast_memalign, size_t,a, size_t,n) { return memalign(a, n); }
#endif
#if _lib_pvalloc
extern Void_t*	F1(_ast_pvalloc, size_t,n) { return pvalloc(n); }
#endif
extern Void_t*	F2(_ast_realloc, Void_t*,p, size_t,n) { return realloc(p, n); }
#if _lib_valloc
extern Void_t*	F1(_ast_valloc, size_t,n) { return valloc(n); }
#endif

#undef	extern

#if _hdr_malloc

#undef	mallinfo
#undef	mallopt
#undef	mstats

#define calloc		______calloc
#define cfree		______cfree
#define free		______free
#define malloc		______malloc
#define pvalloc		______pvalloc
#define realloc		______realloc
#define valloc		______valloc

#if !_UWIN

#include	<malloc.h>

typedef struct mallinfo Mallinfo_t;
typedef struct mstats Mstats_t;

#endif

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#if _lib_mallopt
extern int	F2(_ast_mallopt, int,cmd, int,value) { return mallopt(cmd, value); }
#endif

#if _lib_mallinfo && _mem_arena_mallinfo
extern Mallinfo_t	F0(_ast_mallinfo, void) { return mallinfo(); }
#endif

#if _lib_mstats && _mem_bytes_total_mstats
extern Mstats_t		F0(_ast_mstats, void) { return mstats(); }
#endif

#undef	extern

#endif /*_hdr_malloc*/

#endif /*!_std_malloc*/

/*
 * ast semi-private workaround for system functions
 * that misbehave by passing bogus addresses to free()
 *
 * not prototyped in any header to keep it ast semi-private
 *
 * to keep malloc() data by disabling free()
 *	extern _vmkeep(int);
 *	int r = _vmkeep(1);
 * and to restore to the previous state
 *	(void)_vmkeep(r);
 */

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int
#if __STD_C
_vmkeep(int v)
#else
_vmkeep(v)
int	v;
#endif
{
	int	r;

	r = !!(_Vmassert & VM_keep);
	if (v)
		_Vmassert |= VM_keep;
	else
		_Vmassert &= ~VM_keep;
	return r;
}

#undef	extern

#if USE_NATIVE

#undef	realloc

extern void*	realloc(void*, size_t);

static void*	native_realloc(void* p, size_t n)
{
	return realloc(p, n);
}

#undef	free

extern void	free(void*);

static void	native_free(void* p)
{
	free(p);
}

#endif

#endif /*_UWIN*/
