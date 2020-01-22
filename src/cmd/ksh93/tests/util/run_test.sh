#!/usr/bin/env ksh
#
# This is invoked by Meson to run each ksh unit test; including the `expect` based interactive
# tests. This script is responsible for setting up the test environment and managing the execution
# of each test. Including skipping tests that aren't compatible with the platform. It is expected to
# be run by the ksh binary built by the build process but it should work with any POSIX shell.

# Some tests fail for inexplicable reasons on some platforms. In some cases, such as Cygwin, things
# simply don't work as expected and probably never will due to quirks of the platform. Another
# example is SunOs/Solaris which has /proc but doesn't support manipulating directory fd's.
#
# The table is a sequence of system/test_name pairs. The system name is the exported value of the
# meson.build `system` var. Which is the output of Meson's `host_machine.system()` or possibly a
# variant; e.g., to distinguish WSL (Windows Subsystem for Linux) from other Linux platforms. It's a
# shame we can't leverage nested data structures and still use a borg standard POSIX shell to run
# this script.
#
# TODO: Eliminate as many of these exceptions as possible. Some of them, especially the Cygwin
# exceptions, will probably never be removed because they involve obscure features that will never
# be made to work on a particular platform. But others should be fixed so the tests do pass and the
# exclusion entry in the table below removed.
tests_to_skip=(
    'cygwin b_jobs.exp'
    'cygwin b_time.exp'
    'cygwin b_times.exp'
    'cygwin b_ulimit.sh'
    'cygwin b_uname.sh'  # too many trivial diffs in uname output
    'cygwin coprocess.sh'
    'cygwin signal.sh'
    'cygwin sh_match.sh'  # too slow, times out
    'openbsd b_times.exp'
    'sunos vi.exp'
    'sunos directoryfd.sh'
    'wsl b_time.exp'
    'wsl b_times.exp'
    'wsl sh_match.sh'  # too slow, times out
    # Tests to be skipped because they are known to be broken when compiled by `shcomp`.
    # TODO: Fix these tests or the shcomp code.
    'shcomp io.sh'
    'shcomp b_set.sh'
    'shcomp treemove.sh'
)

# I'm not a fan of the errexit mechanism; not least because it means you can't do
#
#   some_command
#   status=$?
#
# However, https://github.com/att/ast/issues/1453 showed that a bogus $PATH can cause this to fail
# while exiting with a zero, success, status if commands like `cat` cannot be found.
set -o errexit

# The use of the "dumb" terminal type is to minimize, and hopefully eliminate completely,
# terminal control/escape sequences that affect the terminal's behavior. This makes writing
# robust unit tests, especially Expect based tests, easier.
export TERM=xterm
export BUILD_DIR=$PWD

# Special exit status for Meson that it recognizes as indicating the test wasn't run. This is
# preferable to returning zero and pretending the test succeeded.
export MESON_SKIPPED_TEST=77
# This isn't special to meson but we use it below if we've detected an `expect` based test has
# timed out.
export MESON_TEST_TIMEOUT=99

function log_info {
    typeset lineno=$1
    print -r "<I> run_test[$lineno]: ${*:2}"
}
alias log_info='log_info $LINENO'

function log_warning {
    typeset lineno=$1
    print -u2 -r "<W> run_test[$lineno]: ${*:2}"
}
alias log_warning='log_warning $LINENO'

function log_error {
    typeset lineno=$1
    print -u2 -r "<E> run_test[$lineno]: ${*:2}"
}
alias log_error='log_error $LINENO'

api_test=false
api_binary=false
shcomp=false
if [[ $1 == shcomp ]]
then
    # Run a ksh script test after compiling it.
    shcomp=true
    shift 1
elif [[ $1 == api ]]
then
    # Run a ksh binary API test.
    api_test=true
    api_binary=$BUILD_DIR/$2
    shift 2
else
    # Run a ksh script test without compiling it.
    :
fi

if [[ $# -ne 1 ]]
then
    log_error "Expected one arg (the test name) possibly preceded by 'shcomp' or 'api', got $#: $*"
    exit 1
fi

# If the test should be skipped do so.
readonly test_name=$1
for test in "${tests_to_skip[@]}"
do
    system=${test% *}
    test_to_skip=${test#* }
    [[ $system == shcomp && $test_to_skip == "$test_name" ]] && \
        exit $MESON_SKIPPED_TEST
    [[ $system == "$MESON_SYSTEM" && $test_to_skip == "$test_name" ]] && \
        exit $MESON_SKIPPED_TEST
done

# TODO: Disable the `io` test on Travis macOS until we understand why it dies from an abort().
# I'm not seeing that failure happen on either of my macOS 10.12 or 10.13 systems.
if [[ $test_name == io && $OS_NAME == darwin && $CI == true ]]
then
    log_info 'Skipping io test on macOS on Travis'
    exit $MESON_SKIPPED_TEST
fi

#
# A test may need to alter its behavior based on the OS we're running on.
#
OS_NAME=$(uname -s | tr '[:upper:]' '[:lower:]')
export OS_NAME

#
# Make sure $USER is set. A CI/CB environment might not set it.
# See https://github.com/att/ast/issues/1391
#
if [[ -z $USER ]]
then
    USER=$(id -un)
    export USER
fi

# Scale the meson test timeout if a multiplier value is in the environment. Note that this requires
# the user explicitly export a MULTIPLIER var that matches the `meson test -t` value. I have opened
# issue https://github.com/mesonbuild/meson/issues/6009 asking that the timeout be exposed by Meson.
if [[ -n "$MULTIPLIER" ]]
then
    TIMEOUT=$(( TIMEOUT * MULTIPLIER ))
fi

#
# Setup the environment for the unit test.
#
if [[ $test_name == *.exp ]]
then
    readonly test_path=$TEST_ROOT/$test_name
elif [[ $api_test == true ]]
then
    readonly test_path=$api_binary
    # Ugh! This is somewhat ugly. There should be a better way to figure out the directory that
    # contains auxiliary files. One option is to put everything, not just API tests, one level below
    # the parent dir of this script.
    TEST_ROOT=$TEST_ROOT/api
else
    readonly test_path=$TEST_ROOT/$test_name
fi

if [[ ! -f $test_path ]]
then
    print -u2 "No such test script: $test_path"
    exit 1
fi

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
TEST_DIR=$(mktemp -dt "ksh.${test_name}.XXXXXX") || { log_error "mktemp -dt failed"; exit 1; }
export TEST_DIR
cd "$TEST_DIR" || { print -u2 "<E> 'cd $TEST_DIR' failed with status $?"; exit 1; }
log_info "TEST_DIR=$TEST_DIR"

#
# Make sure we search for external commands in the temporary test dir, then the test source dir,
# then the shell auxiliary commands, before any other directory in $PATH.
#
# This helps ensure that if the test creates a script file named "zzz" and there is an executable
# external command of the same name in PATH that we use the command created by the unit test.
# See issue #429.
#
# Note that a few additions to the path are to support specific platforms. For example, we want the
# GNU version of utilities like `diff` that are in the /usr/gnu/bin directory on Solaris. The
# /usr/xpg4/bin is to get POSIX versions of various commands but that also causes ksh to default to
# the "att" universe. The /xxx/bsd is to ensure ksh defaults to the "ucb" universe as almost all
# unit tests expect GNU or BSD behavior.
#
export OLD_PATH=$PATH
export SAFE_PATH="$TEST_DIR:$TEST_DIR/space dir:$TEST_ROOT:$BUILD_DIR/src/cmd/builtin"
export FULL_PATH=$SAFE_PATH:/xxx/bsd:/usr/gnu/bin:/usr/xpg4/bin:$OLD_PATH
export PATH=$FULL_PATH

#
# Create a couple of named pipes (fifos) for the unit test to use as barriers rather than using
# arbitrary sleeps. The numeric suffix reflects the file-descriptor that will be open for
# read-write on the fifo; e.g., `exec 9<>fifo9`.
#
mkfifo fifo9
mkfifo fifo8

#
# Figure out how many times to execute an empty loop to consume 100ms. This is used in a few places
# to create a CPU bound loop that can be executed for a specific duration with a minimum number of
# gettimeofday() syscalls (due to sampling $SECONDS).
#
integer iters_per_100ms
SECONDS=0
for ((iters_per_100ms = 0; ; iters_per_100ms++))
do
    (( iters_per_100ms % 20000 == 0 && SECONDS >= 0.1 )) && break
done
export ITERS_PER_10MS=$((iters_per_100ms / 10))
log_info ITERS_PER_10MS=$ITERS_PER_10MS

#
# We don't want the tests to modify the command history and the like of the user running the tests.
#
mkdir "$TEST_DIR/home"
mkdir "$TEST_DIR/home/space dir"
export HOME=$TEST_DIR/home
export HISTFILE=$TEST_DIR/sh_history
export ENV=/./$HOME/.kshrc  # inhibit reading system wide kshrc

# This is used to capture the `expect` based test line that failed.
typeset -a failure_lines

# Run a ksh API test. Modeled loosely on the older run_interactive function.
function run_api {
    $test_path > "$test_name.out" 2> "$test_name.err"
    exit_status=$?

    if [[ -e $TEST_ROOT/$test_name.out ]]
    then
        if ! diff -q "$TEST_ROOT/$test_name.out" "$test_name.out" > /dev/null
        then
            log_error "Stdout for $test_name had unexpected differences:"
            diff -U3 "$TEST_ROOT/$test_name.out" "$test_name.out" >&2
            exit_status=1
        fi
    elif [[ -s $test_name.out ]]
    then
            log_error "Stdout for $test_name should have been empty:"
            cat "$test_name.out" >&2
            exit_status=1
    fi

    if [[ -e $TEST_ROOT/$test_name.err ]]
    then
        if ! diff -q "$TEST_ROOT/$test_name.err" "$test_name.err" > /dev/null
        then
            log_error "Stderr for $test_name had unexpected differences:"
            diff -U3 "$TEST_ROOT/$test_name.err" "$test_name.err" >&2
            exit_status=1
        fi
    elif [[ -s $test_name.err ]]
    then
            log_error "Stderr for $test_name should have been empty:"
            cat "$test_name.err" >&2
            exit_status=1
    fi

    if [[ $exit_status -eq 0 ]]
    then
        # We only remove the temp test dir if the test is successful. Otherwise we leave it since
        # it may contain useful clues about why the test failed.
        cd /tmp || exit 1
        rm -rf "$TEST_DIR"
    fi

    return $exit_status
}

# Run an interactive ksh test via `expect`.
function run_interactive {
    final_iteration=$1
    # This is a no-op on the first invocation. It is needed so retries have a clean slate.
    if [[ -f $HOME/.kshrc ]]
    then
        rm -rf -- *
        mkdir "$HOME"
    fi

    cp "$TEST_ROOT/util/interactive.kshrc" "$HOME/.kshrc"

    # Use pre-populated history file for `hist` command unit test.
    if [[ $test_name == "b_hist.exp" ]]; then
        cp "$TEST_ROOT/data/sh_history" "$HISTFILE"
    fi

    exec 9<>fifo9
    rm -f interactive.tmp.log
    expect -n -c "source $TEST_ROOT/util/interactive.expect.rc" -f "$test_path" \
        > "$test_name.out" 2> "$test_name.err" &
    expect_pid=$!
    # Spawn a background task to kill the `expect` if it gets too close to the meson timeout.
    (
        read -r -t $(( TIMEOUT - SECONDS - 1 )) -u9 _x
        [[ $_x == 'done' ]] && exit 0
        kill -USR1 $expect_pid
        sleep 0.1
        kill -0 $expect_pid && kill -KILL $expect_pid
    ) &
    wait $expect_pid
    exit_status=$?
    print -u9 'done'
    (( exit_status == 265 )) && exit_status=$MESON_TEST_TIMEOUT
    (( exit_status == MESON_TEST_TIMEOUT )) && final_iteration=1

    if [[ -e $TEST_ROOT/$test_name.out ]]
    then
        if ! diff -q "$TEST_ROOT/$test_name.out" "$test_name.out" > /dev/null
        then
            log_error "Stdout for $test_name had unexpected differences:"
            diff -U3 "$TEST_ROOT/$test_name.out" "$test_name.out" >&2
            (( exit_status == 0 )) && exit_status=1
        fi
    elif [[ -s $test_name.out ]]
    then
            log_error "Stdout for $test_name should have been empty:"
            cat "$test_name.out" >&2
            (( exit_status == 0 )) && exit_status=1
    fi

    if [[ -e $TEST_ROOT/$test_name.err ]]
    then
        if ! diff -q "$TEST_ROOT/$test_name.err" "$test_name.err" > /dev/null
        then
            log_error "Stderr for $test_name had unexpected differences:"
            diff -U3 "$TEST_ROOT/$test_name.err" "$test_name.err" >&2
            (( exit_status == 0 )) && exit_status=1
        fi
    elif [[ -s $test_name.err ]]
    then
            log_error "Stderr for $test_name should have been empty:"
            cat "$test_name.err" >&2
            (( exit_status == 0 )) && exit_status=1
    fi

    if [[ $exit_status -eq 0 ]]
    then
        # We only remove the temp test dir if the test is successful. Otherwise we leave it since
        # it may contain useful clues about why the test failed.
        cd /tmp || exit 1
        rm -rf "$TEST_DIR"
    else
        # The final `.*` is because there may be a [ctrl-M] present (@#%! windows line endings).
        typeset line=$(sed -ne 's/^.*(file .*\.exp" line \([0-9]*\)).*$/\1/p' interactive.tmp.log)
        failure_lines+=("${line:-0}")

        if [[ $final_iteration -eq 0 ]]
        then
            log_warning "The last 20 lines of expect's interactive.tmp.log:"
            tail -20 interactive.tmp.log >&2
        fi
    fi

    return $exit_status
}

#
# Make sure any locale vars set by the user (or the continuous build environment) don't affect the
# tests. The only var we don't unset or change is `LANG` because we expect `meson test` to set it.
#
unset LC_ALL
unset LC_COLLATE
unset LC_CTYPE
unset LC_MESSAGES
unset LC_MONETARY
unset LC_NUMERIC
unset LC_TIME

#
# Make sure the user's environment doesn't affect tests. For example, if we don't unset these vars
# (or set them to known values) then things like the hist.exp test is likely to fail because the
# user's preferred editor may confuse the test.
#
unset EDITOR
unset FCEDIT
unset PAGER
unset VIEWER
unset VISUAL

#
# A couple of tests involve deep recursion that might require a stack up to 16MB, double that if
# run with ASAN enabled. Similarly, some tests need a large number of file descriptors.
#
ulimit=$(ulimit -s)
if [[ $ulimit == +([[:digit:]]) ]] && (( ulimit < 32764 ))
then
    ulimit -s 32764
fi

ulimit=$(ulimit -n)
if [[ $ulimit == +([[:digit:]]) ]] && (( ulimit < 1024 ))
then
    ulimit -n 1024
fi

if [[ $test_name == *.exp ]]
then
    # Interactive test.
    if ! command -v expect > /dev/null
    then
        log_info "Skipping $test_name because no expect command could be found"
        exit $MESON_SKIPPED_TEST
    fi

    # Interactive tests are flakey. Especially on CI test environments like Travis. So make several
    # attempts before giving up and reporting failure.
    set +o errexit
    status=0
    for i in 1 2 3
    do
        run_interactive $(( i == 3 ))
        status=$?
        [[ $status -eq 0 || $status -eq $MESON_TEST_TIMEOUT ]] && break
        log_info "Iteration $i of interactive test '$test_name' failed on line ${failure_lines[-1]}"
    done
    if [[ $status -eq 0 ]]
    then
        log_info "Iteration $i of interactive test '$test_name' passed"
    else
        log_warning "The entire expect's interactive.tmp.log:"
        cat interactive.tmp.log >&2
        if (( failure_lines[0] != failure_lines[1] || failure_lines[1] != failure_lines[2] ))
        then
            if (( status != MESON_TEST_TIMEOUT ))
            then
                log_info 'Ignoring test failures since it looks like the system is overloaded'
                log_info 'See issue #812'
                status=0
            fi
        fi
    fi
    exit $status
elif [[ $api_test == true ]]
then
    set +o errexit
    run_api
    status=$?
    exit $status
else
    # Non-interactive test.
    #
    # Create the actual unit test script by concatenating the stock preamble and postscript to the
    # unit test. Then run the composed script.
    readonly test_script=$test_name
    {
        print "#!$SHELL"
        cat "$TEST_ROOT/util/preamble.sh"
        cat "$test_path"
        cat "$TEST_ROOT/util/postscript.sh"
    } > "$test_script"
    chmod 755 "$test_script"

    set +o errexit
    if [[ $shcomp == false ]]
    then
        $SHELL -p "$TEST_DIR/$test_script" "$test_name" < /dev/null
        exit_status=$?
    else
        # The SHCOMP env var must be exported by Meson. If it isn't something is seriously wrong.
        # shellcheck disable=SC2153
        [[ -n "$SHCOMP" ]] || exit 1
        "$SHCOMP" "$test_script" > "$test_script.shcomp" || exit
        "$SHELL" -p "$TEST_DIR/$test_script.shcomp" "$test_name" < /dev/null
        exit_status=$?
    fi

    if (( exit_status == 0 || exit_status == MESON_SKIPPED_TEST ))
    then
        cd /tmp || exit 1
        rm -rf "$TEST_DIR"
    fi
    exit $exit_status
fi
