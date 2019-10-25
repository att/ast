// Verify basic functionality of the `optget_long()` function and related code. When possible we
// also execute each test using the platform `getopt_long()` function since the two should behave
// the same modulo the expected differences.
#include "config_ast.h"  // iwyu pragma: keep

#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "optget_long.h"
#include "terror.h"

struct optget_test {
    int rv;
    int ind;
    int opt;
    char *arg;
    int64_t num;
    bool plus;
};

// Old GNU `getopt_long_only()` implementations have a bug. See `check_for_getopt_long_only_bug()`.
static bool getopt_long_only_works = true;

#define TERROR(line, ...)                        \
    do {                                         \
        (Tstline = line), tsterror(__VA_ARGS__); \
    } while (0)

// Calculate length of a NULL terminated list of strings.
static __inline__ int argv_len(char *const *argv) {
    int n = 0;
    while (*argv++) ++n;
    return n;
}

// Convert a `struct optget_option*` to a `struct option*`. Caller must free the returned pointer.
struct option *construct_struct_option(const struct optget_option *long_opts) {
    int n;
    for (n = 0; long_opts[n].name; ++n) {
        ;  // empty loop
    }
    ++n;
    struct option *lo = malloc(n * sizeof(struct option));

    for (int i = 0; i < n; ++i) {
        lo[i].name = long_opts[i].name;
        lo[i].flag = long_opts[i].flag;
        lo[i].val = long_opts[i].val;
        if (long_opts[i].has_arg == optget_no_arg) {
            lo[i].has_arg = no_argument;
        } else if (long_opts[i].has_arg == optget_required_arg) {
            lo[i].has_arg = required_argument;
        } else {
            lo[i].has_arg = optional_argument;
        }
    }

    return lo;
}

// Execute a sequence of `optget_long()` or `optget_long_only()` calls; verifying each result
// against successive elements in a list of expected values.
static void _test_optget_long(int line, bool long_only, int argc, char *const *argv,
                              const char *short_opts, const struct optget_option *long_opts,
                              const struct optget_test *results) {
    int (*funcp)(int, char *const *, const char *, const struct optget_option *) =
        long_only ? optget_long_only : optget_long;
    const char *funcname = long_only ? "optget_long_only" : "optget_long";

    optget_ind = 0;
    for (int step = 0;; ++step) {
        int rv = funcp(argc, argv, short_opts, long_opts);
        if (rv != results[step].rv) {
            TERROR(line, "%s() step #%d rv wrong: expected %d, got %d", funcname, step,
                   results[step].rv, rv);
        }

        if (optget_ind != results[step].ind) {
            TERROR(line, "%s() step #%d ind wrong: expected %d, got %d", funcname, step,
                   results[step].ind, optget_ind);
        }

        if (results[step].rv == -1) break;  // the test is done, none of the other vars matter

        if (rv == '?' && optget_opt != results[step].opt) {
            TERROR(line, "%s() step #%d opt wrong: expected %d, got %d", funcname, step,
                   results[step].opt, optget_opt);
        }

        if (optget_arg || results[step].arg) {
            bool okay = optget_arg && results[step].arg && !strcmp(optget_arg, results[step].arg);
            if (!okay) {
                TERROR(line, "%s() step #%d arg wrong: expected |%s|, got |%s|", funcname, step,
                       results[step].arg, optget_arg);
            }
        }

        if (optget_plus != results[step].plus) {
            TERROR(line, "%s() step #%d plus wrong: expected %d, got %d", funcname, step,
                   results[step].plus, optget_plus);
        }
        if (rv == -2 && optget_num != results[step].num) {
            TERROR(line, "%s() step #%d opt wrong: expected %" PRId64 ", got %" PRId64, funcname,
                   step, results[step].num, optget_num);
        }
    }
}

// Execute a sequence of `getopt_long()` calls. Verify each result against successive elements in a
// list of expected values.
//
// This verifies that our test produces the expected sequence of results when calling
// `getopt_long()` (or `getopt_long_only()`). Our `optget_long()` implementation is meant to be
// compatible with `getopt_long()`; modulo the two `optget_long()` extensions. So we want to verify
// that our test definition is correct when those two extenions are not used.
//
// This can't be called if the test uses the numeric flag, `#`, or the `+` option prefix supported
// by `optget_long()`. Those features are not supported by `getopt_long()` or emulated by this
// function.
static void _test_getopt_long(int line, bool long_only, int argc, char *const *argv,
                              const char *short_opts, const struct optget_option *long_opts,
                              const struct optget_test *results) {
    if (long_only && !getopt_long_only_works) return;

    int (*funcp)(int, char *const *, const char *, const struct option *, int *) =
        long_only ? getopt_long_only : getopt_long;
    const char *funcname = long_only ? "getopt_long_only" : "getopt_long";
    char so[100] = "+:";  // we require POSIXLY_CORRECT and '?' vs ':' behavior
    (void)strlcpy(so + 2, short_opts, sizeof(so) - 2);
    struct option *lo = construct_struct_option(long_opts);

    optind = opterr = 0;
    for (int step = 0;; ++step) {
        int rv = funcp(argc, argv, so, lo, NULL);
        if (rv != results[step].rv) {
            TERROR(line, "%s() step #%d rv wrong: expected %d, got %d", funcname, step,
                   results[step].rv, rv);
        }

        if (optind != results[step].ind) {
            TERROR(line, "%s() step #%d ind wrong: expected %d, got %d", funcname, step,
                   results[step].ind, optind);
        }

        if (results[step].rv == -1) break;

        if (rv == '?' && optopt != results[step].opt) {
            // Some getopt_long_only() implementations have a bug. Bundled short options like `-ab`,
            // where `a` is recognized and `b` is not, does not correctly set `optopt` to `b`; it
            // sets `optopt` to zero. Our optget_long_only() behaves correctly. This works around
            // that bug to avoid false positive failures.
            if (!(long_only && optopt == 0 && results[step].opt != 0)) {
                TERROR(line, "%s() step #%d opt wrong: expected %d, got %d", funcname, step,
                       results[step].opt, optopt);
            }
        }

        if (optarg || results[step].arg) {
            bool okay = optarg && results[step].arg && !strcmp(optarg, results[step].arg);
            if (!okay) {
                TERROR(line, "%s() step #%d arg wrong: expected |%s|, got |%s|", funcname, step,
                       results[step].arg, optarg);
            }
        }
    }

    free(lo);
}

static void test_optget_long(int line, int argc, char *const *argv, const char *short_opts,
                             const struct optget_option *long_opts,
                             const struct optget_test *results) {
    // If the test includes behavior unique to `optget_long()` skip the verification of the test
    // definition against the  standard `getopt_long()`.
    if (*short_opts != '#' && *short_opts != '+') {
        _test_getopt_long(line, false, argc, argv, short_opts, long_opts, results);
    }

    _test_optget_long(line, false, argc, argv, short_opts, long_opts, results);
}

static void test_optget_long_only(int line, int argc, char *const *argv, const char *short_opts,
                                  const struct optget_option *long_opts,
                                  const struct optget_test *results) {
    // If the test includes behavior unique to `optget_long()` skip the verification of the test
    // definition against the  standard `getopt_long()`.
    if (*short_opts != '#' && *short_opts != '+') {
        _test_getopt_long(line, true, argc, argv, short_opts, long_opts, results);
    }

    _test_optget_long(line, true, argc, argv, short_opts, long_opts, results);
}

// Verify the behavior when there are no short or long options defined. Even when neither, either,
// or both, of the behavior modifier symbols are present in the short options string. Note that the
// presence of those behavior modifier symbols when no other chars are present also means there are
// no options defined.
static void test_no_options() {
    const char *short_opts1 = "";
    const char *short_opts2 = "+";
    const char *short_opts3 = "#";
    const char *short_opts4 = "+#";
    const char *short_opts[] = {short_opts1, short_opts2, short_opts3, short_opts4, NULL};
    struct optget_option long_opts[] = {
        {NULL, optget_no_arg, NULL, 0},
    };
    char *const argv1[] = {"cmd", "-x", "arg1", NULL};
    char *const argv2[] = {"cmd", "--x", "arg1", NULL};
    struct optget_test *results;

    for (const char **so = short_opts; *so; ++so) {
        // No options and no argv entries other than the command name should always return -1 on
        // the first call.
        results = (struct optget_test[]){{.rv = -1, .ind = 1}};
        test_optget_long(__LINE__, 1, argv1, *so, long_opts, results);

        // No options and an argv entry that looks like a short option means an unrecognized
        // option is reported.
        results = (struct optget_test[]){{.rv = '?', .opt = 'x', .ind = 2},  //
                                         {.rv = -1, .ind = 2}};
        test_optget_long(__LINE__, 3, argv1, *so, long_opts, results);

        // No options and an argv entry that looks like a long option means an unrecognized
        // option is reported.
        results = (struct optget_test[]){{.rv = '?', .ind = 2},  //
                                         {.rv = -1, .ind = 2}};
        test_optget_long(__LINE__, 3, argv2, *so, long_opts, results);

        // No options and one argv entry that does not look like an option.
        results = (struct optget_test[]){{.rv = -1, .ind = 1}};
        test_optget_long(__LINE__, 3, argv1 + 1, *so, long_opts, results);
    }
}

// Verify the handling of numeric flags; e.g., `-123`.
static void test_numeric_options() {
    const char *short_opts1 = "";
    const char *short_opts2 = "+";
    const char *short_opts3 = "#";
    const char *short_opts4 = "+#";
    struct optget_option long_opts1[] = {
        {"123", optget_no_arg, NULL, -7},
        {NULL, optget_no_arg, NULL, 0},
    };
    char *const argv[] = {"cmd", "-123", "--123", "arg1", NULL};
    int argc = argv_len(argv);

    // We haven't requested recognition of numbers that look like short flags.
    struct optget_test results1[] = {{.rv = '?', .opt = '1', .ind = 1},  //
                                     {.rv = '?', .opt = '2', .ind = 1},
                                     {.rv = '?', .opt = '3', .ind = 2},
                                     {.rv = -7, .ind = 3},
                                     {.rv = -1, .ind = 3}};
    test_optget_long(__LINE__, argc, argv, short_opts1, long_opts1, results1);
    test_optget_long(__LINE__, argc, argv, short_opts2, long_opts1, results1);

    // We have requested recognition of numbers that look like short flags.
    struct optget_test results2[] = {{.rv = -2, .ind = 2, .num = 123},  //
                                     {.rv = -7, .ind = 3},
                                     {.rv = -1, .ind = 3}};
    test_optget_long(__LINE__, argc, argv, short_opts3, long_opts1, results2);
    test_optget_long(__LINE__, argc, argv, short_opts4, long_opts1, results2);
}

// Verify complex combinations of short and long options, some that take an optional argument and
// some that take a required argument, works correctly.
static void test_complex_options() {
    // Note that using "#" as a short flag is legal even though it's presence as the first char of
    // the short options string is special -- enabling integer as a flag behavior; e.g., `-123`. I
    // hope no one ever uses the two behaviors at the same time but it is allowed.
    const char *short_opts = "a#x::";
    struct optget_option long_opts[] = {
        {"help", optget_no_arg, NULL, -5},
        {"all", optget_no_arg, NULL, 'a'},
        {"level", optget_required_arg, NULL, 'l'},
        {"user", optget_optional_arg, NULL, -3},
        {NULL, optget_no_arg, NULL, 0},
    };
    char **argv;
    int argc;
    struct optget_test *results;

    argv = (char *[]){"cmd",          "-ax",     "-#",   "--help", "--all", "-xX",
                      "--level=lvl1", "--level", "lvl2", "--user", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'a', .ind = 1},  //
                                     {.rv = 'x', .ind = 2},
                                     {.rv = '#', .ind = 3},
                                     {.rv = -5, .ind = 4},
                                     {.rv = 'a', .ind = 5},
                                     {.rv = 'x', .ind = 6, .arg = "X"},
                                     {.rv = 'l', .ind = 7, .arg = "lvl1"},
                                     {.rv = 'l', .ind = 9, .arg = "lvl2"},
                                     {.rv = -3, .ind = 10, .arg = NULL},
                                     {.rv = -1, .ind = 10}};
    test_optget_long(__LINE__, argc, argv, short_opts, long_opts, results);

    // A short option that has an optional value has to bundle the value with the option. Otherwise,
    // if the value is a separate arg it is ignored and terminates the option scan (at least in
    // POSIXLY_CORRECT mode which we require).
    argv = (char *[]){"cmd", "-x", "X", "--help", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'x', .ind = 2},  //
                                     {.rv = -1, .ind = 2}};
    test_optget_long(__LINE__, argc, argv, short_opts, long_opts, results);

    // The special "--" arg terminates the scan. If an arg that looks like a valid option follows
    // that special arg it is treated as a normal, non-option, arg.
    argv = (char *[]){"cmd", "-a", "--", "--help", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'a', .ind = 2},  //
                                     {.rv = -1, .ind = 3}};
    test_optget_long(__LINE__, argc, argv, short_opts, long_opts, results);
}

// Verify the handling of options that are prefixed by `+`. Something unique to ksh; not a POSIX
// mandated behavior.
static void test_plus_options() {
    const char *short_opts1 = "ny";
    const char *short_opts2 = "+ny";
    struct optget_option long_opts1[] = {
        {"help", optget_no_arg, NULL, -5},
        {NULL, optget_no_arg, NULL, 0},
    };
    char **argv;
    int argc;
    struct optget_test *results;

    // We haven't requested recognition of options prefixed by `+` so should recognize "-n" but not
    // "+n".
    argv = (char *[]){"cmd", "-n", "+n", "--help", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'n', .ind = 2},  //
                                     {.rv = -1, .ind = 2}};
    test_optget_long(__LINE__, argc, argv, short_opts1, long_opts1, results);

    // We have requested recognition of options prefixed by `+` so should recognize "-n" and "+n".
    argv = (char *[]){"cmd", "-n", "-y", "+y", "+n", "-n", "--help", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'n', .ind = 2, .plus = false},  //
                                     {.rv = 'y', .ind = 3, .plus = false},
                                     {.rv = 'y', .ind = 4, .plus = true},
                                     {.rv = 'n', .ind = 5, .plus = true},
                                     {.rv = 'n', .ind = 6, .plus = false},
                                     {.rv = -5, .ind = 7},
                                     {.rv = -1, .ind = 7}};
    test_optget_long(__LINE__, argc, argv, short_opts2, long_opts1, results);

    // We have requested recognition of options prefixed by `+` so should recognize "-n" and "+n"
    // plus bundling behind either prefix.
    argv = (char *[]){"cmd", "-ny", "+yn", "--", "--help", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'n', .ind = 1, .plus = false},  //
                                     {.rv = 'y', .ind = 2, .plus = false},
                                     {.rv = 'y', .ind = 2, .plus = true},
                                     {.rv = 'n', .ind = 3, .plus = true},
                                     {.rv = -1, .ind = 4}};
    test_optget_long(__LINE__, argc, argv, short_opts2, long_opts1, results);

    // We have requested recognition of short options prefixed by `+` but there is an unrecognized
    // `+y` argument.
    argv = (char *[]){"cmd", "-n", "-y", "+y", "+n", "-n", "--help", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'n', .ind = 2},  //
                                     {.rv = 'y', .ind = 3},
                                     {.rv = -1, .ind = 3}};
    test_optget_long(__LINE__, argc, argv, short_opts1, long_opts1, results);

    // We have requested recognition of short options prefixed by `+` and all options are
    // recognized.
    argv = (char *[]){"cmd", "-n", "-y", "+y", "+n", "-n", "--help", NULL};
    argc = argv_len(argv);
    results = (struct optget_test[]){{.rv = 'n', .ind = 2},  //
                                     {.rv = 'y', .ind = 3},
                                     {.rv = 'y', .ind = 4, .plus = true},
                                     {.rv = 'n', .ind = 5, .plus = true},
                                     {.rv = 'n', .ind = 6},
                                     {.rv = -5, .ind = 7},
                                     {.rv = -1, .ind = 7}};
    test_optget_long(__LINE__, argc, argv, short_opts2, long_opts1, results);
}

// Verify the handling of long flags with a short flag prefix when we explicitly recognize that
// situation; e.g., `kill -HUP` being equivalent to `kill --HUP` rather than `kill -H -U -P`.
//
// Note that `getopt_long_only()` and our `optget_long_only()` are slightly weird. For example, you
// can bundle a short option followed by a long option. I think this is slightly weird because I
// would expect that after seeing a valid short option only short options would be recognized until
// the next argument is scanned. But that is not how modern `getopt_long_only()` implementations
// behave and we intend to be compatible with those implementations.
static void test_long_only() {
    struct optget_option long_opts[] = {
        {"123", optget_no_arg, NULL, -7},
        {"all", optget_no_arg, NULL, 'a'},
        {NULL, optget_no_arg, NULL, 0},
    };
    char *const argv[] = {"cmd", "-vHUP", "-123",  "-v",  "-456", "-vall", "-x",   "-al",
                          "-vx", "--123", "--all", "--a", "--",   "-v",    "arg1", NULL};
    int argc = argv_len(argv);
    struct optget_test results[] = {{.rv = 'v', .ind = 1},   // -vHUP
                                    {.rv = '?', .ind = 2},   // -vHUP
                                    {.rv = -7, .ind = 3},    // -123
                                    {.rv = 'v', .ind = 4},   // -v
                                    {.rv = '?', .ind = 5},   // -456
                                    {.rv = 'v', .ind = 5},   // -vall
                                    {.rv = 'a', .ind = 6},   // -vall
                                    {.rv = '?', .ind = 7},   // -x  notice .opt isn't set to 'x'
                                    {.rv = 'a', .ind = 8},   // -al
                                    {.rv = 'v', .ind = 8},   // -vx
                                    {.rv = '?', .ind = 9},   // -vx  notice .opt isn't set to 'x'
                                    {.rv = -7, .ind = 10},   // -123
                                    {.rv = 'a', .ind = 11},  // --all
                                    {.rv = 'a', .ind = 12},  // --a
                                    {.rv = -1, .ind = 13}};  // --

    // We haven't requested recognition of numbers that look like short options as well as treating
    // unrecognized bundles of short options as if they were long options.
    test_optget_long_only(__LINE__, argc, argv, "v", long_opts, results);

    // We have requested recognition of numbers that look like short options as well as treating
    // unrecognized bundles of short options as if they were long options.
    //
    // This should produce the same sequence of results as the "v" case because "long_only" takes
    // precedence over short flags that otherwise look like numbers. In other words, you can't
    // meaningfully use the two behaviors together to distinguish the two cases.
    test_optget_long_only(__LINE__, argc, argv, "#v", long_opts, results);
}

// Basic tests that options which require, or have optional, values are handled correctly.
static void test_options_with_values() {
    const char *short_opts = "a:bx:y::";
    struct optget_option long_opts[] = {
        {"all", optget_required_arg, NULL, 'a'},
        {"yes", optget_optional_arg, NULL, -3},
        {NULL, optget_no_arg, NULL, 0},
    };
    char *const argv[] = {"cmd",       "-a",    "aaa",       "-b",  "-aabc", "-b",   "-xxxx",
                          "-x",        "def",   "--all",     "ALL", "--yes", "-y",   "-b",
                          "--yes=YES", "-bxNO", "--yes=why", "-y",  "--",    "arg1", NULL};
    int argc = argv_len(argv);
    struct optget_test *results;

    results = (struct optget_test[]){{.rv = 'a', .ind = 3, .arg = "aaa"},  //
                                     {.rv = 'b', .ind = 4},
                                     {.rv = 'a', .ind = 5, .arg = "abc"},
                                     {.rv = 'b', .ind = 6},
                                     {.rv = 'x', .ind = 7, .arg = "xxx"},
                                     {.rv = 'x', .ind = 9, .arg = "def"},
                                     {.rv = 'a', .ind = 11, .arg = "ALL"},
                                     {.rv = -3, .ind = 12},
                                     {.rv = 'y', .ind = 13},
                                     {.rv = 'b', .ind = 14},
                                     {.rv = -3, .ind = 15, .arg = "YES"},
                                     {.rv = 'b', .ind = 15},
                                     {.rv = 'x', .ind = 16, .arg = "NO"},
                                     {.rv = -3, .ind = 17, .arg = "why"},
                                     {.rv = 'y', .ind = 18},
                                     {.rv = -1, .ind = 19}};
    test_optget_long(__LINE__, argc, argv, short_opts, long_opts, results);
}

// Old GNU `getopt_long_only()` implementations, such as found in Fedora 28, have a bug which makes
// them unsuitable for verifying the correctness of our tests.
void check_for_getopt_long_only_bug() {
    char *const argv[] = {"cmd", "-vHUP", "arg1", NULL};
    int argc = argv_len(argv);
    struct option long_opts[] = {
        {NULL, optget_no_arg, NULL, 0},
    };
    int rv;

    optind = opterr = 0;
    rv = getopt_long_only(argc, argv, "+:v", long_opts, NULL);
    if (rv != 'v' || optind != 1) goto broken;
    rv = getopt_long_only(argc, argv, "+:v", long_opts, NULL);
    if (rv != '?' || optind != 2) goto broken;
    return;  // looks okay to use

broken:
    tinfo("getopt_long_only() is broken -- skipping those tests");
    getopt_long_only_works = false;
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    check_for_getopt_long_only_bug();

    // Test capabilities of `optget_long()` where we should be compatible with `getopt_long()`.
    test_no_options();
    test_options_with_values();
    test_complex_options();

    // Test capabilities of `optget_long()` not supported by `getopt_long()`. Note that some of the
    // sub-tests will still verify compatibility with `getopt_long()`.
    test_numeric_options();
    test_plus_options();
    test_long_only();

    texit(0);
}
