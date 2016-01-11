# regression tests for the fmt utilitiy

function DATA
{
	for f
	do	if	[[ ! -f $f ]]
		then	case $f in
			big.in)	for ((i = 0; i < 1024; i++))
				do	print -n "abcdefghijklmnop "
				done
				print
				;;
			big.out)for ((i = 0; i < 256; i++))
				do	print "abcdefghijklmnop abcdefghijklmnop abcdefghijklmnop abcdefghijklmnop"
				done
				;;
			esac > $f
		fi
	done
}

TEST 01 'basics'

	EXEC
		INPUT - $'aaa\n\nzzz'
		OUTPUT - $'aaa\n\nzzz'
	EXEC
		INPUT - $'aaa \n\nzzz'
	EXEC
		INPUT - $'aaa\n\nzzz '
	EXEC
		INPUT - $'aaa\n\n zzz'
		OUTPUT - $'aaa\n\n zzz'
	EXEC
		INPUT -n - $'aaa'
		OUTPUT - $'aaa'
	EXEC
		INPUT -n - $'aaa\n\nzzz'
		OUTPUT - $'aaa\n\nzzz'

TEST 02 'line buffer stress'

	DO	DATA big.in big.out
	EXEC	big.in
		SAME OUTPUT big.out

TEST 03 '--optget'

	EXEC	--optget
		INPUT - $'"[+NAME?builtin - add, delete, or display shell built-ins]"
"[+DESCRIPTION?\\bbuiltin\\b can be used to add, delete, or display "
	"built-in commands in the current shell environment.  A "
	"built-in command executes in the current shell process "
	"and can have side effects in the current shell.  On most "
	"systems, the invocation time for built-in commands is one "
	"or two orders of magnitude less than commands that create "
	"a separate process.]" 
"[+?For each \\apathname\\a specified, the basename of the pathname "
	"determines the name of the built-in.  For each basename, "
	"the shell looks for a C level function in the current shell "
	"whose name is determined by prepending \\bb_\\b to the built-in "
	"name.  If \\apathname\\a contains a \\b/\\b, then the built-in is "
	"bound to this pathname.  A built-in bound to a pathname will "
	"only be executed if \\apathname\\a is the first executable "
	"found during a path search.  Otherwise, built-ins are found "
	"prior to performing the path search.]"
"[+?If no \\apathname\\a operands are specified, then \\bbuiltin\\b displays "
	"the current list of built-ins, or just the special built-ins if "
	"\\b-s\\b is specified, on standard output.  The full pathname for "
	"built-ins that are bound to pathnames are displayed.]"
"[+?Libraries containing built-ins can be specified with the \\b-f\\b "
	"option.  If the library contains a function named \\blib_init\\b(), "
	"this function will be invoked with argument \\b0\\b when the "
	"library is loaded.  The \\blib_init\\b() function can load "
	"built-ins by invoking an appropriate C level function.  In "
	"this case there is no restriction on the C level function name.]"
"[+?The C level function will be invoked with three arguments.  The first "
	"two are the same as \\bmain\\b() and the third one is a pointer.]"
"[+?\\bbuiltin\\b cannot be invoked from a restricted shell.]"
"[d?Deletes each of the specified built-ins. Special built-ins cannot "
	"be deleted.]"
"[f]:[lib?On systems with dynamic linking, \\alib\\a names a shared library "
	"to load and search for built-ins.  The shared library prefix and/or "
	"suffix, which depend on the system, can be omitted. Once a library "
	"is loaded, its symbols become available for the current and "
	"subsequent invocations of \\bbuiltin\\b.  Multiple libraries can be "
	"specified with separate invocations of \\bbuiltin\\b.  Libraries are "
	"searched in the reverse order in which they are specified.]"
"[s?Display only the special built-ins.]"'
		OUTPUT - $'"[+NAME?builtin - add, delete, or display shell built-ins]"
"[+DESCRIPTION?\\bbuiltin\\b can be used to add, delete, or display "
    "built-in commands in the current shell environment. A built-in command "
    "executes in the current shell process and can have side effects in the "
    "current shell. On most systems, the invocation time for built-in "
    "commands is one or two orders of magnitude less than commands that "
    "create a separate process.]"
"[+?For each \\apathname\\a specified, the basename of the pathname "
    "determines the name of the built-in. For each basename, the shell looks "
    "for a C level function in the current shell whose name is determined by "
    "prepending \\bb_\\b to the built-in name. If \\apathname\\a contains a "
    "\\b/\\b, then the built-in is bound to this pathname. A built-in bound to "
    "a pathname will only be executed if \\apathname\\a is the first "
    "executable found during a path search. Otherwise, built-ins are found "
    "prior to performing the path search.]"
"[+?If no \\apathname\\a operands are specified, then \\bbuiltin\\b displays "
    "the current list of built-ins, or just the special built-ins if \\b-s\\b "
    "is specified, on standard output. The full pathname for built-ins that "
    "are bound to pathnames are displayed.]"
"[+?Libraries containing built-ins can be specified with the \\b-f\\b "
    "option. If the library contains a function named \\blib_init\\b(), this "
    "function will be invoked with argument \\b0\\b when the library is "
    "loaded. The \\blib_init\\b() function can load built-ins by invoking an "
    "appropriate C level function. In this case there is no restriction on "
    "the C level function name.]"
"[+?The C level function will be invoked with three arguments. The first "
    "two are the same as \\bmain\\b() and the third one is a pointer.]"
"[+?\\bbuiltin\\b cannot be invoked from a restricted shell.]"
"[d?Deletes each of the specified built-ins. Special built-ins cannot be "
    "deleted.]"
"[f]:[lib?On systems with dynamic linking, \\alib\\a names a shared "
    "library to load and search for built-ins. The shared library prefix "
    "and/or suffix, which depend on the system, can be omitted. Once a "
    "library is loaded, its symbols become available for the current and "
    "subsequent invocations of \\bbuiltin\\b. Multiple libraries can be "
    "specified with separate invocations of \\bbuiltin\\b. Libraries are "
    "searched in the reverse order in which they are specified.]"
"[s?Display only the special built-ins.]"'

	EXEC	--optget
		INPUT - $'"[+NAME?date - set/list/convert dates]"
"[+DESCRIPTION?\\bdate\\b sets the current date and time (with appropriate"
"	privilege), lists the current date or file dates, or converts"
"	dates.]"
"[+?Most common \\adate\\a forms are recognized, including those for"
"	\\bcrontab\\b(1), \\bls\\b(1), \\btouch\\b(1), and the default"
"	output from \\bdate\\b itself.]"
"[+?If the \\adate\\a operand consists of 4, 6, 8, 10 or 12 digits followed"
"	by an optional \\b.\\b and two digits then it is interpreted as:"
"	\\aHHMM.SS\\a, \\addHHMM.SS\\a, \\ammddHHMM.SS\\a, \\ammddHHMMyy.SS\\a or"
"	\\ayymmddHHMM.SS\\a, or \\ammddHHMMccyy.SS\\a or \\accyymmddHHMM.SS\\a."
"	Conflicting standards and practice allow a leading or trailing"
"	2 or 4 digit year for the 10 and 12 digit forms; the X/Open trailing"
"	form is used to disambiguate (\\btouch\\b(1) uses the leading form.)"
"	Avoid the 10 digit form to avoid confusion. The digit fields are:]{"
"		[+cc?Century - 1, 19-20.]"
"		[+yy?Year in century, 00-99.]"
"		[+mm?Month, 01-12.]"
"		[+dd?Day of month, 01-31.]"
"		[+HH?Hour, 00-23.]"
"		[+MM?Minute, 00-59.]"
"		[+SS?Seconds, 00-60.]"
"}"
"[+?If more than one \\adate\\a operand is specified then:]{"
"		[+1.?Each operand sets the reference date for the next"
"			operand.]"
"		[+2.?The date is listed for each operand.]"
"		[+3.?The system date is not set.]"
"}"

"[a:access-time|atime?List file argument access times.]"
"[c:change-time|ctime?List file argument change times.]"
"[d:date?Use \\adate\\a as the current date and do not set the system"
"	clock.]:[date]"
"[e:epoch?Output the date in seconds since the epoch."
"	Equivalent to \\b--format=%#\\b.]"
"[E:elapsed?Interpret pairs of arguments as start and stop dates, sum the"
"	differences between all pairs, and list the result as a"
"	\\bfmtelapsed\\b(3) elapsed time on the standard output. If there are"
"	an odd number of arguments then the last time argument is differenced"
"	with the current time.]"
"[f:format?Output the date according to the \\bstrftime\\b(3) \\aformat\\a."
"	For backwards compatibility, a first argument of the form"
"	\\b+\\b\\aformat\\a is equivalent to \\b-f\\b format."
"	\\aformat\\a is in \\bprintf\\b(3) style, where %\\afield\\a names"
"	fixed size field, zero padded if necessary,"
"	and \\\\\\ac\\a and \\\\\\annn\\a sequences are as in C. Invalid"
"	%\\afield\\a specifications and all other characters are copied"
"	without change. \\afield\\a may be preceded by \\b%-\\b to turn off"
"	padding or \\b%_\\b to pad with space, otherwise numeric fields"
"	are padded with \\b0\\b and string fields are padded with space."
"	\\afield\\a may also be preceded by \\bE\\b for alternate era"
"	representation or \\bO\\b for alternate digit representation (if"
"	supported by the current locale.) The fields are:]:[format]{"
"		[+%?% character]"
"		[+a?abbreviated weekday name]"
"		[+A?full weekday name]"
"		[+b?abbreviated month name]"
"		[+c?\\bctime\\b(3) style date without the trailing newline]"
"		[+C?2-digit century]"
"		[+d?day of month number]"
"		[+D?date as \\amm/dd/yy\\a]"
"		[+e?blank padded day of month number]"
"		[+E?unpadded day of month number]"
"		[+f?locale default override date format]"
"		[+F?locale default date format]"
"		[+g?\\bls\\b(1) \\b-l\\b recent date with \\ahh:mm\\a]"
"		[+G?\\bls\\b(1) \\b-l\\b distant date with \\ayyyy\\a]"
"		[+h?abbreviated month name]"
"		[+H?24-hour clock hour]"
"		[+i?international \\bdate\\b(1) date with time zone type name]"
"		[+I?12-hour clock hour]"
"		[+j?1-offset Julian date]"
"		[+J?0-offset Julian date]"
"		[+k?\\bdate\\b(1) style date]"
"		[+K?all numeric date; equivalent to \\b%Y-%m-%d+%H:%M:%S\\b]"
"		[+l?\\bls\\b(1) \\b-l\\b date; equivalent to \\b%Q/%g/%G/\\b]"
"		[+m?month number]"
"		[+M?minutes]"
"		[+n?newline character]"
"		[+N?time zone type name]"
"		[+p?meridian (e.g., \\bAM\\b or \\bPM\\b)]"
"		[+Q?\\a<del>recent<del>distant<del>\\a: \\a<del>\\a is a unique"
"			delimter character; \\arecent\\a format for recent"
"			dates, \\adistant\\a format otherwise]"
"		[+r?12-hour time as \\ahh:mm:ss meridian\\a]"
"		[+R?24-hour time as \\ahh:mm\\a]"
"		[+s?number of seconds since the epoch]"
"		[+S?seconds]"
"		[+t?tab character]"
"		[+T?24-hour time as \\ahh:mm:ss\\a]"
"		[+u?weekday number 1(Monday)-7]"
"		[+U?week number with Sunday as the first day]"
"		[+V?ISO week number (i18n is \\afun\\a)]"
"		[+w?weekday number 0(Sunday)-6]"
"		[+W?week number with Monday as the first day]"
"		[+x?locale date style that includes month, day and year]"
"		[+X?locale time style that includes hours and minutes]"
"		[+y?2-digit year (you\'ll be sorry)]"
"		[+Y?4-digit year]"
"		[+z?time zone \\aSHHMM\\a west of GMT offset where S is"
"			\\b+\\b or \\b-\\b]"
"		[+Z?time zone name]"
"		[++|!flag?set (+) or clear (!) \\aflag\\a for the remainder"
"			of \\aformat\\a. \\aflag\\a may be:]{"
"			[+l?enable leap second adjustments]"
"			[+u?UTC time zone]"
"		}"
"		[+#?number of seconds since the epoch]"
"		[+??alternate?use \\aalternate\\a format if a default format"
"			override has not been specified, e.g., \\bls\\b(1) uses"
"			\\"%?%l\\"; export TM_OPTIONS=\\"format=\'\\aoverride\\a\'\\""
"			to override the default]"
"}"
"[i:incremental|adjust?Set the system time in incrementatl adjustments to"
"	avoid complete time shift shock. Negative adjustments still maintain"
"	monotonic increasing time. Not available on all systems.]"
"[l:leap-seconds?Include leap seconds in time calculations. Leap seconds"
"	after the ast library release date are not accounted for.]"
"[m:modify-time|mtime?List file argument modify times.]"
"[n!:network?Set network time.]"
"[p:parse?Add \\aformat\\a to the list of \\bstrptime\\b(3) parse conversion"
"	formats. \\aformat\\a follows the same conventions as the"
"	\\b--format\\b option, with the addition of these format"
"	fields:]:[format]{"
"		[+|?If the format failed before this point then restart"
"			the parse with the remaining format.]"
"		[+&?Call the \\btmdate\\b(3) heuristic parser. This is"
"			is the default when \\b--parse\\b is omitted.]"
"}"
"[s:show?Show the date without setting the system time.]"
"[u:utc|gmt|zulu?Output dates in \\acoordinated universal time\\a (UTC).]"'
		OUTPUT - $'"[+NAME?date - set/list/convert dates]"
"[+DESCRIPTION?\\bdate\\b sets the current date and time (with appropriate "
    "privilege), lists the current date or file dates, or converts dates.]"
"[+?Most common \\adate\\a forms are recognized, including those for "
    "\\bcrontab\\b(1), \\bls\\b(1), \\btouch\\b(1), and the default output from "
    "\\bdate\\b itself.]"
"[+?If the \\adate\\a operand consists of 4, 6, 8, 10 or 12 digits "
    "followed by an optional \\b.\\b and two digits then it is interpreted as: "
    "\\aHHMM.SS\\a, \\addHHMM.SS\\a, \\ammddHHMM.SS\\a, \\ammddHHMMyy.SS\\a or "
    "\\ayymmddHHMM.SS\\a, or \\ammddHHMMccyy.SS\\a or \\accyymmddHHMM.SS\\a. "
    "Conflicting standards and practice allow a leading or trailing 2 or 4 "
    "digit year for the 10 and 12 digit forms; the X/Open trailing form is "
    "used to disambiguate (\\btouch\\b(1) uses the leading form.) Avoid the 10 "
    "digit form to avoid confusion. The digit fields are:]"
    "{"
        "[+cc?Century - 1, 19-20.]"
        "[+yy?Year in century, 00-99.]"
        "[+mm?Month, 01-12.]"
        "[+dd?Day of month, 01-31.]"
        "[+HH?Hour, 00-23.]"
        "[+MM?Minute, 00-59.]"
        "[+SS?Seconds, 00-60.]"
    "}"
"[+?If more than one \\adate\\a operand is specified then:]"
    "{"
        "[+1.?Each operand sets the reference date for the next "
            "operand.]"
        "[+2.?The date is listed for each operand.]"
        "[+3.?The system date is not set.]"
    "}"
"[a:access-time|atime?List file argument access times.]"
"[c:change-time|ctime?List file argument change times.]"
"[d:date?Use \\adate\\a as the current date and do not set the system "
    "clock.]:[date]"
"[e:epoch?Output the date in seconds since the epoch. Equivalent to "
    "\\b--format=%#\\b.]"
"[E:elapsed?Interpret pairs of arguments as start and stop dates, sum "
    "the differences between all pairs, and list the result as a "
    "\\bfmtelapsed\\b(3) elapsed time on the standard output. If there are an "
    "odd number of arguments then the last time argument is differenced with "
    "the current time.]"
"[f:format?Output the date according to the \\bstrftime\\b(3) \\aformat\\a. "
    "For backwards compatibility, a first argument of the form "
    "\\b+\\b\\aformat\\a is equivalent to \\b-f\\b format. \\aformat\\a is in "
    "\\bprintf\\b(3) style, where %\\afield\\a names fixed size field, zero "
    "padded if necessary, and \\\\\\ac\\a and \\\\\\annn\\a sequences are as in C. "
    "Invalid %\\afield\\a specifications and all other characters are copied "
    "without change. \\afield\\a may be preceded by \\b%-\\b to turn off padding "
    "or \\b%_\\b to pad with space, otherwise numeric fields are padded with "
    "\\b0\\b and string fields are padded with space. \\afield\\a may also be "
    "preceded by \\bE\\b for alternate era representation or \\bO\\b for "
    "alternate digit representation (if supported by the current locale.) "
    "The fields are:]:[format]"
    "{"
        "[+%?% character]"
        "[+a?abbreviated weekday name]"
        "[+A?full weekday name]"
        "[+b?abbreviated month name]"
        "[+c?\\bctime\\b(3) style date without the trailing newline]"
        "[+C?2-digit century]"
        "[+d?day of month number]"
        "[+D?date as \\amm/dd/yy\\a]"
        "[+e?blank padded day of month number]"
        "[+E?unpadded day of month number]"
        "[+f?locale default override date format]"
        "[+F?locale default date format]"
        "[+g?\\bls\\b(1) \\b-l\\b recent date with \\ahh:mm\\a]"
        "[+G?\\bls\\b(1) \\b-l\\b distant date with \\ayyyy\\a]"
        "[+h?abbreviated month name]"
        "[+H?24-hour clock hour]"
        "[+i?international \\bdate\\b(1) date with time zone type name]"
        "[+I?12-hour clock hour]"
        "[+j?1-offset Julian date]"
        "[+J?0-offset Julian date]"
        "[+k?\\bdate\\b(1) style date]"
        "[+K?all numeric date; equivalent to \\b%Y-%m-%d+%H:%M:%S\\b]"
        "[+l?\\bls\\b(1) \\b-l\\b date; equivalent to \\b%Q/%g/%G/\\b]"
        "[+m?month number]"
        "[+M?minutes]"
        "[+n?newline character]"
        "[+N?time zone type name]"
        "[+p?meridian (e.g., \\bAM\\b or \\bPM\\b)]"
        "[+Q?\\a<del>recent<del>distant<del>\\a: \\a<del>\\a is a unique "
            "delimter character; \\arecent\\a format for recent dates, "
            "\\adistant\\a format otherwise]"
        "[+r?12-hour time as \\ahh:mm:ss meridian\\a]"
        "[+R?24-hour time as \\ahh:mm\\a]"
        "[+s?number of seconds since the epoch]"
        "[+S?seconds]"
        "[+t?tab character]"
        "[+T?24-hour time as \\ahh:mm:ss\\a]"
        "[+u?weekday number 1(Monday)-7]"
        "[+U?week number with Sunday as the first day]"
        "[+V?ISO week number (i18n is \\afun\\a)]"
        "[+w?weekday number 0(Sunday)-6]"
        "[+W?week number with Monday as the first day]"
        "[+x?locale date style that includes month, day and year]"
        "[+X?locale time style that includes hours and minutes]"
        "[+y?2-digit year (you\'ll be sorry)]"
        "[+Y?4-digit year]"
        "[+z?time zone \\aSHHMM\\a west of GMT offset where S is \\b+\\b or "
            "\\b-\\b]"
        "[+Z?time zone name]"
        "[++|!flag?set (+) or clear (!) \\aflag\\a for the remainder of "
            "\\aformat\\a. \\aflag\\a may be:]"
            "{"
                "[+l?enable leap second adjustments]"
                "[+u?UTC time zone]"
            "}"
        "[+#?number of seconds since the epoch]"
        "[+??alternate?use \\aalternate\\a format if a default format "
            "override has not been specified, e.g., \\bls\\b(1) uses \\"%?%l\\"; "
            "export TM_OPTIONS=\\"format=\'\\aoverride\\a\'\\" to override the "
            "default]"
    "}"
"[i:incremental|adjust?Set the system time in incrementatl adjustments "
    "to avoid complete time shift shock. Negative adjustments still maintain "
    "monotonic increasing time. Not available on all systems.]"
"[l:leap-seconds?Include leap seconds in time calculations. Leap seconds "
    "after the ast library release date are not accounted for.]"
"[m:modify-time|mtime?List file argument modify times.]"
"[n!:network?Set network time.]"
"[p:parse?Add \\aformat\\a to the list of \\bstrptime\\b(3) parse conversion "
    "formats. \\aformat\\a follows the same conventions as the \\b--format\\b "
    "option, with the addition of these format fields:]:[format]"
    "{"
        "[+|?If the format failed before this point then restart the "
            "parse with the remaining format.]"
        "[+&?Call the \\btmdate\\b(3) heuristic parser. This is is the "
            "default when \\b--parse\\b is omitted.]"
    "}"
"[s:show?Show the date without setting the system time.]"
"[u:utc|gmt|zulu?Output dates in \\acoordinated universal time\\a (UTC).]"'

	EXEC	--optget
		INPUT - $'[+NAME?date - set/list/convert dates]
[+DESCRIPTION?\\bdate\\b sets the current date and time (with appropriate
	privilege), lists the current date or file dates, or converts
	dates.]
[+?Most common \\adate\\a forms are recognized, including those for
	\\bcrontab\\b(1), \\bls\\b(1), \\btouch\\b(1), and the default
	output from \\bdate\\b itself.]
[+?If the \\adate\\a operand consists of 4, 6, 8, 10 or 12 digits followed
	by an optional \\b.\\b and two digits then it is interpreted as:
	\\aHHMM.SS\\a, \\addHHMM.SS\\a, \\ammddHHMM.SS\\a, \\ammddHHMMyy.SS\\a or
	\\ayymmddHHMM.SS\\a, or \\ammddHHMMccyy.SS\\a or \\accyymmddHHMM.SS\\a.
	Conflicting standards and practice allow a leading or trailing
	2 or 4 digit year for the 10 and 12 digit forms; the X/Open trailing
	form is used to disambiguate (\\btouch\\b(1) uses the leading form.)
	Avoid the 10 digit form to avoid confusion. The digit fields are:]{
		[+cc?Century - 1, 19-20.]
		[+yy?Year in century, 00-99.]
		[+mm?Month, 01-12.]
		[+dd?Day of month, 01-31.]
		[+HH?Hour, 00-23.]
		[+MM?Minute, 00-59.]
		[+SS?Seconds, 00-60.]
}
[+?If more than one \\adate\\a operand is specified then:]{
		[+1.?Each operand sets the reference date for the next
			operand.]
		[+2.?The date is listed for each operand.]
		[+3.?The system date is not set.]
}

[a:access-time|atime?List file argument access times.]
[c:change-time|ctime?List file argument change times.]
[d:date?Use \\adate\\a as the current date and do not set the system
	clock.]:[date]
[e:epoch?Output the date in seconds since the epoch.
	Equivalent to \\b--format=%#\\b.]
[E:elapsed?Interpret pairs of arguments as start and stop dates, sum the
	differences between all pairs, and list the result as a
	\\bfmtelapsed\\b(3) elapsed time on the standard output. If there are
	an odd number of arguments then the last time argument is differenced
	with the current time.]
[f:format?Output the date according to the \\bstrftime\\b(3) \\aformat\\a.
	For backwards compatibility, a first argument of the form
	\\b+\\b\\aformat\\a is equivalent to \\b-f\\b format.
	\\aformat\\a is in \\bprintf\\b(3) style, where %\\afield\\a names
	fixed size field, zero padded if necessary,
	and \\\\\\ac\\a and \\\\\\annn\\a sequences are as in C. Invalid
	%\\afield\\a specifications and all other characters are copied
	without change. \\afield\\a may be preceded by \\b%-\\b to turn off
	padding or \\b%_\\b to pad with space, otherwise numeric fields
	are padded with \\b0\\b and string fields are padded with space.
	\\afield\\a may also be preceded by \\bE\\b for alternate era
	representation or \\bO\\b for alternate digit representation (if
	supported by the current locale.) The fields are:]:[format]{
		[+%?% character]
		[+a?abbreviated weekday name]
		[+A?full weekday name]
		[+b?abbreviated month name]
		[+c?\\bctime\\b(3) style date without the trailing newline]
		[+C?2-digit century]
		[+d?day of month number]
		[+D?date as \\amm/dd/yy\\a]
		[+e?blank padded day of month number]
		[+E?unpadded day of month number]
		[+f?locale default override date format]
		[+F?locale default date format]
		[+g?\\bls\\b(1) \\b-l\\b recent date with \\ahh:mm\\a]
		[+G?\\bls\\b(1) \\b-l\\b distant date with \\ayyyy\\a]
		[+h?abbreviated month name]
		[+H?24-hour clock hour]
		[+i?international \\bdate\\b(1) date with time zone type name]
		[+I?12-hour clock hour]
		[+j?1-offset Julian date]
		[+J?0-offset Julian date]
		[+k?\\bdate\\b(1) style date]
		[+K?all numeric date; equivalent to \\b%Y-%m-%d+%H:%M:%S\\b]
		[+l?\\bls\\b(1) \\b-l\\b date; equivalent to \\b%Q/%g/%G/\\b]
		[+m?month number]
		[+M?minutes]
		[+n?newline character]
		[+N?time zone type name]
		[+p?meridian (e.g., \\bAM\\b or \\bPM\\b)]
		[+Q?\\a<del>recent<del>distant<del>\\a: \\a<del>\\a is a unique
			delimter character; \\arecent\\a format for recent
			dates, \\adistant\\a format otherwise]
		[+r?12-hour time as \\ahh:mm:ss meridian\\a]
		[+R?24-hour time as \\ahh:mm\\a]
		[+s?number of seconds since the epoch]
		[+S?seconds]
		[+t?tab character]
		[+T?24-hour time as \\ahh:mm:ss\\a]
		[+u?weekday number 1(Monday)-7]
		[+U?week number with Sunday as the first day]
		[+V?ISO week number (i18n is \\afun\\a)]
		[+w?weekday number 0(Sunday)-6]
		[+W?week number with Monday as the first day]
		[+x?locale date style that includes month, day and year]
		[+X?locale time style that includes hours and minutes]
		[+y?2-digit year (you\'ll be sorry)]
		[+Y?4-digit year]
		[+z?time zone \\aSHHMM\\a west of GMT offset where S is
			\\b+\\b or \\b-\\b]
		[+Z?time zone name]
		[++|!flag?set (+) or clear (!) \\aflag\\a for the remainder
			of \\aformat\\a. \\aflag\\a may be:]{
			[+l?enable leap second adjustments]
			[+u?UTC time zone]
		}
		[+#?number of seconds since the epoch]
		[+??alternate?use \\aalternate\\a format if a default format
			override has not been specified, e.g., \\bls\\b(1) uses
			"%?%l"; export TM_OPTIONS="format=\'\\aoverride\\a\'"
			to override the default]
}
[i:incremental|adjust?Set the system time in incrementatl adjustments to
	avoid complete time shift shock. Negative adjustments still maintain
	monotonic increasing time. Not available on all systems.]
[l:leap-seconds?Include leap seconds in time calculations. Leap seconds
	after the ast library release date are not accounted for.]
[m:modify-time|mtime?List file argument modify times.]
[n!:network?Set network time.]
[p:parse?Add \\aformat\\a to the list of \\bstrptime\\b(3) parse conversion
	formats. \\aformat\\a follows the same conventions as the
	\\b--format\\b option, with the addition of these format
	fields:]:[format]{
		[+|?If the format failed before this point then restart
			the parse with the remaining format.]
		[+&?Call the \\btmdate\\b(3) heuristic parser. This is
			is the default when \\b--parse\\b is omitted.]
}
[s:show?Show the date without setting the system time.]
[u:utc|gmt|zulu?Output dates in \\acoordinated universal time\\a (UTC).]'
		OUTPUT - $'[+NAME?date - set/list/convert dates]
[+DESCRIPTION?\\bdate\\b sets the current date and time (with appropriate
    privilege), lists the current date or file dates, or converts dates.]
[+?Most common \\adate\\a forms are recognized, including those for
    \\bcrontab\\b(1), \\bls\\b(1), \\btouch\\b(1), and the default output from
    \\bdate\\b itself.]
[+?If the \\adate\\a operand consists of 4, 6, 8, 10 or 12 digits
    followed by an optional \\b.\\b and two digits then it is interpreted as:
    \\aHHMM.SS\\a, \\addHHMM.SS\\a, \\ammddHHMM.SS\\a, \\ammddHHMMyy.SS\\a or
    \\ayymmddHHMM.SS\\a, or \\ammddHHMMccyy.SS\\a or \\accyymmddHHMM.SS\\a.
    Conflicting standards and practice allow a leading or trailing 2 or 4
    digit year for the 10 and 12 digit forms; the X/Open trailing form is
    used to disambiguate (\\btouch\\b(1) uses the leading form.) Avoid the 10
    digit form to avoid confusion. The digit fields are:]
    {
        [+cc?Century - 1, 19-20.]
        [+yy?Year in century, 00-99.]
        [+mm?Month, 01-12.]
        [+dd?Day of month, 01-31.]
        [+HH?Hour, 00-23.]
        [+MM?Minute, 00-59.]
        [+SS?Seconds, 00-60.]
    }
[+?If more than one \\adate\\a operand is specified then:]
    {
        [+1.?Each operand sets the reference date for the next
            operand.]
        [+2.?The date is listed for each operand.]
        [+3.?The system date is not set.]
    }
[a:access-time|atime?List file argument access times.]
[c:change-time|ctime?List file argument change times.]
[d:date?Use \\adate\\a as the current date and do not set the system
    clock.]:[date]
[e:epoch?Output the date in seconds since the epoch. Equivalent to
    \\b--format=%#\\b.]
[E:elapsed?Interpret pairs of arguments as start and stop dates, sum
    the differences between all pairs, and list the result as a
    \\bfmtelapsed\\b(3) elapsed time on the standard output. If there are an
    odd number of arguments then the last time argument is differenced with
    the current time.]
[f:format?Output the date according to the \\bstrftime\\b(3) \\aformat\\a.
    For backwards compatibility, a first argument of the form
    \\b+\\b\\aformat\\a is equivalent to \\b-f\\b format. \\aformat\\a is in
    \\bprintf\\b(3) style, where %\\afield\\a names fixed size field, zero
    padded if necessary, and \\\\\\ac\\a and \\\\\\annn\\a sequences are as in C.
    Invalid %\\afield\\a specifications and all other characters are copied
    without change. \\afield\\a may be preceded by \\b%-\\b to turn off padding
    or \\b%_\\b to pad with space, otherwise numeric fields are padded with
    \\b0\\b and string fields are padded with space. \\afield\\a may also be
    preceded by \\bE\\b for alternate era representation or \\bO\\b for
    alternate digit representation (if supported by the current locale.)
    The fields are:]:[format]
    {
        [+%?% character]
        [+a?abbreviated weekday name]
        [+A?full weekday name]
        [+b?abbreviated month name]
        [+c?\\bctime\\b(3) style date without the trailing newline]
        [+C?2-digit century]
        [+d?day of month number]
        [+D?date as \\amm/dd/yy\\a]
        [+e?blank padded day of month number]
        [+E?unpadded day of month number]
        [+f?locale default override date format]
        [+F?locale default date format]
        [+g?\\bls\\b(1) \\b-l\\b recent date with \\ahh:mm\\a]
        [+G?\\bls\\b(1) \\b-l\\b distant date with \\ayyyy\\a]
        [+h?abbreviated month name]
        [+H?24-hour clock hour]
        [+i?international \\bdate\\b(1) date with time zone type name]
        [+I?12-hour clock hour]
        [+j?1-offset Julian date]
        [+J?0-offset Julian date]
        [+k?\\bdate\\b(1) style date]
        [+K?all numeric date; equivalent to \\b%Y-%m-%d+%H:%M:%S\\b]
        [+l?\\bls\\b(1) \\b-l\\b date; equivalent to \\b%Q/%g/%G/\\b]
        [+m?month number]
        [+M?minutes]
        [+n?newline character]
        [+N?time zone type name]
        [+p?meridian (e.g., \\bAM\\b or \\bPM\\b)]
        [+Q?\\a<del>recent<del>distant<del>\\a: \\a<del>\\a is a unique
            delimter character; \\arecent\\a format for recent dates,
            \\adistant\\a format otherwise]
        [+r?12-hour time as \\ahh:mm:ss meridian\\a]
        [+R?24-hour time as \\ahh:mm\\a]
        [+s?number of seconds since the epoch]
        [+S?seconds]
        [+t?tab character]
        [+T?24-hour time as \\ahh:mm:ss\\a]
        [+u?weekday number 1(Monday)-7]
        [+U?week number with Sunday as the first day]
        [+V?ISO week number (i18n is \\afun\\a)]
        [+w?weekday number 0(Sunday)-6]
        [+W?week number with Monday as the first day]
        [+x?locale date style that includes month, day and year]
        [+X?locale time style that includes hours and minutes]
        [+y?2-digit year (you\'ll be sorry)]
        [+Y?4-digit year]
        [+z?time zone \\aSHHMM\\a west of GMT offset where S is \\b+\\b or
            \\b-\\b]
        [+Z?time zone name]
        [++|!flag?set (+) or clear (!) \\aflag\\a for the remainder of
            \\aformat\\a. \\aflag\\a may be:]
            {
                [+l?enable leap second adjustments]
                [+u?UTC time zone]
            }
        [+#?number of seconds since the epoch]
        [+??alternate?use \\aalternate\\a format if a default format
            override has not been specified, e.g., \\bls\\b(1) uses "%?%l";
            export TM_OPTIONS="format=\'\\aoverride\\a\'" to override the
            default]
    }
[i:incremental|adjust?Set the system time in incrementatl adjustments
    to avoid complete time shift shock. Negative adjustments still maintain
    monotonic increasing time. Not available on all systems.]
[l:leap-seconds?Include leap seconds in time calculations. Leap seconds
    after the ast library release date are not accounted for.]
[m:modify-time|mtime?List file argument modify times.]
[n!:network?Set network time.]
[p:parse?Add \\aformat\\a to the list of \\bstrptime\\b(3) parse conversion
    formats. \\aformat\\a follows the same conventions as the \\b--format\\b
    option, with the addition of these format fields:]:[format]
    {
        [+|?If the format failed before this point then restart the
            parse with the remaining format.]
        [+&?Call the \\btmdate\\b(3) heuristic parser. This is is the
            default when \\b--parse\\b is omitted.]
    }
[s:show?Show the date without setting the system time.]
[u:utc|gmt|zulu?Output dates in \\acoordinated universal time\\a (UTC).]'

	EXEC	--optget
		INPUT - $'[+DESCRIPTION?The \\bpackage\\b command controls source and binary
    packages. It is a \\bsh\\b(1) script coded for maximal portability. All
    package files are in the \\b$PACKAGEROOT\\b directory tree.
    \\b$PACKAGEROOT\\b must at minumum contain a \\bbin/package\\b command or a
    \\blib/package\\b directory. Binary package files are in the
    \\b$INSTALLROOT\\b (\\b$PACKAGEROOT/arch/\\b\\ahosttype\\a) tree, where
    \\ahosttpe\\a=`\\bpackage\\b`. All \\aactions\\a but \\bhost\\b and \\buse\\b
    require the current directory to be under \\b$PACKAGEROOT\\b. See
    \\bDETAILS\\b for more information.]
[+?Note that no environment variables need be set by the user;
    \\bpackage\\b determines the environment based on the current working
    directory. The \\buse\\b action starts a \\bsh\\b(1) with the environment
    initialized. \\bCC\\b, \\bCCFLAGS\\b, \\bHOSTTYPE\\b and \\bSHELL\\b may be set
    by explicit command argument assignments to override the defaults.]
[+?Packages are composed of components. Each component is built and
    installed by an \\bast\\b \\bnmake\\b(1) makefile. Each package is also
    described by an \\bnmake\\b makefile that lists its components and
    provides a content description. The package makefile and component
    makefiles provide all the information required to read, write, build
    and install packages.]
[+?Package recipients only need \\bsh\\b(1) and \\bcc\\b(1) to build and
    install source packages, and \\bsh\\b to install binary packages.
    \\bnmake\\b and \\bksh93\\b are required to write new packages. An
    \\b$INSTALLROOT/bin/cc\\b script may be supplied for some architectures.
    This script supplies a reasonable set of default options for compilers
    that accept multiple dialects or generate multiple object/executable
    formats.]
[+?The command arguments are composed of a sequence of words: zero or
    more \\aqualifiers\\a, one \\aaction\\a, and zero or more action-specific
    \\aarguments\\a, and zero or more \\aname=value\\a definitions. \\apackage\\a
    names a particular package. The naming scheme is a \\b-\\b separated
    hierarchy; the leftmost parts describe ownership, e.g.,
    \\bgnu-fileutils\\b, \\bast-base\\b. If no packages are specified then all
    packages are operated on. \\boptget\\b(3) documentation options are also
    supported. The default with no arguments is \\bhost type\\b.]
[+?The qualifiers are:]
    {
        [+debug|environment?Show environment and actions but do not
            execute.]
        [+flat?Collapse \\b$INSTALLROOT\\b { bin fun include lib } onto
            \\b$PACKAGEROOT\\b.]
        [+force?Force the action to override saved state.]
        [+never?Run make -N and show other actions.]
        [+only?Only operate on the specified packages.]
        [+quiet?Do not list captured action output.]
        [+show?Run make -n and show other actions.]
        [+verbose?Provide detailed action output.]
        [+DEBUG?Trace the package script actions in detail.]
    }
[+?The actions are:]
    {
        [+admin\\b [\\ball\\b]] [\\bdb\\b \\afile\\a]] [\\bon\\b \\apattern\\a]]
        [\\aaction\\a ...]]?Apply \\aaction\\a ... to the hosts listed in
            \\afile\\a. If \\afile\\a is omitted then \\badmin.db\\b is assumed.
            The caller must have \\brcp\\b(1) and \\brsh\\b(1) or \\bscp\\b(1)
            and \\bssh\\b(1) access to the hosts. Output for \\aaction\\a is
            saved per-host in the file \\aaction\\a\\b.log/\\b\\ahost\\a. Logs
            can be viewed by \\bpackage admin\\b [\\bon\\b \\ahost\\a]]
            \\bresults\\b [\\aaction\\a]]. By default only local PACKAGEROOT
            hosts are selected from \\afile\\a; \\ball\\b selects all hosts.
            \\bon\\b \\apattern\\a selects only hosts matching the \\b|\\b
            separated \\apattern\\a. \\afile\\a contains four types of lines.
            Blank lines and lines beginning with \\b#\\b are ignored. Lines
            starting with \\aid\\a=\\avalue\\a are variable assignments. Set
            admin_ping to local conventions if \\"\'$admin_ping$\'\\" fails. If
            a package list is not specified on the command line the
            \\aaction\\a applies to all packages; a variable assigment
            \\bpackage\\b=\\"\\alist\\a\\" applies \\aaction\\a to the packages in
            \\alist\\a for subsequent hosts in \\afile\\a. The remaining line
            type is a host description consisting of 6 tab separated
            fields. The first 3 are mandatory; the remaining 3 are updated
            by the \\badmin\\b action. \\afile\\a is saved in \\afile\\a\\b.old\\b
            before update. The fields are:]
            {
                [+hosttype?The host type as reported by
                    \\"\\bpackage\\b\\".]
                [+[user@]]host?The host name and optionally user name
                    for \\brcp\\b(1) and \\brsh\\b(1) access.]
                [+[remote::]]PACKAGEROOT?The absolute remote package
                    root directory and optionally the remote protocol (rsh
                    or ssh) if the directory is on a different server than
                    the master package root directory. If
                    \\blib/package/admin/\'$admin_env$\'\\b exists under this
                    directory then it is sourced by \\bsh\\b(1) before
                    \\aaction\\a is done. If this field begins with \\b-\\b
                    then the host is ignored. If this field contains \\b:\\b
                    then \\bditto\\b(1) is used to sync the remote \\bsrc\\b
                    directory hierarchy to the local one. These directories
                    must exist on the remote side: \\blib/package\\b,
                    \\bsrc/cmd\\b, \\bsrc/lib\\b.]
                [+date?\\aYYMMDD\\a of the last action.]
                [+time?Elapsed wall time for the last action.]
                [+M T W?The \\badmin\\b action \\bmake\\b, \\btest\\b and
                    \\bwrite\\b action error counts. A non-numeric value in
                    any of these fields disables the corresponding action.]
            }
        [+contents\\b [ \\apackage\\a ... ]]?List description and
            components for \\apackage\\a on the standard output.]
        [+copyright\\b [ \\apackage\\a ... ]]?List the general copyright
            notice(s) for \\apackage\\a on the standard output. Note that
            individual components in \\apackage\\a may contain additional or
            replacement notices.]
        [+export\\b \\avariable\\a ...?List \\aname\\a=\\avalue\\a for
            \\avariable\\a, one per line. If the \\bonly\\b attribute is
            specified then only the variable values are listed.]
        [+help\\b [ \\aaction\\a ]]?Display help text on the standard
            error (standard output for \\aaction\\a).]
        [+host\\b [ \\aattribute\\a ... ]]?List
            architecture/implementation dependent host information on the
            standard output. \\btype\\b is listed if no attributes are
            specified. Information is listed on a single line in
            \\aattribute\\a order. The attributes are:]
            {
                [+canon \\aname\\a?An external host type name to be
                    converted to \\bpackage\\b syntax.]
                [+cpu?The number of cpus; 1 if the host is not a
                    multiprocessor.]
                [+name?The host name.]
                [+rating?The cpu rating in pseudo mips; the value is
                    useful useful only in comparisons with rating values of
                    other hosts. Other than a vax rating (mercifully) fixed
                    at 1, ratings can vary wildly but consistently from
                    vendor mips ratings. \\bcc\\b(1) may be required to
                    determine the rating.]
                [+type?The host type, usually in the form
                    \\avendor\\a.\\aarchitecture\\a, with an optional trailing
                    -\\aversion\\a. The main theme is that type names within
                    a family of architectures are named in a similar,
                    predictable style. OS point release information is
                    avoided as much as possible, but vendor resistance to
                    release incompatibilities has for the most part been
                    futile.]
            }
        [+html\\b [ \\aaction\\a ]]?Display html help text on the standard
            error (standard output for \\aaction\\a).]
        [+install\\b [ flat ]] [ \\aarchitecture\\a ... ]] \\adirectory\\a [
            \\apackage\\a ... ]]?Copy the package binary hierarchy to
            \\adirectory\\a. If \\aarchitecture\\a is omitted then all
            architectures are installed. If \\bflat\\b is specified then
            exactly one \\aarchitecture\\a must be specified; this
            architecture will be installed in \\adirectory\\a without the
            \\barch/\\b\\aHOSTTYPE\\a directory prefixes. Otherwise each
            architecture will be installed in a separate
            \\barch/\\b\\aHOSTTYPE\\a subdirectory of \\adirectory\\a. The
            \\aarchitecture\\a \\b-\\b names the current architecture.
            \\adirectory\\a must be an existing directory. If \\apackage\\a is
            omitted then all binary packages are installed. This action
            requires \\bnmake\\b.]
        [+license\\b [ \\apackage\\a ... ]]?List the source license(s) for
            \\apackage\\a on the standard output. Note that individual
            components in \\apackage\\a may contain additional or replacement
            licenses.]
        [+list\\b [ \\apackage\\a ... ]]?List the name, version and
            prerequisites for \\apackage\\a on the standard output.]
        [+make\\b [ \\apackage\\a ]] [ \\atarget\\a ... ]]?Build and
            install. The default \\atarget\\a is \\binstall\\b, which makes and
            installs \\apackage\\a. If the standard output is a terminal then
            the output is also captured in
            \\b$INSTALLROOT/lib/package/gen/make.out\\b. The build is done in
            the \\b$INSTALLROOT\\b directory tree viewpathed on top of the
            \\b$PACKAGEROOT\\b directory tree. If \\bflat\\b is specified then
            the \\b$INSTALLROOT\\b { bin fun include lib } directories are
            linked to the same directories in the package root. Only one
            architecture may be \\bflat\\b. Leaf directory names matching the
            \\b|\\b-separated shell pattern \\b$MAKESKIP\\b are ignored. The
            \\bview\\b action is done before making.]
        [+read\\b [ \\apackage\\a ... | \\aarchive\\a ... ]]?Read the named
            package or archive(s). Must be run from the package root
            directory. Archives are searched for in \\b.\\b and
            \\blib/package/tgz\\b. Each package archive is read only once.
            The file \\blib/package/tgz/\\b\\apackage\\a[.\\atype\\a]]\\b.tim\\b
            tracks the read time. See the \\bwrite\\b action for archive
            naming conventions. Text file archive member are assumed to be
            ASCII or UTF-8 encoded.]
        [+regress?\\bdiff\\b(1) the current and previous \\bpackage test\\b
            results.]
        [+release\\b [ [\\aCC\\a]]\\aYY-MM-DD\\a [ [\\acc\\a]]\\ayy-mm-dd\\a ]]
            ]] [ \\apackage\\a ]]?Display recent changes for the date range
        [\\aCC\\a]]\\aYY-MM-DD\\a (up to [\\acc\\a]]\\ayy-mm-dd\\a.), where
            \\b-\\b means lowest (or highest.) If no dates are specified then
            changes for the last 4 months are listed. \\apackage\\a may be a
            package or component name.]
        [+remove\\b [ \\apackage\\a ]]?Remove files installed for
            \\apackage\\a.]
        [+results\\b [ \\bfailed\\b ]] [ \\bpath\\b ]] [ \\bold\\b ]] [
            \\bmake\\b | \\btest\\b | \\bwrite\\b ]]?List results and interesting
            messages captured by the most recent \\bmake\\b (default),
            \\btest\\b or \\bwrite\\b action. \\bold\\b specifies the previous
            results, if any (current and previous results are retained.)
            \\b$HOME/.pkgresults\\b, if it exists, must contain an
            \\begrep\\b(1) expression of result lines to be ignored.
            \\bfailed\\b lists failures only and \\bpath\\b lists the results
            file path name only.]
        [+setup\\b [ beta ]] [ binary ]] [ source ]] [ url \\aalias\\a ]]
        [ \\aarchitecture\\a ... ]] [ \\aurl\\a ]] [ \\apackage\\a ...
            ]]?This action initializes the current directory as a package
            root, runs the \\bupdate\\b action to download new or out of date
            packages, and runs the \\bread\\b action on those packages. If
            \\bflat\\b is specified then the \\b$INSTALLROOT\\b { bin fun
            include lib } directories are linked to the same directories in
            the package root. Only one architecture may be \\bflat\\b.
            \\bbeta\\b acesses beta packages; download these at your own
            risk. The \\bmake\\b action must be run separately to build
            updated source packages. \\aalias\\a specifies a url alias file
            that contains definitions for the shell variables \\burl\\b,
            \\bauthorize\\b and \\bpassword\\b. If \\aalias\\a and \\aurl\\a are
            omitted then the \\aurl\\a from
            \\b$PACKAGEROOT/lib/package/tgz/\'$default_url$\'\\b is used. If
            \\aurl\\a is specified and
            \\b$PACKAGEROOT/lib/package/tgz/\'$default_url$\'\\b does not exist
            then it is initialized with the current \\aurl\\a, \\bauthorize\\b
            and \\bpassword\\b values and read permission for the current
            user only. If no packages are specified then all previously
            downloaded packages are updated.]
        [+test\\b [ \\apackage\\a ]]?Run the regression tests for
            \\apackage\\a. If the standard output is a terminal then the
            output is also captured in
            \\b$INSTALLROOT/lib/package/gen/test.out\\b. In general a package
            must be made before it can be tested. Components tested with
            the \\bregress\\b(1) command require \\bksh93\\b.]
        [+update\\b [ authorize \\aname\\a ]] [ beta ]] [ binary ]] [
            password \\apassword\\a ]] [ source ]] [ url \\aalias\\a ]] [
            \\aarchitecture\\a ... ]] [ \\aurl\\a ]] [ \\apackage\\a ...
            ]]?Download the latest release of the selected and required
            packages from \\aurl\\a (e.g.,
            \\bhttp://www.research.att.com/sw/download\\b) into the directory
            \\b$PACKAGEROOT/lib/package/tgz\\b. \\bbeta\\b acesses beta
            packages; download these at your own risk. If \\aarchitecture\\a
            is omitted then only architectures already present in the tgz
            directory will be downloaded. If \\aarchitecture\\a is \\b-\\b then
            all posted architectures will be downloaded. If \\aurl\\a matches
            \\b*.url\\b then it is interpreted as a file whose contents is
            the url, else if \\aurl\\a is specified then it is copied to the
            file \\b$PACKAGEROOT/lib/package/tgz/default.url\\b, otherwise
            the url in \\bdefault.url\\b is used. If \\apackage\\a is omitted
            then only packages already present in the tgz directory will be
            downloaded. If \\apackage\\a is \\b-\\b then all posted packages
            will be downloaded. If \\bsource\\b and \\bbinary\\b are omitted
            then both source and binary packages will be downloaded. If
            \\bonly\\b is specified then only the named packages are updated;
            otherwise the closure of required packages is updated. This
            action requires \\bcurl\\b(1), \\bwget\\b(1) or a shell that
            supports io to \\b/dev/tcp/\\b\\ahost\\a/\\aport\\a.]
        [+use\\b [ \\auid\\a | \\apackage\\a | - ]] [ command ...]]?Run
            \\acommand\\a, or an interactive shell if \\acommand\\a is omitted,
            with the environment initialized for using the package (can you
            say \\ashared\\a \\alibrary\\a or \\adll\\a without cussing?) If
            either \\auid\\a or \\apackage\\a is specified then it is used to
            determine a \\b$PACKAGEROOT\\b, possibly different from the
            current directory. For example, to try out bozo`s package:
            \\bpackage use bozo\\b. The \\buse\\b action may be run from any
            directory. If the file \\b$INSTALLROOT/lib/package/profile\\b is
            readable then it is sourced to initialize the environment.]
        [+verify\\b [ \\apackage\\a ]]?Verify installed binary files
            against the checksum files in
            \\b$INSTALLROOT/lib/\\b\\apackage\\a\\b/gen/*.sum\\b. The checksum
            files contain mode, user and group information. If the checksum
            matches for a given file then the mode, user and group are
            changed as necessary to match the checksum entry. A warning is
            printed on the standard error for each mismatch. Requires the
            \\bast\\b package \\bcksum\\b(1) command.]
        [+view\\b?Initialize the architecture specific viewpath
            hierarchy. If \\bflat\\b is specified then the \\b$INSTALLROOT\\b {
            bin fun include lib } directories are linked to the same
            directories in the package root. Only one architecture may be
            \\bflat\\b. The \\bmake\\b action implicitly calls this action.]
        [+write\\b [\\aformat\\a]] \\atype\\a ... [ \\apackage\\a ...]]?Write
            a package archive for \\apackage\\a. All work is done in the
            \\b$PACKAGEROOT/lib/package\\b directory. \\aformat\\a-specific
            files are placed in the \\aformat\\a subdirectory. A
            \\apackage\\a[.\\atype\\a]]\\b.tim\\b file in this directory tracks
            the write time and prevents a package from being read in the
            same root it was written. If more than one file is generated
            for a particular \\aformat\\a then those files are placed in the
            \\aformat\\a/\\apackage\\a subdirectory. File names in the
            \\aformat\\a subdirectory will contain the package name, a
            \\ayyyy-mm-dd\\a date, and for binary packages, \\aHOSTTYPE\\a. If
            \\apackage\\a is omitted then an ordered list of previously
            written packages is generated. If \\bonly\\b is specified then
            only the named packages will be written; otherwise prerequisite
            packages are written first. Package components must be listed
            in \\apackage\\a\\b.pkg\\b. \\aformat\\a may be one of:]
            {
                [+cyg?Generate a \\bcygwin\\b package.]
                [+exp?Generate an \\bexptools\\b maintainer source
                    archive and \\aNPD\\a file, suitable for \\bexpmake\\b(1)]
                [+lcl?Generate a package archive suitable for
                    restoration into the local source tree (i.e., the
                    source is not annotated for licencing.)]
                [+pkg?Generate a \\bpkgmk\\b(1) package suitable for
                    \\bpkgadd\\b(1).]
                [+rpm?Generate an \\brpm\\b(1) package.]
                [+tgz?Generate a \\bgzip\\b(1) \\btar\\b(1) package
                    archive. This is the default.]
            }
        [+?\\btype\\b specifies the package type which must be one of
            \\bsource\\b, \\bbinary\\b or \\bruntime\\b. A source package
            contains the source needed to build the corresponding binary
            package. A binary package includes the libraries and headers
            needed for compiling and linking against the public interfaces.
            A runtime package contains the commands and required dynamic
            libraries.]
        [+?A package may be either a \\bbase\\b or \\bdelta\\b. A base
            package contains a complete copy of all components. A delta
            package contains only changes from a previous base package.
            Delta recipients must have the \\bast\\b \\bpax\\b(1) command (in
            the \\bast-base\\b package.) If neither \\bbase\\b nor \\bdelta\\b is
            specified, then the current base is overwritten if there are no
            deltas referring to the current base. Only the \\btgz\\b and
            \\blcl\\b formats support \\bdelta\\b. If \\bbase\\b is specified
            then a new base and two delta archives are generated: one delta
            to generate the new base from the old, and one delta to
            generate the old base from the new; the old base is then
            removed. If \\bdelta\\b is specified then a new delta referring
            to the current base is written.]
        [+?\\apackage\\a\\b.pkg\\b may reference other packages. By default
            a pointer to those packages is written. The recipient \\bpackage
            read\\b will then check that all required packages have been
            downloaded. If \\bclosure\\b is specified then the components for
            all package references are included in the generated package.
            This may be useful for \\blcl\\b and versioning.]
        [+?All formats but \\blcl\\b annotate each \\bsource\\b file (not
            already annotated) with a license comment as it is written to
            the package archive using \\bproto\\b(1).]
    }
[+DETAILS?The package directory hierarchy is rooted at
    \\b$PACKAGEROOT\\b. All source and binaries reside under this tree. A two
    level viewpath is used to separate source and binaries. The top view is
    architecture specific, the bottom view is shared source. All building
    is done in the architecture specific view; no source view files are
    intentionally changed. This means that many different binary
    architectures can be made from a single copy of the source.]
[+?Independent \\b$PACKAGEROOT\\b hierarchies can be combined by
    appending \\b$INSTALLROOT:$PACKAGEROOT\\b pairs to \\bVPATH\\b. The
    \\bVPATH\\b viewing order is from left to right. Each \\b$PACKAGEROOT\\b
    must have a \\b$PACKAGEROOT/lib/package\\b directory.]
[+?Each package contains one or more components. Component source for
    the \\afoo\\a command is in \\b$PACKAGEROOT/src/cmd/\\b\\afoo\\a, and source
    for the \\abar\\a library is in \\b$PACKAGEROOT/src/lib/lib\\b\\abar\\a. This
    naming is for convenience only; the underlying makefiles handle
    inter-component build order. The \\bINIT\\b component, which contains
    generic package support files, is always made first, then the
    components named \\bINIT\\b*, then the component order determined by the
    closure of component makefile dependencies.]
[+?\\b$PACKAGEROOT/lib/package\\b contains package specific files. The
    package naming convention is \\agroup\\a[-\\apart\\a]]; e.g., \\bast-base\\b,
    \\bgnu-fileutils\\b. The *\\b.pkg\\b files are ast \\bnmake\\b(1) makefiles
    that contain the package name, package components, references to other
    packages, and a short package description. *\\b.pkg\\b files are used by
    \\bpackage write\\b to generate new source and binary packages.]
[+?\\b$PACKAGEROOT/lib/package/\\b\\agroup\\a\\b.lic\\b files contain license
    information that is used by the \\bast\\b \\bproto\\b(1) and \\bnmake\\b(1)
    commands to generate source and binary license strings. \\agroup\\a is
    determined by the first \\b:PACKAGE:\\b operator name listed in the
    component \\bnmake\\b makefile. \\agroup\\a\\b.lic\\b files are part of the
    licensing documentation and must not be altered; doing so violates the
    license. Each component may have its own \\bLICENSE\\b file that
    overrides the \\agroup\\a\\b.lic\\b file. The full text of the licenses are
    in the \\b$PACKAGEROOT/lib/package/LICENSES\\b and
    \\b$INSTALLROOT/lib/package/LICENSES\\b directories.]
[+?A few files are generated in \\b$PACKAGEROOT/lib/package/gen\\b and
    \\b$INSTALLROOT/lib/package/gen\\b. \\apackage\\a\\b.ver\\b contains one line
    consisting of \\apackage version release\\a \\b1\\b for the most recent
    instance of \\apackage\\a read into \\b$PACKAGEROOT\\b, where \\apackage\\a
    is the package name, \\aversion\\a is the \\aYYYY-MM-DD\\a base version,
    and \\arelease\\a is \\aversion\\a for the base release or \\aYYYY-MM-DD\\a
    for delta releases. \\apackage\\a\\b.req\\b contains *\\b.ver\\b entries for
    the packages required by \\apackage\\a, except that the fourth field is
    \\b0\\b instead of \\b1\\b. All packages except \\bINIT\\b require the
    \\bINIT\\b package. A simple sort of \\apackage\\a\\b.pkg\\b and *\\b.ver\\b
    determines if the required package have been read in. Finally,
    \\apackage\\a\\b.README\\b and \\apackage\\a\\a.html\\b contain the README text
    for \\apackage\\a and all its components. Included are all changes added
    to the component \\bRELEASE\\b, \\bCHANGES\\b or \\bChangeLog\\b files dated
    since the two most recent base releases. Component \\bRELEASE\\b files
    contain tag lines of the form [\\aYY\\a]]\\aYY-MM-DD\\a [ \\atext\\a ]] (or
    \\bdate\\b(1) format dates) followed by README text, in reverse
    chronological order (newer entries at the top of the file.) \\bpackage
    release\\b lists this information, and \\bpackage contents ...\\b lists
    the descriptions and components.]
[+?\\b$HOSTYPE\\b names the current binary architecture and is determined
    by the output of \\bpackage\\b (no arguments.) The \\b$HOSTTYPE\\b naming
    scheme is used to separate incompatible executable and object formats.
    All architecture specific binaries are placed under \\b$INSTALLROOT\\b
    (\\b$PACKAGEROOT/arch/$HOSTTYPE\\b.) There are a few places that match
    against \\b$HOSTTYPE\\b when making binaries; these are limited to
    makefile compiler workarounds, e.g., if \\b$HOSTTYPE\\b matches \\bhp.*\\b
    then turn off the optimizer for these objects. All other architecture
    dependent logic is handled either by the \\bast\\b \\biffe\\b(1) command or
    by component specific configure scripts.]
[+?Each component contains an \\bast\\b \\bnmake\\b(1) makefile (either
    \\bNmakefile\\b or \\bMakefile\\b) and a \\bMAM\\b (make abstract machine)
    file (\\bMamfile\\b.) A Mamfile contains a portable makefile description
    that is used by \\bmamake\\b(1) to simulate \\bnmake\\b. Currently there is
    no support for old-make/gnu-make makefiles; if the binaries are just
    being built then \\bmamake\\b will suffice; if source or makefile
    modifications are anticipated then \\bnmake\\b (in the \\bast-base\\b
    package) should be used. Mamfiles are automatically generated by
    \\bpackage write\\b.]
[+?Most component C source is prototyped. If \\b$CC\\b (default value
    \\bcc\\b) is not a prototyping C compiler then \\bpackage make\\b runs
    \\bproto\\b(1) on portions of the \\b$PACKAGEROOT/src\\b tree and places
    the converted output files in the \\b$PACKAGEROOT/proto/src\\b tree.
    Converted files are then viewpathed over the original source.
    \\bproto\\b(1) converts an ANSI C subset to code that is compatible with
    K&R, ANSI, and C++ dialects.]
[+?All scripts and commands under \\b$PACKAGEROOT\\b use \\b$PATH\\b
    relative pathnames (via the \\bast\\b \\bpathpath\\b(3) function); there
    are no imbedded absolute pathnames. This means that binaries generated
    under \\b$PACKAGEROOT\\b may be copied to a different root; users need
    only change their \\b$PATH\\b variable to reference the new installation
    root \\bbin\\b directory. \\bpackage install\\b installs binary packages in
    a new \\b$INSTALLROOT\\b.]'
		OUTPUT - $'[+DESCRIPTION?The \\bpackage\\b command controls source and binary
    packages. It is a \\bsh\\b(1) script coded for maximal portability. All
    package files are in the \\b$PACKAGEROOT\\b directory tree.
    \\b$PACKAGEROOT\\b must at minumum contain a \\bbin/package\\b command or a
    \\blib/package\\b directory. Binary package files are in the
    \\b$INSTALLROOT\\b (\\b$PACKAGEROOT/arch/\\b\\ahosttype\\a) tree, where
    \\ahosttpe\\a=`\\bpackage\\b`. All \\aactions\\a but \\bhost\\b and \\buse\\b
    require the current directory to be under \\b$PACKAGEROOT\\b. See
    \\bDETAILS\\b for more information.]
[+?Note that no environment variables need be set by the user;
    \\bpackage\\b determines the environment based on the current working
    directory. The \\buse\\b action starts a \\bsh\\b(1) with the environment
    initialized. \\bCC\\b, \\bCCFLAGS\\b, \\bHOSTTYPE\\b and \\bSHELL\\b may be set
    by explicit command argument assignments to override the defaults.]
[+?Packages are composed of components. Each component is built and
    installed by an \\bast\\b \\bnmake\\b(1) makefile. Each package is also
    described by an \\bnmake\\b makefile that lists its components and
    provides a content description. The package makefile and component
    makefiles provide all the information required to read, write, build
    and install packages.]
[+?Package recipients only need \\bsh\\b(1) and \\bcc\\b(1) to build and
    install source packages, and \\bsh\\b to install binary packages.
    \\bnmake\\b and \\bksh93\\b are required to write new packages. An
    \\b$INSTALLROOT/bin/cc\\b script may be supplied for some architectures.
    This script supplies a reasonable set of default options for compilers
    that accept multiple dialects or generate multiple object/executable
    formats.]
[+?The command arguments are composed of a sequence of words: zero or
    more \\aqualifiers\\a, one \\aaction\\a, and zero or more action-specific
    \\aarguments\\a, and zero or more \\aname=value\\a definitions. \\apackage\\a
    names a particular package. The naming scheme is a \\b-\\b separated
    hierarchy; the leftmost parts describe ownership, e.g.,
    \\bgnu-fileutils\\b, \\bast-base\\b. If no packages are specified then all
    packages are operated on. \\boptget\\b(3) documentation options are also
    supported. The default with no arguments is \\bhost type\\b.]
[+?The qualifiers are:]
    {
        [+debug|environment?Show environment and actions but do not
            execute.]
        [+flat?Collapse \\b$INSTALLROOT\\b { bin fun include lib } onto
            \\b$PACKAGEROOT\\b.]
        [+force?Force the action to override saved state.]
        [+never?Run make -N and show other actions.]
        [+only?Only operate on the specified packages.]
        [+quiet?Do not list captured action output.]
        [+show?Run make -n and show other actions.]
        [+verbose?Provide detailed action output.]
        [+DEBUG?Trace the package script actions in detail.]
    }
[+?The actions are:]
    {
        [+admin\\b [\\ball\\b]] [\\bdb\\b \\afile\\a]] [\\bon\\b \\apattern\\a]][\\aaction\\a ...]]?Apply
            \\aaction\\a ... to the hosts listed in \\afile\\a. If \\afile\\a is
            omitted then \\badmin.db\\b is assumed. The caller must have
            \\brcp\\b(1) and \\brsh\\b(1) or \\bscp\\b(1) and \\bssh\\b(1) access
            to the hosts. Output for \\aaction\\a is saved per-host in the
            file \\aaction\\a\\b.log/\\b\\ahost\\a. Logs can be viewed by
            \\bpackage admin\\b [\\bon\\b \\ahost\\a]] \\bresults\\b [\\aaction\\a]].
            By default only local PACKAGEROOT hosts are selected from
            \\afile\\a; \\ball\\b selects all hosts. \\bon\\b \\apattern\\a selects
            only hosts matching the \\b|\\b separated \\apattern\\a. \\afile\\a
            contains four types of lines. Blank lines and lines beginning
            with \\b#\\b are ignored. Lines starting with \\aid\\a=\\avalue\\a
            are variable assignments. Set admin_ping to local conventions
            if \\"\'$admin_ping$\'\\" fails. If a package list is not specified
            on the command line the \\aaction\\a applies to all packages; a
            variable assigment \\bpackage\\b=\\"\\alist\\a\\" applies \\aaction\\a
            to the packages in \\alist\\a for subsequent hosts in \\afile\\a.
            The remaining line type is a host description consisting of 6
            tab separated fields. The first 3 are mandatory; the remaining
            3 are updated by the \\badmin\\b action. \\afile\\a is saved in
            \\afile\\a\\b.old\\b before update. The fields are:]
            {
                [+hosttype?The host type as reported by
                    \\"\\bpackage\\b\\".]
                [+[user@]]host?The host name and optionally user name
                    for \\brcp\\b(1) and \\brsh\\b(1) access.]
                [+[remote::]]PACKAGEROOT?The absolute remote package
                    root directory and optionally the remote protocol (rsh
                    or ssh) if the directory is on a different server than
                    the master package root directory. If
                    \\blib/package/admin/\'$admin_env$\'\\b exists under this
                    directory then it is sourced by \\bsh\\b(1) before
                    \\aaction\\a is done. If this field begins with \\b-\\b
                    then the host is ignored. If this field contains \\b:\\b
                    then \\bditto\\b(1) is used to sync the remote \\bsrc\\b
                    directory hierarchy to the local one. These directories
                    must exist on the remote side: \\blib/package\\b,
                    \\bsrc/cmd\\b, \\bsrc/lib\\b.]
                [+date?\\aYYMMDD\\a of the last action.]
                [+time?Elapsed wall time for the last action.]
                [+M T W?The \\badmin\\b action \\bmake\\b, \\btest\\b and
                    \\bwrite\\b action error counts. A non-numeric value in
                    any of these fields disables the corresponding action.]
            }
        [+contents\\b [ \\apackage\\a ... ]]?List description and
            components for \\apackage\\a on the standard output.]
        [+copyright\\b [ \\apackage\\a ... ]]?List the general copyright
            notice(s) for \\apackage\\a on the standard output. Note that
            individual components in \\apackage\\a may contain additional or
            replacement notices.]
        [+export\\b \\avariable\\a ...?List \\aname\\a=\\avalue\\a for
            \\avariable\\a, one per line. If the \\bonly\\b attribute is
            specified then only the variable values are listed.]
        [+help\\b [ \\aaction\\a ]]?Display help text on the standard
            error (standard output for \\aaction\\a).]
        [+host\\b [ \\aattribute\\a ... ]]?List
            architecture/implementation dependent host information on the
            standard output. \\btype\\b is listed if no attributes are
            specified. Information is listed on a single line in
            \\aattribute\\a order. The attributes are:]
            {
                [+canon \\aname\\a?An external host type name to be
                    converted to \\bpackage\\b syntax.]
                [+cpu?The number of cpus; 1 if the host is not a
                    multiprocessor.]
                [+name?The host name.]
                [+rating?The cpu rating in pseudo mips; the value is
                    useful useful only in comparisons with rating values of
                    other hosts. Other than a vax rating (mercifully) fixed
                    at 1, ratings can vary wildly but consistently from
                    vendor mips ratings. \\bcc\\b(1) may be required to
                    determine the rating.]
                [+type?The host type, usually in the form
                    \\avendor\\a.\\aarchitecture\\a, with an optional trailing
                    -\\aversion\\a. The main theme is that type names within
                    a family of architectures are named in a similar,
                    predictable style. OS point release information is
                    avoided as much as possible, but vendor resistance to
                    release incompatibilities has for the most part been
                    futile.]
            }
        [+html\\b [ \\aaction\\a ]]?Display html help text on the standard
            error (standard output for \\aaction\\a).]
        [+install\\b [ flat ]] [ \\aarchitecture\\a ... ]] \\adirectory\\a [\\apackage\\a ... ]]?Copy
            the package binary hierarchy to \\adirectory\\a. If
            \\aarchitecture\\a is omitted then all architectures are
            installed. If \\bflat\\b is specified then exactly one
            \\aarchitecture\\a must be specified; this architecture will be
            installed in \\adirectory\\a without the \\barch/\\b\\aHOSTTYPE\\a
            directory prefixes. Otherwise each architecture will be
            installed in a separate \\barch/\\b\\aHOSTTYPE\\a subdirectory of
            \\adirectory\\a. The \\aarchitecture\\a \\b-\\b names the current
            architecture. \\adirectory\\a must be an existing directory. If
            \\apackage\\a is omitted then all binary packages are installed.
            This action requires \\bnmake\\b.]
        [+license\\b [ \\apackage\\a ... ]]?List the source license(s) for
            \\apackage\\a on the standard output. Note that individual
            components in \\apackage\\a may contain additional or replacement
            licenses.]
        [+list\\b [ \\apackage\\a ... ]]?List the name, version and
            prerequisites for \\apackage\\a on the standard output.]
        [+make\\b [ \\apackage\\a ]] [ \\atarget\\a ... ]]?Build and
            install. The default \\atarget\\a is \\binstall\\b, which makes and
            installs \\apackage\\a. If the standard output is a terminal then
            the output is also captured in
            \\b$INSTALLROOT/lib/package/gen/make.out\\b. The build is done in
            the \\b$INSTALLROOT\\b directory tree viewpathed on top of the
            \\b$PACKAGEROOT\\b directory tree. If \\bflat\\b is specified then
            the \\b$INSTALLROOT\\b { bin fun include lib } directories are
            linked to the same directories in the package root. Only one
            architecture may be \\bflat\\b. Leaf directory names matching the
            \\b|\\b-separated shell pattern \\b$MAKESKIP\\b are ignored. The
            \\bview\\b action is done before making.]
        [+read\\b [ \\apackage\\a ... | \\aarchive\\a ... ]]?Read the named
            package or archive(s). Must be run from the package root
            directory. Archives are searched for in \\b.\\b and
            \\blib/package/tgz\\b. Each package archive is read only once.
            The file \\blib/package/tgz/\\b\\apackage\\a[.\\atype\\a]]\\b.tim\\b
            tracks the read time. See the \\bwrite\\b action for archive
            naming conventions. Text file archive member are assumed to be
            ASCII or UTF-8 encoded.]
        [+regress?\\bdiff\\b(1) the current and previous \\bpackage test\\b
            results.]
        [+release\\b [ [\\aCC\\a]]\\aYY-MM-DD\\a [ [\\acc\\a]]\\ayy-mm-dd\\a ]]]] [ \\apackage\\a ]]?Display
            recent changes for the date range [\\aCC\\a]]\\aYY-MM-DD\\a (up to
        [\\acc\\a]]\\ayy-mm-dd\\a.), where \\b-\\b means lowest (or highest.)
            If no dates are specified then changes for the last 4 months
            are listed. \\apackage\\a may be a package or component name.]
        [+remove\\b [ \\apackage\\a ]]?Remove files installed for
            \\apackage\\a.]
        [+results\\b [ \\bfailed\\b ]] [ \\bpath\\b ]] [ \\bold\\b ]] [\\bmake\\b | \\btest\\b | \\bwrite\\b ]]?List
            results and interesting messages captured by the most recent
            \\bmake\\b (default), \\btest\\b or \\bwrite\\b action. \\bold\\b
            specifies the previous results, if any (current and previous
            results are retained.) \\b$HOME/.pkgresults\\b, if it exists,
            must contain an \\begrep\\b(1) expression of result lines to be
            ignored. \\bfailed\\b lists failures only and \\bpath\\b lists the
            results file path name only.]
        [+setup\\b [ beta ]] [ binary ]] [ source ]] [ url \\aalias\\a ]][ \\aarchitecture\\a ... ]] [ \\aurl\\a ]] [ \\apackage\\a ...]]?This
            action initializes the current directory as a package root,
            runs the \\bupdate\\b action to download new or out of date
            packages, and runs the \\bread\\b action on those packages. If
            \\bflat\\b is specified then the \\b$INSTALLROOT\\b { bin fun
            include lib } directories are linked to the same directories in
            the package root. Only one architecture may be \\bflat\\b.
            \\bbeta\\b acesses beta packages; download these at your own
            risk. The \\bmake\\b action must be run separately to build
            updated source packages. \\aalias\\a specifies a url alias file
            that contains definitions for the shell variables \\burl\\b,
            \\bauthorize\\b and \\bpassword\\b. If \\aalias\\a and \\aurl\\a are
            omitted then the \\aurl\\a from
            \\b$PACKAGEROOT/lib/package/tgz/\'$default_url$\'\\b is used. If
            \\aurl\\a is specified and
            \\b$PACKAGEROOT/lib/package/tgz/\'$default_url$\'\\b does not exist
            then it is initialized with the current \\aurl\\a, \\bauthorize\\b
            and \\bpassword\\b values and read permission for the current
            user only. If no packages are specified then all previously
            downloaded packages are updated.]
        [+test\\b [ \\apackage\\a ]]?Run the regression tests for
            \\apackage\\a. If the standard output is a terminal then the
            output is also captured in
            \\b$INSTALLROOT/lib/package/gen/test.out\\b. In general a package
            must be made before it can be tested. Components tested with
            the \\bregress\\b(1) command require \\bksh93\\b.]
        [+update\\b [ authorize \\aname\\a ]] [ beta ]] [ binary ]] [password \\apassword\\a ]] [ source ]] [ url \\aalias\\a ]] [\\aarchitecture\\a ... ]] [ \\aurl\\a ]] [ \\apackage\\a ...]]?Download
            the latest release of the selected and required packages from
            \\aurl\\a (e.g., \\bhttp://www.research.att.com/sw/download\\b)
            into the directory \\b$PACKAGEROOT/lib/package/tgz\\b. \\bbeta\\b
            acesses beta packages; download these at your own risk. If
            \\aarchitecture\\a is omitted then only architectures already
            present in the tgz directory will be downloaded. If
            \\aarchitecture\\a is \\b-\\b then all posted architectures will be
            downloaded. If \\aurl\\a matches \\b*.url\\b then it is interpreted
            as a file whose contents is the url, else if \\aurl\\a is
            specified then it is copied to the file
            \\b$PACKAGEROOT/lib/package/tgz/default.url\\b, otherwise the url
            in \\bdefault.url\\b is used. If \\apackage\\a is omitted then only
            packages already present in the tgz directory will be
            downloaded. If \\apackage\\a is \\b-\\b then all posted packages
            will be downloaded. If \\bsource\\b and \\bbinary\\b are omitted
            then both source and binary packages will be downloaded. If
            \\bonly\\b is specified then only the named packages are updated;
            otherwise the closure of required packages is updated. This
            action requires \\bcurl\\b(1), \\bwget\\b(1) or a shell that
            supports io to \\b/dev/tcp/\\b\\ahost\\a/\\aport\\a.]
        [+use\\b [ \\auid\\a | \\apackage\\a | - ]] [ command ...]]?Run
            \\acommand\\a, or an interactive shell if \\acommand\\a is omitted,
            with the environment initialized for using the package (can you
            say \\ashared\\a \\alibrary\\a or \\adll\\a without cussing?) If
            either \\auid\\a or \\apackage\\a is specified then it is used to
            determine a \\b$PACKAGEROOT\\b, possibly different from the
            current directory. For example, to try out bozo`s package:
            \\bpackage use bozo\\b. The \\buse\\b action may be run from any
            directory. If the file \\b$INSTALLROOT/lib/package/profile\\b is
            readable then it is sourced to initialize the environment.]
        [+verify\\b [ \\apackage\\a ]]?Verify installed binary files
            against the checksum files in
            \\b$INSTALLROOT/lib/\\b\\apackage\\a\\b/gen/*.sum\\b. The checksum
            files contain mode, user and group information. If the checksum
            matches for a given file then the mode, user and group are
            changed as necessary to match the checksum entry. A warning is
            printed on the standard error for each mismatch. Requires the
            \\bast\\b package \\bcksum\\b(1) command.]
        [+view\\b?Initialize the architecture specific viewpath
            hierarchy. If \\bflat\\b is specified then the \\b$INSTALLROOT\\b {
            bin fun include lib } directories are linked to the same
            directories in the package root. Only one architecture may be
            \\bflat\\b. The \\bmake\\b action implicitly calls this action.]
        [+write\\b [\\aformat\\a]] \\atype\\a ... [ \\apackage\\a ...]]?Write
            a package archive for \\apackage\\a. All work is done in the
            \\b$PACKAGEROOT/lib/package\\b directory. \\aformat\\a-specific
            files are placed in the \\aformat\\a subdirectory. A
            \\apackage\\a[.\\atype\\a]]\\b.tim\\b file in this directory tracks
            the write time and prevents a package from being read in the
            same root it was written. If more than one file is generated
            for a particular \\aformat\\a then those files are placed in the
            \\aformat\\a/\\apackage\\a subdirectory. File names in the
            \\aformat\\a subdirectory will contain the package name, a
            \\ayyyy-mm-dd\\a date, and for binary packages, \\aHOSTTYPE\\a. If
            \\apackage\\a is omitted then an ordered list of previously
            written packages is generated. If \\bonly\\b is specified then
            only the named packages will be written; otherwise prerequisite
            packages are written first. Package components must be listed
            in \\apackage\\a\\b.pkg\\b. \\aformat\\a may be one of:]
            {
                [+cyg?Generate a \\bcygwin\\b package.]
                [+exp?Generate an \\bexptools\\b maintainer source
                    archive and \\aNPD\\a file, suitable for \\bexpmake\\b(1)]
                [+lcl?Generate a package archive suitable for
                    restoration into the local source tree (i.e., the
                    source is not annotated for licencing.)]
                [+pkg?Generate a \\bpkgmk\\b(1) package suitable for
                    \\bpkgadd\\b(1).]
                [+rpm?Generate an \\brpm\\b(1) package.]
                [+tgz?Generate a \\bgzip\\b(1) \\btar\\b(1) package
                    archive. This is the default.]
            }
        [+?\\btype\\b specifies the package type which must be one of
            \\bsource\\b, \\bbinary\\b or \\bruntime\\b. A source package
            contains the source needed to build the corresponding binary
            package. A binary package includes the libraries and headers
            needed for compiling and linking against the public interfaces.
            A runtime package contains the commands and required dynamic
            libraries.]
        [+?A package may be either a \\bbase\\b or \\bdelta\\b. A base
            package contains a complete copy of all components. A delta
            package contains only changes from a previous base package.
            Delta recipients must have the \\bast\\b \\bpax\\b(1) command (in
            the \\bast-base\\b package.) If neither \\bbase\\b nor \\bdelta\\b is
            specified, then the current base is overwritten if there are no
            deltas referring to the current base. Only the \\btgz\\b and
            \\blcl\\b formats support \\bdelta\\b. If \\bbase\\b is specified
            then a new base and two delta archives are generated: one delta
            to generate the new base from the old, and one delta to
            generate the old base from the new; the old base is then
            removed. If \\bdelta\\b is specified then a new delta referring
            to the current base is written.]
        [+?\\apackage\\a\\b.pkg\\b may reference other packages. By default
            a pointer to those packages is written. The recipient \\bpackage
            read\\b will then check that all required packages have been
            downloaded. If \\bclosure\\b is specified then the components for
            all package references are included in the generated package.
            This may be useful for \\blcl\\b and versioning.]
        [+?All formats but \\blcl\\b annotate each \\bsource\\b file (not
            already annotated) with a license comment as it is written to
            the package archive using \\bproto\\b(1).]
    }
[+DETAILS?The package directory hierarchy is rooted at
    \\b$PACKAGEROOT\\b. All source and binaries reside under this tree. A two
    level viewpath is used to separate source and binaries. The top view is
    architecture specific, the bottom view is shared source. All building
    is done in the architecture specific view; no source view files are
    intentionally changed. This means that many different binary
    architectures can be made from a single copy of the source.]
[+?Independent \\b$PACKAGEROOT\\b hierarchies can be combined by
    appending \\b$INSTALLROOT:$PACKAGEROOT\\b pairs to \\bVPATH\\b. The
    \\bVPATH\\b viewing order is from left to right. Each \\b$PACKAGEROOT\\b
    must have a \\b$PACKAGEROOT/lib/package\\b directory.]
[+?Each package contains one or more components. Component source for
    the \\afoo\\a command is in \\b$PACKAGEROOT/src/cmd/\\b\\afoo\\a, and source
    for the \\abar\\a library is in \\b$PACKAGEROOT/src/lib/lib\\b\\abar\\a. This
    naming is for convenience only; the underlying makefiles handle
    inter-component build order. The \\bINIT\\b component, which contains
    generic package support files, is always made first, then the
    components named \\bINIT\\b*, then the component order determined by the
    closure of component makefile dependencies.]
[+?\\b$PACKAGEROOT/lib/package\\b contains package specific files. The
    package naming convention is \\agroup\\a[-\\apart\\a]]; e.g., \\bast-base\\b,
    \\bgnu-fileutils\\b. The *\\b.pkg\\b files are ast \\bnmake\\b(1) makefiles
    that contain the package name, package components, references to other
    packages, and a short package description. *\\b.pkg\\b files are used by
    \\bpackage write\\b to generate new source and binary packages.]
[+?\\b$PACKAGEROOT/lib/package/\\b\\agroup\\a\\b.lic\\b files contain license
    information that is used by the \\bast\\b \\bproto\\b(1) and \\bnmake\\b(1)
    commands to generate source and binary license strings. \\agroup\\a is
    determined by the first \\b:PACKAGE:\\b operator name listed in the
    component \\bnmake\\b makefile. \\agroup\\a\\b.lic\\b files are part of the
    licensing documentation and must not be altered; doing so violates the
    license. Each component may have its own \\bLICENSE\\b file that
    overrides the \\agroup\\a\\b.lic\\b file. The full text of the licenses are
    in the \\b$PACKAGEROOT/lib/package/LICENSES\\b and
    \\b$INSTALLROOT/lib/package/LICENSES\\b directories.]
[+?A few files are generated in \\b$PACKAGEROOT/lib/package/gen\\b and
    \\b$INSTALLROOT/lib/package/gen\\b. \\apackage\\a\\b.ver\\b contains one line
    consisting of \\apackage version release\\a \\b1\\b for the most recent
    instance of \\apackage\\a read into \\b$PACKAGEROOT\\b, where \\apackage\\a
    is the package name, \\aversion\\a is the \\aYYYY-MM-DD\\a base version,
    and \\arelease\\a is \\aversion\\a for the base release or \\aYYYY-MM-DD\\a
    for delta releases. \\apackage\\a\\b.req\\b contains *\\b.ver\\b entries for
    the packages required by \\apackage\\a, except that the fourth field is
    \\b0\\b instead of \\b1\\b. All packages except \\bINIT\\b require the
    \\bINIT\\b package. A simple sort of \\apackage\\a\\b.pkg\\b and *\\b.ver\\b
    determines if the required package have been read in. Finally,
    \\apackage\\a\\b.README\\b and \\apackage\\a\\a.html\\b contain the README text
    for \\apackage\\a and all its components. Included are all changes added
    to the component \\bRELEASE\\b, \\bCHANGES\\b or \\bChangeLog\\b files dated
    since the two most recent base releases. Component \\bRELEASE\\b files
    contain tag lines of the form [\\aYY\\a]]\\aYY-MM-DD\\a [ \\atext\\a ]] (or
    \\bdate\\b(1) format dates) followed by README text, in reverse
    chronological order (newer entries at the top of the file.) \\bpackage
    release\\b lists this information, and \\bpackage contents ...\\b lists
    the descriptions and components.]
[+?\\b$HOSTYPE\\b names the current binary architecture and is determined
    by the output of \\bpackage\\b (no arguments.) The \\b$HOSTTYPE\\b naming
    scheme is used to separate incompatible executable and object formats.
    All architecture specific binaries are placed under \\b$INSTALLROOT\\b
    (\\b$PACKAGEROOT/arch/$HOSTTYPE\\b.) There are a few places that match
    against \\b$HOSTTYPE\\b when making binaries; these are limited to
    makefile compiler workarounds, e.g., if \\b$HOSTTYPE\\b matches \\bhp.*\\b
    then turn off the optimizer for these objects. All other architecture
    dependent logic is handled either by the \\bast\\b \\biffe\\b(1) command or
    by component specific configure scripts.]
[+?Each component contains an \\bast\\b \\bnmake\\b(1) makefile (either
    \\bNmakefile\\b or \\bMakefile\\b) and a \\bMAM\\b (make abstract machine)
    file (\\bMamfile\\b.) A Mamfile contains a portable makefile description
    that is used by \\bmamake\\b(1) to simulate \\bnmake\\b. Currently there is
    no support for old-make/gnu-make makefiles; if the binaries are just
    being built then \\bmamake\\b will suffice; if source or makefile
    modifications are anticipated then \\bnmake\\b (in the \\bast-base\\b
    package) should be used. Mamfiles are automatically generated by
    \\bpackage write\\b.]
[+?Most component C source is prototyped. If \\b$CC\\b (default value
    \\bcc\\b) is not a prototyping C compiler then \\bpackage make\\b runs
    \\bproto\\b(1) on portions of the \\b$PACKAGEROOT/src\\b tree and places
    the converted output files in the \\b$PACKAGEROOT/proto/src\\b tree.
    Converted files are then viewpathed over the original source.
    \\bproto\\b(1) converts an ANSI C subset to code that is compatible with
    K&R, ANSI, and C++ dialects.]
[+?All scripts and commands under \\b$PACKAGEROOT\\b use \\b$PATH\\b
    relative pathnames (via the \\bast\\b \\bpathpath\\b(3) function); there
    are no imbedded absolute pathnames. This means that binaries generated
    under \\b$PACKAGEROOT\\b may be copied to a different root; users need
    only change their \\b$PATH\\b variable to reference the new installation
    root \\bbin\\b directory. \\bpackage install\\b installs binary packages in
    a new \\b$INSTALLROOT\\b.]'

TEST 04 '--optget vs. "..."'

	EXEC	--optget -w76
		INPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one of"
"	the directories on \\b$PATH\\b, searched in order. Each alias is"
"	is a " FOO " and another    "    BAR   " at the end.]"'
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one of the "
    "directories on \\b$PATH\\b, searched in order. Each alias is is a " FOO " and "
    "another " BAR " at the end.]"'

	EXEC	--optget --width=69
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one of the "
    "directories on \\b$PATH\\b, searched in order. Each alias is is a " FOO " "
    "and another " BAR " at the end.]"'

	EXEC	--optget --width=68
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one of "
    "the directories on \\b$PATH\\b, searched in order. Each alias is is a " FOO " "
    "and another " BAR " at the end.]"'

	EXEC	--optget --width=66
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one of "
    "the directories on \\b$PATH\\b, searched in order. Each alias is is "
    "a " FOO " and another " BAR " at the end.]"'

	EXEC	--optget --width=65
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one of "
    "the directories on \\b$PATH\\b, searched in order. Each alias is "
    "is a " FOO " and another " BAR " at the end.]"'

	EXEC	--optget --width=64
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one "
    "of the directories on \\b$PATH\\b, searched in order. Each alias "
    "is is a " FOO " and another " BAR " at the end.]"'

	EXEC	--optget --width=62
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in one "
    "of the directories on \\b$PATH\\b, searched in order. Each "
    "alias is is a " FOO " and another " BAR " at the end.]"'

	EXEC	--optget --width=61
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in "
    "one of the directories on \\b$PATH\\b, searched in order. Each "
    "alias is is a " FOO " and another " BAR " at the end.]"'

	EXEC	--optget --width=60
		OUTPUT - $'"[+?Method aliases may be defined in \\b../" ALIASES "\\b in "
    "one of the directories on \\b$PATH\\b, searched in order. "
    "Each alias is is a " FOO " and another " BAR " at the end.]"'

TEST 05 '--optget vs. "...\n..."'

	EXEC	--optget -w76
		INPUT - $'"\\n\\n[ file ... ]\\n\\n"'
		OUTPUT - $'    "\n "\n"[ file ... ]"\n    "\n "'

	EXEC	--optget -w76
		INPUT - $'"\\n"\n"\\n[ file ... ]\\n"\n"\\n"'
		OUTPUT - $'"[ file ... ]"'

TEST 06 '--optget vs. --usage'

	EXEC	--optget -w76
		INPUT - $'[-?\\n@(#)$Id: id (AT&T Research) 2004-06-11 $\\n]
[-author?Glenn Fowler <gsf@research.att.com>]
[-author?David Korn <dgk@research.att.com>]
[-copyright?Copyright (c) 1992-2005 AT&T Corp.]
[-license?http://www.opensource.org/licenses/cpl1.0.txt]
[--catalog?libcmd]
[+NAME?id - return user identity]
[+DESCRIPTION?If no \auser\a operand is specified \bid\b writes user
and group IDs and the corresponding user and group names of the invoking
process to standard output.  If the effective and real IDs do not match,
both are written.  Any supplementary groups the current process belongs
to will also be written.]
[+?If a \auser\a operand is specified and the process has permission,
the user and group IDs and any supplementary group IDs of the selected
user will be written to standard output.]
[+?If any options are specified, then only a portion of the information
is written.]
[n:name?Write the name instead of the numeric ID.]
[r:real?Writes real ID instead of the effective ID.]
[[a?This option is ignored.]
[g:group?Writes only the group ID.]
[u:user?Writes only the user ID.]
[G:groups?Writes only the supplementary group IDs.]
[s:fair-share?Writes fair share scheduler IDs and groups on systems that support fair share scheduling.]

[user]

[+EXIT STATUS?]{[+0?Successful completion.][+>0?An error occurred.]}[+SEE ALSO?\blogname\b(1), \bwho\b(1), \bgetgroups\b(2)]'
		OUTPUT - $'[-?\\n@(#)$Id: id (AT&T Research) 2004-06-11 $
    ]
[-author?Glenn Fowler <gsf@research.att.com>]
[-author?David Korn <dgk@research.att.com>]
[-copyright?Copyright (c) 1992-2005 AT&T Corp.]
[-license?http://www.opensource.org/licenses/cpl1.0.txt]
[--catalog?libcmd]
[+NAME?id - return user identity]
[+DESCRIPTION?If no \\auser\\a operand is specified \\bid\\b writes user and
    group IDs and the corresponding user and group names of the invoking
    process to standard output. If the effective and real IDs do not match,
    both are written. Any supplementary groups the current process belongs to
    will also be written.]
[+?If a \\auser\\a operand is specified and the process has permission, the
    user and group IDs and any supplementary group IDs of the selected user
    will be written to standard output.]
[+?If any options are specified, then only a portion of the information is
    written.]
[n:name?Write the name instead of the numeric ID.]
[r:real?Writes real ID instead of the effective ID.]
[[a?This option is ignored.]
[g:group?Writes only the group ID.]
[u:user?Writes only the user ID.]
[G:groups?Writes only the supplementary group IDs.]
[s:fair-share?Writes fair share scheduler IDs and groups on systems that
    support fair share scheduling.]
[user]
[+EXIT STATUS?]
    {
        [+0?Successful completion.]
        [+>0?An error occurred.]
    }
[+SEE ALSO?\blogname\b(1), \\bwho\\b(1), \\bgetgroups\\b(2)]'
