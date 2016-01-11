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
/*
 * Advanced Software Technology Library
 * AT&T Research
 *
 * std + posix + ast
 */

#ifndef _AST_H
#define _AST_H		1

#ifndef _AST_STD_H
#include <ast_std.h>
#endif

#ifndef _SFIO_H
#include <sfio.h>
#endif

#ifndef	ast
#define ast		_ast_info
#endif

#ifndef PATH_MAX
#define PATH_MAX	1024
#endif

/*
 * workaround botched headers that assume <stdio.h>
 */

#ifndef FILE
#ifndef _SFIO_H
struct _sfio_s;
#endif
#define FILE		struct _sfio_s
#ifndef	__FILE_typedef
#define __FILE_typedef	1
#endif
#ifndef _FILEDEFED
#define _FILEDEFED	1
#endif
#endif

/*
 * exit() support -- this matches shell exit codes
 */

#define EXIT_BITS	8			/* # exit status bits	*/

#define EXIT_USAGE	2			/* usage exit code	*/
#define EXIT_QUIT	((1<<(EXIT_BITS))-1)	/* parent should quit	*/
#define EXIT_NOTFOUND	((1<<(EXIT_BITS-1))-1)	/* command not found	*/
#define EXIT_NOEXEC	((1<<(EXIT_BITS-1))-2)	/* other exec error	*/

#define EXIT_CODE(x)	((x)&((1<<EXIT_BITS)-1))
#define EXIT_CORE(x)	(EXIT_CODE(x)|(1<<EXIT_BITS)|(1<<(EXIT_BITS-1)))
#define EXIT_TERM(x)	(EXIT_CODE(x)|(1<<EXIT_BITS))

/*
 * NOTE: for compatibility the following work for EXIT_BITS={7,8}
 */

#define EXIT_STATUS(x)	(((x)&((1<<(EXIT_BITS-2))-1))?(x):EXIT_CODE((x)>>EXIT_BITS))

#define EXITED_CORE(x)	(((x)&((1<<EXIT_BITS)|(1<<(EXIT_BITS-1))))==((1<<EXIT_BITS)|(1<<(EXIT_BITS-1)))||((x)&((1<<(EXIT_BITS-1))|(1<<(EXIT_BITS-2))))==((1<<(EXIT_BITS-1))|(1<<(EXIT_BITS-2))))
#define EXITED_TERM(x)	((x)&((1<<EXIT_BITS)|(1<<(EXIT_BITS-1))))

/*
 * astconflist() flags
 */

#define ASTCONF_parse		0x0001
#define ASTCONF_write		0x0002
#define ASTCONF_read		0x0004
#define ASTCONF_lower		0x0008
#define ASTCONF_base		0x0010
#define ASTCONF_defined		0x0020
#define ASTCONF_quote		0x0040
#define ASTCONF_table		0x0080
#define ASTCONF_matchcall	0x0100
#define ASTCONF_matchname	0x0200
#define ASTCONF_matchstandard	0x0400
#define ASTCONF_error		0x0800
#define ASTCONF_system		0x1000
#define ASTCONF_AST		0x2000

/*
 * pathcanon()/pathdev() flags and info
 */

#define PATH_PHYSICAL		0x001
#define PATH_DOTDOT		0x002
#define PATH_EXISTS		0x004
#define PATH_DEV		0x008
#define PATH_ABSOLUTE		0x010	/* NOTE: shared with pathaccess() below */
#define PATH_CANON		0x020
#define PATH_DROP_HEAD_SLASH2	0x040
#define PATH_DROP_TAIL_SLASH	0x080
#define PATH_EXCEPT_LAST	0x100
#define PATH_VERIFIED(n)	(((n)&0xfffff)<<12)
#define PATH_GET_VERIFIED(n)	(((n)>>12)&0xfffff)

typedef struct Pathpart_s
{
	unsigned short	offset;
	unsigned short	size;
} Pathpart_t;

typedef struct Pathdev_s
{
	int		fd;
	int		oflags;
	pid_t		pid;
	Pathpart_t	prot;
	Pathpart_t	host;
	Pathpart_t	port;
	Pathpart_t	path;
} Pathdev_t;

/*
 * pathaccess() flags
 */

#define PATH_READ	0x04
#define PATH_WRITE	0x02
#define PATH_EXECUTE	0x01
#define	PATH_REGULAR	0x08
/* NOTE: PATH_ABSOLUTE shared with pathcanon()/pathdev() above */

/*
 * touch() flags
 */

#define PATH_TOUCH_CREATE	01
#define PATH_TOUCH_VERBATIM	02

/*
 * pathcheck() info
 */

typedef struct
{
	unsigned long	date;
	char*		feature;
	char*		host;
	char*		user;
} Pathcheck_t;

/*
 * strgrpmatch() flags
 */

#define STR_MAXIMAL	0x01		/* maximal match		*/
#define STR_LEFT	0x02		/* implicit left anchor		*/
#define STR_RIGHT	0x04		/* implicit right anchor	*/
#define STR_ICASE	0x08		/* ignore case			*/
#define STR_GROUP	0x10		/* (|&) inside [@|&](...) only	*/
#define STR_INT		0x20		/* deprecated int* match array	*/

/*
 * fmtquote() flags
 */

#define FMT_ALWAYS	0x01		/* always quote			*/
#define FMT_ESCAPED	0x02		/* already escaped		*/
#define FMT_SHELL	0x04		/* escape $ ` too		*/
#define FMT_WIDE	0x08		/* don't escape 8 bit chars	*/
#define FMT_PARAM	0x10		/* disable FMT_SHELL ${$( quote	*/

/*
 * chrexp() flags
 */

#define FMT_EXP_CHAR	0x020		/* expand single byte chars	*/
#define FMT_EXP_LINE	0x040		/* expand \n and \r		*/
#define FMT_EXP_WIDE	0x080		/* expand \u \U \x wide chars	*/
#define FMT_EXP_NOCR	0x100		/* skip \r			*/
#define FMT_EXP_NONL	0x200		/* skip \n			*/

/*
 * multibyte macros
 */

#define mbmax()			(ast.mb_cur_max)

#define mbcoll()		(ast.mb_xfrm!=0)
#define mbwide()		(mbmax()>1)

#define mbwidth(w)		(ast.mb_width?(*ast.mb_width)(w):1)
#define mbxfrm(t,f,n)		(mbcoll()?(*ast.mb_xfrm)((char*)(t),(char*)(f),n):0)
#define mbalpha(w)		(ast.mb_alpha?(*ast.mb_alpha)(w):isalpha((w)&0xff))

#define mbsrtowcs(w,s,n,q)	(*ast._ast_mbsrtowcs)((s),(w),(n),(mbstate_t*)(q))
#define wcsrtombs(s,w,n,q)	(*ast._ast_wcsrtombs)((s),(w),(n),(mbstate_t*)(q))

/* the converse does not always hold! */
#define utf32invalid(u)	((u)>0x0010FFFF||(u)>=0x0000D800&&(u)<=0x0000DFFF||(u)>=0xFFFE&&(u)<=0xFFFF)

#define UTF8_LEN_MAX		6	/* UTF-8 only uses 5 */

/*
 * common macros
 */

#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#define getconf(x)	strtol(astconf((x),NiL,NiL),NiL,0)
#define integralof(x)	(((char*)(x))-((char*)0))
#define newof(p,t,n,x)	((p)?(t*)realloc((char*)(p),sizeof(t)*(n)+(x)):(t*)calloc(1,sizeof(t)*(n)+(x)))
#define oldof(p,t,n,x)	((p)?(t*)realloc((char*)(p),sizeof(t)*(n)+(x)):(t*)malloc(sizeof(t)*(n)+(x)))
#define pointerof(x)	((void*)((char*)0+(x)))
#define roundof(x,y)	(((x)+(y)-1)&~((y)-1))
#define ssizeof(x)	((int)sizeof(x))

#define streq(a,b)	(*(a)==*(b)&&!strcmp(a,b))
#define strneq(a,b,n)	(*(a)==*(b)&&!strncmp(a,b,n))
#define strsignal(s)	fmtsignal(s)

#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NiL		0
#define NoP(x)		(void)(x)
#else
#define NiL		((char*)0)
#define NoP(x)		(&x,1)
#endif

#if !defined(NoF)
#define NoF(x)		void _DATA_ ## x () {}
#if !defined(_DATA_)
#define _DATA_
#endif
#endif

#if !defined(NoN)
#define NoN(x)		void _STUB_ ## x () {}
#if !defined(_STUB_)
#define _STUB_
#endif
#endif

#define NOT_USED(x)	NoP(x)

typedef int (*Ast_confdisc_f)(const char*, const char*, const char*);
typedef int (*Strcmp_context_f)(const char*, const char*, void*);
typedef int (*Strcmp_f)(const char*, const char*);

#include <ast_errorf.h>

#ifndef _VMALLOC_H
struct Vmdisc_s;
#endif

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern char*		astgetconf(const char*, const char*, const char*, int, Error_f);
extern char*		astconf(const char*, const char*, const char*);
extern Ast_confdisc_f	astconfdisc(Ast_confdisc_f);
extern void		astconflist(Sfio_t*, const char*, int, const char*);
extern off_t		astcopy(int, int, off_t);
extern int		astlicense(char*, int, char*, char*, int, int, int);
extern int		astquery(int, const char*, ...);
extern void		astwinsize(int, int*, int*);

extern ssize_t		base64encode(const void*, size_t, void**, void*, size_t, void**);
extern ssize_t		base64decode(const void*, size_t, void**, void*, size_t, void**);
extern int		chresc(const char*, char**);
extern int		chrexp(const char*, char**, int*, int);
extern int		chrtoi(const char*);
extern char*		conformance(const char*, size_t);
extern ssize_t		debug_printf(int, const char*, ...);
extern int		eaccess(const char*, int);
extern char*		fmtbase(intmax_t, int, int);
#define fmtbasell(a,b,c) fmtbase(a,b,c) /* until 2014-01-01 */
extern char*		fmtbuf(size_t);
extern char*		fmtclock(Sfulong_t);
extern char*		fmtelapsed(unsigned long, int);
extern char*		fmterror(int);
extern char*		fmtesc(const char*);
extern char*		fmtesq(const char*, const char*);
extern char*		fmtident(const char*);
extern char*		fmtip4(uint32_t, int);
extern char*		fmtfmt(const char*);
extern char*		fmtgid(int);
extern char*		fmtint(intmax_t, int);
extern char*		fmtmatch(const char*);
extern char*		fmtmode(int, int);
extern char*		fmtnesq(const char*, const char*, size_t);
extern char*		fmtnum(unsigned long, int);
extern char*		fmtperm(int);
extern char*		fmtquote(const char*, const char*, const char*, size_t, int);
extern char*		fmtre(const char*);
extern char*		fmtscale(Sfulong_t, int);
extern char*		fmtsignal(int);
extern char*		fmttime(const char*, time_t);
extern char*		fmtuid(int);
extern char*		fmtversion(unsigned long);
extern void*		memdup(const void*, size_t);
extern void		memfatal(void);
extern int		memfatal_20130509(struct Vmdisc_s*);
extern unsigned int	memhash(const void*, int);
extern unsigned long	memsum(const void*, int, unsigned long);
extern char*		pathaccess(char*, const char*, const char*, const char*, int);
extern char*		pathaccess_20100601(const char*, const char*, const char*, int, char*, size_t);
extern char*		pathbin(void);
extern char*		pathcanon(char*, int);
extern char*		pathcanon_20100601(char*, size_t, int);
extern char*		pathcat(char*, const char*, int, const char*, const char*);
extern char*		pathcat_20100601(const char*, int, const char*, const char*, char*, size_t);
extern int		pathcd(const char*, const char*);
extern int		pathcheck(const char*, const char*, Pathcheck_t*);
extern char*		pathdev(int, const char*, char*, size_t, int, Pathdev_t*);
extern int		pathexists(char*, int);
extern char*		pathfind(const char*, const char*, const char*, char*, size_t);
extern int		pathgetlink(const char*, char*, int);
extern int		pathinclude(const char*);
extern char*		pathkey(char*, char*, const char*, const char*, const char*);
extern char*		pathkey_20100601(const char*, const char*, const char*, char*, size_t, char*, size_t);
extern size_t		pathnative(const char*, char*, size_t);
extern int		pathopen(int, const char*, char*, size_t, int, int, mode_t);
extern char*		pathpath(char*, const char*, const char*, int);
extern char*		pathpath_20100601(const char*, const char*, int, char*, size_t);
extern size_t		pathposix(const char*, char*, size_t);
extern char*		pathprobe(char*, char*, const char*, const char*, const char*, int);
extern char*		pathprobe_20100601(const char*, const char*, const char*, int, char*, size_t, char*, size_t);
extern size_t		pathprog(const char*, char*, size_t);
extern char*		pathrepl(char*, const char*, const char*);
extern char*		pathrepl_20100601(char*, size_t, const char*, const char*);
extern int		pathsetlink(const char*, const char*);
extern char*		pathshell(void);
extern char*		pathtemp(char*, size_t, const char*, const char*, int*);
extern char*		pathtmp(char*, const char*, const char*, int*);
extern ssize_t		qpencode(const void*, size_t, void**, void*, size_t, void**);
extern ssize_t		qpdecode(const void*, size_t, void**, void*, size_t, void**);
extern char*		setenviron(const char*);
extern int		stracmp(const char*, const char*);
extern char*		strcopy(char*, const char*);
extern unsigned long	strelapsed(const char*, char**, int);
extern int		stresc(char*);
extern int		strexp(char*, int);
extern long		streval(const char*, char**, long(*)(const char*, char**));
extern long		strexpr(const char*, char**, long(*)(const char*, char**, void*), void*);
extern int		strgid(const char*);
extern int		strgrpmatch(const char*, const char*, int*, int, int);
extern int		strgrpmatch_20120528(const char*, const char*, ssize_t*, int, int);
extern int		strngrpmatch(const char*, size_t, const char*, ssize_t*, int, int);
extern unsigned int	strhash(const char*);
extern void*		strlook(const void*, size_t, const char*);
extern int		strmatch(const char*, const char*);
extern int		strmode(const char*);
extern int		strnacmp(const char*, const char*, size_t);
extern char*		strncopy(char*, const char*, size_t);
extern int		strnpcmp(const char*, const char*, size_t);
extern double		strntod(const char*, size_t, char**);
extern _ast_fltmax_t	strntold(const char*, size_t, char**);
extern long		strntol(const char*, size_t, char**, int);
extern intmax_t		strntoll(const char*, size_t, char**, int);
extern long		strnton(const char*, size_t, char**, char*, int);
extern unsigned long	strntoul(const char*, size_t, char**, int);
extern intmax_t		strntonll(const char*, size_t, char**, char*, int);
extern uintmax_t	strntoull(const char*, size_t, char**, int);
extern int		strnvcmp(const char*, const char*, size_t);
extern int		stropt(const char*, const void*, int, int(*)(void*, const void*, int, const char*), void*);
extern int		strpcmp(const char*, const char*);
extern int		strperm(const char*, char**, int);
extern void*		strpsearch(const void*, size_t, size_t, const char*, char**);
extern void*		strsearch(const void*, size_t, size_t, Strcmp_f, const char*, void*);
extern void		strsort(char**, int, Strcmp_f);
extern void		strsort_r(char**, size_t, Strcmp_context_f, void*);
extern char*		strsubmatch(const char*, const char*, int);
extern unsigned long	strsum(const char*, unsigned long);
extern char*		strtape(const char*, char**);
extern int		strtoip4(const char*, char**, uint32_t*, unsigned char*);
extern long		strton(const char*, char**, char*, int);
extern intmax_t		strtonll(const char*, char**, char*, int);
extern int		struid(const char*);
extern int		struniq(char**, int);
extern int		strvcmp(const char*, const char*);

extern size_t		utf32toutf8(char*, uint32_t);
extern size_t		utf8toutf32(uint32_t*, const char*, size_t);
extern size_t		utf8toutf32v(uint32_t*, const char*);
extern size_t		utf8towc(wchar_t*, const char*, size_t);

extern ssize_t		utf32stowcs(wchar_t*, uint32_t*, size_t);
extern ssize_t		wcstoutf32s(uint32_t*, wchar_t*, size_t);

extern size_t		ast_mbrchar(wchar_t*, const char*, size_t, Mbstate_t*);

#undef			extern

/*
 * C library global data symbols not prototyped by <unistd.h>
 */

#if !defined(environ) && defined(__DYNAMIC__)
#define	environ		__DYNAMIC__(environ)
#else
extern char**		environ;
#endif

#include <ast_debug.h>

#include <ast_api.h>

/* api specific mb/wc macros */

#if ASTAPI(20130913)

#define mbsinit(q)		(*(q)=ast._ast_mbstate_init.mb_state)
#define mbinit(q)		(*(q)=ast._ast_mbstate_init)
#define mberrno(q)		((q)->mb_errno)
#define mbsize(s,n,q)		(*ast._ast_mbrlen)((char*)(s),(n),(mbstate_t*)(q))
#define mbchar(w,s,n,q)		(((s)+=(ast_mbrchar)((wchar_t*)(w),(char*)(s),(n),(q))),(*(w)))
#define mbconv(s,w,q)		(*ast._ast_wcrtomb)((s),(w),(mbstate_t*)(q))

#define mbtinit(q)		(mbwide()?(mbinit(q),0):0)
#define mbtsize(s,n,q)		(mbwide()?mbsize((s),(n),(q)):(!!*(s)))
#define mbtchar(w,s,n,q)	(mbwide()?mbchar((w),(s),(n),(q)):(*(unsigned char*)(s++)))
#define mbtconv(s,w,q)		(mbwide()?mbconv((s),(w),(q)):((*(s)=(w)),1))

#else

#define mb2wc(w,p,n)		(*ast.mb_towc)(&(w),(char*)(p),(n))
#define mbchar(p)		(mbwide()?((ast.tmp_int=(*ast.mb_towc)(&ast.tmp_wchar,(char*)(p),mbmax()))>0?((p+=ast.tmp_int),ast.tmp_wchar):(p+=ast.mb_sync+1,ast.tmp_int)):(*(unsigned char*)(p++)))
#define mbnchar(p,n)		(mbwide()?((ast.tmp_int=(*ast.mb_towc)(&ast.tmp_wchar,(char*)(p),n))>0?((p+=ast.tmp_int),ast.tmp_wchar):(p+=ast.mb_sync+1,ast.tmp_int)):(*(unsigned char*)(p++)))
#define mbinit()		(mbwide()?(*ast.mb_towc)((wchar_t*)0,(char*)0,mbmax()):0)
#define mbsize(p)		(mbwide()?(*ast.mb_len)((char*)(p),mbmax()):((p),1))
#define mbnsize(p,n)		(mbwide()?(*ast.mb_len)((char*)(p),n):((p),1))
#define mbconv(s,w)		(ast.mb_conv?(*ast.mb_conv)(s,w):((*(s)=(w)),1))

#endif

/* generic plugin version support */

#undef	AST_PLUGIN_VERSION
#define AST_PLUGIN_VERSION(v)	((v)>AST_VERSION?(v):AST_VERSION)

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern unsigned long	plugin_version(void);

#undef	extern

#endif
