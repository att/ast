#pragma prototyped

/*
 * lzh decoder
 */

#include <codex.h>

#define WINBIT_MIN	10
#define WINBIT_MAX	16

#define WINDOW_MIN	(1<<WINBIT_MIN)
#define WINDOW_MAX	(1<<WINBIT_MAX)

#define MATCH_MAX	256	/* formerly F (not more than UCHAR_MAX + 1) */
#define THRESHOLD	3	/* choose optimal value */

#define NC		(UCHAR_MAX+MATCH_MAX-THRESHOLD+2) /* alphbet 0..NC-1 */
#define CBIT		9	/* $\lfloor \log_2 NC \rfloor + 1$ */
#define USHRT_BIT	16	/* (CHAR_BIT * sizeof(ui2)) */
#define NP		(WINBIT_MAX + 1)
#define NT		(USHRT_BIT + 3)
#define PBIT		5 /* smallest int s.t. (1<<PBIT))>(WINBIT_MAX+1) */
#define TBIT		5 /* smallest int s.t. (1<<TBIT)>NT */
#define NPT		0x80
#define N1		286	/* alphabet size */
#define N2		(2*N1-1) /* # of nodes in Huffman tree */
#define EXTRABITS	8	/* >= log2(F-THRESHOLD+258-N1) */
#define BUFBITS		16	/* >= log2(MAXBUF) */
#define LENFIELD	4	/* bit size of length field for tree output */
#define SNP		(8*1024/64)
#define SNP2		(SNP*2-1)
#define N_CHAR		(256+60-THRESHOLD+1)
#define TREESIZE_C	(N_CHAR*2)
#define TREESIZE_P	(128*2)
#define TREESIZE	(TREESIZE_C+TREESIZE_P)
#define ROOT_C		0
#define ROOT_P		TREESIZE_C
#define MAGIC0		18
#define MAGIC5		19

struct State_s; typedef struct State_s State_t;

typedef int (*Decode_f)(State_t*);

typedef uint8_t ui1;
typedef uint16_t ui2;
typedef uint32_t ui4;

struct State_s
{
	Codex_t*	codex;

	Decode_f	init_c;
	Decode_f	init_p;
	Decode_f	decode_c;
	Decode_f	decode_p;

	int		method;

	ui1		buf[SF_BUFSIZE];
	ui1*		ip;
	ui1*		ie;

	ui4		window;
	ui4		count;
	ui4		nxt;
	ui4		loc;
	ui4		cpylen;
	ui4		cpy;
	ui4		bitbuf;

	ui2		maxmatch;
	ui2		bitcount;
	ui2		blocksize;

	ui1		c_len[NC];
	ui1		p_len[NPT];
	ui2		left[2 * NC - 1];
	ui2		right[2 * NC - 1];
	ui2		c_table[4096];
	ui2		p_table[256];

	ui4		n_max;
	short		child[TREESIZE];
	short		parent[TREESIZE];
	short		block[TREESIZE];
	short		edge[TREESIZE];
	short		stock[TREESIZE];
	short		node[TREESIZE / 2];
	ui2		freq[TREESIZE];
	ui2		total_p;
	int		avail;
	int		eof;
	int		n1;
	int		most_p;
	ui4		nextcount;
	ui4		snp;
	int		flag;
	int		flagcnt;
	int		matchpos;
	int		offset;
	ui4		pbit;

	ui1		text[1L<<WINBIT_MAX];
};

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

#define GETBITS(p,n)	((n)<=(p)->bitcount ? ((((p)->bitbuf)>>((p)->bitcount-=(n)))&((1L<<(n))-1)) : getbits(p,n,0))
#define PEEKBITS(p,n)	((n)<=(p)->bitcount ? ((((p)->bitbuf)>>((p)->bitcount-(n)))&((1L<<(n))-1)) : getbits(p,n,1))
#define SKIPBITS(p,n)	((p)->bitcount-=(n))

static int
getbits(register State_t* state, int n, int peek)
{
	while (state->bitcount < n)
	{
		state->bitbuf <<= 8;
		state->bitbuf |= GETCHAR(state);
		state->bitcount += 8;
	}
	if (peek)
	{
		state->eof = 0;
		return PEEKBITS(state, n);
	}
	return GETBITS(state, n);
}

static int
make_table(State_t* state, int nchar, ui1* bitlen, int tablebits, ui2* table, ui4 tablesize)
{
#if 1
	ui2	count[17];	/* count of bitlen */
	ui2	weight[17];	/* 0x10000ul >> bitlen */
	ui2	start[17];	/* first code of bitlen */
	ui2	total;
	int	i;
	int	j;
	int	k;
	int	l;
	int	m;
	int	n;
	int	available;
	ui2*	p;

	if (state->eof)
		return -1;
	available = nchar;

	/* initialize */

	for (i = 1; i <= 16; i++)
	{
		count[i] = 0;
		weight[i] = 1 << (16 - i);
	}

	/* count */

	for (i = 0; i < nchar; i++)
		count[bitlen[i]]++;

	/* calculate first code */

	total = 0;
	for (i = 1; i <= 16; i++)
	{
		start[i] = total;
		total += weight[i] * count[i];
	}
	if (total)
	{
		if (state->codex->disc->errorf)
			(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: invalid compression table", state->codex->meth->name);
		return -1;
	}

	/* shift data for make table. */

	m = 16 - tablebits;
	for (i = 1; i <= tablebits; i++)
	{
		start[i] >>= m;
		weight[i] >>= m;
	}

	/* initialize */

	j = start[tablebits + 1] >> m;
	k = 1 << tablebits;
	if (j != 0)
		for (i = j; i < k; i++)
			table[i] = 0;

	/* create table and tree */

	for (j = 0; j < nchar; j++)
	{
		k = bitlen[j];
		if (k == 0)
			continue;
		i = start[k];
		l = i + weight[k];
		if (k <= tablebits)
		{
			if (l > tablesize)
			{
				if (state->codex->disc->errorf)
					(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: compression table overflow", state->codex->meth->name);
				return -1;
			}

			/* code in table */

			while (i < l)
				table[i++] = j;
		}
		else
		{
			/* code not in table */

			p = &table[i >> m];
			i <<= tablebits;
			n = k - tablebits;

			/* make tree (n length) */

			while (--n >= 0)
			{
				if (*p == 0)
				{
					state->right[available] = state->left[available] = 0;
					*p = available++;
				}
				if (i & 0x8000)
					p = &state->right[*p];
				else
					p = &state->left[*p];
				i <<= 1;
			}
			*p = j;
		}
		start[k] = l;
	}
#else
	ui2 count[17], weight[17], start[18], *p;
	ui4 i, k, len, ch, jutbits, avail, nextcode, mask;

	for (i = 1; i <= 16; i++) count[i] = 0;
	for (i = 0; i < nchar; i++) count[bitlen[i]]++;

	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = start[i] + (count[i] << (16 - i));
	if (start[17] != (ushort)((unsigned) 1 << 16))
	{
		if (state->codex->disc->errorf)
			(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: invalid compression table", state->codex->meth->name);
		return -1;
	}

	jutbits = 16 - tablebits;
	for (i = 1; i <= tablebits; i++) {
		start[i] >>= jutbits;
		weight[i] = (unsigned) 1 << (tablebits - i);
	}
	while (i <= 16) {
	   weight[i] = (unsigned) 1 << (16 - i);
	   i++;
        }

	i = start[tablebits + 1] >> jutbits;
	if (i != (ushort)((unsigned) 1 << 16)) {
		k = 1 << tablebits;
		while (i != k) table[i++] = 0;
	}

	avail = nchar;
	mask = (unsigned) 1 << (15 - tablebits);
	for (ch = 0; ch < nchar; ch++) {
		if ((len = bitlen[ch]) == 0) continue;
		nextcode = start[len] + weight[len];
		if (len <= tablebits) {
			if (nextcode > tablesize)
			{
				if (state->codex->disc->errorf)
					(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: compression table overflow", state->codex->meth->name);
				return -1;
			}
			for (i = start[len]; i < nextcode; i++) table[i] = ch;
		} else {
			k = start[len];
			p = &table[k >> jutbits];
			i = len - tablebits;
			while (i != 0) {
				if (*p == 0) {
					state->right[avail] = state->left[avail] = 0;
					*p = avail++;
				}
				if (k & mask) p = &state->right[*p];
				else          p = &state->left[*p];
				k <<= 1;  i--;
			}
			*p = ch;
		}
		start[len] = nextcode;
	}
#endif
	return 0;
}

static int
get_p_len(State_t* state, int k, int nbit, int i_special)
{
	int	i;
	int	c;
	int	n;

	if (!(n = GETBITS(state, nbit)))
	{
		c = GETBITS(state, nbit);
		for (i = 0; i < elementsof(state->p_table); i++)
			state->p_table[i] = c;
		for (i = 0; i < k; i++)
			state->p_len[i] = 0;
	}
	else
	{
		i = 0;
		while (i < n)
		{
			if ((c = GETBITS(state, 3)) == 7)
				while (GETBITS(state, 1))
					c++;
			state->p_len[i++] = c;
			if (i == i_special)
			{
				c = GETBITS(state, 2);
				while (--c >= 0)
					state->p_len[i++] = 0;
			}
		}
		while (i < k)
			state->p_len[i++] = 0;
		if (make_table(state, k, state->p_len, 8, state->p_table, elementsof(state->p_table)))
			return -1;
	}
	return 0;
}

static int
get_c_len(State_t* state)
{
	int	i;
	int	c;
	int	n;
	ui2	b;

	n = GETBITS(state, CBIT);
	if (state->eof)
		return -1;
	if (n == 0)
	{
		c = GETBITS(state, CBIT);
		for (i = 0; i < elementsof(state->c_table); i++)
			state->c_table[i] = c;
		for (i = 0; i < elementsof(state->c_len); i++)
			state->c_len[i] = 0;
	}
	else
	{
		i = 0;
		while (i < n)
		{
			b = PEEKBITS(state, 16);
			c = state->p_table[b >> 8];
			while (c >= NT)
				c = ((b <<= 1) & 0x100) ? state->right[c] : state->left[c];
			SKIPBITS(state, state->p_len[c]);
			if (c <= 2)
			{
				if (c == 0)
					c = 1;
				else if (c == 1)
					c = GETBITS(state, 4) + 3;
				else
					c = GETBITS(state, CBIT) + 20;
				while (--c >= 0)
					state->c_len[i++] = 0;
			}
			else
				state->c_len[i++] = c - 2;
		}
		while (i < NC)
			state->c_len[i++] = 0;
		if (make_table(state, NC, state->c_len, 12, state->c_table, elementsof(state->c_table)))
			return -1;
	}
	return 0;
}

static void
reconst(State_t* state, int start, int end)
{
	int	i;
	int	j;
	int	k;
	int	l;
	int	b;
	ui4	f;
	ui4	g;

	b = 0;
	for (i = j = start; i < end; i++)
	{
		if ((k = state->child[i]) < 0)
		{
			state->freq[j] = (state->freq[i] + 1) / 2;
			state->child[j] = k;
			j++;
		}
		if (state->edge[b = state->block[i]] == i)
			state->stock[--state->avail] = b;
	}
	j--;
	i = end - 1;
	l = end - 2;
	while (i >= start)
	{
		while (i >= l)
		{
			state->freq[i] = state->freq[j];
			state->child[i] = state->child[j];
			i--;
			j--;
		}
		f = state->freq[l] + state->freq[l + 1];
		for (k = start; f < state->freq[k]; k++)
			;
		while (j >= k)
		{
			state->freq[i] = state->freq[j];
			state->child[i] = state->child[j];
			i--;
			j--;
		}
		state->freq[i] = f;
		state->child[i] = l + 1;
		i--;
		l -= 2;
	}
	f = 0;
	for (i = start; i < end; i++)
	{
		if ((j = state->child[i]) < 0)
			state->node[~j] = i;
		else
			state->parent[j] = state->parent[j - 1] = i;
		if ((g = state->freq[i]) == f)
			state->block[i] = b;
		else
		{
			state->edge[b = state->block[i] = state->stock[state->avail++]] = i;
			f = g;
		}
	}
}

static int
swap_inc(State_t* state, int p)
{
	int	b;
	int	q;
	int	r;
	int	s;

	b = state->block[p];
	if ((q = state->edge[b]) != p)
	{
		r = state->child[p];
		s = state->child[q];
		state->child[p] = s;
		state->child[q] = r;
		if (r >= 0)
			state->parent[r] = state->parent[r - 1] = q;
		else
			state->node[~r] = q;
		if (s >= 0)
			state->parent[s] = state->parent[s - 1] = p;
		else
			state->node[~s] = p;
		p = q;
	}
	else if (b != state->block[p + 1])
	{
		if (++state->freq[p] == state->freq[p - 1])
		{
			state->stock[--state->avail] = b;
			state->block[p] = state->block[p - 1];
		}
		return state->parent[p];
	}
	state->edge[b]++;
	if (++state->freq[p] == state->freq[p - 1])
		state->block[p] = state->block[p - 1];
	else
		state->edge[state->block[p] = state->stock[state->avail++]] = p;
	return state->parent[p];
}

static void
update_c(State_t* state, int p)
{
	int	q;

	if (state->freq[ROOT_C] == 0x8000)
		reconst(state, 0, state->n_max * 2 - 1);
	state->freq[ROOT_C]++;
	q = state->node[p];
	do
	{
		q = swap_inc(state, q);
	} while (q != ROOT_C);
}

static void
update_p(State_t* state, int p)
{
	int	q;

	if (state->total_p == 0x8000)
	{
		reconst(state, ROOT_P, state->most_p + 1);
		state->total_p = state->freq[ROOT_P];
		state->freq[ROOT_P] = 0xffff;
	}
	q = state->node[p + N_CHAR];
	while (q != ROOT_P)
	{
		q = swap_inc(state, q);
	}
	state->total_p++;
}

static void
make_new_node(State_t* state, int p)
{
	int	q;
	int	r;

	r = state->most_p + 1;
	q = r + 1;
	state->node[~(state->child[r] = state->child[state->most_p])] = r;
	state->child[q] = ~(p + N_CHAR);
	state->child[state->most_p] = q;
	state->freq[r] = state->freq[state->most_p];
	state->freq[q] = 0;
	state->block[r] = state->block[state->most_p];
	if (state->most_p == ROOT_P)
	{
		state->freq[ROOT_P] = 0xffff;
		state->edge[state->block[ROOT_P]]++;
	}
	state->parent[r] = state->parent[q] = state->most_p;
	state->edge[state->block[q] = state->stock[state->avail++]]
		= state->node[p + N_CHAR] = state->most_p = q;
	update_p(state, p);
}

static void
ready_made(State_t* state, ui1* tbl)
{
	int	i;
	int	j;
	ui4	code;
	ui4	weight;

	j = *tbl++;
	weight = 1 << (16 - j);
	code = 0;
	for (i = 0; i < state->snp; i++)
	{
		while (*tbl == i)
		{
			j++;
			tbl++;
			weight >>= 1;
		}
		state->p_len[i] = j;
		code += weight;
	}
}

static int
read_tree_c(State_t* state)
{
	int	i;
	int	c;

	i = 0;
	while (i < N1)
	{
		if (GETBITS(state, 1))
			state->c_len[i] = GETBITS(state, LENFIELD) + 1;
		else
			state->c_len[i] = 0;
		if (++i == 3 && state->c_len[0] == 1 && state->c_len[1] == 1 && state->c_len[2] == 1)
		{
			c = GETBITS(state, CBIT);
			for (i = 0; i < N1; i++)
				state->c_len[i] = 0;
			for (i = 0; i < 4096; i++)
				state->c_table[i] = c;
			return 0;
		}
	}
	return make_table(state, N1, state->c_len, 12, state->c_table, elementsof(state->c_table));
}

static void
read_tree_p(State_t* state)
{
	int	i;
	int	c;

	i = 0;
	while (i < SNP)
	{
		state->p_len[i] = GETBITS(state, LENFIELD);
		if (++i == 3 && state->p_len[0] == 1 && state->p_len[1] == 1 && state->p_len[2] == 1)
		{
			c = GETBITS(state, WINBIT_MAX - 6);
			for (i = 0; i < SNP; i++)
				state->c_len[i] = 0;
			for (i = 0; i < 256; i++)
				state->c_table[i] = c;
			return;
		}
	}
}

static int
set_c_d0(State_t* state, ui4 n_max, ui4 maxmatch, ui4 snp)
{
	int	i;
	int	j;
	int	f;

	state->n_max = n_max;
	state->maxmatch = maxmatch;
	state->snp = snp;
	state->n1 = (state->n_max >= 256 + state->maxmatch - THRESHOLD + 1)
		? 512 : state->n_max - 1;
	for (i = 0; i < TREESIZE_C; i++)
	{
		state->stock[i] = i;
		state->block[i] = 0;
	}
	for (i = 0, j = state->n_max * 2 - 2; i < state->n_max; i++, j--)
	{
		state->freq[j] = 1;
		state->child[j] = ~i;
		state->node[i] = j;
		state->block[j] = 1;
	}
	state->avail = 2;
	state->edge[1] = state->n_max - 1;
	i = state->n_max * 2 - 2;
	while (j >= 0)
	{
		f = state->freq[j] = state->freq[i] + state->freq[i - 1];
		state->child[j] = i;
		state->parent[i] = state->parent[i - 1] = j;
		if (f == state->freq[j + 1])
			state->edge[state->block[j] = state->block[j + 1]] = j;
		else
			state->edge[state->block[j] = state->stock[state->avail++]] = j;
		i -= 2;
		j--;
	}
	return 0;
}

static ui1	table_fix[16] =
{
	3, 0x01, 0x04, 0x0c, 0x18, 0x30
};

static int
init_c_d0(State_t* state)
{
	set_c_d0(state, 314, 60, 1 << (12 - 6));
	ready_made(state, table_fix);
	return make_table(state, state->snp, state->p_len, 8, state->p_table, elementsof(state->p_table));
}

static int
decode_c_d0(State_t* state)
{
	int	c;

	c = state->child[ROOT_C];
	while (c)
		c = state->child[c - GETBITS(state, 1)];
	c = ~c;
	update_c(state, c);
	if (c == state->n1)
		c += GETBITS(state, 8);
	return c;
}

static int
init_p_d0(State_t* state)
{
	state->freq[ROOT_P] = 1;
	state->child[ROOT_P] = ~(N_CHAR);
	state->node[N_CHAR] = ROOT_P;
	state->edge[state->block[ROOT_P] = state->stock[state->avail++]] = ROOT_P;
	state->most_p = ROOT_P;
	state->total_p = 0;
	state->nextcount = 64;
	return 0;
}

static int
decode_p_d0(State_t* state)
{
	int	c;

	while (state->count > state->nextcount)
	{
		make_new_node(state, (int)(state->nextcount / 64));
		if ((state->nextcount += 64) >= state->window)
			state->nextcount = 0xffffffff;
	}
	c = state->child[ROOT_P];
	while (c)
		c = state->child[c - GETBITS(state, 1)];
	c = (~c) - N_CHAR;
	update_p(state, c);
	return (c << 6) + GETBITS(state, 6);
}

static int
init_c_d1(State_t* state)
{
	return set_c_d0(state, 286, MATCH_MAX, 1 << (12 - 6));
}

static int
decode_start_fix(State_t* state)
{
	state->n_max = 314;
	state->maxmatch = 60;
	state->snp = 1 << (12 - 6);
#if 0
	start_c_dyn(state);
#endif
	ready_made(state, table_fix);
	return make_table(state, state->snp, state->p_len, 8, state->p_table, elementsof(state->p_table));
}

static int
init_c_s0(State_t* state)
{
	state->n_max = 286;
	state->maxmatch = MATCH_MAX;
	state->snp = 1 << (WINBIT_MAX - 6);
	state->blocksize = 0;
	return 0;
}

static ui1	table_s0[16] =
{
	2, 0x01, 0x01, 0x03, 0x06, 0x0D, 0x1F, 0x4E
};

static int
decode_c_s0(State_t* state)
{
	int	j;

	if (state->blocksize == 0) /* read block head */
	{
		/* read block blocksize */
		state->blocksize = GETBITS(state, BUFBITS);
		if (read_tree_c(state))
			return -1;
		if (GETBITS(state, 1))
			read_tree_p(state);
		else
			ready_made(state, table_s0);
		if (make_table(state, SNP, state->p_len, 8, state->p_table, elementsof(state->p_table)))
			return 0;
	}
	state->blocksize--;
	j = state->c_table[GETBITS(state, 4)];
	while (j >= N1)
	{
		if (GETBITS(state, 1))
			j = state->right[j];
		else
			j = state->left [j];
	}
	if (j == N1 - 1)
		j += GETBITS(state, EXTRABITS);
	return j;
}

static int
init_p_s0(State_t* state)
{
	return 0;
}

static int
decode_p_s0(State_t* state)
{
	int	j;

	j = state->p_table[GETBITS(state, 8)];
	if (j >= state->snp)
	{
		do
		{
			if (GETBITS(state, 1))
				j = state->right[j];
			else
				j = state->left[j];
		} while (j >= state->snp);
	}
	return (j << 6) + GETBITS(state, 6);
}

static int
init_c_s1(State_t* state)
{
	if (state->window <= (14 * 1024))
	{
		state->snp = 14;
		state->pbit = 4;
	}
	else
	{
		state->snp = 16;
		state->pbit = 5;
	}
	state->blocksize = 0;
	return 0;
}

static int
decode_c_s1(State_t* state)
{
	ui4	j;
	ui2	b;

	if (!state->blocksize && (!(state->blocksize = GETBITS(state, 16)) || get_p_len(state, NT, TBIT, 3) || get_c_len(state) || get_p_len(state, state->snp, state->pbit, -1)))
		return -1;
	state->blocksize--;
	b = PEEKBITS(state, 16);
	j = state->c_table[b >> 4];
	while (j >= NC)
		j = ((b <<= 1) & 0x10) ? state->right[j] : state->left[j];
	SKIPBITS(state, state->c_len[j]);
	return state->eof ? -1 : j;
}

static int
init_p_s1(State_t* state)
{
	return 0;
}

static int
decode_p_s1(State_t* state)
{
	ui4	j;
	ui2	b;

	b = PEEKBITS(state, 16);
	j = state->p_table[b >> 8];
	while (j >= state->snp)
		j = ((b <<= 1) & 0x100) ? state->right[j] : state->left[j];
	SKIPBITS(state, state->p_len[j]);
	if (j)
		j = (1 << (j - 1)) + GETBITS(state, j - 1);
	return j;
}

static int
init_c_s2(State_t* state)
{
	state->offset = 0x100 - 2;
	return 0;
}

static int
decode_c_s2(State_t* state)
{
	if (GETBITS(state, 1))
		return GETBITS(state, 8);
	state->matchpos = GETBITS(state, 11);
	return GETBITS(state, 4) + 0x100;
}

static int
init_p_s2(State_t* state)
{
	return 0;
}

static int
decode_p_s2(State_t* state)
{
	return (state->loc - state->matchpos - MAGIC0) & 0x7ff;
}

static int
init_c_s3(State_t* state)
{
	int	i;

	state->flagcnt = 0;
	for (i = 0; i < 256; i++)
		memset(&state->text[i * 13 + 18], i, 13);
	for (i = 0; i < 256; i++)
		state->text[256 * 13 + 18 + i] = i;
	for (i = 0; i < 256; i++)
		state->text[256 * 13 + 256 + 18 + i] = 255 - i;
	memset(&state->text[256 * 13 + 512 + 18], 0, 128);
	memset(&state->text[256 * 13 + 512 + 128 + 18], ' ', 128 - 18);
	return 0;
}

static int
decode_c_s3(State_t* state)
{
	int	c;

	if (state->flagcnt == 0)
	{
		state->flagcnt = 8;
		state->flag = GETCHAR(state);
	}
	state->flagcnt--;
	c = GETCHAR(state);
	if ((state->flag & 1) == 0)
	{
		state->matchpos = c;
		c = GETCHAR(state);
		state->matchpos += (c & 0xf0) << 4;
		c &= 0x0f;
		c += 0x100;
	}
	state->flag >>= 1;
	return c;
}

static int
init_p_s3(State_t* state)
{
	return 0;
}

static int
decode_p_s3(State_t* state)
{
	return (state->loc - state->matchpos - MAGIC5) & 0xfff;
}

#define N_D	2
#define N_S	4

static Decode_f	decoders[] =
{
	init_c_d0,	decode_c_d0,
	init_c_d1,	decode_c_d0,
	init_c_s0,	decode_c_s0,
	init_c_s1,	decode_c_s1,
	init_c_s2,	decode_c_s2,
	init_c_s3,	decode_c_s3,

	init_p_d0,	decode_p_d0,
	init_p_d0,	decode_p_d0,
	init_p_s0,	decode_p_s0,
	init_p_s1,	decode_p_s1,
	init_p_s2,	decode_p_s2,
	init_p_s3,	decode_p_s3,
};

static int
lzh_decoder(char* s, int p, Codexdisc_t* disc)
{
	register int	i;
	register int	n;

	if (!(n = *s++))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "decoder name expected");
		return -1;
	}
	if (n == 'd')
		i = 0;
	else if (n == 's')
		i = 2 * N_D;
	else
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: d[ynamic] or s[tatic] decoder name expected", s - 1);
		return -1;
	}
	i += 2 * (int)strtol(s, &s, 10);
	if (i < 0 || i >= (elementsof(decoders) / 2))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%c%d: unknown decoder name", n, i);
		return -1;
	}
	if (p)
		i += 2 * (N_D + N_S);
	return i;
}

static char* const	arc[][4] =
{
	{	"lh0"				},
	{	"lh1",	"4k",	"d0",	"s0"	},
	{	"lh2",	"8k",	"d1"		},
	{	"lh3",	"8k",	"s0"		},
	{	"lh4",	"4k",	"s1"		},
	{	"lh5",	"8k",	"s1"		},
	{	"lhd"				},
	{	"lhj",	"26624","s1"		},
	{	"lz4"				},
	{	"lz5",	"4k",	"s3"		},
	{	"lz6",	"32k",	"s1"		},
	{	"lz7",	"64k",	"s1"		},
	{	"lzs",	"2k",	"s2"		},
};

static int
lzh_options(Codexmeth_t* meth, Sfio_t* sp)
{
	register int		i;
	register int		j;

	for (i = 0; i < elementsof(arc); i++)
	{
		sfprintf(sp, "[+%s?Equivalent to \b", arc[i][0]);
		if (!arc[i][1])
			sfprintf(sp, "copy");
		else
		{
			sfprintf(sp, "lzh");
			for (j = 1; j < elementsof(arc[i]) && arc[i][j]; j++)
				sfprintf(sp, "-%s", arc[i][j]);
		}
		sfprintf(sp, "\b.]");
	}
	return 0;
}

static int
lzh_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	const char*		name;
	char*			s;
	int			i;
	int			j;
	int			w;

	name = args[0];
	args += 2;
	if (!args[0])
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: method parameter expected", name);
		return -1;
	}
	if (!args[1])
	{
		for (i = 0;; i++)
		{
			if (i >= elementsof(arc))
			{
				if (p->disc->errorf)
					(*p->disc->errorf)(NiL, p->disc, 2, "%s: unknown method", name);
				return 0;
			}
			if (streq(args[0], arc[i][0]))
			{
				args = arc[i] + 1;
				break;
			}
		}
		if (!args[0])
		{
			p->meth = 0;
			return 0;
		}
	}
	if ((w = strton(args[0], &s, NiL, 0)) < WINDOW_MIN || w > WINDOW_MAX || *s)
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: window size must be in [%d..%d]", args[0], WINDOW_MIN, WINDOW_MAX);
		return -1;
	}
	if (!args[1])
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: at least one coder must be specified", name);
		return -1;
	}
	if ((i = lzh_decoder(args[1], 0, p->disc)) < 0 || (j = lzh_decoder(args[2] ? args[2] : args[1], 1, p->disc)) < 0)
		return -1;
	if (!(state = newof(0, State_t, 1, w - 1)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->window = w;
	state->init_c = decoders[i];
	state->decode_c = decoders[i + 1];
	state->init_p = decoders[j];
	state->decode_p = decoders[j + 1];
	state->codex = p;
	p->data = state;
	return 0;
}

static int
lzh_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	memset(&state->count, 0, sizeof(*state) - offsetof(State_t, count));
	state->ip = state->ie = 0;
	state->bitcount = 0;
	state->cpylen = 0;
	state->eof = 0;
	state->offset = 0x100 - 3;
	return ((*state->init_c)(state) || (*state->init_p)(state)) ? -1 : 0;
}

static ssize_t
lzh_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register char*		s = (char*)buf;
	register char*		e = s + n;
	int			c;
	int			i;
	ui4			j;

	if (n == 0 || state->eof)
		return 0;
	while (state->cpylen)
	{
		*s++ = state->text[state->loc++] = state->text[state->cpy++];
		if (state->loc >= state->window)
			state->loc = 0;
		if (state->cpy >= state->window)
			state->cpy = 0;
		state->cpylen--;
		if (s >= e)
			return s - (char*)buf;
	}
	for (;;)
	{
		c = (*state->decode_c)(state);
		if (c < 0)
			break;
		else if (c <= UCHAR_MAX)
		{
			state->count++;
			*s++ = state->text[state->loc++] = c;
			if (state->loc >= state->window)
				state->loc = 0;
			if (s >= e)
				break;
		}
		else
		{
			j = c - state->offset;
			i = state->loc - (*state->decode_p)(state) - 1;
			if (i < 0)
				i += state->window;
			state->count += j;
			while (j--)
			{
				*s++ = state->text[state->loc++] = state->text[i++];
				if (i >= state->window)
					i = 0;
				if (state->loc >= state->window)
					state->loc = 0;
				if (s >= e)
				{
					if (state->cpylen = j)
						state->cpy = i;
					return s - (char*)buf;
				}
			}
		}
	}
	return (n = s - (char*)buf) ? n : -1;
}

Codexmeth_t	codex_lzh =
{
	"lzh",
	"lzh compression. The options are ordered. A single option alias"
	" (example \blzh-lh5\b) or \awindow-ccode\a[-\apcode\a]"
	" (example \blzh-4k-d0-s0\b) are accepted. \apcode\a defaults to"
	" \accode\a if omitted.",
	"[+window?Window size <= 64k.]:[size]"
	"[+coding?\accode\a or \apcode\a coding method { d0 s0 s1 s2 s3 }.]"
	"[+(version)?codex-lzh 1998-11-11]",
	CODEX_DECODE|CODEX_COMPRESS,
	lzh_options,
	0,
	lzh_open,
	0,
	lzh_init,
	0,
	lzh_read,
	0,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(lzh)
};

CODEXLIB(lzh)
