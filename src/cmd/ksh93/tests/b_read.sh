# Tests for read builtin
read <<!
hello world
!
[[ $REPLY == 'hello world' ]] || log_error "read builtin failed"

# -A Unset var and then create an indexed array containing each field in the line starting at index
# 0.
read -A foo<<!
bar baz
!
[[ $foo[0] = "bar" ]] && [[ $foo[1] == "baz" ]] || log_error "read -A does not create indexed arrays"

# -a is an alias for -A
read -a foo<<!
bar baz
!
[[ $foo[0] = "bar" ]] && [[ $foo[1] == "baz" ]] || log_error "read -a does not create indexed arrays"

#-C Unset var and read var as a compound variable. Equivalent to -aksh
read -C foo<<!
( foo1=bar1 foo2=bar2 )
!
[[ ${foo.foo1} = "bar1" ]] && [[ ${foo.foo2} = "bar2" ]] ||
    log_error "read -C does recognize compound variables"
unset foo

#-d delim Read until delimiter delim instead of to the end of line.
read -d x <<!
hello worldxfoobar
!
[[ $REPLY == 'hello world' ]] || log_error "read -d does misbehaves with custom delimeter"

#-m method Unset var and read var as a compound variable in the specified method. Currently only
# json and ksh methods are supported.
# TODO: See issue#820

#-r Do not treat \ specially when processing the input line.
read -r var <<\!

!
if [[ $var != "" ]]
then
    log_error "\\ should not be treated specially with read -r"
fi

#-s Save a copy of the input as an entry in the shell history file.
echo "This line should be in history" | read -s foo
history | tail -n1 | grep -q "This line should be in history" ||
    log_error "read -s does not save input to history"

#-S Treat the input as if it was saved from a spreadsheet in csv format.
IFS=',' read -S a b c <<<'foo,"""title"" data",bar'
[[ $b == '"title" data' ]] || log_error '"" inside "" not handled correctly with read -S'

# ==========
#-u fd Read from file descriptor number fd instead of standard input. If fd is p, the co-process
# input file descriptor is used. The default value is 0.
echo bar > $TEST_DIR/bar
exec 7< $TEST_DIR/bar
read -u7 foo
[[ $foo = "bar" ]] || log_error "read -u does not work"
unset foo
exec 7<&-

# -u without arguments should show an error
actual=$(read -u 2>&1)
expected="read: -u: expected argument for option"
[[ "$actual" =~ "$expected" ]] || log_error "read -u without any arguments should show an error" "$expected" "$actual"

#-t timeout Specify a timeout timeout in seconds when reading from a terminal or pipe.
(echo -n hello; sleep 1) | read -t0.5 foo
echo $foo
# foo should be empty due to timeout
[[ $foo = "" ]] || log_error "read -t does not timeout"

#-N count Read exactly ncount characters. For binary fields count is the number of bytes.
unset foo
(echo -n helloworld) | read -N5 foo
[[ $foo = "hello" ]] || log_error "read -N does not work"

#-n count Read at most count characters. For binary fields count is the number of bytes.
# Return as soon as some characters have been read if reading from a slow device
unset foo
(echo -n hello; sleep 1; echo -n world) | read -n10 foo
[[ $foo = "hello" ]] || log_error "read -n does not work"

read <<!
hello \
world
!
[[ $REPLY == 'hello world' ]] || log_error "read continuation failed"

read <<\!
hello \
    world \

!
[[ $REPLY == 'hello     world' ]] || log_error "read continuation2 failed"

read <<\!
\
a\
\
\
b
!
if [[ $REPLY != ab ]]
then
    log_error "read multiple continuation failed"
fi

print x:y | IFS=: read a b
if [[ $a != x ]]
then
    log_error "IFS=: read ... not working"
fi

expect=a
read -n1 actual <<!
abc
!
[[ $actual == $expect ]] || log_error "read -n1 failed" "$expect" "$actual"

expect=ABC
read -n3 actual <<!
ABCDEFG
!
[[ $actual == $expect ]] || log_error "read -n3 here-document failed" "$expect" "$actual"

expect=abc
unset actual
{ read -N3 actual; read -N1 b;}  <<!
abcdefg
!
[[ $actual == $expect ]] || log_error "read -N3 here-document failed" "$expect" "$actual"
expect=d
actual="$b"
[[ $actual == $expect ]] || log_error "read -N1 here-document failed" "$expect" "$actual"

# This test is flakey under Cygwin. The actual execution time is sometimes double what is allowed.
# See https://github.com/att/ast/issues/1289.
# TODO: Figure out how to make this behavior reliable on Cygwin.
if [[ $OS_NAME != cygwin* ]]
then
    typeset -F3 start_x=SECONDS total_t delay=0.02
    typeset reps=50 leeway=5
    sleep $(( 2 * leeway * reps * delay )) |
        for (( i=0 ; i < reps ; i++ ))
        do
            read -N1 -t $delay
        done
    (( total_t = SECONDS - start_x ))
    if (( total_t > leeway * reps * delay ))
    then
        log_error "read -t in pipe taking $total_t secs - $(( reps * delay )) minimum - too long"
    elif (( total_t < reps * delay ))
    then
        log_error "read -t in pipe taking $total_t secs - $(( reps * delay )) minimum - too fast"
    fi
fi

print "one\ntwo" | { read line
    print $line | /bin/cat > /dev/null
    read line
}

if [[ $line != two ]]
then
    log_error "read from pipeline failed"
fi

line=two
read line < /dev/null
if [[ $line != "" ]]
then
    log_error "read from /dev/null failed"
fi


($SHELL -c 'read x[1] <<< hello') 2> /dev/null || log_error 'read x[1] not working'

#FIXME#($SHELL read -s foobar <<\!
#FIXME#testing
#FIXME#!
#FIXME#) 2> /dev/null || log_error ksh read -s var fails

behead()
{
    read line
    left=$(cat)
}
print $'line1\nline2' | behead
if [[ $left != line2 ]]
then
    log_error "read reading ahead on a pipe"
fi


print -n $'{ read -r line;print $line;}\nhello' > $TEST_DIR/script
chmod 755 $TEST_DIR/script
if [[ $($SHELL < $TEST_DIR/script) != hello ]]
then
    log_error 'read of incomplete line not working correctly'
fi

read baz <<< 'foo\\\\bar'
[[ $baz == 'foo\\bar' ]] || log_error 'read of foo\\\\bar not getting foo\\bar'

printf '\\\000' | read -r -d ''
[[ $REPLY == $'\\' ]] || log_error "read -r -d'' ignores -r"
