#!/bin/sh
# This script does not do feature detection in the usual sense. In fact it
# must be run after the normal Meson feature detection and the config_ast.h
# header has been created. That is why it isn't part of the feature detection
# code in the "features" subdirectory.
#
# This script creates dynamically generated source files. Which can involve
# detecting whether the platform supports specific symbols (e.g.,
# ABI_AIO_XFER_MAX) but that is secondary.
#
set -e
bin_dir="$MESON_SOURCE_ROOT/bin"
comp_dir="$MESON_SOURCE_ROOT/src/lib/libast/comp"
PATH=$bin_dir:$PATH
INC_DIRS=""
INC_DIRS="$INC_DIRS -I$MESON_BUILD_ROOT"
INC_DIRS="$INC_DIRS -I$MESON_SOURCE_ROOT/src/lib/libast/include"
INC_DIRS="$INC_DIRS -I$MESON_SOURCE_ROOT/src/lib/libast/features"
INC_DIRS="$INC_DIRS -I$MESON_SOURCE_ROOT/src/cmd/std"

if [ -z "$CC" ];
then
    CC=cc
fi

cd "$MESON_BUILD_ROOT"

if cc --version | grep -q "GCC"
then
    gcc_major_version=$(cc -dumpversion | cut -d. -f1)
    if [ "$gcc_major_version" -ge 9 ]
    then
        extra_flags="-fno-diagnostics-show-line-numbers"
    fi
fi

# Generate the conftab.[ch] source files.
# shellcheck disable=SC2086
"$comp_dir/conf.sh" $CC -std=gnu99 -D_BLD_DLL $INC_DIRS $extra_flags

# Generate header files whose content depends on the current platform.
"$MESON_SOURCE_ROOT/scripts/siglist.sh" > features/siglist.h
# shellcheck disable=SC2086
$CC -D_BLD_DLL $INC_DIRS -std=gnu99 -o "sfinit" "$MESON_SOURCE_ROOT/etc/sfinit.c"
./sfinit > features/sfinit.h
