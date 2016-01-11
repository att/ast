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
#include	"vcdhdr.h"

/*	Transforming data by byte-wise differencing
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
static ssize_t hamming(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t hamming(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		srcsz, tarsz, s, t, maxs;
	Vcchar_t	*srcdt, *tardt, *output, *dt;
	Vcdisc_t	*disc;

	if(!vc)
		return -1;

	vc->undone = 0;

	/* target data to be transformed */
	if((tarsz = (ssize_t)size) <= 0)
		return tarsz;
	if(tarsz > 0 && !(tardt = (Vcchar_t*)data) )
		return -1;

	/* source data to diff against */
	disc = vcgetdisc(vc);
	srcsz = disc ? (ssize_t)disc->size : 0;
	if(srcsz > 0 && !(srcdt = disc ? (Vcchar_t*)disc->data : NIL(Vcchar_t*)) )
		return -1;

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), tarsz, 0)) )
		return -1;

	if(srcsz <= 0) /* no source data, just copy */
		memcpy(output, tardt, tarsz);
	else /* transform data */
	{	for(dt = output, t = 0; t < tarsz; )
		{	if((maxs = tarsz-t) > srcsz)
				maxs = srcsz;
			for(s = 0; s < maxs; ++s, ++t)
				*dt++ = tardt[t] - srcdt[s];
		}
	}

	dt = output;
	if(vcrecode(vc, &output, &tarsz, 0, 0) < 0 )
		return -1;
	if(dt != output)
		vcbuffer(vc, dt, -1, -1);

	if(out)
		*out = (Void_t*)output;
	return tarsz;
}


#if __STD_C
static ssize_t unhamming(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unhamming(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		srcsz, tarsz, s, t, maxs;
	Vcchar_t	*srcdt, *tardt, *output, *dt;
	Vcdisc_t	*disc;

	if(!vc)
		return -1;

	vc->undone = 0;

	/* retrieve transformed target data */
	if((tarsz = (ssize_t)size) <= 0)
		return tarsz;
	if(tarsz > 0 && !(tardt = (Vcchar_t*)data) )
		return -1;
	if(vcrecode(vc, &tardt, &tarsz, 0, 0) < 0 )
		return -1;

	/* source data to diff against */
	disc = vcgetdisc(vc);
	srcsz = disc ? (ssize_t)disc->size : 0;
	if(srcsz > 0 && !(srcdt = disc ? (Vcchar_t*)disc->data : NIL(Vcchar_t*)) )
		return -1;

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), tarsz, 0)) )
		return -1;

	if(srcsz <= 0)
		memcpy(output, tardt, tarsz);
	else
	{	for(dt = output, t = 0; t < tarsz; )
		{	if((maxs = tarsz-t) > srcsz)
				maxs = srcsz;
			for(s = 0; s < maxs; ++s, ++t)
				*dt++ = tardt[t] + srcdt[s];
		}
	}

	if(out)
		*out = (Void_t*)output;
	return tarsz;
}

Vcmethod_t _Vchamming =
{	hamming,
	unhamming,
	0,
	"hamming", "Byte-wise differencing (like Hamming distance).",
	"[-version?hamming (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	0,
	1024*1024,
	VC_MTSOURCE
};

VCLIB(Vchamming)
