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
	Vcinx_t		*dist, *lcp;
	Vcinx_t		i, k, p, m, n, s, maxs, sz;
	Vcinx_t		*peak, period;
	Vcsfx_t		*sfx;

	if(!data || (sz = (Vcinx_t)dtsz) <= 16 /* too little data */ )
		return -1;

	/* sfx is the indices of substrings sorted in lex order */
	if(!(lcp = (Vcinx_t*)calloc(1, sz*sizeof(Vcinx_t))) )
		return -1;
	if(!(sfx = vcsfxsort(data, (size_t)sz)) )
	{	free(lcp);
		return -1;
	}

	/* lcp[] keeps lengths of longest common prefixes between adj elts in sfx */
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
	dist = sfx->inv; memset(dist, 0, sz*sizeof(Vcinx_t));
	maxs = 4*vclogi(sz); /* bound search to save time */
	for(i = 0; i < sz; ++i)
	{	for(m = 0, s = maxs, k = i+1; k < sz && s >= 0; ++k, --s)
		{	if(lcp[k] == 0)
				break;
			if((m = sfx->idx[k] - sfx->idx[i]) > 0 ) /* match distance */
				break;
		}
		for(n = 0, s = maxs, p = i-1; p >= 0 && s >= 0; --p, --s)
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

#define NEIGHBOR	2 /* #neighbors to check on each side */
	peak = sfx->idx; memset(peak, 0, sz*sizeof(Vcinx_t));

	maxs = 0; /* max #matches at any distance */
	for(p = 0; p < sz/2; ++p)
		if(dist[p] > maxs)
			maxs = dist[p];
	maxs = maxs/8 > 0 ? maxs/8 : maxs/4 > 0 ? maxs/4 : maxs/2 > 0 ? maxs/2 : maxs;

	for(s = 0, p = 0; p < 2*NEIGHBOR+1; ++p)
		s += dist[p]; /* total size of neighbors of p=NEIGHBOR */
	for(p = NEIGHBOR; 2*p < sz; ++p ) /* a peak is > 2*(sum of neighbors) */
	{	if(dist[p] >= maxs && dist[p] > 2*(s-dist[p]) )
			peak[p] = 1;
		s += dist[p+NEIGHBOR+1] - dist[p-NEIGHBOR];
	}

	period = 0; /* elect best period */
	maxs = 2*vclogi(sz); /* a candidate period must have many repeats */
	for(p = NEIGHBOR; 2*p < sz; ++p)
	{	if(!peak[p] || dist[p] < dist[period]/8 )
			continue;

		n = 1; /* count number of repeats */
		for(s = 0, k = p+p; s < maxs && 2*k < sz; s += 1, k += p)
			if(dist[k] > 0)
				n += 1;
		if(2*n < maxs) /* too few repeats to be a period */
			continue;

		if(period == 0)
			period = p;
		else
		{	m = n = 0; /* count repeats for period and p */
			for(k = 2; k*p < sz; ++k)
			{	m += dist[k*period];
				n += dist[k*p];
			}
			if(n > m)
				period = p;
		}
	}

	if(period == 0) /* pick early peak unless something else dramatically better */
	{	for(p = NEIGHBOR; 2*p < sz; ++p)
		{	if(!peak[p] || dist[p] < maxs || dist[p] <= dist[period] )
				continue;
			else if(period == 0)
				period = p;
			else if((p%period) != 0 && dist[p] > (p/period)*dist[period] )
				period = p;
		}
	}

	free(lcp);
	free(sfx);
	return (ssize_t)period;
}
