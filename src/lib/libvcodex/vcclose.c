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

/*	Close a Vcodex_t handle
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
int vcclose(Vcodex_t* vc)
#else
int vcclose(vc)
Vcodex_t*	vc;
#endif
{
	Vcodex_t	*coder;

	if(!vc)
		return -1;

	coder = (vc->flags&VC_CLOSECODER) ? vc->coder : NIL(Vcodex_t*);

	if(vc->disc && vc->disc->eventf &&
	   (*vc->disc->eventf)(vc, VC_CLOSING, NIL(Void_t*), vc->disc) < 0 )
		return -1;
	vcbuffer(vc, NIL(Vcchar_t*), -1, -1); /* free all cached buffers */
	if(vcfreecontext(vc, NIL(Vccontext_t*)) < 0 )
		return -1;
	if(vc->meth && vc->meth->eventf &&
	   (*vc->meth->eventf)(vc, VC_CLOSING, NIL(Void_t*)) < 0 )
		return -1;
	free(vc);

	if(coder && vcclose(coder) < 0 )
		return -1;

	return 0;
}
