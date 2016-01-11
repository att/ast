/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"vmtest.h"

#define	MAPSIZE	(128*1024*1024)

char	*shmfile;
char	*mapfile;

tmain()
{
	int		k, code = 0;
	Vmalloc_t	*shm, *map;
	char		*arg[5];
	pid_t		ppid, cpid;

	if(k = tchild())
	{	cpid = getpid();
		shmfile = argv[k];
		mapfile = argv[k+1];

		tinfo("Child[pid=%d]: allocating heap memory before opening shm region", cpid);
		for(k = 0; k < 1024; ++k)
			if(!malloc(32*1024) )
				terror("Child[pid=%d]: Can't allocate segment %d", cpid, k);

		if(*shmfile)
		{	tinfo("Child[pid=%d]: opening shm region", cpid);
			if(!(shm = vmmopen(shmfile, 1, MAPSIZE)) )
				terror("Child[pid=%d]: Can't open shm region in child process", cpid);
			tinfo("Child[pid=%d]: shm region opened", cpid);
		}

		tinfo("Child[pid=%d]: allocating heap memory before opening map region", cpid);
		for(k = 0; k < 1024; ++k)
			if(!malloc(32*1024) )
				terror("Child[pid=%d]: Can't allocate segment %d", cpid, k);

		if(*mapfile)
		{	tinfo("Child[pid=%d]: opening map region", cpid);
			if(!(map = vmmopen(mapfile, -1, MAPSIZE)) )
				terror("Child[pid=%d]: Can't open map region in child process", cpid);
			tinfo("Child[pid=%d]: map region opened", cpid);
		}
	}
	else
	{	ppid = getpid();
		shmfile = tstfile("shm", -1);
		mapfile = tstfile("map", -1);
		(void)unlink(shmfile);
		(void)unlink(mapfile);
		
		tinfo("Parent[pid=%d]: %s: opening shm region", ppid, shmfile);
		if(shm = vmmopen(shmfile, 1, MAPSIZE) )
			tinfo("Parent[pid=%d]: %s: shm region opened", ppid, shmfile);
		else
		{	tnote("shm not supported");
			shmfile = "";
		}

		tinfo("Parent[pid=%d]: %s: opening map region", ppid, mapfile);
		if(map = vmmopen(mapfile, -1, MAPSIZE) )
			tinfo("Parent[pid=%d]: %s: map region opened", ppid, mapfile);
		else
		{	tnote("map not supported");
			mapfile = "";
		}

		switch((cpid = fork()) ) /* make a child process */
		{ default :
			code = twait(&cpid, 1);
			break;
		  case 0 :
			arg[0] = argv[0];
			arg[1] = "--child";
			arg[2] = shmfile;
			arg[3] = mapfile;
			arg[4] = 0;
			if(execv(argv[0], arg) < 0 )
				terror("Could not exec child process");
		  case -1:
			terror("Could not fork a child process");
		}
	}

	vmmrelease(shm, 1); vmclose(shm);
	vmmrelease(map, 1); vmclose(map);

	texit(code);
}
