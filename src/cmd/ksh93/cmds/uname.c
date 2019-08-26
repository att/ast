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
 * AT&T Research
 *
 * uname
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "builtins.h"
#include "error.h"
#include "option.h"
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

static bool output(bool sep, uint32_t flags, uint32_t flag, const char *value, const char *name) {
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

int b_uname(int argc, char **argv, Shbltin_t *context) {
    uint32_t flags = 0;
    bool sep = false;
    int n;
    char *t;
    struct utsname ut;
    char buf[257];

    if (cmdinit(argc, argv, context, 0)) return -1;
    while ((n = optget(argv, sh_optuname))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
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
            case ':':
                error(2, "%s", opt_info.arg);
                break;
            case '?':
                error(ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
        }
    }
    argv += opt_info.index;
    if (error_info.errors || *argv) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
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
