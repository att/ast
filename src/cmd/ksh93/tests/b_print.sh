# Tests for print builtin

# ======
# The `print -R` flag has some very unusual semantics. Make sure we verify most of them.
expect="-"
actual=$(print -R -)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-x"
actual=$(print -R -x)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-- x"
actual=$(print -R -- x)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-- xabc"
actual=$(print -Rn -- x; print abc)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-- xabc"
actual=$(print -nR -- x; print abc)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-z xabc"
actual=$(print -R -n -z x; print abc)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-u2 xabc"
actual=$(print -RZ -n -u2 x; print abc)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-z xabc"
actual=$(print -Rn -n -z x; print abc)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

expect="-n -z xabc"
actual=$(print -Rn -n -n -z x; print abc)
[[ $actual == $expect ]] || log_error "print -R not working correctly" "$expect" "$actual"

# ======
if [[ $(print - -) != - ]]
then
    log_error "print - not working correctly"
fi

# ======
if [[ $(print -- -) != - ]]
then
    log_error "print -- not working correctly"
fi

# ======
# -e Unless -f is specified, process \ sequences in each string operand as described above. This is the default behavior.
print -e "\n" | od | head -n1 | grep -q "012 *$" || log_error "print -e should interpret escape sequences"

# ======
# -n Do not append a new-line character to the output.
[[ $(print -n "hello world") = "hello world" ]] || log_error "print -n should not append a newline"

# ======
# -f format       Write the string arguments using the format string format and do not append a
# new-line. See printf for details on how to specify format.
# Add a very basic test for 'print -f'. Format strings are tested by `printf` tests, shall we redo
# all tests here ?
[[ $(print -f "%s" "hello world") = "hello world" ]] || log_error "printf -f does not recognize format string"

# ======
# -p Write to the current co-process instead of standard output.
# Start a cat coprocess
cat |&
print -p "Hello World"
read -p foo
[[ "$foo" = "Hello World" ]] || log_error "print -p should write to current coprocess"

# ======
# -r Do not process \ sequences in each string operand as described above.
print -r "\a" | od | head -n1 | grep -q "007 *$" && log_error "print -r should not interpret escape sequences"

# ======
# -s Write the output as an entry in the shell history file instead of standard output.
if print -s 'print hello world' 2> /dev/null
then
    [[ $(history -1) == *'hello world'* ]] || log_error 'history file does not contain result of print -s'
else
    log_error 'print -s fails'
fi

# ======
# -u fd Write to file descriptor number fd instead of standard output. The default value is 1.
exec 5>foo
print -u5 "bar baz"
[[ $(cat foo) = "bar baz" ]] || log_error "print -u should print to file descriptor"

# ======
# print -u without a value should show an error
actual=$(print -u 2>&1)
expected="print: -u: expected argument for option"
[[ "$actual" =~ "$expected" ]] ||
    log_error "print -u without arguments should show an error" "$expected" "$actual"

# ======
# -v Treat each string as a variable name and write the value in %B format. Cannot be used with -f.
foo=bar
[[ $(print -v foo) = "bar" ]] || log_error "print -v should print variable value"
unset foo

# ======
# -C Treat each string as a variable name and write the value in %#B format. Cannot be used with -f.
foo=(bar baz)
[[ $(print -C foo) = "(bar baz)" ]] || log_error "print -C should print compound variable in single line"
unset foo

# ======
if [[ $(print -f "%b" "\a\n\v\b\r\f\E\03\\oo") != $'\a\n\v\b\r\f\E\03\\oo' ]]
then
    log_error 'print -f "%b" not working'
fi

# ======
if [[ $(print -f "%P" "[^x].*b\$") != '*[!x]*b' ]]
then
    log_error 'print -f "%P" not working'
fi

# ======
if [[ $(print -f "%(pattern)q" "[^x].*b\$") != '*[!x]*b' ]]
then
    log_error 'print -f "%(pattern)q" not working'
fi

# ======
# Check if history file is updated correclty if entry does not end with newline
if print -s -f 'print foo' 2> /dev/null
then
    [[ $(history -1) == *'foo' ]] || log_error 'history file does not contain result of print -s -f'
else
    log_error 'print -s -f fails'
fi

# ======
# Check that I/O errors are detected <https://github.com/att/ast/issues/1093>
actual=$(
    {
        (
            trap "" PIPE
            for ((i = SECONDS + 1; SECONDS < i; )); do
                print hi || {
                    print $? >&2
                    exit
                }
            done
        ) | true
    } 2>&1
)
expect=$': print: I/O error\n1'
if [[ $actual != *"$expect" ]]
then
    log_error 'I/O error not detected' "$expect" "$actual"
fi
