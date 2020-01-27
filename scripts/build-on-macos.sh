#!/bin/sh
set -e

echo ==== Installing prereqs
brew list
# Note that python3 is already installed. As is python2. We need to unlink
# python2 to avoid conflicts when meson is installed.
brew unlink python@2
brew install meson
brew install sphinx

. scripts/travis_common.sh

mkdir build
cd build

echo ==== Configuring the build
if ! meson -Dwarnings-are-errors=true --prefix="$(mktemp -dt ksh.XXXXXX)" \
    --buildtype=debug
then
    cat meson-logs/meson-log.txt
    exit 1
fi

echo ==== Building the code
echo "<I> CC=$CC"
ninja || exit

echo ==== Running unit tests
# shellcheck disable=SC2039
ulimit -n 1024
CORE_COUNT=$(sysctl -n hw.ncpu)
export MESON_TESTTHREADS=$(( 2 * ${CORE_COUNT:-1} ))
echo "<I> CORE_COUNT=$CORE_COUNT MESON_TESTTHREADS=$MESON_TESTTHREADS"

# Enable command auditing.
echo "/tmp/ksh_auditfile;$(id -u)" | sudo tee /etc/ksh_audit

ninja install
if ! meson test -t2 --setup=malloc
then
    cat meson-logs/testlog-malloc.txt
    exit 1
fi
