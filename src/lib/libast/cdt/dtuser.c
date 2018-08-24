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
#include "config_ast.h"  // IWYU pragma: keep

#include "aso.h"
#include "ast_aso.h"
#include "cdtlib.h"

/* Perform various functions on the user's behalf.
**
** Written by Kiem-Phong Vo, phongvo@gmail.com (01/05/2012)
*/

// Lock dt->data->user.lock.
int dtuserlock(Dt_t *dt) { return pthread_mutex_lock(&dt->data->user.lock); }

// Unlock dt->data->user.lock.
int dtuserunlock(Dt_t *dt) { return pthread_mutex_unlock(&dt->data->user.lock); }

/* managing the user data slot dt->data->user.data */
void *dtuserdata(Dt_t *dt, void *data, int set) {
    if (set == 0) /* just return current value */
        return asogetptr(&dt->data->user.data);
    else
        while (1) {
            void *current = dt->data->user.data;
            if (asocasptr(&dt->data->user.data, current, data) == current) return current;
        }
}

/* announcing an event on the user's behalf */
int dtuserevent(Dt_t *dt, int flags, void *data) {
    if (!dt->disc->eventf)
        return 0;
    else
        return (*dt->disc->eventf)(dt, DT_ANNOUNCE | DT_USER | flags, data, dt->disc);
}
