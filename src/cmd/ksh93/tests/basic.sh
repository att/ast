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

# ==========
# Can we get the version string. It would be nice to compare this to the actual version used during
# the build but that's more work than it is worth.
# Regression for issue #836.
#
# TODO: Remove the stderr redirection when issue #837 is fixed.
actual=$($SHELL --version 2>&1)
expect='version'
[[ "$actual" == *${expect}* ]] || log_error "failed to get version string" "*${expect}*" "$actual"

# test basic file operations like redirection, pipes, file expansion
set -- \
    go+r    0000    \
    go-r    0044    \
    ug=r    0330    \
    go+w    0000    \
    go-w    0022    \
    ug=w    0550    \
    go+x    0000    \
    go-x    0011    \
    ug=x    0660    \
    go-rx    0055    \
    uo-wx    0303    \
    ug-rw    0660    \
    o=    0007
while (( $# >= 2 ))
do
    umask 0
    umask $1
    expect=$2
    actual=$(umask)
    [[ $actual == $expect ]] || log_error "umask 0; umask $1 failed" "$expect" "$actual"
    shift 2
done

umask u=rwx,go=rx || log_error "umask u=rws,go=rx failed"
if [[ $(umask -S) != u=rwx,g=rx,o=rx ]]
then
    log_error 'umask -S incorrect'
fi

pwd=$PWD
[[ $SHELL != /* ]] && SHELL=$pwd/$SHELL
um=$(umask -S)
( umask 0777; > foobar )
rm -f foobar
> foobar
[[ -r foobar ]] || log_error 'umask not being restored after subshell'
umask "$um"
rm -f foobar
# optimizer bug test
> foobar
for i in 1 2
do
        print foobar*
        rm -f foobar
done > out
if   [[ "$(<out)"  != "foobar"$'\n'"foobar*" ]]
then
    print -u2 "optimizer bug with file expansion"
fi

mkdir top_dir
cd top_dir
mkdir dir
actual="$(print */)"
expect=dir/
[[ $actual == $expect ]] ||
    log_error 'file expansion with trailing / not working' "$expect" "$actual"

actual="$(print */)"
expect=dir/
[[ $actual == $expect ]] ||
    log_error 'file expansion with single file not working' "$expect" "$actual"

print hi > .foo
actual="$(print *)"
expect=dir
[[ $actual == $expect ]] ||
    log_error 'file expansion leading . not working' "$expect" "$actual"
cd ..

date > dat1 || log_error "date > dat1 failed"
test -r dat1 || log_error "dat1 is not readable"
x=dat1
cat <$x > dat2 || log_error "cat < $x > dat2 failed"
cat dat1 dat2 | cat  | cat | cat > dat3 || log_error "cat pipe failed"
cat > dat4 <<!
$(date)
!
cat dat1 dat2 | cat  | cat | cat > dat5 &
wait $!
set -- dat*
if (( $# != 5 ))
then
    log_error "dat* matches only $# files"
fi

if (command > foo\\abc) 2> /dev/null
then
    set -- foo*
    if [[ $1 != 'foo\abc' ]]
    then
        log_error 'foo* does not match foo\abc'
    fi

fi

if ( : > TT* && : > TTfoo ) 2>/dev/null
then
    set -- TT*
    if (( $# < 2 ))
    then
        log_error 'TT* not expanding when file TT* exists'
    fi

fi

cd ~- || log_error "cd back failed"
cat > $TEST_DIR/script <<- !
    #! $SHELL
    print -r -- \$0
!
chmod 755 $TEST_DIR/script
if [[ $($TEST_DIR/script) != "$TEST_DIR/script" ]]
then
    log_error '$0 not correct for #! script'
fi

bar=foo
eval foo=\$bar
if [[ $foo != foo ]]
then
    log_error 'eval foo=\$bar not working'
fi

bar='foo=foo\ bar'
eval $bar
if [[ $foo != 'foo bar' ]]
then
    log_error 'eval foo=\$bar, with bar="foo\ bar" not working'
fi

cd /tmp
cd ../../tmp || log_error "cd ../../tmp failed"
if [[ $PWD != /tmp ]]
then
    log_error 'cd ../../tmp is not /tmp'
fi

( sleep 2; cat <<!
foobar
!
) | cat > $TEST_DIR/foobar &
wait $!
foobar=$( < $TEST_DIR/foobar)
if [[ $foobar != foobar ]]
then
    log_error "$foobar is not foobar"
fi

{
    print foo
    /bin/echo bar
    print bam
} > $TEST_DIR/foobar
if [[ $( < $TEST_DIR/foobar) != $'foo\nbar\nbam' ]]
then
    log_error "output file pointer not shared correctly"
fi

cat > $TEST_DIR/foobar <<\!
    print foo
    /bin/echo bar
    print bam
!
chmod +x $TEST_DIR/foobar
if [[ $($TEST_DIR/foobar) != $'foo\nbar\nbam' ]]
then
    log_error "script not working"
fi

if [[ $($TEST_DIR/foobar | /bin/cat) != $'foo\nbar\nbam' ]]
then
    log_error "script | cat not working"
fi

if [[ $( $TEST_DIR/foobar) != $'foo\nbar\nbam' ]]
then
    log_error "output file pointer not shared correctly"
fi

rm -f $TEST_DIR/foobar
x=$( (print foo) ; (print bar) )
if [[ $x != $'foo\nbar' ]]
then
    log_error " ( (print foo);(print bar ) failed"
fi

x=$( (/bin/echo foo) ; (print bar) )
if [[ $x != $'foo\nbar' ]]
then
    log_error " ( (/bin/echo);(print bar ) failed"
fi

x=$( (/bin/echo foo) ; (/bin/echo bar) )
if [[ $x != $'foo\nbar' ]]
then
    log_error " ( (/bin/echo);(/bin/echo bar ) failed"
fi


log_info 'TODO: Skipping test "builtin replaces standard input pipe"'
#cat > $TEST_DIR/script <<\!
#if [[ -p /dev/fd/0 ]]
#then
#    builtin cat
#    cat - > /dev/null
#    [[ -p /dev/fd/0 ]] && print ok
#else    print no
#fi

#!
#chmod +x $TEST_DIR/script
#case $( (print) | $TEST_DIR/script;:) in
#ok)    ;;
#no)    log_error "[[ -p /dev/fd/0 ]] fails for standard input pipe" ;;
#*)    log_error "builtin replaces standard input pipe" ;;
#esac
print 'print $0' > $TEST_DIR/script
print ". $TEST_DIR/script" > $TEST_DIR/scriptx
chmod +x $TEST_DIR/scriptx
if [[ $($TEST_DIR/scriptx) != $TEST_DIR/scriptx ]]
then
    log_error '$0 not correct for . script'
fi

cd $TEST_DIR || { log_error "cd $TEST_DIR failed"; exit 1; }
print ./b > ./a; print ./c > b; print ./d > c; print ./e > d; print "echo \"hello there\"" > e
for x in a b c d e
do
    chmod 755 $x  # chmod builtin at the moment only handles one filename
done
x=$(./a)
if [[ $x != "hello there" ]]
then
    log_error "nested scripts failed"
fi

x=$( (./a) | cat)
if [[ $x != "hello there" ]]
then
    log_error "scripts in subshells fail"
fi

cd ~- || log_error "cd back failed"
x=$( (/bin/echo foo) 2> /dev/null )
if [[ $x != foo ]]
then
    log_error "subshell in command substitution fails"
fi

exec 7>& 1
exec 1>&-
x=$(print hello)
if [[ $x != hello ]]
then
    log_error "command subsitution with stdout closed failed"
fi

exec >& 7
cd $pwd
x=$(cat <<\! | $SHELL
/bin/echo | /bin/cat
/bin/echo hello
!
)
if [[ $x != $'\n'hello ]]
then
    log_error "$SHELL not working when standard input is a pipe"
fi

x=$( (/bin/echo hello) 2> /dev/null )
if [[ $x != hello ]]
then
    log_error "subshell in command substitution with 1 closed fails"
fi

cat > $TEST_DIR/script <<- \!
read line 2> /dev/null
print done

!
if [[ $($SHELL $TEST_DIR/script <&-) != done ]]
then
    log_error "executing script with 0 closed fails"
fi

trap '' INT
cat > $TEST_DIR/script <<- \!
trap 'print bad' INT
kill -s INT $$
print good
!
chmod +x $TEST_DIR/script
if [[ $($SHELL  $TEST_DIR/script) != good ]]
then
    log_error "traps ignored by parent not ignored"
fi

trap - INT
cat > $TEST_DIR/script <<- \!
read line
/bin/cat
!
if [[ $($SHELL $TEST_DIR/script <<!
one
two
!
)    != two ]]
then
    log_error "standard input not positioned correctly"
fi

word=$(print $'foo\nbar' | { read line; /bin/cat;})
if [[ $word != bar ]]
then
    log_error "pipe to { read line; /bin/cat;} not working"
fi

word=$(print $'foo\nbar' | ( read line; /bin/cat) )
if [[ $word != bar ]]
then
    log_error "pipe to ( read line; /bin/cat) not working"
fi

if [[ $(print x{a,b}y) != 'xay xby' ]]
then
    log_error 'brace expansion not working'
fi

if [[ $(for i in foo bar
      do ( tgz=$(print $i)
      print $tgz)
      done) != $'foo\nbar' ]]
then
    log_error 'for loop subshell optimizer bug'
fi

unset a1
optbug()
{
    set -A a1  foo bar bam
    integer i
    for ((i=0; i < 3; i++))
    do
        (( ${#a1[@]} < 2 )) && return 0
        set -- "${a1[@]}"
        shift
        set -A a1 -- "$@"
    done

    return 1
}
optbug ||  log_error 'array size optimzation bug'
wait # not running --pipefail which would interfere with subsequent tests
: $(jobs -p) # required to clear jobs for next jobs -p (interactive side effect)
sleep 20 &
pids=$!
if [[ $(jobs -p) != $! ]]
then
    log_error 'jobs -p not reporting a background job'
fi

sleep 20 &
pids="$pids $!"
foo()
{
    set -- $(jobs -p)
    (( $# == 2 )) || log_error "$# jobs not reported -- 2 expected"
}
foo
kill $pids

[[ $( (trap 'print alarm' ALRM; sleep 4) & sleep 2; kill -ALRM $!; sleep 2; wait) == alarm ]] || log_error 'ALRM signal not working'

[[ $($SHELL -c 'trap "" HUP; $SHELL -c "(sleep 2;kill -HUP $$)& sleep 4;print done"') != done ]] && log_error 'ignored traps not being ignored'

[[ $($SHELL -c 'o=foobar; for x in foo bar; do (o=save);print $o;done' 2> /dev/null ) == $'foobar\nfoobar' ]] || log_error 'for loop optimization subshell bug'

command exec 3<> /dev/null
if $bin_cat /dev/fd/3 >/dev/null 2>&1
then
    exec 3>&-

    [[ $($SHELL -c 'cat <(print foo)' 2> /dev/null) == foo ]] || log_error 'process substitution not working'

    [[ $($SHELL -c  $'tee >(grep \'1$\' > '$TEST_DIR/scriptx$') > /dev/null <<-  \!!!
	line0
	line1
	line2
	!!!
    wait
    cat '$TEST_DIR/scriptx 2> /dev/null)  == line1 ]] || log_error '>() process substitution fails'
    > $TEST_DIR/scriptx

    [[ $($SHELL -c  $'
    for i in 1
    do
    tee >(grep \'1$\' > '$TEST_DIR/scriptx$') > /dev/null  <<-  \!!!
	line0
	line1
	line2
	!!!
    done

    wait
    cat '$TEST_DIR/scriptx 2>> /dev/null) == line1 ]] || log_error '>() process substitution fails in for loop'

    [[ $({ $SHELL -c 'cat <(for i in x y z; do print $i; done)';} 2> /dev/null) == $'x\ny\nz' ]] ||
        log_error 'process substitution of compound commands not working'

    for tee in "$(whence tee)" $bin_tee
    do
    print xxx > $TEST_DIR/file
        $tee  >(sleep 1;cat > $TEST_DIR/file) <<< "hello" > /dev/null
        [[ $(< $TEST_DIR/file) != hello ]] && log_error "process substitution does not wait for >() to complete with $tee"
        print yyy > $TEST_DIR/file2
        $tee >(cat > $TEST_DIR/file) >(sleep 1;cat > $TEST_DIR/file2) <<< "hello" > /dev/null
        [[ $(< $TEST_DIR/file2) != hello ]] && log_error "process substitution does not wait for second of two >() to complete with $tee"
        print xxx > $TEST_DIR/file
        $tee  >(sleep 1;cat > $TEST_DIR/file) >(cat > $TEST_DIR/file2) <<< "hello" > /dev/null
        [[ $(< $TEST_DIR/file) != hello ]] && log_error "process substitution does not wait for first of two >() to complete with $tee"
    done

    if [[ $(print <(print foo) & sleep .5; kill $! 2>/dev/null) == /dev/fd* ]]
    then
        expect='/dev/fd/+(\d) v=bam /dev/fd/+(\d)'
        actual=$( print <(print foo) v=bam <(print bar))
        [[ $actual == $expect ]] ||
            log_error 'assignments after command substitution not treated as arguments' "$expect" "$actual"
    fi

    {
        producer() {
            for ((i = 0; i < 20000; i++ ))
            do
                print xxxxx${i}xxxxx
            done
        }
        consumer() {
            while read var
            do
                print ${var}
            done < ${1}
        }
        consumer <(producer) > /dev/null
    } &
    pid=$!

    # On most systems a five second timeout is adequate. On my WSL (Windows Subsystem for Linux) VM
    # This test takes six seconds. Hence the ten second read timeout.
    (read -t 10 -u 9 x && exit 0; kill -HUP $pid) 2> /dev/null  &
    wait $pid || log_error "process substitution hangs"
    print -u 9 exit
    empty_fifos
fi

[[ $($SHELL -cr 'command -p :' 2>&1) == *restricted* ]]  || log_error 'command -p not restricted'
print cat >  $TEST_DIR/scriptx
chmod +x $TEST_DIR/scriptx
[[ $($SHELL -c "print foo | $TEST_DIR/scriptx ;:" 2> /dev/null ) == foo ]] || log_error 'piping into script fails'
[[ $($SHELL -c 'X=1;print -r -- ${X:=$(expr "a(0)" : '"'a*(\([^)]\))')}'" 2> /dev/null) == 1 ]] || log_error 'x=1;${x:=$(..."...")} failure'
[[ $($SHELL -c 'print -r -- ${X:=$(expr "a(0)" : '"'a*(\([^)]\))')}'" 2> /dev/null) == 0 ]] || log_error '${x:=$(..."...")} failure'
if cat /dev/fd/3 >/dev/null 2>&1  || whence mkfifo > /dev/null
then
    [[ $(cat <(print hello) ) == hello ]] || log_error "process substitution not working outside for or while loop"
    $SHELL -c '[[ $(for i in 1;do cat <(print hello);done ) == hello ]]' 2> /dev/null|| log_error "process substitution not working in for or while loop"
fi

exec 3> /dev/null
print 'print foo "$@"' > $TEST_DIR/scriptx
[[ $( print "($TEST_DIR/scriptx bar)" | $SHELL 2>/dev/null) == 'foo bar' ]] || log_error 'script pipe to shell fails'
print "#! $SHELL" > $TEST_DIR/scriptx
print 'print  -- $0' >> $TEST_DIR/scriptx
chmod +x $TEST_DIR/scriptx
[[ $($TEST_DIR/scriptx) == $TEST_DIR/scriptx ]] || log_error  "\$0 is $0 instead of $TEST_DIR/scriptx"
cat > $TEST_DIR/scriptx <<- \EOF
    myfilter() { x=$(print ok | cat); print  -r -- $SECONDS;}
    set -o pipefail
    sleep 3 | myfilter
EOF
(( $($SHELL $TEST_DIR/scriptx) > 2.0 )) && log_error 'command substitution causes pipefail option to hang'
exec 3<&-
( typeset -r foo=bar) 2> /dev/null || log_error 'readonly variables set in a subshell cannot unset'

$SHELL -c 'x=${ print hello;}; [[ $x == hello ]]' 2> /dev/null || log_error '${ command;} not supported'

$SHELL 2> /dev/null <<- \EOF || log_error 'multiline ${...} command substitution not supported'
    x=${
        print hello
    }
    [[ $x == hello ]]
EOF

$SHELL 2> /dev/null <<- \EOF || log_error '${...} command substitution with side effects not supported '
    y=bye
    x=${
        y=hello
        print hello
    }
    [[ $y == $x ]]
EOF

$SHELL   2> /dev/null <<- \EOF || log_error 'nested ${...} command substitution not supported'
    x=${
        print ${ print hello;} $(print world)
    }
    [[ $x == 'hello world' ]]
EOF

$SHELL   2> /dev/null <<- \EOF || log_error 'terminating } is not a reserved word with ${ command }'
    x=${    { print -n } ; print -n hello ; }  ; print ' world' }
    [[ $x == '}hello world' ]]
EOF

$SHELL   2> /dev/null <<- \EOF || log_error '${ command;}xxx not working'
    f()
    {
        print foo
    }
    [[ ${ f;}bar == foobar ]]
EOF

unset foo
[[ ! ${foo[@]} ]] || log_error '${foo[@]} is not empty when foo is unset'
[[ ! ${foo[3]} ]] || log_error '${foo[3]} is not empty when foo is unset'
[[ $(print  "[${ print foo }]") == '[foo]' ]] || log_error '${...} not working when } is followed by ]'
[[ $(print  "${ print "[${ print foo }]" }") == '[foo]' ]] || log_error 'nested ${...} not working when } is followed by ]'
unset foo
foo=$(false) > /dev/null && log_error 'failed command substitution with redirection not returning false'
expect=foreback
actual=`print -n fore; (sleep 2;print back)&`
[[ $actual == $expect ]] ||
    log_error "\`\`command substitution background process output error" "$expect" "$actual"
actual=$(print -n fore; (sleep 2;print back)&)
[[ $actual == $expect ]] ||
    log_error "\$() command substitution background process output error" "$expect" "$actual"
actual=${ print -n fore; (sleep 2;print back)& }
[[ $actual == $expect ]] ||
    log_error "\${} command substitution background process output error" "$expect" "$actual"
function abc { sleep 2; print back; }
function abcd { abc & }
actual=$(print -n fore;abcd)
[[ $actual == $expect ]] ||
    log_error "\$() command substitution background with function process output error" "$expect" "$actual"

for false in false $bin_false
do
    x=$($false) && log_error "x=\$($false) should fail"
    $($false) && log_error "\$($false) should fail"
    $($false) > /dev/null && log_error "\$($false) > /dev/null should fail"
done

if env x-a=y >/dev/null 2>&1
then
    [[ $(env 'x-a=y'  $SHELL -c 'env | grep x-a') == *x-a=y* ]] || log_error 'invalid environment variables not preserved'
fi

float s=SECONDS
for i in 1 2
do
      print $i
done | while read sec; do ( $bin_sleep $sec; $bin_sleep $sec) done
(( (SECONDS-s)  < 4)) && log_error '"command | while read...done" finishing too fast'

s=SECONDS
set -o pipefail
for ((i=0; i < 30; i++))
do
    print hello
    sleep .1
done | $sleep 1
(( (SECONDS-s) < 2 )) || log_error 'early termination not causing broken pipe'

[[ $({ trap 'print trap' 0; print -n | $bin_cat; } & wait $!) == trap ]] || log_error 'trap on exit not getting triggered'
var=$({ trap 'print trap' ERR; print -n | $bin_false; } & wait $!)
[[ $var == trap ]] || log_error 'trap on ERR not getting triggered'

expect=""
actual=$(
    function fun
    {
        $bin_false && echo FAILED
    }
    : works if this line deleted : |
    fun
    : works if this line deleted :
)
[[ $actual == $expect ]] || log_error "pipe to function with conditional fails" "$expect" "$actual"
actual=$(
    : works if this line deleted : |
    { $bin_false && echo FAILED; }
    : works if this line deleted :
)
[[ $actual == $expect ]] || log_error "pipe to { ... } with conditional fails" "$expect" "$actual"

actual=$(
    : works if this line deleted : |
    ( $bin_false && echo FAILED )
    : works if this line deleted :
)
[[ $actual == $expect ]] || log_error "pipe to ( ... ) with conditional fails" "$expect" "$actual"

( $SHELL -c 'trap : DEBUG; x=( $foo); exit 0') 2> /dev/null  || log_error 'trap DEBUG fails'

set -o pipefail
float start=$SECONDS end
for ((i=0; i < 2; i++))
do
    print foo
    sleep 1.5
done | { read; $bin_true; end=$SECONDS ;}
(( (SECONDS-start) < 1 )) && log_error "pipefail not waiting for pipe to finish"
set +o pipefail
(( (SECONDS-end) > 2 )) &&  log_error "pipefail causing $bin_true to wait for other end of pipe"


{ env A__z=C+SHLVL $SHELL -c : ;} 2> /dev/null || log_error "SHLVL with wrong attribute fails"

float t0=SECONDS
{ time sleep 1.5 | $bin_true ;} 2> /dev/null
(( (SECONDS-t0) < 1 )) && log_error 'time not waiting for pipeline to complete'

cat > $TEST_DIR/foo.sh <<- \EOF
    eval "cat > /dev/null  < /dev/null"
    sleep 1
EOF
float sec=SECONDS
. $TEST_DIR/foo.sh  | cat > /dev/null
(( (SECONDS-sec) < .7 ))  && log_error '. script does not restore output redirection with eval'

file=$TEST_DIR/foobar
for ((n=0; n < 1000; n++))
do
    > $file
    { sleep .001;echo $? >$file;} | cat > /dev/null
    if [[ !  -s $file ]]
    then
        log_error 'output from pipe is lost with pipe to builtin'
        break;
    fi

done


$SHELL -c 'kill -0 123456789123456789123456789' 2> /dev/null && log_error 'kill not catching process id overflows'

[[ $($SHELL -c '{ cd..; print ok;}' 2> /dev/null) == ok ]] || log_error 'command name ending in .. causes shell to abort'

$SHELL -xc '$(LD_LIBRARY_PATH=$LD_LIBRARY_PATH exec $SHELL -c :)' > /dev/null 2>&1  || log_error "ksh -xc '(name=value exec ksh)' fails with err=$?"

$SHELL 2> /dev/null -c $'for i;\ndo :;done' || log_error 'for i ; <newline> not vaid'

# RHBZ#1117316
set +o pipefail
foo=`false | true`
[[ $? -eq 0 ]] || log_error "Incorrect exit status from command substitution"
