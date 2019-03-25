#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    const char *actual_result = NULL;

    struct StringToStringTest tests[] = {
        {"%c", "c"}, {"%d", "i"}, {"%D", "i"},   {"%f", "f"},      {"%h", "h"}, {"%i", "i"},
        {"%j", "j"}, {"%l", "l"}, {"%p", "p"},   {"%s", "s"},      {"%t", "t"}, {"%z", "z"},
        {"%e", "d"}, {"%g", "d"}, {"%*d", "1i"}, {"%(123)d", "i"}, {NULL, NULL}};

    for (int i = 0; tests[i].input; ++i) {
        actual_result = fmtfmt(tests[i].input);
        if (strcmp(actual_result, tests[i].expected_result)) {
            terror(
                "fmtfmt() :: Failed to convert format to signature :: Expected Result : %s, Actual "
                "Result : %s\n",
                tests[i].expected_result, actual_result);
        }
    }

    texit(0);
}
