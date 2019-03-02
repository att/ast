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
#include <inttypes.h>
#include <sys/types.h>

#include "ast_assert.h"
#include "name.h"


static void test_dprint_vt() {
    struct Value v1;
    struct Value v2;
    struct Value *v2p = &v2;

    STORE_VTP(v2p, i, 789);

    STORE_VT(v1, vp, (void *)0x1234);
    DPRINT_VT(v1);

    STORE_VT(v1, up, v2p);
    DPRINT_VT(v1);
}

int main() {
    _dprintf_debug = true;
    test_dprint_vt();
    return 0;
}
