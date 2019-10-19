//
// `times` builtin command
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>
#include <sys/time.h>

#include "builtins.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"

static const char *short_options = "+:";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

// Print user and system mode CPU times.
static_fn void print_times(struct timeval utime, struct timeval stime) {
    int ut_min = utime.tv_sec / 60;
    int ut_sec = utime.tv_sec % 60;
    int ut_ms = utime.tv_usec / 1000;
    int st_min = stime.tv_sec / 60;
    int st_sec = stime.tv_sec % 60;
    int st_ms = stime.tv_usec / 1000;
    sfprintf(sfstdout, "%dm%d.%03ds %dm%d.%03ds\n", ut_min, ut_sec, ut_ms, st_min, st_sec, st_ms);
}

#if _lib_getrusage

// Use getrusage() rather than times() since the former typically has higher resolution.
#include <sys/resource.h>

// Print user and system mode CPU times for both the shell and its child processes.
static_fn void print_cpu_times() {
    struct rusage usage;

    // Print the time (user & system) consumed by the shell.
    getrusage(RUSAGE_SELF, &usage);
    print_times(usage.ru_utime, usage.ru_stime);
    // Print the time (user & system) consumed by the child processes of the shell.
    getrusage(RUSAGE_CHILDREN, &usage);
    print_times(usage.ru_utime, usage.ru_stime);
}

#else  // _lib_getrusage

// Use times() since getrusage() isn't available. Note that it typically has a lower resolution
// which is why we prefer getrusage().
#include <sys/times.h>

// Print user and system mode CPU times for both the shell and its child processes.
static_fn void print_cpu_times() {
    struct timeval utime, stime;
    double dtime;
    long clk_tck = sysconf(_SC_CLK_TCK);
    struct tms cpu_times;
    times(&cpu_times);

    // Print the time (user & system) consumed by the shell.
    dtime = (double)cpu_times.tms_utime / clk_tck;
    utime.tv_sec = dtime / 60;
    utime.tv_usec = 1000000 * (dtime - utime.tv_sec);
    dtime = (double)cpu_times.tms_stime / clk_tck;
    stime.tv_sec = dtime / 60;
    stime.tv_usec = 1000000 * (dtime - utime.tv_sec);
    print_times(utime, stime);

    // Print the time (user & system) consumed by the child processes of the shell.
    dtime = (double)cpu_times.tms_cutime / clk_tck;
    utime.tv_sec = dtime / 60;
    utime.tv_usec = 1000000 * (dtime - utime.tv_sec);
    dtime = (double)cpu_times.tms_cstime / clk_tck;
    stime.tv_sec = dtime / 60;
    stime.tv_usec = 1000000 * (dtime - utime.tv_sec);
    print_times(utime, stime);
}

#endif  // _lib_getrusage

int b_times(int argc, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    int opt;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optget_ind;

    if (*argv) {
        builtin_usage_error(shp, cmd, "unexpected arguments");
        return 2;
    }

    print_cpu_times();
    return 0;
}
