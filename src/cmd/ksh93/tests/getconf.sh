# Tests for `getconf` builtin

# ==========
#   -a, --all       Call the native getconf(1) with option -a.
# actual=$(getconf -a)
# [[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -a'"

# ==========
#   -b, --base      List base variable name sans call and standard prefixes.
actual=$(getconf -b)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -b'"

# ==========
#   -c, --call=RE   Display variables with call prefix that matches RE. The call
#                   prefixes are:
#                     CS    confstr(2)
#                     PC    pathconf(2)
#                     PC    fpathconf(2)
#                     SC    sysconf(2)
#                     SI    sysinfo(2)
#                     XX    Constant value.
# TODO: Passing invalid configuration parameter should show an error.
actual=$(getconf -c=CS RELEASE)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -c'"

# ==========
#   -d, --defined   Only display defined values when no operands are specified.
actual=$(getconf -d)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -d'"
[[ "$actual" =~ "=undefined" ]] || log_error "'getconf -d' should not output undefined values"

# ==========
#   -l, --lowercase List variable names in lower case.
actual=$(getconf -l)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -l'"

# https://github.com/att/ast/issues/1171
# getconf -l | grep -q "[[:upper:]]" || log_error "'getconf -l' should not output uppercase variables"

# ==========
#   -n, --name=RE   Display variables with name that match RE.
# https://github.com/att/ast/issues/1171
# actual=$(getconf -n=xyz)
# [[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -l'"

# ==========
#   -p, --portable  Display the named writable variables and values in a form
#                   that can be directly executed by sh(1) to set the values. If
#                   name is omitted then all writable variables are listed.
actual=$(getconf -p)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -p'"
getconf -p | grep "^getconf " || log_error "Names should be prefixed with getconf"

# ==========
#   -q, --quote     "..." quote values.
# https://github.com/att/ast/issues/1173
# actual=$(getconf -q)

# ==========
#   -r, --readonly  Display the named readonly variables in name=value form. If
#                   name is omitted then all readonly variables are listed.
actual=$(getconf -r)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -r'"

# ==========
#   -s, --standard=RE
#                   Display variables with standard prefix that matches RE. Use
#                   the --table option to view all standard prefixes, including
#                   local additions. The standard prefixes available on all
#                   systems are:
#                     AES
#                     AST
#                     C
#                     GNU
#                     POSIX
#                     SVID
#                     XBS5
#                     XOPEN
#                     XPG
actual=$(getconf -s AST)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -s'"
[[ "$actual" =~ "UNIVERSE=ucb" ]] || log_error "'getconf -s AST' should include name/value pair for UNIVERSE"
# ==========
#   -t, --table     Display the internal table that contains the name, standard,
#                   standard section, and system call symbol prefix for each
#                   variable.
actual=$(getconf -t)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -t'"

# ==========
#   -w, --writable  Display the named writable variables in name=value form. If
#                   name is omitted then all writable variables are listed.
actual=$(getconf -w)
[[ ! -z "$actual" ]] && [[ $? -eq 0 ]] || log_error "Failed to execute 'getconf -w'"

# ==========
#   -v, --specification=name
#                   Call the native getconf(1) with option -v name.
# 
# https://github.com/att/ast/issues/1174
# actual=$(getconf -v=foo)
