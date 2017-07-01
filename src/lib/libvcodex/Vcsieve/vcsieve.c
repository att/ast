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

/*	Delta compression based on string matching
**
**	Written by Kiem-Phong Vo
*/

/* codes to tell which data section was secondarily compressed */
#define VCS_UNTRCMP	0001	/* un-diffed data compressed	*/
#define VCS_DIFFCMP	0002	/* diffed data compressed	*/
#define VCS_INSTCMP	0004	/* instructions compressed	*/
#define VCS_UNSZCMP	0010	/* un-diffed dtsz compressed	*/
#define VCS_DFSZCMP	0020	/* diffed dtsz compressed	*/
#define VCS_ADDRCMP	0040	/* address space compressed	*/

/* Instruction codes */
#define VCS_ISIZE	(1<<3)	/* size of instruction space	*/
#define VCS_UNTR	0	/* untransformed data		*/
#define VCS_HERE	1
#define VCS_SELF	2
#define VCS_KBEGIN	3	/* start of cached addresses	*/
#define VCS_KSIZE	(VCS_ISIZE - VCS_KBEGIN)

/* get instruction, cache initialization */
#define VCSIGET(i)	(((i) & (VCS_ISIZE-1))) /* VCS_ISIZE must be a power-of-2 */
#define VCSKINIT(si)	(memset((si)->cache, 0, sizeof((si)->cache)), (si)->cpos = 0 )

#define VCS_REVERSE	(1<<3) 	/* reverse matching was done	*/
#define VCS_MAP		(1<<4)	/* data was mapped for matching	*/
#define VCS_DIFF	(1<<5)	/* differencing approx matches	*/
#define VCS_MMIN	(1<<14)	/* setting min matching size	*/

#define VCS_MATCH	19	/* min size of exact matches	*/

#define ENCODE(z,m)	((z) - (m)) /* code z given z >= m	*/
#define DECODE(z,m)	((z) + (m)) /* decode z given above	*/

typedef struct _sieve_s
{	Vclzparse_t	vcpa;	/* data to pass to parser	*/
	ssize_t		cache[VCS_KSIZE]; /* cached addresses	*/
	ssize_t		cpos;	/* current cache replacement	*/
	Vcio_t*		untr;	/* to write untransformed data	*/
	Vcio_t*		diff;	/* to write transformed data	*/
	Vcio_t*		inst;	/* to write control bytes	*/
	Vcio_t*		unsz;	/* to write sizes of unmatched	*/
	Vcio_t*		dfsz;	/* to write sizes of matched	*/
	Vcio_t*		addr;	/* to write match addresses	*/
	Vcodex_t*	vch;	/* Huffman coder		*/
	ssize_t		mmin;	/* min size of any exact match	*/
	int		type;	/* REVERSE|MAP|DIFF		*/
	Vcchar_t*	cmap;	/* byte map, if any		*/
} Sieve_t;

static Vcmtarg_t	_Siargs[] =
{	{ "delta", "Delta compression with approximate matching", (Void_t*)VCS_DIFF },
	{ "reverse", "Allowing reverse matching", (Void_t*)VCS_REVERSE },
	{ "map", "Specifying byte pairs to map for matching", (Void_t*)VCS_MAP },
	{ "mmin", "Setting amount of exact match before approximate matching", (Void_t*)VCS_MMIN },
	{ NIL(char*), "Delta compression with exact matching", NIL(Void_t*) }
};

/* code and decode addresses of matches */
#if __STD_C
static ssize_t address(Sieve_t* si, ssize_t addr, ssize_t size, ssize_t here, int* inst)
#else
static ssize_t address(si, addr, size, here, inst)
Sieve_t*	si;
ssize_t		addr;	/* address to code or decode	*/
ssize_t		size;	/* size of matched segment	*/
ssize_t		here;	/* current target data position	*/
int*		inst;	/* instruction type		*/
#endif
{
	ssize_t	ad, a, k;
	ssize_t	*cache = si->cache;

	if(*inst < 0) /* encoding */
	{	*inst = VCS_HERE; ad = here - addr;
		if(addr < ad)
			{ *inst = VCS_SELF; ad = addr; }
		for(k = 0; k < VCS_KSIZE; ++k)
			if((a = vcintcode(addr, cache[k], 0, here, VC_ENCODE)) < ad )
				{ *inst = k + VCS_KBEGIN; ad = a; }
		a = addr;
	}
	else
	{	if((k = VCSIGET(*inst)) == VCS_HERE )
			ad = here - addr;
		else if(k == VCS_SELF)
			ad = addr;
		else	ad = vcintcode(addr, cache[k - VCS_KBEGIN], 0, here, VC_DECODE);
		a = ad;
	}

	cache[si->cpos] = (a += size) < 0 ? 0 : a;
	if((si->cpos += 1) >= VCS_KSIZE)
		si->cpos = 0;

	return ad;
}

#if __STD_C
static ssize_t putinst(Vclzparse_t* vcpa, int type, Vclzmatch_t* mtch, ssize_t n)
#else
static ssize_t putinst(vcpa, type, mtch, n)
Vclzparse_t*	vcpa;
int		type;	/* type of instruction		*/
Vclzmatch_t*	mtch;	/* list of matched fragments	*/
ssize_t		n;	/* number of fragments		*/
#endif
{
	ssize_t		size, z, sz, addr;
	Vcchar_t	*dt, *endd, *ts, *ms;
	int		md, ty;
	Vcchar_t	*cmap = (vcpa->type&type&VCLZ_MAP) ? vcpa->cmap : NIL(Vcchar_t*);
	Sieve_t		*si = (Sieve_t*)vcpa;

	if(n <= 0) /* nothing to do */
		return 0;

	size = 0;

	if(mtch->mpos < 0) /* output any initial unmatched data */
	{	if((z = mtch->size) <= 0 )
			RETURN(-1);
		size += z;

		vcioputc(si->inst, VCS_UNTR);
		vcioputu(si->unsz, ENCODE(z, 1));

		ts = si->vcpa.tar + (mtch->tpos - si->vcpa.nsrc);
		for(endd = (dt = vcionext(si->untr)) + z; dt < endd; ++dt, ++ts )
			*dt = *ts;
		vcioskip(si->untr, z);

		/**/DEBUG_PRINT(9, "untr %6d  ", z);
		/**/DEBUG_PRINT(9, "tpos %6d\n", mtch->tpos - vcpa->nsrc);

		n -= 1; mtch += 1;
	}

	while(n > 0)
	{	if((si->type&VCS_DIFF) && n > 1)
			z = (mtch[n-1].tpos + mtch[n-1].size) - mtch->tpos; /* absorb all */
		else	z = mtch->size; /* coding each matched segment separately */
 		if(z < si->mmin)
			RETURN(-1); /* something went terribly wrong */
		size += z;

		/* code the matching address */	
		ty = -1; sz = (type&VCLZ_REVERSE) ? -z : z;
		if((addr = address(si, mtch->mpos, sz, mtch->tpos, &ty)) < 0 )
			RETURN(-1);

		ty |= ((type&VCLZ_MAP) ? VCS_MAP : 0) | ((type&VCLZ_REVERSE) ? VCS_REVERSE : 0);
		if((si->type&VCS_DIFF) && n > 1)
			ty |= VCS_DIFF;
		vcioputc(si->inst, ty);

		vcioputu(si->dfsz, ENCODE(z, si->mmin));
		vcioputu(si->addr, addr);

		/**/DEBUG_PRINT(9, "%s ", (ty&VCS_DIFF) ? "diff" : "copy");
		/**/DEBUG_PRINT(9, "%6d  ", z);
		/**/DEBUG_PRINT(9, "tpos %6d  ", mtch->tpos - vcpa->nsrc);
		/**/DEBUG_PRINT(9, "mpos %6d  ", mtch->mpos);
		/**/DEBUG_PRINT(9, "here %6d  ", mtch->tpos);
		/**/DEBUG_PRINT(9, "addr %6d", addr);
		/**/DEBUG_PRINT(9, "<%d>  ", VCSIGET(ty) );
		/**/DEBUG_PRINT(9, "type %s,", (ty&VCS_REVERSE)? "r" : "f");
		/**/DEBUG_PRINT(9, "%s  ", (ty&VCS_MAP)? "m" : "i");
		/**/DEBUG_PRINT(9, (ty&VCS_DIFF) ? "#frags %d\n" : "\n", n);

		if(ty&VCS_DIFF) /* transforming data by differencing */
		{	/* corresponding source and target data */
			if(mtch->mpos >= si->vcpa.nsrc)
				ms = si->vcpa.tar + (mtch->mpos - si->vcpa.nsrc);
			else	ms = si->vcpa.src + mtch->mpos;
			ts = si->vcpa.tar + (mtch->tpos - si->vcpa.nsrc);

			endd = (dt = vcionext(si->diff)) + z;
			for(md = (ty&VCS_REVERSE) ? -1 : 1; dt < endd; ++dt, ++ts, ms += md )
				*dt = *ms - (cmap ? cmap[*ts] : *ts);
			vcioskip(si->diff, z);

			n = 0;
		}
		else if(n > 1) /* output the unmatched data in the gap */
		{	if((z = mtch[1].tpos - (mtch[0].tpos + mtch[0].size)) <= 0 )
				RETURN(-1);
			size += z;

			vcioputc(si->inst, VCS_UNTR);
			vcioputu(si->unsz, ENCODE(z, 1));

			ts = si->vcpa.tar + (mtch[0].tpos + mtch[0].size) - si->vcpa.nsrc;
			for(endd = (dt = vcionext(si->untr)) + z; dt < endd; ++dt, ++ts )
				*dt = *ts;
			vcioskip(si->untr, z);

			/**/DEBUG_PRINT(9, "untr %6d  ", z);
			/**/DEBUG_PRINT(9, "tpos %6d\n", mtch->tpos + mtch->size - vcpa->nsrc);
		}

		n -= 1; mtch += 1;
	}

	return size;
}

#if __STD_C
static ssize_t sieve(Vcodex_t* vc, const Void_t* tar, size_t ntar, Void_t** del)
#else
static ssize_t sieve(vc, tar, ntar, del)
Vcodex_t*	vc;
Void_t* 	tar;
size_t		ntar;
Void_t**	del;
#endif
{
	ssize_t		sz, hd, ninst, naddr, ndfsz, nunsz, ndiff, nuntr;
	Vcchar_t	ctrl, *dt, *output, *instdt, *addrdt, *dfszdt, *unszdt, *diffdt, *untrdt;
	Vcio_t		io, inst, addr, diff, untr, dfsz, unsz;
	Sieve_t		*si = vcgetmtdata(vc, Sieve_t*);
	Vcdisc_t	*disc = vcgetdisc(vc);

	if(ntar == 0)
		return 0;

	if(si->mmin <= 0) /* mmin must be positive */
	{	sz = vcperiod(tar, 64*1024 < ntar ? 64*1024 : ntar)/2;
		si->mmin = (sz >= 4 && sz < VCS_MATCH) ? sz : VCS_MATCH;;
	} /**/DEBUG_PRINT(2,"mmin=%d\n",si->mmin);

	sz = disc ? disc->size : 0;
	si->vcpa.src  = sz == 0 ? NIL(Vcchar_t*) : (Vcchar_t*)disc->data;
	si->vcpa.nsrc = sz;
	si->vcpa.tar  = (Vcchar_t*)tar;
	si->vcpa.ntar = ntar;
	si->vcpa.mmin = si->mmin;
	si->vcpa.cmap = si->cmap;
	si->vcpa.type = ((si->type & VCS_REVERSE) ? VCLZ_REVERSE : 0) |
			((si->type & VCS_MAP) ? VCLZ_MAP : 0);

#define HEAD	128 /* a sufficiently large header */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), 6*ntar, HEAD)) )
		RETURN(-1);

	vcioinit(&untr, output+0*ntar, ntar); si->untr = &untr; /* untransformed data */
	vcioinit(&diff, output+1*ntar, ntar); si->diff = &diff; /* diff-ed data */
	vcioinit(&inst, output+2*ntar, ntar); si->inst = &inst; /* control bytes */
	vcioinit(&unsz, output+4*ntar, ntar); si->unsz = &unsz; /* size of unmatcheds */
	vcioinit(&dfsz, output+3*ntar, ntar); si->dfsz = &dfsz; /* size of diffs */
	vcioinit(&addr, output+5*ntar, ntar); si->addr = &addr; /* addresses of segments */

	/* initialize address cache then run the parser to code matching segments */
	VCSKINIT(si);
	vclzparse(&si->vcpa, 0);

	nuntr = vciosize(&untr); untrdt = vciodata(&untr); /**/DEBUG_PRINT(2,"Raw: untr=%d, ", nuntr);
	ndiff = vciosize(&diff); diffdt = vciodata(&diff); /**/DEBUG_PRINT(2,"diff=%d, ", ndiff);
	ninst = vciosize(&inst); instdt = vciodata(&inst); /**/DEBUG_PRINT(2,"inst=%d, ", ninst);
	nunsz = vciosize(&unsz); unszdt = vciodata(&unsz); /**/DEBUG_PRINT(2,"unsz=%d, ", nunsz);
	ndfsz = vciosize(&dfsz); dfszdt = vciodata(&dfsz); /**/DEBUG_PRINT(2,"dfsz=%d, ", ndfsz);
	naddr = vciosize(&addr); addrdt = vciodata(&addr); /**/DEBUG_PRINT(2,"addr=%d\n", naddr);

	ctrl = 0; /* control byte to tell what was secondarily compressed */

	/* secondary encoding of the resulting data */
	if(vc->coder && nuntr > 0 && (sz = vcapply(vc->coder, untrdt, nuntr, &dt)) < nuntr)
	{	if(sz < 0)
			RETURN(-1);
		ctrl |= VCS_UNTRCMP;
		untrdt = dt; nuntr = sz;
	} /**/DEBUG_PRINT(2,"untr=%d, ", nuntr);

	if(vc->coder && ndiff > 0 && (sz = vcapply(vc->coder, diffdt, ndiff, &dt)) < ndiff)
	{	if(sz < 0)
			RETURN(-1);
		ctrl |= VCS_DIFFCMP;
		diffdt = dt; ndiff = sz;
	} /**/DEBUG_PRINT(2,"diff=%d, ", ndiff);

	if(ninst > 0 && (sz = vcapply(si->vch, instdt, ninst, &dt)) < ninst)
	{	if(sz < 0)
			RETURN(-1);
		ctrl |= VCS_INSTCMP;
		instdt = dt; ninst = sz;
	} /**/DEBUG_PRINT(2,"inst=%d, ", ninst);

	if(nunsz > 0 && (sz = vcapply(si->vch, unszdt, nunsz, &dt)) < nunsz )
	{	if(sz < 0)
			RETURN(-1);
		ctrl |= VCS_UNSZCMP;
		unszdt = dt; nunsz = sz;
	} /**/DEBUG_PRINT(2,"unsz=%d, ", nunsz);

	if(ndfsz > 0 && (sz = vcapply(si->vch, dfszdt, ndfsz, &dt)) < ndfsz )
	{	if(sz < 0)
			RETURN(-1);
		ctrl |= VCS_DFSZCMP;
		dfszdt = dt; ndfsz = sz;
	} /**/DEBUG_PRINT(2,"dfsz=%d, ", ndfsz);

	if(naddr > 0 && (sz = vcapply(si->vch, addrdt, naddr, &dt)) < naddr )
	{	if(sz < 0)
			RETURN(-1);
		ctrl |= VCS_ADDRCMP;
		addrdt = dt; naddr = sz;
	} /**/DEBUG_PRINT(2,"addr=%d\n", naddr);

	/* size of header */
	hd = vcsizeu(ntar) +  /* original data size		*/
	     vcsizeu(si->mmin) + /* min length of exact matches	*/
	     sizeof(Vcchar_t) + /* byte telling cmp-ed sections	*/
	     vcsizeu(nuntr) + /* size of untransformed data	*/
	     vcsizeu(ndiff) + /* size of transformed data	*/
	     vcsizeu(ninst) + /* size of control bytes		*/
	     vcsizeu(nunsz) + /* size of unmatched data		*/
	     vcsizeu(ndfsz) + /* size of diff-ed data		*/
	     vcsizeu(naddr);  /* size of addr of matched data	*/

	sz = hd + nuntr + ndiff + ninst + nunsz + ndfsz + naddr;
	/**/DEBUG_PRINT(2,"source=%d, ", si->vcpa.nsrc);
	/**/DEBUG_PRINT(2,"target=%d, ", si->vcpa.ntar);
	/**/DEBUG_PRINT(2,"cmpsiz=%d\n", sz);

	/* output data */
	output -= hd;
	vcioinit(&io, output, sz);
	vcioputu(&io, ntar);
	vcioputu(&io, si->mmin);
	vcioputc(&io, ctrl);
	vcioputu(&io, nuntr);
	vcioputu(&io, ndiff);
	vcioputu(&io, ninst);
	vcioputu(&io, nunsz);
	vcioputu(&io, ndfsz);
	vcioputu(&io, naddr);
	vcioputs(&io, untrdt, nuntr);
	vcioputs(&io, diffdt, ndiff);
	vcioputs(&io, instdt, ninst);
	vcioputs(&io, unszdt, nunsz);
	vcioputs(&io, dfszdt, ndfsz);
	vcioputs(&io, addrdt, naddr); /**/DEBUG_ASSERT(vciosize(&io) == sz);

	/* reduce space usage */
	vcbuffer(si->vch, NIL(Vcchar_t*), -1, -1);
	if(vc->coder)
		vcbuffer(vc->coder, NIL(Vcchar_t*), -1, -1);

	if(del)
		*del = output;
	return sz;
}

#if __STD_C
static ssize_t unsieve(Vcodex_t* vc, const Void_t* del, size_t ndel, Void_t** out)
#else
static ssize_t unsieve(vc, del, ndel, out)
Vcodex_t*	vc;
Void_t*		del;
size_t		ndel;
Void_t**	out;
#endif
{
	Vcchar_t	*ms, *ts, *endts, *dt, *endd, *tar;
	ssize_t		ntar, un, df, in, uz, dz, ad, here, mmin;
	int		ctrl, k, d;
	Vcio_t		io, untr, diff, unsz, dfsz, addr;
	Vcchar_t	*inst;
	Sieve_t		*si = vcgetmtdata(vc, Sieve_t*);
	Vcdisc_t	*disc = vcgetdisc(vc);
	Vcchar_t	*cmap = si->cmap;

	/* get source data, if any */
	dz = disc ? disc->size : 0;
	si->vcpa.src  = dz == 0 ? NIL(Vcchar_t*) : (Vcchar_t*)disc->data;
	si->vcpa.nsrc = dz;

	vcioinit(&io, del, ndel);

	if(vciomore(&io) <= 0 || (ntar = vciogetu(&io)) <= 0 )
		return ntar;
	if(vciomore(&io) <= 0 || (mmin = vciogetu(&io)) < 0 )
		RETURN(-1);
	if(vciomore(&io) <= 0 || (ctrl = vciogetc(&io)) < 0 )
		RETURN(-1);
	if(vciomore(&io) <= 0 || (un = vciogetu(&io)) < 0 )
		RETURN(-1);
	if(vciomore(&io) <= 0 || (df = vciogetu(&io)) < 0 )
		RETURN(-1);
	if(vciomore(&io) <= 0 || (in = vciogetu(&io)) < 0 )
		RETURN(-1);
	if(vciomore(&io) <= 0 || (uz = vciogetu(&io)) < 0 )
		RETURN(-1);
	if(vciomore(&io) <= 0 || (dz = vciogetu(&io)) < 0 )
		RETURN(-1);
	if(vciomore(&io) <= 0 || (ad = vciogetu(&io)) < 0 )
		RETURN(-1);

	if(vciomore(&io) < un) /* get the unmatched data */
		RETURN(-1);
	dt = vcionext(&io); vcioskip(&io, un);
	if(ctrl&VCS_UNTRCMP )
	{	if(!vc->coder || (un = vcapply(vc->coder, dt, un, &dt)) < 0 )
			RETURN(-1);
	}
	vcioinit(&untr, dt, un);

	if(vciomore(&io) < df) /* get the diff-ed data */
		RETURN(-1);
	dt = vcionext(&io); vcioskip(&io, df);
	if(ctrl&VCS_DIFFCMP )
	{	if(!vc->coder || (df = vcapply(vc->coder, dt, df, &dt)) < 0 )
			RETURN(-1);
	}
	vcioinit(&diff, dt, df);

	if(vciomore(&io) < in)
		RETURN(-1);
	dt = vcionext(&io); vcioskip(&io, in);
	if(ctrl&VCS_INSTCMP )
	{	if((in = vcapply(si->vch, dt, in, &dt)) < 0)
			RETURN(-1);
	}
	inst = dt;

	if(vciomore(&io) < uz)
		RETURN(-1);
	dt = vcionext(&io); vcioskip(&io, uz);
	if(ctrl&VCS_UNSZCMP )
	{	if((uz = vcapply(si->vch, dt, uz, &dt)) < 0)
			RETURN(-1);
	}
	vcioinit(&unsz, dt, uz);

	if(vciomore(&io) < dz)
		RETURN(-1);
	dt = vcionext(&io); vcioskip(&io, dz);
	if(ctrl&VCS_DFSZCMP )
	{	if((dz = vcapply(si->vch, dt, dz, &dt)) < 0)
			RETURN(-1);
	}
	vcioinit(&dfsz, dt, dz);

	if(vciomore(&io) < ad)
		RETURN(-1);
	dt = vcionext(&io); vcioskip(&io, ad);
	if(ctrl&VCS_ADDRCMP )
	{	if((ad = vcapply(si->vch, dt, ad, &dt)) < 0)
			RETURN(-1);
	}
	vcioinit(&addr, dt, ad);

	if(!(tar = vcbuffer(vc, NIL(Vcchar_t*), ntar, 0)) )
		RETURN(-1);

	VCSKINIT(si);
	for(k = 0, here = si->vcpa.nsrc, endts = (ts = tar)+ntar; ts < endts; )
	{	/* get the control byte for this instruction */	
		if(k >= in)
			RETURN(-1);
		ctrl = inst[k++];

		if(ctrl == VCS_UNTR )
		{	if(vciomore(&unsz) <= 0 )
				RETURN(-1);
			if((uz = vciogetu(&unsz)) < 0 )
				RETURN(-1);	

			/* copy unmatched data */
			uz = DECODE(uz, 1);
			if((ts+uz) > endts)
				RETURN(-1);

			if(vciomore(&untr) < uz)
				RETURN(-1);
			dt = vcionext(&untr); vcioskip(&untr, uz);
			for(endd = dt+uz; dt < endd; ++ts, ++dt)
				*ts = *dt;

			/**/DEBUG_PRINT(8, "untr %6d  ", uz);
			/**/DEBUG_PRINT(8, "tpos %6d\n", here - si->vcpa.nsrc);

			here += uz;
		}
		else
		{	if(vciomore(&dfsz) <= 0 )
				RETURN(-1);
			if((dz = vciogetu(&dfsz)) < 0 )
				RETURN(-1);	

			/* decode sieved data */
			dz = DECODE(dz, mmin);
			if((ts+dz) > endts)
				RETURN(-1);

			if(vciomore(&addr) <= 0 )
				RETURN(-1);
			if((df = vciogetu(&addr)) < 0 )
				RETURN(-1);
			if((ad = address(si, df, (ctrl&VCS_REVERSE) ? -dz : dz, here, &ctrl)) < 0 )
				RETURN(-1);

			/**/DEBUG_PRINT(8, "%s ", (ctrl&VCS_DIFF) ? "diff" : "copy");
			/**/DEBUG_PRINT(8, "%6d  ", dz);
			/**/DEBUG_PRINT(8, "tpos %6d  ", here - si->vcpa.nsrc);
			/**/DEBUG_PRINT(8, "mpos %6d  ", ad);
			/**/DEBUG_PRINT(8, "here %6d  ", here);
			/**/DEBUG_PRINT(8, "addr %6d  ", df);
			/**/DEBUG_PRINT(8, "type %s,", (ctrl&VCS_REVERSE)? "r" : "f");
			/**/DEBUG_PRINT(8, "%s\n", (ctrl&VCS_MAP)? "m" : "i");

			if(ad >= si->vcpa.nsrc)
				ms = tar + (ad - si->vcpa.nsrc);
			else	ms = si->vcpa.src + ad;

			endd = (dt = vcionext(&diff)) + dz;
			if(ctrl&VCS_DIFF)
			{	if(vciomore(&diff) < dz)
					RETURN(-1);
				vcioskip(&diff, dz);
			}

			d = (ctrl&VCS_REVERSE) ? -1 : 1;
			if(!(ctrl&VCS_MAP) )
			{	if(ctrl&VCS_DIFF)
					for(; dt < endd; ++dt, ++ts, ms += d)
						*ts = (Vcchar_t)(*ms - *dt);
				else	for(; dt < endd; ++dt, ++ts, ms += d)
						*ts = (Vcchar_t)(*ms);
			}
			else if(!cmap)
				RETURN(-1);
			else
			{	if(ctrl&VCS_DIFF)
					for(; dt < endd; ++dt, ++ts, ms += d)
						*ts = cmap[(Vcchar_t)(*ms - *dt)];
				else	for(; dt < endd; ++dt, ++ts, ms += d)
						*ts = cmap[(Vcchar_t)(*ms)];
			}

			here += dz;
		}

	} /**/DEBUG_ASSERT(ntar == here-si->vcpa.nsrc);

	vcbuffer(si->vch, NIL(Vcchar_t*), -1, -1);
	if(vc->coder)
		vcbuffer(vc->coder, NIL(Vcchar_t*), -1, -1);

	if(out)
		*out = tar;
	return ntar;
}

/* return the byte map, if any */
#if __STD_C
static ssize_t sieveextract(Vcodex_t* vc, Vcchar_t** datap)
#else
static ssize_t sieveextract(vc, datap)
Vcodex_t*	vc;
Vcchar_t**	datap;	/* basis string for persistence	*/
#endif
{
	Vcchar_t	*dt, cm[256];
	ssize_t		k, sz;
	Vcio_t		io;
	Sieve_t		*si = vcgetmtdata(vc, Sieve_t*);

	sz = 0; /* construct the pair mapping if any */
	if(si->cmap)
		for(k = 0; k < 256; ++k)
			if(si->cmap[k] > k)
				{ cm[sz] = k; cm[sz+1] = si->cmap[k]; sz += 2; }
	cm[sz] = 0;

	if(!(dt = vcbuffer(vc, NIL(Vcchar_t*), sz + 4*sizeof(ssize_t), 0)) )
		RETURN(-1);
	vcioinit(&io, dt, sz + 4*sizeof(ssize_t));

	/* code the byte mapping */
	vcioputu(&io, sz);
	vcioputs(&io, cm, sz);

	if(datap)
		*datap = dt;

	return vciosize(&io);
}

/* reconstruct the handle, possibly with a byte map */
#if __STD_C
static int sieverestore(Vcmtcode_t* mtcd)
#else
static int sieverestore(mtcd)
Vcmtcode_t*	mtcd;
#endif
{
	Vcio_t		io;
	ssize_t		sz, k, n;
	char		*args;

	vcioinit(&io, mtcd->data, mtcd->size);

	if((sz = vciogetu(&io)) < 0 )
		return -1;

	/* reconstruct the argument string */
	if(sz == 0)
		args = NIL(char*);
	else
	{	if(!(args = (char*)malloc(32 + sz)) )
			return -1;

		for(k = 0; _Siargs[k].name; ++k)
			if(_Siargs[k].data == (Void_t*)VCS_MAP)
				break;
		memcpy(args, _Siargs[k].name, (n = strlen(_Siargs[k].name)) );
		args[n] = '=';
		vciogets(&io, args+n+1, sz);
		args[n+1+sz] = 0;
	}

	/* reconstruct the handle for decoding */
	mtcd->coder = vcopen(0, Vcsieve, args, mtcd->coder, VC_DECODE);

	if(args)
		free(args);

	return mtcd->coder ? 1 : -1;
}

#if __STD_C
static int sievent(Vcodex_t* vc, int type, Void_t* init)
#else
static int sievent(vc, type, init)
Vcodex_t*	vc;
int		type;
Void_t*		init;
#endif
{
	Sieve_t		*si;
	ssize_t		z;
	char		*data, val[1024];
	Vcmtarg_t	*arg;
	Vcmtcode_t	*mtcd;

	if(type == VC_OPENING)
	{	if(!(si = (Sieve_t*)calloc(1, sizeof(Sieve_t)+256)) )
			RETURN(-1);

		for(data = (char*)init; data && *data; )
		{	data = vcgetmtarg(data, val, sizeof(val), _Siargs, &arg);
			switch(TYPECAST(int,arg->data) )
			{ case VCS_REVERSE :
				si->type |= VCS_REVERSE;
				break;
			  case VCS_DIFF :
				si->type |= VCS_DIFF;
				break;
			  case VCS_MAP :
				si->type |= VCS_MAP;
				si->cmap = (Vcchar_t*)(si+1);
				for(z = 0; z < 256; ++z)
					si->cmap[z] = z;
				for(z = 0; val[z] && val[z+1]; z += 2)
				{	si->cmap[val[z]] = val[z+1];
					si->cmap[val[z+1]] = val[z];
				}
				break;
			  case VCS_MMIN :
				if(isdigit(*val) )
					si->mmin = (ssize_t)vcatoi(val);
				break;
			}
		}

		/* Sizes and addresses are Huffman coded */
		if(!(si->vch = vcopen(0, Vchuffman, 0, 0, vc->flags&(VC_ENCODE|VC_DECODE))) )
		{	free(si);
			RETURN(-1);
		}

		si->vcpa.parsef = putinst;

		vcsetmtdata(vc, si);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if((si = vcgetmtdata(vc, Sieve_t*)) )
		{	if(si->vch)
				vcclose(si->vch);
			free(si);
		}

		vcsetmtdata(vc, NIL(Sieve_t*));
		return 0;
	}
	else if(type == VC_EXTRACT)
	{	if(!(mtcd = (Vcmtcode_t*)init) )
			return -1;
		if((mtcd->size = sieveextract(vc, &mtcd->data)) < 0 )
			return -1;
		return 1;
	}
	else if(type == VC_RESTORE)
	{	if(!(mtcd = (Vcmtcode_t*)init) )
			return -1;
		return sieverestore(mtcd) < 0 ? -1 : 1;
	}
	else	return 0;
}

Vcmethod_t _Vcsieve =
{	sieve,
	unsieve,
	sievent,
	"sieve", "(Delta) Compression via sieving.",
	"[-?\n@(#)$Id: vcodex-sieve (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	_Siargs,
	1024*1024,
	VC_MTSOURCE
};

VCLIB(Vcsieve)
