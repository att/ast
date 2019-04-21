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

#include <stddef.h>

#include "sfhdr.h"
#include "sfio.h"
#include "vthread.h"

/*      External variables and functions used only by Sfio
**      Written by Kiem-Phong Vo
*/

/* code to initialize mutexes */
static Vtmutex_t Sfmutex;
static Vtonce_t Sfonce = VTONCE_INITDATA;
static_fn void _sfoncef() {
    (void)vtmtxopen(_Sfmutex, VT_INIT);
    (void)vtmtxopen(&_Sfpool.mutex, VT_INIT);
    (void)vtmtxopen(sfstdin->mutex, VT_INIT);
    (void)vtmtxopen(sfstdout->mutex, VT_INIT);
    (void)vtmtxopen(sfstderr->mutex, VT_INIT);
    _Sfdone = 1;
}

#if _ast_sizeof_pointer < 8
#define SF_MAXM_DEFAULT (0)
#else
#define SF_MAXM_DEFAULT (128 * 1024 * 1024)
#endif

#define SF_TEST_DEFAULT (0)

/* global variables used internally to the package */
Sfextern_t _Sfextern = {
    0,                // _Sfpage
    {.next = NULL},   // _Sfpool
    NULL,             // _Sfpmove
    NULL,             // _Sfstack
    NULL,             // _Sfnotify
    NULL,             // _Sfstdsync
    {.readf = NULL},  // _Sfudisc
    NULL,             // _Sfcleanup
    0,                // _Sfexiting
    0,                // _Sfdone
    &Sfonce,          // _Sfonce
    _sfoncef,         // _Sfoncef
    &Sfmutex,         // _Sfmutex
    SF_MAXM_DEFAULT,  // _Sfmaxm
    SF_TEST_DEFAULT   // _Sftest
};

ssize_t _Sfi = -1;   /* value for a few fast macro functions    */
ssize_t _Sfmaxr = 0; /* default (unlimited) max record size     */

#if vt_threaded
static Vtmutex_t _Sfmtxin, _Sfmtxout, _Sfmtxerr;
#define SFMTXIN (&_Sfmtxin)
#define SFMTXOUT (&_Sfmtxout)
#define SFMTXERR (&_Sfmtxerr)
#define SF_STDSAFE SF_MTSAFE
#else
#define SFMTXIN (0)
#define SFMTXOUT (0)
#define SFMTXERR (0)
#define SF_STDSAFE (0)
#endif

Sfio_t _Sfstdin = SFNEW(NULL, -1, 0, (SF_READ | SF_STATIC | SF_STDSAFE), NULL, SFMTXIN);
Sfio_t _Sfstdout = SFNEW(NULL, -1, 1, (SF_WRITE | SF_STATIC | SF_STDSAFE), NULL, SFMTXOUT);
Sfio_t _Sfstderr = SFNEW(NULL, -1, 2, (SF_WRITE | SF_STATIC | SF_STDSAFE), NULL, SFMTXERR);

#undef sfstdin
#undef sfstdout
#undef sfstderr

Sfio_t *sfstdin = &_Sfstdin;
Sfio_t *sfstdout = &_Sfstdout;
Sfio_t *sfstderr = &_Sfstderr;
