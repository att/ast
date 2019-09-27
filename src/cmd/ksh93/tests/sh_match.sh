########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2012 AT&T Intellectual Property          #
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
########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                    Copyright (c) 2012 Roland Mainz                   #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#                                                                      #
#                Roland Mainz <roland.mainz@nrubsig.org>               #
#                                                                      #
########################################################################

#
# Copyright (c) 2012, Roland Mainz. All rights reserved.
#

#
# Test module for ".sh.match"
#

#
# This test module tests the .sh.match pattern matching facility
#

# Start with basic character class matching tests. This is primarily to verify that the underlying
# AST regex code is working as expected before moving on to more complex tests.
#
# TODO: Add richer tests. Especially tests of non-ASCII characters.
[[ 1 =~ [[:digit:]] ]] || log_error 'pattern [[:digit:]] broken'
[[ x =~ [[:digit:]] ]] && log_error 'pattern [[:digit:]] broken'
[[ 5 =~ [[:alpha:]] ]] && log_error 'pattern [[:alpha:]] broken'
[[ z =~ [[:alpha:]] ]] || log_error 'pattern [[:alpha:]] broken'
[[ 3 =~ [[:alnum:]] ]] || log_error 'pattern [[:alnum:]] broken'
[[ y =~ [[:alnum:]] ]] || log_error 'pattern [[:alnum:]] broken'
[[ / =~ [[:alnum:]] ]] && log_error 'pattern [[:alnum:]] broken'
[[ 3 =~ [[:lower:]] ]] && log_error 'pattern [[:lower:]] broken'
[[ y =~ [[:lower:]] ]] || log_error 'pattern [[:lower:]] broken'
[[ B =~ [[:lower:]] ]] && log_error 'pattern [[:lower:]] broken'
[[ 3 =~ [[:upper:]] ]] && log_error 'pattern [[:upper:]] broken'
[[ y =~ [[:upper:]] ]] && log_error 'pattern [[:upper:]] broken'
[[ B =~ [[:upper:]] ]] || log_error 'pattern [[:upper:]] broken'
[[ 7 =~ [[:word:]] ]] || log_error 'pattern [[:word:]] broken'
[[ x =~ [[:word:]] ]] || log_error 'pattern [[:word:]] broken'
[[ _ =~ [[:word:]] ]] || log_error 'pattern [[:word:]] broken'
[[ + =~ [[:word:]] ]] && log_error 'pattern [[:word:]] broken'
[[ . =~ [[:space:]] ]] && log_error 'pattern [[:space:]] broken'
[[ X =~ [[:space:]] ]] && log_error 'pattern [[:space:]] broken'
[[ ' ' =~ [[:space:]] ]] || log_error 'pattern [[:space:]] broken'
[[ $'\t' =~ [[:space:]] ]] || log_error 'pattern [[:space:]] broken'
[[ $'\v' =~ [[:space:]] ]] || log_error 'pattern [[:space:]] broken'
[[ $'\f' =~ [[:space:]] ]] || log_error 'pattern [[:space:]] broken'
[[ $'\n' =~ [[:space:]] ]] || log_error 'pattern [[:space:]] broken'
[[ . =~ [[:blank:]] ]] && log_error 'pattern [[:blank:]] broken'
[[ X =~ [[:blank:]] ]] && log_error 'pattern [[:blank:]] broken'
[[ ' ' =~ [[:blank:]] ]] || log_error 'pattern [[:blank:]] broken'
[[ $'\t' =~ [[:blank:]] ]] || log_error 'pattern [[:blank:]] broken'
if [[ $OS_NAME == openbsd ]]
then
    log_warning 'skipping two [[:blank:]] tests because isblank() is broken on openbsd'
else
    [[ $'\v' =~ [[:blank:]] ]] && log_error 'pattern [[:blank:]] broken'
    [[ $'\f' =~ [[:blank:]] ]] && log_error 'pattern [[:blank:]] broken'
fi
[[ $'\n' =~ [[:blank:]] ]] && log_error 'pattern [[:blank:]] broken'
[[ Z =~ [[:print:]] ]] || log_error 'pattern [[:print:]] broken'
[[ ' ' =~ [[:print:]] ]] || log_error 'pattern [[:print:]] broken'
[[ $'\cg' =~ [[:print:]] ]] && log_error 'pattern [[:print:]] broken'
[[ Z =~ [[:cntrl:]] ]] && log_error 'pattern [[:cntrl:]] broken'
[[ ' ' =~ [[:cntrl:]] ]] && log_error 'pattern [[:cntrl:]] broken'
[[ $'\cg' =~ [[:cntrl:]] ]] || log_error 'pattern [[:cntrl:]] broken'
[[ \$ =~ [[:graph:]] ]] || log_error 'pattern [[:graph:]] broken'
[[ ' ' =~ [[:graph:]] ]] && log_error 'pattern [[:graph:]] broken'
[[ \$ =~ [[:punct:]] ]] || log_error 'pattern [[:punct:]] broken'
[[ / =~ [[:punct:]] ]] || log_error 'pattern [[:punct:]] broken'
[[ ' ' =~ [[:punct:]] ]] && log_error 'pattern [[:punct:]] broken'
[[ x =~ [[:punct:]] ]] && log_error 'pattern [[:punct:]] broken'
[[ ' ' =~ [[:xdigit:]] ]] && log_error 'pattern [[:xdigit:]] broken'
[[ x =~ [[:xdigit:]] ]] && log_error 'pattern [[:xdigit:]] broken'
[[ 0 =~ [[:xdigit:]] ]] || log_error 'pattern [[:xdigit:]] broken'
[[ 9 =~ [[:xdigit:]] ]] || log_error 'pattern [[:xdigit:]] broken'
[[ A =~ [[:xdigit:]] ]] || log_error 'pattern [[:xdigit:]] broken'
[[ a =~ [[:xdigit:]] ]] || log_error 'pattern [[:xdigit:]] broken'
[[ F =~ [[:xdigit:]] ]] || log_error 'pattern [[:xdigit:]] broken'
[[ f =~ [[:xdigit:]] ]] || log_error 'pattern [[:xdigit:]] broken'
[[ G =~ [[:xdigit:]] ]] && log_error 'pattern [[:xdigit:]] broken'
[[ g =~ [[:xdigit:]] ]] && log_error 'pattern [[:xdigit:]] broken'

# ========
# Verify an invalid char class name is handled without a SIGSEGV or similar failure.
# Regression for https://github.com/att/ast/issues/1409.
actual=$(case x in [x[:bogus:]]) echo x ;; esac)
expect=''
[[ $actual == $expect ]] || log_error 'invalid char class name' "$expect" "$actual"

# ========
[[ 3 =~ \w ]] || log_error 'pattern \w broken'
[[ y =~ \w ]] || log_error 'pattern \w broken'
[[ / =~ \w ]] && log_error 'pattern \w broken'
[[ 3 =~ \W ]] && log_error 'pattern \w broken'
[[ y =~ \W ]] && log_error 'pattern \w broken'
[[ / =~ \W ]] || log_error 'pattern \w broken'
[[ . =~ \s ]] && log_error 'pattern \s broken'
[[ X =~ \s ]] && log_error 'pattern \s broken'
[[ ' ' =~ \s ]] || log_error 'pattern \s broken'
[[ $'\t' =~ \s ]] || log_error 'pattern \s broken'
[[ $'\v' =~ \s ]] || log_error 'pattern \s broken'
[[ $'\f' =~ \s ]] || log_error 'pattern \s broken'
[[ $'\n' =~ \s ]] || log_error 'pattern \s broken'
[[ x =~ \d ]] && log_error 'pattern \d broken'
[[ 9 =~ \d ]] || log_error 'pattern \d broken'
[[ x =~ \D ]] || log_error 'pattern \D broken'
[[ 9 =~ \D ]] && log_error 'pattern \D broken'
[[ 7 =~ \b ]] || log_error 'pattern \b broken'
[[ x =~ \b ]] || log_error 'pattern \b broken'
[[ _ =~ \b ]] || log_error 'pattern \b broken'
[[ + =~ \b ]] || log_error 'pattern \b broken'
[[ 'x y ' =~ .\b.\b ]] || log_error 'pattern \b broken'
[[ ' xy ' =~ .\b.\b ]] && log_error 'pattern \b broken'
[[ 7 =~ \B ]] && log_error 'pattern \B broken'
[[ x =~ \B ]] && log_error 'pattern \B broken'
[[ _ =~ \B ]] && log_error 'pattern \B broken'
[[ + =~ \B ]] || log_error 'pattern \B broken'

# tests
function test_xmlfragment1
{
    typeset -r testscript='test1_script.sh'

    cp $TEST_ROOT/data/sh_match_test.sh $testscript
    cp $TEST_ROOT/data/sh_match1.xml testfile1.xml
    cp $TEST_ROOT/data/sh_match2.xml testfile2.xml

    compound -r -a tests=(
        (
            file='testfile1.xml'
            expected_output=$'12824 characters to process...\n#input and output OK (12825 characters).'
        )
        (
            file='testfile2.xml'
            expected_output=$'201 characters to process...\n#input and output OK (202 characters).'
        )
    )
    compound out=( typeset stdout stderr ; integer res )
    integer i
    typeset expected_output
    typeset testname

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        nameref tst=tests[i]
        testname="${0}/${i}/${tst.file}"
        expected_output="${tst.expected_output}"

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset "${testscript}" "${tst.file}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

        [[ "${out.stdout}" == "${expected_output}" ]] ||
            log_error "${testname}" "${ printf '%q\n' "${expected_output}" ; }" \
                                    "${ printf '%q\n' "${out.stdout}" ; }"
        [[ "${out.stderr}" == '' ]] ||
            log_error "${testname}" "" "${ printf '%q\n' "${out.stderr}" ; }"
        (( out.res == 0 )) || log_error "${testname}" "0" "${out.res}"
    done

    return 0
}

# test whether the [[ -v .sh.match[x][y] ]] operator works, try1
function test_testop_v1
{
    compound out=( typeset stdout stderr ; integer res )
    integer i
    typeset testname
    typeset expected_output

    compound -r -a tests=(
        (
            cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}" ;                   [[ -v .sh.match[2][3] ]] || print "OK"'
            expected_output='OK'
        )
        (
            cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}" ; integer i=2 j=3 ; [[ -v .sh.match[$i][$j] ]] || print "OK"'
            expected_output='OK'
        )
        (
            cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}" ; integer i=2 j=3 ; [[ -v .sh.match[i][j] ]] || print "OK"'
            expected_output='OK'
        )
    )

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        nameref tst=tests[i]
        testname="${0}/${i}/${tst.cmd}"
        expected_output="${tst.expected_output}"

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${tst.cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

        [[ "${out.stdout}" == "${expected_output}" ]] || log_error "${testname}: Expected stdout==${ printf '%q\n' "${expected_output}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
        [[ "${out.stderr}" == '' ]] || log_error "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
        (( out.res == 0 )) || log_error "${testname}: Unexpected exit code ${out.res}"
    done

    return 0
}

# test whether the [[ -v .sh.match[x][y] ]] operator works, try2
function test_testop_v2
{
    compound out=( typeset stdout stderr ; integer res )
    integer i
    integer j
    integer j
    typeset testname
    typeset cmd

    compound -r -a tests=(
        (
            cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}"'
            integer y=6
            expected_output_1d=$'[0]\n[1]\n[2]'
            expected_output_2d=$'[0][0]\n[0][1]\n[0][2]\n[0][3]\n[0][4]\n[1][0]\n[1][1]\n[1][3]\n[2][2]\n[2][4]'
        )
        # FIXME: Add more hideous horror tests here
    )

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        nameref tst=tests[i]

        #
        # test first dimension, by plain number
        #
        cmd="${tst.cmd}"
        for (( j=0 ; j < tst.y ; j++ )) ; do
            cmd+="; $( printf "[[ -v .sh.match[%d] ]] && print '[%d]'\n" j j )"
        done
        cmd+='; true'

        testname="${0}/${i}/plain_number_index_1d/${cmd}"

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

        [[ "${out.stdout}" == "${tst.expected_output_1d}" ]] || log_error "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_1d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
        [[ "${out.stderr}" == '' ]] || log_error "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
        (( out.res == 0 )) || log_error "${testname}: Unexpected exit code ${out.res}"


        #
        # test second dimension, by plain number
        #
        cmd="${tst.cmd}"
        for (( j=0 ; j < tst.y ; j++ )) ; do
            for (( k=0 ; k < tst.y ; k++ )) ; do
                cmd+="; $( printf "[[ -v .sh.match[%d][%d] ]] && print '[%d][%d]'\n" j k j k )"
            done
        done
        cmd+='; true'

        testname="${0}/${i}/plain_number_index_2d/${cmd}"

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

        [[ "${out.stdout}" == "${tst.expected_output_2d}" ]] || log_error "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_2d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
        [[ "${out.stderr}" == '' ]] || log_error "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
        (( out.res == 0 )) || log_error "${testname}: Unexpected exit code ${out.res}"

        #
        # test first dimension, by variable index
        #
        cmd="${tst.cmd} ; integer i"
        for (( j=0 ; j < tst.y ; j++ )) ; do
            cmd+="; $( printf "(( i=%d )) ; [[ -v .sh.match[i] ]] && print '[%d]'\n" j j )"
        done
        cmd+='; true'

        testname="${0}/${i}/variable_index_1d/${cmd}"

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

        [[ "${out.stdout}" == "${tst.expected_output_1d}" ]] || log_error "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_1d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
        [[ "${out.stderr}" == '' ]] || log_error "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
        (( out.res == 0 )) || log_error "${testname}: Unexpected exit code ${out.res}"


        #
        # test second dimension, by variable index
        #
        cmd="${tst.cmd} ; integer i j"
        for (( j=0 ; j < tst.y ; j++ )) ; do
            for (( k=0 ; k < tst.y ; k++ )) ; do
                cmd+="; $( printf "(( i=%d , j=%d )) ; [[ -v .sh.match[i][j] ]] && print '[%d][%d]'\n" j k j k )"
            done
        done
        cmd+='; true'

        testname="${0}/${i}/variable_index_2d/${cmd}"

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

        [[ "${out.stdout}" == "${tst.expected_output_2d}" ]] || log_error "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_2d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
        [[ "${out.stderr}" == '' ]] || log_error "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
        (( out.res == 0 )) || log_error "${testname}: Unexpected exit code ${out.res}"

    done

    return 0
}

# test whether ${#.sh.match[0][@]} returns the right number of elements
function test_num_elements1
{
    compound out=( typeset stdout stderr ; integer res )
    integer i
    typeset testname
    typeset expected_output

    compound -r -a tests=(
        (
            cmd='s="a1a2a3" ; d="${s//~(E)([[:alpha:]])|([[:digit:]])/dummy}" ; printf "num=%d\n" "${#.sh.match[0][@]}"'
            expected_output='num=6'
        )
        (
            cmd='s="ababab" ; d="${s//~(E)([[:alpha:]])|([[:digit:]])/dummy}" ; printf "num=%d\n" "${#.sh.match[0][@]}"'
            expected_output='num=6'
        )
        (
            cmd='s="123456" ; d="${s//~(E)([[:alpha:]])|([[:digit:]])/dummy}" ; printf "num=%d\n" "${#.sh.match[0][@]}"'
            expected_output='num=6'
        )
    )

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        nameref tst=tests[i]
        testname="${0}/${i}/${tst.cmd}"
        expected_output="${tst.expected_output}"

        out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${tst.cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

        [[ "${out.stdout}" == "${expected_output}" ]] || log_error "${testname}: Expected stdout==${ printf '%q\n' "${expected_output}" ; }, got ${ printf '%q\n' "${out.stdout}" ; }"
        [[ "${out.stderr}" == '' ]] || log_error "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
        (( out.res == 0 )) || log_error "${testname}: Unexpected exit code ${out.res}"
    done

    return 0
}

function test_nomatch
{
    integer j k
    compound c
    compound -a c.attrs

    attrdata=$' x=\'1\' y=\'2\' z="3" end="world"'
    dummy="${attrdata//~(Ex-p)(?:
        [[:space:]]+
        ( # four different types of name=value syntax
            (?:([:_[:alnum:]-]+)=([^\"\'[:space:]]+?))|    #x='foo=bar huz=123'
            (?:([:_[:alnum:]-]+)=\"([^\"]*?)\")|        #x='foo="ba=r o" huz=123'
            (?:([:_[:alnum:]-]+)=\'([^\']*?)\')|        #x="foox huz=123"
            (?:([:_[:alnum:]-]+))                #x="foox huz=123"
        )
        )/D}"
    for (( j=0 ; j < ${#.sh.match[0][@]} ; j++ ))
    do
        if [[ -v .sh.match[2][j] && -v .sh.match[3][j] ]]
        then
            c.attrs+=( name="${.sh.match[2][j]}" value="${.sh.match[3][j]}" )
        fi

        if [[ -v .sh.match[4][j] && -v .sh.match[5][j] ]]
        then
            c.attrs+=( name="${.sh.match[4][j]}" value="${.sh.match[5][j]}" )
        fi

        if [[ -v .sh.match[6][j] && -v .sh.match[7][j] ]] ; then
            c.attrs+=( name="${.sh.match[6][j]}" value="${.sh.match[7][j]}" )
        fi
    done

    expect='(
	typeset -a attrs=(
		[0]=(
			name=x
			value=1
		)
		[1]=(
			name=y
			value=2
		)
		[2]=(
			name=z
			value=3
		)
		[3]=(
			name=end
			value=world
		)
	)
)'
    got=$(print -v c)
    [[ $got == "$expect" ]] || log_error 'unset submatches not handled correctly'
}

# ========
# This is a regexp to extract pattern to pattern entries from related API unit test source. It emits
# the patterns in a form this script can easily use; i.e., the two patterns separated by a tab.
# See the next two test functions.
p2p_sep=$'\t'
p2p_re=$'/ tests\\[\\] =/,/NULL, NULL/s/^.*{"\\(.*\\)", "\\(.*\\)"},$/\\1\t\\2/p'
# Generate a list of files from the source tree of the project. Then generate a file containing some
# text from those files.
find "$SRC_ROOT/src" -type f > glob2ere.files
: > glob2ere.txt
while read file
do
    sort --random-sort $file | head -5 >> glob2ere.txt
done < glob2ere.files
# These patterns are known not to be correctly converted to a glob. We still test them but only
# issue a warning rather than failing the test. See https://github.com/att/ast/issues/1367.
typeset -A ere2glob_blacklist
ere2glob_blacklist['^x\!y$']=1
ere2glob_blacklist['^x|y$']=1
ere2glob_blacklist['^x\a|b$']=1
# These patterns are known not to be correctly converted to a ere. We still test them but only
# issue a warning rather than failing the test. See https://github.com/att/ast/issues/1367.
typeset -A glob2ere_blacklist
glob2ere_blacklist['x!y']=1
glob2ere_blacklist['x|y']=1
glob2ere_blacklist['~(E)z.?a']=1
glob2ere_blacklist['x\a|b']=1

# ========
# This is related to unit test API/string/fmtmatch. The difference is this uses ksh to convert the
# pattern then uses ksh to verify both patterns match the same lines.
function test_ere2glob {
    # Extract the patterns from the API test and verify the equivalence of the two forms.
    IFS="$p2p_sep"
    integer i=0
    sed -ne "$p2p_re" "$SRC_ROOT/src/lib/libast/tests/string/fmtmatch.c" |
        while read ere glob
        do
            let i=i+1
            # log_info "pattern #$i ere=|$ere|  glob=|$glob|"
            actual=$(print -f '%P' -- "$ere")
            [[ "$actual" == "$glob" ]] ||
                log_error "pattern #$i converting ere |$ere| to glob" "$glob" "$actual"

            : > ere2glob.glob
            : > ere2glob.ere
            while read line
            do
                [[ "$line" = $glob ]] && print -- "$line" >> ere2glob.glob
                [[ "$line" =~ $ere ]] && print -- "$line" >> ere2glob.ere
            done < glob2ere.txt

            if ! cmp -s ere2glob.glob ere2glob.ere
            then
                if [[ ${ere2glob_blacklist["$ere"]} == 1 ]]
                then
                    log_warning "pattern #$i ere |$ere| and glob |$glob| produce diff output"
                else
                    mv ere2glob.glob ere2glob.glob.$i
                    mv ere2glob.ere ere2glob.ere.$i
                    log_error "pattern #$i ere |$ere| and glob |$glob| produce diff output" \
                        "see content of ere2glob.glob.$i" "see content of ere2glob.ere.$i"
                fi
            fi
        done
}

# ========
# This is related to unit test API/string/fmtre. The difference is this uses ksh to convert the
# pattern then uses ksh to verify both patterns match the same lines.
function test_glob2ere {
    # Extract the patterns from the API test and verify the equivalence of the two forms.
    IFS="$p2p_sep"
    integer i=0
    sed -ne "$p2p_re" "$SRC_ROOT/src/lib/libast/tests/string/fmtre.c" |
        while read glob ere
        do
            let i=i+1
            # log_info "pattern #$i ere=|$ere|  glob=|$glob|"
            actual=$(print -f '%R' -- "$glob")
            [[ "$actual" == "$ere" ]] ||
                log_error "pattern #$i converting glob |$glob| to ere" "$ere" "$actual"

            : > glob2ere.glob
            : > glob2ere.ere
            while read line
            do
                [[ "$line" = $glob ]] && print -- "$line" >> glob2ere.glob
                [[ "$line" =~ $ere ]] && print -- "$line" >> glob2ere.ere
            done < glob2ere.txt

            if ! cmp -s glob2ere.glob glob2ere.ere
            then
                if [[ ${glob2ere_blacklist["$glob"]} == 1 ]]
                then
                    log_warning "pattern #$i ere |$ere| and glob |$glob| produce diff output"
                else
                    mv glob2ere.glob glob2ere.glob.$i
                    mv glob2ere.ere glob2ere.ere.$i
                    log_error "pattern #$i ere |$ere| and glob |$glob| produce diff output" \
                        "see content of glob2ere.glob.$i" "see content of glob2ere.ere.$i"
                fi
            fi
        done
}

# ========
# Run the tests.
test_xmlfragment1
test_testop_v1
test_testop_v2
test_num_elements1
test_nomatch
test_ere2glob
test_glob2ere

set +u
x=1234
compound co
: "${x//~(X)([012])|([345])/ }"
x=$(print -v .sh.match)
typeset -m co.array=.sh.match
y=$(print -v co.array)
[[ $y == "$x" ]] || 'typeset -m of .sh.match to variable not working'
