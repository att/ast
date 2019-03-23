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
//
// command [-pvVx] name [arg...]
// whence [-afvp] name...
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "shtable.h"
#include "stk.h"

#define P_FLAG (1 << 0)
#define V_FLAG (1 << 1)
#define A_FLAG (1 << 2)
#define F_FLAG (1 << 3)
#define X_FLAG (1 << 4)
#define Q_FLAG (1 << 5)
#define T_FLAG (1 << 6)

static_fn int whence(Shell_t *, char **, int);

//
// Command is called with argc==0 when checking for -V or -v option. In this case return 0 when -v
// or -V or unknown option, otherwise the shift count to the command is returned.
//
int b_command(int argc, char *argv[], Shbltin_t *context) {
    int n, flags = 0;
    Shell_t *shp = context->shp;
    Optdisc_t disc;

    memset(&disc, 0, sizeof(disc));
    disc.version = OPT_VERSION;
    opt_info.disc = &disc;
    opt_info.index = opt_info.offset = 0;

    while ((n = optget(argv, sh_optcommand))) {
        switch (n) {
            case 'p': {
                if (sh_isoption(shp, SH_RESTRICTED)) {
                    errormsg(SH_DICT, ERROR_exit(1), e_restricted, "-p");
                    __builtin_unreachable();
                }
                sh_onstate(shp, SH_DEFPATH);
                break;
            }
            case 'v': {
                flags |= X_FLAG;
                break;
            }
            case 'V': {
                flags |= V_FLAG;
                break;
            }
            case 'x': {
                shp->xargexit = 1;
                break;
            }
            case ':': {
                if (argc == 0) return 0;
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                if (argc == 0) return 0;
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            default: { break; }
        }
    }
    if (argc == 0) return flags ? 0 : opt_info.index;
    argv += opt_info.index;
    if (error_info.errors || !*argv) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    return whence(shp, argv, flags);
}

//
// For the whence command.
//
int b_whence(int argc, char *argv[], Shbltin_t *context) {
    int flags = 0, n;
    Shell_t *shp = context->shp;
    UNUSED(argc);

    if (*argv[0] == 't') flags = V_FLAG;
    while ((n = optget(argv, sh_optwhence))) {
        switch (n) {
            case 't': {
                flags |= T_FLAG;
                break;
            }
            case 'a': {
                flags |= A_FLAG;
            }
            // FALLTHRU
            case 'v': {
                flags |= V_FLAG;
                break;
            }
            case 'f': {
                flags |= F_FLAG;
                break;
            }
            case 'P':
            case 'p': {
                flags |= P_FLAG;
                flags &= ~V_FLAG;
                break;
            }
            case 'q': {
                flags |= Q_FLAG;
                break;
            }
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            default: { break; }
        }
    }
    argv += opt_info.index;
    if (error_info.errors || !*argv) {
        errormsg(SH_DICT, ERROR_usage(2), optusage(NULL));
        __builtin_unreachable();
    }
    if (flags & T_FLAG) flags &= ~V_FLAG;
    return whence(shp, argv, flags);
}

static_fn int whence(Shell_t *shp, char **argv, int flags) {
    const char *name;
    Namval_t *np;
    const char *cp;
    int aflag, r = 0;
    const char *msg;
    Dt_t *root;
    Namval_t *nq;
    char *notused;
    Pathcomp_t *pp = NULL;
    int notrack = 1;

    if (flags & Q_FLAG) flags &= ~A_FLAG;
    while ((name = *argv++)) {
        bool tofree = false;
        char *sp = NULL;

        aflag = ((flags & A_FLAG) != 0);
        cp = NULL;
        np = NULL;
        if (flags & P_FLAG) goto search;
        if (flags & Q_FLAG) goto bltins;
        // Reserved words first.
        if (sh_lookup(name, shtab_reserved)) {
            if (flags & T_FLAG) {
                sfprintf(sfstdout, "%s\n", "keyword");
            } else {
                sfprintf(sfstdout, "%s%s\n", name,
                         (flags & V_FLAG) ? sh_translate(is_reserved) : "");
            }
            if (!aflag) continue;
            aflag++;
        }
        // Non-tracked aliases.
        if ((np = nv_search(name, shp->alias_tree, 0)) && !nv_isnull(np) &&
            !(notrack = nv_isattr(np, NV_TAGGED)) && (cp = nv_getval(np))) {
            if (flags & V_FLAG) {
                if (nv_isattr(np, NV_EXPORT)) {
                    msg = sh_translate(is_xalias);
                } else {
                    msg = sh_translate(is_alias);
                }
                sfprintf(sfstdout, msg, name);
            }
            if (flags & T_FLAG) {
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
        root = (flags & F_FLAG) ? shp->bltin_tree : shp->fun_tree;
        np = nv_bfsearch(name, root, &nq, &notused);
        if (np) {
            if (is_abuiltin(np) && nv_isnull(np)) goto search;
            cp = "";
            if (flags & (V_FLAG | T_FLAG)) {
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
            if (flags & Q_FLAG) continue;
            if (flags & T_FLAG) {
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
            notrack = 1;
        }
        do {
            if (path_search(shp, name, &pp, 2 + (aflag > 1))) {
                cp = name;
                if ((flags & P_FLAG) && *cp != '/') cp = NULL;
            } else {
                cp = stkptr(stkstd, PATH_OFFSET);
                if (*cp == 0) {
                    cp = NULL;
                } else if (*cp != '/') {
                    tofree = true;
                    sp = path_fullname(shp, cp);
                    cp = sp;
                }
            }
            if (flags & Q_FLAG) {
                pp = NULL;
                r |= !cp;
            } else if (cp) {
                if (flags & V_FLAG) {
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
                if (flags & T_FLAG) {
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
                if (tofree) {
                    free(sp);
                    tofree = false;
                }
            } else if (aflag <= 1) {
                r |= 1;
                if (flags & V_FLAG) errormsg(SH_DICT, ERROR_exit(0), e_found, sh_fmtq(name));
            }
        } while (pp);
    }
    return r;
}
