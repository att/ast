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
// File name expansion
//
// David Korn
// AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "ast_glob.h"
#include "cdt.h"
#include "defs.h"
#include "name.h"
#include "path.h"
#include "sfio.h"
#include "stk.h"
#include "variables.h"

#define argbegin argnxt.cp
static const char *sufstr;
static int suflen;
static_fn int scantree(Shell_t *, Dt_t *, const char *, struct argnod **);

//
// This routine builds a list of files that match a given pathname.
// Uses external routine strgrpmatch() to match each component.
// A leading . must match explicitly.
//
#ifndef GLOB_AUGMENTED
#define GLOB_AUGMENTED 0
#endif

static_fn char *nextdir(glob_t *gp, char *dir) {
    Shell_t *shp = sh_getinterp();
    Pathcomp_t *pp = gp->gl_handle;
    if (!dir) {
        pp = path_get(shp, "");
    } else {
        pp = pp->next;
    }
    gp->gl_handle = pp;
    if (pp) return pp->name;
    return NULL;
}

int path_expand(Shell_t *shp, const char *pattern, struct argnod **arghead) {
    glob_t gdata;
    struct argnod *ap;
    glob_t *gp = &gdata;
    int flags, extra = 0;
#if SHOPT_BASH
    int off;
    char *sp, *cp, *cp2;
#endif

    sh_stats(STAT_GLOBS);
    memset(gp, 0, sizeof(gdata));
    flags = GLOB_GROUP | GLOB_AUGMENTED | GLOB_NOCHECK | GLOB_NOSORT | GLOB_STACK | GLOB_LIST |
            GLOB_DISC;
    if (sh_isoption(shp, SH_MARKDIRS)) flags |= GLOB_MARK;
    if (sh_isoption(shp, SH_GLOBSTARS)) flags |= GLOB_STARSTAR;
#if SHOPT_BASH
    if (sh_isoption(shp, SH_NULLGLOB)) flags &= ~GLOB_NOCHECK;
    if (sh_isoption(shp, SH_NOCASEGLOB)) flags |= GLOB_ICASE;
#endif
    if (sh_isstate(shp, SH_COMPLETE)) {
        extra += scantree(shp, shp->alias_tree, pattern, arghead);
        extra += scantree(shp, shp->fun_tree, pattern, arghead);
        gp->gl_nextdir = nextdir;
        flags |= GLOB_COMPLETE;
        flags &= ~GLOB_NOCHECK;
    }
#if SHOPT_BASH
    off = stktell(shp->stk);
    if (off) sp = stkfreeze(shp->stk, 0);
    if (sh_isoption(shp, SH_BASH)) {
        // For bash, FIGNORE is a colon separated list of suffixes to ignore
        // when doing filename/command completion. GLOBIGNORE is similar to ksh
        // FIGNORE, but colon separated instead of being an augmented shell
        // pattern. Generate shell patterns out of those here.
        if (sh_isstate(shp, SH_FCOMPLETE)) {
            cp = nv_getval(sh_scoped(shp, VAR_FIGNORE));
        } else {
            static Namval_t *GLOBIGNORENOD;
            if (!GLOBIGNORENOD) GLOBIGNORENOD = nv_open("GLOBIGNORE", shp->var_tree, 0);
            cp = nv_getval(sh_scoped(shp, GLOBIGNORENOD));
        }
        if (cp) {
            flags |= GLOB_AUGMENTED;
            sfputr(shp->stk, "@(", -1);
            if (!sh_isstate(shp, SH_FCOMPLETE)) {
                sfputr(shp->stk, cp, -1);
                for (cp = stkptr(shp->stk, off); *cp; cp++) {
                    if (*cp == ':') *cp = '|';
                }
            } else {
                cp2 = strtok(cp, ":");
                if (!cp2) cp2 = cp;
                do {
                    sfputc(shp->stk, '*');
                    sfputr(shp->stk, cp2, -1);
                    cp2 = strtok(NULL, ":");
                    if (cp2) {
                        *(cp2 - 1) = ':';
                        sfputc(shp->stk, '|');
                    }
                } while (cp2);
            }
            sfputc(shp->stk, ')');
            gp->gl_fignore = stkfreeze(shp->stk, 1);
        } else if (!sh_isstate(shp, SH_FCOMPLETE) && sh_isoption(shp, SH_DOTGLOB)) {
            gp->gl_fignore = "";
        }
    } else
#endif
        gp->gl_fignore = nv_getval(sh_scoped(shp, VAR_FIGNORE));
    if (suflen) gp->gl_suffix = sufstr;
    gp->gl_intr = &shp->trapnote;
    suflen = 0;
    if (strncmp(pattern, "~(N", 3) == 0) flags &= ~GLOB_NOCHECK;
    ast_glob(pattern, flags, 0, gp);
#if SHOPT_BASH
    if (off) {
        stkset(shp->stk, sp, off);
    } else {
        stkseek(shp->stk, 0);
    }
#endif
    sh_sigcheck(shp);
    for (ap = (struct argnod *)gp->gl_list; ap; ap = ap->argnxt.ap) {
        ap->argchn.ap = ap->argnxt.ap;
        if (!ap->argnxt.ap) ap->argchn.ap = *arghead;
    }
    if (gp->gl_list) *arghead = (struct argnod *)gp->gl_list;
    return gp->gl_pathc + extra;
}

//
// Scan tree and add each name that matches the given pattern.
//
static_fn int scantree(Shell_t *shp, Dt_t *tree, const char *pattern, struct argnod **arghead) {
    Namval_t *np;
    struct argnod *ap;
    int nmatch = 0;
    char *cp;

    np = (Namval_t *)dtfirst(tree);
    for (; np && !nv_isnull(np); (np = (Namval_t *)dtnext(tree, np))) {
        cp = nv_name(np);
        if (strmatch(cp, pattern)) {
            stkseek(shp->stk, ARGVAL);
            sfputr(shp->stk, cp, -1);
            ap = (struct argnod *)stkfreeze(shp->stk, 1);
            ap->argbegin = NULL;
            ap->argchn.ap = *arghead;
            ap->argflag = ARG_RAW | ARG_MAKE;
            *arghead = ap;
            nmatch++;
        }
    }
    return nmatch;
}

//
// File name completion.
// Generate the list of files found by adding an suffix to end of name.
// The number of matches is returned.
//
int path_complete(Shell_t *shp, const char *name, const char *suffix, struct argnod **arghead) {
    sufstr = suffix;
    suflen = (int)strlen(suffix);
    return path_expand(shp, name, arghead);
}

static_fn int checkfmt(Sfio_t *sp, void *vp, Sffmt_t *fp) {
    UNUSED(sp);
    UNUSED(vp);
    UNUSED(fp);

    return -1;
}

int path_generate(Shell_t *shp, struct argnod *todo, struct argnod **arghead) {
    char *cp;
    int brace;
    struct argnod *ap;
    struct argnod *top = NULL;
    struct argnod *apin;
    char *pat, *rescan;
    char *format = NULL;
    char comma, range = 0;
    int first, last, incr, count = 0;
    char end_char;
    char tmp[32];

    if (!sh_isoption(shp, SH_BRACEEXPAND)) return path_expand(shp, todo->argval, arghead);
    todo->argchn.ap = NULL;
again:
    apin = ap = todo;
    todo = ap->argchn.ap;
    cp = ap->argval;
    range = comma = brace = 0;
    // First search for {...,...}.
    while (1) {
        switch (*cp++) {
            case '{': {
                if (brace++ == 0) pat = cp;
                break;
            }
            case '}': {
                if (--brace > 0) break;
                if (brace == 0 && comma && *cp != '(') goto endloop1;
                comma = brace = 0;
                break;
            }
            case '.': {
                if (brace != 1) break;
                if (*cp != '.') break;

                char *endc;
                incr = 1;
                if (isdigit(*pat) || *pat == '+' || *pat == '-') {
                    first = strtol(pat, &endc, 0);
                    if (endc == (cp - 1)) {
                        last = strtol(cp + 1, &endc, 0);
                        if (*endc == '.' && endc[1] == '.') {
                            incr = strtol(endc + 2, &endc, 0);
                        } else if (last < first) {
                            incr = -1;
                        }
                        if (incr) {
                            if (*endc == '%') {
                                Sffmt_t fmt;
                                memset(&fmt, 0, sizeof(fmt));
                                fmt.version = SFIO_VERSION;
                                fmt.form = endc;
                                fmt.extf = checkfmt;
                                sfprintf(sfstdout, "%!", &fmt);
                                if (!(fmt.flags & (SFFMT_LLONG | SFFMT_LDOUBLE))) {
                                    switch (fmt.fmt) {
                                        case 'c':
                                        case 'd':
                                        case 'i':
                                        case 'o':
                                        case 'u':
                                        case 'x':
                                        case 'X': {
                                            format = endc;
                                            endc = (char *)fmt.form;  // discard const qualifier
                                            break;
                                        }
                                        default: { break; }
                                    }
                                }
                            } else {
                                format = "%d";
                            }
                            if (*endc == '}') {
                                cp = endc + 1;
                                range = 2;
                                goto endloop1;
                            }
                        }
                    }
                } else if ((cp[2] == '}' || (cp[2] == '.' && cp[3] == '.')) &&
                           ((*pat >= 'a' && *pat <= 'z' && cp[1] >= 'a' && cp[1] <= 'z') ||
                            (*pat >= 'A' && *pat <= 'Z' && cp[1] >= 'A' && cp[1] <= 'Z'))) {
                    first = *pat;
                    last = cp[1];
                    cp += 2;
                    if (*cp == '.') {
                        incr = strtol(cp + 2, &endc, 0);
                        cp = endc;
                    } else if (first > last) {
                        incr = -1;
                    }
                    if (incr && *cp == '}') {
                        cp++;
                        range = 1;
                        goto endloop1;
                    }
                }
                cp++;
                break;
            }
            case ',': {
                if (brace == 1) comma = 1;
                break;
            }
            case '\\': {
                cp++;
                break;
            }
            case 0: {  // insert on stack
                ap->argchn.ap = top;
                top = ap;
                if (todo) goto again;
                for (; ap; ap = apin) {
                    apin = ap->argchn.ap;
                    if (!sh_isoption(shp, SH_NOGLOB)) {
                        brace = path_expand(shp, ap->argval, arghead);
                    } else {
                        ap->argchn.ap = *arghead;
                        *arghead = ap;
                        brace = 1;
                    }
                    if (brace) {
                        count += brace;
                        (*arghead)->argflag |= ARG_MAKE;
                    }
                }
                return count;
            }
            default: { break; }
        }
    }

endloop1:
    rescan = cp;
    cp = pat - 1;
    *cp = 0;

    while (1) {
        brace = 0;
        if (range) {
            if (range == 1) {
                pat[0] = first;
                cp = &pat[1];
            } else {
                *(rescan - 1) = 0;
                pat = tmp;
                assert(format);
                sfsprintf(pat, sizeof(tmp), format, first);
                *(rescan - 1) = '}';
                cp = &end_char;
                *cp = 0;
            }
            if (incr * (first + incr) > last * incr) {
                *cp = '}';
            } else {
                first += incr;
            }
        } else {  // generate each pattern and put on the todo list
            while (1) {
                switch (*++cp) {
                    case '\\': {
                        cp++;
                        break;
                    }
                    case '{': {
                        brace++;
                        break;
                    }
                    case ',': {
                        if (brace == 0) goto endloop2;
                        break;
                    }
                    case '}': {
                        if (--brace < 0) goto endloop2;
                    }
                    default: { break; }
                }
            }
        }

    endloop2:
        brace = *cp;
        *cp = 0;
        sh_sigcheck(shp);
        ap = stkseek(shp->stk, ARGVAL);
        ap->argflag = ARG_RAW;
        ap->argchn.ap = todo;
        sfputr(shp->stk, apin->argval, -1);
        sfputr(shp->stk, pat, -1);
        sfputr(shp->stk, rescan, -1);
        todo = ap = (struct argnod *)stkfreeze(shp->stk, 1);
        if (brace == '}') break;
        if (!range) pat = cp + 1;
    }
    goto again;
}
