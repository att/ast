#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char buffer[4] = "foo";
    char buffer_to_append[] = "barbaz";

    strlcpy(buffer, buffer_to_append, sizeof(buffer));

    if (strcmp(buffer, "bar"))
        terror(
            "strlcat() :: Failed to copy buffer with truncation :: Actual Result: %s, Expected "
            "Result: %s",
            buffer, "bar");

    texit(0);
}
