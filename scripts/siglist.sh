#!/bin/sh
#
# Generate entries for the 'static struct _m_ map[]' table in src/lib/libast/features/signal.c.
#
# shellcheck disable=SC2021
for sig in $(/bin/kill -l | tr '[a-z]' '[A-Z]')
do
    case $sig in
    *[!A-Z0-9_]*|*MIN|*MAX)
        ;;
    *)	echo "#if defined(SIG$sig) && !defined(HAD_SIG$sig)"
        echo "0, \"$sig\", SIG$sig,"
        echo "#endif"
        ;;
    esac
done
exit 0
