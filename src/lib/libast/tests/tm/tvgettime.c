#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>
#include <sys/time.h>

#include "terror.h"
#include "tv.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    Tv_t tv;
    struct timeval timeval;

    tvgettime(&tv);

    gettimeofday(&timeval, NULL);

    // If time difference is greater than 1 second, treat it as an error.
    if (timeval.tv_sec - tv.tv_sec > 1) {
        terror("tvgetime() failed :: tvgetime() failed to get current time");
    }

    texit(0);
}
