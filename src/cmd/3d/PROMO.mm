.H 1 3d
.B 3d
is the historical name for
.IB n DFS ,
the multiple dimension file system.
It is implemented as a shared library that is preloaded
before any other shared library using the recently documented
.RI ( ahem )
.B LD_PRELOAD
or
.B _RLD_LIST
feature of the runtime linker.
.B 3d
intercepts pathname system calls (and other calls if tracing is
turned on) and provides a logical namespace on top of the underlying
physical file system.
The shared library implementation currently works
only on
.IR bsd.i386 ,
.IR linux.i386 ,
.IR osf.alpha ,
.IR sun4 ,
.IR sol.* ,
and
.IR sgi.* .
.P
.B 3d
works in conjunction with
.B ksh88
or
.BR ksh93 .
The
.B 3d
command with no arguments enters a
.B 3d
shell.
The
.B vpath
builtin controls the logical
filesystem view:
.EX
vpath dir_1 dir_2
.EE
mounts the directory hierarchy
.I dir_1
on top of
.IR dir_2 .
Files under
.I dir_1
overlay files under
.IR dir_2 .
Files under
.I dir_2
are readonly;
any files under
.I dir_2
that are modified are first copied to the
corresponding place in
.IR dir_1 .
.I vpaths
may be chained:
.EX
vpath dir_2 dir_3
.EE
.I vpaths
are per-process; every process may have a different view.
.I vpaths
are a handy way to separate source and generated files.
Typical
.B advsoft
users set up
.B 3d
at login or
.B xterm
time:
.EX
export HOSTTYPE=$(package)
vpath $HOME/arch/$HOSTTYPE $HOME
.EE
with source files in
.LR $HOME/src/(cmd|lib)/* .
.B nmake
is run in
.L $HOME/arch/$HOSTTYPE/src/(cmd|lib)/*
and generated files (objects and executables) are dropped in the top hierarchy.
.P
.B 2d
can prefix any command to disable 
.B 3d
for that command:
.EX
2d ls *.c
.EE
.P
.B 3d
also provides tracing, monitoring and call intercept services.
.B vpm
uses monitoring to graph the process and io hierarchy.
The
.I cs
name server uses pathname call intercepts to translate network
pathnames.
Try this for any host:
.EX
cat /dev/tcp/hostname/inet.daytime
.EE
