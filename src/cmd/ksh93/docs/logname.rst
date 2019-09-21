.. default-role:: code

:index:`logname` -- return the user's login name
================================================

Synopsis
--------
| logname

Description
-----------
`logname` writes the users's login name to standard output.  The login
name is the string that is returned by the `getlogin`\(2) function.
If `getlogin`\(2) does not return successfully, the corresponding to
the real user id of the calling process is used instead.

Exit Status
-----------
:0: Successful Completion.
:>0: An error occurred.

See Also
--------
`getlogin`\(2)
