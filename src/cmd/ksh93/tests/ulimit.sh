# Tests for ulimit builtin

# ==========
# -c The number of 512-byte blocks on the size of core dumps.
ulimit -c 0
expected=0
actual=$(ulimit -c)
[[ "$actual" = "$expected" ]] || log_error "ulimit -c 0 does not set size of core dump to 0" "$expected" "$actual"

# ==========
# -d The number of K-bytes on the size of the data area.
ulimit -d 1024
expected=1024
actual=$(ulimit -d)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -d unlimited does not set size of data area to unlimited" "$expected" "$actual"

# ==========
# -f The number of 512-byte blocks on files that can be written by the current process or by child
#    processes (files of any size may be read).
ulimit -f unlimited
expected=unlimited
actual=$(ulimit -f)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -f does not set number of blocks that can be written by processes" "$expected" "$actual"

# ==========
# -m The number of K-bytes on the size of physical memory.
ulimit -m unlimited
expected=unlimited
actual=$(ulimit -m)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -m does not set number of K-bytes on the size of physical memory" "$expected" "$actual"

# ==========
# -n The number of file descriptors plus 1.
ulimit -n 1024
expected=1024
actual=$(ulimit -n)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -n does not set number of file descriptors" "$expected" "$actual"

# ==========
# -s The number of K-bytes on the size of the stack area.
ulimit -s 1024
expected=1024
actual=$(ulimit -s)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -s does not set size of stack area" "$expected" "$actual"

# ==========
# -t The number of CPU seconds to be used by each process.
ulimit -t unlimited
expected=unlimited
actual=$(ulimit -t)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -t does not set number of CPU seconds to be used by each process" "$expected" "$actual"

# ==========
# -v The number of K-bytes for virtual memory.
ulimit -v unlimited
expected=unlimited
actual=$(ulimit -v)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -v does not set number of K-bytes for virtual memory" "$expected" "$actual"

# ==========
# -M, --as        The address space limit in Kibytes.
ulimit -M unlimited
expected=unlimited
actual=$(ulimit -M)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -M does not set address space limits" "$expected" "$actual"

# ==========
# -x, --locks     The number of file locks.
# Setting file lock limits is not supported on macOS
if [[ "$OS_NAME" != "Darwin" && "$OS_NAME" != "FreeBSD" && "$OS_NAME" != "OpenBSD" ]]
then
    ulimit -x unlimited
    expected=unlimited
    actual=$(ulimit -x)
    [[ "$actual" = "$expected" ]] ||
        log_error "ulimit -x does not set number of file locks" "$expected" "$actual"
fi

# ==========
# -l, --memlock   The locked address space in Kibytes.
ulimit -l 0
expected=0
actual=$(ulimit -l)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -l does not set limit of locked address space" "$expected" "$actual"

# ==========
# -q, --msgqueue  The message queue size in Kibytes.
# Setting message queue limits is not supported on macOS
if [[ "$OS_NAME" != "Darwin" &&  "$OS_NAME" != "FreeBSD" ]]
then
    ulimit -q 800
    expected=800
    actual=$(ulimit -q)
    [[ "$actual" = "$expected" ]] ||
        log_error "ulimit -q does not set message queue size" "$expected" "$actual"
fi

# ==========
# -e, --nice      The scheduling priority.
# Setting scheduling priority is not supported on macOS
if [[ "$OS_NAME" != "Darwin" ]]
then
    ulimit -e 0
    expected=0
    actual=$(ulimit -e)
    [[ "$actual" = "$expected" ]] ||
        log_error "ulimit -e does not set scheduling priority" "$expected" "$actual"
fi

# ==========
# -r, --rtprio    The max real time priority.
# Setting max real time priority is not supported on macOS
if [[ "$OS_NAME"  != "Darwin" ]]
then
    ulimit -r 0
    expected=0
    actual=$(ulimit -r)
    [[ "$actual" = "$expected" ]] ||
        log_error "ulimit -r does not set max real time priority" "$expected" "$actual"
fi

# ==========
# -H A hard limit is set or displayed.
# TODO: How to test this ?

# ==========
# -S A soft limit is set or displayed.
ulimit -S -d 1024
expected=1024
actual=$(ulimit -S -d)
[[ "$actual" = "$expected" ]] ||
    log_error "ulimit -S -d unlimited does not set soft limit of data area to 1024" "$expected" "$actual"
