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
#include	"vchhdr.h"

/*	Group segments of data for more effective Huffman coding.
**
**	Written by Binh Dao Vo and Kiem-Phong Vo
*/

#define GRP_NTBL	32	/* max number of tables		*/
#define GRP_IMAX	20	/* max initial number of tables	*/
#define GRP_IMIN	4	/* min initial number of tables	*/
#define GRP_ITER	3	/* desired number of iterations	*/

#define GRP_HUGE	((ssize_t)((~((size_t)0)) >> 1) )

typedef struct _table_s
{	ssize_t		size[VCH_SIZE]; /* code length table	*/
	int		maxs;	/* max code length		*/
	int		runb;	/* the run object if any	*/
	ssize_t		nblks;	/* # of associated blocks	*/
	ssize_t		cost;	/* cost of encoding		*/
} Table_t;

typedef struct _obj_s
{	Vchobj_t	obj;	/* object			*/
	Vcchar_t	size;	/* Huffman code size in part	*/
	ssize_t		freq;	/* object frequency in part	*/
} Obj_t;

typedef struct _group_s
{	Vcodex_t*	huf;	/* Huffman coder/decoder	*/
	Vcodex_t*	mtf;	/* MTF coder/decoder		*/

	ssize_t		ptsz;	/* actual part size		*/
	ssize_t		npts;	/* number of parts		*/
	Vcchar_t*	part;	/* table index for each part	*/
	Vcchar_t*	work;	/* working space for indices	*/
	ssize_t*	ppos;	/* position of part[] in objs[]	*/
	ssize_t*	sort;	/* for sorting part positions	*/

	Obj_t*		obj;	/* objs and their frequencies	*/

	ssize_t		hufsz;	/* size of single Huffman code 	*/
	ssize_t		cmpsz;	/* current best compressed size	*/
	ssize_t		ntbl;	/* number of coding tables	*/
	Table_t		tbl[GRP_NTBL]; /* best coding tables	*/
} Group_t;

#undef GRPfreq
#undef GRPFREQ
#undef GRPsize
#undef GRPSIZE

/* sum frequencies for distinct objects */
#define GRPfreq(fr,oo,kk) \
switch(kk) \
{ case 16: fr[oo->obj] += oo->freq; oo++;  case 15: fr[oo->obj] += oo->freq; oo++;  \
  case 14: fr[oo->obj] += oo->freq; oo++;  case 13: fr[oo->obj] += oo->freq; oo++;  \
  case 12: fr[oo->obj] += oo->freq; oo++;  case 11: fr[oo->obj] += oo->freq; oo++;  \
  case 10: fr[oo->obj] += oo->freq; oo++;  case  9: fr[oo->obj] += oo->freq; oo++;  \
  case  8: fr[oo->obj] += oo->freq; oo++;  case  7: fr[oo->obj] += oo->freq; oo++;  \
  case  6: fr[oo->obj] += oo->freq; oo++;  case  5: fr[oo->obj] += oo->freq; oo++;  \
  case  4: fr[oo->obj] += oo->freq; oo++;  case  3: fr[oo->obj] += oo->freq; oo++;  \
  case  2: fr[oo->obj] += oo->freq; oo++;  case  1: fr[oo->obj] += oo->freq; oo++;  \
}
#define GRPFREQ(fr,o,n) \
do {	Obj_t* oo = (Obj_t*)(o); \
	ssize_t nn = (ssize_t)(n); \
	for(; nn > 0; nn -= VCH_SW) GRPfreq(fr, oo, nn >= VCH_SW ? VCH_SW : nn); \
} while(0)

/* accumulate coding size of all objects */
#define GRPsize(v,sz,oo,kk) \
switch(kk) \
{ case 16: v += sz[oo->obj] * oo->freq; oo++; case 15: v += sz[oo->obj] * oo->freq; oo++; \
  case 14: v += sz[oo->obj] * oo->freq; oo++; case 13: v += sz[oo->obj] * oo->freq; oo++; \
  case 12: v += sz[oo->obj] * oo->freq; oo++; case 11: v += sz[oo->obj] * oo->freq; oo++; \
  case 10: v += sz[oo->obj] * oo->freq; oo++; case  9: v += sz[oo->obj] * oo->freq; oo++; \
  case  8: v += sz[oo->obj] * oo->freq; oo++; case  7: v += sz[oo->obj] * oo->freq; oo++; \
  case  6: v += sz[oo->obj] * oo->freq; oo++; case  5: v += sz[oo->obj] * oo->freq; oo++; \
  case  4: v += sz[oo->obj] * oo->freq; oo++; case  3: v += sz[oo->obj] * oo->freq; oo++; \
  case  2: v += sz[oo->obj] * oo->freq; oo++; case  1: v += sz[oo->obj] * oo->freq; oo++; \
}
#define GRPSIZE(v,sz,o,n) \
do {	Obj_t* oo = (Obj_t*)(o); \
	ssize_t nn = (ssize_t)(n); v = 0; \
	for(; nn > 0; nn -= VCH_SW) GRPsize(v, sz, oo, nn >= VCH_SW ? VCH_SW : nn); \
} while(0)


#if __STD_C
static int objcmp(Void_t* one, Void_t* two, Void_t* hdl)
#else
static int objcmp(one, two, hdl)
Void_t*		one;
Void_t*		two;
Void_t*		hdl;
#endif
{
	int	d;
	Obj_t	*o1 = (Obj_t*)one;
	Obj_t	*o2 = (Obj_t*)two;

	if((d = o1->size - o2->size) != 0)
		return d;
	else	return (int)o1->obj - (int)o2->obj;
}

/* Construct a list of distinct objects and frequencies from data[]. This list
** is used for fast computation of total frequencies, encoding lengths, etc.
** It is good for data transformed by a Burrows-Wheeler transform since the
** distribution is skewed toward a few small values.
*/
#if __STD_C
static int grpinit(Group_t* grp, Vchobj_t* data, size_t dtsz, ssize_t ptsz)
#else
static int grpinit(grp, data, dtsz, ptsz)
Group_t*	grp;
Vchobj_t*	data;
size_t		dtsz;
ssize_t		ptsz;
#endif
{
	ssize_t		freq[VCH_SIZE], size[VCH_SIZE];
	ssize_t		i, k, d, p, npts;

 	if(ptsz >= (ssize_t)dtsz )
		ptsz = (ssize_t)dtsz;
	grp->ptsz = ptsz;
	grp->npts = npts = (dtsz+ptsz-1)/ptsz; /* guaranteed >= 1 */
	grp->cmpsz = (dtsz < VCH_SIZE ? VCH_SIZE : dtsz)*VC_BITSIZE; /* starting cost */
	grp->ntbl = 0;

	if(grp->part)
		free(grp->part);
	if(!(grp->part = (Vcchar_t*)calloc(2*npts, sizeof(Vcchar_t))) )
		return -1;
	grp->work = grp->part + npts;

	if(grp->ppos)
		free(grp->ppos);
	if(!(grp->ppos = (ssize_t*)calloc((2*npts+1), sizeof(ssize_t))) )
		return -1;
	grp->sort = grp->ppos + npts+1;

	if(grp->obj)
		free(grp->obj);
	if(!(grp->obj = (Obj_t*)calloc(dtsz, sizeof(Obj_t))) )
		return -1;

	/* ptsz is such that a object frequency should fit in a byte */
	for(d = 0, p = 0, i = 0; i < npts; i += 1, d += ptsz)
	{	grp->ppos[i] = p;

		CLRTABLE(freq,VCH_SIZE);
		ADDFREQ(freq, Vchobj_t*, data+d, i == npts-1 ? dtsz-d : ptsz);
		CLRTABLE(size,VCH_SIZE);
		vchsize(VCH_SIZE, freq, size, 0);
		for(k = 0; k < VCH_SIZE; ++k)
		{	if(freq[k] != 0) /* info of non-trivial code only */
			{	grp->obj[p].obj = (Vchobj_t)k;
				grp->obj[p].size = (Vcchar_t)size[k];
				grp->obj[p].freq = freq[k];
				p += 1;
			}
		}

		/* sort by code lengths */
		vcqsort(grp->obj+grp->ppos[i], p-grp->ppos[i], sizeof(Obj_t), objcmp, 0);
	}
	grp->ppos[npts] = p;

	return 0;
}

/* sorting parts by heaviest elements */
#if __STD_C
static int partcmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int partcmp(one, two, disc)
Void_t*	one;
Void_t*	two;
Void_t*	disc;
#endif
{
	int		p, q, n, m, d;
	Group_t		*grp = (Group_t*)disc;

	p = *((size_t*)one); m = grp->ppos[p+1] - grp->ppos[p]; p = grp->ppos[p];
	q = *((size_t*)two); n = grp->ppos[q+1] - grp->ppos[q]; q = grp->ppos[q];
	if(m > n)
		m = n;
	if(m > 8)
		m = 8;

	for(n = 0; n < m; ++n )
		if((d = (int)grp->obj[p+n].obj - (int)grp->obj[q+n].obj) != 0 )
			return d;

	for(n = 0; n < m; ++n )
		if((d = (int)grp->obj[p+n].freq - (int)grp->obj[q+n].freq) != 0 )
			return d;

	return p-q;
}

/* compute an optimal clustering with ntbl clusters */
#if __STD_C
static void grppart(Group_t* grp, ssize_t ntbl, int niter)
#else
static void grppart(grp, ntbl, niter)
Group_t*	grp;
ssize_t		ntbl;	/* # of tables aiming for	*/
int		niter;	/* # of iterations to run	*/
#endif
{
	ssize_t		i, k, p, q, z, n, t, iter;
	Vcchar_t	*dt, tmp[VCH_SIZE];
	Table_t		tbl[GRP_NTBL];
	int		map[GRP_NTBL];
	ssize_t		freq[GRP_NTBL][VCH_SIZE], pfr[VCH_SIZE], psz[VCH_SIZE], *fr, *sz;
	Vcchar_t	*part = grp->part, *work = grp->work;
	ssize_t		npts = grp->npts, *ppos = grp->ppos, *sort = grp->sort;
	Obj_t		*obj = grp->obj;

	if(ntbl > npts)
		ntbl = npts;
	if(ntbl > GRP_NTBL)
		ntbl = GRP_NTBL;

	if(grp->ntbl <= 0 || ntbl < grp->ntbl) /* making new tables */
	{	/* sort parts so that similar prefixes group together */
		for(k = 0; k < npts; ++k)
			sort[k] = k;
		vcqsort(sort, npts, sizeof(ssize_t), partcmp, grp);

		/* now make tables */
		for(z = npts/ntbl, p = 0, t = 0; t < ntbl; t += 1)
		{	fr = freq[t]; CLRTABLE(fr, VCH_SIZE);

			for(n = p+z > npts ? (npts-p) : z; n > 0; --n, ++p)
			{	k = sort[p];
				GRPFREQ(fr, obj+ppos[k], ppos[k+1]-ppos[k]);
			}

			tbl[t].maxs = vchsize(VCH_SIZE, fr, tbl[t].size, &tbl[t].runb);
			tbl[t].nblks = 0;
			tbl[t].cost = 0;
		}
	}
	else /* increasing number of tables */
	{	/**/DEBUG_ASSERT(ntbl <= GRP_NTBL && grp->ntbl <= GRP_NTBL);
		memcpy(tbl,grp->tbl,grp->ntbl*sizeof(Table_t));
		n = ntbl - grp->ntbl; ntbl = grp->ntbl;
		for(; n > 0; --n)
		{	for(z = 0, p = -1, i = 0; i < grp->ntbl; ++i)
			{	if(tbl[i].cost <= z)
					continue;
				z = tbl[p = i].cost;
			}
			if(p < 0) /* if p >= 0, it's the highest cost table */
				break;

			/* split blocks of table p into two tables, p and q */
			q = ntbl; ntbl += 1;
			z = tbl[p].nblks/2 - 1; fr = freq[p]; CLRTABLE(fr, VCH_SIZE);
			for(i = 0; i < npts; ++i)
			{	if(work[i] != p)
					continue;
				GRPFREQ(fr, obj+ppos[i], ppos[i+1]-ppos[i]);
				if((z -= 1) == 0) /* start 2nd table */
					{ fr = freq[q]; CLRTABLE(fr, VCH_SIZE); }
			}

			/* make sure neither table will considered for further splitting */
			tbl[p].maxs = vchsize(VCH_SIZE, freq[p], tbl[p].size, &tbl[p].runb);
			tbl[q].maxs = vchsize(VCH_SIZE, freq[q], tbl[q].size, &tbl[q].runb);
			tbl[p].cost = tbl[q].cost = 0;
			tbl[p].nblks = tbl[q].nblks = 0;
		}
	}

	/**/DEBUG_PRINT(2,"\tgrppart: #table aiming for=%d\n",ntbl);
	for(iter = 1;; iter++)
	{	/**/DEBUG_PRINT(2,"\t\titer=%d ", iter); DEBUG_PRINT(2,"cmpsz=%d ", (grp->cmpsz+7)/8);

		for(k = 0; k < ntbl; ++k)
		{	fr = freq[k]; sz = tbl[k].size;
			if((z = tbl[k].maxs) > 0)
			{	z += (z <= 4 ? 15 : z <= 8 ? 7 : z <= 16 ? 1 : 0);
				for(p = 0; p < VCH_SIZE; ++p)
				{	if(fr[p] != 0)
						fr[p] = 0; /* clear frequency table */
					else	sz[p] = z; /* 0-freq obj gets dflt length */
				}
			}
			tbl[k].cost  = 0;
			tbl[k].nblks = 0;
		}

		for(i = 0; i < npts; i += 1)
		{	ssize_t		bestz, bestt;

			/* find the table best matching this part */
			p = ppos[i]; n = ppos[i+1] - ppos[i];
			for(bestz = GRP_HUGE, bestt = -1, k = 0; k < ntbl; ++k)
			{	if(tbl[k].maxs == 0) /* representing a run */
					z = (n == 1 && obj[p].obj == tbl[k].runb) ? 0 : GRP_HUGE;
				else /* normal table, tally up the lengths */
					{ sz = tbl[k].size; GRPSIZE(z, sz, obj+p, n); }
				if(z < bestz || bestt < 0)
				{	bestz = z;
					bestt = k;
				}
			}

			work[i] = bestt; /* assignment, then add frequencies */
			fr = freq[bestt]; GRPFREQ(fr, obj+p, n );
			tbl[bestt].nblks += 1;
		}

		/* remove empty tables */
		for(p = k = 0; k < ntbl; ++k)
		{	map[k] = k; /* start as identity map */

			if(tbl[k].nblks <= 0) /* empty table */
				continue;

			if(k > p) /* need to move this table */
			{	memcpy(tbl+p, tbl+k, sizeof(Table_t));
				memcpy(freq[p], freq[k], VCH_SIZE*sizeof(ssize_t));
				map[k] = p; /* table at k was moved to p */
			}
			p += 1; /* move beyond known valid slot */
		}
		if(p < ntbl) /* tables were moved, reset part indexes */
		{	for(i = 0; i < npts; ++i)
			{	/**/DEBUG_ASSERT(work[i] < ntbl);
				work[i] = map[work[i]];
				/**/DEBUG_ASSERT(work[i] < p);
			}
			ntbl = p;
		}

		z = 0; /* recompute encoding cost given new grouping */
		for(k = 0; k < ntbl; ++k)
		{	fr = freq[k]; sz = tbl[k].size;
			tbl[k].maxs = vchsize(VCH_SIZE, fr, sz, &tbl[k].runb);
			if(tbl[k].maxs > 0)
			{	DOTPRODUCT(p,fr,sz,VCH_SIZE); /* encoding size */
				n = vchputcode(VCH_SIZE, sz, tbl[k].maxs, tmp, sizeof(tmp));
				p += (n + vcsizeu(n))*8;
				tbl[k].cost = p;
				z += p; /* add to total cost */
			}
			else
			{	/**/DEBUG_ASSERT(tbl[k].runb >= 0);
				z += (tbl[k].cost = 2*8); /* one 0-byte and the run byte */
			}
		}

		if(ntbl > 1) /* add the cost of encoding the indices */
		{	n = vcapply(grp->mtf,work,npts,&dt); /* mtf transform */
			CLRTABLE(pfr,VCH_SIZE); ADDFREQ(pfr, Vcchar_t*, dt, n);
			k = vchsize(VCH_SIZE, pfr, psz, NIL(int*));
			DOTPRODUCT(p, pfr, psz, VCH_SIZE);
			n = vchputcode(VCH_SIZE, psz, k, tmp, sizeof(tmp));
			z += p + (n + vcsizeu(n))*8;
		}

		/**/DEBUG_PRINT(2,"z=%d\n", (z+7)/8);
		if(z > grp->hufsz)
		{	/**/DEBUG_PRINT(2, "Stop iterating: z=%d > huffman size\n", z);
			grp->ntbl = 0;
			return;
		}

		if(z < (p = grp->cmpsz) )
		{	grp->ntbl = ntbl;
			grp->cmpsz = z;
			memcpy(part, work, npts);
			memcpy(grp->tbl, tbl, ntbl*sizeof(Table_t));
		}

		if(ntbl == 1 || iter >= niter || (iter > 1 && z >= p-64) )
		{	/**/DEBUG_PRINT(2,"\t\t#table=%d ",grp->ntbl);
			/**/DEBUG_PRINT(2,"cmpsz=%d\n", (grp->cmpsz+7)/8);
			return;
		}
	}
}

/* select a part size based on the data size. */
static ssize_t grpptsz(size_t dtsz)
{
	ssize_t	ptsz, exp, lnz;

	lnz = vclogi(dtsz); /* log_2 of data size */
	if(lnz >= 16)
	{	exp = (exp = (lnz-15)) > 8 ? 8 : exp;
		ptsz = (1 << exp) * lnz; /* ptsz here is >= 32 */
	}
	else	ptsz = dtsz/(1<<10); /* ptsz here <= 64 */

	ptsz = ptsz < 16 ? 16 : ptsz > 1024 ? 1024 : ptsz;
	/**/DEBUG_PRINT(2,"grpptsz: dtsz=%d, ", dtsz);
	/**/DEBUG_PRINT(2,"lnz=%d, ", lnz);
	/**/DEBUG_PRINT(2,"ptsz=%d\n",ptsz);
	return ptsz;
}

#if __STD_C
static ssize_t grphuff(Vcodex_t* vc, const Void_t* data, size_t dtsz, Void_t** out)
#else
static ssize_t grphuff(vc, data, dtsz, out)
Vcodex_t*	vc;	/* Vcodex handle		*/
Void_t*		data;	/* target data to be compressed	*/
size_t		dtsz;	/* data size			*/
Void_t**	out;	/* to return output buffer 	*/
#endif
{
	ssize_t		n, i, p, k;
	ssize_t		*sz, npts, ntbl, ptsz, itbl;
	ssize_t		freq[VCH_SIZE], size[VCH_SIZE];
	Vcchar_t	*part;
	Table_t		*tbl;
	Vcbit_t		b, *bt, bits[GRP_NTBL][VCH_SIZE];
	Vcchar_t	*output, *dt;
	ssize_t		n_output;
	Vcio_t		io;
	Group_t		*grp = vcgetmtdata(vc, Group_t*);
	/**/DEBUG_DECLARE(static int, N_grphuff) DEBUG_COUNT(N_grphuff);

	if(dtsz == 0)
		return 0;

	/* get the size of doing Huffman alone */
	CLRTABLE(freq,VCH_SIZE);
	ADDFREQ(freq, Vchobj_t*, data, dtsz);
	CLRTABLE(size,VCH_SIZE);
	vchsize(VCH_SIZE, freq, size, 0);
	DOTPRODUCT(grp->hufsz, freq, size, VCH_SIZE);
	grp->hufsz += 256*8; /* plus est. header cost */

	/* set desired part size and bounds for number of groups */
	ptsz = grpptsz(dtsz);

	/* initialize data structures for fast frequency calculations */
	if(grpinit(grp, (Vcchar_t*)data, dtsz, ptsz) < 0)
		return -1;

	/* now make the best grouping */
	if((itbl = vclogi(grp->npts)) > GRP_IMAX)
		itbl = GRP_IMAX;
	else if(itbl < GRP_IMIN)
		itbl = GRP_IMIN;
	/**/DEBUG_PRINT(2,"grphuff: dtsz=%d, ", dtsz); DEBUG_PRINT(2,"ptsz=%d, ", ptsz);
	/**/DEBUG_PRINT(2,"npts=%d, ", grp->npts); DEBUG_PRINT(2,"itbl=%d\n", itbl);
	grppart(grp, itbl, GRP_ITER);
	if(grp->ntbl == 0) /* single table, simplify */
	{	grpinit(grp, (Vcchar_t*)data, dtsz, dtsz);
		grppart(grp, 1, 1);
	}

	/* short-hands */
	part = grp->part; npts = grp->npts; ptsz = grp->ptsz;
	tbl = grp->tbl; ntbl = grp->ntbl;

	/* get space for output */
	n_output = (ntbl+1)*(2*VCH_SIZE) + (grp->cmpsz+7)/8; /* upper-bound for output size */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), n_output, 0)) )
		return -1;
	vcioinit(&io, output, n_output);

	vcioputu(&io, dtsz);
	vcioputu(&io, ntbl);
	vcioputu(&io, ptsz); /* the part size used */

	/* output the coding tables and compute the coding bits */
	for(k = 0; k < ntbl; ++k)
	{	vcioputc(&io, tbl[k].maxs);
		if(tbl[k].maxs == 0) /* coding a run */
			vcioputc(&io,tbl[k].runb);
		else /* coding a code table */
		{	dt = vcionext(&io); n = vciomore(&io);
			if((n = vchputcode(VCH_SIZE, tbl[k].size, tbl[k].maxs, dt, n)) < 0)
				return -1;
			else	vcioskip(&io, n);
			vchbits(VCH_SIZE, tbl[k].size, bits[k]);
		}
	}

	/* compress and output the indices */
	if((n = vcapply(grp->mtf, part, npts, &dt)) < 0 )
		return -1;
	if((n = vcapply(grp->huf, dt, n, &dt)) < 0 )
		return -1;
	vcioputu(&io,n);
	vcioputs(&io,dt,n);
	vcbuffer(grp->mtf, NIL(Vcchar_t*), -1, -1);
	vcbuffer(grp->huf, NIL(Vcchar_t*), -1, -1);

	/* now write out the encoded data */
	vciosetb(&io, b, n, VC_ENCODE);
	for(p = 0, i = 0; i < npts; i += 1, p += ptsz)
	{	if(tbl[part[i]].maxs == 0)
			continue;

		sz = tbl[part[i]].size;
		bt = bits[part[i]];
		dt = ((Vcchar_t*)data)+p;
		for(k = i == npts-1 ? dtsz-p : ptsz; k > 0; --k, ++dt)
			vcioaddb(&io, b, n, bt[*dt], sz[*dt]);
	}
	vcioendb(&io, b, n, VC_ENCODE);

	dt = output;
	n = vciosize(&io); /**/DEBUG_ASSERT(n <= n_output);
	if(vcrecode(vc, &output, &n, 0, 0) < 0 )
		return -1;
	if(dt != output)
		vcbuffer(vc, dt, -1, -1);

	if(out)
		*out = output;
	return n;
}

#if __STD_C
static ssize_t grpunhuff(Vcodex_t* vc, const Void_t* orig, size_t dtsz, Void_t** out)
#else
static ssize_t grpunhuff(vc, orig, dtsz, out)
Vcodex_t*	vc;	/* Vcodex handle		*/
Void_t*		orig;	/* data to be uncompressed	*/
size_t		dtsz;	/* data size			*/
Void_t**	out;	/* to return output buffer 	*/
#endif
{
	reg Vcbit_t	b;
	ssize_t		k, p, sz, n, ntop;
	short		*node, *size;
	ssize_t		npts, ptsz, ntbl;
	Vcchar_t	*part, *dt, *output, *o, *endo, *data;
	Table_t		tbl[GRP_NTBL];
	Vcbit_t		bits[GRP_NTBL][VCH_SIZE];
	Vchtrie_t	*trie[GRP_NTBL];
	Vcio_t		io;
	Group_t		*grp = vcgetmtdata(vc, Group_t*);
	int		rv = -1;

	if(dtsz == 0)
		return 0;

	data = (Vcchar_t*)orig; sz = (ssize_t)dtsz;
	if(vcrecode(vc, &data, &sz, 0, 0) < 0 )
		return -1;
	dtsz = sz;

	for(k = 0; k < GRP_NTBL; ++k)
		trie[k] = NIL(Vchtrie_t*);

	vcioinit(&io,data,dtsz);

	if((sz = (ssize_t)vciogetu(&io)) < 0)	/* size of decoded data */
		goto done;
	if((ntbl = (ssize_t)vciogetu(&io)) < 0)	/* # of coding tables	*/
		goto done;
	if((ptsz = (ssize_t)vciogetu(&io)) < 0)	/* size of each part	*/
		goto done;

	for(k = 0; k < ntbl; ++k)
	{	if((tbl[k].maxs = vciogetc(&io)) < 0) /* max code size	*/
			goto done;
		else if(tbl[k].maxs == 0) /* this is a run */
			tbl[k].runb = vciogetc(&io);
		else /* construct code table and trie for fast matching	*/
		{	dt = vcionext(&io); n = vciomore(&io);
			if((n = vchgetcode(VCH_SIZE, tbl[k].size, tbl[k].maxs, dt, n)) < 0)
				goto done;
			else	vcioskip(&io,n);

			vchbits(VCH_SIZE, tbl[k].size, bits[k]);
			if(!(trie[k] = vchbldtrie(VCH_SIZE, tbl[k].size, bits[k])) )
				goto done;
		}
	}

	/* reconstruct the array of part indices */
	if((n = vciogetu(&io)) < 0)
		goto done;
	dt = vcionext(&io); vcioskip(&io,n);
	if((n = vcapply(grp->huf, dt, n, &dt)) < 0)
		goto done;
	if((npts = vcapply(grp->mtf, dt, n, &part)) < 0)
		goto done;

	/* buffer to reconstruct the original data */
	if(!(output = vcbuffer(vc, (Vcchar_t*)0, sz, 0)) )
		goto done;
	endo = (o = output) + sz;

	vciosetb(&io, b, n, VC_DECODE);
	for(k = 0; k < npts; ++k)
	{	dt = o + (k == npts-1 ? endo-o : ptsz); /* end of this part */
		if(tbl[part[k]].maxs == 0) /* reconstruct a run */
		{	p = tbl[part[k]].runb;
			while(o < dt)
				*o++ = (Vcchar_t)p;
		}
		else /* reconstructing a Huffman-coded set of bytes */
		{	node = trie[part[k]]->node;
			size = trie[part[k]]->size;
			ntop = trie[part[k]]->ntop;
			for(sz = ntop, p = 0;;)
			{	vciofilb(&io, b, n, sz);

				p += (b >> (VC_BITSIZE-sz));
				if(size[p] > 0)
				{	vciodelb(&io, b, n, size[p]);
					*o = (Vcchar_t)node[p];
					if((o += 1) >= dt)
						break;
					sz = ntop; p = 0;
				}
				else if(size[p] == 0)
					return -1;
				else
				{	vciodelb(&io, b, n, sz);
					sz = -size[p]; p = node[p];
				}
			}
		}
	} /**/DEBUG_ASSERT(o == endo);
	vcioendb(&io, b, n, VC_DECODE);

	if(out)
		*out = output;
	rv = o-output;

done:	for(k = 0; k < GRP_NTBL; ++k)
		if(trie[k])
			vchdeltrie(trie[k]);

	vcbuffer(grp->huf, NIL(Vcchar_t*), -1, -1);
	vcbuffer(grp->mtf, NIL(Vcchar_t*), -1, -1);

	if(data != orig)
		vcbuffer(vc, data, -1, -1);

	return rv;
}

/* Event handler */
#if __STD_C
static int grpevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int grpevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Group_t	*grp;
	int	rv = -1;

	if(type == VC_OPENING)
	{	if(!(grp = (Group_t*)calloc(1, sizeof(Group_t))) )
			return -1;

		/* open the entropy coder handle */
		grp->huf = vcopen(NIL(Vcdisc_t*), Vchuffman, 0, 0, vc->flags);
		grp->mtf = vcopen(NIL(Vcdisc_t*), Vcmtf, 0, 0, vc->flags);
		if(!grp->huf || !grp->mtf )
			goto do_closing;

		vcsetmtdata(vc, grp);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	rv = 0;
	do_closing:
		if((grp = vcgetmtdata(vc, Group_t*)) )
		{	if(grp->huf)
				vcclose(grp->huf);
			if(grp->mtf)
				vcclose(grp->mtf);
			if(grp->part)
				free(grp->part);
			if(grp->ppos)
				free(grp->ppos);
			if(grp->obj)
				free(grp->obj);
			free(grp);
		}

		vcsetmtdata(vc, NIL(Group_t*));
		return rv;
	}
	else if(type == VC_FREEBUFFER)
	{	if((grp = vcgetmtdata(vc, Group_t*)) )
		{	if(grp->mtf)
				vcbuffer(grp->mtf, NIL(Vcchar_t*), -1, -1);
			if(grp->huf)
				vcbuffer(grp->huf, NIL(Vcchar_t*), -1, -1);
		}
		return 0;
	}
	else	return 0;
}

Vcmethod_t	_Vchuffgroup =
{	grphuff,
	grpunhuff,
	grpevent,
	"huffgroup", "Huffman encoding by groups.",
	"[-version?huffgroup (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	0,
	1024*1024,
	0
};

VCLIB(Vchuffgroup)
