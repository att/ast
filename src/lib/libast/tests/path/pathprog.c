#include "config_ast.h"  // IWYU pragma: keep

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char path[PATH_MAX];

    pathprog(argv[0], path, sizeof(path));

    if (strcmp(path, argv[0]))
        terror("pathprog() fails to return correct path. Expected: %s Actual: %s", path, argv[0]);

    texit(0);
}
