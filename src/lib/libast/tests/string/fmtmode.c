#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    const char *actual_result = NULL;

    struct IntegerToStringTest tests[] = {{0777, "-rwxrwxrwx"},
                                          {0555, "-r-xr-xr-x"},
                                          {0444, "-r--r--r--"},
                                          {0755, "-rwxr-xr-x"},
                                          {0, NULL}};

    for (int i = 0; tests[i].input; ++i) {
        actual_result = fmtmode(tests[i].input);
        if (strcmp(actual_result, tests[i].expected_result)) {
            terror(
                "fmtmode() failed :: Failed to convert mode to formatted string :: Actual Result : "
                "%s, Expected Result : %s",
                actual_result, tests[i].expected_result);
        }
    }
    texit(0);
}
