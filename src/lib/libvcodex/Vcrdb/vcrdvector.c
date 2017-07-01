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

/*	Linear-time bucket sorting the records belonging to field.
**
**	While sorting, the data can also be transformed based on the
**	states of the sorted indices. Data transformation is done
**	on request by specifying 'trdt'. This method was found by
**	Binh Vo as a cheaper way to transform data than padding
**	the fields and using the Vctable transform. However, this
**	transformation often performs worse than Vctable so it is
**	implemented here for comparison testing purposes only.
**
**	Written by Kiem-Phong Vo
*/

/* structure to keep ranges of indices not completely sorted yet */
typedef struct _sort_s
{	struct _sort_s*	next;	/* fifo queue link	*/
	ssize_t*	vect;	/* vector part to sort	*/
	ssize_t		size;	/* number of elements	*/
} Sort_t;

#if __STD_C
int vcrdvector(Vcrdtable_t* tbl, ssize_t f, Vcchar_t* trdt, ssize_t trsz, int type )
#else
int vcrdvector(tbl, f, trdt, trsz, type)
Vcrdtable_t*	tbl;  /* table with data to be sorted	*/
ssize_t		f;    /* field to be sorted		*/
Vcchar_t*	trdt; /* space for transformed data	*/
ssize_t		trsz; /* size of above space		*/
int		type; /* encoding or decoding		*/
#endif
{
	ssize_t		p, r, cmin, cmax, cpos, flen, recn;
	Vcchar_t	*enddt, *rdt, *dt;
	Vcrdrecord_t	*rcrd;
	Sort_t		*so, *nv;
	Sort_t		*csort = NIL(Sort_t*), *nsort = NIL(Sort_t*);
	ssize_t		*vect, *vtmp = NIL(ssize_t*);
	int		c, *cldt = NIL(int*);
	ssize_t		*clsz;
	int		fsep = -1, rsep = -1;
	ssize_t		fvec[257], *freq = fvec+1; /* freq[-1] counts sorted records */
	int		allequal = 1; /* the status of all strings involved */
#define err_return(x)	do { (allequal = (x)); goto done; } while(0)

	if(!tbl || !tbl->info || f < 0 || f >= tbl->fldn)
		err_return(-1);
	if(tbl->recn <= 0)
		return 0;

	if(!trdt || trsz <= 0) /* if no data, computing transform vector only */
	{	trdt = NIL(Vcchar_t*);
		trsz = 0;
		type = VC_ENCODE;
	}

	rcrd = tbl->fld[f].rcrd; /* records being processed */

	/* temp space for sorting */
	if(!(vtmp = (ssize_t*)malloc(2*tbl->recn*sizeof(ssize_t))) )
		err_return(-1);

	flen = 0;
	if(type == VC_ENCODE)
	{	if(trdt) /* transforming data, do not change transform vector */
		{	enddt = trdt + trsz;
			vect = vtmp + tbl->recn;
		}
		else /* computing transform vector only */
		{	if(tbl->fld[f].type & VCRD_VECTOR)
				return 0;

			enddt = NIL(Vcchar_t*);
			vect = tbl->fld[f].vect;
		}
	}
	else /* if(type == VC_DECODE) */
	{	/**/DEBUG_ASSERT(trdt != NIL(Vcchar_t*));

		vect = tbl->fld[f].vect;
		clsz = vtmp + tbl->recn;

		if(!(cldt = (int*)malloc(tbl->recn*sizeof(int))) )
			err_return(-1);
		for(r = 0; r < tbl->recn; ++r)
		{	cldt[r] = 0; /* data for a column while being decoded */
			clsz[r] = 0; /* to calculate length of each row */
		}

		if(tbl->info->flen && (flen = tbl->info->flen[f]) > 0) /* fixed length field */
		{	if(trsz < flen*tbl->recn)
				err_return(-1);
			enddt = trdt + flen*tbl->recn;
		}
		else
		{	if((fsep = tbl->info->fsep) < 0 || (rsep = tbl->info->rsep) < 0)
				err_return(-1);
			enddt = trdt + trsz;
		}
	}

	/* start with the entire range */
	if(!(csort = (Sort_t*)malloc(sizeof(Sort_t))) )
		err_return(-1);
	csort->next = NIL(Sort_t*);
	csort->vect = vect;
	csort->size = tbl->recn;
	for(r = 0; r < tbl->recn; ++r)
		vect[r] = r;

	for(rdt = trdt, cpos = 0;; (csort = nsort), ++cpos )
	{	if(type == VC_ENCODE)
		{	if(!trdt) /* computing transform vector only */
				recn = tbl->recn;
			else /* transform data of current column */
			{	for(recn = 0, p = 0; p < tbl->recn; ++p)
				{	r = vect[p];
					if(cpos >= rcrd[r].dtsz )
						recn += 1; /* record completed */
					else if(rdt < enddt)
						*rdt++ = rcrd[r].data[cpos];
					else	err_return(-1);
				}
			}
		}
		else /* if(type == VC_DECODE) */
		{	/* reset signal for completed records */
			if(cpos > 0)
			{	for(p = 0; p < tbl->recn; ++p)
				{	if(clsz[p] < 0)
					{	clsz[p] = -clsz[p];
						cldt[p] = -1;
					}
				}
			}

			for(recn = 0, dt = rdt, p = 0; p < tbl->recn; ++p)
			{	r = vect[p]; /* record being processed */
				if(cldt[r] < 0 ) /* record already done */
					recn += 1;
				else if(dt < enddt) /* byte belongs to it */
				{	cldt[r] = *dt++;
					clsz[r] += 1;
				}
				else	err_return(-1); /* out of data? */
			}

			/* rewrite trdt[] with unsorted data */
			for(p = 0; p < tbl->recn; ++p)
			{	if(cldt[p] < 0)
					continue;
				*rdt++ = cldt[p];

				/* set state if record ends to reset cldt[] later above */
				if((flen >  0 && clsz[r] >= flen) ||
				   (flen <= 0 && (cldt[p] == fsep || cldt[p] == rsep)) )
					clsz[p] = -clsz[p];
			} /**/DEBUG_ASSERT(rdt == dt);
		}

		if(!csort && recn >= tbl->recn)
			break; /* done processing */

		/* radix sort based on current character position */
		for(nsort = NIL(Sort_t*); csort; )
		{	so = csort; csort = csort->next;

			memset(fvec, 0, sizeof(fvec)); /* compute frequencies */
			cmin = 256; cmax = -1;
			for(p = 0; p < so->size; ++p)
			{	r = so->vect[p]; /* record being looked at */
				if(type == VC_DECODE)
					c = cldt[r];
				else	c = rcrd[r].dtsz <= cpos ? -1 : rcrd[r].data[cpos];
				if(c >= 0 )
				{	cmin = c < cmin ? c : cmin;
					cmax = c > cmax ? c : cmax;
				}
				freq[c] += 1;
			}

			if(cmin > cmax) /* this range finishes with all elements equal */
				free(so);
			else if(cmin == cmax && freq[-1] <= 0) /* all same character */
			{	so->next = nsort; /* sort again by next position */
				nsort = so;
			}
			else /* if(cmin < cmax || freq[-1] > 0), i.e.,  multiple groups */
			{	allequal = 0; /* strings are different */

				r = freq[-1]; freq[-1] = 0; /* allocate group space */
				for(c = cmin; c <= cmax; ++c)
					{ p = freq[c]; freq[c] = r; r += p; }

				for(p = 0; p < so->size; ++p) /* sort by distribution */
				{	r = so->vect[p];
					if(type == VC_DECODE)
						c = cldt[r];
					else	c = rcrd[r].dtsz <= cpos ? -1 : rcrd[r].data[cpos];
					vtmp[freq[c]++] = r; /* move into its place */
				}
				for(p = 0; p < so->size; ++p) /* update sorted vector */
					so->vect[p] = vtmp[p];

				/* continue sorting nontrivial subgroups */
				for(r = freq[-1], c = cmin; c <= cmax; r = freq[c], c += 1)
				{	if((freq[c]-r) <= 1 )
						continue;
					if(!(nv = (Sort_t*)malloc(sizeof(Sort_t))) )
						err_return(-1);
					nv->vect = so->vect + r;
					nv->size = freq[c] - r;
					nv->next = nsort;
					nsort = nv;
				}

				free(so); /* this range has been partitioned */
			}
		}
	}

	if(trdt) /* set amount of processed data */
		tbl->parz = rdt - trdt;

	if(type == VC_DECODE) /* turn data from column-major to row-major */
	{	if(!(rdt = (Vcchar_t*)malloc(tbl->parz)) )
			err_return(-1);
		if(tbl->fld[f].data)
			free(tbl->fld[f].data);
		tbl->fld[f].data = rdt;

		for(cmax = 0, r = 0; r < tbl->recn; ++r)
		{	rcrd[r].data = rdt;
			rcrd[r].dtsz = clsz[r];
			rdt += clsz[r];
			if(clsz[r] > cmax)
				cmax = clsz[r];
			clsz[r] = 0;
		}

		if(flen > 0)
		{	if(flen != cmax)
				err_return(-1);
			tbl->fld[f].type |= VCRD_FIXED;
		}
		tbl->fld[f].maxz = cmax;

		for(dt = trdt, recn = 0; recn < tbl->recn;)
		{	for(r = 0; r < tbl->recn; ++r)
			{	if(clsz[r] < 0)
					continue;
				rcrd[r].data[clsz[r]] = *dt++;
				if((clsz[r] += 1) == rcrd[r].dtsz)
				{	clsz[r] = -1;
					recn += 1; /* count completed records */
				}
			}
		}
	}

#ifdef DEBUG /* check integrity of sorted vector */
	for(r = 1; r < tbl->recn; ++r)
	{	Vcchar_t	*s, *t;
		ssize_t		ns, nt;
		s = rcrd[vect[r-1]].data; ns = rcrd[vect[r-1]].dtsz;
		t = rcrd[vect[r]].data; nt = rcrd[vect[r]].dtsz;
		for(p = 0; p < ns && p < nt; ++p)
		{	/**/DEBUG_ASSERT(s[p] <= t[p]);
			if(s[p] < t[p])
				break;
		}
		if(p < ns)
			/**/DEBUG_ASSERT(p < nt && s[p] < t[p]);
		else /*if(p == ns)*/
		{	/**/DEBUG_ASSERT(ns <= nt);
			if(ns == nt)
				/**/DEBUG_ASSERT(vect[r-1] < vect[r]);
		}
	}
#endif

done:	/* free all locally allocated space */
	if(cldt)
		free(cldt);
	if(vtmp)
		free(vtmp);
	while((so = csort) != NIL(Sort_t*) )
	{	csort = so->next;
		free(so);
	} 
	while((so = nsort) != NIL(Sort_t*) )
	{	nsort = so->next;
		free(so);
	} 

	/* so we know that an internal transform vector was done */
	if(allequal >= 0 && !trdt )
		tbl->fld[f].type |= VCRD_VECTOR;

	return allequal;
}
