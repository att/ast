#!/bin/bash
set -e

source /source/scripts/travis_common.sh

# See the "compiler: gcc" and INSTALL_REQUIREMENTS lines in the .travis.yml
# config file. They had better be in agreement and both specify "gcc".
# Strictly speaking this isn't necessary but is here to mirror what happens in
# the non-Docker test environments (e.g., macOS).
export CC=gcc

# Git repo is mounted to this directory in docker
cd /source

if [[ $SCAN_TYPE == "csbuild" ]]; then
    echo ==== Running csbuild
    exec csbuild --cswrap-timeout=5 --git-commit-range ${COMMIT_RANGE} -c 'meson -Dbuild-api-tests=false build; ninja -C build' -w1
fi

mkdir build
cd build

echo ==== Configuring the build
if ! meson -Dwarnings-are-errors=true
then
    cat meson-logs/meson-log.txt
    exit 1
fi

echo ==== Building the code
echo CC=$CC
ninja

echo ==== Running unit tests
ulimit -n 1024
CORE_COUNT=$(nproc)
export MESON_TESTTHREADS=$(( 4 * ${CORE_COUNT:-1} ))
echo MESON_TESTTHREADS=$MESON_TESTTHREADS

if ! meson test --setup=malloc
then
    cat meson-logs/testlog-malloc.txt
    exit 1
fi
