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

integer j=4
base=/home/dgk/foo//bar
string1=$base/abcabcabc

if [[ ${string1:0} != "$string1" ]]
then
    log_error "string1:0"
fi

if [[ ${string1: -1} != "c" ]]
then
    log_error "string1: -1"
fi

if [[ ${string1:0:1000} != "$string1" ]]
then
    log_error "string1:0"
fi

if [[ ${string1:1} != "${string1#?}" ]]
then
    log_error "string1:1"
fi

if [[ ${string1:1:4} != home ]]
then
    log_error "string1:1:4"
fi

if [[ ${string1: -5:4} != bcab ]]
then
    log_error "string1: -5:4"
fi

if [[ ${string1:1:j} != home ]]
then
    log_error "string1:1:j"
fi

if [[ ${string1:(j?1:0):j} != home ]]
then
    log_error "string1:(j?1:0):j"
fi

if [[ ${string1%*zzz*} != "$string1" ]]
then
    log_error "string1%*zzz*"
fi

if [[ ${string1%%*zzz*} != "$string1" ]]
then
    log_error "string1%%*zzz*"
fi

if [[ ${string1#*zzz*} != "$string1" ]]
then
    log_error "string1#*zzz*"
fi

if [[ ${string1##*zzz*} != "$string1" ]]
then
    log_error "string1##*zzz*"
fi

if [[ ${string1%+(abc)} != "$base/abcabc" ]]
then
    log_error "string1%+(abc)"
fi

if [[ ${string1%%+(abc)} != "$base/" ]]
then
    log_error "string1%%+(abc)"
fi

if [[ ${string1%/*} != "$base" ]]
then
    log_error "string1%/*"
fi

if [[ "${string1%/*}" != "$base" ]]
then
    log_error '"string1%/*"'
fi

if [[ ${string1%"/*"} != "$string1" ]]
then
    log_error 'string1%"/*"'
fi

if [[ ${string1%%/*} != "" ]]
then
    log_error "string1%%/*"
fi

if [[ ${string1#*/bar} != /abcabcabc ]]
then
    log_error "string1#*bar"
fi

if [[ ${string1##*/bar} != /abcabcabc ]]
then
    log_error "string1#*bar"
fi

if [[ "${string1#@(*/bar|*/foo)}" != //bar/abcabcabc ]]
then
    log_error "string1#@(*/bar|*/foo)"
fi

if [[ ${string1##@(*/bar|*/foo)} != /abcabcabc ]]
then
    log_error "string1##@(*/bar|*/foo)"
fi

if [[ ${string1##*/@(bar|foo)} != /abcabcabc ]]
then
    log_error "string1##*/@(bar|foo)"
fi

foo=abc
if [[ ${foo#a[b*} != abc ]]
then
    log_error "abc#a[b*} != abc"
fi

if [[ ${foo//[0-9]/bar} != abc ]]
then
    log_error '${foo//[0-9]/bar} not expanding correctly'
fi

foo='(abc)'
if [[ ${foo#'('} != 'abc)' ]]
then
    log_error "(abc)#( != abc)"
fi

if [[ ${foo%')'} != '(abc' ]]
then
    log_error "(abc)%) != (abc"
fi

foo=a123b456c
if [[ ${foo/[0-9]?/""} != a3b456c ]]
then
    log_error '${foo/[0-9]?/""} not expanding correctly'
fi

if [[ ${foo//[0-9]/""} != abc ]]
then
    log_error '${foo//[0-9]/""} not expanding correctly'
fi

if [[ ${foo/#a/b} != b123b456c ]]
then
    log_error '${foo/#a/b} not expanding correctly'
fi

if [[ ${foo/#?/b} != b123b456c ]]
then
    log_error '${foo/#?/b} not expanding correctly'
fi

if [[ ${foo/%c/b} != a123b456b ]]
then
    log_error '${foo/%c/b} not expanding correctly'
fi

if [[ ${foo/%?/b} != a123b456b ]]
then
    log_error '${foo/%?/b} not expanding correctly'
fi

while read -r pattern string expected
do
    if (( expected ))
    then
        if [[ $string != $pattern ]]
        then
            log_error "$pattern does not match $string"
        fi

        if [[ ${string##$pattern} != "" ]]
        then
            log_error "\${$string##$pattern} not null"
        fi

        if [[ "${string##$pattern}" != '' ]]
        then
            log_error "\"\${$string##$pattern}\" not null"
        fi

        if [[ ${string/$pattern} != "" ]]
        then
            log_error "\${$string/$pattern} not null"
        fi
    else
        if [[ $string == $pattern ]]
        then
            log_error "$pattern matches $string"
        fi
    fi

done <<- \EOF
    +(a)*+(a)    aabca    1
    !(*.o)        foo.o    0
    !(*.o)        foo.c    1
EOF
xx=a/b/c/d/e
yy=${xx#*/}
if [[ $yy != b/c/d/e ]]
then
    log_error '${xx#*/} != a/b/c/d/e when xx=a/b/c/d/e'
fi

if [[ ${xx//\//\\} != 'a\b\c\d\e' ]]
then
    log_error '${xx//\//\\} not working'
fi

x=[123]def
if [[ "${x//\[@(*)\]/\{\1\}}" != {123}def ]]
then
    log_error 'closing brace escape not working'
fi

xx=%28text%29
if [[ ${xx//%28/abc\)} != 'abc)text%29' ]]
then
     log_error '${xx//%28/abc\)} not working'
fi

xx='a:b'
str='(){}[]*?|&^%$#@l'
for ((i=0 ; i < ${#str}; i++))
do
    [[ $(eval print -r -- \"\${xx//:/\\${str:i:1}}\") == "a${str:i:1}b" ]] || log_error "substitution of \\${str:i:1}} failed"
    [[ $(eval print -rn -- \"\${xx//:/\'${str:i:1}\'}\") == "a${str:i:1}b" ]] || log_error "substitution of '${str:i:1}' failed"
    [[ $(eval print -r -- \"\${xx//:/\"${str:i:1}\"}\") == "a${str:i:1}b" ]] || log_error "substitution of \"${str:i:1}\" failed"
done

[[ ${xx//:/\\n} == 'a\nb' ]]  || log_error "substituion of \\\\n failed"
[[ ${xx//:/'\n'} == 'a\nb' ]] || log_error "substituion of '\\n' failed"
[[ ${xx//:/"\n"} ==  'a\nb' ]] || log_error "substituion of \"\\n\" failed"
[[ ${xx//:/$'\n'} ==  $'a\nb' ]] || log_error "substituion of \$'\\n' failed"
unset foo
foo=one/two/three
if [[ ${foo//'/'/_} != one_two_three ]]
then
    log_error 'single quoting / in replacements failed'
fi

if [[ ${foo//"/"/_} != one_two_three ]]
then
    log_error 'double quoting / in replacements failed'
fi

if [[ ${foo//\//_} != one_two_three ]]
then
    log_error 'escaping / in replacements failed'
fi

function myexport
{
    nameref var=$1
    if (( $# > 1 ))
    then
        export    $1=$2
    fi

    if (( $# > 2 ))
    then
        print $(myexport "$1" "$3" )
        return
    fi

    typeset val
    val=$(export | grep "^$1=")
    print ${val#"$1="}

}
export dgk=base
if [[ $(myexport dgk fun) != fun ]]
then
    log_error 'export inside function not working'
fi

val=$(export | grep "^dgk=")
if [[ ${val#dgk=} != base ]]
then
    log_error 'export not restored after function call'
fi

if [[ $(myexport dgk fun fun2) != fun2 ]]
then
    log_error 'export inside function not working with recursive function'
fi

val=$(export | grep "^dgk=")
if [[ ${val#dgk=} != base ]]
then
    log_error 'export not restored after recursive function call'
fi

if [[ $(dgk=try3 myexport dgk) != try3 ]]
then
    log_error 'name=value not added to export list with function call'
fi

val=$(export | grep "^dgk=")
if [[ ${val#dgk=} != base ]]
then
    log_error 'export not restored name=value function call'
fi

unset zzz
if [[ $(myexport zzz fun) != fun ]]
then
    log_error 'export inside function not working for zzz'
fi

if [[ $(export | grep "zzz=") ]]
then
    log_error 'zzz exported after function call'
fi

set -- foo/bar bam/yes last/file/done
if [[ ${@/*\/@(*)/\1} != 'bar yes done' ]]
then
    log_error '\1 not working with $@'
fi

var=(foo/bar bam/yes last/file/done)
if [[ ${var[@]/*\/@(*)/\1} != 'bar yes done' ]]
then
    log_error '\1 not working with ${var[@]}'
fi

var='abc_d2ef.462abc %%'
if [[ ${var/+(\w)/Q} != 'Q.462abc %%' ]]
then
    log_error '${var/+(\w)/Q} not workding'
fi

if [[ ${var//+(\w)/Q} != 'Q.Q %%' ]]
then
    log_error '${var//+(\w)/Q} not workding'
fi

if [[ ${var//+(\S)/Q} != 'Q Q' ]]
then
    log_error '${var//+(\S)/Q} not workding'
fi

var=$($SHELL -c 'v=/vin:/usr/vin r=vin; : ${v//vin/${r//v/b}};typeset -p .sh.match') 2> /dev/null
[[ $var == 'typeset -a .sh.match=((vin vin) )' ]] || log_error '.sh.match not correct when replacement pattern contains a substring match'
foo='foo+bar+'
[[ $(print -r -- ${foo//+/'|'}) != 'foo|bar|' ]] && log_error "\${foobar//+/'|'}"
[[ $(print -r -- ${foo//+/"|"}) != 'foo|bar|' ]] && log_error '${foobar//+/"|"}'
[[ $(print -r -- "${foo//+/'|'}") != 'foo|bar|' ]] && log_error '"${foobar//+/'"'|'"'}"'
[[ $(print -r -- "${foo//+/"|"}") != 'foo|bar|' ]] && log_error '"${foobar//+/"|"}"'
unset x
x=abcedfg
: ${x%@(d)f@(g)}
[[ ${.sh.match[0]} == dfg ]] || log_error '.sh.match[0] not dfg'
[[ ${.sh.match[1]} == d ]] || log_error '.sh.match[1] not d'
[[ ${.sh.match[2]} == g ]] || log_error '.sh.match[2] not g'
x=abcedddfg
: ${x%%+(d)f@(g)}
[[ ${.sh.match[1]} == ddd ]] || log_error '.sh.match[1] not ddd'
unset a b
a='\[abc @(*) def\]'
b='[abc 123 def]'
[[ ${b//$a/\1} == 123 ]] || log_error "\${var/pattern} not working with \[ in pattern"
unset foo
foo='(win32.i386) '
[[ ${foo/'('/'(x11-'} == '(x11-win32.i386) ' ]] || log_error "\${var/pattern} not working with ' in pattern"
$SHELL -c $'v=\'$(hello)\'; [[ ${v//\'$(\'/-I\'$(\'} == -I"$v" ]]' 2> /dev/null || log_error "\${var/pattern} not working with \$( as pattern"
unset X
$SHELL -c '[[ ! ${X[@]:0:300} ]]' 2> /dev/null || log_error '${X[@]:0:300} with X undefined fails'
$SHELL -c '[[ ${@:0:300} == "$0" ]]' 2> /dev/null || log_error '${@:0:300} with no arguments fails'
i=20030704
[[ ${i#{6}(?)} == 04 ]] ||  log_error '${i#{6}(?)} not working'
[[ ${i#{6,6}(?)} == 04 ]] ||  log_error '${i#{6,6}(?)} not working'
LC_ALL=posix
i="   ."
[[ $(printf "<%s>\n" ${i#' '}) == '<.>' ]] || log_error 'printf "<%s>\n" ${i#' '} failed'
unset x
x=foo
[[ "${x%o}(1)" == "fo(1)" ]] ||  log_error 'print ${}() treated as pattern'
unset i pattern string
string=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghigklmnopqrstuvwxyz
integer i
for((i=0; i < ${#string}; i++))
do
    pattern+='@(?)'
done
[[ $(string=$string $SHELL -c  ": \${string/$pattern/}; print \${.sh.match[26]}") == Z ]] || log_error -u2 'sh.match[26] not Z'
: ${string/$pattern/}
(( ${#.sh.match[@]} == 53 )) || log_error '.sh.match has wrong number of elements'
[[ ${.sh.match[@]:2:4} == 'B C D E'  ]] || log_error '${.sh.match[@]:2:4} incorrect'

D=$';' E=$'\\\\' Q=$'"' S=$'\'' M='nested pattern substitution failed'

x='-(-)-'
[[ ${x/*%(())*/\1} == '(-)' ]] || log_error $M
x='-(-)-)-'
[[ ${x/*%(())*/\1} == '(-)' ]] || log_error $M
x='-(-()-)-'
[[ ${x/*%(())*/\1} == '()' ]] || log_error $M
x='-(-\)-)-'
[[ ${x/*%(())*/\1} == '(-\)' ]] || log_error $M
x='-(-\\)-)-'
[[ ${x/*%(())*/\1} == '(-\\)' ]] || log_error $M
x='-(-(-)-'
[[ ${x/*%(())*/\1} == '(-)' ]] || log_error $M
x='-(-(-)-)-'
[[ ${x/*%(())*/\1} == '(-)' ]] || log_error $M
x='-(-[-]-)-'
[[ ${x/*%(()[])*/\1} == '(-[-]-)' ]] || log_error $M
x='-[-(-)-]-'
[[ ${x/*%(()[])*/\1} == '(-)' ]] || log_error $M
x='-(-[-)-]-'
[[ ${x/*%(()[])*/\1} == '-(-[-)-]-' ]] || log_error $M
x='-(-[-]-)-'
[[ ${x/*%([]())*/\1} == '[-]' ]] || log_error $M
x='-[-(-)-]-'
[[ ${x/*%([]())*/\1} == '[-(-)-]' ]] || log_error $M
x='-(-[-)-]-'
[[ ${x/*%([]())*/\1} == '-(-[-)-]-' ]] || log_error $M

x='-((-))-'
[[ ${x/*%(())*/\1} == '(-)' ]] || log_error $M
x='-((-))-'
[[ ${x/~(-g)*%(())*/\1} == '((-))-' ]] || log_error $M
x='-((-))-'
[[ ${x/~(-g:*)*%(())*/\1} == '(-)' ]] || log_error $M
x='-((-))-'
[[ ${x/~(+g)*%(())*/\1} == '(-)' ]] || log_error $M
x='-((-))-'
[[ ${x/~(+g:*)*%(())*/\1} == '(-)' ]] || log_error $M
x='-((-))-'
[[ ${x/*(?)*%(())*(?)*/:\1:\2:\3:} == ':-(:(-):)-:' ]] || log_error $M
x='-((-))-'
[[ ${x/~(-g)*(?)*%(())*(?)*/:\1:\2:\3:} == '::((-))::-' ]] || log_error $M
x='-((-))-'
[[ ${x/~(-g:*(?))*%(())*(?)*/:\1:\2:\3:} == '::(-):)-:' ]] || log_error $M
x='-((-))-'
[[ ${x/~(+g)*(?)*%(())*(?)*/:\1:\2:\3:} == ':-(:(-):)-:' ]] || log_error $M
x='-((-))-'
[[ ${x/~(+g:*(?))*%(())*(?)*/:\1:\2:\3:} == ':-(:(-):)-:' ]] || log_error $M
x='call(a+b,x/(c/d),(0));'
[[ ${x/+([[:alnum:]])*([[:space:]])@(*%(()))*/:\1:\2:\3:} == ':call::(a+b,x/(c/d),(0)):' ]] || log_error $M

x='-(-;-)-'
[[ ${x/*%(()D${D})*/\1} == '-(-;-)-' ]] || log_error $M
x='-(-);-'
[[ ${x/*%(()D${D})*/\1} == '(-)' ]] || log_error $M
x='-(-)\;-'
[[ ${x/*%(()D${D})*/\1} == '(-)' ]] || log_error $M
x='-(-\;-)-'
[[ ${x/*%(()D${D}E${E})*/\1} == '(-\;-)' ]] || log_error $M
x='-(-)\;-'
[[ ${x/*%(()D${D}E${E})*/\1} == '(-)' ]] || log_error $M
x='-(-(-)\;-)-'
[[ ${x/*%(()D${D}E${E})*/\1} == '(-)' ]] || log_error $M

x='-(-")"-)-'
[[ ${x/*%(()Q${Q})*/\1} == '(-")"-)' ]] || log_error $M
x='-(-\")"-)-'
[[ ${x/*%(()Q${Q})*/\1} == '(-\")"-)' ]] || log_error $M
x='-(-\")\"-)-'
[[ ${x/*%(()Q${Q})*/\1} == '(-\")\"-)' ]] || log_error $M
x=$'-(-\\\'")\\\'-)-'
[[ ${x/*%(()Q${S}Q${Q})*/\1} == $'(-\\\'")\\\'-)' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()Q${S}Q${Q})*/\1} == $'-(-\\\'")"-)-' ]] || log_error $M
x=$'-(-\\\'")"\'-)-'
[[ ${x/*%(()Q${S}Q${Q})*/\1} == $'(-\\\'")"\'-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()Q${S}Q${Q})*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\')\\\'\'-)-'
[[ ${x/*%(()Q${S}Q${Q})*/\1} == $'-(-\')\\\'\'-)-' ]] || log_error $M
x=$'-(-\'")\'-)-'
[[ ${x/*%(()L${S}Q${Q})*/\1} == $'(-\'")\'-)' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()L${S}Q${Q})*/\1} == $'-(-\\\'")"-)-' ]] || log_error $M
x=$'-(-\\\'")"\'-)-'
[[ ${x/*%(()L${S}Q${Q})*/\1} == $'(-\\\'")"\'-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()L${S}Q${Q})*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\')\\\'\'-)-'
[[ ${x/*%(()L${S}Q${Q})*/\1} == $'-(-\')\\\'\'-)-' ]] || log_error $M
x='-(-")"-)-'
[[ ${x/*%(()Q${Q})*/\1} == '(-")"-)' ]] || log_error $M
x='-(-\")"-)-'
[[ ${x/*%(()Q${Q})*/\1} == '(-\")"-)' ]] || log_error $M
x='-(-\")\"-)-'
[[ ${x/*%(()Q${Q})*/\1} == '(-\")\"-)' ]] || log_error $M

x='-(-\)-)-'
[[ ${x/*%(()E${E})*/\1} == '(-\)-)' ]] || log_error $M
x='-(-\\)-)-'
[[ ${x/*%(()E${E})*/\1} == '(-\\)' ]] || log_error $M
x='-(-\")"-)-'
[[ ${x/*%(()E${E}Q${Q})*/\1} == '(-\")' ]] || log_error $M
x='-(-\")\"-)-'
[[ ${x/*%(()E${E}Q${Q})*/\1} == '(-\")' ]] || log_error $M
x=$'-(-\'")"-)-'
[[ ${x/*%(()E${E}Q${S}Q${Q})*/\1} == $'-(-\'")"-)-' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()E${E}Q${S}Q${Q})*/\1} == $'(-\\\'")"-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()E${E}Q${S}Q${Q})*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()E${E}L${S}Q${Q})*/\1} == $'(-\\\'")"-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()E${E}L${S}Q${Q})*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\\"\')\\\'\\"-)-'
[[ ${x/*%(()E${E}L${S}Q${Q})*/\1} == $'(-\\"\')\\\'\\"-)' ]] || log_error $M
x=$'-(-\\"\')\\\'\\"\'-)-'
[[ ${x/*%(()E${E}L${S}Q${Q})*/\1} == $'-(-\\"\')\\\'\\"\'-)-' ]] || log_error $M

x='-(-;-)-'
[[ ${x/*%(()D\;)*/\1} == '-(-;-)-' ]] || log_error $M
x='-(-);-'
[[ ${x/*%(()D\;)*/\1} == '(-)' ]] || log_error $M
x='-(-)\;-'
[[ ${x/*%(()D\;)*/\1} == '(-)' ]] || log_error $M
x='-(-\;-)-'
[[ ${x/*%(()D\;E\\)*/\1} == '(-\;-)' ]] || log_error $M
x='-(-);-'
[[ ${x/*%(()D\;E\\)*/\1} == '(-)' ]] || log_error $M
x='-(-)\;-'
[[ ${x/*%(()D\;E\\)*/\1} == '(-)' ]] || log_error $M
x='-(-(-)\;-)-'
[[ ${x/*%(()D\;E\\)*/\1} == '(-)' ]] || log_error $M

x='-(-")"-)-'
[[ ${x/*%(()Q\")*/\1} == '(-")"-)' ]] || log_error $M
x='-(-\")"-)-'
[[ ${x/*%(()Q\")*/\1} == '(-\")"-)' ]] || log_error $M
x='-(-\")\"-)-'
[[ ${x/*%(()Q\")*/\1} == '(-\")\"-)' ]] || log_error $M
x=$'-(-\\\'")\\\'-)-'
[[ ${x/*%(()Q\'Q\")*/\1} == $'(-\\\'")\\\'-)' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()Q\'Q\")*/\1} == $'-(-\\\'")"-)-' ]] || log_error $M
x=$'-(-\\\'")"\'-)-'
[[ ${x/*%(()Q\'Q\")*/\1} == $'(-\\\'")"\'-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()Q\'Q\")*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\')\\\'\'-)-'
[[ ${x/*%(()Q\'Q\")*/\1} == $'-(-\')\\\'\'-)-' ]] || log_error $M
x=$'-(-\'")\'-)-'
[[ ${x/*%(()L\'Q\")*/\1} == $'(-\'")\'-)' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()L\'Q\")*/\1} == $'-(-\\\'")"-)-' ]] || log_error $M
x=$'-(-\\\'")"\'-)-'
[[ ${x/*%(()L\'Q\")*/\1} == $'(-\\\'")"\'-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()L\'Q\")*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\')\\\'\'-)-'
[[ ${x/*%(()L\'Q\")*/\1} == $'-(-\')\\\'\'-)-' ]] || log_error $M
x='-(-")"-)-'
[[ ${x/*%(()Q\")*/\1} == '(-")"-)' ]] || log_error $M
x='-(-\")"-)-'
[[ ${x/*%(()Q\")*/\1} == '(-\")"-)' ]] || log_error $M
x='-(-\")\"-)-'
[[ ${x/*%(()Q\")*/\1} == '(-\")\"-)' ]] || log_error $M

x='-(-\)-)-'
[[ ${x/*%(()E\\)*/\1} == '(-\)-)' ]] || log_error $M
x='-(-\\)-)-'
[[ ${x/*%(()E\\)*/\1} == '(-\\)' ]] || log_error $M
x='-(-\")"-)-'
[[ ${x/*%(()E\\Q\")*/\1} == '(-\")' ]] || log_error $M
x='-(-\")\"-)-'
[[ ${x/*%(()E\\Q\")*/\1} == '(-\")' ]] || log_error $M
x=$'-(-\'")"-)-'
[[ ${x/*%(()E\\Q\'Q\")*/\1} == $'-(-\'")"-)-' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()E\\Q\'Q\")*/\1} == $'(-\\\'")"-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()E\\Q\'Q\")*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\\\'")"-)-'
[[ ${x/*%(()E\\L\'Q\")*/\1} == $'(-\\\'")"-)' ]] || log_error $M
x=$'-(-\\"\')\'\\"-)-'
[[ ${x/*%(()E\\L\'Q\")*/\1} == $'(-\\"\')\'\\"-)' ]] || log_error $M
x=$'-(-\\"\')\\\'\\"-)-'
[[ ${x/*%(()E\\L\'Q\")*/\1} == $'(-\\"\')\\\'\\"-)' ]] || log_error $M
x=$'-(-\\"\')\\\'\\"\'-)-'
[[ ${x/*%(()E\\L\'Q\")*/\1} == $'-(-\\"\')\\\'\\"\'-)-' ]] || log_error $M

pattern=00
var=100
[[ $( print $(( ${var%%00} )) ) == 1 ]] || log_error "arithmetic with embeddded patterns fails"
[[ $( print $(( ${var%%$pattern} )) ) == 1 ]] || log_error "arithmetic with embeddded pattern variables fails"
if [[ ax == @(a)* ]] && [[ ${.sh.match[1]:0:${#.sh.match[1]}}  != a ]]
then
    log_error '${.sh.match[1]:1:${#.sh.match[1]}} not expanding correctly'
fi

string='foo(d:\nt\box\something)bar'
expected='d:\nt\box\something'
[[ ${string/*\(+([!\)])\)*/\1} == "$expected" ]] || log_error "substring expansion failed '${string/*\(+([!\)])\)*/\1}' returned -- '$expected' expected"

b1=$'\342\202\254\342\202\254\342\202\254\342\202\254w'
b2=$'\342\202\254\342\202\254\342\202\254\342\202\254'
# Gah! On FreeBSD 11 the LANG var, if left set to `C`, affects the handling of multibyte chars even
# though LC_ALL is set. So unset LANG.
actual=$($SHELL -c "unset LANG; export LC_ALL=en_US.UTF-8; print -r '$b1$b2' | wc -m")
expect="10"
[[ $actual -eq $expect ]] || log_error "char count incorrect" "$expect" "$actual"

actual=$(LC_ALL=en_US.UTF-8 $SHELL -c "b='$b1$b2'; print \"\${b:4:1}\"")
expect="w"
[[ "$actual" == "$expect" ]] || \
    log_error 'multibyte ${b:4:1} not working correctly' "$expect" "$actual"

{ $SHELL -c 'unset x;[[ ${SHELL:$x} == $SHELL ]]';} 2> /dev/null || log_error '${var:$x} fails when x is not set'
{ $SHELL -c 'x=;[[ ${SHELL:$x} == $SHELL ]]';} 2> /dev/null || log_error '${var:$x} fails when x is null'

#    subject        mode    pattern            result    #
set --                            \
    'a$z'        'E'    '[$]|#'        'a($)z'    \
    'a#z'        'E'    '[$]|#'        'a(#)z'    \
    'a$z'        'Elr'    '[$]|#'        'a$z'    \
    'a#z'        'Elr'    '[$]|#'        'a#z'    \
    'a$'        'E'    '[$]|#'        'a($)'    \
    'a#'        'E'    '[$]|#'        'a(#)'    \
    'a$'        'Elr'    '[$]|#'        'a$'    \
    'a#'        'Elr'    '[$]|#'        'a#'    \
    '$z'        'E'    '[$]|#'        '($)z'    \
    '#z'        'E'    '[$]|#'        '(#)z'    \
    '$z'        'Elr'    '[$]|#'        '$z'    \
    '#z'        'Elr'    '[$]|#'        '#z'    \
    '$'        'E'    '[$]|#'        '($)'    \
    '#'        'E'    '[$]|#'        '(#)'    \
    '$'        'Elr'    '[$]|#'        '($)'    \
    '#'        'Elr'    '[$]|#'        '(#)'    \
    'a$z'        'E'    '\$|#'        'a$z()'    \
    'a$z'        'E'    '\\$|#'        'a$z'    \
    'a$z'        'E'    '\\\$|#'    'a($)z'    \
    'a#z'        'E'    '\\\$|#'    'a(#)z'    \
    'a$z'        'Elr'    '\\\$|#'    'a$z'    \
    'a#z'        'Elr'    '\\\$|#'    'a#z'    \
    'a$'        'E'    '\\\$|#'    'a($)'    \
    'a#'        'E'    '\\\$|#'    'a(#)'    \
    'a$'        'Elr'    '\\\$|#'    'a$'    \
    'a#'        'Elr'    '\\\$|#'    'a#'    \
    '$z'        'E'    '\\\$|#'    '($)z'    \
    '#z'        'E'    '\\\$|#'    '(#)z'    \
    '$z'        'Elr'    '\\\$|#'    '$z'    \
    '#z'        'Elr'    '\\\$|#'    '#z'    \
    '$'        'E'    '\\\$|#'    '($)'    \
    '#'        'E'    '\\\$|#'    '(#)'    \
    '$'        'Elr'    '\\\$|#'    '($)'    \
    '#'        'Elr'    '\\\$|#'    '(#)'    \
#    do not delete this line            #
unset i o
while    (( $# >= 4 ))
do
    i=$1
    eval o="\${i/~($2)$3/\\(\\0\\)}"
    if [[ "$o" != "$4" ]]
    then
        log_error "i='$1'; \${i/~($2)$3/\\(\\0\\)} failed -- expected '$4', got '$o'"
    fi

    eval o="\${i/~($2)($3)/\\(\\1\\)}"
    if [[ "$o" != "$4" ]]
    then
        log_error "i='$1'; \${i/~($2)($3)/\\(\\1\\)} failed -- expected '$4', got '$o'"
    fi

    shift 4
done

# Multibyte char substring tests.
export LC_ALL=en_US.UTF-8
wc1=$'\342\202\254'
wc2=$'\342\202\255'

expect="a"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x:0:1}"')
[[ "$actual" == "$expect" ]] || log_error '${x:0:1} wrong' "$expect" "$actual"

expect="$wc1"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x:1:1}"')
[[ "$actual" == "$expect" ]] || log_error '${x:1:1} wrong' "$expect" "$actual"

expect="$wc2"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x:3:1}"')
[[ "$actual" == "$expect" ]] || log_error '${x:3:1} wrong' "$expect" "$actual"

expect="e"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x:4:1}"')
[[ "$actual" == "$expect" ]] || log_error '${x:4:1} wrong' "$expect" "$actual"

expect="${wc1}c${wc2}e"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x:1}"')
[[ "$actual" == "$expect" ]] || log_error '${x:1} wrong' "$expect" "$actual"

expect="e"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x: -1:1}"')
[[ "$actual" == "$expect" ]] || log_error '${x: -1:1} wrong' "$expect" "$actual"

expect="$wc2"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x: -2:1}"')
[[ "$actual" == "$expect" ]] || log_error '${x: -2:1} wrong' "$expect" "$actual"

expect="${wc1}c${wc2}"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x:1:3}"')
[[ "$actual" == "$expect" ]] || log_error '${x:1:3} wrong' "$expect" "$actual"

expect="${wc1}c${wc2}e"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x:1:20}"')
[[ "$actual" == "$expect" ]] || log_error '${x:1:20} wrong' "$expect" "$actual"

expect="c${wc2}e"
actual=$(x="a${wc1}c${wc2}e" $SHELL -c 'print "${x#??}"')
[[ "$actual" == "$expect" ]] || log_error '${x#??} wrong' "$expect" "$actual"

unset LC_ALL

x='a one and a two'
[[ "${x//~(E)\<.\>/}" == ' one and  two' ]]  || log_error "\< and \> not working in with ere's"

{
$SHELL -c 'typeset x="123" ; integer i=100 ; print -n "${x:i:5}"'
} 2> /dev/null || log_error '${x:i:j} fails when i > strlen(x)'

got=$($SHELL -c 'A=""; B="B"; for I in ${A[@]} ${B[@]}; do echo "\"$I\""; done')
[[ $got == $'"B"' ]] || log_error '"\"$I\"" fails when $I is empty string'

A='|'
[[ $A == $A ]] || log_error 'With A="|",  [[ $A == $A ]] does not match'

x="111 222 333 444 555 666"
[[ $x == ~(E)(...).(...).(...) ]]
[[ -v .sh.match[0] ]] ||   log_error '[[ -v .sh.match[0] ]] should be true'
[[ -v .sh.match[3] ]] ||   log_error '[[ -v .sh.match[3] ]] should be true'
[[ -v .sh.match[4] ]] &&   log_error '[[ -v .sh.match[4] ]] should be false'
[[ ${#.sh.match[@]} == 4 ]] || log_error "\${#.sh.match[@]} should be 4, not ${#.sh.match[@]}"

x="foo bar"
dummy=${x/~(E)(*)/}
[[ ${ print -v .sh.match;} ]] && log_error 'print -v should show .sh.match empty when there are no matches'

if $SHELL -c 'set 1 2 3 4 5 6 7 8 9 10 11 12; : ${##[0-9]}' 2>/dev/null
then
    set 1 2 3 4 5 6 7 8 9 10 11 12
    [[ ${##[0-9]} == 2 ]] || log_error '${##[0-9]} should be 2 with $#==12'
    [[ ${###[0-9]} == 2 ]] || log_error '${###[0-9]} should be 2 with $#==12'
    [[ ${#%[0-9]} == 1 ]] || log_error '${#%[0-9]} should be 1 with $#==12'
    [[ ${#%%[0-9]} == 1 ]] || log_error '${#%%[0-9]} should be 1 with $#==12'
else
    log_error '${##[0-9]} give syntax error'
fi

{
  $SHELL -c 'x="a123 456 789z"; : ${x//{3}(\d)/ }' &
  sleep .5; kill $!; wait $!
} 2> /dev/null || log_error $'tokenizer can\'t handle ${var op {..} }'

function foo
{
    typeset x="123 456 789 abc"
    typeset dummy="${x/~(E-g)([[:digit:]][[:digit:]])((X)|([[:digit:]]))([[:blank:]])/_}"
    exp=$'(\n\t[0]=\'123 \'\n\t[1]=12\n\t[2]=3\n\t[4]=3\n\t[5]=\' \'\n)'
    [[ $(print -v .sh.match) == "$exp" ]] || log_error '.sh.match not correct with alternations'
}
foo

x="a 1 b"
d=${x/~(E)(([[:digit:]])[[:space:]]*|([[:alpha:]]))/X}
[[ $(print -v .sh.match) == $'(\n\t[0]=a\n\t[1]=a\n\t[3]=a\n)' ]] || log_error '.sh.match not sparse'

unset v
typeset -a arr=( 0 1 2 3 4 )
for v in "${arr[@]:5}"
do
    log_error "\${arr[@]:5} should not generate $v"
    break
done

for v in "${arr[@]:1:0}"
do
    log_error "\${arr[@]:1:0} should not generate ${v:-empty_string}"
    break
done

for v in "${arr[@]:0:-1}"
do
    log_error "\${arr[@]:0:-1} should not generate ${v:-empty_string}"
    break
done

set 1 2 3 4
for v in "${@:5}"
do
    log_error "\${@:5} should not generate $v"
    break
done

for v in "${@:1:0}"
do
    log_error "\${@:1:0} should not generate ${v:-empty_string}"
    break
done

for v in "${@:0:-1}"
do
    log_error "\${@:0:-1} should not generate ${v:-empty_string}"
    break
done

unset v d
v=abbbc
d="${v/~(E)b{2,4}/dummy}"
[[ ${.sh.match} == bbb ]] || log_error '.sh.match wrong after ${s/~(E)b{2,4}/dummy}'
[[ $d == adummyc ]] || log_error '${s/~(E)b{2,4}/dummy} not working'

x=1234
: "${x//~(X)([012])|([345])/}"
[[ ${.sh.match[1][600..602]} ]] && log_error '${.sh.match[0][600:602]} is not the emptry string'

: "${x//~(X)([012])|([345])/}"
x=$(print -v .sh.match)
compound co
typeset -m co.array=.sh.match
[[ $x == "$(print -v co.array)" ]] || log_error 'typeset -m for .sh.match to compound variable not working'
#: "${x//~(X)([345])|([012])/}"
[[ $x == "$(print -v co.array)" ]] || log_error 'typeset -m for .sh.match to compound variable not working2'
