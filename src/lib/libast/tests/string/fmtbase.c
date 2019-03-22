#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

typedef struct Test {
    int input;
    char *result;
} Test;

void decimal_to_octal() {
    Test tests_without_prefix[] = {{10, "12"}, {15, "17"}, {23, "27"}, {199, "307"}, {0, NULL}};

    Test tests_with_prefix[] = {
        {10, "8#12"}, {15, "8#17"}, {23, "8#27"}, {199, "8#307"}, {0, NULL}};

    const char *result;

    for (int i = 0; tests_without_prefix[i].result; ++i) {
        result = fmtbase(tests_without_prefix[i].input, 8, 0);

        if (strcmp(result, tests_without_prefix[i].result))
            terror(
                "fmtbase() :: Failed to convert decimal to octal number :: Expected: %s, Actual: "
                "%s",
                tests_without_prefix[i].result, result);
    }

    for (int i = 0; tests_with_prefix[i].result; ++i) {
        result = fmtbase(tests_with_prefix[i].input, 8, 1);

        if (strcmp(result, tests_with_prefix[i].result))
            terror(
                "fmtbase() :: Failed to convert decimal to octal number :: Expected: %s, Actual: "
                "%s",
                tests_with_prefix[i].result, result);
    }
}

void decimal_to_hex() {
    Test tests_without_prefix[] = {{10, "a"}, {15, "f"}, {23, "17"}, {199, "c7"}, {0, NULL}};

    Test tests_with_prefix[] = {
        {10, "16#a"}, {15, "16#f"}, {23, "16#17"}, {199, "16#c7"}, {0, NULL}};

    const char *result;

    for (int i = 0; tests_without_prefix[i].result; ++i) {
        result = fmtbase(tests_without_prefix[i].input, 16, 0);

        if (strcmp(result, tests_without_prefix[i].result))
            terror(
                "fmtbase() :: Failed to convert decimal to octal number :: Expected: %s, Actual: "
                "%s",
                tests_without_prefix[i].result, result);
    }

    for (int i = 0; tests_with_prefix[i].result; ++i) {
        result = fmtbase(tests_with_prefix[i].input, 16, 1);

        if (strcmp(result, tests_with_prefix[i].result))
            terror(
                "fmtbase() :: Failed to convert decimal to octal number :: Expected: %s, Actual: "
                "%s",
                tests_with_prefix[i].result, result);
    }
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    decimal_to_octal();
    decimal_to_hex();
    texit(0);
}
