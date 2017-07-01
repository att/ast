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
	unsigned char	*addr[10], buf[1024];
	Vmalloc_t	*vm;
	int		n, k, pfd[2];
#define ALLOCSZ	29
#define BEFORE	-3
#define AFTER	7

	if(!(vm = vmopen(Vmdcheap,Vmdebug,VM_DBCHECK)) )
		terror("Can't open region handle");

	if(pipe(pfd) < 0)
		terror("Can't puff a pipe");
	vmdebug(pfd[1]);

	for(k = 0; k < 10; ++k)
	{	if(!(addr[k] = vmalloc(vm, ALLOCSZ)) )
			terror("Allocation of block[%d] failed", k);
		if((((Vmulong_t)addr[k])%ALIGN) != 0)
			terror("Unaligned addr");
	}

	/* error for freeing something non-existent */
	vmfree(vm, (Void_t*)1);
	if((n = read(pfd[0], buf, sizeof(buf))) <= 0)
		terror("No corruption messages written");
	buf[n] = 0;
	tinfo(buf);

	/* error for freeing something twice */
	if(vmfree(vm,addr[0]) < 0)
		terror("Failed freeing a block");
	vmfree(vm, addr[0]); 
	if((n = read(pfd[0], buf, sizeof(buf))) <= 0)
		terror("No corruption messages written");
	buf[n] = 0;
	tinfo(buf);

	/* resize, then corrupt a byte in front */
	if((addr[2] = vmresize(vm,addr[2],256,VM_RSMOVE|VM_RSCOPY)) == NIL(Void_t*))
		terror("Failed resizing");
	addr[2][BEFORE] = 0; /* corrupting a byte in front of addr[9] */
	if(vmfree(vm,addr[2]) < 0)
		terror("Failed freeing a block");
	if((n = read(pfd[0], buf, sizeof(buf))) <= 0)
		terror("No corruption messages written");
	buf[n] = 0;
	tinfo(buf);

	/* resize a non-existent block */
	if(vmresize(vm, (Void_t*)3, 256, VM_RSMOVE|VM_RSCOPY) != NIL(Void_t*) )
		terror("Resizing a nonexistent block succeeded");
	if((n = read(pfd[0], buf, sizeof(buf))) <= 0)
		terror("No corruption messages written");
	buf[n] = 0;
	tinfo(buf);

	/* resize a freed block */
	vmfree(vm, addr[3]);
	if((addr[3] = vmresize(vm,addr[3],256,VM_RSMOVE|VM_RSCOPY)) != NIL(Void_t*))
		terror("Resizing a free block succeeded");
	if((n = read(pfd[0], buf, sizeof(buf))) <= 0)
		terror("No corruption messages written");
	buf[n] = 0;
	tinfo(buf);

	/* corrupting a byte in back */
	if((addr[4] = vmresize(vm,addr[4],256,VM_RSMOVE|VM_RSCOPY)) == NIL(Void_t*))
		terror("Failed resizing");
	addr[4][256+AFTER] = 0;
	if(vmfree(vm,addr[4]) < 0)
		terror("Failed freeing a block");
	if((n = read(pfd[0], buf, sizeof(buf))) <= 0)
		terror("No corruption messages written");
	buf[n] = 0;
	tinfo(buf);

	texit(0);
}
