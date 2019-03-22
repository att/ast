#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    char *actual_result;

    struct IntegerToStringTest tests[] = {{10, "10.00s"},
                                          {60, "1m00s"},
                                          {60 * 60, "1h00m"},
                                          {24 * 60 * 60, "1d00h"},
                                          {28 * 24 * 60 * 60, "4w00d"},
                                          {364 * 24 * 60 * 60, "11M29d"},
                                          {365 * 24 * 60 * 60, "1Y00M"},
                                          {10 * 365 * 24 * 60 * 60, "9Y11M"},
                                          {0, NULL}};

    for (int i = 0; tests[i].expected_result; ++i) {
        actual_result = fmtelapsed(tests[i].input, 1);

        if (strcmp(actual_result, tests[i].expected_result)) {
            terror(
                "fmtelapsed() :: Failed to convert seconds to readable time :: Expected: %s, "
                "Actual: %s",
                tests[i].expected_result, actual_result);
        }
    }

    texit(0);
}
