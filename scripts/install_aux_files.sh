#!/bin/sh
#
# Do installation steps that are hard to do via standard Meson functions such
# as `install_subdir()`.
#
ksh_aux_dir="${MESON_INSTALL_DESTDIR_PREFIX}/share/ksh"
ksh_man_dir="$ksh_aux_dir/man"
ksh_man_src="$MESON_SOURCE_ROOT/src/cmd/ksh93/docs"
umask 022

set -x
mkdir -m 755 "$ksh_aux_dir"
mkdir -m 755 "$ksh_man_dir"
mkdir -m 755 "$ksh_man_dir/man1"

# Note: I can't figure out how to make Sphinx `make man` emit the
# documentation in the expected hierachy under the _build dir. Since we
# currently only care about section one (i.e., command) documentation force
# those docs to be installed in that directory.
cd "$ksh_man_src" || exit 99
if [ -n "$(command -v sphinx-build)" ]
then
    make clean
    make man
fi
cp _build/man/*.1 "$ksh_man_dir/man1"

# Old versions of Meson (e.g., 0.44) don't recognize the `install_mode` option
# of functions like `install_subdir()` and don't use sensible default
# permissions (e.g., they have no public read/execute). So try to ensure all
# the aux files have reasonable permissions.
find "$ksh_aux_dir" -type d -print0 | xargs -0 chmod 755
find "$ksh_aux_dir" -type f -print0 | xargs -0 chmod 644
