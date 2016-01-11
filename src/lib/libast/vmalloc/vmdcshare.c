/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#if defined(_UWIN) && defined(_BLD_ast)

void _STUB_vmdcshare(){}

#else

#include	"vmhdr.h"
#include	<sys/types.h>
#include	<string.h>
#if _hdr_unistd
#include	<unistd.h>
#endif

#include	<sys/mman.h>	/* mmap() headers	*/
#include	<sys/file.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#include	<sys/shm.h>	/* shm headers		*/
#include	<sys/ipc.h>

/* Create a discipline to allocate based on mmap() or shmget().
** Both ways can be used for allocating shared memory across processes.
** However, mmap() also allows for allocating persistent memory.
**
** Written by Kiem-Phong Vo, phongvo@gmail.com
*/

/* magic word signaling region is being initialized */
#define MM_JUST4US	((unsigned int)(('P'<<24) | ('N'<<16) | ('B'<<8) | ('I')) ) /* 1347306057 */

/* magic word signaling file/segment is ready */
#define	MM_MAGIC	((unsigned int)(('P'<<24) | ('&'<<16) | ('N'<<8) | ('8')) ) /* 1344687672 */

/* default mimimum region size */
#define MM_MINSIZE	(64*_Vmpagesize)

/* flags for actions on region closing */
#define MM_DETACH	01	/* detach all attached memory	*/
#define MM_REMOVE	02	/* remove files/segments	*/

/* macros to get the data section and size */
#define MMHEAD(name)	ROUND(sizeof(Mmvm_t)+strlen(name), ALIGN)
#define MMDATA(mmvm)	((Vmuchar_t*)(mmvm)->base + MMHEAD(mmvm->name))
#define MMSIZE(mmvm)	((mmvm)->size - MMHEAD(mmvm->name))

#ifdef S_IRUSR
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#else
#define FILE_MODE	0644
#endif

typedef struct _mmvm_s
{	unsigned int	magic;	/* magic bytes		*/
	Void_t*		base;	/* address to map to	*/
	ssize_t		size;	/* total data size	*/
	ssize_t		busy;	/* amount in use	*/
	key_t		shmkey;	/* shared segment's key	*/
	int		shmid;	/* shared segment's ID	*/
	int		proj;	/* project number	*/
	char		name[1];/* file or shm name	*/
} Mmvm_t;

typedef struct _mmdisc_s
{	Vmdisc_t	disc;	/* Vmalloc discipline	*/
	int		init;	/* initializing state	*/
	int		mode;	/* closing modes	*/
	Mmvm_t*		mmvm;	/* shared memory data	*/
	ssize_t		size;	/* desired memory size	*/
	key_t		shmkey;	/* shared segment's key	*/
	int		shmid;	/* shared segment's ID	*/
	int		proj;	/* shm project ID 	*/
	char		name[1];/* backing store/strID	*/
} Mmdisc_t;

#if DEBUG
#include	<stdio.h>
#include	<string.h>
int _vmmdump(Vmalloc_t* vm, int fd)
{
	char		mesg[1024];
	Mmdisc_t	*mmdc = (Mmdisc_t*)vm->disc;

	fd = fd < 0 ? 2 : fd;
	sprintf(mesg, "File: %s\n", mmdc->name ); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Project: %10d\n", mmdc->proj); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Memory:  %#010lx\n", mmdc->mmvm); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Size:    %10d\n", mmdc->size); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Shmid:   %10d\n", mmdc->shmid); write(fd, mesg, strlen(mesg));

	sprintf(mesg, "File header:\n"); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Magic:   %10d\n", mmdc->mmvm->magic); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Base:    %#010lx\n", mmdc->mmvm->base); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Size:    %10d\n", mmdc->mmvm->size); write(fd, mesg, strlen(mesg));
	sprintf(mesg, "Busy:    %10d\n", mmdc->mmvm->busy); write(fd, mesg, strlen(mesg));
	return 0;
}
#endif /*DEBUG*/

/* make a key from a string and a project number -- this is like ftok() */
static key_t mmkey(char* str, int proj)
{
	unsigned int	hash;
	key_t		key;

	/* Fowler-Knoll-Vo hash function */
#if _ast_sizeof_int == 8 /* 64-bit hash */
#define	FNV_PRIME	((1<<40) + (1<<8) + 0xb3)
#define FNV_OFFSET	14695981039346656037
#else /* 32-bit hash */
#define	FNV_PRIME	((1<<24) + (1<<8) + 0x93)
#define FNV_OFFSET	2166136261
#endif
	for(hash = FNV_OFFSET; *str; ++str)
		hash = (hash ^ str[0]) * FNV_PRIME;
	hash = (hash ^ proj) * FNV_PRIME; /* hash project number */

	return (key = (key_t)hash) <= 0 ? -key : key;
}

/* fix the mapped address for a region */
static Mmvm_t* mmfix(Mmvm_t* mmvm, Mmdisc_t* mmdc, int fd)
{
	Void_t	*base = mmvm->base;
	ssize_t	size = mmvm->size;

	if(base != (Void_t*)mmvm) /* mmvm is not right yet */
	{	/**/DEBUG_ASSERT(!base || (base && (VMLONG(base)%_Vmpagesize) == 0) );
		if(mmdc->proj < 0)
		{	munmap((Void_t*)mmvm, size); 
			mmvm = (Mmvm_t*)mmap(base, size, (PROT_READ|PROT_WRITE), (MAP_FIXED|MAP_SHARED), fd, (off_t)0);
		}
		else
		{	shmdt((Void_t*)mmvm);
			mmvm = (Mmvm_t*)shmat(mmdc->shmid, base, 0);
		}
		if(!mmvm || mmvm == (Mmvm_t*)(-1) )
			mmvm = NIL(Mmvm_t*);
	}

	return mmvm;
}

/* initialize region data */
static int mminit(Mmdisc_t* mmdc)
{
	Void_t		*base;
	int		try;
	int		fd = -1;
	ssize_t		extent, size = 0;
	Mmvm_t		*mmvm = NIL(Mmvm_t*);
	int		rv = -1;

	if(mmdc->mmvm) /* already done this */
		return 0;

	/* fixed size region so make it reasonably large */
	if((size = mmdc->size) < MM_MINSIZE )
		size =  MM_MINSIZE;
	size += MMHEAD(mmdc->name) + ALIGN;
	size  = ROUND(size, _Vmpagesize);

	/* get/create the initial segment of data */
	if(mmdc->proj < 0 ) /* proj < 0 means doing mmap() */
	{	/* this can be done in multiple processes */
		if((fd = open(mmdc->name, O_RDWR|O_CREAT, FILE_MODE)) < 0)
		{	/**/DEBUG_MESSAGE("vmdcshare: open() failed");
			goto done;
		}

		/* Note that the location being written to is always zero! */
		if((extent = (ssize_t)lseek(fd, (off_t)0, SEEK_END)) < 0)
		{	/**/DEBUG_MESSAGE("vmdcshare: lseek() to get file size failed");
			goto done;
		}

		if(extent < size) /* make the file size large enough */
		{	if(lseek(fd, (off_t)size, 0) != (off_t)size || write(fd, "", 1) != 1 )
			{	/**/DEBUG_MESSAGE("vmdcshare: attempt to extend file failed");
				goto done;
			}
		}

		/* map the file into memory */
		mmvm = (Mmvm_t*)mmap(NIL(Void_t*), size, (PROT_READ|PROT_WRITE), MAP_SHARED, fd, (off_t)0);
		if(!mmvm || mmvm == (Mmvm_t*)(-1) ) /* initial mapping failed */
		{	/**/DEBUG_MESSAGE("vmdcshare: mmap() failed");
			goto done;
		}
	}
	else 
	{	/* make the key and get/create an id for the share mem segment */
		if((mmdc->shmkey = mmkey(mmdc->name, mmdc->proj)) < 0 )
			goto done;
		if((mmdc->shmid = shmget(mmdc->shmkey, size, IPC_CREAT|FILE_MODE)) < 0 )
		{	/**/DEBUG_MESSAGE("vmdcshare: shmget() failed");
			goto done;
		}

		/* map the data segment into memory */
		mmvm = (Mmvm_t*)shmat(mmdc->shmid, NIL(Void_t*), 0);
		if(!mmvm || mmvm == (Mmvm_t*)(-1) ) /* initial mapping failed */
		{	/**/DEBUG_MESSAGE("vmdcshare: attempt to attach memory failed");
			shmctl(mmdc->shmid, IPC_RMID, 0);
			goto done;
		}
	}

#if 0
	switch (mmvm->magic)
	{
	case 0:			write(2, "vmalloc: mmvm->magic=0\n", 23); break;
	case MM_JUST4US:	write(2, "vmalloc: mmvm->magic=MM_JUST4US\n", 32); break;
	case MM_MAGIC:		write(2, "vmalloc: mmvm->magic=MM_MAGIC\n", 29); break;
	}
#endif
	/* all processes compete for the chore to initialize data */
	if(asocasint(&mmvm->magic, 0, MM_JUST4US) == 0 ) /* lucky winner: us! */
	{	if((base = vmmaddress(size)) != NIL(Void_t*))
		{	mmvm->base = base; /* this will be the base of the map */
			mmvm->size = size;
		}
		if(!base || !(mmvm = mmfix(mmvm, mmdc, fd)) )
		{	/* remove any resource just created */
			if(mmdc->proj >= 0) 
				shmctl(mmdc->shmid, IPC_RMID, 0);	
			else	unlink(mmdc->name);
			goto done;
		}

		mmdc->init = 1; 
		mmvm->base = base;
		mmvm->size = size;
		mmvm->busy = 0;
		mmvm->shmkey = mmdc->shmkey;
		mmvm->shmid = mmdc->shmid;
		mmvm->proj = mmdc->proj;
		strcpy(mmvm->name, mmdc->name);
		if(mmdc->proj < 0 ) /* flush to file */
			msync((Void_t*)mmvm, MMHEAD(mmvm->name), MS_SYNC);

		rv = 0; /* success, return this value to indicate a new map */
	}
	else /* wait for someone else to finish initialization */
	{	/**/DEBUG_ASSERT(mmdc->init == 0);
		if(mmvm->magic != MM_JUST4US && mmvm->magic != MM_MAGIC)
		{	/**/DEBUG_MESSAGE("vmdcshare: bad magic numbers");
			goto done;
		}

		for(try = 0;; asospinrest() ) /* wait for region completion */
		{	if(asogetint(&mmvm->magic) == MM_MAGIC )
				break;
			else if((try += 1) <= 0 ) /* too many tries */
			{	/**/DEBUG_MESSAGE("vmdcshare: waiting time exhausted");
				goto done;
			}
		}

		/* mapped the wrong memory */
		if(mmvm->proj != mmdc->proj || strcmp(mmvm->name, mmdc->name) != 0 )
		{	/**/DEBUG_MESSAGE("vmdcshare: wrong initialization parameters");
			goto done;
		}

		if(mmvm->base != (Void_t*)mmvm) /* not yet at the right address */
		{	if(!(mmvm = mmfix(mmvm, mmdc, fd)) )
			{	/**/DEBUG_MESSAGE("vmdcshare: Can't fix address");
				goto done;
			}
		}

		rv = 1; /* success, return this value to indicate a finished map */
	}

done:	if(fd >= 0)
		(void)close(fd);

	if(rv >= 0 ) /* successful construction of region */
	{	/**/DEBUG_ASSERT(mmvm && mmvm != (Mmvm_t*)(-1));
		mmdc->mmvm = mmvm;
	}
	else if(mmvm && mmvm != (Mmvm_t*)(-1)) /* error, remove map */
	{	/**/DEBUG_MESSAGE("vmdcshare: error during opening region");
		if(mmdc->proj < 0)
			(void)munmap((Void_t*)mmvm, size);
		else	(void)shmdt((Void_t*)mmvm);
	}

	return rv;
}

#if __STD_C /* end a file mapping */
static int mmend(Mmdisc_t* mmdc)
#else
static int mmend(mmdc)
Mmdisc_t*	mmdc;
#endif
{
	Mmvm_t		*mmvm;
	struct shmid_ds	shmds;

	if(!(mmvm = mmdc->mmvm) )
		return 0;

	if(mmdc->proj < 0 )
	{	(void)msync(mmvm->base, mmvm->size, MS_ASYNC);
		if(mmdc->mode&MM_DETACH)
		{	if(mmvm->base )
				(void)munmap(mmvm->base, mmvm->size);
		}
		if(mmdc->mode&MM_REMOVE)
			(void)unlink(mmdc->name);
	}
	else 
	{	if(mmdc->mode&MM_DETACH)
		{	if(mmvm->base )
				(void)shmdt(mmvm->base);
		}
		if(mmdc->mode&MM_REMOVE)
		{	if(mmdc->shmid >= 0 )
				(void)shmctl(mmdc->shmid, IPC_RMID, &shmds);
		}
	}

	mmdc->mmvm = NIL(Mmvm_t*);
	return 0;
}

#if __STD_C
static Void_t* mmgetmem(Vmalloc_t* vm, Void_t* caddr,
			size_t csize, size_t nsize, Vmdisc_t* disc)
#else
static Void_t* mmgetmem(vm, caddr, csize, nsize, disc)
Vmalloc_t*	vm;
Void_t*		caddr;
size_t		csize;
size_t		nsize;
Vmdisc_t*	disc;
#endif
{
	Mmvm_t		*mmvm;
	Mmdisc_t	*mmdc = (Mmdisc_t*)disc;

	if(!(mmvm = mmdc->mmvm) ) /* bad data */
		return NIL(Void_t*);

	/* this region allows only a single busy block! */
	if(caddr) /* resizing/freeing an existing block */
	{	if(caddr == MMDATA(mmvm) && nsize <= MMSIZE(mmvm) )
		{	mmvm->busy = nsize;
			return MMDATA(mmvm);
		}
		else	return NIL(Void_t*);
	}
	else /* requesting a new block */
	{	if(mmvm->busy == 0 )
		{	mmvm->busy = nsize;
			return MMDATA(mmvm);
		}
		else	return NIL(Void_t*);
	}
}

#if __STD_C
static int mmexcept(Vmalloc_t* vm, int type, Void_t* data, Vmdisc_t* disc)
#else
static int mmexcept(vm, type, data, disc)
Vmalloc_t*	vm;
int		type;
Void_t*		data;
Vmdisc_t*	disc;
#endif
{
	int		rv;
	Mmdisc_t	*mmdc = (Mmdisc_t*)disc;

	if(type == VM_OPEN)
	{	if(data) /* VM_OPEN event at start of vmopen() */
		{	if((rv = mminit(mmdc)) < 0 ) /* initialization failed */
				return -1;
			else if(rv == 0) /* just started a new map */
			{	/**/DEBUG_ASSERT(mmdc->init == 1);
				/**/DEBUG_ASSERT(mmdc->mmvm->magic == MM_JUST4US);
				return 0;
			}
			else /* an existing map was reconstructed */
			{	/**/DEBUG_ASSERT(mmdc->init == 0);
				/**/DEBUG_ASSERT(mmdc->mmvm->magic == MM_MAGIC);
				*((Void_t**)data) = MMDATA(mmdc->mmvm);
				return 1;
			}
		}
		else	return 0;
	}
	else if(type == VM_ENDOPEN) /* at end of vmopen() */
	{	if(mmdc->init) /* this is the initializing process! */
		{	/**/DEBUG_ASSERT(mmdc->mmvm->magic == MM_JUST4US);
			asocasint(&mmdc->mmvm->magic, MM_JUST4US, MM_MAGIC);

			if(mmdc->proj < 0) /* sync data to file now */
				msync((Void_t*)mmdc->mmvm, MMHEAD(mmdc->name), MS_SYNC);
		} /**/DEBUG_ASSERT(mmdc->mmvm->magic == MM_MAGIC);
		return 0;
	}
	else if(type == VM_CLOSE)
		return 1; /* tell vmclose not to free memory segments */
	else if(type == VM_ENDCLOSE) /* this is the final closing event */
	{	(void)mmend(mmdc);
		(void)vmfree(Vmheap, mmdc);
		return 0; /* all done */
	}
	else if(type == VM_DISC)
		return -1;
	else	return 0;
}

#if __STD_C
Vmdisc_t* vmdcshare(char* name, int proj, ssize_t size, int mode )
#else
Vmdisc_t* vmdcshare(name, proj, size, mode )
char*		name;	/* key or file persistent store		*/
int		proj;	/* project ID, < 0 for doing mmap	*/
ssize_t		size;	/* desired size for memory segment	*/
int		mode;	/*  1: keep memory segments		*/
			/*  0: release memory segments		*/
			/* -1: like 0 plus removing files/shms	*/
#endif
{
	Mmdisc_t	*mmdc;

	VMPAGESIZE();

	if(!name || !name[0] )
	{	/**/DEBUG_MESSAGE("vmdcshare: store name not given");
		return NIL(Vmdisc_t*);
	}
	if(size <= 0 )
	{	/**/DEBUG_MESSAGE("vmdcshare: store size <= 0");
		return NIL(Vmdisc_t*);
	}

	/* create discipline structure for getting memory from mmap */
	if(!(mmdc = vmalloc(Vmheap, sizeof(Mmdisc_t)+strlen(name))) )
	{	/**/DEBUG_MESSAGE("vmdcshare: failed to allocate discipline ");
		return NIL(Vmdisc_t*);
	}

	memset(mmdc, 0, sizeof(Mmdisc_t));
	mmdc->disc.memoryf = mmgetmem;
	mmdc->disc.exceptf = mmexcept;
	mmdc->disc.round   = (size/4) > _Vmsegsize ? _Vmsegsize : size/4;
	mmdc->disc.round   = ROUND(mmdc->disc.round, _Vmpagesize);
	mmdc->mmvm = NIL(Mmvm_t*);
	mmdc->size = size;
	mmdc->shmkey = -1;
	mmdc->shmid = -1;
	mmdc->init = 0;
	mmdc->mode = mode > 0 ? 0 : mode == 0 ? MM_DETACH : (MM_DETACH|MM_REMOVE);
	mmdc->proj = proj;
	strcpy(mmdc->name, name);

	return (Vmdisc_t*)mmdc;
}

#endif
