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
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r ${Command}[$1]: "${@:2}"
	(( Errors+=1 ))
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0

tmp=$(mktemp -dt) || { err_exit mktemp -dt failed; exit 1; }
trap "cd /; rm -rf $tmp" EXIT

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
	[[ $r == "$s" ]] || err_exit 'r is not equal to s'
	typeset -C x=r.x
	y=(xa=bb xq=cc)
	y2=xyz
	z2=xyz
	typeset -C z=y
	[[ $y == "$z" ]] || err_exit 'y is not equal to z'
	typeset -C s.y=z
	[[ $y == "${s.y}" ]] || err_exit 'y is not equal to s.y'
	.sh.q=$y
	typeset -C www=.sh.q
	[[ $www == "$z" ]] || err_exit 'www is not equal to z'
	typeset -C s.x=r.x
	[[ ${s.x} == "${r.x}" ]] || err_exit 's.x is not equal to r.x'

	function foo
	{
		nameref x=$1 y=$2
		typeset z=$x
		y=$x
		[[ $x == "$y" ]] || err_exit "x is not equal to y with ${!x}"
	}
	foo r.y y
	[[ $y == "${r.y}" ]] || err_exit 'y is not equal to r.y'
	typeset -C y=z
	foo y r.y
	[[ $y == "${r.y}" ]] || err_exit 'y is not equal to r.y again'
	typeset -C y=z
	(
		q=${z}
		[[ $q == "$z" ]] || err_exit 'q is not equal to z'
		z=abc
	)
	[[ $z == "$y" ]] || err_exit 'value of z not preserved after subshell'
	unset z y r s x z2 y2 www .sh.q
done
typeset -T Frame_t=( typeset file lineno )
Frame_t frame
[[ $(typeset -p frame) == 'Frame_t frame=()' ]] || err_exit 'empty fields in type not displayed'
x=( typeset -a arr=([2]=abc [4]=(x=1 y=def));zz=abc)
typeset -C y=x
[[ "$x" == "$y" ]] || print -u2 'y is not equal to x'
Type_t z=(y=(xa=bb xq=cc))
typeset -A arr=([foo]=one [bar]=2)
typeset -A brr=([foo]=one [bar]=2)
[[ "${arr[@]}" == "${brr[@]}" ]] || err_exit 'arr is not brr'
for ((i=0; i < 1; i++))
do	typeset -m zzz=x
	[[ $zzz == "$y" ]] || err_exit 'zzz is not equal to y'
	typeset -m x=zzz
	[[ $x == "$y" ]] || err_exit 'x is not equal to y'
	Type_t t=(y=(xa=bb xq=cc))
	typeset -m r=t
	[[ $r == "$z" ]] || err_exit 'r is not equal to z'
	typeset -m t=r
	[[ $t == "$z" ]] || err_exit 't is not equal to z'
	typeset -m crr=arr
	[[ "${crr[@]}" == "${brr[@]}" ]] || err_exit 'crr is not brr'
	typeset -m arr=crr
	[[ "${arr[@]}" == "${brr[@]}" ]] || err_exit 'brr is not arr'
done
typeset -m brr[foo]=brr[bar]
[[ ${brr[foo]} == 2 ]] || err_exit 'move an associative array element fails'
[[ ${brr[bar]} ]] && err_exit 'brr[bar] should be unset after move'
unset x y zzz
x=(a b c)
typeset -m x[1]=x[2]
[[ ${x[1]} == c ]] || err_exit 'move an indexed array element fails'
[[ ${x[2]} ]] && err_exit 'x[2] should be unset after move'
cat > $tmp/types <<- \+++
	typeset -T Pt_t=(float x=1. y=0.)
	Pt_t p=(y=2)
	print -r -- ${p.y}
+++
expected=2
got=$(. $tmp/types) 2>/dev/null
[[ "$got" == "$expected" ]] || err_exit "typedefs in dot script failed -- expected '$expected', got '$got'"
typeset -T X_t=(
	typeset x=foo y=bar
	typeset s=${_.x}
	create()
	{
		_.y=bam
	}
)
X_t x
[[ ${x.x} == foo ]] || err_exit 'x.x should be foo'
[[ ${x.y} == bam ]] || err_exit 'x.y should be bam'
[[ ${x.s} == ${x.x} ]] || err_exit 'x.s should be x.x'
typeset -T Y_t=( X_t r )
Y_t z
[[ ${z.r.x} == foo ]] || err_exit 'z.r.x should be foo'
[[ ${z.r.y} == bam ]] || err_exit 'z.r.y should be bam'
[[ ${z.r.s} == ${z.r.x} ]] || err_exit 'z.r.s should be z.r.x'

unset xx yy
typeset -T xx=(typeset yy=zz)
xx=yy
( typeset -T xx=(typeset yy=zz) ) 2>/dev/null && err_exit 'type redefinition should fail'
$SHELL 2> /dev/null <<- +++ || err_exit 'typedef with only f(){} fails'
	typeset -T X_t=(
		f()
		{
			print ok
		}
	)
+++
$SHELL 2> /dev/null <<- +++ || err_exit 'unable to redefine f discipline function'
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
$SHELL 2> /dev/null <<- +++ && err_exit 'invalid discipline name should be an error'
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
# compound variables containing type variables
Type_t r
var=(
	typeset x=foobar
	Type_t	r
	integer z=5
)
[[ ${var.r} == "$r" ]] || err_exit 'var.r != r'
(( var.z == 5)) || err_exit 'var.z !=5'
[[ "$var" == *x=foobar* ]] || err_exit '$var does not contain x=foobar'

typeset -T A_t=(
	typeset x=aha
	typeset b=${_.x}
)
unset x
A_t x
expected=aha
got=${x.b}
[[ "$got" == "$expected" ]] || err_exit "type '_' reference failed -- expected '$expected', got '$got'"

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
[[ "$got" == "$expected" ]] || err_exit "_.g where g is a function in type discipline method failed -- expected '$expected', got '$got'"

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
[[ "$got" == "$expected" ]] || err_exit "array assignment of subscripts in type discipline arithmetic failed -- expected '$expected', got '$got'"

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
	err_exit "copy to associative array of types in function failed -- expected '$expected', got $got"
}

$SHELL > /dev/null  <<- '+++++' || err_exit 'passing _ as nameref arg not working'
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
) ==  "$expected" ]] 2> /dev/null || err_exit  '_.a=(name=one) not expanding correctly'
expected='A_t x=(name=xxx)'
[[ $( $SHELL << \+++
	typeset -T A_t=(
		typeset name
	)
	A_t x=(name="xxx")
	typeset -p x
+++
) ==  "$expected" ]] || err_exit  'empty field in definition does not expand correctly'

typeset -T Foo_t=(
	integer x=3
	integer y=4
	len() { print -r -- $(( sqrt(_.x**2 + _.y**2))) ;}
)
Foo_t foo
[[ ${foo.len} == 5 ]] || err_exit "discipline function len not working"

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
[[  $(do_benchmarks) == 5 ]] || err_exit 'scoping of nameref of type variables in arithmetic expressions not working'

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
[[ $(do_something x.foolist) == 5  ]] || err_exit 'subscripts not honored for arrays of type with disciplines'

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
[[ $x == "$expected" ]] || err_exit 'compound variable with array of types with no elements not working'

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
[[ $got == "$expected" ]] ||  err_exit 'types with arrays of types as members fails'

typeset -T x_t=(
	integer dummy 
	function set
	{
		[[ ${.sh.name} == v ]] || err_exit  "name=${.sh.name} should be v"
		[[ ${.sh.subscript} == 4 ]] || err_exit "subscript=${.sh.subscript} should be 4"
		[[ ${.sh.value} == hello ]] || err_exit  "value=${.sh.value} should be hello"
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
[[ ${bar.s[a]} == foobar ]] && err_exit '${var:=val} for types assigns to type instead of type instance'

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
[[ ${#bar.foo[*]} == 1 ]] || err_exit "bar.foo should have 1 element not  ${#bar.foo[*]}"
[[ ${bar.foo[*]} == b ]] || err_exit "bar.foo[*] should be 'b'  not  ${bar.foo[*]}"
[[ ${bar.foo[a]} == b ]] || err_exit "bar.foo[a] should be 'b'  not  ${bar.foo[*]}"

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
[[ $x == *a=4*b=5* ]] || err_exit 'static types in a function not working'
{ eval "[[ $x ]]";} 2> /dev/null || err_exit 'arrays of types leaving side effects in subshells'

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
[[ $(a.print_b) == B ]] || err_exit 'default discipline not working'
[[ $(b.print_b) == 1 ]] || err_exit 'discipline override not working'

$SHELL 2> /dev/null -c 'true || { typeset -T Type_t=(typeset name=foo);
	Type_t z=(name=bar) ;}' || err_exit 'unable to parse type command until typeset -T executes'

cd "$tmp"
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
if	n=$(FPATH=$PWD PATH=$PWD:$PATH $SHELL 2> /dev/null -c 'A_t a; print ${a.b.n}') 
then	(( n==5 )) || err_exit 'dynamic loading of types gives wrong result'
else	err_exit 'unable to load types dynamically'
fi

# check that typeset -T reproduces a type.
if	$SHELL  > /dev/null 2>&1  -c 'typeset -T'
then	$SHELL > junk1 <<- \+++EOF
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
	diff junk[12] > /dev/null || err_exit 'typeset -T not idempotent'
	$SHELL -c '. ./junk1;print "typeset +f"' > junk2
	[[ -s junk2 ]] || err_exit 'non-discipline-method functions found'
else
	err_exit 'typeset -T not supported'
fi

[[ $($SHELL -c 'typeset -T x=( typeset -a h ) ; x j; print -v j.h') ]] && err_exit 'type with indexed array without elements inserts element 0' 

[[ $($SHELL  -c 'typeset -T x=( integer -a s ) ; compound c ; x c.i ; c.i.s[4]=666 ; print -v c') == *'[0]'* ]] &&  err_exit 'type with indexed array with non-zero element inserts element 0'


{ $SHELL -c '(sleep 3;kill $$)& typeset -T x=( typeset -a s );compound c;x c.i;c.i.s[7][5][3]=hello;x c.j=c.i;[[ ${c.i} == "${c.j}" ]]';} 2> /dev/null
exitval=$?
if	[[ $(kill -l $exitval) == TERM ]]
then	err_exit 'clone of multi-dimensional array timed out'
elif	((exitval))
then	err_exit "c.i and c.j are not the same multi-dimensional array"
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
(( z.len == 5 )) || err_exit 'z.len should be 5'
(( z.slen == 1 )) || err_exit 'z.slen should be 1'
(( .sh.type.foobar_t.slen == 1 )) || err_exit '.sh.type.foobar_t.slen should be 1'
(( .sh.type.foobar_t.len == 1 )) || err_exit '.sh.type.foobar_t.len should be 1'

typeset -T z_t=( typeset -a ce )
z_t x1
x1.ce[3][4]=45
compound c
z_t -a c.x2
c.x2[9]=x1
got=$(typeset +p "c.x2[9].ce")
exp='typeset -a c.x2[9].ce'
[[ $got == "$exp" ]] || err_exit "typeset +p 'c.x2[9].ce' failed -- expected '$exp', got '$got'"

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
[[ $got == "$exp" ]] || err_exit 'typeset -p of nested type not correct'

typeset -T u_t=(
	integer dummy 
	unset()
	{
		print unset
	}
)
unset z
u_t -a x | read z
[[ $z == unset ]]  && err_exit 'unset discipline called on type creation'

{ z=$($SHELL 2> /dev/null 'typeset -T foo; typeset -T') ;} 2> /dev/null
[[ $z == 'typeset -T foo' ]] || err_exit '"typeset -T foo; typeset -T" failed'

{ z=$($SHELL 2> /dev/null 'typeset -T foo=bar; typeset -T') ;} 2> /dev/null
[[ $z ]] && err_exit '"typeset -T foo=bar" should not creates type foo'

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
if	(( exitval=$?))
then	if	[[ $(kill -l $exitval) == SEGV ]]
	then	err_exit 'typeset -m in type discipline causes exception'
	else	err_exit 'typeset -m in type discipline gives wrong value'
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
main 2> /dev/null && err_exit 'type assignment to compound array instance should generate an error'

{	$SHELL -c 'typeset -T Foo_t=(integer -a data=([0]=0) );Foo_t x=(data[0]=2);((x.data[0]==2))'
} 2> /dev/null || err_exit 'type definition with integer array variable not working'

typeset -T Bar_t=(
	typeset -a foo
)
Bar_t bar
bar.foo+=(bam)
[[ ${bar.foo[0]} == bam ]] || err_exit 'appending to empty array variable in type does not create element 0'

$SHELL 2> /dev/null -c 'typeset -T y_t=(compound extensions);y_t x;x.extensions=( integer i=1 )' || err_exit 'compound variable defined in type unable to be extended'

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
[[ ${ad.c.ar.value} == foo ]] || err_exit 'references to type elements not working'

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
[[ ${c.ar[a].fifo_name} == fifo_a ]] || err_exit 'fifo_name c.ar[a] not fifo_a'
[[ ${c.ar[b].fifo_name} == fifo_b ]] || err_exit 'fifo_name c.ar[b] not fifo_b'
[[ ${c.ar[a].in_fd} == "${c.ar[b].in_fd}" ]] && err_exit 'c.ar[a].in_fd and c.ar[b].in_fd are the same' 
redirect  {c.ar[a].in_fd}<&-
redirect  {c.ar[b].in_fd}<&-
redirect  {c.ar[a].out_fd}>&-
redirect  {c.ar[b].out_fd}>&-
rm -f fifo_a fifo_b

typeset -T Z_t=(compound -a x)
Z_t z
[[ $(typeset -p z.x) ==  *'-C -a'* ]] || err_exit 'typeset -p for compound array element not displaying attributes'  

out='foo f 123'
typeset -T bam_t=(
	f=123
	function out { print foo f ${_.f}; }
)
bam_t f
[[ ${f.f} == 123 ]] || err_exit "f.f is ${f.f} should be 123"
[[ ${ f.out } == "$out" ]] 2> /dev/null || err_exit "f.out is ${ f.out } should be $out"
typeset -T bar_t=(
	bam_t foo
	b=456
	function out { _.foo.out; }
)
bar_t b
[[ ${b.b} == 456 ]] || err_exit "b.b is ${b.b} should be 456"
[[ ${ b.out } == "$out" ]] || err_exit "b.out is ${ b.out } should be $out"
typeset -T baz_t=(
	bar_t bar
	z=789
	function out { _.bar.out ;}
)
baz_t z
[[ ${z.z} == 789 ]] || err_exit "z.z is ${z.z} should be 789"
[[ ${ z.out } == "$out" ]] 2> /dev/null || err_exit "z.out should be $out"

$SHELL  2> /dev/null <<- \EOF || err_exit 'typeset -p with types not working'
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
[[ $got == "$exp" ]] || err_exit 'read -C for compound variable containing a type not working correctly'
) & wait $!
[[ $? == 0 ]] || err_exit 'read -C for compound variable containing a type not working correctly'

unset c
exp=$(compound c=( zz_t d=( typeset -C -a bar=( [4]=( zz_t b=( typeset -C -a bar)))));print -v c)
read -C got <<< "$exp"
[[ $got == "$exp" ]] || err_exit 'read -C for compound variable containing a nested type not working correctly'

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
[[ $? == 0 ]] || err_exit 'assignment in nested type fails'
[[ $got == "$exp" ]] || err_exit 'assignment in nested type returns wrong value'

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
[[ $got == "$exp" ]] || err_exit "expansion of variable containing a type that contains an index array of types not correct."

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
[[ $got == "$exp" ]] || err_exit 'typeset -T not displaying types correctly'

typeset -T Apar_t=(typeset name)
typeset -T Child_t=(Apar_t apar )
Child_t A=( apar=( name=IV01111))
Child_t C=( apar=( name=IV09557))
[[ ${A.apar.name} == IV01111 ]] || err_exit "A.apar.name is ${A.apar.name} should be IV01111"
[[ ${A.apar.name} == "${C.apar.name}" ]] && err_exit 'string fields in nested typed not working correctly'

unset ar
Pt_t -a ar=((x=3 y=4) (x=5 y=6))
ar=()
[[ $(typeset -p ar) == 'Pt_t -a ar=()' ]] || err_exit 'ar=() for an index array of Pt_t not correct'

unset ar
Pt_t -A ar=([one]=(x=3 y=4) [two]=(x=5 y=6))
ar=()
[[ $(typeset -p ar) == 'Pt_t -A ar=()' ]] || err_exit 'ar=() for an associative array of Pt_t not correct'

typeset -T Type=(
	typeset x
        integer  y=5
        function x.get
	{
            ((.sh.value = ++_.y))
        }
    )
Type obj
[[ ${obj.x} == 6 ]] || err_exit '_ for type variable not set to type'

got=$($SHELL  2> /dev/null <<- \EOF || err_exit 'short integer arrays in types fails'
	typeset -T X_t=(typeset -si -a arr=(7 8) )
	X_t x
	print -r -- $((x.arr[1]))
EOF)
[[ $got == 8 ]] || err_exit 'sort integer arrays in types not working correctly'

typeset -T p_t=(
	integer fd
	compound events=( bool pollin=false)
)
compound c
p_t -A c.p
c.p[2]=(fd=0 events=( bool pollin=true))
[[ ${c.p[2].events.pollin} == true ]] || err_exit 'c.p[2]=(fd=0 events=( bool pollin=true)) does not set pollin to true'

typeset -T xx_t=(
    integer i=2
    function printi { print -r -- ${_.i}.${_.__.i} ;}
)
typeset -T yy_t=( integer i=3; xx_t x)
yy_t y
[[ $(y.x.printi) == 2.3 ]] || err_exit 'discipline function with nested type using _.__ not working as command substitution'
[[ ${y.x.printi} == 2.3 ]] || err_exit 'discipline function with nested type us
ing _.__ not working as ${var.function}'

typeset -T pp_t=( integer fd ;
      function pinit { print $(( _.fd=$1 )) ;} 
)
compound c
pp_t -a c.pl
[[ $(c.pl[3].pinit 7) == 7 ]] 2> /dev/null || err_exit 'arrays of types in arithmetic expressions not working'

$SHELL 2> /dev/null -c '
typeset -T p_t=( integer fd=-1 ; compound events=(  bool pollin=false ; );)
p_t --man'
[[ $? == 2 ]] || err_exit 'unable to generated manpage for types that contain compound variables'

export FPATH=$tmp/fundir
mkdir -p $FPATH
print 'typeset -T My_t=(integer i j)' > $FPATH/My_t
$SHELL 2> /dev/null -c 'My_t a=(i=1 j=2); [[ "${a.i} ${a.j}" == "1 2" ]]' || 
	err_exit "dynamic loading of type with assignment fails"

$SHELL 2> /dev/null -c 'typeset -T My_t=(readonly x); My_t foo' && err_exit 'unset type subvariables defined as readonly are required to be specified for each type instance'

typeset -T Goo_t=(typeset x)
Goo_t -A foo
foo[bar]=(x=1) foo[baz]=(x=2)
exp='Goo_t -A foo=([bar]=(x=1) [baz]=(x=2))'
[[ $(typeset -p foo) == "$exp" ]] || print 'typeset -p for associative array of types not correct'

$SHELL <<- \!!! || err_exit 'creating an empty array of a type variable with required field fails'
	typeset -T My_t=(readonly x)
	My_t -a foo 2> /dev/null
	foo[0]=(typeset -r x=bar)
	exp='My_t -a foo=((typeset -r x=bar))'
	[[ $(typeset -p foo) == "$exp" ]]
!!!

$SHELL <<- \!!! || err_exit 'creating an array with required fields with assigments fails'
	typeset -T My_t=(readonly x)
	My_t -a foo=((x=5) (x=6)) 2> /dev/null
	exp='My_t -a foo=((typeset -r x=5) (typeset -r x=6))'
	[[ $(typeset -p foo) == "$exp" ]]
!!!

$SHELL <<- \!!! && err_exit 'creating an array with required fields missing does not fail'
	typeset -T My_t=(typeset -r x y)
	My_t -a foo=((typeset -r x=5;)) 2> /dev/null
!!!

$SHELL <<- \!!! && err_exit 'creating an array element with required field missing does not fail'
	typeset -T My_t=(typeset -r x y)
	My_t -a foo
	{ foo[1]=(x=2);} 2> /dev/null
!!!

$SHELL <<- \!!! && err_exit 'appending an array element with required field missing does not fail'
	typeset -T My_t=(typeset -r x y)
	My_t -a foo
	{ foo+=(y=3);} 2> /dev/null
!!!

typeset -T My_t=(integer x y)
My_t -a A
A=()
A+=(x=1 y=2)
exp='My_t -a A=((typeset -l -i x=1;typeset -l -i y=2))'
[[ $(typeset -p A) == "$exp" ]] || err_exit 'empty assignment to array of types creates element 0 by mistake'

$SHELL 2> /dev/null -c 'typeset -T a_t=(x=3 y=4); a_t b=(x=1)' || err_exit 'Cannot create instances for type names starting with the letter a'

$SHELL 2> /dev/null -c 'typeset -T X=(typeset x; function x.get { :; }); X -a xs=((x=yo) (x=jo)); [[ $(typeset -p xs) == "X -a xs=((x=yo) (x=jo))" ]]' || err_exit 'X -a xs=((v1) (v2)) where X is a type, not working'

exit $((Errors<125?Errors:125))

