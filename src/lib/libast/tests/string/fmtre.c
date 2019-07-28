// See also src/lib/libast/tests/string/fmtmatch.c.
// See also src/lib/cmd/ksh93/tests/sh_match.sh.
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

static const struct StringToStringTest tests[] = {
    {"", ""},  // empty ksh pattern should yield an empty ERE pattern
    {"*", "$"},
    {"*(xyz)", "^(xyz)*$"},
    {"*-(x)", "^(x)*?$"},
    {"*-.", "-\\.$"},
    {"*y", "y$"},
    {"?-x", "^.-x$"},
    {"+(xyz)", "^(xyz)+$"},
    {"?(xyz)", "^(xyz)?$"},
    {"@(xyz)", "^(xyz)$"},
    {"a*b", "^a.*b$"},
    {"a?b*", "^a.b"},
    {"a?c*d", "^a.c.*d$"},
    {"x!y", "^x\\!y$"},
    {"x", "^x$"},
    {"x?*|\\(|b?", "^x..*|\\(|(b.$"},
    {"x@y", "^x@y$"},
    {"x~y", "^x~y$"},
    {"x[!abc]y", "^x[^abc]y$"},
    {"x[^]y", "^x\\^y$"},
    {"x[^abc]y", "^x[abc^]y$"},
    {"x|y", "^x|y$"},
    {"@(x|y)", "^(x|y)$"},
    {"x{", "^x{$"},
    {"x{}", "^x{}$"},
    {"{", "^{$"},
    {"}", "^}$"},
    {"{}", "^{}$"},
    {"~(x)[~abc]y", "^(?x)[~abc]y$"},
    {"x{(x)}y", "^x{(x)}y$"},
    {"x{}(a)y", "^x(a){}y$"},
    {"x{abc}(y)", "^x(y){abc}$"},
    {"x{abc}-(y)", "^x(y){abc}?$"},
    {"x{abc}-y", "^x{abc}-y$"},
    {"~(E)z.?a", "^z.?a$"},
    {"\\\\", "^\\\\$"},
    {"+", "^\\+$"},
    {"x*{-(*a)}y", "^x.*{-(.*a)}y$"},
    {"x\\a|b", "^x\\a|b$"},
    {"\\(x", "^\\(x$"},
    {"\\(|x", "^\\(|(x$"},
    {"^", "^\\^$"},
    {"$", "^\\$$"},
    {".", "^\\.$"},
    {"(x|y)", "^(x|y)$"},

    // Ksh patterns that cannot be translated to ERE patterns.
    {"(", NULL},
    {"\\", NULL},
    {"~E)z.?a", NULL},
    {"~(Ez.?a", NULL},
    {"\\(x)", NULL},
    {"\\(|", NULL},
    {"\\(|)", NULL},
    {"|", NULL},
    {"*|", NULL},
    {"(|", NULL},
    {"x|", NULL},
    {"(x|)", NULL},
    {"\\", NULL},
    {"\\{", NULL},
    {"\\}", NULL},
    {"?-(x", NULL},

    // The remaining tests are not meant to be in any manner realistic. They are meant to confirm
    // that the function handles the hardcoded nesting limit of 32 frames is handled correctly. This
    // is meant to test the cases where pushing another level would result in overflowing the fixed
    // size context stack.

    // 32 levels of nesting is okay.
    {"((((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))))",
     "^((((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))))$"},
    // 33 levels of nesting is an error.
    {"(((((((((((((((((((((((((((((((((x)))))))))))))))))))))))))))))))))", NULL},

    // 32 levels of nesting is okay.
    {"{}({}({}({}({}({}({}({}({}({}({}({}({}({}({}({}("
     "{}({}({}({}({}({}({}({}({}({}({}({}({}({}({}({}("
     "x))))))))))))))))))))))))))))))))",
     "^(((((((((((((((((((((((((((((((("
     "x){}){}){}){}){}){}){}){}){}){}){}){}){}){}){}){})"
     "{}){}){}){}){}){}){}){}){}){}){}){}){}){}){}){}$"},
    // 33 levels of nesting is an error.
    {"{}({}({}({}({}({}({}({}({}({}({}({}({}({}({}({}("
     "{}({}({}({}({}({}({}({}({}({}({}({}({}({}({}({}({}("
     "x)))))))))))))))))))))))))))))))))",
     NULL},

    // 32 levels of nesting is okay.
    {"@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@("
     "x))))))))))))))))))))))))))))))))",
     "^((((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))))$"},
    // 33 levels of nesting is an error.
    {"@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@(@("
     "x)))))))))))))))))))))))))))))))))",
     NULL},

    {NULL, NULL}};

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    for (int i = 0; tests[i].input; ++i) {
        char *actual_result = fmtre(tests[i].input);
        // Is the conversion expected to fail and did it fail? Then continue.
        if (!tests[i].expected_result && !actual_result) continue;

        if (!tests[i].expected_result || !actual_result) {
            terror(
                "fmtre() :: Failed to convert ksh pattern to ERE\nInput: %d |%s|\n"
                "Expect: |%s|\nActual: |%s|",
                i, tests[i].input, tests[i].expected_result ? tests[i].expected_result : "(null)",
                actual_result ? actual_result : "(null)");
            continue;
        }

        if (strcmp(actual_result, tests[i].expected_result)) {
            terror(
                "fmtre() :: Failed to convert ksh pattern to ERE\nInput: %d |%s|\n"
                "Expect: |%s|\nActual: |%s|",
                i, tests[i].input, tests[i].expected_result, actual_result);
        }
    }

    texit(0);
}
