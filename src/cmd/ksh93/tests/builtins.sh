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

# test shell builtin commands
builtin getconf
: ${foo=bar} || log_error ": failed"
[[ $foo == bar ]] || log_error ": side effects failed"
set -- - foobar
[[ $# == 2 && $1 == - && $2 == foobar ]] || log_error "set -- - foobar failed"
set -- -x foobar
[[ $# == 2 && $1 == -x && $2 == foobar ]] || log_error "set -- -x foobar failed"
getopts :x: foo || log_error "getopts :x: returns false"
[[ $foo == x && $OPTARG == foobar ]] || log_error "getopts :x: failed"
OPTIND=1
getopts :r:s var -r
if [[ $var != : || $OPTARG != r ]]
then
    log_error "'getopts :r:s var -r' not working"
fi

OPTIND=1
getopts :d#u OPT -d 16177
if [[ $OPT != d || $OPTARG != 16177 ]]
then
    log_error "'getopts :d#u OPT=d OPTARG=16177' failed -- OPT=$OPT OPTARG=$OPTARG"
fi

OPTIND=1
while getopts 'ab' option -a -b
do
    [[ $OPTIND == $((OPTIND)) ]] || log_error "OPTIND optimization bug"
done

USAGE=$'[-][S:server?Operate on the specified \asubservice\a:]:[subservice:=pmserver]
    {
        [p:pmserver]
        [r:repserver]
        [11:notifyd]
    }'
set pmser p rep r notifyd -11
while (( $# > 1 ))
do
    OPTIND=1
    getopts "$USAGE" OPT -S $1
    [[ $OPT == S && $OPTARG == $2 ]] || log_error "OPT=$OPT OPTARG=$OPTARG -- expected OPT=S OPTARG=$2"
    shift 2
done

false ${foo=bar} &&  log_error "false failed"
read <<!
hello world
!
[[ $REPLY == 'hello world' ]] || log_error "read builtin failed"
print x:y | IFS=: read a b
if [[ $a != x ]]
then
    log_error "IFS=: read ... not working"
fi

read <<!
hello \
world
!
[[ $REPLY == 'hello world' ]] || log_error "read continuation failed"
read -d x <<!
hello worldxfoobar
!
[[ $REPLY == 'hello world' ]] || log_error "read builtin failed"
read <<\!
hello \
    world \

!
[[ $REPLY == 'hello     world' ]] || log_error "read continuation2 failed"
print "one\ntwo" | { read line
    print $line | /bin/cat > /dev/null
    read line
}
read <<\!
\
a\
\
\
b
!
if [[ $REPLY != ab ]]
then
    log_error "read multiple continuation failed"
fi

if [[ $line != two ]]
then
    log_error "read from pipeline failed"
fi

line=two
read line < /dev/null
if [[ $line != "" ]]
then
    log_error "read from /dev/null failed"
fi

if [[ $(print -R -) != - ]]
then
    log_error "print -R not working correctly"
fi

if [[ $(print -- -) != - ]]
then
    log_error "print -- not working correctly"
fi

print -f "hello%nbar\n" size > /dev/null
if ((    size != 5 ))
then
    log_error "%n format of printf not working"
fi

print -n -u2 2>&1-
[[ -w /dev/fd/1 ]] || log_error "2<&1- with built-ins has side effects"
x=$0
if [[ $(eval 'print $0') != $x ]]
then
    log_error '$0 not correct for eval'
fi

$SHELL -c 'read x <<< hello' 2> /dev/null || log_error 'syntax <<< not recognized'
($SHELL -c 'read x[1] <<< hello') 2> /dev/null || log_error 'read x[1] not working'
unset x
readonly x
set -- $(readonly)
if   [[ " $@ " != *" x "* ]]
then
    log_error 'unset readonly variables are not displayed'
fi

if [[ $(    for i in foo bar
        do
            print $i
            continue 10
        done
        ) != $'foo\nbar' ]]
then
    log_error 'continue breaks out of loop'
fi

(continue bad 2>/dev/null && log_error 'continue bad should return an error')
(break bad 2>/dev/null && log_error 'break bad should return an error')
(continue 0 2>/dev/null && log_error 'continue 0 should return an error')
(break 0 2>/dev/null && log_error 'break 0 should return an error')
breakfun() { break;}
continuefun() { continue;}
for fun in break continue
do
    if [[ $(    for i in foo
            do
                ${fun}fun
                print $i
            done
        ) != foo ]]
    then
        log_error "$fun call in ${fun}fun breaks out of for loop"
    fi
done

if [[ $(print -f "%b" "\a\n\v\b\r\f\E\03\\oo") != $'\a\n\v\b\r\f\E\03\\oo' ]]
then
    log_error 'print -f "%b" not working'
fi

if [[ $(print -f "%P" "[^x].*b\$") != '*[!x]*b' ]]
then
    log_error 'print -f "%P" not working'
fi

if [[ $(print -f "%(pattern)q" "[^x].*b\$") != '*[!x]*b' ]]
then
    log_error 'print -f "%(pattern)q" not working'
fi

if [[ $(abc: for i in foo bar;do print $i;break abc;done) != foo ]]
then
    log_error 'break labels not working'
fi

if [[ $(command -v if)    != if ]]
then
    log_error    'command -v not working'
fi

$SHELL -c 'command -p ls >/dev/null' 2>/dev/null || log_error 'command -p not working'

read -r var <<\!

!
if [[ $var != "" ]]
then
    log_error "read -r of blank line not working"
fi

mkdir -p $TEST_DIR/a/b/c 2>/dev/null || log_error  "mkdir -p failed"
$SHELL -c "cd $TEST_DIR/a/b; cd c" || log_error "initial script relative cd fails"

# The `| sort` is because ksh doesn't guarantee the order of the output of the `trap` command.
expect="trap -- 'print TERM' TERM trap -- 'print USR1' USR1"
actual=$(echo $($SHELL -c 'trap "print TERM" TERM; trap "print USR1" USR1; trap' | sort) )
[[ $actual == $expect ]] || log_error "'trap' failed" "$expect" "$actual"

expect='print TERM'
actual=$(echo $($SHELL -c 'trap "print TERM" TERM; trap "print USR1" USR1; trap -p TERM') )
[[ $actual == $expect ]] || log_error "'trap -p TERM' failed" "$expect" "$actual"

[[ $($SHELL -c 'trap "print ok" SIGTERM; kill -s SIGTERM $$' 2> /dev/null) == ok ]] || log_error 'SIGTERM not recognized'
[[ $($SHELL -c 'trap "print ok" sigterm; kill -s sigterm $$' 2> /dev/null) == ok ]] || log_error 'SIGTERM not recognized'
[[ $($SHELL -c '( trap "" TERM);kill $$;print bad' == bad) ]] 2> /dev/null && log_error 'trap ignored in subshell causes it to be ignored by parent'
${SHELL} -c 'kill -1 -$$' 2> /dev/null
[[ $(kill -l $?) == HUP ]] || log_error 'kill -1 -pid not working'
${SHELL} -c 'kill -1 -$$' 2> /dev/null
[[ $(kill -l $?) == HUP ]] || log_error 'kill -n1 -pid not working'
${SHELL} -c 'kill -s HUP -$$' 2> /dev/null
[[ $(kill -l $?) == HUP ]] || log_error 'kill -HUP -pid not working'
n=123
typeset -A base
base[o]=8#
base[x]=16#
base[X]=16#
for i in d i o u x X
do
    if (( $(( ${base[$i]}$(printf "%$i" $n) )) != n  ))
    then
        log_error "printf %$i not working"
    fi
done

if [[ $( trap 'print done' EXIT) != done ]]
then
    log_error 'trap on EXIT not working'
fi

if [[ $( trap 'print done' EXIT; trap - EXIT) == done ]]
then
    log_error 'trap on EXIT not being cleared'
fi

if [[ $(LC_MESSAGES=C type test) != 'test is a shell builtin' ]]
then
    log_error 'whence -v test not a builtin'
fi

builtin -d test
if [[ $(type test) == *builtin* ]]
then
    log_error 'whence -v test after builtin -d incorrect'
fi

typeset -Z3 percent=$(printf '%o\n' "'%'")
forrmat=\\${percent}s
if   [[ $(printf "$forrmat") != %s ]]
then
    log_error "printf $forrmat not working"
fi

if (( $(printf 'x\0y' | wc -c) != 3 ))
then
    log_error 'printf \0 not working'
fi

if [[ $(printf "%bx%s\n" 'f\to\cbar') != $'f\to' ]]
then
    log_error 'printf %bx%s\n  not working'
fi

alpha=abcdefghijklmnop
if [[ $(printf "%10.*s\n" 5 $alpha) != '     abcde' ]]
then
    log_error 'printf %10.%s\n  not working'
fi

float x2=.0000625
if [[ $(printf "%10.5E\n" x2) != 6.25000E-05 ]]
then
    log_error 'printf "%10.5E" not normalizing correctly'
fi

x2=.000000001
if [[ $(printf "%g\n" x2 2>/dev/null) != 1e-09 ]]
then
    log_error 'printf "%g" not working correctly'
fi

#FIXME#($SHELL read -s foobar <<\!
#FIXME#testing
#FIXME#!
#FIXME#) 2> /dev/null || log_error ksh read -s var fails
if [[ $(printf +3 2>/dev/null) !=   +3 ]]
then
    log_error 'printf is not processing formats beginning with + correctly'
fi

if printf "%d %d\n" 123bad 78 >/dev/null 2>/dev/null
then
    log_error "printf not exiting non-zero with conversion errors"
fi

if [[ $(trap --version 2> /dev/null;print done) != done ]]
then
    log_error 'trap builtin terminating after --version'
fi

if [[ $(set --version 2> /dev/null;print done) != done ]]
then
    log_error 'set builtin terminating after --veresion'
fi

unset -f foobar
function foobar
{
    print 'hello world'
}
OPTIND=1
if [[ $(getopts  $'[+?X\ffoobar\fX]' v --man 2>&1) != *'Xhello world'X* ]]
then
    log_error '\f...\f not working in getopts usage strings'
fi

if [[ $(printf '%H\n' $'<>"& \'\tabc') != '&lt;&gt;&quot;&amp;&nbsp;&apos;&#9;abc' ]]
then
    log_error 'printf %H not working'
fi

if [[ $(printf '%(html)q\n' $'<>"& \'\tabc') != '&lt;&gt;&quot;&amp;&nbsp;&apos;&#9;abc' ]]
then
    log_error 'printf %(html)q not working'
fi

if [[ $( printf 'foo://ab_c%(url)q\n' $'<>"& \'\tabc') != 'foo://ab_c%3C%3E%22%26%20%27%09abc' ]]
then
    log_error 'printf %(url)q not working'
fi

if [[ $(printf '%R %R %R %R\n' 'a.b' '*.c' '^'  '!(*.*)') != '^a\.b$ \.c$ ^\^$ ^(.*\..*)!$' ]]
then
    log_error 'printf %T not working'
fi

if [[ $(printf '%(ere)q %(ere)q %(ere)q %(ere)q\n' 'a.b' '*.c' '^'  '!(*.*)') != '^a\.b$ \.c$ ^\^$ ^(.*\..*)!$' ]]
then
    log_error 'printf %(ere)q not working'
fi

if [[ $(printf '%..:c\n' abc) != a:b:c ]]
then
    log_error "printf '%..:c' not working"
fi

if [[ $(printf '%..*c\n' : abc) != a:b:c ]]
then
    log_error "printf '%..*c' not working"
fi

if [[ $(printf '%..:s\n' abc def ) != abc:def ]]
then
    log_error "printf '%..:s' not working"
fi

if [[ $(printf '%..*s\n' : abc def) != abc:def ]]
then
    log_error "printf '%..*s' not working"
fi

[[ $(printf '%q\n') == '' ]] || log_error 'printf "%q" with missing arguments'

# We won't get hit by the one second boundary twice, right?
#
# TODO: Figure out how to make this test more robust.
actual=$(printf '%T\n' now | sed -e 's/GMT/UTC/')
expect=$(date)
if [[ "$actual" != "$expect" ]]
then
   # The timezone in the %T expansion may be GMT while the date command is UTC or vice-versa.
   # So make sure they both say UTC since the two strings are equivalent.
   actual=$(printf '%T\n' now | sed -e 's/GMT/UTC/')
   expect=$(date | sed -e 's/GMT/UTC/')
   if [[ "$actual" != "$expect" ]]
   then
      log_error 'printf "%T" now wrong output' "$expect" "$actual"
   fi
fi

behead()
{
    read line
    left=$(cat)
}
print $'line1\nline2' | behead
if [[ $left != line2 ]]
then
    log_error "read reading ahead on a pipe"
fi

expect=a
read -n1 actual <<!
abc
!
[[ $actual == $expect ]] || log_error "read -n1 failed" "$expect" "$actual"

print -n $'{ read -r line;print $line;}\nhello' > $TEST_DIR/script
chmod 755 $TEST_DIR/script
if [[ $($SHELL < $TEST_DIR/script) != hello ]]
then
    log_error 'read of incomplete line not working correctly'
fi

set -f
set -- *
if   [[ $1 != '*' ]]
then
    log_error 'set -f not working'
fi

unset pid1 pid2
false &
pid1=$!
pid2=$(
    wait $pid1
    (( $? == 127 )) || log_error "job known to subshell"
    print $!
)
wait $pid1
(( $? == 1 )) || log_error "wait not saving exit value"
wait $pid2
(( $? == 127 )) || log_error "subshell job known to parent"
env=
v=$(getconf LIBPATH 2> /dev/null)
for v in ${v//,/ }
do
    v=${v#*:}
    v=${v%%:*}
    eval [[ \$$v ]] && env="$env $v=\"\$$v\""
done

if [[ $(foo=bar; eval foo=\$foo $env exec -c \$SHELL -c \'print \$foo\') != bar ]]
then
    log_error '"name=value exec -c ..." not working'
fi

$SHELL -c 'OPTIND=-1000000; getopts a opt -a' 2> /dev/null
[[ $? == 1 ]] || log_error 'getopts with negative OPTIND not working'
getopts 'n#num' opt  -n 3
[[ $OPTARG == 3 ]] || log_error 'getopts with numerical arguments failed'
if [[ $($SHELL -c $'printf \'%2$s %1$s\n\' world hello') != 'hello world' ]]
then
    log_error 'printf %2$s %1$s not working'
fi

val=$(( 'C' ))
set -- \
    "'C"    $val    0    \
    "'C'"    $val    0    \
    '"C'    $val    0    \
    '"C"'    $val    0    \
    "'CX"    $val    1    \
    "'CX'"    $val    1    \
    "'C'X"    $val    1    \
    '"CX'    $val    1    \
    '"CX"'    $val    1    \
    '"C"X'    $val    1
while (( $# >= 3 ))
do
    arg=$1 expect=$2 code=$3
    shift 3
    for fmt in '%d' '%g'
    do
        actual=$(printf "$fmt" "$arg" 2>/dev/null)
        err=$(printf "$fmt" "$arg" 2>&1 >/dev/null)
        printf "$fmt" "$arg" >/dev/null 2>&1
        ret=$?
        [[ $actual == $expect ]] || log_error "printf $fmt $arg failed" "$expect" "$actual"
        if (( $code ))
        then
            [[ $err ]] || log_error "printf $fmt $arg failed, error message expected"
        else
            [[ $err ]] &&
               log_error "$err: printf $fmt $arg failed, error message not expected" "" "$err"
        fi

        (( $ret == $code )) || log_error "printf $fmt $arg failed -- wrong status" "$code" "$ret"
    done
done

((n=0))
((n++)); ARGC[$n]=1 ARGV[$n]=""
((n++)); ARGC[$n]=2 ARGV[$n]="-a"
((n++)); ARGC[$n]=4 ARGV[$n]="-a -v 2"
((n++)); ARGC[$n]=4 ARGV[$n]="-a -v 2 x"
((n++)); ARGC[$n]=4 ARGV[$n]="-a -v 2 x y"
for ((i=1; i<=n; i++))
do
    set -- ${ARGV[$i]}
    OPTIND=0
    while getopts -a tst "av:" OPT
    do
    :
    done

    expect="${ARGC[$i]}"
    actual="$OPTIND"
    [[ $actual == $expect ]] ||
        log_error "\$OPTIND after getopts loop incorrect" "$expect" "$actual"
done

options=ab:c
optarg=foo
set -- -a -b $optarg -c bar
while getopts $options opt
do
    case $opt in
    a|c)
       expect=""
       actual="$OPTARG"
       [[ $actual == $expect ]] ||
          log_error "getopts $options \$OPTARG for flag $opt failed" "$expect" "$actual"
       ;;
    b)
       expect="$optarg"
       actual="$OPTARG"
       [[ $actual == $expect ]] ||
          log_error "getopts $options \$OPTARG failed" "$expect" "$actual"
       ;;
    *) log_error "getopts $options failed" "" "$opt" ;;
    esac
done

[[ $($SHELL 2> /dev/null -c 'readonly foo; getopts a: foo -a blah; echo foo') == foo ]] || log_error 'getopts with readonly variable causes script to abort'

expect=abc
unset actual
{ read -N3 actual; read -N1 b;}  <<!
abcdefg
!
[[ $actual == $expect ]] || log_error "read -N3 here-document failed" "$expect" "$actual"
expect=d
actual="$b"
[[ $actual == $expect ]] || log_error "read -N1 here-document failed" "$expect" "$actual"

expect=ABC
read -n3 actual <<!
ABCDEFG
!
[[ $actual == $expect ]] || log_error "read -n3 here-document failed" "$expect" "$actual"

#(print -n a;sleep 1; print -n bcde) | { read -N3 a; read -N1 b;}
#[[ $a == $exp ]] || log_error "read -N3 from pipe failed -- expected '$exp', got '$a'"
#exp=d
#[[ $b == $exp ]] || log_error "read -N1 from pipe failed -- expected '$exp', got '$b'"
#(print -n a;sleep 1; print -n bcde) | read -n3 a
#exp=a
#[[ $a == $exp ]] || log_error "read -n3 from pipe failed -- expected '$exp', got '$a'"
#rm -f $TEST_DIR/fifo
#if mkfifo $TEST_DIR/fifo 2> /dev/null
#then
#    (print -n a; sleep 1;print -n bcde)  > $TEST_DIR/fifo &
#    {
#    read -u5 -n3 -t2 a || log_error 'read -n3 from fifo timedout'
#    read -u5 -n1 -t2 b || log_error 'read -n1 from fifo timedout'
#    } 5< $TEST_DIR/fifo
#    exp=a
#    [[ $a == $exp ]] || log_error "read -n3 from fifo failed -- expected '$exp', got '$a'"
#    rm -f $TEST_DIR/fifo
#    mkfifo $TEST_DIR/fifo 2> /dev/null
#    (print -n a; sleep 1;print -n bcde) > $TEST_DIR/fifo &
#    {
#    read -u5 -N3 -t2 a || log_error 'read -N3 from fifo timed out'
#    read -u5 -N1 -t2 b || log_error 'read -N1 from fifo timedout'
#    } 5< $TEST_DIR/fifo
#    exp=abc
#    [[ $a == $exp ]] || log_error "read -N3 from fifo failed -- expected '$exp', got '$a'"
#    exp=d
#    [[ $b == $exp ]] || log_error "read -N1 from fifo failed -- expected '$exp', got '$b'"
#fi

#rm -f $TEST_DIR/fifo

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

# test for debug trap
[[ $(typeset -i i=0
    trap 'print $i' DEBUG
    while (( i <2))
    do
        (( i++))
    done) == $'0\n0\n1\n1\n2' ]]  || log_error  "DEBUG trap not working"
typeset -F3 start_x=SECONDS total_t delay=0.02
typeset reps=50 leeway=5
sleep $(( 2 * leeway * reps * delay )) |
for (( i=0 ; i < reps ; i++ ))
do
    read -N1 -t $delay
done

(( total_t = SECONDS - start_x ))
if (( total_t > leeway * reps * delay ))
then
    log_error "read -t in pipe taking $total_t secs - $(( reps * delay )) minimum - too long"
elif (( total_t < reps * delay ))
then
    log_error "read -t in pipe taking $total_t secs - $(( reps * delay )) minimum - too fast"
fi

$SHELL -c 'sleep $(printf "%a" .95)' 2> /dev/null || log_error "sleep doesn't except %a format constants"
$SHELL -c 'test \( ! -e \)' 2> /dev/null ; [[ $? == 1 ]] || log_error 'test \( ! -e \) not working'
[[ $(ulimit) == "$(ulimit -fS)" ]] || log_error 'ulimit is not the same as ulimit -fS'

tmpfile=$TEST_DIR/file.2
print $'\nprint -r -- "${.sh.file} ${LINENO} ${.sh.lineno}"' > $tmpfile
actual="$(. $tmpfile)"
expect="$tmpfile 2 1"
[[ $actual == $expect ]] || log_error 'dot command not working' "$expect" "$actual"

tmpfile=$TEST_DIR/file.3
print -r -- "'xxx" > $tmpfile
actual="$($SHELL -c ". $tmpfile"$'\n print ok' 2> /dev/null)"
expect="ok"
[[ $actual == $expect ]] ||
   log_error 'syntax error in dot command affects next command' "$expect" "$actual"

typeset -r z=3
y=5
for i in 123 z  %x a.b.c
do
    ( unset $i)  2>/dev/null && log_error "unset $i should fail"
done

a=()
for i in y y  y[8] t[abc] y.d a.b  a
do
    unset $i ||  print -u2  "log_error unset $i should not fail"
done

[[ $($SHELL -c 'y=3; unset 123 y;print $?$y') == 1 ]] 2> /dev/null ||  log_error 'y is not getting unset with unset 123 y'
[[ $($SHELL -c 'trap foo TERM; (trap;(trap) )') == 'trap -- foo TERM' ]] || log_error 'traps not getting reset when subshell is last process'

n=$(printf "%b" 'a\0b\0c' | wc -c)
(( n == 5 )) || log_error '\0 not working with %b format with printf'

t=$(ulimit -t)
[[ $($SHELL -c 'ulimit -v 15000 2>/dev/null; ulimit -t') == "$t" ]] || log_error 'ulimit -v changes ulimit -t'

$SHELL 2> /dev/null -c 'cd ""' && log_error 'cd "" not producing an error'
[[ $($SHELL 2> /dev/null -c 'cd "";print hi') != hi ]] && log_error 'cd "" should not terminate script'

bincat=$(whence -p cat)
builtin cat
out=$TEST_DIR/seq.out
seq 11 >$out
cmp -s <(print -- "$($bincat<( $bincat $out ) )") <(print -- "$(cat <( cat $out ) )") || log_error "builtin cat differs from $bincat"

[[ $($SHELL -c '{ printf %R "["; print ok;}' 2> /dev/null) == ok ]] || log_error $'\'printf %R "["\' causes shell to abort'

v=$( $SHELL -c $'
    trap \'print "usr1"\' USR1
    trap exit USR2
    sleep 1 && {
        kill -USR1 $$ && sleep 1
        kill -0 $$ 2>/dev/null && kill -USR2 $$
    } &
    sleep 2 | read
    echo done

' ) 2> /dev/null
[[ $v == $'usr1\ndone' ]] ||  log_error 'read not terminating when receiving USR1 signal'

mkdir $TEST_DIR/tmpdir1
cd $TEST_DIR/tmpdir1
pwd=$PWD
cd ../tmpdir1
[[ $PWD == "$pwd" ]] || log_error 'cd ../tmpdir1 causes directory to change'
cd "$pwd"
mv $TEST_DIR/tmpdir1 $TEST_DIR/tmpdir2
cd ..  2> /dev/null || log_error 'cannot change directory to .. after current directory has been renamed'
[[ $PWD == "$TEST_DIR" ]] || log_error 'after "cd $TEST_DIR/tmpdir1; cd .." directory is not $TEST_DIR'

cd "$TEST_DIR"
mkdir $TEST_DIR/tmpdir2/foo
pwd=$PWD
cd $TEST_DIR/tmpdir2/foo
mv $TEST_DIR/tmpdir2 $TEST_DIR/tmpdir1
cd ../.. 2> /dev/null || log_error 'cannot change directory to ../.. after current directory has been renamed'
[[ $PWD == "$TEST_DIR" ]] || log_error 'after "cd $TEST_DIR/tmpdir2; cd ../.." directory is not $TEST_DIR'
cd "$TEST_DIR"
rm -rf tmpdir1

cd /etc
cd ..
[[ $(pwd) == / ]] || log_error 'cd /etc;cd ..;pwd is not /'
cd /etc
cd ../..
[[ $(pwd) == / ]] || log_error 'cd /etc;cd ../..;pwd is not /'
cd /etc
cd .././..
[[ $(pwd) == / ]] || log_error 'cd /etc;cd .././..;pwd is not /'
cd /usr/bin
cd ../..
[[ $(pwd) == / ]] || log_error 'cd /usr/bin;cd ../..;pwd is not /'
cd /usr/bin
cd ..
[[ $(pwd) == /usr ]] || log_error 'cd /usr/bin;cd ..;pwd is not /usr'
cd "$TEST_DIR"
if mkdir $TEST_DIR/t1
then
    (
        cd $TEST_DIR/t1
        > real_t1
        (
            cd ..
            mv t1 t2
            mkdir t1
        )
        [[ -f real_t1 ]] || log_error 'real_t1 not found after parent directory renamed in subshell'
    )
fi

cd "$TEST_DIR"

> foobar
CDPATH= $SHELL 2> /dev/null -c 'cd foobar' && log_error "cd to a regular file should fail"

cd "$TEST_DIR"
mkdir foo .bar
cd foo
cd ../.bar 2> /dev/null || log_error 'cd ../.bar when ../.bar exists should not fail'

$SHELL +E -i <<- \! && log_error 'interactive shell should not exit 0 after false'
    false
    exit
!

if kill -L > /dev/null 2>&1
then
    [[ $(kill -l HUP) == "$(kill -L HUP)" ]] || log_error 'kill -l and kill -L are not the same when given a signal name'
    [[ $(kill -l 9) == "$(kill -L 9)" ]] || log_error 'kill -l and kill -L are not the same when given a signal number'
    [[ $(kill -L) == *'9) KILL'* ]] || log_error 'kill -L output does not contain 9) KILL'
fi


unset ENV
v=$($SHELL 2> /dev/null +o rc -ic $'getopts a:bc: opt --man\nprint $?')
[[ $v == 2* ]] || log_error 'getopts --man does not exit 2 for interactive shells'

read baz <<< 'foo\\\\bar'
[[ $baz == 'foo\\bar' ]] || log_error 'read of foo\\\\bar not getting foo\\bar'

: ~root
[[ $(builtin) == *.sh.tilde* ]] &&  log_error 'builtin contains .sh.tilde'

IFS=',' read -S a b c <<<'foo,"""title"" data",bar'
[[ $b == '"title" data' ]] || log_error '"" inside "" not handled correctly with read -S'

PATH=/bin:/usr/bin
basename=$(whence -p basename)
cmp=$(whence -p cmp)
.sh.op_astbin=/opt/ast/bin
PATH=/opt/ast/bin:$PATH
PATH=/opt/ast/bin:/bin:/usr/bin
[[ ${SH_OPTIONS} == *astbin=/opt/ast/bin* ]] || log_error "SH_OPTIONS=${SH_OPTIONS} but should contain astbin=/opt/ast/bin"
[[ $(whence basename) == /opt/ast/bin/basename ]] || log_error "basename bound to $(whence basename) but should be bound to /opt/ast/bin/basename"
[[ $(whence cmp) == /opt/ast/bin/cmp ]] || log_error "cmp bound to $(whence cmp) but should be bound to /opt/ast/bin/cmp"
.sh.op_astbin=/bin
SH_OPTIONS=astbin=/bin
[[ ${SH_OPTIONS} == *astbin=/bin* ]] || log_error "SH_OPTIONS=${SH_OPTIONS} but should contain astbin=/bin"
[[ $(whence basename) == /bin/basename ]] || log_error "basename bound to $(whence basename) but should be bound to /bin/basename"
[[ $(whence cmp) == /bin/cmp ]] || log_error "cmp bound to $(whence cmp) but should be bound to /bin/cmp"
.sh.op_astbin=/opt/ast/bin
[[ $(whence basename) == /opt/ast/bin/basename ]] || log_error "basename bound to $(whence basename) but should be rebound to /opt/ast/bin/basename"
[[ $(whence cmp) == /opt/ast/bin/cmp ]] || log_error "cmp bound to $(whence cmp) but should be rebound to /opt/ast/bin/cmp"
PATH=/bin:/usr/bin:/opt/ast/bin
[[ $(whence basename) == "$basename" ]] || log_error "basename bound to $(whence basename) but should be bound to $basename when PATH=$PATH"
[[ $(whence cmp) == "$cmp" ]] || log_error "cmp bound to $(whence cmp) but should be bound to $cmp when PATH=$PATH"

unset y
expect='outside f, 1, 2, 3, outside f'
actual=$(
    f() {
        if [ -n "${_called_f+_}" ]; then
            for y; do
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

{ $SHELL -c 'kill %' ;} 2> /dev/null
[[ $? == 1 ]] || log_error "'kill %' has wrong exit status"

printf '\\\000' | read -r -d ''
[[ $REPLY == $'\\' ]] || log_error "read -r -d'' ignores -r"

wait
unset i
integer i
for (( i=0 ; i < 256 ; i++ ))
do
    sleep 2 &
done

while ! wait
do
    true
done

[[ $(jobs -l) ]] && log_error 'jobs -l should not have any output'

# tests with cd
pwd=$PWD
exec {fd}</dev

cd /dev
if cd ~-
then
    [[ $PWD == "$pwd" ]] || log_error "directory is $PWD, should be $pwd"
else
    log_error "unable to cd ~- back to $pwd"
fi

if cd -f $fd
then
    [[ -r null ]] || log_error 'cannot find "null" file in /dev'
else
    log_error 'cannot cd to ~{fd} when fd is /dev'
fi

mkdir $TEST_DIR/oldpwd
OLDPWD=$TEST_DIR/oldpwd
cd - > $TEST_DIR/cd.out
actual=$(< $TEST_DIR/cd.out)
expect="$TEST_DIR/oldpwd"
[[ $actual == $expect ]] || log_error "cd - does not recognize overridden OLDPWD variable"
[[ $PWD == $expect ]] || log_error "cd - does not recognize overridden OLDPWD variable"

cd $TEST_DIR
[[ $(OLDPWD="$TEST_DIR/oldpwd" cd -) == "$TEST_DIR/oldpwd" ]] ||
    log_error "cd - does not recognize overridden OLDPWD variable if it is overridden in new scope"

[[ $(pwd -f $fd) == /dev ]] || log_error "pwd -f $fd should be /dev"

$SHELL <<- \EOF
    # $HOME is set to a temporary directory by test framework
    # Get actual home directory of the user
    home=~$USER
    unset HOME
    cd
    [[ $(pwd) == "$home" ]]
EOF
[[ $? == 0 ]] || log_error 'cd with no arguments fails if HOME is unset'

cd "$TEST_DIR"
if mkdir -p f1
then
    redirect {d}<f1
    pwd=$(pwd)
    ( cd -f $d && [[ $(pwd) == "$pwd/f1" ]]) || log_error '$(pwd) does not show new directory'
    [[ $(pwd) == "$pwd" ]] || log_error '$(pwd) is not $pwd'
    [[ $(/bin/pwd) == "$pwd" ]] || log_error  '/bin/pwd is not "$pwd"'
    [[ $(/bin/pwd) == "$(pwd)" ]] || log_error  '/bin/pwd is not pwd'
    cd "$pwd"
    rmdir "$pwd/f1"
fi

$SHELL 2> /dev/null <<- \!!! || log_error 'alarm during read causes core dump'
    function input_feed
    {
        typeset i
        for ((i=0; i<3 ; i++))
        do
            print hello,world
            sleep .3
        done
    }
    alarm -r alarm_handler +.1
    function alarm_handler.alarm
    {
        print "goodbye world" | read arg1 arg2
    }

    input_feed | while IFS=',' read arg1 arg2
    do
    :
    done
!!!
# test for eval bug when called from . script in a startup file.
print $'eval : foo\nprint ok' > $TEST_DIR/evalbug
print ". $TEST_DIR/evalbug" >$TEST_DIR/envfile
[[ $(ENV=$TEST_DIR/envfile $SHELL -i -c : 2> /dev/null) == ok ]] || log_error 'eval inside dot script called from profile file not working'

# test cd to a directory that doesn't have execute permission
if mkdir -p $TEST_DIR/a/b
then
    chmod -x $TEST_DIR/a/b
    cd $TEST_DIR/a/b 2> /dev/null && log_error 'cd to directory without execute should fail'
fi
chmod +x $TEST_DIR/a/b  # so the test temp dir can be removed when the test completes

# -s flag writes to history file
if print -s 'print hello world' 2> /dev/null
then
    [[ $(history -1) == *'hello world'* ]] || log_error 'history file does not contain result of print -s'
else
    log_error 'print -s fails'
fi

# Check if history file is updated correclty if entry does not end with newline
if print -s -f 'print foo' 2> /dev/null
then
    [[ $(history -1) == *'foo' ]] || log_error 'history file does not contain result of print -s -f'
else
    log_error 'print -s -f fails'
fi

builtin  -d set 2> /dev/null && log_error 'buitin -d allows special builtins to be deleted'

builtin -f $LIBSAMPLE_PATH sample || log_error "Failed to load sample builtin"

sample >/dev/null || log_error "Sample builtin should exit with 0 status"
