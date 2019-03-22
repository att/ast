#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

struct test {
    const char *input;
    int result;
};

// Test for octal numbers. It also checks if next string is returned correctly.
void test_octal() {
    char *input = "\\012\\345\\670";
    char *nextstr = NULL;
    int next, expected;

    next = chresc(input, &nextstr);
    expected = 012;
    if (next != expected)
        terror("chresc() :: Failed to convert octal string \\012 :: Expected: %d, Actual: %d",
               expected, next);

    next = chresc(nextstr, &nextstr);
    expected = 0345;
    if (next != expected)
        terror("chresc() :: Failed to convert octal string \\345 :: Expected: %d, Actual: %d",
               expected, next);

    next = chresc(nextstr, &nextstr);
    expected = 0670;
    if (next != expected)
        terror("chresc() :: Failed to convert octal string \\670:: Expected: %d, Actual: %d",
               expected, next);
}

// Test for hexadecimal numbers
void test_hex() {
    int actual, expected;

    struct test tests[] = {{"\\x01", 1},   {"\\x23", 35},  {"\\x45", 69},  {"\\x67", 103},
                           {"\\x89", 137}, {"\\xab", 171}, {"\\xcd", 205}, {"\\xef", 239},
                           {"\\xAB", 171}, {"\\xCD", 205}, {"\\xEF", 239}, {"\\x{f}", 15},
                           {"\\x[f]", 15}, {NULL, 0}};

    for (int i = 0; tests[i].input; ++i) {
        actual = chresc(tests[i].input, NULL);
        expected = tests[i].result;
        if (actual != expected)
            terror("chresc() failed :: Failed to convert '%s' :: Expected: %d, Actual: %d",
                   tests[i].input, expected, actual);
    }
}

void test_escape_sequences() {
    int actual, expected;
    const char *input;

    struct test tests[] = {{"\\a", CC_bel},
                           {"\\b", '\b'},
                           // TODO: These has been marked as deprecated. Consider removing them.
                           // { "\\c", 0 },
                           // { "\\C", 0 },
                           {"\\e", CC_esc},
                           {"\\E", CC_esc},
                           {"\\M-", CC_esc},
                           {"\\t", '\t'},
                           {"\\n", '\n'},
                           {"\\v", '\v'},
                           {"\\f", '\f'},
                           {"\\r", '\r'},
                           {"\\uf", 15},
                           {"\\wf", 15},
                           {"\\Uf", 15},
                           {"\\Wf", 15},
                           {NULL, 0}};

    for (int i = 0; tests[i].input; ++i) {
        input = tests[i].input;
        actual = chresc(input, NULL);
        expected = tests[i].result;
        if (actual != expected)
            terror("chresc() failed :: Failed to convert '%s' :: Expected: %d, Actual: %d",
                   tests[i].input, expected, actual);
    }
}

void test_normal_string() {
    int actual, expected;
    char input[] = "This is a message";
    char *next_string = input;

    do {
        expected = *next_string;
        actual = chresc(next_string, &next_string);
        if (actual != expected)
            terror("chresc() failed :: Failed to read string :: Expected: %d, Actual: %d", expected,
                   actual);
    } while (*next_string);
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    test_octal();
    test_hex();
    test_escape_sequences();
    test_normal_string();

    texit(0);
}
