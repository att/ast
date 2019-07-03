# Tests for sleep builtin
#
# sleep suspends execution for at least the time specified by duration or until
# a SIGALRM signal is received. duration may be one of the following:
#
# Note: The 0.1 second slop is to workaround quirks of platforms like Cygwin where the sleep is
# often slightly less than the requested duration. Note, however, that even on more mainstream UNIX
# implementations it is theoretically possible for the sleep to be slightly less than a full second.
float slop=0.1

# ======
# integer
# The number of seconds to sleep.
SECONDS=0
sleep 1
actual=$SECONDS
expect=1
(( actual + slop >= expect )) ||
    log_error "sleep 1 should sleep for at least 1 second" "$expect" "$actual"

# ======
# floating point
# The number of seconds to sleep. The actual granularity depends on the
# underlying system, normally around 1 millisecond.
SECONDS=0
sleep 0.1
actual=$SECONDS
expect=0.1
(( actual + slop / 10 >= expect )) ||
    log_error "sleep 0.1 should sleep for at least 0.1 second" "$expect" "$actual"

# ======
# PnYnMnDTnHnMnS
# An ISO 8601 duration where at least one of the duration parts must be
# specified.
SECONDS=0
sleep "P0Y0M0DT0H0M1S"
actual=$SECONDS
expect=1.0
(( actual + slop >= expect )) ||
    log_error "sleep 1 should sleep for at least 1 second" "$expect" "$actual"

# ======
# PnW   An ISO 8601 duration specifying n weeks.
# Sleep for 0 weeks
sleep P0W || log_error "sleep does not recocgnize PnW"

# ======
# pnYnMnDTnHnmnS
# A case insensitive ISO 8601 duration except that M specifies months,
# m before s or S specifies minutes and after specifies milliseconds, u
# or U specifies microseconds, and n specifies nanoseconds.
SECONDS=0
sleep "p0Y0M0DT0H0M1S"
actual=$SECONDS
expect=1.0
(( actual + slop >= expect )) ||
    log_error "sleep 1 should sleep for at least 1 second" "$expect" "$actual"

# ======
# date/time
#       Sleep until the date(1) compatible date/time.
today=$(date +"%Y-%m-%d")
# This should return immediately
sleep "$today" || log_error "sleep does not recognize date parameter"

# ======
# -s Sleep until a signal or a timeout is received. If duration is
#    omitted or 0 then no timeout will be used.
SECONDS=0
sleep -s 1
actual=$SECONDS
expect=1.0
(( actual + slop >= expect )) ||
    log_error "sleep 1 should sleep for at least 1 second" "$expect" "$actual"

# ======
$SHELL -c 'sleep $(printf "%a" .95)' ||
    log_error "sleep doesn't except %a format constants"

# ======
# Verify unexpected arguments result in an error.
expect="sleep: one operand expected"
actual=$(sleep 0 .3 2>&1)
[[ $actual == $expect ]] || log_error "unexpected args isn't an error" "$expect" "$actual"

# ======
# Verify an invalid interval results in an error.
expect="sleep: 1sx: bad number"
actual=$(sleep 1sx 2>&1)
[[ $actual == $expect ]] || log_error "invalid interval isn't an error" "$expect" "$actual"

# ======
# Verify complex interval is parsed without error. We can't easily verify the interval is correctly
# parsed so we just hope not to see an error message and the actual sleep is within a reasonable
# window.
SECONDS=0
$SHELL -c 'sleep 0d0h0m0.01s' >fifo8 2>&1 &
wait
(( SECONDS <= 0.3 )) || log_error "complex interval isn't handled correctly" "<= 0.3" "$SECONDS"
actual=''
expect=''
read -u8 -t0.01 actual
[[ $actual == $expect ]] || log_error "complex interval isn't parsed" "$expect" "$actual"
