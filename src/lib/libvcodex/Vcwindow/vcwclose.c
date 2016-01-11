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
#include	"vcwhdr.h"

/*	Close a Vcwindow_t handle
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
int vcwclose(Vcwindow_t* vcw)
#else
int vcwclose(vcw)
Vcwindow_t*	vcw;
#endif
{
	if(!vcw)
		return -1;

	if(vcw->disc && vcw->disc->eventf &&
	   (*vcw->disc->eventf)(vcw, VCW_CLOSING, 0, vcw->disc) < 0 )
		return -1;

	if(vcw->meth && vcw->meth->eventf &&
	   (*vcw->meth->eventf)(vcw, VCW_CLOSING) < 0 )
		return -1;

	free(vcw);

	return 0;
}

