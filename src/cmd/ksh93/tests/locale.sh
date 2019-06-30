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

unset LANG ${!LC_*}

# test shift-jis \x81\x40 ... \x81\x7E encodings
# (shift char followed by 7 bit ascii)

typeset -i16 chr
for locale in $(PATH=/bin:/usr/bin locale -a 2>/dev/null | grep -i jis)
do
    export LC_ALL=$locale
    for ((chr=0x40; chr<=0x7E; chr++))
    do
        c=${chr#16#}
        for s in \\x81\\x$c \\x$c
        do
            b="$(printf "$s")"
            eval n=\$\'$s\'
            [[ $b == "$n" ]] || log_error "LC_ALL=$locale printf difference for \"$s\" -- expected '$n', got '$b'"
            u=$(print -- $b)
            q=$(print -- "$b")
            [[ $u == "$q" ]] || log_error "LC_ALL=$locale quoted print difference for \"$s\" -- $b => '$u' vs \"$b\" => '$q'"
        done
    done
done

# this locale is supported by ast on all platforms
# EU for { decimal_point="," thousands_sep="." }

locale=en_US.UTF-8

export LC_ALL=C

# test multibyte value/trace format -- $'\303\274' is UTF-8 u-umlaut

c=$(LC_ALL=C $SHELL -c "printf $':%2s:\n' $'\303\274'")
u=$(LC_ALL=$locale $SHELL -c "printf $':%2s:\n' $'\303\274'" 2>/dev/null)
if [[ "$c" != "$u" ]]
then
    LC_ALL=$locale
    x=$'+2+ typeset item.text\
+3+ item.text=\303\274\
+4+ print -- \303\274\
\303\274\
+5+ eval $\'arr[0]=(\\n\\ttext=\\303\\274\\n)\'
+2+ arr[0].text=ü\
+6+ print -- \303\274\
ü\
+7+ eval txt=$\'(\\n\\ttext=\\303\\274\\n)\'
+2+ txt.text=\303\274\
+8+ print -- \'(\' text=$\'\\303\\274\' \')\'\
( text=\303\274 )'
    u=$(LC_ALL=$locale PS4='+$LINENO+ ' $SHELL -x -c "
        item=(typeset text)
        item.text=$'\303\274'
        print -- \"\${item.text}\"
        eval \"arr[0]=\$item\"
        print -- \"\${arr[0].text}\"
        eval \"txt=\${arr[0]}\"
        print -- \$txt
    " 2>&1)
    [[ "$u" == "$x" ]] || log_error LC_ALL=$locale multibyte value/trace format failed

    x=$'00fc\n20ac'
    u=$(LC_ALL=$locale $SHELL -c $'printf "%04x\n" \$\'\"\303\274\"\' \$\'\"\xE2\x82\xAC\"\'')
    [[ $u == $x ]] || log_error LC_ALL=$locale multibyte %04x printf format failed
fi


if (( $($SHELL -c $'export LC_ALL='$locale$'; print -r "\342\202\254\342\202\254\342\202\254\342\202\254w\342\202\254\342\202\254\342\202\254\342\202\254" | wc -m' 2>/dev/null) == 10 ))
then
    LC_ALL=$locale $SHELL -c b1=$'"\342\202\254\342\202\254\342\202\254\342\202\254w\342\202\254\342\202\254\342\202\254\342\202\254"; [[ ${b1:4:1} == w ]]' || log_error 'multibyte ${var:offset:len} not working correctly'
fi

# TODO: On FreeBSD 11.1 it's `wc` command gives `LANG` higher priority than `LC_ALL`.
if [[ $OS_NAME != freebsd ]]
then

# The following bytes are the UTF-8 encoding of "\u[20ac]", twice.
printf $'\342\202\254\342\202\254' > $TEST_DIR/two_euro_chars.txt
locale=en_US.UTF-8
set -- $($SHELL -c "
    export LANG=$locale
    export LC_ALL=C
    command wc -m < $TEST_DIR/two_euro_chars.txt
    unset LC_ALL
    command wc -m < $TEST_DIR/two_euro_chars.txt
    export LC_ALL=C
    command wc -m < $TEST_DIR/two_euro_chars.txt
")
actual=$*
expect="6 2 6"
[[ $actual == $expect ]] || log_error "command wc LC_ALL default failed" "$expect" "$actual"

set -- $($SHELL -c "
    if builtin wc 2>/dev/null || builtin -f cmd wc 2>/dev/null
    then
        export LANG=$locale
        export LC_ALL=C
        wc -m < $TEST_DIR/two_euro_chars.txt
        unset LC_ALL
        wc -m < $TEST_DIR/two_euro_chars.txt
        export LC_ALL=C
        wc -m < $TEST_DIR/two_euro_chars.txt
    fi

")
actual=$*
expect="6 2 6"
[[ $actual == $expect ]] || log_error "builtin wc LC_ALL default failed" "$expect" "$actual"

fi  # $OS_NAME != freebsd

# multibyte char straddling buffer boundary

# See https://github.com/att/ast/issues/177
#locale=C_EU.UTF-8
#
#{
#    unset i
#    integer i
#    for ((i = 0; i < 163; i++))
#    do    print "#234567890123456789012345678901234567890123456789"
#    done
#    printf $'%-.*c\n' 15 '#'
#    for ((i = 0; i < 2; i++))
#    do    print $': "\xe5\xae\x9f\xe8\xa1\x8c\xe6\xa9\x9f\xe8\x83\xbd\xe3\x82\x92\xe8\xa1\xa8\xe7\xa4\xba\xe3\x81\x97\xe3\x81\xbe\xe3\x81\x99\xe3\x80\x82" :'
#    done
#} > ko.dat
#
#LC_ALL=$locale $SHELL < ko.dat 2> /dev/null || log_error "script with multibyte char straddling buffer boundary fails"
#
##    exp    LC_ALL        LC_NUMERIC    LANG
#set -- \
#    2,5    $locale        C        ''        \
#    2.5    C        $locale        ''        \
#    2,5    $locale        ''        C        \
#    2,5    ''        $locale        C        \
#    2.5    C        ''        $locale        \
#    2.5    ''        C        $locale        \
#
#unset a b c
#unset LC_ALL LC_NUMERIC LANG
#integer a b c
#while (( $# >= 4 ))
#do
#    exp=$1
#    unset H V
#    typeset -A H
#    typeset -a V
#    [[ $2 ]] && V[0]="export LC_ALL=$2;"
#    [[ $3 ]] && V[1]="export LC_NUMERIC=$3;"
#    [[ $4 ]] && V[2]="export LANG=$4;"
#    for ((a = 0; a < 3; a++))
#    do
#        for ((b = 0; b < 3; b++))
#        do
#            if (( b != a ))
#            then
#                for ((c = 0; c < 3; c++))
#                do
#                    if (( c != a && c != b ))
#                    then
#                        T=${V[$a]}${V[$b]}${V[$c]}
#                        if [[ ! ${H[$T]} ]]
#                        then
#                            H[$T]=1
#                            got=$($SHELL -c "${T}print \$(( $exp ))" 2>&1)
#                            [[ $got == $exp ]] || log_error "${T} sequence failed -- expected '$exp', got '$got'"
#                        fi
#                    fi
#                done
#            fi
#        done
#    done
#    shift 4
#done
#
# setocale(LC_ALL,"") after setlocale() initialization

locale=en_US.UTF-8

log_info 'TODO: Enable the `join` test when it is a builtin again (issues #411, #129).'
if false; then
printf 'f1\357\274\240f2\n' > input1
printf 't2\357\274\240f1\n' > input2
printf '\357\274\240\n' > delim
print "export LC_ALL=$locale
join -1 1 -2 2 -o 1.2,2.1 -t \$(cat delim) input1 input2 > out" > script
$SHELL -c 'unset LANG ${!LC_*}; $SHELL ./script' ||
log_error "join test script failed -- exit code $?"
exp=$(printf 'f2\357\274\240t2')
got="$(<out)"
[[ $got == "$exp" ]] || log_error "LC_ALL test script failed -- expected '$exp', got '$got'"
fi  # if false

# ==========
# Multibyte identifiers.
#
exp=OK
got=$(export LC_ALL=en_US.UTF-8; $SHELL -c "$(printf '\u[0101]=OK; print ${\u[0101]}')" 2>&1)
[[ $got == "$exp" ]] || log_error "multibyte variable definition/expansion failed" "$exp" "$got"
got=$(export LC_ALL=en_US.UTF-8; $SHELL -c "$(printf 'function \u[0101]\n{\nprint OK;\n};
\u[0101]')" 2>&1)
[[ $got == "$exp" ]] || log_error "multibyte ksh function definition/execution failed" "$exp" "$got"
got=$(export LC_ALL=en_US.UTF-8; $SHELL -c "$(printf '\u[0101]()\n{\nprint OK;\n}; \u[0101]')" 2>&1)
[[ $got == "$exp" ]] || log_error "multibyte posix function definition/execution failed" "$exp" "$got"
got=$(export LC_ALL=en_US.UTF-8; $SHELL -c "$(printf '\w[0101]=OK; print ${\w[0101]}')" 2>&1)
[[ $got == "$exp" ]] || log_error "multibyte variable definition/expansion failed" "$exp" "$got"
got=$(export LC_ALL=en_US.UTF-8; $SHELL -c "$(printf 'function \w[0101]\n{\nprint OK;\n};
\w[0101]')" 2>&1)
[[ $got == "$exp" ]] || log_error "multibyte ksh function definition/execution failed" "$exp" "$got"
got=$(export LC_ALL=en_US.UTF-8; $SHELL -c "$(printf '\w[0101]()\n{\nprint OK;\n}; \w[0101]')" 2>&1)
[[ $got == "$exp" ]] || log_error "multibyte posix function definition/execution failed" "$exp" "$got"

# Test for multibyte character at buffer boundary.
LC_ALL=C
{
    print 'cat << \\EOF'
    for ((i=1; i < 164; i++))
    do print 123456789+123456789+123456789+123456789+123456789
    done
    print $'multibyte string: \342\202\254XX\342\202\254YY '
    for ((i=1; i < 10; i++))
    do print 123456789+123456789+123456789+123456789+123456789
    done
    print EOF
} > multibyte_script
chmod +x multibyte_script
actual=$(LC_ALL=$locale $SHELL ./multibyte_script)
expect=$(( 163 * 50 + 30 + 9 * 50 - 1 ))
[[ ${#actual} == $expect ]] || log_error 'here doc contains wrong number of chars with multibyte
locale' "$expect" "${#actual}"
expect=$'multibyte string: \342\202\254XX\342\202\254YY '
[[ $actual == *${expect}* ]] || log_error "here_doc
doesn't contain line with multibyte chars" "*${expect}*" "$actual"

actual=$(LC_ALL=$locale $SHELL -c 'x=$'\''a\342\202\254c'\''; print -r -- ${#x}')
expect=3
(( actual == expect  )) || log_error 'character length of multibyte character wrong' "$expect" "$actual"

actual=$(LC_ALL=$locale $SHELL -c 'typeset -R6 x=$'\''a\342\202\254c'\'';print -r -- "${x}"')
expect=$'   a\342\202\254c'
[[ $actual == $expect ]] || log_error 'typeset -R6 should begin with three spaces' "$expect" "$actual"

actual=$(LC_ALL=$locale $SHELL -c 'typeset -L6 x=$'\''a\342\202\254c'\'';print -r -- "${x}"')
expect=$'a\342\202\254c   '
[[ $actual == $expect ]] || log_error 'typeset -L6 should end in three spaces' "$expect" "$actual"

if   $SHELL -c "export LC_ALL=en_US.UTF-8; c=$'\342\202\254'; [[ \${#c} == 1 ]]" 2>/dev/null
then
    LC_ALL=en_US.UTF-8
    unset i p1 p2 x
    for i in 9 b c d 20 1680 2000 2001 2002 2003 2004 2005 2006 2008 2009 200a 2028 2029 3000 # 1803 2007 202f  205f
    do
        # TODO: Remove this when char U+1680 is no longer broken on FreeBSD 11.
        [[ $OS_NAME == freebsd && $i == 1680 ]] && continue
        # TODO: Remove this when char U+1680 is no longer broken on Solaris.
        [[ $OS_NAME == sunos && $i == 1680 ]] && continue

        if ! eval "[[ \$'\\u[$i]' == [[:space:]] ]]"
        then
            x+=,$i
        fi

    done
    if [[ -n "$x" ]]
    then
        if [[ $x == ,*,* ]]
        then
            p1=s p2="are not space characters"
        else
            p1=  p2="is not a space character"
        fi

        log_error "unicode char$p1 ${x#?} $p2 in locale $LC_ALL"
    fi

    unset x
    x=$(export LC_ALL=en_US.UTF-8; printf "hello\u[20ac]\xee world")
    LC_ALL=en_US.UTF-8 eval $'[[ $(print -r -- "$x") == $\'hello\\u[20ac]\\xee world\' ]]' || log_error '%q with unicode and non-unicode not working'
    if [[ $(whence od) ]]
    then
        expected='68 65 6c 6c 6f e2 82 ac ee 20 77 6f 72 6c 64 0a'
        actual=$(print -r -- "$x" | command od -An -tx1 |
                 sed -e 's/^ *//' -e 's/ *$//' -e 's/   */ /g')
        if [[ "$actual" != "$expected" ]]
        then
            log_error $(echo 'incorrect string from printf %q:';
                       echo "expected '$expected'";
                echo "actual   '$actual'")
        fi
    fi
fi

typeset -r utf8_euro_char1=$'\u[20ac]'
typeset -r utf8_euro_char2=$'\342\202\254'
(( (${#utf8_euro_char1} == 1) && (${#utf8_euro_char2} == 1) )) \
        || export LC_ALL='en_US.UTF-8'
[[ "$(printf '\u[20ac]')" == $'\342\202\254' ]]  || log_error 'locales not handled correctly in command substitution'
