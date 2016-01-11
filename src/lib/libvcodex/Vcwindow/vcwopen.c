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

/*	Open a handle for computing matching windows.
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
Vcwindow_t* vcwopen(Vcwdisc_t* disc, Vcwmethod_t* meth)
#else
Vcwindow_t* vcwopen(disc, meth)
Vcwdisc_t*	disc;
Vcwmethod_t*	meth;	/* window matching method	*/
#endif
{
	Vcwindow_t*	vcw;

	if(!disc || (!disc->srcf && !disc->tarf) )
		return NIL(Vcwindow_t*);
	if(disc->srcf && sfseek(disc->srcf, (Sfoff_t)0, 0) < 0)
		return NIL(Vcwindow_t*);
	if(disc->tarf && sfseek(disc->tarf, (Sfoff_t)0, 0) < 0)
		return NIL(Vcwindow_t*);

	if(!meth)
		meth = Vcwdecode;

	if(!(vcw = (Vcwindow_t*)calloc(1,sizeof(Vcwindow_t))) )
		return NIL(Vcwindow_t*);

	vcw->disc = disc;
	vcw->meth = meth;
	vcw->mtdata = NIL(Void_t*);

	if(disc->eventf && (*disc->eventf)(vcw, VCW_OPENING, 0, disc) < 0)
	{	vcwclose(vcw);
		return NIL(Vcwindow_t*);
	}

	if(meth->eventf && (*meth->eventf)(vcw, VCW_OPENING) < 0)
	{	vcwclose(vcw);
		return NIL(Vcwindow_t*);
	}

	return vcw;
}
