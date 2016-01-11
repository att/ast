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
 * pax slt format for ansi and ibm labeled tapes
 */

#include "format.h"

#include <tm.h>

#define ANSI		1
#define IBM		2

#define SLT_ID		"slt"
#define LABEL_MAX	2048
#define HDR_SIZE	80
#define NAME_SIZE	17
#define VARHDR_SIZE	9

typedef struct Slt_s
{
	unsigned char*	mapin;		/* input map			*/
	unsigned char*	mapout;		/* output map			*/
	char		id[NAME_SIZE+1]; /* name id			*/
	char		volume[64];	/* volume id			*/
	char		format[7];	/* format id			*/
	char		implementation[8];/* implementation id		*/
	char		owner[15];	/* owner id			*/
	char		standards[20];	/* standards id			*/
	int		done;		/* label read done		*/
	int		ibm;		/* ibm format			*/
	int		peek;		/* buf (size peek) already read	*/
	int		section;	/* inside section		*/
	int		sequence;	/* sequence number		*/
	int		vol;		/* emit volume prologue		*/
	char		buf[LABEL_MAX]; /* header buffer		*/
	char		last[5];	/* last label			*/
} Slt_t;

/*
 * get label header number
 */

static long
getlabnum(register char* p, int byte, int width, int base)
{
	register char*	e;
	register int	c;
	long		n;

	p += byte - 1;
	c = *(e = p + width);
	*e = 0;
	n = strtol(p, NiL, base);
	*e = c;
	return n;
}

/*
 * get label header string
 */

static char*
getlabstr(register char* p, int byte, int width, register char* s)
{

	register char*	e;
	char*		v;

	v = s;
	p += byte - 1;
	e = p + width;
	while (p < e && (*s = *p++) != ' ')
		s++;
	*s = 0;
	return v;
}

/*
 * return length of next label
 * variable length labels have label number > 3 and Vnnnn at position 5
 * where nnnn is the decimal length of the entire label
 * nnnn may be < HDR_SIZE but label block must be >= HDR_SIZE
 * 0 returned at end of label group
 */

static int
getlabel(Pax_t* pax, register Archive_t* ap, register File_t* f)
{
	register Slt_t*	slt = (Slt_t*)ap->data;
	register int	c;
	register int	n;

	if (c = slt->peek)
	{
		slt->peek = 0;
		return c;
	}
	if (slt->done || (c = paxread(pax, ap, slt->buf, (off_t)HDR_SIZE, (off_t)LABEL_MAX, 0)) < HDR_SIZE)
		return *slt->last = slt->done = c = 0;
	if (slt->buf[4] == 'V' && ((n = getlabnum(slt->buf, 4, 1, 10)) < 1 || n > 3) && (n = getlabnum(slt->buf, 6, 4, 10)) != c)
	{
		if ((c = n - c) > 0)
		{
			if (ap->io->blocked || paxread(pax, ap, slt->buf + HDR_SIZE, (off_t)0, (off_t)c, 1) != c)
			{
				c = HDR_SIZE;
				error(2, "%s: %-*.*s: variable length label record too short", f->name, c, c, slt->buf);
			}
			else
				c = n;
		}
		else if (n <= VARHDR_SIZE)
			c = VARHDR_SIZE;
		else
			c = n;
	}
	if (!ap->io->blocked && !*slt->last && slt->buf[3] == '2' && (strneq(slt->buf, "HDR", 3) || strneq(slt->buf, "EOF", 3) || strneq(slt->buf, "EOV", 3)))
		getlabstr(slt->buf, 26, 4, slt->last);
	if (*slt->last && strneq(slt->buf, slt->last, 4))
		slt->done = 1;
	message((-4, "label: %-*.*s", c, c, slt->buf));
	return c;
}

/*
 * output file HDR/EOF/EOV labels
 */

static void
putlabels(Pax_t* pax, register Archive_t* ap, register File_t* f, char* type)
{
	register Slt_t*	slt = (Slt_t*)ap->data;
	struct tm*	tm;

	switch (*type)
	{
	case 'E':
		paxwrite(pax, ap, slt->buf, 0);
		break;
	case 'H':
		slt->sequence++;
		break;
	}
	tm = gmtime(&f->st->st_mtime);
	sfsprintf(slt->buf, sizeof(slt->buf), "%s1%-17.17s000001%04d%04d000100 %02d%03d 00000 %06d%-6.6sD%-7.7s       ", type, f->id, slt->section, slt->sequence, tm->tm_year, tm->tm_yday, f->record.blocks, slt->format, slt->implementation);
	paxwrite(pax, ap, slt->buf, HDR_SIZE);
	sfsprintf(slt->buf, sizeof(slt->buf), "%s2%c%05d%05d%010d%s%c                     00                            ", type, state.record.format, state.blocksize, state.record.size, f->st->st_size, type, '2');
	paxwrite(pax, ap, slt->buf, HDR_SIZE);
	paxwrite(pax, ap, slt->buf, 0);
	if (streq(type, "EOV"))
	{
		slt->section++;
		slt->sequence = 0;
	}
	else
		slt->section = 1;
}

static int
slt_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	register Slt_t*		slt;
	char*			s;
	char*			t;
	int			lab;
	long			n;
	off_t			x;
	char			hdr[HDR_SIZE + 1];

	static const char	key[] = "VOL1";

	if (size < HDR_SIZE)
		return 0;
	memcpy(hdr, buf, HDR_SIZE);
	if (fp->flags & CONV)
	{
		ccmapstr(state.map.e2n, hdr, HDR_SIZE);
		if (!strneq(hdr, key, sizeof(key) - 1))
			return 0;
		ccmapstr(state.map.e2n, hdr, HDR_SIZE);
		convert(ap, SECTION_CONTROL, CC_NATIVE, CC_EBCDIC);
		if (!ap->convert[0].on)
			convert(ap, SECTION_DATA, CC_NATIVE, CC_EBCDIC);
	}
	else
	{
		ccmapstr(state.map.a2n, hdr, HDR_SIZE);
		if (!strneq(hdr, key, sizeof(key) - 1))
			return 0;
		convert(ap, SECTION_CONTROL, CC_NATIVE, CC_ASCII);
		if (!ap->convert[0].on)
			convert(ap, SECTION_DATA, CC_NATIVE, CC_ASCII);
	}
	if (!(slt = newof(0, Slt_t, 1, 0)))
	{
		nospace();
		return -1;
	}
	ap->data = slt;
	getlabstr(hdr, 5, 6, state.volume);
	getlabstr(hdr, 25, 6, slt->format);
	getlabstr(hdr, 31, 7, slt->implementation);
	getlabstr(hdr, 38, 14, slt->owner);
	paxget(pax, ap, 0, &x);
	ap->io->blocked = !x;
	if (ap->checkdelta)
	{
		if (!(lab = getlabel(pax, ap, f)))
			return 0;
		if (strneq(slt->buf, "UVL1", 4) && strneq(slt->buf + 5, ID, IDLEN))
		{
			ap->checkdelta = 0;
			s = slt->buf + 10;
			f->st->st_mtime = getlabnum(slt->buf, 14, 10, 10);
			n = getlabnum(slt->buf, 24, 10, 10);
			f->st->st_uid = DELTA_LO(n);
			f->st->st_gid = DELTA_HI(n);
			if (t = strchr(s, ' '))
				*t = 0;
			deltaset(ap, s);
		}
		else
			slt->peek = lab;
	}
	return 1;
}

static int
slt_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
slt_getheader(Pax_t* pax, register Archive_t* ap, register File_t* f, int wfd)
{
	register Slt_t*	slt = (Slt_t*)ap->data;
	register char*	s;
	register int	i;
	register off_t	n;
	int		lab;
	int		type;

 again:
	if (!(lab = getlabel(pax, ap, f)))
		return 0;
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_mode = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH;
	f->st->st_uid = state.uid;
	f->st->st_gid = state.gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->st->st_size = 0;
	f->linktype = NOLINK;
	f->linkpath = 0;
	f->uidname = 0;
	f->gidname = 0;
	type = 0;
	do
	{
		if (strneq(slt->buf, "HDR", 3))
		{
			if (getlabnum(slt->buf, 4, 1, 10) != ++type)
				error(3, "%s format HDR label out of sequence", ap->format->name);
			if (type == 1)
			{
				s = f->name = paxstash(pax, &ap->stash.head, NiL, NAME_SIZE + 3);
				for (i = 4; i <= NAME_SIZE + 3; i++)
				{
					if (slt->buf[i] == ' ')
					{
						if (i >= NAME_SIZE + 3 || slt->buf[i + 1] == ' ')
							break;
						*s++ = '.';
					}
					else
						*s++ = isupper(slt->buf[i]) ? tolower(slt->buf[i]) : slt->buf[i];
				}
				if ((n = getlabnum(slt->buf, 40, 2, 10)) > 0 && n < 99)
					sfsprintf(s, 3, ".%02d", n);
				else
					*s = 0;
				f->record.section = getlabnum(slt->buf, 28, 4, 10);
				getlabstr(slt->buf, 5, NAME_SIZE, f->id = slt->id);
				getlabstr(slt->buf, 61, 6, slt->format);
				getlabstr(slt->buf, 67, 7, slt->implementation);
#if SAVESET
				if (streq(slt->format, SAVESET_ID) && streq(slt->implementation, SAVESET_IMPL))
					ap->format = SAVESET;
#endif
				f->st->st_mtime = 0;
				if (n = getlabnum(slt->buf, 43, 2, 10))
				{
					if (slt->buf[41] == '0') n += 100;
					if ((i = getlabnum(slt->buf, 45, 3, 10)) >= 0 && i <= 365)
					{
						f->st->st_mtime = i;
						while (n-- > 70) f->st->st_mtime += ((n % 4) || n == 100) ? 365 : 366;
						f->st->st_mtime *= 24L * 60L * 60L;
						f->st->st_mtime += 12L * 60L * 60L;
					}
				}
				if (!f->st->st_mtime)
					f->st->st_mtime = NOW;
			}
			else if (type == 2)
			{
				switch (f->record.format = slt->buf[4])
				{
				case 'D': /* decimal variable	*/
				case 'F': /* fixed length	*/
				case 'S': /* spanned		*/
				case 'U': /* input block size	*/
				case 'V': /* binary variable	*/
					break;
				default:
					error(2, "%s record format %c not supported", ap->format->name, f->record.format);
					f->skip = 1;
				}
				state.blocksize = getlabnum(slt->buf, 6, 5, 10);
				state.record.size = getlabnum(slt->buf, 11, 5, 10);
				if (!ap->io->blocked) f->st->st_size = getlabnum(slt->buf, 16, 10, 10);
				state.record.offset = getlabnum(slt->buf, 51, 2, 10);
			}
		}
		else if (!ap->io->blocked && strneq(slt->buf, "VOL1", 4))
		{
			paxunread(pax, ap, slt->buf, lab);
			if (!(getprologue(ap)))
				return 0;
			goto again;
		}
	} while ((lab = getlabel(pax, ap, f)));
	return 1;
}

static int
slt_getdata(Pax_t* pax, register Archive_t* ap, register File_t* f, int wfd)
{
	register Slt_t*	slt = (Slt_t*)ap->data;
	register off_t	n;
	register off_t	size;
	int		c;
	int		i;
	int		j;
	int		k;
	int		nl;
	off_t		m;
	Sfio_t*		wfp;

	if (wfd < 0)
		wfp = 0;
	else if (!(wfp = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)))
	{
		error(2, "%s: cannot write", f->name);
		return -1;
	}
	ap->io->empty = 0;
	nl = state.record.line;
	size = 0;
	for (;;)
	{
		if (ap->io->blocked)
			n = paxread(pax, ap, state.tmp.buffer, (off_t)0, (off_t)state.buffersize, 0);
		else if ((m = f->st->st_size - size) <= 0)
			n = 0;
		else if (wfp) 
		{
			if (m > state.buffersize)
				m = state.buffersize;
			n = paxread(pax, ap, state.tmp.buffer, (off_t)0, m, 1);
		}
		else
			n = paxread(pax, ap, NiL, (off_t)0, m, 1);
		if (n < 0)
			break;
		if (n == 0)
		{
			k = 1;
			ap->sum--;
			while (getlabel(pax, ap, f))
			{
				if (strneq(slt->buf, "EOV1", 4))
					k = 0;
				else if (!strneq(slt->buf, "EOF", 3) && !strneq(slt->buf, "EOV", 3) && !strneq(slt->buf, "UTL", 3) && ++n >= 16 && !state.keepgoing)
					error(3, "%s: %s: %d invalid %s end of file/volume labels detected", ap->name, f->name, n, ap->format->name);
			}
			if (n)
				error(1, "%s: %s: %d invalid %s end of file/volume labels detected", ap->name, f->name, n, ap->format->name);
			if (k)
			{
				ap->sum++;
				break;
			}
			f->record.section++;
			f->id = strcpy(state.tmp.buffer, f->id);
			f->name = strcpy(state.tmp.buffer + NAME_SIZE + 1, f->name);
			for (;;)
			{
				newio(ap, 0, 0);
				if (getprologue(ap))
				{
					File_t		v;
					struct stat	st;

					v.st = &st;
					if (getheader(ap, &v))
					{
						if (streq(f->id, v.id) && streq(f->name, v.name) && f->record.section == v.record.section)
						{
							f->id = v.id;
							f->name = v.name;
							break;
						}
						error(1, "volume containing %s id %s section %d required", f->name, f->id, f->record.section);
					}
					ap->volume--;
				}
				ap->part--;
			}
			ap->sum++;
			continue;
		}
		if (f->record.format == 'V')
		{
			if ((k = ((unsigned char*)state.tmp.buffer)[0] << 8 | ((unsigned char*)state.tmp.buffer)[1]) != n)
				error(3, "%s: invalid %s V format block descriptor [%d!=%d]", f->name, ap->format->name, k, n);
			i = 4;
		}
		else
			i = 0;
		while (i < n)
		{
			i += state.record.offset;
			if (state.tmp.buffer[i] == '^')
				switch (f->record.format)
				{
				case 'F':
					if (slt->ibm)
						break;
					for (j = i; j < n && state.tmp.buffer[j] == '^'; j++);
					if (j < n)
						break;
					/*FALLTHROUGH*/
				case 'D':
				case 'S':
					i = n;
					continue;
				}

			/*
			 * get record size
			 */

			switch (f->record.format)
			{
			case 'D':
				if (sfsscanf(&state.tmp.buffer[i], "%4d", &k) != 1)
					k = -1;
				j = i + 4;
				break;
			case 'F':
				if (i + state.record.size > n)
					k = n - i;
				else if (state.record.line || state.record.offset)
					k = state.record.size;
				else
					k = n;
				j = i;
				break;
			case 'S':
				switch (state.tmp.buffer[i])
				{
				case '0':
				case '3':
					nl = 1;
					break;
				default:
					nl = 0;
					break;
				}
				if (sfsscanf(&state.tmp.buffer[i + 1], "%4d", &k) != 1)
					k = -1;
				j = i + 5;
				break;
			case 'U':
				k = n;
				j = i;
				break;
			case 'V':
				nl = !(state.tmp.buffer[i + 2] & 01);
				k = ((unsigned char*)state.tmp.buffer)[i] << 8 | ((unsigned char*)state.tmp.buffer)[i + 1];
				j = i + 4;
				break;
			}
			if (k < 0)
			{
				error(2, "invalid %s %c record size", ap->format->name, f->record.format);
				break;
			}
			m = i += k;
			if (slt->mapin)
				ccmapstr(slt->mapin, &state.tmp.buffer[j], m - j);
			if (state.record.line)
				switch (f->record.format)
				{
				case 'F':
				case 'U':
					while (--m >= j && state.tmp.buffer[m] == ' ');
					m++;
					break;
				}
			k = m - j + nl;
			size += k;
			if (wfp)
			{
				if (nl)
				{
					c = state.tmp.buffer[m];
					state.tmp.buffer[m] = '\n';
				}
				if (sfwrite(wfp, &state.tmp.buffer[j], k) != k)
				{
					error(ERROR_SYSTEM|1, "%s: write error", f->name);
					break;
				}
				if (nl)
					state.tmp.buffer[m] = c;
			}
		}
	}
	if (f->st->st_size && f->st->st_size != size)
		error(1, "%s: header size %I*d does not match data size %I*d", f->name, sizeof(f->st->st_size), f->st->st_size, sizeof(size), size);
	f->st->st_size = size;
	if (wfp)
	{
		sfclose(wfp);
		setfile(ap, f);
	}
	if (n < 0)
	{
		error(ERROR_SYSTEM|3, "%s: %s: archive read error", ap->name, f->name);
		return -1;
	}
	return 1;
}

static int
slt_getepilogue(Pax_t* pax, Archive_t* ap)
{
	return 1;
}

static int
slt_backup(Pax_t* pax, Archive_t* ap)
{
#ifdef MTIOCTOP
	struct mtop	mt;

	mt.mt_op = MTBSF;
	mt.mt_count = 1;
	if (ioctl(ap->io->fd, MTIOCTOP, &mt))
	{
		error(ERROR_SYSTEM|3, "%s: %s archive seek MTIO error", ap->name, ap->format->name);
		return -1;
	}
	return 0;
#else
	error(3, "%s: %s archive seek requires MTIO", ap->name, ap->format->name);
	return -1;
#endif
}

static int
slt_putprologue(Pax_t* pax, Archive_t* ap, int append)
{
	register Slt_t*	slt = (Slt_t*)ap->data;

	if (!ap->locked && slt->vol)
	{
		slt->vol = 0;
		ap->locked = 1;
		putlabels(pax, ap, state.record.file, "HDR");
		ap->locked = 0;
	}
	if (ap->format->flags & CONV)
	{
		convert(ap, SECTION_CONTROL, CC_NATIVE, CC_EBCDIC);
		if (!ap->convert[0].on)
			convert(ap, SECTION_DATA, CC_NATIVE, CC_EBCDIC);
	}
#if DEBUG
	if (ap->io->blok)
		ap->io->blocked = 1;
	else
#endif
	ap->io->blocked = !ap->io->unblocked;
	if (!slt->owner[0])
	{
		strncpy(slt->owner, fmtuid(getuid()), sizeof(slt->owner) - 1);
		slt->owner[sizeof(slt->owner) - 1] = 0;
	}
	strupper(slt->owner);
	if (!state.volume[0])
	{
		strncpy(state.volume, slt->owner, sizeof(state.volume) - 1);
		state.volume[sizeof(state.volume) - 1] = 0;
	}
	strupper(state.volume);
	strncpy(slt->format, SLT_ID, sizeof(slt->format) - 1);
	strncpy(slt->implementation, IMPLEMENTATION, sizeof(slt->implementation) - 1);
	if (ap->format->flags & CONV)
		sfsprintf(slt->standards, sizeof(slt->standards), "%-5.5s%-5.5s%-5.5s%-4.4s", "ATT", "1", "EBCDIC", "1979");
	else
		sfsprintf(slt->standards, sizeof(slt->standards), "%-5.5s%-5.5s%-5.5s%-4.4s", "ISO", "646", "IRV", "1990");
	sfsprintf(slt->buf, sizeof(slt->buf), "VOL1%-6.6s              %-6.6s%-7.7s%-14.14s                            4", state.volume, slt->format, slt->implementation, slt->owner);
	paxwrite(pax, ap, slt->buf, HDR_SIZE);
	sfsprintf(slt->buf, sizeof(slt->buf), "VOL2%-19.19s                                                         ", slt->standards);
	paxwrite(pax, ap, slt->buf, HDR_SIZE);
	if (ap->delta && !(ap->delta->format->flags & PSEUDO))
	{
		sfsprintf(slt->buf, sizeof(slt->buf), "UVL1 %-6.6s%c%-6.6s%010ld%010ld                                         ", ID, ap->delta->compress ? TYPE_COMPRESS : TYPE_DELTA, ((Compress_format_t*)ap->delta->format)->variant, state.operation == OUT ? (long)ap->size : (long)0, state.operation == OUT ? ap->checksum : 0L);
		paxwrite(pax, ap, slt->buf, HDR_SIZE);
	}
	return 1;
}

static int
slt_putheader(Pax_t* pax, Archive_t* ap, File_t* f)
{
	if (state.complete)
		complete(ap, f, 4 * HDR_SIZE);
	putlabels(pax, ap, f, "HDR");
	return 1;
}

static int
recordout(Pax_t* pax, Archive_t* ap, File_t* f, Sfio_t* fp)
{
	register Slt_t*	slt = (Slt_t*)ap->data;
	register int	c;
	register char*	p;
	register char*	recdat;
	register char*	blkdat;
	char*		rec;
	char*		blk;
	int		span;

	int		count = 0;
	int		partial = 0;
	int		truncated = 0;

	static char	span_out[] = "0132";

	if (!fp)
		error(3, "record output from buffer not supported");
	ap->record = f;
	f->record.blocks = 0;
	span = 0;
	blk = state.tmp.buffer;

	/*
	 * file loop
	 */

	for (;;)
	{
		p = blk;
		switch (state.record.format)
		{
		case 'V':
			p += 4;
			break;
		}
		blkdat = p;

		/*
		 * block loop
		 */

		for (;;)
		{
			rec = p;
			switch (state.record.format)
			{
			case 'D':
			case 'V':
				p += 4;
				break;
			case 'S':
				p += 5;
				break;
			}
			recdat = p;

			/*
			 * check for partial record from previous block
			 */

			if (partial)
			{
				memcpy(recdat, f->record.partial, partial);
				p += partial;
				partial = 0;
			}

			/*
			 * record loop
			 */

			span &= 01;
			span <<= 1;
			for (;;)
			{
				if (p >= &rec[state.record.size] && state.record.size)
				{
					if (state.record.line)
					{
						truncated++;
						while ((c = sfgetc(fp)) != EOF && c != '\n');
					}
					break;
				}
				else if (p >= &blk[state.blocksize])
				{
					if (state.record.format == 'S' || state.record.format == 'V')
					{
						if (p > recdat)
						{
							span |= 01;
							break;
						}
					}
					else if (partial = p - recdat)
					{
						/*
						 * save partial record for next block
						 */

						if (!f->record.partial && !(f->record.partial = newof(0, char, state.blocksize, 0)))
							nospace();
						memcpy(f->record.partial, recdat, partial);
					}
					p = rec;
					goto eob;
				}
				else if ((c = sfgetc(fp)) == EOF)
				{
					if (p == recdat)
					{
						if (rec == blkdat) goto eof;
						p = rec;
						goto eob;
					}
					break;
				}
				else if (c == '\n' && state.record.line)
					break;
				else
					*p++ = c;
			}
			switch (state.record.format)
			{
			case 'D':
				c = recdat[0];
				sfsprintf(rec, 4, "%04d", p - rec);
				recdat[0] = c;
				break;
			case 'F':
				if (c != EOF || state.record.pad)
				{
					memset(p, ' ', state.record.size - (p - rec));
					p = rec + state.record.size;
				}
				break;
			case 'S':
				c = recdat[0];
				sfsprintf(rec, 4, "%c%04d", span_out[span], p - rec);
				recdat[0] = c;
				break;
			case 'U':
				if (p == recdat) *p++ = ' ';
				break;
			case 'V':
				rec[0] = ((p - rec) >> 8) & 0xff;
				rec[1] = (p - rec) & 0xff;
				rec[2] = span;
				rec[3] = 0;
				break;
			}
			if (slt->mapout)
				ccmapstr(slt->mapout, recdat, p - recdat);
			count++;
			if (p >= &blk[state.blocksize] || state.record.format == 'U')
				break;
		}
	eob:
		switch (state.record.format)
		{
		case 'D':
		case 'S':
			if (state.record.pad)
			{
				memset(p, '^', state.blocksize - (p - blk));
				p = blk + state.blocksize;
			}
			break;
		case 'V':
			blk[0] = ((p - blk) >> 8) & 0xff;
			blk[1] = (p - blk) & 0xff;
			blk[2] = 0;
			blk[3] = 0;
			break;
		}
		paxwrite(pax, ap, blk, p - blk);
		f->record.blocks++;
	}
 eof:
	ap->record = 0;
	if (truncated)
		error(1, "%s: %d out of %d record%s truncated", f->name, truncated, count, count == 1 ? "" : "s");
	return 0;
}

static int
slt_putdata(Pax_t* pax, Archive_t* ap, File_t* f, int rfd)
{
	register size_t		m;
	register ssize_t	n;
	register off_t		c;
	int			r;
	Buffer_t*		bp;
	Sfio_t*			rfp;

	if (ap->io->blocked)
		return 0;
	r = 1;
	if (f->st->st_size > 0)
	{
		if (state.record.format == 'F' && !state.record.line)
		{
			/*
			 * this is faster than recordout()
			 */

			ap->record = f;
			c = f->st->st_size;
			while (c > 0)
			{
				n = m = c > state.record.size ? state.record.size : c;

				/*
				 * NOTE: we expect that all but the last
				 *	 read returns state.record.size
				 *	 if not the the intermediate short
				 *	 reads are filled with 0's
				 */

				if (!r)
				{
					if (rfd >= 0)
						n = read(rfd, state.tmp.buffer, m);
					else if (bp = getbuffer(rfd))
					{
						memcpy(ap->io->next, bp->next, m);
						bp->next += m;
					}
					else if (paxread(pax, f->ap, state.tmp.buffer, (off_t)0, (off_t)m, 1) <= 0)
						n = -1;
				}
				if (n <= 0)
				{
					if (n)
						error(ERROR_SYSTEM|2, "%s: read error", f->path);
					else
						error(2, "%s: file size changed", f->path);
					memzero(state.tmp.buffer, state.record.size);
					r = -1;
				}
				else
				{
					c -= n;
					if (n < state.record.size && (c > 0 || state.record.pad))
					{
						memzero(state.tmp.buffer + n, state.record.size - n);
						n = state.record.size;
					}
					paxwrite(pax, ap, state.tmp.buffer, n);
				}
			}
			ap->record = 0;
			if (rfd >= 0)
				close(rfd);
		}
		else if (rfd < 0)
			recordout(pax, ap, f, NiL);
		else if (!(rfp = sfnew(NiL, NiL, SF_UNBOUND, rfd, SF_READ)))
		{
			error(1, "%s: cannot read", f->path);
			close(rfd);
		}
		else
		{
			recordout(pax, ap, f, rfp);
			sfclose(rfp);
		}
	}
	return r;
}

static int
slt_puttrailer(Pax_t* pax, Archive_t* ap, File_t* f)
{
	putlabels(pax, ap, f, "EOF");
	return 0;
}

static off_t
slt_putepilogue(Pax_t* pax, Archive_t* ap)
{
	register Slt_t*	slt = (Slt_t*)ap->data;

	if (!ap->locked)
	{
		ap->locked = 1;
		putlabels(pax, ap, state.record.file, "EOV");
		ap->locked = 0;
		slt->vol = 1;
	}
	paxwrite(pax, ap, slt->buf, 0);
	paxwrite(pax, ap, slt->buf, 0);
	return ap->io->count;
}

static int
slt_validate(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	register Slt_t*	slt = (Slt_t*)ap->data;
	register char*	s;

	if (f->type != X_IFREG)
	{
		error(2, "%s: only regular files copied in %s format", f->path, ap->format->name);
		return 0;
	}
	if (s = strrchr(f->name, '/'))
	{
		s++;
		error(1, "%s: file name stripped to %s", f->name, s);
	}
	else
		s = f->name;
	if (strlen(s) > sizeof(slt->id) - 1)
	{
		error(2, "%s: file name too long", f->name);
		return 0;
	}
	f->id = strupper(strcpy(slt->id, s));
	return 1;
}

Format_t	pax_slt_format =
{
	"ansi",
	"slt",
	"ANSI standard label tape; for tape devices only",
	ANSI,
	ARCHIVE|NOHARDLINKS|IN|OUT,
	4,
	4,
	0,
	pax_slt_next,
	0,
	slt_done,
	slt_getprologue,
	0,
	slt_getdata,
	0,
	slt_getepilogue,
	slt_putprologue,
	slt_putheader,
	slt_putdata,
	slt_puttrailer,
	slt_putepilogue,
	0,
	slt_backup,
	0,
	slt_validate
};

static int
ibm_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	register Slt_t*	slt;
	int		n;

	if ((n = slt_getprologue(pax, fp, ap, f, buf, size)) > 0 && state.record.charset)
	{
		slt = (Slt_t*)ap->data;
		slt->mapin = state.map.e2n;
		slt->mapout = state.map.n2e;
	}
	return n;
}

Format_t	pax_ibm_format =
{
	"ibm",
	"elt",
	"EBCDIC standard label tape; for tape devices only",
	IBM,
	ARCHIVE|CONV|NOHARDLINKS|IN|OUT,
	4,
	4,
	0,
	PAXNEXT(ibm),
	0,
	slt_done,
	ibm_getprologue,
	0,
	slt_getdata,
	0,
	slt_getepilogue,
	slt_putprologue,
	slt_putheader,
	slt_putdata,
	slt_puttrailer,
	slt_putepilogue,
	0,
	slt_backup,
	0,
	slt_validate
};

PAXLIB(ibm)
