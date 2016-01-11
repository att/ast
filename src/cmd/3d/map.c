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
 * 3d mount map support
 */

#include "3d.h"

/*
 * visit a mapping and compute its size or copy mapping
 */

int
mapget(Map_t* map, char* buf, int flags)
{
	register char*	p;
	register int	z;
	register char*	v;
	register int	n;
#if FS
	Mount_t*	mp = 0;
#endif

	NoP(flags);
	if (!(n = map->keysize))
		n = strlen(map->key);
	switch (z = map->valsize & T_SIZE)
	{
#if FS
	case T_MOUNT:
		mp = (Mount_t*)map->val;
		if (state.visit.fs && state.visit.fs != mp->fs)
			return 0;
		v = mp->fs->special;
		z = mp->fs->specialsize;
		break;
#endif
	case 0:
		z = strlen(map->val);
		/*FALLTHROUGH*/
	default:
		v = map->val;
		break;
	}
	if (p = buf)
	{
		strncpy(p, map->key, n);
		p += n;
		*p++ = ' ';
		if (
#if FS
			mp ||
#endif
			state.visit.prefix)
		{
			*p++ = '/';
			*p++ = '#';
#if FS
			if (!mp)
#endif
			{
				p = strcopy(p, state.visit.prefix);
				*p++ = '/';
				if (*v == '/')
					*p++ = '#';
			}
		}
		strncpy(p, v, z);
		p += z;
#if FS
		if (mp)
		{
			if (mp->channel)
				p += sfsprintf(p, 0, "/channel=%d", MSG_CHANNEL_SYS(mp->channel));
			p += getattr(mp->attr, p);
			if (mp->physical)
				p += sfsprintf(p, 0, "%-*s", mp->physicalsize ? mp->physicalsize : strlen(mp->physical), mp->physical);
		}
#endif
		*p++ = ' ';
		n = p - buf;
	}
	else
		n += z + state.visit.prelen
#if FS
		+ (mp ? ((mp->channel ? 16 : 0) + (mp->physical ? (mp->physicalsize ? mp->physicalsize : strlen(mp->physical)) : 0) + getattr(mp->attr, NiL)) : 0)
#endif
		+ 6;
	return n;
}

/*
 * dump table mappings into buf
 * the mapping size is returned
 * if tab==0 then all 3d mappings are processed
 * if buf==0 then the size is just computed
 */

int
mapdump(Table_t* tab, char* buf, int flags)
{
	register Fs_t*		fs;
	register int		n;
	register int		sum = 0;
	register char*		p = buf;

	if (flags & (MAP_EXEC|MAP_INIT))
	{
		if (state.kernel)
		{
			if (p)
				*p = 0;
			return 1;
		}
		if (p)
		{
			n = sfsprintf(p, 0, "%s", TABLE_PREFIX);
			p += n;
		}
		else
			n = sizeof(TABLE_PREFIX) - 1;
		sum += n;
	}
	if (!tab)
	{
		for (fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
			if (fs->get)
			{
				n = (*fs->get)(fs, p, NiL, flags);
				if (p)
					p += n;
				sum += n;
			}
	}
	else
	{
		n = iterate(tab, mapget, p, flags);
		if (p)
			p += n;
		sum += n;
	}
	if (p)
	{
		if (p > buf)
		{
			if (!(flags & MAP_INIT))
				p--;
			else if ((p - buf) == (sizeof(TABLE_PREFIX) - 1))
				p = buf;
			else
				*(p - 1) = '\n';
		}
		*p = 0;
		sum = p - buf;
	}
	return sum;
}

#if LICENSED

/*
 * check fs license
 */

static int
licensed(register Fs_t* fs)
{
	if ((fs->flags & (FS_INTERNAL|FS_LICENSED)) == FS_INTERNAL)
	{
		if (!(fs->flags & FS_VALIDATED))
		{
			fs->flags |= FS_VALIDATED;
			if (!*state.license)
			{
				Pathcheck_t	pc;

				if (pathcheck(PATHCHECK, error_info.id, &pc))
					*state.license = LICENSE_NONE;
				else if (!pc.feature)
					*state.license = LICENSE_ALL;
				else
					sfsprintf(state.license, sizeof(state.license), "%c%s%c", LICENSE_SEP, pc.feature, LICENSE_SEP);
				message((-2, "license: feature=%s", state.license));
			}
			if (*state.license == LICENSE_ALL)
				fs->flags |= FS_LICENSED;
			else
			{
				char	buf[sizeof(fs->special) + 4];

				sfsprintf(buf, sizeof(buf), "*%c%s%c*", LICENSE_SEP, fs->special, LICENSE_SEP);
				message((-1, "mount: license=%s pattern=%s", state.license, buf));
				if (strmatch(state.license, buf))
					fs->flags |= FS_LICENSED;
			}
			if (!(fs->flags & FS_LICENSED))
				error(2, "%s service not licensed", fs->special);
		}
		if (!(fs->flags & FS_LICENSED))
			return 0;
	}
	return 1;
}

#else

#define licensed(p)	(1)

#endif

#if FS

/*
 * copy attributes into table buf
 */

int
getattr(const char* attr, char* buf)
{
	register char*	a = (char*)attr;
	register char*	b = buf;
	register int	c;

	if (b)
	{
		while (c = *b++ = *a++)
			if (c == ' ')
				*(b - 1) = '/';
		return b - buf - 1;
	}
	return strlen(a);
}

/*
 * add/delete attributes
 * attributes stored as
 *
 *	(<space>name=value)*
 */

const char*
setattr(char* attr, const char* op, register const char* oe)
{
	register const char*	s;
	register const char*	v;
	register char*		a;
	register char*		x;
	register int		n;
	int			k;
	int			m;

	if (s = op)
	{
		for (;;)
		{
			if (s >= oe || *s == '/')
				return op;
			if (*s++ == '=')
			{
				v = s - 1;
				n = v - op;
				while (s < oe && *s != '/')
					s++;
				k = s - v - 1;
				m = s - op;
				a = attr;
				while (*a)
				{
					a++;
					if (strneq(op, a, n) && a[n] == '=')
					{
						if (!k || strncmp(op, a, m) || a[m] && a[m] != ' ')
						{
							if (!(x = strchr(a, ' ')))
							{
								if (a > attr)
									a--;
								*a = 0;
							}
							else
								while (*a++ = *++x);
						}
						else
							k = 0;
						break;
					}
					if (!(a = strchr(a, ' ')))
						break;
				}
				if (k && ((k = strlen(attr)) + m) < (ATTR_MAX - 1))
				{
					a = attr + k;
					*a++ = ' ';
					strncpy(a, op, m);
					a[m] = 0;
				}
				if (s++ >= oe)
					return 0;
				op = s;
			}
		}
	}
	return op;
}

#endif

/*
 * handle `arg /#<op>'
 */

static int
special(register const char* op, int opsize, const char* arg, int argsize)
{
	register const char*	oe;
	register Fs_t*		fs;
	const char*		ov;
	const char*		org;
	Map_t*			map;
	int			arglen;
	unsigned long		x;
#if FS
	register Mount_t*	mp;
#endif

	if (!(arglen = argsize))
		arglen = strlen(arg);
	oe = op + (opsize ? opsize : strlen(op));
	if (!(x = getkey(op, oe, 0)))
	{
		if (arglen)
		{
			errno = ENODEV;
			return -1;
		}
		for (fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
			if (fs->set)
				(*fs->set)(fs, state.null, 0, state.null, 0);
		return 0;
	}
	if (state.key.invert)
	{
		errno = ENODEV;
		return -1;
	}
	org = arg;
	if (!argsize && *arg)
	{
		if (!pathreal(arg, P_PATHONLY|P_ABSOLUTE|P_NOSLASH, NiL))
			return -1;
		arg = (const char*)state.path.name;
	}
	ov = (char*)state.key.next;
	for (fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
		if (!x || x == fs->key)
	{
		if (!licensed(fs))
			return 0;
#if FS
		if (fs->flags & FS_FS)
		{
			if (map = search(&state.vmount, arg, arglen, NiL, op < oe ? 0 : T_DELETE))
			{
				mp = (Mount_t*)map->val;
				if (mp->fs != fs)
				{
					errno = EEXIST;
					return -1;
				}
				message((-2, "mount: old fs=%s map=%-*s arg=%-*s op=%-*s", fs->special, map->keysize, map->key, arglen, arg, ov ? (oe - ov) : 6, ov));
				if (setattr(mp->attr, ov, oe))
				{
					errno = EEXIST;
					return -1;
				}
			}
			else if (op < oe)
			{
				if (arglen)
				{
					for (mp = state.mount;; mp++)
					{
						if (mp >= state.mount + elementsof(state.mount))
							return -1;
						if (!mp->fs)
							break;
					}
					if (map = search(&state.vmount, arg, arglen, (char*)mp, argsize ? T_MOUNT : (T_MOUNT|T_ALLOCATE)))
					{
						message((-2, "mount: new fs=%s map=%-*s arg=%-*s op=%-*s", fs->special, map->keysize, map->key, arglen, arg, ov ? (oe - ov) : 6, ov));
						fs->flags |= FS_REFERENCED;
						if (!(fs->flags & FS_NAME) && !state.cache)
							state.cache = 1;
						mp->fs = fs;
						mp->logical = map->key;
						mp->logicalsize = map->keysize;
						if (ov)
						{
							if ((oe - ov) > 8 && strneq(ov, "channel=", 8))
							{
								mp->channel = strtol(ov + 8, (char**)&ov, 0);
								if (++ov >= oe)
									ov = 0;
							}
							if (ov = setattr(mp->attr, ov, oe))
							{
								if (opsize)
								{
									mp->physical = (char*)ov - 1;
									mp->physicalsize = oe - mp->physical;
								}
								else
								{
									mp->physical = strcpy(newof(0, char, strlen(ov), 2), ov - 1);
									mp->physicalsize = 0;
								}
							}
						}
					}
				}
				else if (setattr(fs->attr, ov, oe))
				{
					errno = EEXIST;
					return -1;
				}
			}
		}
		else
#endif
		if (fs->set)
		{
			if (ov)
			{
				if ((*fs->set)(fs, (fs->flags & FS_RAW) ? org : arg, argsize, ov, oe - ov))
					return -1;
			}
			else if (arglen)
			{
				if ((*fs->set)(fs, (fs->flags & FS_RAW) ? org : arg, argsize, state.null, 0))
					return -1;
			}
			else if ((*fs->set)(fs, state.null, 0, state.null, 0))
				return -1;
		}
		if (op < oe)
			break;
	}
	if (op < oe && fs >= state.fs + elementsof(state.fs))
	{
		errno = ENODEV;
		return -1;
	}
	return 0;
}

/*
 * set a single 3d table mapping
 * size==0 for volatile args
 * from="-" maps to from=""
 * to="-" maps to to=""
 */

int
mapset(Table_t* tab, const char* from, int fromsize, register const char* to, int tosize)
{
	register Map_t*	old;
	register int	n;
	int		x;
	char		buf[PATH_MAX + 1];

	if (state.safe && !(state.test & 0100))
	{
		errno = EPERM;
		return -1;
	}
	if (!fromsize && (!from || !from[0] || from[0] == '-' && !from[1]) || fromsize == 1 && from[0] == '-')
	{
		from = state.null;
		fromsize = 0;
	}
	if (!tosize && (!to || !to[0] || to[0] == '-' && !to[1]) || tosize == 1 && to[0] == '-')
	{
		to = state.null;
		tosize = 0;
	}
	message((-2, "mount: %-*s %-*s", from == state.null ? 1 : fromsize ? fromsize : strlen(from), from == state.null ? "-" : from, to == state.null ? 1 : tosize ? tosize : strlen(to), to == state.null ? "-" : to));
	if ((!tosize || tosize >= 2) && to[0] == '/' && to[1] == '#')
	{
		to += 2;
		if (tosize)
			tosize -= 2;
		return special(to, tosize, from, fromsize);
	}
	if (!*from)
	{
		if (!*to)
			while (tab->table->key)
				search(tab, tab->table->key, tab->table->keysize, NiL, T_DELETE);
		return 0;
	}
	if (fromsize)
	{
		if (tosize || !to[0])
		{
			/*
			 * no malloc during initialization
			 * so we have to believe from and to here
			 */

			return search(tab, from, fromsize, to, tosize) ? 0 : -1;
		}
		n = *(from + fromsize);
		*((char*)from + fromsize) = 0;
	}
	if (!licensed(&state.fs[tab == &state.vpath ? FS_view : tab == &state.vmap ? FS_map : FS_safe]))
		return 0;
	if (x = tab != &state.vintercept)
	{
		if (!pathreal(from, P_PATHONLY|P_ABSOLUTE|P_NOSLASH, NiL))
			return -1;
		if (fromsize)
		{
			*((char*)from + fromsize) = n;
			fromsize = 0;
		}
		from = (const char*)state.path.name;
	}
	if (!fromsize)
		fromsize = strlen(from);
	old = search(tab, from, fromsize, NiL, *to ? 0 : T_DELETE);
	if (!*to)
	{
		search(&state.vmount, from, fromsize, NiL, T_DELETE);
		return 0;
	}
	if (tab == &state.vmap)
	{
		if (old)
		{
			if (!(old->valsize & T_SIZE))
				free(old->val);
			if ((old->valsize = tosize) & T_SIZE)
				old->val = (char*)to;
			else
				old->val = strcpy(newof(0, char, strlen(to), 1), to);
		}
		else
			search(tab, from, fromsize, to, tosize|T_ALLOCATE);
	}
	else if (old)
	{
		/*
		 * ok if previous mapping matched
		 */

		if (x && !(to = pathreal(to, P_PATHONLY|P_ABSOLUTE|P_NOSLASH, NiL)))
			return -1;
		if ((n = T_VALSIZE(old)) == (tosize ? tosize : strlen(to)) && strneq(old->val, to, n))
			return 0;

		/*
		 * already have different mapping
		 */

		errno = EEXIST;
		return -1;
	}
	else
	{
		strncpy(buf, from, PATH_MAX);
		buf[PATH_MAX] = 0;
		if (x)
		{
			if (!pathreal(to, P_PATHONLY|P_ABSOLUTE|P_NOSLASH, NiL))
				return -1;
			to = (const char*)state.path.name;
		}
		if (tab == &state.vsafe || !streq(buf, to))
			search(tab, buf, fromsize, to, T_ALLOCATE);
	}
	return 0;
}

/*
 * initialize mappings from space separated pairs in buf
 * if readonly!=0 then buf must not be changed after this call
 */

int
mapinit(const char* buf, int readonly)
{
	register char*	bp = (char*)buf;
	register char*	from;
	register char*	to;
	int		fromsize;
	int		tosize;

#if DEBUG_TABLE
	if (!error_info.trace)
		error_info.trace = -3;
	message((-1, "TABLE `%s'", bp));
#endif
	if (strneq(bp, TABLE_PREFIX, sizeof(TABLE_PREFIX) - 1))
		bp += sizeof(TABLE_PREFIX) - 1;
#if DEBUG_TABLE
	if (bp != buf)
		message((-1, "TABLE `%s'", bp));
#endif
	if (!readonly)
		fromsize = tosize = 0;
	while (*bp)
	{
		for (from = bp; *bp && *bp != ' '; bp++);
		if (readonly)
		{
			fromsize = bp - from;
			if (*bp)
				bp++;
		}
		else if (*bp)
			*bp++ = 0;
		for (to = bp; *bp && *bp != ' ' && *bp != '\n'; bp++);
		if (readonly)
			tosize = bp - to;
		else if (*bp)
			*bp++ = 0;
#if DEBUG_TABLE
		if (mapset(&state.vpath, from, fromsize, to, tosize))
		{
			if (readonly)
				message((-1, "TABLE %-*s -> %-*s FAILED", fromsize, from, tosize, to));
			else
				message((-1, "TABLE %s -> %s FAILED", from, to));
			return -1;
		}
		if (state.table.version != TABLE_VERSION)
		{
			message((-1, "TABLE state.table.version=%d != TABLE_VERSION=%d", state.table.version, TABLE_VERSION));
			return -1;
		}
#else
		if (mapset(&state.vpath, from, fromsize, to, tosize) || state.table.version != TABLE_VERSION)
			return -1;
#endif
		if (*bp)
			bp++;
	}
	return 0;
}
