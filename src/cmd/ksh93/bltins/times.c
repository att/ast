//
// `times` builtin command
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <math.h>
#include <string.h>
#include <sys/times.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

int b_times(int argc, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;
    const char *cmd = argv[0];

    while ((argc = optget(argv, sh_opttimes))) {
        switch (argc) {
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            default: { break; }
        }
    }
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    if (*argv) {
        errormsg(SH_DICT, 2, "%s: unexpected arguments", cmd);
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    struct tms cpu_times;
    clock_t rv = times(&cpu_times);
    if (rv == (clock_t)-1) {
        errormsg(SH_DICT, ERROR_usage(2), "times() function unexpectedly failed: errno %d %s",
                 errno, strerror(errno));
        __builtin_unreachable();
    }

    double utime, stime, utime_min, utime_sec, stime_min, stime_sec;

    utime = (double)cpu_times.tms_utime / shp->gd->lim.clk_tck;
    utime_min = floor(utime / 60);
    utime_sec = utime - utime_min;
    stime = (double)cpu_times.tms_stime / shp->gd->lim.clk_tck;
    stime_min = floor(stime / 60);
    stime_sec = stime - stime_min;
    sfprintf(sfstdout, "%dm%.2fs %dm%.2fs\n", (int)utime_min, utime_sec, (int)stime_min, stime_sec);

    utime = (double)cpu_times.tms_cutime / shp->gd->lim.clk_tck;
    utime_min = floor(utime / 60);
    utime_sec = utime - utime_min;
    stime = (double)cpu_times.tms_cstime / shp->gd->lim.clk_tck;
    stime_min = floor(stime / 60);
    stime_sec = stime - stime_min;
    sfprintf(sfstdout, "%dm%.2fs %dm%.2fs\n", (int)utime_min, utime_sec, (int)stime_min, stime_sec);

    return 0;
}
