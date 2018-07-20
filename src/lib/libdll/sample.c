#include "config_ast.h"  // IWYU pragma: keep

#include <stdio.h>

// Version of the libast API that plugin is linked to.
unsigned long plugin_version(void) { return 20131127; }

int b_sample(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    printf("This is a sample builtin");
    fflush(stdout);
    return 0;
}
