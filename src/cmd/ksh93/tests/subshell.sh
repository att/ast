########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2014 AT&T Intellectual Property          #
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

# Some platforms have an extremely large default value for the number of open files. For example,
# FreeBSD 11.1 has a default of 44685. This test fails with such a large value. Every other platform
# has a limit an order of magnitude smaller. So limit the allowable values to something more
# reasonable.
#
# TBD is why such a large default limit causes the test to fail.
ulimit -n 512

z=()
z.foo=( [one]=hello [two]=(x=3 y=4) [three]=hi)
z.bar[0]=hello
z.bar[2]=world
z.bar[1]=(x=4 y=5)
val='(
	typeset -a bar=(
		[0]=hello
		[2]=world
		[1]=(
			x=4
			y=5
		)
	)
	typeset -A foo=(
		[one]=hello
		[three]=hi
		[two]=(
			x=3
			y=4
		)
	)
)'

[[ $z == "$val" ]] || log_error 'compound variable with mixed arrays not working'
z.bar[1]=yesyes
[[ ${z.bar[1]} == yesyes ]] || log_error 'reassign of index array compound variable fails'
z.bar[1]=(x=12 y=5)
[[ ${z.bar[1]} == $'(\n\tx=12\n\ty=5\n)' ]] || log_error 'reassign array simple to compound variable fails'
eval val="$z"
(
    z.foo[three]=good
    [[ ${z.foo[three]} == good ]] || log_error 'associative array assignment in subshell not working'
)
[[ $z == "$val" ]] || log_error 'compound variable changes after associative array assignment'
eval val="$z"
(
    z.foo[two]=ok
    [[ ${z.foo[two]} == ok ]] || log_error 'associative array assignment to compound variable in subshell not working'
    z.bar[1]=yes
    [[ ${z.bar[1]} == yes ]] || log_error 'index array assignment to compound variable in subshell not working'
)
[[ $z == "$val" ]] || log_error 'compound variable changes after associative array assignment'

x=(
    foo=( qqq=abc rrr=def)
    bar=( zzz=no rst=fed)
)
eval val="$x"
(
    unset x.foo
    [[ ${x.foo.qqq} ]] && log_error 'x.foo.qqq should be unset'
    x.foo=good
    [[ ${x.foo} == good ]] || log_error 'x.foo should be good'
)
[[ $x == "$val" ]] || log_error 'compound variable changes after unset leaves'
unset l
(
    l=( a=1 b="BE" )
)
[[ ${l+foo} != foo ]] || log_error 'l should be unset'

TEST_notfound=notfound
while whence $TEST_notfound >/dev/null 2>&1
do
    TEST_notfound=notfound-$RANDOM
done


integer BS=1024 nb=64 ss=60 bs no
for bs in $BS 1
do
    $SHELL -c '
        {
            sleep '$ss'
            kill -KILL $$
        } &
        set -- $(printf %.'$(($BS*$nb))'c x | dd bs='$bs')
        print ${#1}
        kill $!
    ' > $TEST_DIR/sub 2>/dev/null
    no=$(<$TEST_DIR/sub)
    (( no == (BS * nb) )) || log_error "shell hangs on command substitution output size >= $BS*$nb with write size $bs -- expected $((BS*nb)), got ${no:-0}"
done

# This time with redirection on the trailing command
for bs in $BS 1
do
    $SHELL -c '
        {
            sleep 2
            sleep '$ss'
            kill -KILL $$
        } &
        set -- $(printf %.'$(($BS*$nb))'c x | dd bs='$bs' 2>/dev/null)
        print ${#1}
        kill $!
    ' > $TEST_DIR/sub 2>/dev/null
    no=$(<$TEST_DIR/sub)
    (( no == (BS * nb) )) || log_error "shell hangs on command substitution output size >= $BS*$nb with write size $bs and trailing redirection -- expected $((BS*nb)), got ${no:-0}"
done

# exercise command substitutuion trailing newline logic w.r.t. pipe vs. tmp file io

set -- \
    'post-line print'                                \
    '$TEST_unset; ($TEST_fork; print 1); print'                    \
    1                                        \
    'pre-line print'                                \
    '$TEST_unset; ($TEST_fork; print); print 1'                    \
    $'\n1'                                        \
    'multiple pre-line print'                            \
    '$TEST_unset; ($TEST_fork; print); print; ($TEST_fork; print 1); print'        \
    $'\n\n1'                                    \
    'multiple post-line print'                            \
    '$TEST_unset; ($TEST_fork; print 1); print; ($TEST_fork; print); print'        \
    1                                        \
    'intermediate print'                                \
    '$TEST_unset; ($TEST_fork; print 1); print; ($TEST_fork; print 2); print'    \
    $'1\n\n2'                                    \
    'simple variable'                                \
    '$TEST_unset; ($TEST_fork; l=2; print "$l"); print $l'                \
    2                                        \
    'compound variable'                                \
    '$TEST_unset; ($TEST_fork; l=(a=2 b="BE"); print "$l"); print $l'        \
    $'(\n\ta=2\n\tb=BE\n)'                                \

export TEST_fork TEST_unset

while (( $# >= 3 ))
do
    txt=$1
    cmd=$2
    exp=$3
    shift 3
    for TEST_unset in '' 'unset var'
    do
        for TEST_fork in '' 'ulimit -c 0'
        do
            for TEST_shell in "eval" "$SHELL -c"
            do
                if ! got=$($TEST_shell "$cmd")
                then
                    log_error "${TEST_shell/*-c/\$SHELL -c} ${TEST_unset:+unset }${TEST_fork:+fork }$txt print failed"
                elif [[ "$got" != "$exp" ]]
                then
                    EXP=$(printf %q "$exp")
                    GOT=$(printf %q "$got")
                    log_error "${TEST_shell/*-c/\$SHELL -c} ${TEST_unset:+unset }${TEST_fork:+fork }$txt command substitution failed -- expected $EXP, got $GOT"
                fi
            done
        done
    done
done

r=$( ($SHELL -c '
    {
        sleep 32
        kill -KILL $$
    } &
    for v in $(set | sed "s/=.*//")
    do
        command unset $v
    done
    typeset -Z5 I
    for ((I = 0; I < 1024; I++))
    do
        eval A$I=1234567890
    done
    a=$(set 2>&1)
    print ok
    kill -KILL $!
') 2>/dev/null)
[[ $r == ok ]] || log_error "large subshell command substitution hangs"

for TEST_command in '' $TEST_notfound
do
    for TEST_exec in '' 'exec'
    do
        for TEST_fork in '' 'ulimit -c 0;'
        do
        for TEST_redirect in '' '>/dev/null'
            do
                for TEST_substitute in '' ': $'
                do

                     TEST_test="$TEST_substitute($TEST_fork $TEST_exec $TEST_command $TEST_redirect 2>/dev/null)"
                    [[ $TEST_test == '('*([[:space:]])')' ]] && continue
                    r=$($SHELL -c '
                        {
                            sleep 2
                            kill -KILL $$
                        } &
                        '"$TEST_test"'
                        kill $!
                        print ok
                        ')
                    [[ $r == ok ]] || log_error "shell hangs on $TEST_test"
                done
            done
        done
    done
done

$SHELL -c '( autoload xxxxx);print -n' ||  log_error 'autoloaded functions in subshells can cause failure'
actual=$($SHELL  <<- ++EOF++
	(trap 'print bar' EXIT;print -n foo)
	++EOF++
)
expect=foobar
[[ $actual == $expect ]] ||
    log_error 'trap on exit when last commands is subshell is not triggered' "$expect" "$actual"

actual=$(
    $SHELL  2>&1  <<- \EOF
		function foo
		{
			x=$( $bin_date > /dev/null 2>&1 ;:)
		}
		# consume almost all fds to push the test to the fd limit #
		integer max=$(ulimit --nofile)
		(( max -= 10 ))
		for ((i=20; i < max; i++))
		do
			exec {i}>&1
                done
		for ((i=0; i < 20; i++))
                do
			y=$(foo)
		done
	EOF
) || {
    expect=''
    log_error "nested command substitution with redirections failed" "$expect" "$actual"
}

exp=0
$SHELL -c $'
    function foobar
    {
        print "hello world"
    }
    [[ $(getopts \'[+?X\ffoobar\fX]\' v --man 2>&1) == *"Xhello worldX"* ]]
    exit '$exp$'
'
got=$?
[[ $got == $exp ]] || log_error "getopts --man runtime callout with nonzero exit terminates shell -- expected '$exp', got '$got'"
exp=ok
got=$($SHELL -c $'
    function foobar
    {
        print "hello world"
    }
    [[ $(getopts \'[+?X\ffoobar\fX]\' v --man 2>&1) == *"Xhello worldX"* ]]
    print '$exp$'
')
[[ $got == $exp ]] || log_error "getopts --man runtime callout with nonzero exit terminates shell -- expected '$exp', got '$got'"

# command substitution variations #
set -- \
    '$('            ')'        \
    '${ '            '; }'        \
    '$(ulimit -c 0; '    ')'        \
    '$( ('            ') )'        \
    '${ ('            '); }'        \
    '`'            '`'        \
    '`('            ')`'        \
    '`ulimit -c 0; '    '`'        \
    # end of table #
exp=ok
testcase[1]='
    if %sexpr "NOMATCH" : ".*Z" >/dev/null%s
    then
        print error
    else
        print ok
    fi

    exit %s
'
testcase[2]='
    function bar
    {
        pipeout=%1$sprintf Ok | tr O o%2$s
        print $pipeout
        return 0
    }
    foo=%1$sbar%2$s || foo="exit status $?"
    print $foo
    exit %3$s
'

while (( $# >= 2 ))
do
    for ((TEST=1; TEST<=${#testcase[@]}; TEST++))
    do
        body=${testcase[TEST]}
        for code in 0 2
        do
            got=${ printf "$body" "$1" "$2" "$code" | $SHELL 2>&1 }
            status=$?
            if (( status != code ))
            then
                log_error "test $TEST '$1...$2 exit $code' failed -- exit status $status, expected $code"
            elif [[ $got != $exp ]]
            then
                log_error "test $TEST '$1...$2 exit $code' failed -- got '$got', expected '$exp'"
            fi
        done
    done
    shift 2
done

# the next tests loop on all combinations of
#    { SUB CAT INS TST APP } X { file-sizes }
# where the file size starts at 1Ki and doubles up to and including 1Mi
#
# the tests and timeouts are done in async subshells to prevent
# the test harness from hanging

SUB=(
    ( BEG='$( '    END=' )'    )
    ( BEG='${ '    END='; }'    )
)
CAT=(  cat  $bin_cat  )
INS=(  ""  "builtin cat; "  "builtin -d cat $bin_cat; "  ": > /dev/null; "  )
APP=(  ""  "; :"  )
TST=(
    ( CMD='print foo | $cat'            EXP=3        )
    ( CMD='$cat < $TEST_DIR/lin'                        )
    ( CMD='cat $TEST_DIR/lin | $cat'                    )
    ( CMD='read v < $TEST_DIR/buf; print $v'        LIM=4*1024    )
    ( CMD='cat $TEST_DIR/buf | read v; print $v'        LIM=4*1024    )
)

# Prime the two data files to 512 bytes each
# $TEST_DIR/lin has newlines every 16 bytes and $TEST_DIR/buf has no newlines
# the outer loop doubles the file size at top

buf=$'1234567890abcdef'
lin=$'\n1234567890abcde'
for ((i=0; i<5; i++))
do
    buf=$buf$buf
    lin=$lin$lin
done

print -n "$buf" > $TEST_DIR/buf
print -n "$lin" > $TEST_DIR/lin

unset SKIP
for ((n=1024; n<=1024*1024; n*=2))
do
    cat $TEST_DIR/buf $TEST_DIR/buf > $TEST_DIR/tmp
    mv $TEST_DIR/tmp $TEST_DIR/buf
    cat $TEST_DIR/lin $TEST_DIR/lin > $TEST_DIR/tmp
    mv $TEST_DIR/tmp $TEST_DIR/lin
    for ((S=0; S<${#SUB[@]}; S++))
    do
        for ((C=0; C<${#CAT[@]}; C++))
        do
            cat=${CAT[C]}
            for ((I=0; I<${#INS[@]}; I++))
            do
            for ((A=0; A<${#APP[@]}; A++))
                do
                for ((T=0; T<${#TST[@]}; T++))
                    do
                    #undent...#
                    if [[ ! ${SKIP[S][C][I][A][T]} ]]
                    then
                        eval "{ x=${SUB[S].BEG}${INS[I]}${TST[T].CMD}${APP[A]}${SUB[S].END}; print \${#x}; } >\$TEST_DIR/out &"
                        m=$!
                        { sleep 4; kill -9 $m; } &
                        k=$!
                        wait $m
                        h=$?
                        kill -9 $k
                        # Suppress messages like this from the test log:
                        # /tmp/ksh.subshell.7fgw2W8/subshell.sh[475]: wait: 8737: Killed
                        wait $k 2> /dev/null
                        got=$(<$TEST_DIR/out)
                        if [[ ! $got ]] && (( h ))
                        then
                            got=HUNG
                        fi

                        if [[ ${TST[T].EXP} ]]
                        then
                            exp=${TST[T].EXP}
                        else
                            exp=$n
                        fi

                        if [[ $got != $exp ]]
                        then
                            # on failure skip similar tests on larger files sizes #
                            SKIP[S][C][I][A][T]=1
                            siz=$(printf $'%#i' $exp)
                            cmd=${TST[T].CMD//\$cat/$cat}
                            cmd=${cmd//\$TEST_DIR\/buf/$siz.buf}
                            cmd=${cmd//\$TEST_DIR\/lin/$siz.lin}
                            log_error "'x=${SUB[S].BEG}${INS[I]}${cmd}${APP[A]}${SUB[S].END} && print \${#x}' failed -- expected '$exp', got '$got'"
                        elif [[ ${TST[T].EXP} ]] || (( TST[T].LIM >= n ))
                        then
                            SKIP[S][C][I][A][T]=1
                        fi

                    fi
                    #...indent#
                    done
                done
            done
        done
    done
done

# specifics -- there's more?

{
    cmd='{ exec 5>/dev/null; print "$(eval ls -d . 2>&1 1>&5)"; } >$TEST_DIR/out &'
    eval $cmd
    m=$!
    { sleep 4; kill -9 $m; } &
    k=$!
    wait $m
    h=$?
    kill -9 $k
    wait $k
    got=$(<$TEST_DIR/out)
} 2>/dev/null
exp=''
if [[ ! $got ]] && (( h ))
then
    got=HUNG
fi

if [[ $got != $exp ]]
then
    log_error "eval '$cmd' failed -- expected '$exp', got '$got'"
fi

float t1=$SECONDS
  $SHELL -c "( $bin_sleep 5 </dev/null >/dev/null 2>&1 & ); exit 0" | cat
  (( (SECONDS-t1) > 4 )) && log_error '/bin/sleep& in subshell hanging'
  ((t1=SECONDS))

$SHELL -c '( sleep 5 </dev/null >/dev/null 2>&1 & );exit 0' | cat
(( (SECONDS-t1) > 4 )) && log_error 'sleep& in subshell hanging'

exp=HOME=$HOME
( HOME=/bin/sh )
got=$(env | grep ^HOME=)
[[ $got == "$exp" ]] ||  log_error "( HOME=/bin/sh ) cleanup failed -- expected '$exp', got '$got'"

cmd='echo $((case x in x)echo ok;esac);:)'
exp=ok
got=$($SHELL -c "$cmd" 2>&1)
[[ $got == "$exp" ]] ||  log_error "'$cmd' failed -- expected '$exp', got '$got'"

cmd='eval "for i in 1 2; do eval /bin/echo x; done"'
exp=$'x\nx'
got=$($SHELL -c "$cmd")
if [[ $got != "$exp" ]]
then
    EXP=$(printf %q "$exp")
    GOT=$(printf %q "$got")
    log_error "'$cmd' failed -- expected $EXP, got $GOT"
fi

(
$SHELL -c 'sleep 20 & pid=$!; { x=$( ( seq 60000 ) );kill -9 $pid;}&;wait $pid'
) 2> /dev/null
(( $? )) ||  log_error 'nested command substitution with large output hangs'

(.sh.foo=foobar)
[[ ${.sh.foo} == foobar ]] && log_error '.sh subvariables in subshells remain set'
[[ $($SHELL -c 'print 1 | : "$(/bin/cat <(/bin/cat))"') ]] && log_error 'process substitution not working correctly in subshells'

# config hang bug
integer i
for ((i=1; i < 1000; i++))
do
    typeset foo$i=$i
done
{
    : $( (ac_space=' '; set | grep ac_space) 2>&1)
} < /dev/null | cat > /dev/null &
sleep  1.5
if kill -KILL $! 2> /dev/null
then
    log_error 'process timed out with hung comsub'
fi

wait $! 2> /dev/null
(( $? > 128 )) && log_error 'incorrect exit status with comsub'

$SHELL 2> /dev/null -c '[[ ${ print foo },${ print bar } == foo,bar ]]' || log_error  '${ print foo },${ print bar } not working'
$SHELL 2> /dev/null -c '[[ ${ print foo; },${ print bar } == foo,bar ]]' || log_error  '${ print foo; },${ print bar } not working'

src=$'true 2>&1\n: $(true | true)\n: $(true | true)\n: $(true | true)\n'$bin_true
exp=ok
got=$( $SHELL -c "(eval '$src'); echo $exp" )
[[ $got == "$exp" ]] || log_error 'subshell eval of pipeline clobbers stdout'

x=$( { time $SHELL -c date >| /dev/null;} 2>&1)
[[ $x == *real*user*sys* ]] || log_error 'time { ...;} 2>&1 in $(...) fails'

x=$($SHELL -c '( function fx { export X=123;  } ; fx; ); echo $X')
[[ $x == 123 ]] && log_error 'global variables set from with functions inside a
subshell can leave side effects in parent shell'

err() { return $1; }
( err 12 ) & pid=$!
: $($bin_date)
wait $pid
actual=$?
expect=12
[[ $actual -eq $expect ]] ||
    log_error 'exit status from subshells not being preserved' "$expect" "$actual"

actual="$(sed 's/^/Hello /' <(print "Fred" | sort))"
expect="Hello Fred"
[[ $actual == $expect ]] ||
    log_error  "process subst of pipeline in cmd subst not working" "$expect" "$actual"

{
$SHELL <<- \EOF
	function foo
	{
		integer i
		print -u2 foobar
		for    ((i=0; i < 8000; i++))
		do
			print abcdefghijk
		done
		print -u2 done
	}
	out=$(eval "foo | cat" 2>&1)
	(( ${#out} == 96011 )) || log_error "\${#out} is ${#out} should be 96011"
EOF
} & pid=$!
$SHELL -c "{ sleep 4 && kill $pid ;}" 2> /dev/null
(( $? == 0 )) &&  log_error 'process has hung'

{
x=$( $SHELL  <<- \EOF
	function func1 { typeset IFS; : $(func2); print END ;}
	function func2 { IFS="BAR"; }
	func1
	func1
EOF
)
} 2> /dev/null
[[ $x == $'END\nEND' ]] || log_error 'bug in save/restore of IFS in subshell'

tmpf=$TEST_DIR/foo
function fun1
{
    $bin_true
    cd - >/dev/null 2>&1
    print -u2 -- "$($bin_date) SUCCESS"
}

print -n $(fun1 2> $tmpf)
[[  $(< $tmpf) == *SUCCESS ]] || log_error 'standard error output lost with command substitution'


cat > foo <<-\EOF
	$SHELL -c 'function g { IFS= ;};function f { typeset IFS;(g);: $V;};f;f'
	EOF
$SHELL 2> /dev/null foo || log_error 'IFS in subshell causes core dump'

unset i
if   [[ -d /dev/fd ]]
then
        integer i
        for ((i=11; i < 29; i++))
        do
            if ! [[ -r /dev/fd/$i  || -w /dev/fd/$i ]]
            then
                a=$($SHELL -c "[[ -r /dev/fd/$i || -w /dev/fd/$i ]]")
                (( $? )) || log_error "file descriptor $i not close on exec"
            fi
        done
fi

trap USR1 USR1
trap ERR ERR
[[ $(trap -p USR1) == USR1 ]] || log_error 'trap -p USR1 in subshell not working'
[[ $(trap -p ERR) == ERR ]] || log_error 'trap -p ERR in subshell not working'
[[ $(trap -p) == *USR* ]] || log_error 'trap -p in subshell does not contain USR'
[[ $(trap -p) == *ERR* ]] || log_error 'trap -p in subshell does not contain ERR'
trap - USR1 ERR

( PATH=/bin:/usr/bin
dot=$(cat <<-EOF
	$(ls -d .)
	EOF
) ) & sleep 1

if kill -0 $! 2> /dev/null
then
    log_error  'command substitution containg here-doc with command substitution fails'
fi

[[ $( { trap "echo foobar" EXIT; ( $bin_printf ""); } & wait) == foobar ]] || \
    log_error  'exit trap not being invoked'

$SHELL 2> /dev/null -c '( PATH=/bin; set -o restricted) ; exit 0'  || \
    log_error 'restoring PATH when a subshell enables restricted exits not working'

$SHELL <<- \EOF
	print > /dev/null  $( ( head -c 1 /dev/zero | (wc -c) 3>&1 ) 3>&1) &
	pid=$!
	sleep 2
	kill -9 $! 2> /dev/null && log_error '/dev/zero in command substitution hangs'
	wait $!
EOF

for f in /dev/stdout /dev/fd/1
do
    if [[ -e $f ]]
    then
        $SHELL -c "x=\$(command -p tee $f </dev/null 2>/dev/null)" || log_error "$f in command substitution fails"
    fi
done

$SHELL > /dev/null -c 'echo $(for x in whatever; do case y in *) true;; esac; done)' || log_error 'syntax error with case in command substitution'

$SHELL 2> /dev/null <<- \EOF || log_error 'cannot run 100000 subshells'
	( for ((i=0; i < 100000; i++))
	do
		(b=$(printf %08d ${i}))
	done )
EOF

print 'print OK'  | out=$(${SHELL})
[[ $out == OK ]] || log_error '$() command substitution not waiting for process completion'

print 'print OK' | out=$( ${SHELL} 2>&1 )
out2="${out}$?"
[[ "$out2" == 'OK0' ]]  ||  log_error -u2 "expected OK0 got $out2"

fun()
{
    foo=` $bin_echo foo`
    print -n stdout=$foo
    print -u2 stderr=$foo
}
[[ `fun 2>&1` == 'stdout=foostderr=foo' ]] || log_error 'nested command substitution with 2>&1 not working'

mkdir $TEST_DIR/bin$$
print 'print foo' > $TEST_DIR/bin$$/foo
chmod +x  $TEST_DIR/bin$$/foo
: $(type foo 2> type.foo.out)
actual=$(< type.foo.out)
actual=${actual#*: }  # remove the script name and line number prefix
expect='whence: foo: not found'
[[ $actual == $expect ]] || log_error "command foo should not have been found" "$expect" "$actual"
: ${ PATH=$TEST_DIR/bin$$:$PATH;}
actual=$(whence foo 2> /dev/null)
expect="$TEST_DIR/bin$$/foo"
[[ $actual == $expect ]] ||
    log_error '${...PATH=...} does not preserve PATH bindings' "$expect" "$actual"

> $TEST_DIR/log
function A
{
    trap 'print TRAP A >> $TEST_DIR/log' EXIT
    print >&2
}
function B
{
    trap 'print TRAP B >> $TEST_DIR/log' EXIT
    A
}
x=${ ( B ) ; }
[[ $(<$TEST_DIR/log) ==  *'TRAP A'*'TRAP B'* ]] || log_error 'trap A and trap B not both executed'

function foo
{
    .sh.value=bam
}
val=${ foo;}
[[ $val ]] && log_error "function foo generates $val but should generate the empty string in command substitution"

x=$(
    for i in a b c
    do
    read A
        print -n "$A"
        STDERR=$(</dev/null)
    done <<< $'y\ny\ny\n'
)
[[ $x == yyy ]] || log_error '$(</dev/null) in a subshell causes failure'


$SHELL -c 'while((SECONDS<3)); do test -z `/bin/false | /bin/false | /bin/doesnotexist`;done;:' 2> /dev/null || log_error 'non-existant last command in pipeline causes `` to fail'

x=$({ sleep .1;false;} | true)
[[ $? != 0 ]] && log_error 'without pipefail, non-zero exit in pipeline causes command substitution to fail'

foo() {
  print -r foo | read
  return 1
}
o1=$(foo "foo") && log_error 'function which fails inside commad substitution should return non-zero exit status for assignments'

# test for larg `` command substitutions
tmpscr=$TEST_DIR/xxx.sh
print 'x=` print -n '"'" > $tmpscr
integer i
for ((i=0; i < 4000; i++))
do
    print xxxxxxxxxxyyyyyyyyyyzzzzzzzzzzaaaaaaaaaabbbbbbbbbbcccccccccc
done >>  $tmpscr
print  "'"'`' >> $tmpscr
(( size= $(wc -c < $tmpscr) -18 ))
$SHELL  "$tmpscr" &
cop=$!
{ sleep 2; kill $cop; } 2>/dev/null &
spy=$!
if   wait $cop 2>/dev/null
then
    kill $spy 2>/dev/null
else    log_error -u2 "\`...\` hangs for large with output size $size"
fi

if [[ -e /dev/zero ]]
then
    (( size = 117*1024 ))
    $SHELL -c 'x=`(dd if=/dev/zero bs=1k count=117 2>/dev/null)`' &
    cop=$!
    { sleep 2; kill $cop; } 2>/dev/null &
    spy=$!
    if wait $cop 2>/dev/null
    then
        kill $spy 2>/dev/null
    else
        log_error -u2 "\`(...)\` hangs for large with output size $size"
    fi
fi

# ========================================
# Test that variables exported in subshells don't affect the outer shell.
# Regression test for issue #7.
function proxy {
    export MYVAR="blah"
    child
    unset MYVAR
}

function child {
    echo "MYVAR=$MYVAR"
}

function test {
        child
        proxy
        child
}

actual="$(test)"
expected="\
MYVAR=
MYVAR=blah
MYVAR="
if [[ $actual != $expected ]]
then
    log_error -u2 "exported vars in subshells not confined to the subshell: $actual"
fi


# ========================================
# Test that closing file descriptors don't affect capturing the output of a
# subshell. Regression test for issue #198.
expected='return value'

function get_value {
    case=$1
    (( case >= 1 )) && exec 3< foo
    (( case >= 2 )) && exec 4< foo
    (( case >= 3 )) && exec 6< foo

    # To trigger the bug we have to spawn an external command. Why is a
    # mystery but not really relevant.
    $bin_true

    (( case >= 1 )) && exec 3<&-
    (( case >= 2 )) && exec 4<&-
    (( case >= 3 )) && exec 6<&-

    print $expected
}

actual=$(get_value 0)
if [[ $actual != $expected ]]
then
    log_error -u2 "failed to capture subshell output when closing fd: case 0"
fi

actual=$(get_value 1)
if [[ $actual != $expected ]]
then
    log_error -u2 "failed to capture subshell output when closing fd: case 1"
fi

actual=$(get_value 2)
if [[ $actual != $expected ]]
then
    log_error -u2 "failed to capture subshell output when closing fd: case 2"
fi

actual=$(get_value 3)
if [[ $actual != $expected ]]
then
    log_error -u2 "failed to capture subshell output when closing fd: case 3"
fi

builtin -d echo
# Check if redirections work if backticks are nested inside $()
foo=$(print `echo bar`)
[[ $foo == "bar" ]] || log_error 'Redirections do not work if backticks are nested inside $()'

# Buffer boundary tests
for exp in 65535 65536
do    got=$($SHELL -c 'x=$(printf "%.*c" '$exp' x); print ${#x}' 2>&1)
    [[ $got == $exp ]] || log_error "large command substitution failed" "$exp" "$got"
done
