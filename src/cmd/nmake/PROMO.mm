.H 1 nmake
.B nmake
is the AT&T
.BR nmake ,
first released in 1985.
.B nmake
is a modern variant of the traditional Feldman
.B make
with an important difference:
.B nmake
maintains state that records
information for future runs.
The state includes:
.BL
.LI
file modification times
.LI
explicit prerequisites (from makefile assertions)
.LI
implicit prerequisites (from
.I #include
scanning)
.LI
action text (used to build targets)
.LI
variable values
.LI
target attributes
.LE
State
.I and
a language to manipulate it finally makes concise
makefiles a reality; concise because rules traditionally placed in
each makefile can now be implemented in a general way in a single
.I "base rules"
file.
The base rules are such a fundamental part of
.B nmake
that most of its visible features are controlled by them.
.P
Most makefiles are just a few lines:
.EX
:PACKAGE: X11

xgame :: README xgame.6 xgame.h xgame.c xutil.c -lXaw -lXmu -lXt
.EE
Automatically generated
.B probe
information provides convenient compiler abstractions:
.EX
CCFLAGS = $(CC.DLL)

ast 4.0 :LIBRARY: ast.c strmatch.c
.EE
In this case if the compiler supports
shared libraries (aka \fBdll\fP\fIs\fP) then
.L "nmake install"
will generate
.I libast.a
and
.I libast.so.4.0
(or the appropriate shared library suffix determined by
.BR CC.SUFFIX.SHARED ).
There are no
.B nmake
makefile generators (because the makefiles are so small),
there is no separate
.L "make depend"
(because the files with
.I .SCAN
attributes are automatically scanned for implicit prerequisites),
and there is no cheating (because all time stamp changes
are detected, not just
.IR newer-than ).
