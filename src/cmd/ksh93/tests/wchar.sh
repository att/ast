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
#
# Tests for \u[...] and \w[...] input and output
#

locales="en_US.UTF-8 en_US.utf8 en_US.ISO-8859-15 en_US.iso885915"
# On FreeBSD 11.1 the native locale subsystem cannot convert the Euro currency symbol from UTF-8 to
# GB1803. See https://github.com/att/ast/issues/577#issuecomment-396113061.
if [[ $OS_NAME != "FreeBSD" ]]
then
    locales="$locales zh_CN.GB18030 zh_CN.gb18030"
fi
supported="C.UTF-8"
locale -a > locale.txt

for lc_all in $locales
do
    if grep -q $lc_all locale.txt
    then
        supported+=" $lc_all"
        log_info "LC_ALL=$lc_all is supported and will be tested"
    else
        log_info "LC_ALL=$lc_all not supported"
    fi
done

exp0=$'0000000 24 27 e2 82 ac 27 0a'
exp2=$'\'\\u[20ac]\''
exp1='$'$exp2

for lc_all in $supported
do
    actual=$(LC_OPTIONS=nounicodeliterals $SHELL -c 'export LC_ALL='${lc_all}';
        printf "%q\n" "$(printf "\u[20ac]")"' |
        iconv -f ${lc_all#*.} -t UTF-8 | od -tx1 | head -1 |
        sed -e 's/^ *//' -e 's/ *$//' -e 's/   */ /g')
    [[ "$actual" == "$exp0" ]] || log_error "${lc_all} nounicodeliterals FAILED" "$exp0" "$actual"

    actual=$(LC_OPTIONS=unicodeliterals $SHELL -c 'export LC_ALL='${lc_all}';
        printf "%(nounicodeliterals)q\n" "$(printf "\u[20ac]")"' |
        iconv -f ${lc_all#*.} -t UTF-8 | od -tx1 | head -1 |
        sed -e 's/^ *//' -e 's/ *$//' -e 's/   */ /g')
    [[ "$actual" == "$exp0" ]] || log_error "${lc_all} (nounicodeliterals) FAILED" "$exp0" "$actual"

    actual=$(LC_OPTIONS=unicodeliterals $SHELL -c 'export LC_ALL='${lc_all}'; printf "%q\n" "$(printf "\u[20ac]")"')
    # TODO: Figure out why these tests needs to verify the actual value matches either of two
    # expected values.
    [[ "$actual" == "$exp1" || "$actual" == "$exp2" ]] ||
        log_error "${lc_all} unicode FAILED" "$exp1" "$actual"

    actual=$(LC_OPTIONS=nounicodeliterals $SHELL -c 'export LC_ALL='${lc_all}'; printf "%(unicodeliterals)q\n" "$(printf "\u[20ac]")"')
    [[ "$actual" == "$exp1" || $actual == "$exp2" ]] ||
        log_error "${lc_all} (unitcodeliterals) FAILED" "$exp1" "$actual"
done
