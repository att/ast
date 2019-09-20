.. default-role:: code

:index:`bg` -- resume jobs in the background
============================================

Synopsis
--------
| bg [job ...]


Description
-----------
`bg` places the given *job*\ s into the background and sends them a
`CONT` signal to start them running. If *job* is omitted, the most
recently started or stopped background job is resumed or continued in
the background.

Each *job* can be specified in one of the following ways:

   :*int*: Refers to a process id (pid).
   :`%`\ *int*: Refers to a job number.
   :`%`\ *str*: Refers to a job whose name begins with *str*.
   :`%?`\ *str*: Refers to a job whose name contains *str*.
   :`%%`: Refers to the current job.
   :`%+`: Refers to the current job.
   :`%-`: Refers to the previous job.

Exit Status
-----------
:0: If all background jobs are started.

:>0: If one more jobs does not exist or there are no background jobs.

See Also
--------
`wait`\(1), `fg`\(1), `disown`\(1), `jobs`\(1)
