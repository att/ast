#include "config_ast.h"  // IWYU pragma: keep

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    const char *linkname = "foo";

    char buff[10];
    const char *filename = "foobarbaz";
    char smallbuff[4];

    int fd = open(filename, O_CREAT, 0666);
    if (fd == -1) terror("Failed to create test file");
    close(fd);

    if (symlink(filename, linkname) < 0) {
        terror("Failed to create symbolic link");
    }

    if ((pathgetlink(linkname, buff, sizeof(buff)) < 0) || strcmp(buff, filename)) {
        terror("pathgetlink() should return filename of link");
    }

    if (pathgetlink(linkname, smallbuff, sizeof(smallbuff)) > 0) {
        terror("pathgetlink() should fail when buffer is small");
    }

    texit(0);
}
