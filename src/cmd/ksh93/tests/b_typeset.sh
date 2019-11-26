# Tests for `typeset` builtin.

integer n=2

typeset -T Type_t=(
    typeset name=foobar
    typeset x=(hi=ok bar=yes)
    typeset y=(xa=xx xq=89)
    typeset -A aa=([one]=abc [two]=def)
    typeset -a ia=(abc def)
    typeset -i z=5
)

for ((i=0; i < 10; i++))
do
    Type_t r s
    [[ $r == "$s" ]] || log_error 'r is not equal to s'
    typeset -C x=r.x
    y=(xa=bb xq=cc)
    y2=xyz
    z2=xyz
    typeset -C z=y
    [[ $y == "$z" ]] || log_error 'y is not equal to z'
    typeset -C s.y=z
    [[ $y == "${s.y}" ]] || log_error 'y is not equal to s.y'
    .sh.q=$y
    typeset -C www=.sh.q
    [[ $www == "$z" ]] || log_error 'www is not equal to z'
    typeset -C s.x=r.x
    [[ ${s.x} == "${r.x}" ]] || log_error 's.x is not equal to r.x'

    function foo
    {
        nameref x=$1 y=$2
        typeset z=$x
        y=$x
        [[ $x == "$y" ]] || log_error "x is not equal to y with ${!x}"
    }
    foo r.y y
    [[ $y == "${r.y}" ]] || log_error 'y is not equal to r.y'
    typeset -C y=z
    foo y r.y
    [[ $y == "${r.y}" ]] || log_error 'y is not equal to r.y again'
    typeset -C y=z
    (
        q=${z}
        [[ $q == "$z" ]] || log_error 'q is not equal to z'
        z=abc
    )
    [[ $z == "$y" ]] || log_error 'value of z not preserved after subshell'
    unset z y r s x z2 y2 www .sh.q
done

typeset -T Frame_t=( typeset file lineno )
Frame_t frame
[[ $(typeset -p frame) == 'Frame_t frame=()' ]] || log_error 'empty fields in type not displayed'
x=( typeset -a arr=([2]=abc [4]=(x=1 y=def));zz=abc)
typeset -C y=x
[[ "$x" == "$y" ]] || print -u2 'y is not equal to x'
Type_t z=(y=(xa=bb xq=cc))
typeset -A arr=([foo]=one [bar]=2)
typeset -A brr=([foo]=one [bar]=2)
[[ "${arr[@]}" == "${brr[@]}" ]] || log_error 'arr is not brr'
for ((i=0; i < 1; i++))
do
    typeset -m zzz=x
    [[ $zzz == "$y" ]] || log_error 'zzz is not equal to y'
    typeset -m x=zzz
    [[ $x == "$y" ]] || log_error 'x is not equal to y'
    Type_t t=(y=(xa=bb xq=cc))
    typeset -m r=t
    [[ $r == "$z" ]] || log_error 'r is not equal to z'
    typeset -m t=r
    [[ $t == "$z" ]] || log_error 't is not equal to z'
    typeset -m crr=arr
    [[ "${crr[@]}" == "${brr[@]}" ]] || log_error 'crr is not brr'
    typeset -m arr=crr
    [[ "${arr[@]}" == "${brr[@]}" ]] || log_error 'brr is not arr'
done

typeset -m brr[foo]=brr[bar]
[[ ${brr[foo]} == 2 ]] || log_error 'move an associative array element fails'
[[ ${brr[bar]} ]] && log_error 'brr[bar] should be unset after move'
unset x y zzz
x=(a b c)
typeset -m x[1]=x[2]
[[ ${x[1]} == c ]] || log_error 'move an indexed array element fails'
[[ ${x[2]} ]] && log_error 'x[2] should be unset after move'
cat > $TEST_DIR/types <<- \+++
	typeset -T Pt_t=(float x=1. y=0.)
	Pt_t p=(y=2)
	print -r -- ${p.y}
+++
expected=2
got=$(. $TEST_DIR/types) 2>/dev/null
[[ "$got" == "$expected" ]] || log_error "typedefs in dot script failed -- expected '$expected', got '$got'"
typeset -T X_t=(
    typeset x=foo y=bar
    typeset s=${_.x}
    create()
    {
        _.y=bam
    }
)
X_t x
[[ ${x.x} == foo ]] || log_error 'x.x wrong' "foo" "${x.x}"
[[ ${x.y} == bam ]] || log_error 'x.y wrong' "bam" "${x.y}"
[[ ${x.s} == ${x.x} ]] || log_error 'x.s != x.x' "${x.x}" "${x.s}"
typeset -T Y_t=( X_t r )
Y_t z
[[ ${z.r.x} == foo ]] || log_error 'z.r.x wrong' "foo" "${z.r.x}"
[[ ${z.r.y} == bam ]] || log_error 'z.r.y wrong' "bam" "${z.r.y}"
# TODO: Re-enable this test. It fails consistently on some platforms (e.g., OpenSuse and Fedora 28)
# depending on the value of STK_FSIZE. See https://github.com/att/ast/issues/1088.
# [[ ${z.r.s} == ${z.r.x} ]] || log_error 'z.r.s != z.r.x' "${z.r.x}" "${z.r.s}"

unset xx yy
typeset -T xx=(typeset yy=zz)
xx=yy
( typeset -T xx=(typeset yy=zz) ) 2>/dev/null && log_error 'type redefinition should fail'
$SHELL 2> /dev/null <<- +++ || log_error 'typedef with only f(){} fails'
	typeset -T X_t=(
		f()
		{
			print ok
		}
	)
+++
$SHELL 2> /dev/null <<- +++ || log_error 'unable to redefine f discipline function'
	typeset -T X_t=(
		x=1
		f()
		{
			print ok
		}
	)
	X_t z=(
		function f
		{
			print override f
		}
	)
+++
$SHELL 2> /dev/null <<- +++ && log_error 'invalid discipline name should be an error'
	typeset -T X_t=(
		x=1
		f()
		{
			print ok
		}
	)
	X_t z=(
		function g
		{
			print override f
		}
	)
+++

# Compound variables containing type variables
Type_t r
var=(
    typeset x=foobar
    Type_t    r
    integer z=5
)
[[ ${var.r} == "$r" ]] || log_error 'var.r != r'
(( var.z == 5)) || log_error 'var.z !=5'
[[ "$var" == *x=foobar* ]] || log_error '$var does not contain x=foobar'

typeset -T A_t=(
    typeset x=aha
    typeset b=${_.x}
)
unset x
A_t x
expected=aha
got=${x.b}
[[ "$got" == "$expected" ]] || log_error "type '_' reference failed -- expected '$expected', got '$got'"

typeset -T Tst_t=(
     function f
     {
         A_t a
         print ${ _.g ${a.x}; }
     }
     function g
     {
         print foo
     }
)
Tst_t tst
expected=foo
got=${ tst.f;}
[[ "$got" == "$expected" ]] || log_error "_.g where g is a function in type discipline method failed -- expected '$expected', got '$got'"

typeset -T B_t=(
    integer -a arr
    function f
    {
        (( _.arr[0] = 0 ))
        (( _.arr[1] = 1 ))
        print ${_.arr[*]}
    }
)
unset x
B_t x
expected='0 1'
got=${ x.f;}
[[ "$got" == "$expected" ]] || log_error "array assignment of subscripts in type discipline arithmetic failed -- expected '$expected', got '$got'"

typeset -T Fileinfo_t=(
    size=-1
    typeset -a text=()
    integer mtime=-1
)
Fileinfo_t -A _Dbg_filenames
Fileinfo_t finfo
function bar
{
    finfo.text=(line1 line2 line3)
    finfo.size=${#finfo.text[@]}
    _Dbg_filenames[foo]=finfo
}
bar

expected='Fileinfo_t -A _Dbg_filenames=([foo]=(size=3;typeset -a text=(line1 line2 line3);typeset -l -i mtime=-1))'
got=$(typeset -p _Dbg_filenames)
[[ "$got" == "$expected" ]] || {
    got=$(printf %q "$got")
    log_error "copy to associative array of types in function failed -- expected '$expected', got $got"
}

$SHELL > /dev/null  <<- '+++++' || log_error 'passing _ as nameref arg not working'
	function f1
	{
		typeset -n v=$1
		print -r -- "$v"
	}
	typeset -T A_t=(
		typeset blah=xxx
		function f { f1 _ ;}
	)
	A_t a
	[[ ${ a.f ./t1;} == "$a" ]]
+++++
expected='A_t b.a=(name=one)'
[[ $( $SHELL << \+++
    typeset -T A_t=(
         typeset name=aha
    )
    typeset -T B_t=(
         typeset     arr
         A_t         a
         f()
         {
             _.a=(name=one)
             typeset -p _.a
         }
    )
    B_t b
    b.f
+++
) ==  "$expected" ]] 2> /dev/null || log_error  '_.a=(name=one) not expanding correctly'
expected='A_t x=(name=xxx)'
[[ $( $SHELL << \+++
    typeset -T A_t=(
        typeset name
    )
    A_t x=(name="xxx")
    typeset -p x
+++
) ==  "$expected" ]] || log_error  'empty field in definition does not expand correctly'

typeset -T Foo_t=(
    integer x=3
    integer y=4
    len() { print -r -- $(( sqrt(_.x**2 + _.y**2))) ;}
)
Foo_t foo
[[ ${foo.len} == 5 ]] || log_error "discipline function len not working"

typeset -T benchmark_t=(
    integer num_iterations
)
function do_benchmarks
{
    nameref tst=b
    integer num_iterations
    (( num_iterations= int(tst.num_iterations * 1.0) ))
    printf "%d\n" num_iterations
}
benchmark_t b=(num_iterations=5)
[[  $(do_benchmarks) == 5 ]] || log_error 'scoping of nameref of type variables in arithmetic expressions not working'

function cat_content
{
    cat <<- EOF
	(
		foo_t -a foolist=(
			( val=3 )
			( val=4 )
			( val=5 )
		)
	)
	EOF
    return 0
}
typeset -T foo_t=(
    integer val=-1
    function print
    {
        print -- ${_.val}
    }
)

function do_something
{
    nameref li=$1 # "li" may be an index or associative array
    li[2].print
}
cat_content | read -C x
[[ $(do_something x.foolist) == 5  ]] || log_error 'subscripts not honored for arrays of type with disciplines'

typeset -T benchcmd_t=(
    float x=1
    float y=2
)
unset x
compound x=(
    float o
    benchcmd_t -a m
    integer h
)
expected=$'(\n\ttypeset -l -i h=0\n\tbenchcmd_t -a m\n\ttypeset -l -E o=0\n)'
[[ $x == "$expected" ]] || log_error 'compound variable with array of types with no elements not working'

expected=$'Std_file_t db.file[/etc/profile]=(action=preserve;typeset -A sum=([8242e663d6f7bb4c5427a0e58e2925f3]=1);)'
{
  got=$($SHELL <<- \EOF
	MAGIC='stdinstall (at&t research) 2009-08-25'
	typeset -T Std_file_t=(
		typeset action
		typeset -A sum
	)
	typeset -T Std_t=(
		typeset magic=$MAGIC
		Std_file_t -A file
	)
	Std_t db=(magic='stdinstall (at&t research) 2009-08-25';Std_file_t -A file=( [/./home/gsf/.env.sh]=(action=preserve;typeset -A sum=([9b67ab407d01a52b3e73e3945b9a3ee0]=1);)[/etc/profile]=(action=preserve;typeset -A sum=([8242e663d6f7bb4c5427a0e58e2925f3]=1);)[/home/gsf/.profile]=(action=preserve;typeset -A sum=([3ce23137335219672bf2865d003a098e]=1);));)
	typeset -p db.file[/etc/profile]
	EOF)
} 2> /dev/null
[[ $got == "$expected" ]] ||  log_error 'types with arrays of types as members fails'

typeset -T x_t=(
    integer dummy
    function set
    {
        [[ ${.sh.name} == v ]] || log_error  "name=${.sh.name} should be v"
        [[ ${.sh.subscript} == 4 ]] || log_error "subscript=${.sh.subscript} should be 4"
        [[ ${.sh.value} == hello ]] || log_error  "value=${.sh.value} should be hello"
    }
)
x_t -a v
v[4]="hello"

typeset -T oset=(
    typeset -A s
)
oset foo bar
: ${foo.s[a]:=foobar}
: ${bar.s[d]:=foobar}
[[ ${bar.s[a]} == foobar ]] && log_error '${var:=val} for types assigns to type instead of type instance'

typeset -T olist=(
    typeset -a l
)
olist foo
foo.l[1]=x
[[  ${!foo.l[*]} == *0* ]] && '0-th elment of foo.l should not be set'

typeset -T oset2=( typeset -A foo )
oset2 bar
: ${bar.foo[a]}
bar.foo[a]=b
[[ ${#bar.foo[*]} == 1 ]] || log_error "bar.foo should have 1 element not  ${#bar.foo[*]}"
[[ ${bar.foo[*]} == b ]] || log_error "bar.foo[*] should be 'b'  not  ${bar.foo[*]}"
[[ ${bar.foo[a]} == b ]] || log_error "bar.foo[a] should be 'b'  not  ${bar.foo[*]}"

{ x=$( $SHELL 2> /dev/null << \++EOF++
	typeset -T ab_t=(
		integer a=1 b=2
		function increment
		{
			(( _.a++, _.b++ ))
		}
	)
	function ar_n
	{
		nameref sn=$2
		sn.increment
		$1 && printf "a=%d, b=%d\n" sn.a sn.b
	}
	function ar
	{
		ab_t -S -a s
		[[ -v s[5] ]] || s[5]=( )
		ar_n $1 s[5]
	}
	x=$(ar false ; ar false ; ar true ; printf ";")
	y=$(ar false ; ar false ; ar true ; printf ";")
	print -r -- "\"$x\"" ==  "\"$y\""
++EOF++
) ;} 2> /dev/null
[[ $x == *a=4*b=5* ]] || log_error 'static types in a function not working'
{ eval "[[ $x ]]";} 2> /dev/null || log_error 'arrays of types leaving side effects in subshells'

typeset -T y_t=(
    typeset dummy
    function print_b
    {
        print "B"
    }
)
y_t a b=(
    function print_b
    {
        print "1"
    }
)
[[ $(a.print_b) == B ]] || log_error 'default discipline not working'
[[ $(b.print_b) == 1 ]] || log_error 'discipline override not working'

$SHELL 2> /dev/null -c 'true || { typeset -T Type_t=(typeset name=foo);
    Type_t z=(name=bar) ;}' || log_error 'unable to parse type command until typeset -T executes'

cd "$TEST_DIR"
FPATH=$PWD
PATH=$PWD:$PATH
cat > A_t <<-  \EOF
	typeset -T A_t=(
		B_t b
	)
EOF
cat > B_t <<-  \EOF
	typeset -T B_t=(
		integer n=5
	)
EOF

unset n
if n=$(FPATH=$PWD PATH=$PWD:$PATH $SHELL 2> /dev/null -c 'A_t a; print ${a.b.n}')
then
    (( n==5 )) || log_error 'dynamic loading of types gives wrong result'
else
    log_error 'unable to load types dynamically'
fi


# check that typeset -T reproduces a type.
if $SHELL  > /dev/null 2>&1  -c 'typeset -T'
then
    $SHELL > junk1 <<- \+++EOF
	typeset -T foo_t=(
		integer x=3 y=4
		float z=1.2
		len()
		{
			((.sh.value=sqrt(_.x**2 + _.y**2) ))
		}
		function count
		{
			print z=$z
		}
	)
	typeset -T
	print 'typeset -T'
	+++EOF
    $SHELL -c '. ./junk1;print "typeset -T"' > junk2
    diff junk[12] > /dev/null || log_error 'typeset -T not idempotent'
    $SHELL -c '. ./junk1;print "typeset +f"' > junk2
    [[ -s junk2 ]] || log_error 'non-discipline-method functions found'
else
    log_error 'typeset -T not supported'
fi


[[ $($SHELL -c 'typeset -T x=( typeset -a h ) ; x j; print -v j.h') ]] && log_error 'type with indexed array without elements inserts element 0'

[[ $($SHELL  -c 'typeset -T x=( integer -a s ) ; compound c ; x c.i ; c.i.s[4]=666 ; print -v c') == *'[0]'* ]] &&  log_error 'type with indexed array with non-zero element inserts element 0'


{ $SHELL -c '(sleep 3;kill $$)& typeset -T x=( typeset -a s );compound c;x c.i;c.i.s[7][5][3]=hello;x c.j=c.i;[[ ${c.i} == "${c.j}" ]]';} 2> /dev/null
exitval=$?
if [[ $(kill -l $exitval) == TERM ]]
then
    log_error 'clone of multi-dimensional array timed out'
elif ((exitval))
then
    log_error "c.i and c.j are not the same multi-dimensional array"
fi


typeset -T foobar_t=(
    float x=1 y=0
    slen()
    {
        print -r -- $((sqrt(_.x**2 + _.y**2)))
    }
    typeset -fS slen
    len()
    {
        print -r -- $((sqrt(_.x**2 + _.y**2)))
    }
)
unset z
foobar_t z=(x=3 y=4)
(( z.len == 5 )) || log_error 'z.len should be 5'
(( z.slen == 1 )) || log_error 'z.slen should be 1'
(( .sh.type.foobar_t.slen == 1 )) || log_error '.sh.type.foobar_t.slen should be 1'
(( .sh.type.foobar_t.len == 1 )) || log_error '.sh.type.foobar_t.len should be 1'

typeset -T z_t=( typeset -a ce )
z_t x1
x1.ce[3][4]=45
compound c
z_t -a c.x2
c.x2[9]=x1
got=$(typeset +p "c.x2[9].ce")
exp='typeset -a c.x2[9].ce'
[[ $got == "$exp" ]] || log_error "typeset +p 'c.x2[9].ce' failed -- expected '$exp', got '$got'"

unset b
typeset -T a_t=(
    typeset a="hello"
)
typeset -T b_t=(
    a_t b
)
compound b
compound -a b.ca
b_t b.ca[4].b
exp='typeset -C b=(typeset -C -a ca=( [4]=(b_t b=(a_t b=(a=hello)))))'
got=$(typeset -p b)
[[ $got == "$exp" ]] || log_error 'typeset -p of nested type not correct'

typeset -T u_t=(
    integer dummy
    unset()
    {
        print unset
    }
)
unset z
u_t -a x | read z
[[ $z == unset ]]  && log_error 'unset discipline called on type creation'

expect='typeset -T foo'
{ actual=$($SHELL 'typeset -T foo; typeset -T') ;}
[[ $actual == $expect ]] || log_error '"typeset -T foo; typeset -T" failed' "$expect" "$actual"

{ z=$($SHELL 2> /dev/null 'typeset -T foo=bar; typeset -T') ;} 2> /dev/null
[[ $z ]] && log_error '"typeset -T foo=bar" should not creates type foo'

{
$SHELL << \EOF
	typeset -T board_t=(
		compound -a board_y
		function binsert
		{
			nameref figure=$1
			integer y=$2 x=$3
			typeset -m "_.board_y[y].board_x[x].field=figure"
		}
	)
	function main
	{
		compound c=(
			board_t b
		)
		for ((i=0 ; i < 2 ; i++ )) ; do
			compound p=( hello=world )
			c.b.binsert p 1 $i
		done
		exp='typeset -C c=(board_t b=(typeset -C -a board_y=( [1]=(typeset -a board_x=( [0]=(field=(hello=world;););[1]=(field=(hello=world)))))))'
		[[ $(typeset -p c) == "$exp" ]] || exit 1
	}
	main
EOF
} 2> /dev/null
if (( exitval=$?))
then
    if [[ $(kill -l $exitval) == SEGV ]]
    then
        log_error 'typeset -m in type discipline causes exception'
    else
        log_error 'typeset -m in type discipline gives wrong value'
    fi
fi

typeset -T pawn_t=(
    print_debug()
    {
        print 'PAWN'
    }
)

function main
{
    compound c=(
        compound -a board
    )

    for ((i=2 ; i < 8 ; i++ )) ; do
        pawn_t c.board[1][$i]
    done

}
main 2> /dev/null && log_error 'type assignment to compound array instance should generate an error'

{    $SHELL -c 'typeset -T Foo_t=(integer -a data=([0]=0) );Foo_t x=(data[0]=2);((x.data[0]==2))'
} 2> /dev/null || log_error 'type definition with integer array variable not working'

typeset -T Bar_t=(
    typeset -a foo
)
Bar_t bar
bar.foo+=(bam)
[[ ${bar.foo[0]} == bam ]] || log_error 'appending to empty array variable in type does not create element 0'

$SHELL 2> /dev/null -c 'typeset -T y_t=(compound extensions);y_t x;x.extensions=( integer i=1 )' || log_error 'compound variable defined in type unable to be extended'

typeset -T addme_t=(
    compound c=(
        compound ar
    )
)
addme_t ad
unset arr ref
nameref ref=ad.c
nameref arr=ref.ar
arr+=( value=foo value2=bar )
[[ ${ad.c.ar.value} == foo ]] || log_error 'references to type elements not working'

typeset -T fifo_t=(
    typeset fifo_name
    integer in_fd
    integer out_fd
    cinit()
    {
        _.fifo_name="$1"
        > ${_.fifo_name}
        redirect {_.out_fd}> ${_.fifo_name}
        redirect {_.in_fd}< ${_.fifo_name}
    }
)

compound c
fifo_t -A c.ar
c.ar[a].cinit fifo_a
c.ar[b].cinit fifo_b
[[ ${c.ar[a].fifo_name} == fifo_a ]] || log_error 'fifo_name c.ar[a] not fifo_a'
[[ ${c.ar[b].fifo_name} == fifo_b ]] || log_error 'fifo_name c.ar[b] not fifo_b'
[[ ${c.ar[a].in_fd} == "${c.ar[b].in_fd}" ]] && log_error 'c.ar[a].in_fd and c.ar[b].in_fd are the same'
redirect  {c.ar[a].in_fd}<&-
redirect  {c.ar[b].in_fd}<&-
redirect  {c.ar[a].out_fd}>&-
redirect  {c.ar[b].out_fd}>&-
rm -f fifo_a fifo_b

typeset -T Z_t=(compound -a x)
Z_t z
[[ $(typeset -p z.x) ==  *'-C -a'* ]] || log_error 'typeset -p for compound array element not displaying attributes'

out='foo f 123'
typeset -T bam_t=(
    f=123
    function out { print foo f ${_.f}; }
)
bam_t f
[[ ${f.f} == 123 ]] || log_error "f.f is ${f.f} should be 123"
[[ ${ f.out } == "$out" ]] 2> /dev/null || log_error "f.out is ${ f.out } should be $out"
typeset -T bar_t=(
    bam_t foo
    b=456
    function out { _.foo.out; }
)
bar_t b
[[ ${b.b} == 456 ]] || log_error "b.b is ${b.b} should be 456"
[[ ${ b.out } == "$out" ]] || log_error "b.out is ${ b.out } should be $out"
typeset -T baz_t=(
    bar_t bar
    z=789
    function out { _.bar.out ;}
)
baz_t z
[[ ${z.z} == 789 ]] || log_error "z.z is ${z.z} should be 789"

# TODO: Re-enable this test. It fails consistently on some platforms depending on the value of
# STK_FSIZE. See https://github.com/att/ast/issues/1088.
# [[ ${ z.out } == "$out" ]] 2> /dev/null || log_error "z.out wrong" "$out" "${ z.out }"

$SHELL  2> /dev/null <<- \EOF || log_error 'typeset -p with types not working'
	typeset -T Man_t=( typeset X)
	Man_t Man
	function bootstrap { : ;}
	[[ $(typeset -p) == *Man_t* ]] 2> /dev/null
EOF

typeset -T zz_t=( compound -a bar )
(
exp="$(
        compound c=( zz_t d )
        integer c.d.bar[4][6][8].b=789
        print -v c)"
read -C got  <<< "$exp"
[[ $got == "$exp" ]] || log_error 'read -C for compound variable containing a type not working correctly'
) & wait $!
[[ $? == 0 ]] || log_error 'read -C for compound variable containing a type not working correctly'

unset c
exp=$(compound c=( zz_t d=( typeset -C -a bar=( [4]=( zz_t b=( typeset -C -a bar)))));print -v c)
read -C got <<< "$exp"
[[ $got == "$exp" ]] || log_error 'read -C for compound variable containing a nested type not working correctly'

exp='ub=(
	Job_t -A job=(
		[IVAI]=(
			schedule=UBCYINV1
		)
	)
)'

got=$( $SHELL 2> /dev/null <<- \EOF
	typeset -T Module_t=( typeset hit; typeset -A call contract table_read table_write )
	typeset -T Command_t=( typeset name parm; typeset -A input output module )
	typeset -T Job_t=( typeset schedule; Command_t -a command )
	typeset -T Schedule_t=( typeset -A job )
	typeset -T Ub_t=( Schedule_t -A schedule; Job_t -A job; Module_t -A module )
	Ub_t ub
	typeset s=UBCYINV1
	typeset n=IVAI
	ub.job[$n].schedule=$s
	print -r -- ub="$ub"
EOF)
[[ $? == 0 ]] || log_error 'assignment in nested type fails'
[[ $got == "$exp" ]] || log_error 'assignment in nested type returns wrong value'

got=$($SHELL <<- \EOF
	typeset -T Module_t=( typeset hit; typeset -A call contract table_read table_write )
	typeset -T Command_t=( typeset name parm; typeset -A input output module )
	typeset -T Job_t=( typeset schedule; Command_t -a command )
	typeset -T Schedule_t=( typeset -A job )
	typeset -T Ub_t=( Schedule_t -A schedule; Job_t -A job; Module_t -A module )
	Ub_t ub
	typeset s=SCHEDULE
	typeset j=JOB
	ub.job[$j].schedule=$s
	ub.job[$j].command[0].name=COMMAND-1
	ub.job[$j].command[0].parm='parm-11,parm-12,parm-13'
	ub.job[$j].command[0].input[INPUT]=input-1
	ub.job[$j].command[0].output[OUTPUT]=output-1
	ub.job[$j].command[1].name=COMMAND-2
	ub.job[$j].command[1].parm='parm-21,parm-22,parm-23'
	ub.job[$j].command[1].input[INPUT]=input-2
	ub.job[$j].command[1].output[OUTPUT]=output-2
	print -r "$ub"
EOF)


exp='(
	Job_t -A job=(
		[JOB]=(
			schedule=SCHEDULE
			Command_t -a command=(
				(
					name=COMMAND-1
					parm=parm-11,parm-12,parm-13
					typeset -A input=(
						[INPUT]=input-1
					)
					typeset -A output=(
						[OUTPUT]=output-1
					)
				)
				(
					name=COMMAND-2
					parm=parm-21,parm-22,parm-23
					typeset -A input=(
						[INPUT]=input-2
					)
					typeset -A output=(
						[OUTPUT]=output-2
					)
				)
			)
		)
	)
)'

[[ $got == "$exp" ]] || log_error "expansion of variable containing a type that contains an index array of types not correct."

exp='typeset -T Job_t
typeset -T Schedule_t
typeset -T Ub_t
typeset -T Job_t=(
	typeset schedule
)
typeset -T Schedule_t=(
	typeset -A job
)
typeset -T Ub_t=(
	Schedule_t -A schedule
	Job_t -A job
)'

got=$($SHELL << EOF
$exp
typeset -T
EOF)
[[ $got == "$exp" ]] || log_error 'typeset -T not displaying types correctly'

typeset -T Apar_t=(typeset name)
typeset -T Child_t=(Apar_t apar )
Child_t A=( apar=( name=IV01111))
Child_t C=( apar=( name=IV09557))
[[ ${A.apar.name} == IV01111 ]] || log_error "A.apar.name is ${A.apar.name} should be IV01111"
[[ ${A.apar.name} == "${C.apar.name}" ]] && log_error 'string fields in nested typed not working correctly'

unset ar
Pt_t -a ar=((x=3 y=4) (x=5 y=6))
ar=()
[[ $(typeset -p ar) == 'Pt_t -a ar=()' ]] || log_error 'ar=() for an index array of Pt_t not correct'

unset ar
Pt_t -A ar=([one]=(x=3 y=4) [two]=(x=5 y=6))
ar=()
[[ $(typeset -p ar) == 'Pt_t -A ar=()' ]] || log_error 'ar=() for an associative array of Pt_t not correct'

typeset -T Type=(
    typeset x
        integer  y=5
        function x.get
    {
            ((.sh.value = ++_.y))
        }
    )
Type obj
[[ ${obj.x} == 6 ]] || log_error '_ for type variable not set to type'

got=$($SHELL  2> /dev/null <<- \EOF || log_error 'short integer arrays in types fails'
	typeset -T X_t=(typeset -si -a arr=(7 8) )
	X_t x
	print -r -- $((x.arr[1]))
EOF)
[[ $got == 8 ]] || log_error 'sort integer arrays in types not working correctly'

typeset -T p_t=(
    integer fd
    compound events=( bool pollin=false)
)
compound c
p_t -A c.p
c.p[2]=(fd=0 events=( bool pollin=true))
[[ ${c.p[2].events.pollin} == true ]] || log_error 'c.p[2]=(fd=0 events=( bool pollin=true)) does not set pollin to true'

typeset -T pp_t=( integer fd ;
      function pinit { print $(( _.fd=$1 )) ;}
)
compound c
pp_t -a c.pl
[[ $(c.pl[3].pinit 7) == 7 ]] 2> /dev/null || log_error 'arrays of types in arithmetic expressions not working'

$SHELL 2> /dev/null -c '
typeset -T p_t=( integer fd=-1 ; compound events=(  bool pollin=false ; );)
p_t --man'
[[ $? == 2 ]] || log_error 'unable to generated manpage for types that contain compound variables'

export FPATH=$TEST_DIR/fundir
mkdir -p $FPATH
print 'typeset -T My_t=(integer i j)' > $FPATH/My_t
$SHELL 2> /dev/null -c 'My_t a=(i=1 j=2); [[ "${a.i} ${a.j}" == "1 2" ]]' ||
    log_error "dynamic loading of type with assignment fails"

$SHELL 2> /dev/null -c 'typeset -T My_t=(readonly x); My_t foo' && log_error 'unset type subvariables defined as readonly are required to be specified for each type instance'

typeset -T Goo_t=(typeset x)
Goo_t -A foo
foo[bar]=(x=1) foo[baz]=(x=2)
exp='Goo_t -A foo=([bar]=(x=1) [baz]=(x=2))'
[[ $(typeset -p foo) == "$exp" ]] || print 'typeset -p for associative array of types not correct'

$SHELL <<- \!!! || log_error 'creating an empty array of a type variable with required field fails'
    typeset -T My_t=(readonly x)
    My_t -a foo 2> /dev/null
    foo[0]=(typeset -r x=bar)
    exp='My_t -a foo=((typeset -r x=bar))'
    [[ $(typeset -p foo) == "$exp" ]]
!!!

$SHELL <<- \!!! || log_error 'creating an array with required fields with assigments fails'
    typeset -T My_t=(readonly x)
    My_t -a foo=((x=5) (x=6)) 2> /dev/null
    exp='My_t -a foo=((typeset -r x=5) (typeset -r x=6))'
    [[ $(typeset -p foo) == "$exp" ]]
!!!

$SHELL <<- \!!! && log_error 'creating an array with required fields missing does not fail'
    typeset -T My_t=(typeset -r x y)
    My_t -a foo=((typeset -r x=5;)) 2> /dev/null
!!!

$SHELL <<- \!!! && log_error 'creating an array element with required field missing does not fail'
    typeset -T My_t=(typeset -r x y)
    My_t -a foo
    { foo[1]=(x=2);} 2> /dev/null
!!!

$SHELL <<- \!!! && log_error 'appending an array element with required field missing does not fail'
    typeset -T My_t=(typeset -r x y)
    My_t -a foo
    { foo+=(y=3);} 2> /dev/null
!!!

typeset -T My_t=(integer x y)
My_t -a A
A=()
A+=(x=1 y=2)
exp='My_t -a A=((typeset -l -i x=1;typeset -l -i y=2))'
[[ $(typeset -p A) == "$exp" ]] || log_error 'empty assignment to array of types creates element 0 by mistake'

$SHELL 2> /dev/null -c 'typeset -T a_t=(x=3 y=4); a_t b=(x=1)' || log_error 'Cannot create instances for type names starting with the letter a'

$SHELL 2> /dev/null -c 'typeset -T X=(typeset x; function x.get { :; }); X -a xs=((x=yo) (x=jo)); [[ $(typeset -p xs) == "X -a xs=((x=yo) (x=jo))" ]]' || log_error 'X -a xs=((v1) (v2)) where X is a type, not working'

# ==========
# Verify -u converts string to uppercase regardless of whether or not it is
# also exported or tagged.
typeset -u test_u=uppercase
typeset -xu test_xu=uppercase
typeset -txu test_txu=uppercase
[[ $test_u != "UPPERCASE" ]] && log_error "typeset -u failed"
[[ $test_xu != "UPPERCASE" ]] && log_error "typeset -xu failed"
[[ $test_txu != "UPPERCASE" ]] && log_error "typeset -txu failed"

# ==========
# Ensure "typeset" for "declare and assign" and "assign after declare" behaves the same.
# Regression: https://github.com/att/ast/issues/1312
typeset KEY='k1'

unset A_ASSO
typeset -A A_ASSO
actual=$(typeset -p A_ASSO)
expect='typeset -A A_ASSO=()'
[[ "$actual" == "$expect" ]] ||
    log_error 'typeset -p output incorrect' "$expect" "$actual"

typeset -A A_ASSO[${KEY}].COMPOUND_SUBNAME="declare_and_assign_noindex_fail"
actual=$(typeset -p A_ASSO)
expect='typeset -A A_ASSO=([k1]=(typeset -A COMPOUND_SUBNAME=([0]=declare_and_assign_noindex_fail);))'
[[ "$actual" == "$expect" ]] ||
    log_error 'typeset -p output incorrect' "$expect" "$actual"

unset B_ASSO
typeset -A B_ASSO
typeset -A B_ASSO[${KEY}].COMPOUND_SUBNAME[0]="declare_and_assign_index_succ"
actual=$(typeset -p B_ASSO)
expect='typeset -A B_ASSO=([k1]=(typeset -a COMPOUND_SUBNAME=(declare_and_assign_index_succ);))'
[[ "$actual" == "$expect" ]] ||
    log_error 'typeset -p output incorrect' "$expect" "$actual"

unset C_ASSO
typeset -A C_ASSO
typeset -A C_ASSO[${KEY}].COMPOUND_SUBNAME
C_ASSO[${KEY}].COMPOUND_SUBNAME="assign_after_declare_noindex_succ"
actual=$(typeset -p C_ASSO)
expect='typeset -A C_ASSO=([k1]=(typeset -A COMPOUND_SUBNAME=([0]=assign_after_declare_noindex_succ);))'
[[ "$actual" == "$expect" ]] ||
    log_error 'typeset -p output incorrect' "$expect" "$actual"

unset D_ASSO
typeset -A D_ASSO
typeset -A D_ASSO[${KEY}].COMPOUND_SUBNAME
D_ASSO[${KEY}].COMPOUND_SUBNAME[0]="assign_after_declare_index_succ"
actual=$(typeset -p D_ASSO)
expect='typeset -A D_ASSO=([k1]=(typeset -A COMPOUND_SUBNAME=([0]=assign_after_declare_index_succ);))'
[[ "$actual" == "$expect" ]] ||
    log_error 'typeset -p output incorrect' "$expect" "$actual"

# ==========
# Ensure enumerating functions works if any of them are marked autoloaded but not actually loaded.
# Regression: https://github.com/att/ast/issues/1436
#
# Use a new shell so we don't have to worry about anything done by prior tests vis-a-vis functions.
# Also, the `LANG=C` is because on some platforms the sort order, even for en_US.UTF-8, can differ
# with respect to underscore characters. But the C locale guarantees an ordering we can depend on.
# This deals with the fact some function names have leading underscores to indicate they are private
# to the ksh implementation.
actual=$(LANG=C $SHELL -c 'typeset -f')
expect='typeset -fu _ksh_print_help*typeset -fu pushd'
[[ "$actual" == $expect ]] ||
    log_error 'typeset -f output incorrect' "$expect" "$actual"
