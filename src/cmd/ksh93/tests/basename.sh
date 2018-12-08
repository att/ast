# Tests for `basename` builtin

# ==========
actual=$(basename "$TEST_DIR/foo.bar")
expect="foo.bar"
[[ "$actual" = "$expect" ]] || log_error "basename failed to print filename" "$expect" "$actual"

# ==========
# Last argument is treated as suffix and removed from output
actual=$(basename "$TEST_DIR/foo.bar" .bar)
expect="foo"
[[ "$actual" = "$expect" ]] || log_error "basename failed to strip suffix" "$expect" "$actual"

# ==========
# -a, --all All operands are treated as string and each modified pathname
#           is printed on a separate line on the standard output.
actual=$(basename -a "$TEST_DIR/foo.bar" "$TEST_DIR/bar.bar" "$TEST_DIR/baz.bar")
expect=$'foo.bar\nbar.bar\nbaz.bar'
[[ "$actual" = "$expect" ]] || log_error "basename -a failed" "$expect" "$actual"

# ==========
# -s, --suffix=suffix
#  All operands are treated as string and each modified
#  pathname, with suffix removed if it exists, is printed on a
#  separate line on the standard output.
actual=$(basename -s bar "$TEST_DIR/foo.bar" "$TEST_DIR/bar.bar" "$TEST_DIR/baz.bar")
expect=$'foo.\nbar.\nbaz.'
[[ "$actual" = "$expect" ]] || log_error "basename -s failed" "$expect" "$actual"

# ==========
actual=$(basename "$TEST_DIR/.bar")
expect=".bar"
[[ "$actual" = "$expect" ]] || log_error "basename failed" "$expect" "$actual"
