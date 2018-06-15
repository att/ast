#!/bin/bash

# Travis currently only provides two cores per VM:
# https://docs.travis-ci.com/user/reference/overview/.
# There is at least one open issue asking that this info be provided to the
# client environment: https://github.com/travis-ci/travis-ci/issues/4696.
# For now just hardcode the expected value.
CORE_COUNT=2

set -e
export LANG=en_US.UTF-8
export CFLAGS='-fno-strict-aliasing -Wno-unknown-pragmas -Wno-missing-braces -Wno-unused-result -Wno-return-type -Wno-int-to-pointer-cast -Wno-parentheses -Wno-unused -Wno-unused-but-set-variable -Wno-cpp -Wno-char-subscripts'

# Git repo is mounted to this directory in docker
cd /source
mkdir build
cd build

echo ==== Configuring the build
if ! meson; 
    then cat meson-logs/meson-log.txt
    exit 1
fi

echo ==== Building the code
ninja

echo ==== Running unit tests
ulimit -n 1024

export MESON_TESTTHREADS=$(( 4 * CORE_COUNT ))

if ! meson test --setup=malloc; then 
    cat meson-logs/testlog-malloc.txt
    exit 1
fi
