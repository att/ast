/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
// This is a program to execute 'execute only' and suid/sgid shell scripts.
// This program must be owned by root and must have the set uid bit set.
// It must not have the set group id bit set.  This program must be installed
// where the define parameter THISPROG indicates to work correctly on system V
//
//  Written by David Korn
//  AT&T Labs
//  Enhanced by Rob Stampfli
//

// The file name of the script to execute is argv[0].
// Argv[1] is the  program name.
// The basic idea is to open the script as standard input, set the effective
//   user and group id correctly, and then exec the shell.
// The complicated part is getting the effective uid of the caller and
//   setting the effective uid/gid.  The program which execs this program
//   may pass file descriptor FDIN as an open file with mode SPECIAL if
//   the effective user id is not the real user id.  The effective
//   user id for authentication purposes will be the owner of this
//   open file.  On systems without the setreuid() call, e[ug]id is set
//   by copying this program to a /tmp/file, making it a suid and/or sgid
//   program, and then execing this program.
// A forked version of this program waits until it can unlink the /tmp
//   file and then exits.  Actually, we fork() twice so the parent can
//   wait for the child to complete.  A pipe is used to guarantee that we
//   do not remove the /tmp file too soon.
//
#include <ast.h>
#include <error.h>
#include <ls.h>
#include <sig.h>
#include <sys/wait.h>
#include "version.h"

#define SPECIAL 04100  // setuid execute only by owner
#define FDIN 10        // must be same as /dev/fd below
#undef FDSYNC
#define FDSYNC 11    // used on sys5 to synchronize cleanup
#define FDVERIFY 12  // used to validate /tmp process
#undef BLKSIZE
#define BLKSIZE sizeof(char *) * 1024
#define THISPROG "/etc/suid_exec"
#define DEFSHELL "/bin/sh"

static void error_exit(const char *);
static int in_dir(const char *, const char *);
static int endsh(const char *);
static void setids(int, int, int);

static const char version[] = "\n@(#)$Id: suid_exec " SH_RELEASE " $\n";
static const char badopen[] = "cannot open";
static const char badexec[] = "cannot exec";
static const char devfd[] = "/dev/fd/10"; /* must match FDIN above */
static char tmpname[] = "/tmp/SUIDXXXXXX";
static char **arglist;

static char *shell;
static char *command;
static uid_t ruserid;
static uid_t euserid;
static gid_t rgroupid;
static gid_t egroupid;
static struct stat statb;

int main(int argc, char *argv[]) {
    int m, n;
    char *p;
    struct stat statx;
    int mode;
    uid_t effuid;
    gid_t effgid;
    UNUSED(argc);

    arglist = argv;
    if ((command = argv[1]) == 0) error_exit(badexec);
    ruserid = getuid();
    euserid = geteuid();
    rgroupid = getgid();
    egroupid = getegid();
    p = argv[0];
    // Open the script for reading first and then validate it.  This prevents someone from pulling a
    // switcheroo while we are validating.
    n = open(p, 0);
    if (n == FDIN) {
        n = dup(n);
        close(FDIN);
    }
    if (n < 0) error_exit(badopen);
    // Validate execution rights to this script.
    if (fstat(FDIN, &statb) < 0 || (statb.st_mode & ~S_IFMT) != SPECIAL) {
        euserid = ruserid;
    } else {
        euserid = statb.st_uid;
    }
    // Do it the easy way if you can.
    if (euserid == ruserid && egroupid == rgroupid) {
        if (access(p, X_OK) < 0) error_exit(badexec);
    } else {
        // Have to check access on each component.
        while (*p++) {
            if (*p == '/' || *p == 0) {
                m = *p;
                *p = 0;
                if (eaccess(argv[0], X_OK) < 0) error_exit(badexec);
                *p = m;
            }
        }
        p = argv[0];
    }
    if (fstat(n, &statb) < 0 || !S_ISREG(statb.st_mode)) error_exit(badopen);
    if (stat(p, &statx) < 0 || statb.st_ino != statx.st_ino || statb.st_dev != statx.st_dev) {
        error_exit(badexec);
    }
    if (stat(THISPROG, &statx) < 0 ||
        (statb.st_ino == statx.st_ino && statb.st_dev == statx.st_dev)) {
        error_exit(badexec);
    }
    close(FDIN);
    if (fcntl(n, F_DUPFD, FDIN) != FDIN) error_exit(badexec);
    close(n);

    // Compute the desired new effective user and group id.
    effuid = euserid;
    effgid = egroupid;
    mode = 0;
    if (statb.st_mode & S_ISUID) effuid = statb.st_uid;
    if (statb.st_mode & S_ISGID) effgid = statb.st_gid;

    // See if group needs setting.
    if (effgid != egroupid) {
        if (effgid != rgroupid || setgid(rgroupid) < 0) mode = S_ISGID;
    }

    // Now see if the uid needs setting.
    if (mode) {
        if (effuid != ruserid) mode |= S_ISUID;
    } else if (effuid) {
        if (effuid != ruserid || setuid(ruserid) < 0) mode = S_ISUID;
    }

    if (mode) setids(mode, effuid, effgid);
    // Only use SHELL if file is in trusted directory and ends in sh.
    shell = getenv("SHELL");
    if (shell == 0 || !endsh(shell) ||
        (!in_dir("/bin", shell) && !in_dir("/usr/bin", shell) && !in_dir("/usr/lbin", shell) &&
         !in_dir("/usr/local/bin", shell))) {
        shell = DEFSHELL;
    }
    argv[0] = command;
    argv[1] = (char *)devfd;
    execv(shell, argv);
    error_exit(badexec);
}

//
// Return true of shell ends in sh of ksh.
//
static int endsh(const char *shell) {
    while (*shell) shell++;
    if (*--shell != 'h' || *--shell != 's') return 0;
    if (*--shell == '/') return (1);
    if (*shell == 'k' && *--shell == '/') return 1;
    return 0;
}

//
// Return true of shell is in <dir> directory.
//
static int in_dir(const char *dir, const char *shell) {
    while (*dir) {
        if (*dir++ != *shell++) return 0;
    }
    // Return true if next character is a '/'.
    return *shell == '/';
}

static void error_exit(const char *message) {
    sfprintf(sfstdout, "%s: %s\n", command, message);
    exit(126);
}

#ifndef eaccess
//
// This version of access checks against effective uid and effective gid.
//
int eaccess(const char *name, int mode) {
    struct stat statb;
    if (stat(name, &statb) == 0) {
        if (euserid == 0) {
            if (!S_ISREG(statb.st_mode) || mode != 1) return 0;
            // Root needs execute permission for someone.
            mode = (S_IXUSR | S_IXGRP | S_IXOTH);
        } else if (euserid == statb.st_uid) {
            mode <<= 6;
        } else if (egroupid == statb.st_gid) {
            mode <<= 3;
        }
        // On some systems you can be in several groups.
        else {
            static int maxgroups;
            gid_t *groups = 0;
            int n;
            if (maxgroups == 0) {
                // First time.
                if ((maxgroups = getgroups(0, groups)) < 0) {
                    // Pre-POSIX system.
                    maxgroups = getconf("NGROUPS_MAX");
                }
            }
            groups = (gid_t *)malloc((maxgroups + 1) * sizeof(gid_t));
            n = getgroups(maxgroups, groups);
            while (--n >= 0) {
                if (groups[n] == statb.st_gid) {
                    mode <<= 3;
                    break;
                }
            }
        }
        if (statb.st_mode & mode) return 0;
    }
    return -1;
}

#endif  // eaccess

static void setids(int mode, int owner, int group) {
    if (mode & S_ISGID) setregid(rgroupid, group);

    // Set effective uid even if S_ISUID is not set.  This is because we are *really* executing EUID
    // root at this point.  Even if S_ISUID is not set, the value for owner that is passsed should
    // be correct.
    setreuid(ruserid, owner);
}
