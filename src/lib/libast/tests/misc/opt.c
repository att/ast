/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "error.h"
#include "option.h"
#include "sfio.h"

struct Info_s;
typedef struct Info_s Info_t;

struct Info_s {
    Info_t *next;
    char *name;
    char *value;
};

static Info_t *info;

static int infof(Opt_t *op, Sfio_t *sp, const char *s, Optdisc_t *dp) {
    UNUSED(op);
    UNUSED(dp);
    Info_t *ip;

    for (ip = info; ip; ip = ip->next) {
        if (!strcmp(s, ip->name)) return sfprintf(sp, "%s", ip->value);
    }
    if (*s == ':') return sfprintf(sp, "%s", *(s + 1) == 'n' ? "" : (s + 2));
    if (!strcmp(s, "options")) {
        return sfprintf(sp,
                        "[Z:zoom?Do it as fast as possible.]\fmore#1\f\fmore#2\f[B:boom?Dump into "
                        "\afile\a.]:[file]");
    }
    if (!strcmp(s, "zero")) return sfprintf(sp, "[+yabba?dabba][+doo?aroni]");
    if (!strcmp(s, "more#1")) return sfprintf(sp, "[C:cram?Cram as much as possible.]\fmore#3\f");
    if (!strcmp(s, "more#2")) return sfprintf(sp, "\fmore#4\f[D:dump?Dump as much as possible.]");
    if (!strcmp(s, "more#3")) return sfprintf(sp, "[K:kill?kill all processes.]");
    if (!strcmp(s, "more#4")) {
        return sfprintf(sp, "[F:fudge?Fudge the statistics to satisfy everyone.]");
    }
    if (!strcmp(s, "more#5")) {
        return sfprintf(
            sp, "\bred\b, \borange\b, \byellow\b, \bgreen\b, \bblue\b, \bindigo\b, \bviolet\b");
    }
    if (!strcmp(s, "more#6")) return sfprintf(sp, "\bred\b");
    return sfprintf(sp, "<* %s info ok *>", s);
}

int main(int argc, char **argv) {
    UNUSED(argc);
    int n;
    int ext;
    int ostr;
    int str;
    int loop;
    Info_t *ip;
    char *s;
    char *command;
    char *usage;
    char **extra;
    char **oargv;
    Optdisc_t disc;

    error_info.id = "opt";
    ast_setlocale(LC_ALL, "");
    error(-1, "test");
    extra = 0;
    ext = 0;
    str = 0;
    while (*(argv + 1)) {
        command = *(argv + 1);
        if (*command == '=' && (s = strchr(command + 1, '='))) {
            argv++;
            *s++ = 0;
            command++;
            ip = calloc(1, sizeof(Info_t));
            ip->name = command;
            ip->value = s;
            ip->next = info;
            info = ip;
        } else if (!strcmp(command, "-")) {
            argv++;
            str = 1;
        } else if (!strcmp(command, "-+")) {
            argv++;
            ast.locale.set |= (1 << LC_MESSAGES);
            error_info.translate = translate;
        } else if (!strcmp(command, "+") && *(argv + 2)) {
            ext += 2;
            argv += 2;
            if (!extra) extra = argv;
        } else {
            break;
        }
    }
    if (!(command = *++argv) || !(usage = *++argv)) {
        error(ERROR_USAGE | 4, "[ - | + usage ... ] command-name usage-string [ arg ... ]");
    }
    argv += str;
    error_info.id = command;
    memset(&disc, 0, sizeof(disc));
    disc.version = OPT_VERSION;
    disc.infof = infof;
    opt_info.disc = &disc;
    loop = strncmp(usage, "[-1c", 4) ? 0 : 3;
    oargv = argv;
    ostr = str;
    for (;;) {
        for (;;) {
            if (!str) {
                if (!(n = optget(argv, usage))) break;
            } else if (!(n = optstr(*argv, usage))) {
                if (!*++argv) break;
                continue;
            }
            if (loop) sfprintf(sfstdout, "[%d] ", loop);
            if (n == '?') {
                sfprintf(sfstdout, "return=%c option=%s name=%s num=%I*d\n", n, opt_info.option,
                         opt_info.name, sizeof(opt_info.number), opt_info.number);
                error(ERROR_USAGE | 4, "%s", opt_info.arg);
            } else if (n == ':') {
                sfprintf(sfstdout, "return=%c option=%s name=%s num=%I*d", n, opt_info.option,
                         opt_info.name, sizeof(opt_info.number), opt_info.number);
                if (!opt_info.option[0]) sfprintf(sfstdout, " str=%s", argv[opt_info.index - 1]);
                sfputc(sfstdout, '\n');
                error(2, "%s", opt_info.arg);
            } else if (n > 0) {
                sfprintf(sfstdout, "return=%c option=%s name=%s arg%-.1s=%s num=%I*d\n", n,
                         opt_info.option, opt_info.name, &opt_info.assignment, opt_info.arg,
                         sizeof(opt_info.number), opt_info.number);
            } else {
                sfprintf(sfstdout, "return=%d option=%s name=%s arg%-.1s=%s num=%I*d\n", n,
                         opt_info.option, opt_info.name, &opt_info.assignment, opt_info.arg,
                         sizeof(opt_info.number), opt_info.number);
            }
            if (extra) {
                for (n = 0; n < ext; n += 2) (void)optget(NULL, extra[n]);
                extra = 0;
            }
        }
        if (!str && *(argv += opt_info.index)) {
            while (*argv) {
                command = *argv++;
                if (loop) sfprintf(sfstdout, "[%d] ", loop);
                sfprintf(sfstdout, "argument=%d value=\"%s\"\n", ++str, command);
            }
        }
        if (--loop <= 0) break;
        argv = oargv;
        str = ostr;
        opt_info.index = 0;
    }
    return error_info.errors != 0;
}
