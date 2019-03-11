#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <string.h>

#include "ast.h"
#include "terror.h"

const char *relative_path = "foo";

void cleanup_handler(void) { unlink(relative_path); }

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char path[PATH_MAX];
    char expanded_relative_path[PATH_MAX];
    const char cwd[PATH_MAX];

    const char *paths = "/usr:/usr/bin:/usr/local/bin";
    pathaccess(paths, 0, 0, 0, path, sizeof(path));

    if (strcmp(path, "/usr")) terror("pathaccess() failed to resolve path");

    paths = "this_path_does_not_exist";

    if (pathaccess(paths, 0, 0, PATH_ABSOLUTE, path, sizeof(path)))
        terror("pathaccess() returns path to a file that does not exist");

    // Test for relative paths
    if (open(relative_path, O_CREAT, 0666) < 0) {
        terror("Failed to create test file");
    }

    atexit(cleanup_handler);

    if (!getcwd(cwd, sizeof(cwd))) terror("Failed to get current working directory");

    snprintf(expanded_relative_path, sizeof(expanded_relative_path), "%s/%s", cwd, relative_path);

    if (!pathaccess("", relative_path, 0, PATH_ABSOLUTE, path, PATH_MAX) ||
        strcmp(path, expanded_relative_path))
        terror("pathaccess() does not recognize relative paths");

    texit(0);
}
