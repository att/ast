/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
#ifndef _VTHREAD_H
#define _VTHREAD_H 1

// #define VTHREAD_VERSION 20001201L

/*      Header for the Vthread library.
**      Note that the macro vt_threaded may be defined
**      outside of vthread.h to suppress threading.
**
**      Written by Kiem-Phong Vo, kpv@research.att.com
*/

#include <errno.h>

/* ast doesn't do threads yet */
#if !defined(vt_threaded)
#define vt_threaded 0
#endif

#if !defined(vt_threaded) || (defined(vt_threaded) && vt_threaded == 1)
#define _may_use_threads 1
#else
#define _may_use_threads 0
#endif
#undef vt_threaded

#if _may_use_threads && !defined(vt_threaded)
#define vt_threaded 1
#include <pthread.h>
typedef pthread_mutex_t _vtmtx_t;
typedef pthread_once_t _vtonce_t;
typedef pthread_t _vtself_t;
typedef pthread_t _vtid_t;
typedef pthread_attr_t _vtattr_t;

#if !defined(PTHREAD_ONCE_INIT) && defined(pthread_once_init)
#define PTHREAD_ONCE_INIT pthread_once_init
#endif

#endif

#ifndef vt_threaded
#define vt_threaded 0
#endif

typedef struct _vtmutex_s Vtmutex_t;
typedef struct _vtonce_s Vtonce_t;
typedef struct _vthread_s Vthread_t;

extern Vthread_t *vtopen(Vthread_t *, int);
extern int vtclose(Vthread_t *);
extern int vtset(Vthread_t *, int, void *);
extern int vtrun(Vthread_t *, void *(*)(void *), void *);
extern int vtkill(Vthread_t *);
extern int vtwait(Vthread_t *);

extern int vtonce(Vtonce_t *, void (*)());

extern Vtmutex_t *vtmtxopen(Vtmutex_t *, int);
extern int vtmtxclose(Vtmutex_t *);
extern int vtmtxlock(Vtmutex_t *);
extern int vtmtxtrylock(Vtmutex_t *);
extern int vtmtxunlock(Vtmutex_t *);
extern int vtmtxclrlock(Vtmutex_t *);

extern void *vtstatus(Vthread_t *);
extern int vterror(Vthread_t *);
extern int vtmtxerror(Vtmutex_t *);
extern int vtonceerror(Vtonce_t *);

#if vt_threaded

/* mutex structure */
struct _vtmutex_s {
    _vtmtx_t lock;
    int count;
    _vtid_t owner;
    int state;
    int error;
};

/* structure for states of thread */
struct _vthread_s {
    _vtself_t self;  /* self-handle             */
    _vtid_t id;      /* thread id               */
    _vtattr_t attrs; /* attributes              */
    size_t stack;    /* stack size              */
    int state;       /* execution state */
    int error;       /* error status    */
    void *exit;      /* exit value              */
};

/* structure for exactly once execution */
struct _vtonce_s {
    int done;
    _vtonce_t once;
    int error;
};

#define VTONCE_INITDATA \
    { 0, PTHREAD_ONCE_INIT }

#define vtstatus(vt) ((vt)->exit)
#define vterror(vt) ((vt)->error)
#define vtmtxerror(mtx) ((mtx)->error)
#define vtonceerror(once) ((once)->error)

#endif /*vt_threaded*/

/* fake structures and functions */
#if !vt_threaded
struct _vtmutex_s {
    int error;
};
struct _vtattr_s {
    int error;
};
struct _vthread_s {
    int error;
};
struct _vtonce_s {
    int error;
};

typedef int _vtmtx_t;
typedef int _vtonce_t;
typedef int _vtself_t;
typedef int _vtid_t;
typedef int _vtattr_t;

#define VTONCE_INITDATA \
    { 0 }

#define vtopen(vt, flgs) (NULL)
#define vtclose(vt) (-1)
#define vtkill(vt) (-1)
#define vtwait(vt) (-1)
#define vtrun(vt, fn, arg) (-1)

#define vtset(vt, t, v) (-1)
#define vtonce(on, fu) (-1)

#define vtmtxopen(mtx, flgs) (NULL)
#define vtmtxclose(mtx) (-1)
#define vtmtxlock(mtx) (-1)
#define vtmtxtrylock(mtx) (-1)
#define vtmtxunlock(mtx) (-1)
#define vtmtxclrlock(mtx) (-1)

#define vtstatus(vt) (NULL)
#define vterror(vt) (0)
#define vtmtxerror(mtx) (0)
#define vtonceerror(once) (0)

#endif /*!vt_threaded*/

#endif  // _VTHREAD_H
