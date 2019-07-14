#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "terror.h"

extern bool _dprintf_debug;
extern int _dprint_fixed_line;
extern int (*_debug_getpid)();
extern char *_debug_lsof;
extern int _dprintf_buf_sz;

static_fn int debug_getpid() { return 666; }

static_fn void test_run_lsof() {
    _debug_lsof = "echo";
    run_lsof();
}

static_fn void test_addr2info() {
    errno = 123;
    const char *info = addr2info(debug_getpid);
    fprintf(stderr, "addr2info(): %s\n", info);
    if (errno != 123) terror("Expected errno == 123 but it is %d", errno);
}

static_fn void test_backtrace() {
    errno = 456;
    dump_backtrace(2);
    if (errno != 456) terror("Expected errno == 456 but it is %d", errno);
    dump_backtrace(-1);
    dump_backtrace(0);
    dump_backtrace(999999);
}

tmain() {
    UNUSED(argc);
    _dprintf_debug = true;
    _dprint_fixed_line = 987;
    _debug_getpid = debug_getpid;
    set_debug_filename(argv[0]);

    test_run_lsof();
    test_addr2info();
    test_backtrace();

    // Verify that a debug message that is longer than can be constructed in a fixed size buffer is
    // properly truncated.
    _dprintf_buf_sz = 80;
    DPRINTF("0123456789012345678901234567890123456789");
    _dprintf_buf_sz = 70;
    DPRINTF("012345678901234567890123456789");

    texit(0);
}
