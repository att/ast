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
# Some basic syntax tests that were originally in the bracket.sh unit test.
test '(' = ')' && log_error '"test ( = )" should not be true'
test ! ! ! 2> /dev/null || log_error 'test ! ! ! should return 0'
test ! ! x 2> /dev/null || log_error 'test ! ! x should return 0'
test ! ! '' 2> /dev/null && log_error 'test ! ! "" should return non-zero'
test ! = -o a || log_error 'test ! \( = -o a \) should return 0'
test ! \( = -o a \) || log_error 'test ! \( = -o a \) should return 0'

# ==========
# POSIX specifies that on error, test builtin should always return value > 1
test 123 -eq 123x 2>/dev/null
[[ $? -ge 2 ]] || log_error 'test builtin should return value greater than 1 on error'

# ==========
# Running test without any arguments should return 1
test
[[ $? -eq 1 ]] || log_error "test builtin should return 1 if expression is missing"

# ==========
# This is not required by POSIX and this behavior seems incompatible with external `test` builtin
# but since `ksh` supports it, we should test it.
test 4/2 -eq 3-1 || log_error "Arithmetic expressions should work inside test builtin"

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

actual=$(test -f arglebargle 2>&1)
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
# test -r
# Verify file readability works.
test -r ""
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -r" "$expect" "$actual"

actual=$(test -r arglebargle 2>&1)
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -r" "$expect" "$actual"

test -r b_test.sh  # "b_test.sh" should be readable 
actual=$?
expect=0
[[ $actual == $expect ]] || log_error "test -r" "$expect" "$actual"

if [[ $OS_NAME == cygwin* ]]
then
    log_info "Skipping test-r test because the platform support for readability is broken"
else
    touch test-r
    chmod 333 test-r
    test -r test-r  # "test-r" should not be readable
    actual=$?
    expect=1
    [[ $actual == $expect ]] || log_error "test -r" "$expect" "$actual"
fi

# =======
# test -b
# Verify block device detection works.
actual=$(test -b arglebargle 2>&1)
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -b" "$expect" "$actual"

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
# test -c
# Verify char device detection works.
actual=$(test -c arglebargle 2>&1)
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -c" "$expect" "$actual"

# The `test -c` is expected to fail because we're not interactive. See b_test.exp for where we
# verify this works when the shell is interactive.
#
# On some platforms (e.g., FreeBSD) /dev/stdout is a char device even though it is a pipe.
# We have to skip this test on those platforms.
if [[ $(ls -Ll /dev/stdout) != c* ]]
then
    test -c /dev/stdout
    actual=$?
    expect=1
    [[ $actual == $expect ]] || log_error "test -c" "$expect" "$actual"
fi

test -c /dev/null
actual=$?
expect=0
[[ $actual == $expect ]] || log_error "test -c" "$expect" "$actual"

# =======
# test -S
# Verify unix domain socket detection works. This requires `nc` (netcat) to create the special file.
actual=$(test -S arglebargle 2>&1)
actual=$?
expect=1
[[ $actual == $expect ]] || log_error "test -S" "$expect" "$actual"

if [[ $nc_available == no ]]
then
    log_info 'Skipping the "test -S" test because nc is not available'
else
    nc -l -U socket &
    sleep 0.2  # need to give `nc` time to create the socket
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
actual=$(test foo =~ foo 2>&1)
actual_status=$?
actual=${actual#*: }
expect='test: =~ operator not supported; use [[...]]'
expect_status=2
[[ "$actual" = "$expect" ]] || log_error "test =~ failed" "$expect" "$actual"
[[ "$actual_status" = "$expect_status" ]] ||
    log_error "test =~ failed with wrong status" "$expect_status" "$actual_status"

test -d . -a '(' ! -f . ')' || log_error 'test not working'
if [[ '!' != ! ]]
then
    log_error 'quoting unary operator not working'
fi

test \( -n x \) -o \( -n y \) 2> /dev/null || log_error 'test ( -n x ) -o ( -n y) not working'
test \( -n x \) -o -n y 2> /dev/null || log_error 'test ( -n x ) -o -n y not working'
