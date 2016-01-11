.xx noindex
.MT 4
.TL

.H 1 "nmake Overview"
.BR nmake (1)
is the standard software construction tool at AT&T and
Alcatel-Lucent.
It is a descendent of the UNIX
.BR make (1)
and is not related to the
.xx link="http://msdn.microsoft.com/library/en-us/vcug98/html/_asug_overview.3a_.nmake_reference.asp	Microsoft program"
of the same name.
.P
Some of the
.B nmake
features described in this overview are:
.BL
.LI
Portable makefiles that are OS and compiler independent.
.LI
Automatic dynamic dependency checking based on file type.
.LI
Automatic recursive make ordering.
.LI
Excellent performance that scales to 10K file, 10M line projects.
.LI
An extensible programming language.
.LI
Parallel execution.
.LI
Many common actions
.RB ( install ,
.BR clobber ,
.BR list.manifest ,
etc.)
provided by default.
.LI
Makefile sizes typically 10 times smaller than
.BR make .
.LI
State based execution.
.LI
Source in multiple directories.
.LI
Viewpathing.
.LI
Portable external package naming.
.LI
Native command, archive and shared library generation.
.LI
No
.BI auto this ,
.BI config that
or
.BI foo tool
commands, no makefile generation, no
.LR "make depend" ,
no separate manifests;
just
.BR nmake .
.LE

.H 1 History
.B nmake
was created and enhanced by Glenn Fowler of AT&T Labs Research.
It began in 1984 as
.B make
with a makefile
.BR cpp (1)
(C preprocessor) front end.
It soon evolved into a separate implementation with a language
rich enough to make preprocessed and generated makefiles obsolete.
The first internal release was in 1985 and the first external open source
release was in 2000.
The current release is available in the
.B ast-base
and
.B ast-open
packages at
.xx link="/sw/download/	Software Systems Research software;"
there is also a
.xx link="/sw/download/faq.html	FAQ."
Although
.B nmake
makefiles are not compatible with UNIX
.B make
or Microsoft
.BR nmake ,
.B nmake
can generate UNIX
.B make
or Microsoft
.B nmake
style makefiles for porting and bootstrapping.

.H 1 Overview
This is a brief overview of common user-level
.B nmake
features used by the component makefiles of the
.xx link="/sw/download/	ast-open"
source package.
Although slightly out of sync with the AT&T release, the
.xx link="http://www.bell-labs.com/project/nmake/	Alcatel-Lucent nmake site"
(who mourns for Bell Labs?) provides a more complete treatment of the details
(the tech writer remained with Lucent after the '95 split.)
The following assumes familiarity with the UNIX or GNU
.B make
program.
There is a glossary of terms in the appendix; the first reference to
each term is in italics.

.H 1 Makefiles
Similar to UNIX
.BR make ,
.B nmake
input is a
.I makefile
consisting of a set of
.I assignments
and
.IR assertions .
By default
.B nmake
searches for a file named
.LR Nmakefile ,
.LR nmakefile ,
.LR Makefile ,
and then
.L makefile
in that order.
A common convention is to use the
.L .mk
suffix when naming makefiles and invoke
.B nmake
with
\f5-f\fP \fIname\fP\f5.mk\fP.
.P
Unlike UNIX makefiles, an
.B nmake
makefile is compiled by reading the makefile,
carrying out the assignments, and creating rules from the assertions.
The compiled makefile name is formed by adding a
.L .mo
suffix to the makefile name if it does not have a
.L .mk
suffix or by changing the
.L .mk
suffix to
.LR .mo .
In addition to the assertions and assignments in the input makefile,
.B nmake
uses a precompiled
.I "base rules"
file and, if defined,
.I "global rules"
files when compiling the makefile.
The makefile is implicitly compiled whenever it,
the base rules, or any referenced global rules file changes.

.H 1 Comments
.B nmake
supports C style comments as well as
.L #
style comments that appear after one or more space characters.
Earlier versions of
.B nmake
passed makefiles through the C preprocessor.
Although still supported, it is now deprecated
in lieu of newer language features.
(It hasn't been dropped because there are still a few legacy projects
that rely on makefile preprocessing.)

.H 1 Assertions
An
.B nmake
assertion is a generalization of a
.B make
assertion.
It consists of a
.I "target list"
(the left hand side), an
.IR operator ,
and a
.I "prerequisite list"
(the right hand side.)
The operator may either be
.L :
as in
.BR make ,
or of the form
\f5:\fP\fIname\fP\f5:\fP where
.I name
is the null string or an alphanumeric.
Many operators of the latter form are defined in the base rules.
Additional operators can be defined in global and user makefiles.
A description of how to define new operators is beyond the scope of this
introduction.
.P
For example, the line
.EX
prog : prog.o prog.h -lm
.EE
specifies that the target
.L prog
depends on the prerequisites
.LR prog.o ,
.L prog.h
and
.LR -lm .
.P
Indented lines after each assertion, if any, define an
.I action
block associated with the assertion.
Unlike
.BR make ,
.B nmake
actions are multi-line blocks that are executed as a unit.
To execute a shell action block,
.B nmake
expands any
.B nmake
variables and then passes the block to the shell for execution.
This allows action blocks to contain shell here documents and
multi-line structured statements without the need for extra quoting.
This is extremely difficult to do in
.BR make ,
where actions are executed one line at a time.
.P
The shell action trace is done by the shell as each command is executed
using the
.BR sh (1)
.L -x
option.
Tracing can be turned off for a target by specifying
.L "set +x"
as the first command in its action block.
Tracing can be turned off for an individual command by prefixing it with
.LR silent :
.EX
test_silent :
	silent echo this will not echo '"+ echo ..."'
.EE
By default a shell action block fails and is terminated when any command
returns non-0 exit status, excluding commands subject to
.LR if ,
.LR while ,
.LR ! ,
.L ||
or
.LR && .
The exit status can be ignored for all commands in a target action
by asserting the
.B .IGNORE
attribute on the target.
The exit status for an individual command can be ignored by prefixing it with
.LR ignore :
.EX
test_ignore :
	ignore false
	echo the first false is ignored
	false
	echo but the second causes the action to fail and skip this echo
.EE
.L silent
and
.L ignore
are implemented as shell functions or aliases, depending on the shell path,
determined by searching for the following {
.B $COSHELL
.B $SHELL
.B ksh
.B sh
.B /bin/sh
}
in order on
.BR $PATH.
.P
The collection of assertions associated with a given target
together defines a
.I rule
for the target.
In most cases a rule is defined by a single assertion,
so the two terms are often used interchangeably.
.P
Both prerequisites and targets can represent many different types of objects.
They may be names of files, names of
.IR "state variables" ,
names of rules, or
.IR attributes .
A state variable is a variable that has both a value and a time stamp;
when the variable value changes its time stamp is also changes.
Attributes are special reserved words that are understood by
.B nmake
itself.
Attribute names all begin with a
.L .
and are all upper case.
The full list of attributes and their meaning can be found in the
Alcatel-Lucent documentation.
An example of an attribute is
.L .MAKE
which marks an action block to be executed by
.B nmake
rather than the default shell.
This is used in the base rules to define many
of the default rules and assertion operators.
An attribute that is often found in user makefiles is
.LR .DONTCARE .
It is normally a fatal error when a prerequisite cannot be found or made;
asserting the
.L .DONTCARE
attribute on the target:
.EX
\fItarget\fP : .DONTCARE
.EE
inhibits the error.
.P
A target whose name contains the character
.L %
defines a special type of rule called a
.IR metarule .
A metarule matches files using the
.L %
like the shell
.L *
wild card.
Metarules are common in the base and global rule files
and are used on occasion inside user makefiles.

.H 1 "Assignments and Variables"
An assignment can be one of the following four types:
.BL
.LI
A simple assignment of the form \fIname\fP \f5=\fP \fIvalue\fP
that delays the expansion of
.I value
until
.I name
is expanded.
.LI
A simple assignment of the form \fIname\fP \f5:=\fP \fIvalue\fP
that expands
.I value
before being assigned to
.IR name .
.LI
A append assignment of the form \fIname\fP \f5+=\fP \fIvalue\fP
that appends the expanded
.I value
to the current value of the variable
.IR name .
.LI
A state variable assignment of the form \fIname\fP \f5==\fP \fIvalue\fP.
.B nmake
scans source files for state variable references and
keeps track of changes to their values.
A state variable can be used as a prerequisite or a target by
specifying \f5(\fP\fIname\fP\f5)\fP.
.LE
.P
Variables are referenced using the notation \f5$(\fP\fIname\fP\f5)\fP
where \fIname\fP is the name of the variable.
In addition each
.I name
can be followed by a
.L :
separated list of expansion operators
that operate on each space separated token in the result of the expansion.
For example, the operator
\f5:N=\fP\fIpattern\fP\f5:\fP
selects tokens that match the shell
.I pattern
and the operator
\f5:T=F:\fP
binds each token to a file and expands the token to the bound path name.
.EX
FILES = main.c stdio.h
print $(FILES:N=*.h:T=F)
.EE
would print
.L /usr/include/stdio.h
on most UNIX systems.
The complete list of expansion operators can be found in the
Alcatel-Lucent documentation.
.P
.B nmake
uses certain conventions for naming related variables.
For example, while the variable
.L CC
names the C compiler as with
.BR make ,
other C compiler related variables also begin with
.LR CC .
Thus, the variable name
.L CCFLAGS
is used rather than
.LR CFLAGS .
In addition, other variables of the form
\f5CC.\fP\fIname\fP
are used to parameterize portable rules and assertion operators.
These are
.IR "probe variables" ,
described in the next section.
In general the user modifiable variables for
.I TOOL
are named
.I TOOLidentifier
and the the corresponding probe variables are named
.IR TOOL.identifier .
.P
.I "Special variables"
are readonly variables maintained by
.BR nmake .
They are used in actions and have values relative to the current
target and prerequisites.
Each special variable name is a single non-alphanumeric character that
may be repeated to access ancestor values, e.g.,
.L "$(<)"
names the current target,
.L "$(<<)"
names the parent target of the current target.
Special variable names may also be used as a rule name prefix to access
information about that rule, e.g.,
.L "$(*.SOURCE.h)"
expands to the prerequisites of the
.L .SOURCE.h
rule.
.P
Special variables are indispensable for several reasons:
.BL
.LI
bound file names are expanded; this is required for viewpath and
.L .SOURCE*
binding
.LI
.IR "specify once and right" :
rules can specify target and prerequisite names once, avoiding errors
caused by out of sync duplication
.LI
portions of rules can be shared, .e.g, edit operators applied to
\f5$(@\fP\fIrule\fP\f5)\fP
.LE
.P
The special variables are:
.VL 6
.LI
.L "$(<)"
The target name.
This may be a list for
.B .JOINT
rules that generate multiple targets from one action.
.LI
.L "$(>)"
The out of date file prerequisites.
This variable is most often used in metarule actions.
.LI
.L "$(*)"
All file prerequisites.
.LI
.L "$(~)"
All prerequisites.
.LI
.L "$(@)"
The action block.
.LI
.L "$(%)"
The metarule target stem or
.L .FUNCTIONAL
arguments.
.LI
.L "$(!)"
Explicit and generated file prerequisites.
.LI
.L "$(&)"
Explicit and generated state prerequisites.
.LI
.L "$(?)"
All explicit and generated prerequisites.
.LI
.L "$(^)"
The original bound name.
A generated target may bind to a file in another directory.
If the target is out of date then
.L "$(<)"
is the target name to be generated, and
.L "$(^)"
is the original name.
For example, a makefile that generates
.L ./libfoo.a
and installs
.L "$(LIBDIR)/libfoo.a"
may bind
.L -lfoo
to
.L "$(LIBDIR)/libfoo.a"
before
.L ./libfoo.a
is generated.
In this case
.L "$(^)"
is
.L "$(LIBDIR)/libfoo.a"
and
.L "$(<)"
is
.LR libfoo.a .
.LI
.L "$(-)"
The current
.B nmake
options, suitable for the command line.
This is a global special variable and is not target related.
\f5$(-\fP\fIoption\fP\f5)\fP
expands to the current setting for
.IR option .
.LI
.L "$(+)"
The current
.B nmake
options, suitable for the
.B nmake
.L set
builtin.
This is a global special variable and is not target related.
\f5$(+\fP\fIoption\fP\f5)\fP
expands to the current setting for
.IR option .
.LI
.L "$(=)"
The command line and
.L .EXPORT
variable settings, suitable for the command line.
This is a global special variable and is not target related.
.LI
.L "$(...)"
The list of all rules and attributes.
This special variable is often used with attribute or pattern match
edit operators.
For example,
.L "$(...:A=.TARGET)"
expands to the names of all target rules and
.L "$(...:N=*.o:T=F)"
expands to the bound names of all rules matching the shell pattern
.L *.o
that bind to existing files.
.LE

.H 1 "Make Probe Information"
The base rules conform to local OS conventions using information generated
by the
.BR probe (1)
command.
Probe information is maintained for each compiler in directories shared
by all users.
Currently
.B nmake
uses
.B make
and
.B pp
(C preprocessor) probe information.
The information is automatically generated and updated when the corresponding
.B probe
script changes.
.P
The make probe information is a set of variable definitions, all of the form
.BI CC. name.
See
.xx link="../probe/probe.html	probe information"
for details.
Typical user makefile make probe variables are:
.VL 6
.LI
.B CC.DEBUG
The
.B cc
option that enables object file debugging information.
.LI
.B CC.DLL
The
.B cc
option(s)
required for shared library object files.
.LI
.B CC.HOSTTYPE
The target architecture host type as determined by
.BR package (1).
.LI
.B CC.OPTIMIZE
The
.B cc
option that enables optimization.
.LI
.B CC.WARN
The
.B cc
option that enables verbose warning and error messages.
.LE

.H 1 "Assertion Operators"
.B nmake
makefiles typically use one or more higher level operators, and more
often than not skip the primitive
.LR : .
This section introduces some commonly used operators.
.P
The
.L ::
operator, whose
.I name
is the null string,
expects source file prerequisites rather than intermediates.
The
.L ::
operator is defined in the
.IR "base rules" .
It generates primitive assertions and actions based
on prerequisite file suffixes use base rule
.IR metarules .
The base rules correspond to the
.B makerules.mo
global rules file, provided with every
.B nmake
installation.
The base rules are read by default before any user makefiles are read.
.P
For example, the makefile
.EX
DEBUG == 1
prog :: prog.y helper.c -lxyz
.EE
specifies that the target program
.L prog
is constructed from the object files generated by
.LR prog.y ,
.L helper.c
and
.LR -lxyz ,
and that object file generation may depend on the variable
.LR DEBUG .
In this case if either
.L prog.c
(and/or
.LR helper.c )
contains the symbol
.L DEBUG
then the C compilation flags for
.L prog.o
(and/or
.LR helper.o )
will contain
.LR -DDEBUG .
In addition, if the value of
.L DEBUG
changes for some future invocation of
.B nmake
then
.L prog.o
(and/or
.LR helper.o )
will be recompiled with that new value.
.P
There are many other operators defined in the base rules.
The \f5:LIBRARY:\fP operator generates both
static and dynamic libraries:
.EX
CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)
xyz 4.0 :LIBRARY: xyz.c strmatch.c
$(INCLUDEDIR) :INSTALLDIR: xyz.h
.EE
The example makefile generates and installs both a static and shared library
named
.BR xyz .
.L $(CC.OPTIMIZE)
generates optimized object files and
.L $(CC.DLL)
generates object files suitable for shared libraries, and also instructs
.L :LIBRARY:
to generate a shared library.
The
.BR probe (1)
information determines the corresponding library file names.
For example, on linux these would be installed:
.EX
$(INCLUDEDIR)/xyz.h
$(LIBDIR)/libxyz.a
$(LIBDIR)/libxyz.so
$(LIBDIR)/libxyz.so.4.0
.EE
on win32 systems these would be installed:
.EX
$(INCLUDEDIR)/xyz.h
$(LIBDIR)/libxyz.a
$(LIBDIR)/xyz.lib
$(BINDIR)/xyz40.dll
.EE
and on
.B cygwin
systems these would be installed:
.EX
$(INCLUDEDIR)/xyz.h
$(LIBDIR)/libxyz.a
$(LIBDIR)/libxyz.dll.a
$(BINDIR)/cygxyz40.dll
.EE
The
.L :LIBRARY:
operator also deletes all static library object files after they have been
added to the static library;
.B nmake
is able to scan the libraries to determine the object file time
stamps using the
.BR libardir (3)
library, also used by
.BR pax (1).
.B libadir
mitigates the differences between vendor archive implementations.
.P
The
.L :PACKAGE:
operator provides portable and consistent external package references.
It appends package include directories, if any, to
.L .SOURCE.h
and package library directories, if any, to
.LR .SOURCE.a .
It also links all command and shared library targets with default
package libraries.
When a package cannot be found all other rules are disabled and
.B nmake
exits with a diagnostic message.
.EX
:PACKAGE: games X11

xgame :: README xgame.6 xgame.h xgame.c xutil.c -lXaw -lXmu -lXt

$(INCLUDEDIR) :INSTALLDIR: xgame.h
.EE
This example specifies that the command source files may reference the X11
headers, and the command executable links against the default X11 libraries,
along with the explicit X11 libraries
.BR -lXaw ,
.B -lXmu
and
.BR -lXt .
The first prerequisite of all
.L :PACKAGE:
assertions is the main package; the main package modifies the header
installation directory by appending the package subdirectory.
.L -
as the first prerequisite of
.L :PACKAGE:
specifies that the package is external.
For this example the main package is
.LR games ,
and
.L $(INCLUDEDIR)
is
.LR $(INSTALLROOT)/include/games .
.P
Many rules provided by the base rules
must be explicitly asserted in UNIX
.B make
makefiles.
For example, the rules
.LR clean ,
.L clobber
and
.L install
are predefined in the base rules, making them
unneeded in user makefiles.
In addition, the
.L pax
and
.L tgz
rules produce
.RB tar (1)
format archives containing source files defined explicitly or implicitly
by the makefile.
A source file is any local non-generated file that is used to generate a
target file.
To specify files that need to be part of the source archive
that are neither referenced in the Makefile or are implicit
prerequisites, use the \f5::\fP operator without any targets.
For example:
.EX
:: README Install Copyright
.EE
adds these files to the generated
.B pax
file when
.B nmake
is invoked with the target
.BR pax .

.H 1 "State File"
A major difference between
.B nmake
and
.B make
is that
.B nmake
uses results from previous runs
to decide what needs to be built.
This information is stored in a file called the
.IR "state file" .
The state file name is formed by adding a
.L .ms
suffix to the makefile name if it does not have a
.L .mk
suffix or by changing the
.L .mk
suffix to
.LR .ms .
The state file retains the following information:
.BL
.LI
File and event times for all generated and terminal files.
.LI
Explicit prerequisite lists.
.LI
Implicit prerequisite lists.
.LI
Action blocks used (before expansion.)
.LI
State variable values.
.LI
Target attributes.
.LE
If any of these have changed for a particular target then the target
is marked out of date.
This is a fundamental departure from
.BR make .
In particular, file time changes into the past or future are detected.
When a change is detected for a target its event time is set to
.IR now ,
and it is this time that is propagated to dependent targets.
.P
The
.I "implicit prerequisites"
are generated by applying
.I "scan rules"
to source files.
The scan rules themselves are specified by the
.L .SCAN
attribute.
The
.L .SCAN
attribute specifies that the action block is a scan description.
A scan description describes the commenting and quoting rules
as well as the syntax for identifiers, includes and conditional compilation.
Include files found within conditional compilation are
given the
.L .DONTCARE
attribute.
Only
.L .DONTCARE
files that exist will be tracked as implicit prerequisites; non-existent
.L .DONTCARE
files are silently ignored.
The scan rules are typically metarules that match files with
a given suffix.
In addition to scanning for include dependencies,
the scan rules check for state variable dependencies
by checking for state variable name matches within the scanned source files.
The base rules contain the scan rules for several
languages including C and C++.
.P
Implicit prerequisite generation is dynamic and incremental.
If a source file has changed from the last time it was scanned then
it is scanned again.
In addition,
if making a rule causes a file to be generated, and the file
that it generates matches a scan metarule, then the generated
file is scanned for dependency information.
This is in contrast with most
.B make
dependency generation tools that either generate this information statically
or collect it as a by-product of compilation
(requiring special compiler hooks.)
The latter technique is prone to
.I off-by-one
compile errors where generated include dependencies may not be noticed by
.B make
until after the compiler runs (and fails.)
.P
The scan information is saved in the state file so that
a file only needs to be scanned again if it has changed.
The state file also stores the value of state variables.
The base rules define variables such as \f5CC\fP and
\f5CCFLAGS\fP to be state variables so that a change
of compiler or compile options triggers recompilation.

.H 1 "Binding and Making"
The process of associating a target or prerequisite
with a physical object such as a file, rule or state variable is called
.IR binding .
The handling of directory searches for binding is described in the
.B "Multiple Directories"
section below.
.P
Names of the form \f5(\fP\fIname\fP\f5)\fP
are bound to state variables.
Names that contain a \f5%\fP are bound to metarules.
Other names may be bound to rules or to filenames.
.P
Libraries can be specified as either \f5-l\fP\fIlib\fP
or \f5+l\fP\fIlib\fP.
On systems that support both dynamic libraries and static libraries,
\f5-l\fP\fIlib\fP will prefer dynamic linking when both dynamic and
static versions of the library are present, whereas \f5+l\fP\fIlib\fP
will prefer the static library.
The dynamic/static preference is on a per-directory basis.
.P
The fundamental
.B nmake
operation is to
.I make
a target.
.B nmake
makes a target by recursively making each of its prerequisites.
To make a given prerequisite, it is first bound.
If it binds to a rule, then the targets for that rule are made.
If the prerequisite binds to a file or a state variable,
and
.B nmake
determines that it needs to be updated,
then the action for that file or state variable is executed.
.P
The method that
.B nmake
uses to determine whether a target needs to be
updated differs from that of
.BR make .
With
.BR make ,
if the file time of the target is older than
that of any of the prerequisites, then the action is triggered.
With
.B nmake ,
if the time of any of the prerequisites
saved in the state file differs (older
.I or
newer) from the time of the file
then action is triggered.
Moreover, if the action completes successfully, the time stamp of the target
is recorded in the state file even if it has not changed.
In this way an action need not change the time stamp
of the target if no change is needed, preventing
anything that depends on this target from being remade.
Another consequence of this model is that restoring an
old source file with the old time stamp is detected as a change by
.BR nmake .
For the first build, when there is no statefile, or
when the accept option, \f5\-A\fP, is specified on
the command line,
.B nmake
uses the
.B make
time stamp model.
.P
The actions for the prerequisites of a given target may be executed in any order;
they may even be executed concurrently.
.P
When
.B nmake
is invoked it makes the rules
.LR .INIT ,
.L .MAIN
and
.LR .DONE ,
in that order.
By default the prerequisite of
.L .MAIN
is the first user makefile target, usually
.L all
or
.L .ALL
defined by the
.L ::
assertion operator.

.H 1 "Multiple Directories"
Unlike
.BR make ,
.B nmake
supports a multiple directory model.
There is no reason that the files specified in the
makefile need to reside in a single directory.
It is often convenient to partition the source for a single makefile into
many subdirectories.
The
.B nmake
.L .SOURCE
rule specifies the directories (and directory order) to be searched
for prerequisite source files.
For example,
.EX
\|.SOURCE : lib common
.EE
specifies that the
.L lib
and
.L common
directories are to be searched, in that order,
when binding names to files.
The current directory
.L .
is always searched first.
In addition, source rules for searching files of any suffix
can be specified by using the target \f5.SOURCE.\fP\fIsuffix\fP.
Using source assertions for include files has another advantage.
The \f5::\fP and the \f5:LIBRARY:\fP operators use the \f5.SOURCE.h\fP
assertion to generate \f5-I\fP directives
for the C or C++ compilers.
.L -I
directives are generated only for directories containing files used for each
compilation.
.P
The ability to specify multiple search directories
eliminates the need for nested makefiles in many cases.
However, it is only useful for building something in
which all component names are known.
Large project management requires recursive makefiles.
Recursive
.B nmake
is
.I not
considered harmful; it is most emphatically
.I essential
for composing complex systems from smaller components.
The base rules supply the
.L :MAKE:
operator for this purpose.
With no prerequisites
.L :MAKE:
determines the build order for all subdirectories that contain a makefile.
Subdirectory makefiles may themselves contain
.L :MAKE:
operators, and these are handled recursively.
Project directory hierarchies typically have top level makefiles containing
a single
.L :MAKE:
assertion;
.B nmake
run at the top level will recursively run
.B nmake
in all subdirectories
.I "in the proper order" .
Subdirectory ordering is determined by
.L :PACKAGE:
assertions and
\f5-l\fP\fIlib\fP
and command prerequisites.
.P
The most important aspect of
.L :MAKE:
is that
.I "component directories"
can be added to or deleted from a directory hierarchy and the next
.B nmake
run will take these changes into account, without the need to modify
global project files or scripts.

.H 1 Viewpathing
Keeping source and generated files separate eases
file management when multiple target architectures are involved.
There are several ways to do this:
.BL
.LI
Make a complete copy of the source tree for each architecture:
although easy to manage, this technique is not space efficient.
And, since there are multiple copies of the source,
it's hard to keep the separate source copies from diverging.
.sp
.LI
Make architecture specific subdirectories for each source tree leaf directory:
this is much more space efficient since all architectures share one
copy of the source.
However, isolating the files for one particular architecture is non-trivial,
since architecture specific directories are distributed throughout the
entire source tree.
.EX
             package-root
               .  .  .
              .   .   .
             .    .    .
            .     .     .
           .      .      .
          .       .       .
         .        .        .
        bin      lib      src
        . .      . .      . .
       .   .    .   .    .   .
      A1   A2  A1   A2  lib cmd
                        .     .
                       .       .
                      .         .
                     .           .
                   libar         foo
                    . .          . .
                   .   .        .   .
                  A1   A2      A1   A2
.EE
.LI
Make a directory tree copy of the source tree (just directories, no files)
for each architecture and make a symlink in the copy for each regular file
in the source tree: this is space efficient and isolates the architecture
specific files under a separate tree.
It is too easy, however, to clobber original source files from within
the architecture specific trees.
.sp
.LI
Make a directory tree copy of the source tree (just directories, no files)
for each architecture and viewpath the architecture tree on top of the
source tree:
this is space efficient and safely separates source from generated files.
.EX
                         package-root
                         .          .
                    .                    .
               .                              .
              src                            arch
             .   .                          .    .
            .     .                      .          .
          lib     cmd                 .                .
          .         .                A1                A2
         .           .               .                  .
        .             .              .                  .
      libar          foo             .                  .
                                    src                src
                                   .   .              .   .
                                  .     .            .     .
                                 lib   cmd          lib   cmd
                                 .       .          .       .
                                .         .        .         .
                              libar      foo     libar      foo
.EE
Viewpathing also allows multiple source trees to be chained together;
this means that source from separate package root directories can be shared.
This technique is useful for isolating local developer debug and enhancement
modifications from the master source.
Viewpaths are specified in the
.L VPATH
environment variable as a
.L :
separated list of root directories:
.EX
VPATH=\fIdeveloper-root\fP:\fIHOSTTYPE-root\fP:\fImaster-root\fP
.EE
If the local host supports DLL preload then the
.BR 3d (1)
command can be used to provide a transparent viewpath view to all
dynamically linked commands.
The
.L VPATH
environment variable provides the initial view, and additional views are
specified with the
.BR ksh (1)
.BR vpath (1)
builtin:
.EX
vpath \fIdeveloper-root\fP \fIHOSTTYPE-root\fP
vpath \fIHOSTTYPE-root\fP \fImaster-root\fP
.EE
.LE

.H 1 Portability
The best way to achieve portability with
.B nmake
makefiles is to
.BL
.LI
specify as little as possible
.LI
use
.BR probe (1)
abstractions, e.g.,
.LR CC.DLL ,
.LR CC.SUFFIX.ARCHIVE ,
etc.
.LI
use predefined base rules operators and variables
.LI
specify all source for all systems and select via
.LR #ifdef s
in the source
.LI
avoid constructs that depend on a particular system or compiler
.LI
avoid makefile conditionals
.LI
avoid absolute pathnames
.LI
do not use explicit
.L -D
or
.L -I
options; define state variables for
.L -D
and
.LR .SOURCE *
assertions for
.L -I
.LI
use
.L .DONTCARE
for libraries that may not be on all systems; better yet, make them
prerequisites in
.L :LIBRARY:
assertions
.LI
use
.BR iffe (1)
to generate configuration dependent headers
.LE
.P
The
.B nmake
base rules ensure that the
.BR probe (1)
abstractions and default rules, operators and variables
function equivalently on all systems.

.H 1 iffe
.BR iffe (1)
(if feature exists)
is a tool that generates configuration header files based on information
probed from the current
.L "$(CC)"
and native system.
.B iffe
processes files that are normally stored in a
directory named
.LR features .
The resulting header files are stored in the directory named
.L FEATURE
.RI ( not
the more intuitive
.L FEATURES
to appease case ignorant filesystems.)
.B iffe
enabled source includes these files
and uses the generated macros in conditional compilation tests.
.P
.B iffe
is able to probe any non-interactive target system behavior through
user specified C programs and shell scripts.
However, many common precoded queries can be leveraged to produce simple
and compact
.B iffe
scripts:
.BL
.LI
Does header file \fIname\fP\f5.h\fP exist as a standard header?
.EX
hdr	\fIname\fP
.EE
.LI
Does header file \fIsys/name\fP\f5.h\fP exist as a standard header?
.EX
sys	\fIname\fP
.EE
.LI
Does the type \fItype\fP exist when
.B sys/types.h
and/or a given set of standard include files are included?
.EX
typ	\fIname\fP	[ \fIheader\fP.h ... ]
.EE
.LI
Is the function \fIname\fP in the standard library or one of
the given libraries?
.EX
lib	\fP\fIname\fP	[ \f5-l\fP\fIlib\fP ... ]
.EE
.LI
Is the data symbol \fIname\fP in the standard library or one of
the given libraries?
.EX
dat	\fP\fIname\fP	[ \f5-l\fP\fIlib\fP ... ]
.EE
.LI
Is the symbol \fIname\fP a member of a given structure when
a given set of files is included?
.EX
mem	\fIstruct_name.member_name\fP	[ \fIheader\fP.h ... ]
.EE
.LI
Is the command \fIname\fP found on the standard path?
.EX
cmd	\fIname\fP
.EE
.LE
Wherever an \fIoperator\fP or \fIname\fP can be specified, a comma separated
list of names can be specified, and the test will
be performed for each \fIoperator\fP and \fIname\fP in the list:
.EX
hdr,sys	stdio,fcntl,socket,mman
.EE
.P
Unlike
.BR configure (1) ,
.B iffe
generated macro names conform to a strict and consistent naming convention.
See
.BR iffe (1)
for more details.
For compatibility the
.L -C
or
.L --config
option can be used to coax
.B iffe
to generate
.BR configure (1)
\f5HAVE_\fP\fIfeature\fP style macros.
Converting from
.B configure
to
.B iffe
is worth the effort, for portability, stability, and size.
The corresponding
.B iffe
scripts are typically more compact and clearly delineate generated vs.
source files -- you didn't really think someone hand-coded that 10K line
.B ImageMagick
configure script.
Our equivalent
.B iffe
script is less than 100 lines.
Granted,
.B iffe
is implemented as a 4K line shell script, but
.I that
script remains constant across all projects.
We have some
.B iffe
scripts, still in use, that haven't changed since 1994.
.P
The base rules provide complete support for
.BR iffe (1)
through these metarules (action blocks omitted):
.EX
FEATURE/% : features/%
FEATURE/% : features/%.c
FEATURE/% : features/%.sh
% : %.iffe
.EE
Since
.B nmake
does automatic implicit prerequisite analysis, a source files need only
.EX
#include "FEATURE/\fP\fIfoo\fP\f5"
.EE
and \f5FEATURE/\fP\fIfoo\fP will be automatically generated/updated before
any dependent file is compiled.
.P
.B nmake
is known to work on these
.xx link="/sw/download/arch.html	architectures."
Although architecture independent makefiles is a prime portability goal, some
architecture specifics are unavoidable.
For the
.B ast
software most of these correspond to C compiler optimizer bugs,
i.e., generated code works fine without
.L "$(CC.OPTIMIZE)"
in
.L CCFLAGS
but fails with it set.
The base rules
.L :NOOPTIMIZE:
assertion operator provides per-file control over optimization:
.EX
CCFLAGS = $(CC.OPTIMIZE)

cmd :: a.c b.c c.c

"sol?.i386|sgi.*" :NOOPTIMIZE: b.c
.EE
This makefile disables C compiler optimization for
.L b.c
on
.L "$(CC.HOSTTYPE)"
architectures that match
.L sol?.i386
or
.LR sgi.* .

.H 1 Execution
.B nmake
can be run in command mode or interactively.
.L "nmake -n query"
enters the query mode;
rule names, assignments and assertions may be entered at the
.L "make>"
prompt.
Entering a name with no operators lists all variable and rule information
for that name.
State rule names are indicated by a balanced parenthese prefix followed
by the rule name, and state variable names are entirely enclosed
in parenthesis.
Query mode is especially helpful for writing and debugging rules.
.P
The \f5\-n\fP option of
.B nmake
is similar to that of
.BR make ,
except that targets with the
.L .ALWAYS
attribute are still executed.
The
.L -N
option inhibits all shell action execution.
The
.L -n
is passed to recursive
.B nmake
invocations (via \f5:MAKE:\fP assertions) using the
.L "$(-)"
special variable.
.P
When make begins execution, it looks for a file named
.L Makeargs
or
.L makeargs
in the current directory.
If found, each line of the file is inserted into the command line
option list.
This allows local additions/overrides without the need to modify
the underlying makefile, which may be on a different viewpath level.
For example a
.L Makeargs
file containing
.EX
--debug-symbols
.EE
will compile with compiler debugging flags enabled.
.P
Before the first action is executed
.B nmake
starts a single shell process, called the
.IR coshell ,
to execute all action blocks.
This cuts down on
.B nmake
.BR fork (2)
or
.BR spawn (2)
overhead and allows command execution to be handled by the most
efficient program.
While an action is executing
.B nmake
determines the next action to execute.
The \f5-j\fP\fInproc\fP option, or the \f5NPROC=\fP\fInproc\fP environment
variable allows
.B nmake
to execute
.I nproc
actions simultaneously.
Internally
.B nmake
builds a dynamic prerequisite graph;
this graph determines target actions that may execute concurrently.
The
.B .SEMAPHORE
attribute provides per-target concurrency control.
The
.B coshell
program is determined by the
.B COSHELL
or
.B SHELL
environment variables, checked in that order,
or
.L /bin/sh
by default.
.L COSHELL
may also point to
.BR coshell (1)
which supports concurrent action execution on separate hosts that share
the same filesystem.
.P
Sometimes it is necessary to circumvent the
.B nmake
state model.
For instance, the
.B ast
software bootstrap build uses
.BR mamake (1)
to build
.BR nmake .
If
.B nmake
were to be run on the bootstrap generated files they would be rebuilt
since the bootstrap provided no
.B nmake
state.
The
.L --accept
or
.L -A
option instructs
.B nmake
to use the
.B make
time stamp model and accept files that are up to date with respect to file
time stamps only.
An up to date state file is generated so that subsequent invocations,
with no other changes, will treat all files as up to date.
.L --accept
is often used with the
.L --touch
or
.L -t
option that sets the time stamp of all out of date generated files
to the current time.

.H 1 Variants
There are now three public variants of
.I make
named
.BR nmake :
.VL 6
.LI
.B "nmake (AT&T Research) 5.6 2011-12-13"
This is the version maintained by the original author.
.LI
.B "Alcatel-Lucent nmake 13"
The the variant that split in 1995 when AT&T spun off Lucent.
.LI
.B "MicroSoft NMAKE"
This variant has some features inspired by the 1985 Portland USENIX paper,
but that's where the similarities end.
.LE
.PP
Use
.EX
nmake -n -f - . 'print $(MAKEVERSION)'
.EE
to list the installed
.B nmake
version.
.P
Here are the known incompatibilities between the AT&T and Lucent
variants.
Bug fixes and backwards compatible enhancements to the AT&T variant
are not listed.

.VL 6
.LI
.B "output serialization"
Output for each action is collected until the action completes.
Without serialization action error messages during parallel builds
may get intermingled to the point of being useless.
.VL 6
.LI
.B AT&T
.B "nmake --serialize"
which sets the
.L CO_SERIALIZE
flag in the
.I coshell
library.
.LI
.B Lucent
Implemented with 3 new commands, one of which is a replacement wrapper for
.B nmake
itself.
.LE

.LI
.B "probe file override"
Allow a user to override the automatically generated probe files.
.VL 6
.LI
.B AT&T
.B nmake
has always searched for probe information in the
.B ../lib/probe/C/make/
directories on the
.B $PATH
environment variable.
In fact, all related files used a
.B $PATH -based
search: e.g.,
makefiles in
.B ../lib/make/ .
This was done to avoid a proliferation of
.IB foo PATH
variables
that would soon become a maintenance and portability nightmare.
To provide a local override, simply create a
.B ../lib/probe/C/make/
dir sibling to a dir on
.B $PATH ,
and place a user writable probe file in it.
Make sure the override dir is to the left of the
.B nmake
bin dir in
.B $PATH .
The probe file name is determined by running
.B "probe --key"
C make
.I "C-compiler-path" .
See
.B "probe --?override"
for more details.
.LI
.B Lucent
The user probe file is placed in a dir on
.B $PROBEPATH ,
or on
.B $VPATH ,
depending on the value of the
.B localprobe
variable.
.LE

.LI
.B "Makerules options"
.I --option=value
instead of
.I option=value
to control optional makerules behavior.
The difference is subtle but significant.
.I option
in the second (makerules variable) form can conflict
with environment and makefile variables.
It also lacks the self documentation of
.BI --?option
or
.B --man
from the
.B nmake
command line.
Makerules differences are the main point of divergence between
Lucent and AT&T
.BR nmake :
.VL 6
.LI
.B AT&T
The makerules variable form is supported for backwards compatibility.
Warnings will eventually be enabled to encourage use of the
makerules option form.
.I "Not all incompatibilities are caught" .
Compatibility checks are triggered by the
.B .PROBE.INIT
rule which is usually done early in the makefile input process.
There may some instances where the
.I --option=value
form must be used, either on the command line or as
.EX
set --\fIoption=value\fP
.EE
in a makefile or global rules file.
.LI
.B Lucent
Many makerules variable additions.
.LE

.LI
.B ":MAKE: ordering"
.VL 6
.LI
.B Lucent
Not implemented.
.LE
.LI
.B ":MAKE: tracing"
Emits a message when a
.B :MAKE:
directory is entered and exited.
By default the entry is listed as
.IR directory :.
.VL 6
.LI
.B AT&T
The directory name is prefixed with
.BI --recurse-enter= text
on entering a directory and
.BI --recurse-leave= text
on leaving a directory.
.LI
.B Lucent
The directory name is prefixed with the makerules variable
.B $(recurse_begin_message)
on entering a directory and
.B $(recurse_end_message)
on leaving a directory.
.LE

.LI
.B .ACTIONWRAP
.B $(.ACTIONWRAP)
is expanded instead of
.B $(@) .
.VL 6
.LI
.B AT&T
Not implemented, pending a more general solution.
.LI
.B Lucent
Its not clear how to prevent
.I all
actions from being wrapped.
.LE

.LI
.B "java support"
Different views on how to integrate java dependency analysis.
.VL 6
.LI
.B AT&T
Based on the ast-jmake package which provides
.BR jmake (1)
and the
.BR JAVA (1M)
and
.BR JAR (1M)
assertion operators.
Fully integrated with viewpathing and implicit prerequisite scanning
in a manner similar to
.BR cc (1)
and implicit includes.
.LI
.B Lucent
See the Alcatel-Lucent documentation.
.LE

.LI
.B "XML build logs"
A richer XML variant of
.I MAM
output.
.VL 6
.LI
.B AT&T
Not implemented, but a good idea for interopability.
Would most likely be implemented by enhancing the
.B --mam
option.
.LI
.B Lucent
Implemented by a using the command
.B xmakelog
instead of
.BR nmake .
Not clear from the documentation why a
.B nmake
option wouldn't do.
.LE

.H 1 Example
The
.B nmake
.L Makefile
and
.L config.iffe
.B iffe
script for the GNU
.BR diff ,
version 2.7, 1994, are listed below.
GNU
.B diff
is a component of the
.B ast-open
package at
.xx link="/sw/download/	Software Systems Research software."
.EX
:PACKAGE: ast

VERSION                 == "2.7"
RELEASE                 == "1994-10-01"

DIFF_PROGRAM            == "diff"
DEFAULT_EDITOR_PROGRAM  == "ed"
PR_PROGRAM              == "pr"

diff :: diff.c analyze.c dir.c io.c util.c context.c ed.c ifdef.c \e
        cmpbuf.c normal.c side.c

diff3:: diff3.c \e
        LICENSE='author="Randy Smith"'

sdiff:: sdiff.c \e
        LICENSE='since=1992,author="Thomas Lord"'

:: COPYING ChangeLog NEWS INSTALL README diagmeet.note \e
        diff.info diff.info-1 diff.info-2 diff.info-3 diff.info-4 \e
        diff.texi texinfo.tex \e
        Makefile.in config.hin

:MSGFUN: fatal message \e
        perror_fatal perror_with_exit perror_with_name pfatal_with_name
.EE
The
.L :PACKAGE:
assertion causes all programs to be compiled with the
.B ast
headers and the
.L -last
library.
.L "nmake install"
will build and install the three programs
.LR diff,
.L diff3
and
.L sdiff
in
.LR "$(BINDIR)" .
The state variable
.L ==
assignments generate properly quoted
.L -D
options in the compile lines for source files that reference the variables.
The
.L :MSGFUN:
assertion declares non-standard message function names for the base rules
.L msgcat
action that reaps source message strings and generates a C locale message
catalog using
.BR msgcc (1)
and
.BR msggen (1).
.B ast
message catalogs use C locale message text as lookup keys for text in
other locales.
The
.L :MSGFUN:
assertion and the
.L msgcat
action automate the process.
.P
All of the source files include
.LR config.h ;
this is automatically generated from
.LR config.iffe ,
listed below, by the base rules
.L "%:%.iffe"
metarule.
The per-command scoped
.L LICENSE
variable assignments provide state variable information for the
.B ast
library
.BR optget (3)
self-documenting option parser (used in place of the GNU
.BR getopt_long (3).)
.EX
set     config

hdr     dirent,fcntl,limits,ndir,time,unistd,vfork
hdr     float,stdarg,stdlib,string
key     const =
lib     dup2,memchr,sigaction,strchr,tmpnam
lib     vfork = fork
sys     dir,file,ndir,wait
typ     pid_t = int

tst     note{ signal handler return type }end compile{
                #include <sys/types.h>
                #include <signal.h>
                #undef signal
                extern void (*signal())();
        }end yes{
                #define RETSIGTYPE void
        }end no{
                #define RETSIGTYPE int
        }end

tst     CLOSEDIR_VOID note{ closedir() is a void function }end nocompile{
                #include <dirent.h>
                main()
                {
                        return closedir(0);
                }
        }end

STDC_HEADERS    = ( HAVE_FLOAT_H & HAVE_STDARG_H & HAVE_STDLIB_H & HAVE_STRING_H )
HAVE_ST_BLKSIZE = mem stat.st_blksize sys/stat.h

run{
        echo "#if _PACKAGE_ast"
        echo "#include <ast_std.h>"
        echo "#endif"
        case $HAVE_VFORK_H in
        1)      echo "#include <vfork.h>" ;;
        esac
}end
.EE
.L "set config"
in this
.B iffe
script generates
.BR configure (1)
compatible
.L HAVE_*
macro names.
The
.IR name = value
statements translate from consistent
.B iffe
names to the ad-hoc
.B configure
names expected in the GNU source files.
The
.L "lib vfork = fork"
statement defines the macro
.L vfork
to be
.L fork
if
.BR vfork (2)
is not in the default libraries.

.H 1 GLOSSARY
.VL 6
.LI
.I action
An \fIaction\fP is part of an \fIassertion\fP that is specified by
indentation. In can be a shell action, an
.B nmake
action, or a scan action.
.LI
.I assertion
An \fIassertion\fP is a statement in a \fImakefile\fP that describes
a \fIrule\fP or part of a \fIrule\fP.
An assertion consists of zero or more \fItarget\fPs, an \fIoperator\fP,
zero or more \fIprerequisites\fP, and is optionally followed by
an \fIaction\fP.
.LI
.I assignment
An \fIassignment\fP is a statement in a \fImakefile\fP the assigns
a value to a \fIvariable\fP.
.LI
.I attribute
An \fIattribute\fP is a special \fIprerequisite\fP or \fItarget\fP
that is used to mark \fIrule\fPs or \fItarget\fPs.
.LI
.I "base rules"
A set of rules that are defined in a makefile that
is shipped with
.BR nmake .
This file contains contains default
rules for building software components.
These rules are implicitly included by
.BR nmake .
.LI
.I bind
\fIbinding\fP is an action taken by
.B nmake
to associate a \fIprerequisite\fP
or \fItarget\fP name with a file or other object.
.LI
.I "component directory"
A directory (and its subdirectories) controlled by a makefile.
.LI
.I "global rules"
A makefile that can be created by a project
that contains rules shared by all components
in that project.
.LI
.I "implicit prerequisite"
A \fIprerequisite\fP that is not specified in the makefile.
.LI
.I makefile
A file consisting of a set of \fIassignment\fPs and \fIassertion\fPs.
.LI
.I making
\fImaking\fP is the action taken by
.B nmake
to bring a \fItarget\fP
up to date by \fIbinding\fP all the \fIprerequisite\fPs and
running the \fIaction\fPs associated with the target if
any of the \fIprerequisite\fPs have changed.
.LI
.I metarule
a \fImetarule\fP is a type of \fIrule\fP in which the \fItarget\fP and
\fIprerequisite\fP consists of a pattern containing the wildcard
character \f5%\fP.
.LI
.I operator
An \fIoperator\fP is part of a \fIrule\fP. It can be the operator \f5:\fP,
the operator \f5::\fP,
or else a \fItarget\fP of the form \f5:\fP\fIname\fP\f5:\fP.
.LI
.I "probe information"
The set of probe variables corresponding to the current
.LR "$(CC)" .
Probe variable values are are generated by a script that is maintained by
.BR probe (1).
The information is automatically generated and updated as probe script changes
are noticed.
Probe information updates do not explicitly depend on the time stamp of the
.L "$(CC)"
executable itself, since it is usually a wrapper program for other (private)
compiler passes.
.LI
.I "probe variable"
A variable that is part of the
.I "probe information"
defined by
.BR probe (1).
.LI
.I prerequisite
A \fIprerequisite\fP is an item that needs to be checked in
order to decide whether a \fItarget\fP needs to be updated.
.LI
.I rule
A \fIrule\fP consists of a \fItarget\fP, an \fIoperator\fP,
zero or more \fIprerequisites\fP, and an \fIaction\fP.
It is generated by combining all the \fIassertion\fPs associated with a
\fItarget\fP.
.LI
.I "scan rule"
A \fIscan rule\fP rule is a type of rule that specifies how
a give source file is to be scanned for include files and
state variables.
.LI
.I "state file"
A \fIstate file\fP is a machine independent format data file that
stores information from the previous run.
It is used as the bases of deciding whether any prerequisites
of a target have changed.
.LI
.I "state variable"
A \fIstate variable\fP is a \fIvariable\fP that is scanned for in source code
and whose value is saved in the \fIstate file\fP.
.LI
.I target
A \fItarget\fP is an item that comes before an \fIoperator\fP in an \fIassertion\fP.
.LI
.I variable
A \fIvariable\fP is an item
that can be used to hold an arbitrary string.
Variable names match the RE
.L [[:alpha:]_.][[:alnum:]_.]*
or one of the special variable names
.LR [-+=%#@<>*~!?] .
.LI
.I viewpath
A \fIviewpath\fP is an ordered set of directories that are used
to search for files during the \fIbind\fP process.
.LE

.H 1 REFERENCES
.VL 6
.LI
.xx link="../publications/open-2000-1.pdf	The AT&T AST OpenSource software collection",
with David Korn and Stephen North and Phong Vo,
Proceedings of the FREENIX Track 2000 Usenix Annual Technical Conference,
pp 187-195, San Diego, CA, June 2000.
.xx link="../publications/prus-1995-1.pdf	Practical Reusable UNIX Software",
chapter 11,
B. Krishnamurthy, editor, Wiley, 1995.
.LI
.IR "Configuration Management" ,
with D. G. Korn and H. Rao and J. J. Snyder and K. P. Vo,
.xx link="../publications/prus-1995-1.pdf	Practical Reusable UNIX Software",
chapter 3,
B. Krishnamurthy, editor, Wiley, 1995.
.LI
.IR "Libraries and File System Architecture" ,
with D. G. Korn and S. C. North and H. Rao and K. P. Vo,
.xx link="../publications/prus-1995-1.pdf	Practical Reusable UNIX Software",
chapter 2,
B. Krishnamurthy, editor, Wiley, 1995.
.LI
.IR "nDFS -- The multiple Dimensional File System" ,
with D. G. Korn and H. Rao,
.IR "Configuration Management" ,
chapter 5,
W. Tichy, editor, Wiley, 1994.
.LI
.IR "Feature Based Portability" ,
with J. J. Snyder and K. P. Vo,
USENIX VHLL Symposium, Santa Fe, NM, October 1994.
.LI
.xx link="../publications/coshell-1993.pdf	The Shell as a Service",
USENIX 1993 Summer Conference Proceedings, Cincinnati, OH, June 1993.
.LI
.IR "A User-Level Replicated File System" ,
with Y. Huang, D. G. Korn and H. C. Rao,
AT&T Bell Laboratories Technical Memorandum 0112670-930414-05, April 1993,
and USENIX 1993 Summer Conference Proceedings, Cincinnati, OH, June 1993.
.LI
.IR "Tools and Techniques for Building and Testing Software Systems" ,
with J. E. Humelsine and C. H. Olson,
AT&T Technical Journal, Vol. 71 No. 6, pp. 46-61, November/December 1992.
.LI
.xx link="../publications/make-1990.pdf	A Case for make",
Software \- Practice and Experience, Vol. 20 No. S1, pp. 30-46, June 1990.
.LI
.xx link="../publications/make-1988.pdf	Product Administration through SABLE and nmake",
with S. Cichinski,
AT&T Technical Journal, Vol. 67 No. 4, pp. 59-70, July/August 1988.
.LI
.xx link="../publications/make-1985.pdf	The Fourth Generation Make",
USENIX 1985 Summer Conference Proceedings, pp. 159-174, Portland, OR, June 1985,
and UNIFORUM 1986 Winter Conference Proceedings, pp. 157-168,
Anaheim, CA, February 1986.
.LI
.xx link="http://www.pcug.org.au/~millerp/rmch/recu-make-cons-harm.html	Recursive Make Considered Harmful",
Peter Miller, AUUGN Journal of AUUG Inc., 19(1), pp. 14-25, 1997.
.LE
