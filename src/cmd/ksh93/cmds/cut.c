/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2012 AT&T Intellectual Property          *
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
 *                 Glenn Fowler <gsf@research.att.com>                  *
 *                  David Korn <dgk@research.att.com>                   *
 *                                                                      *
 ***********************************************************************/
/*
 * David Korn
 * AT&T Bell Laboratories
 *
 * cut fields or columns from fields from a file
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "ast.h"
#include "builtins.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

typedef struct Delim_s {
    char *str;
    int len;
    int chr;
} Delim_t;

typedef struct Cut_s {
    int mb;
    int eob;
    int cflag;
    int nosplit;
    int sflag;
    int nlflag;
    int reclen;
    Delim_t wdelim;
    Delim_t ldelim;
    unsigned char space[UCHAR_MAX + 1];
    int list[2]; /* NOTE: must be last member */
} Cut_t;

#define HUGE INT_MAX
#define BLOCK 8 * 1024
#define C_BYTES 1
#define C_CHARS 2
#define C_FIELDS 4
#define C_SUPRESS 8
#define C_NOSPLIT 16
#define C_NONEWLINE 32

#define SP_LINE 1
#define SP_WORD 2
#define SP_WIDE 3

/*
 * compare the first of an array of integers
 */

static int mycomp(const void *a, const void *b) {
    if (*((int *)a) < *((int *)b)) return -1;
    if (*((int *)a) > *((int *)b)) return 1;
    return 0;
}

static Cut_t *cutinit(int mode, char *str, Delim_t *wdelim, Delim_t *ldelim, size_t reclen) {
    int *lp;
    int c;
    int n = 0;
    int range = 0;
    char *cp = str;
    Cut_t *cut = stkalloc(stkstd, sizeof(Cut_t) + strlen(cp) * sizeof(int));
    if (!cut) error(ERROR_exit(1), "out of space");
    cut->mb = mbwide();
    if (cut->mb) {
        memset(cut->space, 0, sizeof(cut->space) / 2);
        memset(cut->space + sizeof(cut->space) / 2, SP_WIDE, sizeof(cut->space) / 2);
    } else {
        memset(cut->space, 0, sizeof(cut->space));
    }

    cut->wdelim = *wdelim;
    if (wdelim->len == 1) cut->space[wdelim->chr] = SP_WORD;
    cut->ldelim = *ldelim;
    cut->eob = (ldelim->len == 1) ? ldelim->chr : 0;
    cut->space[cut->eob] = SP_LINE;
    cut->cflag = (mode & C_CHARS) && cut->mb;
    cut->nosplit = (mode & (C_BYTES | C_NOSPLIT)) == (C_BYTES | C_NOSPLIT) && cut->mb;
    cut->sflag = (mode & C_SUPRESS) != 0;
    cut->nlflag = (mode & C_NONEWLINE) != 0;
    cut->reclen = reclen;
    lp = cut->list;
    for (;;) {
        switch (c = *cp++) {
            case ' ':
            case '\t':
                while (*cp == ' ' || *cp == '\t') cp++;
            // FALLTHROUGH
            case 0:
            case ',':
                if (range) {
                    --range;
                    n = (n ? (n - range) : (HUGE - 1));
                    if (n < 0) {
                        error(ERROR_exit(1), "invalid range for c/f option");
                    }
                    *lp++ = range;
                    *lp++ = n;
                } else {
                    *lp++ = --n;
                    *lp++ = 1;
                }
                if (c == 0) {
                    int *dp;
                    *lp = HUGE;
                    n = 1 + (lp - cut->list) / 2;
                    lp = cut->list;
                    qsort(lp, n, 2 * sizeof(*lp), mycomp);
                    // Eliminate overlapping regions.
                    for (range = -2, dp = lp; *lp != HUGE; lp += 2) {
                        if (lp[0] <= range) {
                            if (lp[1] == HUGE) {
                                dp[-1] = HUGE;
                                break;
                            }
                            c = lp[0] + lp[1] - range;
                            if (c > 0) {
                                range += c;
                                dp[-1] += c;
                            }
                        } else {
                            range = *dp++ = lp[0];
                            if (lp[1] == HUGE) {
                                *dp++ = HUGE;
                                break;
                            }
                            range += (*dp++ = lp[1]);
                        }
                    }
                    *dp = HUGE;
                    lp = cut->list;
                    /* convert ranges into gaps */
                    for (n = 0; *lp != HUGE; lp += 2) {
                        c = *lp;
                        *lp -= n;
                        n = c + lp[1];
                    }
                    return cut;
                }
                n = range = 0;
                break;

            case '-':
                if (range) error(ERROR_exit(1), "bad list for c/f option");
                range = n ? n : 1;
                n = 0;
                break;

            default:
                if (!isdigit(c)) error(ERROR_exit(1), "bad list for c/f option");
                n = 10 * n + (c - '0');
                break;
        }
    }
    /* NOTREACHED */
}

/*
 * cut each line of file <fdin> and put results to <fdout> using list <list>
 */

static void cutcols(Cut_t *cut, Sfio_t *fdin, Sfio_t *fdout) {
    int c;
    int len;
    int ncol = 0;
    char *bp;
    int skip;  // non-zero for don't copy
    int must;
    const char *xx;
    const int *lp;

    for (;;) {
        len = cut->reclen;
        if (len) {
            bp = sfreserve(fdin, len, -1);
        } else {
            bp = sfgetr(fdin, '\n', 0);
        }

        if (!bp && !(bp = sfgetr(fdin, 0, SF_LASTR))) break;
        len = sfvalue(fdin);
        xx = 0;
        lp = cut->list;
        ncol = skip = *lp;
        if (!ncol) ncol = *++lp;
        must = 1;
        do {
            if (cut->nosplit) {
                const char *s = bp;
                int w = len < ncol ? len : ncol;
                int z;

                while (w > 0) {
                    if (!(*s & 0x80)) {
                        z = 1;
                    } else if ((z = mblen(s, w)) <= 0) {
                        if (s == bp && xx) {
                            w += s - xx;
                            bp = (char *)(s = xx);
                            xx = 0;
                            continue;
                        }
                        xx = s;
                        if (skip) s += w;
                        w = 0;
                        break;
                    }
                    s += z;
                    w -= z;
                }
                c = s - bp;
                ncol = !w && ncol >= len;
            } else if (cut->cflag) {
                const char *s = bp;
                int w = len;
                int z;

                while (w > 0 && ncol > 0) {
                    ncol--;
                    if (!(*s & 0x80) || (z = mblen(s, w)) <= 0) z = 1;
                    s += z;
                    w -= z;
                }
                c = s - bp;
                ncol = !w && (ncol || !skip);
            } else {
                c = ncol;
                if (c > len) {
                    c = len;
                } else if (c == len && !skip) {
                    ncol++;
                }
                ncol -= c;
            }
            if (!skip && c) {
                if (sfwrite(fdout, (char *)bp, c) < 0) return;
                must = 0;
            }
            bp += c;
            if (ncol) break;
            len -= c;
            // TODO: Below statement was commented as a "fix" for
            // https://github.com/att/ast/issues/1157 Coverity Scan, CID#279521, has identified a
            // theoretical path to the next assignment where `lp` has already been incremented.
            // Which would result in accessing memory whose content is undefined.
            //
            // assert(lp == cut->list);
            ncol = *++lp;
            skip = !skip;
        } while (ncol != HUGE);
        if (!cut->nlflag && (skip || must || cut->reclen)) {
            if (cut->ldelim.len > 1) {
                sfwrite(fdout, cut->ldelim.str, cut->ldelim.len);
            } else {
                sfputc(fdout, cut->ldelim.chr);
            }
        }
    }
}

/*
 * cut each line of file <fdin> and put results to <fdout> using list <list>
 * stream <fdin> must be line buffered
 */

static void cutfields(Cut_t *cut, Sfio_t *fdin, Sfio_t *fdout) {
    unsigned char *sp = cut->space;
    unsigned char *cp;
    unsigned char *wp;
    int c, nfields;
    const int *lp = cut->list;
    unsigned char *copy;
    int nodelim, empty, inword = 0;
    unsigned char *ep;
    unsigned char *bp, *first;
    int lastchar;
    wchar_t w;
    Sfio_t *fdtmp = NULL;
    long offset = 0;
    unsigned char mb[8];
    /* process each buffer */
    while ((bp = (unsigned char *)sfreserve(fdin, SF_UNBOUND, -1)) && (c = sfvalue(fdin)) > 0) {
        cp = bp;
        ep = cp + --c;
        lastchar = cp[c];
        if (lastchar != cut->eob) *ep = cut->eob;
        /* process each line in the buffer */
        while (cp <= ep) {
            first = cp;
            if (!inword) {
                nodelim = empty = 1;
                copy = cp;
                nfields = *(lp = cut->list);
                if (nfields) {
                    copy = 0;
                } else {
                    nfields = *++lp;
                }
            } else if (copy) {
                copy = cp;
            }
            inword = 0;
            do {
                /* skip over non-delimiter characters */
                if (cut->mb) {
                    bool done = false;
                    while (!done) {
                        c = sp[*(unsigned char *)cp++];
                        switch (c) {
                            case 0:
                                break;
                            case SP_WIDE:
                                wp = --cp;
                                c = mbtowc(&w, (char *)cp, ep - cp);
                                if (c <= 0) {
                                    // Mbchar possibly spanning buffer boundary -- fun stuff.
                                    if ((ep - cp) < MB_CUR_MAX) {
                                        int i;
                                        int j;
                                        int k;

                                        if (lastchar != cut->eob) {
                                            *ep = lastchar;
                                            c = mbtowc(&w, (char *)cp, ep - cp);
                                            if (c > 0) goto sp_wide_error_recovery;
                                        }
                                        if (copy) {
                                            empty = 0;
                                            c = cp - copy;
                                            if (c > 0 && sfwrite(fdout, (char *)copy, c) < 0) {
                                                goto failed;
                                            }
                                        }
                                        for (i = 0; i <= (ep - cp); i++) mb[i] = cp[i];
                                        bp = (unsigned char *)sfreserve(fdin, SF_UNBOUND, -1);
                                        if (!bp || (c = sfvalue(fdin)) <= 0) {
                                            goto failed;
                                        }
                                        cp = bp;
                                        ep = cp + --c;
                                        lastchar = cp[c];
                                        if (lastchar != cut->eob) *ep = cut->eob;
                                        j = i;
                                        k = 0;
                                        while (j < MB_CUR_MAX) mb[j++] = cp[k++];
                                        c = mbtowc(&w, (char *)mb, j);
                                        if (c <= 0) {
                                            c = i;
                                            w = 0;
                                        }
                                        first = bp = cp += c - i;
                                        if (copy) {
                                            copy = bp;
                                            if (w == cut->ldelim.chr) {
                                                lastchar = cut->ldelim.chr;
                                            } else if (w != cut->wdelim.chr) {
                                                empty = 0;
                                                if (sfwrite(fdout, (char *)mb, c) < 0) goto failed;
                                            }
                                        }
                                        c = 0;
                                    } else {
                                        w = *cp;
                                        c = 1;
                                    }
                                }
                            sp_wide_error_recovery:
                                cp += c;
                                c = w;
                                if (c == cut->wdelim.chr) {
                                    c = SP_WORD;
                                    done = true;
                                } else if (c == cut->ldelim.chr) {
                                    c = SP_LINE;
                                    done = true;
                                }
                                break;
                            default:
                                wp = cp - 1;
                                done = true;
                                break;
                        }
                    }
                } else {
                    while (!(c = sp[*cp++])) {
                        ;
                    }
                    wp = cp - 1;
                }
                /* check for end-of-line */
                if (c == SP_LINE) {
                    if (cp <= ep) break;
                    if (lastchar == cut->ldelim.chr) break;
                    /* restore cut->last character */
                    if (lastchar != cut->eob) *ep = lastchar;
                    inword++;
                    if (!sp[lastchar]) break;
                }
                nodelim = 0;
                if (--nfields > 0) continue;
                nfields = *++lp;
                if (copy) {
                    empty = 0;
                    c = wp - copy;
                    if (c > 0 && sfwrite(fdout, (char *)copy, c) < 0) goto failed;
                    copy = 0;
                } else {
                    /* set to delimiter unless the first field */
                    copy = empty ? cp : wp;
                }
            } while (!inword);
            if (!inword) {
                if (!copy) {
                    if (nodelim) {
                        if (!cut->sflag) {
                            if (offset) {
                                sfseek(fdtmp, (Sfoff_t)0, SEEK_SET);
                                sfmove(fdtmp, fdout, offset, -1);
                            }
                            copy = first;
                        }
                    } else {
                        sfputc(fdout, '\n');
                    }
                }
                if (offset) sfseek(fdtmp, offset = 0, SEEK_SET);
            }
            if (copy && (c = cp - copy) > 0 && (!nodelim || !cut->sflag) &&
                sfwrite(fdout, (char *)copy, c) < 0) {
                goto failed;
            }
        }
        /* see whether to save in tmp file */
        if (inword && nodelim && !cut->sflag && (c = cp - first) > 0) {
            /* copy line to tmpfile in case no fields */
            if (!fdtmp) fdtmp = sftmp(BLOCK);
            sfwrite(fdtmp, (char *)first, c);
            offset += c;
        }
    }
failed:
    if (fdtmp) sfclose(fdtmp);
}

int b_cut(int argc, char **argv, Shbltin_t *context) {
    char *cp = NULL;
    Sfio_t *fp;
    char *s;
    int n;
    Cut_t *cut;
    int mode = 0;
    Delim_t wdelim = {NULL, 0, 0};
    Delim_t ldelim = {NULL, 0, 0};
    size_t reclen = 0;

    if (cmdinit(argc, argv, context, 0)) return -1;
    wdelim.chr = '\t';
    ldelim.chr = '\n';
    wdelim.len = ldelim.len = 1;
    while ((n = optget(argv, sh_optcut))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'b':
            case 'c':
                if (mode & C_FIELDS) {
                    error(2, "f option already specified");
                    break;
                }
                cp = opt_info.arg;
                if (opt_info.option[1] == 'b') {
                    mode |= C_BYTES;
                } else {
                    mode |= C_CHARS;
                }
                break;
            case 'D':
                ldelim.str = opt_info.arg;
                if (mbwide()) {
                    s = opt_info.arg;
                    ldelim.chr = mb1char(&s);
                    if ((n = s - opt_info.arg) > 1) {
                        ldelim.len = n;
                        break;
                    }
                }
                ldelim.chr = *(unsigned char *)opt_info.arg;
                ldelim.len = 1;
                break;
            case 'd':
                wdelim.str = opt_info.arg;
                if (mbwide()) {
                    s = opt_info.arg;
                    wdelim.chr = mb1char(&s);
                    n = s - opt_info.arg;
                    if (n > 1) {
                        wdelim.len = n;
                        break;
                    }
                }
                wdelim.chr = *(unsigned char *)opt_info.arg;
                wdelim.len = 1;
                break;
            case 'f':
                if (mode & (C_CHARS | C_BYTES)) {
                    error(2, "c option already specified");
                    break;
                }
                cp = opt_info.arg;
                mode |= C_FIELDS;
                break;
            case 'n':
                mode |= C_NOSPLIT;
                break;
            case 'N':
                mode |= C_NONEWLINE;
                break;
            case 'R':
            case 'r':
                if (opt_info.num > 0) reclen = opt_info.num;
                break;
            case 's':
                mode |= C_SUPRESS;
                break;
            case ':':
                error(2, "%s", opt_info.arg);
                break;
            case '?':
                error(ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
        }
    }
    argv += opt_info.index;
    if (error_info.errors) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (!cp) {
        error(2, "b, c or f option must be specified");
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (!*cp) error(3, "non-empty b, c or f option must be specified");
    if ((mode & (C_FIELDS | C_SUPRESS)) == C_SUPRESS) error(3, "s option requires f option");
    if (ldelim.chr < 0) {
        // This is probably impossible but be paranoid and make linters like Coverity happy.
        error(ERROR_usage(2), "-D option value is invalid");
        __builtin_unreachable();
    }
    if (wdelim.chr < 0) {
        // This is probably impossible but be paranoid and make linters like Coverity happy.
        error(ERROR_usage(2), "-d option value is invalid");
        __builtin_unreachable();
    }
    cut = cutinit(mode, cp, &wdelim, &ldelim, reclen);
    cp = *argv;
    if (cp) argv++;
    do {
        if (!cp || !strcmp(cp, "-")) {
            fp = sfstdin;
        } else if (!(fp = sfopen(NULL, cp, "r"))) {
            error(ERROR_system(0), "%s: cannot open", cp);
            continue;
        }
        if (mode & C_FIELDS) {
            cutfields(cut, fp, sfstdout);
        } else {
            cutcols(cut, fp, sfstdout);
        }
        if (fp != sfstdin) sfclose(fp);
        cp = *argv++;
    } while (cp);
    if (sfsync(sfstdout)) error(ERROR_system(0), "write error");
    return error_info.errors != 0;
}
