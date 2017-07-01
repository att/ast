/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2014 AT&T Intellectual Property          *
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
 * pax { pax tar ustar } formats
 */

#include "format.h"

#undef	SOKTYPE

#define TARTYPE         (-1)

#define PAX		1
#define OLD		2
#define TAR		3

#define TAR_HEADER	TBLOCK
#define TAR_LARGENUM	0200
#define TAR_SUMASK	((1L<<(sizeof(tar->header.chksum)-1)*3)-1)

#define TARSIZEOF(m)	sizeof(((Tarheader_t*)&state)->m)

typedef struct Tar_s
{
	Tarheader_t	header;
	Tarheader_t	last[16];	/* last n headers		*/
	int		lastindex;	/* last[] index			*/
} Tar_t;

/*
 * output extended header keyword assignment record
 */

void
putkey(Archive_t* ap, Sfio_t* sp, Option_t* op, const char* value, Sfulong_t number)
{
	register size_t		n;
	register size_t		m;
	register size_t		o;

	if (value)
		n = strlen(value);
	else
	{
		n = sfprintf(ap->tmp.key, "%I*u", sizeof(number), number);
		sfstrseek(ap->tmp.key, 0, SEEK_SET);
	}
	message((-5, "putkey %s=%s", op->name, value ? value : sfstrseek(ap->tmp.key, 0, SEEK_CUR)));
	n += strlen(op->name) + 3 + ((op->flags & OPT_VENDOR) ? sizeof(VENDOR) : 0);
	o = 0;
	for (;;)
	{
		sfprintf(ap->tmp.key, "%I*u", sizeof(n), n);
		m = sfstrtell(ap->tmp.key);
		sfstrseek(ap->tmp.key, 0, SEEK_SET);
		if (m == o)
			break;
		n += m - o;
		o = m;
	}
	sfprintf(sp, "%I*u ", sizeof(n), n);
	if (op->flags & OPT_VENDOR)
		sfprintf(sp, "%s.", VENDOR);
	sfprintf(sp, "%s=", op->name, value);
	if (value)
		sfprintf(sp, "%s", value);
	else
		sfprintf(sp, "%I*u", sizeof(number), number);
	sfputc(sp, '\n');
}

/*
 * compute tar->header checksum
 */

static unsigned long
tar_checksum(Archive_t* ap, int check, unsigned long sum)
{
	register Tar_t*			tar = (Tar_t*)ap->data;
	register unsigned char*		p;
	register unsigned char*		e;
	register unsigned char*		t;
	register unsigned long		u;
	register unsigned long		s;
	register long			c;
	register const unsigned char*	map;
	unsigned char			tmp[TARSIZEOF(chksum)];

	p = (unsigned char*)tar->header.chksum;
	e = p + TARSIZEOF(chksum);
	t = tmp;
	while (p < e)
	{
		*t++ = *p;
		*p++ = ' ';
	}
	u = 0;
	s = 0;
	p = (unsigned char*)&tar->header;
	e = p + TAR_HEADER;
	if (!ap->convert[SECTION_CONTROL].on)
		while (p < e)
		{
			c = *p++;
			u += c;
			if (check)
			{
				if (c & 0x80)
					c |= (-1) << 8;
				s += c;
			}
		}
	else
	{
		map = ((ap->mio.mode & O_ACCMODE) == O_WRONLY) ? ap->convert[SECTION_CONTROL].f2t : ap->convert[SECTION_CONTROL].t2f;
		while (p < e)
		{
			c = map[*p++];
			u += c;
			if (check)
			{
				if (c & 0x80)
					c |= (-1) << 8;
				s += c;
			}
		}
	}
	p = (unsigned char*)tar->header.chksum;
	e = p + TARSIZEOF(chksum);
	t = tmp;
	while (p < e)
		*p++ = *t++;
	u &= TAR_SUMASK;
	if (check)
	{
		if ((sum &= TAR_SUMASK) == u)
			return 1;
		if (sum == (s &= TAR_SUMASK))
		{
			if (!ap->old.warned)
			{
				ap->old.warned = 1;
				error(1, "%s: %s format archive generated with signed checksums", ap->name, ap->format->name);
			}
			return 1;
		}
		if (ap->entry > 1)
		{
			if (s != u)
				error(state.keepgoing ? 1 : 3, "%s: %s format checksum error (%ld != %ld or %ld)", ap->name, ap->format->name, sum, u, s);
			else
				error(state.keepgoing ? 1 : 3, "%s: %s format checksum error (%ld != %ld)", ap->name, ap->format->name, sum, u);
			return state.keepgoing;
		}
		return 0;
	}
	return u;
}

/*
 * check if tar file name may be split to fit in header
 * return
 *	<0	extension header required with error message for non-extension formats
 *	 0	no split needed
 *	>0	f->name offset to split
 */

static int
tar_longname(Archive_t* ap, File_t* f)
{
	register char*	s;
	register char*	b;
	register int	n;

	if (!(n = f->namesize) || --n <= TARSIZEOF(name))
		return 0;
	switch (ap->format->variant)
	{
	case PAX:
	case TAR:
		if (n > (TARSIZEOF(prefix) + TARSIZEOF(name) + 1))
		{
			if (ap->format->variant != PAX && state.strict)
				goto toolong;
			f->longname = 1;
			return -1;
		}
		s = f->name + n;
		b = s - TARSIZEOF(name) - 1;
		while (--s >= b)
			if (*s == '/' && (n = s - f->name) <= TARSIZEOF(prefix))
			{
				if (!n)
					break;
				return s - f->name;
			}
		if (ap->format->variant != PAX && state.strict)
		{
			error(2, "%s: file base name too long -- %d max", f->name, TARSIZEOF(name));
			f->skip = 1;
		}
		else
			f->longname = 1;
		return -1;
	case OLD:
	toolong:
		error(2, "%s: file name too long -- %d max", f->name, TARSIZEOF(prefix) + TARSIZEOF(name) + !!strchr(f->name, '/'));
		f->skip = 1;
		return -1;
	}
	return 0;
}

/*
 * check if f->linkpath is too long
 * return
 *	<0	extension header required with error message for non-extension formats
 *	 0	it fits
 */

static int
tar_longlink(Archive_t* ap, File_t* f)
{
	if (f->linktype != NOLINK && strlen(f->linkpath) > TARSIZEOF(linkname))
	{
		switch (ap->format->variant)
		{
		case TAR:
			if (!state.strict)
				break;
			/*FALLTHROUGH*/
		case OLD:
			error(2, "%s: link name too long -- %d max", f->linkpath, TARSIZEOF(linkname));
			f->skip = 1;
			return -1;
		}
		f->longlink = 1;
		return -1;
	}
	return 0;
}

/*
 * generate extended header path name from fmt
 */

static char*
headname(Archive_t* ap, File_t* f, const char* fmt)
{
	char*		s;
	char*		t;
	size_t		n;

	if (!ap->tmp.hdr && !(ap->tmp.hdr = sfstropen()))
		nospace();
	listprintf(ap->tmp.hdr, ap, f, fmt);
	if (sfstrtell(ap->tmp.hdr) > TARSIZEOF(name))
	{
		sfstrseek(ap->tmp.hdr, 0, SEEK_SET);
		s = f->name;
		if (t = strrchr(s, '/'))
			f->name = t + 1;
		if ((n = strlen(f->name)) > TARSIZEOF(name) / 2)
			f->name = f->name + n - TARSIZEOF(name) / 2;
		listprintf(ap->tmp.hdr, ap, f, fmt);
		f->name = s;
	}
	if (!(s = sfstruse(ap->tmp.hdr)))
		nospace();
	return s;
}

/*
 * synthesize an extended tar header
 */

static void
synthesize(Archive_t* ap, File_t* f, char* name, int type, char* buf, size_t n)
{
	Buffer_t*	bp;
	char*		base;
	char*		next;
	File_t		h;
	struct stat	st;

	ap->sum -= 2;
	initfile(ap, &h, &st, name, X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH);
	h.extended = type;
	h.st->st_gid = f->st->st_gid;
	h.st->st_uid = f->st->st_uid;
	h.st->st_mtime = NOW;
	h.st->st_size = n;
	h.fd = setbuffer(0);
	bp = getbuffer(h.fd);
	base = bp->base;
	next = bp->next;
	bp->base = bp->next = buf;
	fileout(ap, &h);
	bp->base = base;
	bp->next = next;
	ap->sum += 2;
}

/*
 * output an extended tar header
 * some keys may already be in the header buffer for type
 */

static int
extend(Archive_t* ap, File_t* f, int type)
{
	unsigned long		n;
	char*			s;
	char*			fmt;
	Hash_position_t*	pos;
	Option_t*		op;
	Sfio_t*			sp;
	Value_t*		vp;
	int			lev;
	int			alt;
	int			split;
	Tv_t			tv;
	File_t			h;
	struct stat		st;
	char			num[64];

	if (!f)
		initfile(ap, f = &h, &st, ".", X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH);
	switch (type)
	{
	case EXTTYPE:
	case TARTYPE:
		sp = ap->tmp.extended;
		fmt = state.header.extended;
		lev = 7;
		alt = 4;
		break;
	case GLBTYPE:
		sp = ap->tmp.global;
		fmt = state.header.global;
		lev = 3;
		alt = 0;
		break;
	default:
		return 0;
	}
	if (pos = hashscan(state.options, 0))
	{
		while (hashnext(pos))
		{
			op = (Option_t*)pos->bucket->value;
			if ((op->flags & (OPT_HEADER|OPT_READONLY)) == OPT_HEADER && op->name == pos->bucket->name && (op->level == lev || op->level == alt))
			{
				message((-5, "extend %s level=%d:%d:%d entry=%d:%d perm=(%s,%I*d) temp=(%s,%I*d)", op->name, op->level, lev, alt, op->entry, ap->entry, op->perm.string, sizeof(op->perm.number), op->perm.number, op->temp.string, sizeof(op->temp.number), op->temp.number));
				vp = &op->perm;
				s = vp->string;
				switch (op->index)
				{
				case OPT_atime:
				case OPT_ctime:
				case OPT_mtime:
					if (op->flags & OPT_SET)
						s = vp->string;
					else if (type != EXTTYPE)
						continue;
					else
					{
						switch (op->index)
						{
						case OPT_atime:
							tvgetatime(&tv, f->st);
							break;
						case OPT_mtime:
							tvgetmtime(&tv, f->st);
							break;
						case OPT_ctime:
							tvgetctime(&tv, f->st);
							break;
						}
						if (!tv.tv_nsec && op->index == OPT_mtime)
							continue;
						s = num + sfsprintf(num, sizeof(num), "%lu.%09lu", tv.tv_sec, tv.tv_nsec);
						while (*(s - 1) == '0')
							s--;
						if (*(s - 1) == '.')
							*s++ = '0';
						*s = 0;
						s = num;
					}
					break;
				case OPT_comment:
					s = state.header.comment;
					break;
				case OPT_gid:
					if (op->flags & OPT_SET)
						f->st->st_gid = vp->number;
					else
					{
						if (type != EXTTYPE)
							f->st->st_gid = state.gid;
						if ((uint32_t)f->st->st_gid <= (unsigned long)07777777)
							continue;
					}
					sfsprintf(s = num, sizeof(num), "%I*u", sizeof(f->st->st_gid), f->st->st_gid);
					break;
				case OPT_gname:
					if (op->flags & OPT_SET)
						f->gidname = s;
					else if (!f->gidname || strlen(f->gidname) < TARSIZEOF(gname) && portable(ap, f->gidname))
						continue;
					s = f->gidname;
					break;
				case OPT_size:
#if _typ_int64_t
					if (type == GLBTYPE)
						continue;
					if (!(f->st->st_size >> 32) && !(state.test & 1))
						continue;
					state.test &= ~1;
					sfsprintf(s = num, sizeof(num), "%I*u", sizeof(f->st->st_size), f->st->st_size);
					break;
#else
					continue;
#endif
				case OPT_uid:
					if (op->flags & OPT_SET)
						f->st->st_uid = vp->number;
					else
					{
						if (type != EXTTYPE)
							f->st->st_uid = state.uid;
						if ((unsigned long)f->st->st_uid <= (unsigned long)07777777)
							continue;
					}
					sfsprintf(s = num, sizeof(num), "%I*u", sizeof(f->st->st_uid), f->st->st_uid);
					break;
				case OPT_uname:
					if (op->flags & OPT_SET)
						f->uidname = s;
					else if (!f->uidname || strlen(f->uidname) < TARSIZEOF(uname) && portable(ap, f->uidname))
						continue;
					s = f->uidname;
					break;
				default:
					if (!s && (op->flags & OPT_SET))
						sfsprintf(s = num, sizeof(num), "%ld", vp->number);
					break;
				}
				if (s && type != TARTYPE)
					putkey(ap, sp, op, s, 0);
			}
		}
		hashdone(pos);
	}
        if (type == TARTYPE)
                split = 0;
        else
        {
		if (type == EXTTYPE)
		{
			if ((split = tar_longname(ap, f)) < 0 || !portable(ap, f->name))
				putkey(ap, sp, &options[OPT_path], f->name, 0);
			if (f->linkpath && (tar_longlink(ap, f) < 0 || !portable(ap, f->linkpath)))
				putkey(ap, sp, &options[OPT_linkpath], f->linkpath, 0);
		}
		else
			split = 0;
		if (type && (n = sfstrtell(sp)))
		{
			if (!(s = sfstruse(sp)))
				nospace();
			synthesize(ap, f, headname(ap, f, fmt), type, s, n);
		}
	}
	return split;
}

static int
tar_getoctal(const char* f, const char* p, size_t n, int z, void* r)
{
	register unsigned char*	s = (unsigned char*)p;
	register unsigned char*	e = s + n;
	register uintmax_t	v = 0;

	while (s < e && *s == ' ')
		s++;
	while (s < e && *s >= '0' && *s <= '7')
		v = (v << 3) + (*s++ - '0');
	if (s < e)
	{
		if (*s == 0x80 || *s == 0xff)
		{
			v = *s++ == 0x80 ? 0 : 0xff;
			e = (unsigned char*)p + 8;
			while (s < e)
				v = (v << 8) + *s++;
		}
		else if (*s && *s != ' ')
			return -1;
	}
	switch (z)
	{
	case 1:
		*(unsigned char*)r = (unsigned char)v;
		break;
	case 2:
		*(uint16_t*)r = (uint16_t)v;
		break;
	case 4:
		*(uint32_t*)r = (uint32_t)v;
		break;
#ifdef _ast_int8_t
	case 8:
		*(uint64_t*)r = (uint64_t)v;
		break;
#endif
	}
	return 0;
}

static int
tar_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	register Tar_t*	tar = (Tar_t*)ap->data;
	char*		s;
	char*		t;
	Format_t*	tp;
	Option_t*	op;
	off_t		n;
	long		num;
	int		i;
	int		m;

 again:
	if (paxread(pax, ap, &tar->header, (off_t)0, (off_t)TAR_HEADER, 0) <= 0)
		return 0;
	if (!*tar->header.name)
	{
		if (ap->entry == 1)
			goto nope;
		return 0;
	}
	if (tar_getoctal("mode", tar->header.mode, 7, sizeof(f->st->st_mode), &f->st->st_mode))
		goto nope;
	if (tar_getoctal("uid", tar->header.uid, 7, sizeof(f->st->st_uid), &f->st->st_uid))
		goto nope;
	if (tar_getoctal("gid", tar->header.gid, 7, sizeof(f->st->st_gid), &f->st->st_gid))
		goto nope;
	if (tar_getoctal("mtime", tar->header.mtime, 11, sizeof(f->st->st_mtime), &f->st->st_mtime))
		goto nope;
	if (tar_getoctal("chksum", tar->header.chksum, 7, sizeof(num), &num))
		goto nope;
	if (!tar_checksum(ap, 1, num) && ap->entry == 1)
	{
		if (!ap->swapio)
		{
			char	tmp[TARSIZEOF(chksum) + 1];

			tmp[TARSIZEOF(chksum)] = 0;
			for (i = 1; i < 4; i++)
			{
				memcpy(tmp, tar->header.chksum, TARSIZEOF(chksum));
				swapmem(i, tmp, tmp, TARSIZEOF(chksum));
				if (!tar_getoctal("chksum", tar->header.chksum, 7, sizeof(num), &num) && tar_checksum(ap, 1, num))
				{
					ap->swapio = i;
					paxunread(pax, ap, &tar->header, TAR_HEADER);
					goto again;
				}
			}
		}
		goto nope;
	}
	if (tar_getoctal("size", tar->header.size, 11, sizeof(f->st->st_size), &f->st->st_size))
		goto nope;
	if (ap->format->variant != OLD)
	{
		if (!streq(tar->header.magic, TMAGIC))
		{
			if (streq(tar->header.magic, TMAGIC "  "))
				/* old gnu tar */;
			else if (ap->entry > 1)
				goto nope;
			ap->format = &pax_tar_format;
		}
		else if (!strneq(tar->header.version, TVERSION, TARSIZEOF(version)))
		{
			tp = &pax_tar_format;
			error(1, "%s: %s format version %-.*s incompatible with implementation version %-.*s -- assuming %s", ap->name, ap->format->name, TARSIZEOF(version), tar->header.version, TARSIZEOF(version), TVERSION, tp->name);
			ap->format = tp;
		}
	}
	*((char*)tar->header.name + TARSIZEOF(name)) = 0;
	if (ap->format->variant != OLD && *tar->header.prefix)
	{
		f->name = paxstash(pax, &ap->stash.head, NiL, TARSIZEOF(prefix) + TARSIZEOF(name) + 2);
		sfsprintf(f->name, TARSIZEOF(prefix) + TARSIZEOF(name) + 2, "%-.*s/%-.*s", TARSIZEOF(prefix), tar->header.prefix, TARSIZEOF(name), tar->header.name);
	}
	else
		f->name = tar->header.name;
	*((char*)tar->header.linkname + TARSIZEOF(name)) = 0;
	f->linktype = NOLINK;
	f->linkpath = 0;
	f->st->st_nlink = 1;
	switch (tar->header.typeflag)
	{
	case LNKTYPE:
		f->linktype = HARDLINK;
		f->st->st_mode |= X_IFREG;
		f->st->st_nlink = 2;
		if (!ap->delta)
			f->st->st_size = 0;
		f->linkpath = paxstash(pax, &ap->stash.link, tar->header.linkname, 0);
		break;
	case SYMTYPE:
		f->linktype = SOFTLINK;
		f->st->st_mode |= X_IFLNK;
		f->linkpath = paxstash(pax, &ap->stash.link, tar->header.linkname, 0);
		break;
	case CHRTYPE:
		f->st->st_mode |= X_IFCHR;
	device:
		if (tar_getoctal("devmajor", tar->header.devmajor, 7, sizeof(i), &i))
			goto nope;
		if (tar_getoctal("devminor", tar->header.devminor, 7, sizeof(m), &m))
			goto nope;
		IDEVICE(f->st, makedev(i, m));
		break;
	case BLKTYPE:
		f->st->st_mode |= X_IFBLK;
		goto device;
	case DIRTYPE:
		f->st->st_mode |= X_IFDIR;
		break;
	case FIFOTYPE:
		f->st->st_mode |= X_IFIFO;
		break;
#ifdef SOKTYPE
	case SOKTYPE:
		f->st->st_mode |= X_IFSOCK;
		break;
#endif
	case EXTTYPE:
	case GLBTYPE:
		ap->format = &pax_pax_format;
		if (f->st->st_size > 0)
		{
			ap->section = SECTION_DATA;
			if (!(s = paxget(pax, ap, f->st->st_size, NiL)))
				error(3, "invalid %s format '%c' extended header", ap->format->name, tar->header.typeflag);
			s[f->st->st_size - 1] = 0;
			setoptions(s, f->st->st_size, NiL, state.usage, ap, tar->header.typeflag);
		}
		ap->sum -= 2;
		f->extended = tar->header.typeflag;
		gettrailer(ap, f);
		f->extended = 0;
		ap->sum += 3;
		deltacheck(ap, NiL);
		goto again;

	case LLNKTYPE:
	case LREGTYPE:
		if ((n = f->st->st_size) > 0)
		{
			if (!(s = paxget(pax, ap, n, NiL)))
			{
				error(2, "%s: invalid %s format long path header", ap->name, ap->format->name);
				return 0;
			}
			op = &options[tar->header.typeflag == LLNKTYPE ? OPT_linkpath : OPT_path];
			op->level = 6;
			op->entry = ap->entry;
			paxstash(pax, &op->temp, s, (size_t)n);
		}
		gettrailer(ap, f);
		goto again;

	case VERTYPE:
		error(1, "version file archive members not supported -- regular file assumed");
		goto regular;

	default:
		error(1, "%s: %s: unknown %s format file type `%c' -- regular file assumed", ap->name, f->name, ap->format->name, tar->header.typeflag);
		/*FALLTHROUGH*/
	case REGTYPE:
	case AREGTYPE:
	case CONTTYPE:
	regular:
		f->st->st_mode |= X_IFREG;
		break;
	}
	f->uidname = 0;
	f->gidname = 0;
	if (ap->format->variant != OLD)
	{
		if (*tar->header.uname && (strtoll(tar->header.uname, &t, 0), *t))
			f->uidname = tar->header.uname;
		if (*tar->header.gname && (strtoll(tar->header.gname, &t, 0), *t))
			f->gidname = tar->header.gname;
	}
	if (++tar->lastindex >= elementsof(tar->last))
		tar->lastindex = 0;
	tar->last[tar->lastindex] = tar->header;
	return 1;
 nope:
	paxunread(pax, ap, &tar->header, TAR_HEADER);
	return 0;
}

static int
tar_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	int	n;

	if (!(ap->data = newof(0, Tar_t, 1, 0)))
		nospace();
	ap->format = fp;
	if ((n = tar_getheader(pax, ap, f)) > 0)
		ap->peek = 1;
	else
	{
		free(ap->data);
		ap->data = 0;
		ap->format = 0;
	}
	return n;
}

static int
tar_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
tar_putprologue(Pax_t* pax, register Archive_t* ap, int append)
{
	if (!(ap->data = newof(0, Tar_t, 1, 0)))
		nospace();
	return 0;
}

static int
tar_putheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	register Tar_t*	tar = (Tar_t*)ap->data;
	register char*	s;
	off_t		n;
	int		i;

	if (f->extended)
		i = 0;
	else
        {
		switch (ap->format->variant)
		{
		case PAX:
			i = extend(ap, f, EXTTYPE);
			break;
		case TAR:
			(void)extend(ap, f, TARTYPE);
			if ((i = tar_longname(ap, f)) < 0)
			{
				if (state.strict)
				{
					ap->entry--;
					return 0;
				}
				synthesize(ap, f, headname(ap, f, "@PaxPathText.%(sequence)s"), LREGTYPE, f->name, f->namesize);
			}
			if (tar_longlink(ap, f) < 0)
			{
				if (state.strict)
				{
					ap->entry--;
					return 0;
				}
				synthesize(ap, f, headname(ap, f, "@PaxLinkText.%(sequence)s"), LLNKTYPE, f->linkpath, strlen(f->linkpath) + 1);
			}
			break;
		case OLD:
			(void)extend(ap, f, TARTYPE);
			if ((i = tar_longname(ap, f)) || tar_longlink(ap, f))
			{
				ap->entry--;
				return 0;
			}
			break;
		}
        }
	if (state.complete)
		complete(ap, f, (ap->format->variant == PAX ? 4 : 1) * TAR_HEADER);
	memzero(&tar->header, TAR_HEADER);
	if (f->longname)
		s = headname(ap, f, "@PaxPathFile.%(sequence)s");
	else
	{
		if (i)
		{
			memcpy(tar->header.prefix, f->name, i);
			i++;
		}
		s = f->name + i;
	}
	sfsprintf(tar->header.name, TARSIZEOF(name) + 1, "%s%s", s, f->type == X_IFDIR ? "/" : "");
	i = 0;
	if (f->extended)
		tar->header.typeflag = f->extended;
	else
		switch (f->linktype)
		{
		case HARDLINK:
			tar->header.typeflag = LNKTYPE;
		linked:
			sfsprintf(tar->header.linkname, TARSIZEOF(linkname) + 1, "%s", f->longlink ? headname(ap, f, "@PaxLinkFile.%(sequence)s") : f->linkpath);
			break;
		case SOFTLINK:
			tar->header.typeflag = SYMTYPE;
			goto linked;
		default:
			switch (ap->format->variant == OLD ? X_IFREG : f->type)
			{
			case X_IFCHR:
				tar->header.typeflag = CHRTYPE;
				i = 1;
				break;
			case X_IFBLK:
				tar->header.typeflag = BLKTYPE;
				i = 1;
				break;
			case X_IFDIR:
				tar->header.typeflag = DIRTYPE;
				break;
			case X_IFIFO:
				tar->header.typeflag = FIFOTYPE;
				break;
#ifdef SOKTYPE
			case X_IFSOCK:
				tar->header.typeflag = SOKTYPE;
				i = 1;
				break;
#endif
			default:
				if (!f->skip && !f->delta.op)
					error(1, "%s: %s: unknown file type %07o -- regular file assumed", ap->name, f->name, f->type);
				/*FALLTHROUGH*/
			case X_IFREG:
				tar->header.typeflag = REGTYPE;
				break;
			}
			break;
		}
	sfsprintf(tar->header.devmajor, TARSIZEOF(devmajor), "%0*o ", TARSIZEOF(devmajor) - 1, i ? major(idevice(f->st)) : 0);
	sfsprintf(tar->header.devminor, TARSIZEOF(devminor), "%0*o ", TARSIZEOF(devminor) - 1, i ? minor(idevice(f->st)) : 0);
	sfsprintf(tar->header.mode, TARSIZEOF(mode), "%0*o ", TARSIZEOF(mode) - 1, f->st->st_mode & X_IPERM);
	sfsprintf(tar->header.uid, TARSIZEOF(uid), "%0*lo ", TARSIZEOF(uid) - 1, f->st->st_uid & (unsigned long)07777777);
	sfsprintf(tar->header.gid, TARSIZEOF(gid), "%0*lo ", TARSIZEOF(gid) - 1, f->st->st_gid & (unsigned long)07777777);
	if (ap->format->variant != PAX && (unsigned long)f->st->st_size > (unsigned long)037777777777)
	{
		tar->header.size[0] = TAR_LARGENUM;
		n = f->st->st_size;
		for (i = 11; i > 0; i--)
		{
			tar->header.size[i] = n & 0377;
			n >>= 8;
		}
	}
	else
		sfsprintf(tar->header.size, TARSIZEOF(size), "%0*lo ", TARSIZEOF(size) - 1, (long)f->st->st_size);
	sfsprintf(tar->header.mtime, TARSIZEOF(mtime), "%0*lo ", TARSIZEOF(mtime) - 1, f->st->st_mtime & (unsigned long)037777777777);
	if (ap->format->variant != OLD)
	{
		strncpy(tar->header.magic, TMAGIC, TARSIZEOF(magic));
		strncpy(tar->header.version, TVERSION, TARSIZEOF(version));
		getidnames(f);
		strcpy(tar->header.uname, f->uidname);
		strcpy(tar->header.gname, f->gidname);
	}
	sfsprintf(tar->header.chksum, TARSIZEOF(chksum), "%0*lo ", TARSIZEOF(chksum) - 1, tar_checksum(ap, 0, 0));
	paxwrite(pax, ap, &tar->header, TAR_HEADER);
	return 1;
}

static off_t
tar_putepilogue(Pax_t* pax, Archive_t* ap)
{
	register Tar_t*	tar = (Tar_t*)ap->data;

	memzero(&tar->header, TAR_HEADER);
	paxwrite(pax, ap, &tar->header, TAR_HEADER);
	paxwrite(pax, ap, &tar->header, TAR_HEADER);
	return ap->io->unblocked ? BLOCKSIZE : state.blocksize;
}

static int
tar_lookup(Pax_t* pax, Archive_t* ap, File_t* f, int index, char** s, Sflong_t* n)
{
	register Tar_t*	tar = (Tar_t*)ap->data;

	switch (index)
	{
	case OPT_chksum:
		*s = tar->header.chksum;
		break;
	case OPT_magic:
		*s = tar->header.magic;
		break;
	case OPT_typeflag:
		*n = tar->header.typeflag;
		break;
	case OPT_version:
		*s = tar->header.version;
		break;
	default:
		return 0;
	}
	return 1;
}

static int
tar_event(Pax_t* pax, Archive_t* ap, File_t* f, void* data, unsigned long event)
{
	register Tarheader_t*	hdr;
	unsigned long		sum;

	switch (event)
	{
	case PAX_EVENT_DELTA_EXTEND:
		putkey(ap, ap->tmp.global, &options[OPT_delta_index], NiL, ap->delta->index + 1);
		extend(ap, NiL, GLBTYPE);
		return 1;
	case PAX_EVENT_SKIP_JUNK:
		hdr = (Tarheader_t*)data;
		if (!isdigit(hdr->chksum[0]) || !isdigit(hdr->chksum[1]) || !isdigit(hdr->chksum[2]) || !isdigit(hdr->chksum[3]) || !isdigit(hdr->chksum[4]) || !isdigit(hdr->chksum[5]) || !isdigit(hdr->chksum[6]) || tar_getoctal("chksum", hdr->chksum, 7, sizeof(sum), &sum) || !tar_checksum(ap, -1, sum))
			return 1;
		return 0;
	}
	return 0;
}

static int
pax_putprologue(Pax_t* pax, register Archive_t* ap, int append)
{
	Format_t*		fp;
	Hash_position_t*	hp;
	Option_t*		op;
	Delta_format_t*		dp;

	if (!(ap->data = newof(0, Tar_t, 1, 0)) || !(ap->tmp.global = sfstropen()) || !(ap->tmp.extended = sfstropen()) || !(ap->tmp.key = sfstropen()))
		nospace();
	if (append)
		return 0;
	if (hp = hashscan(state.options, 0))
	{
		while (hashnext(hp))
		{
			op = (Option_t*)hp->bucket->value;
			if ((op->flags & OPT_GLOBAL) && op->name == hp->bucket->name && (op->level == 3 || op->level == 0) && op->perm.string)
				putkey(ap, ap->tmp.global, op, op->perm.string, 0);
		}
		hashdone(hp);
	}
	if (ap->delta && !(ap->delta->format->flags & PSEUDO))
	{
		fp = ap->delta->format;
		putkey(ap, ap->tmp.global, &options[OPT_delta_method], fp->name, 0);
		if ((dp = (Delta_format_t*)fp->data) && dp->variant)
			putkey(ap, ap->tmp.global, &options[OPT_delta_version], dp->variant, 0);
		if (ap->delta->compress)
			putkey(ap, ap->tmp.global, &options[OPT_delta_compress], 0, 1);
		if (ap->delta->base)
		{
			putkey(ap, ap->tmp.global, &options[OPT_delta_base_size], NiL, ap->delta->base->size);
			putkey(ap, ap->tmp.global, &options[OPT_delta_base_checksum], NiL, ap->delta->base->checksum & 0xffffffff);
		}
	}
	extend(ap, NiL, GLBTYPE);
	return 1;
}

Format_t	pax_tar_format =
{
	"oldtar",
	0,
	"pre-POSIX tar with symlinks",
	OLD,
	ARCHIVE|LINKTYPE|SLASHDIR|IN|OUT|APPEND,
	DEFBUFFER,
	DEFBLOCKS,
	BLOCKSIZE,
	PAXNEXT(tar),
	0,
	tar_done,
	tar_getprologue,
	tar_getheader,
	0,
	0,
	0,
	tar_putprologue,
	tar_putheader,
	0,
	0,
	tar_putepilogue,
	0,
	0,
	0,
	0,
	tar_event,
	PAX_EVENT_SKIP_JUNK
};

Format_t	pax_pax_format =
{
	"pax",
	0,
	"POSIX 1003.1-2001 extended ustar",
	PAX,
	ARCHIVE|DELTAINFO|LINKTYPE|SLASHDIR|STANDARD|SUM|IN|OUT|APPEND,
	DEFBUFFER,
	DEFBLOCKS,
	BLOCKSIZE,
	PAXNEXT(pax),
	0,
	tar_done,
	tar_getprologue,
	tar_getheader,
	0,
	0,
	0,
	pax_putprologue,
	tar_putheader,
	0,
	0,
	tar_putepilogue,
	0,
	0,
	0,
	0,
	tar_event,
	PAX_EVENT_DELTA_EXTEND
};

Format_t	pax_ustar_format =
{
	"ustar",
	"tar",
	"POSIX 1003.1-1988 tar",
	TAR,
	ARCHIVE|LINKTYPE|SLASHDIR|IN|OUT|APPEND,
	DEFBUFFER,
	DEFBLOCKS,
	BLOCKSIZE,
	PAXNEXT(ustar),
	0,
	tar_done,
	tar_getprologue,
	tar_getheader,
	0,
	0,
	0,
	tar_putprologue,
	tar_putheader,
	0,
	0,
	tar_putepilogue,
	tar_lookup,
	0,
	0,
	0,
	tar_event,
	PAX_EVENT_SKIP_JUNK
};

PAXLIB(ustar)
