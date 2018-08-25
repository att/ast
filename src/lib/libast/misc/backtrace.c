#include "config_ast.h"  // IWYU pragma: keep

#include <dlfcn.h>
#include <stdio.h>

#if _hdr_execinfo
#include <execinfo.h>
#endif

// Write a primitive backtrace to stderr. This can be called from anyplace in
// the code where you would like to understand the call sequence leading to
// that point in the code.
#if _hdr_execinfo
void dump_backtrace(int max_frames, int skip_levels) {
    if (max_frames <= 0) return;

    char text[512];
    void *callstack[128];
    int n_max_frames = sizeof(callstack) / sizeof(callstack[0]);
    int n_frames = backtrace(callstack, n_max_frames);
    char **symbols = backtrace_symbols(callstack, n_frames);

    max_frames += skip_levels;
    if (n_frames < max_frames) max_frames = n_frames;

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

void dump_backtrace(int max_frames, int skip_levels) {
    UNUSED(max_frames);
    UNUSED(skip_levels);
    static char *msg = "Sorry, but dump_backtrace() does not work on your system.\n";
    write(2, msg, strlen(msg));
}

#endif  // _hdr_execinfo
