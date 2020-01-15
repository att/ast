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

[[ ${.sh.version} == "$KSH_VERSION" ]] || log_error '.sh.version != KSH_VERSION'
unset ss
[[ ${@ss} ]] && log_error '${@ss} should be empty string when ss is unset'
[[ ${!ss} == ss ]] ||  log_error '${!ss} should be ss when ss is unset'
[[ ${#ss} == 0 ]] ||  log_error '${#ss} should be 0 when ss is unset'
# RANDOM
if (( RANDOM==RANDOM || $RANDOM==$RANDOM ))
then
    log_error RANDOM variable not working
fi

# ==========
# SECONDS tracks time correctly.
#
# The wide range of acceptable values is because when running on VMs like Travis CI (especially its
# macOS VMs) the sleep time can be considerably longer than we asked for.
SECONDS=0
sleep 0.1
(( SECONDS > 0.05 && SECONDS < 0.3 )) || log_error "SECONDS variable not working" "0.100" "$SECONDS"

# ==========
# _
set abc def
if [[ $_ != def ]]
then
    log_error _ variable not working
fi

# ERRNO
#set abc def
#rm -f foobar#
#ERRNO=
#2> /dev/null < foobar#
#if (( ERRNO == 0 ))
#then
#    log_error ERRNO variable not working
#fi

# PWD
if [[ !  $PWD -ef . ]]
then
    log_error PWD variable failed, not equivalent to .
fi

# PPID
exp=$$
got=${ $SHELL -c 'print $PPID'; }
if [[ ${ $SHELL -c 'print $PPID'; } != $$ ]]
then
    log_error "PPID variable failed -- expected '$exp', got '$got'"
fi

# OLDPWD
old=$PWD
cd /
if [[ $OLDPWD != $old ]]
then
    log_error "OLDPWD variable failed -- expected '$old', got '$OLDPWD'"
fi

cd $old || log_error cd failed
# REPLY
read <<-!
foobar
!
if [[ $REPLY != foobar ]]
then
    log_error REPLY variable not working
fi

integer save=$LINENO
# LINENO
LINENO=10
#
#  These lines intentionally left blank
#
if (( LINENO != 13))
then
    log_error LINENO variable not working
fi

LINENO=save+10
IFS=:
x=a::b::c
if [[ $x != a::b::c ]]
then
    log_error "word splitting on constants"
fi

set -- $x
if [[ $# != 5 ]]
then
    log_error ":: doesn't separate null arguments "
fi

set x
if x$1=0
then
    log_error "x\$1=value treated as an assignment"
fi

# Check for attributes across subshells
typeset -i x=3
y=1/0
if ( x=y )
then
    log_error "attributes not passed to subshells"
fi

unset x
function x.set
{
    nameref foo=${.sh.name}.save
    foo=${.sh.value}
    .sh.value=$0
}
x=bar
if [[ $x != x.set ]]
then
    log_error 'x.set does not override assignment'
fi

x.get()
{
    nameref foo=${.sh.name}.save
    .sh.value=$foo
}

if [[ $x != bar ]]
then
    log_error 'x.get does not work correctly'
fi

typeset +n foo
unset foo
foo=bar
(
    unset foo
    set +u
    if [[ $foo != '' ]]
    then
        log_error '$foo not null after unset in subsehll'
    fi

)
if [[ $foo != bar ]]
then
    log_error 'unset foo in subshell produces side effect '
fi

unset foo
if [[ $( { : ${foo?hi there} ; } 2>&1) != *'hi there' ]]
then
    log_error '${foo?hi there} with foo unset does not print hi there on 2'
fi

x=$0
set foobar
if [[ ${@:0} != "$x foobar" ]]
then
    log_error '${@:0} not expanding correctly'
fi

set --
if [[ ${*:0:1} != "$0" ]]
then
    log_error '${@:0} not expanding correctly'
fi

ACCESS=0
function COUNT.set
{
        (( ACCESS++ ))
}
COUNT=0
(( COUNT++ ))
if (( COUNT != 1 || ACCESS!=2 ))
then
    log_error " set discipline failure COUNT=$COUNT ACCESS=$ACCESS"
fi

LANG=C
if [[ $LANG != C ]]
then
    log_error "C locale not working"
fi

unset RANDOM
unset -n foo
foo=junk
function foo.get
{
    .sh.value=stuff
    unset -f foo.get
}
if [[ $foo != stuff ]]
then
    log_error "foo.get discipline not working"
fi

if [[ $foo != junk ]]
then
    log_error "foo.get discipline not working after unset"
fi

# special variables
set -- 1 2 3 4 5 6 7 8 9 10
sleep 1000 &
if [[ $(print -r -- ${#10}) != 2 ]]
then
    log_error '${#10}, where ${10}=10 not working'
fi

for i in @ '*' ! '#' - '?' '$'
do
    false
    eval foo='$'$i bar='$'{$i}
    if [[ ${foo} != "${bar}" ]]
    then
        log_error "\$$i not equal to \${$i}"
    fi

    command eval bar='$'{$i%?} || log_error "\${$i%?} gives syntax error"
    if [[ $i != [@*] && ${foo%?} != "$bar"  ]]
    then
        log_error "\${$i%?} not correct"
    fi

    command eval bar='$'{$i#?} || log_error "\${$i#?} gives syntax error"
    if [[ $i != [@*] && ${foo#?} != "$bar"  ]]
    then
        log_error "\${$i#?} not correct"
    fi

    command eval foo='$'{$i} bar='$'{#$i} || log_error "\${#$i} gives syntax error"
    if [[ $i != @([@*]) && ${#foo} != "$bar" ]]
    then
        log_error "\${#$i} not correct"
    fi
done
kill $!
unset x
CDPATH=/
x=$(cd ${TEST_DIR#/})
if [[ $x != $TEST_DIR ]]
then
    log_error 'CDPATH does not display new directory'
fi

CDPATH=/:
x=$(cd ${TEST_DIR%/*}; cd ${TEST_DIR##*/})
if [[ $x ]]
then
    log_error 'CDPATH displays new directory when not used'
fi

x=$(cd ${TEST_DIR#/})
if [[ $x != $TEST_DIR ]]
then
    log_error "CDPATH ${TEST_DIR#/} does not display new directory"
fi

TMOUT=100
(TMOUT=20)
if (( TMOUT !=100 ))
then
    log_error 'setting TMOUT in subshell affects parent'
fi

unset y
function setdisc # var
{
    eval function $1.get'
    {
        .sh.value=good
    }
        '
}
y=bad
setdisc y
if [[ $y != good ]]
then
    log_error 'setdisc function not working'
fi

integer x=$LINENO
: $'\
'
if (( LINENO != x+3  ))
then
    log_error '\<newline> gets linenumber count wrong'
fi

set --
set -- "${@-}"
if (( $# !=1 ))
then
    log_error '"${@-}" not expanding to null string'
fi

for i in : % + / 3b '**' '***' '@@' '{' '[' '}' !!  '*a' '$$'
do
    (eval : \${"$i"} ) && log_error "\${$i} not an syntax error"
done

unset IFS
( IFS='  ' ; read -r a b c <<-!
	x  y z
	!
	if [[ $b ]]
	then
		log_error 'IFS="  " not causing adjacent space to be null string'
	fi

)
read -r a b c <<-!
x  y z
!
if [[ $b != y ]]
then
    log_error 'IFS not restored after subshell'
fi


# The next part generates 3428 IFS set/read tests.
unset IFS x
function split
{
    i=$1 s=$2 r=$3
    IFS=': '
    set -- $i
    IFS=' '
    g="[$#]"
    while    :
    do    case $# in
        0)    break ;;
        esac
        g="$g($1)"
        shift
    done

    case "$g" in
    "$s")    ;;
    *)    log_error "IFS=': '; set -- '$i'; expected '$s' got '$g'" ;;
    esac
    print "$i" | IFS=": " read arg rem; g="($arg)($rem)"
    case "$g" in
    "$r")    ;;
    *)    log_error "IFS=': '; read '$i'; expected '$r' got '$g'" ;;
    esac
}
for str in     \
    '-'    \
    'a'    \
    '- -'    \
    '- a'    \
    'a -'    \
    'a b'    \
    '- - -'    \
    '- - a'    \
    '- a -'    \
    '- a b'    \
    'a - -'    \
    'a - b'    \
    'a b -'    \
    'a b c'
do
    IFS=' '
    set x $str
    shift
    case $# in
    0)    continue ;;
    esac
    f1=$1
    case $f1 in
    '-')    f1='' ;;
    esac
    shift
    case $# in
    0)    for d0 in '' ' '
        do
            for d1 in '' ' ' ':' ' :' ': ' ' : '
            do
                case $f1$d1 in
                '')    split "$d0$f1$d1" "[0]" "()()" ;;
                ' ')    ;;
                *)    split "$d0$f1$d1" "[1]($f1)" "($f1)()" ;;
                esac
            done
        done
        continue
        ;;
    esac
    f2=$1
    case $f2 in
    '-')    f2='' ;;
    esac
    shift
    case $# in
    0)    for d0 in '' ' '
        do
            for d1 in ' ' ':' ' :' ': ' ' : '
            do
                case ' ' in
                $f1$d1|$d1$f2)    continue ;;
                esac
                for d2 in '' ' ' ':' ' :' ': ' ' : '
                do
                    case $f2$d2 in
                    '')    split "$d0$f1$d1$f2$d2" "[1]($f1)" "($f1)()" ;;
                    ' ')    ;;
                    *)    split "$d0$f1$d1$f2$d2" "[2]($f1)($f2)" "($f1)($f2)" ;;
                    esac
                done
            done
        done
        continue
        ;;
    esac
    f3=$1
    case $f3 in
    '-')    f3='' ;;
    esac
    shift
    case $# in
    0)    for d0 in '' ' '
        do
            for d1 in ':' ' :' ': ' ' : '
            do
                case ' ' in
                $f1$d1|$d1$f2)    continue ;;
                esac
                for d2 in ' ' ':' ' :' ': ' ' : '
                do
                    case $f2$d2 in
                    ' ')    continue ;;
                    esac
                    case ' ' in
                    $f2$d2|$d2$f3)    continue ;;
                    esac
                    for d3 in '' ' ' ':' ' :' ': ' ' : '
                    do
                        case $f3$d3 in
                        '')    split "$d0$f1$d1$f2$d2$f3$d3" "[2]($f1)($f2)" "($f1)($f2)" ;;
                        ' ')    ;;
                        *)    x=$f2$d2$f3$d3
                            x=${x#' '}
                            x=${x%' '}
                            split "$d0$f1$d1$f2$d2$f3$d3" "[3]($f1)($f2)($f3)" "($f1)($x)"
                            ;;
                        esac
                    done
                done
            done
        done
        continue
        ;;
    esac
done
unset IFS


{ $SHELL -c '
function foo
{
    typeset SECONDS=0
    sleep 1.5
    print $SECONDS

}
x=$(foo)
(( x >1 && x < 2 ))
'
} || log_error 'SECONDS not working in function'
cat > $TEST_DIR/script <<-\!
	posixfun()
	{
		unset x
		nameref x=$1
		print  -r -- "$x"
	}
	function fun
	{
		nameref x=$1
		print  -r -- "$x"
	}
	if [[ $1 ]]
	then
		file=${.sh.file}
	else
		print -r -- "${.sh.file}"
	fi
!

chmod +x $TEST_DIR/script
. $TEST_DIR/script  1
[[ $file == $TEST_DIR/script ]] || log_error ".sh.file not working for dot scripts"
[[ $($SHELL $TEST_DIR/script) == $TEST_DIR/script ]] || log_error ".sh.file not working for scripts"
[[ $(posixfun .sh.file) == $TEST_DIR/script ]] || log_error ".sh.file not working for posix functions"
[[ $(fun .sh.file) == $TEST_DIR/script ]] || log_error ".sh.file not working for functions"
[[ $(posixfun .sh.fun) == posixfun ]] || log_error ".sh.fun not working for posix functions"
[[ $(fun .sh.fun) == fun ]] || log_error ".sh.fun not working for functions"
[[ $(posixfun .sh.subshell) == 1 ]] || log_error ".sh.subshell not working for posix functions"
[[ $(fun .sh.subshell) == 1 ]] || log_error ".sh.subshell not working for functions"
(
    [[ $(posixfun .sh.subshell) == 2 ]]  || log_error ".sh.subshell not working for posix functions in subshells"
    [[ $(fun .sh.subshell) == 2 ]]  || log_error ".sh.subshell not working for functions in subshells"
    (( .sh.subshell == 1 )) || log_error ".sh.subshell not working in a subshell"
)
TIMEFORMAT='this is a test'
[[ $({ { time :;} 2>&1;}) == "$TIMEFORMAT" ]] || log_error 'TIMEFORMAT not working'
: ${.sh.version}
[[ $(alias integer) == *.sh.* ]] && log_error '.sh. prefixed to alias name'
: ${.sh.version}
[[ $(whence rm) == *.sh.* ]] && log_error '.sh. prefixed to tracked alias name'
: ${.sh.version}
[[ $(cd /bin;env | grep '^PWD=') == *.sh.* ]] && log_error '.sh. prefixed to PWD'
# unset discipline bug fix
dave=dave
function dave.unset
{
    unset dave
}
unset dave
[[ $(typeset +f) == *dave.* ]] && log_error 'unset discipline not removed'

x=$(
    dave=dave
    function dave.unset
    {
        print dave.unset
    }
)
[[ $x == dave.unset ]] || log_error 'unset discipline not called with subset completion'

print 'print ${VAR}' > $TEST_DIR/script
unset VAR
VAR=new $TEST_DIR/script > $TEST_DIR/out
got=$(<$TEST_DIR/out)
[[ $got == new ]] || log_error "previously unset environment variable not passed to script, expected 'new', got '$got'"
[[ ! $VAR ]] || log_error "previously unset environment variable set after script, expected '', got '$VAR'"
unset VAR
VAR=old
VAR=new $TEST_DIR/script > $TEST_DIR/out
got=$(<$TEST_DIR/out)
[[ $got == new ]] || log_error "environment variable covering local variable not passed to script, expected 'new', got '$got'"
[[ $VAR == old ]] || log_error "previously set local variable changed after script, expected 'old', got '$VAR'"
unset VAR
export VAR=old
VAR=new $TEST_DIR/script > $TEST_DIR/out
got=$(<$TEST_DIR/out)
[[ $got == new ]] || log_error "environment variable covering environment variable not passed to script, expected 'new', got '$got'"
[[ $VAR == old ]] || log_error "previously set environment variable changed after script, expected 'old', got '$VAR'"

(
    unset dave
    function  dave.append
    {
        .sh.value+=$dave
        dave=
    }
    dave=foo; dave+=bar
    [[ $dave == barfoo ]] || exit 2
)
case $? in
0)     ;;
1)     log_error 'append discipline not implemented';;
*)     log_error 'append discipline not working';;
esac
.sh.foobar=hello
{
    function .sh.foobar.get
    {
        .sh.value=world
    }
} || log_error "cannot add get discipline to .sh.foobar"
[[ ${.sh.foobar} == world ]]  || log_error 'get discipline for .sh.foobar not working'
x='a|b'
IFS='|'
set -- $x
[[ $2 == b ]] || log_error '$2 should be b after set'
exec 3>&2 2> /dev/null
set -x
( IFS= ) 2> /dev/null
set +x
exec 2>&3-
set -- $x
[[ $2 == b ]] || log_error '$2 should be b after subshell'
: & pid=$!
( : & )
[[ $pid == $! ]] || log_error '$! value not preserved across subshells'
unset foo
typeset -A foo
function foo.set
{
    case ${.sh.subscript} in
    bar)    if ((.sh.value > 1 ))
            then
                .sh.value=5
                foo[barrier_hit]=yes
            fi
        ;;
    barrier_hit)
        if [[ ${.sh.value} == yes ]]
        then
            foo[barrier_not_hit]=no
        else
            foo[barrier_not_hit]=yes
        fi
        ;;
    esac
}
foo[barrier_hit]=no
foo[bar]=1
(( foo[bar] == 1 )) || log_error 'foo[bar] should be 1'
[[ ${foo[barrier_hit]} == no ]] || log_error 'foo[barrier_hit] should be no'
[[ ${foo[barrier_not_hit]} == yes ]] || log_error 'foo[barrier_not_hit] should be yes'
foo[barrier_hit]=no
foo[bar]=2
(( foo[bar] == 5 )) || log_error 'foo[bar] should be 5'
[[ ${foo[barrier_hit]} == yes ]] || log_error 'foo[barrier_hit] should be yes'
[[ ${foo[barrier_not_hit]} == no ]] || log_error 'foo[barrier_not_hit] should be no'
unset x
typeset -i x
function x.set
{
    typeset sub=${.sh.subscript}
    (( sub > 0 )) && (( x[sub-1]= x[sub-1] + .sh.value ))
}
x[0]=0 x[1]=1 x[2]=2 x[3]=3
[[ ${x[@]} == '12 8 5 3' ]] || log_error 'set discipline for indexed array not working correctly'
float seconds
((SECONDS=3*4))
seconds=SECONDS
(( seconds < 12 || seconds > 12.1 )) &&  log_error "SECONDS is $seconds and should be close to 12"
unset a
function a.set
{
    print -r -- "${.sh.name}=${.sh.value}"
}
[[ $(a=1) == a=1 ]] || log_error 'set discipline not working in subshell assignment'
[[ $(a=1 :) == a=1 ]] || log_error 'set discipline not working in subshell command'

[[ ${.sh.subshell} == 0 ]] || log_error '${.sh.subshell} should be 0'
(
    [[ ${.sh.subshell} == 1 ]] || log_error '${.sh.subshell} should be 1'
    (
        [[ ${.sh.subshell} == 2 ]] || log_error '${.sh.subshell} should be 2'
    )
)

set -- {1..32768}
(( $# == 32768 )) || log_error "\$# failed -- expected 32768, got $#"
set --

unset r v x
path=$PATH
x=foo
# We would like to include LINENO in this list but mucking with it affects the line numbers that
# appear in the diagnostic messages.
for v in EDITOR VISUAL OPTIND CDPATH FPATH PATH ENV RANDOM SECONDS _
do
    nameref r=$v
    unset $v
    if ( $SHELL -c "unset $v; : \$$v" )
    then
    [[ $r ]] && log_error "unset $v failed -- expected '', got '$r'"
        r=$x
        [[ $r == $x ]] || log_error "$v=$x failed -- expected '$x', got '$r'"
    else
        log_error "unset $v; : \$$v failed"
    fi
done
PATH=$path

cd $TEST_DIR
print print -n zzz > zzz
chmod +x zzz
exp='aaazzz'
got=$($SHELL -c 'unset SHLVL; print -n aaa; ./zzz' 2>&1)
[[ $got == "$exp" ]] || log_error "unset SHLVL causes script failure -- expected '$exp', got '$got'"

mkdir glean
for cmd in date ok
do
    exp="$cmd ok"
    rm -f $cmd
    print print $exp > glean/$cmd
    chmod +x glean/$cmd
    got=$(CDPATH=:.. $SHELL -c "PATH=:/bin:/usr/bin; date > /dev/null; cd glean && ./$cmd" 2>&1)
    [[ $got == "$exp" ]] || log_error "cd with CDPATH after PATH change failed -- expected '$exp', got '$got'"
done

v=LC_CTYPE
unset $v
[[ -v $v ]] && log_error "unset $v; [[ -v $v ]] failed"
eval $v=C
[[ -v $v ]] || log_error "$v=C; [[ -v $v ]] failed"

cmd='set --nounset; unset foo; : ${!foo*}'
$SHELL -c "$cmd" || log_error "'$cmd' exit status $?, expected 0"

SHLVL=1
level=$($SHELL -c $'$SHELL -c \'print -r "$SHLVL"\'')
[[ $level  == 3 ]]  || log_error "SHLVL should be 3 not $level"

[[ $($SHELL -c '{ x=1; : ${x.};print ok;}') == ok ]] || log_error '${x.} where x is a simple variable causes shell to abort'

expect=': .sh: is read only'
actual=$($SHELL -c 'unset .sh' 2>&1)
[[ $? == 1 ]] || log_error 'unset .sh should return 1'
[[ $actual == *$expect ]] || log_error 'unset .sh should report it is readonly'

x=$($SHELL -c 'foo=bar foobar=fbar; print -r -- ${!foo*}')
[[ $x == 'foo '* ]] || log_error 'foo not included in ${!foo*}'

# ==========
# Verify that the special .sh.sig compound var contains at least a few of the expected members.
#
function contains_sig_var1 {
    typeset want=$1
    for name in "${!.sh.sig@}"
    do
        [[ $want == $name ]] && return 0
    done
    return 1
}

function contains_sig_var2 {
    typeset want=$1
    for name in ${!.sh.sig*}
    do
        [[ $want == $name ]] && return 0
    done
    return 1
}

contains_sig_var1 .sh.sig.pid    || log_error '.sh.sig.pid not in ${!.sh.sig@]}'
contains_sig_var1 .sh.sig.uid    || log_error '.sh.sig.uid not in ${!.sh.sig@]}'
contains_sig_var1 .sh.sig.signo  || log_error '.sh.sig.signo not in ${!.sh.sig@]}'
contains_sig_var1 .sh.sig.status || log_error '.sh.sig.status not in ${!.sh.sig@]}'
contains_sig_var1 .sh.sig.value  || log_error '.sh.sig.value.q not in ${!.sh.sig@]}'

contains_sig_var2 .sh.sig.pid    || log_error '.sh.sig.pid not in ${!.sh.sig*]}'
contains_sig_var2 .sh.sig.uid    || log_error '.sh.sig.uid not in ${!.sh.sig*]}'
contains_sig_var2 .sh.sig.signo  || log_error '.sh.sig.signo not in ${!.sh.sig*]}'
contains_sig_var2 .sh.sig.status || log_error '.sh.sig.status not in ${!.sh.sig*]}'
contains_sig_var2 .sh.sig.value  || log_error '.sh.sig.value.q not in ${!.sh.sig*]}'

unset x
integer x=1
[[ $(x+=3 command eval echo \$x) == 4 ]] || log_error '+= assignment for environment variables for command special_built-in not working'
(( $x == 1 )) || log_error 'environment not restored afer command special_builtin'
[[ $(x+=3 eval echo \$x) == 4 ]] || log_error '+= assignment for environment variables for built-ins not working'

# Tests for ${$parameter}
set abc def
abc=foo
def=bar
[[ ${$2:1:1} == a ]] || log_error '${$2:1:1} not correct with $2=def and def=bar'
OPTIND=2
[[ ${$OPTIND:1:1} == e ]] || log_error '${$OPTIND:1:1} not correct with OPTIND=2 and $2=def'

# Check if ${.sh.file} is set to correct value after sourcing a file
# https://github.com/att/ast/issues/472
cat > $TEST_DIR/foo.sh <<EOF
echo "foo"
EOF
. $TEST_DIR/foo.sh > /dev/null
expect="$0"
actual="${.sh.file}"
[[ $actual == $expect ]] ||
    log_error ".sh.file is not set to correct value after sourcing a file" "$expect" "$actual"

# Check if version string is set to nonsense value in arithmetic context
# Due to backward compatibility concerns any major release can have maximum yyyy.99.99 releases
# i.e. minor versions and patches can not go above number 99.
[[ $((.sh.version)) -ge 20170000 ]] && [[ $((.sh.version)) -le 20990000 ]] || log_error "Version string is set incorrectly to $((.sh.version))"

# `.sh.pwdfd` variable points to file descriptor of current working directory
actual="$(pwd -f ${.sh.pwdfd})"
expect="$PWD"
[[ "$actual" = "$expect" ]] || log_error ".sh.pwdfd should point to fd of current working directory"
