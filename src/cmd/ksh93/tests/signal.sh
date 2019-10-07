########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2013 AT&T Intellectual Property          #
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

# ====================
# Verify enumerating signal names and converting names to numbers works.
#
unset n s t
typeset -A SIG
for s in $(kill -l)
do
    if ! n=$(kill -l $s 2>/dev/null)
    then
        log_error "'kill -l $s' failed"
    elif ! t=$(kill -l $n 2>/dev/null)
    then
        log_error "'kill -l $n' failed"
    elif [[ $s == ?(SIG)$t ]]
    then
        SIG[${s#SIG}]=1
    elif ! m=$(kill -l $t 2>/dev/null)
    then
        log_error "'kill -l $t' failed"
    elif [[ $m != $n ]]
    then
        log_error "'kill -l $s' => $n, 'kill -l $n' => $t, kill -l $t => $m -- expected $n"
    fi
done

# ====================
# Verify pipefail and trapping SIGPIPE interact properly.
#
empty_fifos
print running > out3
(
    set --pipefail
    {
        # The `head` command will terminate after reading ten lines which will cause the next
        # `print hello` to generate SIGPIPE. We want to see two SIGPIPEs delivered.
        $SHELL 2> out2 <<#'EOF'
            g=false
            trap 'print -u2 -n SIGPIPE; $g && exit 27; g=true' PIPE
            while true
            do
                print hello
                sleep 0.01
            done
            EOF
    } | head > /dev/null
    print $? > out3
) &
cop=$!

{ read -u9 -t5 x; [[ $x == okay ]] || kill -s TERM $cop; } &
watchdog=$!

if wait $cop
then
    print -u9 okay  # cleanly terminate watchdog process
    sleep 0.01      # give it a chance to exit
else
    log_error "pipe with --pipefail PIPE trap hangs"
fi
kill $watchdog 2> /dev/null  # should already be dead but just in case
wait  # reap watchdog process

expect=27
actual="$(< out3)"
[[ $actual == $expect ]] || log_error "SIGPIPE with wrong error code" "$expect" "$actual"

expect='SIGPIPESIGPIPE'
actual="$(< out2)"
[[ $actual == $expect ]] ||
    log_error "SIGPIPE output on standard error is not correct" "$expect" "$actual"

# ====================
# Verify SIGCHLD can be trapped.
#
empty_fifos
expect='01sigchld23'
actual=$(
    sigchld=no
    trap 'print -n sigchld; sigchld=yes' SIGCHLD
    { read -u9 x; print -u8 go; } &
    for ((i=0; i < 4; i++))
    do
        print -n $i
        [[ $i -eq 1 ]] && { print -u9 die; read -u8 x; while [[ $sigchld == no ]]; do :; done; }
    done
)
[[ $actual == $expect ]] || log_error 'SIGCHLD not working:' "$expect" "$actual"

# ====================
# Verify SIGINT trapping works for various complicated cases of nested scripts.
#
empty_fifos
typeset -A expected
expected[...]="1-main 2-main 3-main SIGINT 3-intr"
expected[..d]="1-main 2-main 3-main SIGINT 3-intr"
expected[.t.]="1-main 2-main 3-main SIGINT 3-intr 2-intr 1-intr 1-0258"
expected[.td]="1-main 2-main 3-main SIGINT 3-intr 2-intr 1-intr 1-0258"
expected[x..]="1-main 2-main 3-main SIGINT 3-intr 2-intr 1-0000"
expected[x.d]="1-main 2-main 3-main SIGINT 3-intr 2-intr 1-0000"
expected[xt.]="1-main 2-main 3-main SIGINT 3-intr 2-intr 1-intr 1-0000"
expected[xtd]="1-main 2-main 3-main SIGINT 3-intr 2-intr 1-intr 1-0000"

cat $TEST_ROOT/util/util.sh $TEST_ROOT/data/sigtst0 > sigtst0
cat $TEST_ROOT/util/util.sh $TEST_ROOT/data/sigtst1 > sigtst1
cat $TEST_ROOT/util/util.sh $TEST_ROOT/data/sigtst2 > sigtst2
cat $TEST_ROOT/util/util.sh $TEST_ROOT/data/sigtst3 > sigtst3
chmod +x sigtst?
$SHELL sigtst0 > sigtst.out
while read ops actual
do
    expect=${expected[$ops]}
    [[ $actual == $expect ]] || log_error "SIGINT $ops test failed" "$expect" "$actual"
done < sigtst.out

# ====================
# Verify SIGUSR1 trapping works.
#
empty_fifos

expect='done SIGUSR1'
actual=$(echo $(
    LC_ALL=C $SHELL -c 'trap "print SIGUSR1 ; exit 0" USR1;
        (trap "" USR1 ; exec kill -USR1 $$ &; print go > fifo9; sleep 0.5; print done);
        read x < fifo9;
        print should not get here'
))
[[ $actual == $expect ]] ||
    log_error "subshell ignoring signal does not send signal to parent" "$expect" "$actual"
read -u9 -t0.1 x || log_error 'subcommand unexpectedly consumed fifo go signal'

expect='done SIGUSR1'
actual=$(echo $(
    LC_ALL=C $SHELL -c 'trap "print SIGUSR1 ; exit 0" USR1;
        (trap "exit" USR1 ; exec kill -USR1 $$ &; print go > fifo9; sleep 0.5; print done);
        read x < fifo9;
        print should not get here'
))
[[ $actual == $expect ]] ||
    log_error "subshell catching signal does not send signal to parent" "$expect" "$actual"
read -u9 -t0.1 x || log_error 'subcommand unexpectedly consumed fifo go signal'

# ====================
# Verify exit due to signal can be mapped to the correct signal name.
#
empty_fifos
for expect in TERM VTALRM PIPE
do
    [[ ${SIG[$expect]} ]] || continue

    $SHELL <<#EOF
        foo() { return 0; }
        trap foo EXIT
        { sleep 0.5; kill -$expect \$\$; sleep 1; kill -KILL \$\$ 2> /dev/null; } &
        while :
        do
            ($bin_true; sleep .1)
        done
        EOF
    status=$?
    actual=$(kill -l $status)
    [[ $actual == $expect ]] ||
        log_error "kill -$expect failed to force exit via expected signal number" "$expect" "$actual"
done

# ====================
# Verify the EXIT trap works correctly when the shell is killed by a signal.
#
SECONDS=0
expect=11
$SHELL -c "trap 'print okay; exit $expect' EXIT
    (sleep 0.1; kill \$\$) &
    sleep 2  # this can be a long sleep because we expect it to be interrupted
    print bad" > sig
actual=$?
[[ $actual == $expect ]] || log_error "exit status failed" "$expect" "$actual"
expect=okay
actual=$(< sig)
[[ $actual == $expect ]] || log_error "output failed" "$expect" "$actual"
(( SECONDS > 1 )) && log_error "took $SECONDS seconds, expected around 0.5"

SECONDS=0
expect=13
$SHELL -c "trap 'print okay; exit $expect' EXIT
    (sleep 0.1; kill \$\$) &
    (sleep 2)  # this can be a long sleep because we expect it to be interrupted
    print bad" > sig
actual=$?
[[ $actual == $expect ]] || log_error "exit status failed" "$expect" "$actual"
expect=okay
actual=$(< sig)
[[ $actual == $expect ]] || log_error "output failed" "$expect" "$actual"
(( SECONDS > 1 )) && log_error "took $SECONDS seconds, expected around 0.5"

SECONDS=0
expect=15
{ $SHELL -c "trap 'print okay; exit $expect' EXIT
    (sleep 0.1; kill \$\$) &
    sleep 2  # this can be a long sleep because we expect it to be interrupted
    print bad" > sig
}
actual=$?
[[ $actual == $expect ]] || log_error "exit status failed" "$expect" "$actual"
expect=okay
actual=$(< sig)
[[ $actual == $expect ]] || log_error "output failed" "$expect" "$actual"
(( SECONDS > 1 )) && log_error "took $SECONDS seconds, expected around 0.5"

SECONDS=0
expect=17
{ $SHELL -c "trap 'print okay; exit $expect' EXIT
    (sleep 0.1; kill \$\$) &
    (sleep 2)  # this can be a long sleep because we expect it to be interrupted
    print bad" > sig
}
actual=$?
[[ $actual == $expect ]] || log_error "exit status failed" "$expect" "$actual"
expect=okay
actual=$(< sig)
[[ $actual == $expect ]] || log_error "output failed" "$expect" "$actual"
(( SECONDS > 1 )) && log_error "took $SECONDS seconds, expected around 0.5"

SECONDS=0
expect=19
output=$($SHELL -c "trap 'print okay; exit $expect' EXIT
    (sleep 0.1; kill \$\$) &
    sleep 2  # this can be a long sleep because we expect it to be interrupted
    print bad"
)
actual=$?
[[ $actual == $expect ]] || log_error "exit status failed" "$expect" "$actual"
expect=okay
actual="$output"
[[ $actual == $expect ]] || log_error "output failed" "$expect" "$actual"
(( SECONDS > 1 )) && log_error "took $SECONDS seconds, expected around 0.5"

SECONDS=0
expect=21
output=$($SHELL -c "trap 'print okay; exit $expect' EXIT
    (sleep 0.1; kill \$\$) &
    (sleep 2)  # this can be a long sleep because we expect it to be interrupted
    print bad"
)
actual=$?
[[ $actual == $expect ]] || log_error "exit status failed" "$expect" "$actual"
expect=okay
actual="$output"
[[ $actual == $expect ]] || log_error "output failed" "$expect" "$actual"
(( SECONDS > 1 )) && log_error "took $SECONDS seconds, expected around 0.5"

# ====================
# Verify an ignored signal can't be trapped in a subshell.
trap '' SIGUSR2
expect=''
actual=$($SHELL -c 'trap date SIGUSR2; trap -p SIGUSR2')
[[ $actual == $expect ]] || log_error 'SIGUSR2 should not have a trap' "$expect" "$actual"
trap -- - SIGUSR2

{
    output=$($SHELL <<'EOF'
        timeout() {
            trap 'trap - TERM; return' TERM
            ( sleep $1; kill -TERM $$ ) >/dev/null 2>&1 &
            sleep 3  # this can be a long sleep because we expect it to be interrupted
        }
        timeout 0.1
        print ok
EOF
    )
    actual=$?
}
expect=0
[[ $actual == $expect ]] ||
    log_error 'return without arguments in trap not preserving exit status' "$expect" "$actual"
expect=ok
actual="$output"
[[ $actual == $expect ]] ||
    log_error 'return without arguments in trap not preserving exit status' "$expect" "$actual"

# ====================
# Verify pipefail and trapping SIGPIPE doesnt' propagate the signal to the exit status.
#
actual=$($SHELL <<'EOF'
    set -o pipefail
    foobar() {
        for ((i=0; i < 10000; i++))
        do
            print abcdefghijklmnopqrstuvwxyz
        done | head > /dev/null
    }
    foobar
    print ok
EOF
)
expect=ok
[[ $actual == $expect ]] ||
    log_error 'SIGPIPE exit status causes PIPE signal to be propagated' "$expect" "$actual"

# ====================
# Verify signal ignored in subshell not propagated to parent.
#
actual=$($SHELL <<'EOF'
    trap "print GNAW" URG
    print 1
    ( sleep 0.1 ; kill -URG $$ ; sleep 0.1 ; print S1 ; )
    print 2
EOF
)
actual=$(print $actual)
expect='1 S1 GNAW 2'
[[ $actual == $expect ]] ||
    log_error 'signal ignored in subshell not propagated to parent' "$expect" "$actual"

# ====================
# Verify termination by a signal is correctly reflected in the exit status.
#
$SHELL <<#'EOF'
    trap : USR2
    for ((i=0; i < 3; i++))
    do
        sleep 0.1
        kill -0 $$ 2> /dev/null && kill -USR2 $$
    done &
    wait
    EOF
actual=$?
expect=$(( $(kill -l USR2) + 128 ))
[[ $actual == $expect ]] ||
    log_error 'wait interrupted by a signal should have USR2 exit status' "$expect" "$actual"

$SHELL <<#'EOF'
    for ((i = 0; i < 3; i++))
    do
        sleep 0.1
        kill -0 $$ 2> /dev/null && kill -USR2 $$
    done &
    wait
    EOF
actual=$(kill -l $?)
expect=USR2
[[ $actual == $expect ]] ||
    log_error 'wait interrupted by signal not caught should exit with the value of that signal+128' "$expect" "$actual"

# ====================
# Verify nested functions can be terminated via a signal.
#
empty_fifos
function b {
    print -u9 go  # tell the monitor we're ready for it to signal us
    sleep 1
    endb=1
}

function a {
    trap 'print function a interrupted' TERM
    b
    enda=1
}

{ read -u9 -t5 x; kill -s TERM $$; } &
unset enda endb
a
[[ $endb == 1 ]] && log_error 'TERM signal did not kill function b'
[[ $enda == 1 ]] || log_error 'TERM signal killed function a'

# ====================
# Verify ???
#
empty_fifos
actual=$($SHELL <<#'EOF'
    trap 'print foo; kill -s USR2 $$; print bar' USR1
    trap 'print USR2' USR2
    kill -s USR1 $$
EOF
)
actual=$(echo $actual)  # flatten newlines to spaces
expect='foo bar USR2'
[[ $actual == $expect ]] ||
    log_error 'trap command not blocking signals until trap command completes' "$expect" "$actual"

# ====================
# Verify ???
#
if   [[ ${SIG[RTMIN]} ]]
then
    compound -a rtar
    function rttrap {
        integer v=${.sh.sig.value}
        integer s=${#rtar[v][@]}
        integer rtnum=$1
        rtar[$v][$s]=(
            integer pid=${.sh.sig.pid}
            integer rtnum=$rtnum
            typeset msg=${v}
            )
        return 0
    }
    trap 'rttrap 0' RTMIN
    trap 'rttrap 1' RTMIN+1
    trap 'rttrap 2' RTMIN+2
    trap 'rttrap 3' RTMIN+3
    trap 'rttrap 4' RTMIN+4
    trap 'rttrap 5' RTMIN+5
    trap 'rttrap 6' RTMIN+6
    trap 'rttrap 7' RTMIN+7
    typeset m  # used in child processes
    integer pid=$$ p i numchildren=64

    # Double-fork so this shell doesn't wait for it down below where we reap all the
    # `kill -q -s RTMIN` subshells.
    ( (read -t 10 x < fifo9 && exit 0; kill $pid 2> /dev/null) & ) &

    for (( p=0; p < numchildren; p++ ))
    do
        {
            for m in 'a' 'b' 'c' 'd' 'e' 'f'
            do
                print p=$p m=$m >> junk
                kill -q $((16#$m)) -s RTMIN+6 $pid
                kill -q $((16#$m)) -s RTMIN+7 $pid
                kill -q $((16#$m)) -s RTMIN+4 $pid
                kill -q $((16#$m)) -s RTMIN+5 $pid
                kill -q $((16#$m)) -s RTMIN+2 $pid
                kill -q $((16#$m)) -s RTMIN+3 $pid
                kill -q $((16#$m)) -s RTMIN   $pid
                kill -q $((16#$m)) -s RTMIN+1 $pid
            done
        } &
    done
    while ! wait
    do
        jobs >&2  # WTF
        true
    done

    expect=6
    actual=${#rtar[@]}
    [[ $actual == $expect ]] || log_error "wrong number of signals" "$expect" "$actual"

    for (( i=0xa ; i <= 0xf; i++ ))
    do
        expect=$(( numchildren * 8 ))
        actual=${#rtar[i][*]}
        [[ $actual == $expect ]] || log_error "wrong number of $i signals" "$expect" "$actual"
    done

    SIG1=RTMIN+1
    SIG2=RTMIN+2
    compound a=(float i=0)
    trap "((a.i+=.00001)); (kill -q0 -$SIG2 $$) &; :" $SIG1
    trap '((a.i+=1))' $SIG2
    for ((j = 0; j < 200; j++))
    do
        kill -q0 -s $SIG1 $$ &
    done
    while ! wait
    do
        true
    done
    expect='typeset -C a=(typeset -l -E i=200.002)'
    actual=$(typeset -p a)
    [[ $actual == $expect ]] || log_error "signals lost" "$expect" "$actual"
    print -u9 exit  # tell the watchdog to exit since we no longer need it
fi

# ====================
# Verify ???
#
float s=SECONDS
(trap - INT; exec sleep 0.5) &
sleep .1
kill -s INT $!
wait $!
actual=$(( SECONDS - s ))
expect=0.5
(( $actual >= $expect )) ||
    log_error "'trap - INT' causing trap to not be ignored" "$expect" "$actual"

# ====================
# Verify kill -qN sends the value N and .sh.sig.value receives it.
#
compound c=(compound -a car; integer cari=0)
trap 'c.car[c.cari++]=.sh.sig' USR1
kill -q4 -s USR1 $$
kill -q5 -s USR1 $$

case "${c.car[0].code}" in
    SI_QUEUE)
        # System has sigqueue().
        expect=4
        actual=${c.car[0].value}
        (( $actual == $expect )) ||
            log_error "\${c.car[0].value} is wrong" "$expect" "$actual"

        expect=5
        actual=${c.car[1].value}
        (( $actual == $expect )) ||
            log_error "\${c.car[1].value} is wrong" "$expect" "$actual"
        ;;
    SI_USER)
        # System lacks sigqueue(), ksh called kill().
        log_info "skipping test: sigqueue() not supported"
        ;;
    *)
        log_error "expected SI_QUEUE or SI_USER" "SI_QUEUE or SI_USER" "${c.car[0].code}"
        ;;
esac
