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

if $SHELL -c '[[ ~root == /* ]]'
then
    x=$(print -r -- ~root)
    [[ $x == ~root ]] || log_error '~user expanded in subshell prevent ~user from working'
fi

function home # id
{
    typeset user=$1

    # On Cygwin on MS Windows we can't count on the Administrator account home dir existing.
    if [[ $OS_NAME == cygwin* && $user == Administrator ]]
    then
        print .
        return
    fi

    typeset IFS=: pwd=/etc/passwd
    set -o noglob
    if [[ -f $pwd ]] && grep -c "^$user:" $pwd > /dev/null
    then
    set -- $(grep "^$user:" $pwd)
        print -r -- "$6"
    else
        print .
    fi
}

OLDPWD=/bin
if [[ ~ != $HOME ]]
then
    log_error '~' not $HOME
fi

x=~
if [[ $x != $HOME ]]
then
    log_error x=~ not $HOME
fi

x=x:~
if [[ $x != x:$HOME ]]
then
    log_error x=x:~ not x:$HOME
fi

if [[ ~+ != $PWD ]]
then
    log_error '~' not $PWD
fi

x=~+
if [[ $x != $PWD ]]
then
    log_error x=~+ not $PWD
fi

if [[ ~- != $OLDPWD ]]
then
    log_error '~' not $PWD
fi

x=~-
if [[ $x != $OLDPWD ]]
then
    log_error x=~- not $OLDPWD
fi

for u in root Administrator
do
    # If we can't find the home dir for the account in the passwd file ignore it.
    expect=$(home $u)
    [[ $expect != '.' ]] || continue

    actual=~$u
    [[ $actual -ef $expect ]] || log_error "~$u not $h" "$expect" "$actual"
done

x=~g.r.emlin
if [[ $x != '~g.r.emlin' ]]
then
    log_error "x=~g.r.emlin failed -- expected '~g.r.emlin', got '$x'"
fi

x=~:~
if [[ $x != "$HOME:$HOME" ]]
then
    log_error "x=~:~ failed, expected '$HOME:$HOME', got '$x'"
fi

HOME=/
[[ ~ == / ]] || log_error '~ should be /'
[[ ~/foo == /foo ]] || log_error '~/foo should be /foo when ~==/'
print $'print ~+\n[[ $1 ]] && $0' > $TEST_DIR/tilde
chmod +x $TEST_DIR/tilde
nl=$'\n'
[[ $($TEST_DIR/tilde foo) == "$PWD$nl$PWD" ]] 2> /dev/null  || log_error 'tilde fails inside a script run by name'
