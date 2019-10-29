/*
 * Copyright (c) 2012, Oracle and/or its affiliates. All rights reserved.
 */

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
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
/*
 * David Korn
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * cmp
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"

#define CMP_VERBOSE 0x01
#define CMP_SILENT 0x02
#define CMP_CHARS 0x04
#define CMP_BYTES 0x08

static const char *short_options = "bcd:i:ln:s";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"print-bytes", optget_no_arg, NULL, 'b'},
    {"print-chars", optget_no_arg, NULL, 'c'},
    {"differences", optget_required_arg, NULL, 'd'},
    {"ignore-initial", optget_required_arg, NULL, 'i'},
    {"skip", optget_required_arg, NULL, 'i'},
    {"verbose", optget_no_arg, NULL, 'l'},
    {"bytes", optget_required_arg, NULL, 'n'},
    {"count", optget_required_arg, NULL, 'n'},
    {"quiet", optget_no_arg, NULL, 's'},
    {"silent", optget_no_arg, NULL, 's'},
    {NULL, 0, NULL, 0}};

static_fn void pretty(Sfio_t *out, int o, int delim, int flags) {
    int m;
    char *s;
    char buf[10];

    s = buf;
    if ((flags & CMP_BYTES) || !(flags & CMP_CHARS)) {
        *s++ = ' ';
        if ((flags & CMP_CHARS) && delim != -1) *s++ = ' ';
        *s++ = '0' + ((o >> 6) & 07);
        *s++ = '0' + ((o >> 3) & 07);
        *s++ = '0' + (o & 07);
    }
    if (flags & CMP_CHARS) {
        *s++ = ' ';
        if (o & 0x80) {
            m = 1;
            *s++ = 'M';
            o &= 0x7f;
        } else {
            m = 0;
        }
        if (isspace(o) || !isprint(o)) {
            if (!m) *s++ = ' ';
            *s++ = '^';
            o ^= 0x40;
        } else if (m) {
            *s++ = '-';
        } else {
            *s++ = ' ';
            *s++ = ' ';
        }
        *s++ = o;
    }
    *s = 0;
    sfputr(out, buf, delim);
}

/*
 * compare two files
 */

static_fn int cmp(const char *file1, Sfio_t *f1, const char *file2, Sfio_t *f2, int flags,
                  Sfoff_t count, Sfoff_t differences) {
    int c1;
    int c2;
    unsigned char *p1 = NULL;
    unsigned char *p2 = NULL;
    Sfoff_t lines = 1;
    unsigned char *e1 = NULL;
    unsigned char *e2 = NULL;
    Sfoff_t pos = 0;
    int n1 = 0;
    int ret = 0;
    unsigned char *last;

    for (;;) {
        if ((c1 = e1 - p1) <= 0) {
            if (count > 0 && !(count -= n1)) return ret;
            p1 = (unsigned char *)sfreserve(f1, SF_UNBOUND, 0);
            if (!p1 || (c1 = sfvalue(f1)) <= 0) {
                if (sferror(f1)) {
                    error(ERROR_exit(2), "read error on %s", file1);
                }
                if ((e2 - p2) > 0 || (sfreserve(f2, SF_UNBOUND, 0) && sfvalue(f2) > 0)) {
                    ret = 1;
                    if (!(flags & CMP_SILENT)) error(ERROR_exit(1), "EOF on %s", file1);
                }
                if (sferror(f2)) {
                    error(ERROR_exit(2), "read error on %s", file2);
                }
                return ret;
            }
            if (count > 0 && c1 > count) c1 = (int)count;
            e1 = p1 + c1;
            n1 = c1;
        }
        if ((c2 = e2 - p2) <= 0) {
            p2 = (unsigned char *)sfreserve(f2, SF_UNBOUND, 0);
            if (!p2 || (c2 = sfvalue(f2)) <= 0) {
                if (sferror(f2)) {
                    error(ERROR_exit(2), "read error on %s", file2);
                }
                if (!(flags & CMP_SILENT)) error(ERROR_exit(1), "EOF on %s", file2);
                return 1;
            }
            e2 = p2 + c2;
        }
        if (c1 > c2) c1 = c2;
        pos += c1;
        if (flags & CMP_SILENT) {
            if (memcmp(p1, p2, c1)) return 1;
            p1 += c1;
            p2 += c1;
        } else {
            last = p1 + c1;
            while (p1 < last) {
                if ((c1 = *p1++) != *p2++) {
                    if (differences >= 0) {
                        if (!differences) return 1;
                        differences--;
                    }
                    if (flags & CMP_VERBOSE) {
                        sfprintf(sfstdout, "%6I*d", sizeof(pos), pos - (last - p1));
                    } else {
                        sfprintf(sfstdout, "%s %s differ: char %I*d, line %I*u", file1, file2,
                                 sizeof(pos), pos - (last - p1), sizeof(lines), lines);
                    }
                    if (flags & (CMP_BYTES | CMP_CHARS | CMP_VERBOSE)) {
                        sfputc(sfstdout, (flags & CMP_VERBOSE) ? ' ' : ',');
                        pretty(sfstdout, c1, -1, flags);
                        pretty(sfstdout, *(p2 - 1), '\n', flags);
                    } else {
                        sfputc(sfstdout, '\n');
                    }
                    if (!differences || (differences < 0 && !(flags & CMP_VERBOSE))) return 1;
                    ret = 1;
                }
                if (c1 == '\n') lines++;
            }
        }
    }
}

//
//  The `cmp` builtin command.
//
int b_cmp(int argc, char **argv, Shbltin_t *context) {
    char *s;
    char *e;
    char *file1;
    char *file2;
    int opt;
    int n;
    struct stat s1;
    struct stat s2;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    Sfio_t *f1 = NULL;
    Sfio_t *f2 = NULL;
    Sfoff_t o1 = 0;
    Sfoff_t o2 = 0;
    Sfoff_t count = -1;
    Sfoff_t differences = -1;
    int flags = 0;

    if (cmdinit(argc, argv, context, 0)) return -1;
    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'b': {
                flags |= CMP_BYTES;
                break;
            }
            case 'c': {
                flags |= CMP_CHARS;
                break;
            }
            case 'd': {
                flags |= CMP_VERBOSE;
                char *cp;
                differences = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || differences < 0) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid -d value", optget_arg);
                    return 2;
                }
                break;
            }
            case 'i': {
                o1 = strtoll(optget_arg, &e, 0);
                if (*e == ':') {
                    o2 = strtoll(e + 1, &e, 0);
                } else {
                    o2 = o1;
                }
                if (*e) {
                    builtin_usage_error(shp, cmd, "%s: skip1:skip2 expected", optget_arg);
                    return 2;
                }
                break;
            }
            case 'l': {
                flags |= CMP_VERBOSE;
                break;
            }
            case 'n': {
                char *cp;
                count = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || count < 0) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid -n value", optget_arg);
                    return 2;
                }
                break;
            }
            case 's': {
                flags |= CMP_SILENT;
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
    if (!(file1 = *argv++) || !(file2 = *argv++)) {
        builtin_usage_error(shp, cmd, "expected two file name arguments");
        return 2;
    }
    n = 2;

    if (!strcmp(file1, "-")) {
        f1 = sfstdin;
    } else if (!(f1 = sfopen(NULL, file1, "r"))) {
        if (!(flags & CMP_SILENT)) error(ERROR_system(0), "%s: cannot open", file1);
        goto done;
    }

    if (!strcmp(file2, "-")) {
        f2 = sfstdin;
    } else if (!(f2 = sfopen(NULL, file2, "r"))) {
        if (!(flags & CMP_SILENT)) error(ERROR_system(0), "%s: cannot open", file2);
        goto done;
    }
    s = *argv++;
    if (s) {
        o1 = strtoll(s, &e, 0);
        if (*e) {
            error(ERROR_exit(0), "%s: %s: invalid skip", file1, s);
            goto done;
        }
        s = *argv++;
        if (s) {
            o2 = strtoll(s, &e, 0);
            if (*e) {
                error(ERROR_exit(0), "%s: %s: invalid skip", file2, s);
                goto done;
            }
        }
        if (*argv) {
            builtin_usage_error(shp, cmd, "too many arguments");
            goto done;
        }
    }
    if (o1 && sfseek(f1, o1, SEEK_SET) != o1) {
        if (!(flags & CMP_SILENT)) error(ERROR_exit(0), "EOF on %s", file1);
        n = 1;
        goto done;
    }
    if (o2 && sfseek(f2, o2, SEEK_SET) != o2) {
        if (!(flags & CMP_SILENT)) error(ERROR_exit(0), "EOF on %s", file2);
        n = 1;
        goto done;
    }
    if (fstat(sffileno(f1), &s1)) {
        error(ERROR_system(0), "%s: cannot stat", file1);
    } else if (fstat(sffileno(f2), &s2)) {
        error(ERROR_system(0), "%s: cannot stat", file1);
    } else if (s1.st_ino == s2.st_ino && s1.st_dev == s2.st_dev && o1 == o2) {
        n = 0;
    } else {
        n = ((flags & CMP_SILENT) && S_ISREG(s1.st_mode) && S_ISREG(s2.st_mode) &&
             (s1.st_size - o1) != (s2.st_size - o2))
                ? 1
                : cmp(file1, f1, file2, f2, flags, count, differences);
    }
done:
    if (f1 && f1 != sfstdin) sfclose(f1);
    if (f2 && f2 != sfstdin) sfclose(f2);
    return n;
}
