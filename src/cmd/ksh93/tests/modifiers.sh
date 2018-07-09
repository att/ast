# These are the tests for various style of parameter expansion modifiers supported by ksh93

# Check ${parameter:-word} style of parameter expansion. ':' is optional and is used to check for null (empty string).
unset foo
[[ ${foo:-bar} == bar ]]  || log_error  '${foo:-bar} not bar when foo is not set'

unset foo
[[ ${foo-bar} == bar ]]  || log_error  '${foo-bar} not bar when foo is not set'

foo=""
[[ ${foo-bar} == "" ]]  || log_error  '${foo-bar} not "" when foo is null'

# Check ${parameter:=word} style of parameter expansion. ':' is optional and is used to check for null (empty string).
unset foo
[[ ${foo:=bar} == bar ]]  || log_error '${foo:=bar} not bar when foo is not set'

unset foo
[[ ${foo=bar} == bar ]]  || log_error '${foo=bar} not bar when foo is not set'

foo=""
[[ ${foo=bar} == "" ]]  || log_error  '${foo=bar} not "" when foo is null'

# Check ${parameter:?word} style of parameter expansion. ':' is optional and is used to check for null (empty string).
expect="bar not set"
actual=$( $SHELL -c 'unset foo; print ${foo:?bar not set}' 2>&1 )
[[ $actual  =~ $expect ]]  || log_error '${foo:?bar} does not display error if foo not set'

actual=$( $SHELL -c 'unset foo; print ${foo?bar not set}' 2>&1 )
[[ $actual =~ $expect ]]  || log_error '${foo?bar} does not display error if foo is not set'

[[ $( $SHELL -c 'foo=""; print ${foo?bar}' 2>&1 ) == "" ]]  || log_error  '${foo=bar} not "" when foo is null'

# When nothing is specified after :?, a default error message is printed
unset foo
[[ $( (print ${foo:?}) 2>&1) =~ "parameter not set" ]] || log_error 'Incorrect error message with ${foo:?}'

# Check ${parameter:+word} style of parameter expansion. ':' is optional and is used to check for null (empty string).
unset foo
[[ ${foo:+bar} == "" ]]  || log_error '${foo:+bar} not null when foo is not set'

unset foo
[[ ${foo+bar} == "" ]]  || log_error '${foo+bar} not null when foo is not set'

foo="non-null value"
[[ ${foo:+bar} == "bar" ]]  || log_error  '${foo:+bar} not bar when foo is not null'
[[ ${foo+bar} == "bar" ]]  || log_error  '${foo+bar} not bar when foo is not null'
