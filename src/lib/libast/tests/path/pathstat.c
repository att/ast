#include "config_ast.h"  // IWYU pragma: keep

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "terror.h"

const char *link_to_temp_file = "link_to_temp_file";

void cleanup_handler(void) { unlink(link_to_temp_file); }

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    const char *temp_file = "temp_file";

    struct stat statbuf;

    if (open(temp_file, O_CREAT, 0666) < 0) terror("Failed to create temp file");

    if (pathstat(temp_file, &statbuf) && statbuf.st_mode == 0666)
        terror("Failed to stat() %s", temp_file);

    if (symlink(temp_file, link_to_temp_file) < 0)
        terror("Failed to create symbolic link to %s", temp_file);

    atexit(cleanup_handler);

    // Unlink target file to create a broken symbolic link
    unlink(temp_file);

    if (pathstat(link_to_temp_file, &statbuf))
        terror("Failed to stat %s if it's broken", link_to_temp_file);

    texit(0);
}
