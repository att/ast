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
#ifndef _CDTLIB_H
#define _CDTLIB_H 1

/*      cdt library/method implementation header
**      this header is exported to the method libraries
**      Written by Kiem-Phong Vo, phongvo@gmail.com (5/25/96)
*/

#include "ast.h"

#include <pthread.h>
#include <unistd.h>

#include "aso.h"
#include "cdt.h"

/* min #bits for a hash table. (1<<this) is table size */
#define DT_HTABLE 10

/* convenient types */
#if !defined(uint)
#define uint unsigned int
#endif
#if !defined(uchar)
#define uchar unsigned char
#endif

/* This struct holds private method data created on DT_OPEN */
struct _dtdata_s {
    pthread_mutex_t lock; /* general dictionary lock    */
    unsigned int type;    /* method type, control flags */
    ssize_t size;         /* number of objects          */
    Dtuser_t user;        /* application's data         */
    Dt_t dict;            /* when DT_INDATA is requested        */
};

/* this structure holds the plugin information */
typedef struct _dtlib_s {
    char *name;           /* short name */
    char *description;    /* short description */
    char *release;        /* release info */
    char *prefix;         /* name prefix */
    Dtmethod_t **methods; /* method list */
} Dtlib_t;

// These macros lock/unlock dictionaries.
#define DTSETLOCK(dt) (((dt)->data->type & DT_SHARE) ? pthread_mutex_lock(&(dt)->data->lock) : 0)
#define DTCLRLOCK(dt) (((dt)->data->type & DT_SHARE) ? pthread_mutex_unlock(&(dt)->data->lock) : 0)

// DTRETURN substitutes for "return".
#define DTRETURN(ob, rv) \
    do {                 \
        (ob) = (rv);     \
        goto dt_return;  \
    } while (0)
#define DTERROR(dt, mesg)                \
    (!((dt)->disc && (dt)->disc->eventf) \
         ? 0                             \
         : (*(dt)->disc->eventf)((dt), DT_ERROR, (void *)(mesg), (dt)->disc))

// Announce completion of an operation of type <type> on some object <object> in dictionary <dt>.
static inline void DTANNOUNCE(Dt_t *dt, void *object, int type) {
    if (!object) return;
    if (!(type & DT_TOANNOUNCE)) return;
    if (!(dt->data->type & DT_ANNOUNCE)) return;
    if (!dt->disc) return;
    if (!dt->disc->eventf) return;
    (*dt->disc->eventf)(dt, DT_ANNOUNCE | type, object, dt->disc);
}

/* short-hands for fields in Dtlink_t.
** note that __hash is used as a hash value
** or as the position in the parent table.
*/
#define _left lh.__left
#define _hash lh.__hash
#define _ppos lh.__hash

#define _rght rh.__rght
#define _ptbl rh.__ptbl

/* tree rotation/linking functions */
#define rrotate(x, y) ((x)->_left = (y)->_rght, (y)->_rght = (x))
#define lrotate(x, y) ((x)->_rght = (y)->_left, (y)->_left = (x))
#define rlink(r, x) ((r) = (r)->_left = (x))
#define llink(l, x) ((l) = (l)->_rght = (x))

#define RROTATE(x, y) (rrotate(x, y), (x) = (y))
#define LROTATE(x, y) (lrotate(x, y), (x) = (y))
#define LLSHIFT(x, t) \
    ((t) = (x)->_rght->_rght, (x)->_rght->_rght = (t)->_left, (t)->_left = (x), (x) = (t))
// #define RRSHIFT(x, t)
//     ((t) = (x)->_left->_left, (x)->_left->_left = (t)->_rght, (t)->_rght = (x), (x) = (t))

extern Dtlink_t *_dtmake(Dt_t *, void *, int);
extern void _dtfree(Dt_t *, Dtlink_t *, int);

#endif  // _CDTLIB_H
