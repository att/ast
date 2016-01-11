.H 1 proto
.B proto
converts ANSI C prototype constructs to constructs compatible
with K&R C, ANSI C, and C++.
Only files with the line
.EX
#pragma prototyped
.EE
in the first 64 lines are processed; other files are silently ignored.
This is how
.B advsoft
source is first shipped to foreign architectures.
Once
.B advsoft
is bootstrapped the
.RB non- proto
source can be built using the
.B advsoft
tools.
.P
.B proto
also converts in the other direction by providing
.I extern
prototypes for a collection of K&R source files and by converting
K&R source files in place.
K&R to ANSI is not 100%, but is a good starting point for manual conversion.
.P
.B proto
is token based (no C grammar); this allows it to convert 
before C preprocessing, and also allows
.B proto
to be inserted into
the standalone
.BR cpp .
This is how
.B nmake
compiles ANSI C source with K&R compilers.
