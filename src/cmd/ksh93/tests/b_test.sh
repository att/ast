# TODO: Move other basic tests of the `test` and `[...]` commands from other unit test modules into
# this unit test.

# =======
# Test that `[...]` with the closing `]` not present is an error.
actual=$( [ x = y 2>&1 )
status=$?
expect=": [: ']' missing"
[[ $actual =~ .*"$expect" ]] || log_error "missing close ] wrong error" "$expect" "$actual"
[[ $status == 2 ]] || log_error "missing close ] wrong status" "2" "$status"

# =======
# An invalid operator is treated as an error.
actual=$(test -@ arglebargle 2>&1)
expect=": test: -@: unknown operator"
[[ $actual =~ .*"$expect" ]] || log_error "unknown operator" "$expect" "$actual"

# =======
# Missing arg is an error.
actual=$(test x = 2>&1)
expect=": test: argument expected"
[[ $actual =~ .*"$expect" ]] || log_error "missing arg" "$expect" "$actual"

# =======
# test -f
# Verify file detection works.
test -f ""
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -f" "$expect" "$actual"

test -f b_test.sh  # "b_test.sh" should be a regular file
actual=$?
expect=0
[[ $actual == $expect ]] || log_error "test -f" "$expect" "$actual"

test -f home  # "home" should be a directory
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -f" "$expect" "$actual"

# =======
# test -b
# Verify block device detection works.
test -b /dev/stdout
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -b" "$expect" "$actual"

if test -e /dev/disk0
then
    test -b /dev/disk0
    actual=$?
    expect=0
    [[ $actual == $expect ]] || log_error "test -b" "$expect" "$actual"
elif test -e /dev/sda
then
    test -b /dev/sda
    actual=$?
    expect=0
    [[ $actual == $expect ]] || log_error "test -b" "$expect" "$actual"
fi

# =======
# test -S
# Verify unix domain socket detection works. This requires `nc` (netcat) to create the special file.
if whence -p nc >/dev/null
then
    nc -U socket -l &
    sleep 0.1  # need to give `nc` time to create the socket
    test -S /dev/stdout
    actual=$?
    expect=1
    [[ $actual == $expect ]] || log_error "test -S" "$expect" "$actual"

    test -S socket
    actual=$?
    expect=0
    [[ $actual == $expect ]] || log_error "test -S" "$expect" "$actual"

    kill %1
    wait
fi

# ==========
# Verify that the POSIX `test` builtin complains loudly when the `=~` operator is used rather than
# failing silently. See //github.com/att/ast/issues/1152.
actual=$($SHELL -c 'test foo =~ foo' 2>&1)
actual_status=$?
actual=${actual#*: }
expect='test: =~ operator not supported; use [[...]]'
expect_status=2
[[ "$actual" = "$expect" ]] || log_error "test =~ failed" "$expect" "$actual"
[[ "$actual_status" = "$expect_status" ]] ||
    log_error "test =~ failed with wrong status" "$expect_status" "$actual_status"
