.H 1 tksh
.I tksh
is an implementation of the Tcl C library
written on top of the library for the new KornShell
.RI ( ksh93 ). 
.I tksh
emulates the behavior of Tcl by using the API that is provided for extending
.IR ksh93 ,
which is similar to the Tcl library in that it allows access to
variables, functions and other state of the interpreter.
This implementation requires no modification to
.IR ksh93 ,
and allows Tcl libraries such as Tk to run on top of
.I ksh93
unchanged,
making it possible to use shell scripts in
place of Tcl scripts. 
.I ksh93
is well suited for use with Tk because it is
backward compatible with
.IR sh ,
making it both easy to learn and easy to extend
existing scripts to provide a graphical user interface.
.P
.I tksh
is not yet
another port of Tk to another language -- it allows Tcl scripts to run without
modification using the ksh93 internals.
This makes it possible to combine
Tcl and
.I ksh93 ,
which is useful for writing
.I ksh93
scripts that use components that have been implemented in Tcl
(such as Tk widgets).
.P
.I tksh
was developed by
\h'0*\w"http://www.research.att.com/~jlk"'Jeff Korn\h'0'
and is distributed by
\h'0*\w"mailto:advsoft@research.att.com"'advsoft@research.att.com\h'0'.
For the latest information vist the
.I tksh
\h'0*\w"http://www.cs.princeton.edu/~jlk/tksh"'home page\h'0'.
