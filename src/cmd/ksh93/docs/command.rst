.. default-role:: code

:index:`command` -- execute a simple command
============================================

Synopsis
--------
| command [flags] [command [arg ...]]

Description
-----------
Without `-v` or `-V`, `command` executes *command* with arguments given
by *arg*, suppressing the shell function lookup that normally occurs.
In addition, if *command* is a special built-in command, then the special
properties are removed so that failures will not cause the script that
executes it to terminate.]

With the `-v` or `-V` options, `command` is equivalent to the
:index:`whence`\(1) command.

Flags
-----
:-p: Causes a default path to be searched rather than the one defined by
   the value of `PATH`.

:-v: Equivalent to `whence` *command* [*arg* ...]].

:-x: If *command* fails because there are too many *arg*\s, it will be
   invoked multiple times with a subset of the arguments on each invocation.
   Arguments that occur prior to the first word that expand to multiple
   arguments and arguments that occur after the last word that expands to
   multiple arguments will be passed on each invocation. The exit status
   will be the maximum invocation exit status.

:-V: Equivalent to `whence `-v` *command* [*arg* ...]].

Exit Status
-----------
If *command* is invoked, the exit status of `command` will be that of
*command*.  Otherwise, it will be one of the following:

:0: `command` completed successfully.

:>0: `-v` or `-V` has been specified and an error occurred.

:126: *command* was found but could not be invoked.

:127: *command* could not be found.

See Also
--------
`whence`\(1), `getconf`\(1)
