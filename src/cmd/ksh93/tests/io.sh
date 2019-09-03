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

unset HISTFILE

function fun
{
    while  command exec 3>&1
    do
        break
    done 2>   /dev/null
    print -u3 good
}
print 'read -r a; print -r -u$1 -- "$a"' > $TEST_DIR/mycat
chmod 755 $TEST_DIR/mycat
for ((i=3; i < 10; i++))
do
    eval "a=\$(print foo | $TEST_DIR/mycat" $i $i'>&1 > /dev/null |cat)' 2> /dev/null
    [[ $a == foo ]] || log_error "bad file descriptor $i in comsub script"
done

exec 3> /dev/null
[[ $(fun) == good ]] || log_error 'file 3 closed before subshell completes'
exec 3>&-
cd $TEST_DIR || { log_error "cd $TEST_DIR failed"; exit ; }
print foo > file1
print bar >> file1
if [[ $(<file1) != $'foo\nbar' ]]
then
    log_error 'append (>>) not working'
fi

set -o noclobber
exec 3<> file1
read -u3 line
exp=foo
if [[ $line != $exp ]]
then
    log_error "read on <> fd failed -- expected '$exp', got '$line'"
fi

if ( 4> file1 ) 2> /dev/null
then
    log_error 'noclobber not causing exclusive open'
fi

set +o noclobber

FDFS=(
    ( dir=/proc/self/fd    semantics='open'    )
    ( dir=/proc/$$/fd    semantics='open'    )
    ( dir=/dev/fd        semantics='open|dup'    )
    ( dir=/dev/fd        semantics='dup'    )
)
for ((fdfs=0; fdfs<${#FDFS[@]}-1; fdfs++))
do
    [[ -e ${FDFS[fdfs].dir} ]] && { command : > ${FDFS[fdfs].dir}/1; } 2>/dev/null >&2 && break
done

exec 3<> file1
if command exec 4< ${FDFS[fdfs].dir}/3
then
    read -u3 got
    read -u4 got
    exp='foo|bar'
    case $got in
    foo)    semantics='open' ;;
    bar)    semantics='dup' ;;
    *)    semantics='failed' ;;
    esac
    [[ $semantics == @(${FDFS[fdfs].semantics}) ]] || log_error "'4< ${FDFS[fdfs].dir}/3' $semantics semantics instead of ${FDFS[fdfs].semantics} -- expected '$exp', got '$got'"
fi

# 2004-11-25 ancient /dev/fd/N redirection bug fix
got=$(
    {
        print -n 1
        print -n 2 > ${FDFS[fdfs].dir}/2
        print -n 3
        print -n 4 > ${FDFS[fdfs].dir}/2
    }  2>&1
)
exp='1234|4'
case $got in
1234)    semantics='dup' ;;
4)    semantics='open' ;;
*)    semantics='failed' ;;
esac
[[ $semantics == @(${FDFS[fdfs].semantics}) ]] || log_error "${FDFS[fdfs].dir}/N $semantics semantics instead of ${FDFS[fdfs].semantics} -- expected '$exp', got '$got'"

cat > close0 <<\!
exec 0<&-
echo $(./close1)
!
print "echo abc" > close1
chmod +x close0 close1
x=$(./close0)
if [[ $x != "abc" ]]
then
    log_error "picked up file descriptor zero for opening script file" "abc" "$x"
fi

cat > close0 <<\!
    for ((i=0; i < 1100; i++))
    do
        exec 4< /dev/null
        read -u4
    done

    exit 0
!
./close0 2> /dev/null || log_error "multiple exec 4< /dev/null can fail"
$SHELL -c '
    trap "rm -f in out" EXIT
    for ((i = 0; i < 1000; i++))
    do
        print -r -- "This is a test"
    done > in
    > out
    exec 1<> out
    builtin cat
    print -r -- "$(<in)"
    cmp -s in out'  2> /dev/null
[[ $? == 0 ]] || log_error 'builtin cat truncates files'
cat >| script <<-\!
	print hello
	( exec 3<&- 4<&-)
	exec 3<&- 4<&-
	print world
!
chmod +x script
[[ $( $SHELL ./script) == $'hello\nworld' ]] || log_error 'closing 3 & 4 causes script to fail'
cd ~- || log_error "cd back failed"
cd $TEST_DIR || { log_error "cd $TEST_DIR failed"; exit ; }
( exec  > '' ) 2> /dev/null  && log_error '> "" does not fail'
unset x
( exec > ${x} ) 2> /dev/null && log_error '> $x, where x null does not fail'
exec <<!
foo
bar
!
( exec 0< /dev/null)
read line
if [[ $line != foo ]]
then
    log_error 'file descriptor not restored after exec in subshell'
fi

exec 3>&- 4>&-
[[ $( {
    read -r line; print -r -- "$line"
    (
            read -r line; print -r -- "$line"
    ) & wait
    while read -r line
    do
        print -r -- "$line"
    done

 } << !
line 1
line 2
line 3
!) == $'line 1\nline 2\nline 3' ]] || log_error 'read error with subshells'
# 2004-05-11 bug fix
cat > $TEST_DIR/1 <<- ++EOF++
	script=$TEST_DIR/2
	trap "rm -f \$script" EXIT
	exec 7> \$script
	for ((i=3; i<9; i++))
	do
	eval "while read -u\$i; do : ; done \$i</dev/null"
		print -u7 "exec \$i< /dev/null"
	done
	
	for ((i=0; i < 60; i++))
	do
		print -u7 -f "%.80c\n"  ' '
	done
	
	print -u7 'print ok'
	exec 7<&-
	chmod +x \$script
	\$script
++EOF++

chmod +x $TEST_DIR/1
[[ $($SHELL  $TEST_DIR/1) == ok ]]  || log_error "parent i/o causes child script to fail"
# 2004-12-20 redirection loss bug fix
cat > $TEST_DIR/1 <<- \++EOF++
	function a
	{
	trap 'print ok' EXIT
	: > /dev/null
	}
	a
++EOF++
chmod +x $TEST_DIR/1
[[ $($TEST_DIR/1) == ok ]] || log_error "trap on EXIT loses last command redirection"
print > /dev/null {n}> $TEST_DIR/1
[[ ! -s $TEST_DIR/1 ]] && newio=1
if [[ $newio && $(print hello | while read -u$n; do print $REPLY; done {n}<&0) != hello ]]
then
    log_error "{n}<&0 not working with for loop"
fi

[[ $({ read -r; read -u3 3<&0; print -- "$REPLY" ;} <<!
hello
world
!) == world ]] || log_error 'I/O not synchronized with <&'
x="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNSPQRSTUVWXYZ1234567890"
for ((i=0; i < 62; i++))
do
    printf "%.39c\n"  ${x:i:1}
done >  $TEST_DIR/seek

if command exec 3<> $TEST_DIR/seek
then
    (( $(3<#) == 0 )) || log_error "not at position 0"
    (( $(3<# ((EOF))) == 40*62 )) || log_error "not at end-of-file"
    command exec 3<# ((40*8)) || log_error "absolute seek fails"
    read -u3
    [[ $REPLY == +(i) ]] || log_error "expected iiii..., got $REPLY"
    [[ $(3<#) == $(3<# ((CUR)) ) ]] || log_error '$(3<#)!=$(3<#((CUR)))'
    command exec 3<# ((CUR+80))
    read -u3
    [[ $REPLY == {39}(l) ]] || log_error "expected lll..., got $REPLY"
    command exec 3<# ((EOF-80))
    read -u3
    [[ $REPLY == +(9) ]] || log_error "expected 999..., got $REPLY"
    command exec 3># ((80))
    print -u3 -f "%.39c\n"  @
    command exec 3># ((80))
    read -u3
    [[ $REPLY == +(@) ]] || log_error "expected @@@..., got $REPLY"
    read -u3
    [[ $REPLY == +(d) ]] || log_error "expected ddd..., got $REPLY"
    command exec 3># ((EOF))
    print -u3 -f "%.39c\n"  ^
    (( $(3<# ((CUR-0))) == 40*63 )) || log_error "not at extended end-of-file"
    command exec 3<# ((40*62))
    read -u3
    [[ $REPLY == +(^) ]] || log_error "expected ddd..., got $REPLY"
    command exec 3<# ((0))
    command exec 3<# *jjjj*
    read -u3
    [[  $REPLY == {39}(j) ]] || log_error "<# pattern failed"
    [[ $(command exec 3<## *llll*) == {39}(k) ]] || log_error "<## pattern not saving standard output"
    read -u3
    [[  $REPLY == {39}(l) ]] || log_error "<## pattern failed to position"
    command exec 3<# *abc*
    read -u3 && log_error "not found pattern not positioning at eof"
    # On Cygwin we can't use socketpair() for pipes. The ksh support for regular pipes is broken
    # which break these tests. So skip this test on Cygwin.
    #
    # TODO: Remove this restriction when support for reading from regular pipes is fixed.
    if [[ $OS_NAME == cygwin* ]]
    then
	log_warning "skipping 'read' tests on Cygwin"
    else
	cat $TEST_DIR/seek | read -r <# *WWW*
	[[ $REPLY == *WWWWW* ]] || log_error '<# not working for pipes'
    fi  # if [[ $OS_NAME == cygwin* ]]
    { < $TEST_DIR/seek <# ((2358336120)) ;} || log_error 'long seek not working'
else
    log_error "$TEST_DIR/seek: cannot open for reading"
fi

command exec 3<&- || 'cannot close 3'
for ((i=0; i < 62; i++))
do
    printf "%.39c\n"  ${x:i:1}
done >  $TEST_DIR/seek

if command exec {n}<> $TEST_DIR/seek
then
    { command exec {n}<#((EOF)) ;} 2> /dev/null || log_error '{n}<# not working'
    if $SHELL -c '{n}</dev/null' 2> /dev/null
    then
        (( $({n}<#) ==  40*62))  || log_error '$({n}<#) not working'
    else
        log_error 'not able to parse {n}</dev/null'
    fi

fi

$SHELL -ic '
{
    print -u2  || exit 2
    print -u3  || exit 3
    print -u4  || exit 4
    print -u5  || exit 5
    print -u6  || exit 6
    print -u7  || exit 7
    print -u8  || exit 8
    print -u9  || exit 9
}  3> /dev/null 4> /dev/null 5> /dev/null 6> /dev/null 7> /dev/null 8> /dev/null 9> /dev/null' > /dev/null 2>&1
exitval=$?
(( exitval ))  && log_error  "print to unit $exitval failed"

log_info "TODO: Skipping test - 'commands with standard output closed produce output'. It should be fixed later."
#$SHELL -c "{ > $TEST_DIR/1 ; date;} >&- 2> /dev/null" > $TEST_DIR/2
#[[ -s $TEST_DIR/1 || -s $TEST_DIR/2 ]] && log_error 'commands with standard output closed produce output'

# ==================
# Verify that symlinks are correctly canonicalized as part of a conditional redirection.
# Regression issue #492.
#
mkdir -p dir1/dir2
ln -s dir1 s1
cd dir1
ln -s dir2 s2
cd ..
expect=symlinks-resolved
print wrong-answer > dir1/dir2/x
print $expect >; s1/s2/x
actual=$(< dir1/dir2/x)
[[ $actual == $expect ]] || log_error "symlink in conditional redirect wrong" "$expect" "$actual"

# See https://github.com/att/ast/issues/1117. It used to be you could close any of stdin, stdout,
# or stderr and that closed fd would be passed to a child process. That behavior is extremely
# dangerous. So we now verify that closing one of those fd actually results in it being open on
# /dev/null in the child process.
#
# TODO: Figure out how to verify the fd is actually open on /dev/null in a portable manner.
$SHELL -c "$SHELL -c ': 3>&1' 1>&- 2>/dev/null" ||
    log_error 'closed standard output passed to subshell'

[[ $(cat  <<- \EOF | $SHELL
	do_it_all()
	{
		dd 2>/dev/null  # not a ksh93 buildin
		return $?
	}
	do_it_all ; exit $?
	hello world
EOF) == 'hello world' ]] || log_error 'invalid readahead on stdin'
$SHELL -c 'exec 3>; /dev/null'  2> /dev/null && log_error '>; with exec should be an error'
$SHELL -c ': 3>; /dev/null'  2> /dev/null || log_error '>; not working with at all'

print hello > $TEST_DIR/1
$SHELL -c "false >; $TEST_DIR/1"
status=$?
(( status == 1 )) || log_error "unexpected exit status" "1" "$status"
expect='hello'
actual="$(<$TEST_DIR/1)"
[[ $actual == $expect ]] || log_error '>; not preserving file on failure' "$expect" "$actual"

$SHELL -c "sed -e 's/hello/hello world/' $TEST_DIR/1" >; $TEST_DIR/1
status=$?
(( status == 0 )) || log_error "unexpected exit status" "0" "$status"
expect='hello world'
actual="$(<$TEST_DIR/1)"
[[ $actual == $expect ]] || log_error '>; not updating file on success' "$expect" "$actual"

$SHELL -c 'exec 3<>; /dev/null'  2> /dev/null && log_error '<>; with exec should be an error'
$SHELL -c ': 3<>; /dev/null'  2> /dev/null || log_error '<>; not working with at all'
print $'hello\nworld' > $TEST_DIR/1
if ! $SHELL -c "false <>; $TEST_DIR/1"  2> /dev/null
then
    [[ $(<$TEST_DIR/1) == $'hello\nworld' ]] || log_error '<>; not preserving file on failure'
fi

if ! $SHELL -c "head -1 $TEST_DIR/1" <>; $TEST_DIR/1  2> /dev/null
then
    [[ $(<$TEST_DIR/1) == hello ]] || log_error '<>; not truncating file on success of head'
fi

print $'hello\nworld' > $TEST_DIR/1
if ! $SHELL -c head  < $TEST_DIR/1 <#((6)) <>; $TEST_DIR/1  2> /dev/null
then
    [[ $(<$TEST_DIR/1) == world ]] || log_error '<>; not truncating file on success of behead'
fi

unset y
read -n1 y <<!
abc
!

if   [[ $y != a ]]
then
    log_error  'read -n1 not working'
fi

# On Cygwin we can't use socketpair() for pipes. The ksh support for regular pipes is broken which
# break these tests. So skip this test on Cygwin.
#
# TODO: Remove this restriction when support for reading from regular pipes is fixed.
if [[ $OS_NAME == cygwin* ]]
then
    log_warning "skipping 'read' tests on Cygwin"
else
    unset a
    { read -N3 a; read -N1 b;}  <<!
abcdefg
!
    [[ $a == abc ]] || log_error 'read -N3 here-document not working'
    [[ $b == d ]] || log_error 'read -N1 here-document not working'
    read -n3 a <<!
abcdefg
!
    [[ $a == abc ]] || log_error 'read -n3 here-document not working'
    (print -n a; sleep 1; print -n bcde) | { read -N3 a; read -N1 b;}
    [[ $a == abc ]] || log_error 'read -N3 from pipe not working'
    [[ $b == d ]] || log_error 'read -N1 from pipe not working'
    (print -n a; sleep 1; print -n bcde) |read -n3 a
    [[ $a == a ]] || log_error 'read -n3 from pipe not working'

    if mkfifo $TEST_DIR/fifo 2> /dev/null
    then
	(print -n a; sleep 2; print -n bcde) > $TEST_DIR/fifo &
	{
	read -u5 -n3 -t3 a || log_error 'read -n3 from fifo timed out'
	read -u5 -n1 -t3 b || log_error 'read -n1 from fifo timed out'
	} 5< $TEST_DIR/fifo
	exp=a
	got=$a
	[[ $got == "$exp" ]] || log_error "read -n3 from fifo failed -- expected '$exp', got '$got'"
	exp=b
	got=$b
	[[ $got == "$exp" ]] || log_error "read -n1 from fifo failed -- expected '$exp', got '$got'"
	rm -f $TEST_DIR/fifo
	wait
	mkfifo $TEST_DIR/fifo 2> /dev/null
	(print -n a; sleep 2; print -n bcde) > $TEST_DIR/fifo &
	{
	read -u5 -N3 -t3 a || log_error 'read -N3 from fifo timed out'
	read -u5 -N1 -t3 b || log_error 'read -N1 from fifo timed out'
	} 5< $TEST_DIR/fifo
	exp=abc
	got=$a
	[[ $got == "$exp" ]] || log_error "read -N3 from fifo failed -- expected '$exp', got '$got'"
	exp=d
	got=$b
	[[ $got == "$exp" ]] || log_error "read -N1 from fifo failed -- expected '$exp', got '$got'"
	wait
    fi

    (
	print -n 'prompt1: '
	sleep .1
	print line2
	sleep .1
	print -n 'prompt2: '
	sleep .1
    ) | {
	read -t2 -n 1000 line1
	read -t2 -n 1000 line2
	read -t2 -n 1000 line3
	read -t2 -n 1000 line4
    }

    [[ $? == 0 ]]                && log_error 'should have timed out'
    [[ $line1 == 'prompt1: ' ]]  || log_error "line1 wrong" "prompt1: " "$line1"
    [[ $line2 == line2 ]]        || log_error "line2 wrong" "line2" "$line2"
    [[ $line3 == 'prompt2: ' ]]  || log_error "line3 wrong" "prompt2: " "$line3"
    [[ ! $line4 ]]               || log_error "line4 should be empty" "" "$line4"

    typeset -a e o=(-n2 -N2)
    integer i
    set -- \
	'a'    'bcd'    'a bcd'    'ab cd' \
	'ab'    'cd'    'ab cd'    'ab cd' \
	'abc'    'd'    'ab cd'    'ab cd' \
	'abcd'    ''    'ab cd'    'ab cd'
    while (( $# >= 3 ))
    do
	a=$1
	b=$2
	e[0]=$3
	e[1]=$4
	shift 4
	for ((i = 0; i < 2; i++))
	do
	    for lc_all in C en_US.UTF-8
	    do
		g=$(LC_ALL=$lc_all $SHELL -c "{ print -n '$a'; sleep 0.2; print -n '$b'; sleep 0.2; } | { read ${o[i]} a; print -n \$a; read a; print -n \ \$a; }")
		[[ $g == "${e[i]}" ]] || log_error "LC_ALL=$lc_all read ${o[i]} from pipe '$a $b' failed -- expected '${e[i]}', got '$g'"
	    done
	done
    done
fi  # if [[ $OS_NAME == cygwin* ]]

export LC_ALL=en_US.UTF-8
typeset -a c=( '' 'A' $'\303\274' $'\342\202\254' )
integer i w
typeset o
if (( ${#c[2]} == 1 && ${#c[3]} == 1 ))
then
for i in 1 2 3
do
    for o in n N
    do
        for w in 1 2 3
        do
            print -nr "${c[w]}" | read -${o}${i} g
            if [[ $o == N ]] && (( i > 1 ))
            then
                e=''
            else
                e=${c[w]}
            fi

            [[ $g == "$e" ]] || log_error "read -${o}${i} failed for '${c[w]}' -- expected '$e', got '$g'"
            done
        done
    done
fi

exec 3<&2
file=$TEST_DIR/file
redirect 5>$file 2>&5
print -u5 -f 'This is a test\n'
print -u2 OK
exec 2<&3
exp=$'This is a test\nOK'
got=$(< $file)
[[ $got == $exp ]] || log_error "output garbled when stderr is duped -- expected $(printf %q "$exp"), got $(printf %q "$got")"
print 'hello world' > $file
1<>; $file  1># ((5))
(( $(wc -c < $file) == 5 )) || log_error "$file was not truncate to 5 bytes"

$SHELL -c "PS4=':2:'
    exec 1> $TEST_DIR/21.out 2> $TEST_DIR/22.out
    set -x
    printf ':1:A:'
    print \$(:)
    print :1:Z:" 1> $TEST_DIR/11.out 2> $TEST_DIR/12.out
[[ -s $TEST_DIR/11.out ]] && log_error "standard output leaked past redirection"
[[ -s $TEST_DIR/12.out ]] && log_error "standard error leaked past redirection"

exp=$':1:A:\n:1:Z:'
got=$(<$TEST_DIR/21.out)
[[ $exp == "$got" ]] || log_error "standard output garbled -- expected $(printf %q "$exp"), got $(printf %q "$got")"
exp=$':2:printf :1:A:\n:2::\n:2:print\n:2:print :1:Z:'
got=$(<$TEST_DIR/22.out)
[[ $exp == "$got" ]] || log_error "standard error garbled -- expected $(printf %q "$exp"), got $(printf %q "$got")"

$SHELL -c 'exec 3<&1 ; exec 1<&- ; exec > outfile; print foobar' ||
    log_error 'exec 1<&- causes failure'
expect=foobar
actual=$(< outfile)
[[ $actual == $expect ]] || log_error 'outfile content wrong' "$expect" "$actual"

print hello there world > $TEST_DIR/foobar
sed  -e 's/there //' $TEST_DIR/foobar  >; $TEST_DIR/foobar
[[ $(<$TEST_DIR/foobar) == 'hello world' ]] || log_error '>; redirection not working on simple command'
print hello there world > $TEST_DIR/foobar
{ sed  -e 's/there //' $TEST_DIR/foobar;print done;} >; $TEST_DIR/foobar 
[[ $(<$TEST_DIR/foobar) == $'hello world\ndone' ]] || log_error '>; redirection not working for compound command'
print hello there world > $TEST_DIR/foobar
$SHELL -c "sed  -e 's/there //' $TEST_DIR/foobar  >; $TEST_DIR/foobar"
[[ $(<$TEST_DIR/foobar) == 'hello world' ]] || log_error '>; redirection not working with -c on a simple command'

# Cygwin lets you open a file for writing even though it doesn't have write permission.
# So skip this test on Cygwin.
if [[ $OS_NAME == cygwin* ]]
then
    log_warning "skipping open for writing of read-only file on Cygwin"
else
    chmod -w $TEST_DIR/foobar
    (: >; $TEST_DIR/foobar) 2> /dev/null && '>; should fail for a file without write permission'
fi  # if [[ $OS_NAME == cygwin* ]]

rm -f "$TEST_DIR/junk"
for (( i=1; i < 50; i++ ))
do
    out=$(/bin/ls "$TEST_DIR/junk" 2>/dev/null)
    if (( $? == 0 ))
    then
        log_error 'wrong error code with redirection'
        break
    fi
done

for i in 1
do
    :
done    {n}< /dev/null
[[ -r /dev/fd/$n ]] &&  log_error "file descriptor n=$n left open after {n}<"

n=$( exec {n}< /dev/null; print -r -- $n)
[[ -r /dev/fd/$n ]] && log_error "file descriptor n=$n left open after subshell"

(
    integer error_count=0
    redirect {n}> $TEST_DIR/foo; print foobar >&{n} > $TEST_DIR/foo
    [[ $(<$TEST_DIR/foo) == foobar ]] || log_error '>& {n} not working for write'
    { got=$( redirect {n}< $TEST_DIR/foo; cat <&{n} ) ;} 2> /dev/null
    [[ $got == foobar ]] || log_error  ' <& {n} not working for read'
    exit $((error_count))
) & wait $!
((error_count += $?))

# According to POSIX <> should redirect stdin when no fd is specified
read -n 1 -t 0.1 <>/dev/zero || log_error '<> should redirect stdin by default'

TMPF=$TEST_DIR/tmpf
echo foo >$TMPF
export TMPF

[[ -n "$($SHELL -c 'echo $(<$TMPF)' <&-)" ]] || log_error "Closing stdin causes failure when reading file through \$(<)"
[[ -n "$($SHELL -c "$SHELL -c 'echo \$(<$TMPF) >&2' >&-" 2>&1)" ]] || log_error "Closing stdout causes failure when reading file through \$(<)"
[[ -n "$($SHELL -c 'echo $(<$TMPF)' 2>&-)" ]]  || log_error "Closing stderr causes failure when reading file through \$(<)"

# ==========
# https://github.com/att/ast/issues/9
echo foo bar > baz
$SHELL -c 'echo xxx 1<>; baz'
actual=$(cat baz)
expect="xxx"
[[ "$actual" = "$expect" ]] || log_error "<>; does not truncate files" "$expect" "$actual"

# ==========
# https://github.com/att/ast/issues/61
seq 10 > $TEST_DIR/foo
$SHELL -c '1<>; $TEST_DIR/foo >#5'
actual=$(cat $TEST_DIR/foo)
expect=$'1\n2\n3\n4'
[[ "$actual" = "$expect" ]] || log_error "Failed to truncate file" "$expect" "$actual"

# ==========
# https://github.com/att/ast/issues/1091
# Detecting EOF when reading from a fifo may fail on platforms like macOS unless our feature
# test for the correct behavior of poll() detects that it is broken.
mkfifo io_fifo_poll
(print abc; sleep 2; print ABC) >io_fifo_poll &
expect=$'abc\nABC'
actual=$( $SHELL -c 'while read foo; do echo "$foo"; done' <io_fifo_poll )
[[ "$actual" = "$expect" ]] || log_error "fifo I/O failed" "$expect" "$actual"

# ==========
# https://github.com/att/ast/issues/1336
# Use the /proc pseudo filesystem on Linux as a convenient way to force a write I/O error.
# TODO: Try to find a mechanism for forcing an I/O error on other platforms.
if [[ $OS_NAME == linux ]]
then
    actual=$($SHELL -c 'echo > /proc/self/uid_map; echo okay' 2>&1)
    expect='write.*failed.*okay'
    [[ "$actual" =~ $expect ]] || log_error "I/O failure not handled" "$expect" "$actual"
fi
