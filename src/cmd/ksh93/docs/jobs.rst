.. default-role:: code

:index:`jobs` -- display status of jobs
=======================================

Synopsis
--------
| jobs [flags] [job ...]

Description
-----------
`jobs` displays information about specified *job*s that were started
by the current shell environment on standard output.  The information
contains the job number enclosed in [...], the status, and the command
line that started the job.

If *job* is omitted, `jobs` displays the status of all stopped jobs,
background jobs, and all jobs whose status has changed since last reported
by the shell.

When `jobs` reports the termination status of a job, the shell removes
the jobs from the list of known jobs in the current shell environment.

Each *job* can be specified as one of the following:

    :*number*: *number* refers to a process id.
    :-*number*: *number* refers to a process group id.
    :%*number*: *number* refer to a job number.
    :%*string*: Refers to a job whose name begins with *string*.
    :%?*string*: Refers to a job whose name contains *string*.
    :+ `or` %%: Refers to the current job.
    :%-: Refers to the previous job.

Flags
-----
:-l: `jobs` displays process id's after the job number in addition to
    the usual information.

:-n: Only the jobs whose status has changed since the last prompt is displayed.

:-p: The process group leader id's for the specified jobs are displayed.

Exit Status
-----------
0 The information for each job is written to standard output.

>0 One or more jobs does not exist.

See Also
--------
`wait`\(1), `ps`\(1), `fg`\(1), `bg`\(1)
