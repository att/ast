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
#include	"dttest.h"

#include	<sys/mman.h>

/* Test concurrency usage of the method Dtrhset.
**
** Written by Kiem-Phong Vo
*/

#ifndef N_PROC
#define N_PROC		64
#endif

#define	N_OBJ		(N_PROC*64*1024) /* #objects to insert	*/
#define SEARCH		4	/* #searches while inserting	*/
#define PROGRESS	(N_OBJ/(N_PROC*4)) /* amount done	*/

#define FORMAT	"%09d"	/* 9-digit numbers with zero-filled	*/

#define INSERT	001	/* to tell if an object was inserted	*/
#define DELETE	002	/* to tell if an object was deleted	*/

typedef struct _obj_s
{	Dtlink_t	link;
	unsigned int	flag;	/* INSERT/DELETE state		*/
	char		str[12]; /* string representation	*/
} Obj_t;

typedef struct _proc_s
{	Obj_t*		obj;	/* list of objects to add	*/
	ssize_t		objn;	/* number of objects in list	*/
} Proc_t;

typedef struct _state_s
{	unsigned int	insert;	/* insertion states		*/
	unsigned int	idone;
	unsigned int	delete;	/* deletion states		*/
	unsigned int	ddone;
} State_t;

typedef struct _disc_s
{	Dtdisc_t	disc;
	unsigned int	lock;
	unsigned char*	addr;
	ssize_t		size;
} Disc_t;

static Disc_t		*Disc;	/* shared discipline structure 	*/
static Obj_t		*Obj;	/* shared object list		*/
static Proc_t		Proc[N_PROC]; /* process workloads	*/
static int		Pnum = N_PROC+1; /* start as parent	*/

static int		Icount;	/* # insertions done		*/
static int		Dcount;	/* # deletions done		*/

static State_t		*State;	/* insert/delete states		*/

/* memory allocator for shared dictionary - no freeing here */
static Void_t* memory(Dt_t* dt, Void_t* addr, size_t size, Dtdisc_t* disc)
{
	int	k;
	Disc_t	*dc = (Disc_t*)disc;

	if(addr || size <= 0 ) /* no freeing */
		return NIL(Void_t*);

	for(k = 0;; asorelax(1<<k), k = (k+1)&07 )
		if(asocasint(&dc->lock, 0, 1) == 0) /* get exclusive use first */
			break;

	size = ((size + sizeof(Void_t*) - 1)/sizeof(Void_t*))*sizeof(Void_t*);
	if(size <= dc->size)
	{	addr = (Void_t*)dc->addr;
		dc->addr += size;
		dc->size -= size;
	}
	else	terror("Out of shared memory");

	asocasint(&dc->lock, 1, 0); /* release exclusive use */

	return addr;
}

static void sigchild(int sig)
{	pid_t	pid;
	int	status;
	char	*st, buf[128];

	pid = wait(&status);
	if(WIFSIGNALED(status))
	{	int	sig = WTERMSIG(status);
		sprintf(buf,"signal %s", sig == 8 ? "fpe" : sig == 11 ? "segv" : "whatever");
		st = buf;
	}
	else if(WCOREDUMP(status))
		st = "coredump";
	else	st = "normal";

	tinfo("Child process %d exited (%s)", pid, st);
	signal(SIGCHLD, sigchild);
}

static void workload(Dt_t* dt, Proc_t* proc, int p)
{
	Obj_t	*os, *or;
	ssize_t	k, s;
	pid_t	pid = getpid();

	Pnum = p+1; /* always positive */

	/* insert objects in 'p' */
	asoincint(&State->insert); /* signaling that we are ready to go */
	while(asogetint(&State->insert) != N_PROC) /* wait until all processes are set */
		usleep(100);
	for(k = 0; k < proc->objn; ++k)
	{	if(k && k%PROGRESS == 0)
			tinfo("\tProcess %d(%d): insertion passing %d", p, pid, k);

		or = proc->obj+k;
		if((os = dtinsert(dt,or)) != or)
			tinfo("\t\tProcess %d(%d): Insert %s, get %0x", p,pid,or->str,os);
		else	os->flag |= INSERT;
		if((os = dtsearch(dt, or)) != or)
			tinfo("\t\tProcess %d(%d): Just inserted %s but not found", p,pid,or->str);
		Icount += 1;

		if(k > SEARCH ) /* search a few elements known to be inserted */
		{	for(s = 0; s < SEARCH; ++s)
			{	ssize_t r = random()%k;
				or = proc->obj+r;
				os = dtsearch(dt,or);
				if(os != or)
					tinfo("\t\tProcess %d(%d): Srch %s(Max %s) get %0x",
						p,pid, or->str, proc->obj[k].str, os);
			}
		}
	}
	tinfo("Process %d(%d): insertion done", p,pid);
	asoincint(&State->idone); /* signaling that this workload has been inserted */
	while(asogetint(&State->idone) > 0) /* wait until parent signal ok to continue */
		usleep(100);

	/* delete objects in 'p' and also in "foe" of p */
	asoincint(&State->delete); /* signaling that we are ready to delete */
	while(asogetint(&State->delete) != N_PROC) /* wait until all processes are set */
		usleep(100);
	for(k = 0; k < proc->objn; ++k)
	{	if(k && k%PROGRESS == 0)
			tinfo("\tProcess %d(%d): deletion passing %d", p, pid, k);

		if(dtdelete(dt, proc->obj+k) == Proc[p].obj+k )
			proc->obj[k].flag |= DELETE;
		Dcount += 1;

		if(k > SEARCH)
		{	for(s = 0; s < SEARCH; ++s)
			{	ssize_t r = random()%k;
				or = proc->obj+r;
				if(dtsearch(dt,or) )
					twarn("\t\tProcess %d(%d): Search %s !NULL",p,pid,or->str);
			}
		}
	}
	asoincint(&State->ddone); /* signaling that workload has been deleted */
	tinfo("Process %d(%d): deletion done", p, pid);
}

tmain()
{
	ssize_t		k, z, objn;
	Obj_t		*o;
	Dt_t		*dt;
	pid_t		pid[N_PROC];
	int		zerof;

	tchild();

	if((zerof = open("/dev/zero", O_RDWR)) < 0)
		terror("Can't open /dev/zero");

	/* get shared memory */
	if((k = 4*N_OBJ*sizeof(Void_t*)) <  64*1024*1024 )
		k = 64*1024*1024;
	z = sizeof(State_t) /* insert/delete states */ +
	    sizeof(Disc_t) /* discipline */ +
	    N_OBJ*sizeof(Obj_t) /*  Obj  */ +
	    k; /* table memory */
	State = (State_t*)mmap(0,z,PROT_READ|PROT_WRITE,MAP_SHARED,zerof,0);
	if(!State || State == (State_t*)(-1))
		terror("mmap failed");
	Disc = (Disc_t*)(State+1);
	Obj  = (Obj_t*)(Disc+1);
	Disc->addr = (unsigned char*)(Obj+N_OBJ);
	Disc->size = k;

	memset(State, 0, sizeof(State_t));

	/* construct the objects to be inserted */
	for(k = 0; k < N_OBJ; ++k)
	{	Obj[k].flag = 0;
		sprintf(Obj[k].str, FORMAT, k);
	}

	/* construct the work-load for each process */
	objn = N_OBJ/N_PROC;
	Proc[0].obj  = &Obj[0];
	Proc[0].objn = objn;
	for(k = 1; k < N_PROC; ++k)
	{	Proc[k].obj  = Proc[k-1].obj + Proc[k-1].objn;
		Proc[k].objn = k < (N_PROC-1) ? objn : N_OBJ - (k*objn);
	}

	/* now create the shared dictionary */
	Disc->disc.key     = DTOFFSET(Obj_t,str);
	Disc->disc.size    = 0;
	Disc->disc.link    = 0;
	Disc->disc.makef   = NIL(Dtmake_f);
	Disc->disc.freef   = NIL(Dtfree_f);
	Disc->disc.comparf = NIL(Dtcompar_f);
	Disc->disc.hashf   = NIL(Dthash_f);
	Disc->disc.memoryf = memory;
	Disc->disc.eventf  = NIL(Dtevent_f);
	if(!(dt = dtopen(&Disc->disc, Dtrhset)) )
		terror("Cannot open dictionary");
	if(dtcustomize(dt, DT_SHARE, 1) != DT_SHARE )
		terror("Cannot turn on share mode");

#if N_PROC <= 1
	Pnum = 1;
	workload(dt, Proc, 0);
#else
	signal(SIGCHLD, sigchild);
	for(k = 0; k < N_PROC; ++k)
	{	if((pid[k] = fork()) < 0 )
			terror("Can't create child process");
		else if(pid[k] > 0) /* parent */
			tinfo("Just launched process %d (pid=%d)", k, pid[k]);
		else 
		{	Pnum = k+1;
			signal(SIGCHLD,SIG_IGN);
			workload(dt, Proc+k, k);
			texit(0);
		}
	}
#endif

	tinfo("\ttrehash: Insertion #procs=%d (free shared mem=%d)", k, Disc->size);
	for(k = 0;; asorelax(1<<k), k = (k+1)&07)
		if(asogetint(&State->idone) == N_PROC) 
			break;
	tinfo("\ttrehash: Insertion completed, checking integrity");

	for(k = 0; k < N_OBJ; ++k)
		if(!dtsearch(dt, Obj+k) )
			terror("Failed to find object %s", Obj[k].str);

	asocasint(&State->idone, N_PROC, 0);

	tinfo("\ttrehash: Deletion (free shared mem=%d)", Disc->size);
	for(k = 0;; asorelax(1<<k), k = (k+1)&07 )
		if(asogetint(&State->ddone) == N_PROC) /* wait until all are deleted */
			break;
	if(dtfirst(dt) )
		terror("Dictionary not empty after deletion!");

	z = 0;
	for(k = 0; k < N_OBJ; ++k)
		if((Obj[k].flag & DELETE) )
			z += 1;
	if(z != N_OBJ)
		twarn("Some deletion was not properly recorded?");

	tinfo("\ttrehash: All testing done.");
	twait(pid, -N_PROC);

	texit(0);
}
