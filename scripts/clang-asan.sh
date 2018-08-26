#!/bin/sh
#
# Helper script to determine a couple of options needed when building with
# clang and ASAN enabled.
#
rpath_arg=no

for arg in $( clang -fsanitize=address -v -o /tmp/clang$$.out etc/hdrs.c 2>&1 | tail -1 )
do
    if [ $rpath_arg = yes ]
    then
        rpath_arg=no
        echo "$arg"
    else
        case "$arg" in
            -rpath)
                rpath_arg=yes
                echo "$arg"
                ;;
            -lpthread)
                echo "$arg"
                ;;
            -L*)
                echo "$arg"
                ;;
            *libclang_rt.asan*)
                echo "$arg"
                ;;
        esac
    fi
done
rm /tmp/clang$$.out
