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
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r ${Command}[$1]: "${@:2}"
	(( Errors+=1 ))
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0
enum Color_t=(red green blue orange yellow)
enum -i Sex_t=(Male Female)
for ((i=0; i < 1000; i++))
do
Color_t x
[[ $x == red ]] || err_exit 'Color_t does not default to red'
x=orange
[[ $x == orange ]] || err_exit '$x should be orange'
( x=violet) 2> /dev/null && err_exit 'x=violet should fail'
x[2]=green
[[ ${x[2]} == green ]] || err_exit '${x[2]} should be green'
(( x[2] == 1 )) || err_exit '((x[2]!=1))'
[[ $((x[2])) == 1 ]] || err_exit '$((x[2]))!=1'
[[ $x == orange ]] || err_exit '$x is no longer orange'
Color_t -A y
y[foo]=yellow
[[ ${y[foo]} == yellow ]] || err_exit '${y[foo]} != yellow'
(( y[foo] == 4 )) || err_exit '(( y[foo] != 4))'
unset y
typeset -a "[Color_t]" z
z[green]=xyz
[[ ${z[green]} == xyz ]] || err_exit '${z[green]} should be xyz'
[[ ${z[1]} == xyz ]] || err_exit '${z[1]} should be xyz'
z[orange]=bam
[[ ${!z[@]} == 'green orange' ]] || err_exit '${!z[@]} == "green orange"'
unset x
Sex_t x
[[ $x == Male ]] || err_exit 'Sex_t not defaulting to Male'
x=female
[[ $x == Female ]] || err_exit 'Sex_t not case sensitive'
unset x y z
done
(
typeset -T X_t=( typeset name=aha )
typeset -a "[X_t]" arr
) 2> /dev/null
[[ $? == 1 ]] || err_exit 'typeset -a [X_t] should generate an error message when X-t is not an enumeriation type'

typeset -a "[Color_t]" arr
arr[green]=foo
[[ ${arr[1]} == ${arr[green]}  ]] || err_exit 'arr[1] != arr[green]'
read -A arr <<<  'x y z xx yy'
[[ ${arr[1]} == ${arr[green]}  ]] || err_exit 'arr[1] != arr[green] after read'

enum Bool=(false true)
Bool -a bar
bar[3]=true
[[ $((5+bar[3])) != 6 ]] && err_exit '$((5+bar[3])) should be 6'

got=$(myvar_c=foo $SHELL -c 'printf "%s\n" "${!myvar_*}"')
[[ $got == myvar_c ]] || err_exit '${!myvar_*} does not expand at start of script'

myvar_c=foo
enum _XX=(foo bar)
[[ ${!myvar_*} ==  myvar_c ]] || err_exit '${!myvar_*} does not expand after enum'

enum matrix=( neg null pos )
exp='matrix -a container=(typeset -a [3]=([1]=neg) typeset -a [4]=([4]=pos) )'
matrix -a container=( [3][1]=neg [4][4]=pos) 2> /dev/null
[[ $(typeset -p container) == "$exp" ]] || err_exit 'multi-dimensional arrays of enums not working'

exp='typeset -C -a b=(typeset -a [50]=([51]=(_Bool v=true)) )'
compound -a a=( [40][41]=( bool v ) )
(( a[40][41].v=1 )) 
compound -a b
typeset -m "b[50][51].v=a[40][41].v"
[[ $(typeset -p b) == "$exp" ]] || err_exit 'typeset -m does not preserve enum'

enum bb=( ya ba )
compound c=( bb -a ar=([4][5]=ba ) )
exp='typeset -C c=(bb -a ar=(typeset -a [4]=([5]=ba) );)'
[[ $(typeset -p c) == "$exp" ]] ||  err_exit 'compound variable contained command array of enums not working'

(
	set -o nounset
	bool bl
	(( bl=true )) 2> /dev/null
) || err_exit 'set -o nounset causes ((bool=true)) to fail'

bool  -a bl
(( bl[4]=false))
[[ ${bl[4]} == false ]] || err_exit "setting enum array element to 0 doesn't expand to enumeration value"

bool -a bia
(( bia[4]=false))
[[ ${bia[3]} ]] &&  err_exit 'empty index array element should not produce a value'
(( bia[3] == 0 )) || err_exit 'empty index array element should be numerically 0'
bool  -A baa
(( baa[4]=false))
[[ ${baa[3]} ]] &&  err_exit 'empty associative array element should not produce a value'
(( baa[3] == 0 )) || err_exit 'empty associative array element should be numerically 0'

bool -A a=( [2]=true [4]=false )
[[ ${a[2]} == true ]] || err_exit 'associative arrary assignment failure'
[[ ${#a[@]} == 2 ]] || err_exit ' bool -A a should only have two elements' 

$SHELL  -uc 'i=1; bool b; ((b=((i==1)?(true):(false)) ));:'  || err_exit 'assignment to enum with ?: fails with set -u'

exit $((Errors<125?Errors:125))
