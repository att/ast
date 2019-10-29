/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
 *                                                                      *
 ***********************************************************************/
/*
 * David Korn
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * cat
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "builtins.h"
#include "error.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"

static char *stdin_argv[] = {"-", NULL};

#define RUBOUT 0177

/* control flags */
#define B_FLAG (1 << 0)
#define E_FLAG (1 << 1)
#define F_FLAG (1 << 2)
#define N_FLAG (1 << 3)
#define S_FLAG (1 << 4)
#define T_FLAG (1 << 5)
#define U_FLAG (1 << 6)
#define V_FLAG (1 << 7)
#define D_FLAG (1 << 8)
#define d_FLAG (1 << 9)

/* character types */
#define T_ERROR 1
#define T_EOF 2
#define T_ENDBUF 3
#define T_NEWLINE 4
#define T_CONTROL 5
#define T_EIGHTBIT 6
#define T_CNTL8BIT 7

#define printof(c) ((c) ^ 0100)

/*
 * called for any special output processing
 */

static_fn int vcat(char *states, Sfio_t *ip, Sfio_t *op, int flags) {
    unsigned char *cp;
    unsigned char *pp;
    unsigned char *cur;
    unsigned char *end;
    unsigned char *buf;
    unsigned char *nxt;
    int n;
    int line;
    int raw;
    int last;
    int c;
    int m;
    int any;
    int header;

    unsigned char meta[3];
    unsigned char tmp[32];

    meta[0] = 'M';
    last = -1;
    *(cp = buf = end = tmp) = 0;
    any = 0;
    header = flags & (B_FLAG | N_FLAG);
    line = 1;
    states[0] = T_ENDBUF;
    raw = !mbwide();
    for (;;) {
        cur = cp;
        if (raw) {
            while (!(n = states[*cp++])) {
                ;
            }
        } else {
            for (;;) {
                while (!(n = states[*cp++])) {
                    ;
                }
                if (n < T_CONTROL) break;
                pp = cp - 1;
                m = mblen((char *)pp, MB_LEN_MAX);
                if (m > 1) {
                    cp += m - 1;
                } else {
                    if (m > 0) break;
                    if (cur == pp) {
                        if (last > 0) {
                            *end = last;
                            last = -1;
                            c = end - pp + 1;
                            m = mblen((char *)pp, MB_LEN_MAX);
                            if (m == c) {
                                any = 1;
                                if (header) {
                                    header = 0;
                                    sfprintf(op, "%6d\t", line);
                                }
                                sfwrite(op, cur, m);
                                *(cp = cur = end) = 0;
                            } else {
                                memcpy(tmp, pp, c);
                                nxt = sfreserve(ip, SF_UNBOUND, 0);
                                if (!nxt) {
                                    states[0] = sfvalue(ip) ? T_ERROR : T_EOF;
                                    *(cp = end = tmp + sizeof(tmp) - 1) = 0;
                                    last = -1;
                                } else if ((n = sfvalue(ip)) <= 0) {
                                    states[0] = n ? T_ERROR : T_EOF;
                                    *(cp = end = tmp + sizeof(tmp) - 1) = 0;
                                    last = -1;
                                } else {
                                    cp = buf = nxt;
                                    end = buf + n - 1;
                                    last = *end;
                                    *end = 0;
                                }
                            mb:
                                n = end - cp + 1;
                                if (n >= (sizeof(tmp) - c)) n = sizeof(tmp) - c - 1;
                                memcpy(tmp + c, cp, n);
                                m = mblen((char *)tmp, MB_LEN_MAX);
                                if (m >= c) {
                                    any = 1;
                                    if (header) {
                                        header = 0;
                                        sfprintf(op, "%6d\t", line);
                                    }
                                    sfwrite(op, tmp, m);
                                    cur = cp += m - c;
                                }
                            }
                            continue;
                        }
                    } else {
                        cp = pp + 1;
                        n = 0;
                    }
                    break;
                }
            }
        }
        c = *--cp;
        m = cp - cur;
        if (m || n >= T_CONTROL) {
        flush:
            any = 1;
            if (header) {
                header = 0;
                sfprintf(op, "%6d\t", line);
            }
            if (m) sfwrite(op, cur, m);
        }
    special:
        switch (n) {
            case T_ERROR:
                if (cp < end) {
                    n = T_CONTROL;
                    goto flush;
                }
                return -1;
            case T_EOF:
                if (cp < end) {
                    n = T_CONTROL;
                    goto flush;
                }
                return 0;
            case T_ENDBUF:
                if (cp < end) {
                    n = T_CONTROL;
                    goto flush;
                }
                c = last;
                nxt = sfreserve(ip, SF_UNBOUND, 0);
                if (!nxt) {
                    *(cp = end = tmp + sizeof(tmp) - 1) = 0;
                    states[0] = (m = sfvalue(ip)) ? T_ERROR : T_EOF;
                    last = -1;
                } else if ((m = sfvalue(ip)) <= 0) {
                    *(cp = end = tmp + sizeof(tmp) - 1) = 0;
                    states[0] = m ? T_ERROR : T_EOF;
                    last = -1;
                } else {
                    buf = nxt;
                    end = buf + m - 1;
                    last = *end;
                    *end = 0;
                    cp = buf;
                }
                if (c >= 0) {
                    n = states[c];
                    if (!n) {
                        *(cur = tmp) = c;
                        m = 1;
                        goto flush;
                    }
                    if (raw || n < T_CONTROL) {
                        cp--;
                        goto special;
                    }
                    tmp[0] = c;
                    c = 1;
                    goto mb;
                }
                break;
            case T_CONTROL:
                do {
                    sfputc(op, '^');
                    sfputc(op, printof(c));
                } while (states[c = *++cp] == T_CONTROL);
                break;
            case T_CNTL8BIT:
                meta[1] = '^';
                do {
                    n = c & ~0200;
                    meta[2] = printof(n);
                    sfwrite(op, (char *)meta, 3);
                } while (states[c = *++cp] == T_CNTL8BIT && raw);
                break;
            case T_EIGHTBIT:
                meta[1] = '-';
                do {
                    meta[2] = c & ~0200;
                    sfwrite(op, (char *)meta, 3);
                } while (states[c = *++cp] == T_EIGHTBIT && raw);
                break;
            case T_NEWLINE:
                if (header && !(flags & B_FLAG)) sfprintf(op, "%6d\t", line);
                if (flags & E_FLAG) sfputc(op, '$');
                sfputc(op, '\n');
                if (!header || !(flags & B_FLAG)) line++;
                header = !(flags & S_FLAG);
                for (;;) {
                    n = states[*++cp];
                    if (n == T_ENDBUF) {
                        if (cp < end || last != '\n') break;
                        nxt = sfreserve(ip, SF_UNBOUND, 0);
                        if (!nxt) {
                            states[0] = sfvalue(ip) ? T_ERROR : T_EOF;
                            cp = end = tmp;
                            *cp-- = 0;
                            last = -1;
                        } else if ((n = sfvalue(ip)) <= 0) {
                            states[0] = n ? T_ERROR : T_EOF;
                            cp = end = tmp;
                            *cp-- = 0;
                            last = -1;
                        } else {
                            buf = nxt;
                            end = buf + n - 1;
                            last = *end;
                            *end = 0;
                            cp = buf - 1;
                        }
                    } else if (n != T_NEWLINE) {
                        break;
                    }
                    if (!(flags & S_FLAG) || any || header) {
                        any = 0;
                        header = 0;
                        if ((flags & (B_FLAG | N_FLAG)) == N_FLAG) sfprintf(op, "%6d\t", line);
                        if (flags & E_FLAG) sfputc(op, '$');
                        sfputc(op, '\n');
                    }
                    if (!(flags & B_FLAG)) line++;
                }
                header = flags & (B_FLAG | N_FLAG);
                break;
            default: { abort(); }
        }
    }
}

static const char *short_options = "bdenstuvABDEST";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"number-nonblank", optget_no_arg, NULL, 'b'},
    {"dos-input", optget_no_arg, NULL, 'd'},
    {"number", optget_no_arg, NULL, 'n'},
    {"unbuffer", optget_no_arg, NULL, 'u'},
    {"show-nonprinting", optget_no_arg, NULL, 'v'},
    {"print-chars", optget_no_arg, NULL, 'v'},
    {"show-all", optget_no_arg, NULL, 'A'},
    {"squeeze-blank", optget_no_arg, NULL, 'B'},
    {"dos-output", optget_no_arg, NULL, 'D'},
    {"show-ends", optget_no_arg, NULL, 'E'},
    {"silent", optget_no_arg, NULL, 'S'},
    {"show-blank", optget_no_arg, NULL, 'T'},
    {NULL, 0, NULL, 0}};

//
// Builtin `cat` command.
//
int b_cat(int argc, char **argv, Shbltin_t *context) {
    int n, opt;
    int flags = 0;
    char *cp;
    Sfio_t *fp;
    char *mode;
    int att;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    int dovcat = 0;
    char states[UCHAR_MAX + 1];

    if (cmdinit(argc, argv, context, 0)) return -1;
    att = !path_is_bsd_universe();
    mode = "r";
    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'A': {
                flags |= T_FLAG | E_FLAG | V_FLAG;
                break;
            }
            case 'B': {
                flags |= S_FLAG;
                break;
            }
            case 'b': {
                flags |= B_FLAG;
                break;
            }
            case 'd': {
                mode = "rt";
                break;
            }
            case 'D': {
                flags |= d_FLAG;
                break;
            }
            case 'E': {
                flags |= E_FLAG;
                break;
            }
            case 'e': {
                flags |= E_FLAG | V_FLAG;
                break;
            }
            case 'n': {
                flags |= N_FLAG;
                break;
            }
            case 's': {
                flags |= att ? F_FLAG : S_FLAG;
                break;
            }
            case 'S': {
                flags |= F_FLAG;
                break;
            }
            case 'T': {
                flags |= T_FLAG;
                break;
            }
            case 't': {
                flags |= T_FLAG | V_FLAG;
                break;
            }
            case 'u': {
                flags |= U_FLAG;
                break;
            }
            case 'v': {
                flags |= V_FLAG;
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

    memset(states, 0, sizeof(states));
    if (flags & V_FLAG) {
        memset(states, T_CONTROL, 32);
        states[RUBOUT] = T_CONTROL;
        memset(states + 0200, T_EIGHTBIT, 0200);
        memset(states + 0200, T_CNTL8BIT, 32);
        states[RUBOUT | 0200] = T_CNTL8BIT;
        states['\n'] = 0;
    }
    if (flags & T_FLAG) states['\t'] = T_CONTROL;
    states[0] = T_ENDBUF;
    if (att) {
        if (flags & V_FLAG) {
            states['\n' | 0200] = T_EIGHTBIT;
            if (!(flags & T_FLAG)) {
                states['\t'] = states['\f'] = 0;
                states['\t' | 0200] = states['\f' | 0200] = T_EIGHTBIT;
            }
        }
    } else if (flags) {
        if (!(flags & T_FLAG)) states['\t'] = 0;
    }
    if (flags & (V_FLAG | T_FLAG | N_FLAG | E_FLAG | B_FLAG | S_FLAG)) {
        states['\n'] = T_NEWLINE;
        dovcat = 1;
    }
    if (flags & d_FLAG) sfopen(sfstdout, NULL, "wt");
    if (!*argv) argv = stdin_argv;
    for (cp = *argv; cp; cp = *++argv) {
        if (!strcmp(cp, "-")) {
            fp = sfstdin;
            if (flags & D_FLAG) sfopen(fp, NULL, mode);
        } else if (!(fp = sfopen(NULL, cp, mode))) {
            if (!(flags & F_FLAG)) error(ERROR_system(0), "%s: cannot open", cp);
            error_info.errors = 1;
            continue;
        }
        if (flags & U_FLAG) sfsetbuf(fp, fp, -1);
        if (dovcat) {
            n = vcat(states, fp, sfstdout, flags);
        } else if (sfmove(fp, sfstdout, SF_UNBOUND, -1) >= 0 && sfeof(fp)) {
            n = 0;
        } else {
            n = -1;
        }
        if (fp != sfstdin) sfclose(fp);
        if (n < 0 && !ERROR_PIPE(errno) && errno != EINTR) {
            if (cp) {
                error(ERROR_system(0), "%s: read error", cp);
            } else {
                error(ERROR_system(0), "read error");
            }
        }
        if (sferror(sfstdout)) break;
    }
    if (sfsync(sfstdout)) error(ERROR_system(0), "write error");
    if (flags & d_FLAG) sfopen(sfstdout, NULL, "w");
    return error_info.errors;
}
