#
# Install the man pages. If /usr/local is the install prefix this will put the `ksh` man page in the
# default man page directory: /usr/local/share/man/man1. The man pages for individual builtins will
# be installed in /usr/local/share/ksh/man.
#
install_man('ksh.1')

# I can't figure out how to make Sphinx `make man` emit the documentation in the expected hierachy
# under the _build dir. Since we currently only care about section one (i.e., command) documentation
# force it to be installed in that directory.
#
# Note: We can use the Meson `add_install_script()` directive if and when we need to handle more
# than section one man pages. Such a script could split the contents of _build/man into separate
# subdirs based on the file name extensions.
install_subdir('_build/man', install_dir: 'share/ksh/man/man1', strip_directory: true)
