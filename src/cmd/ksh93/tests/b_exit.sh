#
# Verify the `exit` command behaves as expected.
#

# =======
actual=$($SHELL -c 'exit' 2>&1)
status=$?
expect=0
[[ -z $actual ]] || log_error 'bare exit' "" "$actual"
[[ $expect == $status ]] || log_error 'bare exit' "$expect" "$status"

# =======
actual=$($SHELL -c 'exit 0' 2>&1)
status=$?
expect=0
[[ -z $actual ]] || log_error 'exit 0' "" "$actual"
[[ $expect == $status ]] || log_error 'exit 0' "$expect" "$status"

# =======
actual=$($SHELL -c 'exit 1' 2>&1)
status=$?
expect=1
[[ -z $actual ]] || log_error 'exit 1' "" "$actual"
[[ $expect == $status ]] || log_error 'exit 1' "$expect" "$status"

# =======
actual=$($SHELL -c 'function e37 { return 37; } ; e37' 2>&1)
status=$?
expect=37
[[ -z $actual ]] || log_error 'exit 37' "" "$actual"
[[ $expect == $status ]] || log_error 'exit 37' "$expect" "$status"

# =======
actual=$($SHELL -c 'exit -1' 2>&1)
status=$?
expect=255
[[ -z $actual ]] || log_error 'exit -1' "" "$actual"
[[ $expect == $status ]] || log_error 'exit -1' "$expect" "$status"

# =======
actual=$($SHELL -c 'exit -2' 2>&1)
status=$?
expect=254
[[ -z $actual ]] || log_error 'exit -2' "" "$actual"
[[ $expect == $status ]] || log_error 'exit -2' "$expect" "$status"

# =======
$SHELL +E -i <<- \! && log_error 'interactive shell should not exit 0 after false'
    false
    exit
!
status=$?
expect=1
[[ $expect == $status ]] || log_error 'bare exit after false' "$expect" "$status"
