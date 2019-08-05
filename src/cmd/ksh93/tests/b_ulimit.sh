# Tests for ulimit builtin

# ==========
# -c The number of 512-byte blocks on the size of core dumps.
ulimit -c 0
expect=0
actual=$(ulimit -c)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -c failed" "$expect" "$actual"

# ==========
# -d The number of K-bytes on the size of the data area.
# This test isn't compatible with ASAN.
if [[ $ASAN_OPTIONS == '' ]]
then
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
fi

# ==========
# -f The number of 512-byte blocks on files that can be written by the current process or by child
#    processes (files of any size may be read).
ulimit -f unlimited
expect=unlimited
actual=$(ulimit -f)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -f failed" "$expect" "$actual"

# ==========
# -m The number of K-bytes on the size of physical memory.
if [[ $OS_NAME != openbsd && $OS_NAME != sunos ]]
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
if [[ $OS_NAME != openbsd ]]
then
    ulimit -v unlimited
    expect=unlimited
    actual=$(ulimit -v)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -v failed" "$expect" "$actual"
fi

# ==========
# -M, --as  The address space limit in Kibytes.
if [[ $OS_NAME != openbsd ]]
then
    ulimit -M unlimited
    expect=unlimited
    actual=$(ulimit -M)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -M failed" "$expect" "$actual"
fi

# ==========
# -x, --locks  The number of file locks.
# Setting file lock limits is not supported on macOS
if [[ $OS_NAME != darwin && $OS_NAME != freebsd && $OS_NAME != openbsd && $OS_NAME != sunos ]]
then
    ulimit -x unlimited
    expect=unlimited
    actual=$(ulimit -x)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -x failed" "$expect" "$actual"
fi

# ==========
# -l, --memlock  The locked address space in Kibytes.
if [[ $OS_NAME != sunos ]]
then
    ulimit -l 0
    expect=0
    actual=$(ulimit -l)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -l failed" "$expect" "$actual"
fi

# ==========
# -q, --msgqueue  The message queue size in Kibytes.
# Setting message queue limits is not supported on macOS
if [[ $OS_NAME != darwin && $OS_NAME != freebsd && $OS_NAME != openbsd && $OS_NAME != sunos ]]
then
    ulimit -q 800
    expect=800
    actual=$(ulimit -q)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -q failed" "$expect" "$actual"
fi

# ==========
# -e, --nice  The scheduling priority.
# Setting scheduling priority is not supported on macOS and OpenBSD
if [[ $OS_NAME != darwin && $OS_NAME != freebsd && $OS_NAME != openbsd && $OS_NAME != sunos ]]
then
    ulimit -e 0
    expect=0
    actual=$(ulimit -e)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -e failed" "$expect" "$actual"
fi

# ==========
# -r, --rtprio  The max real time priority.
# Setting max real time priority is not supported on macOS and OpenBSD
if [[ $OS_NAME != darwin && $OS_NAME != freebsd && $OS_NAME != openbsd && $OS_NAME != sunos ]]
then
    ulimit -r 0
    expect=0
    actual=$(ulimit -r)
    [[ "$actual" = "$expect" ]] || log_error "setting ulimit -r failed" "$expect" "$actual"
fi

# ==========
# -H A hard limit is set or displayed.
# `ulimit -aH` should show all hard limits
actual="$(ulimit -aH)"
expect="address space limit (Kibytes)  (-M)  $(ulimit -HM)
core file size (blocks)        (-c)  $(ulimit -Hc)
cpu time (seconds)             (-t)  $(ulimit -Ht)
data size (Kibytes)            (-d)  $(ulimit -Hd)
file size (blocks)             (-f)  $(ulimit -Hf)
locks                          (-x)  $(ulimit -Hx)
locked address space (Kibytes) (-l)  $(ulimit -Hl)
message queue size (Kibytes)   (-q)  $(ulimit -Hq)
nice                           (-e)  $(ulimit -He)
nofile                         (-n)  $(ulimit -Hn)
nproc                          (-u)  $(ulimit -Hu)
pipe buffer size (bytes)       (-p)  $(ulimit -Hp)
max memory size (Kibytes)      (-m)  $(ulimit -Hm)
rtprio                         (-r)  $(ulimit -Hr)
socket buffer size (bytes)     (-b)  $(ulimit -Hb)
sigpend                        (-i)  $(ulimit -Hi)
stack size (Kibytes)           (-s)  $(ulimit -Hs)
swap size (Kibytes)            (-w)  $(ulimit -Hw)
threads                        (-T)  $(ulimit -HT)
process size (Kibytes)         (-v)  $(ulimit -Hv)"

[[ "$actual" = "$expect" ]] || log_error "ulimit -aH failed" "$expect" "$actual"

# ==========
# `ulimit -a` should show all soft limits
actual="$(ulimit -a)"
expect="address space limit (Kibytes)  (-M)  $(ulimit -M)
core file size (blocks)        (-c)  $(ulimit -c)
cpu time (seconds)             (-t)  $(ulimit -t)
data size (Kibytes)            (-d)  $(ulimit -d)
file size (blocks)             (-f)  $(ulimit -f)
locks                          (-x)  $(ulimit -x)
locked address space (Kibytes) (-l)  $(ulimit -l)
message queue size (Kibytes)   (-q)  $(ulimit -q)
nice                           (-e)  $(ulimit -e)
nofile                         (-n)  $(ulimit -n)
nproc                          (-u)  $(ulimit -u)
pipe buffer size (bytes)       (-p)  $(ulimit -p)
max memory size (Kibytes)      (-m)  $(ulimit -m)
rtprio                         (-r)  $(ulimit -r)
socket buffer size (bytes)     (-b)  $(ulimit -b)
sigpend                        (-i)  $(ulimit -i)
stack size (Kibytes)           (-s)  $(ulimit -s)
swap size (Kibytes)            (-w)  $(ulimit -w)
threads                        (-T)  $(ulimit -T)
process size (Kibytes)         (-v)  $(ulimit -v)"
[[ "$actual" = "$expect" ]] || log_error "ulimit -a failed" "$expect" "$actual"

# ==========
# -S A soft limit is set or displayed.
ulimit -S -n 512
expect=512
actual=$(ulimit -S -n)
[[ "$actual" = "$expect" ]] || log_error "setting ulimit -S -n failed" "$expect" "$actual"

# ==========
# Setting a very high limit should fail
actual=$(ulimit -n 102410241024 2>&1)
expect="limit exceeded"
[[ "$actual" =~ "$expect" ]] || log_error "ulimit -n with very high limit should fail" "$expect" "$actual"
