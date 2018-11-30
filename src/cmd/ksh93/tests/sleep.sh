# Tests for sleep builtin

# sleep suspends execution for at least the time specified by duration or until
# a SIGALRM signal is received. duration may be one of the following:

# ======
# integer
# The number of seconds to sleep.
SECONDS=0
sleep 1
[[ $SECONDS -ge 1 ]] || log_error "sleep 1 should sleep for at least 1 second"

# ======
# floating point
# The number of seconds to sleep. The actual granularity depends on the
# underlying system, normally around 1 millisecond.
SECONDS=0
sleep 0.1
[[ $SECONDS -ge 0.1 ]] || log_error "sleep 0.1 should sleep for at least 0.1 second"

# ======
# PnYnMnDTnHnMnS
# An ISO 8601 duration where at least one of the duration parts must be
# specified.
SECONDS=0
sleep "P0Y0M0DT0H0M1S"
[[ $SECONDS -ge 1 ]] || log_error "sleep P0Y0M0DT0H0M1S should sleep for at least 1 second"

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
[[ $SECONDS -ge 1 ]] || log_error "sleep p0Y0M0DT0H0M1S should sleep for at least 1 second"

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
[[ $SECONDS -ge 1 ]] || log_error "sleep -s 1 should sleep for at least 1 second"

# ======
$SHELL -c 'sleep $(printf "%a" .95)' 2> /dev/null || log_error "sleep doesn't except %a format constants"
