/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2012 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * pax block io support
 */

#include "pax.h"

#define show(s,n)		fmtquote(s,NiL,NiL,(n)>64?64:(n),0)

#if _sys_mtio
#include <ast_tty.h>
#include <sys/mtio.h>
#if _sys_ioctl
#include <sys/ioctl.h>
#endif
#endif
#ifdef MTIOCTOP
#if defined(MTIOBSF) && !defined(MTBSF)
#define MTBSF	MTIOBSF
#endif
#if defined(MTIOBSR) && !defined(MTBSR)
#define MTBSR	MTIOBSR
#endif
#if defined(MTIOEOM) && !defined(MTEOM)
#define MTEOM	MTIOEOM
#endif
#if defined(MTIOFSF) && !defined(MTFSF)
#define MTFSF	MTIOFSF
#endif
#if defined(MTIOWEOF) && !defined(MTWEOF)
#define MTWEOF	MTIOWEOF
#endif
#if !defined(MTBSF) || !defined(MTBSR) || !defined(MTWEOF) || defined(__hppa)/*hppa-compiler-signal-10*/
#undef	MTIOCTOP
#endif
#endif

#define CVT(a,b,c,m,t) \
	do \
	{ \
		if (c) \
			switch ((a)->convert[SECTION(a)].on) \
			{ \
			case 1: \
				if (state.forceconvert || SECTION(a) != SECTION_DATA || TEXT(b,c,t)) \
					(a)->convert[SECTION(a)].on = 2; \
				else \
				{ \
					(a)->convert[SECTION(a)].on = 0; \
					break; \
				} \
			case 2: \
				ccmapstr(m, b, c); \
				break; \
			} \
		if ((a)->swapio) \
			swapmem((a)->swapio, b, b, c); \
	} while (0)

#define TEXT(b,c,t)	text(t,(unsigned char*)b,c)

#define CONVERT(a,b,c)	CVT(a,b,c,(a)->convert[SECTION(a)].f2t,(a)->convert[SECTION(a)].f2a)
#define REVERT(a,b,c)	CVT(a,b,c,(a)->convert[SECTION(a)].t2f,(a)->convert[SECTION(a)].t2a)

/*
 * return 1 if b is text data in f charset
 */

static const unsigned char	ascii_text[] =
{
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static int
text(unsigned char* map, register unsigned char* b, ssize_t n)
{
	register unsigned char*	e;
	register int		c;

	if (n > 256)
		n = 256;
	e = b + n;
	while (b < e)
	{
		c = *b++;
		c = ccmapchr(map, c);
		if (!ascii_text[c])
			return 0;
	}
	return 1;
}

#if 0 && DEBUG

/*
 * --blok=i	input is BLOK file
 * --blok=o	output file is BLOK file
 */

static int
blokread(register Archive_t* ap, char* buf, int n)
{
	register int		i;
	register int		j;
	char			c;

	if (!ap->io->blokflag)
	{
		ap->io->blokeof = 0;
		ap->io->blokflag = 1;
		if ((i = read(ap->io->fd, buf, ap->io->blok ? 4 : n)) < 4 || !strneq(buf, "\002\014\017\013", 4))
		{
			if (ap->io->blok)
				error(3, "%s: input archive is not a BLOK file", ap->name);
			return i;
		}
		if (i > 4 && lseek(ap->io->fd, (off_t)4, SEEK_SET) != 4)
			error(3, "%s: cannot seek on input archive BLOK file -- use --blok=i", ap->name);
		ap->io->blok = 1;
	}
	if (ap->io->blok)
	{
		j = 0;
		do
		{
			if ((i = read(ap->io->fd, &c, 1)) < 1)
			{
				if (!i)
					return 0;
				if (!ap->io->blokeof)
				{
					ap->io->blokeof = 1;
					return 0;
				}
				return -1;
			}
			j <<= 7;
			j |= c & 0177;
		} while (c & 0200);
		if (j > 0)
		{
			if (j > n)
				error(2, "%s: blokread buffer overflow (%d>%d)", ap->name, j, n);
			if ((i = read(ap->io->fd, buf, j)) != j)
				error(2, "%s: blokread blocking error", ap->name);
		}
		else
			i = 0;
	}
	else
		i = read(ap->io->fd, buf, n);
	return i;
}

static int
blokwrite(register Archive_t* ap, char* buf, int n)
{
	register char*	s;
	register int	i;
	register int	j;
	char		blk[9];

	if (ap->io->blok)
	{
		s = blk;
		if (!ap->io->blokflag)
		{
			ap->io->blokflag = 1;
			*s++ = 002;
			*s++ = 014;
			*s++ = 017;
			*s++ = 013;
		}
		i = 0;
		if (j = (n >> 21) & 0177)
		{
			*s++ = j | 0200;
			i = 1;
		}
		if ((j = (n >> 14) & 0177) || i)
		{
			*s++ = j | 0200;
			i = 1;
		}
		if ((j = (n >> 7) & 0177) || i)
		{
			*s++ = j | 0200;
			i = 1;
		}
		*s++ = n & 0177;
		j = s - blk;
		if ((i = write(ap->io->fd, blk, j)) != j)
			error(ERROR_SYSTEM|3, "%s: blokwrite count write error (%d!=%d)", ap->name, i, j);
		if (n <= 0)
			i = n;
		else if ((i = write(ap->io->fd, buf, n)) != n)
			error(ERROR_SYSTEM|3, "%s: blokwrite data write error (%d!=%d", ap->name, i, n);
	}
	else
		i = write(ap->io->fd, buf, n);
	return i;
}

#define read(f,b,n)	blokread(f,b,n)
#define write(f,b,n)	blokwrite(f,b,n)

#endif

#if 0

static ssize_t
ewrite(int f, void* b, size_t n)
{
	static int	count = 1;

	if (!count)
	{
		sfprintf(sfstderr, "AHA ENOSPC\n");
		errno = ENOSPC;
		return -1;
	}
	count--;
	return write(f, b, n);
}

#undef	write
#define write(f,b,n)	ewrite(f,b,n)

#endif

static char*
bstatus(Bio_t* io)
{
	char*	s;

	s = sfprints("<%p,%d,%d,%I*d,%d,%d>", io->buffer, io->unread, io->fill, sizeof(io->count), io->count, io->next - (io->buffer + io->unread), io->last - (io->buffer + io->unread));
	return strcpy(fmtbuf(strlen(s) + 1), s);
}

/*
 * initialize buffered io
 */

void
binit(register Archive_t* ap)
{
	unsigned long	n;
	unsigned long	u;

	if (ap->io->buffer)
		return;
	if (ap->delta)
		ap->delta->hdr = ap->delta->hdrbuf;
	ap->io->buffersize = state.buffersize;
	n = 2 * state.buffersize;
	if ((ap->io->mode & O_ACCMODE) != O_WRONLY)
		u = MAXUNREAD;
	else if (!(ap->format->flags & OUT))
		error(3, "%s: archive format not supported on output" , ap->format->name);
	else
		u = 0;
	if (!(ap->io->buffer = newof(0, char, n, u)))
		error(3, "%s: cannot allocate buffer", ap->name);
	ap->io->unread = u;
	ap->io->next = ap->io->last = ap->io->buffer + u;
	message((-7, "binit(%s) %s", ap->name, bstatus(ap->io)));
}

/*
 * skip files on tape fd
 */

int
bskip(register Archive_t* ap)
{
	long		c;
	int		skip = ap->io->skip;
#ifdef MTIOCTOP
	struct mtop	mt;
#ifdef MTEOM
	int		mteom = 1;
#endif
#ifdef MTFSF
	int		mtfsf = 1;
#endif
#endif

	if (ap->io->mode != O_RDONLY)
	{
		ap->io->next = ap->io->last = ap->io->buffer + ap->io->unread;
		ap->io->eof = 0;
	}
	while (skip)
	{
#ifdef MTIOCTOP
#ifdef MTEOM
		if (skip < 0 && mteom)
		{
			mt.mt_op = MTEOM;
			mt.mt_count = 1;
			if (ioctl(ap->io->fd, MTIOCTOP, &mt) >= 0)
			{
				if (ap->io->mode != O_RDONLY)
					ap->io->eof = 1;
				break;
			}
			mteom = 0;
		}
#endif
#ifdef MTFSF
		if (mtfsf)
		{
			mt.mt_op = MTFSF;
			mt.mt_count = 1;
			if (ioctl(ap->io->fd, MTIOCTOP, &mt) >= 0)
			{
				skip--;
				continue;
			}
			if (errno != ENOTTY)
			{
				if (ap->io->mode != O_RDONLY)
					ap->io->eof = 1;
				break;
			}
			mtfsf = 0;
		}
#endif
#endif
		while ((c = read(ap->io->fd, state.tmp.buffer, state.tmp.buffersize)) > 0);
		if (c < 0)
		{
			if (ap->io->mode != O_RDONLY)
				ap->io->eof = 1;
			break;
		}
		skip--;
	}
	return 0;
}

/*
 * fill input buffer at ap->io->last
 * if must!=0 then EOF causes query for next input volume file
 */

static int
bfill(register Archive_t* ap, int must)
{
	register int	c;

	if (ap->io->eof)
		return -1;
	if (ap->io->skip)
		ap->io->skip = bskip(ap);
	while ((c = read(ap->io->fd, ap->io->last, ap->io->buffersize)) <= 0)
	{
		if (must)
			newio(ap, c, 0);
		else
		{
			ap->io->eof = 1;
			return -1;
		}
	}
	message((-8, "read(%s,%d): %s", ap->name, c, show(ap->io->last, c)));
	ap->io->eof = 0;
	ap->io->last += c;
	ap->io->fill++;
	message((-7, "bfill(%s) %s", ap->name, bstatus(ap->io)));
	return 0;
}

/*
 * sum and convert a bread()/bget() chunk
 */

static void
chunk(register Archive_t* ap, char* t, char* f, register size_t n, char* o)
{
	if (ap->sum > 0)
	{
		if (ap->flags & SUM)
			FNVSUM(ap->memsum, f, n);
		else
		{
			ap->memsum = memsum(f, n, ap->memsum);
			ap->old.memsum = omemsum(f, n, ap->old.memsum);
		}
	}
	if (o)
	{
		if (t != f)
			memcpy(t, f, n);
		CONVERT(ap, t, n);
	}
}

/*
 * buffered char input
 * at least n chars required, m chars max
 * if b is 0 then n chars are skipped
 * if must!=0 then EOF causes query for next input volume file
 */

off_t
bread(register Archive_t* ap, void* ob, off_t n, off_t m, int must)
{
	register char*		s = (char*)ob;
	register ssize_t	c;
	char*			b;
	register off_t		r;
	register off_t		z;

	if (ap->io->eof)
		return -1;
	if (ob)
		message((-7, "bread(%s,%I*d,%I*d) %s", ap->name, sizeof(n), n, sizeof(m), m, bstatus(ap->io)));
	if (m <= 0)
		m = n;
	b = s;
	r = m;
	if (ap->io->blocked)
	{
		if (!s)
			b = s = state.tmp.buffer;
		while ((c = read(ap->io->fd, s, r > ap->io->buffersize ? ap->io->buffersize : r)) <= 0)
		{
			if (must)
				newio(ap, c, 0);
			else if (ap->io->empty)
			{
				ap->io->eof = 1;
				return -1;
			}
			else
			{
				if (c < 0)
					ap->io->eof = 1;
				else
					ap->io->empty = 1;
				break;
			}
		}
		ap->io->empty = 0;
		chunk(ap, s, s, c, ob);
		s += c;
		r -= c;
	}
	else
		for (;;)
		{
			if ((c = ap->io->last - ap->io->next) < r)
			{
				if (c > 0)
				{
					if (ob)
						memcpy(s, ap->io->next, c);
					chunk(ap, s, ap->io->next, c, ob);
					s += c;
					r -= c;
				}
				ap->io->next = ap->io->last = ap->io->buffer + ap->io->unread;
				if (!ob && ap->sum <= 0 && ap->io->seekable && (z = r / BUFFERSIZE) && lseek(ap->io->fd, z *= BUFFERSIZE, SEEK_CUR) >= 0)
				{
					s += z;
					r -= z;
				}
				if (bfill(ap, must) < 0)
					break;
			}
			else
			{
				chunk(ap, s, ap->io->next, r, ob);
				ap->io->next += r;
				s += r;
				r = 0;
				break;
			}
		}
	ap->io->count += (r = m - r);
	if (r < n)
	{
		if (ob && r)
		{
			bunread(ap, b, r);
			return 0;
		}
		return -1;
	}
#if DEBUG
	if (ob)
		message((-7, "bread(%s,%I*d@%I*d) %s: %s", ap->name, sizeof(r), r, sizeof(ap->io->count), ap->io->count, bstatus(ap->io), show(b, r)));
	else
		message((-7, "bread(%s) skip(%I*d@%I*d) %s", ap->name, sizeof(r), r, sizeof(ap->io->count), ap->io->count, bstatus(ap->io)));
#endif
	return r;
}

/*
 * pushback for next bread()
 */

void
bunread(register Archive_t* ap, void* b, register int n)
{
	ap->io->eof = 0;
	ap->io->count -= n;
	if (ap->io->next == (ap->io->buffer + ap->io->unread))
		ap->io->last = ap->io->next + n;
	else if ((ap->io->next -= n) < ap->io->buffer + ap->io->unread)
	{
		if (ap->io->next < ap->io->buffer)
			error(PANIC, "bunread(%s,%d): too much pushback", ap->name, n);
		if (b)
			memcpy(ap->io->next, b, n);
		REVERT(ap, ap->io->next, n);
	}
	message((-7, "bunread(%s,%d@%I*d) %s: %s", ap->name, n, sizeof(ap->io->count), ap->io->count, bstatus(ap->io), show(ap->io->next, n)));
}

/*
 * O_RDONLY bread() n chars and return a pointer to the char buffer
 * O_WRONLY return output buffer pointer and available size
 */

char*
bget(register Archive_t* ap, register off_t n, off_t* p)
{
	register char*	b;
	char*		t;
	size_t		i;
	size_t		j;
	size_t		m;
	int		must;

	if ((ap->io->mode & O_ACCMODE) == O_WRONLY)
	{
		if (p)
			*p = ap->io->last - ap->io->next;
		return ap->io->next;
	}
	if (n < 0)
	{
		n = -n;
		must = 0;
	}
	else if (n > 0)
		must = 1;
	else
	{
		if (!ap->io->eof && ap->io->seekable)
		{
			if (ap->io->last > ap->io->next)
				n = ap->io->last - ap->io->next;
			else if ((n = ap->io->size - (ap->io->offset + ap->io->count)) < 0)
				n = 0;
			else if (n > ap->io->buffersize)
				n = ap->io->buffersize;
		}
		if (p)
			*p = n;
		return n ? ap->io->next : (char*)0;
	}
	if (n > ap->io->buffersize)
	{
		i = ap->io->next - ap->io->buffer;
		j = ap->io->last - ap->io->buffer;
		m = roundof(n, PAX_DEFBUFFER * PAX_BLOCK);
		message((-8, "bget(%s,%I*d,%d): reallocate %u=>%d", ap->name, sizeof(n), n, must, ap->io->buffersize, m));
		if (!(b = newof(ap->io->buffer, char, 2 * m, ap->io->unread)))
			error(3, "%s: cannot reallocate buffer", ap->name);
		ap->io->buffersize = m;
		if (b != ap->io->buffer)
		{
			ap->io->buffer = b;
			ap->io->next = b + i;
			ap->io->last = b + j;
		}
	}
	if (p)
		*p = n;
	b = ap->io->next;
	ap->io->next += n;
	while (ap->io->next > ap->io->last)
	{
		if (ap->io->last > ap->io->buffer + ap->io->unread + ap->io->buffersize)
		{
                        i = ap->io->last - b;
			j = roundof(i, IOALIGN) - i;
                        t = ap->io->next = ap->io->buffer + ap->io->unread + j;
                        ap->io->last = t + i;
                        if (m = b - t)
			{
                        	while (i > m)
                        	{
                                	message((-8, "bget(%s,%I*d,%d) overlapping memcpy n=%I*d i=%d m=%d next=%p last=%p", ap->name, sizeof(n), n, must, sizeof(n), n, i, m, ap->io->next + n, ap->io->last));
                                	memcpy(t, b, m);
                                	t += m;
                                	b += m;
                                	i -= m;
                        	}
				message((-8, "bget(%s,%I*d,%d): slide %u align %u", ap->name, sizeof(n), n, must, i, j));
                        	memcpy(t, b, i);
			}
			b = ap->io->next;
			ap->io->next += n;
		}
		if (bfill(ap, must) < 0)
			return 0;
	}
	chunk(ap, b, b, n, b);
	ap->io->count += n;
	message((-7, "bget(%s,%I*d@%I*d,%d): %s", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count, must, show(b, n)));
	return b;
}

#ifndef bsave

/*
 * save current position for possible backup()
 */

void
bsave(register Archive_t* ap)
{
	state.backup = *ap->io;
	message((-7, "bsave(%s,@%I*d)", ap->name, sizeof(ap->io->count), ap->io->count));
}

#endif

/*
 * back up input to bsave()'d position and prime output buffer
 */

void
backup(register Archive_t* ap)
{
	register off_t	n;
	register off_t	m;
#ifdef MTIOCTOP
	struct mtop	mt;
#endif

	if (ap->format->backup)
		(*ap->format->backup)(&state, ap);
	else
	{
		message((-7, "backup(%s) old %s new %s", ap->name, bstatus(&state.backup), bstatus(ap->io)));
		if (ap->io->fill == state.backup.fill)
		{
			/*
			 * same buffer window
			 */

			m = ap->io->last - (ap->io->buffer + ap->io->unread);
			if ((n = lseek(ap->io->fd, -m, SEEK_CUR)) == -1)
			{
#ifdef MTIOCTOP
				mt.mt_op = MTBSR;
				mt.mt_count = 1;
				if (ioctl(ap->io->fd, MTIOCTOP, &mt))
					goto bad;
#else
				goto bad;
#endif
			}
			message((-7, "backup(%s) %I*d => %I*d", ap->name, sizeof(m), m, sizeof(n), n));
			ap->io->count = state.backup.count;
			ap->io->next = state.backup.next;
		}
		else
			error(PANIC, "%s: backup over intervening hard read not implemented yet", ap->name);
		ap->io->last = ap->io->buffer + ap->io->unread + state.blocksize;
		message((-7, "backup(%s) %s", ap->name, bstatus(ap->io)));
	}
	return;
 bad:
	error(3, "%s: cannot position %s archive for append", ap->name, ap->format->name);
}

/*
 * flush buffered input
 */

void
bflushin(register Archive_t* ap, int hard)
{
	ap->io->count += ap->io->last - ap->io->next;
	ap->io->next = ap->io->last = ap->io->buffer + ap->io->unread;
	if (hard && !ap->io->eof)
	{
		while (read(ap->io->fd, ap->io->next, ap->io->buffersize) > 0);
		ap->io->eof = 1;
	}
}

/*
 * buffered seek
 */

off_t
bseek(register Archive_t* ap, off_t pos, int op, int hard)
{
	off_t	l;
	off_t	u;

	message((-8, "bseek(%s,%I*d,%d,%d)", ap->name, sizeof(pos), pos, op, hard));
	if (hard)
	{
		if (op == SEEK_CUR)
			return -1;
	}
	else
	{
		l = ap->io->next - (ap->io->buffer + ap->io->unread);
		u = ap->io->last - ap->io->next;
		if (op == SEEK_CUR)
		{
			if ((pos + ap->io->count) < 0)
				return -1;
			if (-pos <= l && pos <= u)
			{
				ap->io->next += pos;
				return ap->io->count += pos;
			}
			pos += ap->io->count;
			op = SEEK_SET;
		}
		else if (op != SEEK_SET || pos < 0)
			return -1;
		else if (-pos <= (l + ap->io->count) && pos <= (u + ap->io->count))
		{
			ap->io->next += (pos - ap->io->count);
			return ap->io->count = pos;
		}
	}
	ap->io->next = ap->io->last = ap->io->buffer + ap->io->unread;
	message((-8, "lseek(%s,%I*d,%d)", ap->name, sizeof(pos), pos + ap->io->offset, op));
	if ((u = lseek(ap->io->fd, ap->io->offset + pos, op)) < 0 && (op != SEEK_SET || (u = pos - ap->io->count) < 0 || u > 0 && bread(ap, NiL, u, u, 1) != u || !(u += ap->io->count + ap->io->offset)))
		return -1;
	ap->io->empty = 0;
	ap->io->eof = 0;
	return ap->io->count = u - ap->io->offset;
}

/*
 * flush buffered output
 */

void
bflushout(register Archive_t* ap)
{
	register int	n;
	register int	c;

	if (n = ap->io->next - (ap->io->buffer + ap->io->unread))
	{
		ap->io->next = ap->io->buffer + ap->io->unread;
		while ((c = write(ap->io->fd, ap->io->next, n)) != n)
		{
			message((-8, "write(%s,%d): %s", ap->name, c, show(ap->io->next, c)));
			if (c <= 0)
				newio(ap, c, n);
			else
			{
				ap->io->next += c;
				n -= c;
			}
		}
		message((-8, "write(%s,%d): %s", ap->name, c, show(ap->io->next, c)));
		ap->io->next = ap->io->buffer + ap->io->unread;
	}
	message((-7, "bflushout(%s) %s", ap->name, bstatus(ap->io)));
}

/*
 * buffered output
 */

void
bwrite(register Archive_t* ap, void* ab, register off_t n)
{
	register char*	b = (char*)ab;
	register long	c;
	long		an;

	if (!ap->raw)
	{
		CONVERT(ap, b, n);
		an = ap->convert[SECTION(ap)].on ? n : 0;
		if (ap->sum > 0)
			ap->memsum = memsum(b, n, ap->memsum);
		if (state.checksum.sum && SECTION(ap) == SECTION_DATA)
			sumblock(state.checksum.sum, b, n);
	}
	if (ap->io->skip)
		ap->io->skip = bskip(ap);
	if (state.maxout && ap->io->count >= state.maxout)
	{
		bflushout(ap);
		newio(ap, 0, 0);
	}
	ap->io->count += n;
	if (ap->io->blocked)
	{
#if DEBUG
		if (n > 0)
			message((-7, "bwrite(%s,%I*d@%I*d): %s", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count + n, show(b, n)));
		else
			message((-7, "bwrite(%s,%I*d@%I*d):", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count + n));
#endif
		while ((c = write(ap->io->fd, b, n)) != n)
		{
			if (n <= 0)
			{
#ifdef MTIOCTOP
				{
					struct mtop	mt;

					mt.mt_op = MTWEOF;
					mt.mt_count = 1;
					if (ioctl(ap->io->fd, MTIOCTOP, &mt) >= 0)
						break;
				}
#endif
				error(3, "%s: cannot write tape EOF marks", ap->name);
			}
			if (c <= 0)
				newio(ap, c, n);
			else if ((n -= c) > 0)
				b += c;
			else
				break;
		}
	}
	else
	{
#if DEBUG
		if (n > 0)
			message((-7, "bwrite(%s,%I*d@%I*d): %s", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count + n, show(b, n)));
		else
			message((-7, "bwrite(%s,%I*d@%I*d):", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count + n));
#endif
		for (;;)
		{
			if ((c = ap->io->buffer + ap->io->unread + state.blocksize - ap->io->next) <= n)
			{
				if (c)
				{
					memcpy(ap->io->next, b, c);
					n -= c;
					b += c;
				}
				ap->io->next = ap->io->buffer + ap->io->unread;
				while ((c = write(ap->io->fd, ap->io->next, state.blocksize)) != state.blocksize)
				{
					if (c <= 0)
						newio(ap, c, n);
					else
					{
						memcpy(state.tmp.buffer, ap->io->buffer + ap->io->unread + c, state.blocksize - c);
						memcpy(ap->io->buffer + ap->io->unread, state.tmp.buffer, state.blocksize - c);
						ap->io->next = ap->io->buffer + ap->io->unread + state.blocksize - c;
						break;
					}
				}
				message((-8, "write(%s,%ld): %s", ap->name, c, show(ap->io->buffer + ap->io->unread, c)));
			}
			else
			{
				memcpy(ap->io->next, b, n);
				ap->io->next += n;
				break;
			}
		}
	}
	if (!ap->raw)
		REVERT(ap, ab, an);
}

/*
 * bwrite() n chars that have been placed in the
 * buffer returned by a previous bget()
 */

void
bput(register Archive_t* ap, register off_t n)
{
	ap->io->count += n;
	message((-7, "bput(%s,%I*d@%I*d): %s", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count, show(ap->io->next, n)));
	CONVERT(ap, ap->io->next, n);
	if (ap->sum > 0)
		ap->memsum = memsum(ap->io->next, n, ap->memsum);
	if (state.checksum.sum && SECTION(ap) == SECTION_DATA)
		sumblock(state.checksum.sum, ap->io->next, n);
	if ((ap->io->next += n) > ap->io->buffer + ap->io->unread + state.blocksize)
	{
		n = (ap->io->next - (ap->io->buffer + ap->io->unread)) - state.blocksize;
		ap->io->count -= n;

		/*
		 * flush out the buffer and slide over the remains
		 */

		ap->raw++;
		bwrite(ap, ap->io->next = ap->io->buffer + ap->io->unread + state.blocksize, n);
		ap->raw--;
	}
}

/*
 * return recommended io block size for fd
 * 0 returned for no recommendation
 */

long
bblock(int fd)
{
#ifdef MTIOCGETBLKINFO
	struct mtblkinfo	mt;

	return ioctl(fd, MTIOCGETBLKINFO, &mt) < 0 ? 0 : mt.recblksz;
#else
	return 0;
#endif
}

static struct
{
	char*		path;
	struct stat*	st;
} dev;

/*
 * find path name in /dev for <dev.st->st_dev,dev.st->st_ino>
 * called by ftwalk()
 */

static int
devpath(register Ftw_t* ftw)
{
	if (ftw->info == FTW_F && ftw->statb.st_rdev == dev.st->st_rdev && S_ISCHR(ftw->statb.st_mode) == S_ISCHR(dev.st->st_mode))
	{
		message((-1, "device name is %s", ftw->path));
		dev.path = strdup(ftw->path);
		return 1;
	}
	return 0;
}

/*
 * initilize tty file pointers for interactive prompting
 */

void
interactive(void)
{
	int	fd;

	if (!state.rtty)
	{
		fd = dup(2);
		if (!(state.rtty = sfopen(NiL, "/dev/tty", "r")) || !(state.wtty = sfopen(NiL, "/dev/tty", "w")))
			error(ERROR_SYSTEM|3, "cannot prompt for interactive input");
		sfsetbuf(state.rtty, NiL, 0);
		sfsetbuf(state.wtty, NiL, 0);
		if (fd >= 0)
			close(fd);
	}
}

/*
 * check for new input or output stream
 * c is the io count causing the newio()
 * n is the pending buffered io count
 */

void
newio(register Archive_t* ap, int c, int n)
{
	register char*	s;
	register char*	rw;
	char*		file;
	char*		io;
	char*		t;
	off_t		z;
	int		i;
	struct stat	st;

	Sfio_t*		cp = 0;
	Sfio_t*		pp = 0;
	int		oerrno = errno;

	if (!ap->part)
		ap->part++;
	if (ap->io->mode != O_RDONLY)
	{
		rw = "write";
		io = "output";
		ap->io->offset += ap->io->count - n;
		ap->io->count = n;
		z = ap->io->offset + ap->io->count;
		if (ap->format->putepilogue && (*ap->format->putepilogue)(&state, ap) < 0)
			return;
	}
	else
	{
		rw = "read";
		io = "input";
		z = ap->io->offset + ap->io->count;
	}
	if (fstat(ap->io->fd, &st) < 0)
		error(ERROR_SYSTEM|3, "%s: cannot stat %s", ap->name, io);
	errno = oerrno;
	switch (X_ITYPE(modex(st.st_mode)))
	{
	case X_IFBLK:
	case X_IFCHR:
		file = 0;
		break;
	default:
		if (ap->io->mode != O_RDONLY)
			switch (c < 0 ? errno : 0)
			{
			case 0:
#ifdef EFBIG
			case EFBIG:
#endif
#ifdef EDQUOT
			case EDQUOT:
#endif
				file = "file";
				break;
			default:
				error(ERROR_SYSTEM|3, "%s: %s %s error -- cannot recover", ap->name, io, rw);
				break;
			}
		else
			file = "file";
		break;
	}
	i = (eomprompt && *eomprompt == '!' && !*(eomprompt + 1)) ? 3 : 1;
	switch (c < 0 ? errno : 0)
	{
	case 0:
	case ENOSPC:
	case ENXIO:
		error(i, "%s: end of %s medium", ap->name, io);
		break;
	default:
		error(ERROR_SYSTEM|i, "%s: %s %s error", ap->name, io, rw);
		break;
	}
	if (ap->total == z)
		error(1, "%s: no %s on part %d", ap->name, io, ap->part--);
	else
		ap->total = z;
	if (!file && (ap->name == definput || ap->name == defoutput))
	{
		dev.path = 0;
		dev.st = &st;
		ftwalk("/dev", devpath, 0, NiL);
		ap->name = dev.path;
	}
	close(ap->io->fd);
	if (eomprompt && *eomprompt == '!')
		file = 0;
	if (file && ap->name != definput && ap->name != defoutput && strmatch(ap->name, "*.+([0-9])") && (s = strrchr(ap->name, '.')) && (int)strtol(++s, NiL, 10) == ap->part)
	{
		/*
		 * the parts will be ap->name in sequence
		 * the first part realloc the name with
		 * enough sequence space
		 */

		if (ap->part == 1)
		{
			c = s - ap->name;
			if (!(t = newof(0, char, s - ap->name, 16)))
				nospace();
			strcpy(t, ap->name);
			s = (ap->name = t) + c;
		}
		sfsprintf(s, 16, "%d", ap->part + 1);
		if ((ap->io->fd = open(ap->name, ap->io->mode|O_BINARY, st.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO))) >= 0)
			goto nextpart;
		error(ERROR_SYSTEM|1, "%s: cannot %s", ap->name, rw);
	}
	if (state.test & 0000040)
		file = 0;
	else if (file || ap->name == definput || ap->name == defoutput)
	{
		for (;;)
		{
			interactive();
			sfputc(state.wtty, CC_bel);
			sfprintf(state.wtty, "Enter part %d %s %s name: ", ap->part + 1, io, file ? file : "device");
			if (!(s = sfgetr(state.rtty, '\n', 1)))
			{
				sfputc(state.wtty, '\n');
				finish(2);
			}
			if (*s)
			{
				if (!file)
					break;
				if ((ap->io->fd = open(s, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) >= 0)
					break;
				error(ERROR_SYSTEM|1, "%s: cannot open", s);
			}
		}
		ap->name = strdup(s);
	}
	if (!file)
	{
		for (;;)
		{
			if (eomprompt && *eomprompt == '!')
			{

				if (!cp && !(cp = sfstropen()))
					nospace();
				sfprintf(cp, "%s %s %d", eomprompt + 1, rw, ap->part + 1);
				if (ap->name)
					sfprintf(cp, " %s", ap->name);
				if (!(s = sfstruse(cp)))
					nospace();
				s = (pp = sfpopen(pp, s, "r")) ? sfgetr(pp, '\n', 1) : (char*)0;
			}
			else
			{
				interactive();
				sfputc(state.wtty, CC_bel);
				if (eomprompt)
					sfprintf(state.wtty, eomprompt, ap->part + 1);
				if (!(s = sfgetr(state.rtty, '\n', 1)))
					sfputc(state.wtty, '\n');
			}
			if (!s)
				finish(2);
			if (*s == '!')
			{
				static char*	last;

				if (*++s)
				{
					if (last)
						free(last);
					last = strdup(s);
				}
				else
					s = last;
				if (!s)
					error(1, "no previous command");
				else if (n = system(s))
					error(1, "exit status %d", n);
			}
			else if (file = *s ? s : ap->name)
			{
				if ((ap->io->fd = open(file, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) >= 0)
					break;
				if (!strchr(file, '/'))
				{
					oerrno = errno;
					file = strtape(file, &t);
					if (!*t && (ap->io->fd = open(file, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) >= 0)
						break;
					errno = oerrno;
				}
				error(ERROR_SYSTEM|1, "cannot %s %s", rw, *s ? s : ap->name);
			}
			else
				error(1, "pathname required");
		}
		if (ap->name != file)
			ap->name = strdup(file);
		if (cp)
			sfclose(cp);
		if (pp)
			sfclose(pp);
	nextpart:
		ap->part++;
		error(1, "continuing %s %d %s on %s", ap->part == ap->volume + 1 ? "volume" : "part", ap->part, io, ap->name);
	}
	else
		ap->part++;
	if (ap->format->putprologue)
		(*ap->format->putprologue)(&state, ap, 0);
}
