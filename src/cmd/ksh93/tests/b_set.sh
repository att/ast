# Tests for set builtin
# TODO: This test is not run through `shcomp` due to a bug in how `set -k` is handled in `shcomp`.

# ==========
# -s Sort the positional parameters.
set -- citrus banana apple
set -s
[[ $1 = apple ]] && [[ $2 = banana ]] && [[ $3 = citrus ]] ||
    log_error "set -s failed to sort positional parameters"

# ==========
# -A name Assign the arguments sequentially to the array named by name
#    starting at subscript 0 rather than to the positional parameters.
set -A foo bar baz
expect="typeset -a foo=(bar baz)"
actual=$(typeset -p foo)
[[ $actual = $expect ]] ||
    log_error "set -A does not create an array" "$expect" "$actual"
unset foo

# ==========
# -a Set the export attribute for each variable whose name does
#    not contain a . that you assign a value in the current shell
#    environment.
set -a
foo="export me!"
env | grep -q "^foo=export me" || log_error "set -a does not export variables"
set +a

# ==========
# -e A simple command that has an non-zero exit status will cause
#    the shell to exit unless the simple command is:
#    +     contained in an && or || list.
#    +     the command immediately following if, while, or
#          until.
#    +     contained in the pipeline following !.
$SHELL -c 'set -e; false; exit 0' &&
    log_error "set -e should exit on first command that fails"

$SHELL -c 'set -e; true && false || true; false || true; exit 0' ||
    log_error "set -e should not exit if failed command is contained in && or ||"

$SHELL -c 'set -e; if false; then :; fi; exit 0' ||
    log_error "set -e should not exit on failed statement following if"

$SHELL -c 'set -e; while false; do :; done; exit 0' ||
    log_error "set -e should not exit on failed statement following while"

$SHELL -c 'set -e; until false; do exit 0; done;' ||
    log_error "set -e should not exit on failed statement following until"

$SHELL -c 'set -e; ! false | exit 0' ||
    log_error "set -e should not exit on failed statement following pipelines starting with !"

# ==========
# TODO - What shall we do with this option ?
# -h Obsolete. Causes each command whose name has the syntax of an
#    alias to become a tracked alias when it is first encountered.

# ==========
# -k This is obsolete. All arguments of the form name=value are
#    removed and placed in the variable assignment list for the
#    command. Ordinarily, variable assignments must precede
#    command arguments.
set -k
cat foo=bar ||
    log_error "set -k should place all arguments of form name=value in variable assignment list"
set +k

# ==========
# -n The shell reads commands and checks for syntax errors, but
#    does not execute the command. Usually specified on command
#    invocation.
$SHELL -c "set -n; echo foo" | grep -q foo && log_error "set -n should not execute commands"

# ==========
# -o[option]      If option is not specified, the list of options and their
#                 current settings will be written to standard output. When
#                 invoked with a + the options will be written in a format that
#                 can be reinput to the shell to restore the settings. Options
#                 -o name can also be specified with --name and +o name can be
#                 specified with --noname except that options names beginning
#                 with no are turned on by omitting no.This option can be
#                 repeated to enable/disable multiple options. The value of
#                 option must be one of the following:

# ==========
# allexport
# Equivalent to -a. There is already test for `set -a`.

# ==========
# TODO: How to test this ?
# bgnice
# Runs background jobs at lower priorities.

# ==========
# braceexpand
# Equivalent to -B. There is already test for set -B

# ==========
# emacs Enables/disables emacs editing mode. It is tested in emacs.exp tests.

# ==========
# errexit
# Equivalent to -e. There is already a test for `set -e`.

# ==========
# TODO
# globstar
# Equivalent to -G.

# ==========
# TODO: Add test for gmacs in emacs.exp tests
# gmacs Enables/disables gmacs editing mode. gmacs editing
# mode is the same as emacs editing mode except for the
# handling of ^T.

# ==========
# histexpand
# Equivalent to -H.  There is already a test for `set -H`


# ==========
# keyword
# Equivalent to -k. There is already a test for `set -k`.

# ==========
# letoctal
# The let builtin recognizes octal constants with
# leading 0.
set -o letoctal
let result=010+010
[[ $result -eq 16 ]] || log_error "set -o letoctal should recognize octal numbers"

# ==========
# TODO
# monitor
# Equivalent to -m.

# ==========
# TODO
# multiline
# Use multiple lines when editing lines that are longer
# than the window width.

# ==========
# noclobber
# Equivalent to -C. There is already a test for `set -C`

# ==========
# noexec
# Equivalent to -n. There is already a test for `set -n`

# ==========
# noglob
# TODO
# Equivalent to -f.

# ==========
# nolog This has no effect. It is provided for backward
# compatibility.

# ==========
# notify
# Equivalent to -b. It should be tested by `set -b` tests.

# nounset
# Equivalent to -u. It should be tested by `set -u` test.

# ==========
# pipefail
# A pipeline will not complete until all components of
# the pipeline have completed, and the exit status of
# the pipeline will be the value of the last command to
# exit with non-zero exit status, or will be zero if
# all commands return zero exit status.
set -o pipefail
false | true | false | true
[[ $? -eq 1 ]] || log_error "Exit status should be the status of last failing command"

set +o pipefail
false | true | false | true
[[ $? -eq 0 ]] || log_error "Exit status should be the status of last command"

# ==========
# TODO
# privileged
# Equivalent to -p. It should be tested through `set -p`.

# ==========
# TODO
# rc Do not run the .kshrc file for interactive shells.
# echo > "echo Don't run me!" > $HOME/.kshrc
# ksh -i -c "exit 0" | grep -q "Don't run me!" && log_error ""

# ==========
# TODO
# showme
# Simple commands preceded by a ; will be traced as if
# -x were enabled but not executed.

# ==========
# TODO
# trackall
# Equivalent to -h.

# ==========
# verbose
# Equivalent to -v. There is already a test for `set -v`.

# ==========
# vi Enables/disables vi editing mode. There are tests for it in `vi.exp` tests.

# ==========
# viraw This option no longer does anything.

# ==========
# xtrace
# Equivalent to -x. There is already test for `set -x`.

# ==========
# TODO
# -p Privileged mode. Disabling -p sets the effective user id to
#    the real user id, and the effective group id to the real
#    group id. Enabling -p restores the effective user and group
#    ids to their values when the shell was invoked. The -p option
#    is on whenever the real and effective user id is not equal or
#    the real and effective group id is not equal. User profiles
#    are not processed when -p is enabled.

# ==========
# -r restricted. Enables restricted shell. This option cannot be
#    unset once enabled.
# Changing directories is not allowed in restricted shell
$SHELL -c "set -r; cd .." 2>/dev/null && log_error "set -r should enable restricted shell"

# ==========
# TODO; How to test this ?
# -t Obsolete. The shell reads one command and then exits.

# ==========
# TODO
# -u If enabled, the shell displays an error message when it tries
#    to expand a variable that is unset.
# cat > nounset-test.sh <<!
# set -u
# echo -n $this_variable_does_not_exist
# echo -n bye
# !
# set -x
# [[ $($SHELL -i nounset-test.sh) = "bye" ]] && log_error "set -u should fail on expanding unset variables"
# set +x

# ==========
# -x Execution trace. The shell will display each command after
#    all expansion and before execution preceded by the expanded
#    value of the PS4 parameter.
expect=$'+ echo hello world\n+ 1> /dev/null'
actual=$($SHELL -c "exec 2>&1; unset PS4; set -x; echo hello world > /dev/null; set +x")
[[ "$actual" = "$expect"  ]] || log_error "set -x should show execution trace" "$expect" "$actual"

# ==========
# -B Enable {...} group expansion. On by default.
set -B
expect="foobar foobaz"
actual=$(echo foo{bar,baz})
[[ "$actual" = "$expect" ]] || log_error "set -B should enable group expansion" "$expect" "$actual"
set +B
expect="foo{bar,baz}"
actual=$(echo foo{bar,baz})
[[ "$actual" = "$expect" ]] || log_error "set -B should enable group expansion" "$expect" "$actual"

# ==========
# -C Prevents existing regular files from being overwritten using
#    the > redirection operator. The >| redirection overrides this
#    noclobber option.
set -C
echo "bar" > foo
echo "bar" > foo && log_error "set -C should prevent writing on existing files"
echo "bar" >| foo || log_error ">| should override noclobber option"
set +C
echo "bar" > foo || log_error "set +C should not prevent writing on existing files"

# ==========
# TODO: It was added with last `ksh93v-`. What shall we do with this ?
# -K, --keys=keylist
#    When used with -s and -A and without arg, it specifes the
#    sorting parameters. For compound arrays and for arrays of
#    types it consists of a comma separated list of keys. Each key
#    can be followed by :n for numerical sort and/or :r for
#    reverse order. The :n and/or :r can be used for arrays of
#    elements.


# ==========
# --default Restore all non-command line options to the default settings.
cat > default-options <<!
Current option settings
allexport                off
bgnice                   off
braceexpand              off
clobber                  on
emacs                    off
errexit                  off
exec                     on
glob                     on
globstar                 off
gmacs                    off
histexpand               off
ignoreeof                off
interactive              off
keyword                  off
letoctal                 off
log                      on
login_shell              off
markdirs                 off
monitor                  off
multiline                off
notify                   off
pipefail                 off
privileged               off
rc                       off
restricted               off
showme                   off
trackall                 off
unset                    on
verbose                  off
vi                       off
viraw                    off
xtrace                   off
!
$SHELL -c "set --default; diff <(set -o) default-options" || log_error "set --default does not restore default options"

# ==========
# --state List the current option state in the form of a set command
#         that can be executed to restore the state.
# Save old state for comparison
set -o > old-state;
# Save command to restore state
set --state > restore-state.sh;
# Restore to default state
set --default;
# Source restore state script
. restore-state.sh;
# Check new state
set -o > new-state;
diff old-state new-state || log_error "--state does not restore state"
