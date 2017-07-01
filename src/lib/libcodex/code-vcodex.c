/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * vcodex encoder/decoder for codex
 */

#include <codex.h>
#include <vclib.h>
#include <ccode.h>

#define VCSF_BUFSIZE	(256*1024)	/* decoding buffer	*/
#define VCSF_BUFMIN	(64*1024)	/* surely affordable!	*/
#define VCSF_WSIZE	(4*1024*1024)	/* dflt encoding window	*/
#define VCSF_SLACK	(16*sizeof(size_t)) /* for extra coding	*/
#define VCSFDTSZ(sz)	((((sz) / 1024) + 2) * 1024)

#define VCSFERROR(s,m)	((s)->codex->disc->errorf ? ((*(s)->codex->disc->errorf)(NiL,(s)->codex->disc,2,m), -1) : -1)

/* these bits are in state->flags and must be outside VC_FLAGS	*/
#define VCSF_DONEHEAD	0001000	/* header was already output	*/
#define VCSF_KEEPSFDC	0002000	/* do not free the Sfdc struct	*/
#define VCSF_VCDIFF	0004000	/* output RFC3284 VCDIFF header	*/
#define VCSF_PLAIN	0010000	/* no header to be output	*/
#define VCSF_TRANS	0020000	/* set transform on VC_DECODE	*/
#define VCSF_FREE	0040000	/* free sfdt on disc pop	*/
#define VCSF_OPEN	0100000	/* open for business		*/

typedef struct State_s
{
	Codex_t*	codex;	/* codex handle			*/

	unsigned int	flags;	/* states of the handle		*/

	char*		transform;/* transform string		*/

	Sfoff_t		pos;	/* current stream position	*/
	Vcwdisc_t	vcwdc;	/* windowing discipline		*/
	Vcwindow_t*	vcw;	/* to match delta-comp windows 	*/

	Vcdisc_t	vcdc;	/* encoding/decoding discipline	*/
	Vcodex_t*	vc;	/* Vcodex handle for de/coding	*/

	Vcio_t*		io;	/* to write integers, etc.	*/

	Vcchar_t*	data;	/* data buffer for external use	*/
	ssize_t		dtsz;
	Vcchar_t*	endd;	/* end of data buffer		*/
	Vcchar_t*	next;	/* where to read/write		*/

	Vcchar_t*	base;	/* buffer for encoded data	*/
	ssize_t		bssz;
	Vcchar_t*	endb;	/* end of base buffer		*/

	Vcchar_t*	code;	/* space to be used by coder	*/
	ssize_t		cdsz;
} State_t;

static int		vcodex_sync(Codex_t*);

#ifndef SKIPVCDX /* deal with the ancient VCDX header that uses method numbers */

#ifndef elementsof
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#endif

typedef struct _old_s /* to map an old method number to a transform */
{	const char*	name;	/* new method name	*/
	int		mtid;	/* old method number	*/
	Vcmethod_t*	meth;	/* corresponding method	*/
} Old_t;

typedef struct _meth_s /* keep method data for reverse construction */
{	Vcmethod_t*	meth;
	ssize_t		size;
	Vcchar_t*	data;
} Meth_t;

static Old_t		old[] =
{	{ "delta",	0	},
	{ "huffman",	1	},
	{ "huffgroup",	2	},
	{ "arith",	3	},
	{ "bwt",	4	},
	{ "rle",	5	},
	{ "mtf",	6	},
	{ "transpose",	7	},
	{ "table",	8	},
	{ "huffpart",	9	},
	{ "map",	50	},
	{ "ama",	100	},
	{ "ss7",	101	},
};

static ssize_t
hdrvcdx(Vcchar_t** datap, ssize_t dtsz, Vcchar_t* code, ssize_t cdsz)
{
	ssize_t		k, n, id, sz;
	Vcchar_t	*dt, buf[1024];
	Vcio_t		io;
	Meth_t		meth[2*elementsof(old)]; /* should not exceed this */

	/* parse old header into list of transforms */
	vcioinit(&io, *datap, dtsz);
	for(n = 0; n < elementsof(meth) && vciomore(&io) > 0; ++n)
	{	id = vciogetc(&io);
		for(k = 0; k < elementsof(old); ++k)
			if(old[k].mtid == id)
				break;
		if(k >= elementsof(old)) /* not matching any transform */
			return dtsz; /* must be new header format */

		if(!old[k].meth && !(old[k].meth = vcgetmeth((char*)old[k].name, 1)))
		{	old[k].mtid = -1;
			return dtsz;
		}
		meth[n].meth = old[k].meth;

		if(vciomore(&io) <= 0 || /* no or bad argument code */
		   (sz = (ssize_t)vciogetu(&io)) < 0 || sz > vciomore(&io) )
			return dtsz; /* must be new header format */

		meth[n].size = sz;
		meth[n].data = vcionext(&io);
		vcioskip(&io, sz);
	}

	if(vciomore(&io) > 0) /* too many transforms */
		return dtsz; /* must be new header */

	/* construct new header in reverse order */
	vcioinit(&io, code, cdsz);
	for(n -= 1; n >= 0; --n)
	{	/* get and write out the method id string */
		if(!(dt = (unsigned char*)vcgetident(meth[n].meth, (char*)buf, sizeof(buf))) )
			return -1; /* error, no id string for method? */
		if((sz = strlen((char*)dt)+1) > vciomore(&io)) /* too long */
			return dtsz; /* must be new header format */
		vcioputs(&io, dt, sz);

		sz = meth[n].size; dt = meth[n].data;
		if((sz + vcsizeu(sz)) > vciomore(&io))
			return dtsz;

		if(sz == 0)
			vcioputu(&io, 0);
		else if(meth[n].meth == Vcrle || meth[n].meth == Vcmtf)
		{	if(*dt == 0) /* coding rle.0 or mtf.0 */
			{	vcstrcode("0", (char*)buf, sizeof(buf));
				vcioputu(&io, 1);
				vcioputc(&io, buf[0]);
			}
			else	vcioputu(&io, 0);
		}
		else /* let us pray for the right coding! */
		{	vcioputu(&io, sz);
			vcioputs(&io, dt, sz);
		}
	}

	*datap = code; /* set to return new code string */
	return vciosize(&io);
}

#endif /* SKIPVCDX */

/* dealing with RFC3284 Vcdiff header */
static ssize_t
hdrvcdiff(Vcio_t* iodt, int indi, Vcchar_t* code, ssize_t cdsz)
{
	Vcio_t	io;
	char	*ident, buf[1024];

	if(indi&VCD_COMPRESSOR) /* no secondary compressor */
		return -1;

	vcioinit(&io, code, cdsz);

	/* write out the method identification string */
	if(!(ident = vcgetident(Vcdelta, buf, sizeof(buf))) )
		return -1;
	if((cdsz = strlen(ident)+1) > vciomore(&io))
		return -1;
	vcioputs(&io, ident, cdsz);

	if(indi&VCD_CODETABLE)
	{	if((cdsz = vciogetu(iodt)) < 0 )
			return -1;

		if(vciomore(&io) < vcsizeu(cdsz))
			return -1;
		vcioputu(&io, cdsz);

		if(vciomore(&io) < cdsz)
			return -1;
		vciomove(iodt, &io, cdsz);
	}
	else
	{	if(vciomore(&io) <= 0)
			return -1;
		vcioputu(&io, 0);
	}

	return vciosize(&io);
}

static int
putheader(State_t* state)
{
	Vcchar_t	*code;
	ssize_t		sz;

	/* header should be done at most once */
	if((state->flags&VCSF_DONEHEAD) )
		return 0;
	state->flags |= VCSF_DONEHEAD;

	/* header not wanted */
	if(state->flags&VCSF_PLAIN)
		return 0;

	/* get the string that codes the methods */
	if((sz = vcextract(state->vc, (Void_t**)&code)) <= 0 )
		return VCSFERROR(state, "transform data not encodable");
	if((4+1+vcsizeu(sz)+sz) > vciomore(state->io))
		return VCSFERROR(state, "transform data abnormally large");

	/* output standard header bytes */
	vcioputc(state->io, VC_HEADER0);
	vcioputc(state->io, VC_HEADER1);
	vcioputc(state->io, VC_HEADER2);
	vcioputc(state->io, (state->flags&VCSF_VCDIFF) ? 0 : VC_HEADER3);

	/* output indicator byte - see also compatibility with RFC3284 */
	vcioputc(state->io, 0);

	/* output data encoding the methods used */
	if(!(state->flags&VCSF_VCDIFF) )
	{	vcioputu(state->io, sz);
		vcioputs(state->io, code, sz);
	}

	return 0;
}

static int
makebuf(State_t* state, ssize_t size)
{
	Vcchar_t	*base, *oldbase;

	size = ((size + VCSF_BUFMIN - 1)/VCSF_BUFMIN)*VCSF_BUFMIN;
	if(state->bssz >= size)
		return 0;

	oldbase = state->base;
	if(!(base = (Vcchar_t*)malloc(size)) )
		return -1;

	memcpy(base, oldbase, state->endb - oldbase);
	state->endb = base + (state->endb - oldbase);
	state->code = base + (state->code - oldbase);
	state->base = base;
	state->bssz = size;
	free(oldbase);

	return 0;
}

static ssize_t
fillbuf(State_t* state, Sfio_t* f, Sfdisc_t* disc)
{
	ssize_t	sz, n;

	if((sz = state->endb - state->code) <= 0 )
	{	state->endb = state->code = state->base;
		sz = 0;
	}
	else if(state->code > state->base)
	{	memcpy(state->base, state->code, sz);
		state->endb = (state->code = state->base) + sz;
	}
	for(; sz < state->bssz; sz += n, state->endb += n)
	{	if(!disc) /* plain read if no discipline set yet */
			n = sfread(f, state->endb, state->bssz-sz);
		else	n = sfrd(f, state->endb, state->bssz-sz, disc);
		if(n <= 0)
			break;
	}

	return sz;
}

static int
extend(char** pc, char** pb, char** pe, int n)
{
	int	x;
	int	o;

	o = *pc - *pb;
	x = (*pe - *pb + 1);
	if (x < n)
		x = n;
	x *= 2;
	if (!(*pb = malloc(x)))
		return -1;
	*pe = *pb + x - 1;
	*pc = * pb + o;
	return 0;
}

static ssize_t
ident(State_t* state, const Void_t* data, size_t dtsz, char* buf, size_t bfsz)
{
	ssize_t		sz, k;
	char		*mt;
	char		*id, *end;
	const char	*name;
	unsigned char	*dt;
	unsigned char	*map;
	int		i;
	int		x;
	Vcio_t		io;

	static char	hex[] = "0123456789ABCDEF";

	if(!data || dtsz <= 0 )
		return -1;
	x = bfsz;
	if(!buf && !(buf = malloc(x)))
		return -1;
	id = buf;
	end = buf + x - 1;
	vcioinit(&io, data, dtsz);
	id = buf;
	map = CCMAP(CC_ASCII, CC_NATIVE);
	while(vciomore(&io) > 0)
	{	
		if(id > buf)
		{	if(id >= end && (bfsz || extend(&id, &buf, &end, 1)))
				return -1;
			*id++ = '^';
		}
		mt = (char*)vcionext(&io);
		sz = vciomore(&io);
		if(sz >= 2 && mt[1] == 0)
		{	/* obsolete index */
			x = mt[0];
			k = 2;
			name = "UNKNOWN";
			for(i = 0; i < elementsof(old); ++i)
				if(old[i].mtid == x)
				{	name = old[i].name;
					break;
				}
			while (i = *name++)
			{	if(id >= end && (bfsz || extend(&id, &buf, &end, 1)))
					return -1;
				*id++ = i;
			}
		}
		else
			for(k = 0; k < sz; ++k)
			{	if(id >= end && (bfsz || extend(&id, &buf, &end, 1)))
					return -1;
				if(mt[k] == 0)
					break;
				*id++ = map ? map[mt[k]] : mt[k];
			}
		if(k >= sz)
			return -1;
		vcioskip(&io, k+1);

		/* get the initialization data, if any */
		if((sz = (ssize_t)vciogetu(&io)) < 0 || sz > vciomore(&io))
			return -1;
		if(sz)
		{	dt = (unsigned char*)vcionext(&io);
			vcioskip(&io, sz);
			k - 2 * sz + 1;
			if((id + k) >= end && (bfsz || extend(&id, &buf, &end, k)))
				return -1;
			*id++ = '=';
			for(k = 0; k < sz; k++)
			{	x = dt[k];
				*id++ = hex[(x>>4)&0xf];
				*id++ = hex[x&0xf];
			}
		}
	}
	*id = 0;
	if (state && !bfsz)
		state->transform = buf;
	return id - buf;
}

static ssize_t
getheader(State_t* state, Sfio_t* f, int init, int identify, int optional, Sfdisc_t* disc)
{
	ssize_t		cdsz, sz;
	Vcchar_t	*code, cdbuf[4*1024];
	int		indi, head, loop;
	Vcio_t		io;

	if(optional)
		identify = -1;
	if(identify)
	{	/* verify header magic -- ignore if no magic */
		if(!(code = sfreserve(f, 4, SF_LOCKR)))
			return 0;
		memcpy(cdbuf, code, 4);
		sfread(f, code, 0);
		if(cdbuf[0]!=VC_HEADER0 || cdbuf[1]!=VC_HEADER1 || cdbuf[2]!=VC_HEADER2 || cdbuf[3]!=VC_HEADER3 && cdbuf[3]!=0)
			return 0;
	}

	for(loop = 0;; ++loop)
	{	
		/* buffer was too small for header data */
		if(loop > 0 && (state->endb - state->base) >= state->bssz &&
		   makebuf(state, state->bssz+VCSF_BUFMIN) < 0)
			return VCSFERROR(state, "out of space");

		/* read header data as necessary */
		sz = state->endb - state->code;
		if(loop > 0 || sz <= 0)
		{	if(fillbuf(state, f, state->vc ? disc : NIL(Sfdisc_t*)) <= 0 ||
			   (sz = state->endb - state->code) <= 0 )
				return identify ? 0 : init ? VCSFERROR(state, "corrupt header data") : -1;
		}

		vcioinit(&io, state->code, sz);

		if(vciomore(&io) < 5) /* need 4-byte header + indicator byte */
			continue;

		if(vciogetc(&io) != VC_HEADER0 ||
		   vciogetc(&io) != VC_HEADER1 ||
		   vciogetc(&io) != VC_HEADER2 ||
		   ((head = vciogetc(&io)) != 0 /* RFC3284 Vcdiff header */ &&
		    head != VC_HEADER3 /* normal Vcodex header */ ) )
			return identify ? 0 : VCSFERROR(state, "unknown header data");

		if((indi = vciogetc(&io)) & VC_EXTRAHEADER )
		{	if((sz = vciogetu(&io)) < 0) /* skip app-specific data */
				continue;
			vcioskip(&io, sz);
		}

		if(head == 0) /* RFC3284 Vcdiff format */
		{	if((cdsz = hdrvcdiff(&io, indi, cdbuf, sizeof(cdbuf))) < 0)
				return VCSFERROR(state, "corrupted vdelta header data");
			else if(cdsz == 0)
				continue;
			else	code = cdbuf;
		}
		else /* Vcodex encoding */
		{	if((cdsz = vciogetu(&io)) < 0 )
				continue;
			else if(cdsz == 0)
				return VCSFERROR(state, "corrupt vcodex header data");
			if(vciomore(&io) < cdsz)
				continue;
			code = vcionext(&io);
			vcioskip(&io, cdsz);

#ifndef SKIPVCDX /* deal with old headers that use method numbers instead of names */
			cdsz = hdrvcdx(&code, cdsz, cdbuf, sizeof(cdbuf));
#endif
		}

		/* successfully read the header data */
		state->code = vcionext(&io);
		break;
	}
	if(cdsz <= 0 )
		return identify ? 0 : VCSFERROR(state, "cannot read header data");
	else if(identify > 0)
		return ident(state, code, cdsz, NiL, 0);
	else
	{	if(state->vc)
			vcclose(state->vc);
		if(!(state->vc = vcrestore(code, cdsz)) )
			return VCSFERROR(state, "data transform initialization failed");
		else if(state->flags & VCSF_TRANS)
			return ident(state, code, cdsz, NiL, 0);
		else	return 1;
	}
}

static ssize_t
encode(State_t* state, Vcchar_t* data, size_t dtsz)
{
	Vcchar_t	*code, ctrl, *dt;
	ssize_t		cdsz, size, sz, bssz, dosz;
	Vcchar_t	*base;
	Vcwmatch_t	*wm;
	Vcio_t		io;

	vcioinit(&io, state->base, state->bssz);
	state->io = &io;

	if(!(state->flags&VCSF_DONEHEAD) && putheader(state) < 0)
		return VCSFERROR(state, "transform encoding failed");

	state->code = NIL(Vcchar_t*); state->cdsz = 0;

	for(size = 0, dosz = dtsz, dt = data; size < dtsz; )
	{	/* control data */
		ctrl = 0;
		state->vcdc.data = NIL(Void_t*);
		state->vcdc.size = 0;

		/* compute a matching window to enhance compression */
		wm = NIL(Vcwmatch_t*);
		if(state->vcw)
			wm = vcwapply(state->vcw, dt, dosz, state->pos);
		if(wm)
		{	/**/DEBUG_ASSERT(wm->msize <= dosz);
			if(wm->wsize > 0 && wm->wpos >= 0)
			{	state->vcdc.data = wm->wdata;
				state->vcdc.size = wm->wsize;
			}

			ctrl = wm->type & (VCD_SOURCEFILE|VCD_TARGETFILE);
			dosz = wm->msize; /* amount doable now */
			/**/ DEBUG_PRINT(2,"dtpos=%d ", state->pos);
			/**/ DEBUG_PRINT(2,"dtsz=%d ", dosz);
			/**/ DEBUG_PRINT(2,"wpos=%d ", wm->wpos);
			/**/ DEBUG_PRINT(2,"wsz=%d ", wm->wsize);
			/**/ DEBUG_PRINT(2,"mtsz=%d\n", wm->msize);
		}
		if(state->vcw) /* set window data */
			vcdisc(state->vc, &state->vcdc);

		vcbuffer(state->vc, NIL(Vcchar_t*), -1, -1); /* free buffers */
		if((cdsz = vcapply(state->vc, dt, dosz, &code)) <= 0 ||
		   (sz = vcundone(state->vc)) >= dosz )
		{	if(cdsz < 0)
				return VCSFERROR(state, "data transform failed");
			ctrl = VC_RAW; /* coder failed, output raw data */
			code = dt;
			cdsz = dosz;
		}
		else
		{	dosz -= (sz > 0 ? sz : 0); /* true processed amount */
			if(state->vcw) /* tell window matcher compressed result */
				vcwfeedback(state->vcw, cdsz);
		}

		vcioputc(&io, ctrl);
		if(ctrl&(VCD_SOURCEFILE|VCD_TARGETFILE) )
		{	vcioputu(&io, wm->wsize);
			vcioputu(&io, wm->wpos);
		}
		vcioputu(&io, cdsz);
		if(vciomore(&io) < cdsz) /* buffer too small */
		{	sz = vciosize(&io);
			bssz = ((cdsz+sz+1023)/1024)*1024;
			if(!(base = (Vcchar_t*)malloc(bssz)) )
				return -1;
			memcpy(base, state->base, vciosize(&io));
			if(state->base)
				free(state->base);
			state->base = base;
			state->bssz = bssz;

			vcioinit(&io, state->base, state->bssz);
			vcioskip(&io, sz);
		}
		vcioputs(&io, code, cdsz);

		state->pos += dosz; /* advance by amount consumed */
		dt += dosz;
		size += dosz;

		if((dosz = dtsz-size) > 0 )
		{	if(wm && wm->more) /* more subwindows to do */
				continue;
			else /* need fresh data */
			{	if(data == state->data) /* shift undone data */
				{	memcpy(data, data+size, dosz);
					state->next = data + dosz;
				}
				break;
			}
		}
	}

	state->code = state->base; state->cdsz = vciosize(&io);

	return size; /* return amount processed */
}

static int
vcodex_ident(Codexmeth_t* meth, const void* head, size_t headsize, char* name, size_t namesize)
{
	unsigned char*	h = (unsigned char*)head;
	ssize_t		n;
	ssize_t		m;
	Vcio_t		io;

	if (headsize >= 6 && h[0] == VC_HEADER0 && h[1] == VC_HEADER1 && h[2] == VC_HEADER2 && (h[3] == 0 || h[3] == VC_HEADER3))
	{
		vcioinit(&io, h + 4, headsize - 4);
		if (vciogetc(&io) & VC_EXTRAHEADER)
		{
			if ((n = vciogetu(&io)) < 0)
				return 0;
			vcioskip(&io, n);
		}
		if ((n = vciogetu(&io)) <= 0 || (m = vciomore(&io)) < n)
			return 0;
		if (ident(NiL, vcionext(&io), n, name, namesize) < 0)
			strncopy(name, meth->name, namesize);
		return 1;
	}
	return 0;
}

static int
vcodex_done(Codex_t* p)
{
	State_t*	state = (State_t*)p->data;
	int		r;
	ssize_t		sz;
	ssize_t		wz;
	Vcio_t		io;

	r = 0;
	if ((p->flags & VCSF_OPEN) && !(r = vcodex_sync(p)))
	{
		/* back to plain text mode */
		state->flags |= VCSF_KEEPSFDC;
		sfdisc(p->sp, NIL(Sfdisc_t*));
		state->flags &= ~VCSF_KEEPSFDC;
		vcioinit(&io, state->base, state->bssz);
		if (!(state->flags & VCSF_DONEHEAD))
		{	
			state->io = &io;
			if (putheader(state) < 0 )
				r = VCSFERROR(state, "Error writing header");
			vcioputu(&io, 0);
		}
		if (!(state->flags & VCSF_PLAIN))
			vcioputc(&io, VC_EOF); /* write the eof marker */
		sz = vciosize(&io); /* output to stream */
		if (sz > 0 && (wz = sfwr(p->sp, state->base, sz, NIL(Sfdisc_t*))) < sz)
			r = VCSFERROR(state, "Error in writing coded data");
	}
	if (state->vc)
		vcclose(state->vc);
	if (state->vcwdc.srcf)
		sfclose(state->vcwdc.srcf);
	if (state->vcwdc.tarf)
		sfclose(state->vcwdc.tarf);
	if (state->vcw)
		vcwclose(state->vcw);
	if ((state->flags & VC_ENCODE) && state->data)
		free(state->data);
	if (state->base)
		free(state->base);
	free(state);
	return r;
}

static int
vcodex_option(Codex_t* p, const char* s, Vcwmethod_t** wmeth, size_t* wsize, unsigned int* vflags, char** source)
{
	int		b;
	char*		e;
	char*		x;
	ssize_t		v;
	Vcwmethod_t*	w;

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
	for (x = e; b = *x++;)
		if (b == '.' || b == '=' || b == ',')
			break;
	if (!b)
		x = 0;
	if (!*e)
	{
		if (!*wsize)
			*wsize = v;
	}
	else if (streq(e, "plain"))
	{
		if (v)
			*vflags |= VCSF_PLAIN;
		else
			*vflags &= ~VCSF_PLAIN;
	}
	else if (streq(e, "source"))
	{
		if (!x)
			goto badarg;
		*source = x;
	}
	else if (streq(e, "vcdiff"))
	{
		if (v)
			*vflags |= VCSF_VCDIFF;
		else
			*vflags &= ~VCSF_VCDIFF;
	}
	else if (w = vcwgetmeth(e))
	{
		while (b = *e++)
			if (b == '.' || b == '=')
			{
				v = strton(e, &e, NiL, 0);
				if (*e)
				{
					if (p->disc->errorf)
						(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: invalid window size", x, e);
					return -1;
				}
				break;
			}
		if (!wmeth)
		{
			*wmeth = w;
			if (v > 1 && !*wsize)
				*wsize = v;
		}
	}
	else
	{
	badarg:
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: unknown option", s, e);
		return -1;
	}
	return 0;
}

static int
vcodex_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	const char*		s;
	int			i;
	unsigned int		vflags;
	char*			source;
	char*			transform;
	ssize_t			wsize;
	Vcwmethod_t*		wmeth;
	Vcodex_t*		vc;

	/*
	 * vcodex options
	 *
	 *	-[no]plain
	 *	-source='path'
	 *	-source="path"
	 *	[-window_method]-window_size
	 *	-window_method-window_size
	 *	-window_method=window_size
	 *	-window_method.window_size
	 */

#if CODEX_VERSION >= 20130501L
	source = p->disc->source;
#else
	source = 0;
#endif
	if (p->flags & CODEX_ENCODE)
		vflags = VC_ENCODE;
	else if (p->flags & CODEX_DECODE)
		vflags = VC_DECODE;
	else
		vflags = 0;
	wmeth = 0;
	wsize = 0;
#if CODEX_VERSION >= 20130501L
	if ((s = p->disc->window) && vcodex_option(p, s, &wmeth, &wsize, &vflags, &source))
		return -1;
#endif
	i = 2;
	while (s = args[i++])
		if (vcodex_option(p, s, &wmeth, &wsize, &vflags, &source))
			return -1;
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		goto bad;
	}
	state->codex = p;
	p->data = state;
	state->flags = vflags;
	transform = args[1];
	if (streq(transform, "vcodex"))
		transform = 0;
	if (flags & CODEX_ENCODE)
	{	
		/* create handle for transformation */
		if (!transform || !(state->vc = vcmake(transform, VC_ENCODE)))
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: invalid encode transform", transform);
			goto bad;
		}

		/* create windowing handle if needed */
		if((state->vc->meth->type & VC_MTSOURCE) && source)
		{	if(!(state->vcwdc.srcf = sfopen(NiL, source, "rb")))
			{	VCSFERROR(state, "Non-existing or unreadable source file.");
				goto bad;
			}
			if(!wmeth)
				wmeth = Vcwprefix;
			state->vcw = vcwopen(&state->vcwdc, wmeth);
			if(!state->vcw)
			{	VCSFERROR(state, "Windowing not possible");
				goto bad;
			}
		}

		/* if no expicit window size then pick min nonzero window size from all coders */
		if(!wsize)
		{	wsize = VCSF_WSIZE;
			for(vc = state->vc; vc; vc = vc->coder)
				if(vc->meth->window && wsize > vc->meth->window)
					wsize = vc->meth->window;
		}

		/* buffer to accumulate data before encoding */
		state->dtsz = wsize;
		if(!(state->data = (Vcchar_t*)malloc(state->dtsz)) )
		{	VCSFERROR(state, "Out of memory for data buffer");
			goto bad;
		}
		state->next = state->data;
		state->endd = state->data + state->dtsz;

		/* buffer for the encoder to output results */
		state->bssz = VCSFDTSZ(state->dtsz);
		if(!(state->base = (Vcchar_t*)malloc(state->bssz)) )
		{	VCSFERROR(state, "Out of memory for output buffer");
			goto bad;
		}
	}
	else /* VC_DECODE */
	{	/* make output buffer */	
		if((state->bssz = wsize) <= 0)
			state->bssz = VCSF_BUFSIZE;
		else if(state->bssz < VCSF_BUFMIN)
			state->bssz = VCSF_BUFMIN;
		if(!(state->base = (Vcchar_t*)malloc(state->bssz)) )
		{	if(!(flags & CODEX_DECODE))
				state->flags = -1;
			VCSFERROR(state, "Out of memory for output buffer");
			goto bad;
		}
		state->code = state->endb = state->base;

		/* reconstruct handle to decode data */
		if(state->flags & VCSF_PLAIN)
		{	if(!transform)
				VCSFERROR(state, "No transform specified for decoding.");
			if(!(state->vc = vcmake(transform, VC_DECODE)) )
			{	VCSFERROR(state, "Ill-defined transformation for decoding.");
				goto bad;
			}
		}
		else
		{	int	encoded;
			if(!flags)
				state->flags |= VCSF_TRANS;
			if((encoded = getheader(state, p->sp, 1, !(flags & CODEX_DECODE), !(state->flags & VCSF_PLAIN), NiL)) <= 0 )
			{	if(!(flags & CODEX_DECODE) || !(state->flags & VCSF_PLAIN))
				{	state->flags = 0;
					return 0;
				}
				VCSFERROR(state, "Badly encoded data, decoding not possible.");
				goto bad;
			}
			if(!(flags & CODEX_DECODE))
			{	
				vcodex_done(p);
				return 0;
			}
		}

		/* construct window handle to get data for delta-decoding */
		if((state->vc->meth->type & VC_MTSOURCE) && source)
		{	if(!(state->vcwdc.srcf = sfopen(NiL, source, "rb")) )
			{	VCSFERROR(state, "Non-existing or unreadable source file.");
				goto bad;
			}
			if(!(state->vcw = vcwopen(&state->vcwdc, NIL(Vcwmethod_t*))) )
			{	VCSFERROR(state, "Windowing not possible");
				goto bad;
			}
		}
	}
	state->vc->errorf = p->disc->errorf;
	p->flags |= VCSF_OPEN;
	return 0;
 bad:
	vcodex_done(p);
	return -1;
}

static int
vcodex_init(Codex_t* p)
{
	State_t*	state = (State_t*)p->data;

	return 0;
}

static ssize_t
vcodex_read(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
{
	Vcchar_t	*dt, *text;
	int		ctrl;
	ssize_t		sz, r, d, m;
	Sfoff_t		pos;
	Vcwmatch_t	*wm;
	Vcio_t		io;
	State_t		*state = (State_t*)CODEX(disc)->data;

	if(!(state->flags&VC_DECODE) )
		return VCSFERROR(state, "decode handle create failed");

	for(sz = 0, dt = (Vcchar_t*)buf; sz < n; sz += r, dt += r)
	{	/* copy already decoded data */
		if((r = state->endd - state->next) > 0 )
		{	r = r > (n-sz) ? (n-sz) : r;
			memcpy(dt, state->next, r);
			state->next += r;
		}
		else /* need to decode a new batch of data */
		{
			if((d = (state->endb - state->code)) < 2*sizeof(size_t))
			{	if(fillbuf(state, f, disc) <= 0 )
					break;
				d = state->endb - state->code;
			}

			vcioinit(&io, state->code, d);

			state->vcdc.data = NIL(Void_t*);
			state->vcdc.size = 0;

			/* get the control byte */
			ctrl = vciogetc(&io);

			/* let upper level write and flush first */
			if((ctrl&VCD_TARGETFILE) && sz > 0)
				break;

			if(ctrl & (VCD_SOURCEFILE|VCD_TARGETFILE) )
			{	if(!state->vcw ||
				   (d = (ssize_t)vciogetu(&io)) < 0 ||
				   (pos = (Sfoff_t)vciogetu(&io)) < 0 ||
				   !(wm = vcwapply(state->vcw, TYPECAST(Void_t*, ctrl), d, pos)) )
				{	VCSFERROR(state, "source window read failed during decode");
					BREAK;
				}
				state->vcdc.data = wm->wdata;
				state->vcdc.size = wm->wsize;
			}
			else if(ctrl == VC_EOF) /* a new decoding context */
			{	state->code = vcionext(&io);
				if(vciomore(&io) > 0 && *state->code == VC_EOF)
					continue; /* skip a sequence of VC_EOF's */
				if(getheader(state, f, 0, 0, 0, disc) < 0 )
					break;
				else	continue;
			}
			else if(ctrl != 0 && ctrl != VC_RAW)
			{	VCSFERROR(state, "data stream corrupt");
				BREAK;
			}

			if(state->vcw)
				vcdisc(state->vc, &state->vcdc);

			/* size of coded data */
			if(vciomore(&io) == 1 && vciopeek(&io) == 0200)
			{	vciogetc(&io);
				d = 0;
				ctrl = VC_RAW;
			}
			else if((d = vciogetu(&io)) <= 0)
			{	VCSFERROR(state, "coded data size read failed");
				BREAK;
			}

			/* make sure all the data is available */
			if((vcionext(&io) + d) > state->endb)
			{	state->code = vcionext(&io);
				if((m = d+VCSF_SLACK) > state->bssz &&
				   makebuf(state, m) < 0 )
					return VCSFERROR(state, "decode buffer allocation error");
				if(fillbuf(state, f, disc) < 0)
					return VCSFERROR(state, "coded data read error");
				if ((m = state->endb - state->code) < d )
					return VCSFERROR(state, "coded data truncated");
				vcioinit(&io, state->code, m);
			}

			/* decode data */
			state->code = vcionext(&io);
			if(ctrl == VC_RAW)
			{	text = state->code;
				m = d;
			}
			else
			{	vcbuffer(state->vc, NIL(Vcchar_t*), -1, -1);
				if((m = vcapply(state->vc, state->code, d, &text)) <= 0)
				{	VCSFERROR(state, "decode failed");
					BREAK;
				}
			}

			state->code += d; /* advance passed processed data */

			/* set plaintext data buffer */
			state->data = state->next = text;
			state->endd = text+m;
			state->dtsz = m;
		}
	}

	return sz;
}

static ssize_t
vcodex_write(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
{
	Vcchar_t	*dt;
	ssize_t		sz, w;
	State_t		*state = (State_t*)CODEX(disc)->data;

	if(!(state->flags & VC_ENCODE) )
		return VCSFERROR(state, "encode handle create failed");

	for(sz = 0, dt = (Vcchar_t*)buf; sz < n; sz += w, dt += w)
	{	if(buf == (Void_t*)state->data)
		{	/* final flush */
			w = state->next - state->data;
			state->next = state->data;

			if((w = encode(state, state->data, w)) < 0 )
			{	VCSFERROR(state, "data encode failed");
				break;
			}
			else	sz += w;

			if(sfwr(f, state->code, state->cdsz, disc) != state->cdsz)
			{	VCSFERROR(state, "encoded data write failed");
				break;
			}

			if(state->next > state->data) /* not done yet */
			{	w = 0; /* so for(;;) won't add to sz, dt */
				continue; /* back to flushing */
			}
			else	break;
		}

		if((w = state->endd - state->next) == 0)
		{	/* flush a full buffer */
			state->next = state->data;
			if((w = encode(state, state->data, state->dtsz)) < 0)
			{	VCSFERROR(state, "data encode failed");
				break;
			}
			if(sfwr(f, state->code, state->cdsz, disc) != state->cdsz)
			{	VCSFERROR(state, "encoded data write failed");
				break;
			}

			w = state->endd - state->next; /* bufferable space */
		}

		/* process data directly if buffer is empty and data is large */
		if(w == state->dtsz && (n-sz) >= w)
		{	if((w = encode(state, dt, w)) < 0)
			{	VCSFERROR(state, "encode failed");
				break;
			}
			if(sfwr(f, state->code, state->cdsz, disc) != state->cdsz)
			{	VCSFERROR(state, "write failed");
				break;
			}
		}
		else /* accumulating data into buffer */
		{	w = w > (n-sz) ? (n-sz) : w;
			memcpy(state->next, dt, w);
			state->next += w;
		}
	}

	return sz;
}

static int
vcodex_sync(Codex_t* p)
{
	ssize_t		sz;
	State_t*	state = (State_t*)p->data;

	return (state->flags & VC_ENCODE) && (sz = state->next - state->data) > 0 && vcodex_write(p->sp, state->data, sz, &p->sfdisc) < 0 ? -1 : 0;
}

Codexmeth_t	codex_vcodex =
{
	"vcodex",
	"vcodex compression.",
	0,
	CODEX_DECODE|CODEX_ENCODE|CODEX_COMPRESS|CODEX_VCODEX,
	0,
	vcodex_ident,
	vcodex_open,
	0,
	vcodex_init,
	vcodex_done,
	vcodex_read,
	vcodex_write,
	vcodex_sync,
	0,
	0,
	0,
	0,
	CODEXNEXT(vcodex)
};

CODEXLIB(vcodex)
