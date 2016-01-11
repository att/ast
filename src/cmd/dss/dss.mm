.xx noindex
.xx meta.description="data stream scan"
.xx meta.keywords="compression data scan query"
.MT 4
.TL
DSS: Data Stream Scan
.AF "AT&T Labs Research - Florham Park NJ"
.AU "Glenn Fowler <gsf@research.att.com>"
.AU "Balachander Krishnamurthy <bala@research.att.com>"

.H 1 Abstract
.I dss
(data stream scan) is a framework for describing, transforming,
reading, querying, and writing streams of record oriented data.
It is implemented as a command and library API.
The API is extended by DLLs (shared libraries) that define data domain
specific I/O, type and query functions.
The goal is to provide a best in class repository for data scanning,
along with up to date documentation as a side effect of
coding to the API.
Numerous large scale network applications have used
.IR dss ,
significantly reducing the amount of time spent in coding.
Typical applications run terabytes of data through
.I dss
and have constructed custom queries very quickly.
The key reasons for the success of
.I dss
are its unique architecture,
generic template, and speed.
In speed,
.I dss
compares extremely favorably against
.BR perl (1),
the typical recourse in the networking community, and against customized
C/C++ code written to deal with a single domain dataset as well.
The range of applications for which
.I dss
has been
successfully applied over the last two years is wide and has involved
large volumes of
.I netflow
data,
.I BGP
data, HTTP proxy and server
logs, and OSPF LSA (Link State Advertisements.)
Often the use of
.I dss
has led to fearless exploration of ideas
across very large volumes of data.

.H 1 Introduction
Parsing is a critical part of record oriented data analysis.
It involves describing the record structure and splitting the
record fields into a form suitable for the analysis tools.
Each tool may have its own idiosyncratic record description and field
identifier syntax and format,
and some tools may be better at hiding the details than others.
For example, tools like
.BR grep (1) ,
.BR awk (1)
and
.BR perl (1)
intermix low and high level definition details within the same program.
Although relational databases provide high level abstractions and GUI
interfaces to the end user, a database expert is usually involved with
loading the data in the first place.
.P
It is not uncommon to have many analysis tools, each with an independent
parser,
applied to the same data within a single group of analysts.
This in itself is not a bad situation.
A relational database may be overkill for an application that needs
to visit each record only once;
hand coded programs may be inappropriate for data that is too large to fit
in memory, or that may benefit from indexed access, or that may undergo many
record format changes.
However, subtle parsing differences between independent analyses can make
comparisons difficult.
The absense of read or load errors does not necessarily mean that the data
was read correctly.
Certainly data access bugs can be fixed as they are discovered, but in the
worst case this could mean translating the fix to every analysis tool.
In some cases it may even be impossible to recover from bugs that affected
intermediate legacy data.
.P
.I dss
provides a data abstraction that can be used to mitigate parsing differences
between different analysis tools.
The key is that the abstraction provides efficient (in space and time) IO
support
.I and
a uniform interface for all data.
The approach has multiple benefits:
.BL
.LI
allows data analysts to focus on analyzing data
.LI
provides a target API for data IO coders
.LI
third parties familiar with the data abstraction model are
automatically proficient in new data domains
.LE
.P
.I dss
did not originate in a vacuum -- it arose out of experience with
.B BGP
data analysis in our lab.
A few in-house
.B BGP
tools gained popularity with the analysts.
Each analyst stepped up with new
.B BGP
feeds and file formats:
Ciso router table dumps,
.B MRT
format data, and ad-hoc flat files.
Each new format prompted a recoding effort, which by this time meant
changing a handful of related but slightly different implementations
that had evolved over a four year period.
Merging the
.B BGP
IO and parsing into a unified API was a big time saver, and also uncovered
several embarassing bugs that had remained hidden for years.
In addition, by employing best of class coding practices
(i.e., releasing the inner hackers)
the merged interface produced an efficient
.B MRT
parser that was 10 times faster than the one published by the originators
of the format.
By the time the analysts requested help on
.B NETFLOW
data the lesson had been learned, and
.I dss
was designed and implemented.

.H 1 "Data Abstraction Model"
.I dss
partitions data stream access into components that
form a uniform model for all data.
The model guides both the implementation and user interfaces.
It supports independent data methods, each with one or more physical
formats.
For example, Cisco router dump and
.B MRT
formats for the
.B BGP
method,
and all versions of the Cisco dump format for the
.B NETFLOW
method.
.P
Each user visible item, whether in the base API or in DLL extensions,
is placed in a dictionary.
A dictionary entry has a name and descriptive text suitable 
for inclusion in a
.BR man (1)
page.
This information can be listed by the runtime interfaces, so up to date
documentation is always available.
.P
Figure 1 illustrates the component relationships in the
.I dss
model.
Some components are part of the default
.I dss
library, but most are implemented as independent DLLs that
are selected and loaded at runtime.
The
.I dss
components are described below.

.H 2 "TRANSFORM Components"
Transforms are generic data independent filters that are applied
to the raw data.
They are automatically detected and applied on input, and are selectively
applied on output, as determined by user and/or data specific options.
.I dss
currently supplies compression transforms for
.IR gzip ,
.IR pzip ,
.I bzip
and
.IR compress ;
delta (\fIfdelta\fP) and encryption support is planned.
A transform may be implemented within the main
.I dss
process
(e.g., the
.BR sfio (3)
.BR gzip (1)
discipline)
or as a separate process (e.g.,
.I gzip
or
.I gunzip
connected by a pipe.)
The current vs. separate process choice may even be delayed until runtime.
For example, on a multi-cpu architecture with idle processors, running
.I gunzip
as a separate process may take less real time than a single process
implementation.
.P
Transforms allow data to be stored in the most efficient (or secure) manner.
The data need only be converted when accessed.
Given the volumes of available network data, compression or deltas,
along with sampling, is essential for archiving.

.H 2 "METHOD Components"
Methods describe the data records for a specific data domain.
A method must be specified before
.I dss
accesses any data.
The method data description includes:
.BL
.LI
A dictionary of file storage formats with:
.BL
.LI
An identification function that automatically determines input formats.
A format may be a different version (\fInetflow\fP versions 1, 5 and 7) or
a completely different structure (\fIBGP\fP \fIMRT\fP files vs. \fICisco\fP
router dumps.)
.LI
A record read function.
This function also handles format specific headers and trailers.
.LI
A record write function.
This function also handles format specific headers and trailers.
.LE
.LI
A dictionary of record field types and names that includes the union of
fields available in all supported formats.
Fields absent in a particular format will have empty values.
For example, the
.I dss
.B netflow
method supports all popular netflow formats.
The same queries can be used on any of the
.I netflow
formats.
The
.B "source mask"
field, not available in the version 1 format, is still available for
version 1 queries, but will have a default value of 0.
If a new
.B foo
field were to be introduced in the version 101 format then it would simply
be added to the
.B netflow
method dictionary but would have an empty value for all but version 101
(and newer) data.
.LI
An optional canonical record that provides access to a C structure
representation of each record.
A method that provides a canonical record can simplify ad-hoc C queries.
The ad-hoc query can be implemented as a
.I dss
.B QUERY
component function.
This function is called for each record in the input data.
.LE

.H 2 "TYPE Components"
A
.I dss
type provides functions that convert data between internal and external data
representations.
Some types may also have a base type that provides alternate access to the
same data.
Non-numeric constants are represented as \f5'...'\fP or \f5"..."\fP quoted
strings.
.P
For example, the
.B ipaddr_t
type in the
.I dss
.B ip_t
type library implements an IP address as a 32 bit unsigned integer
(the base type), and converts between the integer representation and
quoted dotted-quad and DNS names.
.P
A type may optionally provide a match function that overrides the default
regular expression string match.
For example, the
.B ipaddr_t
type match function does IP prefix matching and the
.B aspath_t
type match function does AS path regular expression matching that treats
AS numbers as individual tokens (as opposed to the inefficient alternative
of converting the AS path to a string and applying a string regular
expression match.)

.H 2 "QUERY Components"
Queries are the user visible part of
.IR dss .
They are used to select, filter, and summarize data streams.
There are two forms of queries.
Interpreted queries provide named access to the data fields using
C style operators.
Dynamic queries provide
.I UNIX
style command access to the record stream.
Queries may be composed in a manner similar to a
.I UNIX
pipeline.
The main difference is that a pointer to the current record, as opposed to
the actual record data, is passed between query components.
Interpreted queries are generic and may be used with any method.
Dynamic queries may be generic or method specific; method specific queries
produce a diagnostic when used with the wrong input method.

.H 1 "dss Library Interface"
The library provides two main interfaces for handling data:
.BL
.LI
C style expression interpreter:
.BL
.LI
basic
.BR number ,
.B string
and
.B reference
types
.LI
compile time callout functions for expression optimization and overloads
.LI
execution time callout functions for all operators
.LE
.LI
domain specific methods:
.BL
.LI
data IO functions
.LI
data types and conversion functions
.LI
specific data field types and names
.LI
method types for handling domain data in different formats
.LE
.LE
The library applies
.BR gzip (1)
and
.BR pzip (1)
decompression as needed for all methods.
.P
Applications based on the library can:
.BL
.LI
perform IO on the data stream
.LI
report information about the data stream
.LI
evaluate expressions to filter records from the stream
.LI
format output for reports or further processing
.LE

.H 1 "dss Command Interface"
The
.BR dss (1)
command provides a command level interface to the
.BR dss (3)
library.
.EX
dss --man
.EE
lists the
.BR man (1)
page on the standard error.
.EX
dss --?method
.EE
lists the available methods; methods are implemented in DLLs
found on sibling directories of
.BR $PATH .
.EX
dss -x foo --man
.EE
provides details on the
.I foo
method.

.H 1 "Performance"
Real time for typical
.I netflow
data queries.
.TS
center allbox;
b b b b b
l l n n n.
Command	Window	Records	Size	Time
interpreted	10 min	1,715,850	-	0m8.27s
library	10 min	1,715,850	-	0m4.27s
interpreted	1 day	229,448,460	27Gb	17m34s
library	1 day	229,448,460	27Gb	9m23s
ascii pipe	1 day	229,448,460	27Gb	56m43s
.TE

.H 1 REFERENCES
.VL 1i
.LI
.xx link="../publications/dss-2004.pdf	DSS: Data Stream Scan",
with Balachander Kirshnamurthy.
.LE
