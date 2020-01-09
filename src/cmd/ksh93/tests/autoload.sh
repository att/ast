# Verify the behavior of autoloaded functions.

# ====================
# Verify that directories in the path search list which should be skipped (e.g., because they don't
# exist) interacts correctly with autoloaded functions.
#
# See https://github.com/att/ast/issues/1454
expect=$"Func cd called with |$TEST_DIR/usr|\n$TEST_DIR/usr"
actual=$($SHELL "$TEST_ROOT/data/skipped_dir")
actual_status=$?
expect_status=0
[[ $actual_status == $expect_status ]] ||
    log_error "autoload function skipped dir test wrong status" "$expect_status" "$actual_status"
[[ $actual == $expect ]] ||
    log_error "autoload function skipped dir test wrong output" "$expect" "$actual"
