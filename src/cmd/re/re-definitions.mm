http://cm.bell-labs.com/7thEdMan/

http://www.opengroup.org/onlinepubs/007904975/basedefs/xbd_chap03.html#tag_03_269

	Basic Regular Expression (BRE)
		A regular expression (see Regular Expression ) used by
		the majority of utilities that select strings from a
		set of character strings.

	Entire Regular Expression
		The concatenated set of one or more basic regular
		expressions or extended regular expressions that make
		up the pattern specified for string selection.

	Extended Regular Expression (ERE)
		A regular expression (see also Regular Expression )
		that is an alternative to the Basic Regular Expression
		using a more extensive syntax, occasionally used by
		some utilities.

	Matched
		A state applying to a sequence of zero or more
		characters when the characters in the sequence
		correspond to a sequence of characters defined by a
		basic regular expression or extended regular expression
		pattern.

	Pattern
		A sequence of characters used either with regular
		expression notation or for pathname expansion, as a
		means of selecting various character strings or
		pathnames, respectively.

	Regular Expression
		A pattern that selects specific strings from a set of
		character strings.

http://www.opengroup.org/onlinepubs/007904975/basedefs/xbd_chap09.html

	For example, matching the BRE "\(.*\).*" against "abcdef" , the
	subexpression "(\1)" is "abcdef" , and matching the BRE
	"\(a*\)*" against "bc" , the subexpression "(\1)" is the null
	string.

	The asterisk shall be special except when used: As the first
	character of a subexpression (after an initial '^' , if any);

	A subexpression can be defined within a BRE by enclosing it
	between the character pairs "\(" and "\)" . Subexpressions can
	be arbitrarily nested.

	The character 'n' shall be a digit from 1 through 9, specifying
	the nth subexpression (the one that begins with the nth "\("
	from the beginning of the pattern and ends with the
	corresponding paired "\)" ). The expression is invalid if less
	than n subexpressions precede the '\n' . For example, the
	expression "\(.*\)\1$" matches a line consisting of two
	adjacent appearances of the same string, and the expression
	"\(a\)*\1" fails to match 'a' . When the referenced
	subexpression matched more than one string, the back-referenced
	expression shall refer to the last matched string. If the
	subexpression referenced by the back-reference matches more
	than one string because of an asterisk ( '*' ) or an interval
	expression (see item (5)), the back-reference shall match the
	last (rightmost) of these strings.

	When a BRE matching a single character, a subexpression, or a
	back-reference is followed by the special character asterisk (
	'*' ), together with that asterisk it shall match what zero or
	more consecutive occurrences of the BRE would match.

	When a BRE matching a single character, a subexpression, or a
	back-reference is followed by an interval expression of the
	format "\{m\}" , "\{m,\}" , or "\{m,n\}" , together with that
	interval expression it shall match what repeated consecutive
	occurrences of the BRE would match.  "\{m,n\}" , together with
	that interval expression it shall match what repeated
	consecutive occurrences of the BRE would match.
	
		BRE Precedence (from high to low)
		Subexpressions/back-references \(\) \n
	
	The implementation may treat the circumflex as an anchor when
	used as the first character of a subexpression.  as the first
	character of a subexpression.  The circumflex shall anchor the
	expression (or optionally subexpression) to the beginning of a
	string; only sequences starting at the first character of a
	string shall be matched by the BRE. For example, the BRE "^ab"
	matches "ab" in the string "abcdef" , but fails to match in the
	string "cdefab" . The BRE "\(^ab\)" may match the former
	string. A portable BRE shall escape a leading circumflex in a
	subexpression to match a literal circumflex.

	A dollar sign ( '$' ) shall be an anchor when used as the last
	character of an entire BRE. The implementation may treat a
	dollar sign as an anchor when used as the last character of a
	subexpression.  the last character of a subexpression.  A
	circumflex ( '^' ) outside a bracket expression shall anchor
	the expression or subexpression it begins to the beginning of a
	string; such an expression or subexpression can match only a
	sequence starting at the first character of a string. For
	example, the EREs "^ab" and "(^ab)" match "ab" in the string
	"abcdef" , but fail to match in the string "cdefab" , and the
	ERE "a^b" is valid, but can never match because the 'a'
	prevents the expression "^b" from matching starting at the
	first character.

	A dollar sign ( '$' ) outside a bracket expression shall anchor
	the expression or subexpression it ends to the end of a string;
	such an expression or subexpression can match only a sequence
	ending at the last character of a string. For example, the EREs
	"ef$" and "(ef$)" match "ef" in the string "abcdef" , but fail
	to match in the string "cdefab" , and the ERE "e$f" is valid,
	but can never match because the 'f' prevents the expression
	"e$" from matching ending at the last character.

http://www.opengroup.org/onlinepubs/007904975/xrat/xbd_chap09.html

	It is possible to determine what strings correspond to
	subexpressions by recursively applying the leftmost longest
	rule to each subexpression, but only with the proviso that the
	overall match is leftmost longest. For example, matching
	"\(ac*\)c*d[ac]*\1" against acdacaaa matches acdacaaa (with
	\1=a); simply matching the longest match for "\(ac*\)" would
	yield \1=ac, but the overall match would be smaller (acdac).
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

	The limit of nine back-references to subexpressions in the RE
	is based on the use of a single-digit identifier; increasing
	this to multiple digits would break historical applications.
	This does not imply that only nine subexpressions are allowed
	in REs. The following is a valid BRE with ten subexpressions:

	\(\(\(ab\)*c\)*d\)\(ef\)*\(gh\)\{2\}\(ij\)*\(kl\)*\(mn\)*\(op\)*\(qr\)*

	The standard developers regarded the common historical
	behavior, which supported "\n*" , but not "\n\{min,max\}" ,
	"\(...\)*" , or "\(...\)\{min,max\}" , as a non-intentional
	result of a specific implementation, and they supported both
	duplication and interval expressions following subexpressions
	and back-references.

	However, one relatively uncommon case was changed to allow an
	extension used on some implementations. Historically, the BREs
	"^foo" and "\(^foo\)" did not match the same string, despite
	the general rule that subexpressions and entire BREs match the
	same strings. To increase consensus, IEEE Std 1003.1-2001 has
	allowed an extension on some implementations to treat these two
	cases in the same way by declaring that anchoring may occur at
	the beginning or end of a subexpression.  Therefore, portable
	BREs that require a literal circumflex at the beginning or a
	dollar sign at the end of a subexpression must escape them.
	Note that a BRE such as "a\(^bc\)" will either match "a^bc" or
	nothing on different systems under the rules.

	Some implementations have extended the BRE syntax to add
	alternation. For example, the subexpression "\(foo$\|bar\)"
	would match either "foo" at the end of the string or "bar"
	anywhere. The extension is triggered by the use of the
	undefined "\|" sequence. Because the BRE is undefined for
	portable scripts, the extending system is free to make other
	assumptions, such that the '$' represents the end-of-line
	anchor in the middle of a subexpression. If it were not for the
	extension, the '$' would match a literal dollar sign under the
	rules.

	The removal of the Back_open_paren Back_close_paren option from
	the nondupl_RE specification is the result of PASC
	Interpretation 1003.2-92 #43 submitted for the ISO POSIX-2:1993
	standard. Although the grammar required support for null
	subexpressions, this section does not describe the meaning of,
	and historical practice did not support, this construct.

http://www.opengroup.org/onlinepubs/007904975/functions/regcomp.html

	size_t re_nsub Number of parenthesized subexpressions

	If the REG_NOSUB flag was not set in cflags, then regcomp()
	shall set re_nsub to the number of parenthesized subexpressions
	(delimited by "\(\)" in basic regular expressions or "()" in
	extended regular expressions) found in pattern.

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

	When matching a basic or extended regular expression, any given
	parenthesized subexpression of pattern might participate in the
	match of several different substrings of string, or it might
	not match any substring even though the pattern as a whole did
	match. The following rules shall be used to determine which
	substrings to report in pmatch when matching regular
	expressions:

	1.If subexpression i in a regular expression is not contained
	within another subexpression, and it participated in the match
	several times, then the byte offsets in pmatch[i] shall
	delimit the last such match.

	2.If subexpression i is not contained within another
	subexpression, and it did not participate in an otherwise
	successful match, the byte offsets in pmatch[i] shall be -1. A
	subexpression does not participate in the match when:

		'*' or "\{\}" appears immediately after the
		subexpression in a basic regular expression, or '*' ,
		'?' , or "{}" appears immediately after the
		subexpression in an extended regular expression, and
		the subexpression did not match (matched 0 times)

		or:

		'|' is used in an extended regular expression to select
		this subexpression or another, and the other
		subexpression matched.

	3.If subexpression i is contained within another subexpression
	j, and i is not contained within any other subexpression that
	is contained within j, and a match of subexpression j is
	reported in pmatch[j], then the match or non-match of
	subexpression i reported in pmatch[i] shall be as described in
	1.  and 2. above, but within the substring reported in pmatch[
	j] rather than the whole string. The offsets in pmatch[i] are
	still relative to the start of string.

	4.If subexpression i is contained in subexpression j, and the
	byte offsets in pmatch[j] are -1, then the pointers in pmatch[
	i] shall also be -1.

	5.If subexpression i matched a zero-length string, then both
	byte offsets in pmatch[i] shall be the byte offset of the
	character or null terminator immediately following the
	zero-length string.

	RATIONALE

	The regexec() function must fill in all nmatch elements of
	pmatch, where nmatch and pmatch are supplied by the
	application, even if some elements of pmatch do not correspond
	to subexpressions in pattern. The application writer should
	note that there is probably no reason for using a value of
	nmatch that is larger than preg-> re_nsub+1.

	The number of subexpressions in the RE is reported in re_nsub
	in preg. With this change to regexec(), consideration was given
	to dropping the REG_NOSUB flag since the user can now specify
	this with a zero nmatch argument to regexec(). However, keeping
	REG_NOSUB allows an implementation to use a different (perhaps
	more efficient) algorithm if it knows in regcomp() that no
	subexpressions need be reported. The implementation is only
	required to fill in pmatch if nmatch is not zero and if
	REG_NOSUB is not specified.  subexpressions need be reported.
	The implementation is only required to fill in pmatch if nmatch
	is not zero and if REG_NOSUB is not specified.
