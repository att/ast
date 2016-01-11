/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * 3d system call trace
 */

#define SYSTRACE3D	1

#include "3d.h"

#if SYSTRACE3D

#define SYSCALL		syscall3d

#include "dir_3d.h"

#ifndef D_FILENO
#define D_FILENO(d)	(1)
#endif
#ifndef D_TYPE
#define D_TYPE(d)	(0)
#endif

#define C_EXIT		(MSG_MASK(MSG_exit))
#define C_IO		(MSG_MASK(MSG_read)|MSG_MASK(MSG_write))
#define C_ZERO		(MSG_MASK(MSG_pipe))

typedef union
{
	void*		pointer;
	int		number;
	char*		string;
	char**		vector;
} ARG;

static void*	dll;

#if _lib_strerror

#undef	strerror	/* otherwise it's _ast_strerror */

#else

extern int	sys_nerr;
extern char*	sys_errlist[];

char*
strerror(int err)
{
	static char	msg[28];

	if (err > 0 && err <= sys_nerr)
		return sys_errlist[err];
	sfsprintf(msg, sizeof(msg), "Error %d", err);
	return msg;
}

#endif

#define MAXBUF	128
#define MAXLIN	79
#define MAXOUT	2048

static void
buffer(char** buf, char* end, register char* s, int n)
{
	register char*	b;
	register char*	be;
	register char*	se;
	register int	c;

	if (n < 0)
		bprintf(buf, end, " %p", s);
	else
	{
		b = *buf;
		be = end;
		if (be - b > MAXBUF)
			be = b + MAXBUF;
		be -= 5;
		be = be;
		se = s + n;
		if (b < be - 1)
		{
			*b++ = ' ';
			*b++ = '"';
			while (b < be && s < se)
			{
				if ((c = *((unsigned char*)s++)) < 040)
				{
					*b++ = '\\';
					switch (c)
					{
					case '\007':
						c = 'a';
						break;
					case '\b':
						c = 'b';
						break;
					case '\f':
						c = 'f';
						break;
					case '\n':
						c = 'n';
						break;
					case '\r':
						c = 'r';
						break;
					case '\t':
						c = 't';
						break;
					case '\013':
						c = 'v';
						break;
					case '\033':
						c = 'E';
						break;
					default:
						if (b < be - 3)
						{
							if (n = s < se && c >= '0' && c <= '9')
								*b++ = '0';
							if (n || ((c >> 3) & 07))
								*b++ = '0' + ((c >> 3) & 07);
							*b++ = '0' + (c & 07);
						}
						continue;
					}
					if (b < be)
						*b++ = c;
				}
				else if (c < 0177)
					*b++ = c;
				else if (c == 0177)
				{
					*b++ = '^';
					if (b >= be)
						break;
					*b++ = '?';
				}
				else if (b < be - 4)
				{
					*b++ = '\\';
					*b++ = '0' + ((c >> 6) & 03);
					*b++ = '0' + ((c >> 3) & 07);
					*b++ = '0' + (c & 07);
				}
			}
			if ((b >= be || s < se) && (be - *buf) >= 4)
			{
				b -= 4;
				*b++ = '"';
				*b++ = '.';
				*b++ = '.';
				*b++ = '.';
			}
			else if (b < be)
				*b++ = '"';
		}
		*buf = b;
	}
}

#if __gnu_linux__
#undef	_no_exit_exit
#endif

#if _lib_syscall && _sys_syscall

#include <sys/syscall.h>

#ifdef SYS_exit

static void
sys_exit(int code)
{
	syscall(SYS_exit, code);
}

#define SYS_EXIT	sys_exit

#endif

#endif

/*
 * initialize the 3d syscall table
 */

void
callinit(void)
{
	register Systrace_t*	cp;

#if defined(__sgi) && defined(_ABIO32)
	sys_trace[0].name = "_exit";
#endif
#if sun && !_sun && _lib_on_exit
	sys_trace[0].name = "__exit";
#endif
	if (dll = dllnext(RTLD_LAZY))
	{
		for (cp = sys_trace; cp < &sys_trace[elementsof(sys_trace)]; cp++)
			if (!(cp->func = (Sysfunc_t)dlsym(dll, cp->name)) && (*cp->name != '_' || !(cp->func = (Sysfunc_t)dlsym(dll, cp->name + 1)) || !*cp->name++))
				cp->func = (Sysfunc_t)nosys;
#if !defined(SYS_EXIT) && _no_exit_exit
		state.libexit = (Exitfunc_t)dlsym(dll, "exit");
#endif
	}
#ifdef SYS_EXIT
	sys_trace[0].func = (Sysfunc_t)sys_exit;
#endif
}

/*
 * dump the 3d syscall table
 */

void
calldump(char** b, char* e)
{
	register Systrace_t*	cp;
	register int		m;

	bprintf(b, e, "\nsystem calls total=%d nosys=%p exit=%p\n\n", elementsof(sys_trace), nosys,
#if _no_exit_exit
		state.libexit
#else
		(Sysfunc_t)0
#endif
		);
	for (cp = sys_trace; cp < &sys_trace[elementsof(sys_trace)]; cp++)
	{
		bprintf(b, e, "  %03d%s %03d %12s", cp - sys_trace, (cp - sys_trace) == cp->index ? " " : "*", cp->call, cp->name);
		for (m = state.trap.size - 1; m >= 0; m--)
			if (MSG_MASK(cp->call) & state.trap.intercept[m].mask)
				bprintf(b, e, " %p[%d]", state.trap.intercept[m].call, m);
		bprintf(b, e, " %p\n", cp->func);
	}
}

#if _no_exit_exit
static int
oksys(void)
{
	return 0;
}

void
exit(int code)
{
	if (state.libexit)
		sys_trace[SYS3D_exit].func = (Sysfunc_t)oksys;
	_exit(code);
	if (state.libexit)
	{
		(*state.libexit)(code);
		state.libexit = 0;
	}
}
#endif

Sysfunc_t
sysfunc(int call)
{
	initialize();
	return sys_trace[call].func;
}

#if !_var_syscall
long
syscall3d(int call, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6)
{
#if 0 /* to convince proto */
}
#endif
#else
long
syscall3d(int call, ...)
{
#endif
	register int		n;
	register long		r;
	register Systrace_t*	cp;
	register ARG*		ap;
	register int		ac;
	int			a;
	int			m;
	int			on;
	char*			b;
	char*			e;
	char*			t;
	char**			p;
	int*			ip;
	ARG			arg[7];
	char			buf[MAXOUT];
	Sysfunc_t		func;
#if _var_syscall
	va_list			vp;
#endif

	initialize();
#if _var_syscall
	va_start(vp, call);
#endif
	cp = sys_trace + call;
#if _var_syscall
	n = cp->args;
	for (r = 1; r <= elementsof(arg); r++)
		arg[r].pointer = (r <= n) ? va_arg(vp, void*) : (void*)0;
	va_end(vp);
#else
	switch (cp->args)
	{
	case 6: arg[6].pointer = a6;
	case 5: arg[5].pointer = a5;
	case 4: arg[4].pointer = a4;
	case 3: arg[3].pointer = a3;
	case 2: arg[2].pointer = a2;
	case 1: arg[1].pointer = a1;
	}
#endif
	if (state.kernel || state.trace.pid <= 1 || (on = fsfd(&state.fs[FS_option])) <= 0 || !(state.test & 0100) && !(MSG_MASK(cp->call) & (state.trace.call & ~MSG_MASK(error_info.trace ? 0 : MSG_nop))))
		on = 0;
	else
	{
		state.kernel++;
		if (!state.trace.count)
		{
			e = (b = buf) + elementsof(buf) - 1;
			if (state.trace.pid > 2)
				bprintf(&b, e, "[%d] ", state.trace.pid);
			bprintf(&b, e, "%s (", cp->name);
			a = A_INPUT;
			if (call == SYS3D_write)
			{
				if ((m = arg[1].number) == on)
					a = 0;
				else if (m == 1 || m == 2)
				{
					struct stat	st;

					n = errno;
					if (!fstat(m, &st) && st.st_ino == state.fs[FS_option].st.st_ino && st.st_dev == state.fs[FS_option].st.st_dev)
						a = 0;
					errno = n;
				}
			}
			if ((state.test & 020) && call == SYS3D_close) bprintf(&b, e, "%s%s%s", state.file[arg[1].number].reserved ? " [RESERVED]" : "", arg[1].number == TABLE_FD ? " [TABLE]" : "", arg[1].number == state.table.fd ? " [table]" : "");
			for (ac = 1; ac <= cp->args && (n = cp->type[ac]) >= a; ac++)
			{
				ap = &arg[ac];
				switch (n)
				{
				case A_INPUT:
					if (a)
					{
						buffer(&b, e, ap->string, arg[ac + 1].number);
						break;
					}
					/*FALLTHROUGH*/
				case A_OUTPUT:
				case A_POINTER:
					bprintf(&b, e, " %p", ap->number);
					break;
				case A_MODE:
					bprintf(&b, e, " 0%o", ap->number);
					break;
				case A_STRING:
					if (t = ap->string)
						buffer(&b, e, t, strlen(t));
					else
						bprintf(&b, e, " (null)");
					break;
				case A_VECTOR:
					bprintf(&b, e, " [");
					for (p = ap->vector; *p && p < ap->vector + 8; p++)
						buffer(&b, e, *p, strlen(*p));
					if (*p)
						bprintf(&b, e, "...");
					bprintf(&b, e, " ]");
					break;
				default:
					bprintf(&b, e, " %d", ap->number);
					break;
				}
			}
			if (!a)
				*b++ = '\n';
			else if (MSG_MASK(cp->call) & C_EXIT)
			{
				bprintf(&b, e, " ) = ?\n");
				state.kernel++;
			}
			n = errno;
			write(on, buf, b - buf);
			errno = n;
		}
		else
		{
			if (MSG_MASK(cp->call) & C_EXIT)
			{
				e = (b = buf) + elementsof(buf) - 1;
				*b++ = '\n';
				if (state.trace.pid > 2)
					bprintf(&b, e, "         [%d] %s\n", state.trace.pid, state.cmd);
				for (n  = 0; n < elementsof(sys_trace); n++)
					if (sys_trace[n].count)
					{
						if (MSG_MASK(sys_trace[n].call) & C_IO)
						{
							bprintf(&b, e, "   %5d %-10s", sys_trace[n].count, sys_trace[n].name);
							if (sys_trace[n].megs)
								bprintf(&b, e, "%5lu.%dm", sys_trace[n].megs, (sys_trace[n].units * 10) >> 20);
							else
								bprintf(&b, e, "%5lu.%dk", sys_trace[n].units >> 10, ((sys_trace[n].units & ((1<<10)-1)) * 10) >> 10);
						}
						else
							bprintf(&b, e, "   %5d %s", sys_trace[n].count, sys_trace[n].name);
						if (b < e)
							*b++ = '\n';
					}
				*b++ = '\n';
				n = errno;
				write(on, buf, b - buf);
				errno = n;
				state.kernel++;
			}
			cp->count++;
		}
	}
	for (m = state.trap.size - 1; m >= 0; m--)
		if (MSG_MASK(cp->call) & state.trap.intercept[m].mask)
			break;
	if (m >= 0)
	{
		n = state.trap.size;
		state.trap.size = m;
		r = (*state.trap.intercept[m].call)(&state.trap.intercept[m], cp->call, call, arg[1].pointer, arg[2].pointer, arg[3].pointer, arg[4].pointer, arg[5].pointer, arg[6].pointer);
		state.trap.size = n;
	}
	else
	{
#if _dynamic_syscall || _static_syscall
#if _dynamic_syscall
		if (dll && cp->func)
#else
		if (dll && cp->func && cp->index < 0)
#endif
		{
			switch (cp->active++)
			{
			case 0:
				func = cp->func;
				break;
			case 1:
				if (!(func = cp->last))
				{
					if (!(cp->last = (Sysfunc_t)dlsym(dll, cp->name)) && (*cp->name != '_' || !(cp->last = (Sysfunc_t)dlsym(dll, cp->name + 1)) || !*cp->name++))
						cp->last = (Sysfunc_t)nosys;
					func = cp->last;
					if (func == cp->func)
					{
						/*
						 * longjmp gets you here
						 */

						cp->active = 3;
					}
					else if (cp->func != (Sysfunc_t)nosys && func == (Sysfunc_t)nosys)
					{
						cp->active = 10;
						e = (b = buf) + elementsof(buf) - 1;
						bprintf(&b, e, "3d: %s: system call loop -- cannot determine the real system call\n", cp->name);
						write(2, buf, b - buf);
					}
				}
				break;
			case 2:
			case 3:
				cp->active = 3;
				func = cp->func;
				break;
			default:
				cp->active = 10;
				func = (Sysfunc_t)nosys;
				break;
			}
			r = (*func)(arg[1].pointer, arg[2].pointer, arg[3].pointer, arg[4].pointer, arg[5].pointer, arg[6].pointer);
			cp->active--;
		}
		else
#endif
#if _lib_syscall
		if (cp->index >= 0)
			r = syscall(cp->index, arg[1].pointer, arg[2].pointer, arg[3].pointer, arg[4].pointer, arg[5].pointer, arg[6].pointer);
		else if (cp->nov >= 0)
			r = syscall(cp->nov, arg[2].pointer, arg[3].pointer, arg[4].pointer, arg[5].pointer, arg[6].pointer, 0);
		else
#endif
		{
#ifndef ENOSYS
#define ENOSYS	EINVAL
#endif
			errno = ENOSYS;
			r = -1;
		}
	}
#if !_mangle_syscall
	if (r > 0 && (MSG_MASK(cp->call) & C_ZERO))
		r = 0;
#endif
	if (on && state.kernel <= 1)
	{
		if (!state.trace.count)
		{
			if ((m = MAXLIN - (b - buf)) < 0)
				m = 0;
			b = buf;
			for (; ac <= cp->args; ac++)
			{
				ap = &arg[ac];
				switch (n = cp->type[ac])
				{
				case A_OUTPUT:
					switch (call)
					{
					case SYS3D_fstat:
#ifdef SYS3D_lstat
					case SYS3D_lstat:
#endif
					case SYS3D_stat:
						if (!r)
						{
							/*UNDENT...*/
#ifdef _3D_STAT_VER
	switch (arg[1].number)
	{
	case _3D_STAT_VER:
#endif
		{
			struct stat*	sp = (struct stat*)ap->pointer;
			bprintf(&b, e, " [ dev=%d ino=%d view=%d mode=0%o nlink=%d uid=%d gid=%d size=%u atime=%u mtime=%u ctime=%u ]", sp->st_dev, sp->st_ino, iview(sp), sp->st_mode, sp->st_nlink, sp->st_uid, sp->st_gid, sp->st_size, sp->st_atime, sp->st_mtime, sp->st_ctime);
			continue;
		}
#ifdef _3D_STAT_VER
#ifdef _3D_STAT64_VER
	case _3D_STAT64_VER:
		{
			struct stat64*	sp = (struct stat64*)ap->pointer;
			bprintf(&b, e, " [ dev=%d ino=%lld view=%d mode=0%o nlink=%d uid=%d gid=%d size=%llu atime=%u mtime=%u ctime=%u ]", sp->st_dev, sp->st_ino, iview(sp), sp->st_mode, sp->st_nlink, sp->st_uid, sp->st_gid, sp->st_size, sp->st_atime, sp->st_mtime, sp->st_ctime);
			continue;
		}
#endif
	}
#endif
						}
						break;
#ifdef SYS3D_getdents
					case SYS3D_getdents:
						if (r > 0)
						{
							struct DIRdirent*	dp = (struct DIRdirent*)ap->pointer;
							struct DIRdirent*	de = (struct DIRdirent*)((char*)dp + r);

							bprintf(&b, e, " [");
							while (dp < de)
							{
#ifdef DIRdirent
								bprintf(&b, e, " %lu \"%s\"", D_FILENO(dp), dp->d_name);
#else
								bprintf(&b, e, " %02d %lu \"%s\"", D_TYPE(dp), D_FILENO(dp), dp->d_name);
#endif
								dp = (struct DIRdirent*)((char*)dp + dp->d_reclen);
							}
							bprintf(&b, e, " ]");
							continue;
						}
						break;
#endif
					case SYS3D_pipe:
						if (!r)
						{
							ip = (int*)ap->pointer;
							bprintf(&b, e, " [ %d %d ]", ip[0], ip[1]);
							continue;
						}
						break;
					}
					/*FALLTHROUGH*/
				case A_INPUT:
					if (n == A_OUTPUT && cp->type[n = 0] == A_SIZE || ac < (elementsof(cp->type) - 1) && cp->type[n = ac + 1] == A_SIZE)
					{
						buffer(&b, e, ap->string, n ? arg[n].number : r);
						break;
					}
					goto pointer;
				case A_MODE:
					bprintf(&b, e, " 0%o", ap->number);
					break;
				case A_POINTER:
				pointer:
					bprintf(&b, e, " %p", ap->pointer);
					break;
				case A_STRING:
					if (r == -1)
						goto pointer;
					buffer(&b, e, ap->string, strlen(ap->string));
					break;
				default:
					bprintf(&b, e, " %d", ap->number);
					break;
				}
			}
#if DEBUG_dirent
			switch (call)
			{
#ifdef SYS3D_readdir
#undef	DIRdirent
			case SYS3D_readdir:
				if (r && (state.test & 0100))
				{
					struct DIRdirent*	dp = (struct DIRdirent*)pointerof(r);

					bprintf(&b, e, " ) = [ %02d %lu \"%s\" ]", D_TYPE(dp), D_FILENO(dp), dp->d_name);
					break;
				}
				goto number;
#endif
#ifdef SYS3D_readdir64
			case SYS3D_readdir64:
				if (r && (state.test & 0100))
				{
					struct dirent64*	dp = (struct dirent64*)pointerof(r);

					bprintf(&b, e, " ) = [ %02d %llu \"%s\" ]", D_TYPE(dp), D_FILENO(dp), dp->d_name);
					break;
				}
				goto number;
#endif
			default:
			number:
				bprintf(&b, e, "%s) = %d", a ? " " : "\t", r);
				break;
			}
#else
			bprintf(&b, e, "%s) = %d", a ? " " : "\t", r);
#endif
			if (r == -1)
				bprintf(&b, e, " [%s]", strerror(errno));
			n = errno;
			t = buf;
			while ((b - t) >= m)
			{
				char*	w;
				char*	x;
				char*	z;
				int	c1;
				int	c2;

				x = w = t + m;
				z = t + m / 2;
				while (x > z && *x != ' ') x--;
				if (x <= z)
					x = w;
				c1 = *x;
				*x++ = '\n';
				c2 = *x;
				*x++ = '\t';
				write(on, t, x - t);
				*--x = c2;
				if ((*--x = c1) == ' ')
					x++;
				t = x;
				m = MAXLIN - 8;
			}
			*b++ = '\n';
			write(on, t, b - t);
			errno = n;
		}
		else if (r >= 0 && (MSG_MASK(cp->call) & C_IO) && (cp->units += r) >= (1<<20))
		{
			cp->megs += cp->units >> 20;
			cp->units &= ((1<<20)-1);
		}
		state.kernel--;
	}
	return r;
}

#else

NoN(syscall)

#endif
