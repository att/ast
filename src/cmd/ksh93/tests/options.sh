########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2014 AT&T Intellectual Property          #
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

unset HISTFILE
export LC_ALL=C ENV=

if [[ $( ${SHELL-ksh} -s hello<<-\!
	print $1
	!
     ) != hello ]]
then
    log_error "${SHELL-ksh} -s not working"
fi

x=$(
    set -e
    false && print bad
    print good
)
if [[ $x != good ]]
then
    log_error 'sh -e not working'
fi

[[ $($SHELL -D -c 'print hi; print $"hello"') == '"hello"' ]] || log_error 'ksh -D not working'

env=$TEST_DIR/.env
print $'(print -u1 aha) &>/dev/null\n(print -u2 aha) &>/dev/null' > $env
rc=$TEST_DIR/.kshrc
print $'PS1=""\nfunction env_hit\n{\n\tprint OK\n}' > $rc

export ENV=/.$env
if [[ ! -o privileged ]]
then
    got=$($SHELL -E -c : 2>/dev/null)
    if [[ $got ]]
    then
        got=$(printf %q "$got")
        log_error "\$ENV file &>/dev/null does not redirect stdout -- expected '', got $got"
    fi

    got=$($SHELL -E -c : 2>&1 >/dev/null)
    if [[ $got != *nonstandard* || $got == *$'\n'* ]]
    then
        got=$(printf %q "$got")
        log_error "\$ENV file &>/dev/null does not redirect stderr -- expected one diagnostic line, got $got"
    fi
fi

export ENV=/.$rc
if [[ -o privileged ]]
then
    [[ $(print env_hit | $SHELL 2>&1) == "OK" ]] &&
        log_error 'privileged nointeractive shell reads $ENV file'
    [[ $(print env_hit | $SHELL -E 2>&1) == "OK" ]] &&
        log_error 'privileged -E reads $ENV file'
    [[ $(print env_hit | $SHELL +E 2>&1) == "OK" ]] &&
        log_error 'privileged +E reads $ENV file'
    [[ $(print env_hit | $SHELL --rc 2>&1) == "OK" ]] &&
        log_error 'privileged --rc reads $ENV file'
    [[ $(print env_hit | $SHELL --norc 2>&1) == "OK" ]] &&
        log_error 'privileged --norc reads $ENV file'
else
    [[ $(print env_hit | $SHELL 2>&1) == "OK" ]] &&
        log_error 'nointeractive shell reads $ENV file'
    [[ $(print env_hit | $SHELL -E 2>&1) == "OK" ]] ||
        log_error '-E ignores $ENV file'
    [[ $(print env_hit | $SHELL +E 2>&1) == "OK" ]] &&
        log_error '+E reads $ENV file'
    [[ $(print env_hit | $SHELL --rc 2>&1) == "OK" ]] ||
        log_error '--rc ignores $ENV file'
    [[ $(print env_hit | $SHELL --norc 2>&1) == "OK" ]] &&
        log_error '--norc reads $ENV file'
    [[ $(print env_hit | $SHELL -i 2>&1) == "OK" ]] ||
        log_error '-i ignores $ENV file'
fi

export ENV=
if [[ -o privileged ]]
then
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL 2>&1) == "OK" ]] &&
        log_error 'privileged nointeractive shell reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL -E 2>&1) == "OK" ]] &&
        log_error 'privileged -E ignores empty $ENV'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL +E 2>&1) == "OK" ]] &&
        log_error 'privileged +E reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --rc 2>&1) == "OK" ]] &&
        log_error 'privileged --rc ignores empty $ENV'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --norc 2>&1) == "OK" ]] &&
        log_error 'privileged --norc reads $HOME/.kshrc file'
else
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL 2>&1) == "OK" ]] &&
        log_error 'nointeractive shell reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL -E 2>&1) == "OK" ]] &&
        log_error '-E ignores empty $ENV'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL +E 2>&1) == "OK" ]] &&
        log_error '+E reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --rc 2>&1) == "OK" ]] &&
        log_error '--rc ignores empty $ENV'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --norc 2>&1) == "OK" ]] &&
        log_error '--norc reads $HOME/.kshrc file'
fi

unset ENV
if [[ -o privileged ]]
then
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL 2>&1) == "OK" ]] &&
        log_error 'privileged nointeractive shell reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL -E 2>&1) == "OK" ]] &&
        log_error 'privileged -E reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL +E 2>&1) == "OK" ]] &&
        log_error 'privileged +E reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --rc 2>&1) == "OK" ]] &&
        log_error 'privileged --rc reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --norc 2>&1) == "OK" ]] &&
        log_error 'privileged --norc reads $HOME/.kshrc file'
else
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL 2>&1) == "OK" ]] &&
        log_error 'nointeractive shell reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL -E 2>&1) == "OK" ]] ||
        log_error '-E ignores $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL +E 2>&1) == "OK" ]] &&
        log_error '+E reads $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --rc 2>&1) == "OK" ]] ||
        log_error '--rc ignores $HOME/.kshrc file'
    [[ $(print env_hit | HOME=$TEST_DIR $SHELL --norc 2>&1) == "OK" ]] &&
        log_error '--norc reads $HOME/.kshrc file'
fi

rm -rf $TEST_DIR/.kshrc

if command set -G 2> /dev/null
then
    mkdir subdir
    cd subdir
    mkdir bar foo
    > bar.c > bam.c
    > bar/foo.c > bar/bam.c
    > foo/bam.c
    set -- **.c
    expected='bam.c bar.c'
    [[ $* == $expected ]] ||
        log_error '-G **.c failed' "$expected" "$*"
    set -- **
    expected='bam.c bar bar.c bar/bam.c bar/foo.c foo foo/bam.c'
    [[ $* == $expected ]] ||
        log_error '-G ** failed' "$expected" "$*"
    set -- **/*.c
    expected='bam.c bar.c bar/bam.c bar/foo.c foo/bam.c'
    [[ $* == $expected ]] ||
        log_error '-G **/*.c failed' "$expected" "$*"
    set -- **/bam.c
    expected='bam.c bar/bam.c foo/bam.c'
    [[ $* == $expected ]] ||
        log_error '-G **/bam.c failed' "$expected" "$*"
    cd ..
fi

t="<$$>.profile.<$$>"
echo "echo '$t'" > $HOME/.profile
cp $SHELL ./-ksh
if [[ -o privileged ]]
then
    actual=$($SHELL -l </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error 'privileged -l reads .profile' "*$t*" "$actual"
    actual=$($SHELL --login </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error 'privileged --login reads .profile' "*$t*" "$actual"
    actual=$($SHELL --login-shell </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error 'privileged --login-shell reads .profile' "*$t*" "$actual"
    actual=$($SHELL --login_shell </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error 'privileged --login_shell reads .profile' "*$t*" "$actual"
    actual=$(exec -a -ksh $SHELL </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error 'privileged exec -a -ksh ksh reads .profile' "*$t*" "$actual"
    actual=$(./-ksh -i </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error 'privileged ./-ksh reads .profile' "*$t*" "$actual"
    actual=$(./-ksh -ip </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error 'privileged ./-ksh -p reads .profile' "*$t*" "$actual"
else
    actual=$($SHELL -l </dev/null 2>&1)
    [[ "$actual" == *$t* ]] ||
        log_error '-l ignores .profile' "*$t*" "$actual"
    actual=$($SHELL --login </dev/null 2>&1)
    [[ "$actual" == *$t* ]] ||
        log_error '--login ignores .profile' "*$t*" "$actual"
    actual=$($SHELL --login-shell </dev/null 2>&1)
    [[ "$actual" == *$t* ]] ||
        log_error '--login-shell ignores .profile' "*$t*" "$actual"
    actual=$($SHELL --login_shell </dev/null 2>&1)
    [[ "$actual" == *$t* ]] ||
        log_error '--login_shell ignores .profile' "*$t*" "$actual"
    actual=$(exec -a -ksh $SHELL </dev/null 2>/dev/null)
    [[ "$actual" == *$t* ]] ||
        log_error 'exec -a -ksh ksh 2>/dev/null ignores .profile' "*$t*" "$actual"
    actual=$(exec -a -ksh $SHELL </dev/null 2>&1)
    [[ "$actual" == *$t* ]] ||
        log_error 'exec -a -ksh ksh 2>&1 ignores .profile' "*$t*" "$actual"
    actual=$(./-ksh -i </dev/null 2>&1)
    [[ "$actual" == *$t* ]] ||
        log_error './-ksh ignores .profile' "*$t*" "$actual"
    actual=$(./-ksh -ip </dev/null 2>&1)
    [[ "$actual" == *$t* ]] &&
        log_error './-ksh -p does not ignore .profile' "*$t*" "$actual"
fi

cd ~-
rm -rf $TEST_DIR/.profile

# { exec interactive login_shell restricted xtrace } in the following test

for opt in \
    allexport all-export all_export bgnice bg-nice bg_nice clobber emacs \
    errexit err-exit err_exit glob globstar glob-star glob_star gmacs \
    ignoreeof ignore-eof ignore_eof keyword log markdirs monitor notify \
    pipefail pipe-fail pipe_fail trackall track-all track_all \
    unset verbose vi viraw vi-raw vi_raw
do
    old=$opt
    if [[ ! -o $opt ]]
    then
        old=no$opt
    fi

    set --$opt || log_error "set --$opt failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"
    [[ -o no$opt ]] && log_error "[[ -o no$opt ]] failed"
    [[ -o no-$opt ]] && log_error "[[ -o no-$opt ]] failed"
    [[ -o no_$opt ]] && log_error "[[ -o no_$opt ]] failed"
    [[ -o ?$opt ]] || log_error "[[ -o ?$opt ]] failed"
    [[ -o ?no$opt ]] || log_error "[[ -o ?no$opt ]] failed"
    [[ -o ?no-$opt ]] || log_error "[[ -o ?no-$opt ]] failed"
    [[ -o ?no_$opt ]] || log_error "[[ -o ?no_$opt ]] failed"

    set --no$opt || log_error "set --no$opt failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"
    [[ -o $opt ]] && log_error "[[ -o $opt ]] failed"

    set --no-$opt || log_error "set --no-$opt failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"
    [[ -o $opt ]] && log_error "[[ -o $opt ]] failed"

    set --no_$opt || log_error "set --no_$opt failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"
    [[ -o $opt ]] && log_error "[[ -o $opt ]] failed"

    set -o $opt || log_error "set -o $opt failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"
    set -o $opt=1 || log_error "set -o $opt=1 failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"
    set -o no$opt=0 || log_error "set -o no$opt=0 failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"
    set --$opt=1 || log_error "set --$opt=1 failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"
    set --no$opt=0 || log_error "set --no$opt=0 failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"

    set -o no$opt || log_error "set -o no$opt failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"
    set -o $opt=0 || log_error "set -o $opt=0 failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"
    set -o no$opt=1 || log_error "set -o no$opt=1 failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"
    set --$opt=0 || log_error "set --$opt=0 failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"
    set --no$opt=1 || log_error "set --no$opt=1 failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"

    set -o no-$opt || log_error "set -o no-$opt failed"
    [[ -o no-$opt ]] || log_error "[[ -o no-$opt ]] failed"

    set -o no_$opt || log_error "set -o no_$opt failed"
    [[ -o no_$opt ]] || log_error "[[ -o no_$opt ]] failed"

    set +o $opt || log_error "set +o $opt failed"
    [[ -o no$opt ]] || log_error "[[ -o no$opt ]] failed"

    set +o no$opt || log_error "set +o no$opt failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"

    set +o no-$opt || log_error "set +o no-$opt failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"

    set +o no_$opt || log_error "set +o no_$opt failed"
    [[ -o $opt ]] || log_error "[[ -o $opt ]] failed"

    set --$old
done

for opt in \
    exec interactive login_shell login-shell logi privileged \
    rc restricted xtrace
do
    [[ -o $opt ]]
    y=$?
    [[ -o no$opt ]]
    n=$?
    case $y$n in
    10|01)    ;;
    *)    log_error "[[ -o $opt ]] == [[ -o no$opt ]]" ;;
    esac
done

for opt in \
    foo foo-bar foo_bar
do
    if [[ -o ?$opt ]]
    then
        log_error "[[ -o ?$opt ]] should fail"
    fi

    if [[ -o ?no$opt ]]
    then
        log_error "[[ -o ?no$opt ]] should fail"
    fi
done

[[ $(set +o) == $(set --state) ]] || log_error "set --state different from set +o"
set -- $(set --state)
[[ $1 == set && $2 == --default ]] || log_error "set --state failed -- expected 'set --default *', got '$1 $2 *'"
shift
restore=$*
shift
off=
for opt
do
    case $opt in
    --not*)    opt=${opt/--/--no} ;;
    --no*)    opt=${opt/--no/--} ;;
    --*)    opt=${opt/--/--no} ;;
    esac
    off="$off $opt"
done
set $off
state=$(set --state)
default=$(set --default --state)
[[ $state == $default ]] || log_error "set --state for default options failed: expected '$default', got '$state'"
set $restore
state=$(set --state)
[[ $state == "set $restore" ]] || log_error "set --state after restore failed: expected 'set $restore', got '$state'"

typeset -a pipeline
pipeline=(
    ( nopipefail=0 pipefail=1 command='false|true|true' )
    ( nopipefail=0 pipefail=1 command='true|false|true' )
    ( nopipefail=1 pipefail=1 command='true|true|false' )
    ( nopipefail=1 pipefail=1 command='false|false|false' )
    ( nopipefail=0 pipefail=0 command='true|true|true' )
    ( nopipefail=0 pipefail=0 command='print hi|(sleep 1;/bin/cat)>/dev/null' )
)
set --nopipefail
for ((i = 0; i < ${#pipeline[@]}; i++ ))
do
    eval ${pipeline[i].command}
    status=$?
    expected=${pipeline[i].nopipefail}
    [[ $status == $expected ]] ||
    log_error "--nopipefail '${pipeline[i].command}' exit status $status -- expected $expected"
done
ftt=0
set --pipefail
for ((i = 0; i < ${#pipeline[@]}; i++ ))
do
    eval ${pipeline[i].command}
    status=$?
    expected=${pipeline[i].pipefail}
    if [[ $status != $expected ]]
    then
        log_error "--pipefail '${pipeline[i].command}' exit status $status -- expected $expected"
        (( i == 0 )) && ftt=1
    fi
done

if (( ! ftt ))
then
    exp=10
    got=$(for((n=1;n<exp;n++))do $SHELL --pipefail -c '(sleep 0.1;false)|true|true' && break; done; print $n)
    [[ $got == $exp ]] || log_error "--pipefail -c '(sleep 0.1;false)|true|true' fails with exit status 0 (after $got/$exp iterations)"
fi

for ((i=0; i < 20; i++))
do
    if ! x=$(true | $bin_echo 123)
    then
        log_error 'command substitution with wrong exit status with pipefai'
        break
    fi
done
(
    set -o pipefail
    false | true
    (( $? )) || log_error 'pipe not failing in subshell with pipefail'
) | wc >/dev/null

$SHELL -c 'set -o pipefail; false | $bin_true;' && log_error 'pipefail not returning failure with sh -c'
exp='1212 or 1221'
got=$(
    set --pipefail
    pipe() { date | cat > /dev/null ;}
    print $'1\n2' |
    while read i
    do
        if pipe $TEST_DIR
        then
            { print -n $i; sleep 2; print -n $i; } &
        fi
    done
    wait
)
[[ $got == @((12|21)(12|21)) ]] || log_error "& job delayed by --pipefail, expected '$exp', got '$got'"
$SHELL -c '[[ $- == *c* ]]' || log_error 'option c not in $-'
> $TEST_DIR/.profile

for i in i l r s D E a b e f h k n t u v x B C G H
do
    HOME=$TEST_DIR ENV= $SHELL -$i >/dev/null 2>&1 <<- ++EOF++ || log_error "option $i not in \$-"
	[[ \$- == *$i* ]] || exit 1
	++EOF++
done

letters=ilrabefhknuvxBCGE
integer j=0
for i in interactive login restricted allexport notify errexit \
    noglob trackall keyword noexec nounset verbose xtrace braceexpand \
    noclobber globstar rc
do
    HOME=$TEST_DIR ENV= $SHELL -o $i >/dev/null 2>&1 <<- ++EOF++ || log_error "option $i not equivalent to ${letters:j:1}"
	[[ \$- == *${letters:j:1}* ]] || exit 1
	++EOF++
    ((j++))
done

export ENV= PS1="(:$$:)"
histfile=$TEST_DIR/history
exp=$(HISTFILE=$histfile $SHELL -c $'function foo\n{\ncat\n}\ntype foo')
for var in HISTSIZE HISTFILE
do
    got=$( ( HISTFILE=$histfile $SHELL +E -ic $'unset '$var$'\nfunction foo\n{\ncat\n}\ntype foo\nexit' ) 2>&1 )
    got=${got##*"$PS1"}
    [[ $got == "$exp" ]] || log_error "function definition inside (...) with $var unset fails -- got '$got', expected '$exp'"
    got=$( { HISTFILE=$histfile $SHELL +E -ic $'unset '$var$'\nfunction foo\n{\ncat\n}\ntype foo\nexit' ;} 2>&1 )
    got=${got##*"$PS1"}
    [[ $got == "$exp" ]] || log_error "function definition inside {...;} with $var unset fails -- got '$got', expected '$exp'"
done
( unset HISTFILE; $SHELL -ic "HISTFILE=$histfile" 2>/dev/null ) || log_error "setting HISTFILE when not in environment fails"

# the next tests loop on all combinations of
#    { SUB PAR CMD ADD }

SUB=(
    ( BEG='$( '    END=' )'    )
    ( BEG='${ '    END='; }'    )
)
PAR=(
    ( BEG='( '    END=' )'    )
    ( BEG='{ '    END='; }'    )
)
CMD=(    command-kill    script-kill    )
ADD=(    ''        '; :'        )

cd $TEST_DIR
print $'#!'$SHELL$'\nkill -KILL $$' > command-kill
print $'kill -KILL $$' > script-kill
chmod +x command-kill script-kill
export PATH=.:$PATH
exp='Killed'
for ((S=0; S<${#SUB[@]}; S++))
do
    for ((P=0; P<${#PAR[@]}; P++))
    do
        for ((C=0; C<${#CMD[@]}; C++))
        do
            for ((A=0; A<${#ADD[@]}; A++))
            do
                cmd="${SUB[S].BEG}${PAR[P].BEG}${CMD[C]}${PAR[P].END} 2>&1${ADD[A]}${SUB[S].END}"
                eval got="$cmd"
                got=${got##*': '}
                got=${got%%'('*}
                [[ $got == "$exp" ]] || log_error "$cmd failed -- got '$got', expected '$exp'"
            done
        done
    done
done

$SHELL 2> /dev/null -c '{; true ;}' || log_error 'leading ; causes syntax error in brace group'
$SHELL 2> /dev/null -c '(; true ;)' || log_error 'leading ; causes syntax error in parenthesis group'

print 'for ((i = 0; i < ${1:-10000}; i++ )); do printf "%.*c\n" 15 x; done' > pipefail
chmod +x pipefail
$SHELL --pipefail -c './pipefail 10000 | sed 1q' >/dev/null 2>&1 &
tst=$!
{ sleep 4; kill $tst; } 2>/dev/null &
spy=$!
wait $tst 2>/dev/null
status=$?
if [[ $status == 0 || $(kill -l $status) == PIPE ]]
then
    kill $spy 2>/dev/null
else
    log_error "pipefail pipeline bypasses SIGPIPE and hangs"
fi

wait

[[ $($SHELL -uc '[[ "${d1.u[z asd].revents}" ]]' 2>&1) == *'d1.u[z asd].revents'* ]] || log_error 'name of unset parameter not in error message'

[[ $($SHELL 2> /dev/null -xc $'set --showme\nprint 1\n; print 2') == 1 ]] || log_error  'showme option with xtrace not working correctly'

$SHELL -uc 'var=foo;unset var;: ${var%foo}' >/dev/null 2>&1 && log_error '${var%foo} should fail with set -u'
$SHELL -uc 'var=foo;unset var;: ${!var}' >/dev/null 2>&1 && log_error '${!var} should fail with set -u'
$SHELL -uc 'var=foo;unset var;: ${#var}' >/dev/null 2>&1 && log_error '${#var} should fail with set -u'
$SHELL -uc 'var=foo;unset var;: ${var-OK}' >/dev/null 2>&1 || log_error '${var-OK} should not fail with set -u'
$SHELL -uc 'var=foo;nset var;: ${var:-OK}' >/dev/null 2>&1 || log_error '${var:-OK} should not fail with set -u'

z=$($SHELL 2>&1 -uc 'print ${X23456789012345}')
[[ $z == *X23456789012345:* ]] || log_error "error message garbled with set -u got $z"

# pipe hang bug fixed 2011-03-15
float start=SECONDS toolong=3
( $SHELL <<-EOF
	set -o pipefail
	(sleep $toolong;kill \$\$> /dev/null) &
	cat $SHELL | for ((i=0; i < 5; i++))
	do
		date | wc > /dev/null
		$SHELL -c 'read -N1'
	done
EOF
) 2> /dev/null
(( (SECONDS-start) > (toolong-0.5) )) && log_error "pipefail causes script to hang"

# showme with arithmetic for loops
$SHELL -n -c $'for((;1;))\ndo ; nothing\ndone'  2>/dev/null  || log_error 'showme commands give syntax error inside arithmetic for loops'

#set -x
float t1=SECONDS
set -o pipefail
print  | while read
do
        if { date | true;} ; true
        then
            sleep 2 &
        fi
done
(( (SECONDS-t1) > .5 )) && log_error 'pipefail should not wait for background processes'

# process source files from profiles as profile files
print '. ./dotfile' > envfile
print $'alias print=:\nprint foobar' > dotfile
[[ $(ENV=$PWD/envfile $SHELL -i -c : 2>/dev/null) == foobar ]] &&
    log_error 'files source from profile does not process aliases correctly'

# tests the set -m puts background jobs in separate process group
Command=$Command LINENO=$LINENO $SHELL -m  <<- \EOF
	error_count=0
	function log_error
	{
		print -u2 -n "\t"
		print -u2 -r ${Command}[$LINENO]: "${@:1}"
		((error_count++))
	}
	[[ $- == *m* ]] || log_error '$- does not contain m when monitor mode specified'
	float t=SECONDS
	sleep 2 & pid=$!
	kill -KILL -$pid 2> /dev/null || log_error 'kill to background group failed'
	wait 2> /dev/null
	(( (SECONDS-t) > 1 )) && log_error 'kill did not kill background sleep'
	exit $error_count
EOF
((error_count+=$?))

# ==========
$SHELL 2> /dev/null <<- \EOF && log_error 'unset variable with set -u on does not terminate script'
	set -e -u -o pipefail
	ls | while read file
	do
		files[${#files[*]}]=$fil
	done
	exit
EOF

# ==========
actual="$($SHELL -vc 'print yes' 2>&1)"
expect="print yes"
[[ "$actual" =~ .*"$expect".* ]] || log_error 'incorrect output from ksh -v' "$expect" "$actual"
