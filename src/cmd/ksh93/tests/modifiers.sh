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
[[ $( (print ${foo:?}) 2>&1) =~ "parameter not set" ]] || log_error 'Incorrect error message with ${foo:?} when foo is not set'

foo=
[[ $( (print ${foo:?}) 2>&1) =~ "parameter null" ]] || log_error 'Incorrect error message with ${foo:?} when foo is null'

# Check ${parameter:+word} style of parameter expansion. ':' is optional and is used to check for null (empty string).
unset foo
[[ ${foo:+bar} == "" ]]  || log_error '${foo:+bar} not null when foo is not set'

unset foo
[[ ${foo+bar} == "" ]]  || log_error '${foo+bar} not null when foo is not set'

foo="non-null value"
[[ ${foo:+bar} == "bar" ]]  || log_error  '${foo:+bar} not bar when foo is not null'
[[ ${foo+bar} == "bar" ]]  || log_error  '${foo+bar} not bar when foo is not null'

# Check for regressions on issue #475 where parens after `-', `+', and `=' were causing syntax
# errors. We check both the unset variable case and the set variable case for each set of symbols.
unset -v foo
for op in - :- = :=
do
    for word in '(word)' 'w(or)d' '(wor)d' 'w(ord)' 'w(ord' 'wor)d'
    do
        if [[ $(eval "echo \${foo${op}${word}}") != "${word}" ]]
        then
            log_error "\${foo${op}${word}} not ${word} when foo is not set"
        fi
    done
done

foo="non-null value"
for op in - :- = :=
do
    for word in '(word)' 'w(or)d' '(wor)d' 'w(ord)' 'w(ord' 'wor)d'
    do
        if [[ $(eval "echo \${foo${op}${word}}") != "${foo}" ]]
        then
            log_error "\${foo${op}${word}} not ${foo} when foo is set"
        fi
    done
done

unset -v foo
for op in + :+
do
    for word in '(word)' 'w(or)d' '(wor)d' 'w(ord)' 'w(ord' 'wor)d'
    do
        if [[ $(eval "echo \${foo${op}${word}}") != "" ]]
        then
            log_error "\${foo${op}${word}} not null when foo is not set"
        fi
    done
done

foo="non-null value"
for op in + :+
do
    for word in '(word)' 'w(or)d' '(wor)d' 'w(ord)' 'w(ord' 'wor)d'
    do
        if [[ $(eval "echo \${foo${op}${word}}") != "${word}" ]]
        then
            log_error "\${foo${op}${word}} not ${word} when foo is set"
        fi
    done
done

# ==========
# https://github.com/att/ast/issues/70
cat > $TEST_DIR/modifier-in-loop.sh <<EOF
unset -v var
for i in 1 2 3 4 5; do
        case \${var+s} in
        ( s )   echo set; unset -v var;;
        ( '' )  echo unset; var=;;
        esac
done
EOF

chmod u+x $TEST_DIR/modifier-in-loop.sh

actual=$($TEST_DIR/modifier-in-loop.sh)
expect=$'unset\nset\nunset\nset\nunset'

[[ "$actual" = "$expect" ]] || log_error '${var+s} expansion fails in loops' "$expect" "$actual"
