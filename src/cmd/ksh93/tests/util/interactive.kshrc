# vim: set filetype=sh sw=4 ts=4 et:

# See the `expect_prompt` function in interactive.expect.rc. This provides a prompt that changes in
# a predictable manner each time the prompt appears. This helps ensure that a complex expect based
# test stays in sync with the output of the shell.
integer prompt_counter=0
PS1='KSH PROMPT:$(( ++prompt_counter )): '

# This is a helper function for use in expect `send` commands to help delimit output for easier
# matching. Alternatively, to ensure the expected marker appears in the output.
function _marker {
    print "@MARKER:$*@"
}
