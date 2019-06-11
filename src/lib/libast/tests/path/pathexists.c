#include "config_ast.h"  // IWYU pragma: keep

#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "terror.h"

char test_path[PATH_MAX] = "foo";

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char path[PATH_MAX];

    int fd = open(test_path, O_CREAT, 0666);
    if (fd == -1) terror("Failed to create test file");

    if (!getcwd(path, PATH_MAX)) terror("Failed to get current directory");

    strcat(path, "/");
    strcat(path, test_path);

    if (pathexists(path, PATH_READ) != 1) terror("Path should be readable");
    // Second call to same path should pick up permissions from cache
    if (pathexists(path, PATH_READ) != 1)
        terror("pathexists() fails to get permissions from cache");

    // https://github.com/att/ast/issues/1215 - `pathexists()` function does not invalidate cache
    // Uncomment following test case when this bug has been fixed.
    // Remove all permissions from file
    // if (chmod(path, 0) < 0) terror("Failed to change permissions");

    // if (pathexists(path, PATH_READ) == 1) terror("Path should not be readable");

    texit(0);
}
