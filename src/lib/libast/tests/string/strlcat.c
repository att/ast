#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char buffer[7] = "foo";
    char buffer_to_append[] = "barbaz";

    strlcat(buffer, buffer_to_append, 7);

    if (strcmp(buffer, "foobar"))
        terror("strlcat() :: Failed to append buffer :: Actual Result: %s, Expected Result: %s",
               buffer, "foobar");

    texit(0);
}
