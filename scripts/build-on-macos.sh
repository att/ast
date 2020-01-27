#!/bin/sh
set -e

echo ==== Installing prereqs
brew list
# Note that python3 is already installed. As it python2. We need to unlink
# python2 and force python3 links in /usr/local to be created.
brew unlink python@2
brew link --overwrite python
brew install meson

. scripts/travis_common.sh

mkdir build
cd build

echo ==== Configuring the build
if ! meson -Dwarnings-are-errors=true --buildtype=debug
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
