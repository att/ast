#!/bin/sh
#
# This wraps the python scripts used during the build configuration phase. We
# do this because we can't count on python to be in PATH under that name. It
# might be installed as a large number of variants. Hence this script to try
# and find it.
#
script=$1
shift

for name in python python3 python2 python3.6 python3.5 python2.7
do
    path=$(command -v $name)
    if test -n "$path"
    then
        exec "$path" "$MESON_SOURCE_ROOT/scripts/$script" "$@"
    fi
done

echo Error: Could not find python under any of the names we know >&2
find / -xdev -name python\* >&2
exit 1
