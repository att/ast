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

PS3='ABC '

cat > $TEST_DIR/1 <<\!
1) foo
2) bar
3) bam
!

select i in foo bar bam
do
    case $i in
        foo)
            break;;
        *)
            log_error "select 1 not working"
            break;;
    esac
done 2> /dev/null <<!
1
!

unset i
select i in foo bar bam
do
    case $i in
    foo)
        log_error "select foo not working" 2>&3
        break;;
    *)
        if [[ $REPLY != foo ]]
        then
            log_error "select REPLY not correct" 2>&3
        fi
        ( set -u; : $i ) || log_error "select: i not set to null" 2>&3
        break;;
    esac
done  3>&2 2> $TEST_DIR/2 <<!
foo
!
