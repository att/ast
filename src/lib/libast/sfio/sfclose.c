/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
#include	"sfhdr.h"

/*	Close a stream. A file stream is synced before closing.
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
int sfclose(Sfio_t* f)
#else
int sfclose(f)
Sfio_t*	f;
#endif
{
	reg int		local, ex, rv;
	Void_t*		data = NIL(Void_t*);
	SFMTXDECL(f); /* declare a local stream variable for multithreading */

	SFMTXENTER(f, -1);

	GETLOCAL(f,local);

	if(!(f->mode&SF_INIT) &&
	   SFMODE(f,local) != (f->mode&SF_RDWR) &&
	   SFMODE(f,local) != (f->mode&(SF_READ|SF_SYNCED)) &&
	   _sfmode(f,SF_SYNCED,local) < 0)
		SFMTXRETURN(f,-1);

	/* closing a stack of streams */
	while(f->push)
	{	reg Sfio_t*	pop;

		if(!(pop = (*_Sfstack)(f,NIL(Sfio_t*))) )
			SFMTXRETURN(f,-1);

		if(sfclose(pop) < 0)
		{	(*_Sfstack)(f,pop);
			SFMTXRETURN(f,-1);
		}
	}

	rv = 0;
	if(f->disc == _Sfudisc)	/* closing the ungetc stream */
		f->disc = NIL(Sfdisc_t*);
	else if(f->file >= 0)	/* sync file pointer */
	{	f->bits |= SF_ENDING;
		rv = sfsync(f);
	}

	SFLOCK(f,0);

	/* raise discipline exceptions */
	if(f->disc && (ex = SFRAISE(f,local ? SF_NEW : SF_CLOSING,NIL(Void_t*))) != 0)
		SFMTXRETURN(f,ex);

	if(!local && f->pool)
	{	/* remove from pool */
		if(f->pool == &_Sfpool)
		{	reg int	n;

			POOLMTXLOCK(&_Sfpool);
			for(n = 0; n < _Sfpool.n_sf; ++n)
			{	if(_Sfpool.sf[n] != f)
					continue;
				/* found it */
				_Sfpool.n_sf -= 1;
				for(; n < _Sfpool.n_sf; ++n)
					_Sfpool.sf[n] = _Sfpool.sf[n+1];
				break;
			}
			POOLMTXUNLOCK(&_Sfpool);
		}
		else
		{	f->mode &= ~SF_LOCK;	/**/ASSERT(_Sfpmove);
			if((*_Sfpmove)(f,-1) < 0)
			{	SFOPEN(f,0);
				SFMTXRETURN(f,-1);
			}
			f->mode |= SF_LOCK;
		}
		f->pool = NIL(Sfpool_t*);
	}

	if(f->data && (!local || (f->flags&SF_STRING) || (f->bits&SF_MMAP) ) )
	{	/* free buffer */
#if _mmap_worthy
		if(f->bits&SF_MMAP)
			SFMUNMAP(f,f->data,f->endb-f->data);
		else
#endif
		if(f->flags&SF_MALLOC)
			data = (Void_t*)f->data;

		f->data = NIL(uchar*);
		f->size = -1;
	}

	/* zap the file descriptor */
	if(_Sfnotify)
		(*_Sfnotify)(f, SF_CLOSING, (void*)((long)f->file));
	if(f->file >= 0 && !(f->flags&SF_STRING))
	{	while(sysclosef(f->file) < 0 )
		{	if(errno == EINTR)
				errno = 0;
			else
			{	rv = -1;
				break;
			}
		}
	}
	f->file = -1;

	SFKILL(f);
	f->flags &= SF_STATIC;
	f->here = 0;
	f->extent = -1;
	f->endb = f->endr = f->endw = f->next = f->data;

	/* zap any associated auxiliary buffer */
	if(f->rsrv)
	{	free(f->rsrv);
		f->rsrv = NIL(Sfrsrv_t*);
	}

	/* delete any associated sfpopen-data */
	if(f->proc)
		rv = _sfpclose(f);

	/* destroy the mutex */
	if(f->mutex)
	{	(void)vtmtxclrlock(f->mutex);
		if(f != sfstdin && f != sfstdout && f != sfstderr)
		{	(void)vtmtxclose(f->mutex);
			f->mutex = NIL(Vtmutex_t*);
		}
	}

	if(!local)
	{	if(f->disc && (ex = SFRAISE(f,SF_FINAL,NIL(Void_t*))) != 0 )
		{	rv = ex;
			goto done;
		}

		if(!(f->flags&SF_STATIC) )
			free(f);
		else
		{	f->disc = NIL(Sfdisc_t*);
			f->stdio = NIL(Void_t*);
			f->mode = SF_AVAIL;
		}
	}

done:
	if(data)
		free(data);
	return rv;
}
