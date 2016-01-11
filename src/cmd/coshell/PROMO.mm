.H 1 coshell
.B coshell
executes blocks of shell commands on lightly loaded hosts in the local network.
There is one
.B coshell
server per user.
This server runs as a daemon on the user's home host,
and only processes running on the home host have access to the server.
The server controls a background
.B ksh
shell process, initiated by
.BR rsh ,
on each of the connected hosts.
.P
.B coshell
is easy to use and administer.
The only privilege required for installation, administration or use is
.B rsh
access to the local hosts.
.B coshell
does requires a uniform file namespace across all hosts on the local network.
.P
.B nmake
and
.B gmake
users can take advantage of network execution with
no makefile modifications.
Shell level access is similar to but more efficient than
.B rsh
and allows host expression matching to replace the explicit host name argument.
.P
Factor of 5 compilation speedups are typical.
.P
To start your local daemon:
.EX
coshell +
.EE
To send
.B nmake
or
.B gmake
actions to
.B coshell
at concurrency level 8:
.EX
export COSHELL=coshell
export NPROC=8
.EE
To run shell commands on a lightly loaded host:
.EX
coshell -r - 'hostname; sort a > b' &
pid=$!
# other stuff
wait $pid
.EE
For interactive
.B coshell
status:
.EX
coshell -
coshell> s
.I "... status ..."
coshell> h
.I "... help ..."
coshell> q
.EE
.B package
generates static host information and
.B ss
lists the dynamic status of hosts on the local network.
