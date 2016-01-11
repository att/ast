.H 1 tw
.B tw
is a combination of
.B find
and
.B xargs
that applies C style expressions on the
.I stat
structure of each file in a directory hierarchy.
.EX
tw
.EE
lists the directory hierarchy under the current directory,
.EX
tw chmod go-rwx
.EE
changes the mode of all files in the directory hierarchy under the current
directory by calling
.B chmod
as few times as the
.I exec
arg limit allows.
.EX
tw -d /usr/src -e "name=='*.[chly]'" grep FOOBAR
.EE
greps for
.B FOOBAR
in all the
.B C
source files in /usr/src. 
.P
.B tw
efficiently prunes the file tree search; it follows symbolic
links by default (logical walk).
The
.B getconf
variable
.B PATH_RESOLVE
controls the default behavior for all
.B advsoft
commands:
.EX
getconf PATH_RESOLVE - physical
.EE
sets the default to not follow symbolic links,
.B logical
to follow,
and
.B metaphysical
to follow only command line symbolic links
(corresponding to the -P, -L and -H command options that override the
default).
.EX
tw -P -e type==LNK
.EE
does a physical walk that lists all symbolic links.
.EX
tw -e "mtime>='yesterday morning'"
.EE
lists all files modified since yesterday morning.
The magic file types from the
.B file
command are available in
.B tw
expressions
.RB ( tw
and
.B file
use the same library routine in
.BR libast ):
.EX
tw -e "(mode & 'u+s') && magic == '*script*'" chmod u-s
.EE
finds all setuid scripts and turns off the setuid mode bit.
The magic file also contains MIME types:
.EX
tw -e "mime == '*/exe'"
.EE
lists all executable programs
.RI ( a.out 's).
