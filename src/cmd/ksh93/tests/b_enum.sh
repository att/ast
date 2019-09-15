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

# =======
# Test what happens when enum is invoked with no arguments.
expect="enum: expected one argument, got 0"
actual=$(enum 2>&1)
[[ $actual =~ "$expect" ]] || log_error "enum with no arguments" "$expect" "$actual"

# TODO: Moving this line after the `for` loop that below changes output. That's a bug. Fix it.
# =======
_Bool --man 2>&1 | grep -q "_Bool - create an instance of type _Bool" ||
    log_error "Enum types should recognize --man option"

# =======
enum Color_t=(red green blue orange yellow)
enum -i Sex_t=(Male Female)

Color_t color
Sex_t sex

actual=${sex.Female}
expect=1
[[ $actual == $expect ]] || log_error "valid enum sym to index wrong" "$expect" "$actual"

actual=${sex.female}
expect=1
[[ $actual == $expect ]] || log_error "valid enum sym to index wrong" "$expect" "$actual"

actual=${sex.MaLe}
expect=0
[[ $actual == $expect ]] || log_error "valid enum sym to index wrong" "$expect" "$actual"

actual=${color.red}
expect=0
[[ $actual == $expect ]] || log_error "valid enum sym to index wrong" "$expect" "$actual"

actual=${color.yellow}
expect=4
[[ $actual == $expect ]] || log_error "valid enum sym to index wrong" "$expect" "$actual"

actual=$( { echo ${color.arglebargle}; } 2>&1 )
expect=": arglebargle: invalid enum constant for color"
[[ $actual =~ .*"$expect"$ ]] || log_error "invalid enum sym to index wrong" "$expect" "$actual"

# Why is this looping 1000 times?
for ((i=0; i < 1000; i++))
do
    Color_t x
    [[ $x == red ]] || log_error 'Color_t does not default to red'
    x=orange
    [[ $x == orange ]] || log_error '$x should be orange'

    ( x=violet) 2> /dev/null && log_error 'x=violet should fail'
    x[2]=green
    [[ ${x[2]} == green ]] || log_error '${x[2]} should be green'
    (( x[2] == 1 )) || log_error '((x[2]!=1))'
    [[ $((x[2])) == 1 ]] || log_error '$((x[2]))!=1'
    [[ $x == orange ]] || log_error '$x is no longer orange'
    Color_t -A y
    y[foo]=yellow
    [[ ${y[foo]} == yellow ]] || log_error '${y[foo]} != yellow'
    (( y[foo] == 4 )) || log_error '(( y[foo] != 4))'
    unset y
    typeset -a "[Color_t]" z
    z[green]=xyz
    [[ ${z[green]} == xyz ]] || log_error '${z[green]} should be xyz'
    [[ ${z[1]} == xyz ]] || log_error '${z[1]} should be xyz'
    z[orange]=bam
    [[ ${!z[@]} == 'green orange' ]] || log_error '${!z[@]} == "green orange"'
    unset x
    Sex_t x
    [[ $x == Male ]] || log_error 'Sex_t not defaulting to Male'
    x=female
    [[ $x == Female ]] || log_error 'Sex_t not case sensitive'
    unset x y z
done

(
typeset -T X_t=( typeset name=aha )
typeset -a "[X_t]" arr
) 2> /dev/null
[[ $? == 1 ]] ||
    log_error 'typeset -a [X_t] should generate error when X-t is not an enumeration type'

typeset -a "[Color_t]" arr
arr[green]=foo
[[ ${arr[1]} == ${arr[green]}  ]] || log_error 'arr[1] != arr[green]'
read -A arr <<<  'x y z xx yy'
[[ ${arr[1]} == ${arr[green]}  ]] || log_error 'arr[1] != arr[green] after read'

enum Bool=(false true)
Bool -a bar
bar[3]=true
[[ $((5+bar[3])) != 6 ]] && log_error '$((5+bar[3])) should be 6'

got=$(myvar_c=foo $SHELL -c 'printf "%s\n" "${!myvar_*}"')
[[ $got == myvar_c ]] || log_error '${!myvar_*} does not expand at start of script'

myvar_c=foo
enum _XX=(foo bar)
[[ ${!myvar_*} ==  myvar_c ]] || log_error '${!myvar_*} does not expand after enum'

enum matrix=( neg null pos )
exp='matrix -a container=(typeset -a [3]=([1]=neg) typeset -a [4]=([4]=pos) )'
matrix -a container=( [3][1]=neg [4][4]=pos) 2> /dev/null
[[ $(typeset -p container) == "$exp" ]] || log_error 'multi-dimensional arrays of enums not working'

exp='typeset -C -a b=(typeset -a [50]=([51]=(_Bool v=true)) )'
compound -a a=( [40][41]=( bool v ) )
(( a[40][41].v=1 ))
compound -a b
typeset -m "b[50][51].v=a[40][41].v"
[[ $(typeset -p b) == "$exp" ]] || log_error 'typeset -m does not preserve enum'

enum bb=( ya ba )
compound c=( bb -a ar=([4][5]=ba ) )
exp='typeset -C c=(bb -a ar=(typeset -a [4]=([5]=ba) );)'
[[ $(typeset -p c) == "$exp" ]] ||  log_error 'compound variable contained command array of enums not working'

(
    set -o nounset
    bool bl
    (( bl=true )) 2> /dev/null
) || log_error 'set -o nounset causes ((bool=true)) to fail'

bool  -a bl
(( bl[4]=false))
[[ ${bl[4]} == false ]] || log_error "setting enum array element to 0 doesn't expand to enumeration value"

bool -a bia
(( bia[4]=false))
[[ ${bia[3]} ]] &&  log_error 'empty index array element should not produce a value'
(( bia[3] == 0 )) || log_error 'empty index array element should be numerically 0'
bool  -A baa
(( baa[4]=false))
[[ ${baa[3]} ]] &&  log_error 'empty associative array element should not produce a value'
(( baa[3] == 0 )) || log_error 'empty associative array element should be numerically 0'

bool -A a=( [2]=true [4]=false )
[[ ${a[2]} == true ]] || log_error 'associative arrary assignment failure'
[[ ${#a[@]} == 2 ]] || log_error ' bool -A a should only have two elements'

$SHELL  -uc 'i=1; bool b; ((b=((i==1)?(true):(false)) ));:'  || log_error 'assignment to enum with ?: fails with set -u'

# =======
actual=$(enum -p _Bool)
expect=$'enum _Bool=(\n\tfalse\n\ttrue\n)'
[[ "$actual" = "$expect" ]] || log_error "enum -p does not print enum" "$expect" "$actual"

# =======
# Convert an indexed array into enum
foo=(bar baz)
enum foo
actual=$(enum -p foo)
expect=$'enum foo=(\n\tbar\n\tbaz\n)'
[[ "$actual" = "$expect" ]] || log_error "enum does not convert indexed array to enum" "$expect" "$actual"
