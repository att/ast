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
#include	"dttest.h"

#include	<vmalloc.h>
#include	<sys/mman.h>
#include	<sys/time.h>

/* Test insert/delete/search in shared/persistent memory region.  */

#ifndef N_PROC
#define N_PROC		48
#endif
#if N_PROC < 8
#undef	N_PROC
#define N_PROC		8
#endif

#define N_INSERT	(N_PROC/3)	/* #concurrent writers		*/
#define N_DELETE	(N_PROC/3)	/* #concurrent deleters		*/
#define N_SEARCH	(N_PROC/3)	/* #concurrent searchers	*/
#define N_WRITER	(N_INSERT+N_DELETE)
#define N_PROCESS	(N_INSERT+N_DELETE+N_SEARCH)
#define W_EXTENT	(2604*N_PROCESS)/* size of a dataset to write	*/
#define COLLISION	(N_INSERT/2)	/* should be < N_INSERT		*/

#define DTMETHOD	Dtrhset	/* storage method to use	*/
#define LONGGONE	(-1000000) /* setting refn to gone	*/

static int	Count[N_INSERT*W_EXTENT];

static char	*Mapstore, *Shmstore;

#define MEMSIZE	(4*N_INSERT*W_EXTENT*(sizeof(Obj_t)+16) + 64*1024*1024)

#define	CDT_DATA	1	/* data section of dictionary	*/
#define CDT_WRITER	2	/* count of started inserters	*/
#define CDT_SEARCHER	3	/* count of started searchers	*/
#define CDT_FINISHED	4	/* count of done process	*/
#define CDT_INSERT	5	/* #inserts across processes	*/
#define CDT_DELETE	6	/* #inserts across processes	*/
#define CDT_INCOMPLETE	7	/* #incomplete objects 		*/

/* a persistent object is a pair of string and decimal number */
typedef struct obj_s
{	Dtlink_t	link;	/* dictionary holder		*/
	int		dval;	/* decimal value		*/
	char*		sval;	/* corrresponding string value	*/
	int		ready;	/* object ready to be used	*/
	int		refn;	/* usage reference count	*/

	/* below are extra data good to help debugging */
	int		type;	/* last announced op		*/
	int		opid;	/* pid of that op		*/
	int		ins;	/* #seen by an insert process	*/
	int		del;	/* #seen by a delete process	*/
	int		srch;	/* #seen by a search process	*/
	int		free;	/* #of times being freed	*/
	int		fpid;	/* process that freed it	*/
	struct obj_s*	next;	/* link for free list		*/
} Obj_t;

/* Cdt discipline to allocate memory from a vmalloc region */
typedef struct _mmdisc_s
{	Dtdisc_t	disc;	/* cdt discipline		*/
	Vmalloc_t*	vm;	/* vmalloc region		*/
	ssize_t		ndel;	/* delete count			*/
	Obj_t*		list;	/* free list			*/
	int		pid;
} Mmdisc_t;

/* allocate data from the shared memory region */
Void_t* mmmemory(Dt_t* dt, Void_t* data, size_t size, Dtdisc_t* disc)
{
	return vmresize(((Mmdisc_t*)disc)->vm, data, size, 0);
}

/* handle dictionary events */
static int mmevent(Dt_t* dt, int type, Void_t* data, Dtdisc_t* disc)
{
	Void_t		*ctrl;
	Mmdisc_t	*mmdc = (Mmdisc_t*)disc;

	if(type == DT_OPEN)
	{	ctrl = vmmvalue(mmdc->vm, CDT_DATA, (Void_t*)0, VM_MMGET);
		if(data) /* at the start of a dictionary opening */
		{	if(!ctrl) /* data area not yet constructed */
				return 0;
			else /* got data area, just return it */
			{	*((Void_t**)data) = ctrl;
				return 1;
			}
		}
		else	return 0;
	}
	else if(type == DT_ENDOPEN)  /* at the end of a dictionary opening */
	{	ctrl = vmmvalue(mmdc->vm, CDT_DATA, (Void_t*)0, VM_MMGET);
		if(!ctrl) /* data area just constructed, record it */
		{	ctrl = vmmvalue(mmdc->vm, CDT_DATA, (Void_t*)dt->data, VM_MMSET);
			return ctrl == (Void_t*)dt->data ? 0 : -1;
		}
		else	return 0; /* data area existed */
	}
	else if(type == DT_CLOSE)
		return 1; /* make sure no objects get deleted */
	else if(type == DT_ENDCLOSE) /* at end of closing, close the memory region */
	{	vmmrelease(mmdc->vm, 0);
		vmclose(mmdc->vm);
		mmdc->vm = NIL(Vmalloc_t*);
		return 0; /* all done */
	}
	else if(type & DT_ANNOUNCE)
	{	Obj_t	*obj = (Obj_t*)data;

		if(obj->refn < 0 ) /* must be the last announcement from dtdelete() */
		{	if(obj->refn != LONGGONE)
				terror("Process %d: refn != LONGGONE op=%d obj[%d,refn=%d,fpid=%d]",
					mmdc->pid, type&~DT_ANNOUNCE, obj->dval, obj->refn, obj->fpid);
			if(!(type&DT_DELETE) )
				tpause("Process %d: Op=%d != DT_DELETE obj[%d,refn=%d,fpid=%d]",
					mmdc->pid, type&~DT_ANNOUNCE, obj->dval, obj->refn, obj->fpid);
		}

		obj->type = type & ~DT_ANNOUNCE; /* record the announcement */
		obj->opid = mmdc->pid;

		if(!(type & DT_DELETE) ) /* test this because obj may be gone */
		{	/* Wait-loop for object completion. The below "if"
			** statement causes the loop to be executed in all
			** cases except for the dtinsert()/dtattach() that
			** will complete constructing the object.
			*/
			if(type & ~(DT_ANNOUNCE|DT_INSERT|DT_ATTACH) )
				while(obj->ready == 0 )
					asorelax(1);

			/* increase reference count */
			if(asoincint(&obj->refn) < 0 )
				tpause("Process %d: refn<0 on adding op=%d obj[%d,refn=%d,fpid=%d]",
					mmdc->pid, type&~DT_ANNOUNCE, obj->dval, obj->refn, obj->fpid);
		}

		return 0;
	}
	else	return 0;
}

/* compare two objects by their integer keys */
static int mmcompare(Dt_t* dt, Void_t* key1, Void_t* key2, Dtdisc_t* disc)
{
	return *((int*)key1) - *((int*)key2);
}

/* free a deleted object */
static void mmfree(Dt_t* dt, Void_t* objarg, Dtdisc_t* disc)
{
	int		refn;
	Obj_t		*obj = (Obj_t*)objarg;
	Mmdisc_t	*mmdc = (Mmdisc_t*)dt->disc;

	if(obj->free > 0 ) /* already freed, not good! */
		terror("Process %d: multiple deletion? obj[%d,sval=%s,free=%d,pid=%d,refn=%d]",
			mmdc->pid, obj->dval, obj->sval, obj->free, obj->fpid, obj->refn );

	while(!obj->ready) /* object must be complete before deletable */
		asorelax(1);

	obj->fpid = mmdc->pid; /* process doing deletion */
	obj->free += 1; /* set indicator that it's free */

	while(obj->refn > 0) /* wait until no further reference to it */
		asorelax(1);

	/* set reference number to LONGGONE */
	if((refn = (int)asocasint(&obj->refn, 0, (uint)LONGGONE)) != 0 )
		terror("Process %d: refn=%d > 0? obj[%d,sval=%s,free=%d,pid=%d,refn=%d,op=%d]",
			mmdc->pid, refn, obj->dval, obj->sval, obj->free, obj->fpid, obj->refn, obj->type );

	obj->dval = -obj->dval; /* deleted objects have negative id */
	obj->next = mmdc->list; /* add to free list, this could be a garbage-collector */
	mmdc->list = obj;
	mmdc->ndel += 1;
}

/* open a shared dictionary based on a common backing store */
static Dt_t* opendictionary(char* actor, char* type, int num, pid_t pid, char* store)
{
	Vmalloc_t	*vm;
	Dt_t		*dt;
	ssize_t		size;
	static Mmdisc_t	Mmdc;	/* CDT discipline for shared dictionary	*/

	/* create/reopen the region backed by a file using mmap */
	if(!(vm = vmmopen(store, store == Mapstore ? -1 : 1, MEMSIZE)) )
		terror("%s %s [num=%d,pid=%d]: Couldn't create vmalloc region", actor, type, num, pid);

	/* discipline for objects identified by their decimal values */
	Mmdc.disc.key  = (ssize_t)DTOFFSET(Obj_t,dval);
	Mmdc.disc.size = (ssize_t)sizeof(int);
	Mmdc.disc.link = (ssize_t)DTOFFSET(Obj_t,link);
	Mmdc.disc.makef = (Dtmake_f)0;
	Mmdc.disc.freef = mmfree;
	Mmdc.disc.comparf = mmcompare;
	Mmdc.disc.hashf = (Dthash_f)0;
	Mmdc.disc.memoryf = mmmemory;
	Mmdc.disc.eventf = mmevent;
	Mmdc.vm = vm;
	Mmdc.ndel = 0;
	Mmdc.list = (Obj_t*)0;
	Mmdc.pid = (int)getpid();

	if(!(dt = dtopen(&Mmdc.disc, DTMETHOD)) ) /* open dictionary with hash-trie */
		terror("%s %s [num=%d,pid=%d]: Can't open dictionary", actor, type, num, pid);
	if(dtcustomize(dt, (DT_ANNOUNCE|DT_SHARE), 1) != (DT_ANNOUNCE|DT_SHARE) )
		terror("%s %s [num=%d,pid=%d]: Can't customize dictionary", actor, type, num, pid);

	return dt;
}

/* Creating a subprocess to write/read a dictionary */
static pid_t makeprocess(char* proc, char* type, char* actor, int num, char* aso)
{
	int	i;
	pid_t	pid;
	char	text[16];
	char	*argv[9];

	if((pid = fork()) < 0 )
		terror("%s %s [num=%d]: Could not fork() a subprocess", actor, type, num);
	else if(pid > 0 ) /* return to parent process */
		return pid;
	else
	{	sprintf(text, "%d", num);
		i = 0;
		argv[i++] = proc;
		if (aso)
			argv[i++] = aso;
		argv[i++] = "--child";
		argv[i++] = Mapstore;
		argv[i++] = Shmstore;
		argv[i++] = type;
		argv[i++] = actor;
		argv[i++] = text;
		argv[i++] = 0;
		if(execv(proc, argv) < 0 )
			terror("%s %s [num=%d]: Could not execv() process %s %s", actor, type, num, proc, type);
	}
	return -1;
}

/* concurrent read/write hashed objects into a mmap-region */
static int readwrite(char* type, char* store, char* actor, char* procnum)
{
	int		i, k, p, num, base, insert, delete, search, unsrch, undel, unins, walk, first, size;
	char		*sval;
	Obj_t		obj, *o, *next, *rv;
	Dt_t		*dt;
	pid_t		pid;
	Mmdisc_t	*mmdc;

	num = atoi(procnum);
	if((pid = getpid()) < 0 )
		terror("%s %s [num=%d]: can't get process id", actor, type, num);

	/* open the shared/persistent dictionary */
	if(!(dt = opendictionary(actor, type, num, pid, store)) )
		terror("%s %s [num=%d,pid=%d]: can't open dictionary", actor, type, num, pid);

	/* get the discipline structure with the Vmalloc region */
	if(!(mmdc = (Mmdisc_t*)dtdisc(dt, NIL(Dtdisc_t*), 0)) )
		terror("%s %s [num=%d,pid=%d]: can't get dictionary discipline", actor, type, num, pid);

	/* all writers wait until they are all ready before going */
	if(strcmp(actor, "inserter") == 0 || strcmp(actor, "deleter") == 0)
	{	p = (int)((long)vmmvalue(mmdc->vm, CDT_WRITER, (Void_t*)1, VM_MMADD)); 
		for(; p < N_WRITER; asorelax(1) ) /* wait until all writers are running */
			p = (int)((long)vmmvalue(mmdc->vm, CDT_WRITER, (Void_t*)0, VM_MMGET));
	}
	else /* searchers wait for everyone to be ready before going */
	{	p = (int)((long)vmmvalue(mmdc->vm, CDT_SEARCHER, (Void_t*)1, VM_MMADD)); 
		k = (int)((long)vmmvalue(mmdc->vm, CDT_WRITER, (Void_t*)0, VM_MMGET)); 
		for(; p+k < N_PROCESS; asorelax(1) ) /* wait until all are running */
		{	p = (int)((long)vmmvalue(mmdc->vm, CDT_SEARCHER, (Void_t*)0, VM_MMGET));
			k = (int)((long)vmmvalue(mmdc->vm, CDT_WRITER, (Void_t*)0, VM_MMGET));
		}
	}

#define BASE(n)	((n)*W_EXTENT + 1) /* base number of objects to be inserted */
	walk = first = insert = delete = search = unsrch = unins = undel = 0;
	if(strcmp(actor, "inserter") == 0 )
	{	base = BASE((num/COLLISION)*COLLISION); /* make these inserters do the same thing */
		tinfo("%s %s [num=%d,pid=%d]: range=[%d,%d) ready to go", actor, type, num, pid, base, base+W_EXTENT);

		for(i = 0; i < W_EXTENT; ++i)
		{	/* insert a new object */
			if(!(o = (Obj_t*)vmalloc(mmdc->vm, sizeof(Obj_t))) )
				terror("%s %s [num=%d,pid=%d]: vmalloc failed", actor, type, num, pid);

			memset(o, 0, sizeof(Obj_t));
			o->dval = i+base;
			if(!(rv = dtinsert(dt, o)) ) /* failed insert */
				terror("%s %s [num=%d,pid=%d]: insert failed", actor, type, num, pid);

			if(rv == o) /* successfully inserted, complete it by making string value */
			{	insert += 1;

				if(!(sval = vmalloc(mmdc->vm, 16)) ) /* construct string value */
					terror("%s %s [num=%d,pid=%d]: vmalloc failed", actor, type, num, pid);
				sprintf(sval, "%d", o->dval);
				o->sval = sval;

				if(o->dval < 0) /* this ain't no way! */
					terror("%s %s [num=%d,pid=%d]: already freed? obj[dval=%d,pid=%d]",
						actor, type, num, pid, o->dval, o->fpid);

				if(o->refn != 1 ) /* everyone else should still be spinning */
					terror("%s %s [num=%d,pid=%d]: refn != 1? obj[dval=%d,refn=%d,op=%d,opid=%d]",
						actor, type, num, pid, o->dval, o->refn, o->type, o->opid);

				o->ready = 1; /* tell the world that object is ready */
				asodecint(&o->refn); /* decrease o's reference count */
			}
			else
			{	unins += 1; /* already been constructed by someone else */
				vmfree(mmdc->vm, o);

				if(rv->refn <= 0 || rv->dval < 0) /* refn should be at least 1 */
					terror("%s %s [num=%d,pid=%d]: refn <= 0? obj[dval=%d,refn=%d,op=%d,opid=%d]",
						actor, type, num, pid, o->dval, o->refn, o->type, o->opid);

				if(!rv->sval) /* count #incomplete objects */
					vmmvalue(mmdc->vm, CDT_INCOMPLETE, (Void_t*)1, VM_MMADD);

				asodecint(&rv->refn); /* decrease rv's reference count */
			}
		}
			
		tinfo("%s %s [num=%d,pid=%d]: done, base=%d try=%d insert=%d[+%d]",
			actor, type, num, pid, base, W_EXTENT, insert, unins);

		/* total number of successful inserts */
		vmmvalue(mmdc->vm, CDT_INSERT, (Void_t*)((long)insert), VM_MMADD );
	}
	else if(strcmp(actor, "deleter") == 0 )
	{	srandom(num);
		size = dtsize(dt);
		tinfo("%s %s [num=%d,pid=%d]: dtsize=%d ready to go", actor, type, num, pid, size);

		for(o = dtfirst(dt); o; o = next )
		{	next = dtnext(dt,o);
			walk += 1;

			if(o->refn <= 0 ) /* refn should be >= 1 */
				terror("%s %s [num=%d,pid=%d]: refn <= 0? obj[dval=%d,refn=%d,op=%d,opid=%d]",
					actor, type, num, pid, o->dval, o->refn, o->type, o->opid);

			if(!o->sval) /* count incomplete objects */
				vmmvalue(mmdc->vm, CDT_INCOMPLETE, (Void_t*)1, VM_MMADD);

			asodecint(&o->refn);

			if(!(rv = dtdelete(dt,o)) )
				undel += 1; /* somebody beat us to it */
			else	delete += 1;
		}

		tinfo("%s %s [num=%d,pid=%d]: done, size=%d[walk=%d] delete=%d[+%d]",
			actor, type, num, pid, size, walk, delete, undel);

		size = 0; /* make sure that free elements were deleted right */
		for(o = mmdc->list; o; o = next)
		{	next = o->next;
			size += 1;
			if(o->free != 1) /* multiply deleted? */
				terror("%s %s [num=%d,pid=%d]: multiple delete? obj[dval=%d,free=%d,fpid=%d]",
					actor, type, num, mmdc->pid, o->dval, o->free, o->fpid);
			if(o->fpid != mmdc->pid) /* who deleted this? */
				terror("%s %s [num=%d,pid=%d]: Wrong deleter obj[dval=%d,free=%d,fpid=%d]",
					actor, type, num, mmdc->pid, o->dval, o->free, o->fpid);
		}
		if(size != delete)
			terror("%s %s [num=%d,pid=%d]: free=%d delete=%d", actor, type, num, pid, size, delete);

		/* save the number of deleted objects */
		vmmvalue(mmdc->vm, CDT_DELETE, (Void_t*)((long)delete), VM_MMADD );
	}
	else if(strcmp(actor, "searcher") == 0 )
	{	srandom(num);
		size = dtsize(dt);
		tinfo("%s %s [num=%d,pid=%d]: dtsize=%d ready to go", actor, type, num, pid, size);

		size = size > COLLISION*W_EXTENT ? size : COLLISION*W_EXTENT;
		if(num%2 == 0 ) /* this searcher searches random elements */
		{	for(k = 0; k < size; ++k)
			{	obj.dval = random()%(N_INSERT*W_EXTENT) + 1;
				if(!(rv = dtsearch(dt, &obj)) )
					unsrch += 1;
				else
				{	search += 1;

					if(rv->dval < 0) /* should not be freed yet */
						terror("%s %s [num=%d,pid=%d]: already freed? obj[%d,fpid=%d]",
							actor, type, num, pid, rv->dval, rv->fpid);

					if(rv->refn <= 0) /* refn should be at least 1 just for us */
						terror("%s %s [num=%d,pid=%d]: refn <= 0? obj[%d,refn=%d,op=%d,opid=%d]",
							actor, type, num, pid, rv->dval, rv->refn, rv->type, rv->opid);

					if(!rv->sval) /* count incomplete objects */
						vmmvalue(mmdc->vm, CDT_INCOMPLETE, (Void_t*)1, VM_MMADD);

					asodecint(&rv->refn);
				}
			}
			size = dtsize(dt);
			tinfo("%s %s [num=%d,pid=%d]: done, dtsize=%d search=%d[+%d]",
				actor, type, num, pid, size, search, unsrch);
		}
		else /* this searcher walks the dictionary */
		{	obj.dval = 0;
			for(k = 0; k < size; ++k)
			{	if((o = dtnext(dt, &obj)) ) 
					walk += 1;
				else if((o = dtfirst(dt)) )
					first += 1;
				else	break; /* empty dictionary */

				if(o->dval < 0) /* should not be deleted yet */
					terror("%s %s [num=%d,pid=%d]: already freed obj[%d,refn=%d,fpid=%d,op=%d,opid=%d ",
						actor, type, num, pid, o->dval, o->refn, o->fpid, o->type, o->opid);
				if(o->refn <= 0 ) /* refn must be >= 1 */
					terror("%s %s [num=%d,pid=%d]: refn<=0? obj[%d,refn=%d,op=%d,opid=%d",
						actor, type, num, pid, o->dval, o->refn, o->type, o->opid);

				if(!o->sval) /* count incomplete objects */
					vmmvalue(mmdc->vm, CDT_INCOMPLETE, (Void_t*)1, VM_MMADD);

				obj.dval = o->dval; /* for next search */
				asodecint(&o->refn); /* reduce reference count */
			}
			size = dtsize(dt);
			tinfo("%s %s [num=%d,pid=%d]: done, dtsize=%d walk=%d first=%d",
				actor, type, num, pid, size, walk, first);
		}
	}
	else	terror("%s %s [num=%d,pid=%d]: unknown actor", actor, num, pid);

	vmmvalue(mmdc->vm, CDT_FINISHED, (Void_t*)1, VM_MMADD); /* count a done process */

	for(p = 0; p < N_PROCESS; asorelax(1) ) /* wait until everyone is done */
		p = (int)((long)vmmvalue(mmdc->vm, CDT_FINISHED, (Void_t*)0, VM_MMGET));

	insert = (ssize_t)(unsigned long)vmmvalue(mmdc->vm, CDT_INSERT, 0, VM_MMGET);
	delete = (ssize_t)(unsigned long)vmmvalue(mmdc->vm, CDT_DELETE, 0, VM_MMGET);

	size = dtsize(dt);
	for(walk = 0, o = dtfirst(dt); o; o = dtnext(dt,o) )
		walk += 1;
	tinfo("%s %s [num=%d,pid=%d]: start walk, dtsize=%d, dtfirst/next=%d ins-del=%d",
		actor, type, num, pid, size, walk, insert-delete);
	if(insert < delete)
		terror("%s %s [num=%d,pid=%d]: insert=%d < delete=%d", actor, type, num, pid, insert, delete);
	if(walk != size)
		terror("%s %s [num=%d,pid=%d]: dtsize=%d != walk=%d", actor, type, num, pid, size, walk);

	/* concurrently walk the dictionary and add count to appropriate fields */
	i = strcmp(actor, "inserter") == 0 ? 1 : strcmp(actor, "deleter") == 0 ? 2 : 3;
	for(k = 0, o = (Obj_t*)dtfirst(dt); o; ++k, o = (Obj_t*)dtnext(dt,o) )
	{	if(o->dval < 0)
			terror("%s %s [num=%d,pid=%d]: object %d already freed",
				actor, type, num, pid, o->dval);

		if(i == 1) /* inserter */
		{	asoincint(&o->ins);
#ifdef DEBUG
			tlog(pid, "%d", o->dval);
#endif
		}
		else if(i == 2) /* deleter */
		{	asoincint(&o->del);
#ifdef DEBUG
			tlog(pid, "%d", o->dval);
#endif
		}
		else
		{	asoincint(&o->srch); /* searcher process */
#ifdef DEBUG
			tlog(pid, "%d", o->dval);
#endif
		}
	}

	/* close dictionary and shared/persistent region */
	dtclose(dt);

	return 0;
}

tmain()
{
	pid_t		wpid[N_INSERT+N_DELETE+N_SEARCH], ppid, pid;
	size_t		size, walk, i, k, p;
	int		t;
	struct timeval	begtm, endtm;
	Dt_t		*dt;
	Obj_t		*o;
	Mmdisc_t	*mmdc;
	Vmalloc_t	*vm;
	char		*aso, *a, *cmd, *store, *type;

	cmd = *argv;
	aso = taso(ASO_PROCESS);
	if(k = tchild())
	{	Mapstore = argv[k++];
		Shmstore = argv[k++];
		type = argv[k++];
		if(strcmp(type, "map") == 0 )
			store = Mapstore;
		else if(strcmp(type, "shm") == 0)
			store = Shmstore;
		else
			terror("%s: invalid store type -- { map shm } expected", type);
		a = argv[k++];
		if(strcmp(a, "inserter") == 0 ||
		   strcmp(a, "deleter") == 0 ||
		   strcmp(a, "searcher") == 0 )
			return readwrite(type, store, a, argv[k]);
		terror("%s: invalid child process operation -- { inserter deleter searcher } expected", a);
	}
	else if(*++argv)
		t = -1;
	else
		t = 0;
	if((ppid = getpid()) < 0 )
		terror("Parent: can't get process id");
	Mapstore = tstfile("map", -1);
	Shmstore = tstfile("shm", -1);
	(void)unlink(Mapstore); 
	(void)unlink(Shmstore); 

	for(; t < 2; t++)
	{	switch (t)
		{
		case -1:
			type = *argv++;
			if(strcmp(type, "map") == 0 )
				store = Mapstore;
			else if(strcmp(type, "shm") == 0)
				store = Shmstore;
			else
				terror("%s: invalid store type -- { map shm } expected", type);
			t = *argv ? -2 : 2;
			break;
		case 0:
			type = "map";
			store = Mapstore;
#if __sun
			twarn("Skipping %s test on __sun because high mmap() memory sync frequency makes it crawl", type);
			continue;
#else
			break;
#endif
		case 1:
			type = "shm";
			store = Shmstore;
			break;
		}

		tinfo("parent %s [pid=%d]: Testing %s concurrent accesses", type, ppid, type);
		gettimeofday(&begtm, 0);

		tinfo("parent %s [pid=%d]: initializing dictionary", type, ppid);
		if(!(dt = opendictionary("parent", type, 0, ppid, store)) )
			terror("parent %s [pid=%d]: Can't open %s dictionary", type, ppid, type);
		if(!(mmdc = (Mmdisc_t*)dtdisc(dt, (Dtdisc_t*)0, 0)) )
			terror("parent %s [pid=%d]: Can't get dictionary discipline", type, ppid);
		tinfo("parent %s [pid=%d]: share dictionary created", type, ppid);

		for(p = i = 0; i < N_INSERT; ++i) /* start inserters */
			if((wpid[p++] = makeprocess(cmd, type, "inserter", i, aso)) < 0 )
				terror("parent %s [pid=%d]: Could not make inserter process %d", type, ppid, i);

		for(i = 0; i < N_DELETE; ++i) /* start deleters */
			if((wpid[p++] = makeprocess(cmd, type, "deleter", i, aso)) < 0 )
				terror("parent %s [pid=%d]: Could not make deleter process %d", type, ppid, i);

		for(i = 0; i < N_SEARCH; ++i) /* start searchers */
			if((wpid[p++] = makeprocess(cmd, type, "searcher", i, aso)) < 0 )
				terror("parent %s [pid=%d]: Could not make searcher process %d", type, ppid, i);

		if (twait(wpid, p))
			terror("workload subprocess error");

		walk = 0; /* count objects from "persistent" dictionary itself */
		for(o = (Obj_t*)dtfirst(dt); o; o = (Obj_t*)dtnext(dt,o) ) 
		{	if(o->dval < 0 || o->dval > N_INSERT*W_EXTENT)
				terror("parent %s [pid=%d]: bad object", type, ppid);
			if(o->ins != N_INSERT)
				terror("parent %s [pid=%d]: object %d has wrong insert count %d",
					type, ppid, o->dval, o->ins);
			if(o->srch != N_SEARCH)
				terror("parent %s [pid=%d]: object %d has wrong search count %d",
					type, ppid, o->dval, o->srch);
			if(o->del != N_DELETE)
				terror("parent %s [pid=%d]: object %d has wrong delete count %d",
					type, ppid, o->dval, o->del);
#ifdef DEBUG
			tlog(ppid, "%d", o->dval);
#endif

			Count[o->dval] += 1;
			walk += 1;
		}

		for(k = 0; k < N_INSERT*W_EXTENT; ++k)
			if(Count[k] > 1) /* should be unique */
				terror("parent %s [pid=%d]: Count[%d] = %d > 1", type, ppid, k, Count[k]);

		size = (ssize_t)dtsize(dt);
		i = (ssize_t)(unsigned long)vmmvalue(mmdc->vm, CDT_INCOMPLETE, (Void_t*)0, VM_MMGET);
		tinfo("parent %s [pid=%d]: dtfirst/dtnext=%d dtsize=%d, incomplete=%d, no error.",
			type, ppid, walk, size, i);
		if(size != walk)
			terror("parent %s [pid=%d]: counts mismatched", type, ppid);

		tinfo("Storage type=%s, #insertions=%d #inserters=%d #deleters=%d #searchers=%d",
			type,  N_INSERT*W_EXTENT, N_INSERT, N_DELETE, N_SEARCH);

		gettimeofday(&endtm, 0);
		if(begtm.tv_usec > endtm.tv_usec)
		{	endtm.tv_sec -= 1;
			endtm.tv_usec += 1000000;
		}
		tinfo("Storage method %s: running time = %ld.%lds",
			DTMETHOD->name, endtm.tv_sec - begtm.tv_sec, endtm.tv_usec - begtm.tv_usec);

		vmmrelease(mmdc->vm, 1); /* clean up file/shmid data */
		dtclose(dt);
	}

	texit(0);
}
