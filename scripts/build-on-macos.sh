#!/bin/sh
set -e
brew install meson

export LANG=en_US.UTF-8
export CFLAGS="-fno-strict-aliasing -Wno-unknown-pragmas -Wno-missing-braces"
CFLAGS="$CFLAGS -Wno-unused-result -Wno-return-type -Wno-int-to-pointer-cast"
CFLAGS="$CFLAGS -Wno-parentheses -Wno-unused"

# Removing all the TRAVIS_*, NodeJS, Ruby and other irrelevant env vars. This
# makes a huge difference in the size of the unit test logfile.
unset GEM_HOME
unset GEM_PATH
unset HAS_JOSH_K_SEAL_OF_APPROVAL
unset MANPATH
unset MY_RUBY_HOME
for v in $(env | sed -n -e '/^TRAVIS/s/=.*//p' -e '/^NVM/s/=.*//p' -e '/^rvm/s/=.*//p')
do
    unset $v
done

mkdir build
cd build

echo ==== Configuring the build
if ! meson
then
    cat meson-logs/meson-log.txt
    exit 1
fi

echo ==== Building the code
echo CC=$CC
ninja || exit

echo ==== Running unit tests
ulimit -n 1024
CORE_COUNT=$(sysctl -n hw.ncpu)
export MESON_TESTTHREADS=$(( 4 * ${CORE_COUNT:-1} ))
echo MESON_TESTTHREADS=$MESON_TESTTHREADS

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
