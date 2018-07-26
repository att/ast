//
// We have our own private `assert()` implementation because too many platform implementations
// cause lint warnings from tools like cppcheck. They also tend to write the error to stdout
// rather than stderr which is wrong.
//
#ifndef _AST_ASSERT_H
#define _AST_ASSERT_H 1

#include <stdio.h>

#undef assert

#ifdef NDEBUG
#define assert(e) ((void)0)
#else  // NDEBUG
#define assert(e) ((void)((e) ? ((void)0) : __assert(#e, __FILE__, __LINE__)))
#define __assert(e, file, line) \
    ((void)fprintf(stderr, "%s:%d: failed assertion '%s'\n", file, line, e), \
     (void)fflush(stderr), \
     abort())
#endif  // NDEBUG

#endif  // _AST_ASSERT_H
