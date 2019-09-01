#
# This script is sourced first when the shell starts. It's purpose is to perform basic setup of the
# shell state. For example, where to find autoloaded functions that ship with the shell and arrange
# for them to be loaded on demand. It should not do anything not suitable for every single new ksh
# process. In particular it should be as fast as possible.
#

# Arrange for standard autoloaded functions to be available. The test for whether or not FPATH is
# already set isn't technically necessary since empty path components are guaranteed not to be
# equivalent to `.` (the CWD). But I prefer to be paranoid since doing so is cheap.
__fpath="${.sh.install_prefix}/share/ksh/functions"
if [[ -z $FPATH ]]
then
    FPATH="$__fpath"
else
    FPATH="$__fpath:$FPATH"
fi

for f in "$__fpath"/*
do
    typeset -fu $(basename $f)
done
unset __fpath
