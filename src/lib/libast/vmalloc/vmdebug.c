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

void _STUB_vmdebug(){}

#else

#include	"vmhdr.h"

/*	Method to help with debugging. This does rigorous checks on
**	addresses and arena integrity.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94.
*/

/* The lay-out of a block allocated by Vmdebug looks like this:
**	pack size 0xdbdb size file line pack magi ----data---- --magi---
**	--------- ----------- --------- --------- ------------ ---------
**
**	pack,size: header required by Vmbest management.
**
**	0xdbdb:	bit pattern to mark that this is a Vmdebug block.
**	size:	the byte count given to vmalloc() or vmresize().
**
**	file,line: the file and line where the block was created.
**
**	pack:	should be the Vmbest pack so that vmregion() will work.
**	magi:	magic bytes to detect overwrites.
**
**	data:	the actual data block.
**	magi:	more magic bytes.
*/

#define KEY_DEBUG	(20130501)	/* a time of change */

/* convenient macros for accessing the above fields */
#define DB_HEAD		(3*sizeof(Head_t))
#define DB_TAIL		(1*sizeof(Head_t))
#define DB_EXTRA	(DB_HEAD+DB_TAIL)

#define DBBLOCK(d)	((Block_t*)((Vmuchar_t*)(d) - 4*sizeof(Head_t)) ) /* Vmbest block */
#define DBBSIZE(d)	(BDSZ(DBBLOCK(d)) ) /* Vmbest block size */
#define DBBEND(d)	((Vmuchar_t*)DBBLOCK(d) + DBBSIZE(d) + sizeof(Head_t) ) /* block end */

#define DBMARK(d)	(((Head_t*)((Vmuchar_t*)(d) - 3*sizeof(Head_t)))->head.one.size )
#define DBSIZE(d)	(((Head_t*)((Vmuchar_t*)(d) - 3*sizeof(Head_t)))->head.two.size )

#define DBFILE(d)	(((Head_t*)((Vmuchar_t*)(d) - 2*sizeof(Head_t)))->head.one.ptrdt )
#define DBLN(d)		(((Head_t*)((Vmuchar_t*)(d) - 2*sizeof(Head_t)))->head.two.intdt )
#define DBLINE(d)	(DBLN(d) < 0 ? -DBLN(d) : DBLN(d))

#define DBPACK(d)	(((Head_t*)((Vmuchar_t*)(d) - sizeof(Head_t)))->head.one.ptrdt )

/* forward/backward translation for addresses between Vmbest and Vmdebug */
#define DB2BEST(d)	((Vmuchar_t*)(d) - 3*sizeof(Head_t))
#define DB2DEBUG(b)	((Vmuchar_t*)(b) + 3*sizeof(Head_t))

/* set file and line number, note that DBLN > 0 so that DBISBAD will work  */
#define DBSETFL(d,f,l)	(DBFILE(d) = (f), DBLN(d) = (f) ? (l) : 1)

/* set and test the state of known to be corrupted */
#define DBSETBAD(d)	(DBLN(d) > 0 ? (DBLN(d) = -DBLN(d)) : -1)
#define DBISBAD(d)	(DBLN(d) <= 0)

#define DB_MAGIC	0x99	   /* the magic byte: 10011001	*/
#define DB_MARK		0xdbdbdbdb /* tell a busy Vmdebug block	*/

/* compute the bounds of the magic areas */
#define DBHEAD(d,begp,endp)	(((begp) = (Vmuchar_t*)(d) - sizeof(Void_t*)), ((endp) = (d)) )
#define DBTAIL(d,begp,endp)	(((begp) = (Vmuchar_t*)(d)+DBSIZE(d)), ((endp) = DBBEND(d)) )

/* structure to keep track of file names */
typedef struct _dbfile_s	Dbfile_t;
struct _dbfile_s
{	Dbfile_t*	next;
	char		file[1];
};
static Dbfile_t*	Dbfile;
	
/* global watch list */
#define S_WATCH	32
static int	Dbnwatch;
static Void_t*	Dbwatch[S_WATCH];

/* types of warnings reported by dbwarn() */
#define	DB_CHECK	0
#define DB_ALLOC	1
#define DB_FREE		2
#define DB_RESIZE	3
#define DB_WATCH	4
#define DB_RESIZED	5

static int Dbinit = 0;
#define DBINIT()	(Dbinit ? 0 : (dbinit(), Dbinit=1) )
static void dbinit()
{	int	fd;	
	if((fd = vmtrace(-1)) >= 0)
		vmtrace(fd);
}

static int	Dbfd = 2;	/* default warning file descriptor */
#if __STD_C
int vmdebug(int fd)
#else
int vmdebug(fd)
int	fd;
#endif
{
	int	old = Dbfd;
	Dbfd = fd;
	return old;
}


/* just an entry point to make it easy to set break point */
#if __STD_C
static void vmdbwarn(Vmalloc_t* vm, char* mesg, int n)
#else
static void vmdbwarn(vm, mesg, n)
Vmalloc_t*	vm;
char*		mesg;
int		n;
#endif
{
	write(Dbfd,mesg,n);
	if(vm->data->mode&VM_DBABORT)
		abort();
}

/* issue a warning of some type */
#if __STD_C
static void dbwarn(Vmalloc_t* vm, Void_t* data, int where,
		   char* file, int line, Void_t* func, int type)
#else
static void dbwarn(vm, data, where, file, line, func, type)
Vmalloc_t*	vm;	/* region holding the block	*/
Void_t*		data;	/* data block			*/
int		where;	/* byte that was corrupted	*/
char*		file;	/* file where call originates	*/
int		line;	/* line number of call		*/
Void_t*		func;	/* function called from		*/
int		type;	/* operation being done		*/
#endif
{
	char	buf[1024], *bufp, *endbuf, *s;
#define SLOP	64	/* enough for a message and an int */

	DBINIT();

	bufp = buf;
	endbuf = buf + sizeof(buf);

	if(type == DB_ALLOC)
		bufp = (*_Vmstrcpy)(bufp, "alloc error", ':');
	else if(type == DB_FREE)
		bufp = (*_Vmstrcpy)(bufp, "free error", ':');
	else if(type == DB_RESIZE)
		bufp = (*_Vmstrcpy)(bufp, "resize error", ':');
	else if(type == DB_CHECK)
		bufp = (*_Vmstrcpy)(bufp, "corrupted data", ':');
	else if(type == DB_WATCH)
		bufp = (*_Vmstrcpy)(bufp, "alert", ':');

	/* region info */
	bufp = (*_Vmstrcpy)(bufp, "region", '=');
	bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(vm), 0), ':');

	if(data)
	{	bufp = (*_Vmstrcpy)(bufp,"block",'=');
		bufp = (*_Vmstrcpy)(bufp,(*_Vmitoa)(VMLONG(data),0),':');
	}

	if(!data)
	{	if(where == DB_ALLOC)
			bufp = (*_Vmstrcpy)(bufp, "failed to get memory", ':');
		else	bufp = (*_Vmstrcpy)(bufp, "locked region", ':');
	}
	else if(type == DB_FREE || type == DB_RESIZE)
	{	if(where == 0)
			bufp = (*_Vmstrcpy)(bufp, "outside region", ':');
		else	bufp = (*_Vmstrcpy)(bufp, "not currently allocated", ':');
	}
	else if(type == DB_WATCH)
	{	bufp = (*_Vmstrcpy)(bufp, "size", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(DBSIZE(data),-1), ':');
		if(where == DB_ALLOC)
			bufp = (*_Vmstrcpy)(bufp,"just allocated", ':');
		else if(where == DB_FREE)
			bufp = (*_Vmstrcpy)(bufp,"being freed", ':');
		else if(where == DB_RESIZE)
			bufp = (*_Vmstrcpy)(bufp,"being resized", ':');
		else if(where == DB_RESIZED)
			bufp = (*_Vmstrcpy)(bufp,"just resized", ':');
	}
	else if(type == DB_CHECK)
	{	bufp = (*_Vmstrcpy)(bufp, "bad byte at", '=');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(where),-1), ':');
		if((s = DBFILE(data)) && (bufp + strlen(s) + SLOP) < endbuf)
		{	bufp = (*_Vmstrcpy)(bufp,"allocated at", '=');
			bufp = (*_Vmstrcpy)(bufp, s, ',');
			bufp = (*_Vmstrcpy)(bufp,(*_Vmitoa)(VMLONG(DBLINE(data)),-1),':');
		}
	}

	/* location where offending call originates from */
	if(file && file[0] && line > 0 && (bufp + strlen(file) + SLOP) < endbuf)
	{	bufp = (*_Vmstrcpy)(bufp, "detected at", '=');
		bufp = (*_Vmstrcpy)(bufp, file, ',');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(line),-1), ',');
		bufp = (*_Vmstrcpy)(bufp, (*_Vmitoa)(VMLONG(func),-1), ':');
	}

	*bufp++ = '\n';
	*bufp = '\0';

	vmdbwarn(vm,buf,(bufp-buf));
}

/* check for watched address and issue warnings */
#if __STD_C
static void dbwatch(Vmalloc_t* vm, Void_t* data,
		    char* file, int line, Void_t* func, int type)
#else
static void dbwatch(vm, data, file, line, func, type)
Vmalloc_t*	vm;
Void_t*		data;
char*		file;
int		line;
Void_t*		func;
int		type;
#endif
{
	int	n;

	for(n = Dbnwatch; n >= 0; --n)
	{	if(Dbwatch[n] == data)
		{	dbwarn(vm,data,type,file,line,func,DB_WATCH);
			return;
		}
	}
}

/* record information about the block */
#if __STD_C
static void dbsetinfo(Vmuchar_t* data, size_t size, char* file, int line)
#else
static void dbsetinfo(data, size, file, line)
Vmuchar_t*	data;	/* real address not the one from Vmbest	*/
size_t		size;	/* the actual requested size		*/
char*		file;	/* file where the request came from	*/
int		line;	/* and line number			*/
#endif
{
	Vmuchar_t	*begp, *endp;
	Dbfile_t	*last, *db;

	DBINIT();

	/* find the file structure */
	if(!file || !file[0])
		db = NIL(Dbfile_t*);
	else
	{	for(last = NIL(Dbfile_t*), db = Dbfile; db; last = db, db = db->next)
			if(strcmp(db->file,file) == 0)
				break;
		if(!db)
		{	db = (Dbfile_t*)vmalloc(Vmheap,sizeof(Dbfile_t)+strlen(file));
			if(db)
			{	(*_Vmstrcpy)(db->file,file,0);
				db->next = Dbfile;
				Dbfile = db->next;
			}
		}
		else if(last) /* move-to-front heuristic */
		{	last->next = db->next;
			db->next = Dbfile;
			Dbfile = db->next;
		}
	}

	DBMARK(data) = DB_MARK;
	DBSIZE(data) = size;

	DBSETFL(data,(db ? db->file : NIL(char*)),line);

	DBPACK(data) = PACK(DBBLOCK(data));

	DBHEAD(data,begp,endp);
	while(begp < endp)
		*begp++ = DB_MAGIC;

	DBTAIL(data,begp,endp);
	while(begp < endp)
		*begp++ = DB_MAGIC;
}

#if __STD_C
static Void_t* dballoc(Vmalloc_t* vm, size_t size, int local)
#else
static Void_t* dballoc(vm, size, local)
Vmalloc_t*	vm;
size_t		size;
int		local;
#endif
{
	size_t		sz;
	Vmuchar_t	*data;
	char		*file;
	int		line;
	Void_t		*func;
	VMFLF(vm,file,line,func);

	asolock(&vm->data->dlck, KEY_DEBUG, ASO_LOCK);

	if(vm->data->mode&VM_DBCHECK)
		vmdbcheck(vm);

	sz = ROUND(size,ALIGN) + DB_EXTRA;
	sz = sz >= sizeof(Body_t) ? sz : sizeof(Body_t);
	if(!(data = (Vmuchar_t*)KPVALLOC(vm, sz, (*(Vmbest->allocf))) ) )
	{	dbwarn(vm, NIL(Vmuchar_t*), DB_ALLOC, file, line, func, DB_ALLOC);
		goto done;
	}

	data = DB2DEBUG(data);
	dbsetinfo(data, size, file, line);

	if(_Vmtrace)
	{	vm->file = file; vm->line = line; vm->func = func;
		(*_Vmtrace)(vm, NIL(Vmuchar_t*), data, size, 0);
	}

	if(Dbnwatch > 0 )
		dbwatch(vm, data, file, line, func, DB_ALLOC);

done:	asolock(&vm->data->dlck, KEY_DEBUG, ASO_UNLOCK);
	return (Void_t*)data;
}

#if __STD_C
static int dbfree(Vmalloc_t* vm, Void_t* data, int local )
#else
static int dbfree(vm, data, local )
Vmalloc_t*	vm;
Void_t*		data;
int		local;
#endif
{
	char		*file;
	int		line;
	Void_t		*func;
	Seg_t		*seg;
	Free_t		*list;
	Free_t		*item;
	int		rv = 0;

	VMFLF(vm,file,line,func);

	if(!data)
		return 0;

	if(asocasint(&vm->data->dlck, 0, KEY_DEBUG))
	{	/* prepend to delayed free list -- handled by another free() that gets the lock */
		asospindecl();
		item = (Free_t*)data;
		for (asospininit();; asospinnext())
		{	item->next = list = vm->data->delay;
			if (asocasptr(&vm->data->delay, list, item) == (void*)list)
				break;
		}
		return 0;
	}

	list = 0;
	for (;;)
	{	/* check to see if memory is from vm */
		for(seg = vm->data->seg; seg; seg = seg->next)
			if((Vmuchar_t*)data >= seg->base && (Vmuchar_t*)data < seg->base+seg->size)
				break;
		if(!seg || DBMARK(data) != DB_MARK)
		{	dbwarn(vm, data, seg ? 1 : 0, file, line, func, DB_FREE);
			goto done;
		}

		if(vm->data->mode&VM_DBCHECK)
			vmdbcheck(vm);

		if(Dbnwatch > 0)
			dbwatch(vm,data,file,line,func,DB_FREE);

		if(_Vmtrace)
		{	vm->file = file; vm->line = line; vm->func = func;
			(*_Vmtrace)(vm, (Vmuchar_t*)data, NIL(Vmuchar_t*), DBSIZE(data), 0);
		}

		memset(DB2BEST(data), 0, DBBSIZE(data)); /* clear memory */

		rv |= KPVFREE((vm), (Void_t*)DB2BEST(data), (*Vmbest->freef));
	done:	
		if(!list && (rv || !(list = vm->data->delay) || asocasptr(&vm->data->delay, list, NIL(Free_t*)) != list))
			break;
		data = (void*)list;
		list = list->next;
	}

 	asocasint(&vm->data->dlck, KEY_DEBUG, 0);

	return rv;
}

/*	Resizing an existing block */
#if __STD_C
static Void_t* dbresize(Vmalloc_t* vm, Void_t* addr, size_t size, int type, int local)
#else
static Void_t* dbresize(vm, addr, size, type, local)
Vmalloc_t*	vm;	/* region allocating from	*/
Void_t*		addr;	/* old block of data		*/
size_t		size;	/* new size			*/
int		type;	/* !=0 for movable, >0 for copy	*/
int		local;
#endif
{
	size_t		sz, oldsize;
	Seg_t		*seg;
	char		*file, *oldfile;
	int		line, oldline;
	Void_t		*func;
	Vmuchar_t	*data = NIL(Vmuchar_t*);
	VMFLF(vm,file,line,func);

	if(!addr)
	{	data = (Vmuchar_t*)dballoc(vm, size, local);
		if(data && (type&VM_RSZERO) )
			memset((Void_t*)data, 0, size);
		return data;
	}
	if(size == 0)
	{	(void)dbfree(vm, addr, local);
		return NIL(Void_t*);
	}

	asolock(&vm->data->dlck, KEY_DEBUG, ASO_LOCK);

	/* check to see if memory is from vm */
	for(seg = vm->data->seg; seg; seg = seg->next)
		if((Vmuchar_t*)addr >= seg->base && (Vmuchar_t*)addr < seg->base+seg->size)
			break;
	if(!seg || DBMARK(addr) != DB_MARK)
	{	dbwarn(vm, addr, seg ? 1 : 0, file, line, func, DB_RESIZE);
		goto done;
	}

	if(vm->data->mode&VM_DBCHECK)
		vmdbcheck(vm);

	if(Dbnwatch > 0)
		dbwatch(vm,addr,file,line,func,DB_RESIZE);

	/* Vmbest data block */
	data = DB2BEST(addr);
	oldsize = DBSIZE(addr);
	oldfile = DBFILE(addr);
	oldline = DBLINE(addr);

	/* do the resize */
	sz = ROUND(size,ALIGN) + DB_EXTRA;
	sz = sz >= sizeof(Body_t) ? sz : sizeof(Body_t);
	data = (Vmuchar_t*)KPVRESIZE(vm, (Void_t*)data, sz, (type&~VM_RSZERO), (*(Vmbest->resizef)) );
	if(!data) /* failed, reset data for old block */
	{	dbwarn(vm, NIL(Vmuchar_t*), DB_ALLOC, file, line, func, DB_RESIZE);
		dbsetinfo((Vmuchar_t*)addr, oldsize, oldfile, oldline);
	}
	else
	{	data = DB2DEBUG(data);
		dbsetinfo(data, size, file, line);

		if(_Vmtrace)
		{	vm->file = file; vm->line = line;
			(*_Vmtrace)(vm, (Vmuchar_t*)addr, data, size, 0);
		}
		if(Dbnwatch > 0)
			dbwatch(vm, data, file, line, func, DB_RESIZED);
	}

	if(data && (type&VM_RSZERO) && size > oldsize)
	{	Vmuchar_t *dt = data+oldsize, *ed = data+size;
		do { *dt++ = 0; } while(dt < ed);
	}

done:	asolock(&vm->data->dlck, KEY_DEBUG, ASO_UNLOCK);
	return (Void_t*)data;
}

/* check for memory overwrites over all live blocks */
#if __STD_C
int vmdbcheck(Vmalloc_t* vm)
#else
int vmdbcheck(vm)
Vmalloc_t*	vm;
#endif
{
	Block_t	*sgb, *bp, *endbp;
	Seg_t*	seg;
	int	rv;

	/* check the meta-data of this region */
	if(vm->data->mode & (VM_MTDEBUG|VM_MTBEST))
	{	if((*vm->meth.eventf)(vm, VM_CHECKARENA, NIL(Void_t*)) < 0 )
			return -1;
		if(!(vm->data->mode&VM_MTDEBUG) )
			return 0;
	}
	else	return -1;

	rv = 0;
	for(seg = vm->data->seg; seg; seg = seg->next)
	{	sgb = (Block_t*)SEGDATA(seg); 
		for(; sgb < seg->endb; sgb = NEXT(sgb) )
		{	if(!(SIZE(sgb)&BUSY) )
				continue;

			bp = (Block_t*)DATA(sgb); endbp = ENDB(sgb);
			for(; bp < endbp; bp = NEXT(bp) )
			{	Vmuchar_t	*data, *begp, *endp;

				if(!(SIZE(bp)&BUSY) ) /* not a busy block */
					continue;

				data = DB2DEBUG(DATA(bp));
				if(DBMARK(data) != DB_MARK) /* not a Vmdebug block */
					continue;

				if(DBISBAD(data)) /* seen this corruption before */
				{	rv += 1;
					continue;
				}

				DBHEAD(data,begp,endp);
				for(; begp < endp; ++begp)
					if(*begp != DB_MAGIC)
						goto set_bad;

				DBTAIL(data,begp,endp);
				for(; begp < endp; ++begp)
				{	if(*begp == DB_MAGIC)
						continue;
				set_bad:
					dbwarn(vm,data,begp-data,NIL(char*),0,0,DB_CHECK);
					DBSETBAD(data);
					rv += 1;
					break;
				}
			}
		}
	}

	return rv;
}

/* set/delete an address to watch */
#if __STD_C
Void_t* vmdbwatch(Void_t* addr)
#else
Void_t* vmdbwatch(addr)
Void_t*		addr;	/* address to insert	*/
#endif
{
	int		n;
	Void_t*		out;

	out = NIL(Void_t*);
	if(!addr)
		Dbnwatch = 0;
	else
	{	for(n = Dbnwatch - 1; n >= 0; --n)
			if(Dbwatch[n] == addr)
				break;
		if(n < 0)	/* insert */
		{	if(Dbnwatch == S_WATCH)	
			{	/* delete left-most */
				out = Dbwatch[0];
				Dbnwatch -= 1;
				for(n = 0; n < Dbnwatch; ++n)
					Dbwatch[n] = Dbwatch[n+1];
			}
			Dbwatch[Dbnwatch] = addr;
			Dbnwatch += 1;
		}
	}
	return out;
}

#if __STD_C
static Void_t* dbalign(Vmalloc_t* vm, size_t size, size_t align, int local)
#else
static Void_t* dbalign(vm, size, align, local)
Vmalloc_t*	vm;
size_t		size;
size_t		align;
int		local;
#endif
{
	Vmuchar_t	*data;
	size_t		sz;
	char		*file;
	int		line;
	Void_t		*func;
	VMFLF(vm,file,line,func);

	if(size <= 0 || align <= 0)
		return NIL(Void_t*);

	asolock(&vm->data->dlck, KEY_DEBUG, ASO_LOCK);

	if((sz = ROUND(size,ALIGN) + DB_EXTRA) < sizeof(Body_t))
		sz = sizeof(Body_t);

	if((data = (Vmuchar_t*)KPVALIGN(vm, sz, align, (*(Vmbest->alignf)))) )
	{	data += DB_HEAD;
		dbsetinfo(data,size,file,line);

		if(_Vmtrace)
		{	vm->file = file; vm->line = line; vm->func = func;
			(*_Vmtrace)(vm,NIL(Vmuchar_t*),data,size,align);
		}
	}

	asolock(&vm->data->dlck, KEY_DEBUG, ASO_UNLOCK);

	return (Void_t*)data;
}

static int dbstat(Vmalloc_t* vm, Vmstat_t* st, int local)
{
	return (*Vmbest->statf)(vm, st, local);
}

static int dbevent(Vmalloc_t* vm, int event, Void_t* arg)
{
	if(event == VM_BLOCKHEAD) /* Vmbest asking for size of extra head */
		return (int)DB_HEAD;
	else	return (*Vmbest->eventf)(vm, event, arg);
}

/* print statistics of region vm. If vm is NULL, use Vmregion */
#if __STD_C
ssize_t vmdbstat(Vmalloc_t* vm)
#else
ssize_t vmdbstat(vm)
Vmalloc_t*	vm;
#endif
{	
	ssize_t		n;
	Vmstat_t	st;

	if(vmstat(vm, &st) >= 0 )
	{	n = strlen(st.mesg);
		write(Dbfd, st.mesg, n);
		return n;
	}
	else	return -1;
}

static Vmethod_t _Vmdebug =
{	dballoc,
	dbresize,
	dbfree,
	0,
	dbstat,
	dbevent,
	dbalign,
	VM_MTDEBUG
};

__DEFINE__(Vmethod_t*,Vmdebug,&_Vmdebug);

#ifdef NoF
NoF(vmdebug)
#endif

#endif
