/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * pax { asc aschk binary cpio } formats
 */

#include "format.h"

#define ASC			1
#define ASCHK			2
#define BINARY			3
#define CPIO			4

#define CPIO_MAGIC		070707
#define CPIO_EXTENDED		1
#define CPIO_HEADER		76
#define CPIO_TRAILER		"TRAILER!!!"
#define CPIO_TRUNCATE(x)	((x)&0177777)
#define CPIO_NAMESIZE		256
#define CPIO_ALIGN		0

#define ASC_HEADER		110
#define ASC_MAGIC		070701
#define ASC_NAMESIZE		1024
#define ASC_ALIGN		4

#define ASCHK_MAGIC		070702

#define BINARY_HEADER		sizeof(Binary_header_t)
#define BINARY_NAMESIZE		256
#define BINARY_ALIGN		2

typedef struct Binary_header_s
{
	short		magic;
	unsigned short	dev;
	unsigned short	ino;
	unsigned short	mode;
	unsigned short	uid;
	unsigned short	gid;
	short		links;
	unsigned short	rdev;
	unsigned short	mtime[2];
	short		namesize;
	unsigned short	size[2];
} Binary_header_t;

typedef struct Localstat_s
{
	long	dev;
	long	ino;
	long	mode;
	long	uid;
	long	gid;
	long	nlink;
	long	rdev;
	long	mtime;
	off_t	size;
	long	dev_major;
	long	dev_minor;
	long	rdev_major;
	long	rdev_minor;
	long	checksum;
} Localstat_t;

typedef struct Cpio_s
{
	int	trailer;
#if CPIO_EXTENDED
	char	opsbuf[PATH_MAX];	/* extended ops buffer		*/
	char*	ops;			/* opsbuf output pointer	*/
#endif
} Cpio_t;

#if CPIO_EXTENDED

/*
 * get and execute extended ops from input
 */

static void
getxops(register Archive_t* ap, register File_t* f)
{
	register char*	p;
	register char*	s;
	register int	c;

	if (f->namesize > (c = strlen(f->name) + 1))
		for (p = f->name + c; c = *p++;)
		{
			for (s = p; *p; p++);
			p++;
			message((-2, "%s: %s: entry %d.%d op = %c%s", ap->name, f->name, ap->volume, ap->entry, c, s));
			switch (c)
			{
			case 'd':
				IDEVICE(f->st, strtol(s, NiL, 16));
				break;
			case 'g':
				f->st->st_gid = strtol(s, NiL, 16);
				break;
			case 's':
				f->st->st_size = strtoll(s, NiL, 16);
				break;
			case 'u':
				f->st->st_uid = strtol(s, NiL, 16);
				break;
			case 'G':
				f->gidname = s;
				break;
			case 'U':
				f->uidname = s;
				break;

				/*
				 * NOTE: ignore unknown ops for future extensions
				 */
			}
		}
}

/*
 * set end of extended ops
 */

static void
setxops(Archive_t* ap, register File_t* f)
{
	register Cpio_t*	cpio = (Cpio_t*)ap->data;
	register int		n;

	if (n = cpio->ops - cpio->opsbuf)
	{
		n++;
		*cpio->ops++ = 0;
		if ((f->namesize += n) > CPIO_NAMESIZE)
			error(1, "%s: extended ops may crash older cpio programs", f->name);
	}
}

/*
 * output filename and extended ops
 */

static void
putxops(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	register Cpio_t*	cpio = (Cpio_t*)ap->data;
	register int		n;

	n = cpio->ops - cpio->opsbuf;
	paxwrite(pax, ap, f->name, f->namesize -= n);
	if (n)
		paxwrite(pax, ap, cpio->ops = cpio->opsbuf, n);
}


/*
 * add extended op string
 */

static void
addxopstr(Archive_t* ap, int op, register char* s)
{
	register Cpio_t*	cpio = (Cpio_t*)ap->data;
	register char*		p = cpio->ops;
	register char*		e = cpio->opsbuf + sizeof(cpio->opsbuf) - 3;

	if (p < e)
	{
		*p++ = op;
		while (*s && p < e)
			*p++ = *s++;
		*p++ = 0;
		cpio->ops = p;
	}
#if DEBUG
	if (*s)
		error(PANIC, "addxopstr('%c',\"%s\") overflow", op, s);
#endif
}

/*
 * add extended op number
 */

static void
addxopnum(Archive_t* ap, int op, Sflong_t n)
{
	char	buf[33];

	sfsprintf(buf, sizeof(buf), "%I*x", sizeof(n), n);
	addxopstr(ap, op, buf);
}

#endif

static int
init(Archive_t* ap)
{
	register Cpio_t*	cpio;

	if (ap->data)
	{
		cpio = (Cpio_t*)ap->data;
		memset(cpio, 0, sizeof(*cpio));
	}
	else
	{
		if (!(cpio = newof(0, Cpio_t, 1, 0)))
			nospace();
		ap->data = cpio;
	}
#if CPIO_EXTENDED
	cpio->ops = cpio->opsbuf;
#endif
	return 1;
}

static int
cpio_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	int		magic;

	if (size >= CPIO_HEADER && buf[0] == '0' && sfsscanf((char*)buf, "%6o", &magic) == 1 && magic == CPIO_MAGIC)
		return init(ap);
	return 0;
}

static int
cpio_common(Pax_t* pax, register Archive_t* ap, register File_t* f, int align, int header)
{
	char*		s;

	f->linktype = NOLINK;
	f->linkpath = 0;
	f->uidname = 0;
	f->gidname = 0;
	switch (ap->format->variant)
	{
	case BINARY:
		align = BINARY_ALIGN;
		header = BINARY_HEADER;
		break;
	case ASC:
	case ASCHK:
		align = ASC_ALIGN;
		header = ASC_HEADER;
		break;
	default:
		align = 0;
		break;
	}
	if (align)
	{
		if (header = (header + f->namesize) % align)
			align -= header;
		else
			align = 0;
	}
	f->name = paxstash(pax, &ap->stash.head, NiL, f->namesize + align);
	paxread(pax, ap, f->name, (off_t)0, (off_t)(f->namesize + align), 1);
	if (f->name[f->namesize - 1])
	{
		paxunread(pax, ap, &f->name[f->namesize - 1], 1);
		f->name[f->namesize - 1] = 0;
		error(state.keepgoing ? 1 : 3, "%s: entry %d.%d file name terminating null missing", ap->name, ap->volume, ap->entry);
	}
#if CPIO_EXTENDED
	getxops(ap, f);
#endif
	if (streq(f->name, CPIO_TRAILER))
	{
		((Cpio_t*)ap->data)->trailer++;
		getdeltaheader(ap, f);
		if (ap->delta)
			setinfo(ap, f);
		return 0;
	}
	switch (f->type = X_ITYPE(f->st->st_mode))
	{
	case X_IFBLK:
	case X_IFCHR:
	case X_IFDIR:
	case X_IFIFO:
	case X_IFLNK:
	case X_IFREG:
	case X_IFSOCK:
		break;
	default:
		error(1, "%s: %s: unknown file type %07o -- regular file assumed", ap->name, f->name, f->type);
		f->type = X_IFREG;
		break;
	}
	f->st->st_mode &= X_IPERM;
	f->st->st_mode |= f->type;
	switch (f->type)
	{
	case X_IFLNK:
		f->linkpath = paxstash(pax, &ap->stash.link, NiL, f->st->st_size);
		f->linktype = SOFTLINK;
		s = f->linkpath;
		while (paxread(pax, ap, s, (off_t)1, (off_t)1, 1) > 0)
		{
			f->st->st_size--;
			if (!*s++)
				break;
			if (!f->st->st_size)
			{
				*s = 0;
				break;
			}
		}
		break;
	default:
		f->linktype = NOLINK;
		break;
	}
	return 1;
}

static int
cpio_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
cpio_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	Localstat_t	lst;

	if (paxread(pax, ap, state.tmp.buffer, (off_t)0, (off_t)CPIO_HEADER, 0) <= 0)
		return 0;
	state.tmp.buffer[CPIO_HEADER] = 0;
	if (state.tmp.buffer[0] == '0' && sfsscanf(state.tmp.buffer, "%6o%6lo%6lo%6lo%6lo%6lo%6lo%6lo%11lo%6o%11I*o",
		&f->magic,
		&lst.dev,
		&lst.ino,
		&lst.mode,
		&lst.uid,
		&lst.gid,
		&lst.nlink,
		&lst.rdev,
		&lst.mtime,
		&f->namesize,
		sizeof(lst.size), &lst.size) == 11 && f->magic == CPIO_MAGIC)
	{
		f->st->st_dev = lst.dev;
		f->st->st_ino = lst.ino;
		f->st->st_mode = lst.mode;
		f->st->st_uid = lst.uid;
		f->st->st_gid = lst.gid;
		f->st->st_nlink = lst.nlink;
		IDEVICE(f->st, lst.rdev);
		f->st->st_mtime = lst.mtime;
		f->st->st_size = lst.size;
		return cpio_common(pax, ap, f, CPIO_ALIGN, CPIO_HEADER);
	}
	paxunread(pax, ap, state.tmp.buffer, CPIO_HEADER);
	return 0;
}

static int
cpio_getepilogue(Pax_t* pax, register Archive_t* ap)
{
	register Cpio_t*	cpio = (Cpio_t*)ap->data;

	if (!cpio->trailer)
	{
		error(2, "%s: %s format corrupt -- epilogue (%s) not found", ap->name, ap->format->name, CPIO_TRAILER);
		return -1;
	}
	return 0;
}

static int
cpio_putprologue(Pax_t* pax, Archive_t* ap, int append)
{
	return init(ap);
}

static int
cpio_putheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	if (state.complete)
		complete(ap, f, CPIO_HEADER + f->namesize);
#if CPIO_EXTENDED
	if (!f->skip && !state.strict)
	{
		getidnames(f);
		addxopstr(ap, 'U', f->uidname);
		addxopstr(ap, 'G', f->gidname);
		if (CPIO_TRUNCATE(idevice(f->st)) != idevice(f->st))
			addxopnum(ap, 'd', (Sflong_t)idevice(f->st));
#if NUMBER_EVEN_THOUGH_NAME
		if (CPIO_TRUNCATE(f->st->st_gid) != f->st->st_gid)
			addxopnum(ap, 'g', (Sflong_t)f->st->st_gid);
		if (CPIO_TRUNCATE(f->st->st_uid) != f->st->st_uid)
			addxopnum(ap, 'u', (Sflong_t)f->st->st_uid);
#endif
		if (f->st->st_size > 0x7fffffff)
			addxopnum(ap, 's', (Sflong_t)f->st->st_size);
		setxops(ap, f);
	}
	else
#endif
	{
		if (CPIO_TRUNCATE(idevice(f->st)) != idevice(f->st))
			error(1, "%s: special device number truncated", f->name);
		if (CPIO_TRUNCATE(f->st->st_gid) != f->st->st_gid)
			error(1, "%s: gid number truncated", f->name);
		if (CPIO_TRUNCATE(f->st->st_uid) != f->st->st_uid)
			error(1, "%s: uid number truncated", f->name);
	}
	sfsprintf(state.tmp.buffer, state.tmp.buffersize, "%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.11lo%0.6o%0.11I*o",
		(long)CPIO_TRUNCATE(CPIO_MAGIC),
		(long)CPIO_TRUNCATE(f->st->st_dev),
		(long)CPIO_TRUNCATE(f->st->st_ino),
		(long)CPIO_TRUNCATE(f->st->st_mode),
		(long)CPIO_TRUNCATE(f->st->st_uid),
		(long)CPIO_TRUNCATE(f->st->st_gid),
		(long)CPIO_TRUNCATE(f->st->st_nlink),
		(long)CPIO_TRUNCATE(idevice(f->st)),
		(long)f->st->st_mtime,
		(long)f->namesize,
		sizeof(intmax_t), (intmax_t)(f->st->st_size + (f->type == X_IFLNK ? f->linkpathsize : 0)));
	paxwrite(pax, ap, state.tmp.buffer, CPIO_HEADER);
#if CPIO_EXTENDED
	putxops(pax, ap, f);
#else
	paxwrite(pax, ap, f->name, f->namesize);
#endif
	if (f->type == X_IFLNK)
	{
		if (streq(f->name, f->linkpath))
			error(1, "%s: symbolic link loops to self", f->name);
		paxwrite(pax, ap, f->linkpath, f->linkpathsize);
		putdeltaheader(ap, f);
	}
	return 1;
}

static off_t
cpio_putepilogue(Pax_t* pax, Archive_t* ap)
{
	char	buf[sizeof(CPIO_TRAILER)];

	putinfo(ap, strcpy(buf, CPIO_TRAILER), ap->delta && !(ap->delta->format->flags & PSEUDO) ? ap->delta->index + 1 : 0, 0);
	return ap->io->unblocked ? BLOCKSIZE : state.blocksize;
}

static int
cpio_event(Pax_t* pax, Archive_t* ap, File_t* f, void* data, unsigned long event)
{
	off_t		n;

	switch (event)
	{
	case PAX_EVENT_BUG_19951031:
		/*
		 * compensate for a pre 19951031 pax bug
		 * that added linknamesize to st_size
		 */

		if (f->st->st_size == f->linkpathsize && paxread(pax, ap, state.tmp.buffer, (off_t)0, n = f->st->st_size + 6, 0) > 0)
		{
			paxunread(pax, ap, state.tmp.buffer, n);
			state.tmp.buffer[6] = 0;
			state.tmp.buffer[n] = 0;
			if (strtol(state.tmp.buffer, NiL, 8) == CPIO_MAGIC && strtol(state.tmp.buffer + f->st->st_size, NiL, 8) != CPIO_MAGIC)
			{
				f->st->st_size = 0;
				if (!ap->warnlinkhead)
				{
					ap->warnlinkhead = 1;
					error(1, "%s: compensating for invalid %s header hard link sizes", ap->name, ap->format->name);
				}
			}
		}
		return 1;
	case PAX_EVENT_DELTA_EXTEND:
		return 1;
	}
	return 0;
}

static int
asc_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	unsigned int		magic;

	if (size >= ASC_HEADER && buf[0] == '0' && sfsscanf((char*)buf, "%6o", &magic) == 1 && magic == ASC_MAGIC)
		return init(ap);
	return 0;
}

static int
asc_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	Localstat_t	lst;

	if (paxread(pax, ap, state.tmp.buffer, (off_t)0, (off_t)ASC_HEADER, 0) <= 0)
		return 0;
	state.tmp.buffer[ASC_HEADER] = 0;
	if (state.tmp.buffer[0] == '0' && sfsscanf(state.tmp.buffer, "%6o%8lx%8lx%8lx%8lx%8lx%8lx%8I*x%8lx%8lx%8lx%8lx%8x%8lx",
		&f->magic,
		&lst.ino,
		&lst.mode,
		&lst.uid,
		&lst.gid,
		&lst.nlink,
		&lst.mtime,
		sizeof(lst.size), &lst.size,
		&lst.dev_major,
		&lst.dev_minor,
		&lst.rdev_major,
		&lst.rdev_minor,
		&f->namesize,
		&lst.checksum) == 14 && f->magic == ASC_MAGIC)
	{
		f->checksum = lst.checksum;
		f->st->st_dev = makedev(lst.dev_major, lst.dev_minor);
		f->st->st_ino = lst.ino;
		f->st->st_mode = lst.mode;
		f->st->st_uid = lst.uid;
		f->st->st_gid = lst.gid;
		f->st->st_nlink = lst.nlink;
		IDEVICE(f->st, makedev(lst.rdev_major, lst.rdev_minor));
		f->st->st_mtime = lst.mtime;
		f->st->st_size = lst.size;
		return cpio_common(pax, ap, f, ASC_ALIGN, ASC_HEADER);
	}
	paxunread(pax, ap, state.tmp.buffer, ASC_HEADER);
	return 0;
}

static int
asc_putheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	int	n;

	if (state.complete)
		complete(ap, f, ASC_HEADER + f->namesize);
	if (ap->format->variant == ASC)
		f->checksum = 0;
	sfsprintf(state.tmp.buffer, state.tmp.buffersize, "%0.6lo%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx",
		(long)(ap->format->variant == ASC ? ASC_MAGIC : ASCHK_MAGIC),
		(long)f->st->st_ino,
		(long)f->st->st_mode,
		(long)f->st->st_uid,
		(long)f->st->st_gid,
		(long)f->st->st_nlink,
		(long)f->st->st_mtime,
		(long)f->st->st_size + (long)f->linkpathsize,
		(long)major(f->st->st_dev),
		(long)minor(f->st->st_dev),
		(long)major(idevice(f->st)),
		(long)minor(idevice(f->st)),
		(long)f->namesize,
		f->checksum);
	paxwrite(pax, ap, state.tmp.buffer, ASC_HEADER);
	paxwrite(pax, ap, f->name, f->namesize);
	if (n = (ASC_HEADER + f->namesize) % ASC_ALIGN)
		while (n++ < ASC_ALIGN)
			paxwrite(pax, ap, "", 1);
	return 1;
}

static int
aschk_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	unsigned int		magic;

	if (size >= ASC_HEADER && buf[0] == '0' && sfsscanf((char*)buf, "%6o", &magic) == 1 && magic == ASCHK_MAGIC)
		return init(ap);
	return 0;
}

static int
aschk_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	Localstat_t	lst;

	if (paxread(pax, ap, state.tmp.buffer, (off_t)0, (off_t)ASC_HEADER, 0) <= 0)
		return 0;
	state.tmp.buffer[ASC_HEADER] = 0;
	if (state.tmp.buffer[0] == '0' && sfsscanf(state.tmp.buffer, "%6o%8lx%8lx%8lx%8lx%8lx%8lx%8I*x%8lx%8lx%8lx%8lx%8x%8lx",
		&f->magic,
		&lst.ino,
		&lst.mode,
		&lst.uid,
		&lst.gid,
		&lst.nlink,
		&lst.mtime,
		sizeof(lst.size), &lst.size,
		&lst.dev_major,
		&lst.dev_minor,
		&lst.rdev_major,
		&lst.rdev_minor,
		&f->namesize,
		&lst.checksum) == 14 && f->magic == ASCHK_MAGIC)
	{
		f->checksum = lst.checksum;
		f->st->st_dev = makedev(lst.dev_major, lst.dev_minor);
		f->st->st_ino = lst.ino;
		f->st->st_mode = lst.mode;
		f->st->st_uid = lst.uid;
		f->st->st_gid = lst.gid;
		f->st->st_nlink = lst.nlink;
		IDEVICE(f->st, makedev(lst.rdev_major, lst.rdev_minor));
		f->st->st_mtime = lst.mtime;
		f->st->st_size = lst.size;
		return cpio_common(pax, ap, f, ASC_ALIGN, ASC_HEADER);
	}
	paxunread(pax, ap, state.tmp.buffer, ASC_HEADER);
	return 0;
}

static unsigned long
aschk_checksum(Pax_t* pax, Archive_t* ap, File_t* f, void* ab, size_t n, register unsigned long sum)
{
	register unsigned char* b = (unsigned char*)ab;
	register unsigned char*	e;

	e = b + n;
	while (b < e)
		sum += *b++;
	return sum;
}

/*
 * convert binary header shorts to long
 */

static long
binary_long(register unsigned short* s)
{
	Integral_t	u;

	u.l = 1;
	if (u.c[0])
	{
		u.s[0] = s[1];
		u.s[1] = s[0];
	}
	else
	{
		u.s[0] = s[0];
		u.s[1] = s[1];
	}
	return u.l;
}

/*
 * convert long to binary header shorts
 */

static void
binary_short(register unsigned short* s, long n)
{
	Integral_t	u;

	u.l = 1;
	if (u.c[0])
	{
		u.l = n;
		s[0] = u.s[1];
		s[1] = u.s[0];
	}
	else
	{
		u.l = n;
		s[0] = u.s[0];
		s[1] = u.s[1];
	}
}

static int
binary_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	short		magic = CPIO_MAGIC;
	int		swap;

	if (size >= BINARY_HEADER && (swap = swapop(&magic, buf, 2)) >= 0)
	{
		ap->swap = swap;
		return init(ap);
	}
	return 0;
}

static int
binary_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	Binary_header_t	hdr;

	if (paxread(pax, ap, &hdr, (off_t)BINARY_HEADER, (off_t)BINARY_HEADER, 0) <= 0)
		return 0;
	if (ap->swap)
	{
		memcpy(state.tmp.buffer, &hdr, BINARY_HEADER);
		swapmem(ap->swap, &hdr, &hdr, BINARY_HEADER);
	}
	f->magic = hdr.magic;
	if (f->magic == CPIO_MAGIC)
	{
		f->namesize = hdr.namesize;
		f->st->st_dev = hdr.dev;
		f->st->st_ino = hdr.ino;
		f->st->st_mode = hdr.mode;
		f->st->st_uid = hdr.uid;
		f->st->st_gid = hdr.gid;
		f->st->st_nlink = hdr.links;
		IDEVICE(f->st, hdr.rdev);
		f->st->st_mtime = binary_long(hdr.mtime);
		f->st->st_size = binary_long(hdr.size);
		return cpio_common(pax, ap, f, CPIO_ALIGN, CPIO_HEADER);
	}
	paxunread(pax, ap, state.tmp.buffer, CPIO_HEADER);
	return 0;
}

static int
binary_putheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	Binary_header_t	hdr;
	int		n;

	if (state.complete)
		complete(ap, f, BINARY_HEADER + f->namesize);
	hdr.magic = CPIO_MAGIC;
	hdr.namesize = f->namesize;
	binary_short(hdr.size, f->st->st_size + (f->type == X_IFLNK ? f->linkpathsize : 0));
	hdr.dev = f->st->st_dev;
	hdr.ino = f->st->st_ino;
	hdr.mode = f->st->st_mode;
	hdr.uid = f->st->st_uid;
	hdr.gid = f->st->st_gid;
	hdr.links = f->st->st_nlink;
	hdr.rdev = idevice(f->st);
	if (hdr.rdev != idevice(f->st))
		error(1, "%s: special device numbers truncated", f->name);
	binary_short(hdr.mtime, (long)f->st->st_mtime);
	paxwrite(pax, ap, &hdr, BINARY_HEADER);
	paxwrite(pax, ap, f->name, f->namesize);
	if (n = (BINARY_HEADER + f->namesize) % BINARY_ALIGN)
		while (n++ < BINARY_ALIGN)
			paxwrite(pax, ap, "", 1);
	if (f->type == X_IFLNK)
	{
		if (streq(f->name, f->linkpath))
			error(1, "%s: symbolic link loops to self", f->name);
		paxwrite(pax, ap, f->linkpath, f->linkpathsize);
		putdeltaheader(ap, f);
	}
	return 1;
}

static int
binary_validate(Pax_t* pax, Archive_t* ap, File_t* f)
{
	if (f->namesize > (CPIO_NAMESIZE + 1))
	{
		error(2, "%s: file name too long", f->name);
		return 0;
	}
	return 1;
}

Format_t	pax_asc_format =
{
	"asc",
	0,
	"s5r4 extended cpio character",
	ASC,
	ARCHIVE|IN|OUT|APPEND,
	DEFBUFFER,
	DEFBLOCKS,
	4,
	pax_asc_next,
	0,
	cpio_done,
	asc_getprologue,
	asc_getheader,
	0,
	0,
	cpio_getepilogue,
	cpio_putprologue,
	asc_putheader,
	0,
	0,
	cpio_putepilogue,
	0,
	0,
	0
};

Format_t	pax_aschk_format =
{
	"aschk",
	0,
	"s5r4 extended cpio character with checksum",
	ASCHK,
	ARCHIVE|IN|OUT|APPEND,
	DEFBUFFER,
	DEFBLOCKS,
	4,
	pax_aschk_next,
	0,
	cpio_done,
	aschk_getprologue,
	aschk_getheader,
	0,
	0,
	cpio_getepilogue,
	cpio_putprologue,
	asc_putheader,
	0,
	0,
	cpio_putepilogue,
	0,
	0,
	aschk_checksum
};

Format_t	pax_binary_format =
{
	"binary",
	"binary-cpio",
	"cpio binary with symlinks",
	BINARY,
	ARCHIVE|IN|OUT|APPEND,
	DEFBUFFER,
	DEFBLOCKS,
	2,
	pax_binary_next,
	0,
	cpio_done,
	binary_getprologue,
	binary_getheader,
	0,
	0,
	cpio_getepilogue,
	cpio_putprologue,
	binary_putheader,
	0,
	0,
	cpio_putepilogue,
	0,
	0,
	0,
	binary_validate
};

Format_t	pax_cpio_format =
{
	"cpio",
	0,
	"cpio character with symlinks",
	CPIO,
	ARCHIVE|IN|OUT|APPEND,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(cpio),
	0,
	cpio_done,
	cpio_getprologue,
	cpio_getheader,
	0,
	0,
	cpio_getepilogue,
	cpio_putprologue,
	cpio_putheader,
	0,
	0,
	cpio_putepilogue,
	0,
	0,
	0,
	0,
	cpio_event,
	PAX_EVENT_BUG_19951031|PAX_EVENT_DELTA_EXTEND
};

PAXLIB(cpio)
