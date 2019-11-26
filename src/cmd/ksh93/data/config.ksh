#!/usr/bin/env ksh
#
# This script is sourced first when the shell starts. It's purpose is to perform basic setup of the
# shell state. For example, where to find autoloaded functions that ship with the shell and arrange
# for them to be loaded on demand. It should not do anything not suitable for every single new ksh
# process. In particular it should be as fast as possible.
#

# See https://github.com/att/ast/issues/1453. Make sure we have a minimal $PATH.
if [[ -z "$PATH" ]]
then
    # Getconf should be in /usr/bin but also check other dirs; especially
    # since, if we can't find it or it produces no output, we want a
    # reasonable set of directories to search for external commands.
    PATH=/usr/bin:/bin:/usr/local/bin
    if whence -p getconf >/dev/null
    then
        __path="$(getconf PATH 2>/dev/null)"
        [[ -n "$__path" ]] && PATH="$__path"
        unset __path
    fi
fi

#
# Arrange for standard autoloaded functions to be available. The test for whether or not FPATH is
# already set isn't technically necessary since empty path components are guaranteed not to be
# equivalent to `.` (the CWD). But I prefer to be paranoid since doing so is cheap.
#
# shellcheck disable=SC2154  # this var is builtin to ksh
__fpath="${.sh.install_prefix}/share/ksh/functions"
if [[ -z ${FPATH:-''} ]]
then
    FPATH="$__fpath"
else
    FPATH="$__fpath:$FPATH"
fi

# Arrange for these functions to be autoloaded by declaring them to be undefined if the
# corresponding file is found in FPATH.
for f in "$__fpath"/*
do
    typeset -fu "${f##*/}"  # this is `basename` but faster
done

# Global vars for the `cd`, `dirs`, `mcd`, `pushd`, `popd`, `prevd`, `nextd` functions.
integer _push_max="${CDSTACK:-32}"
integer _push_top="${CDSTACK:-32}"
# shellcheck disable=SC2034  # this var is used by autoloaded functions
typeset -a _push_stack

unset __fpath
