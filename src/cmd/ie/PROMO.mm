.H 1 ie
.B ie
executes a dynamically linked command with the
.BR ksh (1)
input line edit discipline pushed on the standard input.
All child processes will also have input line editing enabled.
.P
.B ie
intercepts the
.BR read (3)
system call by preloading a DLL (shared library) at runtime,
so it only works on architectures that support DLL preload.
Statically-linked, set-uid or set-gid commands will not work with
.BR ie .
