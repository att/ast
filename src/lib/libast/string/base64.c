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
/*
 * mime base64 encode/decode
 *
 * Glenn Fowler
 * David Korn
 * AT&T Research
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include "stdlib.h"

#include "ast.h"         // IWYU pragma: keep
#include "ast_assert.h"  // IWYU pragma: keep

#define PAD '='

// #define B64_UC 3
#define B64_EC 4
#define B64_CHUNK 15
#define B64_PAD 64
#define B64_SPC 65
#define B64_IGN 66

static unsigned char map[UCHAR_MAX + 1];

static const unsigned char alp[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//
// Mime base64 encode.
//
ssize_t base64encode(const void *fb, size_t fz, void *tb, size_t tz) {
    const unsigned char *fp;
    const unsigned char *fe;
    unsigned char *tp;
    unsigned char *te;
    unsigned char *tc;
    unsigned long b;
    unsigned char tmp[B64_EC * B64_CHUNK];
    const unsigned char *m = alp;

    fp = fe = fb;
    if (fz >= 3) {
        int n = fz % 3;
        fe += fz - n;
        fz = n;
    }
    tp = tb;
    te = tp + tz - B64_EC + 1;

    for (;;) {
        tc = tp + B64_EC * B64_CHUNK;
        do {
            if (fp >= fe) goto done;
            if (tp >= te) {
                tp = tmp;
                te = tp + sizeof(tmp) - B64_EC + 1;
            }
            b = *fp++ << 16;
            b |= *fp++ << 8;
            b |= *fp++;
            *tp++ = m[b >> 18];
            *tp++ = m[(b >> 12) & 077];
            *tp++ = m[(b >> 6) & 077];
            *tp++ = m[b & 077];
        } while (tp < tc);
        *tp++ = '\n';
    }
done:
    if (fz) {
        if (tp >= te) {
            tp = tmp;
            te = tp + sizeof(tmp) - B64_EC + 1;
        }
        b = *fp++ << 16;
        if (fz == 2) b |= *fp++ << 8;
        *tp++ = m[b >> 18];
        *tp++ = m[(b >> 12) & 077];
        *tp++ = (fz == 2) ? m[(b >> 6) & 077] : PAD;
        *tp++ = PAD;
    }
    if (tp > (unsigned char *)tb && *(tp - 1) == '\n') tp--;
    if (tp < te) *tp = 0;
    return tp - (unsigned char *)tb;
}

//
// Mime base64 decode.
//
ssize_t base64decode(const void *fb, size_t fz, void *tb, size_t tz) {
    unsigned char *fp;
    unsigned char *tp;
    unsigned char *fe;
    unsigned char *te;
    unsigned char *tx;
    unsigned char *m;
    int c;
    int state;
    unsigned long v;
    ssize_t n;

    m = map;
    if (!map[0]) {
        memset(m, B64_IGN, sizeof(map));
        for (const unsigned char *ap = alp; *ap; ++ap) m[*ap] = ap - alp;
        m[PAD] = B64_PAD;
        m[' '] = m['\t'] = m['\n'] = B64_SPC;
    }
    fp = (unsigned char *)fb;
    fe = fp + fz;
    tp = (unsigned char *)tb;
    te = tp + tz;
    if (tz > 2) tz = 2;
    tx = te - tz;
    n = 0;

    for (;;) {
        state = 0;
        v = 0;
        while (fp < fe) {
            if ((c = m[*fp++]) < 64) {
                v = (v << 6) | c;
                if (++state == 4) {
                    if (tp >= tx) {
                        if (n) {
                            n += 3;
                        } else {
                            n = tp - (unsigned char *)tb + 4;
                            if (tp < te) {
                                *tp++ = (v >> 16);
                                if (tp < te) {
                                    *tp++ = (v >> 8);
                                    if (tp < te) *tp++ = (v);
                                }
                            }
                        }
                    } else {
                        *tp++ = (v >> 16);
                        *tp++ = (v >> 8);
                        *tp++ = (v);
                    }
                    state = 0;
                    v = 0;
                }
            } else if (c == B64_PAD) {
                break;
            }
        }
        switch (state) {
            case 0:
                goto done;
            case 1:
                break;
            case 2:
                if (tp < te) {
                    *tp++ = v >> 4;
                } else if (n) {
                    n++;
                } else {
                    n = tp - (unsigned char *)tb + 2;
                }
                break;
            case 3:
                if (tp < te) {
                    *tp++ = v >> 10;
                    if (tp < te) {
                        *tp++ = v >> 2;
                    } else {
                        n = tp - (unsigned char *)tb + 2;
                    }
                } else if (n) {
                    n += 2;
                } else {
                    n = tp - (unsigned char *)tb + 3;
                }
                break;
            default:
                abort();
        }
        while (fp < fe && ((c = m[*fp++]) == B64_PAD || c == B64_SPC)) {
            ;  // empty body
        }
        if (fp >= fe || c >= 64) break;
        fp--;
    }
done:
    if (n) {
        n--;
    } else {
        if (tp < te) *tp = 0;
        n = tp - (unsigned char *)tb;
    }
    return n;
}
