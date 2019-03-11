#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char path[PATH_MAX];

    const char *paths = "/usr:/usr/bin:/usr/local/bin";
    pathaccess(paths, 0, 0, 0, path, PATH_MAX);

    if (strcmp(path, "/usr")) terror("pathaccess() failed to resolve path");

    paths = "this_path_does_not_exist";

    if (pathaccess(paths, 0, 0, PATH_ABSOLUTE, path, PATH_MAX))
        terror("pathaccess() returns path to a file that does not exist");

    texit(0);
}
