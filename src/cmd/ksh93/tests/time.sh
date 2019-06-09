# Tests for time and times builtins

# Make sure all builtins are not enabled by default via PATH. Doing so makes `builtin -d`
# ineffective. Which breaks (at the time I write this) the final test in this module.
# See issue #960.
PATH=$NO_BUILTINS_PATH

# Make sure that we can just run these commands
time
times
