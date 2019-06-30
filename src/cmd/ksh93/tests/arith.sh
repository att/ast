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

# Some platforms, such as OpenBSD and Cygwin, do not handle NaN correctly.
# See https://marc.info/?l=openbsd-bugs&m=152488432922625&w=2
if typeset -f .sh.math.signbit >/dev/null && (( signbit(-NaN) ))
then
    HAVE_signbit=1
else
    log_warning '-lm does not support signbit(-NaN)'
    HAVE_signbit=0
fi

trap '' FPE # NOTE: osf.alpha requires this (no ieee math)

integer x=1 y=2 z=3
if (( 2+2 != 4 ))
then
    log_error 2+2!=4
fi

if ((x+y!=z))
then
    log_error x+y!=z
fi

if (($x+$y!=$z))
then
    log_error $x+$y!=$z
fi

if (((x|y)!=z))
then
    log_error "(x|y)!=z"
fi

if ((y >= z))
then
    log_error "y>=z"
fi

if ((y+3 != z+2))
then
    log_error "y+3!=z+2"
fi

if ((y<<2 != 1<<3))
then
    log_error "y<<2!=1<<3"
fi

if ((133%10 != 3))
then
    log_error "133%10!=3"
    if (( 2.5 != 2.5 ))
    then
    log_error 2.5!=2.5
    fi
fi

d=0
((d || 1)) || log_error 'd=0; ((d||1))'
if (( d++!=0))
then
    log_error "d++!=0"
fi

if (( --d!=0))
then
    log_error "--d!=0"
fi

if (( (d++,6)!=6 && d!=1))
then
    log_error '(d++,6)!=6 && d!=1'
fi

d=0
if (( (1?2+1:3*4+d++)!=3 || d!=0))
then
    log_error '(1?2+1:3*4+d++) !=3'
fi

for ((i=0; i < 20; i++))
do
    :
done

if (( i != 20))
then
    log_error 'for (( expr)) failed'
fi

for ((i=0; i < 20; i++)); do    : ; done

if (( i != 20))
then
    log_error 'for (( expr));... failed'
fi
for ((i=0; i < 20; i++)) do    : ; done

if (( i != 20))
then
    log_error 'for (( expr))... failed'
fi

if (( (i?0:1) ))
then
    log_error '(( (i?0:1) )) failed'
fi

if (( (1 || 1 && 0) != 1 ))
then
    log_error '( (1 || 1 && 0) != 1) failed'
fi

if (( (_=1)+(_x=0)-_ ))
then
    log_error '(_=1)+(_x=0)-_ failed'
fi

if ((  (3^6) != 5))
then
    log_error '((3^6) != 5) failed'
fi

integer x=1
if (( (x=-x) != -1 ))
then
    log_error '(x=-x) != -1 failed'
fi

i=2
if (( 1$(($i))3 != 123 ))
then
    log_error ' 1$(($i))3 failed'
fi

((pi=4*atan(1.)))
point=(
    float x
    float y
)
(( point.x = cos(pi/6), point.y = sin(pi/6) ))
if (( point.x*point.x + point.y*point.y > 1.01 ))
then
    log_error 'cos*cos +sin*sin > 1.01'
fi

if (( point.x*point.x + point.y*point.y < .99 ))
then
    log_error 'cos*cos +sin*sin < .99'
fi

if [[ $((y=x=1.5)) != 1 ]]
then
    log_error 'typecast not working in arithmetic evaluation'
fi

typeset -E x=1.5
( ((x++)) )
if [[ $? == 0 ]]
then
    log_error 'postincrement of floating point allowed'
fi

( ((++x)) )
if [[ $? == 0 ]]
then
    log_error 'preincrement of floating point allowed'
fi

x=1.5
( ((x%1.1)) )
if [[ $? == 0 ]]
then
    log_error 'floating point allowed with % operator'
fi

x=.125
if [[ $(( 4 * x/2 )) != 0.25 ]]
then
    log_error '(( 4 * x/2 )) is not 0.25, with x=.125'
fi

if [[ $(( pow(2,3) )) != 8 ]]
then
    log_error '$(( pow(2,3) )) != 8'
fi

( [[ $(( pow(2,(3)) )) == 8 ]] )
if (( $? ))
then
    log_error '$(( pow(2,(3)) )) != 8'
fi

unset x
integer x=1; integer x=1
if [[ $x != 1 ]]
then
    log_error 'two consecutive integer x=1 not working'
fi

unset z
{ z=$(typeset -RZ2 z2; (( z2 = 8 )); print $z2); }
if [[ $z != "08" ]]
then
    log_error "typeset -RZ2 leading 0 decimal not working [z=$z]"
fi

{ z=$(typeset -RZ3 z3; (( z3 = 8 )); print $z3); }
if [[ $z != "008" ]]
then
    log_error "typeset -RZ3 leading 0 decimal not working [z=$z]"
fi

unset z
typeset -Z3 z=010
(( z=z+1))
if [[ $z != 011 ]]
then
    log_error "leading 0's in -Z not treated as decimal"
fi

unset x
integer x=0
if [[ $((x+=1)) != 1  ]] || ((x!=1))
then
    log_error "+= not working"
    x=1
fi

x=1
if [[ $((x*=5)) != 5  ]] || ((x!=5))
then
    log_error "*= not working"
    x=5
fi

if [[ $((x%=4)) != 1  ]] || ((x!=1))
then
    log_error "%= not working"
    x=1
fi

if [[ $((x|=6)) != 7  ]] || ((x!=7))
then
    log_error "|= not working"
    x=7
fi

if [[ $((x&=5)) != 5  ]] || ((x!=5))
then
    log_error "&= not working"
    x=5
fi
function newscope
{
    float x=1.5
    (( x += 1 ))
    print -r -- $x
}
if [[ $(newscope) != 2.5 ]]
then
    log_error "arithmetic using wrong scope"
fi
unset x
integer y[3]=9 y[4]=2 i=3
(( x = y[3] + y[4] ))

if [[ $x != 11 ]]
then
    log_error "constant index array arithmetic failure"
fi

(( x = $empty y[3] + y[4] ))
if [[ $x != 11 ]]
then
    log_error "empty constant index array arithmetic failure"
fi

(( x = y[i] + y[i+1] ))
if [[ $x != 11 ]]
then
    log_error "variable subscript index array arithmetic failure"
fi

integer a[5]=3 a[2]=4
(( x = y[a[5]] + y[a[2]] ))
if [[ $x != 11 ]]
then
    log_error "nested subscript index array arithmetic failure"
fi

unset y
typeset -Ai y
y[three]=9 y[four]=2
three=four
four=three
(( x = y[three] + y[four] ))
if [[ $x != 11 ]]
then
    log_error "constant associative array arithmetic failure"
fi

(( x = y[$three] + y[$four] ))
if [[ $x != 11 ]]
then
    log_error "variable subscript associative array arithmetic failure"
fi

$SHELL -nc '((a = 1))' || log_error "sh -n fails with arithmetic"
$SHELL -nc '((a.b++))' || log_error "sh -n fails with arithmetic2"
unset z
float z=7.5
if { (( z%2 != 1)); }
then
    log_error '% not working on floating point'
fi

chr=(a ' ' '=' '\r' '\n' '\\' '\"' '$' "\\'" '[' ']' '(' ')' '<' '\xab' '\040' '`' '{' '}' '*' '\E')
val=(97 32  61 13 10 92 34 36 39 91 93 40 41 60 171 32 96 123 125 42 27)

q=0
for ((i=0; i < ${#chr[@]}; i++))
do
    if (( '${chr[i]}' != ${val[i]} ))
    then
    log_error "(( '${chr[i]}'  !=  ${val[i]} ))"
    fi
    if [[ $(( '${chr[i]}' )) != ${val[i]} ]]
    then
    log_error "(( '${chr[i]}' )) !=  ${val[i]}"
    fi
    if [[ $(( L'${chr[i]}' )) != ${val[i]} ]]
    then
    log_error "(( '${chr[i]}' )) !=  ${val[i]}"
    fi
    if eval '((' "'${chr[i]}'" != ${val[i]} '))'
    then
    log_error "eval (( '${chr[i]}'  !=  ${val[i]} ))"
    fi
    if eval '((' "'${chr[i]}'" != ${val[i]} ' + $q ))'
    then
    log_error "eval (( '${chr[i]}'  !=  ${val[i]} ))"
    fi
done

unset x
typeset -ui x=4294967293
[[ $x != 4294967293 ]]  && log_error "unsigned integers not working"
x=32767
x=x+1
[[ $x != 32768 ]]  && log_error "unsigned integer addition not working"
unset x
float x=99999999999999999999999999
if (( x < 1e20 ))
then
    log_error 'large integer constants not working'
fi

unset x  y
function foobar
{
    nameref x=$1
    (( x +=1 ))
    print $x
}

x=0 y=4
if [[ $(foobar y) != 5 ]]
then
    log_error 'name references in arithmetic statements in functions broken'
fi

if (( 2**3 != pow(2,3) ))
then
    log_error '2**3 not working'
fi

if (( 2**3*2 != pow(2,3)*2 ))
then
    log_error '2**3*2 not working'
fi

if (( 4**3**2 != pow(4,pow(3,2)) ))
then
    log_error '4**3**2 not working'
fi

if (( (4**3)**2 != pow(pow(4,3),2) ))
then
    log_error '(4**3)**2 not working'
fi

typeset -Z3 x=11
typeset -i x
if (( x != 11 ))
then
    log_error '-Z3 not treated as decimal'
fi

unset x
typeset -ui x=-1
(( x >= 0 )) || log_error 'unsigned integer not working'
(( $x >= 0 )) || log_error 'unsigned integer not working as $x'

unset x
typeset -ui42 x=50
if [[ $x != 42#18 ]]
then
    log_error 'display of unsigned integers in non-decimal bases wrong'
fi

$SHELL -c 'i=0;(( ofiles[i] != -1 && (ofiles[i] < mins || mins == -1) )); exit 0' ||
    log_error 'lexical error with arithemtic expression'
$SHELL -c '(( +1 == 1))' 2> /dev/null || log_error 'unary + not working'
typeset -E val=123.01234567890
[[ $val == 123.0123456789 ]] || log_error "rounding error val=$val"
if [[ $(print x$((10))=foo) != x10=foo ]]
then
    log_error 'parsing error with x$((10))=foo'
fi

$SHELL -c 'typeset x$((10))=foo' || log_error 'typeset x$((10)) parse error'
unset x
x=$(( exp(log(2.0)) ))
(( x > 1.999 && x < 2.001 )) || log_error 'composite functions not working'
unset x y n
typeset -Z8 x=0 y=0
integer n
for    (( n=0; n < 20; n++ ))
do
    let "x = $x+1"
    (( y = $y+1 ))
done

(( x == n ))  || log_error 'let with zero filled fields not working'
(( y == n ))  || log_error '((...)) with zero filled fields not working'

typeset -RZ3 x=10
[[ $(($x)) == 10 && $((1$x)) == 1010 ]] || log_error 'zero filled fields not preserving leading zeros'
unset y
[[ $(let y=$x;print $y) == 10 && $(let y=1$x;print $y) == 1010 ]] || log_error 'zero filled fields not preserving leading zeros with let'
unset i ip ipx
typeset -i hex=( 172 30 18 1)
typeset -ui ip=0 ipx=0
integer i
for ((i=0; i < 4; i++))
do
    (( ip =  (ip<<8) | hex[i]))
done

for ((i=0; i < 4; i++))
do
    (( ipx = ip % 256 ))
    (( ip /= 256 ))
    (( ipx != hex[3-i] )) && log_error "hex digit $((3-i)) not correct"
done

unset x
x=010
(( x == 10 )) || log_error 'leading zeros in x treated as octal arithmetic with $((x))'
(( $x == 8 )) || log_error 'leading zeros not treated as octal arithmetic with $x'

unset x
typeset -Z x=010
(( x == 10 )) || log_error 'leading zeros not ignored for arithmetic'
(( $x == 10 )) || log_error 'leading zeros not ignored for arithmetic with $x'

typeset -i i=x
(( i == 10 )) || log_error 'leading zeros not ignored for arithmetic assignment'
(( ${x:0:1} == 0 )) || log_error 'leading zero should not be stripped for x:a:b'

c010=3
(( c$x  == 3 )) || log_error 'leading zero with variable should not be stripped'
[[ $( ($SHELL -c '((++1))' 2>&1) 2>/dev/null ) == *++1:* ]] || log_error "((++1)) not generating error message"

i=2
(( "22" == 22 )) || log_error "double quoted constants fail"
(( "2$i" == 22 )) || log_error "double quoted variables fail"
(( "18+$i+2" == 22 )) || log_error "double quoted expressions fail"

# 04-04-28 bug fix
unset i; typeset -i i=01-2
(( i == -1 )) || log_error "01-2 is not -1"

cat > $TEST_DIR/script <<-\!
tests=$*
typeset -A blop
function blop.get
{
    .sh.value=777
}

function mkobj
{
    nameref obj=$1
    obj=()
    [[ $tests == *1* ]] && {
        (( obj.foo = 1 ))
        (( obj.bar = 2 ))
        (( obj.baz = obj.foo + obj.bar ))    # ok
        echo $obj
    }
    [[ $tests == *2* ]] && {
        (( obj.faz = faz = obj.foo + obj.bar ))    # ok
        echo $obj
    }
    [[ $tests == *3* ]] && {
        # case 3, 'active' variable involved, w/ intermediate variable
        (( obj.foz = foz = ${blop[1]} ))    # coredump
        echo $obj
    }
    [[ $tests == *4* ]] && {
        # case 4, 'active' variable, in two steps
        (( foz = ${blop[1]} ))    # ok
        (( obj.foz = foz ))        # ok
        echo $obj
    }
    [[ $tests == *5* ]] && {
        # case 5, 'active' variable involved, w/o intermediate variable
        (( obj.fuz = ${blop[1]} ))    # coredump
        echo $obj
    }
    [[ $tests == *6* ]] && {
        echo $(( obj.baz = obj.foo + obj.bar ))    # coredump
    }
    [[ $tests == *7* ]] && {
        echo $(( obj.foo + obj.bar ))    # coredump
    }
}

mkobj bla
!
chmod +x $TEST_DIR/script
[[ $($TEST_DIR/script 1) != '( bar=2 baz=3 foo=1 )' ]] && log_error 'compound var arithmetic failed'
[[ $($TEST_DIR/script 2) != '( faz=0 )' ]] && log_error 'compound var arithmetic failed'
[[ $($TEST_DIR/script 3) != '( foz=777 )' ]] && log_error 'compound var arithmetic failed'
[[ $($TEST_DIR/script 4) != '( foz=777 )' ]] && log_error 'compound var arithmetic failed'
[[ $($TEST_DIR/script 5) != '( fuz=777 )' ]] && log_error 'compound var arithmetic failed'
[[ $($TEST_DIR/script 6) != '0' ]] && log_error 'compound var arithmetic failed'
[[ $($TEST_DIR/script 7) != '0' ]] && log_error 'compound var arithmetic failed'
unset foo
typeset -F1 foo=123456789.19
[[ $foo == 123456789.2 ]] || log_error 'typeset -F1 not working correctly'

# divide by zero

for expr in '1/(1/2)' '8%(1/2)' '8%(1.0/2)'
do
    [[ $( ( $SHELL -c "( (($expr)) )  || print ok" ) ) == ok ]] || log_error "divide by zero not trapped: $expr"
done

for expr in '1/(1.0/2)' '1/(1/2.0)'
do
    [[ $( ( $SHELL -c "( print -r -- \$(($expr)) )" ) ) == 2 ]] || log_error "invalid value for: $expr"
done

[[ $((5||0)) == 1 ]] || log_error '$((5||0))'" == $((5||0)) should be 1"
$SHELL -c 'integer x=3 y=2; (( (y += x += 2) == 7  && x==5))' || log_error '((y += x += 2)) not working'
$SHELL -c 'b=0; [[ $((b?a=1:b=9)) == 9 ]]' || log_error 'b?a=1:b=9 not working'

unset x
(( x = 4*atan(1.0) ))
[[ $x == "$((x))" ]] || log_error  '$x !- $((x)) when x is pi'

$SHELL -c  "[[  ${x//./} == {14,100}(\d) ]]" || log_error 'pi has less than 14 significant places'
if (( Inf+1 == Inf ))
then
    set \
        Inf        inf    \
        -Inf        -inf    \
        Nan        nan    \
        -Nan        -nan    \
        1.0/0.0        inf
    while (( $# >= 2 ))
    do
        expect="$2"
        actual=$(printf "%g\n" $(($1)))
        [[ $actual == $expect ]] || log_error "printf '%g\\n' \$(($1)) failed" "$expect" "$actual"
        if (( HAVE_signbit ))
        then
            actual=$(printf "%g\n" $1)
            [[ $actual == $expect ]] || log_error "printf '%g\\n' $1 failed" "$expect" "$actual"
            actual=$(printf -- $(($1)))
            [[ $actual == $expect ]] || log_error "print -- \$(($1)) failed" "$expect" "$actual"
        fi
        shift 2
    done
    (( 1.0/0.0 == Inf )) || log_error '1.0/0.0 != Inf'
    [[ $(print -- $((0.0/0.0))) == ?(-)nan ]] || log_error '0.0/0.0 != NaN'
    (( Inf*Inf == Inf )) || log_error 'Inf*Inf != Inf'
    (( NaN != NaN )) || log_error 'NaN == NaN'
    (( -5*Inf == -Inf )) || log_error '-5*Inf != -Inf'
    if (( HAVE_signbit ))
    then
        actual=$(print -- $(( sqrt(-1.0) )))
        [[ $actual == ?(-)nan ]]|| log_error 'sqrt(-1.0) != NaN' "?(-)nan" "$actual"
    fi
    if [[ $OS_NAME != sunos ]]
    then
        # SunOS returns "nan" which is not unreasonable but not what every other platform returns.
        (( pow(1.0,Inf) == 1.0 )) || log_error 'pow(1.0,Inf) != 1.0' "1.0" "$(( pow(1.0,Inf) ))"
    fi
    (( pow(Inf,0.0) == 1.0 )) || log_error 'pow(Inf,0.0) != 1.0' "1.0" "$(( pow(1.0,0.0) ))"
    actual=$(( NaN/Inf ))
    [[ $actual == ?(-)nan ]] || log_error 'NaN/Inf != NaN' "?(-)nan" "$actual"
    (( 4.0/Inf == 0.0 )) || log_error '4.0/Inf != 0.0'
else
    log_error 'Inf and NaN not working'
fi

unset x y n r
expect=14.555
float x=$expect
float y=$(printf "%a" x)
actual=$y
[[ $actual == $expect ]] || log_error "output of printf %a not self preserving" "$expect" "$actual"

unset x y r
float x=-0 y=-0.0
r=-0
[[ $((-0)) == 0 ]] || log_error '$((-0)) should be 0'
[[ $(( -1*0)) == 0 ]] || log_error '$(( -1*0)) should be 0'
[[ $(( -1.0*0)) == -0 ]] || log_error '$(( -1.0*0)) should be -0'
[[ $(printf "%g %g %g\n" x $x $((x)) ) == '-0 -0 -0' ]] || log_error '%g of x $x $((x)) for x=-0 should all be -0'
[[ $(printf "%g %g %g\n" y $x $((y)) ) == '-0 -0 -0' ]] || log_error '%g of y $y $((y)) for y=-0.0 should all be -0'
$SHELL -c '(( x=));:' && log_error '((x=)) should be an error'
$SHELL -c '(( x+=));:' && log_error '((x+=)) should be an error'
$SHELL -c '(( x=+));:' && log_error '((x=+)) should be an error'
$SHELL -c 'x=();x.arr[0]=(z=3); ((x.arr[0].z=2))' || log_error '(((x.arr[0].z=2)) should not be an error'

float t
float a b r
v="-0.0 0.0 +0.0 -1.0 1.0 +1.0"
for a in $v
do
    for b in $v
    do
        (( expect = copysign(a, b) ))
        (( actual = copysign(a, b) ))
        [[ $actual == $expect ]] || {
            msg=$(printf "float t=copysign(%3.1f, %3.1f)\n" a b)
            log_error "$msg" "$expect" "$actual"
        }
    done
done

typeset -l y y_ascii
(( y=sin(90) ))
y_ascii=$y
(( y == y_ascii )) || log_error "no match,\n\t$(printf "%a\n" y)\n!=\n\t$(printf "%a\n" y_ascii)"

( $SHELL  <<- \EOF
    p=5
    t[p]=6
    while (( t[p] != 0 )) ; do
        ((
        p+=1 ,
        t[p]+=2 ,
        p+=3 ,
        t[p]+=5 ,
        p+=1 ,
        t[p]+=2 ,
        p+=1 ,
        t[p]+=1 ,
        p-=6  ,
        t[p]-=1
        ))
    :
    done
EOF) ||  log_error 'error with comma expression'

N=(89551 89557)
i=0 j=1
[[ $(printf "%d" N[j]-N[i]) == 6 ]] || log_error 'printf %d N[i]-N[j] failed'
[[ $((N[j]-N[i])) == 6 ]] || log_error  '$((N[j]-N[i])) incorrect'

unset a x
x=0
((a[++x]++))
(( x==1)) || log_error '((a[++x]++)) should only increment x once'
(( a[1]==1))  || log_error 'a[1] not incremented'

unset a
x=0
((a[x++]++))
(( x==1)) || log_error '((a[x++]++)) should only increment x once'
(( a[0]==1))  || log_error 'a[0] not incremented'

unset a
x=0
((a[x+=2]+=1))
(( x==2)) || log_error '((a[x+=2]++)) should result in x==2'
(( a[2]==1))  || log_error 'a[0] not 1'

unset a i
typeset -a a
i=1
(( a[i]=1 ))
(( a[0] == 0 )) || log_error 'a[0] not 0'
(( a[1] == 1 )) || log_error 'a[1] not 1'

unset a
typeset -i a
for ((i=0;i<1000;i++))
do ((a[RANDOM%2]++))
done
(( (a[0]+a[1])==1000)) || log_error '(a[0]+a[1])!=1000'

function fequal {
    (( fabs($1 - $2) < 0.000001 ))
}

actual=$(( 4.**3 / 10 ))
expect=6.4
fequal $actual $expect || log_error '4.**3/10 != 6.4' "$expect" "$actual"

actual=$(( (.5 + 3) / 7 ))
expect=0.5
fequal $actual $expect || log_error '(.5+3)/7 !== .5' "$expect" "$actual"

function .sh.math.mysin x
{
        ((.sh.value = x - x**3/6. + x**5/120.-x**7/5040. + x**9/362880.))
}

(( abs(sin(.5)-mysin(.5)) < 1e-6 )) || log_error 'mysin() not close to sin()'

$SHELL <<- \EOF || log_error "arithmetic functions defined and referenced in compound command not working"
{
    function .sh.math.mysin x
    {
            ((.sh.value = x-x**3/6. + x**5/120.-x**7/5040. + x**9/362880.))
    }
    (( abs(sin(.5)-mysin(.5)) < 1e-6 ))
    exit 0
}
EOF



function .sh.math.max x y z
{
    .sh.value=x
    (( y > x )) && .sh.value=y
    (( z > .sh.value )) && .sh.value=z
}
(( max(max(3,8,5),7,5)==8)) || log_error 'max(max(3,8,5),7,5)!=8'
(( max(max(3,8,5),7,9)==9)) || log_error 'max(max(3,8,9),7,5)!=9'
(( max(6,max(3,9,5),7)==9 )) || log_error 'max(6,max(3,8,5),7)!=9'
(( max(6,7, max(3,8,5))==8 )) || log_error 'max(6,7,max(3,8,5))!=8'

enum color_t=(red green blue yellow)
color_t shirt pants=blue
(( pants == blue )) || log_error 'pants should be blue'
(( shirt == red )) || log_error 'pants should be red'
(( shirt != green )) || log_error 'shirt should not be green'
(( pants != shirt )) || log_error 'pants should be the same as shirt'
(( pants = yellow ))
(( pants == yellow )) || log_error 'pants should be yellow'

unset z
integer -a z=( [1]=90 )
function x
{
    nameref nz=$1
    float x y
    float x=$((log10(nz))) y=$((log10($nz)))
    (( abs(x-y) < 1e-10 )) || log_error '$nz and nz differs in arithmetic expression when nz is reference to array instance'
}
x z[1]

unset x
float x
x=$( ($SHELL -c 'print -- $(( asinh(acosh(atanh(sin(cos(tan(atan(acos(asin(tanh(cosh(sinh(asinh(acosh(atanh(sin(cos(tan(atan(acos(asin(tanh(cosh(sinh(.5)))))))))))))))))))))))) )) ';:) )
(( abs(x-.5) < 1.e-10 )) || log_error 'bug in composite function evaluation'

unset x
typeset -X x=16
{ (( $x == 16 )); } || log_error 'expansions of hexfloat not working in arithmetic expansions'

unset foo
function foobar
{
    (( foo = 8))
}
typeset -i foo
foobar
(( foo == 8 )) || log_error  'arithmetic assignment binding to the wrong scope'

(( tgamma(4)/12 )) || log_error 'floating point attribute for functions not preserved'

unset F
function f
{
 ((F=1))
}
f
[[ $F == 1 ]] || log_error 'scoping bug with arithmetic expression'

F=1
function f
{
 typeset F
 ((F=2))
}
[[ $F == 1 ]] || log_error 'scoping bug2 with arithmetic expression'

unset play foo x z
typeset -A play
x=foo
play[$x]=(x=2)
for ((i=0; i < 2; i++))
do
    (( play[$x].y , z++  ))
done
(( z==2 )) || log_error 'unset compound array variable error with for loop optimization'

[[ $($SHELL -c 'print -- $(( ldexp(1, 4) ))' ) == 16 ]] ||
    log_error 'function ldexp not implement or not working correctly'


$SHELL -c 'str="0x1.df768ed398ee1e01329a130627ae0000p-1";typeset -l -E x;((x=str))' ||
    log_error '((x=var)) fails for hexfloat with var begining with 0x1.nnn'

x=(3 6 12)
(( x[2] /= x[0]))
(( x[2] == 4 ))  || log_error '(( x[2] /= x[0])) fails for index array'

x=([0]=3 [1]=6 [2]=12)
(( x[2] /= x[0]))
(( x[2] == 4 )) || log_error '(( x[2] /= x[0])) fails for associative array'

actual=$($SHELL -c 'compound -a x;compound -a x[0].y; integer -a x[0].y[0].z; (( x[0].y[0].z[2]=3 )); typeset -p x')
expect='typeset -C -a x=((typeset -C -a y=( [0]=(typeset -a -l -i z=([2]=3);))))'
[[ "$actual" == "$expect" ]] || log_error '(( x[0].y[0].z[2]=3 )) not working' "$expect" "$actual"

unset x
let x=010
[[ $x == 10 ]] || log_error 'let treating 010 as octal'
set -o letoctal
let x=010
[[ $x == 8 ]] || log_error 'let not treating 010 as octal with letoctal on'

float z=0
integer aa=2 a=1
typeset -A A
A[a]=(typeset -A AA)
A[a].AA[aa]=1
(( z= A[a].AA[aa]++ ))
(( z == 1 )) ||  log_error "z should be 1 but is $z for associative array of
associative array arithmetic"
[[ ${A[a].AA[aa]} == 2 ]] || log_error '${A[a].AA[aa]} should be 2 after ++ operation for associative array of associative array arithmetic'
unset A[a]

A[a]=(typeset -a AA)
A[a].AA[aa]=1
(( z += A[a].AA[aa++]++ ))
(( z == 2 )) ||  log_error "z should be 2 but is $z for associative array of index array arithmetic"
(( aa == 3 )) || log_error "subscript aa should be 3 but is $aa after ++"
[[ ${A[a].AA[aa-1]} == 2 ]] || log_error '${A[a].AA[aa]} should be 2 after ++ operation for ssociative array of index array arithmetic'
unset A

typeset -a A
A[a]=(typeset -A AA)
A[a].AA[aa]=1
(( z += A[a].AA[aa]++ ))
(( z == 3 )) ||  log_error "z should be 3 but is $z for index array of associative array arithmetic"
[[ ${A[a].AA[aa]} == 2 ]] || log_error '${A[a].AA[aa]} should be 2 after ++ operation for index array of associative array arithmetic'
unset A[a]

A[a]=(typeset -a AA)
A[a].AA[aa]=1
(( z += A[a++].AA[aa++]++ ))
(( z == 4 )) ||  log_error "z should be 4 but is $z for index array of
index array arithmetic"
[[ ${A[a-1].AA[aa-1]} == 2 ]] || log_error '${A[a].AA[aa]} should be 2 after ++ operation for index array of index array arithmetic'
(( aa == 4 )) || log_error "subscript aa should be 4 but is $aa after ++"
(( a == 2 )) || log_error "subscript a should be 2 but is $a after ++"
unset A

unset r x
integer x
r=020
(($r == 16)) || log_error 'leading 0 not treated as octal inside ((...))'
x=$(($r))
(( x == 16 )) || log_error 'leading 0 not treated as octal inside $((...))'
x=$r
((x == 20 )) || log_error 'leading 0 should not be treated as octal outside ((...))'
print -- -020 | read x
((x == -20)) || log_error 'numbers with leading -0 should not be treated as octal outside ((...))'
print -- -8#20 | read x
((x == -16)) || log_error 'numbers with leading -8# should be treated as octal'

unset x
x=0x1
let "$x==1" || log_error 'hex constants not working with let'
(( $x == 1 )) || log_error 'arithmetic with $x, where $x is hex constant not working'
for i in 1
do
    (($x == 1)) || log_error 'arithmetic in for loop with $x, where $x is hex constant not working'
done

x=010
let "$x==10" || log_error 'arithmetic with $x where $x is 010 should be decimal in let'
(( 9.$x == 9.01 )) || log_error 'arithmetic with 9.$x where x=010 should be 9.01'
(( 9$x == 9010 )) || log_error 'arithmetic with 9$x where x=010 should be 9010'

x010=99
((x$x == 99 )) || log_error 'arithtmetic with x$x where x=010 should be $x010'
(( 3+$x == 11 )) || log_error '3+$x where x=010 should be 11 in ((...))'

let "(3+$x)==13" || log_error 'let should not recognize leading 0 as octal'
unset x
typeset -RZ3 x=10
(( $x == 10 )) || log_error 'leading 0 in -RZ should not create octal constant with ((...))'
let "$x==10" || log_error 'leading 0 in -RZ should not create octal constant with let'

unset v x
expect=0x1.0000000000000000000000000000p+6
actual=$(printf $'%.28a\n' 64)
[[ $actual == $expect ]] || log_error "'printf %.28a 64' failed" "$expect" "$actual"

# redirections with ((...)) should not cause a syntax error
$SHELL '(($(echo 1+1 | tee /dev/fd/3))) >/dev/null 3>&1'
((  $? )) && log_error 'redirections with ((...))) yield a syntax error'

(( (2 ** 63) ==  2*((2 ** 63)/2) )) || log_error 'integer division with numbers near intmax not working'

$SHELL -c '(( (2**63 / -1) == -(2**63) ))' || log_error 'integer division with denominator -1 fails'

# tests for math functions with array arguments
function .sh.math.mean arr
{
    IFS=+
    typeset var="${arr[*]}"
    (( .sh.value =  ($var)/${#arr[@]} ))
}

function .sh.math.median arr
{
    set -s -A arr1 -- "${arr[@]}"
    integer m=${#arr[@]}
    ((.sh.value = arr1[m/2] ))
}

function .sh.math.dotprod arr1 arr2
{
    integer m=${#arr1[@]} n=${#arr2[@]}
    typeset x y
    set -A  x -- ${arr1[@]}
    set -A  y -- ${arr2[@]}
    (( .sh.value=0 ))
    (( m < n )) && ((n=m))
    for ((m=0; m < n; m++))
    do
        ((.sh.value += x[m]*y[m] ))
    done
}

function .sh.math.norm arr
{
    ((.sh.value = sqrt(dotprod(arr,arr)) ))
}

x=( 9.2 2 3 6.4 5)
y=( 1 2 3)
z=([zero]=9.2 [one]=2  [two]=3 [three]=6.4 [four]=5)

actual=$(( mean(x) ))
expect=5.12
fequal $actual $expect || log_error "mean of index array wrong" "$expect" "$actual"

actual=$(( mean(z) ))
expect=5.12
fequal $actual $expect || log_error "mean of associative array wrong" "$expect" "$actual"

actual=$(( median(x) ))
expect=5
fequal $actual $expect || log_error "median of index array wrong" "$expect" "$actual"

actual=$(( median(z) ))
expect=5
fequal $actual $expect || log_error "median of associative wrong" "$expect" "$actual"

actual=$(( dotprod(x, y) ))
expect=22.2
fequal $actual $expect || log_error "dotprod of two index arrays wrong" "$expect" "$actual"

actual=$(( dotprod(x, x) ))
expect=163.6
fequal $actual $expect || log_error "dotprod of two identical index arrays wrong" "$expect" "$actual"

actual=$(( dotprod(z, y) ))
expect=28.2
fequal $actual $expect || log_error "dotprod of index and associative array wrong" "$expect" "$actual"

actual=$(( dotprod(z, z) ))
expect=163.6
fequal $actual $expect || log_error "dotprod of two identical associaive arrays wrong" "$expect" "$actual"

actual=$(( norm(x) ))
expect=$(( sqrt(163.6) ))
fequal $actual $expect || log_error "norm of index array wrong" "$expect" "$actual"

actual=$(( norm(z) ))
expect=$(( sqrt(163.6) ))
fequal $actual $expect || log_error "norm of associative array wrong" "$expect" "$actual"

# Note: When running under ASAN this can cause the stack to grow to more than 8MB. You may need to
# do `ulimit -s -S 16384` for this to pass.
$SHELL -c 'for ((i = 0; i < 1023; i++)); do eval a$i=a$((i+1)); done; a1023=999; print $((a0))' ||
    log_error 'arithmetic recursive evaluation too deep'

integer count=0 i
compound -a x=( (pid=1) (pid=2) )
for ((i=0; i < 2; i++))
do
    (( x[i].pid == x[0].pid )) && ((count++))
done
(( count==1 )) || log_error 'x[i].pid==x[0].pid should be true only once'

#bug with short integers that causes core dumps
$SHELL <<- \EOF || log_error 'short integer bug causing core dumps'
    typeset -s -i -a t
    typeset -s -i p
    (( p=2**17 )) # tape start position
    (( t[p]+=13))
    while (( t[p] != 0 ))
    do
        ((t[p]-=1 , p+=1))
    done
    exit 0
EOF

float x
((x.HOGWARDS_IN_THE_SKY ==0 )) || log_error 'x.HOGWARDS_IN_THE_SKY is unknown and should have value 0'

unset IFS i
set -u
float -a ar
function f
{
    integer i=0 ar_i=0
    for    (( i=0 ; i < 3 ; i++ ))
    do
        (( ar[ar_i++]=i))
    done
    printf "%q\n" "${ar[*]}"
}
[[ $(f) == "'0 1 2'" ]] ||
    log_error '0 value for variable in arithmetic expression inside function with set -u fails'

[[ $(( (2**32) << 67 )) == 0 ]] || log_error 'left shift count 67 is non-zero'

[[ 0x123 -eq 0x122+0x1 ]] || log_error "[[...]] does not support math operations on hexadecimal numbers"
