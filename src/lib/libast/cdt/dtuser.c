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
#include	"dthdr.h"

/* Perform various functions on the user's behalf.
**
** Written by Kiem-Phong Vo, phongvo@gmail.com (01/05/2012)
*/

/* managing the lock dt->data->user.lock */
int dtuserlock(Dt_t* dt, unsigned int key, int type)
{
	if(key == 0)
		return -1;
	else if(type > 0)
		return asolock(&dt->data->user.lock, key, ASO_LOCK);
	else if(type < 0)
		return asolock(&dt->data->user.lock, key, ASO_UNLOCK);
	else	return asolock(&dt->data->user.lock, key, ASO_TRYLOCK);
}

/* managing the user data slot dt->data->user.data */
Void_t* dtuserdata(Dt_t* dt, Void_t* data, int set)
{
	if(set == 0) /* just return current value */
		return asogetptr(&dt->data->user.data);
	else while(1)
	{	Void_t	*current = dt->data->user.data;
		if(asocasptr(&dt->data->user.data, current, data) == current)
			return	current;
	}
}

/* announcing an event on the user's behalf */
int dtuserevent(Dt_t* dt, int flags, Void_t* data)
{
	if(!dt->disc->eventf)
		return 0;
	else	return (*dt->disc->eventf)(dt, DT_ANNOUNCE|DT_USER|flags, data, dt->disc);
}
