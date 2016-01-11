.xx meta.keywords="regex regular expression standard interpretation"
.TL
An Interpretation of the POSIX regex Standard
.AF "AT&T Research - Florham Park NJ"
.AU "Glenn Fowler <gsf@research.att.com>"
.MT 4
.ND "January 2003"
.AS 2
Many passages in the POSIX
.B regex
standard seem to be open for interpretation.
Differences between several published
.xx link="http://www.research.att.com/~gsf/testregex/	implementations"
of the
.B regex
API bear this out.
Instead of relegating these differences to the
.I "undefined behavior"
bucket, this paper proposes a resolution to each
by direct application of the standard text.
.AE

.H 1 "Background"
The POSIX
.B regex
standard is spread across four documents:
.TS
center;
r c l.
glossary	G	\h'0*1'http://www.opengroup.org/onlinepubs/007904975/basedefs/xbd_chap03.html\h'0'
api	A	\h'0*1'http://www.opengroup.org/onlinepubs/007904975/functions/regcomp.html\h'0'	API"
definition	D	\h'0*1'http://www.opengroup.org/onlinepubs/007904975/basedefs/xbd_chap09.html\h'0'
rationale	R	\h'0*1'http://www.opengroup.org/onlinepubs/007904975/xrat/xbd_chap09.html\h'0'
.TE
.P
It describes
.BR BRE s
(basic regular expressions, a.k.a.,
.BR grep (1)
style) and
.BR ERE s
(extended regular expressions, a.k.a.,
.BR egrep (1)
style)
and how an RE of each type matches subject strings.
The standard also provides an API:
.BR regcomp (3)
for compiling an RE, and
.BR regexec (3)
for matching a compiled RE against a subject string.
The
.B regexec
API
.EX
   int regexec(const regex_t* restrict preg, const char* restrict string,
               size_t nmatch, regmatch_t pmatch[restrict], int eflags);
.EE
is at the center of multiple, conflicting interpretations of the standard.
These interpretations differ on the setting of the
.L pmatch[]
array for index values > 0.
This note presents examples that demonstrate interpretation conflicts,
and then provides standard references that,
.IR "when taken as a whole" ,
resolve the conflicts.

.H 1 "Notation"
Standard references use the notation
[\fIdocument\fP:\fIbegin\fP[-\fIend\fP]]
where
.I document
is the document letter, { A D G R }, from the table above,
.I begin
is the beginning line number, and
.I end
is the ending line number.
Line numbers are taken from the 2001 X/Open printing.
Unfortunately the online links do not display line numbers.
For example, [A:37179-37180] is the reference for the
.B regexec
API prototype above.
.P
Example patterns, subject strings, and
.L pmatch[]
array values use the regression test notation of
.xx link="http://www.research.att.com/~gsf/testregex/	testregex."
You can download the source and compile it against your favorite regex
implementation.
All of the examples in this note have been placed in the file
.xx link="http://www.research.att.com/~gsf/testregex/interpretation.dat	interpretation.dat;"
you can download this file and use it as input to
.BR testregex .
For example, the
.B testregex
input
.EX
:RE#01:E	a+			xaax	(1,3)
.EE
specifies that the ERE pattern "a+" matched against the
subject string "xaax" yields
.L pmatch[0].rm_so==1
and
.LR pmatch[0].rm_eo==3 .
The example is labeled RE#01 for indexing and referencing.
.EX
:RE#02:B	.\e(a*\e).		xaax	(0,4)(1,3)
.EE
specifies that the BRE pattern ".\e(a*\e)." matched against the subject
string "xaax" yields
.LR pmatch[0].rm_so==0 ,
.LR pmatch[0].rm_eo==4 ,
.LR pmatch[1].rm_so==1 ,
.LR pmatch[1].rm_eo==3 .
(?,?) denotes
.L rm_so
and
.L rm_eo
values of -1, i.e., a non-match.
The first field allows additional flags that exercise all of the
.B REG_*
.B regcomp
and
.B regexec
flags; see
.BR testregex (1)
or
.B "testregex --man"
for details.
Note that
.B tab
is the field separator in the
.B testregex
syntax; if you mouse snarf then make sure that
.B tabs
are preserved.

.H 1 "regex Glossary"
.VL 6 3
.LI
.RB [G:41] "Basic Regular Expression (BRE)"
A regular expression used by the majority of utilities that select strings
from a set of character strings. 
.LI
.RB [G:148] "Entire Regular Expression"
The concatenated set of one or more basic regular expressions or extended
regular expressions that make up the pattern specified for string selection. 
.LI
.RB [G:158] "Extended Regular Expression (ERE)"
A regular expression that is an alternative to the Basic Regular
Expression using a more extensive syntax, occasionally used by some utilities. 
.LI
.RB [G:269] "Pattern"
A sequence of characters used either with regular expression notation or for
pathname expansion, as a means of selecting various character strings or
pathnames, respectively. 
.LI
.RB [G:316] "Regular Expression"
A pattern that selects specific strings from a set of character strings. 
.LE

.H 1 "A subexpression is ..."
The
.B regex
standard is surprisingly cavalier with terminology:
some terms are used interchangeably, some are used in a general context
in one section and a specific context in another, and some are
used without any definition whatsoever.
Acutely subject to this abuse are:
.IR RE ,
.IR pattern ,
.IR subpattern ,
.IR expression ,
and
.IR subexpression .
In particular,
.I subpattern
and
.I subexpression
are central to the description of the matching algorithm and how
.L pmatch[]
is assigned.
Any interpretation of the
.B regex
standard involving these terms, absent a precise and accurate definition
for each, is useless.
.P
.I subexpression
appears 70 times, and each reference is in the context of parenthesis grouping:
.VL 6 3
.LI
[D:5909-5911]
For example, matching the BRE "\e(.*\e).*" against "abcdef" , the
subexpression "(\e1)" is "abcdef" , and matching the BRE
"\e(a*\e)*" against "bc" , the subexpression "(\e1)" is the null
string.
.LI
[D:5984-5988]
The asterisk shall be special except when used: As the first
character of a subexpression (after an initial '^' , if any);
.LI
[D:6094-6097]
A subexpression can be defined within a BRE by enclosing it
between the character pairs "\e(" and "\e)" . Subexpressions can
be arbitrarily nested.
.LI
[D:6100-6109]
The character 'n' shall be a digit from 1 through 9, specifying
the nth subexpression (the one that begins with the nth "\e("
from the beginning of the pattern and ends with the
corresponding paired "\e)" ). The expression is invalid if less
than n subexpressions precede the '\en' . For example, the
expression "\e(.*\e)\e1$" matches a line consisting of two
adjacent appearances of the same string, and the expression
"\e(a\e)*\e1" fails to match 'a' . When the referenced
subexpression matched more than one string, the back-referenced
expression shall refer to the last matched string. If the
subexpression referenced by the back-reference matches more
than one string because of an asterisk ( '*' ) or an interval
expression (see item (5)), the back-reference shall match the
last (rightmost) of these strings.
.LI
[D:6110-6112]
When a BRE matching a single character, a subexpression, or a
back-reference is followed by the special character asterisk ('*' ),
together with that asterisk it shall match what zero or
more consecutive occurrences of the BRE would match.
.LI
[D:6114-6117]
When a BRE matching a single character, a subexpression, or a
back-reference is followed by an interval expression of the
format "\e{m\e}" , "\e{m,\e}" , or "\e{m,n\e}" , together with that
interval expression it shall match what repeated consecutive
occurrences of the BRE would match. "\e{m,n\e}" , together with
that interval expression it shall match what repeated
consecutive occurrences of the BRE would match.
.LI
[D:6127-6129]
A subexpression repeated by an asterisk ('*') or an interval expression
shall not match a null expression unless this is the only match for the
repetition or it is necessary to satisfy the exact or minimum number of
occurrences for the interval expression.
.LI
[D:6136]
Subexpressions/back-references \e(\e) \en
.LI
[D:6145-6151]
The implementation may treat the circumflex as an anchor when
used as the first character of a subexpression. The circumflex
shall anchor the
expression (or optionally subexpression) to the beginning of a
string; only sequences starting at the first character of a
string shall be matched by the BRE. For example, the BRE "^ab"
matches "ab" in the string "abcdef" , but fails to match in the
string "cdefab" . The BRE "\e(^ab\e)" may match the former
string. A portable BRE shall escape a leading circumflex in a
subexpression to match a literal circumflex.
.LI
[D:6152-6156]
A dollar sign ( '$' ) shall be an anchor when used as the last
character of an entire BRE. The implementation may treat a
dollar sign as an anchor when used as the last character of a
subexpression. The dollar sign shall anchor the expression (or
optionally subexpression) to the end of the string being matched;
the dollar sign can be said to match the end-of-string following
the last character.
.LI
[D:6265-6270]
A circumflex ( '^' ) outside a bracket expression shall anchor
the expression or subexpression it begins to the beginning of a
string; such an expression or subexpression can match only a
sequence starting at the first character of a string. For
example, the EREs "^ab" and "(^ab)" match "ab" in the string
"abcdef" , but fail to match in the string "cdefab" , and the
ERE "a^b" is valid, but can never match because the 'a'
prevents the expression "^b" from matching starting at the
first character.
.LI
[D:6271-6276]
A dollar sign ( '$' ) outside a bracket expression shall anchor
the expression or subexpression it ends to the end of a string;
such an expression or subexpression can match only a sequence
ending at the last character of a string. For example, the EREs
"ef$" and "(ef$)" match "ef" in the string "abcdef" , but fail
to match in the string "cdefab" , and the ERE "e$f" is valid,
but can never match because the 'f' prevents the expression
"e$" from matching ending at the last character.
.LI
[R:2359-2370]
It is possible to determine what strings correspond to
subexpressions by recursively applying the leftmost longest
rule to each subexpression, but only with the proviso that the
overall match is leftmost longest. For example, matching
"\e(ac*\e)c*d[ac]*\e1" against acdacaaa matches acdacaaa (with
\e1=a); simply matching the longest match for "\e(ac*\e)" would
yield \e1=ac, but the overall match would be smaller (acdac).
Conceptually, the implementation must examine every possible
match and among those that yield the leftmost longest total
matches, pick the one that does the longest match for the
leftmost subexpression, and so on. Note that this means that
matching by subexpressions is context-dependent: a
subexpression within a larger RE may match a different string
from the one it would match as an independent RE, and two
instances of the same subexpression within the same larger RE
may match different lengths even in similar sequences of
characters. For example, in the ERE "(a.*b)(a.*b)" , the two
identical subexpressions would match four and six characters,
respectively, of accbaccccb.
.LI
[R:2512-2520]
The limit of nine back-references to subexpressions in the RE
is based on the use of a single-digit identifier; increasing
this to multiple digits would break historical applications.
This does not imply that only nine subexpressions are allowed
in REs. The following is a valid BRE with ten subexpressions:
.EX
\e(\e(\e(ab\e)*c\e)*d\e)\e(ef\e)*\e(gh\e)\e{2\e}\e(ij\e)*\e(kl\e)*\e(mn\e)*\e(op\e)*\e(qr\e)*
.EE
The standard developers regarded the common historical
behavior, which supported "\en*" , but not "\en\e{min,max\e}" ,
"\e(...\e)*" , or "\e(...\e)\e{min,max\e}" , as a non-intentional
result of a specific implementation, and they supported both
duplication and interval expressions following subexpressions
and back-references.
.LI
[R:2537-2544]
However, one relatively uncommon case was changed to allow an
extension used on some implementations. Historically, the BREs
"^foo" and "\e(^foo\e)" did not match the same string, despite
the general rule that subexpressions and entire BREs match the
same strings. To increase consensus, IEEE Std 1003.1-2001 has
allowed an extension on some implementations to treat these two
cases in the same way by declaring that anchoring may occur at
the beginning or end of a subexpression. Therefore, portable
BREs that require a literal circumflex at the beginning or a
dollar sign at the end of a subexpression must escape them.
Note that a BRE such as "a\e(^bc\e)" will either match "a^bc" or
nothing on different systems under the rules.
.LI
[R:2549-2554]
Some implementations have extended the BRE syntax to add
alternation. For example, the subexpression "\e(foo$\e|bar\e)"
would match either "foo" at the end of the string or "bar"
anywhere. The extension is triggered by the use of the
undefined "\e|" sequence. Because the BRE is undefined for
portable scripts, the extending system is free to make other
assumptions, such that the '$' represents the end-of-line
anchor in the middle of a subexpression. If it were not for the
extension, the '$' would match a literal dollar sign under the
rules.
.LI
[R:2617-2620]
The removal of the Back_open_paren Back_close_paren option from
the nondupl_RE specification is the result of PASC
Interpretation 1003.2-92 #43 submitted for the ISO POSIX-2:1993
standard. Although the grammar required support for null
subexpressions, this section does not describe the meaning of,
and historical practice did not support, this construct.
.LI
[A:37188]
size_t re_nsub Number of parenthesized subexpressions
.LI
[A:37206-37208]
If the REG_NOSUB flag was not set in cflags, then regcomp()
shall set re_nsub to the number of parenthesized subexpressions
(delimited by "\e(\e)" in basic regular expressions or "()" in
extended regular expressions) found in pattern.
.LI
[A:37220-37257]
If nmatch is 0 or REG_NOSUB was set in the cflags argument to
regcomp(), then regexec() shall ignore the pmatch argument.
Otherwise, the application shall ensure that the pmatch
argument points to an array with at least nmatch elements, and
regexec() shall fill in the elements of that array with offsets
of the substrings of string that correspond to the
parenthesized subexpressions of pattern: pmatch[i].rm_so
shall be the byte offset of the beginning and pmatch[i].rm_eo
shall be one greater than the byte offset of the end of
substring i. (Subexpression i begins at the ith matched open
parenthesis, counting from 1.) Offsets in pmatch[0] identify
the substring that corresponds to the entire regular
expression. Unused elements of pmatch up to pmatch[nmatch-1]
shall be filled with -1. If there are more than nmatch
subexpressions in pattern ( pattern itself counts as a
subexpression), then regexec() shall still do the match, but
shall record only the first nmatch substrings.
.sp
When matching a basic or extended regular expression, any given
parenthesized subexpression of pattern might participate in the
match of several different substrings of string, or it might
not match any substring even though the pattern as a whole did
match. The following rules shall be used to determine which
substrings to report in pmatch when matching regular
expressions:
.AL
.LI
If subexpression i in a regular expression is not contained
within another subexpression, and it participated in the match
several times, then the byte offsets in pmatch[i] shall
delimit the last such match.
.LI
If subexpression i is not contained within another
subexpression, and it did not participate in an otherwise
successful match, the byte offsets in pmatch[i] shall be -1. A
subexpression does not participate in the match when:
.DS I F 0
\|'*' or "\e{\e}" appears immediately after the
subexpression in a basic regular expression, or '*' ,
\|'?' , or "{}" appears immediately after the
subexpression in an extended regular expression, and
the subexpression did not match (matched 0 times)
.sp
or:
.sp
\|'|' is used in an extended regular expression to select
this subexpression or another, and the other
subexpression matched.
.DE
.LI
If subexpression i is contained within another subexpression
j, and i is not contained within any other subexpression that
is contained within j, and a match of subexpression j is
reported in pmatch[j], then the match or non-match of
subexpression i reported in pmatch[i] shall be as described in
1. and 2. above, but within the substring reported in pmatch[
j] rather than the whole string. The offsets in pmatch[i] are
still relative to the start of string.
.LI
If subexpression i is contained in subexpression j, and the
byte offsets in pmatch[j] are -1, then the pointers in pmatch[
i] shall also be -1.
.LI
If subexpression i matched a zero-length string, then both
byte offsets in pmatch[i] shall be the byte offset of the
character or null terminator immediately following the
zero-length string.
.LE
.LI
[A:37363-37366]
The regexec() function must fill in all nmatch elements of
pmatch, where nmatch and pmatch are supplied by the
application, even if some elements of pmatch do not correspond
to subexpressions in pattern. The application writer should
note that there is probably no reason for using a value of
nmatch that is larger than preg-> re_nsub+1.
.LI
[A:37407-37413]
The number of subexpressions in the RE is reported in re_nsub
in preg. With this change to regexec(), consideration was given
to dropping the REG_NOSUB flag since the user can now specify
this with a zero nmatch argument to regexec(). However, keeping
REG_NOSUB allows an implementation to use a different (perhaps
more efficient) algorithm if it knows in regcomp() that no
subexpressions need be reported. The implementation is only
required to fill in pmatch if nmatch is not zero and if
REG_NOSUB is not specified.
.LE
.P
This sentence is as close as the standard gets to a definition:
.VL 6 3
.LI
[A:37225-37226]
Subexpression i begins at the ith matched open parenthesis, counting from 1.
.LE
.P
Using nonterminals from the BRE [D:6371-6731] and ERE [D:6452-6452] grammar
productions (text not listed in this document) yields the following:
.VL 6 3
.LI
.B DEFINITION
A
.I subexpression
corresponds to the
.L "Back_open_paren RE_expression Back_close_paren"
form of the
.L nondupl_RE
BRE grammar production or
the
.L "'(' extended_reg_exp ')'"
form of the
.L ERE_expression
ERE grammar production.
Subexpression i begins at the ith matched open parenthesis
.RL ( Back_open_paren
for BREs and '(' for EREs),
starting from the left and counting from 1.
Subexpression 0 is the entire RE.
.LE
.P
This definition and the subexpression match rule [R:2359-2370] can be used to
to examine a class of EREs where the top level catenation operands are
subexpressions.
(A top level subexpression is not contained in any other subexpression
except subexpression 0.)
The subexpression match rule in pseudo code is:
.BL
.LI
determine the longest of the leftmost matches for subexpression-0
[R:2359-2361]
.LI
for 1<=\fIi\fP<=\fBre_nsub\fP
determine the longest match for
.RI subexpression- i
consistent with the matches already determined for
.RI subexpression- j,
0<=\fIj\fP<\fIi\fP.
[R:2359-2370] [A:37235-37257]
.LE
For example, given
.EX
:RE#03:E	(a?)((ab)?)		ab	(0,2)(0,0)(0,2)(0,2)
.EE
the subexpressions are:
.EX
subexpression-0	(a?)((ab)?)
subexpression-1	(a?)
subexpression-2	((ab)?)
subexpression-3	(ab)
.EE
The longest of the leftmost matches for subexpression-0 is (0,2).
The longest match for subexpression-1, consistent with the match
for subexpression-0, is (0,0); otherwise if it had matched (0,1) then
subexpression-2 would not match and the subexpression-0 match would be
limited to (0,1).
The longest match for subexpression-2, consistent with the matches
for subexpression-0 and subexpression-1, is (0,2).
The longest match for subexpression-3, consistent with the matches
for subexpression-0, subexpression-1 and subexpression-2, is (0,2).
This table illustrates the matching:
.EX
subexpr	pattern			match
   0	(a?)((ab)?)		(0,2)
   1	(a?)			(0,0)
   2	((ab)?)			(0,2)
   3	(ab)			(0,2)
.EE
RE#04 is a similar example that exposes the associativity of subexpression
concatenation:
.EX
:RE#04:E	(a?)((ab)?)(b?)		ab	(0,2)(0,1)(1,1)(?,?)(1,2)

subexpr	pattern			match
   0	(a?)((ab)?)(b?)		(0,2)
   1	(a?)			(0,1)
   2	((ab)?)			(1,1)
   3	(ab)			(?,?)
   4	(b?)			(1,2)
.EE
[R:2363-2365] also shows that parenthesis can be used to alter the 
order of matching:
.EX
:RE#05:E	((a?)((ab)?))(b?)	ab	(0,2)(0,2)(0,0)(0,2)(0,2)(2,2)

subexpr	pattern			match
   0	((a?)((ab)?))(b?)	(0,2)
   1	((a?)((ab)?))		(0,2)
   2	(a?)			(0,0)
   3	((ab)?)			(0,2)
   4	(ab)			(0,2)
   5	(b?)			(2,2)
.EE
In RE#05 the extra parenthesis (around subexpression-1 and subexpression-2 in
RE#04) form a new subexpression-1, and change the
match for the last subexpression
.L (b?)
to (2,2) (from (1,2) in RE#04.)
.EX
:RE#06:E	(a?)(((ab)?)(b?))	ab	(0,2)(0,1)(1,2)(1,1)(?,?)(1,2)

subexpr	pattern			match
   0	(a?)(((ab)?)(b?))	(0,2)
   1	(a?)			(0,1)
   2	(((ab)?)(b?))		(1,2)
   3	((ab)?)			(1,1)
   4	(ab)			(?,?)
   5	(b?)			(1,2)
.EE
In RE#06 the extra parenthesis pair forces right associativity and results
in the same match of (1,2) for the last subexpression
.L (b?)
as in RE#04.
These examples show that:
.VL 6 3
.LI
.B PROPERTY
Subexpression grouping can alter the precedence of concatenation.
.LI
.B PROPERTY
Subexpression concatenation is right associative.
.LE
.P
The following examples examine replicated subexpressions.
.EX
:RE#07:E	(.?)			x	(0,1)(0,1)
:RE#08:E	(.?){1}			x	(0,1)(0,1)
:RE#09:E	(.?)(.?)		x	(0,1)(0,1)(1,1)
:RE#10:E	(.?){2}			x	(0,1)(1,1)
:RE#11:E	(.?)*			x	(0,1)(0,1)
.EE
[D:6227-6234] specifies that RE#07 and RE#08 are equivalent, and that
RE#09 and RE#10 are equivalent, and
[D:6217-6219] specifies that RE#09 and RE#11 are equivalent.
.VL 6 3
.LI
[D:6227-6234]
When an ERE matching a single character or an ERE enclosed in
parentheses is followed by an interval expression of the format "{m}" ,
"{m,}" , or "{m,n}" , together with that interval expression it shall
match what repeated consecutive occurrences of the ERE would match. The
values of m and n are decimal integers in the range 0 <= m<= n<=
{RE_DUP_MAX}, where m specifies the exact or minimum number of
occurrences and n specifies the maximum number of occurrences. The
expression "{m}" matches exactly m occurrences of the preceding ERE,
"{m,}" matches at least m occurrences, and "{m,n}" matches any number
of occurrences between m and n, inclusive.
.LI
[D:6217-6219]
When an ERE matching a single character or an ERE enclosed in
parentheses is followed by the special character asterisk ( '*' ),
together with that asterisk it shall match what zero or more
consecutive occurrences of the ERE would match.
.LE
In RE#09 subexpression-1 matches (0,1), leaving the null string at (1,1) for
subexpression-2.
In RE#10 the first iteration of subexpression-1 matches (0,1), the same
as subexpression-1 in RE#09, and the second iteration of subexpression-1
matches (1,1), the same as subexpression-2 in RE#09.
RE#07 and RE#08 show that only one iteration is needed to match the subject
string, so the match in RE#11 requires only one iteration, and as such is the
last iteration of [D:6107-6109] [A:37235-37237].
RE#10 and RE#11 also illustrate [D:6127-6129] [D:6239-6241], which
specify that a repeated RE matches the null string only if it is the only
match (not this case) or if it is necessary to satisfy an interval expression
minimum (2 in this case.)
.VL 6 3
.LI
[D:6239-6241]
An ERE matching a single character repeated by an '*' , '?' , or an
interval expression shall not match a null expression unless this is
the only match for the repetition or it is necessary to satisfy the
exact or minimum number of occurrences for the interval expression.
.LE
.P
The following examples dig deeper into replicated subexpressions.
.EX
:RE#12:E	(.?.?)			xxx	(0,2)(0,2)
:RE#13:E	(.?.?){1}		xxx	(0,2)(0,2)
:RE#14:E	(.?.?)(.?.?)		xxx	(0,3)(0,2)(2,3)
:RE#15:E	(.?.?){2}		xxx	(0,3)(2,3)
:RE#16:E	(.?.?)(.?.?)(.?.?)	xxx	(0,3)(0,2)(2,3)(3,3)
:RE#17:E	(.?.?){3}		xxx	(0,3)(3,3)
:RE#18:E	(.?.?)*			xxx	(0,3)(2,3)
.EE
Here RE#14 shows that only two iterations are needed for a complete match,
making the last iteration match for RE#18 (2,3), since the first
iteration matched (0,2), as in RE#14.

.H 1 "A subpattern is ..."
The term
.I subpattern
appears exactly once:
.VL 6
.LI
[D:5907-5908]
Consistent with the whole match being the longest of the leftmost matches,
each subpattern, from left to right, shall match the longest possible string.
.LE
Consider RE#04 and RE#05 again:
.EX
:RE#04:E	(a?)((ab)?)(b?)		ab	(0,2)(0,1)(1,1)(?,?)(1,2)
:RE#05:E	((a?)((ab)?))(b?)	ab	(0,2)(0,2)(0,0)(0,2)(0,2)(2,2)
.EE
If a subpattern were an entity that combined adjacent subexpressions,
e.g.,
.L (a?)((ab)?)
in RE#04, then [D:5907-5908] would violate [R:2359-2370].
Similarly, if a subpattern were an entity that "went inside" subexpressions,
e.g.,
.L (a?)
in RE#05, then again [D:5907-5908] would violate [R:2359-2370].
In other words, a subpattern can be neither larger than nor smaller than
a subexpression;
a subpattern must be a grammatical entity equivalent to a subexpression.
This corresponds to the nonterminal
.L nondupl_RE
in the BRE grammar; there is no direct correspondence to a nonterminal
in the ERE grammar.
However, if the optional duplication operator (*,+,?,range) is included then
subpattern corresponds to
.L simple_RE
in the BRE grammar and
.L ERE_expression
in the ERE grammar, and both [D:5907-5908] and [R:2359-2370] are satisfied.
.VL 6 3
.LI
.B DEFINITION
A
.I subpattern
corresponds to the
.L "simple_RE"
nonterminal in the BRE grammar or the
.L "ERE_expression"
nonterminal in the ERE grammar.
.LE
This means that subexpressions and subpatterns are of equal importance
in RE matching.
Also note that any other definition for subpattern will put
[D:5907-5908] in direct conflict with [R:2359-2370].
.P
RE#19, RE#20 and RE#21 examine the relationship between subexpression
and subpattern:
.EX
:RE#19:E	a?((ab)?)(b?)		ab	(0,2)(1,1)(?,?)(1,2)
:RE#20:E	(a?)((ab)?)b?		ab	(0,2)(0,1)(1,1)(?,?)
:RE#21:E	a?((ab)?)b?		ab	(0,2)(1,1)(?,?)
.EE
.P
These are all variations of RE#04.
Other than subexpression renumbering, the match for the subexpression
.L ((ab)?)
must be the same in RE#04, RE#19, RE#20 and RE#21.
.L a?
is a subpattern in RE#19 and RE#21, of equal matching importance to
.L (a?)
in RE#04, and
.L b?
is a subpattern in RE#20 and RE#21, of equal matching
importance to
.L (b?)
in RE#04.

.H 1 "The Dark Corners ...
The remaining examples explore dark corners of the standard
and implementations.
Although the differences between some of the examples are subtle,
for some implementations it may mean the difference between an answer and
a core dump.
.P
In RE#22 subexpression
.L (a*)
matches the null string at (0,0), and continues to match at that position
until the minimal range count is satisfied.
.EX
:RE#22:E	(a*){2}			xxxxx	(0,0)(0,0)
.EE
RE#23 through RE#27 expose implementations that sometimes do
.I "first match"
for alternation within subexpressions.
Some implementations erroneously match the first iteration of
subexpression-1 in RE#24 through RE#27 to (0,1).
RE#27 is equivalent to RE#26; the match requires two iterations, the first
matching (0,2) and the last matching (2,3).
.EX
:RE#23:E	(ab?)(b?a)		aba	(0,3)(0,2)(2,3)
:RE#24:E	(a|ab)(ba|a)		aba	(0,3)(0,2)(2,3)
:RE#25:E	(a|ab|ba)		aba	(0,2)(0,2)
:RE#26:E	(a|ab|ba)(a|ab|ba)	aba	(0,3)(0,2)(2,3)
:RE#27:E	(a|ab|ba)*		aba	(0,3)(2,3)
.EE
RE#28 through RE#33 expose implementations that report short matches
for some repeated subexpressions.
Some implementations report incorrect matches for
subexpression-1 in RE#30 and RE#33.
.EX
:RE#28:E	(aba|a*b)		ababa	(0,3)(0,3)
:RE#29:E	(aba|a*b)(aba|a*b)	ababa	(0,5)(0,2)(2,5)
:RE#30:E	(aba|a*b)*		ababa	(0,5)(2,5)
:RE#31:E	(aba|ab|a)		ababa	(0,3)(0,3)
:RE#32:E	(aba|ab|a)(aba|ab|a)	ababa	(0,5)(0,2)(2,5)
:RE#33:E	(aba|ab|a)*		ababa	(0,5)(2,5)
.EE
RE#34 through RE#36 expose implementations that report subexpression matches
for earlier iterations of the subexpression.
Some implementations report a match for subexpression-2 in RE#36
while reporting the (2,3) match for subexpression-1: clearly a bug.
.EX
:RE#34:E	(a(b)?)			aba	(0,2)(0,2)(1,2)
:RE#35:E	(a(b)?)(a(b)?)		aba	(0,3)(0,2)(1,2)(2,3)(?,?)
:RE#36:E	(a(b)?)+		aba	(0,3)(2,3)(?,?)
.EE
RE#37 and RE#38 expose implementations that give priority to subexpression
matching over subpattern matching.
.EX
:RE#37:E	(.*)(.*)		xx	(0,2)(0,2)(2,2)
:RE#38:E	.*(.*)			xx	(0,2)(2,2)
.EE
RE#39 through RE#41 expose implementations that treat explicit vs. implicit
subexpression repetition differently.
This is a theme common to many of the previous examples.
Again, the subexpression in RE#41 requires two iterations to match,
and the second iteration matches (5,7), as illustrated by RE#40.
.EX
:RE#39:E	(a.*z|b.*y)		azbazby	(0,5)(0,5)
:RE#40:E	(a.*z|b.*y)(a.*z|b.*y)	azbazby	(0,7)(0,5)(5,7)
:RE#41:E	(a.*z|b.*y)*		azbazby	(0,7)(5,7)
.EE
RE#42 is another
.I "first match"
test.
Some implementations erroneously report a match of (0,1) for subexpression-1.
.EX
:RE#42:E	(.|..)(.*)		ab	(0,2)(0,2)(2,2)
.EE
RE#43 through RE#45 require only one iteration of subexpression-1 to
match the entire subject string.
RE#45 exposes three separate bugs in the implementations that were tested.
The most common was
.IR "over iteration" ,
where subexpression-1 is matched for a second iteration to the null string
at (3,3).
.EX
:RE#43:E	((..)*(...)*)			xxx		(0,3)(0,3)(?,?)(0,3)
:RE#44:E	((..)*(...)*)((..)*(...)*)	xxx		(0,3)(0,3)(?,?)(0,3)(3,3)(?,?)(?,?)
:RE#45:E	((..)*(...)*)*			xxx		(0,3)(0,3)(?,?)(0,3)
.EE
RE#46 through RE#82 are nasty;
backreferences are intuitive neither for the implementor nor the user.
.P
RE#49, RE#53, RE#67 and RE#68 illustrate the second part of the
.I subpattern
rule:
.VL 6
.LI
[D:5908-5909]
For this purpose, a null string shall be considered to be longer than
no match at all.
.LE
RE#53 requires close examination to see why the match is (0,2)(1,1)(2,2)
instead of (0,2)(0,1)(?,?).
The match of (0,1) for subexpression-1 is longer than (1,1), but
subexpression-1 can be repeated, and that second iteration allows
subexpression-2 to match (2,2), which is longer than (?,?) by [D:5908-5909].
.EX
:RE#46:B	\e(a\e{0,1\e}\e)*b\e1	ab	(0,2)(1,1)
:RE#47:B	\e(a*\e)*b\e1		ab	(0,2)(1,1)
:RE#48:B	\e(a*\e)b\e1*		ab	(0,2)(0,1)
:RE#49:B	\e(a*\e)*b\e1*		ab	(0,2)(1,1)
:RE#50:B	\e(a\e{0,1\e}\e)*b\e(\e1\e)	ab	(0,2)(1,1)(2,2)
:RE#51:B	\e(a*\e)*b\e(\e1\e)		ab	(0,2)(1,1)(2,2)
:RE#52:B	\e(a*\e)b\e(\e1\e)*		ab	(0,2)(0,1)(?,?)
:RE#53:B	\e(a*\e)*b\e(\e1\e)*		ab	(0,2)(1,1)(2,2)
:RE#54:B	\e(a\e{0,1\e}\e)*b\e1	aba	(0,3)(0,1)
:RE#55:B	\e(a*\e)*b\e1		aba	(0,3)(0,1)
:RE#56:B	\e(a*\e)b\e1*		aba	(0,3)(0,1)
:RE#57:B	\e(a*\e)*b\e1*		aba	(0,3)(0,1)
:RE#58:B	\e(a*\e)*b\e(\e1\e)*		aba	(0,3)(0,1)(2,3)
:RE#59:B	\e(a\e{0,1\e}\e)*b\e1	abaa	(0,3)(0,1)
:RE#60:B	\e(a*\e)*b\e1		abaa	(0,3)(0,1)
:RE#61:B	\e(a*\e)b\e1*		abaa	(0,4)(0,1)
:RE#62:B	\e(a*\e)*b\e1*		abaa	(0,4)(0,1)
:RE#63:B	\e(a*\e)*b\e(\e1\e)*		abaa	(0,4)(0,1)(3,4)
:RE#64:B	\e(a\e{0,1\e}\e)*b\e1	aab	(0,3)(2,2)
:RE#65:B	\e(a*\e)*b\e1		aab	(0,3)(2,2)
:RE#66:B	\e(a*\e)b\e1*		aab	(0,3)(0,2)
:RE#67:B	\e(a*\e)*b\e1*		aab	(0,3)(2,2)
:RE#68:B	\e(a*\e)*b\e(\e1\e)*		aab	(0,3)(2,2)(3,3)
:RE#69:B	\e(a\e{0,1\e}\e)*b\e1	aaba	(0,4)(1,2)
:RE#70:B	\e(a*\e)*b\e1		aaba	(0,4)(1,2)
:RE#71:B	\e(a*\e)b\e1*		aaba	(0,3)(0,2)
:RE#72:B	\e(a*\e)*b\e1*		aaba	(0,4)(1,2)
:RE#73:B	\e(a*\e)*b\e(\e1\e)*		aaba	(0,4)(1,2)(3,4)
:RE#74:B	\e(a\e{0,1\e}\e)*b\e1	aabaa	(0,4)(1,2)
:RE#75:B	\e(a*\e)*b\e1		aabaa	(0,5)(0,2)
:RE#76:B	\e(a*\e)b\e1*		aabaa	(0,5)(0,2)
:RE#77:B	\e(a*\e)*b\e1*		aabaa	(0,5)(0,2)
:RE#78:B	\e(a*\e)*b\e(\e1\e)*		aabaa	(0,5)(0,2)(3,5)
:RE#79:B	\e(x\e)*a\e1		a	NOMATCH
:RE#80:B	\e(x\e)*a\e1*		a	(0,1)(?,?)
:RE#81:B	\e(x\e)*a\e(\e1\e)		a	NOMATCH
:RE#82:B	\e(x\e)*a\e(\e1\e)*		a	(0,1)(?,?)(?,?)
:RE#83:E	(aa(b(b))?)+		aabbaa	(0,6)(4,6)(?,?)(?,?)
:RE#84:E	(a(b)?)+		aba	(0,3)(2,3)(?,?)
:RE#85:E	([ab]+)([bc]+)([cd]*)		abcd		(0,4)(0,2)(2,3)(3,4)
:RE#86:B	\e([ab]*\e)\e([bc]*\e)\e([cd]*\e)\e1	abcdaa		(0,5)(0,1)(1,3)(3,4)
:RE#87:B	\e([ab]*\e)\e([bc]*\e)\e([cd]*\e)\e1	abcdab		(0,6)(0,2)(2,3)(3,4)
:RE#88:B	\e([ab]*\e)\e([bc]*\e)\e([cd]*\e)\e1*	abcdaa		(0,6)(0,1)(1,3)(3,4)
:RE#89:B	\e([ab]*\e)\e([bc]*\e)\e([cd]*\e)\e1*	abcdab		(0,6)(0,2)(2,3)(3,4)
:RE#90:E	^(A([^B]*))?(B(.*))?		Aa		(0,2)(0,2)(1,2)
:RE#91:E	^(A([^B]*))?(B(.*))?		Bb		(0,2)(?,?)(?,?)(0,2)(1,2)
:RE#92:B	.*\e([AB]\e).*\e1			ABA		(0,3)(0,1)
:RE#93:B$	[^A]*A				\enA		(0,2)
.EE

.H 1 "Conclusion"
It is possible to use the 2001 issue of the POSIX
.B regex
standard,
.IR "with the addition of one sentence" ,
to resolve the interpretation differences that have surfaced since 1995.
That key sentence is a precise and consistent definition for the term
.IR subpattern .
By noting the relationship between
.I subpatterns
and
.IR subexpressions ,
the proposed definition is shown to be the only one that can be
consistent with all parts of the standard.
