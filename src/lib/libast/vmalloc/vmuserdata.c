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
#include	"vmhdr.h"

/*	Get a location in region to store values
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com
*/

#if __STD_C
Void_t* vmuserdata(Vmalloc_t* vm, unsigned int dtid, ssize_t size)
#else
Void_t* vmuserdata(vm, dtid, size)
Vmalloc_t*	vm;
unsigned int	dtid;	/* ID of the requested data	*/
ssize_t		size;	/* size of requested memory	*/
#endif
{
	unsigned int	key;
	Vmuser_t	*u;
	Vmdata_t	*vmdt = vm->data;

	key = asothreadid();
	asolock(&vmdt->ulck, key, ASO_LOCK);

	for(u = vmdt->user; u; u = u->next)
		if(u->dtid == dtid) /* found the entry matching key */
			break;

	if(!u && size > 0 && /* try making a new entry */
	   (u = KPVALLOC(vm, sizeof(Vmuser_t) + size, vm->meth.allocf)) )
	{	memset((Void_t*)(u+1), 0, size);
		u->dtid = dtid;
		u->size = size;
		u->data = (Void_t*)(u+1);
		u->next = vmdt->user;
		vmdt->user = u;
	}

	asolock(&vmdt->ulck, key, ASO_UNLOCK);

	return u ? u->data : NIL(Void_t*);
}
