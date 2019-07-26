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
/*
 * posix regex record executor
 * multiple record sized-buffer interface
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>

#include "ast.h"  // IWYU pragma: keep
#include "ast_regex.h"
#include "reglib.h"

/*
 * call regnexec() on records selected by Boyer-Moore
 */

int regrexec(const regex_t *p, const char *s, size_t len, size_t nmatch, regmatch_t *match,
             regflags_t flags, int sep, void *handle, regrecord_t record) {
    unsigned char *buf;
    unsigned char *beg;
    unsigned char *l;
    unsigned char *r;
    unsigned char *x;
    size_t *skip;
    size_t *fail;
    Bm_mask_t **mask;
    size_t index;
    ssize_t n;
    unsigned char *end;
    size_t mid;
    int complete;
    int exactlen;
    int leftlen;
    int rightlen;
    int inv;
    Bm_mask_t m;
    Env_t *env;
    Rex_t *e;

    if (!s || !p || !(env = p->re_info) || (e = env->rex)->type != REX_BM) return REG_BADPAT;
    inv = (flags & REG_INVERT) != 0;
    buf = beg = (unsigned char *)s;
    end = buf + len;
    mid = (len < e->re.bm.right) ? 0 : (len - e->re.bm.right);
    skip = e->re.bm.skip;
    fail = e->re.bm.fail;
    mask = e->re.bm.mask;
    complete = e->re.bm.complete && !nmatch;
    exactlen = e->re.bm.size;
    leftlen = e->re.bm.left + exactlen;
    rightlen = exactlen + e->re.bm.right;
    index = leftlen++;
    for (;;) {
        while (index < mid) index += skip[buf[index]];
        if (index < HIT) goto impossible;
        index -= HIT;
        m = mask[n = exactlen - 1][buf[index]];
        do {
            if (!n--) goto possible;
        } while (m &= mask[n][buf[--index]]);
        if ((index += fail[n + 1]) < len) continue;
    impossible:
        if (inv) {
            l = r = buf + len;
            goto invert;
        }
        n = 0;
        goto done;
    possible:
        r = (l = buf + index) + exactlen;
        while (l > beg) {
            if (*--l == sep) {
                l++;
                break;
            }
        }
        if ((r - l) < leftlen) goto spanned;
        while (r < end && *r != sep) r++;
        if ((r - (buf + index)) < rightlen) goto spanned;
        if (complete || ((env->rex = ((r - l) > 128) ? e : e->next) &&
                         !(n = regnexec(p, (char *)l, r - l, nmatch, match, flags)))) {
            if (inv) {
            invert:
                x = beg;
                while (beg < l) {
                    while (x < l && *x != sep) x++;
                    n = (*record)(handle, (char *)beg, x - beg);
                    if (n) goto done;
                    beg = ++x;
                }
            } else {
                n = (*record)(handle, (char *)l, r - l);
                if (n) goto done;
            }
            index = (r - buf) + leftlen;
            if (index >= len) {
                n = (inv && (++r - buf) < len) ? (*record)(handle, (char *)r, (buf + len) - r) : 0;
                goto done;
            }
            beg = r + 1;
        } else if (n != REG_NOMATCH) {
            goto done;
        } else {
        spanned:
            index += exactlen;
            if (index >= mid) goto impossible;
        }
    }
done:
    env->rex = e;
    return n;
}
