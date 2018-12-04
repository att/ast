# Tests for whence builtin

# Make sure all builtins are not enabled by default via PATH. Doing so makes `builtin -d`
# ineffective. Which breaks (at the time I write this) the final test in this module.
# See issue #960.
PATH=$NO_BUILTINS_PATH

# ==========
# -a Displays all uses for each name rather than the first.
actual=$(whence -a sleep)
[[ $actual =~ "sleep is a shell builtin" ]] || log_error "whence -a should recognize shell builtins"
[[ $actual =~ "bin/sleep" ]] || log_error "whence -a should recognize commands in PATH"
# [[ $actual =~ "sleep is an undefined function" ]] || log_error "whence -a should recognize undefined functions"


# ==========
# -f Do not check for functions.
function cat {
    :
}
[[ $(whence -f cat) =~ "bin/cat" ]] || log_error "whence -f should ignore functions"
unset -f cat

# ==========
# -p Do not check to see if name is a reserved word, a built-in, an alias, or a function. This turns
#    off the -v option.
builtin cat
[[ $(whence -p cat) =~ "bin/cat" ]] || log_error "whence -p should search in PATH"
builtin -d cat

# ==========
# -q Quiet mode. Returns 0 if all arguments are built-ins, functions, or are programs found on the
#    path.
actual=$(whence -q cat)
[[ $? -eq 0 ]] || log_error "whence -q fails to find cat command"
[[ ${#actual} -eq 0 ]] || log_error "whence -q should have empty output"

whence -q no-such-command &&
    log_error "whence -q of non-existent command should report failure status"

# ==========
# -v For each name you specify, the shell displays a line that indicates if that name is one of the
#    following:

# Reserved word
[[ $(whence -v if) = "if is a keyword" ]] || log_error "whence -v does not recognize keywords"

# Alias
alias sample_alias=cat
[[ $(whence -v sample_alias) = "sample_alias is an alias for cat" ]] ||
    log_error "whence -v does not recognize aliases"
unalias sample_alias

# Built-in
[[ $(whence -v true) = "true is a shell builtin" ]] || log_error "whence -v does not recognize builtins"

# Undefined function
actual=$(whence -v this_function_does_not_exit 2>&1)
expect="whence: this_function_does_not_exit: not found"
[[ "$actual" =~ "$expect" ]] || log_error "whence -v fails to detect undefined functions" "$expect" "$actual"

# Function
function sample_function {
    :
}
[[ $(whence -v sample_function) = "sample_function is a function" ]] || log_error "whence -v does not recognize functions"
unset -f sample_function


# Tracked alias
[[ $(whence -v cat) =~ "cat is a tracked alias" ]] || log_error "whence -v does not recognize tracked aliases"

# Program
# TODO: On first invocation all external programs become tracked aliases (even if set +h is set).
# How to test this ?

# ==========
# -t Output only the type for each command. This option is kept for bash compatibility.
actual=$(whence -t if)
expect="keyword"
[[ "$actual"  = "$expect" ]] || log_error "whence -t does not recognize keywords" "$expect" "$actual"

alias sample_alias=cat
actual=$(whence -t sample_alias)
expect="alias"
[[ "$actual" = "$expect" ]] || log_error "whence -v does not recognize aliases" "$expect" "$actual"
unalias sample_alias

actual=$(whence -t true)
expect="builtin"
[[ "$actual"  = "$expect" ]] || log_error "whence -t does not recognize builtins" "$expect" "$actual"

function sample_function {
    :
}
actual=$(whence -t sample_function)
expect="function"
[[ "$actual" = "$expect" ]] || log_error "whence -t does not recognize functions" "$expect" "$actual"
unset -f sample_function

actual=$(whence -t ls)
expect="file"
[[ "$actual"  = "$expect" ]] || log_error "whence -t does not recognize files" "$expect" "$actual"
