#
# Make sure when called via `meson test` we've got the expected args.
#
shcomp=false
if [[ $# -eq 2 && $1 == shcomp ]]
then
    shcomp=true
    shift 1
fi

if [[ $# -ne 1 ]]
then
    print -u2 "<E> Expected one arg (the test name) possibly preceded by 'shcomp', got $#: $@"
    exit 99
fi

#
# Setup the environment for the unit test.
#
export TEST_SRC_DIR=${0%/*/*}  # capture the parent directory containing this script
readonly test_name=$1
readonly test_path=$TEST_SRC_DIR/$test_name.sh
readonly test_script=$test_name.sh
export BUILD_DIR=$PWD

#
# Create a temp dir and make it the CWD for the unit test. It will be removed by the unit test
# postscript if there are no test failures.
#
# The use of `mktemp -dt` isn't ideal as it has slightly different meaning on BSD and GNU. But for
# our purposes that doesn't matter. It simply means the temp file name will contain the X's on a BSD
# system.
#
export TEST_DIR=$(mktemp -dt ksh.${test_name}.XXXXXXX) ||
    { print -u2 "<E> mktemp -dt failed"; exit 99; }
cd $TEST_DIR || { print -u2 "<E> 'cd $TEST_DIR' failed with status $?"; exit 99; }

#
# Make sure we search for external commands in the temporary test dir, then the test source dir,
# then the shell auxiliary commands, before any other directory in $PATH.
#
# This helps ensure that if the test creates a script file named "zzz" and there is an executable
# external command of the same name in PATH that we use the command created by the unit test.
# See issue #429.
#
export ORIG_PATH=$PATH
export SAFE_PATH=$TEST_DIR:$TEST_SRC_DIR:$BUILD_DIR/src/cmd/builtin
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
export HOME=$TEST_DIR/home
export HISTFILE=$TEST_DIR/sh_history
#
# A test may need to alter its behavior based on the OS we're running on.
#
export OS_NAME=$(uname -s)

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
# Create the actual unit test script by concatenating the stock preamble and postscript to the unit
# test. Then run the composed script.
#
echo "#!$SHELL"                       > $test_script
cat $TEST_SRC_DIR/util/preamble.sh   >> $test_script
cat $test_path                       >> $test_script
cat $TEST_SRC_DIR/util/postscript.sh >> $test_script
chmod 755 $test_script
if [[ $shcomp == false ]]
then
    $TEST_DIR/$test_script $test_name < /dev/null
else
    $SHCOMP $test_script > $test_script.comp
    $SHELL $TEST_DIR/$test_script.comp $test_name < /dev/null
fi
