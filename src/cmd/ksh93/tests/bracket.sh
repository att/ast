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

null=''
if [[ ! -z $null ]]
then
    log_error "-z: null string should be of zero length"
fi

file=$TEST_DIR/original
newer_file=$TEST_DIR/newer
if [[ -z $file ]]
then
    log_error "-z: $file string should not be of zero length"
fi

if [[ -a $file ]]
then
    log_error "-a: $file shouldn't exist"
fi

if [[ -e $file ]]
then
    log_error "-e: $file shouldn't exist"
fi

> $file
if [[ ! -a $file ]]
then
    log_error "-a: $file should exist"
fi

if [[ ! -e $file ]]
then
    log_error "-e: $file should exist"
fi

chmod 777 $file
if [[ ! -r $file ]]
then
    log_error "-r: $file should be readable"
fi

if [[ ! -w $file ]]
then
    log_error "-w: $file should be writable"
fi

if [[ ! -w $file ]]
then
    log_error "-x: $file should be executable"
fi

if [[ ! -w $file || ! -r $file ]]
then
    log_error "-rw: $file should be readable/writable"
fi

if [[ -s $file ]]
then
    log_error "-s: $file should be of zero size"
fi

if [[ ! -f $file ]]
then
    log_error "-f: $file should be an ordinary file"
fi

if [[  -d $file ]]
then
    log_error "-f: $file should not be a directory file"
fi

if [[  ! -d . ]]
then
    log_error "-d: . should not be a directory file"
fi

if [[  -f /dev/null ]]
then
    log_error "-f: /dev/null  should not be an ordinary file"
fi

chmod 000 $file
if [[ $OS_NAME == cygwin* ]]
then
    log_info 'skipping [[ -r $file ]] test on Cygwin'
else
    if [[ -r $file ]]
    then
        log_error "-r: $file should not be readable"
    fi
fi

if [[ ! -O $file ]]
then
    log_error "-r: $file should be owned by me"
fi

if [[ $OS_NAME == cygwin* ]]
then
    log_info 'skipping [[ -w $file ]] test on Cygwin'
else
    if [[ -w $file ]]
    then
        log_error "-w: $file should not be writable"
    fi
fi

if [[ $OS_NAME == cygwin* ]]
then
    log_info 'skipping [[ -x $file ]] test on Cygwin'
else
    if [[ -w $file ]]
    then
        log_error "-x: $file should not be executable"
    fi
fi

if [[ $OS_NAME == cygwin* ]]
then
    log_info 'skipping [[ -w $file || -r $file ]] test on Cygwin'
else
    if [[ -w $file || -r $file ]]
    then
        log_error "-rw: $file should not be readable/writable"
    fi
fi

if [[ -z x && -z x || ! -z x ]]
then
    :
else
    log_error "wrong precedence"
fi

if [[ -z x && (-z x || ! -z x) ]]
then
    log_error "() grouping not working"
fi

if [[ foo < bar ]]
then
    log_error "foo comes before bar"
fi

[[ . -ef $(pwd) ]] || log_error ". is not $PWD"
set -o allexport
[[ -o allexport ]] || log_error '-o: did not set allexport option'
if [[ -n  $null ]]
then
    log_error "'$null' has non-zero length"
fi

if [[ ! -r /dev/fd/0 ]]
then
    log_error "/dev/fd/0 not open for reading"
fi

if [[ ! -w /dev/fd/2 ]]
then
    log_error "/dev/fd/2 not open for writing"
fi

sleep 1
> $newer_file
if [[ ! $file -ot $newer_file ]]
then
    log_error "$file should be older than $newer_file"
fi

if [[ $file -nt $newer_file ]]
then
    log_error "$newer_file should be newer than $file"
fi

if [[ $file != $TEST_DIR/* ]]
then
    log_error "$file should match $TEST_DIR/*"
fi

if [[ $file == $TEST_DIR'/*' ]]
then
    log_error "$file should not equal $TEST_DIR'/*'"
fi

[[ ! ( ! -z $null && ! -z x) ]]    || log_error "negation and grouping"
[[ -z '' || -z '' || -z '' ]]    || log_error "three ors not working"
[[ -z '' &&  -z '' && -z '' ]]    || log_error "three ors not working"
(exit 8)
if [[ $? -ne 8 || $? -ne 8 ]]
then
    log_error 'value $? within [[...]]'
fi

x='(x'
if [[ '(x' != '('* ]]
then
    log_error " '(x' does not match '('* within [[...]]"
fi

if [[ '(x' != "("* ]]
then
    log_error ' "(x" does not match "("* within [[...]]'
fi

if [[ '(x' != \(* ]]
then
    log_error ' "(x" does not match \(* within [[...]]'
fi

if [[ 'x(' != *'(' ]]
then
    log_error " 'x(' does not match '('* within [[...]]"
fi

if [[ 'x&' != *'&' ]]
then
    log_error " 'x&' does not match '&'* within [[...]]"
fi

if [[ 'xy' == *'*' ]]
then
    log_error " 'xy' matches *'*' within [[...]]"
fi

if [[ 3 > 4 ]]
then
    log_error '3 < 4'
fi

if [[ 4 < 3 ]]
then
    log_error '3 > 4'
fi

[[ 3 -lt 4 ]] || log_error "-lt does not work"

if [[ 3x > 4x ]]
then
    log_error '3x < 4x'
fi

x='@(bin|dev|?)'
cd /
if [[ $(print $x) != "$x" ]]
then
    log_error 'extended pattern matching on command arguments'
fi

if [[ dev != $x ]]
then
    log_error 'extended pattern matching not working on variables'
fi

if [[ -u $SHELL ]]
then
    log_error "setuid on $SHELL"
fi

if [[ -g $SHELL ]]
then
    log_error "setgid on $SHELL"
fi

chmod 600 $file
exec 4> $file
print -u4 foobar
if [[ ! -s $file ]]
then
    log_error "-s: $file should be non-zero"
fi

exec 4>&-
if [[ 011 -ne 11 ]]
then
    log_error "leading zeros in arithmetic compares not ignored"
fi

{
    set -x
    [[ foo > bar ]]
} 2> /dev/null || { set +x; log_error "foo<bar with -x enabled" ;}
set +x
(
    eval "[[ (a) ]]"
) 2> /dev/null || log_error "[[ (a) ]] not working"
> $file
chmod 4755 "$file"
if test -u $file && test ! -u $file
then
    log_error "test ! -u suidfile not working"
fi

for i in '(' ')' '[' ']'
do    [[ $i == $i ]] || log_error "[[ $i != $i ]]"
done
(
    [[ aaaa == {4}(a) ]] || log_error 'aaaa != {4}(a)'
    [[ aaaa == {2,5}(a) ]] || log_error 'aaaa != {2,4}(a)'
    [[ abcdcdabcd == {3,6}(ab|cd) ]] || log_error 'abcdcdabcd == {3,4}(ab|cd)'
    [[ abcdcdabcde == {5}(ab|cd)e ]] || log_error 'abcdcdabcd == {5}(ab|cd)e'
) || log_error 'errors with {..}(...) patterns'
[[ D290.2003.02.16.temp == D290.+(2003.02.16).temp* ]] || log_error 'pattern match bug with +(...)'
rm -rf $file
touch -t 01020304.05 $file
[[ -N $file ]] && log_error 'test -N $TEST_DIR/*: st_mtime>st_atime after creat'
# Update only mtime to mimic the file was written
touch -m -t 01020708.09 $file
[[ -N $file ]] || log_error 'test -N $TEST_DIR/*: st_mtime<=st_atime after write'
# Update only atime to mimic the file was read
touch -a -t 01020809.10 $file
[[ -N $file ]] && log_error 'test -N $TEST_DIR/*: st_mtime>st_atime after read'
if rm -rf "$file" && ln -s / "$file"
then
    [[ -L "$file" ]] || log_error '-L not working'
    [[ -L "$file"/ ]] && log_error '-L with file/ not working'
fi

# ==========
[[ -c /dev/null ]] || log_error "-c fails to detect character devices"

# ==========
# TODO: How to test for block devices ?
# [[ -b /dev/sda ]] || log_error "-b fails to detect block devices"

# ==========
mkdir "$TEST_DIR/this_dir_has_sticky_bit_set"
chmod +t "$TEST_DIR/this_dir_has_sticky_bit_set"
[[ -k "$TEST_DIR/this_dir_has_sticky_bit_set" ]] || log_error "-k fails to detect sticky bit"

# ==========
mkfifo "$TEST_DIR/this_is_a_pipe"
[[ -p "$TEST_DIR/this_is_a_pipe" ]] || "-p fails to detect pipes"

$SHELL -c 't=1234567890; [[ $t == @({10}(\d)) ]]' 2> /dev/null || log_error ' @({10}(\d)) pattern not working'
$SHELL -c '[[ att_ == ~(E)(att|cus)_.* ]]' 2> /dev/null || log_error ' ~(E)(att|cus)_* pattern not working'
$SHELL -c '[[ att_ =~ (att|cus)_.* ]]' 2> /dev/null || log_error ' =~ ere not working'
$SHELL -c '[[ abc =~ a(b)c ]]' 2> /dev/null || log_error '[[ abc =~ a(b)c ]] fails'
$SHELL -xc '[[ abc =~  \babc\b ]]' 2> /dev/null || log_error '[[ abc =~ \babc\b ]] fails'
[[ abc == ~(E)\babc\b ]] || log_error '\b not preserved for ere when not in ()'
[[ abc == ~(iEi)\babc\b ]] || log_error '\b not preserved for ~(iEi) when not in ()'

e=$($SHELL -c '[ -z "" -a -z "" ]' 2>&1)
[[ $e ]] && log_error "[ ... ] compatibility check failed -- $e"
i=hell
[[ hell0 == $i[0] ]]  ||  log_error 'pattern $i[0] interpreded as array ref'
[[ $($SHELL -c 'case  F in ~(Eilr)[a-z0-9#]) print ok;;esac' 2> /dev/null) == ok ]] || log_error '~(Eilr) not working in case command'
[[ $($SHELL -c "case  Q in ~(Fi)q |  \$'\E') print ok;;esac" 2> /dev/null) == ok ]] || log_error '~(Fi)q | \E  not working in case command'

for l in C en_US.ISO8859-15
do    [[ $($SHELL -c "LC_COLLATE=$l" 2>&1) ]] && continue
    export LC_COLLATE=$l
    set -- \
        'A'   0 1 1   0 1 1      1 0 0   1 0 0   \
        'Z'   0 1 1   0 1 1      1 0 0   1 0 0   \
        '/'   0 0 0   0 0 0      1 1 1   1 1 1   \
        '.'   0 0 0   0 0 0      1 1 1   1 1 1   \
        '_'   0 0 0   0 0 0      1 1 1   1 1 1   \
        '-'   1 1 1   1 1 1      0 0 0   0 0 0   \
        '%'   0 0 0   0 0 0      1 1 1   1 1 1   \
        '@'   0 0 0   0 0 0      1 1 1   1 1 1   \
        '!'   0 0 0   0 0 0      1 1 1   1 1 1   \
        '^'   0 0 0   0 0 0      1 1 1   1 1 1   \
        # retain this line #
    while    (( $# >= 13 ))
    do    c=$1
        shift
        for p in \
            '[![.-.]]' \
            '[![.-.][:upper:]]' \
            '[![.-.]A-Z]' \
            '[!-]' \
            '[!-[:upper:]]' \
            '[!-A-Z]' \
            '[[.-.]]' \
            '[[.-.][:upper:]]' \
            '[[.-.]A-Z]' \
            '[-]' \
            '[-[:upper:]]' \
            '[-A-Z]' \
            # retain this line #
        do
            expect=$1
            shift
            [[ $c == $p ]]
            actual=$?
            [[ $actual == $expect ]] ||
                log_error "[[ '$c' == $p ]] for LC_COLLATE=$l failed" "$expect" "$actual"
        done
    done
done
integer n
if ( : < /dev/tty ) 2>/dev/null && exec {n}< /dev/tty
then
    [[ -t  $n ]] || log_error "[[ -t  n ]] fails when n > 9"
fi

foo=([1]=a [2]=b [3]=c)
[[ -v foo[1] ]] ||  log_error 'foo[1] should be set'
[[ ${foo[1]+x} ]] ||  log_error '${foo[1]+x} should be x'
[[ ${foo[@]+x} ]] ||  log_error '${foo[@]+x} should be x'
unset foo[1]
[[ -v foo[1] ]] && log_error 'foo[1] should not be set'
[[ ${foo[1]+x} ]] &&  log_error '${foo[1]+x} should be empty'
bar=(a b c)
[[ -v bar[1] ]]  || log_error 'bar[1] should be set'
[[ ${bar[1]+x} ]] ||  log_error '${foo[1]+x} should be x'
unset bar[1]
[[ ${bar[1]+x} ]] &&  log_error '${foo[1]+x} should be empty'
[[ -v bar ]] || log_error 'bar should be set'
[[ -v bar[1] ]] && log_error 'bar[1] should not be set'
integer z=( 1 2 4)
[[ -v z[1] ]] || log_error 'z[1] should be set'
unset z[1]
[[ -v z[1] ]] && log_error 'z[1] should not be set'
typeset -si y=( 1 2 4)
[[ -v y[6] ]] && log_error 'y[6] should not be set'
[[ -v y[1] ]] ||  log_error  'y[1] should be set'
unset y[1]
[[ -v y[1] ]] && log_error 'y[1] should not be set'
x=abc
[[ -v x[0] ]] || log_error  'x[0] should be set'
[[ ${x[0]+x} ]] || log_error print  '${x[0]+x} should be x'
[[ -v x[3] ]] && log_error 'x[3] should not be set'
[[ ${x[3]+x} ]] && log_error  '${x[0]+x} should be Empty'
unset x
[[ ${x[@]+x} ]] && log_error  '${x[@]+x} should be Empty'
unset x y z foo bar

{ x=$($SHELL -c '[[ (( $# -eq 0 )) ]] && print ok') 2> /dev/null;}
[[ $x == ok ]] || log_error '((...)) inside [[...]] not treated as nested ()'

$SHELL 2> /dev/null -c '[[(-n foo)]]' || log_error '[[(-n foo)]] should not require space in front of ('

$SHELL 2> /dev/null -c '[[ "]" == ~(E)[]] ]]' || log_error 'pattern "~(E)[]]" does not match "]"'

unset var
[[ -v var ]] &&  log_error '[[ -v var ]] should be false after unset var'
float var
[[ -v var ]]  ||  log_error '[[ -v var ]] should be true after float var'
unset var
[[ -v var ]] &&  log_error '[[ -v var ]] should be false after unset var again'

$SHELL 2> /dev/null -c '[[ 1<2 ]]' ||  log_error '[[ 1<2 ]] not parsed correctly'

false
x=$( [[ -z $(printf $? >&2) && -z $(printf $? >&2) ]] 2>&1)
[[ $x$? == 110 ]] || log_error '$? not reserved when expanding [[...]]'

$SHELL -c 2> /dev/null '[[ AATAAT =~ (AAT){2} ]]' || log_error '[[ AATAAT =~ (AAT){2} ]] does not match'

$SHELL -c 2> /dev/null '[[ AATAATCCCAATAAT =~ (AAT){2}CCC(AAT){2} ]]' || log_error '[[ AATAATCCCAATAAT =~ (AAT){2}CCC(AAT){2} ]] does not match'

[[ 0x10 -eq 16 ]] || log_error 'heximal constants not working in [[...]]'

[[ 010 -eq 10 ]] || log_error '010 not 10 in [[...]]'

x=10

([[ x -eq 10 ]]) 2> /dev/null || log_error 'x -eq 10 fails in [[...]] with x=10'
