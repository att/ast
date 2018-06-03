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

function fun
{
    integer i
    unset xxx
    for i in 0 1
    do    xxx[$i]=$i
    done
}

set -A x zero one two three four 'five six'
if [[ $x != zero ]]
then
    log_error '$x is not element 0'
fi

if [[ ${x[0]} != zero ]]
then
    log_error '${x[0] is not element 0'
fi

if (( ${#x[0]} != 4 ))
then
    log_error "length of ${x[0]} is not 4"
fi

if (( ${#x[@]} != 6  ))
then
    log_error 'number of elements of x is not 6'
fi

if [[ ${x[2]} != two  ]]
then
    log_error ' element two is not 2'
fi

if [[ ${x[@]:2:1} != two  ]]
then
    log_error ' ${x[@]:2:1} is not two'
fi

set -A y -- ${x[*]}
if [[ $y != zero ]]
then
    log_error '$x is not element 0'
fi

if [[ ${y[0]} != zero ]]
then
    log_error '${y[0] is not element 0'
fi

if (( ${#y[@]} != 7  ))
then
    log_error 'number of elements of y is not 7'
fi

if [[ ${y[2]} != two  ]]
then
    log_error ' element two is not 2'
fi

set +A y nine ten
if [[ ${y[2]} != two  ]]
then
    log_error ' element two is not 2'
fi

if [[ ${y[0]} != nine ]]
then
    log_error '${y[0] is not nine'
fi

unset y[4]
if (( ${#y[@]} != 6  ))
then
    log_error 'number of elements of y is not 6'
fi

if (( ${#y[4]} != 0  ))
then
    log_error 'string length of unset element is not 0'
fi

unset foo
if (( ${#foo[@]} != 0  ))
then
    log_error 'number of elements of unset variable foo is not 0'
fi

foo=''
if (( ${#foo[0]} != 0  ))
then
    log_error 'string length of null element is not 0'
fi

if (( ${#foo[@]} != 1  ))
then
    log_error 'number of elements of null variable foo is not 1'
fi

unset foo
foo[0]=foo
foo[3]=bar
unset foo[0]
unset foo[3]
if (( ${#foo[@]} != 0  ))
then
    log_error 'number of elements of left in variable foo is not 0'
fi

unset foo
foo[3]=bar
foo[0]=foo
unset foo[3]
unset foo[0]
if (( ${#foo[@]} != 0  ))
then
    log_error 'number of elements of left in variable foo again is not 0'
fi

fun
if (( ${#xxx[@]} != 2  ))
then
    log_error 'number of elements of left in variable xxx is not 2'
fi

fun
if (( ${#xxx[@]} != 2  ))
then
    log_error 'number of elements of left in variable xxx again is not 2'
fi

set -A foo -- "${x[@]}"
if (( ${#foo[@]} != 6  ))
then
    log_error 'number of elements of foo is not 6'
fi

if (( ${#PWD[@]} != 1  ))
then
    log_error 'number of elements of PWD is not 1'
fi

unset x
x[2]=foo x[4]=bar
if (( ${#x[@]} != 2  ))
then
    log_error 'number of elements of x is not 2'
fi

s[1]=1 c[1]=foo
if [[ ${c[s[1]]} != foo ]]
then
    log_error 'c[1]=foo s[1]=1; ${c[s[1]]} != foo'
fi

unset s
typeset -Ai s
y=* z=[
s[$y]=1
s[$z]=2
if (( ${#s[@]} != 2  ))
then
    log_error 'number of elements of  is not 2'
fi

(( s[$z] = s[$z] + ${s[$y]} ))
if [[ ${s[$z]} != 3  ]]
then
    log_error '[[ ${s[$z]} != 3  ]]'
fi

if (( s[$z] != 3 ))
then
    log_error '(( s[$z] != 3 ))'
fi

(( s[$y] = s[$y] + ${s[$z]} ))
if [[ ${s[$y]} != 4  ]]
then
    log_error '[[ ${s[$y]} != 4  ]]'
fi

if (( s[$y] != 4 ))
then
    log_error '(( s[$y] != 4 ))'
fi

set -A y 2 4 6
typeset -i y
z=${y[@]}
typeset -R12 y
typeset -i y
if   [[ ${y[@]} != "$z" ]]
then
    log_error 'error in array conversion from int to R12'
fi

if   (( ${#y[@]} != 3  ))
then
    log_error 'error in count of array conversion from int to R12'
fi

unset abcdefg
:  ${abcdefg[1]}
set | grep '^abcdefg$' >/dev/null && log_error 'empty array variable in set list'
unset x y
x=1
typeset -i y[$x]=4
if [[ ${y[1]} != 4 ]]
then
    log_error 'arithmetic expressions in typeset not working'
fi

unset foo
typeset foo=bar
typeset -A foo
if [[ ${foo[0]} != bar ]]
then
    log_error 'initial value not preserved when typecast to associative'
fi

unset foo
foo=(one two)
typeset -A foo
foo[two]=3
if [[ ${#foo[*]} != 3 ]]
then
    log_error 'conversion of indexed to associative array failed'
fi

set a b c d e f g h i j k l m
if [[ ${#} != 13 ]]
then
    log_error '${#} not 13'
fi

unset xxx
xxx=foo
if [[ ${!xxx[@]} != 0 ]]
then
    log_error '${!xxx[@]} for scalar not 0'
fi

if [[ ${11} != k ]]
then
    log_error '${11} not working'
fi

if [[ ${@:4:1} != d ]]
then
    log_error '${@:4:1} not working'
fi

foovar1=abc
foovar2=def
if [[ ${!foovar@} != +(foovar[[:alnum:]]?([ ])) ]]
then
    log_error '${!foovar@} does not expand correctly'
fi

if [[ ${!foovar1} != foovar1 ]]
then
    log_error '${!foovar1} != foovar1'
fi

unset xxx
: ${xxx[3]}
if [[ ${!xxx[@]} ]]
then
    log_error '${!xxx[@]} should be null'
fi

integer i=0
{
    set -x
    xxx[++i]=1
    set +x
} 2> /dev/null
if (( i != 1))
then
    log_error 'execution trace side effects with array subscripts'
fi

unset list
: $(set -A list foo bar)
if (( ${#list[@]} != 0))
then
    log_error '$(set -A list ...) leaves side effects'
fi

unset list
list= (foo bar bam)
( set -A list one two three four)
if [[ ${list[1]} != bar ]]
then
    log_error 'array not restored after subshell'
fi

XPATH=/bin:/usr/bin:/usr/ucb:/usr/local/bin:.:/sbin:/usr/sbin
xpath=( $( IFS=: ; echo $XPATH ) )
if [[ $(print -r  "${xpath[@]##*/}") != 'bin bin ucb bin . sbin sbin' ]]
then
    log_error '${xpath[@]##*/} not applied to each element'
fi

foo=( zero one '' three four '' six)
integer n=-1
if [[ ${foo[@]:n} != six ]]
then
    log_error 'array offset of -1 not working'
fi

if [[ ${foo[@]: -3:1} != four ]]
then
    log_error 'array offset of -3:1 not working'
fi

$SHELL -c 'x=(if then else fi)' 2> /dev/null  || log_error 'reserved words in x=() assignment not working'
unset foo
foo=one
foo=( $foo two)
if [[ ${#foo[@]} != 2 ]]
then
    log_error 'array getting unset before right hand side evaluation'
fi

foo=(143 3643 38732)
export foo
typeset -i foo
if [[ $($SHELL -c 'print $foo') != 143 ]]
then
    log_error 'exporting indexed array not exporting 0-th element'
fi

( $SHELL   -c '
    unset foo
    typeset -A foo=([0]=143 [1]=3643 [2]=38732)
    export foo
    typeset -i foo
    [[ $($SHELL -c "print $foo") == 143 ]]'
) 2> /dev/null ||
        log_error 'exporting associative array not exporting 0-th element'
unset foo
typeset -A foo
foo[$((10))]=ok 2> /dev/null || log_error 'arithmetic expression as subscript not working'
unset foo
typeset -A foo
integer foo=0
[[ $foo == 0 ]] || log_error 'zero element of associative array not being set'
unset foo
typeset -A foo=( [two]=1)
for i in one three four five
do    : ${foo[$i]}
done

if [[ ${!foo[@]} != two ]]
then
    log_error 'error in subscript names'
fi

unset x
x=( 1 2 3)
(x[1]=8)
[[ ${x[1]} == 2 ]] || log_error 'index array produce side effects in subshells'
x=( 1 2 3)
(
    x+=(8)
    [[ ${#x[@]} == 4 ]] || log_error 'index array append in subshell error'
)
[[ ${#x[@]} == 3 ]] || log_error 'index array append in subshell effects parent'
x=( [one]=1 [two]=2 [three]=3)
(x[two]=8)
[[ ${x[two]} == 2 ]] || log_error 'associative array produce side effects in subshells'
unset x
x=( [one]=1 [two]=2 [three]=3)
(
    x+=( [four]=4 )
    [[ ${#x[@]} == 4 ]] || log_error 'associative array append in subshell error'
)
[[ ${#x[@]} == 3 ]] || log_error 'associative array append in subshell effects parent'
unset x
integer i
for ((i=0; i < 40; i++))
do
    x[i]=$i
done

[[  ${#x[@]} == 40 ]] || log_error 'index arrays loosing values'
[[ $( ($SHELL -c 'typeset -A var; (IFS=: ; set -A var a:b:c ;print ${var[@]});:' )2>/dev/null) == 'a b c' ]] || log_error 'change associative to index failed'
unset foo
[[ $(foo=good
for ((i=0; i < 2; i++))
do
    [[ ${foo[i]} ]] && print ok
done) == ok ]] || log_error 'invalid optimization for subscripted variables'
(
x=([foo]=bar)
set +A x bam
) 2> /dev/null && log_error 'set +A with associative array should be an error'
unset bam foo
foo=0
typeset -A bam
unset bam[foo]
bam[foo]=value
[[ $bam == value ]] && log_error 'unset associative array element error'
: only first element of an array can be exported
unset bam
print 'print ${var[0]} ${var[1]}' > $TEST_DIR/script
chmod +x $TEST_DIR/script
[[ $($SHELL -c "var=(foo bar);export var;$TEST_DIR/script") == foo ]] || log_error 'export array not exporting just first element'

unset foo
set --allexport
foo=one
foo[1]=two
foo[0]=three
[[ $foo == three ]] || log_error '--allexport not working with arrays'
set --noallexport
unset foo

cat > $TEST_DIR/script <<- \!
    typeset -A foo
    print foo${foo[abc]}
!
[[ $($SHELL -c "typeset -A foo;$TEST_DIR/script")  == foo ]] 2> /dev/null || log_error 'empty associative arrays not being cleared correctly before scripts'
[[ $($SHELL -c "typeset -A foo;foo[abc]=abc;$TEST_DIR/script") == foo ]] 2> /dev/null || log_error 'associative arrays not being cleared correctly before scripts'
unset foo
foo=(one two)
[[ ${foo[@]:1} == two ]] || log_error '${foo[@]:1} == two'
[[ ! ${foo[@]:2} ]] || log_error '${foo[@]:2} not null'
unset foo
foo=one
[[ ! ${foo[@]:1} ]] || log_error '${foo[@]:1} not null'
function EMPTY
{
        typeset i
        typeset -n ARRAY=$1
        for i in ${!ARRAY[@]}
        do      unset ARRAY[$i]
        done

}
unset foo
typeset -A foo
foo[bar]=bam
foo[x]=y
EMPTY foo
[[ $(typeset | grep foo$) == *associative* ]] || log_error 'array lost associative attribute'
[[ ! ${foo[@]}  ]] || log_error 'array not empty'
[[ ! ${!foo[@]}  ]] || log_error 'array names not empty'

unset foo
foo=bar
set -- "${foo[@]:1}"
(( $# == 0 )) || log_error '${foo[@]:1} should not have any values'

expect=4
: ${_foo[actual=4]}
(( actual == $expect )) || log_error "subscript of unset variable not evaluated" "$expect" "$actual"

expect=7
: ${_foo[actual=$expect]}
(( actual == $expect )) || log_error "subscript of unset variable not evaluated" "$expect" "$actual"

unset foo bar
foo[5]=4
bar[4]=3
bar[0]=foo
foo[0]=bam
foo[4]=5
[[ ${!foo[2+2]} == 'foo[4]' ]] || log_error '${!var[sub]} should be var[sub]'
[[ ${bar[${foo[5]}]} == 3 ]] || log_error  'array subscript cannot be an array instance'
[[ $bar[4] == 3 ]] || log_error '$bar[x] != ${bar[x]} inside [[ ]]'
(( $bar[4] == 3  )) || log_error '$bar[x] != ${bar[x]} inside (( ))'
[[ $bar[$foo[5]] == 3 ]]  || log_error '$bar[foo[x]] != ${bar[foo[x]]} inside [[ ]]'
(( $bar[$foo[5]] == 3  )) || log_error '$bar[foo[x]] != ${bar[foo[x]]} inside (( ))'
x=$bar[4]
[[ $x == 4 ]] && log_error '$bar[4] should not be an array in an assignment'
x=${bar[$foo[5]]}
(( $x == 3 )) || log_error '${bar[$foo[sub]]} not working'

[[ $($SHELL  <<- \++EOF+++
    typeset -i test_variable=0
    typeset -A test_array
    test_array[1]=100
    read test_array[2] <<-!
	2
	!
    read test_array[3] <<-!
	3
	!
    test_array[3]=4
    print "val=${test_array[3]}"
++EOF+++
) == "val=4" ]] 2> /dev/null || log_error 'after reading array[j] and assign array[j] fails'
[[ $($SHELL <<- \+++EOF+++
    pastebin=( typeset -a form)
    pastebin.form+=( name="name"   data="clueless" )
    print -r -- ${pastebin.form[0].name}
+++EOF+++
) == name ]] 2> /dev/null ||  log_error 'indexed array in compound variable not working'

unset foo bar
: ${foo[bar=2]}
[[ $bar == 2 ]] || log_error 'subscript not evaluated for unset variable'

unset foo bar
bar=1
typeset -a foo=([1]=ok [2]=no)
[[ $foo[bar] == ok ]] || log_error 'typeset -a not working for simple assignment'

unset foo
typeset -a foo=([1]=(x=ok) [2]=(x=no))
[[ $(typeset | grep 'foo$') == *index* ]] || log_error 'typeset -a not creating an indexed array'

foo+=([5]=good)
[[ $(typeset | grep 'foo$') == *index* ]] || log_error 'append to indexed array not preserving array type'

unset foo
typeset -A foo=([1]=ok [2]=no)
[[ $foo[bar] == ok ]] && log_error 'typeset -A not working for simple assignment'

unset foo
typeset -A foo=([1]=(x=ok) [2]=(x=no))
[[ ${foo[bar].x} == ok ]] && log_error 'typeset -A not working for compound assignment'
[[ $($SHELL -c 'typeset -a foo;typeset | grep "foo$"'  2> /dev/null) == *index* ]] || log_error 'typeset fails for indexed array with no elements'
xxxxx=(one)
[[ $(typeset | grep xxxxx$) == *'indexed array'* ]] || log_error 'array of one element not an indexed array'

unset foo
foo[1]=(x=3 y=4)
{ [[ ${!foo[1].*} == 'foo[1].x foo[1].y' ]] ;} 2> /dev/null || log_error '${!foo[sub].*} not expanding correctly'

unset x
x=( typeset -a foo=( [0]="a" [1]="b" [2]="c" ))

[[  ${@x.foo} == 'typeset -a'* ]] || log_error 'x.foo is not an indexed array'
x=( typeset -A foo=( [0]="a" [1]="b" [2]="c" ))

[[  ${@x.foo} == 'typeset -A'* ]] || log_error 'x.foo is not an associative array'

$SHELL -c $'x=(foo\n\tbar\nbam\n)' 2> /dev/null || log_error 'compound array assignment with new-lines not working'
$SHELL -c $'x=(foo\n\tbar:\nbam\n)' 2> /dev/null || log_error 'compound array assignment with labels not working'
$SHELL -c $'x=(foo\n\tdone\nbam\n)' 2> /dev/null || log_error 'compound array assignment with reserved words not working'
[[ $($SHELL -c 'typeset -A A; print $(( A[foo].bar ))' 2> /dev/null) == 0 ]] || log_error 'unset variable not evaluating to 0'

unset a
typeset -A a
a[a].z=1
a[z].z=2
unset a[a]
[[ ${!a[@]} == z ]] || log_error '"unset a[a]" unsets entire array'

unset a
a=([x]=1 [y]=2 [z]=(foo=3 bar=4))
eval "b=$(printf "%B\n" a)"
eval "c=$(printf "%#B\n" a)"
[[ ${a[*]} == "${b[*]}" ]] || log_error 'printf %B not preserving values for arrays'
[[ ${a[*]} == "${c[*]}" ]] || log_error 'printf %#B not preserving values for arrays'

unset a
a=(zero one two three four)
a[6]=six
[[ ${a[-1]} == six ]] || log_error 'a[-1] should be six'
[[ ${a[-3]} == four ]] || log_error 'a[-3] should be four'
[[ ${a[-3..-1]} == 'four six' ]] || log_error "a[-3,-1] should be 'four six'"

FILTER=(typeset scope)
FILTER[0].scope=include
FILTER[1].scope=exclude
[[ ${#FILTER[@]} == 2 ]] ||  log_error "FILTER array should have two elements not ${#FILTER[@]}"

unset x
function x.get
{
    print sub=${.sh.subscript}
}
x[2]=
z=$(: ${x[1]} )
[[ $z == sub=1 ]] || log_error 'get function not invoked for index array'

unset x
typeset -A x
function x.get
{
    print sub=${.sh.subscript}
}
x[2]=
z=$(: ${x[1]} )
[[ $z == sub=1 ]] || log_error 'get function not invoked for associative array'

unset y
i=1
a=(11 22)
typeset -m y=a[i]
[[ $y == 22 ]] || log_error 'typeset -m for index array not working'
[[ ${a[i]} || ${a[0]} != 11 ]] && log_error 'typeset -m for index array not deleting element'

unset y
a=([0]=11 [1]=22)
typeset -m y=a[$i]
[[ $y == 22 ]] || log_error 'typeset -m for associative array not working'
[[ ${a[$i]} || ${a[0]} != 11 ]] && log_error 'typeset -m for associative array not deleting element'
unset x a j

typeset -a a=( [0]="aa" [1]="bb" [2]="cc" )
typeset -m 'j=a[0]'
typeset -m 'a[0]=a[1]'
typeset -m 'a[1]=j'
[[ ${a[@]} == 'bb aa cc' ]] || log_error 'moving index array elements not working'
unset a j

typeset -A a=( [0]="aa" [1]="bb" [2]="cc" )
typeset -m 'j=a[0]'
typeset -m 'a[0]=a[1]'
typeset -m 'a[1]=j'
[[ ${a[@]} == 'bb aa cc' ]] || log_error 'moving associative array elements not working'
unset a j
[[ $(typeset -p a) ]] && log_error 'unset associative array after typeset -m not working'

z=(a b c)
unset x
typeset -m x[1]=z
[[ ${x[1][@]} == 'a b c' ]] || log_error 'moving indexed array to index array element not working'

unset x z
z=([0]=a [1]=b [2]=c)
typeset -m x[1]=z
[[ ${x[1][@]} == 'a b c' ]] || log_error 'moving associative array to index array element not working'

{
typeset -a arr=(
    float
)
} 2> /dev/null
[[ ${arr[0]} == float ]] || log_error 'typeset -a should not expand alias for float'
unset arr

{
typeset -r -a arr=(
    float
)
} 2> /dev/null
[[ ${arr[0]} == float ]] || log_error 'typeset -r -a should not expand alias for float'
{
typeset -a arr2=(
    typeset +r
)
} 2> /dev/null
[[ ${arr2[0]} == typeset ]] || log_error 'typeset -a should not process declarations'
unset arr2

$SHELL 2> /dev/null -c $'typeset -a arr=(\nfor)' || log_error 'typeset -a should allow reserved words as first argument'

$SHELL 2> /dev/null -c $'typeset -r -a arr=(\nfor)' || log_error 'typeset -r -a should allow reserved words as first argument'

typeset arr2[6]
[[ ${#arr2[@]} == 0 ]] || log_error 'declartion "typeset array[6]" should not show any elements'

arr2[1]=def
[[ ${arr2[1]} == def ]] || log_error 'declaration "typeset array[6]" causes arrays causes wrong side effects'

unset foo
typeset foo[7]
[[ ${#foo[@]} == 0 ]] || log_error 'typeset foo[7] should not have one element'

a=123 $SHELL  2> /dev/null -c 'integer a[5]=3 a[2]=4; unset a;x=0; ((a[++x]++));:' || log_error 'unsetting array variable leaves side effect'

unset foo
foo=(aa bb cc)
foo=( ${foo[@]:1} )
[[ ${foo[@]} == 'bb cc' ]] || log_error "indexed array assignment using parts of array for values gives wrong result of ${foo[@]}"

unset foo
foo=([xx]=aa [yy]=bb [zz]=cc)
foo=( ${foo[yy]} ${foo[zz]} )
[[ ${foo[@]} == 'bb cc' ]] || log_error "associative array assignment using parts of array for values gives wrong result of ${foo[@]}"

unset foo
typeset -a foo=(abc=1 def=2)
[[ ${foo[1]} == def=2 ]] || log_error "index array with elements containing = not working"

unset foo
typeset -a foo=( a b )
typeset -p foo[10]
[[ ${!foo[@]} == '0 1' ]] || log_error 'typeset -p foo[10] has side effect'

unset foo
exp='typeset -a foo=((11 22) (66) )'
x=$(
    typeset -a foo=( ( 11 22 ) ( 44 55 ) )
    foo[1]=(66)
    typeset -p foo
) 2> /dev/null
[[ $x == "$exp" ]] || log_error 'setting element 1 to index fooay failed'
unset foo
exp='typeset -a foo=((11 22) (x=3))'
x=$(
    typeset -a foo=( ( 11 22 ) ( 44 55 ) )
    foo[1]=(x=3)
    typeset -p foo
) 2> /dev/null
[[ $x == "$exp" ]] || log_error 'setting element 1 of array to compound variable failed'

# Test for cloning a very large index array.
(
    trap 'x=$?; exit $(( $x != 0 ))' EXIT
    $SHELL <<- \EOF
    (
        print '('
        integer i
        for ((i=0 ; i < 16384 ; i++ )) ; do
                    printf '\tinteger var%i=%i\n' i i
        done

        printf 'typeset -a ar=(\n'

        for ((i=0 ; i < 16384 ; i++ )) ; do
            printf '\t[%d]=%d\n' i i
        done

        print ')'
        print ')'
    ) | read -C hugecpv
    compound hugecpv2=hugecpv
    v=$(typeset -p hugecpv)
    [[ ${v/hugecpv/hugecpv2} == "$(typeset -p hugecpv2)" ]]
EOF
) || log_error 'copying a large array fails'

unset foo
typeset -a foo
foo+=(bar)
[[ ${foo[0]} == bar ]] || 'appending to empty array not working'

unset isnull
typeset -A isnull
isnull[mdapp]=Y
: ${isnull[@]}
isnull[mdapp]=N
[[ ${isnull[*]} != *N* ]] && log_error 'bug after ${arr[@]} with one element associative array'

unset arr2
arr2=()
typeset -A arr2
unset arr2
[[ $(typeset -p arr2) ]] && log_error 'unset associative array of compound variables not working'

arr3=(x=3)
typeset -A arr3
[[  $(typeset -p arr3) == 'typeset -A arr3=()' ]] || log_error 'typeset -A does not first unset compound variable.'

arr4=(x=3)
typeset -a arr4
[[  $(typeset -p arr4) == 'typeset -a arr4' ]] || log_error 'typeset -a does not first unset compound variable.'

alias foo=bar
arr5=(foo bar)
[[ $(typeset -p arr5) == 'typeset -a arr5=(foo bar)' ]] || log_error 'typeset expanding non-declaration aliases'

typeset -A Foo
Foo=( [a]=AA;[b]=BB)
[[ ${Foo[a]} == AA ]] || log_error 'Fooa[a] is {Foo[a]} not AA'

$SHELL 2> /dev/null  <<- \+++ || log_error '${ar[${a}..${b}]} not working'
    typeset -a ar=([0]=a [1]=b [2]=c)
    integer a=1 b=2
    [[ ${ar[${a}..${b}]} == 'b c' ]]
+++

unset A
integer -A A
(( A[a] ))
[[ ${!A[@]} ]] &&  log_error '(( A[a] )) should not create element a of A'

actual=$($SHELL 2> /dev/null -c 'integer n=0; read  a[n++]<<<foo;read  a[n++]<<<bar;typeset -p a')
exitval=$?
(( exitval == 0))  || log_error "read a[n++] has bad exit value $exitval"
expect='typeset -a a=(foo bar)'
[[ $actual == "$expect" ]] || log_error "read a[n++] yields wrong answer" "$expect" "$actual"

integer -a ar
ar+=( 4 )
ar+=( 5 6  )
exp=$'(\n\t4\n\t5\n\t6\n)'
[[ $(print -v ar) == "$exp" ]] || log_error 'print -v not working for integer arrays'

unset ar b
IFS=$'\n' read -rd '' -A ar <<< $'a\nb\nc\nd\ne\nf'
b=(a b c d e f '')
[[ $(print -v ar) == "$(print -v b)" ]] || log_error 'read -d"" with IFS=\n not working'

$SHELL 2> /dev/null -c 'a=(foo bar); [[ $(typeset -a) == *"a=("*")"* ]]' || log_error '"typeset -a" not working'

ar=(9 11 6 3.5 22)
set -s -A ar
[[ $(typeset -p ar) == *'(11 22 3.5 6 9)' ]] || log_error 'set -s -A ar failed'
set -s -Aar -K:n
[[ $(typeset -p ar) == *'(3.5 6 9 11 22)' ]] || log_error 'set -s -A -Kn ar failed'
set -s -Aar -K:r
[[ $(typeset -p ar) == *'(9 6 3.5 22 11)' ]] || log_error 'set -s -A -Kr ar failed'
set -s -Aar -K:nr
[[ $(typeset -p ar) == *'(22 11 9 6 3.5)' ]] || log_error 'set -s -A -Knr ar failed'
unset ar[3]
set -s -Aar
[[ $(typeset -p ar) == *'(11 22 3.5 9)' ]] || log_error 'set -s -A ar failed with elemet 3 deleted'

typeset -A ar=( ["@"]=1 ["*"]=2 ["!"]=3 ["$"]=4 ["|"]=5 ["'"]=6 ["&"]=7 ["#"]=8 ["["]=9 ["]"]=10 )
exp="typeset -A ar=(['!']=3 ['#']=8 ['\$']=4 ['&']=7 [\$'\'']=6 ['*']=2 ['@']=1 ['[']=9 [']']=10 ['|']=5)"
[[ $(typeset -p ar) == "$exp" ]] || log_error 'associative array quoting error'

unset ar
ar=(foo bar bam)
ar=()
[[ $(typeset -p ar) == 'typeset -a ar' ]] || log_error 'ar=() does not preserve index array type'

unset ar
typeset -A ar=([0]=foo [1]=bar [2]=bam)
ar=()
[[ $(typeset -p ar) == 'typeset -A ar=()' ]] || log_error 'ar=() does not preserve associative array type'

unset ar
typeset -CA ar
ar[1]=(foo=1; bar=2)
ar[3]=(foo=3; bar=4)
ar=()
[[ $(typeset -p ar) == 'typeset -C -A ar=()' ]] || log_error 'ar=()  does not preserve -C attribute'

unset ar
ar=(foo bar bam)
ar=()
[[ $(typeset -p ar) == 'typeset -a ar' ]] || log_error 'ar=() for index array should preserve index array type'

unset ar
typeset -A ar=([1]=foo [3]=bar)
ar=()
[[ $(typeset -p ar) == 'typeset -A ar=()' ]] || log_error 'ar=() for associative array should preserve index array type'

unset ar
integer -a ar=( 2 3 4 )
ar=()
[[ $(typeset -p ar) == 'typeset -a -l -i ar' ]] || log_error 'ar=() for index array should preserve attributes'

unset ar
integer -a ar=( 2 3 4 )
integer -A ar=([1]=9 [3]=12)
ar=()
[[ $(typeset -p ar) == 'typeset -A -l -i ar=()' ]] || log_error 'ar=() for associative array should preserve attributes'

unset foo bar
typeset -a foo=([1]=w [2]=x) bar=(a b c)
foo+=("${bar[@]}")
[[ $(typeset -p foo) == 'typeset -a foo=([1]=w [2]=x [3]=a [4]=b [5]=c)' ]] || log_error 'Appending does not work if array contains empty indexes'
