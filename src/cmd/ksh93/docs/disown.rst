.. default-role:: code

:index:`disown` -- disassociate a job from the current shell
============================================================

Synopsis
--------
| disown [job ...]

Description
-----------
`disown` prevents the current shell from sending a `HUP` signal to
each of the given *job*\s when the current shell terminates a login session.

If *job* is omitted, the most recently started or stopped background job
is used.

Each *job* can be specified as one of the following:

    :*number*: *number* refers to a process id.

    :-*number*: *number* refers to a process group id.

    :%*number*: *number* refer to a job number.

    :%*string*: Refers to a job whose name begins with *string*.

    :%??*string*: Refers to a job whose name contains *string*.

    :%+ `or` %%: Refers to the current job.

    :%-: Refers to the previous job.

Exit Status
-----------
:0: If all jobs are successfully disowned.

:>0: If one more *job*\s does not exist.

See Also
--------
`wait`\(1), `bg`\(1), `jobs`\(1)
