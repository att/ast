#pragma prototyped

/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 *
 * Test Vectors (from FIPS PUB 180-1)
 * "abc"
 *   A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 * "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *   84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 * A million repetitions of "a"
 *   34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 */

#define sha1_description "FIPS 180-1 SHA-1 secure hash algorithm 1."
#define sha1_options	"[+(version)?sha1 (FIPS 180-1) 1996-09-26]\
			 [+(author)?Steve Reid <steve@edmweb.com>]"
#define sha1_match	"sha1|SHA1|sha-1|SHA-1"
#define sha1_scale	0
#define sha1_flags	SUM_INDICATOR

#define sha1_padding	md5_pad

typedef struct Sha1_s
{
	_SUM_PUBLIC_
	_SUM_PRIVATE_
	uint32_t	count[2];
	uint32_t	state[5];
	uint8_t		buffer[64];
	uint8_t		digest[20];
	uint8_t		digest_sum[20];
} Sha1_t;

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/*
 * blk0() and blk() perform the initial expand.
 * I got the idea of expanding during the round function from SSLeay
 */
#if _ast_intswap
# define blk0(i) \
	(block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) \
	 | (rol(block->l[i], 8) & 0x00FF00FF))
#else
# define blk0(i) block->l[i]
#endif
#define blk(i) \
	(block->l[i & 15] = rol(block->l[(i + 13) & 15] \
				^ block->l[(i + 8) & 15] \
				^ block->l[(i + 2) & 15] \
				^ block->l[i & 15], 1))

/*
 * (R0+R1), R2, R3, R4 are the different operations (rounds) used in SHA1
 */
#define R0(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);
#define R1(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);
#define R2(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); \
	w = rol(w, 30);
#define R3(v,w,x,y,z,i) \
	z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
	w = rol(w, 30);
#define R4(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
	w = rol(w, 30);

typedef union {
	unsigned char c[64];
	unsigned int l[16];
} CHAR64LONG16;

#ifdef __sparc_v9__
static void do_R01(uint32_t *a, uint32_t *b, uint32_t *c,
		   uint32_t *d, uint32_t *e, CHAR64LONG16 *);
static void do_R2(uint32_t *a, uint32_t *b, uint32_t *c,
		  uint32_t *d, uint32_t *e, CHAR64LONG16 *);
static void do_R3(uint32_t *a, uint32_t *b, uint32_t *c,
		  uint32_t *d, uint32_t *e, CHAR64LONG16 *);
static void do_R4(uint32_t *a, uint32_t *b, uint32_t *c,
		  uint32_t *d, uint32_t *e, CHAR64LONG16 *);

#define nR0(v,w,x,y,z,i) R0(*v,*w,*x,*y,*z,i)
#define nR1(v,w,x,y,z,i) R1(*v,*w,*x,*y,*z,i)
#define nR2(v,w,x,y,z,i) R2(*v,*w,*x,*y,*z,i)
#define nR3(v,w,x,y,z,i) R3(*v,*w,*x,*y,*z,i)
#define nR4(v,w,x,y,z,i) R4(*v,*w,*x,*y,*z,i)

static void
do_R01(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
       uint32_t *e, CHAR64LONG16 *block)
{
	nR0(a,b,c,d,e, 0); nR0(e,a,b,c,d, 1); nR0(d,e,a,b,c, 2);
	nR0(c,d,e,a,b, 3); nR0(b,c,d,e,a, 4); nR0(a,b,c,d,e, 5);
	nR0(e,a,b,c,d, 6); nR0(d,e,a,b,c, 7); nR0(c,d,e,a,b, 8);
	nR0(b,c,d,e,a, 9); nR0(a,b,c,d,e,10); nR0(e,a,b,c,d,11);
	nR0(d,e,a,b,c,12); nR0(c,d,e,a,b,13); nR0(b,c,d,e,a,14);
	nR0(a,b,c,d,e,15); nR1(e,a,b,c,d,16); nR1(d,e,a,b,c,17);
	nR1(c,d,e,a,b,18); nR1(b,c,d,e,a,19);
}

static void
do_R2(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
      uint32_t *e, CHAR64LONG16 *block)
{
	nR2(a,b,c,d,e,20); nR2(e,a,b,c,d,21); nR2(d,e,a,b,c,22);
	nR2(c,d,e,a,b,23); nR2(b,c,d,e,a,24); nR2(a,b,c,d,e,25);
	nR2(e,a,b,c,d,26); nR2(d,e,a,b,c,27); nR2(c,d,e,a,b,28);
	nR2(b,c,d,e,a,29); nR2(a,b,c,d,e,30); nR2(e,a,b,c,d,31);
	nR2(d,e,a,b,c,32); nR2(c,d,e,a,b,33); nR2(b,c,d,e,a,34);
	nR2(a,b,c,d,e,35); nR2(e,a,b,c,d,36); nR2(d,e,a,b,c,37);
	nR2(c,d,e,a,b,38); nR2(b,c,d,e,a,39);
}

static void
do_R3(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
      uint32_t *e, CHAR64LONG16 *block)
{
	nR3(a,b,c,d,e,40); nR3(e,a,b,c,d,41); nR3(d,e,a,b,c,42);
	nR3(c,d,e,a,b,43); nR3(b,c,d,e,a,44); nR3(a,b,c,d,e,45);
	nR3(e,a,b,c,d,46); nR3(d,e,a,b,c,47); nR3(c,d,e,a,b,48);
	nR3(b,c,d,e,a,49); nR3(a,b,c,d,e,50); nR3(e,a,b,c,d,51);
	nR3(d,e,a,b,c,52); nR3(c,d,e,a,b,53); nR3(b,c,d,e,a,54);
	nR3(a,b,c,d,e,55); nR3(e,a,b,c,d,56); nR3(d,e,a,b,c,57);
	nR3(c,d,e,a,b,58); nR3(b,c,d,e,a,59);
}

static void
do_R4(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
      uint32_t *e, CHAR64LONG16 *block)
{
	nR4(a,b,c,d,e,60); nR4(e,a,b,c,d,61); nR4(d,e,a,b,c,62);
	nR4(c,d,e,a,b,63); nR4(b,c,d,e,a,64); nR4(a,b,c,d,e,65);
	nR4(e,a,b,c,d,66); nR4(d,e,a,b,c,67); nR4(c,d,e,a,b,68);
	nR4(b,c,d,e,a,69); nR4(a,b,c,d,e,70); nR4(e,a,b,c,d,71);
	nR4(d,e,a,b,c,72); nR4(c,d,e,a,b,73); nR4(b,c,d,e,a,74);
	nR4(a,b,c,d,e,75); nR4(e,a,b,c,d,76); nR4(d,e,a,b,c,77);
	nR4(c,d,e,a,b,78); nR4(b,c,d,e,a,79);
}
#endif

/*
 * Hash a single 512-bit block. This is the core of the algorithm.
 */
static void
sha1_transform(uint32_t state[5], const unsigned char buffer[64]) {
	uint32_t a, b, c, d, e;
	CHAR64LONG16 *block;
	CHAR64LONG16 workspace;

	block = &workspace;
	(void)memcpy(block, buffer, 64);

	/* Copy sha->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

#ifdef __sparc_v9__
	do_R01(&a, &b, &c, &d, &e, block);
	do_R2(&a, &b, &c, &d, &e, block);
	do_R3(&a, &b, &c, &d, &e, block);
	do_R4(&a, &b, &c, &d, &e, block);
#else
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
#endif

	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	/* Wipe variables */
	a = b = c = d = e = 0;
}

static int
sha1_block(register Sum_t* p, const void* s, size_t len)
{
	Sha1_t*		sha = (Sha1_t*)p;
	uint8_t*	data = (uint8_t*)s;
	unsigned int	i, j;

	if (len) {
		j = sha->count[0];
		if ((sha->count[0] += len << 3) < j)
			sha->count[1] += (len >> 29) + 1;
		j = (j >> 3) & 63;
		if ((j + len) > 63) {
			(void)memcpy(&sha->buffer[j], data, (i = 64 - j));
			sha1_transform(sha->state, sha->buffer);
			for ( ; i + 63 < len; i += 64)
				sha1_transform(sha->state, &data[i]);
			j = 0;
		} else {
			i = 0;
		}
	
		(void)memcpy(&sha->buffer[j], &data[i], len - i);
	}
	return 0;
}

static int
sha1_init(Sum_t* p)
{
	register Sha1_t*	sha = (Sha1_t*)p;

	sha->count[0] = sha->count[1] = 0;
	sha->state[0] = 0x67452301;
	sha->state[1] = 0xEFCDAB89;
	sha->state[2] = 0x98BADCFE;
	sha->state[3] = 0x10325476;
	sha->state[4] = 0xC3D2E1F0;

	return 0;
}

static Sum_t*
sha1_open(const Method_t* method, const char* name)
{
	Sha1_t*	sha;

	if (sha = newof(0, Sha1_t, 1, 0))
	{
		sha->method = (Method_t*)method;
		sha->name = name;
		sha1_init((Sum_t*)sha);
	}
	return (Sum_t*)sha;
}

/*
 * Add padding and return the message digest.
 */

static const unsigned char final_200 = 128;
static const unsigned char final_0 = 0;

static int
sha1_done(Sum_t* p)
{
	Sha1_t*	sha = (Sha1_t*)p;
	unsigned int i;
	unsigned char finalcount[8];

	for (i = 0; i < 8; i++) {
		/* Endian independent */
		finalcount[i] = (unsigned char)
			((sha->count[(i >= 4 ? 0 : 1)]
			  >> ((3 - (i & 3)) * 8)) & 255);
	}

	sha1_block(p, &final_200, 1);
	while ((sha->count[0] & 504) != 448)
		sha1_block(p, &final_0, 1);
	/* The next Update should cause a sha1_transform() */
	sha1_block(p, finalcount, 8);

	for (i = 0; i < elementsof(sha->digest); i++)
	{
		sha->digest[i] = (unsigned char)((sha->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
		sha->digest_sum[i] ^= sha->digest[i];
	}
	memset(sha->count, 0, sizeof(sha->count));
	memset(sha->state, 0, sizeof(sha->state));
	memset(sha->buffer, 0, sizeof(sha->buffer));
	return 0;
}

static int
sha1_print(Sum_t* p, Sfio_t* sp, register int flags, size_t scale)
{
	register Sha1_t*	sha = (Sha1_t*)p;
	register unsigned char*	d;
	register int		n;

	d = (flags & SUM_TOTAL) ? sha->digest_sum : sha->digest;
	for (n = 0; n < elementsof(sha->digest); n++)
		sfprintf(sp, "%02x", d[n]);
	return 0;
}

static int
sha1_data(Sum_t* p, Sumdata_t* data)
{
	register Sha1_t*	sha = (Sha1_t*)p;

	data->size = elementsof(sha->digest);
	data->num = 0;
	data->buf = sha->digest;
	return 0;
}
