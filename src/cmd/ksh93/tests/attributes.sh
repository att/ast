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

r=readonly u=Uppercase l=Lowercase i=22 i8=10 L=abc L5=def uL5=abcdef xi=20
x=export t=tagged H=hostname LZ5=026 RZ5=026 Z5=123 lR5=ABcdef R5=def n=l
for option in u l i i8 L L5 LZ5 RZ5 Z5 r x H t R5 uL5 lR5 xi n
do
    typeset -$option $option
done

(r=newval) && log_error readonly attribute fails
i=i+5
if ((i != 27))
then
    log_error integer attributes fails
fi

if [[ $i8 != 8#12 ]]
then
    log_error integer base 8 fails
fi

if [[ $u != UPPERCASE ]]
then
    log_error uppercase fails
fi

if [[ $l != lowercase ]]
then
    log_error lowercase fails
fi

if [[ $n != lowercase ]]
then
    log_error reference variables fail
fi

if [[ t=tagged != $(typeset -t) ]]
then
    log_error tagged fails
fi

if [[ t != $(typeset +t) ]]
then
    log_error tagged fails
fi

if [[ $Z5 != 00123 ]]
then
    log_error zerofill fails
fi

if [[ $RZ5 != 00026 ]]
then
    log_error right zerofill fails
fi

L=12345
if [[ $L != 123 ]]
then
    log_error leftjust fails
fi

if [[ $L5 != "def  " ]]
then
    log_error leftjust fails
fi

if [[ $uL5 != ABCDE ]]
then
    log_error leftjust uppercase fails
fi

if [[ $lR5 != bcdef ]]
then
    log_error rightjust fails
fi

if [[ $R5 != "  def" ]]
then
    log_error rightjust fails
fi

if [[ $($SHELL -c 'echo $x') != export ]]
then
    log_error export fails
fi

if [[ $($SHELL -c 'xi=xi+4;echo $xi') != 24 ]]
then
    log_error export attributes fails
fi

x=$(foo=abc $SHELL <<!
    foo=bar
    $SHELL -c  'print \$foo'
!
)
if [[ $x != bar ]]
then
    log_error 'environment variables require re-export'
fi

# This corresponds to the parenthetical statement below in the ksh documentation:
#
#   If no vname arguments are given, a list of vnames (and optionally the values) of the variables
#   is printed. (Using + rather than - keeps the values from being printed.)
#
# That is, `typeset +` is expected to emit a list of var names without their values.
(typeset + | grep '^SHELL$' >typeset_plus.out) || log_error 'typeset + not working'
[[ -s typeset_plus.out ]] || log_error 'typeset + not working' 'SHELL' "$(< typeset_plus.out)"

(typeset -L-5 buf="A")
if [[ $? == 0 ]]
then
    log_error 'typeset allows negative field for left/right adjust'
fi

a=b
readonly $a=foo
if [[ $b != foo ]]
then
    log_error 'readonly $a=b not working'
fi

if [[ $(export | grep '^PATH=') != PATH=* ]]
then
    log_error 'export not working'
fi

picture=(
    bitmap=/fruit
    size=(typeset -E x=2.5)
)
string="$(print $picture)"
if [[ "${string}" != *'size=( typeset -E'* ]]
then
    log_error 'print of compound exponential variable not working'
fi

sz=(typeset -E y=2.2)
string="$(print $sz)"
if [[ "${sz}" == *'typeset -E -F'* ]]
then
     log_error 'print of exponential shows both -E and -F attributes'
fi

print 'typeset -i m=48/4+1;print -- $m' > $TEST_DIR/script
chmod +x $TEST_DIR/script
typeset -Z2 m
if [[ $($TEST_DIR/script) != 13 ]]
then
    log_error 'attributes not cleared for script execution'
fi

print 'print VAR=$VAR' > $TEST_DIR/script
typeset -L70 VAR=var
$TEST_DIR/script > $TEST_DIR/script.1
[[ $(< $TEST_DIR/script.1) == VAR= ]] || log_error 'typeset -L should not be inherited'
typeset -Z  LAST=00
unset -f foo
function foo
{
        if [[ $1 ]]
        then
    LAST=$1
        else    ((LAST++))
        fi

}
foo 1
if (( ${#LAST} != 2 ))
then
    log_error 'LAST!=2'
fi

foo
if (( ${#LAST} != 2 ))
then
    log_error 'LAST!=2'
fi

[[ $(set | grep LAST) == LAST=02 ]] || log_error "LAST not correct in set list"
set -a
unset foo
foo=bar
if [[ $(export | grep ^foo=) != 'foo=bar' ]]
then
    log_error 'all export not working'
fi

unset foo
read foo <<!
bar
!
if [[ $(export | grep ^foo=) != 'foo=bar' ]]
then
    log_error 'all export not working with read'
fi

if [[ $(typeset | grep PS2) == PS2 ]]
then
    log_error 'typeset without arguments outputs names without attributes'
fi

unset a z q x
w1=hello
w2=world
t1="$w1 $w2"
b1=aGVsbG8gd29ybGQ=
b2=aGVsbG8gd29ybGRoZWxsbyB3b3JsZA==

z=$b1
typeset -b x=$b1
[[ $x == "$z" ]] || print -u2 'binary variable not expanding correctly'

[[  $(printf "%B" x) == $t1 ]] || log_error 'typeset -b not working'
typeset -b -Z5 a=$b1

[[  $(printf "%B" a) == $w1 ]] || log_error 'typeset -b -Z5 not working'

typeset -b q=$x$x
[[ $q == $b2 ]] || log_error 'typeset -b not working with concatination'
[[  $(printf "%B" q) == $t1$t1 ]] || log_error 'typeset -b concatination not working'

x+=$b1
[[ $x == $b2 ]] || log_error 'typeset -b not working with append'
[[  $(printf "%B" x) == $t1$t1 ]] || log_error 'typeset -b append not working'

typeset -b -Z20 z=$b1
(( $(printf "%B" z | wc -c) == 20 )) || log_error 'typeset -b -Z20 not storing 20 bytes'
{
    typeset -b v1 v2
    read -N11 v1
    read -N22 v2
} << !
hello worldhello worldhello world
!
[[ $v1 == "$b1" ]] || log_error "v1=$v1 should be $b1"
[[ $v2 == "$x" ]] || log_error "v1=$v2 should be $x"
if env '!=1' >/dev/null
then
    [[ $(env '!=1' $SHELL -c 'echo ok') == ok ]] ||
        log_error 'malformed environment terminates shell'
fi

unset var
typeset -b var
printf '12%Z34' | read -r -N 5 var
[[ $var == MTIAMzQ= ]] || log_error 'binary files with zeros not working'
unset var
if command typeset -usi var=0xfffff
then
    (( $var == 0xffff )) || log_error 'unsigned short integers not working'
else
    log_error 'typeset -usi cannot be used for unsigned short'
fi

[[ $($SHELL -c 'unset foo;typeset -Z2 foo; print ${foo:-3}') == 3 ]]  || log_error  '${foo:-3} not 3 when typeset -Z2 field undefined'
[[ $($SHELL -c 'unset foo;typeset -Z2 foo; print ${foo:=3}') == 03 ]]  || log_error  '${foo:=3} not 3 when typeset -Z2 foo undefined'

unset foo bar
unset -f fun
function fun
{
    export foo=hello
    typeset -x  bar=world
    [[ $foo == hello ]] || log_error 'export scoping problem in function'
}
fun
[[ $(export | grep ^foo=) == 'foo=hello' ]] || log_error 'export not working in functions'
[[ $(export | grep ^bar=) ]] && log_error 'typeset -x not local'
[[ $($SHELL -c 'typeset -r IFS=;print -r $(pwd)') == "$(pwd)" ]] ||
    log_error 'readonly IFS causes command substitution to fail'

fred[66]=88
[[ $(typeset -pa) == *fred* ]] || log_error 'typeset -pa not working'

unset x y z
typeset -LZ3 x=abcd y z=00abcd
y=03
[[ $y == "3  " ]] || log_error '-LZ3 not working for value 03'
[[ $x == "abc" ]] || log_error '-LZ3 not working for value abcd'
[[ $x == "abc" ]] || log_error '-LZ3 not working for value 00abcd'

unset x z
set +a
[[ $(typeset -p z) ]] && log_error "typeset -p for z undefined failed"

unset z
x='typeset -i z=45'
eval "$x"
[[ $(typeset -p z) == "$x" ]] || log_error "typeset -p for '$x' failed"
[[ $(typeset +p z) == "${x%=*}" ]] || log_error "typeset +p for '$x' failed"

unset z
x='typeset -a z=(a b c)'
eval "$x"
[[ $(typeset -p z) == "$x" ]] || log_error "typeset -p for '$x' failed"
[[ $(typeset +p z) == "${x%=*}" ]] || log_error "typeset +p for '$x' failed"

unset z
x='typeset -C z=(
foo=bar
xxx=bam
)'
eval "$x"
x=${x//$'\t'}
x=${x//$'(\n'/'('}
x=${x//$'\n'/';'}
x=${x%';)'}')'
[[ $(typeset -p z) == "$x" ]] || log_error "typeset -p for '$x' failed"
[[ $(typeset +p z) == "${x%%=*}" ]] || log_error "typeset +p for '$x' failed"

unset z
x='typeset -A z=([bar]=bam [xyz]=bar)'
eval "$x"
[[ $(typeset -p z) == "$x" ]] || log_error "typeset -p for '$x' failed"
[[ $(typeset +p z) == "${x%%=*}" ]] || log_error "typeset +p for '$x' failed"

unset z
foo=abc
x='typeset -n z=foo'
eval "$x"
[[ $(typeset -p z) == "$x" ]] || log_error "typeset -p for '$x' failed"
[[ $(typeset +p z) == "${x%%=*}" ]] || log_error "typeset +p for '$x' failed"

typeset +n z
unset foo z
typeset -T Pt_t=(
    float x=1 y=2
)
Pt_t z
x=${z//$'\t'}
x=${x//$'(\n'/'('}
x=${x//$'\n'/';'}
x=${x%';)'}')'
[[ $(typeset -p z) == "Pt_t z=$x" ]] || log_error "typeset -p for type failed"
[[ $(typeset +p z) == "Pt_t z" ]] || log_error "typeset +p for type failed"

unset z
function foo
{
    typeset -p bar
}
bar=xxx
[[ $(foo) == bar=xxx ]] || log_error 'typeset -p not working inside a function'
unset foo
typeset -L5 foo
[[ $(typeset -p foo) == 'typeset -L 5 foo' ]] || log_error 'typeset -p not working for variables with attributes but without a value'
{ $SHELL  <<- EOF
    typeset -L3 foo=aaa
    typeset -L6 foo=bbbbbb
    [[ \$foo == bbbbbb ]]
EOF
}  || log_error 'typeset -L should not preserve old attributes'
{ $SHELL <<- EOF
    typeset -R3 foo=aaa
    typeset -R6 foo=bbbbbb
    [[ \$foo == bbbbbb ]]
EOF
} || log_error 'typeset -R should not preserve old attributes'

expected='YWJjZGVmZ2hpag=='
unset foo
typeset -b -Z10 foo
read foo <<< 'abcdefghijklmnop'
[[ $foo == "$expected" ]] || log_error 'read foo, where foo is "typeset -b -Z10" not working'

unset foo
typeset -b -Z10 foo
read -N10 foo <<< 'abcdefghijklmnop'
[[ $foo == "$expected" ]] || log_error 'read -N10 foo, where foo is "typeset -b -Z10" not working'

unset foo
typeset  -b -A foo
read -N10 foo[4] <<< 'abcdefghijklmnop'
[[ ${foo[4]} == "$expected" ]] || log_error 'read -N10 foo, where foo is "typeset  -b -A" foo not working'

unset foo
typeset  -b -a foo
read -N10 foo[4] <<< 'abcdefghijklmnop'
[[ ${foo[4]} == "$expected" ]] || log_error 'read -N10 foo, where foo is "typeset  -b -a" foo not working'
[[ $(printf %B foo[4]) == abcdefghij ]] || log_error 'printf %B for binary associative array element not working'
[[ $(printf %B foo[4]) == abcdefghij ]] || log_error 'printf %B for binary indexed array element not working'
unset foo

$SHELL -c 'export foo=(bar=3)' && log_error 'compound variables cannot be exported'

$SHELL -c 'builtin date' &&
{

# check env var changes against a builtin that uses the env var

SEC=1234252800
ETZ=EST5EDT
EDT=03
PTZ=PST8PDT
PDT=00

CMD="date -f%H \\#$SEC"

export TZ=$ETZ

set -- \
    "$EDT $PDT $EDT"    ""        "TZ=$PTZ"    "" \
    "$EDT $PDT $EDT"    ""        "TZ=$PTZ"    "TZ=$ETZ" \
    "$EDT $PDT $EDT"    "TZ=$ETZ"    "TZ=$PTZ"    "TZ=$ETZ" \
    "$PDT $EDT $PDT"    "TZ=$PTZ"    ""        "TZ=$PTZ" \
    "$PDT $EDT $PDT"    "TZ=$PTZ"    "TZ=$ETZ"    "TZ=$PTZ" \
    "$EDT $PDT $EDT"    "foo=bar"    "TZ=$PTZ"    "TZ=$ETZ" \

while    (( $# >= 4 ))
do
    expect=$1
    actual=$(print $($SHELL -c "builtin date; $2 $CMD; $3 $CMD; $4 $CMD"))
    [[ $actual == $expect ]] ||
        log_error "[ '$2'  '$3'  '$4' ] env sequence failed" "$expect" "$actual"
    shift 4
done


}

unset v
typeset -H v=/dev/null
[[ $v == *nul* ]] || log_error 'typeset -H for /dev/null not working'

unset x
(typeset +C x) && log_error 'typeset +C should be an error'
(typeset +A x) && log_error 'typeset +A should be an error'
(typeset +a x) && log_error 'typeset +a should be an error'

unset x
{
x=$($SHELL -c 'integer -s x=5;print -r -- $x')
}
[[ $x == 5 ]] || log_error 'integer -s not working'

[[ $(typeset -l) == *namespace*.sh* ]] && log_error 'typeset -l should not contain namespace .sh'

unset actual
expect=100
((actual=$expect))
[[ $actual == $expect ]] || log_error "typeset -l fails on numeric value" "$expect" "$actual"

unset s
typeset -a -u s=( hello world chicken )
[[ ${s[2]} == CHICKEN ]] || log_error 'typeset -u not working with indexed arrays'
unset s
typeset -A -u s=( [1]=hello [0]=world [2]=chicken )
[[ ${s[2]} == CHICKEN ]] || log_error 'typeset -u not working with associative arrays'
expected=$'(\n\t[0]=WORLD\n\t[1]=HELLO\n\t[2]=CHICKEN\n)'
[[ $(print -v s) == "$expected" ]] || log_error 'typeset -u for associative array does not display correctly'

unset s
if command typeset -M totitle s
then
    [[ $(typeset +p s) == 'typeset -M totitle s' ]] || log_error 'typeset -M totitle does not display correctly with typeset -p'
fi


{ $SHELL  <<-  \EOF
    compound -a a1
    for ((i=1 ; i < 100 ; i++ ))
        do
    [[ "$( typeset + a1[$i] )" == '' ]] && a1[$i].text='hello'
    done

    [[ ${a1[70].text} == hello ]]
EOF
}
(( $? )) && log_error  'typeset + a[i] not working'

typeset groupDB="" userDB=""
typeset -l -L1 DBPick=""
[[ -n "$groupDB" ]]  && log_error 'typeset -l -L1 causes unwanted side effect'

HISTFILE=foo
typeset -u PS1='hello --- '
HISTFILE=foo
[[ $HISTFILE == foo ]] || log_error  'typeset -u PS1 affects HISTFILE'

typeset -a a=( aA= ZQ= bA= bA= bw= Cg= )
typeset -b x
for     (( i=0 ; i < ${#a[@]} ; i++ ))
do
     x+="${a[i]}"
done

expect="hello"
actual="$(printf "%B" x)"
[[ $actual == $expect ]] || log_error "append for typeset -b not working" "$expect" "$actual"

(
    trap 'exit $?' EXIT
    $SHELL -c 'typeset v=foo; [[ $(typeset -p v[0]) == v=foo ]]'
) || log_error 'typeset -p v[0] not working for simple variable v'

unset x
expected='typeset -a x=(a\=3 b\=4)'
typeset -a x=( a=3 b=4)
[[ $(typeset -p x) == "$expected" ]] || log_error 'assignment elements in typeset -a assignment not working'

# We need to unset `push_stack` because it is defined automatically in each shell instance.
expect='typeset -a q=(a b c)'
actual=$($SHELL -c "$expect; unset _push_stack; typeset -pa")
[[ "$expect" == "$actual" ]] || 
    log_error 'typeset -pa does not list only index arrays' "$expect" "$actual"

expect='typeset -C z=(foo=bar)'
actual=$($SHELL -c "$expect; typeset -pC")
[[ "$expect" == "$actual" ]] ||
    log_error 'typeset -pC does not list only compound variables' "$expect" "$actual"

expect='typeset -A y=([a]=foo)'
actual=$($SHELL -c "$expect; typeset -pA");
[[ "$expect" == "$actual" ]] ||
    log_error 'typeset -pA does not list only associative arrays' "$expect" "$actual"

$SHELL -c 'typeset -C arr=( aa bb cc dd )' &&
    log_error 'invalid compound variable assignment not reported'

unset x
typeset -l x=
[[ ${x:=foo} == foo ]] || log_error '${x:=foo} with x unset, not foo when x is a lowercase variable'

unset x
typeset -L4 x=$'\001abcdef'
[[ ${#x} == 5 ]] || log_error "width of character '\01' is not zero"

unset x
typeset -L x=-1
command typeset -F x=0-1 || log_error 'typeset -F after typeset -L fails'

unset val
typeset -i val=10#0-3
typeset -Z val=0-1
[[ $val == 0-1 ]] || log_error 'integer attribute not cleared for subsequent typeset'

unset x
typeset -L -Z x=foo
[[ $(typeset -p x) == 'typeset -Z 3 -L 3 x=foo' ]] || log_error '-LRZ without [n] not defaulting to width of variable'

unset foo
typeset -Z2 foo=3
[[ $(typeset -p foo) == 'typeset -Z 2 -R 2 foo=03' ]] || log_error '-Z2  not working'
export foo
[[ $(typeset -p foo) == 'typeset -x -Z 2 -R 2 foo=03' ]] || log_error '-Z2  not working after export'

unset foo
export foo=/C/TEMP
typeset -H expect=$foo
actual=$( $SHELL -c 'typeset -H foo;print -r -- "$foo')
[[ $actual == "$expect" ]] ||
    log_error 'typeset -H not working for export variables' "$expect" "$actual"

typeset -Z4 VAR1
VAR1=1
expect=$(typeset -p VAR1)
export VAR1
actual=$(typeset -p VAR1)
actual=${actual/ -x/}
[[ $actual == "$expect" ]] ||
    log_error 'typeset -x causes zerofill width to change' "$expect" "$actual"

unset var
typeset -bZ6 var
for i in 2 3
do
    read -r -N6 var
    [[ $var == dHdvdG93 ]] &&  ((i !=2)) && log_error 'loop optimization bug with typeset -b variables'
done <<< 'twotowthreetfourro'

# https://github.com/att/ast/issues/537
unset foo bar
# Export each variable that gets assigned
set -a
foo=${bar:=baz}
env | grep -q bar || log_error 'Variable bar should be exported'
set +a
