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
	Vmalloc_t	*vm;
	Void_t		*addr[10];
	Void_t		*mem;
	int		i;

	Vmdcheap->round = 64;
	if(!(vm = vmopen(Vmdcheap, Vmbest, 0)) )
		terror("Open failed");

	for(i = 0; i < 10; ++i)
		if(!(addr[i] = vmalloc(vm,15)) )
			terror("vmalloc failed");
	for(i = 0; i < 10; ++i)
		if(vmfree(vm,addr[i]) < 0)
			terror("vmfree failed");
	for(i = 0; i < 10; ++i)
		if(!(addr[i] = vmalloc(vm,15)) )
			terror("vmalloc failed");

	mem = 0;
	if(posix_memalign(&mem, 3, 128) != EINVAL)
		terror("Bad return value from posix_memalign()");
	if(mem)
		terror("Bad memory");

	if(posix_memalign(&mem, 3*sizeof(Void_t*), 128) != EINVAL)
		terror("Bad return value from posix_memalign()");
	if(mem)
		terror("Bad memory");

	if(posix_memalign(&mem, (sizeof(Void_t*)<<4), 128) != 0 )
		terror("posix_memalign() failed");
	if(!mem)
		terror("Bad memory");

	texit(0);
}
