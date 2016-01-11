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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"vctest.h"

int main()
{
	char		*tar, *del, *t;
	Vcodex_t	*vc;
	ssize_t		k, n;

	tar = "000011111a0b1c2000";

	if(!(vc = vcopen(0, Vcrle, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vcrle handle");

	if((n = vcapply(vc, tar, strlen(tar), &del)) <= 0)
		terror("Vcrle failed");

	if(!(vc = vcopen(0, Vcrle, 0, 0, VC_DECODE)) )
		terror("Cannot open Vcunrle handle");
	if((n = vcapply(vc, del, n, &t)) != strlen(tar) )
		terror("Vcunrle returns wrong size");
	for(k = 0; k < n; ++k)
		if(t[k] != tar[k])
			terror("Vcunrle computed bad byte");

	if(!(vc = vcopen(0, Vcrle, "0", 0, VC_ENCODE)) )
		terror("Cannot open Vcrle0 handle");

	if((n = vcapply(vc, tar, strlen(tar), &del)) <= 0)
		terror("Vcrle failed2");

	if(!(vc = vcopen(0, Vcrle, "0", 0, VC_DECODE)) )
		terror("Cannot open Vcunrle0 handle2");
	if((n = vcapply(vc, del, n, &t)) != strlen(tar) )
		terror("Vcunrle0 returns wrong size2");
	for(k = 0; k < n; ++k)
		if(t[k] != tar[k])
			terror("Vcunrle0 computed bad byte2");

	exit(0);
}
