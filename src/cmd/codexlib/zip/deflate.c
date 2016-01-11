#pragma prototyped

/*
 * zip deflate decoder
 */

/* inflate.c -- Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 */

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.	 Thank you.
   [The history has been moved to the file ChangeLog.]
 */

/*
   Inflate deflated (PKZIP's method 8 compressed) data.	 The compression
   method searches for as much of the current string of bytes (up to a
   length of 258) in the previous 32K bytes.  If it doesn't find any
   matches (of at least length 3), it codes the next byte.  Otherwise, it
   codes the length of the matched string and its distance backwards from
   the current position.  There is a single Huffman code that codes both
   single bytes (called "literals") and match lengths.	A second Huffman
   code codes the distance information, which follows a length code.  Each
   length or distance code actually represents a base value and a number
   of "extra" (sometimes zero) bits to get to add to the base value.  At
   the end of each deflated block is a special end-of-block (EOB) literal/
   length code.	 The decoding process is basically: get a literal/length
   code; if EOB then done; if a literal, emit the decoded byte; if a
   length then get the distance and emit the referred-to bytes from the
   sliding window of previously emitted data.

   There are (currently) three kinds of inflate blocks: stored, fixed, and
   dynamic.  The compressor outputs a chunk of data at a time and decides
   which method to use on a chunk-by-chunk basis.  A chunk might typically
   be 32K to 64K, uncompressed.	 If the chunk is uncompressible, then the
   "stored" method is used.  In this case, the bytes are simply stored as
   is, eight bits per byte, with none of the above coding.  The bytes are
   preceded by a count, since there is no longer an EOB code.

   If the data are compressible, then either the fixed or dynamic methods
   are used.  In the dynamic method, the compressed data are preceded by
   an encoding of the literal/length and distance Huffman codes that are
   to be used to decode this block.  The representation is itself Huffman
   coded, and so is preceded by a description of that code.  These code
   descriptions take up a little space, and so for small blocks, there is
   a predefined set of codes, called the fixed codes.  The fixed method is
   used if the block ends up smaller that way (usually for quite small
   chunks); otherwise the dynamic method is used.  In the latter case, the
   codes are customized to the probabilities in the current block and so
   can code it much better than the pre-determined fixed codes can.

   The Huffman codes themselves are decoded using a multi-level table
   lookup, in order to maximize the speed of decoding plus the speed of
   building the decoding tables.  See the comments below that precede the
   lbits and dbits tuning parameters.
 */


/*
   Notes beyond the 1.93a appnote.txt:

   1. Distance pointers never point before the beginning of the output
	  stream.
   2. Distance pointers can point back across blocks, up to 32k away.
   3. There is an implied maximum of 7 bits for the bit length table and
      15 bits for the actual data.
   4. If only one code exists, then it is encoded using one bit.  (Zero
      would be more efficient, but perhaps a little confusing.)	 If two
	  codes exist, they are coded using one bit each (0 and 1).
   5. There is no way of sending zero distance codes--a dummy must be
      sent if there are none.  (History: a pre 2.0 version of PKZIP would
      store blocks with no distance codes, but this was discovered to be
      too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
      zero distance codes, which is sent as one code of zero bits in
      length.
   6. There are up to 286 literal/length codes.	 Code 256 represents the
      end-of-block.  Note however that the static length tree defines
      288 codes just to fill out the Huffman codes.  Codes 286 and 287
      cannot be used though, since there is no length base or extra bits
      defined for them.	 Similarily, there are up to 30 distance codes.
      However, static trees define 32 codes (all 5 bits) to fill out the
      Huffman codes, but the last two had better not show up in the data.
   7. Unzip can check dynamic Huffman blocks for complete code sets.
      The exception is that a single code would not be complete (see #4).
   8. The five bits following the block type is really the number of
      literal codes sent minus 257.
   9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
      (1+6+6).	Therefore, to output three times the length, you output
      three codes (1+1+1), whereas to output four times the same length,
      you only need two codes (1+3).  Hmm.
  10. In the tree reconstruction algorithm, Code = Code + Increment
      only if BitLength(i) is not zero.	 (Pretty obvious.)
  11. Correction: 4 Bits: # of Bit Length codes - 4  (4 - 19)
  12. Note: length code 284 can represent 227-258, but length code 285
      really is 258.  The last length deserves its own, short code
      since it gets used a lot in very redundant files.	 The length
      258 is special since 258 - 3 (the min match length) is 255.
  13. The literal/length and distance code bit lengths are read as a
      single stream of lengths.	 It is possible (and advantageous) for
      a repeat code (16, 17, or 18) to go across the boundary between
      the two sets of lengths.
 */

#include "zip.h"
#include "huff.h"

#define WINDOW		32768

#define STORED_BLOCK	0
#define STATIC_TREES	1
#define DYNAMIC_TREES	2

/* Save to local */
#define BITS_LOCAL \
	ulg bit_buf = state->bit_buf; \
	ulg bit_len = state->bit_len;

/* Restore to state */
#define BITS_GLOBAL \
	state->bit_buf = bit_buf; \
	state->bit_len = bit_len;

#define MASK_BITS(n)	((((ulg)1)<<(n))-1)
#define NEXTBYTE(p)	(((p)->ip < (p)->ie) ? (int)*(p)->ip++ : fill(p))
#define NEEDBITS(p,n)	{while (bit_len<(n)){bit_buf|=((ulg)NEXTBYTE(p))<<bit_len;bit_len+=8;}}
#define GETBITS(n)	(bit_buf & MASK_BITS(n))
#define DUMPBITS(n)	{bit_buf>>=(n);bit_len-=(n);}

struct State_s; typedef struct State_s State_t;

struct State_s
{
	Codex_t*	codex;

	Vmalloc_t*	vm;		/* memory region for tl, td */

	uch		buf[SF_BUFSIZE];
	uch		slide[WINDOW];

	int		method;

	int		eof;		/* init to 0 from this member on */
	int		last;

	uch*		ip;
	uch*		ie;

	ulg		wp;		/* current position in slide */
	Huff_t*		fixed_tl;	/* inflate static */
	Huff_t*		fixed_td;	/* inflate static */
	int		fixed_bl;	/* inflate static */
	int		fixed_bd;	/* inflate static */
	ulg		bit_buf;	/* bit buffer */
	ulg		bit_len;	/* bits in bit buffer */
	ulg		copy_len;
	ulg		copy_pos;
	Huff_t*		tl;		/* literal length state table */
	Huff_t*		td;		/* literal distance state table */
	int		bl;		/* number of bits decoded by tl[] */
	int		bd;		/* number of bits decoded by td[] */
};

static int
fill(State_t* state)
{
	ssize_t	r;

	if (state->eof)
		return 0;
	if ((r = sfrd(state->codex->sp, state->buf, sizeof(state->buf), &state->codex->sfdisc)) <= 0)
	{
		state->eof = 1;
		return 0;
	}
	state->ie = (state->ip = state->buf) + r;
	return *state->ip++;
}

/* The inflate algorithm uses a sliding 32K byte window on the uncompressed
   stream to find repeated byte strings.  This is implemented here as a
   circular buffer.  The index is updated simply by incrementing and then
   and'ing with 0x7fff (32K-1). */
/* It is left to other modules to supply the 32K area.	It is assumed
   to be usable as if it were declared "uch slide[32768];" or as just
   "uch *slide;" and then malloc'ed in the latter case.	 The definition
   must be in unzip.h, included above. */

#define lbits 9			/* bits in base literal/length lookup table */
#define dbits 6			/* bits in base distance lookup table */

/* Tables for deflate from PKZIP's appnote.txt. */
static ush cplens[] = {		/* Copy lengths for literal codes 257..285 */
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
	/* note: see note #13 above about the 258 in this list. */
static ush cplext[] = {		/* Extra bits for literal codes 257..285 */
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */
static ush cpdist[] = {		/* Copy offsets for distance codes 0..29 */
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577};
static ush cpdext[] = {		/* Extra bits for distance codes */
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
	12, 12, 13, 13};

/* inflate (decompress) the codes in a deflated (compressed) block.
   Return an error code or zero if it all goes ok. */

static uch*
inflate_codes(State_t* state, register uch* p, register uch* e)
{
	register ulg	x;	/* table entry flag/number of extra bits */
	Huff_t*		t;	/* pointer to table entry */
	Huff_t*		tl;	/* literal length state table */
	Huff_t*		td;	/* literal distance state table */
	int		bl;	/* number of bits decoded by tl[] */
	int		bd;	/* number of bits decoded by td[] */
	ulg		l;
	ulg		w;
	ulg		d;
	uch*		slide;
	BITS_LOCAL;

	if (p == e)
		return p;
	slide = state->slide;
	tl = state->tl;
	td = state->td;
	bl = state->bl;
	bd = state->bd;
	w = state->wp;

	/* inflate the coded data until end of block */

	for (;;)
	{
		NEEDBITS(state, (ulg)bl);
		t = tl + GETBITS(bl);
		x = t->e;
		while (x > 16)
		{
			if (x == 99)
				return 0;
			DUMPBITS(t->b);
			x -= 16;
			NEEDBITS(state, x);
			t = t->v.t + GETBITS(x);
			x = t->e;
		}
		DUMPBITS(t->b);
		if (x == 16)
		{
			/* literal */
			w &= WINDOW - 1;
			*p++ = slide[w++] = (uch)t->v.n;
			if (p >= e)
			{
				state->wp = w;
				BITS_GLOBAL;
				return p;
			}
		}
		else if (x == 15)
			break;
		else
		{
			/* get length of block to copy */

			NEEDBITS(state, x);
			l = t->v.n + GETBITS(x);
			DUMPBITS(x);

			/* decode distance of block to copy */

			NEEDBITS(state, (ulg)bd);
			t = td + GETBITS(bd);
			x = t->e;
			while (x > 16)
			{
				if (x == 99)
					return 0;
				DUMPBITS(t->b);
				x -= 16;
				NEEDBITS(state, x);
				t = t->v.t + GETBITS(x);
				x = t->e;
			}
			DUMPBITS(t->b);
			NEEDBITS(state, x);
			d = w - t->v.n - GETBITS(x);
			DUMPBITS(x);

			/* do the copy */

			while (l--)
			{
				d &= WINDOW - 1;
				w &= WINDOW - 1;
				*p++ = slide[w++] = slide[d++];
				if (p >= e)
				{
					state->copy_len = l;
					state->wp = w;
					state->copy_pos = d;
					BITS_GLOBAL;
					return p;
				}
			}
		}
	}
	state->wp = w;
	state->method = -1;
	BITS_GLOBAL;
	return p;
}

/* "decompress" an inflated type 0 (stored) block. */

static uch*
inflate_stored(State_t* state, register uch* p, register uch* e)
{
	ulg		l;
	ulg		w;
	BITS_LOCAL;

	/* go to byte boundary */

	l = bit_len & 7;
	DUMPBITS(l);

	/* get the length and its complement */

	NEEDBITS(state, 16);
	l = GETBITS(16);
	DUMPBITS(16);
	NEEDBITS(state, 16);
	if (l != (ulg)((~bit_buf) & 0xffff))
	{
		BITS_GLOBAL;
		return 0;
	}
	DUMPBITS(16);

	/* read and output the compressed data */

	w = state->wp;
	for (;;)
	{
		if (!l--)
		{
			state->method = -1;
			break;
		}
		w &= WINDOW - 1;
		NEEDBITS(state, 8);
		*p++ = state->slide[w++] = (uch)GETBITS(8);
		DUMPBITS(8);
		if (p >= e)
		{
			state->copy_len = l;
			break;
		}
	}
	state->wp = w;
	BITS_GLOBAL;
	return p;
}

/* decompress an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom state, or at least precompute the
   Huffman tables. */

static int
inflate_fixed(State_t* state)
{
	/* if first time, set up tables for fixed blocks */

	if (!state->fixed_tl)
	{
		Huff_t*		tl;		/* literal/length code table */
		int		i;		/* temporary variable */
		ulg		l[288];		/* length list for huff() */

		/* literal table */

		for (i = 0; i < 144; i++)
			l[i] = 8;
		for (; i < 256; i++)
			l[i] = 9;
		for (; i < 280; i++)
			l[i] = 7;

		/* make a complete, but wrong code set */

		for (; i < 288; i++)	 
			l[i] = 8;
		state->fixed_bl = 7;
		if (huff(l, 288, 257, cplens, cplext, &tl, &state->fixed_bl, state->vm))
			return -1;

		/* distance table -- make an incomplete code set */

		for (i = 0; i < 30; i++)
			l[i] = 5;
		state->fixed_bd = 5;
		if (huff(l, 30, 0, cpdist, cpdext, &state->fixed_td, &state->fixed_bd, state->vm) > 1)
			return -1;
		state->fixed_tl = tl;
	}
	state->tl = state->fixed_tl;
	state->td = state->fixed_td;
	state->bl = state->fixed_bl;
	state->bd = state->fixed_bd;
	return 0;
}

/* decompress an inflated type 2 (dynamic Huffman codes) block. */

static int
inflate_dynamic(State_t* state)
{
	int	i;		/* temporary variables */
	ulg	j;
	ulg	l;		/* last length */
	ulg	n;		/* number of lengths to get */
	Huff_t*	tl;		/* literal/length code table */
	Huff_t*	td;		/* distance code table */
	int	bl;		/* lookup bits for tl */
	int	bd;		/* lookup bits for td */
	ulg	nb;		/* number of bit length codes */
	ulg	nl;		/* number of literal/length codes */
	ulg	nd;		/* number of distance codes */
#ifdef PKZIP_BUG_WORKAROUND
	ulg ll[288+32];		/* literal/length and distance code lengths */
#else
	ulg ll[286+30];		/* literal/length and distance code lengths */
#endif

	static ulg border[] = {  /* Order of the bit length code lengths */
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

	BITS_LOCAL;

	state->fixed_tl = 0;
	vmclear(state->vm);

	/* read in table lengths */

	NEEDBITS(state, 5);
	nl = 257 + GETBITS(5);	/* number of literal/length codes */
	DUMPBITS(5);
	NEEDBITS(state, 5);
	nd = 1 + GETBITS(5);	/* number of distance codes */
	DUMPBITS(5);
	NEEDBITS(state, 4);
	nb = 4 + GETBITS(4);	/* number of bit length codes */
	DUMPBITS(4);
#ifdef PKZIP_BUG_WORKAROUND
	if (nl > 288 || nd > 32)
#else
	if (nl > 286 || nd > 30)
#endif
	{
		/* bad lengths */

		BITS_GLOBAL;
		return -1;
	}

	/* read in bit-length-code lengths */

	for (j = 0; j < nb; j++)
	{
		NEEDBITS(state, 3);
		ll[border[j]] = GETBITS(3);
		DUMPBITS(3);
	}
	for (; j < 19; j++)
		ll[border[j]] = 0;

	/* build decoding table for trees--single level, 7 bit lookup */

	bl = 7;
	if (i = huff(ll, 19, 19, NULL, NULL, &tl, &bl, state->vm))
	{
		/* incomplete code set */

		BITS_GLOBAL;
		return -1;
	}

	/* read in literal and distance code lengths */

	n = nl + nd;
	i = l = 0;
	while ((ulg)i < n)
	{
		NEEDBITS(state, (ulg)bl);
		j = (td = tl + (GETBITS(bl)))->b;
		DUMPBITS(j);
		j = td->v.n;
		if (j < 16)		/* length of code in bits (0..15) */
			ll[i++] = l = j;/* save last length in l */
		else if (j == 16)	/* repeat last length 3 to 6 times */
		{
			NEEDBITS(state, 2);
			j = 3 + GETBITS(2);
			DUMPBITS(2);
			if ((ulg)i + j > n)
			{
				BITS_GLOBAL;
				return -1;
			}
			while (j--)
				ll[i++] = l;
		}
		else if (j == 17)	/* 3 to 10 zero length codes */
		{
			NEEDBITS(state, 3);
			j = 3 + GETBITS(3);
			DUMPBITS(3);
			if ((ulg)i + j > n)
			{
				BITS_GLOBAL;
				return -1;
			}
			while (j--)
				ll[i++] = 0;
			l = 0;
		}
		else			/* j == 18: 11 to 138 0 length codes */
		{
			NEEDBITS(state, 7);
			j = 11 + GETBITS(7);
			DUMPBITS(7);
			if ((ulg)i + j > n)
			{
				BITS_GLOBAL;
				return -1;
			}
			while (j--)
				ll[i++] = 0;
			l = 0;
		}
	}
	BITS_GLOBAL;

	/* free decoding table for trees */

	vmclear(state->vm);

	/* build the decoding tables for literal/length and distance codes */

	bl = lbits;
	i = huff(ll, nl, 257, cplens, cplext, &tl, &bl, state->vm);
	if (bl == 0)				  /* no literals or lengths */
		i = 1;
	if (i)
	{
		/* incomplete code set */

		if (i == 1 && state->codex->disc->errorf)
			(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: incomplete literal tree", state->codex->meth->name);
		return -1;
	}
	bd = dbits;
	i = huff(ll + nl, nd, 0, cpdist, cpdext, &td, &bd, state->vm);
	if (bd == 0 && nl > 257)
	{
		/* lengths but no distances */

		if (state->codex->disc->errorf)
			(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: incomplete distance tree", state->codex->meth->name);
		return -1;
	}
	if (i == 1)
	{
#ifdef PKZIP_BUG_WORKAROUND
		i = 0;
#else
		if (state->codex->disc->errorf)
			(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: incomplete distance tree", state->codex->meth->name);
#endif
	}
	if (i)
		return -1;
	state->tl = tl;
	state->td = td;
	state->bl = bl;
	state->bd = bd;
	return 0;
}

static int
deflate_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	State_t*	state;
	Vmalloc_t*	vm;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
	{
		vmclose(vm);
		return -1;
	}
	state->vm = vm;
	state->codex = p;
	p->data = state;
	return 0;
}

static int
deflate_close(Codex_t* p)
{
	State_t* state = (State_t*)p->data;

	if (!state)
		return -1;
	if (state->vm)
		vmclose(state->vm);
	free(state);
	return 0;
}

static int
deflate_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	vmclear(state->vm);
	state->tl = state->fixed_tl = 0;
	state->method = -1;
	memset((char*)state + offsetof(State_t, eof), 0, sizeof(*state) - offsetof(State_t, eof));
	return 0;
}

static ssize_t
deflate_read(Sfio_t* sp, void* buf, size_t size, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register uch*		p = (uch*)buf;
	register uch*		e = p + size;
	register uch*		q;
	ulg			l;
	ulg			w;
	ulg			d;


	if (state->copy_len > 0)
	{
		l = state->copy_len;
		w = state->wp;
		if (state->method != STORED_BLOCK)
		{
			d = state->copy_pos;
			while (l && p < e)
			{
				l--;
				d &= WINDOW - 1;
				w &= WINDOW - 1;
				*p++ = state->slide[w++] = state->slide[d++];
			}
			state->copy_pos = d;
		}
		else
		{
			BITS_LOCAL;
			while (l && p < e)
			{
				l--;
				w &= WINDOW - 1;
				NEEDBITS(state, 8);
				*p++ = state->slide[w++] = (uch)GETBITS(8);
				DUMPBITS(8);
			}
			BITS_GLOBAL;
			if (!l)
				state->method = -1;
		}
		state->copy_len = l;
		state->wp = w;
	}
	while (p < e)
	{
		if (state->method == -1)
		{
			BITS_LOCAL;
			if (state->eof)
			{
				BITS_GLOBAL;
				break;
			}
			if (state->last)
			{
				state->last = state->wp = 0;
				l = bit_len & 7;
				DUMPBITS(l);
			}

			/* read in last block bit */

			NEEDBITS(state, 1);
			if (GETBITS(1))
				state->last = 1;
			DUMPBITS(1);

			/* read in block type */

			NEEDBITS(state, 2);
			state->method = (int)GETBITS(2);
			DUMPBITS(2);
			state->tl = 0;
			state->copy_len = 0;
			BITS_GLOBAL;
		}
		switch(state->method)
		{
		case STORED_BLOCK:
			q = inflate_stored(state, p, e);
			break;
		case STATIC_TREES:
			q = (state->tl || !inflate_fixed(state)) ? inflate_codes(state, p, e) : (uch*)0;
			break;
		case DYNAMIC_TREES:
			q = (state->tl || !inflate_dynamic(state)) ? inflate_codes(state, p, e) : (uch*)0;
			break;
		default:
			q = 0;
			break;
		}
		if (!q)
		{
			if (p < e || state->eof)
				break;
			return -1;
		}
		p = q;
	}
	return p - (uch*)buf;
}

Codexmeth_t	codex_zip_deflate =
{
	"deflate",
	"zip deflate compression (PKZIP method 8). Option is level"
	" { 6:faster ... 9:better }.",
	0,
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	deflate_open,
	deflate_close,
	deflate_init,
	0,
	deflate_read,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};
