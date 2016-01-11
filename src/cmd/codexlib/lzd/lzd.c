#pragma prototyped

/*
 * lzd decoder snarfed from the public domain booz -- Rahul Dhesi 1991/07/07
 */

#include <codex.h>

#define MEM_BLOCK_SIZE	8192
#define OUT_BUF_SIZE	(MEM_BLOCK_SIZE/2)
#define IN_BUF_SIZE	(MEM_BLOCK_SIZE/2)

#define STACKSIZE	2000
#define INBUFSIZ	(IN_BUF_SIZE - SPARE)
#define OUTBUFSIZ	(OUT_BUF_SIZE - SPARE)
#define MEMERR		2
#define IOERR		1
#define MAXBITS		13
#define CLEAR		256         /* clear code */
#define Z_EOF		257         /* end of file marker */
#define FIRST_FREE	258         /* first free code */
#define MAXMAX		8192        /* max code + 1 */
#define SPARE		4

typedef struct Table_s
{
	unsigned int	next;
	char		z_ch;
} Table_t;

typedef struct State_s
{
	Codex_t*	codex;

	unsigned char	buf[SF_BUFSIZE];
	unsigned char*	ip;
	unsigned char*	ie;

	unsigned long	bitbuf;
	unsigned short	bitcount;
	unsigned short	bitmask;

	unsigned int	old_code;
	unsigned int	suf_code;
	unsigned int	max_code;
	unsigned int	free_code;
	unsigned int	in_code;
	unsigned int	fin_char;
	unsigned int	nbits;
	unsigned int	cpy;
	unsigned int	bad;
	unsigned int	eof;
	unsigned int	sp;
	unsigned int	stack[STACKSIZE + SPARE];

	Table_t		table[MAXMAX];

	char		spare[SPARE];
} State_t;

#define GETCHAR(p)	((p)->ip < (p)->ie ? (int)*(p)->ip++ : fill(p))

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

static int
getcode(State_t* state)
{
	unsigned short	x;

	while (state->nbits > state->bitcount)
	{
		state->bitbuf |= GETCHAR(state) << state->bitcount;
		state->bitcount += 8;
	}
	x = state->bitbuf & state->bitmask;
	state->bitbuf >>= state->nbits;
	state->bitcount -= state->nbits;
	return state->eof ? Z_EOF : x;
}

static void
push(State_t* state, int c)
{
	state->stack[state->sp++] = c;
	if (state->sp >= STACKSIZE)
		state->sp--;
}
   
#define pop(p)    ((p)->stack[--(p)->sp])

static void
clrcode(State_t* state)
{
	state->bitmask = (1 << (state->nbits = 9)) - 1;
	state->max_code = 512;
	state->free_code = FIRST_FREE;
}

static void
addcode(register State_t* state, int suf_code, int old_code)
{
	state->table[state->free_code].z_ch = suf_code;
	state->table[state->free_code].next = old_code;
	if (++state->free_code >= state->max_code && state->nbits < MAXBITS)
	{
		state->nbits++;
		state->bitmask <<= 1;
		state->bitmask |= 1;
		state->max_code = state->max_code << 1;
	}
}

static int
lzd_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*		state;

	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->codex = p;
	p->data = state;
	return 0;
}

static int
lzd_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	state->bitcount = 0;
	state->bad = state->cpy = state->eof = 0;
	state->ip = state->ie = 0;
	state->sp = 0;
	clrcode(state);
	return 0;
}

static ssize_t
lzd_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register char*		s = (char*)buf;
	register char*		e = s + n;
	register int		c;

	if (state->eof)
		return state->bad;
	if (state->cpy)
	{
		state->cpy = 0;
		while (state->sp != 0)
		{
			*s++ = pop(state);
			if (s >= e)
			{
				state->cpy = 1;
				return s - (char*)buf;
			}
		}
	}
	while ((c = getcode(state)) != Z_EOF)
	{
		if (c == CLEAR)
		{
			clrcode(state);
			*s++ = state->fin_char = state->suf_code = state->old_code = c = getcode(state);
			if (s >= e)
				return s - (char*)buf;
		}
		else
		{
			state->in_code = c;
			if (c >= state->free_code)
			{
				c = state->old_code;
				push(state, state->fin_char);
			}
			while (c > 255)
			{
				push(state, state->table[c].z_ch);
				c = state->table[c].next;
			}
			state->suf_code = state->fin_char = c;
			addcode(state, state->suf_code, state->old_code);
			state->old_code = state->in_code;
			push(state, state->suf_code);
			while (state->sp != 0)
			{
				*s++ = pop(state);
				if (s >= e)
				{
					state->cpy = 1;
					return s - (char*)buf;
				}
			}
		}
	}
	state->eof = 1;
	return s - (char*)buf;
}

Codexmeth_t	codex_lzd =
{
	"lzd",
	"\bzoo\b(1) archive lzd compression.",
	"[+(version)?codex-lzd 1991-07-07]"
	"[+(author)?Rahul Dhesi]",
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	lzd_open,
	0,
	lzd_init,
	0,
	lzd_read,
	0,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(lzd)
};

CODEXLIB(lzd)
