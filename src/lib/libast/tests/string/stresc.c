#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    // Very simple test to check if escape sequences are expanded
    // `stresc()` internally uses `chresc()` function to expand escape sequences
    // and it has more thorough tests.
    char input[] = "\\n\\r";
    char expected_result[] = "\n\r";

    if (stresc(input) <= 0 || strcmp(input, expected_result)) {
        terror("stresc() failed :: Actual Result : %s, Expected Result: %s", input,
               expected_result);
    }

    texit(0);
}
