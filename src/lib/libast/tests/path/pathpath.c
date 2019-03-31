#include "config_ast.h"  // IWYU pragma: keep

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "terror.h"

char test_path[PATH_MAX] = "foo";

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char path[PATH_MAX];
    char absolute_path[PATH_MAX];
    char absolute_test_path[PATH_MAX];

    getcwd(absolute_test_path, sizeof(absolute_test_path));
    strcat(absolute_test_path, "/");
    strcat(absolute_test_path, "foo");

    int fd = open(test_path, O_CREAT, 0666);
    if (fd == -1) terror("Failed to create test file");

    // Search this path in current directory and expand it
    pathpath("foo", NULL, PATH_ABSOLUTE, path, sizeof(path));
    if (strcmp(path, absolute_test_path))
        terror("Failed to expand path of file in current directory");

    if (!pathpath("cat", NULL, PATH_EXECUTE, absolute_path, sizeof(absolute_path)))
        terror("Failed to find `cat` in current $PATH");

    if (!pathpath(absolute_path, NULL, PATH_EXECUTE, path, sizeof(path)))
        terror("Failed to find `%s` with absolute path", path);

    // Tests for serching paths through `FPATH`
    setenv("PATH", "", 1);
    setenv("FPATH", "/bin:/usr/bin:/usr/local/bin", 1);
    if (!pathpath("cat", NULL, PATH_EXECUTE, path, sizeof(path)))
        terror("Failed to search path through `FPATH`");

    texit(0);
}
