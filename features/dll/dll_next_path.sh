#!/bin/sh
lib=
for d in  /shlib /usr/shlib /lib /usr/lib
do
    if test -d $d
    then
        for s in "*.*" "*[!a]*"
        do
            # shellcheck disable=SC2043
            # Notice the `break 3` in the body of the loop.
            for b in libc
            do
                for i in $d/$b.$s
                do
                    if test -f "$i"
                    then
                        lib=$i
                    fi
                done
                case $lib in
                ?*) break 3 ;;
                esac
            done
        done
    fi
done

case $lib in
*.[0-9]*.[0-9]*)
   # shellcheck disable=SC2001
   i=$(echo "$lib" | sed 's,\([^0-9]*[0-9]*\).*,\1,')
   if test -f "$i"
   then
      lib=$i
   fi
   ;;
esac

# Some run time linkers barf with /lib/xxx if /usr/lib/xxx is there.
case $lib in
   /usr*) ;;
   *) if test -f "/usr$lib"
   then
      lib=/usr$lib
   fi
   ;;
esac

case $lib in
   "")
      lib=/lib/libc.so.1
      ;;
esac

case $lib in
   /usr/lib/*)
    case $(package 2>/dev/null) in
    sgi.mips3)
        abi=/lib32
        ;;
    sgi.mips4)
        abi=/lib64
        ;;
    *)
        abi=
        ;;
    esac
    case $abi in
    ?*)
        if test -d $abi
        then
           # shellcheck disable=SC2001
           lib=$(echo "$lib" | sed 's,/usr/lib/,,')
            lib=$abi/$lib
        fi
        ;;
    esac
    ;;
esac
echo "$lib"
