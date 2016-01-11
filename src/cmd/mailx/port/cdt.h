/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2011 The Regents of the University of California an*
*                                                                      *
* Redistribution and use in source and binary forms, with or           *
* without modification, are permitted provided that the following      *
* conditions are met:                                                  *
*                                                                      *
*    1. Redistributions of source code must retain the above           *
*       copyright notice, this list of conditions and the              *
*       following disclaimer.                                          *
*                                                                      *
*    2. Redistributions in binary form must reproduce the above        *
*       copyright notice, this list of conditions and the              *
*       following disclaimer in the documentation and/or other         *
*       materials provided with the distribution.                      *
*                                                                      *
*    3. Neither the name of The Regents of the University of California*
*       names of its contributors may be used to endorse or            *
*       promote products derived from this software without            *
*       specific prior written permission.                             *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND               *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,          *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF             *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             *
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS    *
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,             *
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED      *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,        *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON    *
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,      *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY       *
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE              *
* POSSIBILITY OF SUCH DAMAGE.                                          *
*                                                                      *
* Redistribution and use in source and binary forms, with or without   *
* modification, are permitted provided that the following conditions   *
* are met:                                                             *
* 1. Redistributions of source code must retain the above copyright    *
*    notice, this list of conditions and the following disclaimer.     *
* 2. Redistributions in binary form must reproduce the above copyright *
*    notice, this list of conditions and the following disclaimer in   *
*    the documentation and/or other materials provided with the        *
*    distribution.                                                     *
* 3. Neither the name of the University nor the names of its           *
*    contributors may be used to endorse or promote products derived   *
*    from this software without specific prior written permission.     *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS"    *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A      *
* PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS    *
* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT     *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF     *
* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  *
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT   *
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF   *
* SUCH DAMAGE.                                                         *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                                 gsf                                  *
*                                                                      *
***********************************************************************/
#ifndef _CDT_H
#define _CDT_H		1

/*
**	Public interface for the dictionary library
**
**      Written by Kiem-Phong Vo (07/15/95)
**	AT&T Bell Laboratories
*/

/*{KPV: standard definitions */

/* The symbol __STD_C indicates that the language is ANSI-C or C++ */
#ifndef __STD_C
#ifdef __STDC__
#define	__STD_C		1
#else
#if __cplusplus || c_plusplus
#define __STD_C		1
#else
#define __STD_C		0
#endif /*__cplusplus*/
#endif /*__STDC__*/
#endif /*__STD_C*/

/* For C++, extern symbols must be protected against name mangling */
#ifndef _BEGIN_EXTERNS_
#if __cplusplus || c_plusplus
#define _BEGIN_EXTERNS_	extern "C" {
#define _END_EXTERNS_	}
#else
#define _BEGIN_EXTERNS_
#define _END_EXTERNS_
#endif
#endif /*_BEGIN_EXTERNS_*/

/* _ARG_ simplifies function prototypes between K&R-C and more modern Cs */
#ifndef _ARG_
#if __STD_C
#define _ARG_(x)	x
#else
#define _ARG_(x)	()
#endif
#endif /*_ARG_*/

/* __INLINE__ if defined is the inline keyword */
#if !defined(__INLINE__)
#if defined(__cplusplus)
#define __INLINE__	inline
#else
#if defined(_WIN32) || _WINIX
#define __INLINE__	__inline
#endif/*_WIN32*/
#endif/*__cplusplus*/
#endif/*__INLINE__*/

/* The type Void_t is properly defined so that Void_t* can address any type */
#ifndef Void_t
#if __STD_C
#define Void_t		void
#else
#define Void_t		char
#endif
#endif /*Void_t*/

/* The NIL() macro simplifies defining nil pointers to a given type */
#ifndef NIL
#define NIL(type)	((type)0)
#endif /*NIL*/

/* For DLLs on systems allowing only pointers across client and library code.  */
#ifndef _PTR_
#if _PACKAGE_ast && !__BLD_DLL && _DLL_INDIRECT_DATA
#define _ADR_ 		/* data access via ptrs	only	*/
#define _PTR_	*
#else
#define _ADR_	&	/* normal exporting of data ok	*/
#define _PTR_ 
#endif
#endif /*_PTR_*/

#if !_PACKAGE_ast && _BLD_DLL && defined(_WIN32)
#if _KPV_ONLY		/* internal code compilation	*/
#define extern	__declspec(dllexport)
#else			/* client code compilation	*/
#define extern	__declspec(dllimport)
#endif
#endif

#if __STD_C
#include <stddef.h>
#else
#include <sys/types.h>
#endif

/*KPV} */

typedef struct _dtlink_s	Dtlink_t;
typedef struct _dthold_s	Dthold_t;
typedef struct _dtdisc_s	Dtdisc_t;
typedef struct _dtmethod_s	Dtmethod_t;
typedef struct _dtdata_s	Dtdata_t;
typedef struct _dt_s		Dt_t;
typedef struct _dt_s		Dict_t;	/* for libdict compatibility */
typedef struct _dtstat_s	Dtstat_t;
typedef Void_t*			(*Dtsearch_f)_ARG_((Dt_t*,Void_t*,int));
typedef Void_t* 		(*Dtmake_f)_ARG_((Dt_t*,Void_t*,Dtdisc_t*));
typedef void 			(*Dtfree_f)_ARG_((Dt_t*,Void_t*,Dtdisc_t*));
typedef int			(*Dtcompar_f)_ARG_((Dt_t*,char*,char*,Dtdisc_t*));
typedef unsigned long		(*Dthash_f)_ARG_((Dt_t*,char*,Dtdisc_t*));
typedef Void_t*			(*Dtmemory_f)_ARG_((Dt_t*,Void_t*,size_t,Dtdisc_t*));
typedef int			(*Dtevent_f)_ARG_((Dt_t*,int,Void_t*,Dtdisc_t*));

struct _dtlink_s
{	Dtlink_t*	right;	/* right child		*/
	union
	{ unsigned long	_hash;	/* hash value		*/
	  Dtlink_t*	_left;	/* left child		*/
	} hl;
};

/* private structure to hold an object */
struct _dthold_s
{	Dtlink_t	hdr;	/* header		*/
	Void_t*		obj;	/* user object		*/
};

/* method to manipulate dictionary structure */
struct _dtmethod_s 
{	Dtsearch_f	searchf; /* search function	*/
	int		type;	/* type of operation	*/
};

/* stuff that may have to be in shared memory */
struct _dtdata_s
{	int		type;	/* type of dictionary			*/
	Dtlink_t*	here;	/* finger to last search element	*/
	union
	{ Dtlink_t**	_htab;	/* hash table				*/
	  Dtlink_t*	_head;	/* linked list				*/
	} hh;
	int		ntab;	/* number of hash slots			*/
	int		size;	/* number of objects			*/
};

/* structure to hold methods that manipulate an object */
struct _dtdisc_s
{	int		key;	/* where key begins in object		*/
	int		size;	/* key size				*/
	int		link;	/* offset to Dtlink_t field		*/
	Dtmake_f	makef;	/* object constructor			*/
	Dtfree_f	freef;	/* object destructor			*/
	Dtcompar_f	comparf;/* to compare two objects		*/
	Dthash_f	hashf;	/* to compute a hash value for objects	*/
	Dtmemory_f	memoryf;/* to get memory for objects		*/
	Dtevent_f	eventf;	/* to process events			*/
};

/* the dictionary structure itself */
struct _dt_s
{	Dtsearch_f	searchf;/* searching function			*/
	Dtdisc_t*	disc;	/* method to manipulate objs		*/
	Dtdata_t*	data;	/* sharable data			*/
	Dtmemory_f	memoryf;/* function to alloc/free memory	*/
	Dtmethod_t*	meth;	/* dictionary method			*/
	int		type;	/* type information			*/
	int		nview;	/* number of parent view dictionaries	*/
	Dt_t*		view;	/* next on viewpath			*/
	Dt_t*		walk;	/* dictionary being walked		*/
};

/* structure to get status of a dictionary */
struct _dtstat_s
{	int	dt_meth;	/* method type				*/
	int	dt_size;	/* number of elements			*/
	int	dt_n;		/* number of chains or levels		*/
	int	dt_max;		/* max size of a chain or a level	*/
	int*	dt_count;	/* counts of chains or levels by size	*/
};

/* supported storage methods */
#define DT_HASH		000001	/* hashing with chaining		*/
#define DT_TREE		000002	/* self-adjusting tree			*/
#define DT_LIST		000004	/* linked list				*/
#define DT_STACK	000010	/* stack				*/
#define DT_QUEUE	000020	/* queue				*/
#define DT_METHODS	000037	/* all currently supported methods	*/

/* events */
#define DT_OPEN		1	/* a dictionary is being opened		*/
#define DT_CLOSE	2	/* a dictionary is being closed		*/
#define DT_DISC		3	/* discipline is about to be changed	*/
#define DT_METH		4	/* method is about to be changed	*/

/* asserts to dtdisc() */
#define DT_COMPARF	000001	/* compare functions equivalent		*/
#define DT_HASHF	000002	/* hash functions equivalent		*/

/* types of search - for internal use only */
#define DT_INSERT	000001	/* insert object if not found		*/
#define DT_DELETE	000002	/* delete object if found		*/
#define DT_SEARCH	000004	/* look for an object			*/
#define DT_NEXT		000010	/* look for next element		*/
#define DT_PREV		000020	/* find previous element		*/
#define DT_RENEW	000040	/* renewing an object			*/
#define DT_CLEAR	000100	/* clearing all objects			*/
#define DT_FIRST	000200	/* get first object			*/
#define DT_LAST		000400	/* get last object			*/
#define DT_MATCH	001000	/* find object matching key		*/

_BEGIN_EXTERNS_
extern Dt_t*		dtopen _ARG_((Dtdisc_t*, Dtmethod_t*));
extern int		dtclose _ARG_((Dt_t*));
extern Dt_t*		dtview _ARG_((Dt_t*, Dt_t*));
extern Dtdisc_t*	dtdisc _ARG_((Dt_t* dt, Dtdisc_t*, int));
extern Dtmethod_t*	dtmethod _ARG_((Dt_t*, Dtmethod_t*));

extern Dtlink_t*	dtflatten _ARG_((Dt_t*));
extern Dtlink_t*	dtextract _ARG_((Dt_t*));
extern int		dtrestore _ARG_((Dt_t*, Dtlink_t*));

extern int		dtwalk _ARG_((Dt_t*, int(*)(Dt_t*,Void_t*,Void_t*), Void_t*));

extern Void_t*		dtrenew _ARG_((Dt_t*, Void_t*));

extern int		dtsize _ARG_((Dt_t*));
extern int		dtstat _ARG_((Dt_t*, Dtstat_t*, int));
extern unsigned long	dtstrhash _ARG_((unsigned long, char*, int));

extern Dtmethod_t _PTR_	_Dthash;
extern Dtmethod_t _PTR_	_Dttree;
extern Dtmethod_t _PTR_	_Dtlist;
extern Dtmethod_t _PTR_	_Dtstack;
extern Dtmethod_t _PTR_	_Dtqueue;
_END_EXTERNS_

#define Dtset		(_ADR_ _Dthash)
#define Dtoset		(_ADR_ _Dttree)
#define Dtlist		(_ADR_ _Dtlist)
#define Dtstack		(_ADR_ _Dtstack)
#define Dtqueue		(_ADR_ _Dtqueue)

#define _DT_(d)		((Dt_t*)(d))

#define dtlink(d,e)	(((Dtlink_t*)(e))->right)
#define dtobj(d,e)	((_DT_(d)->disc->link < 0) ? (((Dthold_t*)(e))->obj) : \
				(Void_t*)((char*)(e) - _DT_(d)->disc->link) )
#define dtfinger(d)	(_DT_(d)->data->here ? dtobj((d),_DT_(d)->data->here) : \
				NIL(Void_t*) )

#define dtfirst(d)	(*(_DT_(d)->searchf))((d),NIL(Void_t*),DT_FIRST)
#define dtnext(d,o)	(*(_DT_(d)->searchf))((d),(Void_t*)(o),DT_NEXT)
#define dtlast(d)	(*(_DT_(d)->searchf))((d),NIL(Void_t*),DT_LAST)
#define dtprev(d,o)	(*(_DT_(d)->searchf))((d),(Void_t*)(o),DT_PREV)
#define dtsearch(d,o)	(*(_DT_(d)->searchf))((d),(Void_t*)(o),DT_SEARCH)
#define dtinsert(d,o)	(*(_DT_(d)->searchf))((d),(Void_t*)(o),DT_INSERT)
#define dtdelete(d,o)	(*(_DT_(d)->searchf))((d),(Void_t*)(o),DT_DELETE)
#define dtmatch(d,o)	(*(_DT_(d)->searchf))((d),(Void_t*)(o),DT_MATCH)
#define dtclear(d)	(*(_DT_(d)->searchf))((d),NIL(Void_t*),DT_CLEAR)

/* A linear congruential hash: h*127 + c + 987654321 */
#define dtcharhash(h,c)	((((unsigned long)(h))<<7) - ((unsigned long)(h)) + \
			 ((unsigned char)(c)) + 987654321L )

#endif /* _CDT_H */
