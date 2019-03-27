#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>
#include <sys/types.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    int64_t actual_result;
    char *error;

    struct StringToIntegerTest tests[] = {{"500", 500}, {"999", 999}, {"-123", -123},
                                          {"0x0f", 15}, {"010", 8},   {NULL, 0}};

    for (int i = 0; tests[i].input; ++i) {
        actual_result = strton64(tests[i].input, &error, NULL, 1);
        if (actual_result != tests[i].expected_result) {
            terror(
                "strton64 failed() :: Failed to convert string to number :: Actual Result : %d, "
                "Expected Result : %d",
                actual_result, tests[i].expected_result);
        }
    }

    texit(0);
}
