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
	let Errors+=1
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0

tmp=$(mktemp -dt) || { err_exit mktemp -dt failed; exit 1; }
trap "cd /; rm -rf $tmp" EXIT

foo=abc
typeset -C bar=(x=3 y=4 t=7)
typeset -A z=([abc]=qqq)
integer r=9
function fn
{
	print global fn $foo
}
function fun
{
	print global fun $foo
}
mkdir -p $tmp/global/bin $tmp/local/bin
cat > $tmp/global/xfun <<- \EOF
	function xfun
	{
		print xfun global $foo
	}
EOF
cat > $tmp/local/xfun <<- \EOF
	function xfun
	{
		print xfun local $foo
	}
EOF
chmod +x "$tmp/global/xfun" "$tmp/local/xfun"
print 'print local prog $1' >  $tmp/local/bin/run
print 'print global prog $1' >  $tmp/global/bin/run
chmod +x "$tmp/local/bin/run" "$tmp/global/bin/run"
PATH=$tmp/global/bin:$PATH
FPATH=$tmp/global

namespace x
{
	foo=bar
	typeset -C bar=(x=1 y=2 z=3)
	typeset -A z=([qqq]=abc)
	function fn
	{
		print local fn $foo
	}
	[[ $(fn) == 'local fn bar' ]] || err_exit 'fn inside namespace should run local function'
	[[ $(fun) == 'global fun abc' ]] || err_exit 'global fun run from namespace not working'
	(( r == 9 )) || err_exit 'global variable r not set in namespace'
false
	[[ ${z[qqq]} == abc ]] || err_exit 'local array element not correct'
	[[ ${z[abc]} == '' ]] || err_exit 'global array element should not be visible when local element exists'
	[[ ${bar.y} == 2 ]] || err_exit 'local variable bar.y not found'
	[[ ${bar.t} == '' ]] || err_exit 'global bar.t should not be visible'
	function runxrun
	{
		xfun
	}
	function runrun
	{
		run $1
	}
	PATH=$tmp/local/bin:/bin
	FPATH=$tmp/local
	[[ $(runxrun) ==  'xfun local bar' ]] || err_exit 'local function on FPATH failed'
	[[ $(runrun $foo) ==  'local prog bar' ]] || err_exit 'local binary on PATH failed'
}
[[ $(fn) == 'global fn abc' ]] || err_exit 'fn outside namespace should run global function'
[[ $(.x.fn) == 'local fn bar' ]] || err_exit 'namespace function called from global failed'
[[  ${z[abc]} == qqq ]] || err_exit 'global associative array should not be affected by definiton in namespace'
[[  ${bar.y} == 4 ]] || err_exit 'global compound variable should not be affected by definiton in namespace'
[[  ${bar.z} == ''  ]] || err_exit 'global compound variable should not see elements in namespace'
[[ $(xfun) ==  'xfun global abc' ]] || err_exit 'global function on FPATH failed'
[[ $(run $foo) ==  'global prog abc' ]] || err_exit 'global binary on PATH failed'
false
[[ $(.x.runxrun) ==  'xfun local bar' ]] || err_exit 'namespace function on FPATH failed'

namespace sp1
{
	compound -a c=( [4]=( bool b=true) )
}
exp=$'(\n\t[4]=(\n\t\t_Bool b=true\n\t)\n)'
[[ $(print -v .sp1.c) == "$exp" ]] || err_exit 'print -v .sp1.c where sp1 is a namespace and c a compound variable not correct'

namespace com.foo
{
	compound container=(compound -a a; integer i=2)
	exp=$(print -v container)
}
[[ $(print -v .com.foo.container) == "${.com.foo.exp}" ]] || err_exit 'compound variables defined in a namespace not expanded the same outside and inside'

namespace a.b
{
	typeset -T x_t=(
		integer i=5
		function pi { printf "%d\n" $((_.i+_.i)); }
	)
}
.a.b.x_t var
[[ $(var.pi) == 10 ]] || print -u2 'discipline functions for types in namespace not working'

namespace com.foo.test1
{
	typeset -T x_t=(
		integer i=9
		function pr { printf "%d/%d\n" _.i _.__.j ; }
	)
	typeset -T y_t=( x_t x ; integer j=5 )
}
.com.foo.test1.y_t v
[[ $(v.x.pr) == 9/5 ]] || err_exit  '_.__ not working with nested types in a namespace'

namespace a.b
{
	function f1 { print OK ; }
	function f2 { f1 ; }
	[[ $(f2) == OK ]] 2> /dev/null || err_exit 'function defined in namespace not found when referenced by another function in the namespace'
}

namespace org.terror
{
	typeset -T x_t=(
		function method_a { print xxx; }
	)
	function main
	{
		x_t x1
		x_t x2=(
			function method_a { print yyy; }
		)
		x1.method_a
    		x2.method_a
	}
	[[ $(main 2> /dev/null)  == $'xxx\nyyy' ]] || err_exit 'discipline override type defined in namespace not working'
}

namespace a.b
{
	function f
	{
		integer i=1
		typeset s=abcd
		[[ $(print "${s:i++:1}") == b ]] || err_exit 'binding function local variables in functions defined in namespace not working'
	}
	f
}

namespace a.b
{
	typeset -T y_t=(
		integer i=9
		function px { p "${_.i}"; }
	)
        function p { printf "%q\n" "$1" ;}
	y_t x
	[[ $(x.px 2> /dev/null) == 9 ]] || err_exit 'function defined in type not found from within a namespace'
}
[[ $(.a.b.x.px 2> /dev/null) == 9 ]] || err_exit 'function defined in type no
t found from outside a.b namespace'


exit $((Errors<125?Errors:125))
