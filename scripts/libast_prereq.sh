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
PATH=$bin_dir:$PATH
INC_DIRS=""
INC_DIRS="$INC_DIRS -I$MESON_BUILD_ROOT"
INC_DIRS="$INC_DIRS -I$MESON_SOURCE_ROOT/src/lib/libast/include"

if [ -z "$CC" ]
then
    CC=cc
fi

cd "$MESON_BUILD_ROOT"

# Generate header files whose content depends on the current platform.
# shellcheck disable=SC2086
$CC -D_BLD_DLL $INC_DIRS -std=gnu99 -o "sfinit" "$MESON_SOURCE_ROOT/etc/sfinit.c"
./sfinit > features/sfinit.h
