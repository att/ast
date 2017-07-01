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
#include	<vcrdb.h>

/* Functions to build tables of relational data, ie, records and fields.
**
** Written by Kiem-Phong Vo
*/

#define		MAXDOT	127		/* convert max of 127 elements	*/
#define		MAXBUF	((MAXDOT+1)*4)	/* size of buffer to decode	*/

#if __STD_C
Vcrdtable_t* vcrdparse(Vcrdinfo_t* info, Vcchar_t* data, ssize_t dtsz, int type)
#else
Vcrdtable_t* vcrdparse(info, data, dtsz, type)
Vcrdinfo_t*	info;	/* information about the data	*/
Vcchar_t*	data;	/* the data to be parsed	*/
ssize_t		dtsz;	/* the size of the data		*/
int		type;	/* VCRD_FIELD if field-oriented	*/
#endif
{
	ssize_t		fldn, recn, f, r, z, maxz;
	Vcchar_t	*dt, *orig, *enddt;
	Vcrdrecord_t	*rcrd;
	ssize_t		*vect;
	Vcrdtable_t	*tbl;

	if(!info)
		return NIL(Vcrdtable_t*);

	if(!(orig = data) || dtsz <= 0)
	{	dtsz = 0;
		fldn = info->fldn; /* fldn&recn must be given if no data to parse */
		recn = info->recn;
	}
	else if(type&VCRD_FIELD ) /* field-oriented data, recn&fldn&flen all needed */
	{	if((recn = info->recn) <= 0 || (fldn = info->fldn) <= 0 || !info->flen)
			return NIL(Vcrdtable_t*);

		for(enddt = data+dtsz, f = 0; f < fldn; ++f)
		{	if(data >= enddt)
				return NIL(Vcrdtable_t*);
			if((z = info->flen[f]) > 0 ) /* fixed-length field */
				data += recn*z;
			else /*if(z <= 0): variable-length field */
			{	if(info->fsep < 0 || info->rsep < 0 )
					return NIL(Vcrdtable_t*);
				for(r = 0; r < recn; ++r)
				{	for(; data < enddt; ++data)
						if(*data == info->fsep || *data == info->rsep)
							break;
					if((data += 1) > enddt) /* incomplete data */
						return NIL(Vcrdtable_t*);
				}
			}
		}
	}
	else /* record-oriented data, fldn & recn can be learned */
	{	for(enddt = data+dtsz, fldn = recn = 0;; )
		{	for(f = 0, dt = data;;)
			{	if(info->fldn > 0 && info->flen && (z = info->flen[f]) > 0 )
				{	if((dt += z) > enddt) /* fixed length field */
					{	f = -1; /* incomplete last record */
						break;
					}
					if((f += 1) == info->fldn)
						break;
				}
				else /* must have valid fsep and rsep to work here */
				{	if(info->fsep < 0 || info->rsep < 0 )
						return NIL(Vcrdtable_t*);

					for(; dt < enddt; ++dt)
						if(*dt == info->fsep || *dt == info->rsep)
							break;
					if(dt >= enddt)
					{	f = -1; /* incomplete last record */
						break;
					}
					else
					{	f += 1;
						dt += 1; /* start at next element */
					}

					if(info->fldn <= 0 ) /* learning fldn */
					{	if(dt[-1] == info->rsep)
							break; /* record ended */
					}
					else if(f == info->fldn) /* last field in record */
					{	if(dt[-1] != info->rsep)
							f = -1; /* ill-formed record */
						break;
					}
					else if(dt[-1] != info->fsep)
					{	f = -1; /* ill-formed record */
						break;
					}
				}
			}

			if(f <= 0) /* data ended with an incomplete record */
				break;
			else /* just processed a complete record */
			{	data = dt;
				recn += 1; /* set number of records and fields */
				if(f > fldn)
					fldn = f;
			}
		}
	}

	if(fldn <= 0 || recn <= 0) /* empty table not allowed */
		return NIL(Vcrdtable_t*);

	/* allocate structure for keeping table data */
	z = sizeof(Vcrdtable_t) + /* size of the table itself */
	    fldn*sizeof(Vcrdfield_t) + /* size of the fields */
	    fldn*recn*sizeof(Vcrdrecord_t) + /* pointers to record data in fields */
	    fldn*recn*sizeof(ssize_t); /* space for transform vectors */
	if(!(tbl = (Vcrdtable_t*)calloc(1, z)) )
		return NIL(Vcrdtable_t*);
	tbl->info = info;
	tbl->recn = recn;
	tbl->fldn = fldn;
	tbl->fld  = (Vcrdfield_t*)(tbl+1);
	rcrd = (Vcrdrecord_t*)(tbl->fld + fldn);
	vect = (ssize_t*)(rcrd + fldn*recn);
	for(f = 0; f < fldn; ++f, rcrd += recn, vect += recn)
	{	tbl->fld[f].rcrd = rcrd;
		tbl->fld[f].vect = vect;
	}

	tbl->parz = dtsz = data-orig; /* size of data consumed */
	if(tbl->parz == 0) /* data will be filled in later */
		return tbl;

	if(type&VCRD_FIELD) /* parse field-oriented data into the table. */
	{	for(enddt = (data=orig)+dtsz, f = 0; f < fldn; ++f)
		{	if((z = info->flen[f]) > 0) /* fixed-length */
			{	for(maxz = 0, r = 0; r < recn; ++r)
				{	tbl->fld[f].rcrd[r].data = data;
					tbl->fld[f].rcrd[r].dtsz = z;
					if(z > maxz)
						maxz = z;
					data += z;
				}
			}
			else /* if(z <= 0): delineated field */
			{	for(maxz = 0, r = 0; r < recn; ++r)
				{	for(dt = data; dt < enddt; ++dt)
						if(*dt == info->fsep || *dt == info->rsep)
							break;
					z = (dt-data)+1;
					tbl->fld[f].rcrd[r].data = data;
					tbl->fld[f].rcrd[r].dtsz = z;
					if(z > maxz)
						maxz = z;
					data += z;
				}
			}

			tbl->fld[f].maxz = maxz;
		}
	}
	else /* record-oriented data */
	{	for(enddt = (data=orig)+dtsz, r = 0; r < recn; ++r )
		{	for(f = 0; f < fldn; ++f)
			{	tbl->fld[f].rcrd[r].data = data;

				if(info->fldn > 0 && (z = info->flen[f]) > 0)
					data += z; /* fixed-length data */
				else /* variable length data */
				{	for(dt = data; dt < enddt; ++dt)
						if(*dt == info->fsep || *dt == info->rsep)
							break;
					z = (dt-data) + 1;
					data += ((*dt != info->rsep) || (f+1) == fldn ) ? z : z-1;
				}

				tbl->fld[f].rcrd[r].dtsz = z;
				if(z > tbl->fld[f].maxz)
					tbl->fld[f].maxz = z;
			}
		}
	}

	if(info->flen) /* set field states */
		for(f = 0; f < fldn; ++f)
			if(info->flen[f] > 0) /* fixed length */
				tbl->fld[f].type |= VCRD_FIXED;

	return tbl;
}

/* fill a particular field with data */
#if __STD_C
ssize_t vcrdfield(Vcrdtable_t* tbl, ssize_t f, ssize_t flen, Vcchar_t* data, ssize_t dtsz)
#else
ssize_t vcrdfield(tbl, f, flen, data, dtsz)
Vcrdtable_t*	tbl;	/* table to parse into	*/
ssize_t		f;	/* field index		*/
ssize_t		flen;	/* fixed length if > 0	*/
Vcchar_t*	data;	/* field data		*/
ssize_t		dtsz;	/* data size		*/
#endif
{
	ssize_t		maxz, z, r, recn;
	int		fsep, rsep;
	Vcrdrecord_t	*rcrd;
	Vcchar_t	*dt, *enddt, *orig;

	if(!tbl || !tbl->info || (recn = tbl->recn) <= 0 || f < 0 || f >= tbl->fldn)
		return -1;

	enddt = (orig = data) + dtsz;

	fsep = tbl->info->fsep; rsep = tbl->info->rsep;

	rcrd = tbl->fld[f].rcrd;
	if(flen > 0) /* fixed-length field */
	{	if(recn*flen > dtsz)
			return -1;
		for(r = 0; r < recn; ++r)
		{	rcrd[r].data = data;
			rcrd[r].dtsz = flen;

			data += flen;
		}
		tbl->fld[f].maxz = flen;
		tbl->fld[f].type |= VCRD_FIXED;
	}
	else /* if(flen <= 0): separator-terminated field */
	{	if(fsep < 0 || rsep < 0)
			return -1;

		for(r = 0; r < recn; ++r)
		{	for(dt = data; dt < enddt; ++dt)
				if(*dt == fsep || *dt == rsep)
					break;
			if(dt >= enddt)
				return -1;
		}

		for(maxz = 0, r = 0; r < recn; ++r)
		{	for(dt = data; dt < enddt; ++dt)
				if(*dt == fsep || *dt == rsep)
					break;
			z = (dt-data)+1;

			rcrd[r].data = data;
			rcrd[r].dtsz = z;
			if(z > maxz)
				maxz = z;

			data += z;
		}
		tbl->fld[f].maxz = maxz;
	}

	return (tbl->parz = (data-orig));
}

/* convert xxx.yyy.zzz... into a sequence of bytes */
#if __STD_C
static ssize_t text2byte(Vcrdinfo_t* info, Vcchar_t* dt, Vcchar_t* cvdt, int flde, int* type)
#else
static ssize_t text2byte(info, dt, cvdt, flde, type)
Vcrdinfo_t*	info;	/* table information	*/
Vcchar_t*	dt;	/* data to convert	*/
Vcchar_t*	cvdt;	/* buffer to convert to	*/
int		flde;	/* required end char	*/
int*		type;	/* conversion type	*/
#endif
{
	int	f, byte, dign, ty;

	for(f = 0, ty = VCRD_DOT;; ++f)
	{	if(f >= MAXDOT) /* restrict conversion to this */
			return -1;

		for(byte = 0, dign = 0;; ++dt)
		{	if(*dt == info->fsep || *dt == info->rsep)
			{	if(dign == 0) /* no conversion done */
					return -1;

				/* see if conversion end character and type match */
				if(*dt != flde || (*type >= 0 && ty != *type) )
					return -1;

				if(cvdt) /* set the converted byte */
					cvdt[f] = (Vcchar_t)byte;

				*type =  ty; /* return the format (dot or dot|slash) */
				return f+1; /* return number of converted bytes */
			}
			else if(*dt == info->dot || *dt == info->slash )
			{	if(dign == 0) /* no conversion */
					return -1;
				if(ty & VCRD_SLASH) /* this must have been last */
					return -1;

				if(*dt == info->slash) /* exactly one more conversion after this */
					ty |= VCRD_SLASH;

				if(cvdt) /* set the converted byte */
					cvdt[f] = (Vcchar_t)byte;

				dt += 1; /* move to next conversion */
				break;
			}
			else if(*dt < info->digit[0] || *dt > info->digit[9])
				return -1; /* not a number */
			else
			{	if(byte == 0 && dign > 0) /* no leading zero allowed */
					return -1;

				dign += 1;
				byte = byte*10 + (*dt - info->digit[0]);
				if(byte >= 256) /* no longer storable in a byte */
					return -1;
			}
		}
	}
}


/* convert a field value from fixed-length format to printable format */
#if __STD_C
static ssize_t byte2text(Vcrdinfo_t* info, Vcchar_t* dt, ssize_t sz,
			 Vcchar_t* cvdt, ssize_t cvsz, int flde, int type)
#else
static ssize_t byte2text(info, dt, sz, cvdt, cvsz, flde, type)
Vcrdinfo_t*	info;	/* table information	*/
Vcchar_t*	dt;	/* data to convert	*/
ssize_t		sz;	/* data length		*/
Vcchar_t*	cvdt;	/* conversion buffer	*/
ssize_t		cvsz;	/* buffer length	*/
int		flde;	/* ending character	*/
int		type;	/* DOT, SLASH, etc.	*/
#endif
{
	ssize_t		z;
	int		b;

	if(sz <= 0 || sz > MAXDOT)
		return -1;

	for(z = 0;;)
	{	/* convert a byte into a sequence of digits */
		if((b = *dt++) >= 100)
		{	if((z += 1) > cvsz)
				return -1;
			*cvdt++ = info->digit[b/100]; b %= 100;
			goto do_2;
		}
		else if(b >= 10)
		{ do_2: if((z += 1) > cvsz)
				return -1;
			*cvdt++ = info->digit[b/10]; b %= 10;
			goto do_1;
		}
		else
		{ do_1: if((z += 1) > cvsz)
				return -1;
			*cvdt++ = info->digit[b];
		}

		/* add separator characters */
		if((z += 1) > cvsz)
			return -1;
		if((sz -= 1) == 0) /* last part done, add ending character */
		{	*cvdt++ = (Vcchar_t)flde;
			return z;
		}
		else if(sz == 1 && (type&VCRD_SLASH) ) /* n.n.../n notation */
			*cvdt++ = info->slash;
		else	*cvdt++ = info->dot; 
	}
}

#if __STD_C
static int flddot(Vcrdtable_t* tbl, ssize_t f, int attrs, int type)
#else
static int flddot(tbl, f, type)
Vcrdtable_t*	tbl;	/* table data		*/
ssize_t		f;	/* field to convert	*/
int		attrs;	/* attributes		*/
int		type;	/* 1/0: encode/decode	*/
#endif
{
	ssize_t		r, z, dtsz, cvsz;
	int		flde; 
	Vcchar_t	*data, cvdt[MAXBUF];
	Vcrdrecord_t	*rcrd;
	Vcrdinfo_t	*info = tbl->info;

	if(type != 0) /* encoding */
	{	if(tbl->fld[f].type & VCRD_FIXED)
			return 0;

		rcrd = tbl->fld[f].rcrd;
		type = -1; /* all records must match each other */
		flde = f < tbl->fldn ? info->fsep : info->rsep;
		if((z = text2byte(info, rcrd[0].data, NIL(Vcchar_t*), flde, &type)) <= 0)
			return 0;

		/* allocate space and convert all records */
		if(!(data = (Vcchar_t*)malloc(z*tbl->recn)) )
			return 0;
		for(r = 0; r < tbl->recn; ++r)
			if(text2byte(info, rcrd[r].data, data+z*r, flde, &type) != z)
				break;

		if(r < tbl->recn)	/* can't convert uniformly, so don't */
		{	free(data);
			return 0;
		}

		/* conversion succeeded, reset pointers to data */
		if(tbl->fld[f].data)
			free(tbl->fld[f].data);
		tbl->fld[f].data = data; /* save this to free on closing table */
		tbl->fld[f].type = type | VCRD_FIXED; /* field is now fixed length */
		for(r = 0; r < tbl->recn; ++r)
		{	rcrd[r].data = data; data += z;
			rcrd[r].dtsz = z;
		}
		tbl->fld[f].maxz = z;
	}
	else /* decoding */
	{	if(!(attrs & (VCRD_DOT|VCRD_SLASH)) ||
		   !(tbl->fld[f].type & VCRD_FIXED) )
			return 0;

		rcrd = tbl->fld[f].rcrd;
		data = NIL(Vcchar_t*); cvsz = dtsz = 0;
		type = (tbl->fld[f].type|attrs) & (VCRD_DOT|VCRD_SLASH);
		flde = f < (tbl->fldn-1) ? info->fsep : info->rsep;
		for(r = 0; r < tbl->recn; ++r)
		{	z = byte2text(tbl->info, rcrd[r].data, rcrd[r].dtsz, cvdt, sizeof(cvdt), flde, type);
			if(z <= 0) /* conversion failed */
			{	if(data)
					free(data);
				return -1;
			}
			if((cvsz + z) > dtsz)
			{	dtsz += (dtsz - cvsz) + z; /* get a rounded and big buffer */
				dtsz = ((dtsz+MAXBUF-1)/MAXBUF)*MAXBUF;
				if(!(data = (Vcchar_t*)realloc(data, dtsz)) )
					return -1;
			}
			memcpy(data+cvsz, cvdt, z);
			cvsz += z; /* total converted data size */
			rcrd[r].dtsz = z; /* reset record size */
		}

		/* reset data */
		if(tbl->fld[f].data)
			free(tbl->fld[f].data);
		tbl->fld[f].data = data;
		for(z = 0, r = 0; r < tbl->recn; ++r)
		{	rcrd[r].data = data;
			data += rcrd[r].dtsz;
			if(rcrd[r].dtsz > z)
				z = rcrd[r].dtsz;
		}
		tbl->fld[f].type &= ~(VCRD_DOT|VCRD_SLASH|VCRD_FIXED);
		tbl->fld[f].maxz = z;
	}

	return 0;
}

/* make a fixed field into a variable field */
#if __STD_C
static int fldvariable(Vcrdtable_t* tbl, ssize_t f)
#else
static int fldvariable(tbl, f)
Vcrdtable_t*	tbl;	/* table data	*/
ssize_t		f;	/* field index	*/
#endif
{
	ssize_t		r, z, maxz;
	int		c;
	Vcrdinfo_t	*info = tbl->info;
	Vcrdrecord_t	*rcrd = tbl->fld[f].rcrd;

	if(info->fsep < 0 || info->rsep < 0)
		return -1;

	/* fix each record by its end character - note that this is irreversible */
	if(tbl->fld[f].type & VCRD_FIXED)
	{	for(r = 0, maxz = 0; r < tbl->recn; ++r)
		{	if((z = rcrd[r].dtsz) != tbl->fld[f].maxz)
				break; /* really bad here but not our problem */

			if((c = rcrd[r].data[z -= 1]) != info->fsep && c != info->rsep)
				break; /* not a field that can be made variable */

			for(; z > 0; --z) /* find the first instance of c */
				if(rcrd[r].data[z-1] != c)
					break;

			if((z += 1) > maxz) /* reset the field length */
				maxz = z;
			rcrd[r].dtsz = z;
		}

		if(r == tbl->recn) /* success */
		{	tbl->fld[f].type &= ~VCRD_FIXED;
			tbl->fld[f].maxz = maxz;
		}
		else /* cannot turn this field into variable length */
		{	for(maxz = tbl->fld[f].maxz; r >= 0; --r)
				rcrd[r].dtsz = maxz;
			return -1;
		}
	}

	return 0;
}

/* set attributes */
#if __STD_C
int vcrdattrs(Vcrdtable_t* tbl, ssize_t f, int attrs, int type)
#else
int vcrdattrs(tbl, f, attrs, type)
Vcrdtable_t*	tbl;	/* table of data	*/
ssize_t		f;	/* f >=0: single field	*/
int		attrs;	/* attributes to un/set	*/
int		type;	/* 1: on, 0: off	*/
#endif
{
	ssize_t	endf;

	if(!tbl)
		return -1;

	if(f >= 0)
	{	if(f >= tbl->fldn)
			return -1;
		endf = f+1;
	}
	else
	{	f = 0;
		endf = tbl->fldn;
	}

	for(; f < endf; ++f)
	{	if(attrs&VCRD_DOT) /* this should be done before VCRD_PAD */
		{	if(tbl->info->dot < 0 || tbl->info->slash < 0)
				return -1;
			flddot(tbl, f, attrs, type);

			attrs &= ~VCRD_PAD; /* don't monkey with this */
		}

		if(attrs&VCRD_PAD)
		{	if(fldvariable(tbl,f) < 0)
				return -1;
			if(type) /* on variable mode + padding */
				tbl->fld[f].type |=  VCRD_PAD;
			else	tbl->fld[f].type &= ~VCRD_PAD;
		}
	}

	return 0;
}

/* close a table */
#if __STD_C
void vcrdclose(Vcrdtable_t* tbl)
#else
void vcrdclose(tbl)
Vcrdtable_t*	tbl;
#endif
{
	ssize_t	f;

	if(tbl)
	{	if(tbl->fldn > 0 && tbl->fld) /* free any field data */
			for(f = 0; f < tbl->fldn; ++f)
				if(tbl->fld[f].data)
					free(tbl->fld[f].data);
		free(tbl);
	}
}

/* Find boundary of a record without any padding.
** The idea is that every record ends with a unique rsep character.
** Thus, any "non-last" field consisting of only rsep must be a padding.
*/
#if __STD_C
ssize_t fldend(Vcrdtable_t* tbl, ssize_t r)
#else
ssize_t fldend(tbl, r)
Vcrdtable_t*	tbl;	/* table data		*/
ssize_t		r;	/* record being checked	*/
#endif
{
	ssize_t 	f;
	int		rsep = tbl->info->rsep;
	Vcrdfield_t	*fld = tbl->fld;

	f = tbl->fldn-1; /* check last field separately */
	if((fld[f].type & VCRD_FIXED) || fld[f].rcrd[r].dtsz > 1)
		return f+1;

	if(fld[f].rcrd[r].data[0] != rsep) /* something really bad happened */
		return -1;

	for(f -= 1; f >= 0; --f) /* skip back past padded data */
		if((fld[f].type&VCRD_FIXED) || fld[f].rcrd[r].dtsz > 1 || fld[f].rcrd[r].data[0] != rsep )
			break;

	return f+2; /* f+1 is the last field with real data */
}

/* get the data from a field or entire table */
#if __STD_C
ssize_t vcrdextract(Vcrdtable_t* tbl, ssize_t fldi, Vcchar_t* data, ssize_t dtsz, int type )
#else
ssize_t vcrdextract(tbl, fldi, data, type)
Vcrdtable_t*	tbl;	/* table to get data from	*/
ssize_t		fldi;	/* field index or -1 for all	*/
Vcchar_t*	data;	/* buffer to return data	*/
ssize_t		dtsz;	/* buffer size			*/
int		type;	/* VCRD_FIELD/VCRD_RECORD	*/
#endif
{
	ssize_t		f, r, endf, sz;
	Vcchar_t	fsep;
	Vcrdrecord_t	*rcrd, *prev;
	Vcrdfield_t	*fld;
	Vcrdinfo_t	*info;

	if(!tbl || !(fld = tbl->fld) || !(info = tbl->info) )
		return -1;

	/* compute needed buffer size for data and allocate buffer */
	sz = 0;
	if(fldi >= 0) /* getting just one field */
	{	if(fldi >= tbl->fldn)
			return -1;
		endf = fldi+1;
		type |= VCRD_FIELD|VCRD_PAD;
	}
	else
	{	fldi = 0;
		endf = tbl->fldn;	
	}
	for(f = fldi; f < endf; ++f)
	{	if(fld[f].type & (VCRD_FIXED | VCRD_PAD) )
			sz += tbl->recn * fld[f].maxz;
		else
		{	rcrd = fld[f].rcrd;
			prev = f > 0 ? fld[f-1].rcrd : NIL(Vcrdrecord_t*);
			for(r = 0; r < tbl->recn; ++r)
			{	if(!(type&VCRD_PAD) && prev &&
				   rcrd[r].data[0] == info->rsep &&
				   prev[r].data[prev[r].dtsz-1] == info->rsep)
					continue;

				sz += rcrd[r].dtsz;
			}
		}
	}

	if(!data || dtsz <= 0 || sz <= 0)
		return sz; /* application only wants the size of data */

	if(data && dtsz > 0 && dtsz < sz)
		return -1; /* buffer too small for data */

	if(type&VCRD_FIELD) /* note that field-oriented format implies VCRD_PAD */
	{	for(f = fldi; f < endf; ++f)
		{	for(rcrd = fld[f].rcrd, r = 0; r < tbl->recn; ++r)
			{	memcpy(data, rcrd[r].data, (dtsz = rcrd[r].dtsz) );
				data += dtsz;
				if(fld[f].type&VCRD_PAD) /* pad field to make all parts equal */
					for(fsep = data[-1]; dtsz < fld[f].maxz; ++dtsz)
						*data++ = fsep;
			}
		}
	}
	else /* record-oriented output */
	{	for(r = 0; r < tbl->recn; ++r)
		{	endf = (type&VCRD_PAD) ? tbl->fldn : fldend(tbl,r);
			for(f = 0; f < endf; ++f)
			{	if(!(type&VCRD_PAD) && f > 0 &&
				   fld[f].rcrd[r].data[0] == info->rsep &&
				   fld[f-1].rcrd[r].data[fld[f-1].rcrd[r].dtsz-1] == info->rsep )
					continue;
					
				memcpy(data, fld[f].rcrd[r].data, (dtsz = fld[f].rcrd[r].dtsz));
				data += dtsz;
				if(fld[f].type&VCRD_PAD) /* pad field to make all parts equal */
					for(fsep = data[-1]; dtsz < fld[f].maxz; ++dtsz)
						*data++ = fsep;
			}
		}
	}

	return sz;
}
