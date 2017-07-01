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
#include	<ast.h> /* put our malloc() in scope! */

#include	"vmtest.h"

#define TMESSAGE() \
	if(lseek(fd, 0, SEEK_CUR) <= pos || lseek(fd, pos, SEEK_SET) != pos || (n = read(fd, buf, sizeof(buf)-1)) <= 0) \
		terror("No debug messages written"); \
	else \
	{	pos += n; \
		buf[n] = 0; \
		tinfo("debug: %s", buf); \
	}

#define ALLOCSZ	29
#define BEFORE	-3
#define AFTER	7

tmain()
{
	unsigned char	*addr[10], buf[1024];
	char		*err;
	int		n, k, fd;
	off_t		pos = 0, cur;

	if(!tchild())
	{	setenv("VMALLOC_OPTIONS", "method=debug,period=1,start=1", 1);
		execl(argv[0], argv[0], "--child", (char*)0);
	}

	err = tstfile("err", 0);
	if((fd = open(err, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR)) < 0)
		terror("Can't create %s", err);
	vmdebug(fd);

	for(k = 0; k < 10; ++k)
	{	if(!(addr[k] = malloc(ALLOCSZ)) )
			terror("Allocation of block[%d] failed", k);
		if((((Vmulong_t)addr[k])%ALIGN) != 0)
			terror("Unaligned addr");
	}

	/* error for freeing something non-existent */
	free((Void_t*)1);
	TMESSAGE();

	/* error for freeing something twice */
	free(addr[0]);
	free(addr[0]); 
	TMESSAGE();

	/* resize, then corrupt a byte in front */
	if((addr[2] = realloc(addr[2],256)) == NIL(Void_t*))
		terror("Failed resizing");
	addr[2][BEFORE] = 0; /* corrupting a byte in front of addr[9] */
	free(addr[2]);
	TMESSAGE();

	/* resize a non-existent block */
	if(realloc((Void_t*)3, 256) != NIL(Void_t*) )
		terror("Resizing a nonexistent block succeeded");
	TMESSAGE();

	/* resize a freed block */
	free(addr[3]);
	if((addr[3] = realloc(addr[3],256)) != NIL(Void_t*))
		terror("Resizing a free block succeeded");
	TMESSAGE();

	/* corrupting a byte in back */
	if((addr[4] = realloc(addr[4],256)) == NIL(Void_t*))
		terror("Failed resizing");
	addr[4][256+AFTER] = 0;
	free(addr[4]);
	TMESSAGE();

	texit(0);
}
