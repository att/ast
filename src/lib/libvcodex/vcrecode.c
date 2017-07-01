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

/* Recode a chunk of data using a secondary coder.
**
** Written by Kiem-Phong Vo 
*/

#if __STD_
int vcrecode(Vcodex_t* vc, Vcchar_t** dtp, ssize_t* dtz, ssize_t head, int type)
#else 
int vcrecode(vc, dtp, dtz, head, type)
Vcodex_t*	vc;	/* coding handle	*/
Vcchar_t**	dtp;	/* input/output data	*/
ssize_t*	dtz;	/* input/output size	*/
ssize_t		head;	/* extra header		*/
int		type;	/* = 0: undone is bad	*/
#endif
{
	ssize_t		sz;
	Vcchar_t	*dt;

	if(!vc->coder) /* nothing to do */
		return 0;

	/* ask follow-on coders to leave head room */
	vc->coder->head += vc->head + head;
	sz = vcapply(vc->coder, *dtp, *dtz, &dt);
	vc->coder->head -= vc->head + head;

	if(sz < 0) /* secondary coder failed */
		return -1;

	if(type == 0) /* no undone data allowed */
	{	if(vcundone(vc->coder) > 0)
			return -1;
	}
	else /* undone data ok and reflected upward */
	{	vc->undone = vc->coder->undone;
	}

	*dtp = dt; *dtz = sz;
	return 0;
}
