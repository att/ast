#!/bin/sh
# This script is used for generating bash compatibility source file that is
# required to build ksh93.
#
set -e
set -x
cd "$MESON_SOURCE_ROOT/src/cmd/ksh93/features"
src="$MESON_SOURCE_ROOT/src/cmd/ksh93/data/bash_pre_rc.sh"
dst="$MESON_SOURCE_ROOT/src/cmd/ksh93/data/bash_pre_rc.c"
echo "const char bash_pre_rc[] = " > "$dst"
sed -e 's/\\/\\\\/g' \
    -e 's/"/\\"/g' -e 's/'"'"'/\\'"'"'/g' \
    -e 's/^[[:space:]]*\(.*\)$/\"\1\\n\"/' "$src" >> "$dst"
echo ';' >> "$dst"
