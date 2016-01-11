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
#include	"vcwhdr.h"

/*	Functions to compute n-gram frequencies, etc.
**
**	Written by Kiem-Phong Vo
*/

/* compute n-gram frequencies */
#if __STD_C
int vcwngfreq(size_t* freq, Vcchar_t* data, size_t size)
#else
int vcwngfreq(freq, data, size)
size_t*		freq;	/* frequency array size NG_FREQ	*/
Vcchar_t*	data;	/* data to be aggregated	*/
size_t		size;	/* size of data in bytes	*/
#endif
{
	size_t	n, gram;

	for(n = 0; n < NG_FREQ; ++n)
		freq[n] = 0;

	if(size < NG_BYTE)
		return 0;

	size -= NG_BYTE;
	for(n = 0, NGINIT(data,gram); ; ++n, ++data, NGNEXT(data,gram) )
	{	freq[NGINDEX(gram)] += 1;
		if(n >= size)
			return 0;
	}
}

/* compute the position in "data" whose frequency vector sfreq
** is closest to the frequency vector dfreq. 
*/
#if __STD_C
double vcwngmatch(int* mtch, size_t* dfreq, size_t size,
		  Vcchar_t* data, size_t dtsz, size_t pos, double stop)
#else
double vcwngmatch(mtch, dfreq, size, data, dtsz, pos, stop)
int*		mtch;	/* to return the best match	*/
size_t*		dfreq;	/* vector of data frequency  	*/
size_t		size;	/* size of data to be matched	*/
Vcchar_t*	data;	/* source data array		*/
size_t		dtsz;	/* size of source data		*/
size_t		pos;	/* starting position of search	*/
double		stop;	/* stop search criterion	*/
#endif
{
	size_t		lfreq[NG_FREQ], rfreq[NG_FREQ];	/* frequency vectors	*/
	Vcchar_t	*lldt, *lrdt;	/* boundaries of left data segment	*/
	Grint_t		llgr, lrgr;	/* left,right n-grams of left segment	*/
	size_t		ldif, lmax;	/* running statistics of left segment	*/
	Vcchar_t	*rldt, *rrdt;	/* boundaries of right data segment	*/
	Grint_t		rlgr, rrgr;	/* left,right n-grams of right segment	*/
	size_t		rdif, rmax;	/* running statistics of right segment	*/
	size_t		bestp, l, r;
	double		bestd;
	Vcchar_t	*edata;

	if(dtsz < size) /* worst possible match */
		return 1.;

	if(pos > (dtsz -= size) )
		pos = dtsz;

	vcwngfreq(rfreq, data+pos, size);

	/* initial frequency vector */
	rdif = rmax = 0;
	for(r = 0; r < NG_FREQ; ++r)
	{	if(dfreq[r] < rfreq[r])
		{	rdif += rfreq[r] - dfreq[r];
			rmax += rfreq[r];
		}
		else
		{	rdif += dfreq[r] - rfreq[r]; 
			rmax += dfreq[r];
		}
	}

	bestp = pos;
	bestd = rdif/(double)rmax;
	if(bestd < stop || dtsz == 0)
		goto done;

	/* starting boundaries of data */
	edata = data + dtsz;
	lldt = rldt = data + pos;
	lrdt = rrdt = data + pos + size - NG_BYTE;

	if(lldt > data)
	{	for(l = 0; l < NG_FREQ; ++l)
			lfreq[l] = rfreq[l];
		ldif = rdif;
		lmax = rmax;

		NGINIT(lldt,llgr);
		NGINIT(lrdt,lrgr);
	}

	if(rldt < edata)
	{	NGINIT(rldt, rlgr);
		NGINIT(rrdt, rrgr);
	}

	while(lldt > data || rldt < edata)
	{	if(lldt > data)
		{	lldt -= 1; NGINIT(lldt, llgr); l = NGINDEX(llgr); /* l coming  */
			r = NGINDEX(lrgr); lrdt -= 1; NGINIT(lrdt, lrgr); /* r leaving */
			if(l != r)
			{	if((lfreq[r] -= 1) < dfreq[r])
					ldif += 1;
				else	{ ldif -= 1; lmax -= 1; }

				if((lfreq[l] += 1) > dfreq[l])
					{ ldif += 1; lmax += 1; }
				else	ldif -= 1;

				if((ldif/(double)lmax) < bestd)
				{	bestd = ldif/(double)lmax;
					bestp = lldt-data;
					if(bestd < stop)
						goto done;
				}
			}
		}

		if(rldt < edata)
		{	l = NGINDEX(rlgr); rldt += 1; NGNEXT(rldt, rlgr); /* l leaving */
			rrdt += 1; NGNEXT(rrdt, rrgr); r = NGINDEX(rrgr); /* r coming  */
			if(l != r)
			{	if((rfreq[l] -= 1) < dfreq[l])
					rdif += 1;
				else	{ rdif -= 1; rmax -= 1; }

				if((rfreq[r] += 1) > dfreq[r])
					{ rdif += 1; rmax += 1; }
				else	rdif -= 1;

				if((rdif/(double)rmax) < bestd)
				{	bestd = rdif/(double)rmax;
					bestp = rldt-data;
					if(bestd < stop)
						goto done;
				}
			}
		}
	}

done:	*mtch = bestp;
	return bestd;
}

/* The signature of a segment of data  is the sum of all its n-grams.
   Thus, each 1K (2^10) segment has a maximum signature of size 2^25.
*/
#if __STD_C
Grint_t vcwngsig(Vcchar_t* data, size_t n)
#else
Grint_t vcwngsig(data, n)
Vcchar_t*	data;
size_t		n;
#endif
{
	Grint_t		sig, key;
	Vcchar_t*	endd;

	endd = data + n - (NG_BYTE-1);
	for(sig = 0, NGINIT(data,key), data += 1; data < endd; ++data)
	{	sig += NGVALUE(key);
		NGNEXT(data,key);
	}

	return sig;
}
