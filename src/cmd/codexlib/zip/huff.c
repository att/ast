#pragma prototyped

#include "huff.h"

/*
   Huffman code decoding is performed using a multi-level table lookup.
   The fastest way to decode is to simply build a lookup table whose
   size is determined by the longest code.  However, the time it takes
   to build this table can also be a factor if the data being decoded
   are not very long.  The most common codes are necessarily the
   shortest codes, so those codes dominate the decoding time, and hence
   the speed.  The idea is you can have a shorter table that decodes the
   shorter, more probable codes, and then point to subsidiary tables for
   the longer codes.  The time it costs to decode the longer codes is
   then traded against the time it takes to make longer tables.

   This results of this trade are in the variables lbits and dbits
   below.  lbits is the number of bits the first level table for literal/
   length codes can decode in one step, and dbits is the same thing for
   the distance codes.	Subsequent tables are also less than or equal to
   those sizes.	 These values may be adjusted either when all of the
   codes are shorter than that, in which case the longest code length in
   bits is used, or when the shortest code is *longer* than the requested
   table size, in which case the length of the shortest code in bits is
   used.

   There are two different values for the two tables, since they code a
   different number of possibilities each.  The literal/length table
   codes 286 possible values, or in a flat code, a little over eight
   bits.  The distance table codes 30 possible values, or a little less
   than five bits, flat.  The optimum values for speed end up being
   about one bit more than those, so lbits is 8+1 and dbits is 5+1.
   The optimum values may differ though from machine to machine, and
   possibly even between compilers.  Your mileage may vary.
 */

/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16		/* maximum bit length of any code (16 for explode) */
#define N_MAX 288	/* maximum number of codes in any set */

int huff(
    ulg *b,		/* code lengths in bits (all assumed <= BMAX) */
    ulg n,		/* number of codes (assumed <= N_MAX) */
    ulg s,		/* number of simple-valued codes (0..s-1) */
    ush *d,		/* list of base values for non-simple codes */
    ush *e,		/* list of extra bits for non-simple codes */
    Huff_t **t,	/* result: starting table */
    int *m,		/* maximum lookup bits, returns actual */
    Vmalloc_t *vm)	/* memory pool */
/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.	Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory.
   The code with value 256 is special, and the tables are constructed
   so that no bits beyond that code are fetched when that code is
   decoded. */
{
    ulg a;			/* counter for codes of length k */
    ulg c[BMAX+1];		/* bit length count table */
    ulg el;			/* length of EOB code (value 256) */
    ulg f;			/* i repeats in table every f entries */
    int g;			/* maximum code length */
    int h;			/* table level */
    register ulg i;		/* counter, current code */
    register ulg j;		/* counter */
    register int k;		/* number of bits in current code */
    int lx[BMAX+1];		/* memory for l[-1..BMAX-1] */
    int *l = lx+1;		/* stack of bits per table */
    register ulg *p;		/* pointer into c[], b[], or v[] */
    register Huff_t *q;		/* points to current table */
    Huff_t r;			/* table entry for structure assignment */
    Huff_t *u[BMAX];		/* table stack */
    ulg v[N_MAX];		/* values in order of bit length */
    register int w;		/* bits before this table == (l * h) */
    ulg x[BMAX+1];		/* bit offsets, then code stack */
    ulg *xp;			/* pointer into x */
    int y;			/* number of dummy codes added */
    ulg z;			/* number of entries in current table */

    /* Generate counts for each bit length */
    el = n > 256 ? b[256] : BMAX; /* set length of EOB code, if any */
    memset(c, 0, sizeof(c));
    p = b;
    i = n;
    do
    {
	c[*p]++;	/* assume all entries <= BMAX */
	p++;		/* Can't combine with above line (Solaris bug) */
    } while(--i);
    if(c[0] == n)	/* null input--all zero length codes */
    {
	*t = (Huff_t *)NULL;
	*m = 0;
	return 0;
    }

    /* Find minimum and maximum length, bound *m by those */
    for(j = 1; j <= BMAX; j++)
	if(c[j])
	    break;
    k = j;			/* minimum code length */
    if((ulg)*m < j)
	*m = j;
    for(i = BMAX; i; i--)
	if(c[i])
	    break;
    g = i;			/* maximum code length */
    if((ulg)*m > i)
	*m = i;

    /* Adjust last length count to fill out codes, if needed */
    for(y = 1 << j; j < i; j++, y <<= 1)
	if((y -= c[j]) < 0)
	    return 2;		/* bad input: more codes than bits */
    if((y -= c[i]) < 0)
	return 2;
    c[i] += y;

    /* Generate starting offsets into the value table for each length */
    x[1] = j = 0;
    p = c + 1;  xp = x + 2;
    while(--i)			/* note that i == g from above */
	*xp++ = (j += *p++);

    /* Make a table of values in order of bit lengths */
    memset(v, 0, sizeof(v));
    p = b;
    i = 0;
    do
    {
	if((j = *p++) != 0)
	    v[x[j]++] = i;
    } while(++i < n);
    n = x[g];			/* set n to length of v */

    /* Generate the Huffman codes and for each, make the table entries */
    x[0] = i = 0;		/* first Huffman code is zero */
    p = v;			/* grab values in bit order */
    h = -1;			/* no tables yet--level -1 */
    w = l[-1] = 0;		/* no bits decoded yet */
    u[0] = (Huff_t *)NULL;	/* just to keep compilers happy */
    q = (Huff_t *)NULL;	/* ditto */
    z = 0;			/* ditto */

    /* go through the bit lengths (k already is bits in shortest code) */
    for(; k <= g; k++)
    {
	a = c[k];
	while(a--)
	{
	    /* here i is the Huffman code of length k bits for value *p */
	    /* make tables up to required level */
	    while(k > w + l[h])
	    {
		w += l[h++];	/* add bits already decoded */

		/* compute minimum size table less than or equal to *m bits */
		z = (z = g - w) > (ulg)*m ? *m : z; /* upper limit */
		if((f = 1 << (j = k - w)) > a + 1) /* try a k-w bit table */
		{		/* too few codes for k-w bit table */
		    f -= a + 1;	/* deduct codes from patterns left */
		    xp = c + k;
		    while(++j < z)/* try smaller tables up to z bits */
		    {
			if((f <<= 1) <= *++xp)
			    break;	/* enough codes to use up j bits */
			f -= *xp;	/* else deduct codes from patterns */
		    }
		}
		if((ulg)w + j > el && (ulg)w < el)
		    j = el - w;	/* make EOB code end at table */
		z = 1 << j;	/* table entries for j-bit table */
		l[h] = j;	/* set table size in stack */

		/* allocate and link in new table */
		q = (Huff_t *)vmalloc(vm, (z + 1)*sizeof(Huff_t));
		if(q == NULL)
		{
		    return 3;	/* not enough memory */
		}

		*t = q + 1;	/* link to list for huft_free() */
		*(t = &(q->v.t)) = (Huff_t *)NULL;
		u[h] = ++q;	/* table starts after link */

		/* connect to last table, if there is one */
		if(h)
		{
		    x[h] = i;		/* save pattern for backing up */
		    r.b = (uch)l[h-1];	/* bits to dump before this table */
		    r.e = (uch)(16 + j);/* bits in this table */
		    r.v.t = q;		/* pointer to this table */
		    j = (i & ((1 << w) - 1)) >> (w - l[h-1]);
		    u[h-1][j] = r;	/* connect to last table */
		}
	    }

	    /* set up table entry in r */
	    r.b = (uch)(k - w);
	    if(p >= v + n)
		r.e = 99;		/* out of values--invalid code */
	    else if(*p < s)
	    {
		r.e = (uch)(*p < 256 ? 16 : 15); /* 256 is end-of-block code */
		r.v.n = (ush)*p++;	/* simple code is just the value */
	    }
	    else
	    {
		r.e = (uch)e[*p - s];	/* non-simple--look up in lists */
		r.v.n = d[*p++ - s];
	    }

	    /* fill code-like entries with r */
	    f = 1 << (k - w);
	    for(j = i >> w; j < z; j += f)
		q[j] = r;

	    /* backwards increment the k-bit code i */
	    for(j = 1 << (k - 1); i & j; j >>= 1)
		i ^= j;
	    i ^= j;

	    /* backup over finished tables */
	    while((i & ((1 << w) - 1)) != x[h])
		w -= l[--h];		/* don't need to update q */
	}
    }

    /* return actual size of base table */
    *m = l[0];

    /* Return true (1) if we were given an incomplete table */
    return y != 0 && g != 1;
}
