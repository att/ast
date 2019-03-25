#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    const char *actual_result = NULL;

    struct IntegerToStringTest test1[] = {
        { 1000, "1.0k" },
        { 2095, "2.1k" },
        { 9*1024*1024, "9.4M" },
        { 1024*1024*1024, "1.1G" },
        { 0, NULL }
    };

    struct IntegerToStringTest test2[] = {
        { 1024, "1.0Ki" },
        { 2095, "2.0Ki" },
        { 9*1024*1024, "9.0Mi" },
        { 1024*1024*1024, "1.0Gi" },
        { 0, NULL }
    };

    for (int i=0; test1[i].input; ++i) {
        actual_result = fmtscale(test1[i].input, 1000);
        if (strcmp(actual_result, test1[i].expected_result)) {
            terror("fmtscale() failed :: Failed to convert number to string :: Actual Result : %s, Expected Result : %s",
                    actual_result, test1[i].expected_result);
        }
    }

    for (int i=0; test2[i].input; ++i) {
        actual_result = fmtscale(test2[i].input, 1024);
        if (strcmp(actual_result, test2[i].expected_result)) {
            terror("fmtscale() failed :: Failed to convert number to string :: Actual Result : %s, Expected Result : %s",
                    actual_result, test2[i].expected_result);
        }
    }

    texit(0);
}
