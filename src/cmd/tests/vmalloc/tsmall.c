/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"vmtest.h"

tmain()
{
	Vmalloc_t*	vm;
	Void_t*		addr[10];

	Vmdcheap->round = 64;
	if(!(vm = vmopen(Vmdcheap, Vmbest, 0)) )
		terror("Open failed");

	if(!(addr[0] = vmalloc(vm, 8)) )
		terror("vmalloc failed");
	if(!(addr[1] = vmalloc(vm, 8)) )
		terror("vmalloc failed");
	if(!(addr[2] = vmalloc(vm, 8)) )
		terror("vmalloc failed");

	if(!(addr[3] = vmalloc(vm, 12)) )
		terror("vmalloc failed");
	if(!(addr[4] = vmalloc(vm, 12)) )
		terror("vmalloc failed");
	if(!(addr[5] = vmalloc(vm, 12)) )
		terror("vmalloc failed");

	if(vmfree(vm, addr[1]) < 0)
		terror("vmfree failed");
	if(vmdbcheck(vm) < 0)
		terror("vmdbcheck failed");

	if(vmfree(vm, addr[4]) < 0)
		terror("vmfree failed");

	if(vmdbcheck(vm) < 0)
		terror("vmdbcheck failed");

	texit(0);
}
