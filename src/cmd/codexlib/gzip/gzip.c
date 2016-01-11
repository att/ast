#pragma prototyped

/*
 * gzip encoder/decoder
 */

#include <codex.h>
#include <zlib.h>
#include <ctype.h>

#define MINLEVEL	1
#define DEFLEVEL	6
#define MAXLEVEL	9

#define X(x)		#x
#define S(x)		X(x)

typedef struct State_s
{
	Codex_t*	codex;
	gzFile*		gz;
	int		crc;
	int		level;
} State_t;

static int
gzip_ident(Codexmeth_t* meth, const void* head, size_t headsize, char* name, size_t namesize)
{
	unsigned char*	h = (unsigned char*)head;

	if (headsize >= 3 && h[0] == 0x1f && h[1] == 0x8b)
	{
		strncopy(name, meth->name, namesize);
		return 1;
	}
	return 0;
}

static int
gzip_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	const char*		s;
	char*			e;
	int			i;
	int			v;
	int			crc = 1;
	int			level = DEFLEVEL;

	i = 2;
	while (s = args[i++])
	{
		if (isdigit(*s))
			v = strton(s, &e, NiL, 0);
		else
		{
			e = (char*)s;
			if (e[0] == 'n' && e[1] == 'o')
			{
				e += 2;
				v = 0;
			}
			else
				v = 1;
		}
		if (!*e)
		{
			if (v < MINLEVEL || v > MAXLEVEL)
			{
				if (p->disc->errorf)
					(*p->disc->errorf)(NiL, p->disc, 2, "%s: compression level must be in [%d..%d]", s, MINLEVEL, MAXLEVEL);
				return -1;
			}
			level = v;
		}
		else if (!streq(e, "crc"))
			crc = v;
		else
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: unknown option", s);
			return -1;
		}
	}
	if (!(p->op = sfopen(NiL, "/dev/null", (p->flags & CODEX_DECODE) ? "r" : "w")))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "cannot swap main stream");
		return -1;
	}
	sfswap(p->op, p->sp);
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->codex = p;
	state->crc = crc;
	state->level = level;
	p->data = state;
	return 0;
}

static int
gzip_init(Codex_t* p)
{
	State_t*	state = (State_t*)p->data;
	char*		m;
	char		mode[8];

	m = mode;
	if (p->flags & CODEX_ENCODE)
		*m++ = 'w';
	else
		*m++ = 'r';
	*m++ = 'b';
	if (!state->crc)
		*m++ = 'c';
	*m++ = 'o';
	*m++ = '0' + state->level;
	*m = 0;
	return (state->gz = gzfopen(p->op, mode)) ? 0 : -1;
}

static int
gzip_done(Codex_t* p)
{
	State_t*	state = (State_t*)p->data;
	int		r;

	r = gzclose(state->gz) ? -1 : 0;
	free(state);
	return r;
}

static ssize_t
gzip_read(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* dp)
{
	return gzread(((State_t*)CODEX(dp)->data)->gz, buf, n);
}

static ssize_t
gzip_write(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* dp)
{
	return gzwrite(((State_t*)CODEX(dp)->data)->gz, buf, n);
}

Codexmeth_t	codex_gzip =
{
	"gzip",
	"gzip compression. The first parameter is the compression level,"
	" " S(MINLEVEL) ":least, " S(MAXLEVEL) ":most, " S(DEFLEVEL) " is the default. nocrc disables crc checks.",
	0,
	CODEX_DECODE|CODEX_ENCODE|CODEX_COMPRESS,
	0,
	gzip_ident,
	gzip_open,
	0,
	gzip_init,
	gzip_done,
	gzip_read,
	gzip_write,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(gzip)
};

CODEXLIB(gzip)
