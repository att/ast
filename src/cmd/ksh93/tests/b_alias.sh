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

alias foo='print hello'
if [[ $(foo) != hello ]]
then
    log_error 'foo, where foo is alias for "print hello" failed'
fi

if [[ $(foo world) != 'hello world' ]]
then
    log_error 'foo world, where foo is alias for "print hello" failed'
fi

alias foo='print hello '
alias bar=world
if [[ $(foo bar) != 'hello world' ]]
then
    log_error 'foo bar, where foo is alias for "print hello " failed'
fi

if [[ $(foo \bar) != 'hello bar' ]]
then
    log_error 'foo \bar, where foo is alias for "print hello " failed'
fi

alias bar='foo world'
if [[ $(bar) != 'hello world' ]]
then
    log_error 'bar, where bar is alias for "foo world" failed'
fi

if [[ $(alias bar) != "bar='foo world'" ]]
then
    log_error 'alias bar, where bar is alias for "foo world" failed'
fi

unalias foo  || log_error  "unalias foo failed"
alias foo 2> /dev/null  && log_error "alias for non-existent alias foo returns true"
unset bar
alias bar="print foo$bar"
bar=bar
if [[ $(bar) != foo ]]
then
    log_error 'alias bar, where bar is alias for "print foo$bar" failed'
fi

unset bar
alias bar='print hello'
if [[ $bar != '' ]]
then
    log_error 'alias bar cause variable bar to be set'
fi

alias !!=print
if [[ $(!! hello 2>/dev/null) != hello ]]
then
    log_error 'alias for !!=print not working'
fi

alias foo=echo
if [[ $(print  "$(foo bar)" ) != bar  ]]
then
    log_error 'alias in command substitution not working'
fi

(unalias foo)
if [[ $(foo bar 2> /dev/null)  != bar  ]]
then
    log_error 'alias not working after unalias in subshell'
fi

builtin -d rm 2> /dev/null
if whence rm > /dev/null
then
    [[ ! $(alias -t | grep rm= ) ]] && log_error 'tracked alias not set'
    PATH=$PATH
    [[ $(alias -t | grep rm= ) ]] && log_error 'tracked alias not cleared'
fi

if hash -r && [[ ! $(hash) ]]
then
    PATH=$TEST_DIR:/bin:/usr/bin
    for i in foo -foo --
    do
        print ':' > $TEST_DIR/$i
        chmod +x $TEST_DIR/$i
        hash -r -- $i || log_error "hash -r -- $i failed with wrong status" 0 $?
        expect="$i=$TEST_DIR/$i"
        actual="$(hash)"
        [[ $actual == $expect ]] || log_error "hash -r -- $i failed" "$expect" "$actual"
    done
else
    log_error 'hash -r failed' '' "$(hash)"
fi

( alias :pr=print) 2> /dev/null || log_error 'alias beginning with : fails'
( alias p:r=print) 2> /dev/null || log_error 'alias with : in name fails'

unalias no_such_alias &&  log_error 'unalias should return non-zero for unknown alias'

for i in compound float integer nameref
do
    [[ $i=$(whence $i) == "$(alias $i)" ]] || log_error "whence $i not matching $(alias $i)"
done

# TODO: alias -t and -x are obsolete options. Shall we add test case for them ?
# =======
# Ensure we have at least one alias to test alias -p
alias foo=bar
alias -p | grep -qv "^alias" && log_error "alias -p does not prepend every line with 'alias'"

# =======
# This should remove foo alias
unalias foo
alias | grep -q "^foo" && log_error "unalias does not remove aliases"

# =======
# This should remove all aliases
unalias -a
[[ $(alias) == "" ]] || log_error "unalias -a does not remove all aliases"

# =======
# https://github.com/att/ast/issues/909
alias foo=bar
unalias foo
unalias foo && log_error "Unaliasing undefined alias should exit with non-zero status"
