#!/usr/bin/env bash

# This script is used for generating bash compatibility source file that is required to build ksh93
set -x
set -e

pushd "$MESON_SOURCE_ROOT/src/cmd/ksh93/features"

# Generate a c source file for ksh93 bash compatiblity
echo "const char bash_pre_rc[] = " > "$MESON_SOURCE_ROOT/src/cmd/ksh93/data/bash_pre_rc.c"
sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/'"'"'/\\'"'"'/g' -e 's/^[[:space:]]*\(.*\)$/\"\1\\n\"/' "$MESON_SOURCE_ROOT/src/cmd/ksh93/data/bash_pre_rc.sh" >> "$MESON_SOURCE_ROOT/src/cmd/ksh93/data/bash_pre_rc.c"
echo ";" >> "$MESON_SOURCE_ROOT/src/cmd/ksh93/data/bash_pre_rc.c"

popd
