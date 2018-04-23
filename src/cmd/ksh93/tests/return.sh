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
# Test the behavior of return and exit with functions

unset HISTFILE

foo=NOVAL bar=NOVAL
file=$TEST_DIR/test
function foo
{
    typeset foo=NOEXIT
    trap "foo=EXIT;rm -f $file" EXIT
    > $file
    if (( $1 == 0 ))
    then
        return $2
    elif (( $1 == 1 ))
    then
        exit $2
    else
        bar "$@"
    fi
}

function bar
{
    typeset bar=NOEXIT
    trap 'bar=EXIT' EXIT
    if (( $1 == 2 ))
    then
        return $2
    elif (( $1 == 3 ))
    then
        exit $2
    fi

}

function funcheck
{
    [[ $foo == EXIT ]] || log_error "foo "$@" : exit trap not set"
    if [[ -f $file ]]
    then
        rm -r $file
        log_error "foo $@: doesn't remove $file"
    fi

    foo=NOVAL bar=NOVAL
}

(exit 0) || log_error "exit 0 is not zero"
(return 0) || log_error "return 0 is not zero"
(exit) || log_error "default exit value is not zero"
(return) || log_error "default return value is not zero"
(exit 35)
ret=$?
if (( $ret != 35 ))
then
    log_error "exit 35 is $ret not 35"
fi

(return 35)
ret=$?
if (( $ret != 35 ))
then
    log_error "return 35 is $ret not 35"
fi

foo 0 0 || log_error "foo 0 0: incorrect return"
funcheck 0 0
foo 0 3
ret=$?
if (( $ret != 3 ))
then
    log_error "foo 0 3: return is $ret not 3"
fi

funcheck 0 3
foo 2 0 || log_error "foo 2 0: incorrect return"
[[ $bar == EXIT ]] || log_error "foo 2 0: bar exit trap not set"
funcheck 2 0
foo 2 3
ret=$?
if (( $ret != 3 ))
then
    log_error "foo 2 3: return is $ret not 3"
fi

[[ $bar == EXIT ]] || log_error "foo 2 3: bar exit trap not set"
funcheck 2 3
(foo 3 3)
ret=$?
if (( $ret != 3 ))
then
    log_error "foo 3 3: return is $ret not 3"
fi

foo=EXIT
funcheck 3 3
cat > $file <<!
return 3
exit 4
!
( . $file )
ret=$?
if (( $ret != 3 ))
then
    log_error "return in dot script is $ret should be 3"
fi

chmod 755 $file
(  $file )
ret=$?
if (( $ret != 3 ))
then
    log_error "return in script is $ret should be 3"
fi

cat > $file <<!
: line 1
# next line should fail and cause an exit
: > /
exit 4
!
( . $file ; exit 5 ) 2> /dev/null
ret=$?
if (( $ret != 1 ))
then
    log_error "error in dot script is $ret should be 1"
fi

(  $file; exit 5 ) 2> /dev/null
ret=$?
if (( $ret != 5 ))
then
    log_error "error in script is $ret should be 5"
fi

cat > $file <<\!
print -r -- "$0"
!
x=$( . $file)
if [[ $x != $0 ]]
then
    log_error "\$0 in a dot script is $x. Should be $0"
fi

x=$($SHELL -i --norc 2> /dev/null <<\!
typeset -i x=1/0
print hello
!
)
if [[ $x != hello ]]
then
    log_error "interactive shell terminates with error in bltin"
fi

x=$( set -e
    false
    print bad
    )
if [[ $x != '' ]]
then
    log_error "set -e doesn't terminate script on error"
fi

x=$( set -e
    trap 'exit 0' EXIT
    false
    print bad
    )
if (( $? != 0 ))
then
    log_error "exit 0 in trap should doesn't set exit value to 0"
fi

$SHELL <<\!
trap 'exit 8' EXIT
exit 1
!
if (( $? != 8 ))
then
    log_error "exit 8 in trap should set exit value to 8"
fi
