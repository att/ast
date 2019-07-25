#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    texit(0);
    UNUSED(argc);
    UNUSED(argv);

    const char *actual_result = NULL;

    // Test integer conversions using scale == 1000.
    struct IntegerToStringTest test1000[] = {{0, "0"},  // the most basic test
                                             {1, "1.0"},
                                             {5, "5.0"},
                                             {9, "9.0"},
                                             {10, "10"},
                                             {666, "666"},
                                             {999, "999"},
                                             {1000, "1.0k"},
                                             {1024, "1.0k"},
                                             {1025, "1.0k"},
                                             {1524, "1.5k"},
                                             {2095, "2.1k"},
                                             {9 * 1024 * 1024, "9.4M"},
                                             {1024 * 1024 * 1024, "1.1G"},
                                             {0, NULL}};

    // Test integer conversions using scale == 1024.
    struct IntegerToStringTest test1024[] = {{0, "0"},  // the most basic test
                                             {1, "1.0"},
                                             {7, "7.0"},
                                             {9, "9.0"},
                                             {10, "10"},
                                             {666, "666"},
                                             {999, "999"},
                                             {1000, "1.0Ki"},
                                             {1024, "1.0Ki"},
                                             {1025, "1.0Ki"},
                                             {1524, "1.5Ki"},
                                             {2095, "2.0Ki"},
                                             {9 * 1024 * 1024, "9.0Mi"},
                                             {1024 * 1024 * 1024, "1.0Gi"},
                                             {0, NULL}};

    for (int i = 0; test1000[i].expected_result; ++i) {
        actual_result = fmtscale(test1000[i].input, 1000);
        if (strcmp(actual_result, test1000[i].expected_result)) {
            terror("fmtscale() %d: %d:\nActual: %s\nExpect: %s", i, test1000[i].input,
                   actual_result, test1000[i].expected_result);
        }
    }

    for (int i = 0; test1024[i].expected_result; ++i) {
        actual_result = fmtscale(test1024[i].input, 1024);
        if (strcmp(actual_result, test1024[i].expected_result)) {
            terror("fmtscale() %d: %d:\nActual: %s\nExpect: %s", i, test1024[i].input,
                   actual_result, test1024[i].expected_result);
        }
    }

    texit(0);
}
