// Test the APIs provided by the src/cmd/ksh93/sh/debug.c module.
//
// WARNING: These tests are highly sensitive to the line number of each statement. So Any change
// to this file is likely to cause a test failure due to line number mismatches.
//
// TODO: Figure out how to modify this test and/or the test framework to make changes to the line
// number emitted by tests like `DPRINT_VT()` output independent of the actual line number.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast_assert.h"
#include "name.h"
#include "sfio.h"

// It is useful for some API unit tests to be able to make _dprintf() output deterministic with
// respect to portions of each log line it writes that would otherwise vary with each invocation
// (e.g., the pid). This is a private API solely for use by unit tests.
extern bool _dprintf_debug;
extern int _dprint_fixed_line;
extern int _debug_max_ptrs;
extern const void *(*_dprint_map_addr_fn)(const void *);
extern int (*_debug_getpid)();

static char *str = "str";
static char *cp = "dval2";
static char *dvar1 = "dvar1";
static char *dvar2 = "dvar2";
static char *nvenv = "nvenv is a string";
static const char *const_str = "const str";
static struct Value v1;
static struct Value v2;
static struct Value *v2p;
static int i;
static int16_t i16;
static int32_t i32;
static int64_t i64;
static double d;
static float f;
static Sfdouble_t sfdouble;
static pid_t pid;
static uid_t uid;
static char **pp = &str;
static struct pathcomp *pathcomp = NULL;
static Namval_t nval1, nval2;
static struct Namref nref;
static struct Namfun namfun;

// Do not alter the initialization of this table. If new vars need to be inserted use function
// append_map_addr(). This helps keep the addresses stable so that test output doesn't contain
// gratuitous changes that requires updating file debug.err.
#define MAX_ADDRS 64
static const void *addr_map[MAX_ADDRS] = {
    &v1,  &v2,  &v2p,      &i,     &i16,   &i32,  &i64,    &d,     &f,     &sfdouble,
    &pid, &uid, &pathcomp, &nval1, &nval2, &nref, &namfun, &dvar1, &dvar2, NULL};

// Append an address to the `addr_map[]` above. This is needed because some addresses can't be
// determined at compile time.
static void append_map_addr(const void *p) {
    int i = 0;
    while (addr_map[i]) ++i;
    if (i + 1 == MAX_ADDRS) {
        fprintf(stderr, "Error: addr_map[] is too small -- increase MAX_ADDRS\n");
        abort();
    }
    addr_map[i] = p;
}

static const void *_map_addr_fn(const void *p) {
    for (int i = 0; addr_map[i]; ++i) {
        if (addr_map[i] == p) return (void *)(((uintptr_t)i + 1) * 0x10);
    }
    return p;
}

static_fn int debug_getpid() { return 1234; }

static void test_dprint_vt() {
    STORE_VT(v1, vp, &v1);
    DPRINT_VT(v1);

    v2p = &v2;
    STORE_VTP(v2p, i, 789);
    STORE_VT(v1, up, v2p);
    DPRINT_VT(v1);

    STORE_VT(v1, cp, str);
    DPRINT_VT(v1);

    STORE_VT(v1, const_cp, const_str);
    DPRINT_VT(v1);

    STORE_VT(v1, np, NULL);
    DPRINT_VT(v1);

    STORE_VT(v1, np, &nval1);
    DPRINT_VT(v1);

    STORE_VT(v1, nrp, &nref);
    DPRINT_VT(v1);

    STORE_VT(v1, shbltinp, NULL);
    DPRINT_VT(v1);

    STORE_VT(v1, rp, NULL);
    DPRINT_VT(v1);

    STORE_VT(v1, funp, NULL);
    DPRINT_VT(v1);

    STORE_VT(v1, i, 321);
    DPRINT_VT(v1);

    STORE_VT(v1, i16, 357);
    DPRINT_VT(v1);

    i = 111;
    STORE_VT(v1, ip, &i);
    DPRINT_VT(v1);

    i16 = 1616;
    STORE_VT(v1, i16p, &i16);
    DPRINT_VT(v1);

    i32 = 3232;
    STORE_VT(v1, i32p, &i32);
    DPRINT_VT(v1);

    i64 = (1LL << 32) + 3LL;
    STORE_VT(v1, i64p, &i64);
    DPRINT_VT(v1);

    d = 2.718282;
    STORE_VT(v1, dp, &d);
    DPRINT_VT(v1);

    f = 3.141593;
    STORE_VT(v1, fp, &f);
    DPRINT_VT(v1);

    sfdouble = 1.23456789e37;
    STORE_VT(v1, sfdoublep, &sfdouble);
    DPRINT_VT(v1);

    pid = 98765;
    STORE_VT(v1, pidp, &pid);
    DPRINT_VT(v1);

    uid = 54321;
    STORE_VT(v1, uidp, &uid);
    DPRINT_VT(v1);

    STORE_VT(v1, pp, pp);
    DPRINT_VT(v1);

    STORE_VT(v1, funp, &namfun);
    DPRINT_VT(v1);

    STORE_VT(v1, pathcomp, pathcomp);
    DPRINT_VT(v1);

    STORE_VT(v1, uc, 'x');
    DPRINT_VT(v1);

    STORE_VT(v1, h, 439);
    DPRINT_VT(v1);

    STORE_VT(v1, l, 0x1234ABCD);
    DPRINT_VT(v1);

    STORE_VT(v1, f, 2.718282);
    DPRINT_VT(v1);

    STORE_VT(v1, d, 3.141593);
    DPRINT_VT(v1);

    // Verify that an invalid string pointer that causes a SIGSEGV is trapped and handled.
    STORE_VT(v1, cp, (char *)0x1234);
    errno = 666;
    DPRINT_VT(v1);
    assert(errno == 666);

    // Printing a NULL ptr to a struct Value should be handled gracefully.
    errno = 777;
    struct Value *null_vtp = NULL;
    DPRINT_VTP(null_vtp);
    assert(errno == 777);

    // Printing an uninitialized struct Value should be handled gracefully.
    struct Value uninitialized_vt;
    memset(&uninitialized_vt, 0, sizeof(uninitialized_vt));
    DPRINT_VT(uninitialized_vt);
}

static void test_dprint_nv() {
    nval1.nvname = dvar1;
    nval1.nvsize = 33;
    STORE_VT(nval1.nvalue, i, 111);
    nval2.nvname = dvar2;
    nval2.nvsize = 66;
    nval2.nvenv = &nval1;
    STORE_VT(nval2.nvalue, cp, cp);

    DPRINT_NV(nval2);

    nval1.nvsize = 99;
    nv_setattr(&nval1, NV_DOUBLE);
    nval1.nvenv = (Namval_t *)nvenv;
    nval1.nvenv_is_cp = true;
    DPRINT_NV(nval1);

    // Verify that pointer loops are handled.
    STORE_VT(nval1.nvalue, np, &nval2);
    STORE_VT(nval2.nvalue, np, &nval1);
    DPRINT_NV(nval1);

    // Verify that too many pointers to detect loops is handled.
    write(2, "\n", 1);
    int saved_debug_max_ptrs = _debug_max_ptrs;
    _debug_max_ptrs = 1;
    STORE_VT(nval1.nvalue, np, &nval2);
    STORE_VT(nval2.nvalue, np, &nval1);
    DPRINT_NV(nval1);
    _debug_max_ptrs = saved_debug_max_ptrs;
}

// Verify that printing nvflag_t objects produces correct output.
static void test_dprint_nvflag() {
    nvflag_t nvflag;

    nvflag = 0;
    DPRINTF("nvflag decoded: %s", nvflag_to_syms(nvflag));

    nvflag = NV_NOPRINT;
    DPRINTF("nvflag decoded: %s", nvflag_to_syms(nvflag));

    // This test is to verify that unrecognized bits in the value are reported. This is fragile and
    // will need to be changed if and when this particular bit is ever assigned a meaning.
    nvflag = NV_LONG | ((nvflag_t)1 << 31);
    DPRINTF("nvflag decoded: %s", nvflag_to_syms(nvflag));
}

int main() {
    _dprintf_debug = true;
    _dprint_fixed_line = 1;
    _dprint_map_addr_fn = _map_addr_fn;
    _debug_getpid = debug_getpid;
    append_map_addr(dvar1);
    append_map_addr(dvar2);
    append_map_addr(nvenv);
    append_map_addr(str);
    append_map_addr(cp);
    append_map_addr(const_str);
    append_map_addr(pp);
    memset(&nval1, 0, sizeof(nval1));
    memset(&nval2, 0, sizeof(nval2));

    test_dprint_vt();
    write(2, "\n", 1);
    test_dprint_nv();
    write(2, "\n", 1);
    test_dprint_nvflag();
    return 0;
}
