/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ast_assert.h"
#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "optget_long.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

static const char *short_options = "ip";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"ignorecase", optget_no_arg, NULL, 'i'},
    {NULL, 0, NULL, 0}};

struct Enum {
    Namfun_t namfun;
    char node[NV_MINSZ + sizeof(char *)];
    int64_t nelem;
    bool iflag;
    const char **values;
};

static_fn int enuminfo(Opt_t *op, Sfio_t *out, const char *str, Optdisc_t *fp) {
    UNUSED(op);
    Namval_t *np;
    struct Enum *ep;
    const char *v;
    np = *(Namval_t **)(fp + 1);
    ep = (struct Enum *)np->nvfun;
    if (strcmp(str, "default") == 0) {
        assert(ep->nelem > 0);
        sfprintf(out, "\b%s\b", ep->values[0]);
    } else if (strcmp(str, "case") == 0) {
        if (ep->iflag) sfprintf(out, "not ");
    } else {
        for (int i = 0; i < ep->nelem; i++) {
            v = ep->values[i];
            sfprintf(out, ", \b%s\b", v);
        }
    }

    return 0;
}

static_fn void free_enum(struct Enum *ep) {
    for (int i = 0; i < ep->nelem; i++) {
        free((char *)ep->values[i]);
    }
    free(ep->values);
    free(ep);
}

static_fn Namfun_t *clone_enum(Namval_t *np, Namval_t *mp, nvflag_t flags, Namfun_t *fp) {
    UNUSED(np);
    UNUSED(mp);
    UNUSED(flags);

    struct Enum *pp = (struct Enum *)fp;
    struct Enum *ep = malloc(sizeof(struct Enum));
    memcpy(ep, pp, sizeof(struct Enum));
    ep->values = malloc(ep->nelem * sizeof(*ep->values));
    for (int i = 0; i < pp->nelem; i++) {
        ep->values[i] = strdup(pp->values[i]);
    }
    return &ep->namfun;
}

static_fn void put_enum(Namval_t *np, const void *val, nvflag_t flags, Namfun_t *fp) {
    struct Enum *ep = (struct Enum *)fp;
    const char *v;
    int n;
    if (!val && !(flags & NV_INTEGER)) {
        nv_putv(np, val, flags, fp);
        nv_disc(np, &ep->namfun, DISC_OP_POP);
        if (!ep->namfun.nofree) free_enum(ep);
        return;
    }
    if (flags & NV_INTEGER) {
        nv_putv(np, val, flags, fp);
        return;
    }

    for (int i = 0; i < ep->nelem; i++) {
        v = ep->values[i];
        if (ep->iflag) {
            n = strcasecmp(v, val);
        } else {
            n = strcmp(v, val);
        }

        if (n == 0) {
            // TODO: Figure out if a static var is correct. The code used to store a pointer to the
            // stack local var `i` which is obviously wrong and only works by accident if the
            // pointer is used before the stack location is overwritten.
            static uint16_t x;
            x = i;
            nv_putv(np, (char *)&x, NV_UINT16, fp);
            return;
        }
    }
    if (nv_isattr(np, NV_NOFREE)) error(ERROR_exit(1), "%s:  invalid value %s", nv_name(np), val);
}

static_fn char *get_enum(Namval_t *np, Namfun_t *fp) {
    if (nv_isattr(np, NV_NOTSET) == NV_NOTSET) return "";

    struct Enum *ep = (struct Enum *)fp;
    long n = nv_getn(np, fp);
    assert(n >= 0);
    if (n < ep->nelem) return (char *)ep->values[n];

    static char buff[6];
    sfsprintf(buff, sizeof(buff), "%u%c", n, 0);
    return buff;
}

static_fn Sfdouble_t get_nenum(Namval_t *np, Namfun_t *fp) { return nv_getn(np, fp); }

//
// This returns the index of an enum symbol when something like the following is executed. In this
// case "2" would be echoed:
//
//     enum Color_t=(red green blue orange yellow)
//     Color_t x
//     echo ${x.blue}
//
static_fn Namval_t *create_enum(Namval_t *np, const void *vp, nvflag_t flags, Namfun_t *fp) {
    UNUSED(flags);
    const char *name = vp;
    struct Enum *ep = (struct Enum *)fp;
    Namval_t *mp = nv_namptr(ep->node, 0);

    mp->nvenv = np;
    for (int i = 0; i < ep->nelem; i++) {
        const char *v = ep->values[i];
        int n;
        if (ep->iflag) {
            n = strcasecmp(v, name);
        } else {
            n = strcmp(v, name);
        }

        if (n == 0) {
            STORE_VT(mp->nvalue, i16, i);
            mp->nvname = (char *)v;
            fp->last = (char *)(name + strlen(name));
            return mp;
        }
    }

    error(ERROR_exit(1), "%s: invalid enum constant for %s", name, nv_name(np));
    __builtin_unreachable();
}

const Namdisc_t ENUM_disc = {.dsize = 0,
                             .putval = put_enum,
                             .getval = get_enum,
                             .getnum = get_nenum,
                             .createf = create_enum,
                             .clonef = clone_enum};

static_fn int sh_outenum(Shell_t *shp, Sfio_t *iop, Namval_t *tp) {
    Namval_t *mp;
    Dt_t *dp = NULL;
    struct Enum *ep;
    int i;

    if (!tp) {
        mp = nv_open(NV_CLASS, shp->var_tree, NV_NOADD | NV_VARNAME);
        if (!mp) return 0;
        dp = nv_dict(mp);
        tp = (Namval_t *)dtfirst(dp);
    }
    while (tp) {
        if (!tp->nvfun) continue;
        ep = (struct Enum *)nv_hasdisc(tp, &ENUM_disc);
        if (!ep) continue;

        sfprintf(iop, "enum %s%s=(\n", (ep->iflag ? "-i " : ""), tp->nvname);
        for (i = 0; i < ep->nelem; i++) sfprintf(iop, "\t%s\n", ep->values[i]);
        sfprintf(iop, ")\n");
        if (!dp) break;
        tp = (Namval_t *)dtnext(dp, tp);
    }
    return 0;
}

int b_enum(int argc, char **argv, Shbltin_t *context) {
    bool pflag = false, iflag = false;
    int i, n, opt;
    ssize_t sz = -1;
    Namval_t *np, *tp, *mp;
    Namarr_t *ap;
    char *cp;
    const char *sp;
    struct Enum *ep;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    struct {
        Optdisc_t opt;
        Namval_t *np;
    } optdisc;

    if (cmdinit(argc, argv, context, ERROR_NOTIFY)) return -1;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'p': {
                pflag = true;
                break;
            }
            case 'i': {
                iflag = true;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }

    argv += optget_ind;
    argc -= optget_ind;
    if (argc != 1) {
        builtin_usage_error(shp, cmd, "expected one argument, got %d", argc);
        return 1;
    }

    while ((cp = *argv++)) {
        np = nv_open(cp, shp->var_tree, NV_VARNAME | NV_NOADD);
        if (!np || !(ap = nv_arrayptr(np)) || ap->fun || ap->nelem < 2) {
            error(ERROR_exit(1), "%s must name an array containing at least two elements", cp);
        }
        n = stktell(shp->stk);
        sfprintf(shp->stk, "%s.%s%c", NV_CLASS, np->nvname, 0);
        tp = nv_open(stkptr(shp->stk, n), shp->var_tree, NV_VARNAME);
        if (pflag) {
            sh_outenum(shp, sfstdout, tp);
            continue;
        }
        stkseek(shp->stk, n);
        n = ap->nelem;
        i = 0;
        nv_onattr(tp, NV_UINT16);
        nv_putval(tp, (char *)&i, NV_INTEGER);
        nv_putsub(np, NULL, 0L, ARRAY_SCAN);
        do {
            sz += strlen(nv_getval(np));
        } while (nv_nextsub(np));
        ep = calloc(1, sizeof(struct Enum));
        ep->nelem = n;
        mp = nv_namptr(ep->node, 0);
        mp->nvshell = shp;
        nv_setsize(mp, 10);
        nv_onattr(mp, NV_UINT16);
        ep->iflag = iflag;

        ep->values = malloc(n * sizeof(*ep->values));
        nv_putsub(np, NULL, 0L, ARRAY_SCAN);
        i = 0;
        do {
            sp = nv_getval(np);
            ep->values[i++] = strdup(sp);
        } while (nv_nextsub(np));
        assert(n == i);

        ep->namfun.dsize = sizeof(struct Enum);
        ep->namfun.disc = &ENUM_disc;
        ep->namfun.type = tp;
        nv_onattr(tp, NV_RDONLY);
        nv_disc(tp, &ep->namfun, DISC_OP_FIRST);
        memset(&optdisc, 0, sizeof(optdisc));
        optdisc.opt.infof = enuminfo;
        optdisc.np = tp;
        nv_addtype(tp, sh_optenum_type, &optdisc, sizeof(optdisc));
        nv_onattr(np, NV_LTOU | NV_UTOL);
    }
    nv_open(0, shp->var_tree, 0);
    return error_info.errors != 0;
}
