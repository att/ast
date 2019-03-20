//
// We have our own private `assert()` implementation because too many platform implementations
// cause lint warnings from tools like cppcheck. They also tend to write the error to stdout
// rather than stderr which is wrong.
//
#ifndef _AST_ASSERT_H
#define _AST_ASSERT_H 1

#include <stdio.h>
#include <stdlib.h>

#undef assert

#ifdef NDEBUG

#define assert(e) ((void)0)

#else  // NDEBUG

// The odd construction is to avoid "multiple unary operator" warnings from lint tools like oclint.
// While still making this syntactically valid for `assert(xyz);` style invocations.
#define assert(e) \
    if (e) {      \
        ;         \
    } else        \
        __assert(#e, __FILE__, __LINE__)

__attribute__((noreturn)) static inline void __assert(const char *e, const char *file, int line) {
    (void)fprintf(stderr, "%s:%d: failed assertion '%s'\n", file, line, e), (void)fflush(stderr);
    (void)fflush(stderr);
    abort();
}

#endif  // NDEBUG

#endif  // _AST_ASSERT_H
