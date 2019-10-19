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
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>

#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "edit.h"
#include "error.h"
#include "fault.h"
#include "history.h"
#include "io.h"
#include "lexstates.h"
#include "name.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"
#include "terminal.h"
#include "variables.h"

#define R_FLAG 1      // raw mode
#define S_FLAG 2      // save in history file
#define A_FLAG 4      // read into array
#define N_FLAG 8      // fixed size read at most
#define NN_FLAG 0x10  // fixed size read exact
#define V_FLAG 0x20   // use default value
#define C_FLAG 0x40   // read into compound variable
#define D_FLAG 8      // must be number of bits for all flags
#define SS_FLAG 0x80  // read .csv format file

struct read_save {
    char **argv;
    char *prompt;
    short fd;
    short plen;
    int flags;
    int mindex;
    ssize_t len;
    long timeout;
};

struct Method {
    char *name;
    void *fun;
};

// Support for json in `read` builtin seems very unstable, so this code is kept disabled.
// See https://github.com/att/ast/issues/820
#if SUPPORT_JSON
static_fn int json2sh(Shell_t *shp, Sfio_t *in, Sfio_t *out) {
    int c, state = 0, lastc = 0, level = 0, line = 1;
    size_t here, offset = stktell(shp->stk);
    char *start;
    bool isname, isnull = false;

    while ((c = sfgetc(in)) > 0) {
        if (c == '\n') line++;
        if (state == 0) {
            switch (c) {
                case '\t':
                case ' ': {
                    if (lastc == ' ' || lastc == '\t') continue;
                    break;
                }
                case ',': {
                    continue;
                }
                case '[':
                case '{': {
                    c = '(';
                    level++;
                    break;
                }
                case ']':
                case '}': {
                    c = ')';
                    level--;
                    break;
                }
                case '"': {
                    state = 1;
                    isname = true;
                    sfputc(shp->stk, c);
                    continue;
                }
                default: { break; }
            }
            sfputc(out, c);
            if (level == 0) break;
        } else if (state == 1) {
            if (c == '"' && lastc != '\\') {
                state = 2;
            } else if (state == 1 && !isalnum(c) && c != '_') {
                isname = false;
            }
            sfputc(shp->stk, c);
        } else if (state == 2) {
            if (c == ' ' || c == '\t') continue;
            if (c == ':') {
                int len;
                while ((c = sfgetc(in)) && iswblank(c)) {
                    ;  // empty loop
                }
                sfungetc(in, c);
                if (!strchr("[{,\"", c)) {
                    if (isdigit(c) || c == '.' || c == '-') {
                        sfwrite(out, "float ", 6);
                    } else if (c == 't' || c == 'f') {
                        sfwrite(out, "bool ", 5);
                    } else if (c == 'n') {
                        char buff[4];
                        isnull = true;
                        sfread(in, buff, 4);
                        sfwrite(out, "typeset  ", 8);
                    }
                }
                start = stkptr(shp->stk, offset);
                here = stktell(shp->stk);
                if (isname && !iswalpha(*(start + 1)) && c != '_') isname = false;
                len = here - offset - 2;
                if (!isname) {
                    char *sp;
                    sfwrite(out, ".[", 2);
                    for (sp = start + 1; len-- > 0; sp++) {
                        if (*sp == '$') {
                            if (sp > start + 1) sfwrite(out, start, sp - start);
                            sfputc(out, '\\');
                            sfputc(out, '$');
                            start = sp;
                        }
                    }
                    len = (sp - start) - 1;
                }
                sfwrite(out, start + 1, len);
                if (!isname) sfputc(out, ']');
                if (isnull) {
                    isnull = false;
                } else {
                    sfputc(out, '=');
                }
                stkseek(shp->stk, offset);
                if (c == '{') c = ' ';
            }
            if (c == ',' || c == '\n' || c == '}' || c == ']' || c == '{') {
                start = stkptr(shp->stk, offset);
                here = stktell(shp->stk);
                if (here > 1) {
                    *stkptr(shp->stk, here - 1) = 0;
                    stresc(start + 1);
                    sfputr(out, sh_fmtq(start + 1), -1);
                    stkseek(shp->stk, offset);
                }
                if (c == '{') {
                    sfputc(out, '=');
                } else {
                    sfputc(out, ' ');
                }
                if (c == '}' || c == ']' || c == '{') sfungetc(in, c);
            }
            c = ' ';
            state = 0;
        }
        lastc = c;
    }
    return 0;
}
#endif

static struct Method methods[] = {
#if SUPPORT_JSON
    {"json", json2sh},
#endif
    {"ksh", NULL},
    {NULL, NULL}};

static const char *short_options = "ad:n:p:rsu:t:vACN:S";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `read` command.
//
int b_read(int argc, char *argv[], Shbltin_t *context) {
    Sfdouble_t sec;
    char *name = NULL;
    int opt, r, flags = 0, fd = 0;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    ssize_t len = 0;
    long timeout = 1000 * shp->st.tmout;
    int save_prompt, fixargs = context->invariant;
    struct read_save *rp;
    int mindex = 0;
    char *method = NULL;
    void *readfn = NULL;
    static char default_prompt[3] = {ESC, ESC};

    rp = (struct read_save *)(context->data);
    if (argc == 0) {
        if (rp) free(rp);
        return 0;
    }
    if (rp) {
        flags = rp->flags;
        timeout = rp->timeout;
        fd = rp->fd;
        argv = rp->argv;
        name = rp->prompt;
        mindex = rp->mindex;
        r = rp->plen;
        goto bypass;
    }

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'a': {
                // `read -a` is an alias for `read -A`
                flags |= A_FLAG;
                break;
            }
            case 'A': {
                flags |= A_FLAG;
                break;
            }
            case 'C': {
                flags |= C_FLAG;
                method = "ksh";
                break;
            }
            case 't': {
                sec = sh_strnum(shp, optget_arg, NULL, 1);
                timeout = sec ? 1000 * sec : 1;
                break;
            }
            case 'd': {
                flags &= ((1 << (D_FLAG + 1)) - 1);
                flags |= (mb1char(&optget_arg) << (D_FLAG + 1)) | (1 << D_FLAG);
                break;
            }
            case 'p': {
                if (shp->cpipe[0] <= 0 || (*optget_arg != '-' && (!strmatch(optget_arg, "+(\\w)") ||
                                                                  isdigit(*optget_arg)))) {
                    name = optget_arg;
                } else {
                    // For backward compatibility.
                    fd = shp->cpipe[0];
                    argv--;
                    argc--;
                }
                break;
            }
#if SUPPORT_JSON
            case 'm': {
                method = optget_arg;
                flags |= C_FLAG;
                break;
            }
#endif
            case 'n':
            case 'N': {
                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX) {
                    builtin_usage_error(shp, cmd, "%s: invalid -%c value", optget_arg, optget_opt);
                    return 2;
                }
                len = n;
                flags &= ((1 << D_FLAG) - 1);
                flags |= (opt == 'n' ? N_FLAG : NN_FLAG);
                break;
            }
            case 'r': {
                flags |= R_FLAG;
                break;
            }
            case 's': {
                // Save in history file.
                flags |= S_FLAG;
                break;
            }
            case 'S': {
                flags |= SS_FLAG;
                break;
            }
            case 'u': {
                if (optget_arg[0] == 'p' && optget_arg[1] == 0) {
                    fd = shp->cpipe[0];
                    if (fd <= 0) {
                        errormsg(SH_DICT, ERROR_exit(1), e_query);
                        __builtin_unreachable();
                    }
                    break;
                }

                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX || !sh_iovalidfd(shp, n)) {
                    builtin_usage_error(shp, cmd, "%s: invalid -u value", optget_arg);
                    return 2;
                }
                fd = n;
                break;
            }
            case 'v': {
                flags |= V_FLAG;
                break;
            }
            case ':': {
                if (optget_opt == 'p' && shp->cpipe[0] > 0) {
                    fd = shp->cpipe[0];
                    break;
                }
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

    if (method) {
        for (mindex = 0; methods[mindex].name; mindex++) {
            if (strcmp(method, methods[mindex].name) == 0) break;
        }
        if (!methods[mindex].name) {
            errormsg(SH_DICT, ERROR_system(1), "%s method not supported", method);
            __builtin_unreachable();
        }
    }

    if (!((r = shp->fdstatus[fd]) & IOREAD) || !(r & (IOSEEK | IONOSEEK))) {
        assert(fd >= 0);  // there is a theoretical path which could reach here with fd == -1
        r = sh_iocheckfd(shp, fd, fd);
    }
    if (fd < 0 || !(r & IOREAD)) {
        errormsg(SH_DICT, ERROR_system(1), e_file + 4);
        __builtin_unreachable();
    }
    // Look for prompt.
    if (!name && *argv && (name = strchr(*argv, '?'))) name++;
    if (name && (r & IOTTY)) {
        r = strlen(name) + 1;
    } else {
        r = 0;
    }
    if (argc == fixargs && (rp = calloc(1, sizeof(struct read_save)))) {
        context->data = rp;
        rp->fd = fd;
        rp->flags = flags;
        rp->timeout = timeout;
        rp->argv = argv;
        rp->prompt = name;
        rp->plen = r;
        rp->len = len;
        rp->mindex = mindex;
    }

bypass:
    shp->prompt = default_prompt;
    if (r && (shp->prompt = (char *)sfreserve(sfstderr, r, SF_LOCKR))) {
        memcpy(shp->prompt, name, r);
        sfwrite(sfstderr, shp->prompt, r - 1);
    }
    shp->timeout = 0;
    save_prompt = shp->nextprompt;
    shp->nextprompt = 0;
    readfn = (flags & C_FLAG) ? methods[mindex].fun : 0;
    r = sh_readline(shp, argv, readfn, fd, flags, len, timeout);
    shp->nextprompt = save_prompt;
    if (r == 0) {
        r = (sfeof(shp->sftable[fd]) || sferror(shp->sftable[fd]));
        if (r && fd == shp->cpipe[0] && errno != EINTR) sh_pclose(shp->cpipe);
    }
    return r;
}

struct timeout {
    Shell_t *shp;
    Sfio_t *iop;
};

//
// Here for read timeout.
//
static_fn void timedout(void *handle) {
    struct timeout *tp = (struct timeout *)handle;
    sfclrlock(tp->iop);
    sh_exit(tp->shp, 1);
}

//
// This is the code to read a line and to split it into tokens.
// <names> is an array of variable names.
// <fd> is the file descriptor.
// <flags> is union of -A, -r, -s, and contains delimiter if not '\n'.
// <timeout> is number of milli-seconds until timeout.
//
int sh_readline(Shell_t *shp, char **names, void *readfn, volatile int fd, int flags, ssize_t size,
                long timeout) {
    ssize_t c;
    unsigned char *cp;
    char *name, *val;
    Sfio_t *iop;
    Namfun_t *nfp;
    unsigned char *cpmax;
    unsigned char *del;
    char *ifs = NULL;
    Namval_t *np = NULL;
    Namval_t *nq = NULL;
    char was_escape = 0;
    char use_stak = 0;
    volatile char was_write = 0;
    volatile char was_share = 1;
    volatile int keytrap;
    int rel, wrd;
    long array_index = 0;
    Timer_t *timeslot = NULL;
    int delim = '\n';
    int jmpval = 0;
    nvflag_t oflags = NV_ASSIGN | NV_VARNAME;
    bool inquote = false;
    checkpt_t buff;
    Edit_t *ep = (struct edit *)shp->gd->ed_context;

    if (!(iop = shp->sftable[fd]) && !(iop = sh_iostream(shp, fd, fd))) return 1;

    memset(&buff, 0, sizeof(buff));
    sh_stats(STAT_READS);
    if (names && (name = *names)) {
        Namval_t *mp = NULL;
        val = strchr(name, '?');
        if (val) *val = 0;
        if (flags & C_FLAG) oflags |= NV_ARRAY;
        np = nv_open(name, shp->var_tree, oflags);
        if (np && nv_isarray(np) && (mp = nv_opensub(np))) np = mp;
        if ((flags & V_FLAG) && shp->gd->ed_context) {
            ((struct edit *)shp->gd->ed_context)->e_default = np;
        }
        if (flags & A_FLAG) {
            Namarr_t *ap;
            flags &= ~A_FLAG;
            array_index = 1;
            if ((ap = nv_arrayptr(np)) && !ap->fun) ap->nelem++;
            nv_unset(np);
            if ((ap = nv_arrayptr(np)) && !ap->fun) ap->nelem--;
            nv_putsub(np, NULL, 0L, 0);
        } else if (flags & C_FLAG) {
            Namval_t *nvenv = np->nvenv;
            if (strchr(name, '[')) nq = np;
            delim = -1;
            nv_unset(np);
            if (!nv_isattr(np, NV_MINIMAL)) np->nvenv = nvenv;
            nv_setvtree(np);
            *(void **)(np->nvfun + 1) = readfn;
        } else {
            name = *++names;
        }
        if (val) *val = '?';
    } else {
        name = 0;
        if (dtvnext(shp->var_tree) || shp->namespace) {
            np = nv_open(nv_name(VAR_REPLY), shp->var_tree, 0);
        } else {
            np = VAR_REPLY;
        }
    }
    keytrap = ep ? ep->e_keytrap : 0;
    if (size || (flags >> D_FLAG)) {  // delimiter not new-line or fixed size read
        if ((shp->fdstatus[fd] & IOTTY) && !keytrap) tty_raw(sffileno(iop), 1);
        if (!(flags & (N_FLAG | NN_FLAG))) {
            delim = ((unsigned)flags) >> (D_FLAG + 1);
            ep->e_nttyparm.c_cc[VEOL] = delim;
            ep->e_nttyparm.c_lflag |= ISIG;
            tty_set(sffileno(iop), TCSADRAIN, &ep->e_nttyparm);
        }
    }
    bool binary = nv_isattr(np, NV_BINARY) == NV_BINARY;
    if (!binary && !(flags & (N_FLAG | NN_FLAG))) {
        Namval_t *mp;
        // Set up state table based on IFS.
        ifs = nv_getval(mp = sh_scoped(shp, VAR_IFS));
        if ((flags & R_FLAG) && shp->ifstable['\\'] == S_ESC) {
            shp->ifstable['\\'] = 0;
        } else if (!(flags & R_FLAG) && shp->ifstable['\\'] == 0) {
            shp->ifstable['\\'] = S_ESC;
        }
        if (delim > 0) shp->ifstable[delim] = S_NL;
        if (delim != '\n') {
            if (ifs && strchr(ifs, '\n')) {
                shp->ifstable['\n'] = S_DELIM;
            } else {
                shp->ifstable['\n'] = 0;
            }
            nv_putval(mp, ifs, NV_RDONLY);
        }
        shp->ifstable[0] = S_EOF;
        if ((flags & SS_FLAG)) {
            shp->ifstable['"'] = S_QUOTE;
            shp->ifstable['\r'] = S_ERR;
        }
    }
    sfclrerr(iop);
    for (nfp = np->nvfun; nfp; nfp = nfp->next) {
        if (nfp->disc && nfp->disc->readf) {
            Namval_t *mp;
            if (nq) {
                mp = nq;
            } else {
                mp = nv_open(name, shp->var_tree, oflags | NV_NOREF);
            }
            if ((c = (*nfp->disc->readf)(mp, iop, delim, nfp)) >= 0) return c;
        }
    }
    if (binary && !(flags & (N_FLAG | NN_FLAG))) {
        flags |= NN_FLAG;
        size = nv_size(np);
    }
    was_write = (sfset(iop, SF_WRITE, 0) & SF_WRITE) != 0;
    if (sffileno(iop) == 0) was_share = (sfset(iop, SF_SHARE, shp->redir0 != 2) & SF_SHARE) != 0;
    if (timeout || (shp->fdstatus[fd] & (IOTTY | IONOSEEK))) {
        sh_pushcontext(shp, &buff, 1);
        jmpval = sigsetjmp(buff.buff, 0);
        if (jmpval) goto done;
        if (timeout) {
            // WARNING: This assumes that ksh is single-threaded and this block will not be run
            // concurrently by multiple threads. It can't be a stack var because the address of
            // the var is passed to `timedout()` after this scope has exited.
            static struct timeout tmout;
            tmout.shp = shp;
            tmout.iop = iop;
            timeslot = sh_timeradd(timeout, 0, timedout, &tmout);
        }
    }
    if (flags & (N_FLAG | NN_FLAG)) {
        char buf[256], *cur, *end, *up, *v;
        char *var = buf;

        // Reserved buffer.
        if ((c = size) >= sizeof(buf)) {
            var = malloc(c + 1);
            end = var + c;
        } else {
            end = var + sizeof(buf) - 1;
        }
        up = cur = var;
        if ((sfset(iop, SF_SHARE, 1) & SF_SHARE) && sffileno(iop) != 0) was_share = 1;
        if (size == 0) {
            (void)sfreserve(iop, 0, 0);
            c = 0;
        } else {
            ssize_t m;
            int f;
            for (;;) {
                c = size;
                if (keytrap) {
                    cp = NULL;
                    f = 0;
                    m = 0;
                    while (c-- > 0 && (buf[m] = ed_getchar(ep, 0))) m++;
                    if (m > 0) cp = (unsigned char *)buf;
                } else {
                    f = 1;
                    cp = sfreserve(iop, c, SF_LOCKR);
                    if (cp) {
                        m = sfvalue(iop);
                    } else if (flags & NN_FLAG) {
                        c = size;
                        m = (cp = sfreserve(iop, c, 0)) ? sfvalue(iop) : 0;
                        f = 0;
                    } else {
                        c = sfvalue(iop);
                        m = (cp = sfreserve(iop, c, SF_LOCKR)) ? sfvalue(iop) : 0;
                    }
                }
                if (m > 0 && (flags & N_FLAG) && !binary && (v = memchr(cp, '\n', m))) {
                    *v++ = 0;
                    m = v - (char *)cp;
                }
                if ((c = m) > size) c = size;
                if (c > 0) {
                    if (c > (end - cur)) {
                        ssize_t cx = cur - var, ux = up - var;
                        m = (end - var) + (c - (end - cur));
                        if (var == buf) {
                            v = malloc(m + 1);
                            var = memcpy(v, var, cur - var);
                        } else {
                            var = realloc(var, m + 1);
                        }
                        end = var + m;
                        cur = var + cx;
                        up = var + ux;
                    }
                    // There is a theoretical path to here where `cur` could be NULL. That should
                    // never happen so assert it can't happen. Coverity CID#340037.
                    assert(cur);
                    if (cur != (char *)cp) memcpy(cur, cp, c);
                    if (f) sfread(iop, cp, c);
                    cur += c;
                    if (!binary && mbwide()) {
                        int x;
                        int z;

                        *cur = 0;
                        x = z = 0;
                        while (up < cur && (z = mblen(up, MB_CUR_MAX)) > 0) {
                            up += z;
                            x++;
                        }
                        if ((size -= x) > 0 && (up >= cur || z < 0) &&
                            ((flags & NN_FLAG) || z < 0 || m > c)) {
                            continue;
                        }
                    }
                }
                if (!binary && mbwide() && (up == var || ((flags & NN_FLAG) && size))) cur = var;
                *cur = 0;
                if (c >= size || (flags & N_FLAG) || m == 0) {
                    if (m) sfclrerr(iop);
                    break;
                }
                size -= c;
            }
        }
        if (timeslot) timerdel(timeslot);
        if (binary && !((size = nv_size(np)) && nv_isarray(np) && c != size)) {
            int optimize = !np->nvfun || !nv_hasdisc(np, &OPTIMIZE_disc);
            char *cp = FETCH_VT(np->nvalue, cp);
            if (optimize && (c == size) && cp && !nv_isarray(np)) {
                memcpy(cp, var, c);
            } else {
                Namval_t *mp;
                if (var == buf) var = memdup(var, c + 1);
                nv_putval(np, var, NV_RAW);
                nv_setsize(np, c);
                if (!nv_isattr(np, NV_IMPORT | NV_EXPORT) && (mp = np->nvenv)) {
                    nv_setsize(mp, c);
                }
            }
        } else {
            nv_putval(np, var, 0);
            if (var != buf) free(var);
        }
        goto done;
    } else if ((cp = (unsigned char *)sfgetr(iop, delim, 0))) {
        c = sfvalue(iop);
    } else if ((cp = (unsigned char *)sfgetr(iop, delim, -1))) {
        c = sfvalue(iop) + 1;
        if (!sferror(iop) && sfgetc(iop) >= 0) {
            errormsg(SH_DICT, ERROR_exit(1), e_overlimit, "line length");
            __builtin_unreachable();
        }
    }
    if (timeslot) timerdel(timeslot);
    if ((flags & S_FLAG) && !shp->gd->hist_ptr) {
        sh_histinit(shp);
        if (!shp->gd->hist_ptr) flags &= ~S_FLAG;
    }
    if (cp) {
        cpmax = cp + c;
        if (*(cpmax - 1) != delim) *(cpmax - 1) = delim;
        if (flags & S_FLAG) sfwrite(shp->gd->hist_ptr->histfp, (char *)cp, c);
        c = shp->ifstable[*cp++];
    } else {
        c = S_NL;
    }
    shp->nextprompt = 2;
    rel = stktell(shp->stk);
    // val==0 at the start of a field.
    val = 0;
    del = 0;
    while (1) {
        switch (c) {
            case S_MBYTE: {
                if (val == 0) val = (char *)(cp - 1);
                if (sh_strchr(ifs, (char *)cp - 1, cpmax - cp + 1) >= 0) {
                    c = mblen((char *)cp - 1, MB_CUR_MAX);
                    if (name) cp[-1] = 0;
                    if (c > 1) cp += (c - 1);
                    c = S_DELIM;
                } else {
                    c = 0;
                }
                continue;
            }
            case S_QUOTE: {
                c = shp->ifstable[*cp++];
                if (inquote && c == S_QUOTE) {
                    c = -1;
                } else {
                    inquote = !inquote;
                }
                if (val) {
                    sfputr(shp->stk, val, -1);
                    use_stak = 1;
                    *val = 0;
                }
                if (c == -1) {
                    sfputc(shp->stk, '"');
                    c = shp->ifstable[*cp++];
                }
                continue;
            }
            case S_ESC: {
                // Process escape character.
                if ((c = shp->ifstable[*cp++]) == S_NL) {
                    was_escape = 1;
                } else {
                    c = 0;
                }
                if (val) {
                    sfputr(shp->stk, val, -1);
                    use_stak = 1;
                    was_escape = 1;
                    *val = 0;
                }
                continue;
            }
            case S_ERR: {
                cp++;
            }
            // FALLTHRU
            case S_EOF: {
                // Check for end of buffer.
                if (val && *val) {
                    sfputr(shp->stk, val, -1);
                    use_stak = 1;
                }
                val = 0;
                if (cp >= cpmax) {
                    c = S_NL;
                    break;
                }
                // Eliminate null bytes.
                c = shp->ifstable[*cp++];
                if (!name && val && (c == S_SPACE || c == S_DELIM || c == S_MBYTE)) c = 0;
                continue;
            }
            case S_NL: {
                if (was_escape) {
                    was_escape = 0;
                    cp = (unsigned char *)sfgetr(iop, delim, 0);
                    if (cp) {
                        c = sfvalue(iop);
                    } else if ((cp = (unsigned char *)sfgetr(iop, delim, -1))) {
                        c = sfvalue(iop) + 1;
                    }
                    if (cp) {
                        if (flags & S_FLAG) sfwrite(shp->gd->hist_ptr->histfp, (char *)cp, c);
                        cpmax = cp + c;
                        c = shp->ifstable[*cp++];
                        val = 0;
                        if (!name && (c == S_SPACE || c == S_DELIM || c == S_MBYTE)) c = 0;
                        continue;
                    }
                }
                c = S_NL;
                break;
            }
            case S_SPACE: {
                // Skip over blanks.
                while ((c = shp->ifstable[*cp++]) == S_SPACE) {
                    ;  // empty loop
                }
                if (!val) continue;
                if (c == S_MBYTE) {
                    if (sh_strchr(ifs, (char *)cp - 1, cpmax - cp + 1) >= 0) {
                        if ((c = mblen((char *)cp - 1, MB_CUR_MAX)) > 1) cp += (c - 1);
                        c = S_DELIM;
                    } else {
                        c = 0;
                    }
                }
                if (c != S_DELIM) break;
            }
            // FALLTHRU
            case S_DELIM: {
                if (!del) del = cp - 1;
                if (name) {
                    // Skip over trailing blanks.
                    while ((c = shp->ifstable[*cp++]) == S_SPACE) {
                        ;  // empty loop
                    }
                    break;
                }
            }
            // FALLTHRU
            case 0: {
                if (val == 0 || was_escape) {
                    val = (char *)(cp - 1);
                    was_escape = 0;
                }
                // Skip over word characters.
                wrd = -1;
                while (1) {
                    while ((c = shp->ifstable[*cp++]) == 0) {
                        if (!wrd) wrd = 1;
                    }
                    if (inquote) {
                        if (c == S_QUOTE) {
                            if (shp->ifstable[*cp] == S_QUOTE) {
                                if (val) {
                                    sfwrite(shp->stk, val, cp - (unsigned char *)val);
                                    use_stak = 1;
                                }
                                val = (char *)++cp;
                            } else {
                                break;
                            }
                        }
                        if (c && c != S_EOF) {
                            if (c == S_NL) {
                                if (val) {
                                    sfwrite(shp->stk, val, cp - (unsigned char *)val);
                                    use_stak = 1;
                                }
                                cp = (unsigned char *)sfgetr(iop, delim, 0);
                                if (!cp) cp = (unsigned char *)sfgetr(iop, delim, -1);
                                val = (char *)cp;
                            }
                            continue;
                        }
                    }
                    if (!del && c == S_DELIM) del = cp - 1;
                    if (name || c == S_NL || c == S_ESC || c == S_EOF || c == S_MBYTE) break;
                    if (wrd < 0) wrd = 0;
                }
                if (wrd > 0) del = (unsigned char *)"";
                if (c != S_MBYTE) cp[-1] = 0;
                continue;
            }
            default: { break; }
        }
        // Assign value and advance to next variable.
        if (!val) val = "";
        if (use_stak) {
            sfputr(shp->stk, val, 0);
            val = stkptr(shp->stk, rel);
        }
        if (!name && *val) {
            // Strip off trailing space delimiters.
            unsigned char *vp = (unsigned char *)val + strlen(val);
            while (shp->ifstable[*--vp] == S_SPACE) {
                ;  // empty loop
            }
            if (vp == del) {
                if (vp == (unsigned char *)val) {
                    vp--;
                } else {
                    while (shp->ifstable[*--vp] == S_SPACE) {
                        ;  // empty loop
                    }
                }
            }
            vp[1] = 0;
        }
        assert(np);
        if (nv_isattr(np, NV_RDONLY)) {
            errormsg(SH_DICT, ERROR_warn(0), e_readonly, nv_name(np));
            jmpval = 1;
        } else {
            nv_putval(np, val, 0);
        }
        val = 0;
        del = 0;
        if (use_stak) {
            stkseek(shp->stk, rel);
            use_stak = 0;
        }
        if (array_index) {
            nv_putsub(np, NULL, array_index++, 0);
            if (c != S_NL) continue;
            name = *++names;
        }
        while (1) {
            if (sh_isoption(shp, SH_ALLEXPORT) && !strchr(nv_name(np), '.') &&
                !nv_isattr(np, NV_EXPORT)) {
                nv_onattr(np, NV_EXPORT);
                sh_envput(shp, np);
            }
            if (name) {
                nv_close(np);
                np = nv_open(name, shp->var_tree, NV_VARNAME);
                name = *++names;
            } else {
                np = NULL;
            }
            if (c != S_NL) break;
            if (!np) goto done;
            if (nv_isattr(np, NV_RDONLY)) {
                errormsg(SH_DICT, ERROR_warn(0), e_readonly, nv_name(np));
                jmpval = 1;
            } else {
                nv_putval(np, "", 0);
            }
        }
    }

done:

    if (timeout || (shp->fdstatus[fd] & (IOTTY | IONOSEEK))) sh_popcontext(shp, &buff);
    if (was_write) sfset(iop, SF_WRITE, 1);
    if (!was_share) sfset(iop, SF_SHARE, 0);
    nv_close(np);
    if ((shp->fdstatus[fd] & IOTTY) && !keytrap) tty_cooked(sffileno(iop));
    if (flags & S_FLAG) hist_flush(shp->gd->hist_ptr);
    if (jmpval > 1) siglongjmp(shp->jmplist->buff, jmpval);
    return jmpval;
}
