#!/bin/sh
# Generate the source module containing the builtin command documentation from
# the raw markup text.
out_file="$MESON_BUILD_ROOT/$MESON_SUBDIR/documentation.c"
exec > "$out_file"
for in_file in "$MESON_SOURCE_ROOT/src/cmd/ksh93/docs/"*.1
do
    cmd_name=$(basename "$in_file" .1)
    [ "$cmd_name" = ksh ] && continue
    echo "const char sh_opt${cmd_name}[] ="
    sed -e 's/\(.*\)/    "\1\\n"/' < "$in_file"
    echo ";"
done
