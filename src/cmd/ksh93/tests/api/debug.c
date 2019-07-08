// Test the APIs provided by the src/cmd/ksh93/sh/debug.c module.
//
// WARNING: These tests are highly sensitive to the line number of each statement. So Any change
// to this file is likely to cause a test failure due to line number mismatches.
//
// TODO: Figure out how to modify this test and/or the test framework to make changes to the line
// number emitted by tests like `DPRINT_VT()` output independent of the actual line number.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "name.h"
#include "sfio.h"

static struct Value v1;
static struct Value v2;
static struct Value *v2p;
static char *str = "str";
static const char *const_str = "const str";
static int i;
static int16_t i16;
static int32_t i32;
static int64_t i64;
static double d;
static float f;
static Sfdouble_t sfdouble;
static pid_t pid;
static uid_t uid;

#define SET_BASE_ADDR(p, offset) _dprint_vt_base_addr = (char *)(p)-offset

static void test_dprint_vt1() {
    SET_BASE_ADDR(&v1, 4);
    STORE_VT(v1, vp, &v1);
    DPRINT_VT(v1);

    v2p = &v2;
    SET_BASE_ADDR(v2p, 4);
    STORE_VTP(v2p, i, 789);
    STORE_VT(v1, up, v2p);
    DPRINT_VT(v1);

    SET_BASE_ADDR(str, 8);
    STORE_VT(v1, cp, str);
    DPRINT_VT(v1);

    SET_BASE_ADDR(const_str, 12);
    STORE_VT(v1, const_cp, const_str);
    DPRINT_VT(v1);

    STORE_VT(v1, i, 321);
    DPRINT_VT(v1);

    STORE_VT(v1, i16, 357);
    DPRINT_VT(v1);
}

static void test_dprint_vt2() {
    SET_BASE_ADDR(&i, 16);
    i = 111;
    STORE_VT(v1, ip, &i);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&i16, 20);
    i16 = 1616;
    STORE_VT(v1, i16p, &i16);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&i32, 24);
    i32 = 3232;
    STORE_VT(v1, i32p, &i32);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&i64, 28);
    i64 = (1LL << 32) + 3LL;
    STORE_VT(v1, i64p, &i64);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&d, 32);
    d = 2.718282;
    STORE_VT(v1, dp, &d);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&f, 36);
    f = 3.141593;
    STORE_VT(v1, fp, &f);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&sfdouble, 40);
    sfdouble = 1.23456789e37;
    STORE_VT(v1, sfdoublep, &sfdouble);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&pid, 44);
    pid = 98765;
    STORE_VT(v1, pidp, &pid);
    DPRINT_VT(v1);

    SET_BASE_ADDR(&uid, 48);
    uid = 54321;
    STORE_VT(v1, uidp, &uid);
    DPRINT_VT(v1);
}

static char cp[] = "dval2";
static Namval_t nval1, nval2;

static void test_dprint_nv() {
    memset(&nval1, 0, sizeof(nval1));
    memset(&nval2, 0, sizeof(nval2));
    nval1.nvname = "dvar1";
    nval1.nvsize = 33;
    STORE_VT(nval1.nvalue, i, 111);
    nval2.nvname = "dvar2";
    nval2.nvsize = 66;
    nval2.nvenv = &nval1;
    STORE_VT(nval2.nvalue, cp, cp);

    SET_BASE_ADDR(&cp, 4);
    DPRINT_NV(nval2);

    nval1.nvsize = 99;
    nv_setattr(&nval1, NV_DOUBLE);
    nval1.nvenv = (Namval_t *)"nvenv is a string";
    nval1.nvenv_is_cp = true;
    DPRINT_NV(nval1);
}

static char **pp = &str;
static struct pathcomp *pathcomp = NULL;

static void test_dprint_vt3() {
    SET_BASE_ADDR(pp, 32);
    STORE_VT(v1, pp, pp);
    DPRINT_VT(v1);

    SET_BASE_ADDR(pathcomp, 64);
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
    SET_BASE_ADDR(0x1234, 0x1234);
    STORE_VT(v1, cp, (char *)0x1234);
    DPRINT_VT(v1);

    // Printing a NULL ptr to a struct Value should be handled gracefully.
    struct Value *null_vtp = NULL;
    DPRINT_VTP(null_vtp);

    // Printing an uninitialized struct Value should be handled gracefully.
    struct Value uninitialized_vt;
    memset(&uninitialized_vt, 0, sizeof(uninitialized_vt));
    DPRINT_VT(uninitialized_vt);
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
    test_dprint_vt1();
    test_dprint_vt2();
    test_dprint_nv();
    test_dprint_vt3();
    test_dprint_nvflag();
    return 0;
}
