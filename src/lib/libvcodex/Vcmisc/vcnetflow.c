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

/* Tranform netflow data for better compression. The main idea is
** to collect data of same types together then use either the Vcbwt
** or the Vctable transform to boost compressibility.
**
** Versions 1, 5, 6 and 7 are simply handled by separating headers
** from records before transforming each data set by Vctable.
**
** Version 9 packets are complex as the format of a data record is
** described by some other type-defining record, i.e., template.
** Occasionally, a template flowset may be lost resulting the loss
** of the size of a data record. If there is sufficient data to try,
** an attempt will be made to estimate a missing record size via
** computing pseudo-periods in data using the function vcperiod().
**
** Written by Kiem-Phong Vo.
*/

#define V1_PACKET	1	/* packet ID for version 1	*/
#define V1_HEADER	16	/* length of version 1 header	*/
#define V1_RECORD	48	/* length of version 1 record	*/

#define V5_PACKET	5	/* packet ID for version 5	*/
#define V5_HEADER	24	/* length of version 5 header	*/
#define V5_RECORD	48	/* length of version 5 record	*/

#define V6_PACKET	6	/* packet ID for version 6	*/
#define V6_HEADER	24	/* length of version 6 header	*/
#define V6_RECORD	52	/* length of version 6 record	*/

#define V7_PACKET	7	/* packet ID for version 7	*/
#define V7_HEADER	24	/* length of version 7 header	*/
#define V7_RECORD	52	/* length of version 7 record	*/

#define V9_PACKET	9	/* packet ID for version 9	*/
#define	V9_HEADER	20	/* length of version 9 header	*/
#define V9_TEMPLATE	0	/* template flowset ID		*/
#define V9_OPTION	1	/* option flowset ID		*/
#define V9_DATA		256	/* data flowset ID are >= 256	*/

/* to decide to process all data at once or by parts */
#define V9_NPCKT	32	/* min required for by parts	*/
#define V9WHOLE(p)	((p) < V9_NPCKT)
#define V9_WHOLE	1	/* doing everything together	*/

/* the below deal with network encoding of 16-bit integers */
#define NETINT(d)	(((d)[0] << 8) + (d)[1] )
#define PUTINT(d,i)	(((d)[0] = ((i)>>8)&255), ((d)[1] = (i)&255) )
#define GETINT(d,i)	(((i) = NETINT(d)), ((d) += 2), (i) )
#define BADINT(d,ed)	((d) > ((ed)-2) ) /* not enough data */


typedef struct _nflrcrd_s /* to hold a set of netflow records */
{	Dtlink_t	link;	/* CDT dictionary holder	*/
	Vccontext_t*	ctxt;	/* context for table transform	*/
	ssize_t		id;	/* template ID			*/
	ssize_t		sz;	/* record length, if known	*/
	Vcchar_t*	data;	/* data array			*/
	ssize_t		dtsz;	/* length of data		*/
	ssize_t		dtbf;	/* length of buffer		*/
	Vcchar_t*	code;	/* tranformed data		*/
	ssize_t		cdsz;	/* size of transformed data	*/
} Nflrcrd_t;

typedef struct _netflow_s /* the internal netflow handle */
{	Vcodex_t*	bwt;	/* for general data transform	*/
	Vcodex_t*	tbl;	/* for fixed-length records	*/
	Dt_t*		rcdt;	/* dictionary of V9-records	*/
	Dtdisc_t	disc;	/* compare/make/free functions	*/
} Netflow_t;

#ifdef __STD_C /* copy data into the buffer of a record type */
static ssize_t rcrdcopy(Nflrcrd_t* rc, Vcchar_t* dt, ssize_t sz)
#else
static ssize_t rcrdcopy(rc, dt, sz)
Nflrcrd_t*	rc;	/* record to copy data into	*/
Vcchar_t*	dt;	/* data to be copied		*/
ssize_t		sz;	/* data size			*/
#endif
{
	ssize_t		z;

	if((z = rc->dtsz+sz) > rc->dtbf )
	{	z = ((z + 1024)/1024)*1024;
		if(!rc->data || rc->dtbf == 0)
			rc->data = (Vcchar_t*)malloc(z);
		else	rc->data = (Vcchar_t*)realloc(rc->data, z);
		if(!rc->data)
			return -1;
		rc->dtbf = z;
	}

	memcpy(rc->data+rc->dtsz, dt, sz);
	return (rc->dtsz += sz);
}

#if __STD_C /* check for a whole v9 packet. return size of padding and packet end */
static ssize_t v9whole(Netflow_t* nfl, Vcchar_t* dt, Vcchar_t* enddt, Vcchar_t** epckt)
#else
static ssize_t v9whole(nfl, dt, enddt, endpckt)
Netflow_t*	nfl;
Vcchar_t*	dt;	/* start of data to parse	*/
Vcchar_t*	enddt;	/* end of data			*/
Vcchar_t**	epckt;	/* to return good end of packet	*/
#endif
{
	ssize_t		id, sz, n, z;
	Nflrcrd_t	*rc, rcrd;
	Vcchar_t	*edt;

	if(dt > (enddt-V9_HEADER) || NETINT(dt) != V9_PACKET)
		return -1; /* corrupted data */

	for(*epckt = dt, dt += V9_HEADER;; *epckt = dt)
	{	if(BADINT(dt,enddt) || GETINT(dt,id) == V9_PACKET)
			return 0;
		if(BADINT(dt,enddt) || GETINT(dt,sz) < 0 )
			return 0;

		if(sz > 0) /* a flowset */
		{	if((edt = dt+sz-4) > enddt) /* flowset boundary */
				return 0;

			if(id == V9_TEMPLATE) /* set sizes of data records */
			{	while(dt < edt)
				{	/* template id and element count */
					if(BADINT(dt,edt) || GETINT(dt,id) < V9_DATA )
						break;
					if(BADINT(dt,edt) || GETINT(dt, n) < 0 )
						break;

					for(sz = 0; n > 0; --n) /* sum sizes of elts */
					{	dt += 2; /* skip the type */
						if(BADINT(dt,edt) || GETINT(dt,z) < 0 )
							break;
						sz += z;
					}

					rcrd.id = id; /* now set record size */
					if(!(rc = dtinsert(nfl->rcdt, &rcrd)) )
						break;
					rc->sz  = sz;
				}
			}

			dt = edt; /* skip to next flowset */
		}
		else /* end of flowsets. compute and return the zero-padding */
		{	if(id > 0) /* will be a bad start for next parse */
				return 0; /* but this one is ok */

			for(z = 4, *epckt = dt;; z += 4, *epckt = dt)
			{	if(BADINT(dt,enddt) || GETINT(dt,id) != 0 )
					return z;
				if(BADINT(dt,enddt) || GETINT(dt,sz) != 0 )
					return z;
			}
		}
	}
}

#if __STD_C /* remove zero-padding from data flowsets */
static void v9padding(Nflrcrd_t* rc, Nflrcrd_t* rcrd)
#else
static void v9padding(rc, rcrd)
Nflrcrd_t*	rc;	/* record set to be checked for padding	*/
Nflrcrd_t*	rcrd;	/* list of head data for all packets	*/
#endif
{
	Vcchar_t	*rdt, *erdt, *fdt, *cdt;
	ssize_t		fsz, pz, sz, cnt;
	/**/DEBUG_ASSERT(rcrd->dtsz%6 == 0);

	if(rc->sz == 0) /* guess a record size */
		rc->sz = vcperiod(rc->data, rc->dtsz);

	if(rc->sz <= 1) /* if template size is trivial */
	{	rc->sz = 0;
		return;
	}

	/* see if all data flowsets are well-formed with respect to zero-padding */
	cnt = 0; fdt = rc->data;
	for(erdt = (rdt = rcrd->data)+rcrd->dtsz; rdt < erdt; rdt += 6)
	{	if(NETINT(rdt) != rc->id)
			continue;
		fsz = NETINT(rdt+2) - 4; /* size of data flowset */
		pz = fsz%rc->sz; /* padding amount */
		sz = fsz - pz;
		cnt += sz/rc->sz; /* count # of records */
		for(fdt += sz; pz > 0; --pz)
			if(*fdt++ != 0)
				break;
		if(pz > 0) /* bad zero-padding */
		{	rc->sz = 0; /* no table transformation, just BWT */
			return;
		}
	}

	/* remove zero-paddings and code their lengths in header */
	fdt = cdt = rc->data;
	for(erdt = (rdt = rcrd->data)+rcrd->dtsz; rdt < erdt; rdt += 6)
	{	if(NETINT(rdt) != rc->id)
			continue;
		fsz = NETINT(rdt+2) - 4; /* size of data flowset */
		pz = fsz%rc->sz; /* padding amount */
		sz = fsz - pz;
		if(fdt != cdt) /* shift data to remove padding */
			memcpy(cdt, fdt, sz);
		cdt += sz;
		fdt += fsz;
		PUTINT(rdt+4,pz); /* code the padding size */
	}
	rc->dtsz = cdt - rc->data; /**/DEBUG_ASSERT(rc->dtsz%rc->sz == 0);
}

#if __STD_C /* transforming version 9 netflow data */
static ssize_t v9flow(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t v9flow(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*dt, *enddt, *epckt, pad[2];
	ssize_t		id, sz, hdr, npckt, whole;
	Nflrcrd_t	*rc, rcrd;
	Netflow_t	*nfl;
	Vcio_t		io;
	Vcchar_t	*output;
	ssize_t		outsz = -1;
	/**/DEBUG_DECLARE(ssize_t, pcktcnt) /**/DEBUG_SET(pcktcnt,0);

	nfl = vcgetmtdata(vc, Netflow_t*);
	memset(&rcrd, 0, sizeof(rcrd));
	for(npckt = 0, enddt = (dt = (Vcchar_t*)data)+size; dt < enddt; dt = epckt )
	{	/* make sure this is a well-defined packet */
		if((sz = v9whole(nfl, dt, enddt, &epckt)) < 0 )
			break; /**/DEBUG_COUNT(pcktcnt);
		PUTINT(pad, sz); /* zero-padding */
		npckt += 1; /* count number of processed packets */

		/* output top part of packet header data */
		if(rcrdcopy(&rcrd, dt, 4) < 0)
			goto done;
		if(rcrdcopy(&rcrd, pad, 2) < 0)
			goto done;

		rcrd.id = V9_PACKET; /* output rest of header */
		if(!(rc = dtinsert(nfl->rcdt, &rcrd)) )
			goto done;
		if(rcrdcopy(rc, dt+4, V9_HEADER-4) < 0 )
			goto done;

		pad[0] = pad[1] = 0; /* process flowset data */
		for(dt += V9_HEADER; dt < epckt; dt += sz-4)
		{	GETINT(dt, id); GETINT(dt, sz);
			if(id == 0 && sz == 0)
				break;

			/* header of this flowset */
			if(rcrdcopy(&rcrd, dt-4, 4) < 0 )
				goto done;
			if(rcrdcopy(&rcrd, pad, 2) < 0)
				goto done;

			rcrd.id = id; /* flowset data */
			if(!(rc = dtinsert(nfl->rcdt, &rcrd)) )
				goto done;
			if(rcrdcopy(rc, dt, sz-4) < 0)
				goto done;
		}
	} /**/DEBUG_PRINT(2,"pcktcnt=%d\n",pcktcnt);

	if(npckt == 0) /* no whole packet found */
		return 0;
	else /* set amount of data actually processed */
	{	vc->undone = size - (dt - (Vcchar_t*)data);
		size = dt - (Vcchar_t*)data;
	}

	whole = V9WHOLE(npckt) ? V9_WHOLE : 0;
	sz = 0; /* compute buffer size */
	for(rc = dtfirst(nfl->rcdt); rc; rc = dtnext(nfl->rcdt, rc))
	{	if(rc->id >= V9_DATA) /* remove padding as needed */ 
			v9padding(rc, &rcrd);

		if(whole) /* will BWT the whole thing together */
		{	rc->cdsz = rc->dtsz;
			rc->code = rc->data;
		}
		else /* processing by parts */
		{	if(rc->sz <= 0) /* use the bwt since column size is unknown */
				rc->cdsz = vcapply(nfl->bwt, rc->data, rc->dtsz, &rc->code);
			else /* use the table transform since # of columns is known */
			{	rc->ctxt = vcinitcontext(nfl->tbl, rc->ctxt);
				vcsetmtarg(nfl->tbl, "columns", TYPECAST(Void_t*,rc->sz), 2);
				rc->cdsz = vcapply(nfl->tbl, rc->data, rc->dtsz, &rc->code);
			}
			if(rc->cdsz < 0)
				return -1;
		}

		sz += vcsizeu(rc->id) + vcsizeu(rc->sz) + vcsizeu(rc->cdsz) + rc->cdsz;
	}

	if(whole) /* will bwt all together */
	{	rcrd.code = rcrd.data;
		rcrd.cdsz = rcrd.dtsz;
	}
	else /* use the table transform on this 2-D array */
	{	rcrd.ctxt = vcinitcontext(nfl->tbl, rcrd.ctxt);
		vcsetmtarg(nfl->tbl, "columns", TYPECAST(Void_t*, 6), 2);
		if((rcrd.cdsz = vcapply(nfl->tbl, rcrd.data, rcrd.dtsz, &rcrd.code)) < 0 )
			goto done;
	}

	hdr = vcsizeu(size) + 1 /* version # */ + 1 /* processing type */;
	sz += vcsizeu(rcrd.cdsz) + rcrd.cdsz;
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, hdr)) )
		goto done;
	vcioinit(&io, output, sz);

	vcioputu(&io, rcrd.cdsz); /* size of coded header data */
	vcioputs(&io, rcrd.code, rcrd.cdsz); /* coded header data */
	for(rc = dtfirst(nfl->rcdt); rc; rc = dtnext(nfl->rcdt, rc))
	{	vcioputu(&io, rc->id);
		vcioputu(&io, rc->sz);
		vcioputu(&io, rc->cdsz);
		vcioputs(&io, rc->code, rc->cdsz);
	} /**/DEBUG_ASSERT(vciosize(&io) == sz);

	if(whole)
	{	if((sz = vcapply(nfl->bwt, output, sz, &dt)) < 0)
			goto done;
		vcbuffer(vc, output, 0, 0); /* free current output */
		if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, hdr)) )
			goto done;
		memcpy(output, dt, sz);
	}

	output -= hdr; outsz = sz + hdr;
	if(out)
		*out = output;
	vcioinit(&io, output, outsz);
	vcioputu(&io, size);
	vcioputc(&io, V9_PACKET);
	vcioputc(&io, whole);

done:	if(rcrd.data)
		free(rcrd.data);
	if(rcrd.ctxt)
		vcfreecontext(vc, rcrd.ctxt);
	return outsz;
}

#if __STD_C /* undo the v9 coding */
static ssize_t unv9flow(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unv9flow(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*dt, *enddt, *output;
	ssize_t		id, sz, pz, pad, whole;
	Nflrcrd_t	*rc, rcrd;
	Vcio_t		io;
	Netflow_t	*nfl = vcgetmtdata(vc, Netflow_t*);

	memset(&rcrd, 0, sizeof(rcrd));

	vcioinit(&io, data, size);

	if(vciomore(&io) <= 0 || (sz = vciogetu(&io)) < 0 )
		return -1;
	if(sz == 0 || !out)
		return sz;

	if(vciomore(&io) <= 0 || vciogetc(&io) != V9_PACKET)
		return -1;

	if(vciomore(&io) <= 0 || (whole = vciogetc(&io)) < 0 )
		return -1;
	if(whole && whole != V9_WHOLE)
		return -1;

	if(whole) /* everything was processed with bwt together */
	{	dt = vcionext(&io); pz = vciomore(&io);
		if((pz = vcapply(nfl->bwt, dt, pz, &dt)) <= 0 )
			return -1;
		vcioinit(&io, dt, pz);
	}

	/* decode packet/flowset header data */
	if(vciomore(&io) <= 0 || (rcrd.cdsz = vciogetu(&io)) <= 0)
		return -1;
	if(vciomore(&io) < rcrd.cdsz || !(rcrd.code = vcionext(&io)) )
		return -1;
	vcioskip(&io, rcrd.cdsz);

	if(!whole)
	{	if((rcrd.cdsz = vcapply(nfl->tbl, rcrd.code, rcrd.cdsz, &rcrd.code)) < 0)
			return -1;
	}
	if(rcrd.cdsz%6 != 0)
		return -1;

	/* decode packet/flowset data */
	while(vciomore(&io) > 0)
	{	if((rcrd.id = vciogetu(&io)) < 0)
			return -1;
		if(!(rc = dtinsert(nfl->rcdt, &rcrd)) )
			return -1;
		if(vciomore(&io) <= 0 || (rc->sz = vciogetu(&io)) < 0 )
			return -1;
		if(vciomore(&io) <= 0 || (rc->cdsz = vciogetu(&io)) < 0 )
			return -1;
		if(vciomore(&io) < rc->cdsz || !(rc->code = vcionext(&io)) )
			return -1;
		vcioskip(&io, rc->cdsz);

		if(whole) /* already decoded */
			continue;

		if(rc->sz == 0)
		{	if((rc->cdsz = vcapply(nfl->bwt, rc->code, rc->cdsz, &rc->code)) < 0)
				return -1;
		}
		else
		{	if((rc->cdsz = vcapply(nfl->tbl, rc->code, rc->cdsz, &rc->code)) < 0)
				return -1;
			if((rc->cdsz % rc->sz) != 0 )
				return -1;
		}
	}

	/* now reassemble all pieces together */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;
	vcioinit(&io, output, sz);
	for(pad = 0, enddt = (dt = rcrd.code)+rcrd.cdsz; dt < enddt; )
	{	if(NETINT(dt) == V9_PACKET) /* finish up last packet */
		{	for(; pad > 0 && vciomore(&io) > 0; --pad)
				vcioputc(&io, 0);
			if(pad > 0)
				return -1;
		}
	
		if(vciomore(&io) < 4) /* write out the ID & SZ */
			return -1;
		vcioputs(&io, dt, 4);

		if(GETINT(dt, id) < 0) /* type of data */
			return -1;
		if(GETINT(dt, sz) < 0) /* size of data */
			return -1;
		if(GETINT(dt, pz) < 0) /* zero-padding */
			return -1;

		rcrd.id = id;
		if(!(rc = dtsearch(nfl->rcdt, &rcrd)) )
			return -1;

		if(id == V9_PACKET) /* starting a new packet */
		{	pad = pz; pz = 0;
			sz = V9_HEADER-4;
		}
		else
		{	sz -= 4;
			if(id >= V9_DATA && rc->sz > 0)
				sz = (sz/rc->sz)*rc->sz;
		}

		if(sz > rc->cdsz || sz > vciomore(&io) )
			return -1;
		vcioputs(&io, rc->code, sz);
		rc->code += sz;
		rc->cdsz -= sz;

		for(; pz > 0 && vciomore(&io) > 0; --pz)
			vcioputc(&io, 0);
		if(pz > 0)
			return -1;
	}
	for(; pad > 0 && vciomore(&io) > 0; --pad)
		vcioputc(&io, 0);
	if(pad > 0)
		return -1;

	if(vciomore(&io) != 0)
		return -1;

	*out = output;
	return vciosize(&io);
}

#if __STD_C /* transforming version 5&7 netflow data */
static ssize_t v1567flow(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t v1567flow(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*dt, *enddt, *pckt, *output;
	ssize_t		sz, id, fid, rcz, hdz;
	Nflrcrd_t	rchd, rcrd;
	Vcio_t		io;
	ssize_t		osz = -1;
	Netflow_t	*nfl = vcgetmtdata(vc, Netflow_t*);

	if(!(dt = (Vcchar_t*)data) || (size < V5_HEADER && size < V7_HEADER) )
		return 0;
	if((fid = NETINT(dt)) == V1_PACKET )
		{ hdz = V1_HEADER; rcz = V1_RECORD; }
	else if(fid == V5_PACKET )
		{ hdz = V5_HEADER; rcz = V5_RECORD; }
	else if(fid == V6_PACKET )
		{ hdz = V6_HEADER; rcz = V6_RECORD; }
	else if(fid == V7_PACKET )
		{ hdz = V7_HEADER; rcz = V7_RECORD; }
	else	return 0;

	memset(&rchd, 0, sizeof(rchd));
	memset(&rcrd, 0, sizeof(rcrd));

	enddt = (dt = (Vcchar_t*)data)+size;
	while((pckt = dt) < enddt)
	{	if(BADINT(dt,enddt) || GETINT(dt,id) != fid )
			break;
		if(BADINT(dt,enddt) || GETINT(dt,sz) < 0 )
			break;
		if((dt += hdz-4) > enddt)
			break;
		if((dt + (sz *= rcz) ) > enddt)
			break;

		/* separate header data from record data */
		if(rcrdcopy(&rchd, dt-hdz, hdz) < 0 )
			goto done;
		if(rcrdcopy(&rcrd, dt, sz) < 0 )
			goto done;
		dt += sz;
	}

	if(pckt == (Vcchar_t*)data)
		return 0;
	else
	{	vc->undone = size - (pckt - (Vcchar_t*)data);
		size = pckt - (Vcchar_t*)data;
	}

	/* continuation processing of header data */
	vcsetmtarg(nfl->tbl, "columns", TYPECAST(Void_t*,hdz), 2);
	if((rchd.cdsz = vcapply(nfl->tbl, rchd.data, rchd.dtsz, &rchd.code)) < 0)
		goto done;

	/* continuation processing of record data */
	vcsetmtarg(nfl->tbl, "columns", TYPECAST(Void_t*,rcz), 2);
	if((rcrd.cdsz = vcapply(nfl->tbl, rcrd.data, rcrd.dtsz, &rcrd.code)) < 0)
		goto done;

	sz = vcsizeu(size) + 1 + vcsizeu(rchd.cdsz) + rchd.cdsz + vcsizeu(rcrd.cdsz) + rcrd.cdsz;
	if(out)
	{	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
			goto done;
		*out = output;
		vcioinit(&io, output, sz);
		vcioputu(&io, size);
		vcioputc(&io, fid);
		vcioputu(&io, rchd.cdsz);
		vcioputs(&io, rchd.code, rchd.cdsz);
		vcioputu(&io, rcrd.cdsz);
		vcioputs(&io, rcrd.code, rcrd.cdsz);
	}
	osz = sz;

done:	if(rchd.data)
		free(rchd.data);
	if(rcrd.data)
		free(rcrd.data);
	return osz;
}

#if __STD_C /* undo version 5&7 netflow coding */
static ssize_t unv1567flow(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unv1567flow(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		id, sz, rcz, hdz, tid;
	Vcchar_t	*dt, *enddt, *output;
	Vcio_t		io;
	Nflrcrd_t	rcrd, rchd;
	Netflow_t	*nfl = vcgetmtdata(vc, Netflow_t*);

	memset(&rcrd, 0, sizeof(rcrd));
	memset(&rchd, 0, sizeof(rchd));

	vcioinit(&io, data, size);

	if(vciomore(&io) <= 0 || (sz = vciogetu(&io)) < 0 )
		return -1;
	if(sz == 0 || !out)
		return sz;

	if(vciomore(&io) <= 0 || (tid = vciogetc(&io)) < 0)
		return -1;
	if(tid == V1_PACKET)
		{ hdz = V1_HEADER; rcz = V1_RECORD; }
	else if(tid == V5_PACKET)
		{ hdz = V5_HEADER; rcz = V5_RECORD; }
	else if(tid == V6_PACKET)
		{ hdz = V6_HEADER; rcz = V6_RECORD; }
	else if(tid == V7_PACKET)
		{ hdz = V7_HEADER; rcz = V7_RECORD; }
	else	return -1;

	if(vciomore(&io) <= 0 || (rchd.cdsz = vciogetu(&io)) < 0)
		return -1;
	if(vciomore(&io) < rchd.cdsz || !(rchd.code = vcionext(&io)) )
		return -1;
	vcioskip(&io, rchd.cdsz);
	if((rchd.cdsz = vcapply(nfl->tbl, rchd.code, rchd.cdsz, &rchd.code)) < 0)
		return -1;
	if((rchd.cdsz%hdz) != 0)
		return -1;

	if(vciomore(&io) <= 0 || (rcrd.cdsz = vciogetu(&io)) < 0)
		return -1;
	if(vciomore(&io) < rcrd.cdsz || !(rcrd.code = vcionext(&io)) )
		return -1;
	vcioskip(&io, rcrd.cdsz);
	if((rcrd.cdsz = vcapply(nfl->tbl, rcrd.code, rcrd.cdsz, &rcrd.code)) < 0)
		return -1;
	if((rcrd.cdsz%rcz) != 0)
		return -1;

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;
	vcioinit(&io, output, sz);

	for(enddt = (dt = rchd.code)+rchd.cdsz; dt < enddt; dt += hdz-4)
	{	if(vciomore(&io) < hdz)
			return -1;
		vcioputs(&io, dt, hdz);

		if(GETINT(dt, id) != tid)
			return -1;
		if(GETINT(dt, sz) < 0)
			return -1;

		sz *= rcz;
		if(sz > rcrd.cdsz || sz > vciomore(&io))
			return -1;
		vcioputs(&io, rcrd.code, sz);
		rcrd.code += sz;
		rcrd.cdsz -= sz;
	}
	if(vciomore(&io) != 0)
		return -1;

	*out = output;
	return vciosize(&io);
}

#if __STD_C
static ssize_t netflow(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t netflow(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		sz, cdsz;
	Vcchar_t	*code, *output;
	Netflow_t	*nfl;
	Vcio_t		io;

	if(!vc || !(nfl = vcgetmtdata(vc,Netflow_t*)) )
		return -1;

	vc->undone = 0;
	if(size == 0)
		return 0;

	if((sz = v9flow(vc, data, size, out)) == 0) /* version 9? */
		sz = v1567flow(vc, data, size, out); /* version 1,5,6,7? */

	if(sz == 0) /* not recognizable netflow data, just run bwt on it */
	{	if((cdsz = vcapply(nfl->bwt, data, size, &code)) < 0)
			return cdsz;

		sz = vcsizeu(size) + 1 + cdsz;
		if(out)
		{	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
				return -1;
			*out = output;
			vcioinit(&io, output, sz);
			vcioputu(&io, size);
			vcioputc(&io, 0); /* not netflow data */
			vcioputs(&io, code, cdsz);
		}
	}

	dtclear(nfl->rcdt); /* clear existing buffers */
	vcbuffer(nfl->bwt, NIL(Vcchar_t*), -1, -1);
	vcbuffer(nfl->tbl, NIL(Vcchar_t*), -1, -1);

	return sz;
}

#if __STD_C
static ssize_t unnetflow(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unnetflow(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output, *code;
	ssize_t		sz, id, cdsz;
	Netflow_t	*nfl;
	Vcio_t		io;

	if(!vc || !(nfl = vcgetmtdata(vc, Netflow_t*)) )
		return -1;
	if(size == 0)
		return 0;

	dtclear(nfl->rcdt);
	vcbuffer(nfl->bwt, NIL(Vcchar_t*), -1, -1);
	vcbuffer(nfl->tbl, NIL(Vcchar_t*), -1, -1);

	vcioinit(&io, data, size);
	if(vciomore(&io) <= 0 || (sz = vciogetu(&io)) < 0 )
		return -1;
	if(vciomore(&io) <= 0 || (id = vciogetc(&io)) < 0 )
		return -1;

	if(id == V9_PACKET)
		return unv9flow(vc, data, size, out);
	else if(id == V1_PACKET || id == V5_PACKET || id == V6_PACKET || id == V7_HEADER)
		return unv1567flow(vc, data, size, out);
	else if(id != 0)
		return -1;

	code = vcionext(&io);
	cdsz = vciomore(&io);
	if((cdsz = vcapply(nfl->bwt, code, cdsz, &output)) < 0)
		return -1; /**/DEBUG_ASSERT(cdsz == sz);

	if(out)
		*out = output;
	return cdsz;
}

/* CDT discipline functions to maintain V9 records by their IDs */
#if __STD_C
static int rcrdcmp(Dt_t* dt, Void_t* one, Void_t* two, Dtdisc_t* disc)
#else
static int rcrdcmp(dt, one, two, disc)
Dt_t*		dt;
Void_t*		one;
Void_t*		two;
Dtdisc_t*	disc;
#endif
{
	return (int)(((Nflrcrd_t*)one)->id - ((Nflrcrd_t*)two)->id );
}

#if __STD_C
static Void_t* rcrdmake(Dt_t* dt, Void_t* arg, Dtdisc_t* disc)
#else
static Void_t* rcrdmake(dt, arg, disc)
Dt_t*		dt;
Void_t*		arg;
Dtdisc_t*	disc;
#endif
{
	Nflrcrd_t	*rc;

	if(!(rc = (Nflrcrd_t*)calloc(1, sizeof(Nflrcrd_t))) )
		return NIL(Void_t*);
	rc->id = ((Nflrcrd_t*)arg)->id;
	return (Void_t*)rc;
}

#if __STD_C
static void rcrdfree(Dt_t* dt, Void_t* arg, Dtdisc_t* disc)
#else
static void rcrdfree(dt, arg, disc)
Dt_t*		dt;
Void_t*		arg;
Dtdisc_t*	disc;
#endif
{
	Nflrcrd_t	*rc = (Nflrcrd_t*)arg;

	if(rc->data)
		free(rc->data);
	free(rc);
}

#if __STD_C /* to create/delete internal structures */
static int nflevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int nflevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Netflow_t	*nfl;
	int		rv = -1;

	if(type == VC_OPENING)
	{	if(!(nfl = (Netflow_t*)calloc(1, sizeof(Netflow_t))) )
			return -1;

		/* handles to transform collected data */
		type = vc->flags & (VC_ENCODE|VC_DECODE);
		if(!(nfl->tbl = vcopen(0, Vctable, 0, vc->coder, type)) )
			goto do_close;
		if(!(nfl->bwt = vcopen(0, Vcbwt, 0, vc->coder, type)) )
			goto do_close;

		/* dictionary to collection data of different types */
		DTDISC(&nfl->disc, 0, 0, 0, rcrdmake, rcrdfree, rcrdcmp, 0, 0, 0);
		if(!(nfl->rcdt = dtopen(&nfl->disc, Dtoset)) )
			goto do_close;

		vcsetmtdata(vc, nfl);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if(!(nfl = vcgetmtdata(vc, Netflow_t*)) )
			return -1;
		rv = 0; /* normal closing */
	do_close: /* free resources */
		if(nfl->tbl)
			vcclose(nfl->tbl);
		if(nfl->bwt)
			vcclose(nfl->bwt);
		if(nfl->rcdt)
			dtclose(nfl->rcdt);
		return rv;
	}
	else	return 0;
}

Vcmethod_t	_Vcnetflow =
{	netflow,
	unnetflow,
	nflevent,
	"netflow", "Netflow data from Cisco routers.",
	"[-version?netflow (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	0,
	4*1024*1024,
	0
};

VCLIB(Vcnetflow)
