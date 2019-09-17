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
########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                 Copyright (c) 2011-2012 Roland Mainz                 #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#                                                                      #
#                 Roland Mainz <roland.mainz@nrubsig.org>              #
#                                                                      #
########################################################################
#
# Copyright (c) 2013, Roland Mainz. All rights reserved.
#
if [[ ! -d /proc ]]
then
    log_warning "Skipping this test since the system does not have a /proc directory"
    cd /tmp
    rm -rf $TEST_DIR
    exit $MESON_SKIPPED_TEST
fi

#
# The test module checks whether directory file descrpitors work.
#
function test_dirfd_basics1
{
    mkdir 'test_dirfd_basics1'
    cd 'test_dirfd_basics1'

    compound out=( typeset stdout stderr ; integer res )
    compound -r -a tests=(
        (
            testname='createfile1_redirect'
            cmd='redirect {dirfd}<"./tmp" ; print "foo1" >[dirfd]/test1 ; cat tmp/test1'
            stdoutpattern='foo1'
        )
        (
            testname='createfile1_cmdgroup'
            cmd='{ print "iam2" >[dirfd]/test1 ; cat tmp/test1 ; } {dirfd}<"./tmp"'
            stdoutpattern='iam2'
        )
        (
            testname='createfile2_inheritfd_cmdgroup'
            cmd='{ print "vem1" >[dirfd]/test1 ; dirfd=$dirfd $SHELL -c "cat </proc/\$\$/fd/\$dirfd/test1" ; } {dirfd}<"./tmp"'
            stdoutpattern='vem1'
        )
        (
            testname='appendfile1_inheritfd_cmdgroup_proc'
            cmd='{ print -n "foo1_" >[dirfd]/test1 ; dfd=$dirfd $SHELL -c "print appendix1 >>/proc/\$\$/fd/\$dfd/test1" ; } {dirfd}<"./tmp"; cat tmp/test1'
            stdoutpattern='foo1_appendix1'
        )
        (
            testname='appendfile1_inheritfd_cmdgroup_devfd'
            cmd='{ print -n "foo2_" >[dirfd]/test1 ; dfd=$dirfd $SHELL -c "print appendix2 >>/dev/fd/\$dfd/test1" ; } {dirfd}<"./tmp"; cat tmp/test1'
            stdoutpattern='foo2_appendix2'
        )
        (
            testname='createdir1_redirect'
            cmd='redirect {dirfd}<"./tmp" ; mkdir [dirfd]/test1 ; print "foo2" >tmp/test1/a ; cat [dirfd]/test1/a ; rm tmp/test1/a ; rmdir [dirfd]/test1'
            stdoutpattern='foo2'
        )
        (
            testname='createdir1_cmdgroup'
            cmd='{ mkdir [dirfd]/test1 ; print "foo3" >tmp/test1/a ; cat [dirfd]/test1/a ; rm tmp/test1/a ; rmdir [dirfd]/test1 ; } {dirfd}<"tmp" ;'
            stdoutpattern='foo3'
        )
    )
    typeset testname
    integer i
    typeset cmd
    typeset devfd

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        nameref tst=tests[i]

        # iterate over all names libast uses to emulate /dev/fd/$fd/<path>
        for devfd in \
            '/proc/$$/fd/${dirfd}/' \
            '/proc/self/fd/${dirfd}/' \
            '/dev/fd/${dirfd}/' \
            '~{dirfd}/' ; do
            cmd="${tst.cmd//\[dirfd\]/${devfd}} ; true"
            testname="${0}/${tst.testname}/${cmd}"

            mkdir 'tmp'

            out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -o errexit -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

            rm -Rf 'tmp'

            expect="${tst.stdoutpattern}"
            actual="${out.stdout}"
            [[ $actual == $expect ]] ||
                log_error "${testname}: Expected stdout to match" "$expect" "$actual"
            expect=''
            actual="${out.stderr}"
            [[ $actual == $expect ]] ||
                log_error "${testname}: Expected empty stderr" "$expect" "$actual"
            expect=0
            actual=$out.res
            (( actual == expect )) ||
                log_error "${testname}: Unexpected exit code" "$expect" "$actual"
        done
    done

    cd $TEST_DIR
}

# Run tests.
test_dirfd_basics1
