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

type /xxxxxx > out1 2> out2
[[ -s out1 ]] && log_error 'type should not write on stdout for not found case'
[[ -s out2 ]] || log_error 'type should write on stderr for not found case'
mkdir dir1 dir2
cat  > dir1/foobar << '+++'
foobar() { print foobar1;}
function dir1 { print dir1;}
+++
cat  > dir2/foobar << '+++'
foobar() { print foobar2;}
function dir2 { print dir2;}
+++
chmod +x dir[12]/foobar
p=$PATH
FPATH=$PWD/dir1
PATH=$FPATH:$p
expect=foobar1
actual=$( foobar)
[[ $actual == $expect ]] || log_error 'foobar output wrong' "$expect" "$actual"
FPATH=$PWD/dir2
PATH=$FPATH:$p
[[ $(foobar) == foobar2 ]] || log_error 'foobar output wrong' "$expect" "$actual"
FPATH=$PWD/dir1
PATH=$FPATH:$p
[[ $(foobar) == foobar1 ]] || log_error 'foobar should output foobar1 again'
FPATH=$PWD/dir2
PATH=$FPATH:$p
[[ ${ foobar;} == foobar2 ]] || log_error 'foobar should output foobar2 with ${}'
[[ ${ dir2;} == dir2 ]] || log_error 'should be dir2'
[[ ${ dir1;} == dir1 ]] 2> /dev/null &&  log_error 'should not be be dir1'
FPATH=$PWD/dir1
PATH=$FPATH:$p
[[ ${ foobar;} == foobar1 ]] || log_error 'foobar should output foobar1 with ${}'
[[ ${ dir1;} == dir1 ]] || log_error 'should be dir1'
[[ ${ dir2;} == dir2 ]] 2> /dev/null &&  log_error 'should not be be dir2'
FPATH=$PWD/dir2
PATH=$FPATH:$p
[[ ${ foobar;} == foobar2 ]] || log_error 'foobar should output foobar2 with ${} again'
PATH=$p
(PATH="/bin")
[[ $($SHELL -c 'print -r -- "$PATH"') == "$PATH" ]] || log_error 'export PATH lost in subshell'
cat > bug1 <<- EOF
	print print ok > $TEST_DIR/ok
	chmod 755 $TEST_DIR/ok
	function a
	{
		typeset -x PATH=$TEST_DIR
		ok
	}
	path=\$PATH
	unset PATH
	a
	PATH=\$path
}
EOF
[[ $($SHELL ./bug1 2>/dev/null) == ok ]] || log_error "PATH in function not working"

cat > bug1 <<- \EOF
	function lock_unlock
	{
		typeset PATH=/usr/bin
		typeset -x PATH=''
	}
	PATH=/usr/bin
	typeset -ft lock_unlock
	lock_unlock
EOF
($SHELL ./bug1) || log_error "path_delete bug"
mkdir tdir
if $SHELL tdir > /dev/null 2>&1
then
    log_error 'not an error to run ksh on a directory'
fi

print 'print hi' > ls
if [[ $($SHELL ls 2> /dev/null) != hi ]]
then
    log_error "$SHELL name not executing version in current directory"
fi

if [[ $(ls -d . 2>/dev/null) == . && $(PATH=/bin:/usr/bin:$PATH ls -d . 2>/dev/null) != . ]]
then
    log_error 'PATH export in command substitution not working'
fi

pwd=$PWD
# get rid of leading and trailing : and trailing :.
PATH=${PATH%.}
PATH=${PATH%:}
PATH=${PATH#.}
PATH=${PATH#:}
var=$(whence date)
dir=$(basename "$var")
for i in 1 2 3 4 5 6 7 8 9 0
do
    if ! whence notfound$i 2> /dev/null
    then
        cmd=notfound$i
        break
    fi
done

print 'print hello' > date
chmod +x date
print 'print notfound' >  $cmd
chmod +x "$cmd"
> foo
chmod 755 foo
for PATH in "$FULL_PATH" ":$FULL_PATH" "$FULL_PATH:" ".:$FULL_PATH" "$FULL_PATH:" "$FULL_PATH:." "$PWD::$FULL_PATH" "$PWD:.:$FULL_PATH" "$FULL_PATH:$PWD" "$FULL_PATH:.:$PWD"
do
        date
        "$cmd"
done > /dev/null 2>&1
builtin -d date 2> /dev/null
if [[ $(PATH=:/usr/bin; date) != 'hello' ]]
then
    log_error "leading : in path not working"
fi

# POSIX: If a command is found, but is not executable, exit status should be 126.
(
    rm -rf noexec
    print 'print cannot execute' > noexec
    noexec > /dev/null 2>&1
)
actual=$?
expect=126
[[ $actual == $expect ]] || log_error "exit status of non-executable is wrong" "$expect" "$actual"

builtin -d rm 2> /dev/null
d=$(dirname "$bin_rm")

for cmd in date foo
do
    exp="$cmd found"
    print print $exp > $cmd
    chmod +x $cmd
    got=$($SHELL -c "unset FPATH; PATH=/dev/null; $cmd" 2>&1)
    [[ $got == $exp ]] && log_error "$cmd as last command should not find ./$cmd with PATH=/dev/null"
    got=$($SHELL -c "unset FPATH; PATH=/dev/null; $cmd" 2>&1)
    [[ $got == $exp ]] && log_error "$cmd should not find ./$cmd with PATH=/dev/null"
    exp=$PWD/./$cmd
    got=$(unset FPATH; PATH=/dev/null; whence ./$cmd)
    [[ $got == $exp ]] || log_error "whence $cmd should find ./$cmd with PATH=/dev/null"
    exp=$PWD/$cmd
    got=$(unset FPATH; PATH=/dev/null; whence $PWD/$cmd)
    [[ $got == $exp ]] || log_error "whence \$PWD/$cmd should find ./$cmd with PATH=/dev/null"
done

exp=''
got=$($SHELL -c "unset FPATH; PATH=/dev/null; whence ./notfound" 2>&1)
[[ $got == $exp ]] || log_error "whence ./$cmd failed -- expected '$exp', got '$got'"
got=$($SHELL -c "unset FPATH; PATH=/dev/null; whence $PWD/notfound" 2>&1)
[[ $got == $exp ]] || log_error "whence \$PWD/$cmd failed -- expected '$exp', got '$got'"

unset FPATH
PATH=/dev/null
for cmd in date foo
do
    exp="$cmd found"
    print print $exp > $cmd
    $bin_chmod +x $cmd
    got=$($cmd 2>&1)
    [[ $got == $exp ]] && log_error "$cmd as last command should not find ./$cmd with PATH=/dev/null"
    got=$($cmd 2>&1; :)
    [[ $got == $exp ]] && log_error "$cmd should not find ./$cmd with PATH=/dev/null"
    exp=$PWD/./$cmd
    got=$(whence ./$cmd)
    [[ $got == $exp ]] || log_error "whence ./$cmd should find ./$cmd with PATH=/dev/null"
    exp=$PWD/$cmd
    got=$(whence $PWD/$cmd)
    [[ $got == $exp ]] || log_error "whence \$PWD/$cmd should find ./$cmd with PATH=/dev/null"
done
exp=''
got=$(whence ./notfound)
[[ $got == $exp ]] || log_error "whence ./$cmd failed -- expected '$exp', got '$got'"
got=$(whence $PWD/notfound)
[[ $got == $exp ]] || log_error "whence \$PWD/$cmd failed -- expected '$exp', got '$got'"

PATH=$d:.
cp "$bin_rm" kshrm
if [[ $(whence kshrm) != $PWD/kshrm  ]]
then
    log_error 'trailing : in pathname not working'
fi

cp "$bin_rm" rm
PATH=.:$d
if [[ $(whence rm) != $PWD/rm ]]
then
    log_error 'leading : in pathname not working'
fi

PATH=$d:. whence rm > /dev/null
if [[ $(whence rm) != $PWD/rm ]]
then
    log_error 'pathname not restored after scoping'
fi
rm rm

mkdir bin
print 'print ok' > bin/tst
chmod +x bin/tst
if [[ $(PATH=$PWD/bin tst 2>/dev/null) != ok ]]
then
    log_error '(PATH=$PWD/bin foo) does not find $PWD/bin/foo'
fi

cd /
if whence ls > /dev/null
then
    PATH=
    if [[ $(whence rm) ]]
    then
        log_error 'setting PATH to Null not working'
    fi

    unset PATH
    if [[ $(whence rm) != /*rm ]]
    then
        log_error 'unsetting path  not working'
    fi

fi

MINIMAL_PATH="$(getconf PATH)"
PATH=/dev:$TEST_DIR
x=$(whence rm)
typeset foo=$(PATH=/xyz:/abc :)
y=$(whence rm)
[[ $x != "$y" ]] && log_error 'PATH not restored after command substitution'
PATH="$MINIMAL_PATH"
x=$(whence ls)
PATH=.:$PWD:${x%/ls}
[[ $(whence ls) == "$x" ]] || log_error 'PATH search bug when .:$PWD in path'
PATH=$PWD:.:${x%/ls}
[[ $(whence ls) == "$x" ]] || log_error 'PATH search bug when :$PWD:. in path'
cd   "${x%/ls}"
[[ $(whence ls) == /* ]] || log_error 'whence not generating absolute pathname'
status=$($SHELL -c $'trap \'print $?\' EXIT;/xxx/a/b/c/d/e 2> /dev/null')
[[ $status == 127 ]] || log_error "not found command exit status $status -- expected 127"
status=$($SHELL -c $'trap \'print $?\' EXIT;/dev/null 2> /dev/null')
[[ $status == 126 ]] || log_error "non executable command exit status $status -- expected 126"
status=$($SHELL -c $'trap \'print $?\' ERR;/xxx/a/b/c/d/e 2> /dev/null')
[[ $status == 127 ]] || log_error "not found command with ERR trap exit status $status -- expected 127"
status=$($SHELL -c $'trap \'print $?\' ERR;/dev/null 2> /dev/null')
[[ $status == 126 ]] || log_error "non executable command ERR trap exit status $status -- expected 126"

PATH=$FULL_PATH

scr=$TEST_DIR/script
exp=126

# Cygwin file permission behavior causes these corner cases to fail. So skip these tests on Cygwin.
# See https://github.com/att/ast/issues/1281
#
# TODO: Figure out if there is some way to make ksh behave on Cygwin like it does on real UNIX/POSIX
# compliant systems for these edge cases without greatly complicating the code.
if [[ $OS_NAME != cygwin* ]]
then

: > $scr
chmod a=x $scr
{ got=$($scr; print $?); } 2>/dev/null
[[ "$got" == "$exp" ]] ||
    log_error "unreadable empty script should fail" "$exp" "$got"
{ got=$(command $scr; print $?); } 2>/dev/null
[[ "$got" == "$exp" ]] ||
    log_error "command of unreadable empty script should fail" "$exp" "$got"
got="$(:; $scr; print $?)"
[[ "$got" == "$exp" ]] 2>/dev/null ||
    log_error "unreadable empty script in [[ ... ]] should fail" "$exp" "$got"
got="$(:; command $scr; print $?)"
[[ "$got" == "$exp" ]] 2>/dev/null ||
    log_error "command unreadable empty script in [[ ... ]] should fail" "$exp" "$got"
got=$($SHELL -c "$scr; print \$?" 2>/dev/null)
[[ "$got" == "$exp" ]] ||
    log_error "\$SHELL -c of unreadable empty script should fail" "$exp" "$got"
got=$($SHELL -c "command $scr; print \$?" 2>/dev/null)
[[ "$got" == "$exp" ]] ||
    log_error "\$SHELL -c of command of unreadable empty script should fail" "$exp" "$got"

rm -f $scr
print : > $scr
chmod a=x $scr
{ got=$($scr; print $?); } 2>/dev/null
[[ "$got" == "$exp" ]] ||
    log_error "unreadable non-empty script should fail" "$exp" "$got"
{ got=$(command $scr; print $?); } 2>/dev/null
[[ "$got" == "$exp" ]] ||
    log_error "command of unreadable non-empty script should fail" "$exp" "$got"
got="$(:; $scr; print $?)" 2>/dev/null
[[ "$got" == "$exp" ]] ||
    log_error "unreadable non-empty script in [[ ... ]] should fail" "$exp" "$got"
got="$(:; command $scr; print $?)" 2>/dev/null
[[ "$got" == "$exp" ]] ||
    log_error "command unreadable non-empty script in [[ ... ]] should fail" "$exp" "$got"
got=$($SHELL -c "$scr; print \$?" 2>/dev/null)
[[ "$got" == "$exp" ]] ||
    log_error "\$SHELL -c of unreadable non-empty script should fail" "$exp" "$got"
got=$($SHELL -c "command $scr; print \$?" 2>/dev/null)
[[ "$got" == "$exp" ]] ||
    log_error "\$SHELL -c of command of unreadable non-empty script should fail" "$exp" "$got"

fi  # [[ $OS_NAME != cygwin* ]]

# whence -a bug fix
cd "$TEST_DIR"
ifs=$IFS
IFS=$'\n'
PATH=$PATH:
> ls
chmod +x ls
ok=
for i in $(whence -a ls)
do
    if [[ $i == *"$PWD/ls" ]]
    then
        ok=1
        break;
    fi

done
[[ $ok ]] || log_error 'whence -a not finding all executables'
rm -f ls
PATH=${PATH%:}

#whence -p bug fix
function foo
{
    :
}
[[ $(whence -p foo) == foo ]] && log_error 'whence -p foo should not find function foo'

# whence -q bug fix
$SHELL -c 'whence -q cat' & pid=$!
sleep 3
kill $! 2> /dev/null && log_error 'whence -q appears to be hung'

FPATH=$PWD
print  'function foobar { :;}' > foobar
autoload foobar;
exec {m}< /dev/null
for ((i=0; i < 25; i++))
do
    ( foobar )
done
exec {m}<& -
exec {n}< /dev/null
(( n > m )) && log_error 'autoload function in subshell leaves file open'

# whence -a bug fix
rmdir=rmdir
mkdir $rmdir || { log_warning "failed to create '$rmdir'"; exit 99; }
cp "$bin_rm" "$rmdir"
{ PATH=:${bin_rm%/rm} $SHELL -c "cd \"$rmdir\";whence -a rm";} > /dev/null 2>&1
exitval=$?
(( exitval==0 )) || log_error "whence -a has exitval $exitval"

[[ ! -d bin ]] && mkdir bin
[[ ! -d fun ]] && mkdir fun
print 'FPATH=../fun' > bin/.paths
cat <<- \EOF > fun/myfun
	function myfun
	{
		print myfun ran
	}
EOF
# TODO: Use the first statement rather than the second once https://github.com/att/ast/issues/1400
# is resolved. The problem is that if an external command is run in the context of executing the
# `typeset -fu` command in ${.sh.install_prefix}/config.ksh script it causes the .paths mechanism to
# fail. The second variant works around that bug by forcing the shell to re-evaluate paths that have
# .paths files (or something of that nature).
#
# actual=$(FPATH= PATH=$PWD/bin:$PATH $SHELL -c 'myfun')
actual=$(FPATH= $SHELL -c 'PATH=$PWD/bin:$PATH; myfun')
expect='myfun ran'
[[ $actual == $expect ]] || log_error 'function myfun not found' "$expect" "$actual"

cp $bin_echo user_to_group_relationship.hdr.query
FPATH=/foobar:
PATH=$FPATH:$PATH:.
[[ $(user_to_group_relationship.hdr.query foobar) == foobar ]] 2> /dev/null || log_error 'Cannot execute command with . in name when PATH and FPATH end in :.'

mkdir -p $TEST_DIR/new/bin
mkdir $TEST_DIR/new/fun
print FPATH=../fun > $TEST_DIR/new/bin/.paths
print FPATH=../xxfun > $TEST_DIR/bin/.paths
cp "$bin_echo" $TEST_DIR/new/bin
PATH=$TEST_DIR/bin:$TEST_DIR/new/bin:$PATH
x=$(whence -p echo 2> /dev/null)
[[ $x == "$TEST_DIR/new/bin/echo" ]] ||  log_error 'nonexistant FPATH directory in .paths file causes path search to fail'

$SHELL 2> /dev/null <<- \EOF || log_error 'path search problem with non-existant directories in PATH'
	PATH=/usr/nogood1/bin:/usr/nogood2/bin:/bin:/usr/bin
	tail /dev/null && tail /dev/null
EOF

( PATH=/bin:usr/bin
cat << END >/dev/null 2>&1
${.sh.version}
END
) || log_error '${.sh.xxx} variables causes cat not be found'

PATH=/bin:/usr/bin
if [[ $(type date) == *builtin* ]]
then
    builtin -d date
    [[ $(type date) == *builtin* ]] && log_error 'builtin -d does not delete builtin'
fi

# https://github.com/att/ast/issues/757
OPATH="$PATH"
PATH=".:$PATH"
mkdir cat || log_error "mkdir cat failed"
cat < /dev/null || log_error "Directories should not be treated as executables"

# Restore PATH
PATH="$OPATH"
