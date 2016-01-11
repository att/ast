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
#include	<vctable.h>
#include	<vcrdb.h>

/* Transform/compress relational table data.
** This codes the values in each field either by their
** ordinal numbers in some dynamically constructed
** dictionary or by a sequence of integers if the values
** are of the form xxx.yyy.../zzz (the /zzz is optional).
** In this way, the relational table is turned into
** a table with fixed-length rows which is then processed
** by Vctable. The values in dictionaries are collected
** and compressed by Vcbwt.
**
** Written by Kiem-Phong Vo (05/01/2008)
*/

#define RT_RSEP		1	/* setting fld separ	*/
#define RT_FSEP		2	/* setting rec separ	*/
#define RT_SCHEMA	3	/* defining schema	*/
#define RT_ALIGN	4	/* defining alignment	*/
#define RT_RENEW	5	/* always renew context	*/

#define RT_MAXINT	(1<<28) /* max codable integer	*/

#define RT_SHORT	16	/* code short fld as-is	*/

/* codes in persistence data for Rtfld_t.type */
#define RT_ISEQ		(1<<0)	/* integer sequence	*/
#define RT_DASH		(1<<1)	/* sequence ends with /	*/
#define RT_CLEAR	(1<<2)	/* clear value set	*/
#define RT_FIXZ		(1<<3)	/* fixed length field	*/
#define RT_ASIS		(1<<4)	/* coded as-is in table	*/

typedef struct _rtval_s /* mapping value to ordinal */ 
{	Dtlink_t	olnk;	/* search by ordinal	*/
	Dtlink_t	dlnk;	/* search by data	*/
	Vcchar_t*	data;	/* a data field value	*/
	ssize_t		dtsz;	/* length of data	*/
	ssize_t		ordn;	/* Note: ordn >= 1	*/
} Rtval_t;

/* A field can be coded either by ordinal numbers of unique
** values or as a sequence of integers if it is of the form
** xxx.yyy.../zzz.
*/
typedef struct _rtfld_s /* all data for a field */
{	int		type;	/* field type		*/
	Rtval_t**	val;	/* decoding value list 	*/
	Dt_t*		ddt;	/* store values by data	*/
	Dt_t*		odt;	/* store vals by order	*/
	ssize_t		asis;	/* fixed size coding	*/
	ssize_t		pcnt;	/* previous # of values	*/
	ssize_t		vcnt;	/* current # of values	*/
				/* or # elts for ISEQ	*/
	ssize_t		vmax;	/* max coding value 	*/
} Rtfld_t;

typedef struct _rtctxt_s /* context of invocation */
{	Vccontext_t	ctxt;
	int		fsep;	/* field separator	*/
	int		rsep;	/* record separator	*/
	ssize_t		algn;	/* field alignment	*/
	ssize_t		recz;	/* 0 if not schematic	*/
	ssize_t		fldn;	/* # of fields in use	*/
	ssize_t*	fldz;	/* size of each field	*/
	Rtfld_t*	fld;	/* list of fields	*/
} Rtctxt_t;

typedef struct _rtable_s
{	Rtctxt_t*	ctxt;	/* default context	*/
	Vcodex_t*	tbl;	/* table transform	*/
	Vcodex_t*	bwt;	/* BWT transform	*/
	int		renew;	/* always renew context	*/
	int		dot;	/* the dot character	*/
	int		dash;	/* the dash character	*/
	int		zero;	/* the zero digit	*/
} Rtable_t;

/* arguments passable to vcopen() */
static Vcmtarg_t _Rtargs[] =
{
	{ "fsep", "Fields are separated  by 'fsep=character'", (Void_t*)RT_FSEP },
	{ "rsep", "Records are separated  by 'rsep=character'", (Void_t*)RT_RSEP },
	{ "schema", "Schema is defined as 'schema=[fldlen1,fldlen2,...]'", (Void_t*)RT_SCHEMA },
	{ "align", "Field alignment is given as 'align=value'", (Void_t*)RT_ALIGN },
	{ "renew", "Context renewal defined as 'renew=[01]'", (Void_t*)RT_RENEW },
	{  0 , "Field and record separators to be computed.", (Void_t*)0 }
};

/* discipline functions and data structures for the CDT library */
#if __STD_C
static int rtdatacmp(Dt_t* dt, void* o1, void* o2, Dtdisc_t* disc)
#else
static int rtdatacmp(dt, o1, o2, disc)
Dt_t*		dt;
void*		o1;
void*		o2;
Dtdisc_t*	disc;
#endif
{
	int		d;
	Rtval_t		*f1 = (Rtval_t*)o1, *f2 = (Rtval_t*)o2;
	Vcchar_t	*s1 = f1->data, *s2 = f2->data, *ends;

	/* compare two objects by their data */
	ends = s1 + (f1->dtsz < f2->dtsz ? f1->dtsz : f2->dtsz);
	for(; s1 < ends; ++s1, ++s2)
		if((d = (int)s1[0] - (int)s2[0]) != 0 )
			return d;
	return f1->dtsz < f2->dtsz ? -1 : f1->dtsz == f2->dtsz ? 0 : 1;
}

#if __STD_C
static int rtordncmp(Dt_t* dt, void* o1, void* o2, Dtdisc_t* disc)
#else
static int rtordncmp(dt, o1, o2, disc)
Dt_t*		dt;
void*		o1;
void*		o2;
Dtdisc_t*	disc;
#endif
{
	Rtval_t	*f1 = (Rtval_t*)o1, *f2 = (Rtval_t*)o2;
	return f1->ordn < f2->ordn ? -1 : f1->ordn == f2->ordn ? 0 : 1;
}

#if __STD_C
static unsigned int rthash(Dt_t* dt, void* o, Dtdisc_t* disc)
#else
static unsigned int rthash(dt, o, disc)
Dt_t*		dt;
void*		o;
Dtdisc_t*	disc;
#endif
{
	Rtval_t	*f = (Rtval_t*)o;
	return f->dtsz == 0 ? 0 : dtstrhash(0, f->data, f->dtsz);
}

#if __STD_C
static void rtfree(Dt_t* dt, void* o, Dtdisc_t* disc)
#else
static void rtfree(dt, o, disc)
Dt_t*		dt;
void*		o;
Dtdisc_t*	disc;
#endif
{
	free(o);
}

static Dtdisc_t	Rtdata =
{	0, 0, DTOFFSET(Rtval_t, dlnk),
	NIL(Dtmake_f), rtfree,
	rtdatacmp, rthash,
	NIL(Dtmemory_f), NIL(Dtevent_f)
};

static Dtdisc_t	Rtordn =
{	0, 0, DTOFFSET(Rtval_t, olnk),
	NIL(Dtmake_f), NIL(Dtfree_f),
	rtordncmp, NIL(Dthash_f),
	NIL(Dtmemory_f), NIL(Dtevent_f)
};

/* convert a sequence xxx.yyy.../zzz into fixed length integers */
#if __STD_C
static ssize_t rta2i(Rtable_t* rt, Rtfld_t* rf, Vcchar_t* dt, ssize_t dz, Vcchar_t* cv, ssize_t cz)
#else
static ssize_t rta2i(rt, rf, dt, dz, cv, cz)
Rtable_t*	rt;	/* table information	*/
Rtfld_t*	rf;	/* field in processing	*/
Vcchar_t*	dt;	/* data to convert	*/
ssize_t		dz;	/* data length		*/
Vcchar_t*	cv;	/* buffer to convert to	*/
ssize_t		cz;	/* buffer length	*/
#endif
{
	Vcchar_t	*edt;
	int		zero, nine;
	ssize_t		intv, v, vmax, vcnt, ndash;
	Vcio_t		io;

	if(dz <= 0 || !(rf->type&RT_ISEQ)) /* not convertible */
		return -1;

	vcioinit(&io, cv, cz);
	ndash = vcnt = 0; vmax = rf->vmax;
	zero = rt->zero; nine = zero+9;
	for(edt = dt+dz; dt < edt; )
	{	intv = *dt++ - zero;
		if(intv < 0 || intv > 9)
			return -1; /* must start with a digit */
		if(intv == 0 && dt < edt && *dt >= zero && *dt <= nine)
			return -1; /* no leading zero allowed */

		for(; dt < edt; ++dt)
		{	v = *dt - zero;
			if(v < 0 || v > 9)
			{	if(*dt != rt->dot && *dt != rt->dash)
					return -1; /* unrecognized character */
				if(ndash > 0 || (dt+1) >= edt)
					return -1; /* bad syntax */

				if(*dt == rt->dash)
					ndash += 1;
				vcnt += 1;

				dt += 1;
				break;
			}
			intv = intv*10 + v;

			if(intv <= 0 || (cv && intv > vmax) || (!cv && intv >= RT_MAXINT) )
				return -1; /* value too large */
		}

		if(cv) /* encoding */
		{	if(vciomore(&io) < vcsizem(vmax))
				return -1;
			vcioputm(&io, intv, vmax);
		}
		else	vmax = vmax > intv ? vmax : intv;
	}

	vcnt += 1;

	if(rf->vcnt > 0 ) /* make sure format remains exactly the same */
	{	if(vcnt != rf->vcnt) /* element count */
			return -1;
		if((rf->type&RT_DASH) && ndash == 0) /* xxx.yyy.../zzz format */
			return -1;
		if(!(rf->type&RT_DASH) && ndash > 0)
			return -1;
	}

	rf->type |= (ndash ? RT_DASH : 0); /* whether or not there is a dash */
	rf->vcnt = vcnt; /* sequence count */
	rf->vmax = vmax > rf->vmax ? vmax : rf->vmax; /* max value to code with */
	return vciosize(&io);
}

#if __STD_C
static ssize_t rti2a(Rtable_t* rt, Rtfld_t* rf, Vcchar_t* dt, ssize_t dz, Vcchar_t* cv, ssize_t cz)
#else
static ssize_t rti2a(rt, rf, dt, dz, cv, cz)
Rtable_t*	rt;	/* table information	*/
Rtfld_t*	rf;	/* field in processing	*/
Vcchar_t*	dt;	/* data to convert	*/
ssize_t		dz;	/* data length		*/
Vcchar_t*	cv;	/* buffer to convert to	*/
ssize_t		cz;	/* buffer length	*/
#endif
{
	ssize_t		k, n;
	Vcchar_t	buf[16], *b, *endb, *c, *endc, zero;
	Vcio_t		io;
	Vcuint32_t	intv, vmax;

	zero = rt->zero; vmax = rf->vmax;

	if(dz <= 0 || (dz%vcsizem(vmax)) != 0 || (dz/vcsizem(vmax)) != rf->vcnt)
		return -1;
	vcioinit(&io, dt, dz);

	endb = buf + sizeof(buf);
	endc = (c = cv) + cz;
	for(k = dz/vcsizem(vmax);; )
	{	if((intv = vciogetm(&io, vmax)) > vmax)
			return -1; /* bad encoded value */	
		for(b = endb;; )
		{	*--b = zero + intv%10;
			if((intv /= 10) == 0)
				break;
		}
		if((n = endb-b) > (endc-c))
			return -1; /* out of buffer */
		memcpy(c, b, n); c += n;

		if((k -= 1) == 0)
			return c-cv;

		if(c >= endc)
			return -1;
		*c++ = (k == 1 && (rf->type&RT_DASH)) ? rt->dash : rt->dot;
	}
}

/* remove a list of values */
#if __STD_C
static void vlclose(Rtval_t** val, ssize_t vm)
#else
static void vlclose(val, vm)
Rtval_t**	val; /* list of values		*/
ssize_t		vm;  /* max index of values	*/
#endif
{
	if(val)
	{	for(; vm >= 0; --vm)
			if(val[vm])
				free(val[vm]);
		free(val);
	}
}

/* clear field definition and associated data in a context */
#if __STD_C
static void rtctxtclear(Rtctxt_t* rc)
#else
static void rtctxtclear(rc)
Rtctxt_t*	rc;
#endif
{
	ssize_t	k;

	if(rc->fldn > 0)
	{	if(rc->fld)
		{	for(k = 0; k < rc->fldn; ++k)
			{	if(rc->fld[k].ddt)
					dtclose(rc->fld[k].ddt);
				if(rc->fld[k].odt)
					dtclose(rc->fld[k].odt);
				if(rc->fld[k].val)
					vlclose(rc->fld[k].val, rc->fld[k].vcnt);
			}
			free(rc->fld);
		}

		if(rc->fldz)
			free(rc->fldz);
	}

	rc->fldn = 0;
	rc->fldz = NIL(ssize_t*);
	rc->recz = 0;
	rc->fld  = NIL(Rtfld_t*);
}

#if __STD_C
static ssize_t rtable(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t rtable(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*d, *dt, *edt;
	ssize_t		f, fldn, fmin, recn;
	ssize_t		u, z, tblz, valz, rowz;
	Rtval_t		*vl, val;
	Vcio_t		io;
	Vcchar_t	*output, *valdt, *tbldt;
	Rtable_t	*rt;
	Rtctxt_t	*rc;
	ssize_t		rv = -1;

	if(size <= 0)
		return 0;

	if(!vc || !data ||
	   !(rt = vcgetmtdata(vc,Rtable_t*)) ||
	   !(rc = vcgetcontext(vc, Rtctxt_t*)) )
		return -1;

	if(rt->renew) /* clear out dictionaries only */
		rtctxtclear(rc);

	vc->undone = size; /* start with doing nothing */

	if(rc->recz <= 0 && (rc->rsep <= 0 || rc->fsep <= 0) )
	{	Vcrdformat_t	*rdf;
		if(!(rdf = vcrdformat((Vcchar_t*)data, size, rc->rsep, rc->algn, 1)) )
			return 0;

		if(rdf->fsep > 0 && rdf->rsep > 0)
		{	rc->fsep = rdf->fsep;
			rc->rsep = rdf->rsep;
		}
		else if(rdf->fldn > 0 && rdf->fldz)
		{	if((rc->fldz = (ssize_t*)malloc(rdf->fldn*sizeof(ssize_t))) )
			{	memcpy(rc->fldz, rdf->fldz, rdf->fldn*sizeof(ssize_t));
				rc->fldn = rdf->fldn;

				rc->recz = 0; /* compute record length */
				for(f = 0; f < rc->fldn; ++f)
					rc->recz += rc->fldz[f];
			}
		}
		free(rdf);

		if(rc->fldn <= 0 && (rc->rsep <= 0 || rc->fsep <= 0) )
			return 0;
	}

	/* compute field and record numbers */
	if(rc->recz > 0) /* schematically defined */
	{	fldn = fmin = rc->fldn;
		recn = size/rc->recz;
		size = recn*rc->recz; /* size of data to be processed */
	}
	else /* records and fields defined by separators */
	{	if(rc->rsep <= 0) /* not a table */
			return 0;
		if(rc->fsep <= 0) /* single field */
			rc->fsep = rc->rsep; 

		recn = fldn = 0; /* count records and number of fields per record */
		fmin = -1; /* fewest fields in a record */
		rowz = -1; /* see if this is a fixed length record */
		for(edt = (dt = (Vcchar_t*)data)+size; dt < edt;)
		{	for(f = 0, d = dt; d < edt; ++d)
			{	if(*d == rc->fsep || *d == rc->rsep)
					f += 1;
				if(*d == rc->rsep)
					break;
			}
			if(d >= edt) /* last incomplete record */
				break;

			if(rowz < 0)
				rowz = f != 1 ? 0 : (d-dt)+1;
			else if(rowz > 0)
				rowz = (f != 1 || (z = (d-dt)+1) != rowz) ? 0 : z;

			if(fmin < 0 || f < fmin) /* smallest # of fields per record */
				fmin = f;
			if(f > fldn) /* largest # of fields per record */
				fldn = f;

			recn += 1; /* count records */

			dt = d+1; /* process next record */
		}
		size = dt - (Vcchar_t*)data; /* size of data to be processed */

		if(fldn == 1 && rowz > 0) /* treat this as a fixed-length table */
		{	rtctxtclear(rc);
			if(!(rc->fldz = (ssize_t*)malloc(sizeof(ssize_t))) )
				return -1;
			rc->fldn = 1;
			rc->fldz[0] = rc->recz = rowz;
		}
	}
	if(fldn <= 0 || recn <= 0)
		return 0; /* nothing to do */
	vc->undone -= size; /* a remaining incomplete record, if any */

	if(fldn != rc->fldn) /* records differ from before */
	{	rtctxtclear(rc);
		rc->fldn = fldn;
	}

	if(!rc->fld) /* allocate field structures */
	{	if(!(rc->fld = (Rtfld_t*)calloc(fldn, sizeof(Rtfld_t))) )
			GOTO(done);
		for(f = 0; f < fldn; ++f)
		{	if(!(rc->fld[f].odt = dtopen(&Rtordn, Dtoset)) )
				GOTO(done);
			if(!(rc->fld[f].ddt = dtopen(&Rtdata, Dtset)) )
				GOTO(done);
			rc->fld[f].pcnt = rc->fld[f].vcnt = 0;
			rc->fld[f].vmax = 0;
		}
	}

	/* check for fields that will be coded in-place in table */
	if(rc->recz > 0) /* fields are schematically defined */
	{	for(f = 0; f < fldn; ++f)
		{	rc->fld[f].type = RT_FIXZ;
			if(rc->fldz[f] <= RT_SHORT || fldn == 1) /* code as-is */
			{	rc->fld[f].type |= RT_ASIS;
				rc->fld[f].vcnt = rc->fldz[f]; /* size of field */
				rc->fld[f].vmax = 0xff; /* vcsizem(0xff) == 1, below */
			}
		}
	}
	else /* using field&record separators */
	{	for(f = 0; f < fldn; ++f)
		{	if(rc->fld[f].pcnt > 0 || fmin != fldn) /* no codable fields */
				rc->fld[f].type = 0;
			else
			{	rc->fld[f].type = RT_ASIS|RT_ISEQ;
				rc->fld[f].asis = -1;
			}
		}

		/* check for fields codable in some special way */
		for(edt = (dt = (Vcchar_t*)data)+size; dt < edt; )
		{	for(u = 0, f = 0;; )
			{	for(d = dt; *d != rc->fsep && *d != rc->rsep; )
					d += 1;
				z = d-dt; /* size of field minus separator */

				/* see if codable as a fixed length integer sequence */
				if(rc->fld[f].type&RT_ISEQ)
				{	if(rta2i(rt, rc->fld+f, dt, z, 0, 0) < 0)
					{	rc->fld[f].type &= ~RT_ISEQ;
						rc->fld[f].vcnt = rc->fld[f].vmax = 0;
					}
				}

				/* see if remaining fixed length */
				if(rc->fld[f].type&RT_ASIS)
				{	if(rc->fld[f].asis == -1)
						rc->fld[f].asis = z;
					else if(z != rc->fld[f].asis)
					{	rc->fld[f].type &= ~RT_ASIS;
						rc->fld[f].asis = -2;
					}
				}

				if(!(rc->fld[f].type & (RT_ISEQ|RT_ASIS)) )
					u += 1; /* an uncodable field */

				f += 1; /* count field */
				dt = d+1; /* next field or record */
				if(*d == rc->rsep) /* end of record */
					break;
			} /**/DEBUG_ASSERT(u <= fldn);
			if(u == fldn) /* no more ISEQ|ASIS fields */
				break;
		}

		/* pick between asis and iseq */
		for(f = 0; f < fldn; ++f)
		{	if((rc->fld[f].type&RT_ISEQ) && (rc->fld[f].type&RT_ASIS) &&
			   rc->fld[f].asis <= (rc->fld[f].vcnt*rc->fld[f].vmax) )
				rc->fld[f].type &= ~RT_ISEQ;

			if(rc->fld[f].type&RT_ASIS)
			{	/**/DEBUG_ASSERT(!(rc->fld[f].type & RT_ISEQ) );
				rc->fld[f].vcnt = rc->fld[f].asis;
				rc->fld[f].vmax = 0xff;
			}
		}
	}

	valz = 0; /* check for new field values and accumulate their total size */
	for(edt = (dt = (Vcchar_t*)data)+size; dt < edt; )
	{	for(f = 0;; ) /* process a record */
		{	if(rc->fld[f].type&RT_FIXZ) /* fixed length field */
				d = dt + rc->fldz[f];
			else for(d = dt; *d != rc->fsep && *d != rc->rsep; )
				d += 1;

			if(!(rc->fld[f].type&(RT_ASIS|RT_ISEQ)) )
			{	val.data = dt; val.dtsz = d-dt;
				if(!(vl = dtsearch(rc->fld[f].ddt, &val)) )
				{	/* insert a new value into dictionaries */
					z = sizeof(Rtval_t) + val.dtsz;
					if(!(vl = (Rtval_t*)malloc(z)) )
						GOTO(done);
					vl->data = (Vcchar_t*)(vl+1);
					if((vl->dtsz = val.dtsz) > 0) 
						memcpy(vl->data, dt, vl->dtsz);
					vl->ordn = (rc->fld[f].vcnt += 1);

					valz += vl->dtsz;
					if(!(rc->fld[f].type&RT_FIXZ) ) /* count separator */
						valz += sizeof(Vcchar_t);

					dtinsert(rc->fld[f].odt, vl); /* ordinal dictionary */
					dtinsert(rc->fld[f].ddt, vl); /* data dictionary */
				}
			}

			f += 1; /* advance field index */
			if(rc->fld[f-1].type&RT_FIXZ)
			{	dt = d;
				if(f >= fldn)
					break;
			}
			else
			{	dt = d+1; /* skip field or record separator */
				if(*d == rc->rsep)
					break;
			}
		}
	}

	for(rowz = 0, f = 0; f < fldn; ++f) /* estimate row size of encoding table */
	{	if(rc->fld[f].type&(RT_ISEQ|RT_ASIS)) /* fixed-length coding in table */
			z = vcsizem(rc->fld[f].vmax) * rc->fld[f].vcnt;
		else /* code by indices in some value set */
		{	rc->fld[f].vmax = rc->fld[f].vcnt;
			z = vcsizem(rc->fld[f].vmax); 
		}
		rowz += z;
	} /**/DEBUG_PRINT(2,"fldn=%d ",fldn); DEBUG_PRINT(2,"recn=%d ",recn); DEBUG_PRINT(2,"rowz=%d\n",rowz);

	tblz = recn*rowz; /* allocate space for code table */
	if(!(tbldt = vcbuffer(rt->tbl, NIL(Vcchar_t*), tblz, 0)) )
		GOTO(done);
	vcioinit(&io, tbldt, tblz);

	/* build the representation table from indices or fixed-size data */
	for(edt = (dt = (Vcchar_t*)data)+size; dt < edt; )
	{	for(f = 0;; )
		{	if(rc->fld[f].type&RT_FIXZ) /* fixed length field */
				d = dt + rc->fldz[f];
			else for(d = dt; *d != rc->fsep && *d != rc->rsep; )
				d += 1;

			if(rc->fld[f].type&RT_ASIS) /* code literally */
			{	/**/DEBUG_ASSERT(rc->fld[f].asis == rc->fld[f].vcnt);
				/**/DEBUG_ASSERT(rc->fld[f].asis == (d-dt));
				vcioputs(&io, dt, d-dt);
			}
			else if(rc->fld[f].type&RT_ISEQ) /* code as an integer sequence */
			{	z = rta2i(rt, rc->fld+f, dt, d-dt, vcionext(&io), vciomore(&io));
				/**/DEBUG_ASSERT(z == rc->fld[f].vcnt*vcsizem(rc->fld[f].vmax));
				if(z <= 0)
					GOTO(done);
				vcioskip(&io, z);
			}
			else /* coding a field value by its ordinal number */
			{	val.data = dt; val.dtsz = d-dt;
				if(!(vl = dtsearch(rc->fld[f].ddt, &val)) )
					GOTO(done);
				/**/DEBUG_ASSERT(vl->ordn <= rc->fld[f].vmax);
				vcioputm(&io, vl->ordn, rc->fld[f].vmax);
			}

			f += 1; /* advance field index */
			if(rc->fld[f-1].type&RT_FIXZ)
			{	dt = d;
				if(f >= fldn)
					break;
			}
			else
			{	dt = d+1; /* skip separator to start of next field */
				if(*d == rc->rsep)
					break;
			}
		}

		for(; f < fldn; ++f) /* fill in missing fields with 0's */
		{	/**/DEBUG_ASSERT(rc->fld[f].type == 0);
			vcioputm(&io, 0, rc->fld[f].vmax);
		}
	} /**/DEBUG_ASSERT(vciosize(&io) == tblz);

	/* output new field value data in order of their ordinal numbers */
	if(!(valdt = vcbuffer(rt->bwt, NIL(Vcchar_t*), valz+1, 0)) )
		GOTO(done);
	vcioinit(&io, valdt, valz);
	for(f = 0; f < fldn; ++f)
	{	if(rc->fld[f].type & (RT_ISEQ|RT_ASIS)) /* data were coded in table */
			continue;

		val.ordn = rc->fld[f].pcnt+1; /* new data should have ordn > pcnt */
		for(vl = dtsearch(rc->fld[f].odt,&val); vl; vl = dtnext(rc->fld[f].odt,vl) )
		{	vcioputs(&io, vl->data, vl->dtsz);
			if(!(rc->fld[f].type&RT_FIXZ) )
				vcioputc(&io, rc->rsep); /* only rsep is used here */
		}

		if(rc->fld[f].pcnt == 0) /* dictionary started empty */
			rc->fld[f].type |= RT_CLEAR;

		rc->fld[f].pcnt = rc->fld[f].vcnt;
	} /**/DEBUG_ASSERT(vciosize(&io) == valz);

	/**/DEBUG_PRINT(2,"Raw valz=%d\n",valz);
	if(valz > 0)
	{	rt->bwt->coder = vc->coder; /* transform field values with bwt */
		if((valz = vcapply(rt->bwt, valdt, valz, &valdt)) < 0 )
			GOTO(done);
		/**/DEBUG_PRINT(2, "Processed valz=%d\n", valz);
	}

	/**/DEBUG_PRINT(2,"Raw tblz=%d\n",tblz);
	rt->tbl->coder = vc->coder; /* transform table with Vctable */
	vcsetmtarg(rt->tbl, "columns", (Void_t*)rowz, 2);
	if((tblz = vcapply(rt->tbl, tbldt, tblz, &tbldt)) < 0 )
		GOTO(done);
	/**/DEBUG_PRINT(2, "Processed tblz=%d\n", tblz);

	/* compute size of output data */
	z = vcsizeu(size) + /* original data size */
	    sizeof(Vcchar_t) + /* the '.' character */
	    sizeof(Vcchar_t) + /* the '/' character */
	    sizeof(Vcchar_t) + /* the '0' character */
	    sizeof(Vcchar_t) + /* record separator */
	    sizeof(Vcchar_t) + /* field separator */
	    vcsizeu(fldn) + /* number of fields per record */
	    vcsizeu(valz) + valz; /* new field values */
	for(f = 0; f < fldn; ++f) /* field format */
	{	z += sizeof(Vcchar_t) + /* type of field */
		     vcsizeu(rc->fld[f].vmax) + /* max bound of indices */
		     vcsizeu(rc->fld[f].vcnt); /* # of elts in sequence */
		if(rc->fld[f].type&RT_FIXZ) /* a fixed length field */
			z += vcsizeu(rc->fldz[f]); 
	}
	z += vcsizeu(rowz) + /* row size of table data */
	     vcsizeu(tblz) + tblz; /* table data */

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), z, 0)) )
		GOTO(done);
	vcioinit(&io, output, z);
	vcioputu(&io, size);
	vcioputc(&io, rt->dot);
	vcioputc(&io, rt->dash);
	vcioputc(&io, rt->zero);
	vcioputc(&io, rc->rsep);
	vcioputc(&io, rc->fsep);
	vcioputu(&io, fldn);
	vcioputu(&io, valz); vcioputs(&io, valdt, valz);
	for(f = 0; f < fldn; ++f)
	{	vcioputc(&io, rc->fld[f].type);
		vcioputu(&io, rc->fld[f].vmax);
		vcioputu(&io, rc->fld[f].vcnt);
		if(rc->fld[f].type&RT_FIXZ)
			vcioputu(&io, rc->fldz[f]);
	}
	vcioputu(&io, rowz);
	vcioputu(&io, tblz); vcioputs(&io, tbldt, tblz);
	/**/DEBUG_ASSERT(vciosize(&io) == z);

	if(out)
		*out = output;
	rv = z;

done:	if(rv < 0) /* if an error happened, clear context */
		rtctxtclear(rc);
	return rv;
}


#if __STD_C
static ssize_t unrtable(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unrtable(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		z, f, v, dtsz, fldn, valz, tblz, rowz;
	ssize_t		type, vmax, vcnt;
	Vcchar_t	*output, *tbldt, *valdt, *dt, *enddt;
	Rtval_t		**val, *vl;
	Vcio_t		io;
	Rtable_t	*rt;
	Rtctxt_t	*rc;
	ssize_t		rv = -1;

	if(size == 0)
		return 0;

	if(!vc || !data ||
	   !(rt = vcgetmtdata(vc,Rtable_t*)) ||
	   !(rc = vcgetcontext(vc, Rtctxt_t*)) )
		GOTO(done);

	vcioinit(&io, data, size);

	/* get the size of original data */
	if(vciomore(&io) <= 0 || (dtsz = vciogetu(&io)) < 0 )
		GOTO(done);

	/* read the coding characters for integer sequences */
	if(vciomore(&io) <= 0 || (rt->dot = vciogetc(&io)) < 0 )
		GOTO(done);
	if(vciomore(&io) <= 0 || (rt->dash = vciogetc(&io)) < 0 )
		GOTO(done);
	if(vciomore(&io) <= 0 || (rt->zero = vciogetc(&io)) < 0 )
		GOTO(done);

	/* read separators */
	if(vciomore(&io) <= 0 || (rc->rsep = vciogetc(&io)) < 0 )
		GOTO(done);
	if(vciomore(&io) <= 0 || (rc->fsep = vciogetc(&io)) < 0 )
		GOTO(done);

	/* read field data */
	if(vciomore(&io) <= 0 || (fldn = vciogetu(&io)) <= 0 )
		GOTO(done);
	if(vciomore(&io) <= 0 || (valz = vciogetu(&io)) < 0 )
		GOTO(done);
	if(vciomore(&io) < valz)
		GOTO(done);
	valdt = vcionext(&io); vcioskip(&io, valz);
	rt->bwt->coder = vc->coder;
	if(valz > 0 && (valz = vcapply(rt->bwt, valdt, valz, &valdt)) < 0)
		GOTO(done);

	if(fldn != rc->fldn || !rc->fld) /* creat arrays of field data */
	{	rtctxtclear(rc);
		rc->fldn = fldn;
		if(!(rc->fld = (Rtfld_t*)calloc(fldn, sizeof(Rtfld_t))) ||
		   !(rc->fldz = (ssize_t*)calloc(fldn, sizeof(ssize_t))) )
			GOTO(done);
	}

	enddt = valdt + valz; /* reconstruct field data */
	for(f = 0; f < fldn; ++f)
	{	if(vciomore(&io) <= 0 || (type = vciogetc(&io)) < 0 )
			GOTO(done);
		if(vciomore(&io) <= 0 || (vmax = vciogetu(&io)) < 0 )
			GOTO(done);
		if(vciomore(&io) <= 0 || (vcnt = vciogetu(&io)) < 0 )
			GOTO(done);
		if(type&RT_FIXZ)
		{	if(vciomore(&io) <= 0 || (z = vciogetu(&io)) < 0)
				GOTO(done);
			rc->fldz[f] = z;
		}

		if(type&(RT_CLEAR|RT_ISEQ|RT_ASIS)) /* clear all current values */
		{	if(rc->fld[f].val)
				vlclose(rc->fld[f].val, rc->fld[f].vcnt);
			rc->fld[f].val = NIL(Rtval_t**);
			rc->fld[f].vcnt = 0;
		}

		rc->fld[f].type = type; /* type of field coding */
		rc->fld[f].vmax = vmax; /* bound of value index */

		if(type&(RT_ISEQ|RT_ASIS) ) /* values coded directly in table */
		{	rc->fld[f].vcnt = vcnt;
			continue;
		}

		if(rc->fld[f].vcnt > 0 && vcnt == rc->fld[f].vcnt)
			continue; /* no new values */

		/* add new values for this field */
		if((val = rc->fld[f].val)) /* get space for new values */
			val = (Rtval_t**)realloc(val,(vcnt+1)*sizeof(Rtval_t*));
		else	val = (Rtval_t**)malloc((vcnt+1)*sizeof(Rtval_t*));
		if(!val)
			GOTO(done);
		rc->fld[f].val = val;

		val[0] = NIL(Rtval_t*); /* values start from location 1 */
		for(v = rc->fld[f].vcnt+1; v <= vcnt; ++v)
		{	if(type&RT_FIXZ)
			{	dt = valdt + rc->fldz[f];
				if(dt > enddt)
					GOTO(done);
			}
			else
			{	for(dt = valdt; dt < enddt; ++dt)
					if(*dt == rc->rsep)
						break;
				if(dt >= enddt) /* incomplete data */
					GOTO(done);
			}

			if(!(vl = (Rtval_t*)malloc(sizeof(Rtval_t)+(dt-valdt))) )
				GOTO(done);
			vl->data = (Vcchar_t*)(vl+1);
			vl->dtsz = dt - valdt;
			memcpy(vl->data, valdt, vl->dtsz);
			val[v] = vl;

			valdt = dt + ((type&RT_FIXZ) ? 0 : 1);
		}
		rc->fld[f].vcnt = vcnt; /* vcnt is the index of the last value! */
	}

	/* get table data */
	if(vciomore(&io) <= 0 || (rowz = vciogetu(&io)) <= 0 )
		GOTO(done);
	if(vciomore(&io) <= 0 || (tblz = vciogetu(&io)) <= 0 )
		GOTO(done);
	if(vciomore(&io) < tblz)
		GOTO(done);
	tbldt = vcionext(&io); vcioskip(&io, tblz);
	rt->tbl->coder = vc->coder;
	if((tblz = vcapply(rt->tbl, tbldt, tblz, &tbldt)) < 0)
		GOTO(done);

	/* now reconstruct original data */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), dtsz, 0)) )
		GOTO(done);
	vcioinit(&io, tbldt, tblz);
	for(enddt = (dt = output)+dtsz; vciomore(&io) > 0; )
	{	for(f = 0; f < fldn; ++f)
		{	if(rc->fld[f].type&RT_ASIS) /* field's own uncoded data */
			{	if(f > 0 && !(rc->fld[f-1].type&RT_FIXZ))
				{	if(dt >= enddt)
						GOTO(done);
					*dt++ = rc->fsep;
				}

				z = (rc->fld[f].type&RT_FIXZ) ? rc->fldz[f] : rc->fld[f].vcnt;
				if(z > vciomore(&io) || (enddt-dt) < z)
					GOTO(done);
				vciogets(&io, dt, z);
				dt += z;
			}
			else if(rc->fld[f].type&RT_ISEQ) /* field coded as integer sequence */
			{	if(f > 0 && !(rc->fld[f-1].type&RT_FIXZ))
				{	if(dt >= enddt)
						GOTO(done);
					*dt++ = rc->fsep;
				}

				v = rc->fld[f].vcnt * vcsizem(rc->fld[f].vmax);
				if(vciomore(&io) < v)
					GOTO(done);
				valdt = vcionext(&io); vcioskip(&io, v);
				if((z = rti2a(rt, rc->fld+f, valdt, v, dt, enddt-dt)) < 0)
					GOTO(done);
				dt += z;
			}
			else /* string data coded by index in value set */
			{	if(vciomore(&io) <= 0 || (v = vciogetm(&io, rc->fld[f].vmax)) < 0)
					GOTO(done);

				if(v == 0) /* a short record with missing fields */
				{	for(f += 1; f < fldn; ++f) /* skip missing fields */
						if((v = vciogetm(&io, rc->fld[f].vmax)) != 0)
							GOTO(done);
					break;
				}

				if(f > 0 && !(rc->fld[f-1].type&RT_FIXZ))
				{	if(dt >= enddt)
						GOTO(done);
					*dt++ = rc->fsep;
				}

				if((z = rc->fld[f].val[v]->dtsz) > (enddt-dt) )
					GOTO(done);
				memcpy(dt, rc->fld[f].val[v]->data, z);
				dt += z;
			}
		}

		if(!(rc->fld[fldn-1].type&RT_FIXZ) )
		{	if(dt >= enddt)
				GOTO(done);
			*dt++ = rc->rsep;
		}
	}
	if(dt != enddt)
		GOTO(done);

	if(out)
		*out = output;
	rv = dtsz;

done:
	return rv;
}

#if __STD_C
static int rtevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int rtevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Rtable_t	*rt;
	Rtctxt_t	*rc;
	Vcmtarg_t	*arg;
	char		*data, val[1024];
	ssize_t		k, fldn, recz, *fldz;
	int		rv = -1;

	if(type == VC_OPENING)
	{	if(!(rt = (Rtable_t*)calloc(1,sizeof(Rtable_t))) )
			return -1;

		/* handles to transform table and field value data */
		type = vc->flags & (VC_ENCODE|VC_DECODE);
		if(!(rt->tbl = vcopen(0, Vctable, 0, 0, type)) )
			goto do_close;
		if(!(rt->bwt = vcopen(0, Vcbwt, 0, 0, type)) )
			goto do_close;

		/* create default context */
		rt->ctxt = (Rtctxt_t*)vcinitcontext(vc, NIL(Vccontext_t*));

		/* by default, contexts are maintained across windows */
		rt->renew = 0;

		/* reference characters for integer sequence conversion */
		rt->dot = '.';
		rt->dash = '/';
		rt->zero = '0';

		vcsetmtdata(vc, rt);
		goto vc_setarg;
	}
	else if(type == VC_SETMTARG) 
	{ vc_setarg :
		if(!(rc = vcgetcontext(vc, Rtctxt_t*)) )
			return -1;
		for(data = (char*)params; data; )
		{	data = vcgetmtarg(data, val, sizeof(val), _Rtargs, &arg);

			type = TYPECAST(int,arg->data);
			if(type == RT_RSEP || type == RT_FSEP ||
			   type == RT_ALIGN || type == RT_SCHEMA )
			{	rtctxtclear(rc);

				if(type == RT_ALIGN || type == RT_SCHEMA)
					rc->rsep = rc->fsep = 0;
			}

			switch(TYPECAST(int,arg->data))
			{ case RT_RENEW :
				rt = vcgetmtdata(vc, Rtable_t*);
				rt->renew = val[0] == '0' ? 0 : 1;
				break;
			  case RT_RSEP:
				rc->rsep = val[0] ? val[0] : 0;
				break;
			  case RT_FSEP:
				rc->fsep = val[0] ? val[0] : 0;
				break;
			  case RT_ALIGN:
				rc->algn = vcatoi(val);
				break;
			  case RT_SCHEMA:
				fldz = NIL(ssize_t*); /* get list of field sizes */
				if((fldn = vcstr2list(val, ',', &fldz)) < 0 || (fldn > 0 && !fldz) )
					return -1;
				for(recz = 0, k = 0; k < fldn; ++k)
				{	if(fldz[k] <= 0) /* bad schema definition */
					{	free(fldz);
						return -1;
					}
					recz += fldz[k]; /* count for record length */
				}
				rc->fldn = fldn;
				rc->fldz = fldz;
				rc->recz = recz;
				break;
			}
		}
		return 0;
	}
	else if(type == VC_INITCONTEXT) /* create a new context for transformation */
	{	if(!params)
			return 0;

		if(!(rc = (Rtctxt_t*)calloc(1,sizeof(Rtctxt_t))) )
			return -1;
		rc->fsep = 0;
		rc->rsep = 0;

		*((Rtctxt_t**)params) = rc;
		return 1;
	}
	else if(type == VC_FREECONTEXT) /* delete an existing context */
	{	if((rc = (Rtctxt_t*)params) )
			rtctxtclear(rc);
		free(rc);
		return 0;
	}
	else if(type == VC_FREEBUFFER)
	{	if((rt = vcgetmtdata(vc, Rtable_t*)) )
		{	if(rt->tbl)
				vcbuffer(rt->tbl, NIL(Vcchar_t*), -1, -1);
			if(rt->bwt)
				vcbuffer(rt->bwt, NIL(Vcchar_t*), -1, -1);
		}
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if(!(rt = vcgetmtdata(vc, Rtable_t*)) )
			return -1;
		rv = 0;

	do_close: /* free any allocated resources */
		if(rt->tbl)
		{	rt->tbl->coder = NIL(Vcodex_t*);
			vcclose(rt->tbl);
		}
		if(rt->bwt)
		{	rt->bwt->coder = NIL(Vcodex_t*);
			vcclose(rt->bwt);
		}
		free(rt);
		vcsetmtdata(vc, NIL(Rtable_t*));

		return rv;
	}

	return 0;
}

Vcmethod_t _Vcrtable =
{	rtable,
	unrtable,
	rtevent,
	"rtable", "Transforming/Compressing a relational data table.",
	"[-?\n@(#)$Id: vcodex-rtable (AT&T Research) 2011-07-20 $\n]" USAGE_LICENSE,
	_Rtargs,
	4*1024*1024,
	0
};

VCLIB(Vcrtable)
