/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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

/* Construct/deconstruct data usable for persistent storage of a handle.
**
** Written by Kiem-Phong Vo.
*/

#define N_CODERS	1024	/* max #coders		*/

typedef struct _store_s
{	Vcmethod_t*	meth;	/* method 		*/
	Vcchar_t*	data;	/* associated data	*/
	ssize_t		dtsz;	/* length of data	*/
} Store_t;

#if __STD_C
ssize_t vcextract(Vcodex_t* vc, Void_t** datap)
#else
ssize_t vcextract(vc, datap)
Vcodex_t*	vc;
Void_t**	datap;
#endif
{
	ssize_t		n, k, dtsz;
	Store_t		store[N_CODERS];
	Vcchar_t	*data;
	Vcodex_t	*coder;
	Vcmtcode_t	mtcd;
	int		rv;
	char		*ident, buf[1024];
	Vcio_t		io;

	if(!(vc->flags&VC_ENCODE) )
		return -1;

	dtsz = 0; /* get all the encoding strings */
	for(n = -1, coder = vc; coder; coder = coder->coder)
	{	if((n += 1) >= N_CODERS)
			return -1; /* too many continuation coders */

		store[n].meth = coder->meth;
		if(!(ident = vcgetident(coder->meth, buf, sizeof(buf))) || !ident[0] )
			return -1;
		dtsz += strlen(ident) + 1;

		store[n].dtsz = 0;
		if(coder->meth->eventf)
		{	mtcd.data = NIL(Vcchar_t*);
			mtcd.size = 0;
			rv = (*coder->meth->eventf)(coder, VC_EXTRACT, (Void_t*)(&mtcd));
			if(rv < 0)
				return -1;
			else if(rv > 0)
			{	if((store[n].dtsz = mtcd.size) < 0 )
					return -1;
				store[n].data = mtcd.data;
			}
		}

		dtsz += vcsizeu(store[n].dtsz) + store[n].dtsz;
	}

	if(!(data = vcbuffer(vc, NIL(Vcchar_t*), dtsz, 0)) )
		return -1;

	/* write out data of all coders */
	vcioinit(&io, data, dtsz);
	for(k = 0; k <= n; ++k)
	{	if(!(ident = vcgetident(store[k].meth, buf, sizeof(buf))) )
			return -1;
		vcioputs(&io, ident, strlen(ident)+1);
		vcioputu(&io, store[k].dtsz);
		vcioputs(&io, store[k].data, store[k].dtsz);
	}

	if(datap)
		*datap = (Void_t*)data;
	return dtsz;
}


#if __STD_C
Vcodex_t* vcrestore(Void_t* data, size_t dtsz)
#else
Vcodex_t* vcrestore(data, dtsz)
Void_t*		data;
size_t		dtsz;
#endif
{
	Vcodex_t	*vc, *coder, *cdr;
	Vcmethod_t	*meth;
	ssize_t		sz, k;
	char		*mt;
	Void_t		*dt;
	Vcmtcode_t	mtcd;
	int		rv;
	Vcio_t		io;

	if(!data || dtsz <= 0 )
		return NIL(Vcodex_t*);

	vcioinit(&io, data, dtsz);
	vc = coder = NIL(Vcodex_t*);

	while(vciomore(&io) > 0)
	{	
		mt = (char*)vcionext(&io);
		for(sz = vciomore(&io), k = 0; k < sz; ++k)
			if(mt[k] == 0)
				break;
		if(k >= sz)
			goto error;
		if(!(meth = vcgetmeth(mt, 1)) )
			goto error;
		vcioskip(&io, k+1);

		/* get the initialization data, if any */
		if((sz = (ssize_t)vciogetu(&io)) < 0 || sz > vciomore(&io))
			goto error; 
		dt = vcionext(&io);
		vcioskip(&io, sz);

		cdr = NIL(Vcodex_t*);
		if(meth->eventf)
		{	mtcd.data  = dt;
			mtcd.size  = sz;
			mtcd.coder = NIL(Vcodex_t*);
			rv = (*meth->eventf)(0, VC_RESTORE, (Void_t*)(&mtcd));
			if(rv < 0 )
				goto error;
			else if(rv > 0)
				cdr = mtcd.coder;
		}

		if(!cdr && !(cdr = vcopen(0, meth, 0, 0, VC_DECODE)) )
		{ error:
			if(vc)
				vcclose(vc);
			return NIL(Vcodex_t*);
		}

		if(!coder)
			vc = cdr;
		else
		{	coder->coder = cdr;
			coder->flags |= VC_CLOSECODER;
		}
		coder = cdr;
	}

	return vc;
}
