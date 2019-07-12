#!/bin/sh
# This is loosely based on the src/cmd/ksh93/tests/util/run_test.sh script.
#
# This helps ensure each API test runs in a unique temp dir that is automatically cleaned up when
# the test successfully terminates.

# API unit tests might call popen() which will use the value of SHELL. So this is hermetic we don't
# want to risk using the user's SHELL. So ensure any subshells use the same shell uses the same one
# running this script.
SHELL=/bin/sh

#
# Make sure when called via `meson test` we've got the expected args.
#
log_info() {
    echo "<I> run_test[$1]: $2"
}
alias log_info='log_info $LINENO'

log_warning() {
    echo "<W> run_test[$1]: $2" >&2
}
alias log_warning='log_warning $LINENO'

log_error() {
    echo "<E> run_test[$1]: $2" >&2
}

alias log_error='log_error $LINENO'

if [ $# -ne 2 ]
then
    log_error "Expected two args (the API test binary pathname and test name), got $#: $*"
    exit 99
fi

export BUILD_DIR=$PWD

x=${1##*/}
readonly test_name=${x%.c}
readonly api_binary=$BUILD_DIR/$1
readonly test_src_dir=$2

#
# A test may need to alter its behavior based on the OS we're running on.
#
# shellcheck disable=SC2155
export OS_NAME=$(uname -s)

#
# Setup the environment for the unit test.
#

#
# Create a temp dir and make it the CWD for the unit test. It will be removed by the unit test
# postscript if there are no test failures.
#
# NOTE: We deliberately includle a space char in the pathname to help ensure we don't introduce
# tests that break when that is true. See issue #166 and #158.
#
# The use of `mktemp -dt` isn't ideal as it has slightly different meaning on BSD and GNU. But for
# our purposes that doesn't matter. It simply means the temp file name will contain the X's on a BSD
# system.
#
# shellcheck disable=SC2155
export TEST_DIR=$(mktemp -dt "api.${test_name}.XXXXXX") ||
    { log_error "mktemp -dt failed"; exit 99; }
cd "$TEST_DIR" || { echo "<E> 'cd $TEST_DIR' failed with status $?" >&2; exit 99; }
log_info "TEST_DIR=$TEST_DIR"

#
# Make sure we search for external commands in the temporary test dir, then the test source dir,
# then the shell auxiliary commands, before any other directory in $PATH.
#
# This helps ensure that if the test creates a script file named "zzz" and there is an executable
# external command of the same name in PATH that we use the command created by the unit test.
# See issue #429.
#
export OLD_PATH="$PATH"
export SAFE_PATH="$TEST_DIR:$TEST_DIR/space dir:$test_src_dir:$BUILD_DIR/src/cmd/builtin"
export FULL_PATH="$SAFE_PATH:$OLD_PATH"
export PATH="$FULL_PATH"

#
# We don't want the tests to modify the command history and the like of the user running the tests
# or be affected by the user's home dir.
#
mkdir "$TEST_DIR/home"
mkdir "$TEST_DIR/space dir"
export HOME=$TEST_DIR/home

# Run a libast API test.
run_api() {
    $api_binary >"$test_name.out" 2>"$test_name.err"
    exit_status=$?

    if [ -e "$test_src_dir/$test_name.out" ]
    then
        if ! diff -q "$test_src_dir/$test_name.out" "$test_name.out >/dev/null"
        then
            log_error "Stdout for $test_name had unexpected differences:"
            diff -U3 "$test_src_dir/$test_name.out" "$test_name.out >&2"
            exit_status=1
        fi
    elif [ -s "$test_name.out" ]
    then
            log_error "Stdout for $test_name should have been empty:"
            cat "$test_name.out" >&2
            exit_status=1
    fi

    if [ -e "$test_src_dir/$test_name.err" ]
    then
        if ! diff -q "$test_src_dir/$test_name.err" "$test_name.err" >/dev/null
        then
            log_error "Stderr for $test_name had unexpected differences:"
            diff -U3 "$test_src_dir/$test_name.err" "$test_name.err" >&2
            exit_status=1
        fi
    elif [ -s "$test_name.err" ]
    then
            log_error "Stderr for $test_name should have been empty:"
            cat "$test_name.err" >&2
            exit_status=1
    fi

    if [ $exit_status -eq 0 ]
    then
        # We only remove the temp test dir if the test is successful. Otherwise we leave it since
        # it may contain useful clues about why the test failed.
        # shellcheck disable=SC2164
        cd /tmp
        rm -rf "$TEST_DIR"
    fi

    return $exit_status
}

run_api
status=$?
exit $status
