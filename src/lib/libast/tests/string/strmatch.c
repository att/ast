#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

struct ShellPatternMatch {
    const char *input;
    const char *pattern;
    int expected_result;
};

void test_matching_patterns() {
    int actual_result, expected_result;
    struct ShellPatternMatch tests[] = {{"foo", "f*", 1},        {"foo", "f??", 1},
                                        {"foo1", "foo[1-9]", 1}, {"foobar", "foo[a-c]ar", 1},
                                        {"f**", "f\\*\\*", 1},   {"f??", "f\\?\\?", 1},
                                        {NULL, NULL, 0}};

    for (int i = 0; tests[i].input; ++i) {
        actual_result = strmatch(tests[i].input, tests[i].pattern);
        expected_result = tests[i].expected_result;

        if (actual_result != expected_result) {
            terror("strmatch() failed :: '%s' failed to match shell pattern '%s'", tests[i].input,
                   tests[i].pattern);
        }
    }

    for (int i = 0; tests[i].input; ++i) {
        actual_result = strgrpmatch(tests[i].input, tests[i].pattern, NULL, 0,
                                    STR_MAXIMAL | STR_LEFT | STR_RIGHT);
        expected_result = tests[i].expected_result;

        if (actual_result != expected_result) {
            terror("strgrpmatch() failed :: '%s' failed to match shell pattern '%s'",
                   tests[i].input, tests[i].pattern);
        }
    }
}

void test_unmatching_patterns() {
    int actual_result, expected_result;
    struct ShellPatternMatch tests[] = {{"foo", "f\\*\\*", 0},   {"foo", "f\\?\\?", 0},
                                        {"foo1", "foo[2-9]", 0}, {"foobar", "foo[c-z]ar", 0},
                                        {"f**", "f\\?\\?", 0},   {"f??", "f\\*\\*", 0},
                                        {NULL, NULL, 0}};

    for (int i = 0; tests[i].input; ++i) {
        actual_result = strmatch(tests[i].input, tests[i].pattern);
        expected_result = tests[i].expected_result;

        if (actual_result != expected_result) {
            terror("strmatch() failed :: '%s' matches unmatching shell pattern '%s'",
                   tests[i].input, tests[i].pattern);
        }
    }

    for (int i = 0; tests[i].input; ++i) {
        actual_result = strgrpmatch(tests[i].input, tests[i].pattern, NULL, 0,
                                    STR_MAXIMAL | STR_LEFT | STR_RIGHT);
        expected_result = tests[i].expected_result;

        if (actual_result != expected_result) {
            terror("strgrpmatch() failed :: '%s' matches unmatching shell pattern '%s'",
                   tests[i].input, tests[i].pattern);
        }
    }
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    test_matching_patterns();
    test_unmatching_patterns();

    texit(0);
}
