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
#include <stdlib.h>
#include <string.h>

#include "ast_assert.h"
#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"
#include "vthread.h"

/*      Management of pools of streams.
**      If pf is not nil, f is pooled with pf and f becomes current;
**      otherwise, f is isolated from its pool. flag can be one of
**      0 or SF_SHARE.
**
**      Written by Kiem-Phong Vo.
*/

/* Note that we do not free the space for a pool once it is allocated.
** This is to prevent memory faults in calls such as sfsync(NULL) that walk the pool
** link list and during such walks may free up streams&pools. Free pools will be
** reused in newpool().
*/
static_fn int delpool(Sfpool_t *p) {
    POOLMTXENTER(p);

    if (p->s_sf && p->sf != p->array) free(p->sf);
    p->mode = SF_AVAIL;

    POOLMTXRETURN(p, 0);
}

static_fn Sfpool_t *newpool(int mode) {
    Sfpool_t *p, *last = &_Sfpool;

    /* look to see if there is a free pool */
    for (last = &_Sfpool, p = last->next; p; last = p, p = p->next) {
        if (p->mode == SF_AVAIL) {
            p->mode = 0;
            break;
        }
    }

    if (!p) {
        POOLMTXLOCK(last);

        p = malloc(sizeof(Sfpool_t));
        if (!p) {
            POOLMTXUNLOCK(last);
            return NULL;
        }

        (void)vtmtxopen(&p->mutex, VT_INIT); /* initialize mutex */

        p->mode = 0;
        p->n_sf = 0;
        p->next = NULL;
        last->next = p;

        POOLMTXUNLOCK(last);
    }

    POOLMTXENTER(p);

    p->mode = mode & SF_SHARE;
    p->s_sf = sizeof(p->array) / sizeof(p->array[0]);
    p->sf = p->array;

    POOLMTXRETURN(p, p);
}

/* move a stream to head */
static_fn int _sfphead(Sfpool_t *p, Sfio_t *f, int n) {
    Sfio_t *head;
    ssize_t k, w, v;
    int rv;

    POOLMTXENTER(p);

    if (n == 0) POOLMTXRETURN(p, 0);

    head = p->sf[0];
    if (SFFROZEN(head)) POOLMTXRETURN(p, -1);

    SFLOCK(head, 0)
    rv = -1;

    if (!(p->mode & SF_SHARE) || (head->mode & SF_READ) || (f->mode & SF_READ)) {
        if (SFSYNC(head) < 0) goto done;
    } else {  // shared pool of write-streams, data can be moved among streams
        if (SFMODE(head, 1) != SF_WRITE &&  //!OCLINT(constant conditional operator)
            _sfmode(head, SF_WRITE, 1) < 0) {
            goto done;
        }
        assert(f->next == f->data);

        v = head->next - head->data; /* pending data            */
        if ((k = v - (f->endb - f->data)) <= 0) {
            k = 0;
        } else /* try to write out amount exceeding f's capacity */
        {
            if ((w = SFWR(head, head->data, k, head->disc)) == k) {
                v -= k;
            } else /* write failed, recover buffer then quit */
            {
                if (w > 0) {
                    v -= w;
                    memmove(head->data, (head->data + w), v);
                }
                head->next = head->data + v;
                goto done;
            }
        }

        /* move data from head to f */
        if ((head->data + k) != f->data) memmove(f->data, (head->data + k), v);
        f->next = f->data + v;
    }

    f->mode &= ~SF_POOL;
    head->mode |= SF_POOL;
    head->next = head->endr = head->endw = head->data; /* clear write buffer */

    p->sf[n] = head;
    p->sf[0] = f;
    rv = 0;

done:
    head->mode &= ~SF_LOCK; /* partially unlock because it's no longer head */

    POOLMTXRETURN(p, rv);
}

/* delete a stream from its pool */
static_fn int _sfpdelete(Sfpool_t *p, Sfio_t *f, int n) {
    POOLMTXENTER(p);

    p->n_sf -= 1;
    for (; n < p->n_sf; ++n) p->sf[n] = p->sf[n + 1];

    f->pool = NULL;
    f->mode &= ~SF_POOL;

    if (p->n_sf == 0 || p == &_Sfpool) {
        if (p != &_Sfpool) delpool(p);
        goto done;
    }

    /* !_Sfpool, make sure head stream is an open stream */
    for (n = 0; n < p->n_sf; ++n) {
        if (!SFFROZEN(p->sf[n])) break;
    }
    if (n < p->n_sf && n > 0) {
        f = p->sf[n];
        p->sf[n] = p->sf[0];
        p->sf[0] = f;
    }

    /* head stream has SF_POOL off */
    f = p->sf[0];
    f->mode &= ~SF_POOL;
    if (!SFFROZEN(f)) _SFOPEN(f)

    /* if only one stream left, delete pool */
    if (p->n_sf == 1) {
        _sfpdelete(p, f, 0);
        _sfsetpool(f);
    }

done:
    POOLMTXRETURN(p, 0);
}

static_fn int _sfpmove(Sfio_t *f, int type) {
    Sfpool_t *p;
    int n;

    if (type > 0) return _sfsetpool(f);
    p = f->pool;
    if (!p) return -1;
    for (n = p->n_sf - 1; n >= 0; --n) {
        if (p->sf[n] == f) break;
    }
    if (n < 0) return -1;

    return type == 0 ? _sfphead(p, f, n) : _sfpdelete(p, f, n);
}

Sfio_t *sfpool(Sfio_t *f, Sfio_t *pf, int mode) {
    int k;
    Sfpool_t *p;
    Sfio_t *rv;

    _Sfpmove = _sfpmove;

    if (!f) {  // return head of pool of pf regardless of lock states
        if (!pf) return NULL;
        if (!pf->pool || pf->pool == &_Sfpool) return pf;
        return pf->pool->sf[0];
    }

    if (f) {  // check for permissions
        SFMTXLOCK(f)
        if ((f->mode & SF_RDWR) != f->mode && _sfmode(f, 0, 0) < 0) {
            SFMTXUNLOCK(f)
            return NULL;
        }
        if (f->disc == _Sfudisc) (void)sfclose((*_Sfstack)(f, NULL));
    }
    if (pf) {
        SFMTXLOCK(pf)
        if ((pf->mode & SF_RDWR) != pf->mode && _sfmode(pf, 0, 0) < 0) {
            SFMTXUNLOCK(f)
            SFMTXUNLOCK(pf)
            return NULL;
        }
        if (pf->disc == _Sfudisc) (void)sfclose((*_Sfstack)(pf, NULL));
    }

    /* f already in the same pool with pf */
    if (f == pf || (pf && f->pool == pf->pool && f->pool != &_Sfpool)) {
        SFMTXUNLOCK(f)
        SFMTXUNLOCK(pf)
        return pf;
    }

    /* lock streams before internal manipulations */
    rv = NULL;
    SFLOCK(f, 0)
    if (pf) SFLOCK(pf, 0)

    if (!pf) /* deleting f from its current pool */
    {
        p = f->pool;
        if (p && p != &_Sfpool) {
            for (k = 0; k < p->n_sf && !pf; ++k) {
                if (p->sf[k] != f) { /* a stream != f represents the pool */
                    pf = p->sf[k];
                }
            }
        }
        if (!pf) /* already isolated */
        {
            rv = f; /* just return self */
            goto done;
        }

        if (_sfpmove(f, -1) < 0 || _sfsetpool(f) < 0) goto done; /* can't delete */

        if (!pf->pool || pf->pool == &_Sfpool || pf->pool->n_sf <= 0) {
            rv = pf;
        } else {
            rv = pf->pool->sf[0]; /* return head of old pool */
        }
        goto done;
    }

    if (pf->pool && pf->pool != &_Sfpool) { /* always use current mode */
        mode = pf->pool->mode;
    }

    if (mode & SF_SHARE) /* can only have write streams */
    {
        if (SFMODE(f, 1) != SF_WRITE &&  //!OCLINT(constant conditional operator)
            _sfmode(f, SF_WRITE, 1) < 0) {
            goto done;
        }
        if (SFMODE(pf, 1) != SF_WRITE &&  //!OCLINT(constant conditional operator)
            _sfmode(pf, SF_WRITE, 1) < 0) {
            goto done;
        }
        if (f->next > f->data && SFSYNC(f) < 0) {  // start f clean
            goto done;
        }
    }

    if (_sfpmove(f, -1) < 0) { /* isolate f from current pool */
        goto done;
    }

    if (!(p = pf->pool) || p == &_Sfpool) /* making a new pool */
    {
        if (!(p = newpool(mode))) goto done;
        if (_sfpmove(pf, -1) < 0) { /* isolate pf from its current pool */
            goto done;
        }
        pf->pool = p;
        p->sf[0] = pf;
        p->n_sf += 1;
    }

    f->pool = p; /* add f to pf's pool */
    if (_sfsetpool(f) < 0) goto done;

    assert(p->sf[0] == pf && p->sf[p->n_sf - 1] == f);
    SFOPEN(pf)
    SFOPEN(f)
    if (_sfpmove(f, 0) < 0) { /* make f head of pool */
        goto done;
    }
    rv = pf;

done:
    if (f) {
        SFOPEN(f)
        SFMTXUNLOCK(f)
    }
    if (pf) {
        SFOPEN(pf)
        SFMTXUNLOCK(pf)
    }
    return rv;
}
