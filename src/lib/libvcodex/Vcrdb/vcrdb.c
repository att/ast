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
#include	<vgraph.h>
#include	<vcrdb.h>

/*	Transform a relational dataset to make it more compressible.
**
**	Written by Kiem-Phong Vo (04/10/2006).
*/

#if DEBUG
#include	<fcntl.h>
writefile(Vcchar_t* data, ssize_t size)
{	int	fd = open("xxx", O_WRONLY|O_CREAT|O_TRUNC, 0644);
	write(fd, data, size);
	close(fd);
}
#endif

/* the below 8 bits are reserved for persistent code */
#define RDB_STATES	(0000377)
#define RDB_SLASH	(0000001) /* string of form x.y.../z	*/
				  /* Note that in old rdb data,	*/
				  /* x.y... alone is signaled	*/
				  /* by info.flen == 0 and	*/
				  /* coded length > 0.		*/
#define RDB_DOT		(0000002) /* x.y... format 		*/

#define RDB_PAD		(0000010) /* pad field to full length	*/
#define RDB_WHOLE	(0000020) /* keep table whole		*/
#define RDB_PLAIN	(0000040) /* no field transformation	*/

#define RDB_BWT		(0001000) /* use BWT for transforming	*/
#if 0
#define RDB_BDV		(0002000) /* test Binh's data sorting	*/
#endif

#define RDB_FSEP	(0010000) /* setting field separator	*/
#define RDB_RSEP	(0020000) /* setting record separator	*/
#define RDB_SCHEMA	(0040000) /* defining a schema		*/
#define RDB_ALIGN	(0100000) /* alignment for fields	*/


#define RDB_HEADER	(0100000) /* should be coded in header	*/

#define RDB_DOTCHAR	('.')	/* the dot character		*/
#define RDB_SLASHCHAR	('/')	/* the slash character		*/

typedef struct _rdbfield_s
{	Vccontext_t*	ctxt;	/* field ctxt in 2nd-ary coder	*/
	int		type;	/* field type in unrdb()	*/
	Vcchar_t*	data;	/* data after transformation	*/
	ssize_t		dtsz;	/* size of data			*/
} Rdbfield_t;

typedef struct _rdbctxt_s /* training context data */
{	Vccontext_t	ctxt;	/* inheritance from Vccontext_t	*/
	Vccontext_t*	tctxt;	/* table ctxt in 2nd-ary coder	*/
	Vcrdinfo_t	info;	/* table characteristics	*/
	Vcrdplan_t*	plan;	/* transform plan for context	*/
	Rdbfield_t*	fld;	/* data for fields		*/
	ssize_t		algn;	/* field alignment, if any	*/
	ssize_t		rawz;	/* before compression		*/
	ssize_t		cmpz;	/* after compression		*/
} Rdbctxt_t;

typedef struct _rdb_s
{	Rdbctxt_t*	ctxt;	/* data for default context 	*/
	Vcodex_t*	vcw;	/* to weight transformed data	*/
	int		type;	/* type of transformation	*/
} Rdb_t;

/* function to get field length from the context structure */
#define FLEN(rc,f)	((rc)->info.flen ?  (rc)->info.flen[f] : 0 )

/* arguments to set separators */
static Vcmtarg_t	_Rdbargs[] =
{	{ "fsep", "Field separator is defined as 'fsep=character'", (Void_t*)RDB_FSEP },
	{ "rsep", "Record separator is defined as 'rsep=character'", (Void_t*)RDB_RSEP },
	{ "schema", "Schema is defined as 'schema=[fldlen1,fldlen2,...]'", (Void_t*)RDB_SCHEMA },
	{ "align", "Field alignment is given as 'align=value'", (Void_t*)RDB_ALIGN },
	{ "plain", "No reordering of field data based on dependency", (Void_t*)RDB_PLAIN },
	{ "pad", "Padding fields to uniform lengths", (Void_t*)RDB_PAD },
	{ "whole", "Whole table is output or for secondary procesing", (Void_t*)RDB_WHOLE },
	{ "bwt", "Using BWT to compute transformation plan", (Void_t*)RDB_BWT },
#if RDB_BDV
	{ "bdv", "Binh's field sorting method", (Void_t*)RDB_BDV },
#endif
	{ 0, "Field and record separators to be computed", (Void_t*)0 }
};

#if __STD_C
static ssize_t rdb(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t rdb(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output, *dt, *whdt;
	ssize_t		i, f, z, cmpz, recz, whsz;
	int		type;
	Vcio_t		io;
	Rdbctxt_t	*rc;
	Rdb_t		*rdb;
	Vcrdtable_t	*tbl = NIL(Vcrdtable_t*);
	ssize_t		rv = -1;
	/**/DEBUG_DECLARE(static int, N_rdb) DEBUG_COUNT(N_rdb);

	if(size <= 0 )
		return 0;
	vc->undone = size; /* start with doing nothing */

	if(!(rdb = vcgetmtdata(vc, Rdb_t*)) || !(rc = vcgetcontext(vc, Rdbctxt_t*)) )
		GOTO(done);

	/* open the compressor to compute weights when constructing a plan */
	if(!rdb->vcw && !(rdb->type & RDB_PLAIN) )
	{	if(!(rdb->type & RDB_BWT) )
			rdb->vcw = VCRD_MATCH;
		else
		{	Vcodex_t*	vcw;
			if(!(vcw = vcopen(0, Vchuffman, 0, 0, VC_ENCODE)) )
				GOTO(done);
			else	rdb->vcw = vcw;
			if(!(vcw = vcopen(0, Vcrle, "0", rdb->vcw, VC_ENCODE|VC_CLOSECODER)) )
				GOTO(done);
			else	rdb->vcw = vcw;
			if(!(vcw = vcopen(0, Vcmtf, "0", rdb->vcw, VC_ENCODE|VC_CLOSECODER)) )
				GOTO(done);
			else	rdb->vcw = vcw;
			if(!(vcw = vcopen(0, Vcbwt, 0, rdb->vcw, VC_ENCODE|VC_CLOSECODER)) )
				GOTO(done);
			else	rdb->vcw = vcw;
		}
	}

	if(rc->info.fldn <= 0 && (rc->info.rsep <= 0 || rc->info.fsep <= 0) )
	{	Vcrdformat_t	*rdf;
		if(!(rdf = vcrdformat((Vcchar_t*)data, size, rc->info.rsep, rc->algn, 1)) )
			GOTO(done);

		if(rdf->fsep > 0 && rdf->rsep > 0)
		{	rc->info.fsep = rdf->fsep;
			rc->info.rsep = rdf->rsep;
		}
		else if(rdf->fldn > 0 && rdf->fldz)
		{	if(rc->info.flen)
				free(rc->info.flen);
			if((rc->info.flen = (ssize_t*)malloc(rdf->fldn*sizeof(ssize_t))) )
			{	memcpy(rc->info.flen, rdf->fldz, rdf->fldn*sizeof(ssize_t));
				rc->info.fldn = rdf->fldn;
			}
		}
		free(rdf);

		if(rc->info.fldn <= 0 && (rc->info.rsep <= 0 || rc->info.fsep <= 0) )
			GOTO(done); /* data is not in a relational format */
	}

	recz = 0; /* record length if fixed-length records */
	if(rc->info.fldn > 0 && rc->info.flen ) 
	{	for(f = 0; f < rc->info.fldn; ++f)
		{	if(rc->info.flen[f] <= 0) /* field length must be positive */
				GOTO(done);
			recz += rc->info.flen[f];
		}
	}

	/* construct table */
	if(!(tbl = vcrdparse(&rc->info, (Vcchar_t*)data, (ssize_t)size, 0)) )
		return 0;
	vc->undone -= (size = (size_t)vcrdsize(tbl));

	/* do field conversion as requested */
	for(f = 0; f < tbl->fldn; ++f)
	{	if(!(tbl->fld[f].type & VCRD_FIXED) )
			vcrdattrs(tbl, f, VCRD_DOT, 1); /* try to convert IP data into bytes */
		if(!(tbl->fld[f].type & VCRD_FIXED) && (rdb->type & RDB_PAD) )
			vcrdattrs(tbl, f, VCRD_PAD, 1); /* pad fields to full length */
	}

	if(rc->plan && rc->plan->fldn != tbl->fldn) /* plan is outdated */
	{	vcrdfreeplan(rc->plan);
		rc->plan = NIL(Vcrdplan_t*);
		rc->rawz = rc->cmpz = 0;

		if(rc->fld) /* redo field structures */
		{	if(vc->coder) /* delete field contexts in secondary coder */
				for(f = 0; f < tbl->fldn; ++f)
					if(rc->fld[f].ctxt)
						vcfreecontext(vc->coder, rc->fld[f].ctxt);
			free(rc->fld);
			rc->fld = NIL(Rdbfield_t*);
		}
	}

	if(!rc->plan) /* need a tranformation plan */
	{	if(rdb->type&RDB_PLAIN)
			rc->plan = vcrdmakeplan(tbl, NIL(Vcodex_t*));
		else	rc->plan = vcrdmakeplan(tbl, rdb->vcw);
		if(!rc->plan)
			goto done;
	}

	if(!(rdb->type&RDB_PLAIN) ) /* transform data */
	{	if(vcrdexecplan(tbl, rc->plan, VC_ENCODE) < 0 )
			GOTO(done);
	}

	cmpz = 0; /* recoding/compressing transformed data */
	for(f = 0; f < tbl->fldn; ++f)
	{	z = (tbl->fld[f].type&(VCRD_FIXED|VCRD_DOT|VCRD_SLASH|VCRD_PAD)) ? tbl->fld[f].maxz : 0;
		cmpz +=	vcsizeu(rc->plan->pred[f]) + /* predictor of field */
			vcsizeu(z) + /* field length if fixed-length field, 0 otherwise */ 
			1; /* indicator RDB_SLASH, RDB_PAD, ... */

		if(rdb->type&RDB_WHOLE) /* code whole table as one */
			continue;

		if(!rc->fld && !(rc->fld = (Rdbfield_t*)calloc(tbl->fldn, sizeof(Rdbfield_t))) )
			GOTO(done);

		/* coding each field separately */
		if((z = vcrdextract(tbl, f, NIL(Vcchar_t*), 0, VCRD_FIELD)) <= 0)
			GOTO(done);
		if(!(dt = vcbuffer(vc, NIL(Vcchar_t*), z, 0)) )
			GOTO(done);
#if RDB_BDV
		if(rdb->type & RDB_BDV)
		{	if(vcrdvector(tbl, f, dt, z, VC_ENCODE) < 0 || vcrdsize(tbl) != z)
				GOTO(done);
		}
		else
#endif
		{	if(vcrdextract(tbl, f, dt, z, VCRD_FIELD) != z)
				GOTO(done);
		}

		rc->fld[f].data = dt; /* field data and size */
		rc->fld[f].dtsz = z;

		if(vc->coder) /* recoding data if required */
		{	rc->fld[f].ctxt = vcinitcontext(vc->coder, rc->fld[f].ctxt);
			if(FLEN(rc,f) > 0 || (rdb->type&RDB_PAD) || (tbl->fld[f].type&VCRD_FIXED) )
			{	vcsetmtarg(vc->coder, "columns",
					   TYPECAST(Void_t*,tbl->fld[f].maxz), 2);
			}
			else
			{	if(rc->info.rsep >= 0)
					vcsetmtarg(vc->coder, "rsep",
						   TYPECAST(Void_t*,rc->info.rsep), 1);
				if(rc->info.fsep >= 0)
					vcsetmtarg(vc->coder, "fsep",
						   TYPECAST(Void_t*,rc->info.fsep), 1);
			}
			if(vcrecode(vc, &rc->fld[f].data, &rc->fld[f].dtsz, 0, 0) < 0 )
				GOTO(done);
			if(rc->fld[f].data != dt) /* free unused memory */
				vcbuffer(vc, dt, -1, 0);
		}

		cmpz +=	vcsizeu(rc->fld[f].dtsz) + rc->fld[f].dtsz; /* data */
	}

	if(rdb->type & RDB_WHOLE) /* process whole table */
	{	type = VCRD_RECORD | VCRD_PAD; /* always padding */

		if((whsz = vcrdextract(tbl, -1, NIL(Vcchar_t*), 0, type)) <= 0 )
			GOTO(done);
		if(!(whdt = dt = vcbuffer(vc, NIL(Vcchar_t*), whsz, 0)) )
			GOTO(done);
		if(vcrdextract(tbl, -1, whdt, whsz, type) != whsz)
			GOTO(done);
		if(vc->coder)
		{	rc->tctxt = vcinitcontext(vc->coder, rc->tctxt);

			if(recz > 0 || (rdb->type&RDB_PAD) )
			{	for(z = 0, f = 0; f < tbl->fldn; ++f)
					z += tbl->fld[f].maxz;
				vcsetmtarg(vc->coder, "columns", TYPECAST(Void_t*,z), 2);
			}
			else
			{	if(rc->info.rsep >= 0)
					vcsetmtarg(vc->coder, "rsep",
						   TYPECAST(Void_t*,rc->info.rsep), 1);
				if(rc->info.fsep >= 0)
					vcsetmtarg(vc->coder, "fsep",
						   TYPECAST(Void_t*,rc->info.fsep), 1);
			}
			if(vcrecode(vc, &whdt, &whsz, 0, 0) < 0 )
				GOTO(done);
			if(whdt != dt) /* free unused memory */
				vcbuffer(vc, dt, -1, 0);
		}

		cmpz += vcsizeu(whsz) + whsz;
	}

	cmpz += vcsizeu(size) + /* size of original data */
		vcsizeu(tbl->recn) + /* number of records */
		vcsizeu(tbl->fldn) + /* number of fields */
		vcsizeu(recz);  /* >0 if fixed fields */

	if(recz > 0) /* schematic data */
	{	for(f = 0; f < tbl->fldn; ++f)
			cmpz += vcsizeu(rc->info.flen[f]);
	}
	else /* variable fields */
	{	cmpz +=	1 + 1 + /* fsep and rsep */
	      		1 + 1 + 10; /* dot, slash and digits */
	}

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), cmpz, 0)) )
		GOTO(done);
	vcioinit(&io, output, cmpz);

	vcioputu(&io, size); /* size of raw data	*/
	vcioputu(&io, tbl->recn); /* # of records	*/
	vcioputu(&io, tbl->fldn); /* # of fields	*/
	vcioputu(&io, recz); /* if >0: record size	*/

	if(recz > 0) /* schematic data */
	{	for(f = 0; f < tbl->fldn; ++f)
			vcioputu(&io, rc->info.flen[f]);
	}
	else /* variable fields */
	{	vcioputc(&io, rc->info.rsep);
		vcioputc(&io, rc->info.fsep);
		vcioputc(&io, rc->info.dot);
		vcioputc(&io, rc->info.slash);
		for(i = 0; i < 10; ++i)
			vcioputc(&io, rc->info.digit[i]);
	}

	for(f = 0; f < tbl->fldn; ++f) /* output field attributes */
	{	vcioputu(&io, rc->plan ? rc->plan->pred[f] : f); /* predictor of field */

		type = tbl->fld[f].type&(VCRD_FIXED|VCRD_DOT|VCRD_SLASH|VCRD_PAD);
		vcioputu(&io, type ? tbl->fld[f].maxz : 0); /* length of a fixed field */

		type = 0; /* indicator for field type */
		if(tbl->fld[f].type&VCRD_SLASH)
			type |= RDB_SLASH;
		if(tbl->fld[f].type&VCRD_DOT)
			type |= RDB_DOT;
		if(tbl->fld[f].type&VCRD_PAD)
			type |= RDB_PAD;
		if(rdb->type & RDB_WHOLE)
			type |= RDB_WHOLE;
#if RDB_BDV
		if(rdb->type & RDB_BDV)
			type |= RDB_BDV;
#endif
		vcioputc(&io, type);

		if(!(rdb->type & RDB_WHOLE) ) /* field data */
		{	vcioputu(&io, rc->fld[f].dtsz);
			vcioputs(&io, rc->fld[f].data, rc->fld[f].dtsz);
			vcbuffer(vc, rc->fld[f].data, -1, -1);
			rc->fld[f].data = NIL(Vcchar_t*);
			rc->fld[f].dtsz = 0;
		}
	}

	if(rdb->type & RDB_WHOLE) /* whole table data */
	{	vcioputu(&io, whsz);
		vcioputs(&io, whdt, whsz);
		vcbuffer(vc, whdt, -1, -1);
	} /**/DEBUG_ASSERT(cmpz == vciosize(&io));

	if(out)
		*out = output;
	rv = vciosize(&io);

	if(vc->coder) /* set state to check for plan redo-ing */
	{	if(rc->cmpz == 0 || rc->rawz == 0)
		{	rc->rawz = size;
			rc->cmpz = cmpz;
		}
		else if((((double)cmpz)/size) > 4*(((double)rc->cmpz)/rc->rawz) )
		{	vcrdfreeplan(rc->plan);
			rc->plan = NIL(Vcrdplan_t*);
			rc->rawz = rc->cmpz = 0;
		}
	}

done:
	if(tbl)
		vcrdclose(tbl);

	return rv;
}

#if __STD_C
static ssize_t unrdb(Vcodex_t* vc, const Void_t* data, size_t dtsz, Void_t** out)
#else
static ssize_t unrdb(vc, data, dtsz, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		dtsz;
Void_t**	out;
#endif
{
	ssize_t		recz, size, sz, flen, z, f;
	Vcchar_t	*output, *dt;
	Rdbctxt_t	*rc;
	Vcio_t		io;
	Vcrdtable_t	*tbl = NIL(Vcrdtable_t*);
	ssize_t		rv = -1;
	/**/DEBUG_DECLARE(static int, N_recn)
	/**/DEBUG_DECLARE(static int, N_unrdb) DEBUG_COUNT(N_unrdb);

	if(dtsz == 0)
		return 0;

	if(!data || !vc || !(rc = vcgetcontext(vc, Rdbctxt_t*)) )
		GOTO(done);

	if(rc->info.flen)
	{	free(rc->info.flen);
		rc->info.flen = NIL(ssize_t*);
	}

	vcioinit(&io, data, dtsz);
	if((size = vciogetu(&io)) <= 0) /* total data size */
		GOTO(done);

	if((z = vciogetu(&io)) <= 0) /* # records */
		GOTO(done);
	else	rc->info.recn = z;

	if((z = vciogetu(&io)) <= 0) /* # fields */
		GOTO(done);
	else
	{	if(z != rc->info.fldn && rc->fld ) /* schema has changed */
		{	if(vc->coder) /* delete contexts in secondary coder */
				for(f = 0; f < rc->info.fldn; ++f)
					if(rc->fld[f].ctxt)
						vcfreecontext(vc->coder, rc->fld[f].ctxt);
			free(rc->fld);
			rc->fld = NIL(Rdbfield_t*);
		}
		rc->info.fldn = z;

		if(rc->info.flen) /* reset the schema data */
			free(rc->info.flen);
		if(!(rc->info.flen = (ssize_t*)calloc(z, sizeof(ssize_t))) )
			GOTO(done);
	}

	if((recz = vciogetu(&io)) < 0) /* record length if fixed */
		GOTO(done);
	if(recz > 0) /* get schema data */
	{	for(flen = 0, f = 0; f < rc->info.fldn; ++f)
		{	if((z = vciogetu(&io)) <= 0)
				GOTO(done);
			rc->info.flen[f] = z;
			flen += z;
		}

		if(flen != recz)
			GOTO(done); /* corrupted */
	}
	else /* variable fields */
	{	if((z = vciogetc(&io)) >= 0 )
			rc->info.rsep = (Vcchar_t)z;
		else	GOTO(done);
		if((z = vciogetc(&io)) >= 0 )
			rc->info.fsep = (Vcchar_t)z;
		else	GOTO(done);
		if((z = vciogetc(&io)) >= 0 )
			rc->info.dot = (Vcchar_t)z;
		else	GOTO(done);
		if((z = vciogetc(&io)) >= 0 )
			rc->info.slash = (Vcchar_t)z;
		else	GOTO(done);
		for(f = 0; f < 10; ++f)
		{	if((z = vciogetc(&io)) >= 0 )
				rc->info.digit[f] = (Vcchar_t)z;
			else	GOTO(done);
		}
	}

	/* create skeleton field structures if not yet done */
	if(!rc->fld && !(rc->fld = (Rdbfield_t*)calloc(rc->info.fldn, sizeof(Rdbfield_t))) )
		GOTO(done);

	/* create skeleton table */
	if(!(tbl = vcrdparse(&rc->info, NIL(Vcchar_t*), 0, 0)) )
		GOTO(done);

	/* create a skeleton transform plan */
	if(rc->plan)
		vcrdfreeplan(rc->plan);
	if(!(rc->plan = vcrdmakeplan(tbl, NIL(Vcodex_t*))) )
		GOTO(done);

	/* read field data and transforming plan */
	for(f = 0; f < rc->info.fldn; ++f)
	{	/* read the predictor */	
		if(vciomore(&io) <= 0 ||
		   (rc->plan->pred[f] = vciogetu(&io)) < 0 || rc->plan->pred[f] >= rc->info.fldn )
			GOTO(done);

		/* field length */
		if(vciomore(&io) <= 0 || (flen = vciogetu(&io)) < 0 )
			GOTO(done);
		if(rc->info.flen[f] > 0 && rc->info.flen[f] != flen)
			GOTO(done);

		/* encoding type */
		if(vciomore(&io) <= 0 || (rc->fld[f].type = vciogetc(&io)) < 0 )
			GOTO(done);
		if(f > 0 && (rc->fld[f].type & RDB_WHOLE) != (rc->fld[0].type & RDB_WHOLE) )
			GOTO(done); /* WHOLE should be uniform to all fields */

		if(rc->info.flen[f] == 0)
		{	/* old Vcrdb coding of a converted field of form x.y... */
			if(flen > 0 && (rc->fld[f].type == 0 || rc->fld[f].type == RDB_SLASH) )
				rc->fld[f].type |= RDB_DOT;
			rc->info.flen[f] = flen;
		}

		if(rc->fld[f].type & RDB_WHOLE) /* do whole table later */
			continue;

		/* size of transformed data */
		if(vciomore(&io) <= 0 || (sz = vciogetu(&io)) <= 0 || sz > vciomore(&io))
			GOTO(done);

		dt = vcionext(&io); vcioskip(&io, sz);
		if(vc->coder)
		{	rc->fld[f].ctxt = vcinitcontext(vc->coder, rc->fld[f].ctxt);
			if(flen > 0)
				vcsetmtarg(vc->coder, "columns", TYPECAST(Void_t*,flen), 2);
			else
			{	if(rc->info.rsep >= 0)
					vcsetmtarg(vc->coder, "rsep",
						   TYPECAST(Void_t*,rc->info.rsep), 1);
				if(rc->info.fsep >= 0)
					vcsetmtarg(vc->coder, "fsep",
						   TYPECAST(Void_t*,rc->info.fsep), 1);
			}
		}
		if(vcrecode(vc, &dt, &sz, 0, 0) < 0 )
			GOTO(done);
#if RDB_BDV
		if(rc->fld[f].type & RDB_BDV)
		{	if((z = vcrdvector(tbl, f, dt, sz, VC_DECODE)) < 0 )
				GOTO(done);
		}
		else
#endif
		{	if((z = vcrdfield(tbl, f, flen, dt, sz)) != sz)
				GOTO(done);
		}
	}

	if(rc->fld[0].type & RDB_WHOLE)
	{	if(vciomore(&io) <= 0 || (sz = vciogetu(&io)) < 0)
			GOTO(done);
		if(vciomore(&io) < sz)
			GOTO(done);
		dt = vcionext(&io);
		vcioskip(&io, sz);

		if(vc->coder)
		{	rc->tctxt = vcinitcontext(vc->coder, rc->tctxt);
			if(recz > 0)
			{	for(z = 0, f = 0; f < tbl->fldn; ++f)
					z += rc->info.flen[f];
				vcsetmtarg(vc->coder, "columns", TYPECAST(Void_t*,z), 2);
			}
			else
			{	if(rc->info.rsep >= 0)
					vcsetmtarg(vc->coder, "rsep",
						   TYPECAST(Void_t*,rc->info.rsep), 1);
				if(rc->info.fsep >= 0)
					vcsetmtarg(vc->coder, "fsep",
						   TYPECAST(Void_t*,rc->info.fsep), 1);
			}
		}
		if(vcrecode(vc, &dt, &sz, 0, 0) < 0)
			GOTO(done);

		if(tbl) /* close the skeleton table and build a new one */
			vcrdclose(tbl);
		if(!(tbl = vcrdparse(&rc->info, dt, sz, VCRD_RECORD)) )
			GOTO(done);
	} /**/DEBUG_ASSERT(vciomore(&io) == 0);

	/* restore padded fields to their original states */
	for(f = 0; f < tbl->fldn; ++f)
	{	int type = (rc->fld[f].type & RDB_PAD) ? VCRD_PAD : 0;
		if(type && vcrdattrs(tbl, f, type, 0) < 0)
			GOTO(done);
	}

	/* reconstruct fields */
	if(vcrdexecplan(tbl, rc->plan, VC_DECODE) < 0)
		GOTO(done);

	/* reconstruct coded data of the form dot/slash */
	for(f = 0; f < tbl->fldn; ++f)
	{	int type = (rc->fld[f].type & RDB_DOT) ? VCRD_DOT : 0;
		type |= (rc->fld[f].type & RDB_SLASH) ? VCRD_SLASH : 0;
		if(type && vcrdattrs(tbl, f, type, 0) < 0)
			GOTO(done);
	} /**/DEBUG_ASSERT(tbl->recn > 0);

	/* get data out */
	if((z = vcrdextract(tbl, -1, NIL(Vcchar_t*), 0, VCRD_RECORD)) < 0 || z != size )
		GOTO(done);
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), z, 0)) )
		GOTO(done);
	if(vcrdextract(tbl, -1, output, z, VCRD_RECORD) != z)
		GOTO(done);
	if(out)
		*out = output;
	rv = z; /**/DEBUG_TALLY(1,N_recn,tbl->recn); DEBUG_PRINT(2,"#records=%d\n",N_recn);

done:	if(tbl)
		vcrdclose(tbl);

	return rv;
}

/* code any data needed as identification in persistent data */
#if __STD_C
static ssize_t rdbextract(Vcodex_t* vc, Vcchar_t** datap)
#else
static ssize_t rdbextract(vc, datap)
Vcodex_t*	vc;
Vcchar_t**	datap;	/* persistent data	*/
#endif
{
	Vcmtarg_t	*arg;
	char		*ident;
	ssize_t		k, z;
	Rdb_t		*rdb = vcgetmtdata(vc, Rdb_t*);

	/* count arguments that need coding */
	for(z = 0, arg = &_Rdbargs[0]; arg->name; ++arg)
	{	if(!(rdb->type & TYPECAST(int,arg->data)) || !(TYPECAST(int,arg->data) & RDB_HEADER) )
			continue;
		z += strlen(arg->name)+1;
	}
	if(z == 0)
		return 0;

	/* allocate memory for identification data, then build it */
	if(!(ident = (char*)vcbuffer(vc, NIL(Vcchar_t*), z, 0)) )
		return -1;
	for(k = 0, arg = _Rdbargs; arg->name && k < z; ++arg)
	{	if(!(rdb->type & TYPECAST(int,arg->data)) || !(TYPECAST(int,arg->data) & RDB_HEADER) )
			continue;
		if(!vcstrcode(arg->name, ident+k, z-k) ) 
			return -1;
		k += strlen(arg->name)+1;
	}

	if(datap) /* return string */
		*datap = (Void_t*)ident;
	return z;
}

/* reconstruct a decoding handle based on previously coded persistent data */
#if __STD_C
static int rdbrestore(Vcmtcode_t* mtcd)
#else
static int rdbrestore(mtcd)
Vcmtcode_t*	mtcd;
#endif
{
	Vcmtarg_t	*arg;
	char		*data;
	ssize_t		dtsz, z, n;
	char		id[128], init[1024];

	data = (char*)mtcd->data;
	dtsz = mtcd->size;
	if(dtsz > sizeof(init)) /* unexpectedly long data! */
		return -1;

	/* data[] should be a set of null-terminated strings */
	for(z = 0; z < dtsz; )
	{	for(arg = _Rdbargs; arg->name; ++arg)
		{	if((n = strlen(arg->name)) > (dtsz - z) )
				continue;
			if(vcstrcode((char*)arg->name, id, sizeof(id)) &&
			   strncmp(data, id, n) == 0)
				break;
		}
		if(arg->name) /* construct the initialization string */
		{	memcpy(init+z, arg->name, n);
			init[z+n] = VC_ARGSEP;
			data += n+1;
			z += n+1;
		}
	}
	init[z] = '\0';

	data =  init[0] ? (char*)init : NIL(char*);
	mtcd->coder = vcopen(0, Vcrdb, data, mtcd->coder, VC_DECODE);
	return mtcd->coder ? 1 : -1;
}

#if __STD_C
static int rdbevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int rdbevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Rdbctxt_t	*rc;
	Rdb_t		*rdb;
	Vcmtarg_t	*arg;
	Vcmtcode_t	*mtcd;
	char		*data, val[1024];
	ssize_t		k, fldn, *fldz;
	int		argtype;

	if(type == VC_OPENING)
	{	if(!(rdb = calloc(1, sizeof(Rdb_t))) )
			return -1;

		/* allocate a default context */
		rdb->ctxt = (Rdbctxt_t*)vcinitcontext(vc, NIL(Vccontext_t*));

		vcsetmtdata(vc, rdb);
		goto vc_setarg;
	}
	else if(type == VC_EXTRACT)
	{	if(!(mtcd = (Vcmtcode_t*)params) )
			return -1;
		if((mtcd->size = rdbextract(vc, &mtcd->data)) < 0 )
			return -1;
		return 1;
	}
	else if(type == VC_RESTORE)
	{	if(!(mtcd = (Vcmtcode_t*)params) )
			return -1;
		return rdbrestore(mtcd) < 0 ? -1 : 1;
	}
	else if(type == VC_SETMTARG)
	{ vc_setarg :
		if(!(rc = vcgetcontext(vc, Rdbctxt_t*)) )
			return -1;
		for(data = (char*)params; data; )
		{	data = vcgetmtarg(data, val, sizeof(val), _Rdbargs, &arg);

			argtype = TYPECAST(int,arg->data);
			if(argtype == RDB_RSEP || argtype == RDB_FSEP ||
			   argtype == RDB_ALIGN || argtype == RDB_SCHEMA )
			{	if(rc->info.flen )
					free(rc->info.flen);
				rc->info.fldn = 0;
				rc->info.flen = NIL(ssize_t*);

				if(argtype == RDB_ALIGN || argtype == RDB_SCHEMA  )
					rc->info.fsep = rc->info.rsep = -1;
			}	

			switch(argtype)
			{ case RDB_RSEP:
				rc->info.rsep = val[0] ? val[0] : -1;
				break;
			  case RDB_FSEP:
				rc->info.fsep = val[0] ? val[0] : -1;
				if(rc->info.rsep < 0 ) /* default record separator is \n */
					rc->info.rsep = '\n';
				break;
			  case RDB_ALIGN:
				rc->algn = (ssize_t)vcatoi(val);
				break;
			  case RDB_SCHEMA:
				fldz = NIL(ssize_t*); /* get list of field sizes */
				if((fldn = vcstr2list(val, ',', &fldz)) < 0 || (fldn > 0 && !fldz) )
					return -1;
				for(k = 0; k < fldn; ++k)
				{	if(fldz[k] <= 0) /* bas schema definition */
					{	free(fldz);
						return -1; /* bad length spec */
					}
				}
				rc->info.fldn = fldn;
				rc->info.flen = fldz;
				break;
			  case RDB_PLAIN:
				if(type == VC_OPENING)
					rdb->type |= RDB_PLAIN;
				break;
			  case RDB_PAD:
				if(type == VC_OPENING)
					rdb->type |= RDB_PAD;
				break;
			  case RDB_WHOLE:
				/* always pad data in this case; otherwise, records with
				   missing fields will be ambiguous on the decoding side.
				*/
				if(type == VC_OPENING)
					rdb->type |= RDB_WHOLE|RDB_PAD;
				break;
			  case RDB_BWT:
				if(type == VC_OPENING)
					rdb->type |= RDB_BWT;
				break;
#if RDB_BDV
			  case RDB_BDV:
				if(type == VC_OPENING && !(rdb->type&(RDB_PAD|RDB_WHOLE)) )
					rdb->type |= RDB_BDV;
				break;
#endif
			}
		}

#if RDB_BDV
		if(rdb->type & (RDB_PAD|RDB_WHOLE))
			rdb->type &= ~RDB_BDV;
#endif

		return 0;
	}
	else if(type == VC_INITCONTEXT)
	{	if(!params)
			return 0;

		if(!(rc = (Rdbctxt_t*)calloc(1,sizeof(Rdbctxt_t))) )
			return -1;
		rc->info.recn = 0;
		rc->info.fldn = 0;
		rc->info.flen = NIL(ssize_t*);
		rc->info.fsep = -1;
		rc->info.rsep = -1;

		/* default dot, slash and digits */
		rc->info.dot = RDB_DOTCHAR;
		rc->info.slash = RDB_SLASHCHAR;
		for(k = 0; k < 10; ++k)
			rc->info.digit[k] = '0' + k;

		*((Rdbctxt_t**)params) = rc;
		return 1;
	}
	else if(type == VC_FREECONTEXT)
	{	if((rc = (Rdbctxt_t*)params) )
		{	if(rc->info.fldn > 0 && rc->info.flen)
				free(rc->info.flen);
			if(rc->plan)
				vcrdfreeplan(rc->plan);
			if(rc->fld)
				free(rc->fld);
			free(rc);
		}
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if((rdb = vcgetmtdata(vc, Rdb_t*)) )
		{	if(rdb->vcw && rdb->vcw != VCRD_MATCH)
				vcclose(rdb->vcw);
			free(rdb);
		}
		vcsetmtdata(vc, NIL(Void_t*));
		return 0;
	}
	else	return 0;
}

Vcmethod_t _Vcrdb =
{	rdb,
	unrdb,
	rdbevent,
	"rdb", "Transforming relational data by field dependency.",
	"[-?\n@(#)$Id: vcodex-rdb (AT&T Research) 2009-02-22 $\n]" USAGE_LICENSE,
	_Rdbargs,
	8*1024*1024,
	0
};

VCLIB(Vcrdb)
