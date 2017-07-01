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

#define	MAPSIZE	(8*1024*1024)
#define ALLOCN	1024
#define ALLOCZ	(MAPSIZE/ALLOCN/2)

char	*shmfile;
char	*mapfile;

tmain()
{
	int		k;
	void		*addr;
	Vmdisc_t	*dc;
	Vmalloc_t	*shm, *map;
	char		*arg[5];
	pid_t		ppid, cpid;

	if(k = tchild())
	{	cpid = getpid();
		shmfile = argv[k];
		mapfile = argv[k+1];

		tinfo("Child[pid=%d]: allocating heap memory before opening shm region", cpid);
		for(k = 0; k < ALLOCN; ++k)
			if(!(addr = malloc(ALLOCZ)) )
				terror("Child[pid=%d]: Can't allocate segment %d", cpid, k);

		tinfo("Child[pid=%d]: opening shm region", cpid);
		if(!(dc = vmdcshare(shmfile, 1, MAPSIZE, 0)) ||
		   !(shm = vmopen(dc, Vmbest, 0)) )
			terror("Child[pid=%d]: Can't open shm region in child process", cpid);
		tinfo("Child[pid=%d]: shm region opened", cpid);

		tinfo("Child[pid=%d]: allocating heap memory before opening map region", cpid);
		for(k = 0; k < ALLOCN; ++k)
			if(!(addr = malloc(ALLOCZ)) )
				terror("Child[pid=%d]: Can't allocate segment %d", cpid, k);

		tinfo("Child[pid=%d]: opening map region", cpid);
		if(!(dc = vmdcshare(mapfile, -1, MAPSIZE, 0)) ||
		   !(map = vmopen(dc, Vmbest, 0)) )
			terror("Child[pid=%d]: Can't open map region in child process", cpid);
		tinfo("Child[pid=%d]: map region opened", cpid);
	}
	else
	{	ppid = getpid();
		shmfile = tstfile("shm", -1);
		mapfile = tstfile("map", -1);
		
		tinfo("Parent[pid=%d]: opening shm region", ppid);
		if(!(dc = vmdcshare(shmfile, 1, MAPSIZE, -1)) ||
		   !(shm = vmopen(dc, Vmbest, 0)) )
			terror("Parent[pid=%d]: Can't create shm region in parent process", ppid);
		tinfo("Parent[pid=%d]: shm region opened", ppid);

		tinfo("Parent[pid=%d]: opening map region", ppid);
		if(!(dc = vmdcshare(mapfile, -1, MAPSIZE, -1)) ||
		   !(map = vmopen(dc, Vmbest, 0)) )
			terror("Parent[pid=%d]: Can't create map region in parent process", ppid);
		tinfo("Parent[pid=%d]: map region opened", ppid);

		switch((cpid = fork()) ) /* make a child process */
		{ default :
			tinfo("Parent[pid=%d]: waiting for child[pid=%d]", ppid, cpid);
			wait(0);
			break;
		  case 0 :
			arg[0] = argv[0];
			arg[1] = "--child";
			arg[2] = shmfile;
			arg[3] = mapfile;
			arg[4] = 0;
			if(execv(arg[0], arg) < 0 )
				terror("Could not exec child process");
		  case -1:
			terror("Could not fork a child process");
		}
	}

	vmclose(shm);
	vmclose(map);

	texit(0);
}
