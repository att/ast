/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"vchdr.h"

/* Compute the highest period in all quasi-cycles of a data set.
**
** Written by Kiem-Phong Vo
*/

#if __STD_C
ssize_t vcperiod(const Void_t* data, ssize_t dtsz)
#else
ssize_t vcperiod(data, dtsz)
Void_t*	data;
ssize_t	dtsz;
#endif
{
	Vcchar_t	*dt;
	Vcsfxint_t	*dist, *peak, *lcp;
	Vcsfxint_t	i, n, k, p, m, s, c, sz;
	Vcsfx_t		*sfx;

#define PEAKBOUND	3 /* bound to check for local peaks */
	if(!data || (sz = (Vcsfxint_t)dtsz) <= PEAKBOUND*PEAKBOUND)
		return -1;

	/* compute suffix array and longest-common-prefix array */
	if(!(lcp = (Vcsfxint_t*)calloc(1, sz*sizeof(Vcsfxint_t))) )
		return -1;
	if(!(sfx = vcsfxsort(data, (size_t)sz)) )
	{	free(lcp);
		return -1;
	}
	for(dt = (Vcchar_t*)data, p = 0, i = 0; i < sz; ++i)
	{	if(sfx->inv[i] == 0)
			continue;
		k = sfx->idx[sfx->inv[i]-1];
		while((i+p) < sz && (k+p) < sz && dt[i+p] == dt[k+p])
			p += 1;
		lcp[sfx->inv[i]] = p;
		if(p > 0)
			p -= 1;
	}

	/* dist[k] counts number of matches at distance k */
	dist = sfx->inv; memset(dist, 0, sz*sizeof(Vcsfxint_t));
	c = vclogi(sz); /* bound search to save time */
	for(i = 0; i < sz; ++i)
	{	for(m = 0, s = c, k = i+1; k < sz && s >= 0; ++k, --s)
		{	if(lcp[k] == 0)
				break;
			if((m = sfx->idx[k] - sfx->idx[i]) > 0 ) /* match distance */
				break;
		}
		for(n = 0, s = c, p = i-1; p >= 0 && s >= 0; --p, --s)
		{	if(lcp[p] == 0)
				break;
			if((n = sfx->idx[p] - sfx->idx[i]) > 0 ) /* match distance */
				break;
		}
		if(m > 0 && m <= n) /* count the closer one */
			dist[m] += 1;
		else if(n > 0 && n <= m)
			dist[n] += 1;
	}

	/* compute an array of candidate peaks */
	peak = sfx->idx; memset(peak, 0, sz*sizeof(Vcsfxint_t));

	/* initialize running sum of distances of a suitable neighborhood */
	for(m = 0, i = 0; i < 2*PEAKBOUND+1; ++i)
		m += dist[i];

	/* a peak is larger than the total sum of a small neighborhood around it */
	for(n = sz - 2*PEAKBOUND, i = PEAKBOUND; i < n; ++i)
	{	if(dist[i] > 2*(m - dist[i]) )
			peak[i] = 1;
		m = m - dist[i-PEAKBOUND] + dist[i+PEAKBOUND];
	}
#ifdef DEBUG
	for(i = 2; i < sz; ++i)
	{	if(dist[i] <= 0)
			continue;
		DEBUG_PRINT(3,"%d: ",i); DEBUG_PRINT(3,"%d",dist[i]);
		if(peak[i])
			DEBUG_WRITE(3, "<p>", 3);
		DEBUG_WRITE(3,"\n", 1);
	}
#endif

	/* a period is a peak with multiples that have matches */
	p = 0; m = 0; c = vclogi(sz);
	for(i = PEAKBOUND; 2*i < sz; ++i)
	{	if(!peak[i] || !peak[2*i])
			continue;
		for(s = 1, n = dist[i], k = i+i; k < sz; k += i)
		{	if(dist[k] == 0)
				continue;
			else if(dist[k] > 2*n ) /* k is a better period */
			{	n = 0;
				break;
			}

			n += dist[k]; /* sum the weights */
			if(peak[k])
			{	s += 1; /* count peaks */
				peak[k] = 0;
			}
		}

		if(s > c && n > m )
			{ p = i; m = n; c = s; }
	}

	/**/DEBUG_PRINT(2, "dtsz=%d, ", sz); DEBUG_PRINT(2, "period=%d\n", p);
	free(lcp);
	free(sfx);
	return (ssize_t)p;
}
