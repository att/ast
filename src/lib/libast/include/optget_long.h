// Symbols for use with the optget_long() function.
#ifndef __OPTGET_LONG_H
#define __OPTGET_LONG_H

#include <stdbool.h>
#include <stdint.h>

// The first three vars mirror the getopt() optind, optopt, and optarg vars. The fourth is present
// to make distingquishing options prefixed by a `+` rather than `-` sign easier.
extern int optget_ind;      // how many args have been scanned
extern int optget_opt;      // set to the option character which was unrecognized
extern char *optget_arg;    // points to the value associated with an option
extern bool optget_plus;    // true if option prefix was `+`, false if `-`
extern int64_t optget_num;  // set to the number found

// In getopt_long() implementations these are named `no_argument`, `required_argument`, and
// `optional_argument`. They're also likely to be preprocessor symbols; i.e, `#define` symbols. The
// `no_argument` symbol is zero in those implementations. We employ the same convention even though
// it would be safer to use a non-zero value.
enum long_arg { optget_no_arg = 0, optget_required_arg, optget_optional_arg };

// We don't name this `option` so that we can also `#include <getopt.h>`
// without causing a name clash.
struct optget_option {
    const char *name;
    enum long_arg has_arg;
    int *flag;
    int val;
};

extern int optget_long(int argc, char *const *argv, const char *short_opts,
                       const struct optget_option *long_opts);

// This is meant for those rare situations where a magic word is prefixed by `-` and thus looks like
// a short flag. For example, `kill -HUP`. Unless you absolutely have to use this do not use it.
// This function has some unexpected behaviors such as letting you concatenate short and long
// options in the same argument. If you simply need to support numbers that look like bundled short
// options (e.g., `-123`) simply preface the `short_opts` string with `#`.
extern int optget_long_only(int argc, char *const *argv, const char *short_opts,
                            const struct optget_option *long_opts);

#endif  // !__OPTGET_LONG_H
