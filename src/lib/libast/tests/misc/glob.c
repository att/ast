#include "config_ast.h"  // iwyu pragma: keep

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast_glob.h"
#include "terror.h"

// This is a bare miniminum test of the ast_glob() function. It's primary purpose is to verify that
// it behaves sanely when passed no flags and no discipline function is used.
static void test_glob_minimalist() {
    glob_t gdata;
    int n;
    int flags = 0;
    static const char *all_paths[] = {"glob.err", "glob.out", "home", "space dir", "space dir/xyz"};

    memset(&gdata, 0, sizeof(gdata));
    n = ast_glob("**", GLOB_STARSTAR, NULL, &gdata);
    if (n) terror("ast_glob() rv: %d", n);
    if (gdata.gl_pathc != sizeof(all_paths) / sizeof(*all_paths)) {
        terror("ast_glob gdata.gl_pathc wrong: %d", gdata.gl_pathc);
    }
    for (int i = 0; i < gdata.gl_pathc; ++i) {
        if (strcmp(all_paths[i], gdata.gl_pathv[i])) {
            terror("ast_glob() %d expect != actual: |%s| != |%s|", i, all_paths[i],
                   gdata.gl_pathv[i]);
        }
    }

    memset(&gdata, 0, sizeof(gdata));
    n = ast_glob("* *", flags, NULL, &gdata);
    if (n) terror("ast_glob() rv: %d", n);
    if (gdata.gl_pathc != 1) terror("ast_glob gdata.gl_pathc: %d", gdata.gl_pathc);
    if (strcmp(all_paths[3], gdata.gl_pathv[0])) {
        terror("ast_glob() %d expect != actual: |%s| != |%s|", 0, all_paths[3], gdata.gl_pathv[0]);
    }

    memset(&gdata, 0, sizeof(gdata));
    n = ast_glob("glob.*", flags, NULL, &gdata);
    if (n) terror("ast_glob() rv: %d", n);
    if (gdata.gl_pathc != 2) terror("ast_glob gdata.gl_pathc: %d", gdata.gl_pathc);
    for (int i = 0; i < gdata.gl_pathc; ++i) {
        if (strcmp(all_paths[i], gdata.gl_pathv[i])) {
            terror("ast_glob() %d expect != actual: |%s| != |%s|", i, all_paths[i],
                   gdata.gl_pathv[i]);
        }
    }
}

// Test GLOB_COMPLETE.
static void test_glob_complete() {
    glob_t gdata;
    int n;
    static const char *all_paths[] = {"space dir/xyz"};

    setenv("PATH", ".:space dir", 1);

    memset(&gdata, 0, sizeof(gdata));
    n = ast_glob("???", GLOB_COMPLETE, NULL, &gdata);
    if (n) terror("ast_glob() rv: %d", n);
    if (gdata.gl_pathc != sizeof(all_paths) / sizeof(*all_paths)) {
        terror("ast_glob gdata.gl_pathc wrong: %d", gdata.gl_pathc);
    }
    for (int i = 0; i < gdata.gl_pathc; ++i) {
        if (strcmp(all_paths[i], gdata.gl_pathv[i])) {
            terror("ast_glob() %d expect != actual: |%s| != |%s|", i, all_paths[i],
                   gdata.gl_pathv[i]);
        }
    }
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    int fd = open("space dir/xyz", O_CREAT | O_TRUNC | O_WRONLY, 0755);
    close(fd);

    test_glob_minimalist();
    test_glob_complete();
    texit(0);
}
