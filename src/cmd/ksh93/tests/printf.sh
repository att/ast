# Tests for printf builtin
# %b    A %b format can be used instead of %s to cause escape sequences in the corresponding arg to
#       be expanded as described in print.
# \a    Alert character.
printf "%b" "\a" | od | head -n1 | grep -q "007 *$" || log_error "%b does not recognize \\a"

# \b    Backspace character.
printf "%b" "\b" | od | head -n1 | grep -q "010 *$" || log_error "%b does not recognize \\b"

# \c    Terminate output without appending newline. The remaining string operands are ignored.
printf "%b" "a\cb" | od | head -n1 | grep -q "141 *$" || log_error "%b does not recognize \\c"

# \f    Formfeed character.
printf "%b" "\f" | od | head -n1 | grep -q "014 *$" || log_error "%b does not recognize \\f"

# \n    Newline character.
printf "%b" "\n" | od | head -n1 | grep -q "012 *$" || log_error "%b does not recognize \\n"

# \t    Tab character.
printf "%b" "\t" | od | head -n1 | grep -q "011 *$" || log_error "%b does not recognize \\t"

# \v    Vertical tab character.
printf "%b" "\v" | od | head -n1 | grep -q "013 *$" || log_error "%b does not recognize \\v"

# \\    Backslash character.
printf "%b" "\\" | od | head -n1 | grep -q "134 *$" || log_error "%b does not recognize \\"

# \E    Escape character (ASCII octal 033).
printf "%b" "\E" | od | head -n1 | grep -q "033 *$" || log_error "%b does not recognize \\E"

# \0x   The 8-bit character whose ASCII code is the 1-, 2-, or
#       3-digit octal number x.
[[ $(printf "%b" "\0101") = 'A' ]] || log_error "%b does not recognize \\0xxx sequences"

# %B A  %B  option causes each of the arguments to be treated as variable names and the binary value
# of variable will be printed.  The alternate flag # causes a compound variable to be output on a
# single line.  This is most useful for compound variables and variables whose attribute is -b.
foo=bar
[[ $(printf "%B" foo) = "bar" ]] || log_error "printf %B does not recognize variables"
unset foo

# %H     A %H format can be used instead of %s to cause characters in arg that are special in HTML
#        and XML to be output as their entity name.  The alternate flag # formats the output for
#        use as a URI.
if [[ $(printf '%H\n' $'<>"& \'\tabc') != '&lt;&gt;&quot;&amp;&nbsp;&apos;&#9;abc' ]]
then
    log_error 'printf %H not working'
fi

# TODO:  This may be buggy. printf "%P" "^foo{bar,baz}$" converts to fo{bar,baz}(o)
# %P     A %P format can be used instead of %s to cause arg to be interpreted as an extended regular
#        expression and be printed as a shell pattern.

# %R     A %R format can be used instead of %s to cause arg to be interpreted as a shell pattern and
#        to be printed as an extended regular expression.
[[ $(printf "%R" "foo{bar,baz}") = "^foo{bar,baz}$" ]] ||
    log_error "printf %R does not convert shell pattern to regex"

# %q     A %q format can be used instead of %s to cause the resulting string to be quoted in a manner
#        than can be reinput to the shell.  When q is preceded by the alternative format specifier, #,
#        the string is quoted in manner suitable as a field in a .csv format file.
[[ $(printf "%q" "foo\nbar\nbaz") = "'foo\nbar\nbaz'" ]] ||
    log_error "printf %q does not quote characters"

# %(date-format)T
#        A %(date-format)T format can be use to treat an argument as a date/time string and to
#        format the date/time according to the date-format as defined for the date(1) command.
#        Values specified as digits are interpreted as described in the touch(1) command.

# This test is can fail if the load on the machine causes the printf and date commands to run on
# opposite sides of the time rolling from one second to the next. We've seen it fail twice in a row
# so try three times. Also, the TZ may be GMT or UTC for the builtin printf and external date
# command and they won't necessarily agree. So normalize both to UTC.
for i in 1 2 3
do
   expect=$(env LANG=C date | sed -e 's/GMT/UTC/')
   actual=$(env LANG=C $SHELL -c 'printf "%T\n" now' | sed -e 's/GMT/UTC/')
   if [[ "$actual" == "$expect" ]]
   then
      break
   else
      log_warning 'printf "%T" now wrong output' "$expect" "$actual"
   fi
done
[[ "$actual" == "$expect" ]] || log_error 'printf "%T" now wrong output' "$expect" "$actual"

# %Z     A %Z format will output a byte whose value is 0.
printf "%Z" | od | head -n1 | grep -q "000000 *$" || log_error "printf %Z does not output null byte"

# %d     The precision field of the %d format can be followed by a .  and the output base.  In this
#        case, the # flag character causes base# to be prepended.

[[ $(printf "%.5d" 100) == "00100" ]] || log_error "printf %d does not recognize precision field"

# #      The # flag, when used with the %d format without an output base, displays the output in
#        powers of 1000 indicated by one of the following suffixes: k M G T P E, and when used with
#        the %i format displays the output in powers of 1024 indicated by one of the following
#        suffixes: Ki Mi Gi Ti Pi Ei.
[[ $(printf "%#d" 1000) = "1.0k" ]] || log_error "printf %#d does not set k suffix"
[[ $(printf "%#d" 10000) = "10k" ]] && [[ $(printf "%#d" 100000) = "100k" ]] ||
    log_error "printf %#d miscalculates k suffix"
[[ $(printf "%#d" 1000000) = "1.0M" ]] || log_error "printf %#d does not set M suffix"
[[ $(printf "%#d" 1000000000)  = "1.0G" ]] || log_error "printf %#d does not set G suffix"
[[ $(printf "%#d" 1000000000000) = "1.0T" ]] || log_error "printf %#d does not set T suffix"
[[ $(printf "%#d" 1000000000000000) = "1.0P" ]] || log_error "printf %#d does not set P suffix"
[[ $(printf "%#d" 1000000000000000000) = "1.0E" ]] || log_error "printf %#d does not set E suffix"

[[ $(printf "%#i" 1024) = "1.0Ki" ]] || log_error "printf %#i does not set ki suffix"
[[ $(printf "%#i" 10240) = "10Ki" ]] && [[ $(printf "%#i" 102400) = "100Ki" ]] ||
    log_error "printf %#i miscalculates ki suffix"
[[ $(printf "%#i" 1048576) = "1.0Mi" ]] || log_error "printf %#d does not set Mi suffix"
[[ $(printf "%#i" 1073741824)  = "1.0Gi" ]] || log_error "printf %#d does not set Gi suffix"
[[ $(printf "%#i" 1099511627776) = "1.0Ti" ]] || log_error "printf %#d does not set Ti suffix"
[[ $(printf "%#i" 1125899906842624) = "1.0Pi" ]] || log_error "printf %#d does not set Pi suffix"
[[ $(printf "%#i" 1152921504606846976) = "1.0Ei" ]] || log_error "printf %#d does not set Ei suffix"

# =      The = flag centers the output within the specified field width.

# L      The L flag, when used with the %c or %s formats, treats precision as character width
#        instead of byte count.
[[ $(printf "%2Ls" "£") = " £" ]] || log_error "printf %L does not recognize character width"

# ,      The , flag, when used with the %d or %f formats, separates groups of digits with the
#        grouping delimiter (, on groups of 3 in the C locale.)
[[ $(printf "%,d" 100000000) = '100,000,000' ]] || log_error ", flag does not delimit numbers"

#     %(csv)q
#           Equivalent to %#q.
[[ $(printf "%(csv)q" "foo bar") = '"foo bar"' ]] || log_error "printf %(csv)q does not quote csv"

#     %(ere)q
#           Equivalent to %R.
[[ $(printf "%(ere)q" "foo{bar,baz}") = "^foo{bar,baz}$" ]] ||
    log_error "printf %(ere)q does not convert shell pattern to regex"


#     %(html)q
#           Equivalent to %H.
if [[ $(printf '%(html)q\n' $'<>"& \'\tabc') != '&lt;&gt;&quot;&amp;&nbsp;&apos;&#9;abc' ]]
then
    log_error 'printf %(html)q not working'
fi

# TODO
#     %(nounicodeliterals)q
#           Equivalent to %0q.

#     %(pattern)q
#           Equivalent to %P.

#     %(unicodeliterals)q
#           Equivalent to %+q.

#     %(url)q
#           Equivalent to %#H.
if [[ $( printf 'foo://ab_c%(url)q\n' $'<>"& \'\tabc') != 'foo://ab_c%3C%3E%22%26%20%27%09abc' ]]
then
    log_error 'printf %(url)q not working'
fi

print -f "hello%nbar\n" size > /dev/null
if ((    size != 5 ))
then
    log_error "%n format of printf not working"
fi

n=123
typeset -A base
base[o]=8#
base[x]=16#
base[X]=16#
for i in d i o u x X
do
    if (( $(( ${base[$i]}$(printf "%$i" $n) )) != n  ))
    then
        log_error "printf %$i not working"
    fi
done

typeset -Z3 percent=$(printf '%o\n' "'%'")
forrmat=\\${percent}s
if   [[ $(printf "$forrmat") != %s ]]
then
    log_error "printf $forrmat not working"
fi

if (( $(printf 'x\0y' | wc -c) != 3 ))
then
    log_error 'printf \0 not working'
fi

if [[ $(printf "%bx%s\n" 'f\to\cbar') != $'f\to' ]]
then
    log_error 'printf %bx%s\n  not working'
fi

alpha=abcdefghijklmnop
if [[ $(printf "%10.*s\n" 5 $alpha) != '     abcde' ]]
then
    log_error 'printf %10.%s\n  not working'
fi

float x2=.0000625
if [[ $(printf "%10.5E\n" x2) != 6.25000E-05 ]]
then
    log_error 'printf "%10.5E" not normalizing correctly'
fi

x2=.000000001
if [[ $(printf "%g\n" x2 2>/dev/null) != 1e-09 ]]
then
    log_error 'printf "%g" not working correctly'
fi

if [[ $(printf +3 2>/dev/null) !=   +3 ]]
then
    log_error 'printf is not processing formats beginning with + correctly'
fi

if printf "%d %d\n" 123bad 78 >/dev/null 2>/dev/null
then
    log_error "printf not exiting non-zero with conversion errors"
fi


if [[ $(printf '%R %R %R %R\n' 'a.b' '*.c' '^'  '!(*.*)') != '^a\.b$ \.c$ ^\^$ ^(.*\..*)!$' ]]
then
    log_error 'printf %R not working'
fi

if [[ $(printf '%(ere)q %(ere)q %(ere)q %(ere)q\n' 'a.b' '*.c' '^'  '!(*.*)') != '^a\.b$ \.c$ ^\^$ ^(.*\..*)!$' ]]
then
    log_error 'printf %(ere)q not working'
fi

if [[ $(printf '%..:c\n' abc) != a:b:c ]]
then
    log_error "printf '%..:c' not working"
fi

if [[ $(printf '%..*c\n' : abc) != a:b:c ]]
then
    log_error "printf '%..*c' not working"
fi

if [[ $(printf '%..:s\n' abc def ) != abc:def ]]
then
    log_error "printf '%..:s' not working"
fi

if [[ $(printf '%..*s\n' : abc def) != abc:def ]]
then
    log_error "printf '%..*s' not working"
fi

[[ $(printf '%q\n') == '' ]] || log_error 'printf "%q" with missing arguments'

# ==========
# printf "%Q" converts seconds to readable time
actual=$(printf "%Q" 66)
expected="1m06s"
[[ "$actual" = "$expected" ]] || log_error "printf %Q does not convert seconds to minutes" "$expected" "$actual"

actual=$(printf "%Q" 3660)
expected="1h01m"
[[ "$actual" = "$expected" ]] || log_error "printf %Q does not convert seconds to hours" "$expected" "$actual"

# ==========
# printf "%p" converts to hexadecimal number
actual=$(printf "%p" 16)
expected="0x10"
[[ "$actual" = "$expected" ]] || log_error "printf %p does not convert to hexadecimal" "$expected" "$actual"

# ==========
if [[ $($SHELL -c $'printf \'%2$s %1$s\n\' world hello') != 'hello world' ]]
then
    log_error 'printf %2$s %1$s not working'
fi

val=$(( 'C' ))
set -- \
    "'C"    $val    0    \
    "'C'"    $val    0    \
    '"C'    $val    0    \
    '"C"'    $val    0    \
    "'CX"    $val    1    \
    "'CX'"    $val    1    \
    "'C'X"    $val    1    \
    '"CX'    $val    1    \
    '"CX"'    $val    1    \
    '"C"X'    $val    1
while (( $# >= 3 ))
do
    arg=$1 expect=$2 code=$3
    shift 3
    for fmt in '%d' '%g'
    do
        actual=$(printf "$fmt" "$arg" 2>/dev/null)
        err=$(printf "$fmt" "$arg" 2>&1 >/dev/null)
        printf "$fmt" "$arg" >/dev/null 2>&1
        ret=$?
        [[ $actual == $expect ]] || log_error "printf $fmt $arg failed" "$expect" "$actual"
        if (( $code ))
        then
            [[ $err ]] || log_error "printf $fmt $arg failed, error message expected"
        else
            [[ $err ]] &&
               log_error "$err: printf $fmt $arg failed, error message not expected" "" "$err"
        fi

        (( $ret == $code )) || log_error "printf $fmt $arg failed -- wrong status" "$code" "$ret"
    done
done

n=$(printf "%b" 'a\0b\0c' | wc -c)
(( n == 5 )) || log_error '\0 not working with %b format with printf'

[[ $($SHELL -c '{ printf %R "["; print ok;}' 2> /dev/null) == ok ]] || log_error $'\'printf %R "["\' causes shell to abort'
