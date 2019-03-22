#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

void test_fmtquote() {
    char *actual_result;

    struct StringToStringTest quote_with_double_quotes[] = {
        {"\a\b\f\n\r\t\v\\\x12", "\"\\a\\b\\f\\n\\r\\t\\v\\\\\\022\""},
        {"$`\"'", "\"\\$\\`\\\"'\""},
        {NULL, NULL}};

    for (int i = 0; quote_with_double_quotes[i].input; ++i) {
        actual_result = fmtquote(quote_with_double_quotes[i].input, "\"", "\"",
                                 strlen(quote_with_double_quotes[i].input), FMT_SHELL);
        if (strcmp(actual_result, quote_with_double_quotes[i].expected_result)) {
            terror("fmtquote() failed :: Expected Result: %s, Actual Result: %s",
                   quote_with_double_quotes[i].expected_result, actual_result);
        }
    }

    struct StringToStringTest quote_with_single_quotes[] = {
        {"This is a message", "'This is a message'"},
        {"$'This is a message'", "'$'\\''This is a message'\\'''"},
        {NULL, NULL}};

    for (int i = 0; quote_with_single_quotes[i].input; ++i) {
        actual_result = fmtquote(quote_with_single_quotes[i].input, "'", "'",
                                 strlen(quote_with_single_quotes[i].input), FMT_SHELL);
        if (strcmp(actual_result, quote_with_single_quotes[i].expected_result)) {
            terror("fmtquote() failed :: Expected Result: %s, Actual Result: %s",
                   quote_with_single_quotes[i].expected_result, actual_result);
        }
    }
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    test_fmtquote();

    texit(0);
}
