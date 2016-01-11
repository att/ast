.TL "" ""
A Tour of the ast Commands and Libraries
.AU "Glenn Fowler" gsf MH HA6163000 2195 2B-134 "gsf@research.att.com"
.MT 4
.H 1 Introduction
The
.I ast
commands and libraries were developed by the
Software Engineering Research Department
.RI ( aka
the Advanced Software [Technology] Department)
at AT&T Bell Laboratories, Murray Hill, NJ.
The strength of
.I ast
is how its individual components
combine to form a cohesive computing environment across UNIX\u\(rg\d
and non-UNIX platforms.
Cohesiveness*
.FS
Footnote test.
.FE
is maintained by well-defined library interfaces for
algorithms upon which the commands are built.
In many cases a library implementation was the driving force behind
command implementations.
.H 1 Motivation
Why should you consider using the
.I ast
software?
After all, many of the commands look like what's already in
.L /bin
and
.LR /usr/local/gnu/bin .
Although there is some replication, there are also some commands you
won't find anywhere else:
.I the
ksh93, nmake, the 3d filesystem, cia, and yeast.
.H 1 Installation
The
.I ast
software installs in a single directory hierarchy, rooted at
.LR $INSTALLROOT ,
usually
.L /usr/local/ast
or
.LR /usr/add-on/ast .
The top level
.L $INSTALLROOT
directories are:
.EX
               bin      executable binaries and scripts
               fun      \fIksh93\fP shell functions
               lib      libraries
       lib/\fIcommand\fP      related files for \fIcommand\fP
       src/cmd/\fIxxx\fP      source for command \fIxxx\fP
    src/lib/lib\fIxxx\fP      source for library \fIxxx\fP
.EE
To access the commands and related data files:
.EX
export PATH=$INSTALLROOT/bin:$PATH
.EE
For each
.I command
that has a related data file
.IR file ,
.I file
is found by searching
.L $PATH
for
.IL "" ../lib/ command / file ,
e.g.,
the magic file for the file command is
.L ../lib/file/magic
on
.LR $PATH .
If
.L $HOME/bin
is before
.L $INSTALLROOT/bin
in
.L $PATH
then you can selectively override standard
.I ast
related files by placing them in the directory
.IL "" $HOME/lib/ command .
This allows executables to be built without hard-coded pathnames and also
requires only a change in
.L $PATH
when
.L $INSTALLROOT
changes.
.P
On systems with shared libraries one of
.EX
export LD_LIBRARY_PATH=$INSTALLROOT/lib:$LD_LIBRARY_PATH
export LIBPATH=$INSTALLROOT/lib:$LIBPATH
export LPATH=$INSTALLROOT/lib:$LPATH
.EE
is required to locate the
.I ast
shared libraries.
.H 1 Exploration
.I tw
combines
.I find
and
.IR xargs .
It provides C style expressions on path names and elements of
.LR "struct stat" .
To find suspicious executables:
.EX
tw -d / -e "uid=='root' && (mode&'u+s') && parent.uid!=uid"
.EE
to change the owner of all of bozo's files:
.EX
tw -d / -e "uid=='bozo'" chown clown:circus
.EE
.I tw
collects file pathname arguments up to the exec arg limit before it
executes the command.
For paranoid users:
.EX
tw chmod go-rwx
.EE
which is equivalent to:
.EX
chmod -R go-rwx
.EE
(Now you don't need to add a
.L -R
option to your favorite commands.)
To find all source files that include
.LR foo.h :
.EX
tw -e "name=='*.[chly]'" grep -l '^#.*include.*["<]foo\.h[>"]'
.EE
.P
.I libast
handles the magic number matching for the
.I file
command.
The magic file,
.LR $INSTALLROOT/lib/file/magic ,
is carefully constructed to give the same output across all architectures:
.EX
cd $HOME/arch
file */bin/cat
.EE
might produce:
.EX
att.i386/bin/cat:   elf i386 executable, 32-bit, little-endian, ..
bsd.i386/bin/cat:   bsd 386 executable, compact, paged, pure, no..
hp.pa/bin/cat:      hp pa-risc executable, shared, dynamically l..
sgi.mips2/bin/cat:  elf mips executable, 32-bit, dynamically lin..
sol.sun4/bin/cat:   elf sparc executable, 32-bit, dynamically li..
sun4/bin/cat:       sun sparc executable, paged, dynamically lin..
.EE
.I tw
uses the same interface, making it easy to search for files based on type,
name and content.
The following searches for executable scripts:
.EX
tw -e "(mode&'+x') && type!=DIR && magic!='*executable*'"
.EE
The
.I tw
algorithm efficiently detects file system loops, so following
symbolic links is not a problem.
The same algorithm is used by all
.I ast
commands that traverse
directory hierarchies,
and the following options to control pathname resolution:
.EX
-L (logical)         follow symbolic links
-P (physical)        don't follow symbolic links
-H (metaphysical)    -L for command line files, -P otherwise
.EE
The
.I ksh93
.I getconf
builtin controls the default traversal mode:
.EX
getconf PATH_RESOLVE - { logical, physical, metaphysical }
.EE
.H 1 Configuration
.H 1 Compression
.H 1 Conclusion
