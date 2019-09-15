.. default-role:: code

.. index: dot
.. index: .

:index:`source` -- execute commands in the current environment
==============================================================

Synopsis
--------
| `source` *name* [*arg* ...]
| `.` *name* [*arg* ...]

Description
-----------
`source` is a special built-in command that executes commands from a
function or a file in the current environment.

If *name* refers to a function defined with the `function` *name*
syntax, the function executes in the current environment as if it had
been defined with the *name*`()` syntax so that there is no scoping.
Otherwise, commands from the file defined by *name* are executed in
the current environment.  Note that the complete script is read before
it begins to execute so that any aliases defined in this script will not
take effect until the script completes execution.

When *name* refers to a file, the `PATH` variable is searched for
the file containing commands.  In this case execute permission is not
required for *name*.

If any *arg*\s are specified, these become the positional parameters for
the duration of the function or script and are restored upon completion.

Exit Status
-----------
If *name* is found, then the exit status is that of the last command
executed.  Otherwise, since this is a special built-in, an error will
cause a non-interactive shell to exit with a non-zero exit status.
An interactive shell returns a non-zero exit status to indicate an error.

See Also
--------
`command`\(1), `ksh`\(1)
