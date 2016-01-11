.H 1 at
The
.B ast
.B at
command is a re-implementations of
.BR at (1)
and
.BR crontab (1),
including a
.BR cs (1)
service \bat.svc\b for the \bat\b and \bcron\b daemons.
On most systems the
.B at
and
.B crontab
commands are installed as
.B ast_at
and
.B ast_crontab
because the \bat\b service executor \b$INSTALLROOT/lib/at/jobs/atx\b must be
installed setuid \broot\b so the daemon can operate multi-user
(enabling \broot\b privelege is always a manual operation with
.BR package (1)
packages.)
