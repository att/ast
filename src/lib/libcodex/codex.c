/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfio decode/encode discipline wrapper
 */

static const char	id[] = "codex";

#include <sfio_t.h>
#include <codex.h>
#include <namval.h>

static int
copy_close(Codex_t* code)
{
	return 0;
}

static Codexmeth_t	codex_copy =
{
	"copy",
	"No-op copy.",
	0,
	CODEX_DECODE|CODEX_ENCODE,
	0,
	0,
	0,
	copy_close,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

Codexstate_t	codexstate =
{
	"codex",
	codex_first,
	{ CODEX_VERSION },
	{ {0}, 0, 0, -1, 0, &codex_copy, 0},
};

static int
codex_except(Sfio_t* sp, int op, void* data, Sfdisc_t* disc)
{
	register Codex_t*	code = CODEX(disc);
	int			f;
	int			r;

	r = 0;
	switch (op)
	{
	case SF_ATEXIT:
		sfdisc(sp, SF_POPDISC);
		break;
	case SF_CLOSING:
	case SF_DPOP:
	case SF_FINAL:
		if (code->meth->syncf)
		{
			SFDCNEXT(sp, f);
			if (r = (*code->meth->syncf)(code))
				sp->_flags |= SF_ERROR;
			SFDCPREV(sp, f);
		}
		if (op != SF_CLOSING)
		{
			if (code->meth->donef)
			{
				SFDCNEXT(sp, f);
				r = (*code->meth->donef)(code);
				SFDCPREV(sp, f);
			}
			if (code->dp)
			{
				sfclose(code->dp);
				code->dp = 0;
			}
			if (code->flags & CODEX_ACTIVE)
				code->flags &= ~CODEX_ACTIVE;
			else if (!(code->flags & CODEX_CACHED))
			{
				if (code->meth->closef)
					r = (*code->meth->closef)(code);
				else if (code->data)
					free(code->data);
				if (code->op)
				{
					sfswap(code->op, code->sp);
					sfclose(code->op);
					code->op = 0;
				}
				free(code);
			}
		}
		break;
	case SF_DBUFFER:
		r = 1;
		break;
	case SF_SYNC:
		if (code->meth->syncf && !(code->flags & CODEX_FLUSH))
		{
			SFDCNEXT(sp, f);
			if ((r = (*code->meth->syncf)(code)) < 0)
				sp->_flags |= SF_ERROR;
			SFDCPREV(sp, f);
		}
		break;
	case CODEX_DATA:
		code->flags |= CODEX_FLUSH;
		sfsync(sp);
		code->flags &= ~CODEX_FLUSH;
		if (data && code->meth->dataf && !(r = (*code->meth->dataf)(code, (Codexdata_t*)data)) && ((Codexdata_t*)data)->size)
			r = 1;
		break;
	case CODEX_GETPOS:
		if (!code->meth->seekf || (*((Sfoff_t*)data) = (*code->meth->seekf)(code, (Sfoff_t)0, SEEK_CUR)) < 0)
			r = -1;
		break;
	case CODEX_SETPOS:
		if (!code->meth->seekf || (*code->meth->seekf)(code, *((Sfoff_t*)data), SEEK_SET) < 0)
			r = -1;
		break;
	case CODEX_SIZE:
		if (!data)
			r = -1;
		else
			code->size = *((Sfoff_t*)data);
		break;
	}
	return r;
}

static int
trace_close(Codex_t* code)
{
	Codexmeth_t*	meth = code->meth->next;
	int		r;

	if (!meth->closef)
		return 0;
	r = (*meth->closef)(code);
	sfprintf(sfstderr, "codex: %d: %s: close()=%d\n", code->index, meth->name, r);
	free(meth);
	return r;
}

static int
trace_init(Codex_t* code)
{
	Codexmeth_t*	meth = code->meth->next;
	int		r;

	if (!meth->initf)
		return 0;
	r = (*meth->initf)(code);
	sfprintf(sfstderr, "codex: %d: %s: init()=%d\n", code->index, meth->name, r);
	return r;
}

static int
trace_done(Codex_t* code)
{
	Codexmeth_t*	meth = code->meth->next;
	int		r;

	if (!meth->donef)
		return 0;
	r = (*meth->donef)(code);
	sfprintf(sfstderr, "codex: %d: %s: done()=%d\n", code->index, meth->name, r);
	return r;
}

static ssize_t
trace_read(Sfio_t* f, void* buf, size_t n, Sfdisc_t* disc)
{
	Codexmeth_t*	meth = CODEX(disc)->meth->next;
	ssize_t		r;

	if (!meth->readf)
		return 0;
	r = (*meth->readf)(f, buf, n, disc);
	sfprintf(sfstderr, "codex: %d: %s: read(%I*u,%s)=%I*d\n", CODEX(disc)->index, meth->name, sizeof(n), n, (CODEX(disc)->flags & CODEX_VERBOSE) ? fmtquote(buf, "\"", "\"", r, 0) : "''", sizeof(r), r, buf);
	return r;
}

static ssize_t
trace_write(Sfio_t* f, const void* buf, size_t n, Sfdisc_t* disc)
{
	Codexmeth_t*	meth = CODEX(disc)->meth->next;
	ssize_t		r;

	if (!meth->writef)
		return 0;
	r = (*meth->writef)(f, buf, n, disc);
	sfprintf(sfstderr, "codex: %d: %s: write(%I*u,%s)=%I*d\n", CODEX(disc)->index, meth->name, sizeof(n), n, (CODEX(disc)->flags & CODEX_VERBOSE) ? fmtquote(buf, "\"", "\"", r, 0) : "''", sizeof(r), r);
	return r;
}

static int
trace_except(Sfio_t* f, int op, void* data, Sfdisc_t* disc)
{
	Codexmeth_t*	meth = CODEX(disc)->meth->next;
	char*		event;
	int		r;
	char		tmp[8];

	r = codex_except(f, op, data, disc);
	switch (op)
	{
	case SF_ATEXIT:
		event = "ATEXIT";
		break;
	case SF_CLOSING:
		event = "CLOSING";
		break;
	case SF_DBUFFER:
		event = "DBUFFER";
		break;
	case SF_DPOLL:
		event = "DPOLL";
		break;
	case SF_DPOP:
		event = "DPOP";
		break;
	case SF_DPUSH:
		event = "DPUSH";
		break;
	case SF_FINAL:
		event = "FINAL";
		break;
	case SF_LOCKED:
		event = "LOCKED";
		break;
	case SF_NEW:
		event = "NEW";
		break;
	case SF_PURGE:
		event = "PURGE";
		break;
	case SF_READ:
		event = "READ";
		break;
	case SF_READY:
		event = "READY";
		break;
	case SF_SEEK:
		event = "SEEK";
		break;
	case SF_SYNC:
		event = "SYNC";
		break;
	case SF_WRITE:
		event = "WRITE";
		break;
	case CODEX_DATA:
		event = "CODEX_DATA";
		break;
	case CODEX_GETPOS:
		event = "CODEX_GETPOS";
		break;
	case CODEX_SETPOS:
		event = "CODEX_SETPOS";
		break;
	case CODEX_SIZE:
		event = "CODEX_SIZE";
		break;
	default:
		sfsprintf(event = tmp, sizeof(tmp), "%d", op);
		break;
	}
	sfprintf(sfstderr, "codex: %d: %s: except(%s,%p%s%s)=%d\n", CODEX(disc)->index, meth->name, event, data, (CODEX(disc)->flags & CODEX_ACTIVE) ? "|ACTIVE" : "", (CODEX(disc)->flags & CODEX_CACHED) ? "|CACHED" : "", r);
	return r;
}

static int
trace_sync(Codex_t* code)
{
	Codexmeth_t*	meth = code->meth->next;
	int		r;

	if (!meth->syncf)
		return 0;
	r = (*meth->syncf)(code);
	sfprintf(sfstderr, "codex: %d: %s: sync()=%d\n", code->index, meth->name, r);
	return r;
}

static Sfoff_t
trace_seek(Codex_t* code, Sfoff_t pos, int op)
{
	Codexmeth_t*	meth = code->meth->next;
	Sfoff_t		r;

	if (!meth->seekf)
		return 0;
	r = (*meth->seekf)(code, pos, op);
	sfprintf(sfstderr, "codex: %d: %s: seek(%I*d,%d)=%I*d\n", code->index, meth->name, sizeof(pos), pos, op, sizeof(r), r);
	return r;
}

static int
trace_data(Codex_t* code, Codexdata_t* data)
{
	Codexmeth_t*	meth = code->meth->next;
	int		r;
	unsigned char*	e;
	unsigned char*	u;

	if (!meth->dataf)
	{
		data->size = 0;
		return 0;
	}
	r = (*meth->dataf)(code, data);
	sfprintf(sfstderr, "codex: %d: %s: data()=%d", code->index, meth->name, r);
	if (r >= 0)
	{
		if (data->buf)
			for (e = (u = (unsigned char*)data->buf) + data->size; u < e; u++)
				sfprintf(sfstderr, "%02x", *u);
		else
			sfprintf(sfstderr, "%0*I*x", data->size * 2, sizeof(data->num), data->num);
	}
	sfprintf(sfstderr, "\n");
	return r;
}

static Codexmeth_t	codex_trace =
{
	"trace",
	"Debug trace wrapper.",
	0,
	CODEX_DECODE|CODEX_ENCODE,
	0,
	0,
	0,
	trace_close,
	trace_init,
	trace_done,
	trace_read,
	trace_write,
	trace_sync,
	trace_seek,
	trace_data,
};

#define OPT_TRACE	1
#define OPT_VERBOSE	2

static const Namval_t		options[] =
{
	"trace",	OPT_TRACE,
	"verbose",	OPT_VERBOSE,
	0,		0
};

/*
 * called by stropt() to set options
 */

static int
setopt(void* a, const void* p, register int n, register const char* v)
{
	NoP(a);
	if (p)
		switch (((Namval_t*)p)->value)
		{
		case OPT_TRACE:
			codexstate.trace = n ? strdup(v) : (char*)0;
			break;
		case OPT_VERBOSE:
			codexstate.verbose = n ? strdup(v) : (char*)0;
			break;
		}
	return 0;
}

static void
save(Codexcache_t* cache, Codex_t* code, const char* name, int namelen, Codexnum_t flags)
{
	if (cache->code && cache->code != CODEXERROR)
	{
		if (cache->code->meth->closef)
			(*cache->code->meth->closef)(cache->code);
		else if (cache->code->data)
			free(cache->code->data);
		if (cache->code->dp)
			sfclose(cache->code->dp);
		free(cache->code);
	}
	cache->code = code;
	cache->flags = flags;
	strncopy(cache->name, name, namelen < sizeof(cache->name) ? (namelen + 1) : sizeof(cache->name));
	cache->cached = ++codexstate.cached;
}

/*
 * single sfio method discipline push of meth!=0 onto sp
 */

static int
push(Sfio_t* sp, const char* name, Codexnum_t flags, Codexdisc_t* disc, Codexmeth_t* meth)
{
	register char**		a;
	register char*		s;
	register char*		b;
	register int		c;
	register int		q;
	char*			v;
	int			f;
	int			namelen;
	int			serial;
	Codexnum_t		deen;
	Sfoff_t			size;
	Codex_t*		code;
	Codexcache_t*		cache;
	Codexcache_t*		cp;
	Codexmeth_t*		trace;
	char*			arg[CODEX_ARGS];
	char			can[CODEX_NAME+CODEX_ARGS];

	/*
	 * check for matching inactive method in the cache
	 * this avoids openf/closef thrashing
	 */

	code = 0;
	cache = 0;
	deen = flags & (CODEX_DECODE|CODEX_ENCODE);
	s = (char*)name;
	q = 0;
	b = s;
	while (c = *b++)
		if (c == q)
			q = 0;
		else if (c == '"' || c == '\'')
			q = c;
		else if (!q && c == '+')
			break;
	namelen = b - s - 1;
	for (cp = codexstate.cache; cp < &codexstate.cache[elementsof(codexstate.cache)]; cp++)
		if (!cp->code)
			(cache = cp)->cached = 0;
		else if (!(cp->code->flags & CODEX_ACTIVE))
		{
			if (strneq(s, cp->name, namelen) && (cp->flags & deen) && (!cp->name[namelen] || cp->name[namelen] == '-' || cp->name[namelen] == '+'))
			{
				cp->cached = ++codexstate.cached;
				if ((code = (cache = cp)->code) == CODEXERROR)
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s: invalid method", s);
					goto bad;
				}
				break;
			}
			else if (!cache || cp->cached < cache->cached)
				cache = cp;
		}
	sfset(sp, SF_SHARE|SF_PUBLIC, 0);
	size = -1;
	if (!code)
	{
		if (!(code = newof(0, Codex_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "out of space");
			goto bad;
		}
		code->flags = deen;
		code->index = ++codexstate.index;
		code->sp = sp;
		code->disc = disc;
		code->meth = meth;
		a = arg;
		*a++ = (char*)s;
		*a++ = b = can;
		q = -1;
		do
		{
			c = *s++;
			if (c == q)
				q = -1;
			else if (c == '"' || c == '\'')
				q = c;
			else if (c == 0 || q < 0 && (c == '-' || c == '+'))
			{
				*b++ = 0;
				v = *(a - 1);
				if (strneq(v, "PASSPHRASE=", 11))
				{
					disc->passphrase = v + 11;
					b = v;
				}
				else if (streq(v, "RETAIN"))
				{
					flags |= CODEX_RETAIN;
					b = v;
				}
				else if (strneq(v, "SIZE=", 5))
				{
					size = strtoll(v + 5, NiL, 0);
					b = v;
				}
				else if (strneq(v, "SOURCE=", 7))
				{
					if (!(code->dp = sfopen(NiL, v + 7, "r")))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: cannot read delta source", v + 7);
						goto bad;
					}
					b = v;
				}
				else if (streq(v, "TRACE"))
				{
					flags |= CODEX_TRACE;
					b = v;
				}
				else if (streq(v, "VERBOSE"))
				{
					flags |= CODEX_VERBOSE;
					b = v;
				}
				else
				{
					if (a >= &arg[elementsof(arg)-1] || b >= &can[sizeof(can)-1])
						break;
					*a++ = b;
				}
			}
			else
				*b++ = c;
		} while (c && b < &can[sizeof(can)-1]);
		*b = 0;
		if (!**(a - 1))
			a--;
		*a = 0;
		s = arg[1];
		if ((*meth->openf)(code, arg, deen))
		{
			if (code->op)
			{
				sfswap(code->op, code->sp);
				sfclose(code->op);
				code->op = 0;
			}
			free(code);
			code = 0;
		}
		if ((flags & (CODEX_TRACE|CODEX_VERBOSE)) != (CODEX_TRACE|CODEX_VERBOSE))
		{
			if (codexstate.trace && strmatch(name, codexstate.trace))
				flags |= CODEX_TRACE;
			if (codexstate.verbose && strmatch(name, codexstate.verbose))
				flags |= CODEX_VERBOSE;
		}
		if ((flags & (CODEX_TRACE|CODEX_VERBOSE)) && (trace = memdup(&codex_trace, sizeof(codex_trace))))
			sfprintf(sfstderr, "codex: %d: %s: open(\"%s\",%s,%s)\n", code ? code->index : 0, meth->name, arg[0], (sp->_flags & SF_READ) ? "READ" : "WRITE", (deen & CODEX_DECODE) ? "DECODE" : "ENCODE");
		else
			trace = 0;
		if (!code)
		{
			if (cache)
				save(cache, CODEXERROR, name, namelen, deen);
			goto bad;
		}
		if (!(meth = code->meth))
			meth = code->meth = &codex_copy;
		code->flags = deen | (code->meth->flags & ~(CODEX_DECODE|CODEX_ENCODE));
		if (trace)
		{
			trace->next = meth;
			code->meth = meth = trace;
			code->sfdisc.readf = trace_read;
			code->sfdisc.writef = trace_write;
			code->sfdisc.exceptf = (flags & CODEX_VERBOSE) ? trace_except : codex_except;
		}
		else
		{
			code->sfdisc.readf = meth->readf;
			code->sfdisc.writef = meth->writef;
			code->sfdisc.exceptf = codex_except;
		}
		if (cache)
			save(cache, code, name, namelen, deen);
	}
	else if (b)
		while (*b++)
		{
			v = b;
			for (v = b; *b && *b != '-' && *b != '+'; b++);
			if (strneq(v, "SIZE=", 5))
				size = strtoll(v + 5, NiL, 0);
			else if (strneq(v, "SOURCE=", 7))
			{
				v += 7;
				if ((b - v) >= sizeof(can))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%-.*s: cannot delta source pathname too long", b - v, v);
					goto bad;
				}
				else
				{
					strncopy(can, v, b - v);
					if (!(code->dp = sfopen(NiL, can, "r")))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: cannot read delta source", can);
						goto bad;
					}
				}
			}
		}
	if (cache)
		code->flags |= CODEX_CACHED|CODEX_ACTIVE;
	if (code->meth == &codex_copy)
	{
		c = 0;
		goto done;
	}
	code->sp = sp;
	code->size = size;
	code->disc = disc;
	code->flags |= CODEX_FLUSH;
	if (sfdisc(sp, &code->sfdisc) != &code->sfdisc)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: sfio discipline push error", name);
		goto bad;
	}
	code->serial = serial = codexstate.serial;
	code->flags &= ~CODEX_FLUSH;
	if (!(sp->_flags & SF_READ))
		sfset(sp, SF_IOCHECK, 1);
	if (code->meth->initf)
	{
		SFDCNEXT(sp, f);
		c = (*code->meth->initf)(code);
		SFDCPREV(sp, f);
		if (c)
			goto bad;
	}
	code->flags |= flags & (CODEX_RETAIN|CODEX_TRACE|CODEX_VERBOSE);
	return serial;
 bad:
	c = -1;
	codexpop(sp, NiL, serial);
 done:
	if (code)
	{
		if (cache)
			code->flags &= ~CODEX_ACTIVE;
		else
		{
			if (meth->closef)
				(*meth->closef)(code);
			else if (code->data)
				free(code->data);
			if (code->dp)
				sfclose(code->dp);
			if (code->op)
			{
				sfswap(code->op, code->sp);
				sfclose(code->op);
				code->op = 0;
			}
			free(code);
		}
	}
	return c;
}

/*
 * recursively identify and push sfcode() disciplines
 * to decode the SF_READ stream ip
 *
 * return:
 *	-1	error
 *	 0	no discipline pushed
 *	>0	discipline serial number
 */

static int
decodex(Sfio_t* ip, Codexnum_t flags, Codexdisc_t* disc)
{
	register Codexmeth_t*	meth;
	void*			hdr;
	size_t			siz;
	int			serial;
	int			i;
	char			buf[CODEX_NAME];

	sfset(ip, SF_SHARE|SF_PUBLIC, 0);
	serial = 0;
	for (;;)
	{
		siz = CODEX_IDENT;
		if (!(hdr = sfreserve(ip, siz, 1)) && (!(siz = sfvalue(ip)) || !(hdr = sfreserve(ip, siz, 1))))
			break;
		meth = codexid(hdr, siz, buf, sizeof(buf));
		sfread(ip, hdr, 0);
		if (!meth)
			break;
		if ((i = codex(ip, NiL, buf, flags, disc, meth)) < 0)
		{
			if (serial)
				codexpop(ip, NiL, serial);
			return -1;
		}
		if (i)
		{
			serial = i;
			if (disc->version >= 20090704L && disc->identify)
				sfprintf(disc->identify, "<%s", meth->name);
		}
	}
	return serial;
}

typedef struct Part_s
{
	char*		name;
	Codexmeth_t*	meth;
	Codexnum_t	flags;
} Part_t;

/*
 * push the codex method composition name onto ip/op
 */

int
codex(Sfio_t* ip, Sfio_t* op, const char* name, Codexnum_t flags, Codexdisc_t* disc, Codexmeth_t* meth)
{
	register char*		s;
	register Part_t*	p;
	register int		c;
	register int		q;
	char*			m;
	Part_t*			b;
	Part_t*			e;
	Codexnum_t		f;
	Codexnum_t		decode;
	Codexnum_t		encode;
	int			invert;
	Part_t			part[256];

	if (!codexstate.initialized)
	{
		codexstate.initialized = 1;
		stropt(getenv("CODEX_OPTIONS"), options, sizeof(*options), setopt, NiL);
	}
	flags &= CODEX_INVERT|CODEX_RETAIN|CODEX_SERIAL|CODEX_TRACE|CODEX_VERBOSE;
	if (!(flags & CODEX_SERIAL))
	{
		flags |= CODEX_SERIAL;
		if (++codexstate.serial < 0)
			codexstate.serial = 1;
	}
	if (!disc)
		disc = &codexstate.disc;
	if (!(s = (char*)name) || !s[0] || s[0] == '-' && !s[1])
	{
		if (!meth)
			return ip ? decodex(ip, flags, disc) : -1;
		s = (char*)(name = meth->name);
	}

	/*
	 * split the method name into component parts
	 * and verify that each part is a known method
	 * CODEX_INVERT is dispatched here
	 */

	if (!(s = strdup(s)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "out of space");
		return -1;
	}
	m = s;
	if (flags & CODEX_INVERT)
	{
		flags &= ~CODEX_INVERT;
		invert = -1;
		decode = CODEX_ENCODE;
		encode = CODEX_DECODE;
		b = &part[elementsof(part)-1];
		e = &part[0];
	}
	else
	{
		invert = 1;
		decode = CODEX_DECODE;
		encode = CODEX_ENCODE;
		b = &part[0];
		e = &part[elementsof(part)-1];
	}
	p = b;
	if (*s == '<')
	{
		s++;
		p->flags = decode;
	}
	else if (*s == '>')
	{
		s++;
		p->flags = encode;
	}
	else
		p->flags = 0;
	p->name = s;
	q = 0;
	do
	{
		if (!(c = *s++))
			;
		else if (c == q)
		{
			q = 0;
			continue;
		}
		else if (q)
			continue;
		else if (c == '"' || c == '\'')
		{
			q = c;
			continue;
		}
		else if (c == '<')
			f = decode;
		else if (c == '>')
			f = encode;
		else if (c == '|' || c == '^')
			f = 0;
		else
			continue;
		*(s - 1) = 0;
		if (codexcmp(p->name, "copy"))
		{
			if (!(p->meth = codexmeth(p->name)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s: unknown coder", p->name);
				free(m);
				return -1;
			}
			if (!c)
				break;
			if (p == e)
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s: too many method components -- %d max", name, elementsof(part));
				free(m);
				return -1;
			}
			p += invert;
		}
		p->flags = f;
		p->name = s;
	} while (c);
	if (!*b->name)
	{
		free(m);
		return 0;
	}
	if (invert < 0)
	{
		e = b;
		b = p;
	}
	else
		e = p;

	/*
	 * assign CODEX_DECODE or CODEX_ENCODE to each part and
	 * verfy that the method supports the assignment
	 * vcodex methods give us some leeway here
	 * otherwise CODEX_DECODE must be on the left and
	 * CODEX_ENCODE must be on the right
	 * CODEX_INVERT has already been handled in the previous loop
	 */

	f = 0;
	if (ip)
		f |= CODEX_DECODE;
	if (op)
		f |= CODEX_ENCODE;
	p = b;
	for (;;)
	{
		if (p->flags == CODEX_ENCODE || p->flags != CODEX_DECODE && !(f & CODEX_DECODE))
		{
			p->flags = CODEX_ENCODE;
			f &= ~CODEX_DECODE;
		}
		else
			p->flags = CODEX_DECODE;
		if (!(p->flags & p->meth->flags) || !p->meth->vcmeth && !(p->flags & f))
		{
			if (disc->errorf)
			{
				if (e != &part[0])
					(*disc->errorf)(NiL, disc, 2, "%s: %s: cannot %s", name, p->name, p->flags == CODEX_DECODE ? ERROR_translate(NiL, NiL, id, "decode") : ERROR_translate(NiL, NiL, id, "encode"));
				else
					(*disc->errorf)(NiL, disc, 2, "%s: cannot %s", p->name, p->flags == CODEX_DECODE ? ERROR_translate(NiL, NiL, id, "decode") : ERROR_translate(NiL, NiL, id, "encode"));
			}
			free(m);
			return -1;
		}
		if (p == e)
			break;
		p++;
	}
#if 0
	p = b;
	for (;;)
	{
		error(-1, "AHA codex [%d] %u %s \"%s\"", p - b, p->flags, p->meth->name, p->name);
		if (p == e)
			break;
		p++;
	}
#endif

	/*
	 * input decode method sfio disciplines pushed from left to right
	 */

	p = b;
	for (;;)
	{
		if (p->flags == CODEX_ENCODE)
			break;
		if (push(ip, p->name, CODEX_DECODE|flags, disc, p->meth) < 0)
		{
			free(m);
			codexpop(ip, op, codexstate.serial);
			return -1;
		}
		if (p == e)
			break;
		p++;
	}

	/*
	 * output encode method sfio disciplines pushed from right to left
	 */

	p = e;
	for (;;)
	{
		if (p->flags == CODEX_DECODE)
			break;
		if (push(op, p->name, CODEX_ENCODE|flags, disc, p->meth) < 0)
		{
			free(m);
			codexpop(ip, op, codexstate.serial);
			return -1;
		}
		if (p == b)
			break;
		p--;
	}
	free(m);
	return codexstate.serial;
}

/*
 * pop contiguous ip and op codex disciplines matching serial number
 * serial==0 pops all contiguous codex disciplines
 */

int
codexpop(register Sfio_t* ip, register Sfio_t* op, int serial)
{
	int	pop;

	pop = 0;
	if (serial >= 0)
	{
		if (ip)
			while (ip->disc && (ip->disc->exceptf == codex_except || ip->disc->exceptf == trace_except) && (!serial || CODEX(ip->disc)->serial == serial))
				if (sfdisc(ip, SF_POPDISC))
					pop++;
		if (op)
			while (op->disc && (op->disc->exceptf == codex_except || op->disc->exceptf == trace_except) && (!serial || CODEX(op->disc)->serial == serial))
				if (sfdisc(op, SF_POPDISC))
					pop++;
	}
	return pop;
}
