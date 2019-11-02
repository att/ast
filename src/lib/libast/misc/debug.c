// This contains functions useful for debugging. For example, `dump_backtrace()` to write a basic
// stack dump to stderr. Or the function that implements the `DPRINTF()` macro.
//
#define NO_MALLOC_WRAPPERS 1
#include "config_ast.h"  // IWYU pragma: keep

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#if _hdr_execinfo
#include <execinfo.h>
#else
#include <string.h>
#endif

#include "ast.h"
#include "tmx.h"

// This is the maximum number of stack frames we'll output from dump_backtrace(). It is also the
// default if the caller specifies zero for the number of frames to output. It's 22 rather than 20
// because we want to ignore the frame for the `dump_backtrace()` and still dump 20 frames. Also,
// some backtrace() implementations include themselves as the first frame in the call stack.
#define MAX_FRAMES 22

// It is useful for some API unit tests to be able to make _dprintf() output deterministic with
// respect to portions of each log line it writes that would otherwise vary with each invocation
// (e.g., the pid).
bool _dprintf_debug = false;
int _dprintf_buf_sz = 1024;  // must match size of buf2[] below
// Setting _dprint_base_line to a non-zero value will cause that value to be displayed rather than
// the actual line.
int _dprint_fixed_line = 0;
char *_debug_lsof = "lsof";
int (*_debug_getpid)() = getpid;

// These are used by `_dprintf()` to ensure that if a `DPRINTF()` invocation causes a SIGSEGV it
// doesn't kill the process and we still get a debug message.
static struct sigaction dprintf_oact;
static sigjmp_buf jbuf;

static_fn void dprintf_segv_handler(int signo) {
    UNUSED(signo);
    siglongjmp(jbuf, 1);
}

static_fn void dprintf_trap_sigsegv() {
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = dprintf_segv_handler;
    sigaction(SIGSEGV, &act, &dprintf_oact);
}

static_fn void dprintf_untrap_sigsegv() { sigaction(SIGSEGV, &dprintf_oact, NULL); }

// This value is used by the dump_backtrace() code.
static const char *ksh_pathname = NULL;

void set_debug_filename(const char *pathname) { ksh_pathname = pathname; }

static_fn uintptr_t _debug_addr(int i, void **addrs) {
    return _dprintf_debug ? ((uintptr_t)i + 1) * 8 : (uintptr_t)addrs[i];
}

// This is initialized to the current timestamp when the first DPRINTF() is executed. This allows us
// to print time deltas in the debug print messages. A delta from the previous debug print is
// usually more helpful than an absolute time.
static Time_t _dprintf_base_time = TMX_NOTIME;

void _dprintf(const char *fname, int lineno, const char *funcname, const char *fmt, ...) {
    int oerrno = errno;
    // Use long rather than pid_t because pid_t may be an int or long depending on the platform.
    long pid = _debug_getpid();
    if (_dprint_fixed_line) lineno = _dprint_fixed_line;

    va_list ap;

    if (_dprintf_base_time == TMX_NOTIME) _dprintf_base_time = tmxgettime();
    Time_t time_delta = _dprintf_debug ? 0.0 : tmxgettime() - _dprintf_base_time;

    // The displayed timestamp will be seconds + milliseconds since the first DPRINTF(). The
    // tmxgettime() return value has a theoretical resolution of nanoseconds but that is more
    // precision than we need or want in these debug messages. We could print microsecond
    // resolution deltas but that is also too fine grained for our needs and takes too much space.
    uint64_t ds = time_delta / 1000000000;
    uint64_t dms = (time_delta % 1000000000) / 1000000;
    char buf1[64];
    (void)snprintf(buf1, sizeof(buf1), "%s:%d", strrchr(fname, '/') + 1, lineno);

    // We don't do three separate writes (the preamble, the actual message, the newline) because
    // that would not be atomic. Even though that would be simpler and less likely to truncate the
    // debug message. We want all three portions of the message to be emitted by a single write()
    // so the content can't be interleaved with other debug messages.
    char buf2[1024];
    int n = snprintf(buf2, sizeof(buf2), "### %ld %3" PRIu64 ".%03" PRIu64 " %-18s %15s() ", pid,
                     ds, dms, buf1, funcname);
    va_start(ap, fmt);
    dprintf_trap_sigsegv();
    if (sigsetjmp(jbuf, 1) == 0) {
        n += vsnprintf(buf2 + n, sizeof(buf2) - n, fmt, ap);
    } else {
        n += snprintf(buf2 + n, sizeof(buf2) - n, ">>>SIGSEGV received while formatting<<<");
    }
    dprintf_untrap_sigsegv();
    va_end(ap);
    if (n >= _dprintf_buf_sz) {
        // The message was too large for the buffer so try to make that clear to the reader.
        n = _dprintf_buf_sz - 4;
        buf2[n++] = '.';
        buf2[n++] = '.';
        buf2[n++] = '.';
    }
    buf2[n++] = '\n';
    write(2, buf2, n);
    errno = oerrno;
}

// Run external command `lsof -p $$` and redirect its output to stderr.
void run_lsof() {
    int oerrno = errno;

    sigset_t sigchld_mask, omask;
    sigemptyset(&sigchld_mask);
    sigaddset(&sigchld_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sigchld_mask, &omask);

    DPRINTF("Running lsof:");
    static char pid_str[20];
    long pid = _debug_getpid();
    snprintf(pid_str, sizeof(pid_str), "%ld", pid);

    pid = fork();
    if (pid == 0) {
        // Setup stdin, stdout, stderr. In this case we want the stdout of lsof
        // to go to our stderr so it is interleaved with DPRINTF() and other
        // diagnostic output.
        close(0);
        (void)open("/dev/null", O_RDONLY);
        dup2(2, 1);
        // Run the program we hope will give us detailed info about each open file descriptor.
        execlp(_debug_lsof, "lsof", "-p", pid_str, NULL);
    }

    // We ignore the return value because a) it should be impossible for this to fail and b) there
    // isn't anything we can do if it does fail. This is solely to reap the process so we don't
    // accumulate a lot of zombies.
    int status;
    (void)waitpid(pid, &status, 0);

    sigprocmask(SIG_SETMASK, &omask, NULL);
    errno = oerrno;
}

#if _hdr_execinfo

// The platform provides the minimum support required for us to emit a backtrace. Whether we can
// provide information at the file+line_number level or just sym_name+offset depends on whether we
// detected a program like `atos` or `addr2line` that we can invoke for more detailed information.
static char *info[MAX_FRAMES];

#if 0  // TODO: enable or remove in the future
// This is a cache of addresses already translated by `atos` or `addr2line`.
// This helps a lot when calling `dump_backtrace()` more than once because
// converting addresses to file and line numbers is very expensive.
#define CACHE_SIZE 1000
struct addr_info {
    void *p;
    char *info;
};
static struct addr_info cached_addrs[CACHE_SIZE] = { {NULL, NULL} };
int cached_addrs_size = 0;

static void add_cached_addr(void *p, const char *info) {
    if (cached_addrs_size == CACHE_SIZE) return;  // cache is full
    // It's slightly dangerous to call strdup() since we may be dumping a
    // backtrace in a signal context where calling malloc() may hang. But we
    // don't have much choice short of statically allocating buffers at build
    // time and wasting a lot of space.
    cached_addrs[++cached_addrs_size].p = p;
    cached_addrs[cached_addrs_size].info = strdup(info);
}

static const char *find_cached_addr(void *p) {
    for (int i = 0; i < cached_addrs_size; ++i) {
        if (cached_addrs[i].p == p) return cached_addrs[i].info;
    }
    return NULL;
}
#endif  // if 0

// Run an external command such as `atos` or `addr2line`, collect its output, and split it into
// lines. Each line has an entry in the `info[]` array above.
static_fn void run_addr2lines_prog(int n_frames, char *path, const char **argv) {
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
        close(2);
        dup2(fds[1], 1);
        (void)open("/dev/null", O_RDONLY);  // stdin
        (void)open("/dev/null", O_WRONLY);  // stderr

        // Run the program we hope will give us detailed info about each address.
        if (_dprintf_debug) {
#define msg "hello unit test\n"
            write(1, msg, sizeof(msg) - 1);
            _exit(0);
        }
        execv(path, (char *const *)argv);
    }
    close(fds[1]);

    static char atos_data[64 * 1024];
    int len = 0;
    int n;
    do {
        n = read(fds[0], atos_data + len, sizeof(atos_data) - len);
        len += n;
    } while (n > 0 && len < sizeof(atos_data));

    // We ignore the return value because a) it should be impossible for this to fail and b) there
    // isn't anything we can do if it does fail. This is solely to reap the process so we don't
    // accumulate a lot of zombies.
    int status;
    (void)waitpid(pid, &status, 0);
    close(fds[0]);

    sigprocmask(SIG_SETMASK, &omask, NULL);

    if (len == 0 || len >= sizeof(atos_data)) return;
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
    char argv_addrs[20 * MAX_FRAMES];  // 20 digits per addr + whitespace is enough for 64 bit addrs
    argv[0] = _pth_atos;
    argv[1] = "-p";
    static char pid_str[20];
    (void)snprintf(pid_str, sizeof(pid_str), "%d", _debug_getpid());
    argv[2] = pid_str;
    for (int i = 0; i < n_frames; ++i) {
        char *addr = argv_addrs + i * 20;
        (void)snprintf(addr, 20, "0x%" PRIXPTR "", _debug_addr(i, addrs));
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
        (void)snprintf(addr, 20, "0x%" PRIXPTR "", _debug_addr(i, addrs));
        argv[i + 3] = addr;
    }
    argv[n_frames + 3] = NULL;

    run_addr2lines_prog(n_frames, _pth_addr2line, argv);
    return info;
}

#else  // _pth_addr2line

// We don't seem to have a usable command for converting addresses to symbol info. It need do
// nothing more than return no details for any of the addresses.
static_fn char **addrs2info(int n_frames, void *addrs[]) {
    UNUSED(n_frames);
    UNUSED(addrs);

    memset(info, 0, sizeof(info));
    run_addr2lines_prog(0, "/bin/true", NULL);  // for the benefit of the debug API unit test
    return info;
}

#endif  // _pth_addr2line
#endif  // _pth_atos

// Given a single address return info about it; e.g., function name, file name, line number.
const char *addr2info(void *addr) {
    int oerrno = errno;
    const char *rv = addrs2info(1, &addr)[0];
    errno = oerrno;
    return rv;
}

// Write a backtrace to stderr. This can be called from anyplace in the code where you would like to
// understand the call sequence leading to that point in the code. It is also called automatically
// when a SIGSEGV is received. Pass a value of zero for `max_frames` to dump the maximum number of
// frames we allow by default. See `MAX_FRAMES` above. If you want fewer lines of context specify a
// value greater than zero but less than `MAX_FRAMES`.
void dump_backtrace(int max_frames) {
    int oerrno = errno;
    void *callstack[MAX_FRAMES];
    int n_frames = backtrace(callstack, MAX_FRAMES);
    char **details = addrs2info(n_frames, callstack);
    char text[512];
    long pid = _debug_getpid();

    (void)snprintf(text, sizeof(text), "### %ld Function backtrace:\n", pid);
    write(2, text, strlen(text));

    // Some backtrace() implementations include that function as the first entry in the call stack;
    // e.g., OpenBSD. But most implementations do not and instead have this function as the first
    // entry in the call stack. Enabling ASAN can also affect the results.
    int bias = 0;
    if ((char *)callstack[0] >= (char *)backtrace &&
        (char *)callstack[0] < (char *)backtrace + 0x20) {
        bias = 1;
    }
    if (_dprintf_debug) {  // we're running via a unit test
        // Only the bottom two frames are consistent across systems. So limit our output to those
        // two frames when testing this code.
        n_frames = 3;
        // When built with ASAN enabled on GNU there is a bogus NULL frame at the bottom of the
        // stack. If we detect that skip that frame.
        Dl_info info;
        dladdr(callstack[0], &info);
        if (info.dli_sname == NULL) bias += 1;
    } else {
        int max = max_frames > 0 ? max_frames : MAX_FRAMES - 2;
        if (max < n_frames) n_frames = max + 1;
    }

    for (int i = 1; i < n_frames; i++) {
        // On macOS the top frame has an address of 0x1. Ignore it and stop.
        if (callstack[i] == (void *)0x1) break;

        if (details[i + bias]) {
            (void)snprintf(text, sizeof(text), "%-3d %s\n", i, details[i + bias]);
        } else {
            Dl_info info;
            dladdr(callstack[i + bias], &info);
            ptrdiff_t offset =
                _dprintf_debug ? 0 : (char *)callstack[i + bias] - (char *)info.dli_saddr;
            // On some platforms (e.g., macOS) ptrdiff_t is just an alias for signed long.
            // cppcheck-suppress invalidPrintfArgType_sint
            (void)snprintf(text, sizeof(text), "%-3d %s + %td\n", i, info.dli_sname, offset);
        }
        write(2, text, strlen(text));
    }

    errno = oerrno;
}

#else  // _hdr_execinfo

#define addr2info_msg "### Sorry, but addr2info() does not work on your system.\n"
#define backtrace_msg "### Sorry, but dump_backtrace() does not work on your system.\n"

const char *addr2info(void *addr) {
    UNUSED(addr);
    int oerrno = errno;

    write(2, addr2info_msg, sizeof(addr2info_msg) - 1);
    errno = oerrno;
    return "";
}

void dump_backtrace(int max_frames) {
    UNUSED(max_frames);
    int oerrno = errno;

    write(2, backtrace_msg, sizeof(backtrace_msg) - 1);
    errno = oerrno;
}

#endif  // _hdr_execinfo
