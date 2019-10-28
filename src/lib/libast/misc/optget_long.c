// This is derived from the GNU `getopt_long()` implementation but has been drastically altered.
// Both to simplify that code by eliminating features we don't need and also to implement behaviors
// of the legacy AST `optget()` function that `optget_long()` replaces.
//
// The `optget_long()` function in this module is similar to `getopt_long()` but incorporates
// features from the old AST `optget()` function that it replaces. In particular it has these
// characteristics:
//
// a) Only supports POSIXLY_CORRECT mode.
//
// b) Optionally allows flags to have a "+" prefix as well as "-" by putting a "+" at the front of
//    the short option string.
//
// c) Optionally allows a numeric flag; e.g., -123 by putting a `#` at the front of
//    the short option string. In this case the special value -2 is returned as the short option
//    that was found and optget_num is set to the number masquarading as a flag (but with the "-"
//    prefix ignored). You cannot prefix the number with "+" even if the short_opts string contains
//    "+". Nor can you specify negative numbers via this mechanism (e.g., `--123`) because the
//    legacy AST `optget()` doesn't allow that. We might want to relax this restriction.
//
// d) The magic prefix characters recognized in the short option string are different from those of
//    the borg standard getopt() and getopt_long() APIs.
//
// e) Allows ambiguous long options; e.g., `--he` as equivalent to `--help` assuming "help" is a
//    valid long option and "he" is not a valid long option.
//
// If you need to specify both (b) and (c) behaviors you must use "+#", not "#+".
//
// Note that using "#" as a short flag is legal even though it's presence as the first char of the
// short options string is special -- enabling integers as a flag behavior; e.g., `-123`. If you
// want to support "-#" but do not want to allow `-123` as a magic number option you have to have at
// least one other short option letter before the "#" character in the short option string.
//
// A short flag that looks like numeric value, if '#' is in the short_opts, returns -2 when such a
// magic number is scanned. And `optget_num` is set to the value.
//
// Similarly this implementation does not implement some `getopt_long()` behaviors (GNU or BSD). For
// example, it always returns '?' for an unknown option and ':' for an option missing a value. You
// can't select the legacy behavior where it would return '?' for both conditions. It does implement
// the GNU `getopt_long()` behavior of specifying a short option that takes an optional argument
// when two consecutive colons following the short option character. Like GNU `getopt()` the value
// must be bundled with the short option. Otherwise it is ignored and terminates the scan.
//
// Also, because the AST `optget()` function did not support it, this implementation does not map
// "-W;foo" to "--foo" like GNU `getopt()` does. If the caller needs this they have to put `W:` in
// the short_opts string and handle the mapping themself.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ast.h"  // for strton64()
#include "ast_assert.h"
#include "optget_long.h"

// The first three vars mirror the getopt() optind, optopt, and optarg vars. The fourth is present
// to make distingquishing options prefixed by a `+` rather than `-` sign easier.
int optget_ind = 0;        // how many args have been scanned
int optget_opt = 0;        // set to the option character which was unrecognized
char *optget_arg = NULL;   // points to the value associated with an option
bool optget_plus = false;  // true if option prefix was `+`, false if `-`
int64_t optget_num = 0;    // set to the number found

static int _first_nonopt = 0;
static int _last_nonopt = 0;
static const char *_next_char = NULL;
static bool _plus_prefix_allowed = false;
static bool _numeric_flag_allowed = false;

// Initialize our scanning state.
static const char *optget_init(const char *short_opts) {
    optget_ind = 1;  // skip argv[0], the command/program name
    _first_nonopt = _last_nonopt = optget_ind;
    _next_char = NULL;
    if (short_opts[0] == '+') {
        _plus_prefix_allowed = true;
        short_opts++;
    } else {
        _plus_prefix_allowed = false;
    }
    if (short_opts[0] == '#') {
        _numeric_flag_allowed = true;
        short_opts++;
    } else {
        _numeric_flag_allowed = false;
    }
    return short_opts;
}

//
// Scan elements of argv, whose length is argc, for option characters given in `short_opts`.
//
// If an element of argv starts with `-`, and is not exactly "-" or "--", then it is an option
// element.  The characters of this element (aside from the initial `-`) are option characters.  If
// `optget_long()` is called repeatedly, it returns successively each of the option characters from
// each of the option elements.
//
// If `optget_long()` finds another option character, it returns that character, updating
// `optget_ind` and `_next_char` so that the next call to `optget_long()` can resume the scan with
// the following option character or argv element.
//
// If there are no more option characters, `optget_long()` returns -1. Then `optget_ind` is the
// index in argv of the first argv element that is not an option.
//
// `short_opts` is a string containing the legitimate option characters. If an option character is
// seen that is not listed in short_opts return `?`.
//
// If a char in short_opts is followed by a colon, that means it wants an arg, so the following text
// in the same argv element, or the text of the following argv element, is returned in `optget_arg`.
// Two colons mean an option that wants an optional arg; if there is text in the current
// argv element, it is returned in `optget_arg`, otherwise `optget_arg` is set to zero.
//
// Long options begin with "--" instead of "-". Their names may be abbreviated as long as the
// abbreviation is unique or is an exact match for some defined option.  If they have an argument,
// it follows the option name in the same argv element, separated from the option name by a `=`, or
// else the in next argv element. When `optget_long()` finds a long-named option, it returns 0 if
// that option's `flag` field is nonzero, the value of the option's `val` field if the `flag` field
// is zero.
//
// `long_opts` is a vector of `struct optget_option` terminated by an element containing a name
// which is zero.
//
static int _optget_long(int argc, char *const *argv, const char *short_opts,
                        const struct optget_option *long_opts, bool long_only) {
    if (argc < 1) return -1;
    if (optget_ind == 0) optget_init(short_opts);  // initialize our scanning state

    // Make sure some results aren't carried forward from the previous call. This is slightly
    // different from most `getopt_long()` implementations but is safer and saner.
    optget_opt = 0;
    optget_arg = NULL;
    // We don't do `optget_plus = false;` at this point because bundled short options need to
    // remember which prefix was used when the first short option was scanned.

    if (!_next_char || *_next_char == '\0') {  // advance to the next argv element
        // Give _first_nonopt & _last_nonopt rational values if optget_ind has been moved back by
        // the user. Who may also have changed the arguments.
        if (_last_nonopt > optget_ind) _last_nonopt = optget_ind;
        if (_first_nonopt > optget_ind) _first_nonopt = optget_ind;

        // The special argv element `--` means stop scanning for options. Skip it as if it were a
        // null option, then skip everything else like a non-option.
        if (optget_ind != argc && !strcmp(argv[optget_ind], "--")) {
            optget_ind++;
            // This condition should never be true since we always use POSIXLY_CORRECT behavior and
            // thus do not permute the args. The GNU version calls exchange() if the assertion
            // failed. Presumably due to its support for non-POSIXLY_CORRECT behavior. But I'm
            // paranoid; hence the assertion.
            assert(!(_first_nonopt != _last_nonopt && _last_nonopt != optget_ind));

            if (_first_nonopt == _last_nonopt) _first_nonopt = optget_ind;
            _last_nonopt = argc;
            optget_ind = argc;
        }

        // If we have done all the argv elements, stop the scan and back over any non-options that
        // we skipped.
        if (optget_ind == argc) {
            // Set the next-arg-index to point at the non-options that we previously skipped, so the
            // caller will digest them.
            if (_first_nonopt != _last_nonopt) optget_ind = _first_nonopt;
            return -1;
        }

        // If we have come to a non-option stop the scan.
        if (argv[optget_ind][1] == '\0') return -1;
        char c = argv[optget_ind][0];
        optget_plus = (c == '+');
        if (!(c == '-' || (_plus_prefix_allowed && optget_plus))) return -1;

        // We have found another option argv element. Skip the initial punctuation.
        _next_char = argv[optget_ind] + 1;
        if (!optget_plus && long_opts && argv[optget_ind][1] == '-') _next_char++;
    }

    // Decode the current option argv element.

    // Check whether the argv element is a long option.
    //
    // If long_only and the argv element has the form "-f", where f is a valid short option, don't
    // consider it an abbreviated form of a long option that starts with f.  Otherwise there would
    // be no way to give the "-f" short option.
    //
    // On the other hand, if there's a long option "fubar" and the argv element is "-fu", do
    // consider that an abbreviation of the long option, just like "--fu", and not "-f" with
    // arg "u".
    //
    // This distinction seems to be the most useful approach.
    if (long_opts && !optget_plus &&
        (argv[optget_ind][1] == '-' ||
         (long_only && (argv[optget_ind][2] || !strchr(short_opts, argv[optget_ind][1]))))) {
        const char *name_end;
        const struct optget_option *p;
        const struct optget_option *pfound = NULL;
        int exact = 0;
        int ambig = 0;
        int option_index;

        for (name_end = _next_char; *name_end && *name_end != '='; name_end++) {
            ;  // empty loop
        }

        // Test all long options for either exact match or abbreviated matches.
        for (p = long_opts, option_index = 0; p->name; p++, option_index++) {
            if (strncmp(p->name, _next_char, name_end - _next_char)) continue;
            if ((unsigned int)(name_end - _next_char) == (unsigned int)strlen(p->name)) {
                // Exact match found.
                pfound = p;
                exact = 1;
                break;
            } else if (!pfound) {
                // First nonexact match found.
                pfound = p;
            } else if (long_only || pfound->has_arg != p->has_arg || pfound->flag != p->flag ||
                       pfound->val != p->val) {
                // Second or later nonexact match found.
                ambig = 1;
            }
        }

        if (ambig && !exact) {
            _next_char += strlen(_next_char);
            optget_ind++;
            optget_opt = 0;
            return '?';
        }

        if (pfound) {
            optget_ind++;
            if (*name_end) {
                if (pfound->has_arg != optget_no_arg) {
                    optget_arg = (char *)name_end + 1;  // discard `const` qualifier
                } else {
                    _next_char += strlen(_next_char);
                    optget_opt = pfound->val;
                    return '?';
                }
            } else if (pfound->has_arg == optget_required_arg) {
                if (optget_ind < argc) {
                    optget_arg = (char *)argv[optget_ind++];  // discard `const` qualifier
                } else {
                    _next_char += strlen(_next_char);
                    optget_opt = pfound->val;
                    return ':';
                }
            }
            _next_char += strlen(_next_char);
            if (pfound->flag) {
                *(pfound->flag) = pfound->val;
                return 0;
            }
            return pfound->val;
        }

        // Can't find it as a long option.  If the option starts with `--` or is not a valid short
        // option, then it's an error. Otherwise interpret it as a short option.
        if (!long_only || argv[optget_ind][1] == '-' || !strchr(short_opts, *_next_char)) {
            optget_ind++;
            _next_char = (char *)"";
            return '?';
        }
    }

    // Look at and handle the next short option-character.
    char c = *_next_char++;
    const char *temp = strchr(short_opts, c);

    // If it isn't a recognized short option, and we allow numeric flags, see if it is a recognized
    // number. If it is return the number.
    if (!temp && _numeric_flag_allowed) {
        char *cp;
        optget_num = strton64(_next_char - 1, &cp, NULL, 0);
        if (!*cp) {  // looks like a number so return it
            ++optget_ind;
            _next_char = NULL;  // force advancing to the next argv element on the next call
            return -2;          // tell the caller a number masquerading as a short option was seen
        }
    }

    // Increment `optget_ind` when we process the last character of the current arg.
    if (*_next_char == '\0') ++optget_ind;

    if (!temp || c == ':' || c == ';') {
        optget_opt = c;
        return '?';
    }

    if (temp[1] != ':') return c;
    if (temp[2] == ':') {
        // This is an option that accepts an argument optionally.
        if (*_next_char != '\0') {
            optget_arg = (char *)_next_char;  // discard `const` qualifier
            optget_ind++;
        } else {
            optget_arg = NULL;
        }
        _next_char = NULL;
        return c;
    }

    // This is an option that requires an argument.
    if (*_next_char != '\0') {
        optget_arg = (char *)_next_char;  // discard `const` qualifier
        // If we end this argv element by taking the rest as an arg, we must advance to the
        // next element now.
        optget_ind++;
    } else if (optget_ind == argc) {
        optget_opt = c;
        c = ':';
    } else {
        // We already incremented `optget_ind' once; increment it again when taking next
        // argv element as argument.
        optget_arg = (char *)argv[optget_ind++];  // discard `const` qualifier
    }
    _next_char = NULL;

    return c;
}

// This should be the usual way to parse CLI args.
int optget_long(int argc, char *const *argv, const char *short_opts,
                const struct optget_option *long_opts) {
    return _optget_long(argc, argv, short_opts, long_opts, false);
}

// This is meant for those rare situations where a magic word is prefixed by `-` and thus looks like
// a short flag. For example, `kill -HUP`. Unless you absolutely have to use this do not use it.
// This function has some unexpected behaviors such as letting you concatenate short and long
// options in the same argument. If you simply need to support numbers that look like bundled short
// options (e.g., `-123`) simply preface the `short_opts` string with `#`.
int optget_long_only(int argc, char *const *argv, const char *short_opts,
                     const struct optget_option *long_opts) {
    return _optget_long(argc, argv, short_opts, long_opts, true);
}
