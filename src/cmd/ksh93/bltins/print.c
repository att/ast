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

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ast.h"
#include "ast_assert.h"
#include "ast_float.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "history.h"
#include "io.h"
#include "name.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"
#include "streval.h"
#include "tmx.h"

union types_t {
    unsigned char c;
    short h;
    int i;
    long l;
    Sflong_t ll;
    Sfdouble_t ld;
    double d;
    float f;
    char *s;
    int *ip;
    char **p;
};

struct printf {
    Sffmt_t sffmt;
    int argsize;
    int intvar;
    char **nextarg;
    char *lastarg;
    char cescape;
    char err;
    Shell_t *sh;
};

struct printmap {
    size_t size;
    char *name;
    char map[3];
};

static const struct printmap Pmap[] = {
    {3, "csv", "#q"},    {3, "ere", "R"},
    {4, "html", "H"},    {17, "nounicodeliterals", "0q"},
    {7, "pattern", "P"}, {15, "unicodeliterals", "+q"},
    {3, "url", "#H"},    {0, NULL, ""},
};

static const char preformat[] = "";

static_fn int extend(Sfio_t *, void *, Sffmt_t *);
static_fn char *genformat(Shell_t *, const char *);
static_fn int fmtvecho(Shell_t *, const char *, struct printf *);
static_fn ssize_t fmtbase64(Shell_t *, Sfio_t *, char *, const char *, int);

static char *nullarg[] = {0, 0};

// The short options deliberately does not include 'R'. Even though it is a valid flag.
static const char *short_options = "enf:prsu:vCR";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `print` command.
//
int b_print(int argc, char *argv[], Shbltin_t *context) {
    int opt;
    bool R_flag = false;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    struct print prdata;

    memset(&prdata, 0, sizeof(prdata));
    prdata.fd = 1;

    bool done = false;
    optget_ind = 0;
    while (!done && (opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'n': {
                prdata.no_newline = true;
                break;
            }
            case 'p': {
                prdata.fd = shp->coutpipe;
                prdata.msg = e_query;
                break;
            }
            case 'f': {
                prdata.format = optget_arg;
                break;
            }
            case 's': {
                // Print to history file.
                if (!sh_histinit(shp)) {
                    errormsg(SH_DICT, ERROR_system(1), e_history);
                    __builtin_unreachable();
                }
                prdata.outfile = shp->gd->hist_ptr->histfp;
                prdata.fd = sffileno(prdata.outfile);
                sh_onstate(shp, SH_HISTORY);
                prdata.write_to_hist = true;
                break;
            }
            case 'e': {
                prdata.raw = false;
                break;
            }
            case 'r': {
                prdata.raw = true;
                break;
            }
            case 'u': {
                if (optget_arg[0] == 'p' && optget_arg[1] == 0) {
                    prdata.fd = shp->coutpipe;
                    prdata.msg = e_query;
                    break;
                }
                prdata.fd = (int)strtol(optget_arg, &optget_arg, 10);
                if (*optget_arg) {
                    prdata.fd = -1;
                } else if (!sh_iovalidfd(shp, prdata.fd)) {
                    prdata.fd = -1;
                } else if (!(shp->inuse_bits & (1 << prdata.fd)) &&
                           (sh_inuse(shp, prdata.fd) ||
                            (shp->gd->hist_ptr &&
                             prdata.fd == sffileno(shp->gd->hist_ptr->histfp)))) {
                    prdata.fd = -1;
                }
                break;
            }
#if SUPPORT_JSON
            case 'j': {
                prdata.fmt_type = "json";
                // TODO: Should this FALL THRU? Is enabling JSON output
                // supposed to also set verbose mode?
                // FALLTHRU
            }
#endif
            case 'v': {
                prdata.verbose = true;
                break;
            }
            case 'C': {
                prdata.vals_are_vars = true;
                break;
            }
            case 'R': {
                // The semantics of the `-R` flag is odd. There is no other builtin command which
                // has anything remotely like this flag. If it is seen we recognize `-Rn` but ignore
                // the `z` in `-Rz` or `-Rnz` or `-Rzn`. We also stop processing flags and treat the
                // `-z` literally when `-R -z` is seen. We also recognize the next `-n` argument
                // regardless of whether `-R` or `-Rn` was seen. But any subsequent args are treated
                // as if `--` was seen. Meaning a second `-n` is treated as a literal `-n` to be
                // printed.
                argv += optget_ind;
                int n = strlen(argv[-1]);
                if (!(argv[-1][0] == '-' && argv[-1][n - 1] == 'R')) {
                    // The current flag must be of the form -*R?.
                    char *R_opt = strchr(*argv, 'R');
                    assert(R_opt);  // it should be impossible to not find the `R` flag in the arg
                    if (*(R_opt + 1) == 'n') prdata.no_newline = true;
                    argv++;
                }
                // Check if the arg after the -R arg is -n. Eat it if it is.
                if (*argv && !strcmp(*argv, "-n")) {
                    prdata.no_newline = true;
                    argv++;
                }
                R_flag = true;
                prdata.raw = true;
                optget_ind = 0;
                done = true;
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

    if (argc < 0 && !(prdata.format = *argv++)) {
        builtin_usage_error(shp, cmd, "too few arguments");
        return 2;
    }
    if ((prdata.verbose || prdata.vals_are_vars) && prdata.format) {
        builtin_usage_error(shp, cmd, "-%c and -f are mutually exclusive",
                            prdata.verbose ? 'v' : 'C');
        return 2;
    }

    // Handle special case of '-' operand for print. But only if -R wasn't used.
    if (!R_flag && argc > 0 && *argv && strcmp(*argv, "-") == 0 && strcmp(argv[-1], "--")) argv++;
    return sh_print(argv, shp, &prdata);
}

int sh_print(char **argv, Shell_t *shp, struct print *pp) {
    int exitval = 0, n;

    if (!pp->msg) pp->msg = e_file + 4;
    if (pp->format) pp->format = genformat(shp, pp->format);
    if (pp->var_name) {
        if (!shp->strbuf2) shp->strbuf2 = sfstropen();
        pp->outfile = shp->strbuf2;
        goto printv;
    }

    if (pp->fd < 0) {
        errno = EBADF;
        n = 0;
    } else if (pp->write_to_hist) {
        n = IOREAD | IOWRITE | IOSEEK;
    } else if (!(n = shp->fdstatus[pp->fd])) {
        n = sh_iocheckfd(shp, pp->fd, pp->fd);
    }

    if (!(n & IOWRITE)) {
        // Don't print error message to stdout for compatibility.
        if (pp->fd == 1) return 1;
        errormsg(SH_DICT, ERROR_system(1), pp->msg);
        __builtin_unreachable();
    }

    if (!pp->write_to_hist && !(pp->outfile = shp->sftable[pp->fd])) {
        sh_onstate(shp, SH_NOTRACK);
        n = SF_WRITE | ((n & IOREAD) ? SF_READ : 0);
        shp->sftable[pp->fd] = pp->outfile = sfnew(NULL, shp->outbuff, IOBSIZE, pp->fd, n);
        sh_offstate(shp, SH_NOTRACK);
        sfpool(pp->outfile, shp->outpool, SF_WRITE);
    }
    // Turn off share to guarantee atomic writes for printf.
    n = sfset(pp->outfile, SF_SHARE | SF_PUBLIC, 0);

printv:
    if (pp->format) {
        // Printf style print.
        Sfio_t *pool;
        struct printf pdata;
        memset(&pdata, 0, sizeof(pdata));
        pdata.sh = shp;
        pdata.sffmt.version = SFIO_VERSION;
        pdata.sffmt.extf = extend;
        pdata.nextarg = argv;
        sh_offstate(shp, SH_STOPOK);
        pool = sfpool(sfstderr, NULL, SF_WRITE);
        do {
            if (shp->trapnote & SH_SIGSET) break;
            pdata.sffmt.form = pp->format;
            sfprintf(pp->outfile, "%!", &pdata);
        } while (*pdata.nextarg && pdata.nextarg != argv);
        if (pdata.nextarg == nullarg && pdata.argsize > 0) {
            sfwrite(pp->outfile, stkptr(shp->stk, stktell(shp->stk)), pdata.argsize);
        }
        // -f flag skips adding newline at the end of output. This causes issues
        // with syncing history if -s and -f are used together. History is synced
        // later with histflush() function.
        // https://github.com/att/ast/issues/425
        if (!pp->write_to_hist && sffileno(pp->outfile) != sffileno(sfstderr))
            if (sfsync(pp->outfile) < 0) exitval = 1;
        sfpool(sfstderr, pool, SF_WRITE);
        if (pdata.err) exitval = 1;
    } else if (pp->verbose || pp->vals_are_vars) {
        while (*argv) {
            fmtbase64(shp, pp->outfile, *argv++, pp->fmt_type, pp->vals_are_vars);
            if (!pp->no_newline) sfputc(pp->outfile, '\n');
        }
    } else {
        // Echo style print.
        if (pp->no_newline && !argv[0]) {
            if (sfsync(NULL) < 0) exitval = 1;
        } else if (sh_echolist(shp, pp->outfile, pp->raw, argv) && !pp->no_newline) {
            sfputc(pp->outfile, '\n');
        }
    }

    if (pp->var_name) {
        nv_putval(pp->var_name, sfstruse(pp->outfile), 0);
    } else if (pp->write_to_hist) {
        hist_flush(shp->gd->hist_ptr);
        sh_offstate(shp, SH_HISTORY);
    } else if (n & SF_SHARE) {
        sfset(pp->outfile, SF_SHARE | SF_PUBLIC, 1);
        if (sfsync(pp->outfile) < 0) exitval = 1;
    }
    if (exitval) errormsg(SH_DICT, 2, e_io);
    return exitval;
}

//
// Echo the argument list onto <outfile>. If <raw> is non-zero then \ is not a special character.
//
// Returns 0 for \c otherwise 1.
//
int sh_echolist(Shell_t *shp, Sfio_t *outfile, int raw, char *argv[]) {
    char *cp;
    int n;
    struct printf pdata;

    pdata.cescape = 0;
    pdata.err = 0;
    while (!pdata.cescape && (cp = *argv++)) {
        if (!raw && (n = fmtvecho(shp, cp, &pdata)) >= 0) {
            if (n) sfwrite(outfile, stkptr(shp->stk, stktell(shp->stk)), n);
        } else {
            sfputr(outfile, cp, -1);
        }

        if (*argv) sfputc(outfile, ' ');
        sh_sigcheck(shp);
    }
    return !pdata.cescape;
}

//
// Modified version of stresc for generating formats.
//
static_fn char strformat(char *s) {
    char *t;
    int c;
    char *b;
    char *p;
    int w;

    b = t = s;
    for (;;) {
        switch (c = *s++) {
            case '\\': {
                if (*s == 0) break;
                c = chrexp(s - 1, &p, &w, FMT_EXP_CHAR | FMT_EXP_LINE | FMT_EXP_WIDE);
                s = p;
                // Conversion failed => empty string.
                if (c < 0) {
                    continue;
                }

                if (w) {
                    t += wctomb(t, c);
                    continue;
                }
                if (c == '%') {
                    *t++ = '%';
                } else if (c == 0) {
                    *t++ = '%';
                    c = 'Z';
                }
                break;
            }
            case 0: {
                *t = 0;
                return t - b;
            }
            default: { break; }
        }
        *t++ = c;
    }
}

static_fn char *genformat(Shell_t *shp, const char *format) {
    char *fp;
    stkseek(shp->stk, 0);
    sfputr(shp->stk, preformat, -1);
    sfputr(shp->stk, format, -1);
    fp = (char *)stkfreeze(shp->stk, 1);
    strformat(fp + sizeof(preformat) - 1);
    return fp;
}

static_fn char *fmthtml(Shell_t *shp, const char *string, int flags) {
    const char *cp = string;
    int c, offset = stktell(shp->stk);
    if (!(flags & SFFMT_ALTER)) {
        while ((c = *(unsigned char *)cp++)) {
            int s;
            s = mblen(cp - 1, MB_CUR_MAX);
            if (s > 1) {
                cp += (s - 1);
                continue;
            }
            if (c == '<') {
                sfputr(shp->stk, "&lt", ';');
            } else if (c == '>') {
                sfputr(shp->stk, "&gt", ';');
            } else if (c == '&') {
                sfputr(shp->stk, "&amp", ';');
            } else if (c == '"') {
                sfputr(shp->stk, "&quot", ';');
            } else if (c == '\'') {
                sfputr(shp->stk, "&apos", ';');
            } else if (c == ' ') {
                sfputr(shp->stk, "&nbsp", ';');
            } else if (!isprint(c) && c != '\n' && c != '\r') {
                sfprintf(shp->stk, "&#%X;", c);
            } else {
                sfputc(shp->stk, c);
            }
        }
    } else {
        while ((c = *(unsigned char *)cp++)) {
            if (strchr("!*'();@&+$,#[]<>~.\"{}|\\-`^% ", c) ||
                (!isprint(c) && c != '\n' && c != '\r')) {
                sfprintf(stkstd, "%%%02X", c);
            } else {
                sfputc(shp->stk, c);
            }
        }
    }
    sfputc(shp->stk, 0);
    return stkptr(shp->stk, offset);
}

static_fn ssize_t fmtbase64(Shell_t *shp, Sfio_t *iop, char *string, const char *fmt, int alt) {
    char *cp;
    Sfdouble_t d;
    ssize_t size;
    Namval_t *np = nv_open(string, shp->var_tree, NV_VARNAME | NV_NOADD);
    Namarr_t *ap;
    static union types_t number;
    UNUSED(fmt);

    if (!np || nv_isnull(np)) {
        if (sh_isoption(shp, SH_NOUNSET)) {
            errormsg(SH_DICT, ERROR_exit(1), e_notset, string);
            __builtin_unreachable();
        }
        return 0;
    }
    if (nv_isattr(np, NV_INTEGER) && !nv_isarray(np)) {
        d = nv_getnum(np);
        if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
            if (nv_isattr(np, NV_LONG)) {
                size = sizeof(Sfdouble_t);
                number.ld = d;
            } else if (nv_isattr(np, NV_SHORT)) {
                size = sizeof(float);
                number.f = (float)d;
            } else {
                size = sizeof(double);
                number.d = (double)d;
            }
        } else {
            if (nv_isattr(np, NV_LONG)) {
                size = sizeof(Sflong_t);
                number.ll = (Sflong_t)d;
            } else if (nv_isattr(np, NV_SHORT)) {
                size = sizeof(short);
                number.h = (short)d;
            } else {
                size = sizeof(short);
                number.i = (int)d;
            }
        }
        return sfwrite(iop, &number, size);
    }
    if (nv_isattr(np, NV_BINARY))
#if 1
    {
        Namfun_t *fp;
        for (fp = np->nvfun; fp; fp = fp->next) {
            if (fp->disc && fp->disc->writef) break;
        }
        if (fp) (*fp->disc->writef)(np, iop, 0, fp);

        int n = nv_size(np);
        if (nv_isarray(np)) {
            nv_onattr(np, NV_RAW);
            cp = nv_getval(np);
            nv_offattr(np, NV_RAW);
        } else {
            cp = FETCH_VT(np->nvalue, cp);
        }

        size = n;
        if (size == 0) size = strlen(cp);
        size = sfwrite(iop, cp, size);
        return n ? n : size;
    } else if (nv_isarray(np) && (ap = nv_arrayptr(np)) && array_elem(ap) &&
               (ap->flags & (ARRAY_UNDEF | ARRAY_SCAN))) {
        Namval_t *nspace = shp->namespace;
        if (*string == '.' && strncmp(string, ".sh.", 4)) shp->namespace = shp->last_table;
        nv_outnode(np, iop, (alt ? -1 : 0), 0);
        sfputc(iop, ')');
        shp->namespace = nspace;
        return sftell(iop);
    }

    Namval_t *nspace = shp->namespace;
    if (alt == 1 && nv_isvtree(np)) nv_onattr(np, NV_EXPORT);
#if SUPPORT_JSON
    if (fmt && strncmp(fmt, "json", 4) == 0) nv_onattr(np, NV_JSON);
#endif
    if (*string == '.') shp->namespace = NULL;
    cp = nv_getval(np);
    if (*string == '.') shp->namespace = nspace;
    if (alt == 1) {
        nv_offattr(np, NV_EXPORT);
    }
#if SUPPORT_JSON
    else if (fmt && strncmp(fmt, "json", 4) == 0) {
        nv_offattr(np, NV_JSON);
    }
#endif

    if (!cp) return 0;
    size = strlen(cp);
    return sfwrite(iop, cp, size);
#else   // 1
        nv_onattr(np, NV_RAW);
    cp = nv_getval(np);
    if (nv_isattr(np, NV_BINARY)) nv_offattr(np, NV_RAW);
    if ((size = nv_size(np)) == 0) size = strlen(cp);
    if (sz) *sz = size;
    return cp;
#endif  // 1
}

static_fn int varname(const char *str, ssize_t n) {
    int c, dot = 1, len = 1;
    if (n < 0) {
        if (*str == '.') str++;
        n = strlen(str);
    }
    for (; n > 0; n -= len) {
        len = mblen(str, MB_CUR_MAX);
        c = mb1char((char **)&str);
        if (dot && !(isalpha(c) || c == '_')) {
            break;
        } else if (dot == 0 && !(isalnum(c) || c == '_' || c == '.')) {
            break;
        }
        dot = (c == '.');
    }
    return n == 0;
}

static_fn const char *mapformat(Sffmt_t *fe) {
    const struct printmap *pm = Pmap;
    while (pm->size > 0) {
        if (pm->size == fe->n_str && strncmp(pm->name, fe->t_str, fe->n_str) == 0) return pm->map;
        pm++;
    }
    return 0;
}

static_fn int extend(Sfio_t *sp, void *v, Sffmt_t *fe) {
    UNUSED(sp);
    char *lastchar = "";
    Sfdouble_t d;
    Sfdouble_t longmin = LDBL_LLONG_MIN;
    Sfdouble_t longmax = LDBL_LLONG_MAX;
    int format = fe->fmt;
    int n;
    int fold = fe->base;
    union types_t *value = (union types_t *)v;
    struct printf *pp = (struct printf *)fe;
    Shell_t *shp = pp->sh;
    char *argp = *pp->nextarg;
    char *w, *s;

    if (fe->n_str > 0 && (format == 'T' || format == 'Q') && varname(fe->t_str, fe->n_str) &&
        (!argp || varname(argp, -1))) {
        if (argp) {
            pp->lastarg = argp;
        } else {
            argp = pp->lastarg;
        }
        if (argp) {
            sfprintf(pp->sh->strbuf, "%s.%.*s%c", argp, fe->n_str, fe->t_str, 0);
            argp = sfstruse(pp->sh->strbuf);
        }
    } else {
        pp->lastarg = NULL;
    }

    fe->flags |= SFFMT_VALUE;
    if (!argp || format == 'Z') {
        switch (format) {
            case 'c': {
                value->c = 0;
                fe->flags &= ~SFFMT_LONG;
                break;
            }
            case 'q': {
                format = 's';
            }
            // FALLTHRU
            case 's':
            case 'H':
            case 'B':
            case 'P':
            case 'R':
            case 'Z':
            case 'b': {
                fe->fmt = 's';
                fe->size = -1;
                fe->base = -1;
                value->s = "";
                fe->flags &= ~SFFMT_LONG;
                break;
            }
            case 'a':
            case 'e':
            case 'f':
            case 'g':
            case 'A':
            case 'E':
            case 'F':
            case 'G': {
                value->ld = 0.;
                break;
            }
            case 'n': {
                value->ip = &pp->intvar;
                break;
            }
            case 'Q': {
                value->ll = 0;
                break;
            }
            case 'T': {
                fe->fmt = 'd';
                value->ll = tmxgettime();
                break;
            }
            default: {
                if (!strchr("DdXxoUu", format)) {
                    errormsg(SH_DICT, ERROR_exit(1), e_formspec, format);
                    __builtin_unreachable();
                }
                fe->fmt = 'd';
                value->ll = 0;
                break;
            }
        }
    } else {
        switch (format) {
            case 'p': {
                value->p = (char **)strtol(argp, &lastchar, 10);
                break;
            }
            case 'n': {
                Namval_t *np = nv_open(argp, shp->var_tree, NV_VARNAME | NV_NOARRAY);
                _nv_unset(np, 0);
                nv_onattr(np, NV_INTEGER);
                STORE_VT(np->nvalue, i32p, calloc(1, sizeof(int32_t)));
                nv_setsize(np, 10);
#if _ast_sizeof_int == _ast_sizeof_int32_t
                value->ip = FETCH_VT(np->nvalue, i32p);
#else
                int32_t sl = 1;
                value->ip = (int *)((char *)FETCH_VT(np->nvalue, i32p) +
                                    (*((char *)&sl) ? 0 : sizeof(int)));
#endif
                nv_close(np);
                break;
            }
            case 'q': {
                if (fe->n_str) {
                    const char *fp = mapformat(fe);
                    if (fp) {
                        if (!isalpha(*fp)) {
                            switch (*fp++) {
                                case '#': {
                                    fe->flags |= SFFMT_ALTER;
                                    break;
                                }
                                case '+': {
                                    fe->flags |= SFFMT_SIGN;
                                    break;
                                }
                                case '0': {
                                    fe->flags |= SFFMT_ZERO;
                                    break;
                                }
                                default: { break; }
                            }
                        }
                        format = *fp;
                    }
                }
            }
            // FALLTHRU
            case 'b':
            case 's':
            case 'B':
            case 'H':
            case 'P':
            case 'R': {
                fe->fmt = 's';
                fe->size = -1;
                if (format == 's' && fe->base >= 0) {
                    value->p = pp->nextarg;
                    pp->nextarg = nullarg;
                } else {
                    fe->base = -1;
                    value->s = argp;
                }
                fe->flags &= ~SFFMT_LONG;
                break;
            }
            case 'c': {
                if (mbwide() && (n = mblen(argp, MB_CUR_MAX)) > 1) {
                    fe->fmt = 's';
                    fe->size = n;
                    value->s = argp;
                } else if (fe->base >= 0) {
                    value->s = argp;
                } else {
                    value->c = *argp;
                }

                fe->flags &= ~SFFMT_LONG;
                break;
            }
            case 'o':
            case 'x':
            case 'X':
            case 'u':
            case 'U': {
                longmax = LDBL_ULLONG_MAX;
            }
            // FALLTHRU
            case '.': {
                if (fe->size == 2 && strchr("bcsqHPRQTZ", *fe->form)) {
                    value->ll = ((unsigned char *)argp)[0];
                    break;
                }
            }
            // FALLTHRU
            case 'd':
            case 'D':
            case 'i': {
                switch (*argp) {
                    case '\'':
                    case '"': {
                        w = argp + 1;
                        if (mbwide() && mblen(w, MB_CUR_MAX) > 1) {
                            value->ll = mb1char(&w);
                        } else {
                            value->ll = *(unsigned char *)w++;
                        }
                        if (w[0] && (w[0] != argp[0] || w[1])) {
                            errormsg(SH_DICT, ERROR_warn(0), e_charconst, argp);
                            pp->err = 1;
                        }
                        break;
                    }
                    default: {
                        d = sh_strnum(shp, argp, &lastchar, 0);
                        if (d < longmin) {
                            errormsg(SH_DICT, ERROR_warn(0), e_overflow, argp);
                            pp->err = 1;
                            d = longmin;
                        } else if (d > longmax) {
                            errormsg(SH_DICT, ERROR_warn(0), e_overflow, argp);
                            pp->err = 1;
                            d = longmax;
                        }
                        // A conversion overlow could occur but won't unless someone is deliberately
                        // trying to do so. That should not cause a security problem in this context
                        // but theoretically could do so.
                        // cppcheck-suppress floatConversionOverflow
                        value->ll = (Sflong_t)d;
                        if (lastchar == *pp->nextarg) {
                            value->ll = *argp;
                            lastchar = "";
                        }
                        break;
                    }
                }
                fe->size = sizeof(value->ll);
                break;
            }
            case 'a':
            case 'e':
            case 'f':
            case 'g':
            case 'A':
            case 'E':
            case 'F':
            case 'G': {
                switch (*argp) {
                    case '\'':
                    case '"': {
                        d = ((unsigned char *)argp)[1];
                        if (argp[2] && (argp[2] != argp[0] || argp[3])) {
                            errormsg(SH_DICT, ERROR_warn(0), e_charconst, argp);
                            pp->err = 1;
                        }
                        break;
                    }
                    default: {
                        d = sh_strnum(shp, *pp->nextarg, &lastchar, 0);
                        break;
                    }
                }
                value->ld = d;
                fe->size = sizeof(value->ld);
                break;
            }
            case 'Q': {
                value->ll = (Sflong_t)strelapsed(*pp->nextarg, &lastchar, 1);
                break;
            }
            case 'T': {
                value->ll = (Sflong_t)tmxdate(*pp->nextarg, &lastchar, TMX_NOW);
                break;
            }
            default: {
                value->ll = 0;
                fe->fmt = 'd';
                fe->size = sizeof(value->ll);
                errormsg(SH_DICT, ERROR_exit(1), e_formspec, format);
                __builtin_unreachable();
            }
        }

        if (format == '.') {
            value->i = value->ll;
        }

        if (*lastchar) {
            errormsg(SH_DICT, ERROR_warn(0), e_argtype, format);
            pp->err = 1;
        }
        pp->nextarg++;
    }
    switch (format) {
        case 'Z': {
            fe->fmt = 'c';
            fe->base = -1;
            value->c = 0;
            break;
        }
        case 'b': {
            n = fmtvecho(shp, value->s, pp);
            if (n >= 0) {
                if (pp->nextarg == nullarg) {
                    pp->argsize = n;
                    return -1;
                }
                value->s = stkptr(shp->stk, stktell(shp->stk));
                fe->size = n;
            }
            break;
        }
        case 'B': {
            if (!shp->strbuf2) shp->strbuf2 = sfstropen();
            // Coverity Scan CID#253849 says there is a theoretical path to this point where
            // value->s points to an empty string. Which is a problem since the pointer is passed
            // by fmtbase64() to nv_open() which requires it to be a non-empty string.
            assert(*value->s);
            fe->size = fmtbase64(shp, shp->strbuf2, value->s, fe->n_str ? fe->t_str : 0,
                                 (fe->flags & SFFMT_ALTER) != 0);
            value->s = sfstruse(shp->strbuf2);
            fe->flags |= SFFMT_SHORT;
            break;
        }
        case 'H': {
            value->s = fmthtml(shp, value->s, fe->flags);
            break;
        }
        case 'q': {
            value->s = sh_fmtqf(value->s, fe->flags, fold);
            break;
        }
        case 'P': {
            s = fmtmatch(value->s);
            if (!s || *s == 0) {
                errormsg(SH_DICT, ERROR_exit(1), e_badregexp, value->s);
                __builtin_unreachable();
            }
            value->s = s;
            break;
        }
        case 'R': {
            s = fmtre(value->s);
            if (!s || *s == 0) {
                errormsg(SH_DICT, ERROR_exit(1), e_badregexp, value->s);
                __builtin_unreachable();
            }
            value->s = s;
            break;
        }
        case 'Q': {
            if (fe->n_str > 0) {
                fe->fmt = 'd';
                fe->size = sizeof(value->ll);
            } else {
                value->s = fmtelapsed(value->ll, 1);
                fe->fmt = 's';
                fe->size = -1;
            }
            break;
        }
        case 'T': {
            if (fe->n_str > 0) {
                n = fe->t_str[fe->n_str];
                fe->t_str[fe->n_str] = 0;
                value->s = fmttmx(fe->t_str, value->ll);
                fe->t_str[fe->n_str] = n;
            } else {
                value->s = fmttmx(NULL, value->ll);
            }
            fe->fmt = 's';
            fe->size = -1;
            break;
        }
        default: { break; }
    }
    return 0;
}

//
// Construct System V echo string out of <cp>. If there are not escape sequences, returns -1.
// Otherwise, puts null terminated result on stack, but doesn't freeze it returns length of output.
//
static_fn int fmtvecho(Shell_t *shp, const char *string, struct printf *pp) {
    const char *cp = string, *cpmax;
    int c;
    int offset = stktell(shp->stk);
    int chlen;
    if (mbwide()) {
        while (1) {
            chlen = mblen(cp, MB_CUR_MAX);
            // Skip over multibyte characters.
            if (chlen > 1) {
                cp += chlen;
            } else if ((c = *cp++) == 0 || c == '\\') {
                break;
            }
        }
    } else {
        while ((c = *cp++) && (c != '\\')) {
            ;  // empty body
        }
    }
    if (c == 0) return -1;
    c = --cp - string;
    if (c > 0) sfwrite(shp->stk, string, c);
    for (; (c = *cp); cp++) {
        if (mbwide() && ((chlen = mblen(cp, MB_CUR_MAX)) > 1)) {
            sfwrite(shp->stk, cp, chlen);
            cp += (chlen - 1);
            continue;
        }
        if (c == '\\') {
            switch (*++cp) {
                case 'E': {
                    c = '\033';
                    break;
                }
                case 'a': {
                    c = '\a';
                    break;
                }
                case 'b': {
                    c = '\b';
                    break;
                }
                case 'c': {
                    pp->cescape++;
                    pp->nextarg = nullarg;
                    goto done;
                }
                case 'f': {
                    c = '\f';
                    break;
                }
                case 'n': {
                    c = '\n';
                    break;
                }
                case 'r': {
                    c = '\r';
                    break;
                }
                case 'v': {
                    c = '\v';
                    break;
                }
                case 't': {
                    c = '\t';
                    break;
                }
                case '\\': {
                    c = '\\';
                    break;
                }
                case '0': {
                    c = 0;
                    cpmax = cp + 4;
                    while (++cp < cpmax && *cp >= '0' && *cp <= '7') {
                        c <<= 3;
                        c |= (*cp - '0');
                    }
                }
                // FALLTHRU
                default: { cp--; }
            }
        }
        sfputc(shp->stk, c);
    }
done:
    c = stktell(shp->stk) - offset;
    sfputc(shp->stk, 0);
    stkseek(shp->stk, offset);
    return c;
}
