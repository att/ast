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
 * file name expansion - posix.2 glob with gnu and ast extensions
 *
 *      David Korn
 *      Glenn Fowler
 *      AT&T Research
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>

#include "ast.h"
#include "ast_glob.h"
#include "ast_regex.h"
#include "sfio.h"
#include "stk.h"

#define GLOB_MAGIC 0xaaaa0000

#define MATCH_RAW 1
#define MATCH_MAKE 2
#define MATCH_META 4

#define MATCHPATH(g) (offsetof(globlist_t, gl_path) + (g)->gl_extra)

/*
 * default gl_diropen
 */

static_fn DIR *gl_diropen(glob_t *gp, const char *path) { return (*gp->gl_opendir)(path); }

/*
 * default gl_dirnext
 */

static_fn char *gl_dirnext(glob_t *gp, DIR *handle) {
    struct dirent *dp = (*gp->gl_readdir)(handle);

    if (!dp) return NULL;
#ifdef D_TYPE
    if (D_TYPE(dp) != DT_UNKNOWN && D_TYPE(dp) != DT_DIR && D_TYPE(dp) != DT_LNK) {
        gp->gl_status |= GLOB_NOTDIR;
    }
#endif
    return dp->d_name;
}

/*
 * default gl_dirclose
 */

static_fn int gl_dirclose(glob_t *gp, DIR *handle) { return (gp->gl_closedir)(handle); }

//
// Default gl_type.
//
static_fn int gl_type(glob_t *gp, const char *path, int flags) {
    struct stat st;

    memset(&st, 0, sizeof(st));
    int stat_rv = (flags & GLOB_STARSTAR) ? (*gp->gl_lstat)(path, &st) : (*gp->gl_stat)(path, &st);
    if (stat_rv == -1) return 0;

    if (S_ISDIR(st.st_mode)) {
        return GLOB_DIR;
    } else if (!S_ISREG(st.st_mode)) {
        return GLOB_DEV;
    } else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        return GLOB_EXE;
    }

    return GLOB_REG;
}

/*
 * default gl_attr
 */

static_fn int gl_attr(glob_t *gp, const char *path, int flags) {
    UNUSED(gp);
    UNUSED(flags);
    UNUSED(path);

    return 0;
}

//
// Default (glob_t*)->gl_nextdir.
//
static_fn char *gl_nextdir(glob_t *gp, char *dir) {
    if (!(dir = gp->gl_nextpath)) dir = gp->gl_nextpath = stkcopy(stkstd, pathbin());
    switch (*gp->gl_nextpath) {
        case 0:
            dir = 0;
            break;
        case ':':
            while (*gp->gl_nextpath == ':') gp->gl_nextpath++;
            dir = ".";
            break;
        default:
            while (*gp->gl_nextpath) {
                if (*gp->gl_nextpath++ == ':') {
                    *(gp->gl_nextpath - 1) = 0;
                    break;
                }
            }
            break;
    }
    return dir;
}

/*
 * error intercept
 */

static_fn int glob_errorcheck(glob_t *gp, const char *path) {
    int r = 1;

    if (gp->gl_errfn) r = (*gp->gl_errfn)(path, errno);
    if (gp->gl_flags & GLOB_ERR) r = 0;
    if (!r) gp->gl_error = GLOB_ABORTED;
    return r;
}

/*
 * remove backslashes
 */

static_fn void glob_trim(char *sp, char *p1, int *n1, char *p2, int *n2) {
    char *dp = sp;
    int c;

    if (p1) *n1 = 0;
    if (p2) *n2 = 0;
    do {
        if ((c = *sp++) == '\\') c = *sp++;
        if (sp == p1) {
            p1 = 0;
            *n1 = sp - dp - 1;
        }
        if (sp == p2) {
            p2 = 0;
            *n2 = sp - dp - 1;
        }
        *dp++ = c;
    } while (c);
}

static_fn void glob_addmatch(glob_t *gp, const char *dir, const char *pat, const char *rescan,
                             char *endslash, int meta) {
    globlist_t *ap;
    int offset;
    int type;

    stkseek(stkstd, MATCHPATH(gp));
    if (dir) {
        sfputr(stkstd, dir, 0);
        --stkstd->next;
        sfputc(stkstd, gp->gl_delim);
    }
    if (endslash) *endslash = 0;
    sfputr(stkstd, pat, 0);
    --stkstd->next;
    if (rescan) {
        if ((*gp->gl_type)(gp, stkptr(stkstd, MATCHPATH(gp)), 0) != GLOB_DIR) return;
        sfputc(stkstd, gp->gl_delim);
        offset = stktell(stkstd);
        /* if null, reserve room for . */
        if (*rescan) {
            sfputr(stkstd, rescan, 0);
            --stkstd->next;
        } else {
            sfputc(stkstd, 0);
        }
        sfputc(stkstd, 0);
        rescan = stkptr(stkstd, offset);
        ap = (globlist_t *)stkfreeze(stkstd, 0);
        ap->gl_begin = (char *)rescan;
        ap->gl_next = gp->gl_rescan;
        gp->gl_rescan = ap;
    } else {
        if (!endslash && (gp->gl_flags & GLOB_MARK) &&
            (type = (*gp->gl_type)(gp, stkptr(stkstd, MATCHPATH(gp)), 0))) {
            if ((gp->gl_flags & GLOB_COMPLETE) && type != GLOB_EXE) {
                stkseek(stkstd, 0);
                return;
            } else if (type == GLOB_DIR && (gp->gl_flags & GLOB_MARK)) {
                sfputc(stkstd, gp->gl_delim);
            }
        }
        ap = (globlist_t *)stkfreeze(stkstd, 1);
        ap->gl_next = gp->gl_match;
        gp->gl_match = ap;
        gp->gl_pathc++;
    }
    ap->gl_flags = MATCH_RAW | meta;
    if (gp->gl_flags & GLOB_COMPLETE) ap->gl_flags |= MATCH_MAKE;
}

/*
 * this routine builds a list of files that match a given pathname
 * uses REG_SHELL of <regex> to match each component
 * a leading . must match explicitly
 */

static_fn void glob_dir(glob_t *gp, globlist_t *ap, int re_flags) {
    char *rescan;
    char *prefix;
    char *pat;
    char *name;
    int c;
    char *dirname;
    void *dirf;
    char first;
    regex_t *ire;
    regex_t *pre;
    regex_t rec;
    regex_t rei;
    int notdir;
    int t1;
    int t2;
    int bracket;

    int anymeta = ap->gl_flags & MATCH_META;
    int complete = 0;
    int err = 0;
    int meta = ((gp->re_flags & REG_ICASE) && *ap->gl_begin != '/') ? MATCH_META : 0;
    int quote = 0;
    int savequote = 0;
    char *restore1 = NULL;
    char *restore2 = NULL;
    regex_t *prec = NULL;
    regex_t *prei = NULL;
    char *matchdir = NULL;
    int starstar = 0;

    if (*gp->gl_intr) {
        gp->gl_error = GLOB_INTR;
        return;
    }
    pat = rescan = ap->gl_begin;
    prefix = dirname = ap->gl_path + gp->gl_extra;
    first = (rescan == prefix);
again:
    bracket = 0;
    bool done = false;
    while (!done) {
        c = *rescan++;
        switch (c) {
            case 0:
                if (meta) {
                    rescan = NULL;
                    done = true;
                    break;
                }
                if (quote) {
                    glob_trim(ap->gl_begin, rescan, &t1, NULL, NULL);
                    rescan -= t1;
                }
                if (!first && !*rescan && *(rescan - 2) == gp->gl_delim) {
                    *(rescan - 2) = 0;
                    c = (*gp->gl_type)(gp, prefix, 0);
                    *(rescan - 2) = gp->gl_delim;
                    if (c == GLOB_DIR) glob_addmatch(gp, NULL, prefix, NULL, rescan - 1, anymeta);
                } else if ((anymeta || !(gp->gl_flags & GLOB_NOCHECK)) &&
                           (*gp->gl_type)(gp, prefix, 0)) {
                    glob_addmatch(gp, NULL, prefix, NULL, NULL, anymeta);
                }
                return;
            case '[':
                if (!bracket) {
                    bracket = MATCH_META;
                    if (*rescan == '!' || *rescan == '^') rescan++;
                    if (*rescan == ']') rescan++;
                }
                break;
            case ']':
                meta |= bracket;
                break;
            case '(':
                if (!(gp->gl_flags & GLOB_AUGMENTED)) break;
            // FALLTHRU
            case '*':
            case '?':
                meta = MATCH_META;
                break;
            case '\\':
                if (!(gp->gl_flags & GLOB_NOESCAPE)) {
                    quote = 1;
                    if (*rescan) rescan++;
                }
                break;
            default:
                if (c == gp->gl_delim) {
                    if (meta) {
                        done = true;
                        break;
                    }
                    pat = rescan;
                    bracket = 0;
                    savequote = quote;
                }
                break;
        }
    }
    anymeta |= meta;
    if (matchdir) goto skip;
    if (pat == prefix) {
        prefix = 0;
        if (!rescan && (gp->gl_flags & GLOB_COMPLETE)) {
            complete = 1;
            dirname = 0;
        } else {
            dirname = ".";
        }
    } else {
        if (pat == prefix + 1) dirname = "/";
        if (savequote) {
            quote = 0;
            glob_trim(ap->gl_begin, pat, &t1, rescan, &t2);
            pat -= t1;
            if (rescan) rescan -= t2;
        }
        *(restore1 = pat - 1) = 0;
    }
    if (!complete && (gp->gl_flags & GLOB_STARSTAR)) {
        while (pat[0] == '*' && pat[1] == '*' && (pat[2] == '/' || pat[2] == 0)) {
            matchdir = pat;
            if (pat[2]) {
                pat += 3;
                while (*pat == '/') pat++;
                if (*pat) continue;
            }
            rescan = *pat ? 0 : pat;
            pat = "*";
            goto skip;
        }
    }
    if (matchdir) {
        rescan = pat;
        goto again;
    }
skip:
    if (rescan) *(restore2 = rescan - 1) = 0;
    if (rescan && !complete && (gp->gl_flags & GLOB_STARSTAR)) {
        char *p = rescan;

        while (p[0] == '*' && p[1] == '*' && (p[2] == '/' || p[2] == 0)) {
            rescan = p;
            starstar = (p[2] == 0);
            if (starstar) break;
            p += 3;
            while (*p == '/') p++;
            if (*p == 0) {
                starstar = 2;
                break;
            }
        }
    }
    if (matchdir) gp->gl_starstar++;
    if (gp->gl_opt) pat = strcpy(gp->gl_opt, pat);
    for (;;) {
        if (complete) {
            if (!(dirname = (*gp->gl_nextdir)(gp, dirname))) break;
            prefix = !strcmp(dirname, ".") ? NULL : dirname;
        }
        if (((!starstar && !gp->gl_starstar) ||
             (*gp->gl_type)(gp, dirname, GLOB_STARSTAR) == GLOB_DIR) &&
            (dirf = (*gp->gl_diropen)(gp, dirname))) {
            if (!(gp->re_flags & REG_ICASE) && ((*gp->gl_attr)(gp, dirname, 0) & GLOB_ICASE)) {
                if (!prei) {
                    err = regcomp(&rei, pat, gp->re_flags | REG_ICASE);
                    if (err) break;
                    prei = &rei;
                    if (gp->re_first) {
                        gp->re_first = 0;
                        gp->re_flags = regstat(prei)->re_flags & ~REG_ICASE;
                    }
                }
                pre = prei;
            } else {
                if (!prec) {
                    err = regcomp(&rec, pat, gp->re_flags);
                    if (err) break;
                    prec = &rec;
                    if (gp->re_first) {
                        gp->re_first = 0;
                        gp->re_flags = regstat(prec)->re_flags;
                    }
                }
                pre = prec;
            }
            if ((ire = gp->gl_ignore) && (gp->re_flags & REG_ICASE)) {
                if (!gp->gl_ignorei) {
                    if (regcomp(&gp->re_ignorei, gp->gl_fignore, re_flags | REG_ICASE)) {
                        gp->gl_error = GLOB_APPERR;
                        break;
                    }
                    gp->gl_ignorei = &gp->re_ignorei;
                }
                ire = gp->gl_ignorei;
            }
            if (restore2) *restore2 = gp->gl_delim;
            while ((name = (*gp->gl_dirnext)(gp, dirf)) && !*gp->gl_intr) {
                // If FIGNORE is set, ignore `.` and `..`.
                // https://github.com/att/ast/issues/11
                if (gp->gl_fignore && (!strcmp(name, ".") || !strcmp(name, ".."))) {
                    continue;
                }
                notdir = (gp->gl_status & GLOB_NOTDIR);
                if (notdir) gp->gl_status &= ~GLOB_NOTDIR;
                if (ire && !regexec(ire, name, 0, NULL, 0)) continue;
                if (matchdir && (name[0] != '.' || (name[1] && (name[1] != '.' || name[2]))) &&
                    !notdir) {
                    glob_addmatch(gp, prefix, name, matchdir, NULL, anymeta);
                }
                if (!regexec(pre, name, 0, NULL, 0)) {
                    if (!rescan || !notdir) glob_addmatch(gp, prefix, name, rescan, NULL, anymeta);
                    if (starstar == 1 || (starstar == 2 && !notdir)) {
                        glob_addmatch(gp, prefix, name, starstar == 2 ? "" : NULL, NULL, anymeta);
                    }
                }
                errno = 0;
            }
            (*gp->gl_dirclose)(gp, dirf);
            if (err || (errno && !glob_errorcheck(gp, dirname))) break;
        } else if (!complete && !glob_errorcheck(gp, dirname)) {
            break;
        }
        if (!complete) break;
        if (*gp->gl_intr) {
            gp->gl_error = GLOB_INTR;
            break;
        }
    }
    if (restore1) *restore1 = gp->gl_delim;
    if (restore2) *restore2 = gp->gl_delim;
    if (prec) regfree(prec);
    if (prei) regfree(prei);
    if (err == REG_ESPACE) gp->gl_error = GLOB_NOSPACE;
}

int ast_glob(const char *pattern, int flags, int (*errfn)(const char *, int), glob_t *gp) {
    globlist_t *ap;
    char *pat;
    globlist_t *top;
    Sfio_t *oldstak;
    char **argv;
    char **av;
    size_t skip;
    unsigned long f;
    int n;
    int x;
    int re_flags;

    const char *nocheck = pattern;
    int optlen = 0;
    int suflen = 0;
    int extra = 1;
    unsigned char intr = 0;

    gp->gl_rescan = 0;
    gp->gl_error = 0;
    gp->gl_errfn = errfn;
    if (flags & GLOB_APPEND) {
        if ((gp->gl_flags |= GLOB_APPEND) ^ (flags | GLOB_MAGIC)) return GLOB_APPERR;
        if (((gp->gl_flags & GLOB_STACK) == 0) == (gp->gl_stak == 0)) return GLOB_APPERR;
        if (gp->gl_starstar > 1) {
            gp->gl_flags |= GLOB_STARSTAR;
        } else {
            gp->gl_starstar = 0;
        }
    } else {
        gp->gl_flags = (flags & 0xffff) | GLOB_MAGIC;
        gp->re_flags = REG_SHELL | REG_NOSUB | REG_LEFT | REG_RIGHT |
                       ((flags & GLOB_AUGMENTED) ? REG_AUGMENTED : 0);
        gp->gl_pathc = 0;
        gp->gl_ignore = 0;
        gp->gl_ignorei = 0;
        gp->gl_starstar = 0;
        if (!(flags & GLOB_DISC)) {
            gp->gl_fignore = NULL;
            gp->gl_suffix = NULL;
            gp->gl_intr = NULL;
            gp->gl_delim = 0;
            gp->gl_handle = NULL;
            gp->gl_diropen = NULL;
            gp->gl_dirnext = NULL;
            gp->gl_dirclose = NULL;
            gp->gl_type = NULL;
            gp->gl_attr = NULL;
            gp->gl_nextdir = NULL;
            gp->gl_stat = 0;
            gp->gl_lstat = 0;
            gp->gl_extra = 0;
        }
        if (!(flags & GLOB_ALTDIRFUNC)) {
            gp->gl_opendir = opendir;
            gp->gl_readdir = readdir;
            gp->gl_closedir = closedir;
            if (!gp->gl_stat) gp->gl_stat = pathstat;
        }
        if (!gp->gl_lstat) gp->gl_lstat = lstat;
        if (!gp->gl_intr) gp->gl_intr = &intr;
        if (!gp->gl_delim) gp->gl_delim = '/';
        if (!gp->gl_diropen) gp->gl_diropen = gl_diropen;
        if (!gp->gl_dirnext) gp->gl_dirnext = gl_dirnext;
        if (!gp->gl_dirclose) gp->gl_dirclose = gl_dirclose;
        if (!gp->gl_type) gp->gl_type = gl_type;
        if (!gp->gl_attr) gp->gl_attr = gl_attr;
        if (flags & GLOB_GROUP) gp->re_flags |= REG_SHELL_GROUP;
        if (flags & GLOB_ICASE) gp->re_flags |= REG_ICASE;
        if (!gp->gl_fignore) {
            gp->re_flags |= REG_SHELL_DOT;
        } else if (*gp->gl_fignore) {
            if (regcomp(&gp->re_ignore, gp->gl_fignore, gp->re_flags)) return GLOB_APPERR;
            gp->gl_ignore = &gp->re_ignore;
        }
        if (gp->gl_flags & GLOB_STACK) {
            gp->gl_stak = 0;
        } else if (!(gp->gl_stak = stkopen(0))) {
            return GLOB_NOSPACE;
        }
        if ((gp->gl_flags & GLOB_COMPLETE) && !gp->gl_nextdir) gp->gl_nextdir = gl_nextdir;
    }
    skip = gp->gl_pathc;
    if (gp->gl_stak) oldstak = stkinstall(gp->gl_stak, 0);
    if (flags & GLOB_DOOFFS) extra += gp->gl_offs;
    if (gp->gl_suffix) suflen = strlen(gp->gl_suffix);
    if (*(pat = (char *)pattern) == '~' && *(pat + 1) == '(') {
        f = gp->gl_flags;
        n = 1;
        x = 1;
        pat += 2;
        bool done = false;
        while (!done) {
            switch (*pat++) {
                case 0:
                case ':':
                    done = true;
                    break;
                case '-':
                    n = 0;
                    break;
                case '+':
                    n = 1;
                    break;
                case 'i':
                    if (n) {
                        f |= GLOB_ICASE;
                    } else {
                        f &= ~GLOB_ICASE;
                    }
                    break;
                case 'M':
                    if (n) {
                        f |= GLOB_BRACE;
                    } else {
                        f &= ~GLOB_BRACE;
                    }
                    break;
                case 'N':
                    if (n) {
                        f &= ~GLOB_NOCHECK;
                    } else {
                        f |= GLOB_NOCHECK;
                    }
                    break;
                case 'O':
                    if (n) {
                        f |= GLOB_STARSTAR;
                    } else {
                        f &= ~GLOB_STARSTAR;
                    }
                    break;
                case ')':
                    flags = (gp->gl_flags = f) & 0xffff;
                    if (f & GLOB_ICASE) {
                        gp->re_flags |= REG_ICASE;
                    } else {
                        gp->re_flags &= ~REG_ICASE;
                    }
                    if (x) optlen = pat - (char *)pattern;
                    done = true;
                    break;
                default:
                    x = 0;
                    break;
            }
        }
    }
    top = ap = stkalloc(
        stkstd, (optlen ? 2 : 1) * strlen(pattern) + sizeof(globlist_t) + suflen + gp->gl_extra);
    ap->gl_next = NULL;
    ap->gl_flags = 0;
    ap->gl_begin = ap->gl_path + gp->gl_extra;
    // TODO: Rewrite this to utilize safer functions like strlcpy(). See issue #956.
    pat = stpcpy(ap->gl_begin, pattern + optlen);
    if (suflen) pat = stpcpy(pat, gp->gl_suffix);
    if (optlen) {
        strlcpy(gp->gl_pat = gp->gl_opt = pat + 1, pattern, optlen);
    } else {
        gp->gl_pat = NULL;
    }
    if (!(flags & GLOB_LIST)) gp->gl_match = 0;
    re_flags = gp->re_flags;
    gp->re_first = 1;
    do {
        gp->gl_rescan = ap->gl_next;
        glob_dir(gp, ap, re_flags);
    } while (!gp->gl_error && (ap = gp->gl_rescan));
    gp->re_flags = re_flags;
    if (gp->gl_pathc == skip) {
        if (flags & GLOB_NOCHECK) {
            gp->gl_pathc++;
            top->gl_next = gp->gl_match;
            gp->gl_match = top;
            strcpy(top->gl_path + gp->gl_extra, nocheck);
        } else {
            gp->gl_error = GLOB_NOMATCH;
        }
    }
    if (flags & GLOB_LIST) {
        gp->gl_list = gp->gl_match;
    } else {
        argv = stkalloc(stkstd, (gp->gl_pathc + extra) * sizeof(char *));
        if (gp->gl_flags & GLOB_APPEND) {
            skip += --extra;
            memcpy(argv, gp->gl_pathv, skip * sizeof(char *));
            av = argv + skip;
        } else {
            av = argv;
            while (--extra > 0) *av++ = 0;
        }
        gp->gl_pathv = argv;
        argv = av;
        ap = gp->gl_match;
        while (ap) {
            *argv++ = ap->gl_path + gp->gl_extra;
            ap = ap->gl_next;
        }
        *argv = 0;
        if (!(flags & GLOB_NOSORT) && (argv - av) > 1) {
            strsort(av, argv - av, strcoll);
            if (gp->gl_starstar > 1) av[gp->gl_pathc = struniq(av, argv - av)] = 0;
            gp->gl_starstar = 0;
        }
    }
    if (gp->gl_starstar > 1) gp->gl_flags &= ~GLOB_STARSTAR;
    if (gp->gl_stak) stkinstall(oldstak, 0);
    return gp->gl_error;
}
