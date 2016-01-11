.xx meta.keywords="regex implementation categorization"
.MT 4
.TL
regex implementation categorization
.AF "AT&T Research - Florham Park NJ"
.AU "Glenn Fowler <gsf@research.att.com>"
.H 1
The
.B regex
tests in
.xx link=categorize.dat
attempt to categorize
.B regex
implementations.
The tests do not address internationalization.
All implementations report the leftmost match; this is omitted from the table.
.so re-categorize.tab
.P
The categories are:
.VL 6
.LI
.B LABEL
The implementation label from
.xx link="./	testregex."
.LI
.B ASSOC
Subpattern (or atom) associativity: either
.B left
or
.BR right .
The subexpression match rule in the rationale requires
.B right
for expressions where each concatenated part is a subexpression.
There is no definition for
.IR subpattern ,
but it would be inconsistent for any definition to require different
associativity than that for subexpressions.
Some claim that the BRE and ERE grammars specify
.B left
associativity, but this interpretation disregards
the subexpression match rule in the rationale.
The grammar can also be interpreted to support
.B right
associativity, and this interpretation is in accord with the rationale.
.LI
.B SUBEXPR
Subexpression semantics:
.B precedence
if subexpressions can override the default associativity;
.B grouping
if subexpressions are for repetition and
.B regmatch_t
grouping only.
The subexpression match rule in the rationale requires
.B precedence .
.LI
.B REP_LONGEST
How repeated subexpressions that match more than once are handled:
.B first
if the longest possible matches occur first;
.B last
if the longest possible matches occur last;
.B unknown
otherwise.
The subexpression match rule in the rationale requires
.B first .
.LI
.B BUGS
Miscellaneous bugs (see
.xx link=categorize.dat
for specific examples):
.VL 6
.LI
.B alternation-order
A change in the order of subexpression alternation operands,
.IR "not involved in a tie" ,
changes
.B regmatch_t
values.
Some implementations with this bug can be coaxed into missing the
overall longest match.
.LI
.B first-match
The first of the leftmost matches, instead of the longest of the
leftmost matches, is returned.
.LI
.B nomatch-match
A back-reference to a
.B regmatch_t
(-1,-1) value is treated as matching.
.LI
.B range-null
A range-repeated subexpression that matches null does not report the match
at offset (0,0).
.LI
.B repeat-artifact
A
.B regmatch_t
value is reported for a repeated match that is not the last match.
.LI
.B repeat-artifact-nomatch
To prevent not matching,
a
.B regmatch_t
value is reported for a repeated match that is not the last match.
.LI
.B repeat-null
A repeated subexpression matches the null string even though it is not
the only match and is not necessary to satisfy the exact or minimum
number of occurrences for an interval expression.
.LI
.B repeat-short
Incorrect
.B regmatch_t
values for a repeated subexpression.
This may be a variant of
.BR repeat-artifact .
.LI
.B subexpression-first
A subexpression match takes precedence over a subpattern
to its left.
.LE
.LE
