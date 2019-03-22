#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

static struct StringToStringTest tests[] = {
    {"*", "$"},       {"x", "^x$"},          {"*y", "y$"},
    {"x|y", "^x|y$"}, {"a?c*d", "^a.c.*d$"}, {"x?*|\\(|b?", "^x..*|\\(|(b.$"},
    {"(", NULL},      {NULL, NULL}};

#include <stdio.h>
tmain() {
    UNUSED(argc);
    UNUSED(argv);

    for (int i = 0; tests[i].input; ++i) {
        char *actual_result = fmtre(tests[i].input);
        // Is the conversion expected to fail and did it fail? Then continue.
        if (!tests[i].expected_result && !actual_result) continue;

        if (!tests[i].expected_result || !actual_result) {
            terror("fmtre() :: Failed to convert ksh pattern to ERE :: Expected: %s, Actual: %s",
                   tests[i].expected_result ? tests[i].expected_result : "(null)",
                   actual_result ? actual_result : "(null)");
            continue;
        }

        if (strcmp(actual_result, tests[i].expected_result)) {
            terror("fmtre() :: Failed to convert ksh pattern to ERE :: Expected: %s, Actual: %s",
                   tests[i].expected_result, actual_result);
        }
    }

    texit(0);
}
