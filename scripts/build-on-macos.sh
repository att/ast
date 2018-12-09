#!/bin/sh
set -e

# As of 2018-06-30 meson 0.46.1 was being installed which fails with a
# StopIteration exception while configuring the build. So force using the
# head of the branch.
brew install --HEAD meson

source scripts/travis_common.sh

mkdir build
cd build

echo ==== Configuring the build
if ! meson -Dwarnings-are-errors=true
then
    cat meson-logs/meson-log.txt
    exit 1
fi

echo ==== Building the code
echo CC="$CC"
ninja || exit

echo ==== Running unit tests
# shellcheck disable=SC2039
ulimit -n 1024
CORE_COUNT=$(sysctl -n hw.ncpu)
export MESON_TESTTHREADS=$(( 4 * ${CORE_COUNT:-1} ))
echo MESON_TESTTHREADS=$MESON_TESTTHREADS

# Enable auditing
echo "/tmp/ksh_auditfile;$(id -u)" | sudo tee /etc/ksh_audit

# TODO: Run with --setup=malloc when Travis macOS is updated to 10.13 or
# newer. macOS 10.12 doesn't honor the MallocLogFile=/dev/null env var which
# results in lots of malloc debug messages being written to stderr which
# breaks the unit tests.
if ! meson test
then
    # cat meson-logs/testlog-malloc.txt
    cat meson-logs/testlog.txt
    exit 1
fi
