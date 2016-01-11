#pragma prototyped

/*
 * checksum filter
 */

#include <codex.h>
#include <sum.h>

typedef struct State_s
{
	Codex_t		codex;

	Sum_t*		sum;
} State_t;

static int
sum_options(Codexmeth_t* meth, Sfio_t* sp)
{
	return sumusage(sp) >= 0 ? 0 : -1;
}

static int
sum_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	register Sum_t*		sum;
	register const char*	s;

	s = args[0];
	if (args[2])
		while (*s && *s++ != '-');
	if (!(sum = sumopen(s)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: unknown method", args[0]);
		return -1;
	}
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->sum = sum;
	p->data = state;
	return 0;
}

static int
sum_init(Codex_t* p)
{
	return suminit(((State_t*)p->data)->sum);
}

static int
sum_close(Codex_t* p)
{
	return sumclose(((State_t*)p->data)->sum);
}

static ssize_t
sum_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	ssize_t		r;

	if ((r = sfrd(sp, buf, n, disc)) > 0)
		sumblock(((State_t*)CODEX(disc)->data)->sum, buf, r);
	return r;
}

static ssize_t
sum_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	ssize_t		r;

	if ((r = sfwr(sp, buf, n, disc)) > 0)
		sumblock(((State_t*)CODEX(disc)->data)->sum, buf, r);
	return r;
}

static int
sum_data(Codex_t* p, Codexdata_t* data)
{
	sumdone(((State_t*)p->data)->sum);
	return sumdata(((State_t*)p->data)->sum, (Sumdata_t*)data);
}

Codexmeth_t	codex_sum =
{
	"sum",
	"checksum filter. The checksum value returned by \bcodexdata\b(3) is"
	" written to the standard error.",
	"[+(version)?codex-sum (AT&T Research) 2003-12-16]"
	"[+(author)?Glenn Fowler <gsf@research.att.com>]",
	CODEX_DECODE|CODEX_ENCODE|CODEX_SUM,
	sum_options,
	0,
	sum_open,
	0,
	sum_init,
	0,
	sum_read,
	sum_write,
	sum_init,
	0,
	sum_data,
	0,
	0,
	CODEXNEXT(sum)
};

CODEXLIB(sum)
