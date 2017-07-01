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

/*	Partition variable length records into fixed-length records.
**
**	Written by Binh Dao Vo and Kiem-Phong Vo
*/

/* In the below, we assume that EOL will be different from both
** AMA_SIZE and AMA_HEAD. This is true for known character maps
** and MUST BE SO for any new one that might be put in use.
*/
#define EOL		('\n')	/* the new line character	*/
#define AMAEOL(x)	((x)&177)	/* \n is in low 7 bits	*/
#define AMA_TEXTLINE	EOL		/* exclude \n in output	*/
#define AMA_FULLLINE	(EOL | (1<<7))	/* include \n in output	*/

#define AMA_SIZE	2 /* AMA record size in first 2 bytes	*/
#define AMA_HEAD	4 /* AMA record header is 4 bytes long	*/

#define AMA_DFLT	AMA_SIZE	/* default coding	*/
#define AMA_ETOA	(1<<8)		/* decode EBCDIC->ASCII	*/

/* the below macros define the coding of record lengths */
#define MAXSIZE		(1 << 16) /* max allowed record size	*/
#define GETSIZE(dt)	(((dt)[0]<<8)+(dt)[1]) /* record size	*/
#define PUTSIZE(dt,v)	(((dt)[0] = (((v)>>8)&0377)), ((dt)[1] = ((v)&0377)) )

typedef struct _vartbl_s
{	int		type;	/* indicator byte		*/
	Vcodex_t*	map;	/* for EBCDIC->ASCII mapping	*/
	Dt_t*		szdt;	/* dictionary sorted by sizes	*/
} Vartbl_t;

typedef struct _tbl_s
{	Dtlink_t	link;
	ssize_t		size;	/* record size			*/
	ssize_t		count;	/* record count			*/
	ssize_t		dtsz;	/* data size			*/
	Vcchar_t*	data;	/* data buffer for all records	*/
	Vccontext_t*	ctxt;	/* context to invoke vc->coder	*/
} Tbl_t;

/* EBCDIC to ASCII mappings as defined in the Vcmap transform */
static char*	E2A[] = { "o2a", "h2a", "e2a", "i2a", "s2a" };

/* arguments to indicate whether or not newline-separated */
static Vcmtarg_t	_Amaargs[] =
{
	{ "etoa", "Mapping to ASCII text on decoding. 'etoa=[Vcmap type, default=o2a]'",
		(Void_t*)AMA_ETOA
	},
	{ "nl", "Records are text lines. 'nl=1' keeps \\n in output",
		(Void_t*)AMA_TEXTLINE
	},
	{ 0, "Each records starts with a 2-byte header defining its size",
		(Void_t*)AMA_DFLT
	}
};


#if __STD_C
static Void_t tblfree(Dt_t* dt, Void_t* obj, Dtdisc_t* disc)
#else
static Void_t tblfree(dt, obj, disc)
Dt_t*		dt;
Void_t*		obj;
Dtdisc_t*	disc;
#endif
{
	free(obj);
}

#if __STD_C
static int tblcmp(Dt_t* dt, Void_t* obj1, Void_t* obj2, Dtdisc_t* disc)
#else
static int tblcmp(dt, obj1, obj2, disc)
Dt_t*		dt;
Void_t*		obj1;
Void_t*		obj2;
Dtdisc_t*	disc;
#endif
{
	return (int)(*((ssize_t*)obj1)) - (int)(*((ssize_t*)obj2));
}

Dtdisc_t Tbldisc =
{	offsetof(Tbl_t,size), sizeof(ssize_t),
	offsetof(Tbl_t,link),
	0, tblfree, tblcmp, 0, 0, 0
};

#if __STD_C
static ssize_t amapart(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t amapart(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		zipsz, sz, z, ntbl;
	Vcchar_t	*tbldt, *zipdt, *dt, *enddt, *rc;
	char		schema[64];
	Dt_t		*szdt;
	Tbl_t		*tbl, *sztbl;
	Vartbl_t	*var;
	Vcio_t		io;

	if(!vc || !(var = vcgetmtdata(vc,Vartbl_t*)) )
		RETURN(-1);

	vc->undone = 0;

	/* ready the dictionary of sizes */
	szdt = var->szdt;
	for(tbl = dtfirst(szdt); tbl; tbl = dtnext(szdt,tbl))
	{	tbl->count = 0;
		tbl->dtsz  = 0;
		tbl->data  = NIL(Vcchar_t*);
	}

	/* make the entry to keep table of sizes */
	z = -1;
	if(!(sztbl = dtmatch(szdt,&z)) )
	{	if(!(sztbl = (Tbl_t*)malloc(sizeof(Tbl_t))) )
			RETURN(-1);
		sztbl->size = -1;
		sztbl->count = 0;
		sztbl->data = NIL(Void_t*);
		sztbl->dtsz = 0;
		sztbl->ctxt = NIL(Vccontext_t*);
		dtinsert(szdt, sztbl);
	}

	/* tally records of the same size */
	for(enddt = (dt = (Vcchar_t*)data) + size;; )
	{
		if(var->type == AMA_DFLT ) /* 2-byte record size header */
		{	if(dt+AMA_SIZE > enddt) /* partial record */
				z = -1;
			else if((z = GETSIZE(dt)) < AMA_SIZE || z >= MAXSIZE)
			{	vc->undone = size; /* corrupted data */
				RETURN(-1);
			}
			else if(z > (enddt-dt) ) /* partial record */
				z = -1;
		}
		else /* new-line delineated records */
		{	for(rc = dt; rc < enddt; ++rc)
				if(*rc == EOL )
					break;
			if(rc == enddt)
				z = -1;
			else if((z = (rc - dt) + 1) >= MAXSIZE)
			{	vc->undone = size; /* cannot handle this data */
				RETURN(-1);
			}
		}

		if(z < 0) /* stopping at a partial record */
		{	vc->undone = enddt - dt;
			size -= vc->undone;
			break;
		}
		else	dt += z; /* skip to next record */

		if(var->type == AMA_DFLT)
			z -= AMA_SIZE; /* strip the 2-byte size header */
		else if(var->type == AMA_TEXTLINE)
			z -= 1; /* strip the new line */
		/* else if(var->type == AMA_FULLLINE); */

		if((tbl = dtmatch(szdt, &z)) )
			tbl->count++;
		else
		{	/* create handles for new record lengths */
			if(!(tbl = (Tbl_t*)calloc(1,sizeof(Tbl_t))) )
				RETURN(-1);

			tbl->size = z;
			tbl->count = 1;
			tbl->data = NIL(Vcchar_t*);
			tbl->dtsz = 0;
			tbl->ctxt = NIL(Vccontext_t*); /* to create a new context */

			dtinsert(szdt, tbl);
		}
	}

	if(dt == (Vcchar_t*)data) /* no record seen */
	{	vc->undone = size;
		return 0;
	}

	/**/DEBUG_PRINT(2,"++++Raw size=%d\n",size);

	/* count records and groups and get sizes of rearranged data */
	sz = 0; ntbl = 1; /* 1 for sztbl */
	for(sz = 0, tbl = dtfirst(szdt); tbl; tbl = dtnext(szdt, tbl) )
	{	if(tbl->count <= 0 || tbl == sztbl)
			continue;
		ntbl += 1;
		sztbl->count += tbl->count;
		tbl->dtsz = tbl->size*tbl->count;
		sz += tbl->dtsz;
	}
	sztbl->dtsz = sztbl->count*2; /* record size is always 2 bytes */
	sz += sztbl->dtsz;

	/* allocate memory to each record group to rearrange data */
	if(!(tbldt = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		RETURN(-1);
	for(dt = tbldt, tbl = dtfirst(szdt); tbl; tbl = dtnext(szdt, tbl))
	{	if(tbl->count <= 0)
			continue;
		tbl->data = dt; dt += tbl->dtsz;
	} /**/DEBUG_ASSERT((dt-tbldt) == sz);

	/* now partition records into groups of same size */
	for(enddt = (dt = (Vcchar_t*)data)+size; dt < enddt; )
	{	if(var->type == AMA_DFLT)
			z = GETSIZE(dt);
		else
		{	for(rc = dt; rc < enddt; ++rc)
				if(*rc == EOL)
					break;
			z = (rc - dt) + 1;
		}

		PUTSIZE(sztbl->data, z); /* save FULL-LENGTH of record */
		sztbl->data += 2;

		if(var->type == AMA_DFLT) /* skip length data */
		{	dt += AMA_SIZE;
			z  -= AMA_SIZE;
		}
		else if(var->type == AMA_TEXTLINE) /* omit new-line */
			z -= 1;
		/* else if(var->type == AMA_FULLLINE); keep new-line */

		/* copy the record data to its group */
		if(!(tbl = dtmatch(szdt, &z)) )
			RETURN(-1);
		memcpy(tbl->data, dt, z);
		tbl->data += z;
		dt += z + (var->type == AMA_TEXTLINE ? 1 : 0);
	} /**/DEBUG_ASSERT(dt == enddt);

	/* continuation data processing and computing output size */
	zipsz = vcsizeu(ntbl) + 1; /* #tables + type */
	for(tbl = dtfirst(szdt); tbl; tbl = dtnext(szdt,tbl))
	{	if(tbl->count <= 0)
			continue;

		tbl->data -= tbl->dtsz; /* reset data pointer */
		if(vc->coder && tbl->dtsz > 0)
		{	tbl->ctxt = vcinitcontext(vc->coder, tbl->ctxt);

			/* pass data characteristics down to secondary coder */
			if(tbl != sztbl)
				z = tbl->size; /* common #columns for all records */
			else
			{	z = 2; /* #columns in the size table is always 2 */

				if(var->type == AMA_FULLLINE) /* pass the new-line character */
					vcsetmtarg(vc->coder,"rsep",TYPECAST(Void_t*,EOL),1);
			}

#if _PACKAGE_ast
			sfsprintf(schema, sizeof(schema), "[%d]", z); /* pass value as schema and #columns */
#else
			sprintf(schema, "[%d]", z); /* pass value as schema and #columns */
#endif
			vcsetmtarg(vc->coder, "schema", schema, 0);
			vcsetmtarg(vc->coder, "columns", TYPECAST(Void_t*,z), 2);

			if(vcrecode(vc, &tbl->data, &tbl->dtsz, 0, 0) < 0 )
				RETURN(-1);
		}

		zipsz += tbl == sztbl ? 0 : vcsizeu(tbl->size); /* record length */
		zipsz += vcsizeu(tbl->dtsz) + tbl->dtsz; /* total data size for table */
	}
		
	if(!(zipdt = vcbuffer(vc, NIL(Vcchar_t*), zipsz, 0)) )
		RETURN(-1);
	vcioinit(&io, zipdt, zipsz);

	vcioputu(&io, ntbl); /* # of tables */
	vcioputc(&io, var->type); /* data type indicator */

	/* output the table of sizes first */
	vcioputu(&io, sztbl->dtsz);
	vcioputs(&io, sztbl->data, sztbl->dtsz);
	ntbl -= 1;

	/* output the rest of the tables */
	for(tbl = dtfirst(szdt); tbl; tbl = dtnext(szdt,tbl))
	{	if(tbl->count <= 0 || tbl == sztbl)
			continue;

		vcioputu(&io, tbl->size); /* number of columns */
		vcioputu(&io, tbl->dtsz); /* size of processed data */
		vcioputs(&io, tbl->data, tbl->dtsz); /* data itself */
		ntbl -= 1;
	} /**/DEBUG_ASSERT(vciosize(&io) == zipsz && ntbl == 0);

	if(tbldt) /* free rearranged data */
		vcbuffer(vc, tbldt, 0, 0);

	/**/DEBUG_PRINT(2,"++++Coded size=%d\n",zipsz);
	if(out)
		*out = zipdt;
	return zipsz;
}

#if __STD_C
static ssize_t amaunpart(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t amaunpart(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*dt, *enddt, *mdt, *rawdt;
	int		eol;
	ssize_t		sz, ntbl;
	Vartbl_t	*var;
	Tbl_t		*tbl, *sztbl;
	Dt_t		*szdt;
	Vcio_t		io;

	vc->undone = 0;
	if(size <= 0)
		return 0;
	/**/DEBUG_PRINT(2, "++++Coded size=%d\n",size);

	if(!(var = vcgetmtdata(vc, Vartbl_t*)) )
		RETURN(-1);

	/* ready the dictionary of sizes */
	szdt = var->szdt;
	for(tbl = dtfirst(szdt); tbl; tbl = dtnext(szdt,tbl))
	{	tbl->count = 0;
		tbl->dtsz  = 0;
		tbl->data  = NIL(Vcchar_t*);
	}

	vcioinit(&io, data, size);

	if((ntbl = vciogetu(&io)) <= 0)
		RETURN(-1);

	var->type = vciogetc(&io);
	if(var->type == AMA_TEXTLINE || var->type == AMA_FULLLINE )
	{	if(var->map) /* var->type should be AMA_DFLT for record translation */
			RETURN(-1);
		eol = var->type & 0177; /* the EOL character used on encoding */
	}

	/* reconstruct the table of record sizes */
	sz = -1;
	if(!(sztbl = dtmatch(szdt, &sz)) )
	{	if(!(sztbl = (Tbl_t*)calloc(1, sizeof(Tbl_t))) )
			RETURN(-1);
		sztbl->count = 0;
		sztbl->size  = sz;
		sztbl->data  = NIL(Void_t*);
		sztbl->dtsz  = 0;
		sztbl->ctxt  = 0;
		dtinsert(szdt, sztbl);
	}
	if(vciomore(&io) <= 0 || (sztbl->dtsz = vciogetu(&io)) <= 0 )
		RETURN(-1);
	if(sztbl->dtsz > vciomore(&io))
		RETURN(-1);
	sztbl->data = vcionext(&io);
	vcioskip(&io, sztbl->dtsz);
	if(vc->coder)
	{	sztbl->ctxt = vcinitcontext(vc->coder, sztbl->ctxt);
		if(vcrecode(vc, &sztbl->data, &sztbl->dtsz, 0, 0) < 0 )
			RETURN(-1);
	}
	if((sztbl->dtsz % 2) != 0)
		RETURN(-1);
	sztbl->count = sztbl->dtsz / 2;

	for(size = 0, ntbl -= 1; ntbl > 0; --ntbl)
	{	if(vciomore(&io) <= 0 || (sz = vciogetu(&io)) < 0)
			RETURN(-1);

		if(!(tbl = dtmatch(szdt, &sz)) )
		{	if(!(tbl = (Tbl_t*)calloc(1, sizeof(Tbl_t))) )
				RETURN(-1);
			tbl->count = 0;
			tbl->size  = sz;
			tbl->data  = NIL(Void_t*);
			tbl->dtsz  = 0;
			tbl->ctxt  = 0;
			dtinsert(szdt, tbl);
		}

		if(vciomore(&io) <= 0 || (tbl->dtsz = vciogetu(&io)) < 0)
			RETURN(-1);
		if(tbl->dtsz > vciomore(&io))
			RETURN(-1);
		tbl->data = vcionext(&io);
		vcioskip(&io, tbl->dtsz);

		if(vc->coder && tbl->size > 0)
		{	if(!(tbl->ctxt = vcinitcontext(vc->coder, tbl->ctxt)) )
				RETURN(-1);
			if(vcrecode(vc, &tbl->data, &tbl->dtsz, 0, 0) < 0 )
				RETURN(-1);
		}

		if(tbl->size > 0)
			tbl->count = tbl->dtsz / tbl->size;
		else /* count 0-length records via sztbl */
		{	if(var->type == AMA_TEXTLINE)
				sz = 1;
			else if(var->type == AMA_DFLT)
				sz = AMA_SIZE;
			else	sz = -1; /* AMA_FULLLINE is never empty */

			tbl->count = 0; /* look for things matching size */
			for(enddt = (dt = sztbl->data) + sztbl->dtsz; dt < enddt; dt += 2)
				if(GETSIZE(dt) == sz)
					tbl->count += 1;
		}
		if(tbl->count*tbl->size != tbl->dtsz)
			RETURN(-1);

		size += tbl->dtsz;
		if(var->type == AMA_DFLT)
		{	if(!var->map)
				size += tbl->count*AMA_SIZE;
			else	size -= tbl->count; /* losing two 0's, adding \n */
		}
		else if(var->type == AMA_TEXTLINE)
			size += tbl->count;
		/* else if(var->type == AMA_FULLINE); */
	}

	if(!(rawdt = vcbuffer(vc, NIL(Vcchar_t*), size, 0)) )
		RETURN(-1);
	data = (Void_t*)rawdt;

	for(enddt = (dt = sztbl->data)+sztbl->dtsz; dt < enddt; dt += 2)
	{	/* get record size */
		sz = GETSIZE(dt);
		if(var->type == AMA_DFLT)
			sz -= AMA_SIZE;
		else if(var->type == AMA_TEXTLINE)
			sz -= 1;
		/* else if(var->type == AMA_FULLLINE); */
		if(sz < 0)
			RETURN(-1);

		/* find the table of this record */
		if(!(tbl = dtmatch(szdt, &sz)) )
			RETURN(-1);
		if((tbl->count -= 1) < 0)
			RETURN(-1);
		if((sztbl->count -= 1) < 0)
			RETURN(-1);

		if(var->type == AMA_TEXTLINE)
		{	memcpy(rawdt, tbl->data, tbl->size);
			rawdt += tbl->size;
			*rawdt = eol; rawdt += 1;
		}
		else if(var->type == AMA_FULLLINE)
		{	memcpy(rawdt, tbl->data, tbl->size);
			rawdt += tbl->size;
		}
		else if(!var->map) /* normal AMA records */
		{	sz += AMA_SIZE; /* 2-byte-header record in original format */
			PUTSIZE(rawdt,sz); rawdt += AMA_SIZE;
			memcpy(rawdt, tbl->data, tbl->size);
			rawdt += tbl->size;
		}
		else /* mapping 4-byte header EBCDIC records to text lines */
		{	memcpy(rawdt, tbl->data+2, tbl->size-2);
			if(vcapply(var->map, rawdt, tbl->size-2, &mdt) != tbl->size-2)
				RETURN(-1);
			if(mdt != rawdt) /* mapping was supposedly done in-place */
				RETURN(-1);
			rawdt += tbl->size-2;
			*rawdt = 012; rawdt += 1; /* add the ASCII \n */
		}

		tbl->data += tbl->size;
	}

	if(sztbl->count != 0 || (rawdt - (Vcchar_t*)data) != size)
		RETURN(-1);

	if(out)
		*out = (Void_t*)data;

	/**/DEBUG_PRINT(2, "++++Decoded size=%d\n",size);
	return (ssize_t)size;
}

#if __STD_C
static int amaevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int amaevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Vartbl_t	*var;
	Vcmtarg_t	*arg;
	char		*data, val[64];
	ssize_t		n;

	if(type == VC_OPENING)
	{	if(!(var = calloc(1, sizeof(Vartbl_t))) )
			return -1;

		var->type = AMA_DFLT; /* default is 2-byte header AMA records */

		if(!(var->szdt = dtopen(&Tbldisc, Dtset)) ) /* dictionary of groups */
		{ 	free(var);
			return -1;
		}

		vcsetmtdata(vc, var);
		goto vc_setarg;
	}
	else if(type == VC_SETMTARG)
	{	if(!(var = vcgetmtdata(vc, Vartbl_t*)) )
			return -1;
	vc_setarg:
		for(data = (char*)params; data; )
		{	arg = NIL(Vcmtarg_t*); val[0] = 0;
			data = vcgetmtarg(data, val, sizeof(val), _Amaargs, &arg);
			if(TYPECAST(int,arg->data) == AMA_TEXTLINE)
				var->type = val[0] == '1' ? AMA_FULLLINE : AMA_TEXTLINE;
			else if(TYPECAST(int,arg->data) == AMA_DFLT)
				var->type = AMA_DFLT;
			else if(TYPECAST(int,arg->data) == AMA_ETOA &&
				(vc->flags&VC_DECODE) ) /* EBCDIC->ASCII transform */
			{	if(!val[0])
					n = 0; /* default is "o2a", IBM OpenEdition */
				else
				{	for(n = 0; val[0] && E2A[n]; ++n)
						if(strcmp(val, E2A[n]) == 0)
							break;
					if(!E2A[n])
						return -1;
				}

				/* construct handle to map characters */
				var->type = AMA_DFLT;
				if(var->map)
					vcclose(var->map);
				if(!(var->map = vcopen(0, Vcmap, E2A[n], 0, VC_ENCODE)) )
					return -1;
				vcsetmtarg(var->map, "inplace", 0, 0);
			}
		}
	}
	else if(type == VC_CLOSING)
	{	if((var = vcgetmtdata(vc, Vartbl_t*)) )
		{	if(var->map)
				vcclose(var->map);
			if(var->szdt )
				dtclose(var->szdt);
			free(var);
		}

		vcsetmtdata(vc, NIL(Void_t*));
	}

	return 0;
}

Vcmethod_t _Vcama =
{	amapart,
	amaunpart,
	amaevent,
	"ama", "Partitioning AMA records or text lines into same-length groups.",
	"[-?\n@(#)$Id: vcodex-ama (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	_Amaargs,
	12*1024*1024,
	0
};

VCLIB(Vcama)
