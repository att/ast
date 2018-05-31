// This module exists because there is code run by signal handlers that allocate memory. Most
// notably `job_waitsafe()`. They need to know if it is safe to do so. So we wrap the malloc family
// of functions.  See issue #563.
//
// TODO: Refactor those signal handlers so they only use async safe functions. Then remove this
// code.
//
#define NO_MALLOC_WRAPPERS 1
#include "config_ast.h"

#if _hdr_stdlib
#include <stdlib.h>
#elif _hdr_malloc
#include <malloc.h>
#endif

#if _hdr_unistd
#include <unistd.h>
#endif

#include <stdbool.h>

volatile bool vmbusy_flag = false;

void *ast_malloc(size_t size) {
    vmbusy_flag = true;
    void *p = malloc(size);
    vmbusy_flag = false;
    return p;
}

void *ast_calloc(size_t count, size_t size) {
    vmbusy_flag = true;
    void *p = calloc(count, size);
    vmbusy_flag = false;
    return p;
}

void *ast_realloc(void *ptr, size_t size) {
    vmbusy_flag = true;
    void *p = realloc(ptr, size);
    vmbusy_flag = false;
    return p;
}

void *ast_valloc(size_t size) {
    vmbusy_flag = true;
    void *p = valloc(size);
    vmbusy_flag = false;
    return p;
}

void ast_free(void *ptr) {
    vmbusy_flag = true;
    free(ptr);
    vmbusy_flag = false;
}
