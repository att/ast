#!/usr/bin/env bash
set -e

# This script is used to setup iffe and other commands that are required for compilation
bin_dir="$MESON_SOURCE_ROOT/bin"

cp "$MESON_SOURCE_ROOT/src/lib/libast/comp/conf.sh" "$bin_dir/conf"
chmod u+x "$bin_dir/conf"
