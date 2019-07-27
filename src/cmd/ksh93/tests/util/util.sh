#
# These mimic the functions of the same name in preamble.sh. This module is intended to be prefixed
# to subshell scripts run by unit tests.
#
readonly test_name=${0##*/}

#
# Make sure the unit test can't inadvertantly modify several critical env vars.
#
readonly FULL_PATH
readonly OLD_PATH
readonly SAFE_PATH
readonly TEST_DIR
readonly TEST_ROOT
readonly BUILD_DIR

#
# Create some functions for reporting and counting errors.
#
integer error_count=0
integer start_of_test_lineno=0  # redefined later to be read-only

function log_info {
    typeset -l lineno=$(( $1 < 0 ? $1 : $1 - $start_of_test_lineno ))
    print -r "<I> ${test_name}[$lineno]: ${@:2}"
}
alias log_info='log_info $LINENO'

function log_warning {
    typeset -l lineno=$(( $1 < 0 ? $1 : $1 - $start_of_test_lineno ))
    print -u2 -r "<W> ${test_name}[$lineno]: ${@:2}"
}
alias log_warning='log_warning $LINENO'

function log_error {
    typeset -l lineno=$(( $1 < 0 ? $1 : $1 - $start_of_test_lineno ))
    typeset -l msg="$2"
    print -u2 -r "<E> ${test_name}[$lineno]: $msg"
    if (( $# > 2 ))
    then
        print -u2 -r "<E> expect: |$3|"
        print -u2 -r "<E> actual: |$4|"
    fi
    (( error_count++ ))
}
alias log_error='log_error $LINENO'

function exit_error_count {
    if (( $error_count == 0 ))
    then
        exit 0
    else
        'log_warning' -1 "error_count = $error_count"
        exit $(( error_count < 125 ? error_count : 125 ))
    fi
}

#
# Open a couple of named pipes (fifos) for the unit test to use as barriers rather than using
# arbitrary sleeps.
#
exec 9<>fifo9
exec 8<>fifo8
function empty_fifos {
    read -u9 -t0.1 x && {
        'log_warning' $1 "fifo9 unexpectedly had data: '$x'"
    }
    read -u8 -t0.1 x && {
        'log_warning' $1 "fifo9 unexpectedly had data: '$x'"
    }
}
alias empty_fifos='empty_fifos $LINENO'

typeset -ri start_of_test_lineno=$LINENO
