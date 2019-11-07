########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2012 AT&T Intellectual Property          #
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
# Test the behavior of co-processes

# ========
# Verify we can't have more than one coprocess running.
# See https://github.com/att/ast/issues/1373
actual="$($SHELL -c 'sleep 10 |& ; sleep 10 |& ; kill $(jobs -p)' 2>&1)"
expect="ksh: coprocess is running; cannot create a new coprocess"
[[ "$actual" == *"$expect" ]] ||
    log_error "concurrent coprocesses were allowed" "$expect" "$actual"

function ping # id
{
    integer x=0
    while ((x++ < 5))
    do
    read -r
        print -r "$1 $REPLY"
    done
}

for cat in cat $bin_cat
do
    $cat |&
    print -p "hello"
    read -p line
    [[ $line == hello ]] || log_error "$cat coprocessing fails"
    exec 5>&p 6<&p
    print -u5 'hello again' || log_error "write on $cat coprocess u5 fails"
    read -u6 line
    [[ $line == 'hello again' ]] || log_error "$cat coprocess after moving fds fails"
    exec 5<&- 6<&-
    wait $!

    ping three |&
    exec 3>&p
    ping four |&
    exec 4>&p
    ping pipe |&

    integer count
    for i in three four pipe four pipe four three pipe pipe three pipe
    do
    case $i in
        three)    to=-u3;;
        four)    to=-u4;;
        pipe)    to=-p;;
        esac
        (( count++ ))
        print $to $i $count
    done

    while ((count > 0))
    do
    (( count-- ))
        read -p
        set -- $REPLY
        if [[ $1 != $2 ]]
        then
            log_error "$cat coprocess $1 does not match $2"
        fi

        case $1 in
        three)    ;;
        four)    ;;
        pipe)    ;;
        *)    log_error "$cat coprocess unknown message +|$REPLY|+" ;;
        esac
    done
    kill $(jobs -p)
    wait

    file=$TEST_DIR/regress
    $cat > $file  <<-!
	$cat |&
	!
    chmod +x $file
    sleep 10 |&
    $file 2> /dev/null || log_error "parent $cat coprocess prevents script coprocess"
    exec 5<&p 6>&p
    exec 5<&- 6>&-
    kill $(jobs -p) 2>/dev/null

    ${SHELL-ksh} |&
    cop=$!
    exp=Done
    print -p $'print hello | '$cat$'\nprint '$exp
    read -t 5 -p
    read -t 5 -p
    got=$REPLY
    if [[ $got != $exp ]]
    then
        log_error "${SHELL-ksh} $cat coprocess io failed -- got '$got', expected '$exp'"
    fi

    exec 5<&p 6>&p
    exec 5<&- 6>&-
    { sleep 4; kill $cop; } 2>/dev/null &
    spy=$!
    if wait $cop 2>/dev/null
    then
        kill $spy 2>/dev/null
    else
        log_error "$cat coprocess hung after 'exec 5<&p 6>&p; exec 5<&- 6>&-'"
    fi

    wait

    {
    echo line1 | grep 'line2'
    echo line2 | grep 'line1'
    } |&
    SECONDS=0 count=0
    while    read -p -t 10 line
    do
    ((count++))
    done
    if (( SECONDS > 8 ))
    then
        log_error "$cat coprocess read -p hanging (SECONDS=$SECONDS count=$count)"
    fi

    wait $!

    ( sleep 3 |& sleep 1 && kill $!; sleep 1; sleep 3 |& sleep 1 && kill $! ) ||
        log_error "$cat coprocess cleanup not working correctly"
    { : |& } 2>/dev/null ||
        log_error "subshell $cat coprocess lingers in parent"
    wait $!

    unset N r e
    integer N=5
    e=12345
    (
        integer i
        for ((i = 1; i <= N; i++))
        do
            print $i |&
            read -p r
            print -n $r
            wait $!
        done
        print
    ) 2>/dev/null | read -t 10 r
    [[ $r == $e ]] || log_error "$cat coprocess timing bug -- expected $e, got '$r'"

    r=
    (
        integer i
        for ((i = 1; i <= N; i++))
        do
            print $i |&
            sleep 0.01
            r=$r$($cat <&p)
            wait $!
        done
        print $r
    ) 2>/dev/null | read -t 10 r
    [[ $r == $e ]] || log_error "$cat coprocess command substitution bug -- expected $e, got '$r'"

    (
        $cat |&
        sleep 0.01
        exec 6>&p
        print -u6 ok
        exec 6>&-
        sleep 2
        kill $! 2> /dev/null
    ) && log_error "$cat coprocess with subshell would hang"
    for sig in USR1 TERM
    do
    if ( trap - $sig ) 2> /dev/null
        then
    if [[ $( { sig=$sig $SHELL  2> /dev/null <<- ++EOF++
			$cat |&
			pid=\$!
			trap "print TRAP" \$sig
			(
				sleep 2
				kill -\$sig \$\$
				sleep 2
				kill -\$sig \$\$
				kill \$pid
				sleep 2
				kill \$\$
			) &
			while    read -p || ((\$? > 256))
			do
			:
			done
		++EOF++
                } ) != $'TRAP\nTRAP' ]] 2> /dev/null
            then
                log_error "traps when reading from $cat coprocess not working"
            fi

            break
        fi

    done

    trap 'sleep_pid=; kill $pid; log_error "$cat coprocess 1 hung"' TERM
    { sleep 5; kill $$; } &
    sleep_pid=$!
    $cat |&
    pid=$!
    exec 5<&p 6>&p
    exp=hi
    print -u6 $exp; read -u5
    [[ $REPLY == "$exp" ]] || log_error "REPLY from $cat coprocess failed -- expected '$exp', got '$REPLY'"
    exec 6>&-
    wait $pid
    trap - TERM
    [[ $sleep_pid ]] && kill $sleep_pid

    trap 'sleep_pid=; kill $pid; log_error "$cat coprocess 2 hung"' TERM
    { sleep 5; kill $$; } &
    sleep_pid=$!
    $cat |&
    pid=$!
    print foo >&p 2> /dev/null || log_error "first write of foo to $cat coprocess failed"
    print foo >&p 2> /dev/null || log_error "second write of foo to $cat coprocess failed"
    exec 3>&p 3>&-
    wait $pid 2> /dev/null
    trap - TERM
    [[ $sleep_pid ]] && kill $sleep_pid

    trap 'sleep_pid=; kill $pid; log_error "$cat coprocess 3 hung"' TERM
    { sleep 5; kill $$; } &
    sleep_pid=$!
    $cat |&
    pid=$!
    print -p foo
    print -p bar
    read <&p || log_error "first read from $cat coprocess failed"
    [[ $REPLY == foo ]] || log_error "first REPLY from $cat coprocess is $REPLY not foo"
    read <&p || log_error "second read from $cat coprocess failed"
    [[ $REPLY == bar ]] || log_error "second REPLY from $cat coprocess is $REPLY not bar"
    kill $pid
    wait $pid 2> /dev/null
    trap - TERM
    [[ $sleep_pid ]] && kill $sleep_pid
done

exp=ksh
got=$(print -r $'#00315
COATTRIBUTES=\'label=make \'
# @(#)$Id: libcoshell (AT&T Research) 2008-04-28 $
_COSHELL_msgfd=5
{ { (eval \'function fun { trap \":\" 0; return 1; }; trap \"exit 0\" 0; fun; exit 1\') && PATH= print -u$_COSHELL_msgfd ksh; } || { times && echo bsh >&$_COSHELL_msgfd; } || { echo osh >&$_COSHELL_msgfd; }; } >/dev/null 2>&1' | $SHELL 5>&1)
[[ $got == $exp ]] || log_error "coshell(3) identification sequence failed -- expected '$exp', got '$got'"

function cop
{
    read
    print ok
}

exp=ok

cop |&
pid=$!
if print -p yo 2>/dev/null
then
    read -p got
else
    got='no coprocess'
fi

[[ $got == $exp ]] || log_error "main coprocess main query failed -- expected $exp, got '$got'"
kill $pid 2>/dev/null
wait

cop |&
pid=$!
(
if print -p yo 2>/dev/null
then
    read -p got
else
    got='no coprocess'
fi

[[ $got == $exp ]] || log_error "main coprocess subshell query failed -- expected $exp, got '$got'"
)
kill $pid 2>/dev/null
wait

exp='no coprocess'

(
cop |&
print $! > $TEST_DIR/pid
)
pid=$(<$TEST_DIR/pid)
if print -p yo 2>/dev/null
then
    read -p got
else
    got=$exp
fi

[[ $got == $exp ]] || log_error "subshell coprocess main query failed -- expected $exp, got '$got'"
kill $pid 2>/dev/null
wait

(
cop |&
print $! > $TEST_DIR/pid
)
pid=$(<$TEST_DIR/pid)
(
if print -p yo 2>/dev/null
then
    read -p got
else
    got=$exp
fi

[[ $got == $exp ]] || log_error "subshell coprocess subshell query failed -- expected $exp, got '$got'"
kill $pid 2>/dev/null
wait
)

function mypipe
{
    read; read
    print -r -- "$REPLY"
}

mypipe |&
print -p "hello"
z="$( $bin_true $($bin_true) )"
{ print -p "world";} 2> /dev/null
read -p
[[ $REPLY == world ]] ||  log_error "expected 'world' got '$REPLY'"
kill $pid 2>/dev/null
wait

function cop
{
    read
    print ok
}
exp=ok
cop |&
pid=$!
(
if   print -p yo 2>/dev/null
then
    read -p got
else
    got='no coprocess'
fi

[[ $got == $exp ]] || log_error "main coprocess subshell query failed -- expected $exp, got '$got'"
)
kill $pid 2>/dev/null
wait

ls -l |&
pid=$!
$bin_tee -a /dev/null <&p > /dev/null
wait $pid
x=$?
[[ $x == 0 ]] || log_error "coprocess exitval should be 0, not $x"
