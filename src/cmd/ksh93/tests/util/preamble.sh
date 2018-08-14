#
# Setup the environment for the unit test.
#
readonly test_file=${0##*/}
readonly test_name=$1

#
# Make sure the unit test can't inadvertantly modify several critical env vars.
#
readonly FULL_PATH
readonly ORIG_PATH
readonly SAFE_PATH
readonly TEST_DIR
readonly TEST_SRC_DIR
readonly BUILD_DIR
readonly OS_NAME

#
# Create some functions for reporting and counting errors.
#
integer error_count=0
integer start_of_test_lineno=0  # redefined later to be read-only

function log_info {
    typeset lineno=$(( $1 < 0 ? $1 : $1 - $start_of_test_lineno ))
    print -r "<I> ${test_name}[$lineno]: ${@:2}"
}
alias log_info='log_info $LINENO'

function log_warning {
    typeset lineno=$(( $1 < 0 ? $1 : $1 - $start_of_test_lineno ))
    print -u2 -r "<W> ${test_name}[$lineno]: ${@:2}"
}
alias log_warning='log_warning $LINENO'

function log_error {
    typeset lineno=$(( $1 < 0 ? $1 : $1 - $start_of_test_lineno ))
    typeset msg="$2"
    print -u2 -r "<E> ${test_name}[$lineno]: $msg"
    if (( $# > 2 ))
    then
        print -u2 -r "<E> expect: |$3|"
        print -u2 -r "<E> actual: |$4|"
    fi
    (( error_count++ ))
}
alias log_error='log_error $LINENO'

#
# Open a couple of named pipes (fifos) for the unit test to use as barriers rather than using
# arbitrary sleeps. The fifos are created by the run_test.sh script which sets up our environment.
#
exec 9<>fifo9
exec 8<>fifo8
function empty_fifos {
    read -u9 -t0.01 x && {
        'log_warning' $1 "fifo9 unexpectedly had data: '$x'"
    }
    read -u8 -t0.01 x && {
        'log_warning' $1 "fifo9 unexpectedly had data: '$x'"
    }
}
alias empty_fifos='empty_fifos $LINENO'

#
# Capture the current line number so we can calculate the correct line number in the unit test file
# that will be appended to this preamble script. This must be the last line in this preamble script.
#
# Note: It is tempting to just do `LINENO=1` and avoid introducing a global var. Doing so, however,
# breaks at least two unit tests. Whether that breakage indicates bugs in ksh or the unit test is
# unknown.
#
typeset -ri start_of_test_lineno=$LINENO
