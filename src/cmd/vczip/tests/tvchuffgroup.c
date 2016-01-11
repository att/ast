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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"vctest.h"

MAIN()
{
	Vcodex_t	*vch, *vcu;
	ssize_t		k, n;
	int		c;
	char		data[64*1024], *d, *endd;

	/* generate random data */
	c = 'a';
	for(endd = (d = data)+sizeof(data); d < endd; )
	{	
		for(k = 0; k < 64 && d < endd; ++k, ++d)
			*d = c + (random()%2);
		if((c += 2) > 'h')
			c = 'a';
	}

	if(!(vch = vcopen(0, Vchuffgroup, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vchuffgroup handle");
	if(!(vcu = vcopen(0, Vchuffgroup, 0, 0, VC_DECODE)) )
		terror("Cannot open Vchuffgroup handle to decode");
	if((k = vcapply(vch, data, sizeof(data), &d)) <= 0)
		terror("Can't compress");
	twarn("Vchuffgroup: original size=%d compressed size=%d\n",sizeof(data),k);
	if((n = vcapply(vcu, d, k, &d)) != sizeof(data))
		terror("Can't decompress n=%d", n);
	for(k = 0; k < sizeof(data); ++k)
		if(d[k] != data[k])
			terror("Bad decompressed data");

	if(!(vch = vcopen(0, Vchuffpart, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vchuffpart handle");
	if(!(vcu = vcopen(0, Vchuffpart, 0, 0, VC_DECODE)) )
		terror("Cannot open Vchuffpart handle to decode");
	if((k = vcapply(vch, data, sizeof(data), &d)) <= 0)
		terror("Can't compress");
	twarn("Vchuffpart: original size=%d compressed size=%d\n",sizeof(data),k);
	if((n = vcapply(vcu, d, k, &d)) != sizeof(data))
		terror("Can't decompress n=%d", n);
	for(k = 0; k < sizeof(data); ++k)
		if(d[k] != data[k])
			terror("Bad decompressed data");

	exit(0);
}
