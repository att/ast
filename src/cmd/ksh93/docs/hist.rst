.. default-role:: code

:index:`hist` -- process command history list
=============================================

Synopsis
--------
| hist [flags] [first [last]]

Description
-----------
`hist` lists, edits, or re-executes, commands previously entered into
the current shell environment.

The command history list references commands by number. The first number
in the list is selected arbitrarily.  The relationship of a number to its
command does not change during a login session.  When the number reaches
32767 the number wraps around to 1 but maintains the ordering.

When commands are edited (when the `-l` option is not specified), the
resulting lines will be entered at the end of the history list and then
re-executed by the current shell.  The `\f?\f` command that caused the
editing will not be entered into the history list.  If the editor returns
a non-zero exit status, this will suppress the entry into the history
list and the command re-execution.  Command line variable assignments
and redirections affect both the \f?\f command and the commands that
are re-executed.

*first* and *last* define the range of commands. *first* and *last*
can be one of the following:

   :*number*: A positive number representing a command number.  A `+`
      sign can precede *number*.

   :-*number*: A negative number representing a command that was
      executed *number* commands previously. For example, `-1` is the
      previous command. This is equivalent to `-N`\ *number*.

   :*string*: *string* indicates the most recently entered command
      that begins with *string*. *string* should not contain an `=`.

If *first* is omitted, the previous command is used, unless `-l` is
specified, in which case it will default to `-16` and *last* will
default to `-1`.

If *first* is specified and *last* is omitted, then *last* will
default to *first* unless `-l` is specified in which case it will
default to `-1`.

If no editor is specified, then the editor specified by the `HISTEDIT`
variable will be used if set, or the `FCEDIT` variable will be used if
set, otherwise, `ed` will be used.

Flags
-----
:-e *editor*: *editor* specifies the editor to use to edit the history
   command.  A value of `-` for *editor* is equivalent to specifying the
   `-s` option.

:-l: List the commands rather than editing and re-executing them.

:-n: Suppress the command numbers when the commands are listed.

:-p: Writes the result of history expansion for each operand to standard
   output.  All other options are ignored.

:-r: Reverse the order of the commands.

:-s: Re-execute the command without invoking an editor.  In this case an
   operand of the form *old*`=`*new* can be specified to change the
   first occurrence of the string *old* in the command to *new* before
   re-executing the command.

:-N *num*: Start at *num* commands back.

Exit Status
-----------
If a command is re-executed, the exit status is that of the command that
gets re-executed.  Otherwise, it is one of the following:

:0: Successfully completion of the listing.

:>0: An error occurred.

See Also
--------
`ksh`\(1), `sh`\(1), `ed`\(1)
