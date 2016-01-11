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
 * 3d file system initialization
 */

#include "3d.h"

static const char id[] =
#if defined(__STDC__) || defined(__STDPP__)
"\n@(#)$Id: 3d [ "
#if DEBUG
	"debug "
#endif
#if FS
	"msg "
#endif
"safe "
#if VCS
	"vcs "
#endif
"] (AT&T Research) 2012-06-25 $\0\n"
#else
"\n@(#)$Id: 3d (AT&T Research) 2011-06-25 $\0\n"
#endif
;

/*
 * if _3d_2d!=0 && getenv(_3d_2d)==0 then 2d
 */

char*	_3d_2d = 0;

/*
 * 3d mount get and set access functions
 */

static int
get_fs(Fs_t* gs, register char* buf, const char* op, int flags)
{
	register Fs_t*		fs;
	register int		n;
	register int		sum = 0;
	register unsigned long	x;
	int			m;
#if FS
	Mount_t*		mp;
#endif

	x = op ? getkey(op, op + strlen(op), 0) : 0;
	for (fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
		if (!(fs->flags & (FS_ERROR|FS_INIT)) && (fs->flags & (FS_BOUND|FS_OPEN)) && (!x || x == fs->key))
		{
			if (buf)
			{
				if (fs->flags & FS_BOUND) n = sfsprintf(buf, 0, "%-*s", fs->servicesize ? fs->servicesize : strlen(fs->service), fs->service);
				else n = sfsprintf(buf, 0, "-");
				n += sfsprintf(buf + n, 0, " /#%s/%s", gs->special, fs->special);
				m = n;
#if FS
				if (fs->ack)
				{
					n += sfsprintf(buf + n, 0, "/ack=");
					n += msggetmask(buf + n, SHRT_MAX, fs->ack);
				}
				if (fs->call != ~0)
				{
					n += sfsprintf(buf + n, 0, "/call=");
					n += msggetmask(buf + n, SHRT_MAX, fs->call);
				}
				if (fs->flags & FS_CLOSE)
					n += sfsprintf(buf + n, 0, "/close");
				if (fs->flags & FS_FLUSH)
					n += sfsprintf(buf + n, 0, "/flush");
				if (fs->flags & FS_GLOBAL)
				{
					n += sfsprintf(buf + n, 0, "/global");
					if (!(fs->flags & FS_ACTIVE))
						for (mp = state.global; mp; mp = mp->global)
							if (mp->fs == fs && mp->channel && mp->channel != -1)
							{
								n += sfsprintf(buf + n, 0, "=%d.%d", state.pid, MSG_CHANNEL_SYS(mp->channel));
								break;
							}
				}
				if (fs->flags & FS_INTERACTIVE)
					n += sfsprintf(buf + n, 0, "/interactive");
				if (fs->match)
					n += sfsprintf(buf + n, 0, "/match=%-*s", fs->matchsize ? fs->matchsize : strlen(fs->match), fs->match);
				if (fs->flags & FS_MONITOR)
					n += sfsprintf(buf + n, 0, "/monitor");
				if (fs->flags & FS_NAME)
					n += sfsprintf(buf + n, 0, "/name");
				if (!(fs->flags & FS_ON))
					n += sfsprintf(buf + n, 0, "/off");
				if (fs->flags & FS_RECEIVE)
					n += sfsprintf(buf + n, 0, "/receive");
				if (fs->flags & FS_REGULAR)
					n += sfsprintf(buf + n, 0, "/regular");
				if (fs->retry)
					n += sfsprintf(buf + n, 0, "/retry=%d", fs->retry);
				if (fs->terse)
				{
					n += sfsprintf(buf + n, 0, "/terse=");
					n += msggetmask(buf + n, SHRT_MAX, fs->terse);
				}
				if (fs->flags & FS_UNIQUE)
					n += sfsprintf(buf + n, 0, "/unique");
				if (fs->flags & FS_WRITE)
					n += sfsprintf(buf + n, 0, "/write");
				n += getattr(fs->attr, buf + n);
#endif
				if ((flags & (MAP_EXEC|MAP_INIT)) && (fs->flags & FS_OPEN))
				{
					if (!(fs->flags & FS_CLOSE))
					{
						if (fs->fd >= RESERVED_FD)
							n += sfsprintf(buf + n, 0, "/fd=%d", fs->fd);
					}
					else if ((flags & MAP_EXEC) && !(fs->flags & FS_BOUND))
						n = 0;
				}
				if (n > m || (fs->flags & FS_BOUND))
				{
					buf += n++;
					*buf++ = ' ';
				}
				else n = 0;
			}
			else n = ((fs->flags & FS_BOUND) ? (fs->servicesize ? fs->servicesize : strlen(fs->service)) : 0) +
				gs->specialsize +
				fs->specialsize +
#if FS
				(fs->ack ? (msggetmask(NiL, 0, fs->ack) + 6) : 0) +
				(fs->call != ~0 ? (msggetmask(NiL, 0, fs->call) + 6) : 0) +
				((fs->flags & FS_CLOSE) ? 6 : 0) +
				((fs->flags & FS_FLUSH) ? 6 : 0) +
				((fs->flags & FS_GLOBAL) ? ((fs->flags & (FS_ACTIVE|FS_CLOSE)) ? 7 : 22) : 0) +
				((fs->flags & FS_INTERACTIVE) ? 11 : 0) +
				(fs->match ? ((fs->matchsize ? fs->matchsize : strlen(fs->match)) + 7) : 0) +
				((fs->flags & FS_MONITOR) ? 0 : 8) +
				((fs->flags & FS_NAME) ? 5 : 0) +
				((fs->flags & FS_ON) ? 0 : 3) +
				((fs->flags & FS_RECEIVE) ? 0 : 8) +
				((fs->flags & FS_REGULAR) ? 0 : 8) +
				(fs->retry ? 12 : 0) +
				(fs->terse ? (msggetmask(NiL, 0, fs->terse) + 8) : 0) +
				((fs->flags & FS_UNIQUE) ? 0 : 7) +
				((fs->flags & FS_WRITE) ? 0 : 6) +
				getattr(fs->attr, NiL) +
#endif
				((flags & (MAP_EXEC|MAP_INIT)) && (fs->flags & (FS_CLOSE|FS_OPEN)) == FS_OPEN ? 10 : 0) +
				6;
			sum += n;
			if (op)
			{
				state.visit.fs = fs;
				break;
			}
		}
	n = iterate(&state.vmount, mapget, buf, flags);
	if (buf) buf += n;
	sum += n;
	state.visit.fs = 0;
	return(sum);
}

/*
 * return mask for msg calls in state.key.*
 */

static unsigned long
getmsgmask(const char* e)
{
	register char*		s;
	register char*		t;
	register int		c;
	register unsigned long	x = ~0;

	if (!(s = state.key.invert)) s = state.key.value;
	if (s != state.one)
	{
		for (t = s; t < (char*)e; t++)
			if (*t == '/')
				break;
		if (c = *t) *t = 0;
		x = msgsetmask(s);
		if (c) *t = c;
	}
	return(state.key.invert ? ~x : x);
}

static int
set_fs(register Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	register Fs_t*		ff;
	register unsigned long	x;
	int			n;
	int			old;
	const char*		oe;
#if FS
	Mount_t*		mp;
	Mount_t*		pm;
#endif

	oe = op + (opsize ? opsize : strlen(op));
	if (!(x = getkey(op, oe, 0)))
	{
		if (!*arg)
			while (state.vmount.table->key)
				search(&state.vmount, state.vmount.table->key, state.vmount.table->keysize, NiL, T_DELETE);
		return(0);
	}
	for (ff = 0, fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
		if ((fs->flags & (FS_BOUND|FS_INTERNAL)) && fs->key == x) break;
		else if (!ff && !fs->flags) ff = fs;
	if (fs >= state.fs + elementsof(state.fs))
	{
		if (!*arg && !state.key.next) return(0);
		if (!(fs = ff)) return(-1);
		fs->specialsize = state.key.next ? (state.key.next - (char*)op - 1) : opsize ? opsize : strlen(op);
		if (fs->specialsize >= elementsof(fs->special))
			fs->specialsize = elementsof(fs->special) - 1;
		strncpy(fs->special, op, fs->specialsize);
		fs->special[fs->specialsize] = 0;
		fs->key = x;
		fs->call = ~0;
		old = 0;
	}
	else if (!*arg && !state.key.next)
	{
		fsdrop(fs, 1);
		return(0);
	}
	else old = 1;
	if (*arg)
	{
		n = argsize ? argsize : strlen(arg);
		if (!fs->service || (fs->servicesize ? fs->servicesize : strlen(fs->service)) != n || strncmp(arg, fs->service, n))
		{
			fsdrop(fs, 1);
			if (fs->servicesize = argsize) fs->service = (char*)arg;
			else fs->service = strcpy(newof(0, char, n, 1), arg);
			fs->flags |= FS_BOUND|FS_ON;
			if (!(fs->flags & FS_INTERNAL)) fs->flags |= FS_FS;
			if (fs == &state.fs[FS_safe] && !(fs->flags & FS_INIT))
				state.safe = fs;
		}
	}
	while ((op = (const char*)state.key.next) && (x = getkey(op, oe, 0)))
		switch (x)
		{
		case HASHKEY2('f','d'):
			fsdrop(fs, 0);
			if ((fs->fd = strtol(state.key.value, NiL, 0)) > 0)
				fsinit(fs, fs->fd);
			break;
		case HASHKEY3('o','f','f'):
			if (state.key.invert) fs->flags |= FS_ON;
			else fs->flags &= ~FS_ON;
			break;
		case HASHKEY2('o','n'):
			if (state.key.invert) fs->flags &= ~FS_ON;
			else fs->flags |= FS_ON;
			break;
#if FS
		case HASHKEY3('a','c','k'):
			fs->ack = getmsgmask(oe);
			break;
		case HASHKEY6('a','c','t','i','v','e'):
			if (!(fs->flags & FS_INTERNAL))
			{
				if (state.key.invert) fs->flags &= ~FS_ACTIVE;
				else fs->flags |= FS_ACTIVE;
			}
			break;
		case HASHKEY4('c','a','l','l'):
			fs->call = getmsgmask(oe);
			break;
		case HASHKEY5('c','l','o','s','e'):
			if (!(fs->flags & FS_INTERNAL))
			{
				if (state.key.invert) fs->flags &= ~FS_CLOSE;
				else fs->flags |= FS_CLOSE;
			}
			break;
		case HASHKEY5('f','l','u','s','h'):
			if (state.key.invert) fs->flags &= ~FS_FLUSH;
			else fs->flags |= FS_FLUSH;
			break;
		case HASHKEY4('f','o','r','k'):
			if (!(fs->flags & FS_INTERNAL))
			{
				if (state.key.invert) fs->flags &= ~FS_FORK;
				else fs->flags |= FS_FORK;
			}
			break;
		case HASHKEY6('g','l','o','b','a','l'):
			if (state.key.invert) fs->flags &= ~FS_GLOBAL;
			else fs->flags |= FS_GLOBAL;
			for (mp = state.global, pm = 0; mp && mp->fs != fs; pm = mp, mp = mp->global);
			if (!mp)
			{
				if (!state.key.invert)
				{
					for (mp = state.mount; mp < state.mount + elementsof(state.mount) && mp->fs; mp++);
					if (mp < state.mount + elementsof(state.mount))
					{
						mp->fs = fs;
						mp->global = state.global;
						state.global = mp;
						if (state.key.value != state.one)
						{
							int	n;
							char*	e;

							n = strtol(state.key.value, &e, 0);
							if (*e == '.' && n == state.pid && (n = strtol(e + 1, NiL, 0)))
								mp->channel = MSG_CHANNEL(state.pid, n);
						}
					}
				}
			}
			else if (state.key.invert)
			{
				mp->fs = 0;
				if (pm) pm->global = mp->global;
				else state.global = 0;
			}
			if (state.global && !state.cache) state.cache = 1;
			break;
		case HASHKEY6('i','n','t','e','r','a'):
			if (state.key.invert) fs->flags &= ~FS_INTERACTIVE;
			else fs->flags |= FS_INTERACTIVE;
			break;
		case HASHKEY4('l','o','a','d'):
			if (!(fs->flags & (FS_INTERNAL|FS_LOAD)) && fs->service)
			{
				void*		dll;
				Fs_get_t	get;
				Fs_set_t	set;

				if ((dll = dlopen(fs->service, RTLD_LAZY)) && (set = (Fs_set_t)dlsym(dll, "set")))
				{
					fs->flags |= FS_LOAD|FS_INIT|FS_ON;
					fs->set = set;
					(*fs->set)(fs, state.null, 0, "init", 4);
					if (get = (Fs_get_t)dlsym(dll, "get"))
						fs->get = get;
				}
			}
			break;
		case HASHKEY5('m','a','t','c','h'):
			if (fs->match)
			{
				if (fs->matchsize) fs->matchsize = 0;
				else free(fs->match);
				fs->match = 0;
			}
			if (!state.key.invert)
			{
				if (opsize)
				{
					fs->match = state.key.value;
					fs->matchsize = state.key.valsize;
				}
				else fs->match = strcpy(newof(0, char, state.key.valsize, 1), state.key.value);
			}
			break;
		case HASHKEY6('m','o','n','i','t','o'):
			if (!(fs->flags & FS_INTERNAL))
			{
				if (state.key.invert) fs->flags &= ~FS_MONITOR;
				else fs->flags |= FS_MONITOR;
			}
			break;
		case HASHKEY4('n','a','m','e'):
			if (!(fs->flags & FS_INTERNAL))
			{
				if (state.key.invert) fs->flags &= ~FS_NAME;
				else fs->flags |= FS_NAME;
			}
			break;
		case HASHKEY6('r','e','c','e','i','v'):
			if (!(fs->flags & FS_INTERNAL))
			{
				if (state.key.invert) fs->flags &= ~FS_RECEIVE;
				else fs->flags |= FS_RECEIVE;
			}
			break;
		case HASHKEY6('r','e','g','u','l','a'):
			if (state.key.invert) fs->flags &= ~FS_REGULAR;
			else fs->flags |= FS_REGULAR;
			break;
		case HASHKEY5('r','e','t','r','y'):
			fs->retry = strtol(state.key.value, NiL, 0);
			break;
		case HASHKEY5('t','e','r','s','e'):
			fs->terse = getmsgmask(oe);
			break;
		case HASHKEY6('u','n','i','q','u','e'):
			if (state.key.invert) fs->flags &= ~FS_UNIQUE;
			else fs->flags |= FS_UNIQUE;
			break;
		case HASHKEY5('w','r','i','t','e'):
			if (state.key.invert) fs->flags &= ~FS_WRITE;
			else fs->flags |= FS_WRITE;
			break;
		default:
			setattr(fs->attr, op, oe);
			break;
#endif
		}
	if (!old)
	{
#if FS
		if ((fs->flags & (FS_ACTIVE|FS_MONITOR)) == FS_ACTIVE)
			fs->call |= MSG_MASK(MSG_fork);
		if (fs->flags & FS_NAME) state.call.name++;
		else state.call.monitor++;
#endif
	}
	return(0);
}

static int
get_map(Fs_t* fs, register char* buf, const char* op, int flags)
{
	register int	n;

	if (op) return(-1);
	state.visit.prefix = fs->special;
	state.visit.prelen = fs->specialsize + 4;
	n = iterate(&state.vmap, mapget, buf, flags);
	state.visit.prelen = 0;
	state.visit.prefix = 0;
	return(n);
}

static int
set_map(Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	NoP(fs);
	return(mapset(&state.vmap, arg, argsize, op, opsize));
}

static int
get_safe(Fs_t* fs, register char* buf, const char* op, int flags)
{
	register int	n;

	if (op) return(-1);
	state.visit.prefix = fs->special;
	state.visit.prelen = fs->specialsize + 4;
	n = iterate(&state.vsafe, mapget, buf, flags);
	state.visit.prelen = 0;
	state.visit.prefix = 0;
	return(n);
}

static int
set_safe(Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	NoP(fs);
	return(mapset(&state.vsafe, arg, argsize, op, opsize));
}

static int
get_intercept(Fs_t* fs, register char* buf, const char* op, int flags)
{
	register int	n;

	if (op)
		return -1;
	state.visit.prefix = fs->special;
	state.visit.prelen = fs->specialsize + 4;
	n = iterate(&state.vintercept, mapget, buf, flags);
	state.visit.prelen = 0;
	state.visit.prefix = 0;
	return n;
}

typedef int (*Init_f)(int, const char*, int);

static int
set_intercept(Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	void*			dll;
	Init_f			init;
	char			buf[PATH_MAX + 1];

	static const char	sym[] = "_3d_init";

	NoP(fs);
	if (!*arg || mapset(&state.vintercept, arg, argsize, op, opsize))
		return -1;
	if (argsize)
	{
		if (argsize > PATH_MAX)
			return -1;
		strncpy(buf, arg, argsize);
		buf[argsize] = 0;
		arg = (const char*)buf;
	}
	if (!(dll = dlopen(arg, RTLD_LAZY)))
	{
		error(2, "%s: %s", arg, dlerror());
		return -1;
	}
	if (!(init = (Init_f)dlsym(dll, sym)))
	{
		error(2, "%s: %s: initialization function not found", arg, sym);
		dlclose(dll);
		return -1;
	}
	return (*init)(0, op, opsize);
}

#if FS

static void
bencode(char** b, char* e, register unsigned long n, int a, int r, int x)
{
	register char*	s;
	register char*	t;
	register int	m;
	register int	z;
	char		buf[16];

	s = buf;
	z = (1 << (r - (x != 0)));
	do
	{
		m = n & ((1 << r) - 1);
		*s++ = m + ((m >= z) ? x : a);
	} while ((n >>= r) && s < &buf[sizeof(buf)]);
	t = *b;
	while (s > buf && t < e)
		*t++ = *--s;
}

#endif

static int
get_option(register Fs_t* fs, register char* buf, const char* op, int flags)
{
	register int		c;
	register int		n;
	register int		sum = 0;
	register unsigned long	x;
	char*			b;
	char*			e;
	int			m;

	x = op ? getkey(op, op + strlen(op), 0) : 0;

	/*
	 * table version
	 */

	if (!x && (flags & (MAP_EXEC|MAP_INIT)) || x == HASHKEY6('v','e','r','s','i','o'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/version=%d ", fs->special, TABLE_VERSION);
			buf += n;
		}
		else n = fs->specialsize + 18;
		sum += n;
	}

	/*
	 * trace output -- special case to capture redirection early
	 */

	if (!x && (fs->flags & FS_BOUND))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "%-*s /#%s/%s ", fs->servicesize ? fs->servicesize : strlen(fs->service), fs->service, state.fs[FS_fs].special, fs->special);
			buf += n;
		}
		else n = (fs->servicesize ? fs->servicesize : strlen(fs->service)) + state.fs[FS_fs].specialsize + fs->specialsize + 4;
		sum += n;
	}

	/*
	 * test mask
	 */

	if (!x && state.test || x == HASHKEY4('t','e','s','t'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/test=0%lo ", fs->special, state.test);
			buf += n;
		}
		else n = fs->specialsize + 23;
		sum += n;
	}

	/*
	 * readdir() view boundaries
	 */

	if (x == HASHKEY6('b','o','u','n','d','a'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/%sboundary ", fs->special, state.boundary ? state.null : "no");
			buf += n;
		}
		else n = fs->specialsize + 16;
		sum += n;
	}

#if DEBUG
	/*
	 * debug level
	 */

	if (!x && error_info.trace || x == HASHKEY5('d','e','b','u','g'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/debug=%d ", fs->special, -error_info.trace);
			buf += n;
		}
		else n = fs->specialsize + 18;
		sum += n;
	}
#endif

	/*
	 * license features
	 */

	if (!x && *state.license || x == HASHKEY6('l','i','c','e','n','s'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/license=%s ", fs->special, state.license);
			buf += n;
		}
		else n = fs->specialsize + strlen(state.license) + 14;
		sum += n;
	}

	/*
	 * 2d && 3d
	 */

	if (!x && (state.in_2d || state.limit < TABSIZE) || x == HASHKEY2(HASHKEYN('2'),'d') || x == HASHKEY2(HASHKEYN('3'),'d'))
	{
		if (buf)
		{
			n = (state.limit == TABSIZE) ? sfsprintf(buf, 0, "- /#%s/%cd ", fs->special, state.in_2d ? '2' : '3') : sfsprintf(buf, 0, "- /#%s/2d=%d ", fs->special, state.limit);
			buf += n;
		}
		else n = fs->specialsize + (state.limit == TABSIZE) ? 8 : 11;
		sum += n;
	}

#if FS

	/*
	 * file table
	 */

	if (state.cache && (!x && (flags & (MAP_EXEC|MAP_INIT)) || x == HASHKEY4('f','i','l','e')))
	{
		b = state.path.name;
		e = b + sizeof(state.path.name) - 1;
		c = -1;
		for (n = m = 0; n <= state.cache; n++)
			if (state.file[n].flags & FILE_CLOEXEC)
			{
				if ((flags & MAP_EXEC) && state.file[n].mount && fssys(state.file[n].mount, MSG_close))
					fscall(state.file[n].mount, MSG_close, 0, n);
			}
			else if (state.file[n].flags & FILE_OPEN)
			{
				if ((x = n - m - 1) > 0)
					bencode(&b, e, x, '0', 3, 0);
				if (x = state.file[n].id.fid[0])
					bencode(&b, e, x, 'a', 4, 0);
				bencode(&b, e, state.file[n].id.fid[1], 'A', 4, 0);
				if (state.file[n].mount && (x = state.file[n].mount - state.mount) != c)
					bencode(&b, e, c = x, 'Q', 4, 'q');
				m = n;
			}
		n = b - state.path.name;
		*b = 0;
		b = state.path.name;
		if (n)
		{
			if (buf)
			{
				n = sfsprintf(buf, 0, "- /#%s/file=%s ", fs->special, b);
 				buf += n;
 			}
 			else n += fs->specialsize + 11;
 			sum += n;
		}
	}

#endif

	/*
	 * fd table
	 */

	if (!x && (flags & (MAP_EXEC|MAP_INIT)) && state.table.fd != TABLE_FD || x == HASHKEY5('t','a','b','l','e'))
	{
		if (buf)
		{
			if ((n = FCNTL(TABLE_FD, F_GETFD, 0)) >= 0)
			{
				n = n ? -1 : FCNTL(TABLE_FD, F_DUPFD, RESERVED_FD);
				CLOSE(TABLE_FD);
			}
			if (state.table.fd <= 0 || FCNTL(state.table.fd, F_DUPFD, TABLE_FD) < 0)
			{
				if (state.table.fd > 0)
					cancel(&state.table.fd);
				n = -1;
			}
			else
			{
				cancel(&state.table.fd);
				state.table.fd = TABLE_FD;
				reserve(&state.table.fd);
			}
			if (n < 0) n = 0;
			else
			{
				n = sfsprintf(buf, 0, "- /#%s/table=%d ", fs->special, n);
				buf += n;
			}
		}
		else n = fs->specialsize + 18;
		sum += n;
	}

	/*
	 * syscall count
	 */

	if (!x && state.trace.count || x == HASHKEY5('c','o','u','n','t'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/%scount ", fs->special, state.trace.count ? state.null : "no");
			buf += n;
		}
		else n = fs->specialsize + 13;
		sum += n;
	}

	/*
	 * syscall trace
	 */

	if (!x && state.trace.pid || x == HASHKEY5('t','r','a','c','e'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/trace=%u ", fs->special, state.trace.pid + ((flags & (MAP_EXEC|MAP_INIT)) && state.trace.pid <= 2));
			buf += n;
		}
		else n = fs->specialsize + 18;
		sum += n;
	}

	/*
	 * syscall calls
	 */

	if (state.trace.call != ~0 && (!x || x == HASHKEY4('c','a','l','l')))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/call=", fs->special);
			n += msggetmask(buf + n, SHRT_MAX, state.trace.call);
			buf += n++;
			*buf++ = ' ';
		}
		else n = fs->specialsize + msggetmask(NiL, 0, state.trace.call) + 12;
		sum += n;
	}

#if FS

	/*
	 * message timeout
	 */

	if (!x && msg_info.timeout != MSG_TIMEOUT || x == HASHKEY6('t','i','m','e','o','u'))
	{
		if (buf)
		{
			n = sfsprintf(buf, 0, "- /#%s/timeout=%d ", fs->special, msg_info.timeout);
			buf += n;
		}
		else n = fs->specialsize + 20;
		sum += n;
	}

#endif

	return(sum);
}

#if DEBUG && FS

#define DUMP_call	(1<<0)
#define DUMP_file	(1<<1)
#define DUMP_fs		(1<<2)
#define DUMP_map	(1<<3)
#define DUMP_mount	(1<<4)
#define DUMP_safe	(1<<5)
#define DUMP_state	(1<<6)
#define DUMP_view	(1<<7)

/*
 * dump Table_t
 */

static void
dumptable(char** b, char* e, Table_t* tab, const char* name)
{
	register Map_t*	cp;
	register Map_t*	ep;
	register int	n;

	if (tab->size)
	{
		bprintf(b, e, "\n%s table\n\n", name);
		for (ep = (cp = tab->table) + tab->size; cp < ep; cp++)
		{
			bprintf(b, e, "  [%d]  %-*s", cp - tab->table, cp->keysize, cp->key);
			if ((n = 32 - cp->keysize) > 0)
				bprintf(b, e, "%*s", n, state.null);
			bprintf(b, e, " %-*s\n", T_VALSIZE(cp), cp->val);
		}
	}
}

/*
 * dump internal state to option output
 */

static void
dump(const char* op, const char* oe)
{
	register char*		e;
	register File_t*	fp;
	register Fs_t*		fs;
	register Mount_t*	mp;
	register int		list;
	register int		n;
	int			on;
	char*			b;

	if ((on = fsfd(&state.fs[FS_option])) <= 0) return;
	if (op == (char*)state.one) list = ~DUMP_call;
	else
	{
		list = 0;
		e = state.key.next;
		state.key.next = (char*)op;
		for (;;)
		{
			switch (getkey(state.key.next, oe, ','))
			{
			case 0:
				break;
			case HASHKEY4('c','a','l','l'):
				list |= DUMP_call;
				continue;
			case HASHKEY4('f','i','l','e'):
				list |= DUMP_file;
				continue;
			case HASHKEY2('f','s'):
				list |= DUMP_fs;
				continue;
			case HASHKEY3('m','a','p'):
				list |= DUMP_map;
				continue;
			case HASHKEY5('m','o','u','n','t'):
				list |= DUMP_mount;
				continue;
			case HASHKEY4('s','a','f','e'):
				list |= DUMP_safe;
				continue;
			case HASHKEY5('s','t','a','t','e'):
				list |= DUMP_state;
				continue;
			case HASHKEY4('v','i','e','w'):
				list |= DUMP_view;
				continue;
			}
			break;
		}
		state.key.next = e;
		if (!list) return;
	}
	e = (b = state.path.name) + sizeof(state.path.name) - 1;
	if (list & DUMP_state)
	{
		bprintf(&b, e, "\nstate %s\n\n", id + 10);
		if (state.limit == TABSIZE) bprintf(&b, e, "           %cd  on\n", state.in_2d ? '2' : '3');
		else bprintf(&b, e, "           2d  %d\n", state.limit);
		bprintf(&b, e, "     boundary  %s\n", state.boundary ? "on" : "off");
		bprintf(&b, e, "        cache  %u\n", state.cache);
		bprintf(&b, e, "         call  %u.%u", state.call.monitor, state.call.name);
		if (state.trace.call != ~0)
		{
			bprintf(&b, e, " ");
			b += msggetmask(b, e - b, state.trace.call);
		}
		bprintf(&b, e, "\n        count  %u\n", state.trace.count);
#if DEBUG
		bprintf(&b, e, "        debug  %d\n", -error_info.trace);
#endif
		bprintf(&b, e, "        level  %d\n", state.level);
#if LICENSED
		bprintf(&b, e, "      license  %s\n", state.license);
#endif
		bprintf(&b, e, "          pid  %u\n", state.pid);
		bprintf(&b, e, "          pwd  %s\n", state.pwd);
		bprintf(&b, e, "        table  %d\n", state.table.fd);
		bprintf(&b, e, "         test  %08o\n", state.test);
		bprintf(&b, e, "        trace  %u\n", state.trace.pid);
		bprintf(&b, e, "      version  %u\n", TABLE_VERSION);
	}
	if (list & DUMP_fs)
	{
		bprintf(&b, e, "\nfs table\n\n");
		for (fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
			if (fs->flags)
			{
				if ((n = fs - state.fs) < 10) n += '0';
				else n -= 10 - 'a';
				bprintf(&b, e, "  [%c]  %-*s", n, sizeof(fs->special), fs->special);
				if (fs->flags & FS_BOUND) bprintf(&b, e, " service=%-*s", fs->servicesize ? fs->servicesize : strlen(fs->service), fs->service);
				if (fs->flags & FS_ACTIVE) bprintf(&b, e, " active");
				if (fs->flags & FS_CLOSE) bprintf(&b, e, " close");
				if (fs->flags & FS_ERROR) bprintf(&b, e, " error");
				if (fs->flags & FS_FLUSH) bprintf(&b, e, " flush");
				if (fs->flags & FS_FORK) bprintf(&b, e, " fork");
				if (fs->flags & FS_FS) bprintf(&b, e, " fs");
				if (fs->flags & FS_GLOBAL) bprintf(&b, e, " global");
				if (fs->flags & FS_INIT) bprintf(&b, e, " init");
				if (fs->flags & FS_INTERACTIVE) bprintf(&b, e, " interactive");
				if (fs->flags & FS_INTERNAL) bprintf(&b, e, " internal");
#if LICENSED
				if (fs->flags & FS_LICENSED) bprintf(&b, e, " licensed");
#endif
				if (fs->flags & FS_LOAD) bprintf(&b, e, " load");
				if (fs->flags & FS_LOCK) bprintf(&b, e, " lock");
				if (fs->flags & FS_MAGIC) bprintf(&b, e, " magic");
				if (fs->flags & FS_MONITOR) bprintf(&b, e, " monitor");
				if (fs->flags & FS_NAME) bprintf(&b, e, " name");
				if (!(fs->flags & FS_ON)) bprintf(&b, e, " off");
				if (fs->flags & FS_OPEN) bprintf(&b, e, " open=%d", fs->fd);
				if (fs->flags & FS_RAW) bprintf(&b, e, " raw");
				if (fs->flags & FS_RECEIVE) bprintf(&b, e, " receive");
				if (fs->flags & FS_REFERENCED) bprintf(&b, e, " referenced");
				if (fs->flags & FS_REGULAR) bprintf(&b, e, " regular");
				if (fs->flags & FS_UNIQUE) bprintf(&b, e, " unique");
				if (fs->flags & FS_VALIDATED) bprintf(&b, e, " validated");
				if (fs->flags & FS_WRITE) bprintf(&b, e, " write");
				if (fs->call != ~0)
				{
					bprintf(&b, e, " call=");
					b += msggetmask(b, e - b, fs->call);
				}
				if (fs->ack)
				{
					bprintf(&b, e, " ack=");
					b += msggetmask(b, e - b, fs->ack);
				}
				if (fs->terse)
				{
					bprintf(&b, e, " terse=");
					b += msggetmask(b, e - b, fs->terse);
				}
				bprintf(&b, e, "%s", fs->attr);
				bprintf(&b, e, "\n");
			}
	}
	if (list & DUMP_mount)
	{
		bprintf(&b, e, "\nmount table\n\n");
		for (mp = state.mount; mp < state.mount + elementsof(state.mount); mp++)
			if (mp->fs)
			{
				if ((n = mp - state.mount) < 10) n += '0';
				else n -= 10 - 'a';
				bprintf(&b, e, "  [%c]  %-*s", n, sizeof(mp->fs->special), mp->fs->special);
				if (mp->logical) bprintf(&b, e, " logical=%-*s", mp->logicalsize ? mp->logicalsize : strlen(mp->logical), mp->logical);
				else if (mp->fs->flags & FS_GLOBAL) bprintf(&b, e, " global");
				if (mp->physical) bprintf(&b, e, " physical=%-*s", mp->physicalsize ? mp->physicalsize : strlen(mp->physical), mp->physical);
				if (mp->channel) bprintf(&b, e, " channel=%u", MSG_CHANNEL_SYS(mp->channel));
				bprintf(&b, e, "%s", mp->attr);
				bprintf(&b, e, "\n");
			}
	}
	if (list & DUMP_file)
	{
		bprintf(&b, e, "\nfile table\n\n");
		for (fp = state.file; fp < state.file + elementsof(state.file); fp++)
			if ((mp = fp->mount) || (fp->flags & (FILE_ERROR|FILE_OPEN)) || fp->reserved)
			{
				bprintf(&b, e, "  [%02d]", fp - state.file);
				if (mp)
				{
					if ((n = mp - state.mount) < 10) n += '0';
					else n -= 10 - 'a';
					bprintf(&b, e, " mount=%s[%c]", mp->fs->special, n);
				}
				if (fp->flags & FILE_CLOEXEC) bprintf(&b, e, " cloexec");
				if (fp->flags & FILE_ERROR) bprintf(&b, e, " error");
				if (fp->flags & FILE_LOCK) bprintf(&b, e, " lock");
				if (fp->id.fid[0] || fp->id.fid[1]) bprintf(&b, e, " fid=%ld%s%ld", fp->id.fid[0], fp->id.fid[1] >= 0 ? "+" : state.null, fp->id.fid[1]);
				if (fp->flags & FILE_OPEN) bprintf(&b, e, " open");
				if (fp->flags & FILE_REGULAR) bprintf(&b, e, " regular");
				if (fp->reserved) bprintf(&b, e, " reserved");
				if (fp->flags & FILE_VIRTUAL) bprintf(&b, e, " virtual");
				if (fp->flags & FILE_WRITE) bprintf(&b, e, " write");
				if (n = fp->flags & ~(FILE_LOCAL - 1)) bprintf(&b, e, " local=%08o", n);
				bprintf(&b, e, "\n");
			}
	}
	if (list & DUMP_view) dumptable(&b, e, &state.vpath, "view");
	if (list & DUMP_map) dumptable(&b, e, &state.vmap, "map");
	if (list & DUMP_safe) dumptable(&b, e, &state.vsafe, "safe");
	if (list & DUMP_call) calldump(&b, e);
	bprintf(&b, e + 1, "\n");
	WRITE(on, state.path.name, b - state.path.name);
}

#endif

static int
set_option(Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	register int		c;
	register const char*	oe;
	register char*		s;
	int			i;
	int			m;
	long			n;
#if FS
	Mount_t*		mp;
#endif

	NoP(argsize);
	oe = op + (opsize ? opsize : strlen(op));
	do switch (n = getkey(op, oe, 0))
	{
	case HASHKEY2(HASHKEYN('2'),'d'):
		state.limit = state.key.value == state.one ? 0 : strtol(state.key.value, NiL, 0);
		if (state.limit > 0) state.in_2d = 0;
		else
		{
			if (state.limit < 0)
				cancel(&state.table.fd);
			state.limit = TABSIZE;
			state.in_2d = 1;
		}
		break;
	case HASHKEY2(HASHKEYN('3'),'d'):
		state.in_2d = strtol(state.key.value, NiL, 0) <= 0;
		state.limit = TABSIZE;
		break;
	case HASHKEY6('b','o','u','n','d','a'):
		state.boundary = strtol(state.key.value, NiL, 0) > 0;
		break;
	case HASHKEY4('c','a','l','l'):
		state.trace.call = getmsgmask(oe);
		if (state.trace.count)
			state.trace.call |= MSG_MASK(MSG_exit);
		break;
	case HASHKEY5('c','o','u','n','t'):
		if (state.trace.count = strtol(state.key.value, NiL, 0))
		{
			state.trace.call |= MSG_MASK(MSG_exit);
			if (!state.trace.pid)
			{
				state.trace.pid = 1;
				goto setout;
			}
		}
		break;

#if DEBUG
	case HASHKEY5('d','e','b','u','g'):
		c = error_info.trace;
		if (error_info.trace = -strtol(state.key.value, NiL, 0));
		{
			if (!c)
			{
				errno = 0;
				message((error_info.trace, "%s [%d]", state.id, state.pid));
			}
			goto setout;
		}
		break;
#endif

#if DEBUG && FS
	case HASHKEY4('d','u','m','p'):
		dump(state.key.value, oe);
		break;
#endif

#if FS
	case HASHKEY4('f','i','l','e'):
		s = state.key.value;
		n = -1;
		m = 0;
		while (s < oe)
		{
			long	fid[2];
			long	off;

			if ((c = *s++) >= '0' && c < '0' + 8)
			{
				i = c - '0';
				while (s < oe && (c = *s++) >= '0' && c < '0' + 8)
					i = (i << 3) + c - '0';
				if (s >= oe) break;
				n += i;
			}
			else n++;
			fid[0] = 0;
			while (c >= 'a' && c < 'a' + 16)
			{
				fid[0] = (fid[0] << 4) + c - 'a';
				if (s >= oe) break;
				c = *s++;
			}
			fid[1] = 0;
			while (c >= 'A' && c < 'A' + 16)
			{
				fid[1] = (fid[1] << 4) + c - 'A';
				if (s >= oe) break;
				c = *s++;
			}
			off = 0;
			while (c >= 'a' && c < 'a' + 16)
			{
				off = (off << 4) + c - 'a';
				if (s >= oe) break;
				c = *s++;
			}
			i = 0;
			for (;;)
			{
				if (c >= 'Q' && c < 'Q' + 8)
					i = (i << 4) + c - 'Q';
				else if (c >= 'q' && c < 'q' + 8)
					i = (i << 4) + c - 'q' + 8;
				else break;
				if (s >= oe) break;
				c = *s++;
			}
			if (i) m = i;
			if (m >= 0 && m < elementsof(state.mount) && !fileinit(n, NiL, state.mount + m, 0))
			{
				state.file[n].id.fid[0] = fid[0];
				state.file[n].id.fid[1] = fid[1];
			}
		}
		break;
#endif

	case HASHKEY4('f','o','r','k'):
		if (state.trace.pid > 2)
			state.trace.pid = state.pid;
		break;
	case HASHKEY4('i','n','i','t'):
		if (!(fs->flags & FS_OPEN) && !FSTAT(2, &fs->st))
		{
			fs->flags |= FS_OPEN;
			fs->fd = 2;
		}
		state.trace.call = ~0;
		break;
	case HASHKEY6('l','i','c','e','n','s'):
		if ((char*)op >= state.table.buf && (char*)oe < state.table.buf + sizeof(state.table.buf))
		{
			if (state.key.valsize >= sizeof(state.license))
				state.key.valsize = sizeof(state.license) - 1;
			memcpy(state.license, state.key.value, state.key.valsize);
			state.license[state.key.valsize] = 0;
		}
		break;

#if DEBUG && FS
	case HASHKEY5('m','o','u','n','t'):
		if (pathreal(arg, P_PATHONLY|P_ABSOLUTE|P_NOSLASH, NiL) && (mp = getmount(state.path.name, &arg)))
			error(0, "getmount: %s: %s + %s", state.path.name, mp->fs->special, *arg ? arg : state.dot);
		break;
#endif

	case HASHKEY5('t','a','b','l','e'):
		if (state.table.fd == TABLE_FD && (i = strtol(state.key.value, NiL, 0)) > 0 && i != TABLE_FD)
		{
			CLOSE(state.table.fd);
			state.table.fd = FCNTL(i, F_DUPFD, TABLE_FD);
			CLOSE(i);
		}
		break;
	case HASHKEY4('t','e','s','t'):
		if (state.key.invert)
		{
			if (*state.key.invert >= '0' && *state.key.invert <= '9') state.key.value = state.key.invert;
			if ((n = strtol(state.key.value, NiL, 0)) <= 0) state.test = 0;
			else state.test &= ~n;
		}
		else state.test |= strtol(state.key.value, NiL, 0);
		break;

#if FS
	case HASHKEY6('t','i','m','e','o','u'):
		msg_info.timeout = strtol(state.key.value, NiL, 0);
		break;
	case HASHKEY6('t','i','m','e','s','t'):
		msg_info.timestamp = !state.key.invert;
		break;
#endif

	case HASHKEY5('t','r','a','c','e'):
		if (state.trace.pid = strtol(state.key.value, NiL, 0))
		{
			if (state.trace.pid > 2)
				state.trace.pid = state.pid;
		setout:
			if (state.fs[FS_option].fd == 2 && (i = FCNTL(2, F_DUPFD, RESERVED_FD)) >= 0)
			{
				state.fs[FS_option].fd = i;
				reserve(&state.fs[FS_option].fd);
			}
		}
		break;
	case HASHKEY6('v','e','r','s','i','o'):
		if ((state.table.version = strtol(state.key.value, NiL, 0)) != TABLE_VERSION)
			return(-1);
		break;
	} while (op = (const char*)state.key.next);
	return(0);
}

static int
get_pwd(register Fs_t* fs, register char* buf, const char* op, int flags)
{
	register int	n = 0;

	NoP(flags);
	if (op) return(-1);
	if (state.pwd)
	{
		if (buf) n = sfsprintf(buf, 0, "%s /#%s ", state.pwd, fs->special);
		else n = state.pwdsize + fs->specialsize + 4;
	}
	return(n);
}

/*
 * set state.pwd from s
 */

static int
setpwd(register const char* s)
{
	int		osiz;
	int		olev;
	struct stat	dot;
	struct stat	pwd;

	if (*s != '/' || *state.pwd == '/')
		return(-1);
	if (STAT(state.dot, &dot))
	{
		message((-1, "%s: cannot stat", state.dot));
		return(-1);
	}
	osiz = state.pwdsize;
	if ((state.pwdsize = strlen(s)) >= sizeof(state.pwdbuf))
		state.pwdsize = sizeof(state.pwdbuf) - 1;
	strncpy(state.pwd, s, state.pwdsize);
	state.pwd[state.pwdsize] = 0;
	state.pwdsize = pathcanon(state.pwd, sizeof(state.pwdbuf), 0) - state.pwd;
	olev = state.level;
	state.level = -1;
	state.path.linkname = 0;
	if ((s = pathreal(state.pwd, 0, &pwd)) && (dot.st_ino == pwd.st_ino && dot.st_dev == pwd.st_dev || state.path.linkname && !STAT(state.path.name, &pwd) && dot.st_ino == pwd.st_ino && dot.st_dev == pwd.st_dev))
	{
		state.level = state.path.level;
		memcpy(state.envpwd + sizeof(var_pwd) - 1, state.pwd, state.pwdsize);
		message((-1, "setpwd: state.pwd=%s state.level=%d state.path.level=%d", state.pwd, state.level, state.path.level));
		return(0);
	}
	message((-1, "%s: cannot set PWD", state.pwd));
	*state.pwd = '.';
	*(state.pwd + 1) = 0;
	state.pwdsize = osiz;
	state.level = olev;
	return(-1);
}

static int
set_pwd(Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	int	c;
	int	r;

	NoP(fs);
	NoP(op);
	NoP(opsize);
	if (!*arg || *state.pwd == '/')
		return(0);
	if (argsize)
	{
		c = *((char*)arg + argsize);
		*((char*)arg + argsize) = 0;
	}
	else c = 0;
	if (op) message((-1, "set_pwd arg=%s op=%-*s", arg, opsize ? opsize : strlen(op), op));
	r = setpwd(arg);
	if (c) *((char*)arg + argsize) = c;
	return(r);
}

static int
get_view(Fs_t* fs, register char* buf, const char* op, int flags)
{
	NoP(fs);
	if (op) return(-1);
	return(iterate(&state.vpath, mapget, buf, flags));
}

static int
set_view(Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	NoP(fs);
	return(mapset(&state.vpath, arg, argsize, op, opsize));
}

/*
 * set state.shell from s
 */

static int
setshell(register const char* s)
{
	if (ACCESS(s, 1))
	{
		message((-1, "%s: cannot access SHELL", s));
		return(-1);
	}
	strncpy(state.shell, s, PATH_MAX);
	return(0);
}

/*
 * static data initialization
 */

#define FSINIT(n,g,s,f,k)	{FS_INTERNAL|f,0,0,sizeof(n)-1,g,s,k,0,~0,0,n}

State_t	state =
{
	id + 10,			/* id			*/
	IDNAME,				/* cmd			*/
	".",				/* dot			*/
	"",				/* null			*/
	"1",				/* one			*/
	"/bin/sh",			/* binsh		*/
	var_3d,				/* env3d		*/
	var_pwd,			/* envpwd		*/
	var_shell,			/* envshell		*/
	var_view,			/* envview		*/
	{
	FSINIT("null",	0,		0,		FS_LICENSED,
		HASHKEY4('n','u','l','l')),
	FSINIT("option",get_option,	set_option,	FS_FORK|FS_LICENSED,
		HASHKEY6('o','p','t','i','o','n')),
	FSINIT("view",	get_view,	set_view,	0,
		HASHKEY4('v','i','e','w')),
	FSINIT("pwd",	get_pwd,	set_pwd,	FS_LICENSED,
		HASHKEY3('p','w','d')),
	FSINIT("fs",	get_fs,		set_fs,		0,
		HASHKEY2('f','s')),
	FSINIT("map",	get_map,	set_map,	0,
		HASHKEY3('m','a','p')),
	FSINIT("safe",	get_safe,	set_safe,	0,
		HASHKEY4('s','a','f','e')),
#if FS
	FSINIT("fd",	0,		0,		FS_FS|FS_NAME,
		HASHKEY2('f','d')),
#endif
	FSINIT("intercept",	get_intercept,	set_intercept,	FS_RAW,
		HASHKEY6('i','n','t','e','r','c')),

	/* NOTE: add internal mounts here */

#if VCS && defined(VCS_FS)
	VCS_FS,
#endif
	},
	"default",			/* default instance name*/
	".../...",			/* opaque		*/
	TABSIZE,			/* limit		*/
};

/*
 * note external control interrupt
 */

static void
note(int sig)
{
	state.control.note++;
	signal(sig, note);
}

/*
 * handle external control interrupt
 */

void
control(void)
{
	char*	s;
	int	fd;
	ssize_t	n;
	char	buf[PATH_MAX];

	if (state.control.note)
	{
		message((-2, "external control interrupt"));
		if (s = state.control.path) n = state.control.pathsize;
		else
		{
			s = "/tmp/3d";
			n = 0;
		}
		if (!n) n = strlen(s);
		sfsprintf(buf, sizeof(buf), "%-*s#%d", n, s, state.pid);
		if ((fd = OPEN(buf, O_RDONLY, 0)) >= 0)
		{
			if ((n = READ(fd, buf, sizeof(buf) - 1)) > 0)
			{
				buf[n] = 0;
				mapinit(buf, 0);
			}
			CLOSE(fd);
			if ((fd = OPEN(buf, O_RDWR|O_TRUNC, 0)) >= 0)
				CLOSE(fd);
		}
		state.control.note = 0;
	}
}

/*
 * push system call intercept
 */

int
intercept(Intercept_f call, unsigned long mask)
{
	register int	i;

	for (i = 0;; i++)
		if (i >= state.trap.size)
		{
			if (i >= elementsof(state.trap.intercept))
				return(-1);
			state.trap.size++;
			break;
		}
		else if (state.trap.intercept[i].call == call)
			break;
	state.trap.intercept[i].call = call;
	state.trap.intercept[i].mask = mask;
	return(0);
}

/*
 * 3d initialization
 */

#define env_2d		(1<<0)
#define env_3d		(1<<1)
#define env_cmd		(1<<2)
#define env_path	(1<<3)
#define env_pwd		(1<<4)
#define env_shell	(1<<5)
#define env_view	(1<<6)
#define env_must	(env_2d|env_3d|env_cmd|env_path|env_pwd|env_shell|env_view)
#define env_home	(1<<7)

#define var_cmd		"_="
#define var_disable	"_3D_DISABLE_="
#define var_home	"HOME="
#define var_path	"PATH="

int
init(int force, const char* opt, int opsize)
{
	register char**	ep = environ;
	register char*	cp;
	register int	i;
	Fs_t*		fs;
	char*		home = 0;
	Handler_t	handler;
	int		n;
	int		oerrno;

	/*
	 * initialize the 3d state
	 */

	if (!force && state.pid) return(0);
	oerrno = errno;
#if DEBUG
	error_info.id = state.cmd;
#endif
#if defined(SIGIO) || defined(SIGPWR)
#if defined(SIGIO)
	n = SIGIO;
#else
	n = SIGPWR;
#endif
	if ((handler = signal(n, note)) != SIG_DFL)
		signal(n, handler);
#endif
	state.pid = getpid();
	state.uid = geteuid();
	state.gid = getegid();
	state.pwd = state.pwdbuf;
	*state.pwd = '.';
	state.pwdsize = 1;
	state.shell = state.envshell + sizeof(var_shell) - 1;
	callinit();
	state.fs[FS_safe].flags |= FS_INIT;
	for (fs = state.fs; fs < state.fs + elementsof(state.fs) && fs->specialsize; fs++)
	{
		fs->flags |= FS_ON;
		if (fs->set) (*fs->set)(fs, state.null, 0, "init", 4);
	}

	/*
	 * extract the 3d tables from table.fd or the top of the environment
	 */

	cp = *(state.env = ep);
	i = 0;
	if ((n = peek(TABLE_FD, state.table.buf, sizeof(state.table.buf) - 1)) > 0 && !mapinit(state.table.buf, 1))
	{
		state.table.size = n + 1;
		state.table.fd = TABLE_FD;
		reserve(&state.table.fd);
		i |= env_view|env_3d;
	}
	else
	{
		state.table.version = TABLE_VERSION;
		if (cp && strneq(cp, state.env3d, sizeof(var_3d) - 1))
		{
			mapinit(cp + sizeof(var_3d) - 1, 1);
			ep++;
			environ++;
			i |= env_view|env_3d;
		}
	}
	if (_3d_2d) n = strlen(_3d_2d);
	else i |= env_2d;

	/*
	 * look for remaining var_* not in env_* mask i
	 */

	while (cp = *ep)
	{
		if (strneq(cp, var_disable, sizeof(var_disable) - 1))
		{
			state.pid = 0;
			errno = oerrno;
			return(0);
		}
		else if (!(i & env_cmd) && strneq(cp, var_cmd, sizeof(var_cmd) - 1))
		{
			state.cmd = cp + sizeof(var_cmd) - 1;
			if ((i |= env_cmd) == env_must) break;
		}
		else if (!(i & env_home) && strneq(cp, var_home, sizeof(var_home) - 1))
		{
			home = cp + sizeof(var_home) - 1;
			if ((i |= env_home) == env_must) break;
		}
		else if (!(i & env_path) && strneq(cp, var_path, sizeof(var_path) - 1))
		{
			state.envpath = cp + sizeof(var_path) - 1;
			if ((i |= env_path) == env_must) break;
		}
		else if (!(i & env_pwd) && strneq(cp, state.envpwd, sizeof(var_pwd) - 1))
		{
			if (geteuid())
				*ep = state.envpwd;
			cp += sizeof(var_pwd) - 1;
			if (!setpwd(cp) && ((i |= env_pwd) == env_must)) break;
		}
		else if (!(i & env_shell) && strneq(cp, state.envshell, sizeof(var_shell) - 1))
		{
			*ep = state.envshell;
			cp += sizeof(var_shell) - 1;
			if (!setshell(cp) && ((i |= env_shell) == env_must)) break;
		}
		else if (!(i & env_view) && strneq(cp, state.envview, sizeof(var_view) - 1))
		{
			char*	mp;
			char*	zp;

			cp += sizeof(var_view) - 1;
			if (mp = strchr(cp, ':')) do
			{
				if (!(zp = strchr(++mp, ':'))) zp = mp + strlen(mp);
				mapset(&state.vpath, cp, mp - cp - 1, mp, zp - mp);
				cp = mp;
			} while (*(mp = zp));
			if ((i |= env_view) == env_must) break;
		}
		else if (!(i & env_3d) && strneq(cp, state.env3d, sizeof(var_3d) - 1))
		{
			mapinit(cp + sizeof(var_3d) - 1, 1);
			if ((i |= env_3d) == env_must) break;
		}
		else if (!(i & env_2d) && strneq(cp, _3d_2d, n) && cp[n] == '=')
		{
			if ((i |= env_2d) == env_must) break;
		}
		ep++;
	}
	if (!(i & env_2d)) state.in_2d = 1;
	if (!(i & env_pwd) && *state.pwd != '/' && setpwd("/") && (!home || setpwd(home)))
	{

		n = state.in_2d;
		state.in_2d = 2;
		if (!getcwd(state.path.name, sizeof(state.path.name)) || setpwd(state.path.name))
		{
			state.pwd = 0;
			if (!n)
			{
				static char	msg[] = "3d: invalid PWD -- falling back to 2d\n";

				write(2, msg, sizeof(msg) - 1);
			}
		}
		else state.in_2d = n;
	}
	if (!(i & env_shell)) strcpy(state.shell, state.binsh);
	if (state.table.fd <= 0 && mapdump(NiL, NiL, MAP_INIT) < sizeof(state.table.buf))
	{
		n = mapdump(NiL, state.table.buf, MAP_INIT);
		keep(state.table.buf, n, 0);
	}
	if (state.table.fd <= 0 && (state.channel.fd = open("/dev/null", O_RDONLY)) >= 0)
		reserve(&state.channel.fd);
	if (state.fs[FS_safe].flags & FS_BOUND)
	{
		state.safe = &state.fs[FS_safe];
		if (!state.pwd || !pathreal(state.pwd, P_PATHONLY, NiL))
		{
#if DEBUG
			error(4, ". is not safe");
#else
			static char	msg[] = "3d: . is not safe\n";

			write(2, msg, sizeof(msg) - 1);
			_exit(2);
#endif
		}
	}
	state.fs[FS_safe].flags &= ~FS_INIT;
	errno = oerrno;
	return(0);
}
