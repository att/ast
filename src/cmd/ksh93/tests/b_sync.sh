# Tests for `sync` builtin

# ==========
# sync -S1
# Will succeed silently if the system has syncfs() otherwise it fails in a predictable manner.
actual=$(sync -S1 2>&1)
e1="sync: syncfs(1) failed [Function not implemented]"
e2="sync: syncfs(1) failed [Operation not applicable]"
[[ "$actual" == "" || "$actual" == "$e1" || "$actual" == "$e2" ]] ||
    log_error "sync -S1" "$e1" "$actual"

# ==========
# sync -s1
actual=$(sync -s1 2>&1)
expect=""
[[ "$actual" == "$expect" ]] || log_error "sync -s1" "$expect" "$actual"

# ==========
# sync -s 999
# An invalid file descriptor should fail. We don't verify the errno portion of the message because
# it can vary across systems -- even though it should be EBADF which should be the same numeric
# value on every system that cares at all about being UNIX compatible.
#
# We don't use fd 3 because it might be opened by a debug malloc subsystem. We expect fd 999 to be
# unused if not invalid.
actual=$(sync -s 999 2>&1)
expect="sync: fsync(999) failed"
[[ "$actual" =~ "$expect".* ]] || log_error "sync -s 999" "$expect" "$actual"

# ==========
# sync -f
actual=$(sync -f 2>&1)
expect=""
[[ "$actual" == "$expect" ]] || log_error "sync -f" "$expect" "$actual"

# ==========
# sync -X
actual=$(sync -X 2>&1)
expect=""
[[ "$actual" == "$expect" ]] || log_error "sync -X" "$expect" "$actual"

# ==========
# sync -x
actual=$(sync -x 2>&1)
expect="sync: -x: unknown option"
[[ "$actual" =~ "$expect".* ]] || log_error "sync -x" "$expect" "$actual"
