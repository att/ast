// This contains functions useful for debugging. For example, `dump_backtrace()` to write a basic
// stack dump to stderr. Or the function that implements the `DPRINTF()` macro.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <dlfcn.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if _hdr_execinfo
#include <execinfo.h>
#else
#include <string.h>
#endif

#include "ast_assert.h"
#include "tmx.h"

// This is the maximum number of stack frames we'll output from dump_backtrace(). It is also the
// default if the caller specifies zero for the number of frames to output. It's 21 rather than 20
// because we want to ignore the frame for the `dump_backtrace()` and still dump 20 frames.
#define MAX_FRAMES 21

// This value is used by the dump_backtrace() code.
static const char *ksh_pathname = NULL;

void set_debug_filename(const char *pathname) { ksh_pathname = pathname; }

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

// The platform provides the minimum support required for us to emit a backtrace. Whether we can
// provide information at the file+line_number level or just sym_name+offset depends on whether we
// detected a program like `atos` or `addr2line` that we can invoke for more detailed information.
static char *info[MAX_FRAMES];

// Run an external command such as `atos` or `addr2line`, collect its output, and split it into
// lines. Each line has an entry in the `info[]` array above.
void run_addr2lines_prog(int n_frames, char *path, const char **argv) {
    int fds[2];
    pipe(fds);

    sigset_t sigchld_mask, omask;
    sigemptyset(&sigchld_mask);
    sigaddset(&sigchld_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sigchld_mask, &omask);

    pid_t pid = fork();
    if (pid == 0) {
        // Setup stdin, stdout, stderr.
        close(0);
        // cppcheck-suppress leakReturnValNotUsed
        (void)open("/dev/null", O_RDONLY);
        dup2(fds[1], 1);
        close(2);
        // cppcheck-suppress leakReturnValNotUsed
        (void)open("/dev/null", O_WRONLY);
        // Run the program we hope will give us detailed info about each address.
        execv(path, (char *const *)argv);
    }
    close(fds[1]);

    static char atos_data[64 * 1024];
    int len = 0;
    int n = 0;
    do {
        n = read(fds[0], atos_data + len, sizeof(atos_data) - len);
        len += n;
    } while (n != 0 && len < sizeof(atos_data));

    // We ignore the return value because a) it should be impossible for this to fail and b) there
    // isn't anything we can do if it does fail. This is solely to reap the process so we don't
    // accumulate a lot of zombies.
    int status;
    (void)waitpid(pid, &status, 0);

    sigprocmask(SIG_SETMASK, &omask, NULL);

    if (len == sizeof(atos_data)) return;
    // Null terminate the final string. We assume the last character is a newline. That should be a
    // safe assumption since it would be perverse for the program we ran to not newline terminate
    // the final line.
    atos_data[len - 1] = '\0';
    info[0] = atos_data;
    char *p = atos_data;
    for (int i = 1; i < n_frames; ++i) {
        p = strchr(p, '\n');
        if (!p) return;
        *p++ = '\0';
        info[i] = p;
    }
}

#ifdef _pth_atos

// Use external command `atos` to convert an address in the ksh binary to a function name, module
// name, and line number.
static_fn char **addrs2info(int n_frames, void *addrs[]) {
    memset(info, 0, sizeof(info));

    // Construct the argument list for the `atos` program. The "+ 4" allows for three fixed args and
    // the terminating NULL pointer.
    const char *argv[MAX_FRAMES + 4];
    // Sixteen hex digits is enough for 64 bit addrs. The extra three chars are for the `0x` prefix
    // from the "%p" format specifier and the terminating null byte.
    char argv_addrs[20 * MAX_FRAMES];  // 20 digits per addr max which is enough for 64 bit addrs
    argv[0] = _pth_atos;
    argv[1] = "-p";
    static char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    argv[2] = pid_str;
    for (int i = 0; i < n_frames; ++i) {
        char *addr = argv_addrs + i * 20;
        if (snprintf(addr, 20, "%p", addrs[i]) >= 20) *addr = '\0';
        argv[i + 3] = addr;
    }
    argv[n_frames + 3] = NULL;

    run_addr2lines_prog(n_frames, _pth_atos, argv);
    return info;
}

#else  // _pth_atos
#ifdef _pth_addr2line

// Use external command `addr2line` to convert an address in the ksh binary to a function name,
// module name, and line number.
static_fn char **addrs2info(int n_frames, void *addrs[]) {
    memset(info, 0, sizeof(info));
    if (!ksh_pathname) return info;

    // Construct the argument list for the `atos` program. The "+ 4" allows for three fixed args and
    // the terminating NULL pointer.
    const char *argv[MAX_FRAMES + 4];
    // Sixteen hex digits is enough for 64 bit addrs. The extra three chars are for the `0x` prefix
    // from the "%p" format specifier and the terminating null byte.
    char argv_addrs[20 * MAX_FRAMES];  // 20 digits per addr max which is enough for 64 bit addrs
    argv[0] = _pth_addr2line;
    argv[1] = "-sfpe";
    argv[2] = ksh_pathname;
    for (int i = 0; i < n_frames; ++i) {
        char *addr = argv_addrs + i * 20;
        if (snprintf(addr, 20, "%p", addrs[i]) >= 20) *addr = '\0';
        argv[i + 3] = addr;
    }
    argv[n_frames + 3] = NULL;

    run_addr2lines_prog(n_frames, _pth_addr2line, argv);
    return info;
}

#else  // _pth_addr2line

// We don't seem to have a usable command for converting addresses to symbol info.
static_fn char **addrs2info(int n_frames, void *addrs[]) {
    UNUSED(n_frames);
    UNUSED(addrs);
    memset(info, 0, sizeof(info));
    return info;
}

#endif  // _pth_addr2line
#endif  // _pth_atos

#define header_msg "### Function backtrace:\n"

// Write a backtrace to stderr. This can be called from anyplace in the code where you would like to
// understand the call sequence leading to that point in the code. It is also called automatically
// when a SIGSEGV is received. Pass a value of zero for `max_frames` to dump the maximum number of
// frames we allow by default. See `MAX_FRAMES` above. If you want fewer lines of context specify a
// value greater than zero but less than `MAX_FRAMES`.
void dump_backtrace(int max_frames) {
    assert(max_frames >= 0);
    if (max_frames > 0 && max_frames < MAX_FRAMES) {
        max_frames += 1;  // add room for this function's frame
    } else {
        max_frames = MAX_FRAMES;
    }

    void *callstack[MAX_FRAMES];
    int n_frames = backtrace(callstack, max_frames);
    // On macOS the top frame has an address of 0x1. Ignore it.
    if (callstack[n_frames - 1] == (void *)0x1) --n_frames;

    char **details = addrs2info(n_frames, callstack);

    write(2, header_msg, sizeof(header_msg) - 1);
    for (int i = 1; i < n_frames; i++) {
        char text[512];
        int n;
        if (details[i]) {
            n = snprintf(text, sizeof(text), "%-3d %s\n", i, details[i]);
        } else {
            Dl_info info;
            dladdr(callstack[i], &info);
            n = snprintf(text, sizeof(text), "%-3d %s + %td\n", i, info.dli_sname,
                         (char *)callstack[i] - (char *)info.dli_saddr);
        }
        write(2, text, n);
    }
}

#else  // _hdr_execinfo

#define header_msg "### Sorry, but dump_backtrace() does not work on your system.\n"

void dump_backtrace(int max_frames) {
    UNUSED(max_frames);
    write(2, header_msg, sizeof(header_msg));
}

#endif  // _hdr_execinfo
