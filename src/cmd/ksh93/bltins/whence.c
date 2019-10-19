/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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

#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "optget_long.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "shtable.h"
#include "stk.h"

static const char *short_options = "+:afpqtvP";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// The `whence` command.
//
int b_whence(int argc, char *argv[], Shbltin_t *context) {
    int flags = 0, opt;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 't': {
                flags |= WHENCE_T_FLAG;
                break;
            }
            case 'a': {
                flags |= WHENCE_A_FLAG;
                flags |= WHENCE_V_FLAG;
                break;
            }
            case 'v': {
                flags |= WHENCE_V_FLAG;
                break;
            }
            case 'f': {
                flags |= WHENCE_F_FLAG;
                break;
            }
            case 'P':
            case 'p': {
                flags |= WHENCE_P_FLAG;
                flags &= ~WHENCE_V_FLAG;
                break;
            }
            case 'q': {
                flags |= WHENCE_Q_FLAG;
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

    if (!*argv) {
        builtin_usage_error(shp, cmd, "expected at least one arg");
        return 2;
    }
    if (flags & WHENCE_T_FLAG) flags &= ~WHENCE_V_FLAG;
    return whence(shp, argv, flags);
}

int whence(Shell_t *shp, char **argv, int flags) {
    const char *name;
    Namval_t *np;
    const char *cp;
    int aflag;
    const char *msg;
    Dt_t *root;
    Namval_t *nq;
    char *notused;
    Pathcomp_t *pp = NULL;
    bool notrack = true;
    int rv = 0;

    if (flags & WHENCE_Q_FLAG) flags &= ~WHENCE_A_FLAG;
    while ((name = *argv++)) {
        char *sp = NULL;

        aflag = ((flags & WHENCE_A_FLAG) != 0);
        cp = NULL;
        np = NULL;
        if (flags & WHENCE_P_FLAG) goto search;
        if (flags & WHENCE_Q_FLAG) goto bltins;
        // Reserved words first.
        if (sh_lookup(name, shtab_reserved)) {
            if (flags & WHENCE_T_FLAG) {
                sfprintf(sfstdout, "%s\n", "keyword");
            } else {
                sfprintf(sfstdout, "%s%s\n", name,
                         (flags & WHENCE_V_FLAG) ? sh_translate(is_reserved) : "");
            }
            if (!aflag) continue;
            aflag++;
        }
        // Non-tracked aliases.
        if ((np = nv_search(name, shp->alias_tree, 0)) && !nv_isnull(np) &&
            !(notrack = nv_isattr(np, NV_TAGGED) == NV_TAGGED) && (cp = nv_getval(np))) {
            if (flags & WHENCE_V_FLAG) {
                if (nv_isattr(np, NV_EXPORT)) {
                    msg = sh_translate(is_xalias);
                } else {
                    msg = sh_translate(is_alias);
                }
                sfprintf(sfstdout, msg, name);
            }
            if (flags & WHENCE_T_FLAG) {
                sfputr(sfstdout, "alias", '\n');
            } else {
                sfputr(sfstdout, sh_fmtq(cp), '\n');
            }
            if (!aflag) continue;
            cp = NULL;
            aflag++;
        }

        // Built-ins and functions next.
    bltins:
        root = (flags & WHENCE_F_FLAG) ? shp->bltin_tree : shp->fun_tree;
        np = nv_bfsearch(name, root, &nq, &notused);
        if (np) {
            if (is_abuiltin(np) && nv_isnull(np)) goto search;
            cp = "";
            if (flags & (WHENCE_V_FLAG | WHENCE_T_FLAG)) {
                if (nv_isnull(np)) {
                    cp = is_ufunction;
                } else if (is_abuiltin(np)) {
                    if (nv_isattr(np, BLT_SPC)) {
                        cp = is_spcbuiltin;
                    } else {
                        cp = is_builtin;
                    }
                } else {
                    cp = is_function;
                }
            }
            if (flags & WHENCE_Q_FLAG) continue;
            if (flags & WHENCE_T_FLAG) {
                if (cp == is_function || cp == is_ufunction) {
                    cp = "function";
                } else if (*name == '/') {
                    cp = "file";
                } else {
                    cp = "builtin";
                }
                sfputr(sfstdout, cp, '\n');
            } else {
                sfprintf(sfstdout, "%s%s\n", name, sh_translate(cp));
            }
            if (!aflag) continue;
            cp = NULL;
            aflag++;
        }

    search:
        if (sh_isstate(shp, SH_DEFPATH)) {
            cp = NULL;
            notrack = true;
        }
        do {
            if (path_search(shp, name, &pp, 2 + (aflag > 1))) {
                cp = name;
                if ((flags & WHENCE_P_FLAG) && *cp != '/') cp = NULL;
            } else {
                cp = stkptr(stkstd, PATH_OFFSET);
                if (*cp == 0) {
                    cp = NULL;
                } else if (*cp != '/') {
                    sp = path_fullname(shp, cp);
                    cp = sp;
                }
            }
            if (flags & WHENCE_Q_FLAG) {
                pp = NULL;
                rv = cp == NULL;
            } else if (cp) {
                if (flags & WHENCE_V_FLAG) {
                    if (*cp != '/') {
                        if (!np && (np = nv_search(name, shp->track_tree, 0))) {
                            const char *command_path = FETCH_VT(np->nvalue, pathcomp)->name;
                            sfprintf(sfstdout, "%s %s %s/%s\n", name, sh_translate(is_talias),
                                     command_path, cp);
                        } else if (!np || nv_isnull(np)) {
                            sfprintf(sfstdout, "%s%s\n", name, sh_translate(is_ufunction));
                        }
                        continue;
                    }
                    sfputr(sfstdout, sh_fmtq(name), ' ');
                    // Built-in version of program.
                    if (*cp == '/' && (np = nv_search(cp, shp->bltin_tree, 0))) {
                        msg = sh_translate(is_builtver);
                    } else if (aflag > 1 || !notrack || strchr(name, '/')) {
                        // Tracked aliases next.
                        msg = sh_translate("is");
                    } else {
                        msg = sh_translate(is_talias);
                    }
                    sfputr(sfstdout, msg, ' ');
                }
                if (flags & WHENCE_T_FLAG) {
                    sfputr(sfstdout, "file", '\n');
                } else {
                    sfputr(sfstdout, sh_fmtq(cp), '\n');
                }
                if (aflag) {
                    if (aflag <= 1) aflag++;
                    if (pp) pp = pp->next;
                } else {
                    pp = NULL;
                }
            } else if (aflag <= 1) {
                rv = 1;
                if (flags & WHENCE_V_FLAG) errormsg(SH_DICT, ERROR_exit(0), e_found, sh_fmtq(name));
            }

            if (sp) {
                free(sp);
                sp = NULL;
            }
        } while (pp);
    }
    return rv;
}
