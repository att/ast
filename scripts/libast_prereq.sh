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
set -x
bin_dir="$MESON_SOURCE_ROOT/bin"
comp_dir="$MESON_SOURCE_ROOT/src/lib/libast/comp"
PATH=$bin_dir:$PATH
INC_DIRS=""
INC_DIRS="$INC_DIRS -I$MESON_BUILD_ROOT"
INC_DIRS="$INC_DIRS -I$MESON_SOURCE_ROOT/src/lib/libast/include"
INC_DIRS="$INC_DIRS -I$MESON_SOURCE_ROOT/src/lib/libast/features"
INC_DIRS="$INC_DIRS -I$MESON_SOURCE_ROOT/src/cmd/std"

cd $MESON_BUILD_ROOT

"$comp_dir/conf.sh" cc -std=gnu99 -D_BLD_DLL -D_BLD_ast $INC_DIRS || exit

$MESON_SOURCE_ROOT/scripts/siglist.sh > features/siglist.h || exit

for name in sfinit signal; do
    cc -D_BLD_DLL -D_BLD_ast $INC_DIRS -std=gnu99 -o $name $MESON_SOURCE_ROOT/src/lib/libast/features/$name.c
    ./$name > features/$name.h
done
