# Tests for restricted shell

# Restricted shell should always be run in a separate shell. Otherwise it will cause issues while
# cleaning up test directory (changing directories is not permitted in restricted shells).

# The actions of rksh are identical to those of ksh, except that the following are disallowed:

# ==========
# Unsetting the restricted option.
actual=$($SHELL -c "set -r; set +r" 2>&1)
expect="set: r: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "set +r should be restricted" "$expect" "$actual"

# ==========
# Changing directory (see cd(1)),
actual=$($SHELL -c "set -r; cd .." 2>&1)
expect="cd: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "cd should be restricted" "$expect" "$actual"

# ==========
# Setting or unsetting the value or attributes of SHELL, ENV, FPATH, or PATH,
actual=$($SHELL -c "set -r; SHELL=foo" 2>&1)
expect="SHELL: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Setting SHELL should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; unset SHELL" 2>&1)
expect="unset: SHELL: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Unsetting SHELL should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; ENV=foo" 2>&1)
expect="ENV: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Setting ENV should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; unset ENV" 2>&1)
expect="unset: ENV: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Unsetting ENV should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; FPATH=foo" 2>&1)
expect="FPATH: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Setting FPATH should be restricted" "$expect" "$actual"

actual=$($SHELL -c "FPATH=.; set -r; unset FPATH" 2>&1)
expect="unset: FPATH: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Unsetting FPATH should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; PATH=foo" 2>&1)
expect="PATH: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Setting PATH should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; unset PATH" 2>&1)
expect="unset: PATH: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Unsetting PATH should be restricted" "$expect" "$actual"

# ==========
# Specifying path or command names containing /,
actual=$($SHELL -c "set -r; /bin/cat" 2>&1)
expect="/bin/cat: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Specifying path through / should be restricted" "$expect" "$actual"

# ==========
# Redirecting output (>, >|, <>, and >>).
actual=$($SHELL -c "set -r; ls > /dev/null" 2>&1)
expect="/dev/null: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Redirecting output through > should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; ls >| cat" 2>&1)
expect="cat: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Redirecting output through >| should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; ls <> /dev/null" 2>&1)
expect="/dev/null: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Redirecting output through <> should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; ls >> /dev/null" 2>&1)
expect="/dev/null: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "Redirecting output through >> should be restricted" "$expect" "$actual"

# ==========
# Adding or deleting built-in commands.
actual=$($SHELL -c "set -r; builtin cat" 2>&1)
expect="builtin: builtin: restricted"
[[ "$actual" = "$expect" ]] || log_error "Enabling a builtin should be restricted" "$expect" "$actual"

actual=$($SHELL -c "set -r; builtin -d sleep" 2>&1)
expect="builtin: builtin: restricted"
[[ "$actual" = "$expect" ]] || log_error "Disabling a builtin should be restricted" "$expect" "$actual"

# ==========
# Using command -p to invoke a command.
actual=$($SHELL -c "set -r; command -p ls" 2>&1)
expect="-p: restricted"
[[ "$actual" =~ "$expect" ]] || log_error "command -p should be restricted" "$expect" "$actual"
