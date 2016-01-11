.xx title="astsa"
.MT 4
.TL

.H 1 "astsa"
.B astsa
implements a small subset of the
.B ast
library for other
.B ast
standalone commands and libraries using X/Open interfaces. 
.P
To get better performance and functionality, consider using any of
the full-featured ast-* packages at
.EX
.xx link="http://www.research.att.com/sw/download/"
.EE
.P
If you already have non-standalone ast installed then you don't
need any of the astsa apis.
.P
If you already have non-standalone versions of any of
.EX
 aso cdt sfio vmalloc
.EE
installed then run this with the api args you already have before
running the first make:
.EX
 ./astsastd [ aso ] [ cdt ] [ sfio ] [ vmalloc ]
.EE
astsa.omk is an old make makefile that builds the headers and objects
and defines these variables for use in other makefiles
.EX
 ASTSA_OPTIMIZE	``-O'' by default
.EE
The astsa files may be combined in a single directory with other ast
standalone packages.
