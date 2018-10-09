//
// This module contains fallback implementations of functions used elsewhere in the code.
// Each fallback implementation is only enabled if the system doesn't provide it.
//
#include "config_ast.h"  // IWYU pragma: keep

// We keep all these includes because it's simpler and cleaner to do this than wrap them in
// `#if !_lib_mkostemp` type pragmas.
#include <stdio.h>   // IWYU pragma: keep
#include <stdlib.h>  // IWYU pragma: keep
#include <string.h>  // IWYU pragma: keep
#include <unistd.h>  // IWYU pragma: keep

#include "ast.h"         // IWYU pragma: keep
#include "ast_assert.h"  // IWYU pragma: keep

//
// We define this symbol, which is otherwise unused, to ensure this module isn't empty. That's
// because empty modules can cause build time warnings.
//
void do_not_use_this_fallback() {
    abort();
}

#if !_lib_mkostemp
// This is a fallback in case the system doesn't provide it.
static_fn int mkostemp(char *template, int oflags) {
    for (int i = 10; i; i--) {
#ifndef __clang_analyzer__
        // cppcheck-suppress  mktempCalled
        char *tp = mktemp(template);
        assert(tp);
#endif

        int fd = open(template, O_CREAT | O_RDWR | O_EXCL | oflags, S_IRUSR | S_IWUSR);
        if (fd != -1) return fd;
    }
    return -1;
}
#endif  // !_lib_mkostemp
