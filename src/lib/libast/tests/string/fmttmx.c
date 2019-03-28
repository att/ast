#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "terror.h"
#include "tmx.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    // Unix Epoch
    Time_t t = 0;
    const char *actual_result;

    const char *expected_result = "Thu Jan  1 00:00:00 UTC 1970";

    // Set timezone to match expected result
    setenv("TZ", "UTC", 1);
    tzset();

    // Format strings will be tested through `tmxfmt()` function tests
    actual_result = fmttmx("", t);

    if (strcmp(actual_result, expected_result)) {
        terror("fmttmx() failed :: Actual Result : %s, Expected Result : %s", actual_result,
               expected_result);
    }
    texit(0);
}
