# Regression tests
# https://github.com/att/ast/issues/851
out=$(getopts -a test $'
[-]
[foo?option 1]
[bar?option 2]

Use option 1
Use option 2

' baz '--help' 2>&1)

[[ "$out" = $'Usage: test [ options ] Use option 1\n   Or: test [ options ] Use option 2' ]] ||
    log_error "getopts shows wrong text for self documenting options"
