#
# Make sure when called via `meson test` we've got the expected args.
#
function log_info {
    typeset lineno=$1
    print -r "<I> run_test[$lineno]: ${@:2}"
}
alias log_info='log_info $LINENO'

function log_warning {
    typeset lineno=$1
    print -u2 -r "<W> run_test[$lineno]: ${@:2}"
}
alias log_warning='log_warning $LINENO'

function log_error {
    typeset lineno=$1
    print -u2 -r "<E> run_test[$lineno]: ${@:2}"
}
alias log_error='log_error $LINENO'

shcomp=false
if [[ $# -eq 2 && $1 == shcomp ]]
then
    shcomp=true
    shift 1
fi

if [[ $# -ne 1 ]]
then
    log_error "Expected one arg (the test name) possibly preceded by 'shcomp', got $#: $@"
    exit 99
fi

#
# A test may need to alter its behavior based on the OS we're running on.
#
export OS_NAME=$(uname -s)

# TODO: Enable the `io` test on Travis macOS once we understand why it dies from an abort().
# I'm not seeing that failure happen on either of my macOS 10.12 or 10.13 systems.
if [[ $test_name == io && $OS_NAME == Darwin && $CI == true ]]
then
    log_info 'Skipping io test on macOS on Travis'
    exit 0
fi

#
# Setup the environment for the unit test.
#
export TEST_SRC_DIR=${0%/*/*}  # capture the parent directory containing this script
readonly test_name=$1
if [[ $test_name == *.exp ]]
then
    readonly test_path=$TEST_SRC_DIR/$test_name
else
    readonly test_path=$TEST_SRC_DIR/$test_name.sh
fi
readonly test_script=$test_name.sh
export BUILD_DIR=$PWD

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
export TEST_DIR=$(mktemp -dt ksh.${test_name}.XXXXXXX) ||
    { log_error "mktemp -dt failed"; exit 99; }
cd $TEST_DIR || { print -u2 "<E> 'cd $TEST_DIR' failed with status $?"; exit 99; }
log_info "TEST_DIR=$TEST_DIR"

#
# Make sure we search for external commands in the temporary test dir, then the test source dir,
# then the shell auxiliary commands, before any other directory in $PATH.
#
# This helps ensure that if the test creates a script file named "zzz" and there is an executable
# external command of the same name in PATH that we use the command created by the unit test.
# See issue #429.
#
export ORIG_PATH=$PATH
export SAFE_PATH="$TEST_DIR:$TEST_DIR/space dir:$TEST_SRC_DIR:$BUILD_DIR/src/cmd/builtin"
export FULL_PATH=$SAFE_PATH:$ORIG_PATH
export PATH=$FULL_PATH

#
# Create a couple of named pipes (fifos) for the unit test to use as barriers rather than using
# arbitrary sleeps. The numeric suffix reflects the file-descriptor that will be open for
# read-write on the fifo; e.g., `exec 9<>fifo9`.
#
mkfifo fifo9
mkfifo fifo8

#
# We don't want the tests to modify the command history and the like of the user running the tests.
#
mkdir $TEST_DIR/home
mkdir "$TEST_DIR/home/space dir"
export HOME=$TEST_DIR/home
export HISTFILE=$TEST_DIR/sh_history

# This is used to capture the `expect` based test line that failed.
typeset -a failure_lines

function run_interactive {
    final_iteration=$1
    # This is a no-op on the first invocation. It is needed so retries have a clean slate.
    if [[ -f $HOME/.kshrc ]]
    then
        rm -rf *
        mkdir $HOME
    fi

    cp $TEST_SRC_DIR/util/interactive.kshrc $HOME/.kshrc

    # Use pre-populated history file for history related tests
    if [[ $test_name == "hist.exp" ]]; then
        cp $TEST_SRC_DIR/util/sh_history $HISTFILE
    fi

    export TERM=dumb
    expect -n -c "source $TEST_SRC_DIR/util/interactive.expect.rc" -f $test_path \
        >$test_name.out 2>$test_name.err
    exit_status=$?

    if [[ -e $TEST_SRC_DIR/$test_name.out ]]
    then
        if ! diff -q $TEST_SRC_DIR/$test_name.out $test_name.out >/dev/null
        then
            log_error "Stdout for $test_name had unexpected differences:"
            diff -U3 $TEST_SRC_DIR/$test_name.out $test_name.out >&2
            exit_status=1
        fi
    elif [[ -s $test_name.out ]]
    then
            log_error "Stdout for $test_name should have been empty:"
            cat $test_name.out >&2
            exit_status=1
    fi

    if [[ -e $TEST_SRC_DIR/$test_name.err ]]
    then
        if ! diff -q $TEST_SRC_DIR/$test_name.err $test_name.err >/dev/null
        then
            log_error "Stderr for $test_name had unexpected differences:"
            diff -U3 $TEST_SRC_DIR/$test_name.err $test_name.err >&2
            exit_status=1
        fi
    elif [[ -s $test_name.err ]]
    then
            log_error "Stderr for $test_name should have been empty:"
            cat $test_name.err >&2
            exit_status=1
    fi

    if [[ $exit_status -eq 0 ]]
    then
        # We only remove the temp test dir if the test is successful. Otherwise we leave it since
        # it may contain useful clues about why the test failed.
        cd /tmp
        rm -rf $TEST_DIR
    else
        # The final `.*` is because there may be a [ctrl-M] present (@#%! windows line endings).
        typeset line=$(sed -ne 's/^.*(file .*\.exp" line \([0-9]*\)).*$/\1/p' interactive.tmp.log)
        failure_lines+=(${line:-0})

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

if [[ $test_name == *.exp ]]
then
    # Interactive test.
    if ! command -v expect >/dev/null
    then
        log_info "Skipping $test_name because no expect command could be found"
        exit 0
    fi

    if [[ $shcomp == true ]]
    then
        log_error "Interactive tests cannot be run via shcomp"
        exit 1
    fi

    if [[ $OS_NAME == FreeBSD ]]
    then
        # TODO: Explore why this was blacklisted or if it can now be enabled on that platform.
        # These tests always fail on the first `expect_prompt` use. Which suggests a bug in how
        # `expect` implements timeouts on FreeBSD 11 (at least when run as a VM).
        log_info "Skipping test on $OS_NAME"
        exit 0
    fi

    # Interactive tests are flakey on CI test environments like Travis. So make several attempts
    # before reporting giving up and reporting failure.
    status=0
    for i in 1 2 3
    do
        run_interactive $(( i == 3 ))
        status=$?
        [[ $status -eq 0 ]] && break
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
            log_info 'Ignoring test failures since it looks like the system is overloaded'
            log_info 'See issue #812'
            status=0
        fi
    fi
    exit $status
else
    # Non-interactive test.
    #
    # Create the actual unit test script by concatenating the stock preamble and postscript to the
    # unit test. Then run the composed script.
    echo "#!$SHELL"                       > $test_script
    cat $TEST_SRC_DIR/util/preamble.sh   >> $test_script
    cat $test_path                       >> $test_script
    cat $TEST_SRC_DIR/util/postscript.sh >> $test_script
    chmod 755 $test_script
    if [[ $shcomp == false ]]
    then
        $TEST_DIR/$test_script $test_name < /dev/null
    else
        $SHCOMP $test_script > $test_script.comp || exit
        $SHELL $TEST_DIR/$test_script.comp $test_name < /dev/null
    fi
fi
