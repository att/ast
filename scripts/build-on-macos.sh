#!/bin/sh
brew install meson

export CFLAGS="-fno-strict-aliasing -Wno-unknown-pragmas -Wno-missing-braces"
CFLAGS="$CFLAGS -Wno-unused-result -Wno-return-type -Wno-int-to-pointer-cast"
CFLAGS="$CFLAGS -Wno-parentheses -Wno-unused -Wno-char-subscripts"

mkdir build
cd build

echo ==== Configuring the build
if ! meson
then
    cat meson-logs/meson-log.txt
    exit 1
fi

echo ==== Building the code;
ninja || exit

# TODO: Enable when the known test failures on macOS have been fixed.
#echo ==== Running unit tests
#ulimit -n 1024
#export MESON_TESTTHREADS=$(( 4 * CORE_COUNT ))
#if ! meson test
#then
#    cat meson-logs/testlog.txt
#    exit 1
#fi
