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

#include <ctype.h>
#include <string.h>
#include <sys/utsname.h>

#if _hdr_unistd
#include <unistd.h>
#endif

#include "ast.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

static const char usage[] =
    "[-?\n@(#)$Id: uname (AT&T Research) 2007-04-19 $\n]" USAGE_LICENSE
    "[+NAME?uname - identify the current system ]"
    "[+DESCRIPTION?By default \buname\b writes the operating system name to"
    "	standard output. When options are specified, one or more"
    "	system characteristics are written to standard output, space"
    "	separated, on a single line. When more than one option is specified"
    "	the output is in the order specfied by the \b-A\b option below."
    "	Unsupported option values are listed as \a[option]]\a. If any unknown"
    "	options are specified then the local \b/usr/bin/uname\b is called.]"
    "[+?If any \aname\a operands are specified then the \bsysinfo\b(2) values"
    "	for each \aname\a are listed, separated by space, on one line."
    "	\bgetconf\b(1), a pre-existing \astandard\a interface, provides"
    "	access to the same information; vendors should spend more time"
    "	using standards than inventing them.]"
    "[+?Selected information is printed in the same order as the options below.]"
    "[a:all?Equivalent to \b-snrvmpio\b.]"
    "[s:system|sysname|kernel-name?The detailed kernel name. This is the default.]"
    "[n:nodename?The hostname or nodename.]"
    "[r:release|kernel-release?The kernel release level.]"
    "[v:version|kernel-version?The kernel version level.]"
    "[m:machine?The name of the hardware type the system is running on.]"
    "[p:processor?The name of the processor instruction set architecture.]"
    "[i:implementation|platform|hardware-platform?The hardware implementation;"
    "	this is \b--host-id\b on some systems.]"
    "[o:operating-system?The generic operating system name.]"
    "[h:host-id|id?The host id in hex.]"
    "[d:domain?The domain name returned by \agetdomainname\a(2).]"
    "[R:extended-release?The extended release name.]"
    "[A:everything?Equivalent to \b-snrvmpiohdR\b.]"
    "[f:list?List all \bsysinfo\b(2) names and values, one per line.]"
    "[S:sethost?Set the hostname or nodename to \aname\a. No output is"
    "	written to standard output.]:[name]"
    "\n"
    "\n[ name ... ]\n"
    "\n"
    "[+SEE ALSO?\bhostname\b(1), \bgetconf\b(1), \buname\b(2),"
    "	\bsysconf\b(2), \bsysinfo\b(2)]";

static const char hosttype[] = HOSTTYPE;

#define OPT_system (1 << 0)
#define OPT_nodename (1 << 1)
#define OPT_release (1 << 2)
#define OPT_version (1 << 3)
#define OPT_machine (1 << 4)
#define OPT_processor (1 << 5)

#define OPT_STANDARD 6

#define OPT_implementation (1 << 6)
#define OPT_operating_system (1 << 7)

#define OPT_ALL 8

#define OPT_hostid (1 << 8)
#define OPT_domain (1 << 10)
#define OPT_extended_release (1 << 13)

#define OPT_TOTAL 14

#define OPT_all (1L << 29)
#define OPT_total (1L << 30)
#define OPT_standard ((1 << OPT_STANDARD) - 1)

#define output(f, v, u)                                                                    \
    do {                                                                                   \
        if ((flags & (f)) &&                                                               \
            (*(v) || ((flags & (OPT_all | OPT_total)) == OPT_all && ((f)&OPT_standard)) || \
             !(flags & (OPT_all | OPT_total)))) {                                          \
            if (sep) {                                                                     \
                sfputc(sfstdout, ' ');                                                     \
            } else {                                                                       \
                sep = 1;                                                                   \
            }                                                                              \
            if (*(v)) {                                                                    \
                sfputr(sfstdout, v, -1);                                                   \
            } else {                                                                       \
                sfprintf(sfstdout, "[%s]", u);                                             \
            }                                                                              \
        }                                                                                  \
    } while (0)

int b_uname(int argc, char **argv, Shbltin_t *context) {
    long flags = 0;
    int sep = 0;
    int n;
    char *s;
    char *t;
    char *e;
    char *sethost = 0;
    int list = 0;
    struct utsname ut;
    char buf[257];

    if (cmdinit(argc, argv, context, 0)) return -1;
    for (;;) {
        switch (optget(argv, usage)) {
            case 'a':
                flags |= OPT_all | ((1L << OPT_ALL) - 1);
                continue;
            case 'd':
                flags |= OPT_domain;
                continue;
            case 'f':
                list = 1;
                continue;
            case 'h':
                flags |= OPT_hostid;
                continue;
            case 'i':
                flags |= OPT_implementation;
                continue;
            case 'm':
                flags |= OPT_machine;
                continue;
            case 'n':
                flags |= OPT_nodename;
                continue;
            case 'o':
                flags |= OPT_operating_system;
                continue;
            case 'p':
                flags |= OPT_processor;
                continue;
            case 'r':
                flags |= OPT_release;
                continue;
            case 's':
                flags |= OPT_system;
                continue;
            case 'v':
                flags |= OPT_version;
                continue;
            case 'A':
                flags |= OPT_total | ((1L << OPT_TOTAL) - 1);
                continue;
            case 'R':
                flags |= OPT_extended_release;
                continue;
            case 'S':
                sethost = opt_info.arg;
                continue;
            case ':':
                s = "/usr/bin/uname";
                if (strcmp(argv[0], s) != 0 && (!eaccess(s, X_OK) || !eaccess(s += 4, X_OK))) {
                    argv[0] = s;
                    return bltin_run(context, argc, argv);
                }
                error(2, "%s", opt_info.arg);
                break;
            case '?':
                error(ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
        }
        break;
    }
    argv += opt_info.index;
    if (error_info.errors || (*argv && (flags || sethost)) || (sethost && flags)) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (sethost) {
        if (sethostname(sethost, strlen(sethost) + 1)) {
            error(ERROR_system(1), "%s: cannot set host name", sethost);
            __builtin_unreachable();
        }
    } else if (list) {
        astconflist(
            sfstdout, NULL,
            ASTCONF_base | ASTCONF_defined | ASTCONF_lower | ASTCONF_quote | ASTCONF_matchcall,
            "CS|SI");
    } else if (*argv) {
        e = &buf[sizeof(buf) - 1];
        while (*argv) {
            s = *argv++;
            t = buf;
            *t++ = 'C';
            *t++ = 'S';
            *t++ = '_';
            while (t < e && (n = *s++)) *t++ = islower(n) ? toupper(n) : n;
            *t = 0;
            sfprintf(sfstdout, "%s%c",
                     *(t = astconf(buf, NULL, NULL))
                         ? t
                         : *(t = astconf(buf + 3, NULL, NULL)) ? t : "unknown",
                     *argv ? ' ' : '\n');
        }
    } else {
        if (!flags) flags = OPT_system;
        memset(&ut, 0, sizeof(ut));
        if (uname(&ut) < 0) {
            error(ERROR_usage(2), "information unavailable");
            __builtin_unreachable();
        }
        output(OPT_system, ut.sysname, "sysname");
        if (flags & OPT_nodename) {
            output(OPT_nodename, ut.nodename, "nodename");
        }
        output(OPT_release, ut.release, "release");
        output(OPT_version, ut.version, "version");
        output(OPT_machine, ut.machine, "machine");
        if (flags & OPT_processor) {
            s = astconf("ARCHITECTURE", NULL, NULL);
            if (!*s) s = ut.machine;
            output(OPT_processor, s, "processor");
        }
        if (flags & OPT_implementation) {
            s = astconf("PLATFORM", NULL, NULL);
            if (!*s) {
                s = astconf("HW_NAME", NULL, NULL);
                if (!*s) {
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
                    s = buf;
                }
            }
            output(OPT_implementation, s, "implementation");
        }
        if (flags & OPT_operating_system) {
            s = astconf("OPERATING_SYSTEM", NULL, NULL);
            if (!*s) {
#ifdef _UNAME_os_DEFAULT
                s = _UNAME_os_DEFAULT;
#else
                s = ut.sysname;
#endif
            }
            output(OPT_operating_system, s, "operating-system");
        }
        if (flags & OPT_extended_release) {
            s = astconf("RELEASE", NULL, NULL);
            output(OPT_extended_release, s, "extended-release");
        }
#if _mem_idnumber_utsname
        output(OPT_hostid, ut.idnumber, "hostid");
#else
        if (flags & OPT_hostid) {
            s = astconf("HW_SERIAL", NULL, NULL);
            if (!(*s)) {
                sfsprintf(s = buf, sizeof(buf), "%08x", gethostid());
            }
            output(OPT_hostid, s, "hostid");
        }
#endif
        if (flags & OPT_domain) {
            s = astconf("SRPC_DOMAIN", NULL, NULL);
            if (!(*s)) {
                getdomainname(buf, sizeof(buf));
                s = buf;
            }
            output(OPT_domain, s, "domain");
        }
        if (sep) sfputc(sfstdout, '\n');
    }

    return error_info.errors;
}
