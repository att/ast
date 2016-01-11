#pragma prototyped

/*
 * compress encoder/decoder
 *
 * compress/zcat discipline snarfed from BSD zopen by
 * Glenn Fowler
 * AT&T Research
 * 1999-06-23
 */

/*-
 * Copyright (c) 1985, 1986, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Diomidis Spinellis and James A. Woods, derived from original
 * work by Spencer Thomas and Joseph Orost.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*-
 * fcompress.c - File compression ala IEEE Computer, June 1984.
 *
 * Compress authors:
 *		Spencer W. Thomas	(decvax!utah-cs!thomas)
 *		Jim McKie		(decvax!mcvax!jim)
 *		Steve Davies		(decvax!vax135!petsd!peora!srd)
 *		Ken Turkowski		(decvax!decwrl!turtlevax!ken)
 *		James A. Woods		(decvax!ihnp4!ames!jaw)
 *		Joe Orost		(decvax!vax135!petsd!joe)
 *
 * Cleaned up and converted to library returning I/O streams by
 * Diomidis Spinellis <dds@doc.ic.ac.uk>.
 */

#include <codex.h>

#define MAGIC1		0x1f
#define MAGIC2		0x9d
#define HEADER		3

#define MINBITS		9
#define MAXBITS		16

#define	BITS		MAXBITS		/* Default bits. */
#define	HSIZE		69001		/* 95% occupancy */

/* A code_int must be able to hold 2**BITS values of type int, and also -1. */
typedef int32_t code_int;
typedef int32_t count_int;

typedef u_char char_type;

#define	BIT_MASK	0x1f		/* Defines for third byte of header. */
#define	BLOCK_MASK	0x80

/*
 * Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
 * a fourth header byte (for expansion).
 */
#define	INIT_BITS 9			/* Initial number of bits/code. */

#define	MAXCODE(n_bits)	((1 << (n_bits)) - 1)

typedef struct s_zstate {
	Codex_t* codex;
	enum {
		S_START, S_MIDDLE, S_EOF
	} zs_state;			/* State of computation */
	int zs_n_bits;			/* Number of bits/code. */
	int zs_maxbits;			/* User settable max # bits/code. */
	code_int zs_maxcode;		/* Maximum code, given n_bits. */
	code_int zs_maxmaxcode;		/* Should NEVER generate this code. */
	count_int zs_htab [HSIZE];
	u_short zs_codetab [HSIZE];
	code_int zs_hsize;		/* For dynamic table sizing. */
	code_int zs_free_ent;		/* First unused entry. */
	/*
	 * Block compression parameters -- after all codes are used up,
	 * and compression rate changes, start over.
	 */
	int zs_block_compress;
	int zs_clear_flg;
	long zs_ratio;
	count_int zs_checkpoint;
	int zs_offset;
	long zs_in_count;		/* Length of input. */
	long zs_bytes_out;		/* Length of compressed output. */
	long zs_sync_out;		/* bytes_out at last sync */
	long zs_out_count;		/* # of codes output (for debugging). */
	char_type zs_buf[BITS];
	union {
		struct {
			long zs_fcode;
			code_int zs_ent;
			code_int zs_hsize_reg;
			int zs_hshift;
		} w;			/* Write paramenters */
		struct {
			char_type *zs_stackp;
			int zs_finchar;
			code_int zs_code, zs_oldcode, zs_incode;
			int zs_roffset, zs_size;
			char_type zs_gbuf[BITS];
		} r;			/* Read parameters */
	} u;
} State_t;

/* Definitions to retain old variable names */
#define	state		zs->zs_state
#define	n_bits		zs->zs_n_bits
#define	maxbits		zs->zs_maxbits
#define	maxcode		zs->zs_maxcode
#define	maxmaxcode	zs->zs_maxmaxcode
#define	htab		zs->zs_htab
#define	codetab		zs->zs_codetab
#define	hsize		zs->zs_hsize
#define	free_ent	zs->zs_free_ent
#define	block_compress	zs->zs_block_compress
#define	clear_flg	zs->zs_clear_flg
#define	ratio		zs->zs_ratio
#define	checkpoint	zs->zs_checkpoint
#define	offset		zs->zs_offset
#define	in_count	zs->zs_in_count
#define	bytes_out	zs->zs_bytes_out
#define	sync_out	zs->zs_sync_out
#define	out_count	zs->zs_out_count
#define	buf		zs->zs_buf
#define	fcode		zs->u.w.zs_fcode
#define	hsize_reg	zs->u.w.zs_hsize_reg
#define	ent		zs->u.w.zs_ent
#define	hshift		zs->u.w.zs_hshift
#define	stackp		zs->u.r.zs_stackp
#define	finchar		zs->u.r.zs_finchar
#define	code		zs->u.r.zs_code
#define	oldcode		zs->u.r.zs_oldcode
#define	incode		zs->u.r.zs_incode
#define	roffset		zs->u.r.zs_roffset
#define	size		zs->u.r.zs_size
#define	gbuf		zs->u.r.zs_gbuf

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type as
 * the codetab.  The tab_suffix table needs 2**BITS characters.  We get this
 * from the beginning of htab.  The output stack uses the rest of htab, and
 * contains characters.  There is plenty of room for any possible stack
 * (stack used to be 8000 characters).
 */

#define	htabof(i)	htab[i]
#define	codetabof(i)	codetab[i]

#define	tab_prefixof(i)	codetabof(i)
#define	tab_suffixof(i)	((char_type *)(htab))[i]
#define	de_stack	((char_type *)&tab_suffixof(1 << BITS))

#define	CHECK_GAP 10000		/* Ratio check interval. */

/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */
#define	FIRST	257		/* First free entry. */
#define	CLEAR	256		/* Table clear output code. */

/*-
 * Algorithm from "A Technique for High Performance Data Compression",
 * Terry A. Welch, IEEE Computer Vol 17, No 6 (June 1984), pp 8-19.
 *
 * Algorithm:
 * 	Modified Lempel-Ziv method (LZW).  Basically finds common
 * substrings and replaces them with a variable size code.  This is
 * deterministic, and can be done on the fly.  Thus, the decompression
 * procedure needs no input table, but tracks the way the table was built.
 */

/*-
 * Output the given code.
 * Inputs:
 * 	code:	A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *		that n_bits =< (long)wordsize - 1.
 * Outputs:
 * 	Outputs code to the file.
 * Assumptions:
 *	Chars are 8 bits long.
 * Algorithm:
 * 	Maintain a BITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

static char_type lmask[9] =
	{0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
static char_type rmask[9] =
	{0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

static int
output(State_t* zs, Sfio_t* f, code_int ocode, Sfdisc_t* dp)
{
	register int bits, r_off;
	register char_type *bp;

	r_off = offset;
	bits = n_bits;
	bp = buf;
	if (ocode >= 0) {
		/* Get to the first byte. */
		bp += (r_off >> 3);
		r_off &= 7;
		/*
		 * Since ocode is always >= 8 bits, only need to mask the first
		 * hunk on the left.
		 */
		*bp = (*bp & rmask[r_off]) | ((ocode << r_off) & lmask[r_off]);
		bp++;
		bits -= (8 - r_off);
		ocode >>= 8 - r_off;
		/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
		if (bits >= 8) {
			*bp++ = ocode;
			ocode >>= 8;
			bits -= 8;
		}
		/* Last bits. */
		if (bits)
			*bp = ocode;
		offset += n_bits;
		if (offset == (n_bits << 3)) {
			bp = buf;
			bits = n_bits;
			bytes_out += bits;
			if (sfwr(f, bp, bits, dp) != bits)
				return (-1);
			bp += bits;
			bits = 0;
			offset = 0;
		}
		/*
		 * If the next entry is going to be too big for the ocode size,
		 * then increase it, if possible.
		 */
		if (free_ent > maxcode || (clear_flg > 0)) {
		       /*
			* Write the whole buffer, because the input side won't
			* discover the size increase until after it has read it.
			*/
			if (offset > 0) {
				if (sfwr(f, buf, n_bits, dp) != n_bits)
					return (-1);
				bytes_out += n_bits;
			}
			offset = 0;

			if (clear_flg) {
				maxcode = MAXCODE(n_bits = INIT_BITS);
				clear_flg = 0;
			} else {
				n_bits++;
				if (n_bits == maxbits)
					maxcode = maxmaxcode;
				else
					maxcode = MAXCODE(n_bits);
			}
		}
	} else {
		/* At EOF, write the rest of the buffer. */
		if (offset > 0) {
			offset = (offset + 7) / 8;
			if (sfwr(f, buf, offset, dp) != offset)
				return (-1);
			bytes_out += offset;
		}
		offset = 0;
	}
	return (0);
}

/*-
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 * 	stdin
 * Outputs:
 * 	code or -1 is returned.
 */
static code_int
getcode(State_t* zs, Sfio_t* f, Sfdisc_t* dp)
{
	register code_int gcode;
	register int r_off, bits;
	register char_type *bp;

	bp = gbuf;
	if (clear_flg > 0 || roffset >= size || free_ent > maxcode) {
		/*
		 * If the next entry will be too big for the current gcode
		 * size, then we must increase the size.  This implies reading
		 * a new buffer full, too.
		 */
		if (free_ent > maxcode) {
			n_bits++;
			if (n_bits == maxbits)	/* Won't get any bigger now. */
				maxcode = maxmaxcode;
			else
				maxcode = MAXCODE(n_bits);
		}
		if (clear_flg > 0) {
			maxcode = MAXCODE(n_bits = INIT_BITS);
			clear_flg = 0;
		}
		size = sfrd(f, gbuf, n_bits, dp);
		if (size <= 0)			/* End of file. */
			return (-1);
		roffset = 0;
		/* Round size down to integral number of codes. */
		size = (size << 3) - (n_bits - 1);
	}
	r_off = roffset;
	bits = n_bits;

	/* Get to the first byte. */
	bp += (r_off >> 3);
	r_off &= 7;

	/* Get first part (low order bits). */
	gcode = (*bp++ >> r_off);
	bits -= (8 - r_off);
	r_off = 8 - r_off;	/* Now, roffset into gcode word. */

	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if (bits >= 8) {
		gcode |= *bp++ << r_off;
		r_off += 8;
		bits -= 8;
	}

	/* High order bits. */
	gcode |= (*bp & rmask[bits]) << r_off;
	roffset += n_bits;

	return (gcode);
}

static void
cl_hash(State_t* zs, register count_int cl_hsize)		/* Reset code table. */
{
	register count_int *htab_p;
	register long i, m1;

	m1 = -1;
	htab_p = htab + cl_hsize;
	i = cl_hsize - 16;
	do {			/* Might use Sys V memset(3) here. */
		*(htab_p - 16) = m1;
		*(htab_p - 15) = m1;
		*(htab_p - 14) = m1;
		*(htab_p - 13) = m1;
		*(htab_p - 12) = m1;
		*(htab_p - 11) = m1;
		*(htab_p - 10) = m1;
		*(htab_p - 9) = m1;
		*(htab_p - 8) = m1;
		*(htab_p - 7) = m1;
		*(htab_p - 6) = m1;
		*(htab_p - 5) = m1;
		*(htab_p - 4) = m1;
		*(htab_p - 3) = m1;
		*(htab_p - 2) = m1;
		*(htab_p - 1) = m1;
		htab_p -= 16;
	} while ((i -= 16) >= 0);
	for (i += 16; i > 0; i--)
		*--htab_p = m1;
}

static int
cl_block(State_t* zs, Sfio_t* f, Sfdisc_t* dp)/* Table clear for block compress. */
{
	register long rat;

	checkpoint = in_count + CHECK_GAP;

	if (sync_out == bytes_out)
		return(0);
	if (in_count > 0x007fffff) {	/* Shift will overflow. */
		rat = bytes_out >> 8;
		if (rat == 0)		/* Don't divide by zero. */
			rat = 0x7fffffff;
		else
			rat = in_count / rat;
	} else
		rat = (in_count << 8) / bytes_out;	/* 8 fractional bits. */
	if (rat > ratio)
		ratio = rat;
	else {
		ratio = 0;
		cl_hash(zs, (count_int) hsize);
		free_ent = FIRST;
		clear_flg = 1;
		if (output(zs, f, (code_int) CLEAR, dp) == -1)
			return (-1);
	}
	sync_out = bytes_out;
	return (0);
}

static int
lzw_ident(Codexmeth_t* meth, const void* head, size_t headsize, char* name, size_t namesize)
{
	unsigned char*	h = (unsigned char*)head;

	if (headsize >= 3 && h[0] == MAGIC1 && h[1] == MAGIC2)
	{
		strncopy(name, meth->name, namesize);
		return 1;
	}
	return 0;
}

static int
lzw_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	zs;
	const char*		s;
	char*			e;

	if (!(zs = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	if (!(s = args[2]))
		maxbits = BITS;
	else if ((maxbits = strton(s, &e, NiL, 0)) < MINBITS || maxbits > MAXBITS || *e)
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: maximum bits per code must be in [%d..%d]", s, MINBITS, MAXBITS);
		return -1;
	}
	zs->codex = p;
	p->data = zs;
	return 0;
}

static int
lzw_init(Codex_t* p)
{
	register State_t*	zs = (State_t*)p->data;
	u_char			header[HEADER];

	hsize = HSIZE;			/* For dynamic table sizing. */
	block_compress = BLOCK_MASK;
	clear_flg = 0;
	ratio = 0;
	checkpoint = CHECK_GAP;
	in_count = 1;			/* Length of input. */
	roffset = 0;
	state = S_START;
	size = 0;
	if (p->flags & CODEX_ENCODE)
	{
		header[0] = MAGIC1;
		header[1] = MAGIC2;
		header[2] = ((maxbits) | block_compress) & 0xff;
		if (sfwr(p->sp, header, sizeof(header), &p->sfdisc) != sizeof(header))
			return -1;
		offset = 0;
		sync_out = 0;
		bytes_out = sizeof(header);
		out_count = 0;			/* # of codes output (for debugging). */
		hshift = 0;
		for (fcode = (long)hsize; fcode < 65536L; fcode *= 2L)
			hshift++;
		hshift = 8 - hshift;	/* Set hash code range bound. */
		hsize_reg = hsize;
		cl_hash(zs, (count_int)hsize_reg);	/* Clear hash table. */
	}
	else
	{
		/* Check the magic number */
		if (sfrd(p->sp, header, sizeof(header), &p->sfdisc) != sizeof(header) ||
		    header[0] != MAGIC1 || header[1] != MAGIC2)
			return (-1);
		maxbits = header[2];	/* Set -b from file. */
		block_compress = maxbits & BLOCK_MASK;
		maxbits &= BIT_MASK;
		if (maxbits > BITS)
			return (-1);
		/* As above, initialize the first 256 entries in the table. */
		for (code = 255; code >= 0; code--) {
			tab_prefixof(code) = 0;
			tab_suffixof(code) = (char_type) code;
		}
	}
	maxcode = MAXCODE(n_bits = INIT_BITS);
	maxmaxcode = 1L << maxbits;
	free_ent = block_compress ? FIRST : 256;
	return 0;
}

/*
 * lzw sync (table flush)
 */

int
lzw_sync(Codex_t* p)
{
	register State_t*	zs = (State_t*)p->data;

	if ((zs->codex->flags & CODEX_ENCODE) && cl_block(zs, p->sp, &p->sfdisc))
		return -1;
	return 0;
}

/*
 * lzw done
 */

int
lzw_done(Codex_t* p)
{
	register State_t*	zs = (State_t*)p->data;

	if (zs->codex->flags & CODEX_ENCODE)
	{
		if (output(zs, p->sp, (code_int)ent, &p->sfdisc))
			return -1;
		out_count++;
		if (output(zs, p->sp, (code_int)-1, &p->sfdisc))
			return -1;
	}
	return 0;
}

/*
 * Decompress read.  This routine adapts to the codes in the file building
 * the "string" table on-the-fly; requiring no table to be stored in the
 * compressed file.  The tables used herein are shared with those of the
 * compress() routine.  See the definitions above.
 */
static ssize_t
lzw_read(Sfio_t* f, Void_t* rbp, size_t num, Sfdisc_t* dp)
{
	State_t *zs = (State_t*)CODEX(dp)->data;
	register u_int count;
	u_char *bp;

	if (num == 0)
		return (0);

	count = num;
	bp = (u_char *)rbp;
	switch (state) {
	case S_START:
		state = S_MIDDLE;
		break;
	case S_MIDDLE:
		goto middle;
	case S_EOF:
		goto eof;
	}


	finchar = oldcode = getcode(zs, f, dp);
	if (oldcode == -1)	/* EOF already? */
		return (0);	/* Get out of here */

	/* First code must be 8 bits = char. */
	*bp++ = (u_char)finchar;
	count--;
	stackp = de_stack;

	while ((code = getcode(zs, f, dp)) > -1) {

		if ((code == CLEAR) && block_compress) {
			for (code = 255; code >= 0; code--)
				tab_prefixof(code) = 0;
			clear_flg = 1;
			free_ent = FIRST - 1;
			if ((code = getcode(zs, f, dp)) == -1)	/* O, untimely death! */
				break;
		}
		incode = code;

		/* Special case for KwKwK string. */
		if (code >= free_ent) {
			*stackp++ = finchar;
			code = oldcode;
		}

		/* Generate output characters in reverse order. */
		while (code >= 256) {
			*stackp++ = tab_suffixof(code);
			code = tab_prefixof(code);
		}
		*stackp++ = finchar = tab_suffixof(code);

		/* And put them out in forward order.  */
middle:		do {
			if (count-- == 0)
				return (num);
			*bp++ = *--stackp;
		} while (stackp > de_stack);

		/* Generate the new entry. */
		if ((code = free_ent) < maxmaxcode) {
			tab_prefixof(code) = (u_short) oldcode;
			tab_suffixof(code) = finchar;
			free_ent = code + 1;
		}

		/* Remember previous code. */
		oldcode = incode;
	}
	state = S_EOF;
eof:	return (num - count);
}

/*-
 * compress write
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

static ssize_t
lzw_write(Sfio_t* f, const Void_t* wbp, size_t num, Sfdisc_t* dp)
{
	State_t *zs = (State_t*)CODEX(dp)->data;
	register code_int i;
	register int c, disp;
	const u_char *bp;
	int count;

	if (num == 0)
		return (0);

	count = num;
	bp = (u_char *)wbp;
	if (state == S_START)
	{
		state = S_MIDDLE;
		ent = *bp++;
		count--;
	}
	for (i = 0; count--;) {
		c = *bp++;
		in_count++;
		fcode = (long)(((long)c << maxbits) + ent);
		i = ((c << hshift) ^ ent);	/* Xor hashing. */

		if (htabof(i) == fcode) {
			ent = codetabof(i);
			continue;
		} else if ((long)htabof(i) < 0)	/* Empty slot. */
			goto nomatch;
		disp = hsize_reg - i;	/* Secondary hash (after G. Knott). */
		if (i == 0)
			disp = 1;
probe:		if ((i -= disp) < 0)
			i += hsize_reg;

		if (htabof(i) == fcode) {
			ent = codetabof(i);
			continue;
		}
		if ((long)htabof(i) >= 0)
			goto probe;
nomatch:	if (output(zs, f, (code_int) ent, dp) == -1)
			return (-1);
		out_count++;
		ent = c;
		if (free_ent < maxmaxcode) {
			codetabof(i) = free_ent++;	/* code -> hashtable */
			htabof(i) = fcode;
		} else if ((count_int)in_count >=
		    checkpoint && block_compress) {
			if (cl_block(zs, f, dp) == -1)
				return (-1);
		}
	}
	return (num);
}

Codexmeth_t	codex_compress =
{
	"compress",
	"compress LZW compression. The first parameter is the maximum number"
	" of bits per code { 9 - 16 }. The default is 16.",
	0,
	CODEX_DECODE|CODEX_ENCODE|CODEX_COMPRESS,
	0,
	lzw_ident,
	lzw_open,
	0,
	lzw_init,
	lzw_done,
	lzw_read,
	lzw_write,
	lzw_sync,
	0,
	0,
	0,
	0,
	CODEXNEXT(compress)
};

CODEXLIB(compress)
