# Tests for command builtin

# ========
function ls { return 1; }
command ls >/dev/null 2>/dev/null || log_error 'command should not run functions'
unset -f ls

# ========
# -p Causes a default path to be searched rather than the one defined by the value of PATH.
PATH="" command -p ls >/dev/null 2>/dev/null || log_error 'command -p should search in default path'

# ========
# -v Equivalent to whence command [arg ...].
[[ $(command -v if) = if ]] || log_error 'command -v not working'
[[ $(command -v if) = $(whence if) ]] || log_error 'command -v should have same output as whence'

# ========
#  -V Equivalent to whence -v command [arg ...].
[[ $(command -V ls) = $(whence -v ls) ]] || log_error "command -V should have same output as whence -v"

# ========
function longline
{
    integer i
    for((i=0; i < $1; i++))
    do
        print argument$i
    done
}

# test command -x option
integer sum=0 n=10000
if ! ${SHELL:-ksh} -c 'print $#' count $(longline $n) > /dev/null  2>&1
then
    for i in $(command command -x ${SHELL:-ksh} -c 'print $#;[[ $1 != argument0 ]]' count $(longline $n) 2> /dev/null)
    do
        ((sum += $i))
   done

   (( sum == n )) || log_error "command -x processed only $sum arguments"
   command -p command -x ${SHELL:-ksh} -c 'print $#;[[ $1 == argument0 ]]' count $(longline $n) > /dev/null  2>&1
   [[ $? != 1 ]] && log_error 'incorrect exit status for command -x'
fi

# ========
# test command -x option with extra arguments
integer sum=0 n=10000
if   ! ${SHELL:-ksh} -c 'print $#' count $(longline $n) > /dev/null  2>&1
then
    for i in $(command command -x ${SHELL:-ksh} -c 'print $#;[[ $1 != argument0 ]]' count $(longline $n) one two three) #2> /dev/null)
    do
        ((sum += $i))
    done

    (( sum  > n )) || log_error "command -x processed only $sum arguments"
    (( (sum-n)%3==0 )) || log_error "command -x processed only $sum arguments"
    (( sum == n+3)) && log_error "command -x processed only $sum arguments"
    command -p command -x ${SHELL:-ksh} -c 'print $#;[[ $1 == argument0 ]]' count $(longline $n) > /dev/null  2>&1
    [[ $? != 1 ]] && log_error 'incorrect exit status for command -x'
fi

# ========
unset y
expect='outside f, 1, 2, 3, outside f'
actual=$(
    f() {
        if [[ -n "${_called_f+_}" ]]
        then
            for y
            do
                printf '%s, ' "$y"
            done
        else
            _called_f= y= command eval '{ typeset +x y; } 2>/dev/null; f "$@"'
        fi

    }
    y='outside f'
    printf "$y, "
    f 1 2 3
    echo "$y"
)
[[ $actual == "$expect" ]] ||
   log_error 'assignments to "command special_built-in" leaving side effects' "$expect" "$actual"

# ========
# Regression test for https://github.com/att/ast/issues/1402.
#
# We throw away stderr because we only want the value of `$t` not the error text from running
# `command` with an invalid flag.
expect='good'
actual=$($SHELL -c 't=good; t=bad command -@; print $t' 2>/dev/null)
[[ $expect == $actual ]] || log_error 'temp var assignment with `command`' "$expect" "$actual"
