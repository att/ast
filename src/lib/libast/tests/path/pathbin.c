#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    int failed = 0;
    char *result = 0;
    char default_astconf_path[BUFSIZ];

    char *custom_path = "/foo:/bar:/baz";

    // Set custom path for testing
    if (setenv("PATH", custom_path, 1)) {
        fprintf(stderr, "Failed to set custom path");
        failed = 1;
    }

    // Check return value of `pathbin()` when `PATH` is a custom path
    result = pathbin();
    if (strcmp(result, custom_path)) {
        fprintf(stderr, "pathbin() function fails when PATH is set to custom path");
        failed = 1;
    }

    // Unset `PATH`
    if (unsetenv("PATH")) {
        fprintf(stderr, "Failed to set custom path");
        failed = 1;
    }

    // When `PATH` is not set, `pathbin()` gets default value from `confstr()` function
    if (confstr(_CS_PATH, default_astconf_path, BUFSIZ) == 0 && errno == EINVAL) {
        fprintf(stderr, "Failed to get default astconf path");
        failed = 1;
    }

    // Check return value of `pathbin()` when `PATH` is not set
    result = pathbin();
    if (strcmp(result, default_astconf_path)) {
        fprintf(stderr, "pathbin() function fails when PATH is not set");
        failed = 1;
    }

    texit(failed);
}
