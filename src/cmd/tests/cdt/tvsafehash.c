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
#ifndef TMETHOD
#define TMETHOD		Dtset /* hash table with chaining */
#endif

#include	"dttest.h"

#include	<vmalloc.h>
#include	<sys/mman.h>

/* Test concurrent insert/delete/search by pingpong objects between
** two dictionaries, one built with mmap and the other shmget.
*/

#ifndef N_PROC
#define N_PROC		64
#endif
#if N_PROC < 2
#undef	N_PROC
#define N_PROC		2
#endif

#define N_CONCUR	((N_PROC/2)*2)	/* #players, must be even	*/
#define N_OBJ		20000	/* total number of objects	*/
#define MEMSIZE		(N_OBJ*2*sizeof(Obj_t) + sizeof(Void_t*)*1024*1024 )

#define	CDT_DATA	1	/* data section of dictionary	*/
typedef struct _cdtdata_s
{	Void_t*		data;
	int		process;
} Cdtdata_t;

/* a persistent object is a pair of string and decimal number */
typedef struct obj_s
{	Dtlink_t	link;	/* dictionary holder		*/
	int		dval;	/* decimal value		*/
	int		accn;	/* # of times accessed		*/
} Obj_t;

/* Cdt discipline to allocate memory from a vmalloc region */
typedef struct _mmdisc_s
{	Dtdisc_t	disc;	/* cdt discipline		*/
	char*		store;	/* backing store file name	*/
	Vmalloc_t*	vm;	/* shm vmalloc region		*/
} Mmdisc_t;

static char	*Mapstore;
static char	*Shmstore;
static Mmdisc_t	Mapdisc, Shmdisc;

/* allocate data from the shared memory region */
Void_t* mmmemory(Dt_t* dt, Void_t* data, size_t size, Dtdisc_t* disc)
{
	return vmresize(((Mmdisc_t*)disc)->vm, data, size, 0);
}

/* handle dictionary events */
static int mmevent(Dt_t* dt, int type, Void_t* data, Dtdisc_t* disc)
{
	Cdtdata_t	*cdtdt;
	Mmdisc_t	*mmdc = (Mmdisc_t*)disc;

	if(!(cdtdt = vmuserdata(mmdc->vm, CDT_DATA, sizeof(Cdtdata_t))) )
		terror("Can't get shared memory data");
	if(type == DT_OPEN)
	{	if(data) /* at the start of a dictionary opening */
		{	if(!asogetptr(&cdtdt->data)) /* data area not yet constructed */
				return 0;
			else /* got data area, just return it */
			{	*((Void_t**)data) = cdtdt->data;
				return 1;
			}
		}
		else	return 0;
	}
	else if(type == DT_ENDOPEN)  /* at the end of a dictionary opening */
	{	if(!asogetptr(&cdtdt->data)) /* save data area for future references */
		{	asocasptr(&cdtdt->data, NIL(Void_t*), (Void_t*)dt->data);
			return asogetptr(&cdtdt->data) == (Void_t*)dt->data ? 0 : -1;
		}
		else	return 0; /* data area existed */
	}
	else if(type == DT_CLOSE) /* starting to close dictionary */
		return 1; /* make sure no objects get deleted */
	else if(type == DT_ENDCLOSE) /* at end of closing, close the memory region */
	{	vmclose(mmdc->vm);
		mmdc->vm = NIL(Vmalloc_t*);
		return 0; /* all done */
	}
	else	return 0;
}

/* compare two objects by their integer keys */
static int mmcompare(Dt_t* dt, Void_t* key1, Void_t* key2, Dtdisc_t* disc)
{
	return *((int*)key1) - *((int*)key2);
}

/* open a shared dictionary based on a common backing store */
static Dt_t* opendictionary(int num, pid_t pid, char* store, int type )
{
	Vmalloc_t	*vm;
	Vmdisc_t	*vmdc;
	Dt_t		*dt;
	ssize_t		size;
	int		proj;
	Mmdisc_t	*mmdc;

	/* create/reopen the region backed by a file using mmap */
	proj = store == Mapstore ? -1 : 1;
	if(!(vmdc = vmdcshare(store, proj, MEMSIZE, type)) )
		terror("[store=%s]: Failed creating discipline for shared memory", store);
	if(!(vm = vmopen(vmdc, Vmbest, 0)) )
		terror("[store=%s]: Failed opening shared vmalloc region", store);

	/* discipline for objects identified by their decimal values */
	mmdc = store == Mapstore ? &Mapdisc : &Shmdisc;
	mmdc->disc.key  = (ssize_t)DTOFFSET(Obj_t,dval);
	mmdc->disc.size = (ssize_t)sizeof(int);
	mmdc->disc.link = (ssize_t)DTOFFSET(Obj_t,link);
	mmdc->disc.makef = (Dtmake_f)0;
	mmdc->disc.freef = (Dtfree_f)0;
	mmdc->disc.comparf = mmcompare;
	mmdc->disc.hashf = (Dthash_f)0;
	mmdc->disc.memoryf = mmmemory;
	mmdc->disc.eventf = mmevent;
	mmdc->store = store;
	mmdc->vm = vm;

	if(!(dt = dtopen(&mmdc->disc, TMETHOD)) )
		terror("Process[num=%d,pid=%d]: Can't open dictionary for %s", num, pid, store);
	dtcustomize(dt, DT_SHARE, 1); /* turn on concurrent access mode */

	return dt;
}

/* Creating a subprocess */
static pid_t makeprocess(char* proc, int num)
{
	int	i;
	pid_t	pid;
	char	text[16];
	char	*argv[8];

	if((pid = fork()) < 0 )
		terror("Process[num=%d]: Could not fork() a subprocess", num);
	else if(pid > 0 ) /* return to parent process */
		return pid;
	else
	{	sprintf(text, "%d", num);
		i = 0;
		argv[i++] = proc;
		argv[i++] = "--child";
		argv[i++] = Mapstore;
		argv[i++] = Shmstore;
		argv[i++] = text;
		argv[i] = 0;
		if(execv(proc, argv) < 0 )
			terror("Process[num=%d]: Could not execv() %s", num, proc);
	}
	return -1;
}

/* concurrent read/write hashed objects into a mmap-region */
static int pingpong(char* procnum)
{
	int		k, p, num, dir, n_move;
	Cdtdata_t	*cdtdt;
	Obj_t		obj, *o;
	Dt_t		*shmdt, *mapdt, *insdt, *deldt;
	Mmdisc_t	*mapdc, *shmdc;
	pid_t		pid;

	num = atoi(procnum);
	if((pid = getpid()) < 0 )
		terror("Process[num=%d]: can't get process id", num);

	/* open the shared dictionaries */
	if(!(mapdt = opendictionary(num, pid, Mapstore, 1)) )
		terror("Process[num=%d,pid=%d]: can't open dictionary for %s", num, pid, Mapstore);
	if(!(mapdc = (Mmdisc_t*)dtdisc(mapdt, NIL(Dtdisc_t*), 0)) )
		terror("Process[num=%d,pid=%d]: can't get dictionary discipline", num, pid);
	if(!(shmdt = opendictionary(num, pid, Shmstore, 1)) )
		terror("Process[num=%d,pid=%d]: can't open dictionary for %s", num, pid, Shmstore);
	if(!(shmdc = (Mmdisc_t*)dtdisc(shmdt, NIL(Dtdisc_t*), 0)) )
		terror("Process[num=%d,pid=%d]: can't get dictionary discipline", num, pid);

	/* wait for all to get going first */
	if(!(cdtdt = vmuserdata(mapdc->vm, CDT_DATA, sizeof(Cdtdata_t))) )
		terror("Process[num=%d,pid=%d]: can't get process count", num, pid);
	
	p = asoaddint(&cdtdt->process, 1);
	for(k = 0; p < N_CONCUR; asorelax(1<<k), k = (k+1)&07 ) /* wait until all inserters are running */
		p = asogetint(&cdtdt->process);
	tinfo("Process[num=%d,pid=%d]: ready to go", num, pid);

	/* delete from one and insert to the other */
	deldt = num%2 == 0 ? mapdt : shmdt;
	insdt = num%2 == 0 ? shmdt : mapdt;
	n_move = 0;
	for(dir = 1; dir >= -1; dir -= 2)
	{	for(k = dir > 0 ? 0 : N_OBJ-1; k >= 0 && k < N_OBJ; k += dir)
		{	obj.dval = k;
			if((o = dtsearch(deldt, &obj)) && (random()%2) == 0 && dtdelete(deldt, o) == o )
			{	dtinsert(insdt, o);
				n_move += 1;
			}
		}
	}
	tinfo("Process[num=%d,pid=%d]: move %d (%s -> %s)",
		num, pid, n_move, num%2 == 0 ? "map" : "shm", num%2 == 0 ? "shm" : "map");

	dtclose(mapdt);
	dtclose(shmdt);

	return 0;
}

tmain()
{
	pid_t		cpid[N_CONCUR], ppid, pid;
	size_t		k, p;
	Dt_t		*mapdt, *shmdt;
	Mmdisc_t	*mapdc, *shmdc;
	Obj_t		*os, *om, obj;

	if(k = tchild())
	{	Mapstore = argv[k++];
		Shmstore = argv[k++];
		return pingpong(argv[k]);
	}
	if((ppid = getpid()) < 0 )
		terror("Can't get process id");
	Mapstore = tstfile("map", -1);
	Shmstore = tstfile("shm", -1);
	(void)unlink(Mapstore); 
	(void)unlink(Shmstore); 

	tinfo("\tParent[pid=%d]: initializing shared dictionaries", ppid);
	if(!(mapdt = opendictionary(0, ppid, Mapstore, -1)) )
		terror("Parent[pid=%d]: Can't open dictionary for %s", ppid, Mapstore);
	if(!(mapdc = (Mmdisc_t*)dtdisc(mapdt, NIL(Dtdisc_t*), 0)) )
		terror("Parent[pid=%d]: Can't get discipline for %s", ppid, Mapstore);

	if(!(shmdt = opendictionary(0, ppid, Shmstore, -1)) )
		terror("Parent[pid=%d]: Can't open dictionary for %s", ppid, Shmstore);
	if(!(shmdc = (Mmdisc_t*)dtdisc(shmdt, NIL(Dtdisc_t*), 0)) )
		terror("Parent[pid=%d]: Can't get discipline for %s", ppid, Shmstore);

	for(k = 0; k < N_OBJ; ++k)
	{	if(random()%2 == 0)
		{	if(!(om = vmalloc(mapdc->vm, sizeof(Obj_t))) )
				terror("Parent[pid=%d]: vmalloc failed k=%d store=%s", ppid, k, Mapstore);
			om->dval = k;
			if(dtinsert(mapdt, om) != om)
				terror("Parent[pid=%d]: dtinsert failed k=%d store=%s", ppid, k, Mapstore);
		}
		else
		{	if(!(os = vmalloc(shmdc->vm, sizeof(Obj_t))) )
				terror("Parent[pid=%d]: vmalloc failed k=%d store=%s", ppid, k, Shmstore);
			os->dval = k;
			if(dtinsert(shmdt, os) != os)
				terror("Parent[pid=%d]: dtinsert failed k=%d store=%s", ppid, k, Shmstore);
		}
	}

	tinfo("\tParent[pid=%d]: creating pingpong subprocesses", ppid);
	for(p = 0; p < N_CONCUR; ++p )
		if((cpid[p] = makeprocess(argv[0], p)) < 0 )
			terror("Parent[pid=%d]: Could not make process %d", ppid, p);
	if (twait(cpid, N_CONCUR))
		terror("workload subprocess error");

	tinfo("\tParent[pid=%d]: check integrity", ppid);

	k = dtsize(mapdt);
	p = dtsize(shmdt);
	tinfo("Parent[pid=%d]: mapdt=%d shmdt=%d", ppid, k, p);
	if((k+p) != N_OBJ)
		terror("Parent[pid=%d]: expecting %d objects but (mapdt+shmdt)=%d", ppid, N_OBJ, k+p);

	for(k = 0; k < N_OBJ; ++k)
	{	Obj_t	obj, *om, *os;

		obj.dval = k;
		om = dtsearch(mapdt, &obj);
		os = dtsearch(shmdt, &obj);
		if(om && os)
			terror("Parent[pid=%d]: object %d is in both dictionaries", ppid, k);
		if(!om && !os)
			terror("Parent[pid=%d]: object %d is in neither dictionary", ppid, k);
	}
	
	/* clean up file/shmid data */
	dtclose(mapdt);
	dtclose(shmdt);

	texit(0);
}
