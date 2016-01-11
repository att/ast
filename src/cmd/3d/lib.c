/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * 3d file system library implementation
 *
 * debug trace levels are controlled by /#option/debug
 * debug level n enables tracing for all levels less than or equal to n
 *
 *	level	trace
 *	  0	no trace
 *	  1	basic calls
 *	  2	mounted fs
 *	  3	fs control
 *	  4	pathcanon
 *	  5	pathreal
 *	  6	pathnext
 *	  7	*unused*
 *	  8	*unused*
 *	  9	*unused*
 *	 10	getkey
 */

#include "3d.h"

#include <cs.h>

/*
 * keep 3d tables from buf,siz in state.table.fd
 * if force!=0 then state.table.fd set to TABLE_FD
 * 0 returned if state.table.fd is ok
 */

int
keep(const char* buf, size_t siz, int force)
{
	int	n;

	if (force && state.in_2d || siz > sizeof(state.table.buf))
	{
		if (state.table.fd > 0)
			cancel(&state.table.fd);
		return(-1);
	}
	if (state.table.fd > 0)
	{
		char	tmp[sizeof(state.table.buf)];

		if (peek(state.table.fd, tmp, sizeof(tmp) - 1) == siz && !memcmp(tmp, buf, siz))
			return(0);
		cancel(&state.table.fd);
	}
#if _stream_peek || _socket_peek
	{
		int	fds[2];

		state.in_2d++;
#if _socket_peek
		n = cspipe(&cs, fds);
#else
		n = pipe(fds);
#endif
		state.in_2d--;
		if (!n && fds[1] == TABLE_FD)
		{
#if _pipe_rw || _lib_socketpair
			fds[1] = fds[0];
			fds[0] = TABLE_FD;
#else
			CLOSE(fds[0]);
			state.in_2d++;
			n = pipe(fds);
			state.in_2d--;
			CLOSE(TABLE_FD);
#endif
		}
		if (!n)
		{
			if (fds[0] == TABLE_FD) state.table.fd = fds[0];
			else
			{
				if (force) CLOSE(TABLE_FD);
				state.table.fd = FCNTL(fds[0], F_DUPFD, TABLE_FD);
				CLOSE(fds[0]);
			}
			n = WRITE(fds[1], buf, siz) == siz;
			CLOSE(fds[1]);
			if (n)
			{
				state.table.size = siz;
				reserve(&state.table.fd);
				return(0);
			}
		}
	}
#else
	sfsprintf(state.path.name, sizeof(state.path.name), "/tmp/3D#%d", state.pid);
	if ((n = OPEN(state.path.name, O_CREAT, S_IRUSR|S_IRGRP|S_IROTH)) >= 0)
	{
		UNLINK(state.path.name);
		if (force) CLOSE(TABLE_FD);
		if (n == TABLE_FD) state.table.fd = TABLE_FD;
		else
		{
			state.table.fd = FCNTL(n, F_DUPFD, TABLE_FD);
			CLOSE(n);
		}
		if (WRITE(state.table.fd, buf, siz) == siz && !LSEEK(state.table.fd, 0L, 0))
		{
			state.table.size = siz;
			reserve(&state.table.fd);
			return(0);
		}
	}
#endif
	if (state.table.fd > 0)
		cancel(&state.table.fd);
	return(-1);
}

/*
 * enable/disable/test 3d/anno state
 * allows state transitions without changing the tables
 */

int
fs3d(register int op)
{
	register int	po;
	register int	n;

	if ((n = FS3D_op(op)) == FS3D_OP_INIT || !state.pid)
		init(1, 0, 0);
	if (!state.in_2d)
		po = FS3D_ON;
	else if (state.limit != TABSIZE)
		po = FS3D_LIMIT(state.limit);
	else
		po = FS3D_OFF;
	switch (n)
	{
	case FS3D_OP_INIT:
	case FS3D_OP_TEST:
		break;
	case FS3D_OP_LIMIT:
		if ((n = FS3D_arg(op)) <= 0 || n > TABSIZE)
			n = TABSIZE;
		state.limit = n;
		state.in_2d = 0;
		break;
	case FS3D_OP_OFF:
		if (state.level > 0)
			po = -1;
		else
		{
			state.limit = TABSIZE;
			state.in_2d = 1;
		}
		break;
	case FS3D_OP_ON:
		state.limit = TABSIZE;
		state.in_2d = 0;
		break;
	default:
		po = -1;
		break;
	}
	message((-1, "fs3d(%d)=%d", op, po));
	return po;
}

/*
 * for code that includes <fs3d.h>
 */

int
fs3d_mount(const char* source, char* target, int flags, void* data)
{
	return mount(source, target, flags, data, 0, 0);
}

/*
 * return 6 char lower case hash of key
 * end is end of key string
 * 0 returned at end of keys
 * state.key.value is set to key value
 * if `no<key>' then state.key.invert gets value and state.key.value=""
 * otherwise state.key.invert=0
 * state.key.next is set to next key if any
 */

unsigned long
getkey(register const char* key, register const char* end, int alt)
{
	register const char*	val;
	register unsigned long	x = 0;
	register int		invert;
	register int		c;
#if DEBUG
	const char*		beg;
#endif

	if (key)
	{
#if DEBUG
		beg = key;
#endif
		if (key < end - 1 && *key == 'n' && *(key + 1) == 'o')
		{
			key += 2;
			invert = 1;
		}
		else invert = 0;
		if (key < end)
		{
			val = ((end - key) > HASHKEYMAX) ? key + HASHKEYMAX : end;
			while (key < end && (c = *key) != '=' && c != '/' && c != alt)
			{
				if (key < val)
					x = (c >= '0' && c <= '9') ? HASHKEYPART(x, HASHKEYN(c)) : HASHKEYPART(x, c);
				key++;
			}
			if (key < end && c == '=')
				key++;
		}
		if (key >= end || *key == '/' || *key == alt)
		{
			state.key.value = state.one;
			state.key.valsize = 1;
		}
		else
		{
			state.key.value = (char*)key;
			while (key < end && *key != '/' && *key != alt)
				key++;
			state.key.valsize = (char*)key - state.key.value;
		}
		while (key < end && (*key == '/' || *key == alt))
			key++;
		if (key < end && *key == '#' && *(key - 1) == '/')
			key++;
		if (invert)
		{
			state.key.invert = state.key.value;
			state.key.invertsize = state.key.valsize;
			state.key.value = state.null;
			state.key.valsize = 0;
		}
		else state.key.invert = 0;
		state.key.next = (key < end) ? (char*)key : (char*)0;
		message((-10, "getkey: key=%-*s hash=%x value=%-*s next=%-*s", end - beg, beg, x, state.key.valsize, state.key.value, state.key.next ? end - state.key.next : 6, state.key.next));
	}
	else state.key.next = 0;
	return(x);
}

#if FS

/*
 * return mount pointer for path
 * under is set to the part of the path under the mount
 */

Mount_t*
getmount(register const char* path, const char** under)
{
	register Map_t*	map;
	int		n;
	int		oerrno;
	struct stat	st;

	if (!(map = search(&state.vmount, path, strlen(path), NiL, T_PREFIX)) || ((Mount_t*)map->val)->fs == state.fs)
		return(0);
	if (under)
	{
		if (*(path += map->keysize)) path++;
		else
		{
			oerrno = errno;
			n = !STAT(path - map->keysize, &st) && !S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode);
			errno = oerrno;
			if (n) return(0);
			else path = (const char*)state.dot;
		}
		*under = path;
	}
	return((Mount_t*)map->val);
}

#endif

int
nosys(void)
{
	errno = EINVAL;
	return(-1);
}

/*
 * copy t into s, return a pointer to the end of s ('\0')
 */

char*
strcopy(register char* s, register const char* t)
{
	if (!t) return(s);
	while (*s++ = *t++);
	return(--s);
}

/*
 * return map for <key,keysize> in tab
 * if val!=0 then key=val inserted if not already entered
 * valsize==0 causes string val to be malloc'd on insertion
 * key is malloc'd on insertion if (valsize|T_ALLOCATE) 
 * valsize==T_DELETE for deletion
 * valsize==T_PREFIX for longest path prefix search and val==visit_mask
 */

Map_t*
search(Table_t* tab, const char* key, int keysize, const char* val, int valsize)
{
	register Map_t*		lo;
	register Map_t*		mid;
	register Map_t*		hi;
	register const char*	sp;
	register const char*	se;
	register const char*	tp;
	register const char*	te;
	int			n;
	Map_t*			end;

	se = key + keysize;
	lo = tab->table;
	end = lo + tab->size;
	if (valsize == T_PREFIX)
	{
		mid = 0;
		hi = end;
		while (lo < hi)
		{
			sp = key;
			tp = lo->key;
			te = tp + lo->keysize;
message((-12, "T_PREFIX: %-*s: key=%-*s mid=%-*s", keysize, key, lo->keysize, lo->key, mid ? mid->keysize : 1, mid ? mid->key : "-"));
			for (;;)
			{
				if (tp >= te)
				{
					if ((sp >= se || *sp == '/') && (!val || !(*((long*)val) & (1 << (lo - tab->table)))))
					{
						mid = lo;
						if (sp >= se)
							goto prefix;
					}
					break;
				}
				if (sp >= se || (n = *((unsigned char*)sp++) - *((unsigned char*)tp++)) < 0)
					goto prefix;
				if (n > 0)
				{
					if (mid && mid->keysize >= (sp - key))
						goto prefix;
					break;
				}
			}
			lo++;
		}
	prefix:
		if (mid && val) *((long*)val) |= (1 << (mid - tab->table));
		return(mid);
	}
	if (end > lo)
	{
		hi = end - 1;
		while (lo <= hi)
		{
			mid = lo + (hi - lo) / 2;
			sp = key;
			tp = mid->key;
			te = tp + mid->keysize;
			for (;;)
			{
				if (tp >= te)
				{
					if (sp >= se)
					{
						if (valsize != T_DELETE)
							return(mid);
						if (mid->valsize & T_ALLOCATE)
							free(mid->key);
#if FS
						if ((mid->valsize & T_SIZE) == T_MOUNT)
							((Mount_t*)mid->val)->fs = 0;
						else
#endif
						if (!(mid->valsize & T_SIZE))
							free(mid->val);
						for (; mid < end; mid++)
							*mid = *(mid + 1);
						tab->size--;
						return(0);
					}
					lo = mid + 1;
					break;
				}
				if (sp >= se || (n = *((unsigned char*)sp++) - *((unsigned char*)tp++)) < 0)
				{
					hi = mid - 1;
					break;
				}
				if (n > 0)
				{
					lo = mid + 1;
					break;
				}
			}
		}
	}
	if (!val || valsize == T_DELETE || tab->size >= elementsof(tab->table))
		return(0);
	tab->size++;
	for (hi = end; hi > lo; hi--)
		*hi = *(hi - 1);
	lo->keysize = keysize;
	if (valsize & T_ALLOCATE) lo->key = strcpy(newof(0, char, keysize, 1), key);
	else lo->key = (char*)key;
	if ((lo->valsize = valsize) & T_SIZE) lo->val = (char*)val;
	else lo->val = strcpy(newof(0, char, strlen(val), 1), val);
	return(lo);
}

/*
 * iterate fun over tab
 * terminates on first negative return from fun
 */

int
iterate(register Table_t* tab, int (*fun)(Map_t*, char*, int), register char* buf, int flags)
{
	register Map_t*	cp;
	register Map_t*	ep;
	register int	n;
	register int	sum;

	sum = 0;
	for (ep = (cp = tab->table) + tab->size; cp < ep; cp++)
	{
		if ((n = (*fun)(cp, buf, flags)) < 0)
			return(0);
		if (buf) buf += n;
		sum += n;
	}
	return(sum);
}

#if FS

/*
 * initialize open file info
 */

int
fileinit(int fd, struct stat* st, Mount_t* mp, int force)
{
	register File_t*	f;
	int			ffd;
	int			ffl;
	struct stat		sb;

	f = state.file + fd;
	if (!force && (f->flags & FILE_ERROR) || !st && FSTAT(fd, st = &sb) || (ffd = FCNTL(fd, F_GETFD, NiL)) == -1 || (ffl = FCNTL(fd, F_GETFL, NiL)) == -1)
	{
		f->flags = FILE_ERROR;
		return -1;
	}
	f->oflag = ffl;
	f->open = 0;
	f->flags = FILE_OPEN;
	if (S_ISREG(st->st_mode))
		f->flags |= FILE_REGULAR;
	if (ffd & FD_CLOEXEC)
		f->flags |= FILE_CLOEXEC;
	if ((ffl & O_ACCMODE) != O_RDONLY)
		f->flags |= FILE_WRITE;
	f->id.fid[0] = st->st_ino;
	f->id.fid[1] = st->st_dev;
	if ((f->mount = mp) && fd > state.cache)
		state.cache = fd;
	if (fd > state.open)
		state.open = fd;
	return 0;
}

#endif
