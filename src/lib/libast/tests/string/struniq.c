#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char *unsorted_strings[] = {"Line 9", "Line 8", "Line 7", "Line 6", "Line 5", "Line 4",
                                "Line 3", "Line 2", "Line 1", "Line 9", "Line 8", "Line 7",
                                "Line 6", "Line 5", "Line 4", "Line 3", "Line 2", "Line 1"};

    char *unique_strings[] = {
        "Line 1", "Line 2", "Line 3", "Line 4", "Line 5", "Line 6", "Line 7", "Line 8", "Line 9",
    };

    strsort(unsorted_strings, sizeof(unsorted_strings) / sizeof(char *), strcmp);
    struniq(unsorted_strings, sizeof(unsorted_strings) / sizeof(char *));

    for (int i = 0; i < sizeof(unique_strings) / sizeof(char *); ++i) {
        if (strcmp(unsorted_strings[i], unique_strings[i])) {
            terror(
                "struniq() failed :: Failed to remove duplicate strings :: Actual Result : %s, "
                "Expected Result "
                ": %s",
                unsorted_strings[i], unique_strings[i]);
        }
    }

    texit(0);
}
