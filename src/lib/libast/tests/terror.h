/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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
#ifndef _TERROR_H
#define _TERROR_H 1

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "aso.h"
#include "ast_assert.h"

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif

#ifndef elementsof
#define elementsof(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef TIMEOUT
#define TIMEOUT 0 /* timeout in minutes */
#endif

static int Tstall;
static int Tstchild;
static int Tstline;
static int Tsttimeout = TIMEOUT;
static char Tstfile[256][256];

#define terror(...)                                  \
    do {                                             \
        (Tstline = __LINE__), tsterror(__VA_ARGS__); \
    } while (0)
#define tinfo(...)                                  \
    do {                                            \
        (Tstline = __LINE__), tstinfo(__VA_ARGS__); \
    } while (0)
#define tpause(...)                                  \
    do {                                             \
        (Tstline = __LINE__), tstpause(__VA_ARGS__); \
    } while (0)
#define tsuccess(...)                                  \
    do {                                               \
        (Tstline = __LINE__), tstsuccess(__VA_ARGS__); \
    } while (0)

#define tchild() ((Tstline = __LINE__), tstchild(argv))
#define topts() ((Tstline = __LINE__), tstopts(argv))
#define tshared(n) ((Tstline = __LINE__), tstshared(n))
#define twait(p, n) ((Tstline = __LINE__), tstwait(p, n))

#define tresource(a, b)

#ifdef DEBUG
#define TSTDEBUG(x) (Tstline = __LINE__), tstwarn x
#else
#define TSTDEBUG(x)
#endif

#ifndef tmain
#define tmain() int main(int argc, char **argv)
#endif /*tmain*/

#ifndef texit
#define texit(v)    \
    {               \
        tcleanup(); \
        exit(v);    \
    }
#endif

static void tcleanup(void) {
    // This used to cleanup temp files but that is now down by the framework that runs unit tests.
}

__attribute__((unused)) static void tnote(char *note) {
    char buf[1024];

#if _SFIO_H
    sfsprintf(buf, sizeof(buf), "[%s] ", note);
#else
    sprintf(buf, "[%s] ", note);
#endif
    write(2, buf, strlen(buf));
}

static void tstputmesg(int line, char *form, va_list args) {
    char *s, buf[1024];
    size_t n;

    for (n = 0; n < sizeof(buf); ++n) buf[n] = 0;

    s = buf;
#if _SFIO_H
    n = 0;
#endif
    if (line >= 0) {
#if _SFIO_H
        sfsprintf(s, sizeof(buf), "\tLine=%d: ", line);
#else
        sprintf(s, "\tLine=%d: ", line);
#endif
        s += (n = strlen(s));
    }
#if _SFIO_H
    sfvsprintf(s, sizeof(buf) - n, form, args);
#else
    vsprintf(s, form, args);
#endif

    if ((n = strlen(buf)) > 0) {
        if (buf[n - 1] != '\n') {
            buf[n] = '\n';
            n += 1;
        }
        write(2, buf, n);
    }
}

__attribute__((noreturn)) void tsterror(char *form, ...) {
    char failform[1024];

    va_list args;
    va_start(args, form);

    if (form) {
#if _SFIO_H
        sfsprintf(failform, sizeof(failform), "FAILED %s [errno=%d]", form, errno);
#else
        sprintf(failform, "FAILED %s [errno=%d]", form, errno);
#endif
        tstputmesg(Tstline, failform, args);
    }

    va_end(args);

    if (Tstchild) {
        signal(SIGTERM, SIG_IGN);
        kill(0, SIGTERM);
    }
    texit(1);
}

void tstsuccess(char *form, ...) {
    va_list args;
    va_start(args, form);

    tstputmesg(Tstline, form, args);

    va_end(args);

    texit(0);
}

void tstinfo(char *form, ...) {
#ifdef INFO
    va_list args;
    va_start(args, form);

    tstputmesg(Tstline, form, args);

    va_end(args);
#else
    UNUSED(form);
#endif
}

void tstwarn(char *form, ...) {
    va_list args;
    va_start(args, form);

    tstputmesg(Tstline, form, args);

    va_end(args);
}

void tstpause(char *form, ...) {
#ifdef INFO
    char pauseform[1024];
#endif

    va_list args;
    va_start(args, form);

#ifdef INFO
#if _SFIO_H
    sfsprintf(pauseform, sizeof(pauseform), "Pausing: %s", form);
#else
    sprintf(pauseform, "Pausing: %s", form);
#endif
    tstputmesg(Tstline, pauseform, args);
#endif

    va_end(args);

    sleep(15);
}

__attribute__((unused)) static int asoerror(int type, const char *mesg) {
    static unsigned long hit;

    if (hit & (1 << type))
        tsterror(0);
    else {
        hit |= (1 << type);
        tsterror("aso error %d: %s", type, mesg);
    }
    return 0;
}

int tstwait(pid_t *proc, int nproc) {
    int code = 2, n, status, reaped = 0, ignore = 0;
    pid_t pid, parent = getpid();

    if (nproc < 0) {
        nproc = -nproc;
        ignore = 1;
    }
    tstinfo("Parent[pid=%d]: waiting for %d child%s", parent, nproc, nproc == 1 ? "" : "ren");
    while ((pid = wait(&status)) > 0) {
        if (proc) {
            for (n = 0; n < nproc; n++) {
                if (proc[n] == pid) {
                    tstinfo("Parent[pid=%d]: process %d[pid=%d] status=%d", parent, n, pid, status);
                    reaped++;
                    break;
                }
            }
            if (n >= nproc)
                tstwarn("Parent[pid=%d]: process UNKNOWN[pid=%d] status=%d", parent, pid, status);
        }
        if (status)
            code = 1;
        else if (code > 1)
            code = 0;
    }
    if (reaped != nproc && !ignore) {
        tsterror("Parent[pid=%d]: expected %d process%s, got %d", parent, nproc,
                 nproc == 1 ? "" : "es", reaped);
        code = 2;
    }
    return code;
}

__attribute__((unused)) static char *tstfile(char *pfx, int n) {
    assert(pfx);
    assert(n >= 0 && n < sizeof(Tstfile) / sizeof(Tstfile[0]));

    if (Tstfile[n][0]) return Tstfile[n];

    static int pid = 0;
    if (!pid) pid = (int)(getpid() % 10000);
    snprintf(Tstfile[n], sizeof(Tstfile[0]), "%s.%c.%d.tst", pfx, '0' + n, pid);
    return Tstfile[n];
}

typedef struct Asotype_s {
    const char *name;
    int mask;
} Asotype_t;

__attribute__((unused)) static void asointr(int sig) {
    int use;

    signal(sig, SIG_IGN);
    if (sig == SIGINT || sig == SIGQUIT || sig == SIGTERM)
        use = sig;
    else
        signal(use = SIGTERM, SIG_IGN);
    kill(0, use);
    switch (sig) {
        case SIGALRM:
            write(2, "\tFAILED due to timeout\n", 23);
            break;
        case SIGBUS:
            write(2, "\tFAILED with SIGBUS\n", 20);
            dump_backtrace(0);
            abort();
        case SIGSEGV:
            write(2, "\tFAILED with SIGSEGV\n", 21);
            dump_backtrace(0);
            abort();
    }
    texit(sig);
}

__attribute__((unused)) static void tstintr(void) {
    setpgid(0, 0);
    signal(SIGINT, asointr);
    signal(SIGQUIT, asointr);
    signal(SIGTERM, asointr);
    signal(SIGBUS, asointr);
    signal(SIGSEGV, asointr);
    if (Tsttimeout) {
        signal(SIGALRM, asointr);
        alarm(Tsttimeout * 60);
    }
}

__attribute__((unused)) static int tstchild(char **argv) {
    char **v = argv;
    char *a;

    Tstchild = 1;
    while ((a = *++v)) {
        if (strcmp(a, "--all") == 0)
            Tstall++;
        else if (strcmp(a, "--child") == 0)
            return (int)(v - argv + 1);
        else if (strncmp(a, "--timeout=", 10) == 0)
            Tsttimeout = atoi(a + 10);
        else if (strcmp(a, "--") == 0)
            break;
        else if (strncmp(a, "--", 2) != 0)
            break;
    }
    tstintr();
    return 0;
}

__attribute__((unused)) static int tstopts(char **argv) {
    char **v = argv;
    char *a;

    while ((a = *++v)) {
        if (strcmp(a, "--all") == 0)
            Tstall++;
        else if (strncmp(a, "--timeout=", 10) == 0)
            Tsttimeout = atoi(a + 10);
        else if (strcmp(a, "--") == 0)
            return (int)(v - argv + 1);
        else if (strncmp(a, "--", 2) != 0)
            break;
    }
    tstintr();
    return (int)(v - argv);
}

static unsigned int Rand = 0xdeadbeef;

__attribute__((unused)) static void trandseed(unsigned int seed) {
    Rand = seed == 0 ? 0xdeadbeef : seed;
}

__attribute__((unused)) static unsigned int trandom(void) {
    Rand = Rand * 17109811 + 751;
    return Rand;
}

__attribute__((unused)) static void *tstshared(size_t n) {
    void *p = NULL;
    int z;

    if ((z = open("/dev/zero", O_RDWR)) >= 0) {
        p = mmap(0, n, PROT_READ | PROT_WRITE, MAP_SHARED, z, 0);
        if (!p || p == (void *)(-1)) {
            // cppcheck-suppress memleak
            p = NULL;
            close(z);
        }
    }
    if (!p) {
#ifdef MAP_ANONYMOUS
        p = mmap(0, n, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
        if (!p || p == (void *)(-1))
#endif
            tsterror("mmap failed on %zu bytes", n);
    }
    memset(p, 0, n);
    return p;
}

struct IntegerToStringTest {
    int input;
    const char *expected_result;
};

struct StringToIntegerTest {
    const char *input;
    int expected_result;
};

// Test conversion of string
struct StringToStringTest {
    const char *input;
    const char *expected_result;
};

#endif  // _TERROR_H
