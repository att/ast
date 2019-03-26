#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "terror.h"

struct Test {
    const char *permissions_to_apply;
    int existing_permissions;
    int expected_result;
};

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char *error;
    int actual_result;

    struct Test tests[] = {{"g=rwx", 0700, 0770},
                           {"o=rwx", 0700, 0707},
                           {"u+rwx,g+rwx,o+rwx", 0, 0777},
                           {"a+rwx", 0, 0777},
                           {"a+rwX", 0, 0666},
                           {"u&rw,g-rwx,o-rwx", 0777, 0600},
                           {"u|rwx,g-rwx,o-rwx", 0077, 0700},
                           {"u-rwx,g-rwx,o-rwx", 0777, 0},
                           {"s,u=rwx", 0777, 02700},
                           {"0755", 0, 0755},
                           {NULL, 0, 0}};

    for (int i = 0; tests[i].permissions_to_apply; ++i) {
        error = NULL;
        actual_result =
            strperm(tests[i].permissions_to_apply, &error, tests[i].existing_permissions);
        if (actual_result != tests[i].expected_result || *error) {
            terror(
                "strperm() failed :: Failed to apply permissions :: Actual Result : %o, Expected "
                "Result : %o",
                actual_result, tests[i].expected_result);
        }
    }

    texit(0);
}
