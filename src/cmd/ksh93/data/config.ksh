#
# This script is sourced first when the shell starts. It's purpose is to perform basic setup of the
# shell state. For example, where to find autoloaded functions that ship with the shell and arrange
# for them to be loaded on demand. It should not do anything not suitable for every single new ksh
# process. In particular it should be as fast as possible.
#

#
# Arrange for standard autoloaded functions to be available. The test for whether or not FPATH is
# already set isn't technically necessary since empty path components are guaranteed not to be
# equivalent to `.` (the CWD). But I prefer to be paranoid since doing so is cheap.
#
__fpath="${.sh.install_prefix}/share/ksh/functions"
if [[ -z ${FPATH:-''} ]]
then
    FPATH="$__fpath"
else
    FPATH="$__fpath:$FPATH"
fi

# Arrange for these function names to be autoloaded by declaring them to be undefined.
for f in "$__fpath"/*
do
    typeset -fu $(basename $f)
done

# Global vars for the `_cd`, `mcd`, `pushd`, `popd`, `dirs` functions.
integer _push_max=${CDSTACK:-32}
integer _push_top=${CDSTACK:-32}
typeset -a _push_stack

# Prefer the `cd` function to the `cd` builtin as the function is needed for functions like
# `dirs` and `popd` to work. Only do this for interactive shells.
if [[ $- == *i* && -f "$__fpath/_cd" ]]
then
    unalias cd 2>/dev/null  # the alias probably doesn't exist so don't complain in that case
    alias cd=_cd
fi

unset __fpath
