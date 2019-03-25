#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    int actual_result;
    char *error = NULL;

    struct StringToIntegerTest tests[] = {
        {"1h5s", 3605},      {"1h5Se", 3605},    {"1hour5sec", 3605}, {"1 hour 5 sec", 3605},
        {"1:0:05", 3605},    {"3mi5da", 432180}, {"1w", 604800},      {"3M5da", 7689600},
        {"3mo5da", 7689600}, {"3MI5da", 432180}, {"3Mi5da", 432180},  {"3Mo5da", 7689600},
        {"3MO5da", 7689600}, {"1Y", 29030400},   {"1S", 580608000},   {NULL, 0}};

    for (int i = 0; tests[i].input; ++i) {
        actual_result = strelapsed(tests[i].input, &error, 1);

        if (actual_result != tests[i].expected_result || *error) {
            terror(
                "fmtelapsed() :: Failed to convert readable time to seconds :: Expected: %d, "
                "Actual: %d :: First unrecognized character : %s",
                tests[i].expected_result, actual_result, error);
        }
    }

    texit(0);
}
