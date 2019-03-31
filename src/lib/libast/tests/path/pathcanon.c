#include "config_ast.h"  // IWYU pragma: keep

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "terror.h"

const char *linkname = "foo";
const char *filename = "foobarbaz";

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char path[PATH_MAX];
    int fd;
    char tmp_parent_realpath[PATH_MAX];

    fd = open(filename, O_CREAT, 0666);
    if (fd == -1) terror("Failed to create test file");
    close(fd);

    if (symlink(filename, linkname) < 0) {
        terror("Failed to create symbolic link");
    }

    if (!realpath("/tmp/..", tmp_parent_realpath)) terror("Failed to get real path of /tmp");

    // Try to resolve physical path from symbolic link
    strcpy(path, linkname);
    pathcanon(path, PATH_MAX, PATH_PHYSICAL);
    if (strcmp(path, filename)) terror("Failed to resolve symbolic link");

    // Try to resolve physical path from broken symbolic link
    unlink(linkname);
    strcpy(path, linkname);
    pathcanon(path, PATH_MAX, PATH_PHYSICAL | PATH_EXISTS);
    if (strcmp(path, "")) terror("Resolving broken link should fail");

    // Try to resolve a single `.`
    strcpy(path, "/tmp/.");
    pathcanon(path, PATH_MAX, 0);
    if (strcmp(path, "/tmp")) terror("Failed to resolve `.` in path");

    // Try to resolve parent directory
    strcpy(path, "/tmp/..");
    pathcanon(path, PATH_MAX, PATH_EXISTS);
    if (strcmp(path, "/")) terror("Failed to resolve `..` in path");

    // Try to resolve non-existing path
    strcpy(path, "/non_existing_path_after_root/..");
    pathcanon(path, PATH_MAX, PATH_PHYSICAL);
    if (strcmp(path, "/")) terror("Failed to resolve path if it contains non-existing directory");

    // Check non-existing path
    strcpy(path, "/non_existing_path_after_root");
    pathcanon(path, PATH_MAX, PATH_EXISTS);
    if (strcmp(path, "")) terror("Resolving non-existing path should fail if `PATH_EXISTS` is set");

    // Try to resolve physical path and check if it exists
    strcpy(path, "/tmp/..");
    pathcanon(path, PATH_MAX, PATH_PHYSICAL | PATH_EXISTS);
    if (strcmp(path, tmp_parent_realpath)) {
        terror("Failed to resolve `..` if `PATH_PHYSICAL | PATH_EXISTS` is set");
    }

    // Try to resolve physical path
    strcpy(path, "/tmp/..");
    pathcanon(path, PATH_MAX, PATH_PHYSICAL);
    if (strcmp(path, tmp_parent_realpath)) {
        terror("Failed to resolve `..` if `PATH_PHYSICAL` is set");
    }

    // This should not verify if each component in path is accessible
    strcpy(path, "/non_existing_path_after_root/..");
    pathcanon(path, PATH_MAX, 0);
    if (strcmp(path, "/")) terror("Failed to resolve non existing with empty flags");

    // `PATH_DOTDOT` verifies if each path is accessible
    strcpy(path, "/non_existing_path_after_root/..");
    pathcanon(path, PATH_MAX, PATH_DOTDOT);
    if (strcmp(path, "")) terror("Resolving non-existing path should fail if `PATH_DOTDOT` is set");

    strcpy(path, "/tmp/..");
    pathcanon(path, PATH_MAX, PATH_DOTDOT);
    if (strcmp(path, "/")) terror("Resolving path fails if `PATH_DOTDOT` is set");

    texit(0);
}
