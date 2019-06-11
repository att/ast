/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
 *                      and is licensed under the                       *
 *                 Eclipse Public License, Version 1.0                  *
 *                    by AT&T Intellectual Property                     *
 *                                                                      *
 *                A copy of the License is available at                 *
 *          http://www.eclipse.org/org/documents/epl-v10.html           *
 *         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
 *                                                                      *
 *              Information and Software Systems Research               *
 *                            AT&T Research                             *
 *                           Florham Park NJ                            *
 *                                                                      *
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                                                                      *
 ***********************************************************************/

#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"
#include "stdlib.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    struct StringToStringTest tests[] = {
        {"foo=bar", "bar"}, {"foo=baz", "baz"}, {"foo", NULL}, {NULL, NULL}};

    const char *actual_result;

    for (int i = 0; tests[i].input; ++i) {
        sh_setenviron(tests[i].input);
        actual_result = sh_getenv("foo");
        if (tests[i].expected_result && strcmp(actual_result, tests[i].expected_result)) {
            terror("Expected Result: %s, Actual Result: %s", tests[i].expected_result,
                   actual_result);
        } else if (tests[i].expected_result == NULL && actual_result != NULL) {
            terror("Expected Result: NULL, Actual Result: %s", actual_result);
        }
    }

    texit(0);
}
