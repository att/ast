# Tests for `mkdir` builtin

# mkdir creates one or more directories. By default, the mode of created
# directories is a=rwx minus the bits set in the umask(1).

umask 000

# ==========
mkdir "$TEST_DIR/foo"
[[ -e "$TEST_DIR/foo" ]] || log_error "mkdir failed"

# ==========
#  -m, --mode=mode Set the mode of created directories to mode. mode is symbolic
#                  or octal mode as in chmod(1). Relative modes assume an
#                  initial mode of a=rwx.
mkdir -m 700 "$TEST_DIR/bar"
[[ -r "$TEST_DIR/foo" ]] || log_error "$TEST_DIR/bar should be readable"
[[ -w "$TEST_DIR/foo" ]] || log_error "$TEST_DIR/bar should be writable"
[[ -x "$TEST_DIR/foo" ]] || log_error "$TEST_DIR/bar should be executable"

# ==========
#  -p, --parents   Create any missing intermediate pathname components. For each
#                  dir operand that does not name an existing directory, effects
#                  equivalent to those caused by the following command shall
#                  occur: mkdir -p -m $(umask -S),u+wx $(dirname dir) && mkdir
#                  [-m mode] dir where the -m mode option represents that option
#                  supplied to the original invocation of mkdir, if any. Each
#                  dir operand that names an existing directory shall be ignored
#                  without error.
mkdir -p "$TEST_DIR/foo/bar/baz"
[[ -d "$TEST_DIR/foo/bar/baz" ]] || log_error "mkdir -p failed"

# ==========
#  -v, --verbose   Print a message on the standard error for each created
#                  directory.
actual=$(mkdir -v "$TEST_DIR/foo.bar" 2>&1)
expect="foo.bar: directory created"
[[ "$actual" =~ "$expect" ]] || log_error "mkdir -v failed" "$expect" "$actual"
