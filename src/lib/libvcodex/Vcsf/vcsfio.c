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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"vchdr.h"

#include	<vcsfio.h>

/*	Sfio discipline to run Vcodex methods for compression.
**
**	Written by Kiem-Phong Vo
*/

#define VCSF_BUFSIZE	(256*1024)	/* decoding buffer	*/
#define VCSF_BUFMIN	(64*1024)	/* surely affordable!	*/
#define VCSF_WSIZE	(4*1024*1024)	/* dflt encoding window	*/
#define VCSF_SLACK	(16*sizeof(size_t)) /* for extra coding	*/
#define VCSFDTSZ(sz)	((((sz) / 1024) + 2) * 1024)

#define VCSFERROR(dc,m)	((dc)->sfdt->errorf ? ((*(dc)->sfdt->errorf)(m), -1) : -1 )

/* these bits are in sfdc->flags and must be outside VC_FLAGS	*/
#define VCSF_DONEHEAD	01000	/* header was already output	*/
#define VCSF_KEEPSFDC	02000	/* do not free the Sfdc struct	*/

typedef struct _sfdc_s
{	Sfdisc_t	disc;	/* Sfio discipline must be 1st	*/
	Sfio_t*		sf;	/* stream to do IO with		*/

	Vcsfdata_t*	sfdt;	/* initialization parameters	*/

	unsigned int	flags;	/* states of the handle		*/

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
} Sfdc_t;

#ifndef SKIPVCDX /* deal with the ancient VCDX header that uses method numbers */

#ifndef elementsof
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#endif

typedef struct _map_s /* to map an old method number to a transform */
{	const char*	name;	/* new method name	*/
	int		mtid;	/* old method number	*/
	Vcmethod_t*	meth;	/* corresponding method	*/
} Map_t;

typedef struct _meth_s /* keep method data for reverse construction */
{	Vcmethod_t*	meth;
	ssize_t		size;
	Vcchar_t*	data;
} Meth_t;

static Map_t		Map[] =
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

#if __STD_C
static ssize_t hdrvcdx(Vcchar_t** datap, ssize_t dtsz, Vcchar_t* code, ssize_t cdsz)
#else
static ssize_t hdrvcdx(datap, dtsz, code, cdsz)
Vcchar_t**	datap;	/* old header data	*/
ssize_t		dtsz;
Vcchar_t*	code;	/* space for new code	*/
ssize_t		cdsz;
#endif
{
	ssize_t		k, n, id, sz;
	Vcchar_t	*dt, buf[1024];
	Vcio_t		io;
	Meth_t		meth[2*elementsof(Map)]; /* should not exceed this */

	/* parse old header into list of transforms */
	vcioinit(&io, *datap, dtsz);
	for(n = 0; n < elementsof(meth) && vciomore(&io) > 0; ++n)
	{	id = vciogetc(&io);
		for(k = 0; k < elementsof(Map); ++k)
			if(Map[k].mtid == id)
				break;
		if(k >= elementsof(Map)) /* not matching any transform */
			return dtsz; /* must be new header format */

		if(!Map[k].meth && !(Map[k].meth = vcgetmeth((char*)Map[k].name, 1)))
		{	Map[k].mtid = -1;
			return dtsz;
		}
		meth[n].meth = Map[k].meth;

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
#if __STD_C
static ssize_t hdrvcdiff(Vcio_t* iodt, int indi, Vcchar_t* code, ssize_t cdsz)
#else
static ssize_t hdrvcdiff(iodt, indi, code, cdsz)
Vcio_t*		iodt;	/* data to be decoded	*/
int		indi;	/* indicator byte	*/
Vcchar_t*	code;	/* code buffer		*/
ssize_t		cdsz;	/* size of code buffer	*/
#endif
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

#if __STD_C
static int putheader(Sfdc_t* sfdc)
#else
static int putheader(sfdc)
Sfdc_t*		sfdc;
#endif
{
	Vcchar_t	*code;
	ssize_t		sz;

	/* header should be done at most once */
	if((sfdc->flags&VCSF_DONEHEAD) )
		return 0;
	sfdc->flags |= VCSF_DONEHEAD;

	/* header not wanted */
	if(sfdc->sfdt->type&VCSF_PLAIN)
		return 0;

	/* get the string that codes the methods */
	if((sz = vcextract(sfdc->vc, (Void_t**)&code)) <= 0 )
		return VCSFERROR(sfdc, "Transform data not encodable.");
	if((4+1+vcsizeu(sz)+sz) > vciomore(sfdc->io))
		return VCSFERROR(sfdc, "Transform data abnormally large.");

	/* output standard header bytes */
	vcioputc(sfdc->io, VC_HEADER0);
	vcioputc(sfdc->io, VC_HEADER1);
	vcioputc(sfdc->io, VC_HEADER2);
	vcioputc(sfdc->io, (sfdc->sfdt->type&VCSF_VCDIFF) ? 0 : VC_HEADER3);

	/* output indicator byte - see also compatibility with RFC3284 */
	vcioputc(sfdc->io, 0);

	/* output data encoding the methods used */
	if(!(sfdc->sfdt->type&VCSF_VCDIFF) )
	{	vcioputu(sfdc->io, sz);
		vcioputs(sfdc->io, code, sz);
	}

	return 0;
}

#if __STD_C
static int makebuf(Sfdc_t* sfdc, ssize_t size)
#else
static int makebuf(sfdc, size)
Sfdc_t*	sfdc;
ssize_t	size;
#endif
{
	Vcchar_t	*base, *oldbase;

	size = ((size + VCSF_BUFMIN - 1)/VCSF_BUFMIN)*VCSF_BUFMIN;
	if(sfdc->bssz >= size)
		return 0;

	oldbase = sfdc->base;
	if(!(base = (Vcchar_t*)malloc(size)) )
		return -1;

	memcpy(base, oldbase, sfdc->endb - oldbase);
	sfdc->endb = base + (sfdc->endb - oldbase);
	sfdc->code = base + (sfdc->code - oldbase);
	sfdc->base = base;
	sfdc->bssz = size;
	free(oldbase);

	return 0;
}

#if __STD_C
static ssize_t fillbuf(Sfdc_t* sfdc, Sfio_t* f, Sfdisc_t* disc)
#else
static ssize_t fillbuf(sfdc, f, disc)
Sfdc_t*		sfdc;
Sfio_t*		f;
Sfdisc_t*	disc;
#endif
{
	ssize_t	sz, n;

	if((sz = sfdc->endb - sfdc->code) <= 0 )
	{	sfdc->endb = sfdc->code = sfdc->base;
		sz = 0;
	}
	else if(sfdc->code > sfdc->base)
	{	memcpy(sfdc->base, sfdc->code, sz);
		sfdc->endb = (sfdc->code = sfdc->base) + sz;
	}
	for(; sz < sfdc->bssz; sz += n, sfdc->endb += n)
	{	if(!disc) /* plain read if no discipline set yet */
			n = sfread(f, sfdc->endb, sfdc->bssz-sz);
		else	n = sfrd(f, sfdc->endb, sfdc->bssz-sz, disc);
		if(n <= 0)
			break;
	}

	return sz;
}

static int
#if _STD_C
extend(char** pc, char** pb, char** pe, int n)
#else
extend(pc, pb, pe, n)
char**	pc;
char**	pb;
char**	pe;
int	n;
#endif
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

#if __STD_C
static ssize_t ident(Sfdc_t* sfdc, Void_t* data, size_t dtsz)
#else
static ssize_t ident(sfdc, data, dtsz)
Sfdc_t*		sfdc;
Void_t*		data;
size_t		dtsz;
#endif
{
	ssize_t		sz, k;
	char		*mt;
	char		*id, *buf, *end;
	unsigned char	*dt;
	int		x;
	Vcio_t		io;

	static char	hex[] = "0123456789ABCDEF";

	if(!data || dtsz <= 0 )
		return -1;
	x = 256;
	if(!(buf = malloc(x)))
		return -1;
	id = buf;
	end = buf + x - 1;
	vcioinit(&io, data, dtsz);
	id = buf;
	while(vciomore(&io) > 0)
	{	
		if(id > buf)
		{	if(id >= end && extend(&id, &buf, &end, 1))
				return -1;
			*id++ = VC_METHSEP;
		}
		mt = (char*)vcionext(&io);
		for(sz = vciomore(&io), k = 0; k < sz; ++k)
		{	if(id >= end && extend(&id, &buf, &end, 1))
				return -1;
			if(mt[k] == 0)
				break;
			*id++ = mt[k];
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
			if((id +k) >= end && extend(&id, &buf, &end, k))
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
	sfdc->sfdt->trans = buf;
	return 1;
}

#if __STD_C
static ssize_t getheader(Sfdc_t* sfdc, Sfio_t* f, int init, int identify, int optional)
#else
static ssize_t getheader(sfdc, f, init, identify, optional)
Sfdc_t*		sfdc;
Sfio_t*		f;
int		init;	/* initial call */
int		identify;
int		optional;
#endif
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
		if(loop > 0 && (sfdc->endb - sfdc->base) >= sfdc->bssz &&
		   makebuf(sfdc, sfdc->bssz+VCSF_BUFMIN) < 0)
			return VCSFERROR(sfdc, "Failure in allocating memory");

		/* read header data as necessary */
		sz = sfdc->endb - sfdc->code;
		if(loop > 0 || sz <= 0)
		{	if(fillbuf(sfdc, f, sfdc->vc ? &sfdc->disc : NIL(Sfdisc_t*)) <= 0 ||
			   (sz = sfdc->endb - sfdc->code) <= 0 )
				return identify ? 0 : init ? VCSFERROR(sfdc, "Bad header data") : -1;
		}

		vcioinit(&io, sfdc->code, sz);

		if(vciomore(&io) < 5) /* need 4-byte header + indicator byte */
			continue;

		if(vciogetc(&io) != VC_HEADER0 ||
		   vciogetc(&io) != VC_HEADER1 ||
		   vciogetc(&io) != VC_HEADER2 ||
		   ((head = vciogetc(&io)) != 0 /* RFC3284 Vcdiff header */ &&
		    head != VC_HEADER3 /* normal Vcodex header */ ) )
			return identify ? 0 : VCSFERROR(sfdc, "Unknown header data");

		if((indi = vciogetc(&io)) & VC_EXTRAHEADER )
		{	if((sz = vciogetu(&io)) < 0) /* skip app-specific data */
				continue;
			vcioskip(&io, sz);
		}

		if(head == 0) /* RFC3284 Vcdiff format */
		{	if((cdsz = hdrvcdiff(&io, indi, cdbuf, sizeof(cdbuf))) < 0)
				return VCSFERROR(sfdc, "Corrupted Vcdiff header data");
			else if(cdsz == 0)
				continue;
			else	code = cdbuf;
		}
		else /* Vcodex encoding */
		{	if((cdsz = vciogetu(&io)) < 0 )
				continue;
			else if(cdsz == 0)
				return VCSFERROR(sfdc, "Corrupted Vcodex header data");
			if(vciomore(&io) < cdsz)
				continue;
			code = vcionext(&io);
			vcioskip(&io, cdsz);

#ifndef SKIPVCDX /* deal with old headers that use method numbers instead of names */
			cdsz = hdrvcdx(&code, cdsz, cdbuf, sizeof(cdbuf));
#endif
		}

		/* successfully read the header data */
		sfdc->code = vcionext(&io);
		break;
	}
	if(cdsz <= 0 )
		return identify ? 0 : VCSFERROR(sfdc, "Failure in obtaining header data");
	else if(identify > 0)
		return ident(sfdc, code, cdsz);
	else
	{	if(sfdc->vc)
			vcclose(sfdc->vc);
		if(!(sfdc->vc = vcrestore(code, cdsz)) )
			return VCSFERROR(sfdc, "Failure in initializing data transforms");
		else if(sfdc->sfdt->type & VCSF_TRANS)
			return ident(sfdc, code, cdsz);
		else	return 1;
	}
}

#if __STD_C
static ssize_t encode(Sfdc_t* sfdc, Vcchar_t* data, size_t dtsz)
#else
static ssize_t encode(sfdc, data, dtsz)
Sfdc_t*		sfdc;
Vcchar_t*	data;
size_t		dtsz;
#endif
{
	Vcchar_t	*code, ctrl, *dt;
	ssize_t		cdsz, size, sz, bssz, dosz;
	Vcchar_t	*base;
	Vcwmatch_t	*wm;
	Vcio_t		io;

	vcioinit(&io, sfdc->base, sfdc->bssz);
	sfdc->io = &io;

	if(!(sfdc->flags&VCSF_DONEHEAD) && putheader(sfdc) < 0)
		return VCSFERROR(sfdc, "Failure in encoding transforms");

	sfdc->code = NIL(Vcchar_t*); sfdc->cdsz = 0;

	for(size = 0, dosz = dtsz, dt = data; size < dtsz; )
	{	/* control data */
		ctrl = 0;
		sfdc->vcdc.data = NIL(Void_t*);
		sfdc->vcdc.size = 0;

		/* compute a matching window to enhance compression */
		wm = NIL(Vcwmatch_t*);
		if(sfdc->vcw)
			wm = vcwapply(sfdc->vcw, dt, dosz, sfdc->pos);
		if(wm)
		{	/**/DEBUG_ASSERT(wm->msize <= dosz);
			if(wm->wsize > 0 && wm->wpos >= 0)
			{	sfdc->vcdc.data = wm->wdata;
				sfdc->vcdc.size = wm->wsize;
			}

			ctrl = wm->type & (VCD_SOURCEFILE|VCD_TARGETFILE);
			dosz = wm->msize; /* amount doable now */
			/**/ DEBUG_PRINT(2,"dtpos=%d ", sfdc->pos);
			/**/ DEBUG_PRINT(2,"dtsz=%d ", dosz);
			/**/ DEBUG_PRINT(2,"wpos=%d ", wm->wpos);
			/**/ DEBUG_PRINT(2,"wsz=%d ", wm->wsize);
			/**/ DEBUG_PRINT(2,"mtsz=%d\n", wm->msize);
		}
		if(sfdc->vcw) /* set window data */
			vcdisc(sfdc->vc, &sfdc->vcdc);

		vcbuffer(sfdc->vc, NIL(Vcchar_t*), -1, -1); /* free buffers */
		if((cdsz = vcapply(sfdc->vc, dt, dosz, &code)) <= 0 ||
		   (sz = vcundone(sfdc->vc)) >= dosz )
		{	if(cdsz < 0)
				VCSFERROR(sfdc, "Error in transforming data");
			ctrl = VC_RAW; /* coder failed, output raw data */
			code = dt;
			cdsz = dosz;
		}
		else
		{	dosz -= (sz > 0 ? sz : 0); /* true processed amount */
			if(sfdc->vcw) /* tell window matcher compressed result */
				vcwfeedback(sfdc->vcw, cdsz);
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
			memcpy(base, sfdc->base, vciosize(&io));
			if(sfdc->base)
				free(sfdc->base);
			sfdc->base = base;
			sfdc->bssz = bssz;

			vcioinit(&io, sfdc->base, sfdc->bssz);
			vcioskip(&io, sz);
		}
		vcioputs(&io, code, cdsz);

		sfdc->pos += dosz; /* advance by amount consumed */
		dt += dosz;
		size += dosz;

		if((dosz = dtsz-size) > 0 )
		{	if(wm && wm->more) /* more subwindows to do */
				continue;
			else /* need fresh data */
			{	if(data == sfdc->data) /* shift undone data */
				{	memcpy(data, data+size, dosz);
					sfdc->next = data + dosz;
				}
				break;
			}
		}
	}

	sfdc->code = sfdc->base; sfdc->cdsz = vciosize(&io);

	return size; /* return amount processed */
}

/* read data from the filter */
#if __STD_C
static ssize_t vcsfdcread(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
static ssize_t vcsfdcread(f, buf, n, disc)
Sfio_t*		f;	/* stream reading from		*/
Void_t*		buf;	/* buffer to read data into	*/
size_t		n;	/* number of bytes requested	*/
Sfdisc_t*	disc;	/* discipline structure		*/
#endif
{
	Vcchar_t	*dt, *text;
	int		ctrl;
	ssize_t		sz, r, d, m;
	Sfoff_t		pos;
	Vcwmatch_t	*wm;
	Vcio_t		io;
	Sfdc_t		*sfdc = (Sfdc_t*)disc;

	if(!(sfdc->flags&VC_DECODE) )
		return VCSFERROR(sfdc, "Handle not created for decoding data");

	for(sz = 0, dt = (Vcchar_t*)buf; sz < n; sz += r, dt += r)
	{	/* copy already decoded data */
		if((r = sfdc->endd - sfdc->next) > 0 )
		{	r = r > (n-sz) ? (n-sz) : r;
			memcpy(dt, sfdc->next, r);
			sfdc->next += r;
		}
		else /* need to decode a new batch of data */
		{
			if((d = (sfdc->endb - sfdc->code)) < 2*sizeof(size_t))
			{	if(fillbuf(sfdc, f, disc) <= 0 )
					break;
				d = sfdc->endb - sfdc->code;
			}

			vcioinit(&io, sfdc->code, d);

			sfdc->vcdc.data = NIL(Void_t*);
			sfdc->vcdc.size = 0;

			/* get the control byte */
			ctrl = vciogetc(&io);

			/* let upper level write and flush first */
			if((ctrl&VCD_TARGETFILE) && sz > 0)
				break;

			if(ctrl & (VCD_SOURCEFILE|VCD_TARGETFILE) )
			{	if(!sfdc->vcw ||
				   (d = (ssize_t)vciogetu(&io)) < 0 ||
				   (pos = (Sfoff_t)vciogetu(&io)) < 0 ||
				   !(wm = vcwapply(sfdc->vcw, TYPECAST(Void_t*, ctrl), d, pos)) )
				{	VCSFERROR(sfdc, "Error in obtaining source window data while decoding");
					BREAK;
				}
				sfdc->vcdc.data = wm->wdata;
				sfdc->vcdc.size = wm->wsize;
			}
			else if(ctrl == VC_EOF) /* a new decoding context */
			{	sfdc->code = vcionext(&io);
				if(vciomore(&io) > 0 && *sfdc->code == VC_EOF)
					continue; /* skip a sequence of VC_EOF's */
				if(getheader(sfdc, f, 0, 0, 0) < 0 )
					break;
				else	continue;
			}
			else if(ctrl != 0 && ctrl != VC_RAW && ctrl != VC_INDEX)
			{	VCSFERROR(sfdc, "Data stream appeared to be corrupted");
				BREAK;
			}

			if(sfdc->vcw)
				vcdisc(sfdc->vc, &sfdc->vcdc);

			/* size of coded data */
			if(vciomore(&io) == 1 && vciopeek(&io) == 0200)
			{	vciogetc(&io);
				d = 0;
				ctrl = VC_RAW;
			}
			else if((d = vciogetu(&io)) <= 0)
			{	VCSFERROR(sfdc, "Error in getting size of coded data");
				BREAK;
			}

			/* make sure all the data is available */
			if((vcionext(&io) + d) > sfdc->endb)
			{	sfdc->code = vcionext(&io);
				if((m = d+VCSF_SLACK) > sfdc->bssz &&
				   makebuf(sfdc, m) < 0 )
					return VCSFERROR(sfdc, "decode buffer allocation error");
				if(fillbuf(sfdc, f, disc) < 0)
					return VCSFERROR(sfdc, "coded data read error");
				if ((m = sfdc->endb - sfdc->code) < d )
					return VCSFERROR(sfdc, "coded data truncated");
				vcioinit(&io, sfdc->code, m);
			}

			/* decode data */
			sfdc->code = vcionext(&io);
			if(ctrl == VC_RAW)
			{	text = sfdc->code;
				m = d;
			}
			else if(ctrl == VC_INDEX)
			{	/* skip index data */
				sfdc->code += d;
				/* nothing placed in output -- loop again */
				continue;
			}
			else
			{	vcbuffer(sfdc->vc, NIL(Vcchar_t*), -1, -1);
				if((m = vcapply(sfdc->vc, sfdc->code, d, &text)) <= 0)
				{	VCSFERROR(sfdc, "Failure in decoding data");
					BREAK;
				}
			}

			sfdc->code += d; /* advance passed processed data */

			/* set plaintext data buffer */
			sfdc->data = sfdc->next = text;
			sfdc->endd = text+m;
			sfdc->dtsz = m;
		}
	}

	return sz;
}

#if __STD_C
static ssize_t vcsfdcwrite(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
static ssize_t vcsfdcwrite(f, buf, n, disc)
Sfio_t*		f;	/* stream writing to		*/
Void_t*		buf;	/* buffer of data to write out	*/
size_t		n;	/* number of bytes requested	*/
Sfdisc_t*	disc;	/* discipline structure		*/
#endif
{
	Vcchar_t	*dt;
	ssize_t		sz, w;
	Sfdc_t		*sfdc = (Sfdc_t*)disc;

	if(!(sfdc->flags & VC_ENCODE) )
		return VCSFERROR(sfdc, "Handle was not created to encode data");

	for(sz = 0, dt = (Vcchar_t*)buf; sz < n; sz += w, dt += w)
	{	if(buf == (Void_t*)sfdc->data)
		{	/* final flush */
			w = sfdc->next - sfdc->data;
			sfdc->next = sfdc->data;

			if((w = encode(sfdc, sfdc->data, w)) < 0 )
			{	VCSFERROR(sfdc, "Error encoding data");
				break;
			}
			else	sz += w;

			if(sfwr(f, sfdc->code, sfdc->cdsz, disc) != sfdc->cdsz)
			{	VCSFERROR(sfdc, "Error writing encoded data");
				break;
			}

			if(sfdc->next > sfdc->data) /* not done yet */
			{	w = 0; /* so for(;;) won't add to sz, dt */
				continue; /* back to flushing */
			}
			else	break;
		}

		if((w = sfdc->endd - sfdc->next) == 0)
		{	/* flush a full buffer */
			sfdc->next = sfdc->data;
			if((w = encode(sfdc, sfdc->data, sfdc->dtsz)) < 0)
			{	VCSFERROR(sfdc, "Error in encoding data");
				break;
			}
			if(sfwr(f, sfdc->code, sfdc->cdsz, disc) != sfdc->cdsz)
			{	VCSFERROR(sfdc, "Error in writing encoded data");
				break;
			}

			w = sfdc->endd - sfdc->next; /* bufferable space */
		}

		/* process data directly if buffer is empty and data is large */
		if(w == sfdc->dtsz && (n-sz) >= w)
		{	if((w = encode(sfdc, dt, n-sz)) < 0)
			{	VCSFERROR(sfdc, "Error in encoding");
				break;
			}
			if(sfwr(f, sfdc->code, sfdc->cdsz, disc) != sfdc->cdsz)
			{	VCSFERROR(sfdc, "Error in writing data");
				break;
			}
		}
		else /* accumulating data into buffer */
		{	w = w > (n-sz) ? (n-sz) : w;
			memcpy(sfdc->next, dt, w);
			sfdc->next += w;
		}
	}

	return sz;
}

/* for the duration of this discipline, the stream is unseekable */
#if __STD_C
static Sfoff_t vcsfdcseek(Sfio_t* f, Sfoff_t addr, int offset, Sfdisc_t* disc)
#else
static Sfoff_t vcsfdcseek(f, addr, offset, disc)
Sfio_t*		f;
Sfoff_t		addr;
int		offset;
Sfdisc_t*	disc;
#endif
{	
	return (Sfoff_t)(-1);
}

/* on close, remove the discipline */
#if __STD_C
static int vcsfdcexcept(Sfio_t* f, int type, Void_t* data, Sfdisc_t* disc)
#else
static int vcsfdcexcept(f,type,data,disc)
Sfio_t*		f;
int		type;
Void_t*		data;
Sfdisc_t*	disc;
#endif
{
	ssize_t		sz;
	ssize_t		wz;
	Sfdc_t		*sfdc = (Sfdc_t*)disc;

	switch(type)
	{
	case VCSF_DISC: /* get the discipline */
		if(data)
			*((Sfdc_t**)data) = sfdc;
		return VCSF_DISC;
	case SF_SYNC:
	case SF_DPOP:
	case SF_CLOSING:
	case SF_ATEXIT:
		if(sfdc->flags & VC_ENCODE)
		{	if((sz = sfdc->next - sfdc->data) > 0 )
			{	
#if _SFIO_H == 1		/* Sfio: this will wind up calling vcsfdcwrite() */
				sfset(f, SF_IOCHECK, 0);
				if((wz = sfwr(f, sfdc->data, sz, disc)) < sz)
				{
					error(1, "AHA#%d sz=%I*d wz=%I*d", __LINE__, sizeof(sz), sz, sizeof(wz), wz);
					return VCSFERROR(sfdc, "Error in writing coded data");
				}
				sfset(f, SF_IOCHECK, 1);

#else				/* Stdio: must call vcsfdcwrite() directly to encode */
				if(vcsfdcwrite(f, sfdc->data, sz, disc) != sz)
					return VCSFERROR(sfdc, "Error in writing coded data");
#endif
				sfdc->next = sfdc->data;
			}

			if(type == SF_CLOSING || type == SF_ATEXIT)
			{	Vcio_t		io;

				/* back to plain text mode */
				sfdc->flags |= VCSF_KEEPSFDC;
				sfdisc(f, NIL(Sfdisc_t*));
				sfdc->flags &= ~VCSF_KEEPSFDC;

				vcioinit(&io, sfdc->base, sfdc->bssz);
				if(!(sfdc->flags&VCSF_DONEHEAD))
				{	sfdc->io = &io;
					if(putheader(sfdc) < 0 )
						return VCSFERROR(sfdc, "Error writing header");
					vcioputu(&io,0);
				}
				if(!(sfdc->sfdt->type&VCSF_PLAIN) )
					vcioputc(&io, VC_EOF); /* write the eof marker */

				sz = vciosize(&io); /* output to stream */
				if(sz > 0 && (wz = sfwr(f, sfdc->base, sz, NIL(Sfdisc_t*))) < sz)
				{
					error(1, "AHA#%d sz=%I*d wz=%I*d", __LINE__, sizeof(sz), sz, sizeof(wz), wz);
					return VCSFERROR(sfdc, "Error in writing coded data");
				}
			}
		}

		if(!(sfdc->flags&VCSF_KEEPSFDC) && (type == SF_CLOSING || type == SF_DPOP) )
		{	if(sfdc->vc)
				vcclose(sfdc->vc);
			if(sfdc->vcwdc.srcf)
				sfclose(sfdc->vcwdc.srcf);
			if(sfdc->vcwdc.tarf)
				sfclose(sfdc->vcwdc.tarf);
			if(sfdc->vcw )
				vcwclose(sfdc->vcw);
			if((sfdc->flags&VC_ENCODE) && sfdc->data)
				free(sfdc->data);
			if(sfdc->base)
				free(sfdc->base);
			if(sfdc->sfdt->type&VCSF_FREE)
				free(sfdc->sfdt);
			free(sfdc);
		}

		break;
	}

	return 0;
}


/* syntax for window size is "[0-9]+[mMkK]" */
#if __STD_C
static ssize_t getwindow(char* spec, Vcwmethod_t** vcwmt)
#else
static ssize_t getwindow(spec, vcwmt)
char*		spec;	/* window/size specification	*/
Vcwmethod_t**	vcwmt;	/* return windowing method	*/
#endif
{
	ssize_t		wsize;
	Vcwmethod_t	*wmeth = NIL(Vcwmethod_t*);

	if(!spec)
		spec = "";

	wsize = (ssize_t)vcatoi(spec);
	while(isdigit(*spec))
		spec += 1;
	if(*spec == 'k' || *spec == 'K') /* kilobyte */
		wsize *= 1024;
	else if(*spec == 'm' || *spec == 'M') /* megabyte */
		wsize *= 1024*1024;

	if(vcwmt)
	{	while(*spec && *spec != VC_ARGSEP)
			spec += 1;
		if(*spec == VC_ARGSEP)
			wmeth = vcwgetmeth(spec+1);
		*vcwmt = wmeth ? wmeth : Vcwprefix;
	}

	return wsize;
}

#if __STD_C
Vcsfio_t* vcsfio(Sfio_t* sf, Vcsfdata_t* sfdt, int type)
#else
Vcsfio_t* vcsfio(sf, sfdt, type)
Sfio_t*		sf;	/* stream to be conditioned	*/
Vcsfdata_t*	sfdt;	/* data to initialize stream	*/
int		type;	/* VC_ENCODE or VC_DECODE or 0	*/
#endif
{
	char		*trans;
	ssize_t		wsize;
	Vcwmethod_t	*wmeth;
	Sfdc_t		*sfdc = NIL(Sfdc_t*);
	Vcsfdata_t	dflt; /* default decoding data	*/
	int		optional;

	if(type & VC_OPTIONAL)
	{	type &= ~VC_OPTIONAL;
		optional = 1;
	}
	else	optional = 0;
	if(!sfdt && type)
	{	sfdt = &dflt; /* assuming coded header data */
		memset(sfdt, 0, sizeof(Vcsfdata_t));
	}

	if(!sf || !sfdt )
		return NIL(Vcsfio_t*);
	if(type != VC_ENCODE && type != VC_DECODE && type)
		return NIL(Vcsfio_t*);

	if(sfdt->type & VCSF_VCDIFF) /* special case for RFC3284 header */
	{	sfdt->type &= ~VCSF_PLAIN;
		trans = "delta";
	}
	else	trans = sfdt->trans;

	/* local error processing function */
#define errorsfio(s)	do { if(sfdt->errorf) (*sfdt->errorf)(s); goto error; } while(0)

	if(!(sfdc = (Sfdc_t*)calloc(1,sizeof(Sfdc_t) + (sfdt == &dflt ? sizeof(dflt) : 0))) )
	{	if(!type)
			sfdt->type = -1;
		errorsfio("Out of memory for transformation structure");
	}

#if _SFIO_H == 1 /* initialize Sfio discipline */
	sfdc->disc.readf   = vcsfdcread;
	sfdc->disc.writef  = vcsfdcwrite;
	sfdc->disc.seekf   = vcsfdcseek;
	sfdc->disc.exceptf = vcsfdcexcept;
#endif

	sfdc->sf   = sf; /* stream to do IO on */
	if(sfdt == &dflt)
	{	sfdt = (Vcsfdata_t*)(sfdc + 1);
		*sfdt = dflt;
	}
	sfdc->sfdt = sfdt; /* init parameters */

	wsize = getwindow(sfdt->window, &wmeth);

	if((sfdc->flags = type) == VC_ENCODE)
	{	/* creat handle for transformation */
		if(!trans || !(sfdc->vc = vcmake(trans, VC_ENCODE)) )
			errorsfio("Ill-defined transformation for encoding.");

		/* create windowing handle if needed */
		if((sfdc->vc->meth->type & VC_MTSOURCE) && sfdt->source)
		{	if(!(sfdc->vcwdc.srcf = sfopen(0,sfdt->source,"rb")) )
				errorsfio("Non-existing or unreadable source file.");
			sfdc->vcw = vcwopen(&sfdc->vcwdc, wmeth);
			if(!sfdc->vcw)
				errorsfio("Windowing not possible");
		}

		/* buffer to accumulate data before encoding */
		sfdc->dtsz = wsize > 0 ? wsize :
			sfdc->vc->meth->window > 0 ? sfdc->vc->meth->window : VCSF_WSIZE;
		if(!(sfdc->data = (Vcchar_t*)malloc(sfdc->dtsz)) )
			errorsfio("Out of memory for data buffer");
		sfdc->next = sfdc->data;
		sfdc->endd = sfdc->data + sfdc->dtsz;

		/* buffer for the encoder to output results */
		sfdc->bssz = VCSFDTSZ(sfdc->dtsz);
		if(!(sfdc->base = (Vcchar_t*)malloc(sfdc->bssz)) )
			errorsfio("Out of memory for output buffer");
	}
	else /* VC_DECODE */
	{	/* make output buffer */	
		if((sfdc->bssz = wsize) <= 0)
			sfdc->bssz = VCSF_BUFSIZE;
		else if(sfdc->bssz < VCSF_BUFMIN)
			sfdc->bssz = VCSF_BUFMIN;
		if(!(sfdc->base = (Vcchar_t*)malloc(sfdc->bssz)) )
		{	if(!type)
				sfdt->type = -1;
			errorsfio("Out of memory for output buffer");
		}
		sfdc->code = sfdc->endb = sfdc->base;

		/* reconstruct handle to decode data */
		if(sfdt->type & VCSF_PLAIN)
		{	if(!trans)
				errorsfio("No transform	specified for decoding.");
			if(!(sfdc->vc = vcmake(trans, VC_DECODE)) )
				errorsfio("Ill-defined transformation for decoding.");
		}
		else
		{	int	encoded;
			if(!type)
				sfdt->type |= VCSF_TRANS;
			if((encoded = getheader(sfdc, sf, 1, !type, optional)) <= 0 )
			{	if(!type || optional)
				{	sfdt->type = 0;
					return NIL(Vcsfio_t*);
				}
				errorsfio("Badly encoded data, decoding not possible.");
			}
			if(!type)
			{	
				free(sfdc->base);
				free(sfdc);
				sfdt->type = 1;
				return (Vcsfio_t*)sf;
			}
		}

		/* construct window handle to get data for delta-decoding */
		if((sfdc->vc->meth->type & VC_MTSOURCE) && sfdt->source)
		{	if(!(sfdc->vcwdc.srcf = sfopen(0,sfdt->source,"rb")) )
				errorsfio("Non-existing or unreadable source file.");
			if(!(sfdc->vcw = vcwopen(&sfdc->vcwdc, NIL(Vcwmethod_t*))) )
				errorsfio("Windowing not possible");
		}
	}

	if(sfdisc(sf, &sfdc->disc) != &sfdc->disc)
		errorsfio("Stream not initializable");

#if _SFIO_H == 1
	sfset(sf, SF_IOCHECK, 1); /* so that sfsync() will call vcsfdcexcept() */
	sfclrerr(sf);
	return (Vcsfio_t*)sf;
#else
	return (Vcsfio_t*)sfdc;
#endif

error:	
	if(sfdc)
	{	if(sfdc->vc)
			vcclose(sfdc->vc);
		if(sfdc->vcwdc.srcf)
			sfclose(sfdc->vcwdc.srcf);
		if(sfdc->vcwdc.tarf)
			sfclose(sfdc->vcwdc.tarf);
		if(sfdc->vcw)
			vcwclose(sfdc->vcw);
		if((sfdc->flags&VC_ENCODE) && sfdc->data)
			free(sfdc->data);
		if(sfdc->base)
			free(sfdc->base);
		free(sfdc);
	}

	return NIL(Vcsfio_t*);
}

#if _SFIO_H != 1

typedef struct _rsrv_s	Rsrv_t;
struct _rsrv_s
{	Rsrv_t*	next;
	Sfio_t*	f;	/* file stream for I/O	*/
	Void_t*	data;	/* reserved data	*/
	ssize_t	dtsz;	/* amount of data	*/
	ssize_t	size;	/* allocated buf size	*/
};

#if __STD_C
static Rsrv_t* vcsfrsrv(Sfio_t* f, ssize_t n)
#else
static Rsrv_t* vcsfrsrv(f, n)
Sfio_t*		f;	/* stream to to create reserve buffer	*/
ssize_t		n;	/* <0: remove, 0: find, >0: buffer size	*/
#endif
{
	Rsrv_t		*p, *r;
	static Rsrv_t	*Rsrv;	/* linked list	*/

	for(p = NIL(Rsrv_t*), r = Rsrv; r; p = r, r = r->next)
		if(r->f == f)
			break;

	if(!r) /* create a new reserve structure if requested */
	{	if(n <= 0)
			return NIL(Rsrv_t*);
		if(!(r = (Rsrv_t*)calloc(1,sizeof(Rsrv_t))) )
			return NIL(Rsrv_t*);
		r->f = f;
	}
	else
	{	if(p) /* remove from list */
			p->next = r->next;
		else	Rsrv = r->next;

		if(n < 0) /* remove all together */
		{	if(r->data && r->size > 0)
				free(r->data);
			free(r);	
			return NIL(Rsrv_t*);
		}
	}

	if(n > r->size) /* allocate buffer as necessary */
	{	if(r->data)
			free(r->data);
		r->size = r->dtsz = 0;
		if(!(r->data = malloc(n)) )
		{	free(r);
			return NIL(Rsrv_t*);
		}
		else	r->size = n;
	}

	r->next = Rsrv; Rsrv = r;
	return r;
}

#if __STD_C
Void_t* sfreserve(Sfio_t* f, ssize_t n, int type)
#else
Void_t* sfreserve(f, n, type)
Sfio_t*		f;
ssize_t		n;
int		type;
#endif
{
	Rsrv_t	*r;
	Sfoff_t	here = 0;

	n = n < 0 ? -n : n; 

	if(type == SF_LASTR)
		return (!(r = vcsfrsrv(f,0)) || r->dtsz <= 0 ) ? NIL(Void_t*) : r->data;

	if(!(r = vcsfrsrv(f, n)) ) /* find/create reserve structure */
		return NIL(Void_t*);

	if(type == SF_LOCKR)
		if((here = sfseek(f, (Sfoff_t)0, 1)) < 0 )
			return NIL(Void_t*);

	if((r->dtsz = sfread(f, r->data, n)) > 0 )
		if(type == SF_LOCKR)
			sfseek(f, here, 0);

	return r->dtsz >= n ? r->data : NIL(Void_t*);
}

#if __STD_C
ssize_t sfvalue(Sfio_t* f)
#else
ssize_t sfvalue(f)
Sfio_t*		f;
#endif
{
	Rsrv_t	*r;
	return (r = vcsfrsrv(f, 0)) ? r->dtsz : (Sfoff_t)(-1);
}

#if __STD_C
char* sfgetr(Sfio_t* f, int nl, int type)
#else
char* sfgetr(f, nl, type)
Sfio_t* 	f;
int		nl;
int		type;
#endif
{
	Rsrv_t	*r;

	if(!(r = vcsfrsrv(f, 1024)) )
		return NIL(char*);
	if(!fgets(r->data, 1024, f) )
		return NIL(char*);
	if(type > 0)
	{	nl = strlen(r->data);
		((char*)r->data)[nl-1] = 0;
	}
	return (char*)r->data;
}

#if __STD_C
int sfclose(Sfio_t* f)
#else
int sfclose(f)
Sfio_t*		f;
#endif
{	vcsfrsrv(f, -1);
	fclose(f);
	return 0;
}

#if __STD_C
Sfoff_t sfsize(Sfio_t* f)
#else
Sfoff_t sfsize(f)
Sfio_t*		f;
#endif
{
	Sfoff_t	pos, siz;
	if(fseek(f, (long)0, 1) < 0 )
		return -1;
	pos = (Sfoff_t)ftell(f);
	fseek(f, (long)0, 2);
	siz = (Sfoff_t)ftell(f);
	fseek(f, (long)pos, 0);
	return siz;
}

#if __STD_C
ssize_t vcsfread(Vcsfio_t* vcf, Void_t* buf, size_t n)
#else
ssize_t vcsfread(vcf, buf, n)
Vcsfio_t*	vcf;
Void_t*		buf;
size_t		n;
#endif
{
	return vcsfdcread(((Sfdc_t*)vcf)->sf, buf, n, (Sfdisc_t*)vcf);
}

#if __STD_C
ssize_t vcsfwrite(Vcsfio_t* vcf, const Void_t* buf, size_t n)
#else
ssize_t vcsfwrite(vcf, buf, n)
Vcsfio_t*	vcf;
Void_t*		buf;
size_t		n;
#endif
{
	return vcsfdcwrite(((Sfdc_t*)vcf)->sf, buf, n, (Sfdisc_t*)vcf);
}

#if __STD_C
int vcsfsync(Vcsfio_t* vcf)
#else
int vcsfsync(vcf)
Vcsfio_t*	vcf;
#endif
{
	return vcsfdcexcept(((Sfdc_t*)vcf)->sf, SF_SYNC, 0, (Sfdisc_t*)vcf);
}

#if __STD_C
int vcsfclose(Vcsfio_t* vcf)
#else
int vcsfclose(vcf)
Vcsfio_t*	vcf;
#endif
{
	Sfio_t	*sf = ((Sfdc_t*)vcf)->sf;

	if(vcsfdcexcept(sf, SF_CLOSING, 0, (Sfdisc_t*)vcf) < 0)
		return -1;
	if(sfclose(sf) != 0)
		return -1;
	return 0;
}

#endif /*!SFIO_H*/
