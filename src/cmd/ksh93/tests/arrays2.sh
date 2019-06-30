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

for ((i=0; i < 4; i++ ))
do
    for ((j=0; j < 5; j++ ))
    do
        a[i][j]=$i$j
    done
done

for ((i=0; i < 4; i++ ))
do
    for ((j=0; j < 5; j++ ))
    do
        [[ ${a[i][j]} == "$i$j" ]] || log_error "\${a[$i][$j]} != $i$j"
    done
done

for ((i=0; i < 4; i++ ))
do
    j=0;for k in ${a[i][@]}
    do
        [[ $k == "$i$j" ]] || log_error "\${a[i][@]} != $i$j"
        (( j++ ))
    done
done

unset a
a=(
    ( 00 01 02 03 04 )
    ( 10 11 12 13 14 15)
    ( 20 21 22 23 24 )
    ( 30 31 32 33 34 )
)

function check
{
    nameref a=$1
    nameref b=a[2]
    typeset c=$1
    integer i j
    for ((i=0; i < 4; i++ ))
    do
        for ((j=0; j < 5; j++ ))
        do
            [[ ${a[$i][$j]} == "$i$j" ]] || log_error "\${$c[$i][$j]} != $i$j"
        done
    done

    (( ${#a[@]} == 4 )) || log_error "\${#$c[@]} not 4"
    (( ${#a[0][@]} == 5 )) || log_error "\${#$c[0][@]} not 5"
    (( ${#a[1][@]} == 6 )) || log_error "\${#$c[1][@]} not 6"
    set -s -- ${!a[@]}
    [[ ${@} == '0 1 2 3' ]] || log_error "\${!$c[@]} not 0 1 2 3"
    set -s -- ${!a[0][@]}
    [[ ${@} == '0 1 2 3 4' ]] || log_error "\${!$c[0][@]} not 0 1 2 3 4"
    set -s -- ${!a[1][@]}
    [[ ${@} == '0 1 2 3 4 5' ]] || log_error "\${!$c[1][@]} not 0 1 2 3 4 5"
    [[ $a == 00 ]] || log_error  "\$$c is not 00"
    [[ ${a[0]} == 00 ]] || log_error  "\${$a[0]} is not 00"
    [[ ${a[0][0]} == 00 ]] || log_error  "${a[0][0]} is not 00"
    [[ ${a[0][0][0]} == 00 ]] || log_error  "\${$c[0][0][0]} is not 00"
    [[ ${a[0][0][1]} == '' ]] || log_error  "\${$c[0][0][1]} is not empty"
    [[ ${b[3]} == 23 ]] || log_error "${!b}[3] not = 23"
}

check a

unset a
typeset -A a
for ((i=0; i < 4; i++ ))
do
    for ((j=0; j < 5; j++ ))
    do
        a[$i][j]=$i$j
    done
done

for ((i=0; i < 4; i++ ))
do
    for ((j=0; j < 5; j++ ))
    do
        [[ ${a[$i][j]} == "$i$j" ]] || log_error "\${a[$i][$j]} == $i$j"
    done
done

a[1][5]=15
b=(
    [0]=( 00 01 02 03 04 )
    [1]=( 10 11 12 13 14 15)
    [2]=( 20 21 22 23 24 )
    [3]=( 30 31 32 33 34 )
)
check b

[[ ${a[1][@]} == "${b[1][@]}" ]] || log_error "a[1] not equal to b[1]"
c=(
    [0]=( [0]=00 [1]=01 [2]=02 [3]=03 [4]=04 )
    [1]=( [0]=10 [1]=11 [2]=12 [3]=13 [4]=14 [5]=15)
    [2]=( [0]=20 [1]=21 [2]=22 [3]=23 [4]=24 )
    [3]=( [0]=30 [1]=31 [2]=32 [3]=33 [4]=34 )
)
check c

typeset -A d
d[0]=( [0]=00 [1]=01 [2]=02 [3]=03 [4]=04 )
d[1]=( [0]=10 [1]=11 [2]=12 [3]=13 [4]=14 [5]=15)
d[2]=( [0]=20 [1]=21 [2]=22 [3]=23 [4]=24 )
d[3]=( [0]=30 [1]=31 [2]=32 [3]=33 [4]=34 )
check d

unset a b c d
[[ ${a-set} ]] || log_error "a is set after unset"
[[ ${b-set} ]] || log_error "b is set after unset"
[[ ${c-set} ]] || log_error "c is set after unset"
[[ ${d-set} ]] || log_error "c is set after unset"

$SHELL 2> /dev/null <<\+++ ||  log_error 'input of 3 dimensional array not working'
typeset x=(
    ( (g G) (h H) (i I) )
    ( (d D) (e E) (f F) )
    ( (a A) (b B) (c C) )
)
[[ ${x[0][0][0]} == g ]] || log_error '${x[0][0][0]} == G'
[[ ${x[1][1][0]} == e ]] || log_error '${x[1][1][0]} == e'
[[ ${x[1][1][1]} == E ]] || log_error '${x[2][2][1]} == C'
[[ ${x[0][2][1]} == I ]] || log_error '${x[0][2][1]} == I'
+++

typeset -a -si x=( [0]=(1 2 3) [1]=(4 5 6) [2]=(7 8 9) )
[[ ${x[1][1]} == 5 ]] || log_error 'changing two dimensional indexed array to short integer failed'
unset x
typeset -A -si x=( [0]=(1 2 3) [1]=(4 5 6) [2]=(7 8 9) )
[[ ${x[1][2]} == 6 ]] || log_error 'changing two dimensional associative array to short integer failed'

unset ar x y
integer -a ar
integer i x y
for (( i=0 ; i < 100 ; i++ ))
do
    (( ar[y][x++]=i ))
    (( x > 9 )) && (( y++ , x=0 ))
done

[[ ${#ar[0][*]} == 10 ]] || log_error "\${#ar[0][*]} is '${#ar[0][*]}', should be 10"
[[ ${#ar[*]} == 10 ]] || log_error  "\${#ar[*]} is '${#ar[*]}', should be 10"
[[ ${ar[5][5]} == 55 ]] || log_error "ar[5][5] is '${ar[5][5]}', should be 55"

unset ar
integer -a ar
x=0 y=0
for (( i=0 ; i < 81 ; i++ ))
do
    nameref ar_y=ar[$y]
    (( ar_y[x++]=i ))
    (( x > 8 )) && (( y++ , x=0 ))
    typeset +n ar_y
done

[[ ${#ar[0][*]} == 9 ]] || log_error "\${#ar[0][*]} is '${#ar[0][*]}', should be 9"
[[ ${#ar[*]} == 9 ]] || log_error  "\${#ar[*]} is '${#ar[*]}', should be 9"
[[ ${ar[4][4]} == 40 ]] || log_error "ar[4][4] is '${ar[4][4]}', should be 40"

$SHELL 2> /dev/null -c 'compound c;float -a c.ar;(( c.ar[2][3][3] = 5))' || 'multi-dimensional arrays in arithemtic expressions not working'

expected='typeset -a -l -E c.ar=(typeset -a [2]=(typeset -a [3]=([3]=5) ) )'
unset c
float c.ar
c.ar[2][3][3]=5
[[ $(typeset -p c.ar) == "$expected" ]] || log_error "c.ar[2][3][3]=5;typeset -c c.ar expands to $(typeset -p c.ar)"

unset values
float -a values=( [1][3]=90 [1][4]=89 )
function fx
{
    nameref arg=$1
    [[ ${arg[0..5]} == '90 89' ]] || log_error '${arg[0..5]} not correct where arg is a nameref to values[1]'
}
fx values[1]

function test_short_integer
{
        compound out=( typeset stdout stderr ; integer res )
    compound -r -a tests=(
        ( cmd='integer -s -r -a x=( 1 2 3 ) ; print "${x[2]}"' stdoutpattern='3' )
        ( cmd='integer -s -r -A x=( [0]=1 [1]=2 [2]=3 ) ; print "${x[2]}"' stdoutpattern='3' )
        # 2D integer arrays: the following two tests crash for both "integer -s" and "integer"
        ( cmd='integer    -r -a x=( [0]=( [0]=1 [1]=2 [2]=3 ) [1]=( [0]=4 [1]=5 [2]=6 ) [2]=( [0]=7 [1]=8 [2]=9 ) ) ; print "${x[1][1]}"' stdoutpattern='5' )
        ( cmd='integer -s -r -a x=( [0]=( [0]=1 [1]=2 [2]=3 ) [1]=( [0]=4 [1]=5 [2]=6 ) [2]=( [0]=7 [1]=8 [2]=9 ) ) ; print "${x[1][1]}"' stdoutpattern='5' )
       )
    integer i

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        nameref tst=tests[i]

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -o errexit -c "${tst.cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"
        expect="${tst.stdoutpattern}"
        actual="${out.stdout}"
        [[ "$actual" == "$expect" ]] || log_error "Expected stdout to match" "$expect" "$actual"
        expect=''
        actual="${out.stderr}"
        [[ "$actual" == $expect ]] || log_error "Expected empty stderr" "$expect" "$actual"
        expect=0
        actual=$out.res
        (( actual == expect )) || log_error "Unexpected exit code" "$expect" "$actual"
    done

    return 0
}
# run tests
test_short_integer

typeset -a arr=( ( 00 ) ( 01 ) ( 02 ) ( 03 ) ( 04 ) ( 05 ) ( 06 ) ( 07 ) ( 08 ) ( 09 ) ( 10 ) )
typeset -i i=10 j=0
{  y=$( echo ${arr[i][j]} ) ;} 2> /dev/null
[[ $y == 10 ]] || log_error '${arr[10][0] should be 10 '

unset A
typeset -A A
typeset -A A[a]
A[a][z]=1
[[ ${!A[a][@]} == z ]] || log_error 'A[a] should have only subscript z'

typeset -a EMPTY_ARRAY=()
typeset -a g_arr=()
function initialize
{
    g_arr[0]=(11 22 33)
    g_arr[1]=( "${EMPTY_ARRAY[@]}" )
}
initialize

expect='typeset -a g_arr[0]=(11 22 33)'
actual=$(typeset -p g_arr[0])
[[ $actual == "$expect" ]] || log_error "typeset -p g_arr[0]" "$expect" "$actual"
expect='typeset -a g_arr[1]'
actual=$(typeset -p g_arr[1])
[[ $actual == "$expect" ]] || log_error "typeset -p g_arr[1]" "$expect" "$actual"
expect='typeset -a g_arr=((11 22 33)  () )'
actual=$(typeset -p g_arr)
[[ $actual == "$expect" ]] || log_error "typeset -p g_arr" "$expect" "$actual"

unset arr
typeset -a arr
typeset -a arr[0]=()
expect='typeset -a arr[0]'
actual=$(typeset -p arr[0])
[[ $actual == "$expect" ]] || log_error "arr[0]" "$expect" "$actual"
expect='typeset -a arr=( () )'
actual=$(typeset -p arr)
[[ $actual == "$expect" ]] || log_error "arr" "$expect" "$actual"

unset foo
typeset -A foo
typeset -A foo[bar]
foo[bar][x]=2
(( foo[bar][x]++ ))
[[ ${foo[bar][x]} == 3 ]] || err_ext 'subscrit gets added incorrectly to an associat array when ++ operator is called'
