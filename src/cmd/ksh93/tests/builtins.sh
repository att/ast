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

# test shell builtin commands
: ${foo=bar} || log_error ": failed"
[[ $foo == bar ]] || log_error ": side effects failed"

false ${foo=bar} &&  log_error "false failed"

if [[ $(print -R -) != - ]]
then
    log_error "print -R not working correctly"
fi

if [[ $(print -- -) != - ]]
then
    log_error "print -- not working correctly"
fi

print -n -u2 2>&1-
[[ -w /dev/fd/1 ]] || log_error "2<&1- with built-ins has side effects"
x=$0
if [[ $(eval 'print $0') != $x ]]
then
    log_error '$0 not correct for eval'
fi

$SHELL -c 'read x <<< hello' 2> /dev/null || log_error 'syntax <<< not recognized'
unset x
readonly x
set -- $(readonly)
if   [[ " $@ " != *" x "* ]]
then
    log_error 'unset readonly variables are not displayed'
fi

if [[ $(    for i in foo bar
        do
            print $i
            continue 10
        done
        ) != $'foo\nbar' ]]
then
    log_error 'continue breaks out of loop'
fi

(continue bad 2>/dev/null && log_error 'continue bad should return an error')
(break bad 2>/dev/null && log_error 'break bad should return an error')
(continue 0 2>/dev/null && log_error 'continue 0 should return an error')
(break 0 2>/dev/null && log_error 'break 0 should return an error')
breakfun() { break;}
continuefun() { continue;}
for fun in break continue
do
    if [[ $(    for i in foo
            do
                ${fun}fun
                print $i
            done
        ) != foo ]]
    then
        log_error "$fun call in ${fun}fun breaks out of for loop"
    fi
done

[[ $(for ((i=0;i<10;i++))
do
    for ((j=0;j<10;j++))
    do
        echo $i $j
        break 2
    done
done) == "0 0" ]] || log_error "break [n] does not break to outer loop"

[[ $(for ((i=0;i<2;i++))
do
    for ((j=0;j<10;j++))
    do
        echo $i $j
        continue 2
    done
done) == $'0 0\n1 0' ]] || log_error "continue [n] does not continue to outer loop"

if [[ $(print -f "%b" "\a\n\v\b\r\f\E\03\\oo") != $'\a\n\v\b\r\f\E\03\\oo' ]]
then
    log_error 'print -f "%b" not working'
fi

if [[ $(print -f "%P" "[^x].*b\$") != '*[!x]*b' ]]
then
    log_error 'print -f "%P" not working'
fi

if [[ $(print -f "%(pattern)q" "[^x].*b\$") != '*[!x]*b' ]]
then
    log_error 'print -f "%(pattern)q" not working'
fi

if [[ $(abc: for i in foo bar;do print $i;break abc;done) != foo ]]
then
    log_error 'break labels not working'
fi

[[ $(echo -n hello) = "hello" ]] || log_error "echo -n should not print a newline"
[[ $(echo -e "hello\tworld") = $'hello\tworld' ]] || log_error "echo -e should interpret escape sequences"

mkdir -p $TEST_DIR/a/b/c 2>/dev/null || log_error  "mkdir -p failed"
$SHELL -c "cd $TEST_DIR/a/b; cd c" || log_error "initial script relative cd fails"

# The `| sort` is because ksh doesn't guarantee the order of the output of the `trap` command.
expect="trap -- 'print TERM' TERM trap -- 'print USR1' USR1"
actual=$(echo $($SHELL -c 'trap "print TERM" TERM; trap "print USR1" USR1; trap' | sort) )
[[ $actual == $expect ]] || log_error "'trap' failed" "$expect" "$actual"

expect='print TERM'
actual=$(echo $($SHELL -c 'trap "print TERM" TERM; trap "print USR1" USR1; trap -p TERM') )
[[ $actual == $expect ]] || log_error "'trap -p TERM' failed" "$expect" "$actual"

[[ $($SHELL -c 'trap "print ok" SIGTERM; kill -s SIGTERM $$' 2> /dev/null) == ok ]] || log_error 'SIGTERM not recognized'
[[ $($SHELL -c 'trap "print ok" sigterm; kill -s sigterm $$' 2> /dev/null) == ok ]] || log_error 'SIGTERM not recognized'
[[ $($SHELL -c '( trap "" TERM);kill $$;print bad' == bad) ]] 2> /dev/null && log_error 'trap ignored in subshell causes it to be ignored by parent'

if [[ $( trap 'print done' EXIT) != done ]]
then
    log_error 'trap on EXIT not working'
fi

if [[ $( trap 'print done' EXIT; trap - EXIT) == done ]]
then
    log_error 'trap on EXIT not being cleared'
fi

if [[ $(LC_MESSAGES=C type test) != 'test is a shell builtin' ]]
then
    log_error 'whence -v test not a builtin'
fi

builtin -d test
if [[ $(type test) == *builtin* ]]
then
    log_error 'whence -v test after builtin -d incorrect'
fi

if [[ $(trap --version 2> /dev/null;print done) != done ]]
then
    log_error 'trap builtin terminating after --version'
fi

if [[ $(set --version 2> /dev/null;print done) != done ]]
then
    log_error 'set builtin terminating after --veresion'
fi


set -f
set -- *
if   [[ $1 != '*' ]]
then
    log_error 'set -f not working'
fi

env=
v=$(getconf LIBPATH 2> /dev/null)
for v in ${v//,/ }
do
    v=${v#*:}
    v=${v%%:*}
    eval [[ \$$v ]] && env="$env $v=\"\$$v\""
done

if [[ $(foo=bar; eval foo=\$foo $env exec -c \$SHELL -c \'print \$foo\') != bar ]]
then
    log_error '"name=value exec -c ..." not working'
fi

#(print -n a;sleep 1; print -n bcde) | { read -N3 a; read -N1 b;}
#[[ $a == $exp ]] || log_error "read -N3 from pipe failed -- expected '$exp', got '$a'"
#exp=d
#[[ $b == $exp ]] || log_error "read -N1 from pipe failed -- expected '$exp', got '$b'"
#(print -n a;sleep 1; print -n bcde) | read -n3 a
#exp=a
#[[ $a == $exp ]] || log_error "read -n3 from pipe failed -- expected '$exp', got '$a'"
#rm -f $TEST_DIR/fifo
#if mkfifo $TEST_DIR/fifo 2> /dev/null
#then
#    (print -n a; sleep 1;print -n bcde)  > $TEST_DIR/fifo &
#    {
#    read -u5 -n3 -t2 a || log_error 'read -n3 from fifo timedout'
#    read -u5 -n1 -t2 b || log_error 'read -n1 from fifo timedout'
#    } 5< $TEST_DIR/fifo
#    exp=a
#    [[ $a == $exp ]] || log_error "read -n3 from fifo failed -- expected '$exp', got '$a'"
#    rm -f $TEST_DIR/fifo
#    mkfifo $TEST_DIR/fifo 2> /dev/null
#    (print -n a; sleep 1;print -n bcde) > $TEST_DIR/fifo &
#    {
#    read -u5 -N3 -t2 a || log_error 'read -N3 from fifo timed out'
#    read -u5 -N1 -t2 b || log_error 'read -N1 from fifo timedout'
#    } 5< $TEST_DIR/fifo
#    exp=abc
#    [[ $a == $exp ]] || log_error "read -N3 from fifo failed -- expected '$exp', got '$a'"
#    exp=d
#    [[ $b == $exp ]] || log_error "read -N1 from fifo failed -- expected '$exp', got '$b'"
#fi

#rm -f $TEST_DIR/fifo

# test for debug trap
[[ $(typeset -i i=0
    trap 'print $i' DEBUG
    while (( i <2))
    do
        (( i++))
    done) == $'0\n0\n1\n1\n2' ]]  || log_error  "DEBUG trap not working"

$SHELL -c 'sleep $(printf "%a" .95)' 2> /dev/null || log_error "sleep doesn't except %a format constants"
$SHELL -c 'test \( ! -e \)' 2> /dev/null ; [[ $? == 1 ]] || log_error 'test \( ! -e \) not working'
[[ $(ulimit) == "$(ulimit -fS)" ]] || log_error 'ulimit is not the same as ulimit -fS'

tmpfile=$TEST_DIR/file.2
print $'\nprint -r -- "${.sh.file} ${LINENO} ${.sh.lineno}"' > $tmpfile
actual="$(. $tmpfile)"
expect="$tmpfile 2 1"
[[ $actual == $expect ]] || log_error 'dot command not working' "$expect" "$actual"

tmpfile=$TEST_DIR/file.3
print -r -- "'xxx" > $tmpfile
actual="$($SHELL -c ". $tmpfile"$'\n print ok' 2> /dev/null)"
expect="ok"
[[ $actual == $expect ]] ||
    log_error 'syntax error in dot command affects next command' "$expect" "$actual"

typeset -r z=3
y=5
for i in 123 z  %x a.b.c
do
    ( unset $i)  2>/dev/null && log_error "unset $i should fail"
done

a=()
for i in y y  y[8] t[abc] y.d a.b  a
do
    unset $i ||  print -u2  "log_error unset $i should not fail"
done

[[ $($SHELL -c 'y=3; unset 123 y;print $?$y') == 1 ]] 2> /dev/null ||  log_error 'y is not getting unset with unset 123 y'
[[ $($SHELL -c 'trap foo TERM; (trap;(trap) )') == 'trap -- foo TERM' ]] || log_error 'traps not getting reset when subshell is last process'

t=$(ulimit -t)
[[ $($SHELL -c 'ulimit -v 15000 2>/dev/null; ulimit -t') == "$t" ]] || log_error 'ulimit -v changes ulimit -t'

$SHELL 2> /dev/null -c 'cd ""' && log_error 'cd "" not producing an error'
[[ $($SHELL 2> /dev/null -c 'cd "";print hi') != hi ]] && log_error 'cd "" should not terminate script'

bincat=$(whence -p cat)
builtin cat
out=$TEST_DIR/seq.out
seq 11 >$out
cmp -s <(print -- "$($bincat<( $bincat $out ) )") <(print -- "$(cat <( cat $out ) )") || log_error "builtin cat differs from $bincat"

v=$( $SHELL -c $'
    trap \'print "usr1"\' USR1
    trap exit USR2
    sleep 1 && {
        kill -USR1 $$ && sleep 1
        kill -0 $$ 2>/dev/null && kill -USR2 $$
    } &
    sleep 2 | read
    echo done

' ) 2> /dev/null
[[ $v == $'usr1\ndone' ]] ||  log_error 'read not terminating when receiving USR1 signal'

mkdir $TEST_DIR/tmpdir1
cd $TEST_DIR/tmpdir1
pwd=$PWD
cd ../tmpdir1
[[ $PWD == "$pwd" ]] || log_error 'cd ../tmpdir1 causes directory to change'
cd "$pwd"
mv $TEST_DIR/tmpdir1 $TEST_DIR/tmpdir2
cd ..  2> /dev/null || log_error 'cannot change directory to .. after current directory has been renamed'
[[ $PWD == "$TEST_DIR" ]] || log_error 'after "cd $TEST_DIR/tmpdir1; cd .." directory is not $TEST_DIR'

cd "$TEST_DIR"
mkdir $TEST_DIR/tmpdir2/foo
pwd=$PWD
cd $TEST_DIR/tmpdir2/foo
mv $TEST_DIR/tmpdir2 $TEST_DIR/tmpdir1
cd ../.. 2> /dev/null || log_error 'cannot change directory to ../.. after current directory has been renamed'
[[ $PWD == "$TEST_DIR" ]] || log_error 'after "cd $TEST_DIR/tmpdir2; cd ../.." directory is not $TEST_DIR'
cd "$TEST_DIR"
rm -rf tmpdir1

cd /etc
cd ..
[[ $(pwd) == / ]] || log_error 'cd /etc;cd ..;pwd is not /'
cd /etc
cd ../..
[[ $(pwd) == / ]] || log_error 'cd /etc;cd ../..;pwd is not /'
cd /etc
cd .././..
[[ $(pwd) == / ]] || log_error 'cd /etc;cd .././..;pwd is not /'
cd /usr/bin
cd ../..
[[ $(pwd) == / ]] || log_error 'cd /usr/bin;cd ../..;pwd is not /'
cd /usr/bin
cd ..
[[ $(pwd) == /usr ]] || log_error 'cd /usr/bin;cd ..;pwd is not /usr'
cd "$TEST_DIR"
if mkdir $TEST_DIR/t1
then
    (
        cd $TEST_DIR/t1
        > real_t1
        (
            cd ..
            mv t1 t2
            mkdir t1
        )
        [[ -f real_t1 ]] || log_error 'real_t1 not found after parent directory renamed in subshell'
    )
fi

cd "$TEST_DIR"

> foobar
CDPATH= $SHELL 2> /dev/null -c 'cd foobar' && log_error "cd to a regular file should fail"

cd "$TEST_DIR"
mkdir foo .bar
cd foo
cd ../.bar 2> /dev/null || log_error 'cd ../.bar when ../.bar exists should not fail'

$SHELL +E -i <<- \! && log_error 'interactive shell should not exit 0 after false'
    false
    exit
!

unset ENV

: ~root
[[ $(builtin) == *.sh.tilde* ]] &&  log_error 'builtin contains .sh.tilde'

PATH=/bin:/usr/bin
basename=$(whence -p basename)
cmp=$(whence -p cmp)
.sh.op_astbin=/opt/ast/bin
PATH=/opt/ast/bin:$PATH
PATH=/opt/ast/bin:/bin:/usr/bin
[[ ${SH_OPTIONS} == *astbin=/opt/ast/bin* ]] || log_error "SH_OPTIONS=${SH_OPTIONS} but should contain astbin=/opt/ast/bin"
[[ $(whence basename) == /opt/ast/bin/basename ]] || log_error "basename bound to $(whence basename) but should be bound to /opt/ast/bin/basename"
[[ $(whence cmp) == /opt/ast/bin/cmp ]] || log_error "cmp bound to $(whence cmp) but should be bound to /opt/ast/bin/cmp"
.sh.op_astbin=/bin
SH_OPTIONS=astbin=/bin
[[ ${SH_OPTIONS} == *astbin=/bin* ]] || log_error "SH_OPTIONS=${SH_OPTIONS} but should contain astbin=/bin"
[[ $(whence basename) == /bin/basename ]] || log_error "basename bound to $(whence basename) but should be bound to /bin/basename"
[[ $(whence cmp) == /bin/cmp ]] || log_error "cmp bound to $(whence cmp) but should be bound to /bin/cmp"
.sh.op_astbin=/opt/ast/bin
[[ $(whence basename) == /opt/ast/bin/basename ]] || log_error "basename bound to $(whence basename) but should be rebound to /opt/ast/bin/basename"
[[ $(whence cmp) == /opt/ast/bin/cmp ]] || log_error "cmp bound to $(whence cmp) but should be rebound to /opt/ast/bin/cmp"
PATH=/bin:/usr/bin:/opt/ast/bin
[[ $(whence basename) == "$basename" ]] || log_error "basename bound to $(whence basename) but should be bound to $basename when PATH=$PATH"
[[ $(whence cmp) == "$cmp" ]] || log_error "cmp bound to $(whence cmp) but should be bound to $cmp when PATH=$PATH"

# tests with cd
pwd=$PWD
exec {fd}</dev

cd /dev
if cd ~-
then
    [[ $PWD == "$pwd" ]] || log_error "directory is $PWD, should be $pwd"
else
    log_error "unable to cd ~- back to $pwd"
fi

if cd -f $fd
then
    [[ -r null ]] || log_error 'cannot find "null" file in /dev'
else
    log_error 'cannot cd to ~{fd} when fd is /dev'
fi

mkdir $TEST_DIR/oldpwd
OLDPWD=$TEST_DIR/oldpwd
cd - > $TEST_DIR/cd.out
actual=$(< $TEST_DIR/cd.out)
expect="$TEST_DIR/oldpwd"
[[ $actual == $expect ]] || log_error "cd - does not recognize overridden OLDPWD variable"
[[ $PWD == $expect ]] || log_error "cd - does not recognize overridden OLDPWD variable"

cd $TEST_DIR
[[ $(OLDPWD="$TEST_DIR/oldpwd" cd -) == "$TEST_DIR/oldpwd" ]] ||
    log_error "cd - does not recognize overridden OLDPWD variable if it is overridden in new scope"

[[ $(pwd -f $fd) == /dev ]] || log_error "pwd -f $fd should be /dev"

$SHELL <<- \EOF
    # $HOME is set to a temporary directory by test framework
    # Get actual home directory of the user
    home=~$USER
    unset HOME
    cd
    [[ $(pwd) == "$home" ]]
EOF
[[ $? == 0 ]] || log_error 'cd with no arguments fails if HOME is unset'

cd "$TEST_DIR"
if mkdir -p f1
then
    redirect {d}<f1
    pwd=$(pwd)
    ( cd -f $d && [[ $(pwd) == "$pwd/f1" ]]) || log_error '$(pwd) does not show new directory'
    [[ $(pwd) == "$pwd" ]] || log_error '$(pwd) is not $pwd'
    [[ $(/bin/pwd) == "$pwd" ]] || log_error  '/bin/pwd is not "$pwd"'
    [[ $(/bin/pwd) == "$(pwd)" ]] || log_error  '/bin/pwd is not pwd'
    cd "$pwd"
    rmdir "$pwd/f1"
fi

TESTDIRSYMLINK="$TEST_DIR/testdirsymlink"
ln -s "$TEST_DIR" "$TEST_DIR/testdirsymlink"

# TODO: cd -@
# Change into the hidden attribute directory of directory which may also be a file. CDPATH is
# ignored. Hidden attribute directories are file system and operating system specific.

cd -L "$TESTDIRSYMLINK"
actual="$(pwd)"
expected="$TEST_DIR/testdirsymlink"
[[ "$actual" = "$expected" ]] || log_error "cd -L should enter logical path" "$expected" "$actual"

# Enter physical path to skip resolving multiple symlinks while testing
cd -P "$TEST_DIR"
cd -P "$TESTDIRSYMLINK" || log_error "cd -P to symlink failed"
actual="$(pwd)"
expected="$OLDPWD"
[[ "$actual" = "$expected"  ]] || log_error "cd -P should enter physical path." "$expected" "$actual"

# Enter physical path to skip resolving multiple symlinks while testing
cd $(pwd -P)
cd "$TESTDIRSYMLINK"

actual="$(pwd)"
expected="$TEST_DIR/testdirsymlink"
[[ "$actual" = "$expected" ]] || log_error "pwd should print logical path" "$expected" "$actual"

actual="$(pwd -L)"
expected="$TEST_DIR/testdirsymlink"
[[ "$actual" = "$expected"  ]] || log_error "pwd -L should print logical path" "$expected" "$actual"

actual="$(pwd -P)"
expected="$OLDPWD"
[[ "$actual" = "$expected"  ]] || log_error "pwd -P should print physical path." "$expected" "$actual"

# test for eval bug when called from . script in a startup file.
print $'eval : foo\nprint ok' > $TEST_DIR/evalbug
print ". $TEST_DIR/evalbug" >$TEST_DIR/envfile
[[ $(ENV=$TEST_DIR/envfile $SHELL -i -c : 2> /dev/null) == ok ]] || log_error 'eval inside dot script called from profile file not working'

# test cd to a directory that doesn't have execute permission
if mkdir -p $TEST_DIR/a/b
then
    chmod -x $TEST_DIR/a/b
    cd $TEST_DIR/a/b 2> /dev/null && log_error 'cd to directory without execute should fail'
fi
chmod +x $TEST_DIR/a/b  # so the test temp dir can be removed when the test completes

# -s flag writes to history file
if print -s 'print hello world' 2> /dev/null
then
    [[ $(history -1) == *'hello world'* ]] || log_error 'history file does not contain result of print -s'
else
    log_error 'print -s fails'
fi

# Check if history file is updated correclty if entry does not end with newline
if print -s -f 'print foo' 2> /dev/null
then
    [[ $(history -1) == *'foo' ]] || log_error 'history file does not contain result of print -s -f'
else
    log_error 'print -s -f fails'
fi

builtin  -d set 2> /dev/null && log_error 'buitin -d allows special builtins to be deleted'

builtin -f $LIBSAMPLE_PATH sample || log_error "Failed to load sample builtin"

sample >/dev/null || log_error "Sample builtin should exit with 0 status"

# List special builtins.
# The locale affects the order of listing builtins.
expect=". : _Bool alias break continue enum eval exec exit export hash login newgrp readonly"
expect="$expect return set shift trap typeset unalias unset "
actual=$(LC_ALL=C builtin -s | tr '\n' ' ')
[[ "$actual" == "$expect" ]] ||
    log_error "builtin -s mismatches list of special builtins" "$expect" "$actual"

builtin -d alias 2>/dev/null && log_error "Deleting a special builtin should fail"

[[ $(builtin -p | grep -v "^builtin") = "" ]] || log_error "builtin -p does not prepend all lines with 'builtin'"

# The -p option causes the word export to be inserted before each one.
[[ $(export -p | grep -v "^export") = "" ]] || log_error "export -p does not prepend all lines with 'export'"
