# =======
getopts 2>&1 | grep -q "Usage: getopts" || log_error "Running getopts without any parameter should show usage info"

set -- - foobar
[[ $# == 2 && $1 == - && $2 == foobar ]] || log_error "set -- - foobar failed"
set -- -x foobar
[[ $# == 2 && $1 == -x && $2 == foobar ]] || log_error "set -- -x foobar failed"

getopts :x: foo || log_error "getopts :x: returns false"
[[ $foo == x && $OPTARG == foobar ]] || log_error "getopts :x: failed"

# ========
OPTIND=1
getopts :r:s var -r
if [[ $var != : || $OPTARG != r ]]
then
    log_error "'getopts :r:s var -r' not working"
fi

# ========
OPTIND=1
getopts :d#u OPT -d 16177
if [[ $OPT != d || $OPTARG != 16177 ]]
then
    log_error "'getopts :d#u OPT=d OPTARG=16177' failed -- OPT=$OPT OPTARG=$OPTARG"
fi

# ========
OPTIND=1
while getopts 'ab' option -a -b
do
    [[ $OPTIND == $((OPTIND)) ]] || log_error "OPTIND optimization bug"
done

# ========
USAGE=$'[-][S:server?Operate on the specified \asubservice\a:]:[subservice:=pmserver]
    {
        [p:pmserver]
        [r:repserver]
        [11:notifyd]
    }'
set pmser p rep r notifyd -11
while (( $# > 1 ))
do
    OPTIND=1
    getopts "$USAGE" OPT -S $1
    [[ $OPT == S && $OPTARG == $2 ]] || log_error "OPT=$OPT OPTARG=$OPTARG -- expected OPT=S OPTARG=$2"
    shift 2
done

# ========
unset -f foobar
function foobar
{
    print 'hello world'
}
OPTIND=1
if [[ $(getopts  $'[+?X\ffoobar\fX]' v --man 2>&1) != *'Xhello world'X* ]]
then
    log_error '\f...\f not working in getopts usage strings'
fi

# ========
$SHELL -c 'OPTIND=-1000000; getopts a opt -a' 2> /dev/null
[[ $? == 1 ]] || log_error 'getopts with negative OPTIND not working'
getopts 'n#num' opt  -n 3
[[ $OPTARG == 3 ]] || log_error 'getopts with numerical arguments failed'

# ========
((n=0))
((n++)); ARGC[$n]=1 ARGV[$n]=""
((n++)); ARGC[$n]=2 ARGV[$n]="-a"
((n++)); ARGC[$n]=4 ARGV[$n]="-a -v 2"
((n++)); ARGC[$n]=4 ARGV[$n]="-a -v 2 x"
((n++)); ARGC[$n]=4 ARGV[$n]="-a -v 2 x y"
for ((i=1; i<=n; i++))
do
    set -- ${ARGV[$i]}
    OPTIND=0
    while getopts -a tst "av:" OPT
    do
    :
    done

    expect="${ARGC[$i]}"
    actual="$OPTIND"
    [[ $actual == $expect ]] ||
        log_error "\$OPTIND after getopts loop incorrect" "$expect" "$actual"
done

# ========
options=ab:c
optarg=foo
set -- -a -b $optarg -c bar
while getopts $options opt
do
    case $opt in
    a|c)
       expect=""
       actual="$OPTARG"
       [[ $actual == $expect ]] ||
          log_error "getopts $options \$OPTARG for flag $opt failed" "$expect" "$actual"
       ;;
    b)
       expect="$optarg"
       actual="$OPTARG"
       [[ $actual == $expect ]] ||
          log_error "getopts $options \$OPTARG failed" "$expect" "$actual"
       ;;
    *) log_error "getopts $options failed" "" "$opt" ;;
    esac
done

# ========
[[ $($SHELL 2> /dev/null -c 'readonly foo; getopts a: foo -a blah; echo foo') == foo ]] || log_error 'getopts with readonly variable causes script to abort'

# ========
v=$($SHELL 2> /dev/null +o rc -ic $'getopts a:bc: opt --man\nprint $?')
[[ $v == 2* ]] || log_error 'getopts --man does not exit 2 for interactive shells'

# Regression tests
# https://github.com/att/ast/issues/851
OPTIND=1
out=$(getopts -a test $'
[-]
[foo?option 1]
[bar?option 2]

Use option 1
Use option 2

' baz '--help' 2>&1)

[[ "$out" = $'Usage: test [ options ] Use option 1\n   Or: test [ options ] Use option 2' ]] ||
    log_error "getopts shows wrong text for self documenting options"
