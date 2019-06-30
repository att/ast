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

# test restricted shell
pwd=$PWD
case $SHELL in
    /*)    ;;
    */*)    SHELL=$pwd/$SHELL;;
    *)    SHELL=$(whence "$SHELL");;
esac

function check_restricted
{
    rm -f out
    LC_MESSAGES=C rksh -c "$@" 2> out > /dev/null
    grep restricted out  > /dev/null 2>&1
}

[[ $SHELL != /* ]] && SHELL=$pwd/$SHELL
ln -s $SHELL rksh
PATH=$PWD:$PATH
rksh -c  '[[ -o restricted ]]' || log_error 'restricted option not set'
[[ $(rksh -c 'print hello') == hello ]] || log_error 'unable to run print'
check_restricted /bin/echo || log_error '/bin/echo not resticted'
check_restricted ./echo || log_error './echo not resticted'
check_restricted 'SHELL=ksh' || log_error 'SHELL asignment not resticted'
check_restricted 'PATH=/bin' || log_error 'PATH asignment not resticted'
check_restricted 'FPATH=/bin' || log_error 'FPATH asignment not resticted'
check_restricted 'ENV=/bin' || log_error 'ENV asignment not resticted'
check_restricted 'print > file' || log_error '> file not restricted'
> empty
check_restricted 'print <> empty' || log_error '<> file not restricted'
print 'echo hello' > script
chmod +x ./script
! check_restricted script ||  log_error 'script without builtins should run in restricted mode'
check_restricted ./script ||  log_error 'script with / in name should not run in restricted mode'
print '/bin/echo hello' > script
! check_restricted script ||  log_error 'script with pathnames should run in restricted mode'
print 'echo hello> file' > script
! check_restricted script ||  log_error 'script with output redirection should run in restricted mode'
print 'PATH=/bin' > script
! check_restricted script ||  log_error 'script with PATH assignment should run in restricted mode'
cat > script <<!
#! $SHELL
print hello
!
! check_restricted 'script;:' ||  log_error 'script with #! pathname should run in restricted mode'
! check_restricted 'script' ||  log_error 'script with #! pathname should run in restricted mode even if last command in script'

for i in PATH ENV FPATH
do
    check_restricted  "function foo { typeset $i=foobar;};foo" || log_error "$i can be changed in function by using typeset"
done
