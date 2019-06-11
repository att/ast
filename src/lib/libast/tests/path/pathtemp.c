#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <unistd.h>

#include "ast.h"
#include "terror.h"

const char *temp_file = NULL;

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char cwd[PATH_MAX];
    int fd;

    if (!getcwd(cwd, sizeof(cwd))) terror("Failed to get current directory");

    temp_file = ast_temp_file(cwd, NULL, &fd, 0);
    if (!temp_file || fd < 0) terror("Failed to create temporary file");

    texit(0);
}
