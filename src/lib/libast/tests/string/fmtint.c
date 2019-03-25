#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    const char *actual_result;

    struct IntegerToStringTest tests[] = {
        {9, "9"},           {123, "123"},   {24321, "24321"}, {10000000, "10000000"},
        {-10000, "-10000"}, {-151, "-151"}, {0, "0"},         {0, NULL}};

    for (int i = 0; tests[i].expected_result; ++i) {
        actual_result = fmtint(tests[i].input, 0);
        if (strcmp(actual_result, tests[i].expected_result)) {
            terror(
                "fmtint() failed :: Failed to convert number to string :: Actual Result : %d, "
                "Expected Result : %d",
                actual_result, tests[i].expected_result);
        }
    }

    texit(0);
}
