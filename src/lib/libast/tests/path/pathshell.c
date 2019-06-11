#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char *shell;

    char *shells[] = {"/bin/bash", "/bin/ksh", "/bin/zsh", NULL};
    for (int i = 0; shells[i]; ++i) {
        setenv("SHELL", shells[i], 1);
        shell = pathshell();
        if (strcmp(shell, shells[i])) terror("pathshell() fails when 'SHELL=%s'", shells[i]);
    }

    // If shell is a variant of csh or it does not exists, default path is returned
    char *csh_paths[] = {"/bin/tcsh", "/bin/csh", "/bin/this_shell_does_not_exist", NULL};
    for (int i = 0; csh_paths[i]; ++i) {
        setenv("SHELL", csh_paths[i], 1);
        shell = pathshell();
        if (strcmp(shell, "/bin/sh")) terror("pathshell() fails when 'SHELL=%s'", csh_paths[i]);
    }

    texit(0);
}
