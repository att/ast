#!/bin/sh
#
# Do installation steps that are hard to do via the standard Meson function
# (e.g., `install_subdir()`).
#
ksh_aux_dir="$MESON_INSTALL_PREFIX/share/ksh"
ksh_man_dir="$ksh_aux_dir/man"
ksh_man_src="$MESON_SOURCE_ROOT/src/cmd/ksh93/docs"
umask 022

set -x
mkdir -m 755 "$ksh_aux_dir"
mkdir -m 755 "$ksh_man_dir"
mkdir -m 755 "$ksh_man_dir/man1"
cd "$ksh_man_src" || exit 99

# At some point we'll want to uncomment this statement. Probably after we've
# converted all the documentation to Sphinx format. At that point we'll want
# to stop committing the `make man` artifacts when the docs are changed.
# Instead, require that be done at install time.
#
# Note: I can't figure out how to make Sphinx `make man` emit the
# documentation in the expected hierachy under the _build dir. Since we
# currently only care about section one (i.e., command) documentation force it
# to be installed in that directory.
#
# make man
cp _build/man/*.1 "$ksh_man_dir/man1"

# Old versions of Meson (e.g., 0.44) don't recognize the `install_mode` option
# of functions like `install_subdir()` and don't use sensible default
# permissions (e.g., no public read/execute). So try to ensure all the aux
# files have reasonable permissions.
find "$ksh_aux_dir" -type d -print0 | xargs -0 chmod 755
find "$ksh_aux_dir" -type f -print0 | xargs -0 chmod 644
