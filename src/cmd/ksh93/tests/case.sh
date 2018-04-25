########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2011 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                    David Korn <dgkorn@gmail.com>                     #
#                                                                      #
########################################################################

bar=foo2
bam=foo[3]
for i in foo1 foo2 foo3 foo4 foo5 foo6
do
    foo=0
    case $i in
    foo1)    foo=1;;
    $bar)    foo=2;;
    $bam)    foo=3;;
    foo[4])    foo=4;;
    ${bar%?}5)
        foo=5;;
    "${bar%?}6")
        foo=6;;
    esac
    if [[ $i != foo$foo ]]
    then
        log_error "$i not matching correct pattern"
    fi
done

f="[ksh92]"
case $f in
\[*\])  ;;
*)      log_error "$f does not match \[*\]";;
esac

if [[ $($SHELL -c '
        x=$(case abc {
            abc)    { print yes;};;
            *)     print no;;
            }
        )
        print -r -- "$x"' 2> /dev/null) != yes ]]
then
    log_error 'case abc {...} not working'
fi

actual=$($SHELL -c '
    case a in
    a) print -n a > /dev/null ;&
    b) print b ;;
    esac
    ')
expect=b
[[ $actual == $expect ]] || log_error 'bug in ;& at end of script' "$expect" "$actual"

actual=$($SHELL -c '
    tmp=foo
    for i in a b
    do    case $i in
        a)    :  tmp=$tmp tmp.h=$tmp.h;;
        b)    ( tmp=bar )
            for j in a
            do    print -r -- $tmp.h
            done
            ;;
        esac
    done
    ')
expect=foo.h
[[ $actual == $expect ]] || log_error "optimizer bug" "$expect" "$actual"

x=$($SHELL -ec 'case a in a) echo 1; false; echo 2 ;& b) echo 3;; esac')
[[ $x == 1 ]] || log_error 'set -e ignored on case fail through'

# https://github.com/att/ast/issues/476
case "[0-9]" in
    [0-9]) log_error "Shell should not match pattern as literal string in case command";;
    "[0-9]") ;;
    *) log_error "Shell fails to match quoted pattern-like string as literal string";;
esac
