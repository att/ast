#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    const char *actual_result = NULL;

    struct IntegerToStringTest tests[] = {{0777, "u=rwx,g=rwx,o=rwx"},
                                          {0555, "u=rx,g=rx,o=rx"},
                                          {0444, "u=r,g=r,o=r"},
                                          {0755, "u=rwx,g=rx,o=rx"},
                                          {0, NULL}};

    for (int i = 0; tests[i].input; ++i) {
        actual_result = fmtperm(tests[i].input);

        if (strcmp(actual_result, tests[i].expected_result)) {
            terror(
                "fmtperm() failed :: Failed to convert mode to string :: Actual Result : %s, "
                "Expected Result : %s",
                actual_result, tests[i].expected_result);
        }
    }

    texit(0);
}
