/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2004-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * pax calib format
 *
 * test registry
 *
 *	0x00200	disable auto-sequence
 *	0x00400 disable latest version selection
 *	0x00800 trace directory type { 0 1 }
 *	0x01000 disable ibm => standard cobol conversions
 *	0x02000 trace all directory types
 *	0x04000 trace partial output record buffers
 *	0x08000 trace record headers in decimal
 *	0x10000 trace record headers in base 4
 *	0x20000 trace record headers in base 4 (alternate format)
 */

#include <paxlib.h>
#include <codex.h>
#include <ccode.h>
#include <ctype.h>
#include <tm.h>

#include "camap.c"

#define MAGIC		"\301\304\331\100\323\311\302\331\306"
#define CHUNK		64

typedef struct Cadir_s
{
	unsigned short	offset;
	unsigned short	blocks;
	size_t		size;
} Cadir_t;

typedef struct Ar_s
{
	char*		format;
	char*		suffix;
	unsigned char*	map;
	unsigned char*	imap;
	unsigned char*	buffer;
	unsigned char*	next;
	int		camap;
	int		count;
	int		digits;
	int		flags;
	int		increment;
	int		linesize;
	int		position;
	int		sequence;
	int		version;
	size_t		bufsize;
	size_t		blocksize;
	size_t		nblocks;
	size_t	 	block;
	size_t		headsize;
	size_t		line;
	size_t		buf;
	size_t		left;
	Cadir_t*	dirs;
	Cadir_t*	dir;
	void*		cam;
} Ar_t;

#define CALIB_LINE	256

#define cabcd5(x)	((((unsigned char*)x)[0]>>4)*10000+(((unsigned char*)x)[0]&0xf)*1000+(((unsigned char*)x)[1]>>4)*100+(((unsigned char*)x)[1]&0xf)*10+(((unsigned char*)x)[2]>>4))
#define casize2(x)	((((unsigned char*)x)[0]<<8)|(((unsigned char*)x)[1]))
#define casize3(x)	((((unsigned char*)x)[0]<<16)|(((unsigned char*)x)[1]<<8)|(((unsigned char*)x)[2]))
#define casize4(x)	((((unsigned char*)x)[0]<<24)|(((unsigned char*)x)[1]<<16)|(((unsigned char*)x)[2]<<8)|(((unsigned char*)x)[3]))

/*
 * cobol keyword map
 */

static const char*	cakey[] =
{
	"FILLER ",
	"PICTURE ",
	"USAGE ",
	"VALUE ",
	"PERFORM ",
	"SUBTRACT ",
	"COMPUTE ",
	"COMPUTATIONAL",
	"COMP",
	"REDEFINES ",
	"RENAMES ",
	"JUSTIFIED ",
	"GIVING ",
	"USING ",
	"CALL ",
	"ALTER ",
	"EQUAL ",
	"GREATER ",
	"POSITIVE ",
	"NEGATIVE ",
	"ELSE ",
	"OTHERWISE ",
	"{22}",
	"VARYING ",
	"FROM ",
	"UNTIL ",
	"THRU ",
	"ROUNDED ",
	"GO TO ",
	"MOVE ",
	"ZERO ",
	"ZEROS ",
	"DEPENDING ",
	"PIC ",
	"BLANK ",
	"OCCURS ",
	"{36}",
	"{37}",
	"{38}",
	"{39}",
	"{40}",
	"{41}",
	"{42}",
	"{43}",
	"{44}",
	"{45}",
	"{46}",
	"{47}",
	"{48}",
	"{49}",
	"{50}",
	"{51}",
	"{52}",
	"{53}",
	"{54}",
	"{55}",
	"{56}",
	"{57}",
	"{58}",
	"{59}",
	"{60}",
	"{61}",
	"{62}",
	"{63}",
	"{64}",
	"{65}",
	"{66}",
	"{67}",
	"{68}",
	"{69}",
	"{70}",
	"{71}",
	"{72}",
	"{73}",
	"{74}",
	"{75}",
	"{76}",
	"{77}",
	"{78}",
	"{79}",
	"{80}",
	"{81}",
	"{82}",
	"{83}",
	"{84}",
	"{85}",
	"{86}",
	"{87}",
	"{88}",
	"{89}",
	"{90}",
	"{91}",
	"{92}",
	"{93}",
	"{94}",
	"{95}",
	"{96}",
	"{97}",
	"{98}",
	"{99}",
	"{100}",
	"{101}",
	"{102}",
	"{103}",
	"{104}",
	"{105}",
	"{106}",
	"{107}",
	"{108}",
	"{109}",
	"{110}",
	"{111}",
	"{112}",
	"{113}",
	"{114}",
	"{115}",
	"{116}",
	"{117}",
	"{118}",
	"{119}",
	"{120}",
	"{121}",
	"{122}",
	"{123}",
	"{124}",
	"{125}",
	"{126}",
	"{127}",
};

/*
 * return next nbits nibble from the input line
 */

#define cagetbits(ar,n)  ((n)<=(ar)->left? \
				((((ar)->buf)>>((ar)->left-=(n)))&((1L<<(n))-1)):\
				_cagetbits((ar), (n)))

static int
_cagetbits(Ar_t* ar, int nbits)
{
	int	c;

	while (ar->left <= 8 * (sizeof(ar->buf) - 1))
	{
		if (ar->count-- > 0)
		{
			c = *ar->next++;
			ar->buf <<= 8;
			ar->buf |= c;
			ar->left += 8;
		}
		else if (ar->left < nbits)
			return 0;
		else
			break;
	}
	return cagetbits(ar, nbits);
}

static int
calib_done(Pax_t* pax, register Paxarchive_t* ap)
{
	Ar_t*	ar;

	if (ar = (Ar_t*)ap->data)
	{
		if (ar->cam)
			camap_close(ar->cam);
		if (ar->dirs)
			free(ar->dirs);
		free(ar);
		ap->data = 0;
	}
	return 0;
}

static int
calib_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*		ar;
	register Cadir_t*	dp;
	register Cadir_t*	de;
	register Cadir_t*	db;
	unsigned char		hdr[30];
	unsigned char		dir[22];
	unsigned char		blk[16];
	off_t			n;
	size_t			m;
	int			j;
	int			k;

	if (sizeof(hdr) <= size)
		memcpy(hdr, buf, sizeof(hdr));
	else if (paxread(pax, ap, hdr, (off_t)sizeof(hdr), (off_t)sizeof(hdr), 0) <= 0)
		return 0;
	else
		paxunread(pax, ap, hdr, sizeof(hdr));
	if (memcmp(hdr, MAGIC, sizeof(MAGIC) - 1))
		return 0;
	if (!(ar = newof(0, Ar_t, 1, 0)))
	{
		paxnospace(pax);
		return -1;
	}
	ar->imap = (ar->map = ccmap(CC_EBCDIC_O, CC_NATIVE)) ? ar->map : ccmap(0, 0);
	ar->bufsize = casize2(&hdr[24]);
	ar->linesize = 80;
	db = dp = de = 0;
	j = 3;
	do
	{
		n = j * ar->bufsize;
		if (paxseek(pax, ap, n, SEEK_SET, 1) != n)
			(*pax->errorf)(NiL, pax, 3, "%s: %s format block header seek error", ap->name, ap->format->name);
		if (paxread(pax, ap, blk, (off_t)sizeof(blk), (off_t)sizeof(blk), 0) <= 0)
			(*pax->errorf)(NiL, pax, 3, "%s: %s format block header read error", ap->name, ap->format->name);
		j = casize2(&blk[10]);
		k = casize2(&blk[12]);
		if (pax->test & 0x20000)
			(*pax->errorf)(NiL, pax, 1, "blk %c%c%c%c%c%c%c%c %02x %02x %02x %02x %02x %02x %02x %02x", ccmapchr(ar->map, blk[0]), ccmapchr(ar->map, blk[1]), ccmapchr(ar->map, blk[2]), ccmapchr(ar->map, blk[3]), ccmapchr(ar->map, blk[4]), ccmapchr(ar->map, blk[5]), ccmapchr(ar->map, blk[6]), ccmapchr(ar->map, blk[7]), blk[8], blk[9], blk[10], blk[11], blk[12], blk[13], blk[14], blk[15]);
		while (k-- > 0)
		{
			if (paxread(pax, ap, dir, (off_t)sizeof(dir), (off_t)sizeof(dir), 0) <= 0)
				(*pax->errorf)(NiL, pax, 3, "%s: %s format header read error", ap->name, ap->format->name);
			if (pax->test & 0x20000)
				(*pax->errorf)(NiL, pax, 1, "dir %c%c%c%c%c%c%c%c %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", ccmapchr(ar->map, dir[0]), ccmapchr(ar->map, dir[1]), ccmapchr(ar->map, dir[2]), ccmapchr(ar->map, dir[3]), ccmapchr(ar->map, dir[4]), ccmapchr(ar->map, dir[5]), ccmapchr(ar->map, dir[6]), ccmapchr(ar->map, dir[7]), dir[8], dir[9], dir[10], dir[11], dir[12], dir[13], dir[14], dir[15], dir[16], dir[17], dir[18], dir[19], dir[20], dir[21]);
			if (dp >= de)
			{
				m = dp - db + CHUNK;
				if (!(db = newof(db, Cadir_t, m, 0)))
				{
					free(ar);
					paxnospace(pax);
					return -1;
				}
				dp = db + m - CHUNK;
				de = db + m - 1;
			}
			dp->offset = casize2(&dir[9]);
			dp->blocks = dir[14];
			dp->size = cabcd5(&dir[15]) * ar->linesize;
			ccmapstr(ar->map, dir, sizeof(dir));
			dp++;
		}
	} while (j);
	dp->offset = 0;
	dp->size = 0;
	ar->dirs = ar->dir = db;
	ap->data = ar;
	if (!(ar->cam = camap_open()))
	{
		calib_done(pax, ap);
		return -1;
	}
	return 1;
}

static int
calib_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int wfd)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	const char*		s;
	unsigned char*		out;
	long			sequence;
	int			c;
	int			bits;
	int			flags;
	int			generate;
	int			noted;
	int			oline;
	int			version;
	unsigned char*		hdr;
	unsigned char*		b;
	unsigned char*		m;
	char*			ofile;
	char*			suffix;
	size_t			block;
	size_t			index;
	ssize_t			z;
	off_t			n;
	Sfio_t*			wfp;
	char			from[CALIB_LINE + 1];
	char			to[CALIB_LINE + 1];
	char			comment[CALIB_LINE + 1];
	unsigned char		outbuf[5 * CALIB_LINE + 1];

				       /*          10        20        30        40        50        60   */
				       /*01234567890123456789012345678901234567890123456789012345678901234*/
	static char		map[] = "? ~.<(+|&!$*);^-/,%_>?:#@'=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	if (ar->suffix)
	{
		wfp = 0;
		suffix = 0;
	}
	else if (wfd < 0)
		return 1;
	else if (!(wfp = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)))
	{
		(*pax->errorf)(NiL, pax, 2, "%s: cannot write", f->name);
		return -1;
	}
	else if (ar->camap)
		camap_init(ar->cam);
	comment[0] = from[0] = to[0] = 0;
	sequence = ar->digits && ar->increment && (ar->position + ar->digits) <= ar->linesize ? ar->sequence : -1;
	noted = !wfp || !pax->warn;
	if (pax->test & 0x00200)
		sequence = -1;
	generate = ar->flags != 0xf1;
	block = 0;
	oline = error_info.line;
	error_info.line = 0;
	ofile = error_info.file;
	error_info.file = f->name;
	version = (pax->test & 0x00400) ? -1 : ar->version;
	for (;;)
	{
		block++;
		while (ar->line <= ar->blocksize - ar->headsize + 1)
		{
			ar->next = &ar->buffer[index = ar->line];
			hdr = ar->next;
			flags = ar->next[1];
			if (flags & 0x0c)
				break;
			ar->line += ar->next[0];
			ar->next += ar->headsize;
			if (flags & 0x80)
			{
				ar->next += 2;
				if (!(flags & 0x01))
					ar->next += 1;
			}
			if (version >= 0 && hdr[ar->headsize - 3] != version)
				continue;
			while ((ar->count = ar->next[-1]) == 1 && ar->next[0] == 0)
				ar->next += 2;
			error_info.line++;
			if (pax->test & 0x08000)
				(*pax->errorf)(NiL, pax, 0, "%03d %3u %3u %5u %3u %03o %4u %4u %4u %4u %4u %4o %4o %4o %4u %4u", error_info.line, ar->count, ar->next - hdr, ar->next - ar->buffer, hdr[0], hdr[1], hdr[2], hdr[3], hdr[4], hdr[5], hdr[6], hdr[7], hdr[8], hdr[9], hdr[10], hdr[11]);
			if (pax->test & 0x10000)
				(*pax->errorf)(NiL, pax, 0, "%03d %3u %3u %3u %03o %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u", error_info.line, ar->count, ar->next - hdr, hdr[0], hdr[1], hdr[2], hdr[3], hdr[4], hdr[5], hdr[6], hdr[7], hdr[8], hdr[9], hdr[10], hdr[11]);
			if (pax->test & 0x20000)
				(*pax->errorf)(NiL, pax, 0, "%03d %3d %03d:%02x:%04..4u:%03u | %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u",
					error_info.line,
					ar->count,
					ar->next[-1], ar->next[-1], ar->next[-1], ar->next[-1] & 0x7f,
					ar->next[0], ar->next[0], ar->next[0], ar->next[0] & 0x7f,
					ar->next[1], ar->next[1], ar->next[1], ar->next[1] & 0x7f,
					ar->next[2], ar->next[2], ar->next[2], ar->next[2] & 0x7f,
					ar->next[3], ar->next[3], ar->next[3], ar->next[3] & 0x7f,
					ar->next[4], ar->next[4], ar->next[4], ar->next[4] & 0x7f,
					ar->next[5], ar->next[5], ar->next[5], ar->next[5] & 0x7f);
			if (flags & 0x40)
			{
				bits = 8;
				m = ar->imap;
			}
			else
			{
				bits = 6;
				m = (unsigned char*)map;
			}
			out = outbuf;
			if (pax->warn)
				noted = 0;
			while (ar->count && (ar->next - ar->buffer) < ar->line)
			{
				if (ar->count < 64)
				{
					ar->left = 0;
					while ((c = cagetbits(ar, bits)) || bits == 8 && ar->count > 0)
						if (out < &outbuf[ar->linesize])
							*out++ = m[c];
						else if (!noted)
						{
							noted = 1;
							(*pax->errorf)(NiL, pax, 2, "%s: overbyte (bits=%d offset=%I*u+%I*u block=%I*u)", ap->name, bits, sizeof(off_t), paxseek(pax, ap, (off_t)0, SEEK_CUR, 0) - ar->bufsize, sizeof(index), index, sizeof(block), block);
						}
					if ((ar->next - ar->buffer) >= ar->line)
						break;
					if ((c = (*ar->next ^ 64)) & 0x80)
						c = 0;
					else
						ar->next++;
				}
				else if (ar->count & 0x80)
				{
					if (pax->test & 0x04000)
						(*pax->errorf)(NiL, pax, 1, "part c=%d n=%d k=%d \"%-.*s\"", c, out - outbuf, ar->count & 0x7f, out - outbuf, outbuf);
					goto key;
				}
				else if ((c = ar->count - 64) & 0x80)
					c = (c + 64) & 0x7f;
				if (pax->test & 0x04000)
					(*pax->errorf)(NiL, pax, 1, "part c=%d:%d:%d r=%d:%d bits=%d n=%d x=%u \"%-.*s\"", c, &outbuf[ar->linesize] - out, c ^ 64, ar->next - ar->buffer, ar->line, bits, out - outbuf, *ar->next, out - outbuf, outbuf);
				if (c > (&outbuf[ar->linesize] - out))
				{
					ar->count = c ^ 64;
					if (bits == 8 && ar->count <= (&outbuf[ar->linesize] - out))
						continue;
					goto key;
				}
				else if (c > 0)
				{
					memset(out, ' ', c);
					out += c;
				}
				while ((ar->next - ar->buffer) < ar->line && (ar->count = *ar->next++) & 0x80)
				{
				key:
					s = cakey[ar->count & 0x7f];
					if (pax->warn && *s == '{') /*'}'*/
						(*pax->errorf)(NiL, pax, 1, "%s: keyword %s (bits=%d offset=%I*u+%I*u block=%I*u)", ap->name, s, bits, sizeof(off_t), paxseek(pax, ap, (off_t)0, SEEK_CUR, 0) - ar->bufsize, sizeof(index), index, sizeof(block), block);
					while (c = *s++)
						if (out < &outbuf[ar->linesize])
							*out++ = c;
						else
						{
							if (!noted)
							{
								noted = 1;
								(*pax->errorf)(NiL, pax, 2, "%s: key overbyte (bits=%d offset=%I*u+%I*u block=%I*u)", ap->name, bits, sizeof(off_t), paxseek(pax, ap, (off_t)0, SEEK_CUR, 0) - ar->bufsize, sizeof(index), index, sizeof(block), block);
							}
							break;
						}
				}
			}
			if (!wfp)
			{
				if (flags & 0x01)
				{
					for (b = outbuf; b < out && *b == ' ';  b++);
					for (; b < out && isdigit(*b); b++);
					for (; b < out && *b == ' '; b++);
					for (m = b; b < out && *b != ' '; b++);
					if ((out - b) > 6 && *m == '/' && *(m + 1) == '*')
					{
						while ((out - ++b) > 4)
						{
							if (*b == 'R' && *(b + 1) == 'E' && *(b + 2) == 'X' && *(b + 3) == 'X')
							{
								suffix = ".REXX";
								break;
							}
						}
						if (suffix)
							break;
						continue;
					}
					if (m == outbuf)
					{
						if ((b - m) == 7 && strneq((char*)m, "NEWPAGE", 7) || (b - m) == 4 && strneq((char*)m, "FILE", 4))
						{
							suffix = ".EZT";
							break;
						}
						else if ((out - outbuf) > 2 && m[0] == 'C' && (m[1] == ' ' || m[1] == '*'))
						{
							suffix = ".F";
							break;
						}
					}
					else if ((b - m) == 7 && strneq((char*)m, "INCLUDE", 7) || (b - m) == 4 && strneq((char*)m, "NAME", 4))
					{
						suffix = ".LNK";
						break;
					}
					else if ((b - m) == 5 && (strneq((char*)m, "TITLE", 5) || strneq((char*)m, "SPACE", 5)))
					{
						suffix = ".ASM";
						break;
					}
					else if ((b - m) == 3 && strneq((char*)m, "DCL", 3))
					{
						suffix = ".PLI";
						break;
					}
					else if ((b - m) == 10 && strneq((char*)m, "SUBROUTINE", 10) || (b - m) == 9 && strneq((char*)m, "DIMENSION", 9) || (b - m) == 6 && strneq((char*)m, "COMMON", 6) || b < out && (b - m) >= 6 && strneq((char*)m, "FORMAT", 6) && (m[6] == '(' || m[6] == ' '))
					{
						suffix = ".F";
						break;
					}
					if (b < out && *m++ == '.' && ((b - m) == 7 && strneq((char*)m, "REPLACE", 7) || (b - m) == 5 && strneq((char*)m, "QUOTE", 5) || (b - m) > 4 && strneq((char*)m, "END-", 4)))
					{
						suffix = ".QBL";
						break;
					}
					if ((b - outbuf) > 6)
					{
						if (outbuf[6] == ' ')
						{
							for (b = outbuf + 7; b < out && *b == ' ';  b++);
							for (m = b; b < out && *b != ' '; b++);
							if (b < out && (b - m) == 2 && isdigit(m[0]) && isdigit(m[1]))
							{
								for (; b < out && *b != '.'; b++);
								if (b < out && *b == '.')
								{
									suffix = ".CPY";
									break;
								}
							}
							else
							{
								for (; b < out && *b == ' '; b++);
								for (m = b; b < out && *b != '.'; b++);
								if (b < out && (b - m) == 8 && strneq((char*)m, "DIVISION", 8))
								{
									suffix = ".COB";
									break;
								}
							}
						}
						else if (outbuf[6] == '*')
							suffix = ".CPY";
					}
				}
			}
			else if (flags & 0x01)
			{
				if (sequence >= 0)
				{
					if ((b = outbuf + ar->position) >= out || *b == ' ' || isdigit(*b) || (out - outbuf) == ar->linesize && (b = out - ar->digits))
					{
						if ((c = ar->position - (out - outbuf)) > 0)
						{
							memset(out, ' ', c);
							out += c;
						}
						b += sfsprintf((char*)b, ar->digits+1, "%0.*lu", ar->digits, generate ? sequence : casize3(&hdr[2]));
						if (out < b)
							out = b;
					}
					sequence += ar->increment;
				}
				for (; out > outbuf && *(out - 1) == ' '; out--);
				*out++ = '\n';
				if ((out - outbuf) > 10 && outbuf[6] == '*' && outbuf[7] == '/')
				{
					if ((out - outbuf) == 11 && outbuf[8] == '/' && (outbuf[9] == '*' || outbuf[9] == ' ' || !outbuf[9]))
						from[0] = to[0] = 0;
					else
					{
						m = 0;
						for (b = outbuf + 8; b < out; b++)
							if (*b == '/')
							{
								if (!m)
									m = b;
								else
								{
									if (*++b == '*' || *b == ' ' || *b == '\n')
									{
										c = m - outbuf - 8;
										memcpy(from, outbuf + 8, c);
										from[c] = 0;
										c = b - m - 2;
										memcpy(to, m + 1, c);
										to[c] = 0;
									}
									break;
								}
							}
					}
				}
				else if (!pax->strict && !(pax->test & 0x01000) && outbuf[0] == '-' && outbuf[1] == 'I' && outbuf[2] == 'N' && outbuf[3] == 'C' && outbuf[4] == ' ')
				{
					for (b = outbuf + 5; *b == ' '; b++);
					for (m = b; m < (out - 1) && *m != ' '; m++);
					if (from[0])
						c = sfsprintf((char*)outbuf, sizeof(outbuf), "       COPY %-.*s REPLACING ==%s== BY ==%s==.\n", m - b, b, from, to);
					else
						c = sfsprintf((char*)outbuf, sizeof(outbuf), "       COPY %-.*s.\n", m - b, b);
					out = outbuf + c;
				}
				z = out - outbuf;
				if (sfwrite(wfp, outbuf, z) != z || ar->camap && camap_write(ar->cam, outbuf, z) < 0)
				{
					(*pax->errorf)(NiL, pax, 2, "%s: write error", f->name);
					goto bad;
				}
			}
			else
			{
				error_info.line--;
				if (pax->warn)
				{
					for (b = outbuf; b < out && *b == ' '; b++);
					for (; out > outbuf && *(out - 1) == ' '; out--);
					*out = 0;
					if (!streq((char*)b, comment))
						(*pax->errorf)(NiL, pax, 0, "comment %s \"%s\"", error_info.file, strcpy(comment, (char*)b));
				}
			}
		}
		if (!wfp)
			break;
		if (--ar->nblocks <= 0)
			break;
		if (ar->buffer[2] == 0xff)
		{
			n = casize2(ar->buffer + ar->blocksize + (block != 1)) * ar->bufsize;
			if (paxseek(pax, ap, n, SEEK_SET, 1) != n)
				(*pax->errorf)(NiL, pax, 3, "%s: %s format data seek error", ap->name, ap->format->name);
		}
		if (!(ar->buffer = (unsigned char*)paxget(pax, ap, (off_t)ar->bufsize, NiL)))
			(*pax->errorf)(NiL, pax, 3, "%s: format member read error (offset=%I*u block=%I*u)", ap->name, sizeof(off_t), paxseek(pax, ap, (off_t)0, SEEK_CUR, 0), sizeof(block), block);
		ar->blocksize = casize2(ar->buffer);
		ar->next = ar->buffer;
		ar->line = 4;
	}
	error_info.line = oline;
	error_info.file = ofile;
	if (wfp)
	{
		if (sfsync(wfp) || ar->camap && camap_done(ar->cam, f->name, wfd) < 0)
		{
			(*pax->errorf)(NiL, pax, 2, "%s: write error", f->name);
			goto bad;
		}
		wfp->_file = -1;
		sfclose(wfp);
	}
	else if (suffix)
		strcpy(ar->suffix, suffix);
	return 1;
 bad:
	wfp->_file = -1;
	sfclose(wfp);
	return -1;
}

static int
calib_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	register unsigned char*	h;
	register off_t		n;
	char*			s;
	char*			t;
	char*			v;
	int			i;
	int			j;
	int			k;
	int			x;
	char			suf[4];

	if (!ar->dir->offset)
		return 0;
	n = ar->bufsize * ar->dir->offset;
	if (paxseek(pax, ap, n, SEEK_SET, 1) != n)
		(*pax->errorf)(NiL, pax, 3, "%s: %s format member seek error", ap->name, ap->format->name);
	ar->nblocks = ar->dir->blocks;
	if (!(ar->buffer = (unsigned char*)paxget(pax, ap, (off_t)ar->bufsize, NiL)))
		(*pax->errorf)(NiL, pax, 3, "%s: %s format member read error", ap->name, ap->format->name);
	ar->blocksize = casize2(&ar->buffer[0]) + 1;
	ar->headsize = 6;
	ar->version = -1;
	ar->next = ar->buffer;
	s = 0;
	k = 0;
	for (j = 1, x = 0, h = ar->buffer + 4; !x && h < ar->buffer + ar->bufsize; j++, h += i)
	{
		if ((i = *h) == 78)
		{
			j = h[2];
			if (j == 5 && !h[1] && !h[3])
				x = 1;
			i -= 6;
			h += 6;
		}
		else
		{
			if (j == 5)
				x = 1;
			i -= 2;
			h += 2;
		}
		if (!(k & (1<<j)))
		{
			k |= (1<<j);
			if (pax->test & 0x02000)
			{
				int	y;

				sfprintf(sfstderr, "head %-8s %d", f->name, j);
				for (y = 0; y < i; y++)
					sfprintf(sfstderr, " %02x", h[y]);
				sfprintf(sfstderr, "\n");
			}
			switch (j)
			{
			case 0:
				switch (ar->flags = h[32])
				{
				case 0x40:
					ar->headsize = 5;
					break;
				case 0xf1:
					ar->headsize = 8;
					break;
				}
				ar->version = h[-3];
				ar->position = h[33];
				ar->digits = h[34] + 1;
				ar->increment = casize2(&h[35]);
				if ((ar->sequence = casize2(&h[37])) > 0x7fff)
					ar->sequence = ar->sequence - 0x10000;
				ar->sequence += ar->increment;
				ccmapstr(ar->map, h, 24);
				f->name = paxstash(pax, &ap->stash.head, (char*)h, 28);
				for (s = f->name + 8; *(s - 1) == ' '; s--);
				*(ar->suffix = s) = 0;
				f->st->st_mtime = f->st->st_ctime = f->st->st_atime = tmscan(sfprints("%-.6s%02u%02u%02u", memcmp(h + 18, "000000", 6) ? (h + 18) : (h + 12), h[24], h[25], h[26]), NiL, "%y%m%d%H%M%S", NiL, NiL, 0);
				if (pax->test & 0x00800)
					(*pax->errorf)(NiL, pax, 0, "head %-8s %d %03o %03o %03o %03o %03o %03o %03o", f->name, j, h[32], h[33], h[34], h[35], h[36], h[37], h[38]);
				break;
			case 1:
				switch (ar->flags = h[32])
				{
				case 0x40:
					ar->headsize = 3;
					break;
				}
				if (h[31] == 0x48)
				{
					ar->digits = 0;
					ar->linesize = 133;
				}
				else
				{
					ar->position = h[33];
					ar->digits = h[34] + 1;
					ar->increment = casize2(&h[35]);
					if ((ar->sequence = casize2(&h[37])) > 0x7fff)
						ar->sequence = ar->sequence - 0x10000;
					ar->sequence += ar->increment;
				}
				ccmapstr(ar->map, h, 32);
				f->name = paxstash(pax, &ap->stash.head, (char*)h, 13);
				for (s = f->name + 8; *(s - 1) == ' '; s--);
				*(ar->suffix = s) = 0;
				f->st->st_mtime = f->st->st_ctime = f->st->st_atime = tmscan(sfprints("%-.6s%-.4s00", memcmp(h + 18, "000000", 6) ? (h + 18) : (h + 12), h + 24), NiL, "%m%d%y%H%M%S", NiL, NiL, 0);
				if (pax->test & 0x00800)
					(*pax->errorf)(NiL, pax, 0, "head %-8s %d %03o %03o %03o %03o %03o %03o %03o", f->name, j, h[32], h[33], h[34], h[35], h[36], h[37], h[38]);
				break;
			case 5:
				if (s)
				{
					switch (h[38])
					{
					case 0x31:
						v = "PLI";
						break;
					case 0x32:
					case 0x42:
					case 0x4c:
						v = "COB";
						break;
					case 0x43:
						v = "EST";
						break;
					case 0x4d:
						v = "ASM";
						break;
					default:
						t = (char*)h + 34;
						ccmapstr(ar->map, t, 3);
						v = suf;
						while (t < ((char*)h + 37) && *t != ' ')
							*v++ = *t++;
						*v = 0;
						if (!*(v = suf))
						{
							if (ar->linesize == 133)
							{
								v = "LST";
								ar->suffix = 0;
							}
							else
								switch (h[38])
								{
								case 0x00:
								case 0x02:
									v = "COB";
									break;
								case 0x01:
								case 0x0a:
									v = 0;
									break;
								default:
									v = 0;
									if (pax->warn)
										(*pax->errorf)(NiL, pax, 1, "%s: no suffix for type=%02x", f->name, h[38]);
									break;
								}
						}
						else if (streq(v, "CB2") || streq(v, "CBL"))
							v = "COB";
						else if (streq(v, "PL1"))
							v = "PLI";
						break;
					}
					if (v)
					{
						*s++ = '.';
						strcpy(s, v);
						if (streq(v, "EZT") || streq(v, "PLI"))
							ar->suffix = 0;
					}
				}
				break;
			}
		}
	}
	for (s = f->name; *s; s++)
		if (!isalnum(*s) && *s != '.')
			*s = '_';
	ar->line = h - ar->buffer;
	ar->camap = 0;
	if (ar->suffix)
	{
		calib_getdata(pax, ap, f, -1);
		ar->line = h - ar->buffer;
		ar->suffix = 0;
	}
	for (i = 0, s = f->name; *s; s++)
		if (*s == '.')
		{
			if (!(pax->test & 0x01000) && (!strcasecmp(s + 1, "COB") || !strcasecmp(s + 1, "CPY")))
				ar->camap = 1;
			break;
		}
		else if (islower(*s))
			i = 1;
	if (i && *s)
		while (*++s)
			if (isupper(*s))
				*s = tolower(*s);
	f->linkpath = 0;
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_mode = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH;
	f->st->st_uid = pax->uid;
	f->st->st_gid = pax->gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->st->st_size = ar->dir->size;
	ar->dir++;
	return 1;
}

Paxformat_t	pax_calib_format =
{
	"ca-librarian",
	"calib|librarian",
	"mvs CA-librarian file",
	0,
	PAX_ARCHIVE|PAX_NOHARDLINKS|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(calib),
	0,
	calib_done,
	calib_getprologue,
	calib_getheader,
	calib_getdata,
};

PAXLIB(calib)
