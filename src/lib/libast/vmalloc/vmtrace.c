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

void _STUB_vmtrace(){}

#else

#include	"vmhdr.h"

/*	Turn on tracing for regions
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94.
*/

static int	Trfile = -1;
static int	Pid = -1;

/* generate a trace of some call */
#if __STD_C
static void trtrace(Vmalloc_t* vm,
		    Vmuchar_t* oldaddr, Vmuchar_t* newaddr, size_t size, size_t align )
#else
static void trtrace(vm, oldaddr, newaddr, size, align)
Vmalloc_t*	vm;		/* region call was made from	*/
Vmuchar_t*	oldaddr;	/* old data address		*/
Vmuchar_t*	newaddr;	/* new data address		*/
size_t		size;		/* size of piece		*/
size_t		align;		/* alignment			*/
#endif
{
	char		buf[1024], *bufp, *endbuf;
	Vmdata_t*	vd = vm->data;
	const char*	file = 0;
	int		line = 0;
	const char*	func = 0;
	int		comma;
	int		n, m, type;
	unsigned int	threadid = asothreadid();
#define SLOP	64

	if(oldaddr == (Vmuchar_t*)(-1)) /* printing busy blocks */
	{	type = 0;
		oldaddr = NIL(Vmuchar_t*);
	}
	else
	{	type = vd->mode&VM_METHODS;
		VMFLF(vm,file,line,func);
	}

	if(Trfile < 0)
		return;

	if(Pid < 0)
		Pid = getpid();

	bufp = buf; endbuf = buf+sizeof(buf);
	bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(oldaddr ? VMLONG(oldaddr) : 0L, 0), ':');
	bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(newaddr ? VMLONG(newaddr) : 0L, 0), ':');
	bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)((Vmulong_t)size, 1), ':');
	bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)((Vmulong_t)align, 1), ':');
	bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(vm), 0), ':');
	if(type&VM_MTBEST)
		bufp = (*_Vmstrcpy)(bufp, "b", ':');
	else if(type&VM_MTLAST)
		bufp = (*_Vmstrcpy)(bufp, "l", ':');
	else if(type&VM_MTPOOL)
		bufp = (*_Vmstrcpy)(bufp, "p", ':');
	else if(type&VM_MTDEBUG)
		bufp = (*_Vmstrcpy)(bufp, "d", ':');
	else	bufp = (*_Vmstrcpy)(bufp, "u", ':');

	comma = 0;
	if(file && file[0] && line > 0)
	{	if((bufp + strlen(file) + SLOP) >= endbuf)
		{	char*	f;
			for(f = bufp + strlen(file); f > file; --f)
				if(f[-1] == '/' || f[-1] == '\\')
					break; 
			file = f;
		}

		bufp = (*_Vmstrcpy)(bufp, "file", '=');
		n = endbuf - bufp - SLOP - 3;
		m = strlen(file);
		if(m > n)
		{	file += (m - n);
			bufp = (*_Vmstrcpy)(bufp, "..", '.');
		}
		bufp = (*_Vmstrcpy)(bufp, file, ',');
		bufp = (*_Vmstrcpy)(bufp, "line", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)((Vmulong_t)line,1), 0);
		comma = 1;
	}
	if(func)
	{	if(comma)
			*bufp++ = ',';
		bufp = (*_Vmstrcpy)(bufp, "func", '=');
#if 1
		bufp = (*_Vmstrcpy)(bufp, (const char*)func, 0);
#else
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)((Vmulong_t)func,0), 0);
#endif
		comma = 1;
	}
	if(threadid)
	{	if(comma)
			*bufp++ = ',';
		bufp = (*_Vmstrcpy)(bufp, "tid", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)((Vmulong_t)threadid, 1), 0);
		comma = 1;
	}
	if(Pid >= 0)
	{	if(comma)
			*bufp++ = ',';
		bufp = (*_Vmstrcpy)(bufp, "pid", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)((Vmulong_t)Pid, 1), 0);
		comma = 1;
	}
	if(comma)
		*bufp++ = ':';

	*bufp++ = '\n';
	*bufp = '\0';

	write(Trfile,buf,(bufp-buf));
}

#if __STD_C
void _vmmessage(const char* s1, long n1, const char* s2, long n2)
#else
void _vmmessage(s1, n1, s2, n2)
const char*	s1;
long		n1;
const char*	s2;
long		n2;
#endif
{
	char		buf[1024], *bufp;

	bufp = buf;
	bufp = (*_Vmstrcpy)(bufp, "vmalloc", ':');
	if (s1)
	{
		bufp = (*_Vmstrcpy)(bufp, s1, ':');
		if (n1)
			bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(n1, 1), ':');
	}
	if (s2)
	{
		bufp = (*_Vmstrcpy)(bufp, s2, ':');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(n2, 0), ':');
	}

	bufp = (*_Vmstrcpy)(bufp, "pid", '=');
	bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)((long)getpid(), 1), ':');

	*bufp++ = '\n';
	write(2,buf,(bufp-buf));
}

#if __STD_C
int vmtrace(int file)
#else
int vmtrace(file)
int	file;
#endif
{
	int	fd;

	_Vmtrace = file >= 0 ? trtrace : 0;

	fd = Trfile;
	Trfile = file;

	return fd;
}

#endif
