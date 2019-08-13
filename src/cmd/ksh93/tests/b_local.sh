# Verify the `local` command behaves correctly.

# =======
# `local` outside a function is an error.
actual=$(local var 2>&1)
expect='local: local can only be used in a function'
[[ $actual == *"$expect" ]] || log_error "local outside a function is an error " "$expect" "$actual"

# =======
# `local` inside a function is allowed and shadows a global, or function, var of the same name.
var=hello
actual=$(
    function shadow1 {
        print -n "func shadow1 var=$var; "
        local var=shadow1
        print -n "func shadow1 var=$var; "
    }
    function shadow2 {
        print -n "func shadow2 var=$var; "
        local var=shadow2
        print -n "func shadow2 var=$var; "
        shadow1
        print -n "func shadow2 var=$var; "
    }
    shadow2; print "global var=$var" 2>&1
)
expect="func shadow2 var=hello; func shadow2 var=shadow2; func shadow1 var=hello;"
expect="$expect func shadow1 var=shadow1; func shadow2 var=shadow2; global var=hello"
[[ $actual == "$expect" ]] || log_error "local inside a function is allowed" "$expect" "$actual"
