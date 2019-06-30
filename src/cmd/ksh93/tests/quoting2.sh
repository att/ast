########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2011 AT&T Intellectual Property          #
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

set -o noglob

if [[ 'hi there' != "hi there" ]]
then
    log_error "single quotes not the same as double quotes"
fi

x='hi there'
if [[ $x != 'hi there' ]]
then
    log_error "$x not the same as 'hi there'"
fi

if [[ $x != "hi there" ]]
then
    log_error "$x not the same as \"hi there \""
fi

if [[ \a\b\c\*\|\"\ \\ != 'abc*|" \' ]]
then
    log_error " \\ differs from '' "
fi

if [[ "ab\'\"\$(" != 'ab\'\''"$(' ]]
then
    log_error " \"\" differs from '' "
fi

if [[ $(print -r - 'abc*|" \') !=  'abc*|" \' ]]
then
    log_error "\$(print -r - '') differs from ''"
fi

if [[ $(print -r - "abc*|\" \\") !=  'abc*|" \' ]]
then
    log_error "\$(print -r - '') differs from ''"
fi

if [[ "$(print -r - 'abc*|" \')" !=  'abc*|" \' ]]
then
    log_error "\"\$(print -r - '')\" differs from ''"
fi

if [[ "$(print -r - "abc*|\" \\")" !=  'abc*|" \' ]]
then
    log_error "\"\$(print -r - "")\" differs from ''"
fi

if [[ $(print -r - "$(print -r - 'abc*|" \')") !=  'abc*|" \' ]]
then
    log_error "nested \$(print -r - '') differs from ''"
fi

if [[ "$(print -r - $(print -r - 'abc*|" \'))" !=  'abc*|" \' ]]
then
    log_error "\"nested \$(print -r - '')\" differs from ''"
fi

if [[ $(print -r - "$(print -r - 'abc*|" \')") !=  'abc*|" \' ]]
then
    log_error "nested \"\$(print -r - '')\" differs from ''"
fi

unset x
if [[ ${x-$(print -r - "abc*|\" \\")} !=  'abc*|" \' ]]
then
    log_error "\${x-\$(print -r - '')} differs from ''"
fi

if [[ ${x-$(print -r - "a}c*|\" \\")} !=  'a}c*|" \' ]]
then
    log_error "\${x-\$(print -r - '}')} differs from ''"
fi

x=$((echo foo)|(cat))
if [[ $x != foo  ]]
then
    log_error "((cmd)|(cmd)) failed"
fi

x=$(print -r -- "\"$HOME\"")
if [[ $x != '"'$HOME'"' ]]
then
    log_error "nested double quotes failed"
fi

unset z
: ${z="a{b}c"}
if [[ $z != 'a{b}c' ]]
then
    log_error '${z="a{b}c"} not correct'
fi

unset z
: "${z="a{b}c"}"
if [[ $z != 'a{b}c' ]]
then
    log_error '"${z="a{b}c"}" not correct'
fi

if [[ $(print -r -- "a\*b") !=  'a\*b' ]]
then
    log_error '$(print -r -- "a\*b") differs from  a\*b'
fi

unset x
if [[ $(print -r -- "a\*b$x") !=  'a\*b' ]]
then
    log_error '$(print -r -- "a\*b$x") differs from  a\*b'
fi

x=hello
set -- ${x+foo bar bam}
if (( $# !=3 ))
then
    log_error '${x+foo bar bam} does not yield three arguments'
fi

set -- ${x+foo "bar bam"}
if (( $# !=2 ))
then
    log_error '${x+foo "bar bam"} does not yield two arguments'
fi

set -- ${x+foo 'bar bam'}
if (( $# !=2 ))
then
    log_error '${x+foo '\''bar bam'\''} does not yield two arguments'
fi

set -- ${x+foo $x bam}
if (( $# !=3 ))
then
    log_error '${x+foo $x bam} does not yield three arguments'
fi

set -- ${x+foo "$x" bam}
if (( $# !=3 ))
then
    log_error '${x+foo "$x" bam} does not yield three arguments'
fi

set -- ${x+"foo $x bam"}
if (( $# !=1 ))
then
    log_error '${x+"foo $x bam"} does not yield one argument'
fi

set -- "${x+foo $x bam}"
if (( $# !=1 ))
then
    log_error '"${x+foo $x bam}" does not yield one argument'
fi

set -- ${x+foo "$x "bam}
if (( $# !=2 ))
then
    log_error '${x+foo "$x "bam} does not yield two arguments'
fi

x="ab$'cd"
if [[ $x != 'ab$'"'cd" ]]
then
    log_error '$'"' inside double quotes not working"
fi

x=`print 'ab$'`
if [[ $x != 'ab$' ]]
then
    log_error '$'"' inside `` quotes not working"
fi

unset a
x=$(print -r -- "'\
\
")
if [[ $x != "'" ]]
then
    log_error 'line continuation in double strings not working'
fi

x=$(print -r -- "'\
$a\
")
if [[ $x != "'" ]]
then
    log_error 'line continuation in expanded double strings not working'
fi

x='\*'
if [[ $(print -r -- $x) != '\*' ]]
then
    log_error 'x="\\*";$x != \*'
fi

if [[ $(print -r -- "\}" ) != '\}' ]]
then
    log_error '(print -r -- "\}"' not working
fi

if [[ $(print -r -- "\{" ) != '\{' ]]
then
    log_error 'print -r -- "\{"' not working
fi

# The following caused a syntax error on earlier versions
foo=foo x=-
if [[  `eval print \\${foo$x}` != foo* ]]
then
    log_error '`eval  print \\${foo$x}`' not working
fi

if [[  "`eval print \\${foo$x}`" != foo* ]]
then
    log_error '"`eval  print \\${foo$x}`"' not working
fi

if ( [[ $() != '' ]] )
then
    log_error '$() not working'
fi

x=a:b:c
set -- $( IFS=:; print $x)
if (( $# != 3))
then
    log_error 'IFS not working correctly with command substitution'
fi

$SHELL -n 2> /dev/null << \! || log_error '$(...) bug with ( in comment'
y=$(
    # ( this line is a bug fix
    print hi
)
!
x=
for j in  glob noglob
do
    for i in 'a\*b' 'a\ b' 'a\bc' 'a\*b' 'a\"b'
    do
        eval [[ '$('print -r -- \'$i\'\$x')' != "'$i'" ]]  && log_error "quoting of $i\$x with $j enabled failed"
        eval [[ '$('print -r -- \'$i\'\${x%*}')' != "'$i'" ]]  && log_error "quoting of $i\${x%*} with $j enabled failed"
        if [[ $j == noglob ]]
        then
            eval [[ '$('print -r -- \'$i\'\${x:-*}')' != "'$i''*'" ]]  && log_error "quoting of $i\${x:-*} with $j enabled failed"
        fi
    done
    set -f
done

foo=foo
[[ "$" == '$' ]] || log_error '"$" != $'
[[ "${foo}$" == 'foo$' ]] || log_error 'foo=foo;"${foo}$" != foo$'
[[ "${foo}${foo}$" == 'foofoo$' ]] || log_error 'foo=foo;"${foo}${foo}$" != foofoo$'
foo='$ '
[[ "$foo" == ~(Elr)(\\\$|#)\  ]] || log_error $'\'$ \' not matching RE \\\\\\$|#\''
[[ "$foo" == ~(Elr)('\$'|#)\  ]] || log_error $'\'$ \' not matching RE \'\\$\'|#\''
foo='# '
[[ "$foo" == ~(Elr)(\\\$|#)\  ]] || log_error $'\'# \' not matching RE \\'\$|#\''
[[ "$foo" == ~(Elr)('\$'|#)\  ]] || log_error $'\'# \' not matching RE \'\\$\'|#\''
[[ '\$' == '\$'* ]] ||   log_error $'\'\\$\' not matching \'\\$\'*'
[[ a+a == ~(E)a\+a ]] || log_error '~(E)a\+a not matching a+a'
[[ a+a =~ a\+a ]] || log_error 'RE a\+a not matching a+a'

exp='ac'
got=$'a\0b'c
[[ $got == "$exp" ]] || log_error "\$'a\\0b'c expansion failed -- expected '$exp', got '$got'"
