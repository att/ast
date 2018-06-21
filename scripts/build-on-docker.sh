#!/bin/bash
set -e
export LANG=en_US.UTF-8
export CC=gcc  # see the "compiler: gcc" line in the .travis.yml config file
export CFLAGS="-fno-strict-aliasing -Wno-unknown-pragmas -Wno-missing-braces"
CFLAGS="$CFLAGS -Wno-unused-result -Wno-return-type -Wno-int-to-pointer-cast"
CFLAGS="$CFLAGS -Wno-parentheses -Wno-unused -Wno-unused-but-set-variable -Wno-cpp"

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

# Git repo is mounted to this directory in docker
cd /source
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
