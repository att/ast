/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2012 AT&T Intellectual Property          *
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
//
// This is the list of resource limits controlled by ulimit.
// This command requires getrlimit(), vlimit(), or ulimit().
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>
#include <sys/resource.h>

#include "b_ulimit.h"

const char e_unlimited[] = "unlimited";
const char *e_units[] = {0, "block", "byte", "Kibyte", "second"};

const int shtab_units[] = {1, 512, 1, 1024, 1};

const Limit_t shtab_limits[] = {{"as", "address space limit", RLIMIT_AS, 'M', LIM_KBYTE},
                                {"core", "core file size", RLIMIT_CORE, 'c', LIM_BLOCK},
                                {"cpu", "cpu time", RLIMIT_CPU, 't', LIM_SECOND},
                                {"data", "data size", RLIMIT_DATA, 'd', LIM_KBYTE},
                                {"fsize", "file size", RLIMIT_FSIZE, 'f', LIM_BLOCK},
                                {"locks", "number of file locks", RLIMIT_LOCKS, 'x', LIM_COUNT},
                                {"memlock", "locked address space", RLIMIT_MEMLOCK, 'l', LIM_KBYTE},
                                {"msgqueue", "message queue size", RLIMIT_MSGQUEUE, 'q', LIM_KBYTE},
                                {"nice", "scheduling priority", RLIMIT_NICE, 'e', LIM_COUNT},
                                {"nofile", "number of open files", RLIMIT_NOFILE, 'n', LIM_COUNT},
                                {"nproc", "number of processes", RLIMIT_NPROC, 'u', LIM_COUNT},
                                {"pipe", "pipe buffer size", RLIMIT_PIPE, 'p', LIM_BYTE},
                                {"rss", "max memory size", RLIMIT_RSS, 'm', LIM_KBYTE},
                                {"rtprio", "max real time priority", RLIMIT_RTPRIO, 'r', LIM_COUNT},
                                {"sbsize", "socket buffer size", RLIMIT_SBSIZE, 'b', LIM_BYTE},
                                {"sigpend", "signal queue size", RLIMIT_SIGPENDING, 'i', LIM_COUNT},
                                {"stack", "stack size", RLIMIT_STACK, 's', LIM_KBYTE},
                                {"swap", "swap size", RLIMIT_SWAP, 'w', LIM_KBYTE},
                                {"threads", "number of threads", RLIMIT_PTHREAD, 'T', LIM_COUNT},
                                {"vmem", "process size", RLIMIT_VMEM, 'v', LIM_KBYTE},
                                {NULL, NULL, RLIMIT_UNKNOWN, '\0', LIM_COUNT}};
