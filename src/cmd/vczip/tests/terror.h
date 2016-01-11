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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	<ast_common.h>

#ifndef NIL
#define NIL(t)	((t)0)
#endif

#if __STD_C
#include	<stdarg.h>
#else
#include	<varargs.h>
#endif

_BEGIN_EXTERNS_

#if !_SFIO_H
extern int	sprintf _ARG_((char*, const char*, ...));
extern int	vsprintf _ARG_((char*, const char*, va_list));
#endif

extern int	atexit _ARG_((void (*)(void)));
extern void	exit _ARG_((int));
extern size_t	strlen _ARG_((const char*));
extern Void_t*	malloc _ARG_((size_t));
extern char*	getenv _ARG_((const char*));

extern int	strncmp _ARG_((const char*, const char*, size_t));
extern int	strcmp _ARG_((const char*, const char*));
extern int	system _ARG_((const char*));

#if !_hdr_unistd
extern int	alarm _ARG_((int));
extern int	sleep _ARG_((int));
extern int	fork();
extern int	wait _ARG_((int*));
extern int	access _ARG_((const char*, int));
extern int	write _ARG_((int, const void*, int));
extern int	unlink _ARG_((const char*));
extern Void_t*	sbrk _ARG_((int));
extern int	getpid();
#endif

extern void	tsterror _ARG_((char*, ...));
extern void	tstwarn _ARG_((char*, ...));
extern void	tstsuccess _ARG_((char*, ...));

_END_EXTERNS_

static int		Tstline;
static char		Tstfile[16][256];

#ifdef __LINE__
#define terror		(Tstline=__LINE__),tsterror
#else
#define terror		(Tstline=-1),tsterror
#endif

#ifdef __LINE__
#define twarn		(Tstline=__LINE__),tstwarn
#else
#define twarn		(Tstline=-1),tstwarn
#endif

#ifdef __LINE__
#define tsuccess	(Tstline=__LINE__),tstsuccess
#else
#define tsuccess	(Tstline=-1),tstsuccess
#endif

#define tmesg		(Tstline=-1),tstwarn

#ifdef DEBUG
#ifdef __LINE__
#define TSTDEBUG(x)	(Tstline=__LINE__),tstwarn x
#else
#define TSTDEBUG(x)	(Tstline=-1),tstwarn x
#endif
#else
#define TSTDEBUG(x)
#endif

#ifndef MAIN
#define MAIN()		int main()
#endif /*MAIN*/

#ifndef TSTEXIT
#define TSTEXIT(v)	{ tstcleanup(); exit(v); }
#endif

static void tstcleanup()
{
#ifdef DEBUG
	twarn("Temp files will not be removed");
#else
	int	i;
	for(i = 0; i < sizeof(Tstfile)/sizeof(Tstfile[0]); ++i)
		if(Tstfile[i][0])
			unlink(Tstfile[i]);
#endif
}

#if __STD_C
static void tstputmesg(int line, char* form, va_list args)
#else
static void tstputmesg(line, form, args)
int	line;
char*	form;
va_list	args;
#endif
{
	char	*s, buf[1024];
	int	n;

	for(n = 0; n < sizeof(buf); ++n)
		buf[n] = 0;

	s = buf; n = 0;
	if(line >= 0)
	{
#if _SFIO_H
		sfsprintf(s, sizeof(buf), "\tLine=%d: ", line);
#else
		sprintf(s, "\tLine=%d: ", line);
#endif
		s += (n = strlen(s));
	}
#if _SFIO_H
	sfvsprintf(s, sizeof(buf)-n, form, args);
#else
	vsprintf(s, form, args);
#endif

	if((n = strlen(buf)) > 0)
	{	if(buf[n-1] != '\n')
		{	buf[n] = '\n';
			n += 1;
		}
		write(2,buf,n);
	}
}


#if __STD_C
void tsterror(char* form, ...)
#else
void tsterror(va_alist)
va_dcl
#endif
{
	char	failform[1024];

	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

#if _SFIO_H
	sfsprintf(failform, sizeof(failform), "Failure: %s", form);
#else
	sprintf(failform, "Failure: %s", form);
#endif

	tstputmesg(Tstline,failform,args);

	va_end(args);

	tstcleanup();
	exit(1);
}


#if __STD_C
void tstsuccess(char* form, ...)
#else
void tstsuccess(va_alist)
va_dcl
#endif
{
	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

	tstputmesg(Tstline,form,args);

	va_end(args);

	tstcleanup();
	exit(0);
}


#if __STD_C
void tstwarn(char* form, ...)
#else
void tstwarn(va_alist)
va_dcl
#endif
{
#if TEST_VERBOSE
	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

	tstputmesg(Tstline,form,args);

	va_end(args);
#endif
}


#if __STD_C
static char* tstfile(int n)
#else
static char* tstfile(n)
int	n;
#endif
{
	static int	Setatexit = 0;
	
	if(!Setatexit)
	{	Setatexit = 1;
		atexit(tstcleanup);
	}

	if(n >= sizeof(Tstfile)/sizeof(Tstfile[0]))
		terror("Bad temporary file request:%d\n", n);

	if(!Tstfile[n][0])
	{
#ifdef DEBUG
#if _SFIO_H
		sfsprintf(Tstfile[n], sizeof(Tstfile[0]), "Tstfile.%c%c%c", '0'+n, '0'+n, '0'+n);
#else
		sprintf(Tstfile[n], "Tstfile.%c%c%c", '0'+n, '0'+n, '0'+n);
#endif
#else
		static int	pid;
		static char*	tmp;
		if (!tmp)
		{	if (!(tmp = (char*)getenv("TMPDIR")) || access(tmp, 0) != 0)
				tmp = "/tmp";
			pid = (int)getpid() % 10000;
                }
#if _SFIO_H
                sfsprintf(Tstfile[n], sizeof(Tstfile[0]), "%s/sft.%c.%d", tmp, '0'+n, pid);
#else
                sprintf(Tstfile[n], "%s/sft.%c.%d", tmp, '0'+n, pid);
#endif
#endif
	}

	return Tstfile[n];
}

unsigned int trandom()
{
	static unsigned int rand = 0xdeadbeef;
	rand = rand*17109811 + 751;
	return rand;
}
