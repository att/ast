.. default-role:: code

:index:`builtin` -- add, delete, or display shell built-ins
===========================================================

Synopsis
--------
| builtin [flags] [pathname ...]

Description
-----------
`builtin` can be used to add, delete, or display built-in commands in the
current shell environment. A built-in command executes in the current
shell process and can have side effects in the current shell. On most
systems, the invocation time for built-in commands is one or two orders
of magnitude less than commands that create a separate process.

For each *pathname* specified, the basename of the pathname determines
the name of the built-in. For each basename, the shell looks for a C level
function in the current shell whose name is determined by prepending `b_`
to the built-in name. If *pathname* contains a `/`, then the built-in
is bound to this pathname. A built-in bound to a pathname will only be
executed if *pathname* is the first executable found during a path
search. Otherwise, built-ins are found prior to performing the path search.

If no *pathname* operands are specified, then `builtin` displays the
current list of built-ins, or just the special built-ins if `-s` is
specified, on standard output. The full pathname for built-ins that are
bound to pathnames are displayed.

.. index:: lib_init()

Libraries containing built-ins can be specified with the `-f` option. If
the library contains a function named `lib_init`\(), this function will
be invoked with argument `0` when the library is loaded. The `lib_init`\()
function can load built-ins by invoking an appropriate C level function. In
this case there is no restriction on the C level function name.

The C level function will be invoked with three arguments. The first
two are the same as `main`\() and the third one is a pointer.

.. index:: restricted shell

`builtin` cannot be invoked from a restricted shell.

Flags
-----
:-d: Deletes each of the specified built-ins. Special built-ins cannot be deleted.

:-f *lib*: On systems with dynamic linking, *lib* names a shared
   library to load and search for built-ins. Libraries are searched
   for in `../lib/ksh` and `../lib` on `$PATH` and in system dependent
   library directories. The system dependent shared library prefix
   and/or suffix may be omitted. Once a library is loaded, its symbols
   become available for the current and subsequent invocations of
   `builtin`. Multiple libraries can be specified with separate invocations
   of `builtin`. Libraries are searched in the reverse order in which
   they are specified.

:-l: List the library base name, plugin YYYYMMDD version stamp, and full
   path for `-f`\ *lib* on one line on the standard output.

:-n: Disable each of the specified built-ins. Special built-ins cannot be
   disabled.  If no built-ns are specified, display all disabled built-ins.

:-p: Causes the output to be in a form of `builtin` commands that can be
   used as input to the shell to recreate the current set of builtins.

:-s: Display only the special built-ins.

Exit Status
-----------
:0: All *pathname* operands and `-f` options processed successfully.

:>0: An error occurred

See Also
--------
`whence`\(1)
