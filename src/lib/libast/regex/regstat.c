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
 * return p stat info
 */
#include "config_ast.h"  // IWYU pragma: keep

#include "ast_regex.h"
#include "reglib.h"

regstat_t *regstat(const regex_t *p) {
    Rex_t *e;

    p->re_info->stats.re_flags = p->re_info->flags;
    p->re_info->stats.re_info = 0;
    e = p->re_info->rex;
    if (e && e->type == REX_BM) {
        p->re_info->stats.re_record = p->re_info->rex->re.bm.size;
        e = e->next;
    } else {
        p->re_info->stats.re_record = 0;
    }
    if (e && e->type == REX_BEG) e = e->next;
    if (e && e->type == REX_STRING) e = e->next;
    if (!e || (e->type == REX_END && !e->next)) p->re_info->stats.re_info |= REG_LITERAL;
    p->re_info->stats.re_record =
        (p->re_info->rex && p->re_info->rex->type == REX_BM) ? p->re_info->rex->re.bm.size : -1;
    return &p->re_info->stats;
}
