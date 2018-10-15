# Tests for ulimit builtin

# ==========
# -c The number of 512-byte blocks on the size of core dumps.
ulimit -c 0
expect=0
actual=$(ulimit -c)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -c failed" "$expect" "$actual"

# ==========
# -d The number of K-bytes on the size of the data area.
limit=$(ulimit -d)
if [[ $limit == unlimited ]]
then
    limit=$((512 * 1024))
else
    limit=$((limit - 1))
fi
ulimit -d $limit
expect=$limit
actual=$(ulimit -d)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -d failed" "$expect" "$actual"

# ==========
# -f The number of 512-byte blocks on files that can be written by the current process or by child
#    processes (files of any size may be read).
ulimit -f unlimited
expect=unlimited
actual=$(ulimit -f)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -f failed" "$expect" "$actual"

# ==========
# -m The number of K-bytes on the size of physical memory.
if [[ $OS_NAME != OpenBSD ]]
then
    ulimit -m unlimited
    expect=unlimited
    actual=$(ulimit -m)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -m failed" "$expect" "$actual"
fi

# ==========
# -n The number of file descriptors.
ulimit -n 1024
expect=1024
actual=$(ulimit -n)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -n failed" "$expect" "$actual"

# ==========
# -s The number of K-bytes on the size of the stack area.
ulimit -s 1024
expect=1024
actual=$(ulimit -s)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -s failed" "$expect" "$actual"

# ==========
# -t The number of CPU seconds to be used by each process.
ulimit -t unlimited
expect=unlimited
actual=$(ulimit -t)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -t failed" "$expect" "$actual"

# ==========
# -v The number of K-bytes for virtual memory.
if [[ $OS_NAME != OpenBSD ]]
then
    ulimit -v unlimited
    expect=unlimited
    actual=$(ulimit -v)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -v failed" "$expect" "$actual"
fi

# ==========
# -M, --as  The address space limit in Kibytes.
if [[ $OS_NAME != OpenBSD ]]
then
    ulimit -M unlimited
    expect=unlimited
    actual=$(ulimit -M)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -M failed" "$expect" "$actual"
fi

# ==========
# -x, --locks  The number of file locks.
# Setting file lock limits is not supported on macOS
if [[ $OS_NAME != Darwin && $OS_NAME != FreeBSD && $OS_NAME != OpenBSD ]]
then
    ulimit -x unlimited
    expect=unlimited
    actual=$(ulimit -x)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -x failed" "$expect" "$actual"
fi

# ==========
# -l, --memlock  The locked address space in Kibytes.
ulimit -l 0
expect=0
actual=$(ulimit -l)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -l failed" "$expect" "$actual"

# ==========
# -q, --msgqueue  The message queue size in Kibytes.
# Setting message queue limits is not supported on macOS
if [[ $OS_NAME != Darwin && $OS_NAME != FreeBSD && $OS_NAME != OpenBSD ]]
then
    ulimit -q 800
    expect=800
    actual=$(ulimit -q)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -q failed" "$expect" "$actual"
fi

# ==========
# -e, --nice  The scheduling priority.
# Setting scheduling priority is not supported on macOS and OpenBSD
if [[ $OS_NAME != Darwin && $OS_NAME != FreeBSD && $OS_NAME != OpenBSD ]]
then
    ulimit -e 0
    expect=0
    actual=$(ulimit -e)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -e failed" "$expect" "$actual"
fi

# ==========
# -r, --rtprio  The max real time priority.
# Setting max real time priority is not supported on macOS and OpenBSD
if [[ $OS_NAME != Darwin && $OS_NAME != FreeBSD && $OS_NAME != OpenBSD ]]
then
    ulimit -r 0
    expect=0
    actual=$(ulimit -r)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -r failed" "$expect" "$actual"
fi

# ==========
# -H A hard limit is set or displayed.
# TODO: How to test this ?

# ==========
# -S A soft limit is set or displayed.
ulimit -S -d 1024
expect=1024
actual=$(ulimit -S -d)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -S -d failed" "$expect" "$actual"
