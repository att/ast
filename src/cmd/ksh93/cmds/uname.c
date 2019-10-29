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
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "builtins.h"
#include "error.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"

static const char *hosttype = HOSTTYPE;

#define OPT_system (1 << 0)
#define OPT_nodename (1 << 1)
#define OPT_release (1 << 2)
#define OPT_version (1 << 3)
#define OPT_machine (1 << 4)
#define OPT_processor (1 << 5)
#define OPT_implementation (1 << 6)
#define OPT_operating_system (1 << 7)
#define OPT_hostid (1 << 8)
#define OPT_domain (1 << 10)

#define OPT_ALL 8
#define OPT_all (1L << 29)

static const char *short_options = "+:adhimnoprsv";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"all", 0, NULL, 'a'},
    {"system", 0, NULL, 's'},
    {"sysname", 0, NULL, 's'},
    {"kernel-name", 0, NULL, 's'},
    {"nodename", 0, NULL, 'n'},
    {"release", 0, NULL, 'r'},
    {"kernel-release", 0, NULL, 'r'},
    {"version", 0, NULL, 'v'},
    {"kernel-version", 0, NULL, 'v'},
    {"machine", 0, NULL, 'm'},
    {"processor", 0, NULL, 'p'},
    {"implementation", 0, NULL, 'i'},
    {"platform", 0, NULL, 'i'},
    {"hardware-platform", 0, NULL, 'i'},
    {"operating-system", 0, NULL, 'o'},
    {"id", 0, NULL, 'h'},
    {"host-id", 0, NULL, 'h'},
    {"domain", 0, NULL, 'd'},
    {NULL, 0, NULL, 0}};

static_fn bool output(bool sep, uint32_t flags, uint32_t flag, const char *value,
                      const char *name) {
    if (!(flags & flag)) return sep;

    if (*value || flags & OPT_all) {
        if (sep) {
            sfputc(sfstdout, ' ');
        } else {
            sep = true;
        }
        if (*value) {
            sfputr(sfstdout, value, -1);
        } else {
            sfprintf(sfstdout, "[%s]", name);
        }
    }

    return sep;
}

//
// The "uname" builtin.
//
int b_uname(int argc, char **argv, Shbltin_t *context) {
    uint32_t flags = 0;
    bool sep = false;
    int opt;
    char *t;
    struct utsname ut;
    char buf[257];
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    if (cmdinit(argc, argv, context, 0)) return -1;
    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'a':
                flags |= OPT_all | ((1L << OPT_ALL) - 1);
                break;
            case 'd':
                flags |= OPT_domain;
                break;
            case 'h':
                flags |= OPT_hostid;
                break;
            case 'i':
                flags |= OPT_implementation;
                break;
            case 'm':
                flags |= OPT_machine;
                break;
            case 'n':
                flags |= OPT_nodename;
                break;
            case 'o':
                flags |= OPT_operating_system;
                break;
            case 'p':
                flags |= OPT_processor;
                break;
            case 'r':
                flags |= OPT_release;
                break;
            case 's':
                flags |= OPT_system;
                break;
            case 'v':
                flags |= OPT_version;
                break;
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

    if (*argv) {
        builtin_usage_error(shp, cmd, "unexpected args");
        return 2;
    }

    if (!flags) flags = OPT_system;
    memset(&ut, 0, sizeof(ut));
    if (uname(&ut) < 0) {
        error(ERROR_usage(2), "information unavailable");
        __builtin_unreachable();
    }
    sep = output(sep, flags, OPT_system, ut.sysname, "sysname");
    if (flags & OPT_nodename) {
        sep = output(sep, flags, OPT_nodename, ut.nodename, "nodename");
    }
    sep = output(sep, flags, OPT_release, ut.release, "release");
    sep = output(sep, flags, OPT_version, ut.version, "version");
    sep = output(sep, flags, OPT_machine, ut.machine, "machine");
    if (flags & OPT_processor) {
        sep = output(sep, flags, OPT_processor, ut.machine, "processor");
    }
    if (flags & OPT_implementation) {
        t = strchr(hosttype, '.');
        if (t) {
            t++;
        } else {
            t = (char *)hosttype;
        }
        if (strlcpy(buf, t, sizeof(buf)) >= sizeof(buf)) {
            // TODO: Figure out what to do if the source is longer than the destination.
            // It should be a can't happen situation but what to do if it does happen?
            ;  // empty body
        }
        sep = output(sep, flags, OPT_implementation, buf, "implementation");
    }
    if (flags & OPT_operating_system) {
        sep = output(sep, flags, OPT_operating_system, ut.sysname, "operating-system");
    }
    if (flags & OPT_hostid) {
#if _mem_idnumber_utsname
        sep = output(sep, flags, OPT_hostid, ut.idnumber, "hostid");
#else
        sfsprintf(buf, sizeof(buf), "%08x", gethostid());
        sep = output(sep, flags, OPT_hostid, buf, "hostid");
#endif
    }
    if (flags & OPT_domain) {
#if AST_SUNOS
        // TODO: Fix this for Solaris.
        sep = output(sep, flags, OPT_domain, "domain.unknown", "domain");
#else
        getdomainname(buf, sizeof(buf));
        sep = output(sep, flags, OPT_domain, buf, "domain");
#endif
    }
    if (sep) sfputc(sfstdout, '\n');

    return error_info.errors;
}
