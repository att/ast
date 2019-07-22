#
# Verify that the special /dev paths such as /dev/stdin, /dev/fd/0, and /dev/tcp/host/port work when
# used in a test and actually opened. This is primarily meant to exercise the `sh_open()` functions
# handling of special paths.
#
readonly host=localhost
readonly port=65123

fd_check=no

# Some tests will want to verify whether or not a fd was allocated. We could use the `lsof` command
# but don't want to add that as a dependency. Instead just do the verification on platforms which
# support /proc/self/fd.
[[ -d /proc/self/fd ]] && fd_check=yes
[[ $fd_check == yes ]] && fd_count=$( ls -1 /proc/$$/fd | wc -l )

# Check if the system supports the /dev/{stdin,stdout,stderr} paths.
[[ -e /dev/stdin ]] && readonly dev_stdin=yes || readonly dev_stdin=no

# ==========
# Verify that testing variations of the special dev paths work as expected.
# Note that these should not result in a fd being opened.
[[ -e /dev/stdin ]] || log_error '-e /dev/stdin failed'
[[ -e /dev/stdout ]] || log_error '-e /dev/stdout failed'
[[ -e /dev/stderr ]] || log_error '-e /dev/stderr failed'
[[ -e /dev/stdinX ]] && log_error '-e /dev/stdinX succeeded but should have failed'
[[ -e /dev/stdoutX ]] && log_error '-e /dev/stdoutX succeeded but should have failed'
[[ -e /dev/stderrX ]] && log_error '-e /dev/stderrX succeeded but should have failed'

[[ -e /dev/fd/0 ]] || log_error '-e /dev/fd/0 failed'
# TODO: This test currently fails. It thinks the bogus path is valid.
# [[ -e /dev/fd/xxx ]] && log_error '-e /dev/fd/xxx succeeded but should have failed'

[[ -e /dev/tcp/localhost/666 ]] || log_error '-e /dev/tcp/localhost/666 failed'
[[ -e /dev/TCP/localhost/666 ]] && \
    log_error '-e /dev/TCP/localhost/666 succeeded but should have failed'

# TODO: This test currently fails. It thinks the bogus path is valid.
# [[ -e /dev/tcp/a_host/666/xxx ]] && \
#    log_error '-e /dev/TCP/a_host/666/xxx succeeded but should have failed'

# TODO: This test currently fails. It thinks the bogus path is valid.
# [[ -e /dev/tcp/a_host ]] && \
#    log_error '-e /dev/TCP/a_host succeeded but should have failed'

# TODO: This test currently fails. It thinks the bogus path is valid.
# [[ -e /dev/tcp/ ]] && log_error '-e /dev/tcp/ succeeded but should have failed'

[[ -e /dev/udp/localhost/666 ]] || log_error '-e /dev/udp/localhost/666 failed'
[[ -e /dev/UDP/localhost/666 ]] && \
    log_error '-e /dev/UDP/localhost/666 succeeded but should have failed'

# TODO: This test currently fails. It thinks the bogus path is valid.
# [[ -e /dev/udp/a_host/666/xxx ]] && \
#     log_error '-e /dev/udp/a_host/666/xxx succeeded but should have failed'

# TODO: This test currently fails. It thinks the bogus path is valid.
# [[ -e /dev/udp/a_host ]] && \
#     log_error '-e /dev/udp/a_host succeeded but should have failed'

# The tests we just ran should not have opened any fds.
if [[ $fd_check == yes ]]
then
    new_fd_count=$( ls -1 /proc/$$/fd | wc -l )
    [[ $new_fd_count -eq $fd_count ]] || \
        log_error 'fds unexpectedly opened' "$fd_count" "$new_fd_count"
fi

# ==========
# Verify that opening the special /dev/stdXXX paths works. This does not require the system to
# natively support these paths. The emulation should also result in a new fd being assigned.
exec {actual_fd}</dev/stdin
expect_fd=10
[[ $actual_fd -ge $expect_fd ]] || \
    log_error 'Opening /dev/stdin failed' "$expect_fd" "$actual_fd"

exec {actual_fd}>/dev/stdout
expect_fd=10
[[ $actual_fd -ge $expect_fd ]] || \
    log_error 'Opening /dev/stdout failed' "$expect_fd" "$actual_fd"

exec {actual_fd}>/dev/stderr
expect_fd=10
[[ $actual_fd -ge $expect_fd ]] || \
    log_error 'Opening /dev/stderr failed' "$expect_fd" "$actual_fd"

# The tests we just ran should have opened three fds.
if [[ $fd_check == yes ]]
then
    new_fd_count=$( ls -1 /proc/$$/fd | wc -l )
    (( fd_count += 3 ))
    [[ new_fd_count -eq fd_count ]] || log_error 'fd count wrong' "$fd_count" "$new_fd_count"
fi

# ==========
# Verify that opening the special /dev/tcp/host/service path works.
# This requires the netcat program.
if [[ $nc_available == no ]]
then
    log_info 'Skipping the /dev/tcp/... I/O tests because nc is not available'
else
    nc -l $host $port >nc.out &
    sleep 0.1  # give nc a chance to bind to the port an listen for connections
    exec {fd}>/dev/tcp/$host/$port || log_error "Could not open /dev/tcp/$host/$port"
    if [[ $fd_check == yes ]]
    then
        new_fd_count=$( ls -1 /proc/$$/fd | wc -l )
        (( new_fd_count == fd_count + 1 )) || \
            log_error 'One fd should have been opened' "$fd_count + 1" "$new_fd_count"
    fi

    expect='hello tcp'
    echo $expect >&$fd || log_error "Could not write to /dev/tcp/$host/$port"
    exec {fd}>&-  # close the connection so that `nc` will exit
    wait
    actual=$(< nc.out)
    [[ $actual == $expect ]] || \
        log_error "Did not capture the expected text written to /dev/tcp/$host/$port" \
            "$expect" "$actual"

    # The tests we just ran should not have have left any open fds.
    if [[ $fd_check == yes ]]
    then
        new_fd_count=$( ls -1 /proc/$$/fd | wc -l )
        [[ $new_fd_count -eq $fd_count ]] || \
            log_error 'fds unexpectedly opened' "$fd_count" "$new_fd_count"
    fi
fi

# ==========
# Verify that opening the special /dev/udp/host/service path works.
# This requires the netcat program.
if [[ $nc_available == no ]]
then
    log_info 'Skipping the /dev/udp/... I/O tests because nc is not available'
else
    nc -u -l $host $port >nc.out &
    sleep 0.1  # give nc a chance to bind to the port an listen for connections
    exec {fd}>/dev/udp/$host/$port || log_error "Could not open /dev/udp/$host/$port"
    if [[ $fd_check == yes ]]
    then
        new_fd_count=$( ls -1 /proc/$$/fd | wc -l )
        (( new_fd_count == fd_count + 1 )) || \
            log_error 'One fd should have been opened' "$fd_count + 1" "$new_fd_count"
    fi

    expect='hello udp'
    echo $expect >&$fd || log_error "Could not write to /dev/udp/$host/$port"
    # Note that while we want to close `fd` so that we don't have a fd leak it doesn't actually
    # cause the `nc` process to exit. That's because UDP is not connection oriented like TCP.
    # Instead we have to kill the process after giving it a chance to write the message we sent.
    exec {fd}>&-  # close the connection
    sleep 0.1
    kill %1  # kill the background `nc`
    wait
    actual=$(< nc.out)
    [[ $actual == $expect ]] || \
        log_error "Did not capture the expected text written to /dev/udp/$host/$port" \
            "$expect" "$actual"

    # The tests we just ran should not have have left any open fds.
    if [[ $fd_check == yes ]]
    then
        new_fd_count=$( ls -1 /proc/$$/fd | wc -l )
        [[ $new_fd_count -eq $fd_count ]] || \
            log_error 'fds unexpectedly opened' "$fd_count" "$new_fd_count"
    fi
fi
