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

mkdir -p $TEST_DIR/global/bin $TEST_DIR/local/bin
cat > $TEST_DIR/global/xfun <<- \EOF
    function xfun
    {
        print xfun global $foo
    }
EOF

cat > $TEST_DIR/local/xfun <<- \EOF
    function xfun
    {
        print xfun local $foo
    }
EOF

chmod +x "$TEST_DIR/global/xfun" "$TEST_DIR/local/xfun"
print 'print local prog $1' >  $TEST_DIR/local/bin/run
print 'print global prog $1' >  $TEST_DIR/global/bin/run
chmod +x "$TEST_DIR/local/bin/run" "$TEST_DIR/global/bin/run"
PATH=$TEST_DIR/global/bin:$PATH
FPATH=$TEST_DIR/global

namespace x
{
    foo=bar
    typeset -C bar=(x=1 y=2 z=3)
    typeset -A z=([qqq]=abc)
    function fn
    {
        print local fn $foo
    }
    [[ $(fn) == 'local fn bar' ]] || log_error 'fn inside namespace should run local function'
    [[ $(fun) == 'global fun abc' ]] || log_error 'global fun run from namespace not working'
    (( r == 9 )) || log_error 'global variable r not set in namespace'
false
    [[ ${z[qqq]} == abc ]] || log_error 'local array element not correct'
    [[ ${z[abc]} == '' ]] || log_error 'global array element should not be visible when local element exists'
    [[ ${bar.y} == 2 ]] || log_error 'local variable bar.y not found'
    [[ ${bar.t} == '' ]] || log_error 'global bar.t should not be visible'
    function runxrun
    {
        xfun
    }
    function runrun
    {
        run $1
    }
    PATH=$TEST_DIR/local/bin:/bin
    FPATH=$TEST_DIR/local
    [[ $(runxrun) ==  'xfun local bar' ]] || log_error 'local function on FPATH failed'
    [[ $(runrun $foo) ==  'local prog bar' ]] || log_error 'local binary on PATH failed'
}

[[ $(fn) == 'global fn abc' ]] || log_error 'fn outside namespace should run global function'
[[ $(.x.fn) == 'local fn bar' ]] || log_error 'namespace function called from global failed'
[[  ${z[abc]} == qqq ]] || log_error 'global associative array should not be affected by definiton in namespace'
[[  ${bar.y} == 4 ]] || log_error 'global compound variable should not be affected by definiton in namespace'
[[  ${bar.z} == ''  ]] || log_error 'global compound variable should not see elements in namespace'
[[ $(xfun) ==  'xfun global abc' ]] || log_error 'global function on FPATH failed'
actual=$(run $foo)
expect='global prog abc'
[[ $actual ==  $expect ]] || log_error 'global binary on PATH failed' "$expect" "$actual"
false
[[ $(.x.runxrun) ==  'xfun local bar' ]] || log_error 'namespace function on FPATH failed'

namespace sp1
{
    compound -a c=( [4]=( bool b=true) )
}
exp=$'(\n\t[4]=(\n\t\t_Bool b=true\n\t)\n)'
[[ $(print -v .sp1.c) == "$exp" ]] || log_error 'print -v .sp1.c where sp1 is a namespace and c a compound variable not correct'

namespace com.foo
{
    compound container=(compound -a a; integer i=2)
    exp=$(print -v container)
}
[[ $(print -v .com.foo.container) == "${.com.foo.exp}" ]] || log_error 'compound variables defined in a namespace not expanded the same outside and inside'

namespace a.b
{
    typeset -T x_t=(
        integer i=5
        function pi { printf "%d\n" $((_.i+_.i)); }
    )
}
.a.b.x_t var
[[ $(var.pi) == 10 ]] || print -u2 'discipline functions for types in namespace not working'

namespace a.b
{
    function f1 { print OK ; }
    function f2 { f1 ; }
    [[ $(f2) == OK ]] || log_error 'function defined in namespace not found when referenced by another function in the namespace'
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
    [[ $(main)  == $'xxx\nyyy' ]] || log_error 'discipline override type defined in namespace not working'
}

namespace a.b
{
    function f
    {
        integer i=1
        typeset s=abcd
        [[ $(print "${s:i++:1}") == b ]] || log_error 'binding function local variables in functions defined in namespace not working'
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
    [[ $(x.px) == 9 ]] || log_error 'function defined in type not found from within a namespace'
}
[[ $(.a.b.x.px) == 9 ]] || log_error 'function defined in type not found from outside a.b namespace'
