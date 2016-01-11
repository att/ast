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

/*	Partition block-descriptor-word data into bdws and records.
**
**	Written by Kiem-Phong Vo
*/

/* the below macros define the coding of record lengths */
#define SIZE		4	/* #bytes coding a block size	*/
#define GETSIZE(dt)	(((dt)[0]<<8)+(dt)[1]) /* get blk size	*/
#define PUTSIZE(dt,v)	(((dt)[0] = (((v)>>8)&0377)), ((dt)[1] = ((v)&0377)) )

#if __STD_C
static ssize_t bdwpart(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t bdwpart(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		sz, nblk, blksz, zipsz;
	Vcchar_t	*dt, *enddt, *blkdt, *zipdt;
	Vcio_t		io;

	/* count processable blocks */
	vc->undone = 0;
	for(nblk = 0, enddt = (dt = (Vcchar_t*)data) + size;; dt += sz, nblk += 1)
	{
		if(dt+SIZE > enddt) /* partial block */
			sz  = -1;
		else if((sz = GETSIZE(dt)) < SIZE) /* corrupted data */
			RETURN(-1);
		else if(dt+sz > enddt) /* partial block */
			sz = -1;

		if(sz < 0) /* end processing at a bad/partial record */
		{	vc->undone = enddt - dt;
			size -= vc->undone;
			break;
		}
	}

	if(size == 0)
		return 0;
	/**/DEBUG_PRINT(2,"++++Raw size=%d\n",size);

	/* get memory to rearrange data */
	blksz = nblk*4;
	zipsz = vcsizeu(size) + vcsizeu(nblk) + vcsizeu(blksz) + size;
	if(!(zipdt = vcbuffer(vc, NIL(Vcchar_t*), zipsz, 0)) )
		RETURN(-1);

	vcioinit(&io, zipdt, zipsz);

	vcioputu(&io, size);
	vcioputu(&io, nblk);
	vcioputu(&io, blksz);
	blkdt = vcionext(&io);
	vcioskip(&io, blksz);

	/* do partitioning */
	for(enddt = (dt = (Vcchar_t*)data)+size; dt < enddt; dt += sz)
	{
		sz = GETSIZE(dt);
		PUTSIZE(blkdt, 4); /* make this look like ama data */
		blkdt += 2;
		PUTSIZE(blkdt, sz);
		blkdt += 2;

		dt += 4;
		sz -= 4;
		vcioputs(&io, dt, sz);
	} /**/DEBUG_ASSERT(dt == enddt);

	if(vc->coder)
	{	dt = blkdt;
		sz = size - blksz;
		blkdt -= blksz;

		if(vcrecode(vc, &blkdt, &blksz, 0, 0) < 0 )
			RETURN(-1);

		if(vcrecode(vc, &dt, &sz, 0, 0) < 0 )
			RETURN(-1);

		zipsz = blksz + sz + vcsizeu(size) + vcsizeu(nblk) + vcsizeu(blksz);
		if(vcioextent(&io) < zipsz)
		{	vcbuffer(vc, NIL(Vcchar_t*), -1, -1);
			if(!(zipdt = vcbuffer(vc, NIL(Vcchar_t*), zipsz, 0)) )
				RETURN(-1);
			vcioinit(&io, zipdt, zipsz);
		}
		else	vcioinit(&io, zipdt, zipsz);

		vcioputu(&io, size);
		vcioputu(&io, nblk);
		vcioputu(&io, blksz);
		vcioputs(&io, blkdt, blksz);
		vcioputs(&io, dt, sz);
	}
		
	/**/DEBUG_PRINT(2,"++++Coded size=%d\n",zipsz);
	if(out)
		*out = zipdt;
	return zipsz;
}

#if __STD_C
static ssize_t bdwunpart(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t bdwunpart(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcio_t		io;
	Vcchar_t	*blkdt, *dt, *rawdt;
	ssize_t		nblk, blksz, sz, dtsz, z, rawsz;

	vc->undone = 0;
	if(size <= 0)
		return 0;
	/**/DEBUG_PRINT(2, "++++Coded size=%d\n",size);

	vcioinit(&io, data, size);
	if((dtsz = vciogetu(&io)) <= 0)
		RETURN(-1);
	if((nblk = vciogetu(&io)) <= 0)
		RETURN(-1);
	if((blksz = vciogetu(&io)) <= 0)
		RETURN(-1);
	blkdt = vcionext(&io);
	vcioskip(&io, blksz);
	if((sz = vciomore(&io)) <= 0)
		RETURN(-1);
	dt = vcionext(&io);

	if(vc->coder)
	{	if(vcrecode(vc, &blkdt, &blksz, 0, 0) < 0 )
			RETURN(-1);

		if(vcrecode(vc, &dt, &sz, 0, 0) < 0 )
			RETURN(-1);
	}

	if(!(data = vcbuffer(vc, NIL(Vcchar_t*), dtsz, 0)) )
		RETURN(-1);
	rawdt = (Vcchar_t*)data; rawsz = dtsz;
	for(; nblk > 0; --nblk)
	{	if(GETSIZE(blkdt) != 4)
			RETURN(-1);
		blkdt += 2;
		if((z = GETSIZE(blkdt)) < 0)
			RETURN(-1);
		blkdt += 2;
		blksz -= 4;

		PUTSIZE(rawdt,z);
		rawdt[2] = rawdt[3] = 0;
		if(rawsz < z-4)
			RETURN(-1);
		memcpy(rawdt+SIZE, dt, z-4);
		rawdt += z;
		rawsz -= z;
		dt += z-4;
	}
	if(blksz != 0 || rawsz != 0)
		RETURN(-1);

	if(out)
		*out = (Void_t*)data;

	/**/DEBUG_PRINT(2, "++++Decoded size=%d\n",dtsz);
	return dtsz;
}

Vcmethod_t _Vcbdw =
{	bdwpart,
	bdwunpart,
	0,
	"bdw", "Partitioning AMA record groups.",
	"[-?\n@(#)$Id: vcodex-bdw (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	NIL(Vcmtarg_t*),
	12*1024*1024,
	0
};

VCLIB(Vcbdw)
