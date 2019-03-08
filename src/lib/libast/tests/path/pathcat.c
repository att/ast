#include "config_ast.h"  // IWYU pragma: keep

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    int failed = 0;
    const char *root, *path1, *path2;
    char concat_path[BUFSIZ];
    int separator;
    char *result;

    root = "/foo;";
    path1 = "bar";
    path2 = "baz";
    separator = (int)';';

    // Test when separator exists in path
    result = pathcat(root, separator, path1, path2, concat_path, 256);

    if (strcmp(result, "")) failed = 1;
    if (strcmp("/foo/bar/baz", concat_path) != 0) failed = 1;

    // Test when separator exists at beginning of path
    root = "/foo";
    separator = (int)'/';
    result = pathcat(root, separator, path1, path2, concat_path, 256);
    if (strcmp(result, "foo")) failed = 1;
    if (strcmp("bar/baz", concat_path) != 0) failed = 1;

    // Test when separator does not exist in path
    separator = (int)'\\';
    result = pathcat(root, separator, path1, path2, concat_path, 256);
    if (result != 0) failed = 1;
    if (strcmp("/foo/bar/baz", concat_path) != 0) failed = 1;

    // Test when `path1` is 0
    path1 = 0;
    result = pathcat(root, separator, path1, path2, concat_path, 256);
    if (result != 0) failed = 1;
    if (strcmp("/foo/baz", concat_path) != 0) failed = 1;

    // Test when `path2` is 0
    path1 = "bar";
    path2 = 0;
    result = pathcat(root, separator, path1, path2, concat_path, 256);
    if (result != 0) failed = 1;
    if (strcmp("/foo/bar", concat_path) != 0) failed = 1;

    // Test when both `path1` and `path2` are 0
    path1 = 0;
    path2 = 0;
    result = pathcat(root, separator, path1, path2, concat_path, 256);
    if (result != 0) failed = 1;
    // TODO:  Investigate why `.` is appended if both `path1` and `path2` are 0.
    if (strcmp("/foo/.", concat_path) != 0) failed = 1;

    texit(failed);
}
