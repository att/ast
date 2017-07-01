/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"vchdr.h"

/*	Managing buffered data for a handle. 
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
Vcchar_t* _vcbuffer(Vcodex_t* vc, Vcchar_t* trunc, ssize_t size, ssize_t head)
#else
Vcchar_t* _vcbuffer(vc, trunc, size, head)
Vcodex_t*	vc;
Vcchar_t*	trunc;	/* if != NULL, a buffer to be truncated	*/
ssize_t		size;	/* the size needed for buffered data 	*/
ssize_t		head;	/* head room in front of buffer 	*/
#endif
{
	Vcbuffer_t	*b, *n;
	/**/DEBUG_DECLARE(static ssize_t, Busy=0)
#ifdef VMFL
	/**/DEBUG_DECLARE(Vmstat_t, statb) DEBUG_ASSERT(vmstat(Vmregion, &statb) >= 0);
#endif

	if(!vc)
		return NIL(Vcchar_t*);

	if(trunc) /* truncating a buffer */
	{	/* find the buffer */
		for(; vc; vc = vc->coder)
		{	for(n = NIL(Vcbuffer_t*), b = vc->list; b; n = b, b = b->next)
				if(trunc >= b->buf && trunc < b->buf+b->size)
					break;
			if(!b) /* not in this handle */
				continue;

			if(n) /* isolate b from buffer pool */
				n->next = b->next;
			else	vc->list = b->next;

			if(size < 0 ) /* just free the buffer */
			{	/**/DEBUG_SET(Busy, Busy - b->size);
				/**/DEBUG_PRINT(2,"free: file=%s ", b->file);
				/**/DEBUG_PRINT(2,"line=%d ",b->line);
				/**/DEBUG_PRINT(2,"size=%d\n",b->size);

				vc->busy -= b->size;
				vc->nbuf -= 1;
				free(b);
				return NIL(Vcchar_t*);
			}

			if(trunc+size > b->buf+b->size) /* no extension */
			{	b->next = vc->list;
				vc->list = b;
				return NIL(Vcchar_t*); 
			}

			size += (head = trunc - (Vcchar_t*)b->buf);
			if(size < 3*b->size/4 )
			{	if(!(n = (Vcbuffer_t*)realloc(b, sizeof(Vcbuffer_t)+size)) )
					RETURN(NIL(Vcchar_t*));
				/**/DEBUG_SET(Busy, Busy - b->size + size);
				/**/DEBUG_PRINT(2,"realloc: file=%s ", b->file);
				/**/DEBUG_PRINT(2,"line=%d ",b->line);
				/**/DEBUG_PRINT(2,"oldsize=%d ",b->size);
				/**/DEBUG_PRINT(2,"newsize=%d\n",size);

				vc->busy -= n->size - size; /* n->size is old b->size */
				n->size = size;
				if(n != b)
					b = n;
			}

			b->next = vc->list;
			vc->list = b;
			return (Vcchar_t*)(&b->buf[head]);
		}

		return NIL(Vcchar_t*);
	}
	else if(size < 0) /* free all buffers */
	{	for(; vc; vc = vc->coder)
		{	if(vc->meth->eventf) /* tell vc to free its internal buffers */
				(*vc->meth->eventf)(vc, VC_FREEBUFFER, 0);

			for(b = vc->list; b; b = n)
			{	n = b->next;

				/**/DEBUG_SET(Busy, Busy - b->size);
				/**/DEBUG_PRINT(2,"free: file=%s ", b->file);
				/**/DEBUG_PRINT(2,"line=%d ",b->line);
				/**/DEBUG_PRINT(2,"size=%d\n",b->size);
				free(b);
			}

			vc->list = NIL(Vcbuffer_t*);
			vc->busy = 0;
			vc->nbuf = 0;
		}

		return NIL(Vcchar_t*);
	}
	else
	{	head = (head <= 0 ? 0 : head) + vc->head; /* required head room */
		if(!(b = (Vcbuffer_t*)malloc(sizeof(Vcbuffer_t)+head+size)) )
			RETURN(NIL(Vcchar_t*));
		b->size = head+size;
		b->next = vc->list;
		b->file = vc->file; vc->file = NIL(char*);
		b->line = vc->line; vc->line = 0;
		/**/DEBUG_SET(Busy, Busy + b->size);
		/**/DEBUG_PRINT(2,"alloc: file=%s ", b->file);
		/**/DEBUG_PRINT(2,"line=%d ",b->line);
		/**/DEBUG_PRINT(2,"size=%d\n",b->size);

		vc->list = b;
		vc->busy += b->size;
		vc->nbuf += 1;
		return (Vcchar_t*)(&b->buf[head]);
	}
}
