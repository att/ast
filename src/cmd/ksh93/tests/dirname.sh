# Tests for `dirname` builtin

# dirname treats string as a file name and returns the name of the directory
# containing the file name by deleting the last component from string.
mkdir "$TEST_DIR/foo"

actual=$(dirname "$TEST_DIR/foo")
expect="$TEST_DIR"
[[ "$actual" = "$expect" ]] || log_error "dirname failed" "$expect" "$actual"

# ==========
#  -f, --file      Print the $PATH relative regular file path for string.
actual=$(dirname -f cat)
expect=$(which cat)
expect=$(which cat) || log_error "dirname -f failed" "$expect" "$actual"

# ==========
#  -r, --relative  Print the $PATH relative readable file path for string.
actual=$(dirname -r cat)
expect=$(which cat)
expect=$(which cat) || log_error "dirname -r failed" "$expect" "$actual"

# ==========
#  -x, --executable
#  Print the $PATH relative executable file path for string.
actual=$(dirname -x cat)
expect=$(which cat) || log_error "dirname -x failed" "$expect" "$actual"
