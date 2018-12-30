// This contains functions useful for debugging. For example, `dump_backtrace()` to write a basic
// stack dump to stderr. Or the function that implements the `DPRINTF()` macro.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if _hdr_execinfo
#include <execinfo.h>
#else
#include <string.h>
#endif

#include "ast_assert.h"
#include "tmx.h"

// This is initialized to the current timestamp when the first DPRINTF() is executed. This allows us
// to print time deltas in the debug print messages. A delta from the previous debug print is
// usually more helpful than an absolute time.
static Time_t _dprintf_base_time = TMX_NOTIME;

void _dprintf(const char *fname, int lineno, const char *funcname, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    if (_dprintf_base_time == TMX_NOTIME) _dprintf_base_time = tmxgettime();
    Time_t time_delta = tmxgettime() - _dprintf_base_time;

    // The displayed timestamp will be seconds+milliseconds since the first DPRINTF(). The
    // tmxgettime() return value has a theoretical resolution of nanoseconds but that is more
    // precision than we need or want in these debug messages. We could print microsecond
    // resolution deltas but that is also too fine grained.
    uint64_t ds = time_delta / 1000000000;
    uint64_t dms = (time_delta % 1000000000) / 1000000;
    char buf[512];
    int n = snprintf(buf, sizeof(buf), "### %3" PRIu64 ".%03" PRIu64 " %4d %-12s %15s() ", ds, dms,
                     lineno, strrchr(fname, '/') + 1, funcname);
    (void)vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
    n = strlen(buf);
    assert(n < sizeof(buf));
    if (n < sizeof(buf) - 1) {
        buf[n] = '\n';
        buf[n + 1] = '\0';
        ++n;
    } else {
        buf[n] = '\n';
    }
    write(2, buf, n);

    va_end(ap);
}

#if _hdr_execinfo
// Write a primitive backtrace to stderr. This can be called from anyplace in the code where you
// would like to understand the call sequence leading to that point in the code. It is also called
// automatically when a SIGSEGV is received.
#define header_msg "### Function backtrace:\n"

void dump_backtrace(int max_frames, int skip_levels) {
    if (max_frames <= 0) return;

    char text[512];
    void *callstack[128];
    int n_max_frames = sizeof(callstack) / sizeof(callstack[0]);
    int n_frames = backtrace(callstack, n_max_frames);
    char **symbols = backtrace_symbols(callstack, n_frames);

    max_frames += skip_levels;
    if (n_frames < max_frames) max_frames = n_frames;

    write(2, header_msg, sizeof(header_msg));
    for (int i = skip_levels; i < max_frames; i++) {
        int n;
        Dl_info info;
        if (dladdr(callstack[i], &info) && info.dli_sname) {
            n = snprintf(text, sizeof(text), "%-3d %s + %td\n", i - skip_levels,
                         info.dli_sname == NULL ? symbols[i] : info.dli_sname,
                         (char *)callstack[i] - (char *)info.dli_saddr);
        } else {
            n = snprintf(text, sizeof(text), "%-3d %s\n", i - skip_levels, symbols[i]);
        }
        write(2, text, n);
    }

    free(symbols);
}

#else  // _hdr_execinfo

#define header_msg "### Sorry, but dump_backtrace() does not work on your system.\n"

void dump_backtrace(int max_frames, int skip_levels) {
    UNUSED(max_frames);
    UNUSED(skip_levels);
    write(2, header_msg, sizeof(header_msg));
}

#endif  // _hdr_execinfo
