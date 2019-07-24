#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>
#include <sys/types.h>

#include "ast.h"
#include "terror.h"

struct base64_tests {
    const char *string;
    const char *base64_string;
};

struct base64_tests tests[] = {{"VIJZoMLCe1dwwqMiPKC1", "VklKWm9NTENlMWR3d3FNaVBLQzE="},
                               {"CHCqOznM5peGMaIoccsL", "Q0hDcU96bk01cGVHTWFJb2Njc0w="},
                               {"RbaVtZYLF9FmuOemXKzi", "UmJhVnRaWUxGOUZtdU9lbVhLemk="},
                               {"2qKoXd2yH3dcoywNQfDL", "MnFLb1hkMnlIM2Rjb3l3TlFmREw="},
                               {"iVFjwRZTPrXzQoEbnisP", "aVZGandSWlRQclh6UW9FYm5pc1A="},
                               {"JDkAUes8uamPqIPwQKGV", "SkRrQVVlczh1YW1QcUlQd1FLR1Y="},
                               {"bf0bCcTT1LySrR9eHi53", "YmYwYkNjVFQxTHlTclI5ZUhpNTM="},
                               {"wwsbeqm8xYinra82fDlx", "d3dzYmVxbTh4WWlucmE4MmZEbHg="},
                               {"pFkDWTcwbGATCiPJpkhy", "cEZrRFdUY3diR0FUQ2lQSnBraHk="},
                               {"ENwnLtgW5VLvNicthw0N", "RU53bkx0Z1c1Vkx2TmljdGh3ME4="},
                               {NULL, NULL}};

// TODO: What shall we do with the test in 'src/lib/libast/tests/misc/base64.c' ?
tmain() {
    UNUSED(argc);
    UNUSED(argv);

    char encoded_result[1024];
    char decoded_result[1024];
    ssize_t result_size;

    for (int i = 0; tests[i].string; ++i) {
        result_size = base64encode(tests[i].string, strlen(tests[i].string), encoded_result,
                                   sizeof(encoded_result));

        if (result_size != strlen(encoded_result)) {
            terror("base64encode() :: Invalid return value :: Expected: %d, Actual: %d",
                   strlen(encoded_result), result_size);
        }

        if (strcmp(tests[i].base64_string, encoded_result)) {
            terror("base64encode() :: Invalid result :: Expected: %s, Actual: %s",
                   tests[i].base64_string, encoded_result);
        }

        result_size = base64decode(encoded_result, strlen(encoded_result), decoded_result,
                                   sizeof(decoded_result));

        if (result_size != strlen(decoded_result)) {
            terror("base64encode() :: Invalid return value :: Expected: %d, Actual: %d",
                   strlen(decoded_result), result_size);
        }

        if (strcmp(decoded_result, tests[i].string)) {
            terror("base64decode() :: Invalid result :: Expected: %s, Actual: %s", tests[i].string,
                   decoded_result);
        }
    }

    texit(0);
}
